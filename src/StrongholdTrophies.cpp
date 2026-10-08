#include "StrongholdTrophies.h"

#include <array>
#include <cctype>
#include <string_view>
#include <unordered_set>

namespace NaxxGuildStrongholds
{
namespace
{
// These are *conceptual* game/lore encounter names. No map/creature ID is
// wired up; do not confuse WotLK and original 40-player encounter variants.
constexpr std::array<Trophy, 5> kTrophies{{
    {"onyxia_head", "Onyxia's Head", "classic_onyxia", "Mounted dragon head"},
    {"ragnaros_flame", "Ragnaros' Flame", "classic_ragnaros", "Molten Core brazier"},
    {"nefarian_banner", "Nefarian's Banner", "classic_nefarian", "Blackwing victory banner"},
    {"cthun_relic", "C'Thun Relic", "classic_cthun", "Ahn'Qiraj sealed relic"},
    {"kelthuzad_sigil", "Kel'Thuzad Sigil", "classic_kelthuzad", "Naxxramas memorial sigil"}
}};

bool ValidReceipt(std::string const& key)
{
    if (key.size() < 16 || key.size() > 80)
        return false;
    for (unsigned char c : key)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == ':'))
            return false;
    return true;
}

bool SameGuild(GuildIdentity const& a, GuildIdentity const& b)
{
    return a.GuildId != 0 && a.CreatedAt != 0 &&
           a.GuildId == b.GuildId && a.CreatedAt == b.CreatedAt;
}

TrophyUnlockDecision FromLifetime(LifecycleDecision d)
{
    switch (d)
    {
        case LifecycleDecision::DifferentGuild:
            return TrophyUnlockDecision::WrongGuild;
        case LifecycleDecision::WrongGeneration:
            return TrophyUnlockDecision::WrongGeneration;
        case LifecycleDecision::Archived:
            return TrophyUnlockDecision::Archived;
        default:
            return TrophyUnlockDecision::PropertyUnverified;
    }
}

TrophyPlacementDecision FromPlacementLifetime(LifecycleDecision d)
{
    switch (d)
    {
        case LifecycleDecision::DifferentGuild:
            return TrophyPlacementDecision::WrongGuild;
        case LifecycleDecision::WrongGeneration:
            return TrophyPlacementDecision::WrongGeneration;
        case LifecycleDecision::Archived:
            return TrophyPlacementDecision::Archived;
        default:
            return TrophyPlacementDecision::PropertyUnverified;
    }
}
}

std::array<Trophy, 5> const& GetTrophyCatalog()
{
    return kTrophies;
}

Trophy const* FindTrophy(std::string_view key)
{
    for (Trophy const& trophy : kTrophies)
        if (trophy.Key == key)
            return &trophy;
    return nullptr;
}

bool ValidateTrophyCatalog()
{
    std::unordered_set<std::string_view> keys;
    std::unordered_set<std::string_view> encounters;
    for (Trophy const& trophy : kTrophies)
        if (trophy.Key.empty() || trophy.DisplayName.empty() ||
            trophy.EncounterKey.empty() || trophy.DecorationConcept.empty() ||
            !keys.insert(trophy.Key).second ||
            !encounters.insert(trophy.EncounterKey).second)
            return false;
    return true;
}

TrophyUnlockProposal ProposeTrophyUnlock(
    Trophy const* trophy, TrophyGuildEligibility const& guild,
    TrophyKillProof const& kill)
{
    TrophyUnlockProposal result;
    if (!kill.FeatureEnabled)
        return result;
    if (!trophy || FindTrophy(trophy->Key) != trophy)
    {
        result.Decision = TrophyUnlockDecision::UnknownTrophy;
        return result;
    }
    if (!guild.GuildIdentityVerified || !guild.Owner.GuildId ||
        !guild.Owner.CreatedAt)
    {
        result.Decision = TrophyUnlockDecision::GuildUnverified;
        return result;
    }
    if (!guild.PropertySnapshotVerified)
    {
        result.Decision = TrophyUnlockDecision::PropertyUnverified;
        return result;
    }
    LifecycleDecision life =
        CheckPropertyLifetime(guild.Property, guild.Owner, true);
    if (life != LifecycleDecision::Allowed)
    {
        result.Decision = FromLifetime(life);
        return result;
    }
    if (!kill.ServerBossKillConfirmed || !kill.EncounterIdentityVerified ||
        !kill.RaidRosterVerified || !kill.UniqueServerEventVerified)
    {
        result.Decision = TrophyUnlockDecision::KillUnverified;
        return result;
    }
    if (kill.EncounterKey != trophy->EncounterKey)
    {
        result.Decision = TrophyUnlockDecision::EncounterMismatch;
        return result;
    }
    if (!kill.RaidMapId || !kill.RaidInstanceId)
    {
        result.Decision = TrophyUnlockDecision::InvalidRaidInstance;
        return result;
    }
    if (!ValidReceipt(kill.EventReceipt))
    {
        result.Decision = TrophyUnlockDecision::InvalidEventReceipt;
        return result;
    }
    if (!guild.ExistingReceiptLookupVerified ||
        !guild.ExistingUnlockLookupVerified)
    {
        result.Decision = TrophyUnlockDecision::UnlockLookupUnverified;
        return result;
    }
    if (guild.ReceiptAlreadyCommitted)
    {
        result.Decision = TrophyUnlockDecision::DuplicateEventReceipt;
        return result;
    }
    if (guild.AlreadyUnlocked)
    {
        result.Decision = TrophyUnlockDecision::AlreadyUnlocked;
        return result;
    }
    if (guild.MinimumGuildHumans < 2 || guild.MinimumGuildHumans > 40)
    {
        result.Decision = TrophyUnlockDecision::InvalidParticipationPolicy;
        return result;
    }
    if (kill.Roster.empty() || kill.Roster.size() > 40)
    {
        result.Decision = TrophyUnlockDecision::InvalidRoster;
        return result;
    }

    std::unordered_set<std::uint64_t> uniqueCharacters;
    std::uint8_t qualified = 0;
    for (TrophyParticipant const& person : kill.Roster)
    {
        if (!person.CharacterGuid ||
            !uniqueCharacters.insert(person.CharacterGuid).second)
        {
            result.Decision = TrophyUnlockDecision::InvalidRoster;
            return result;
        }

        // Other verified guilds can take part in the SAME raid and can be
        // evaluated separately. Bots can take part but do NOT count towards
        // the minimum number of participating guild players.
        if (SameGuild(person.Guild, guild.Owner) &&
            person.GuildMembershipVerified &&
            person.EncounterParticipationVerified &&
            !person.IsPlayerbot)
            ++qualified;
    }
    result.VerifiedHumanGuildParticipants = qualified;
    if (qualified < guild.MinimumGuildHumans)
    {
        result.Decision = TrophyUnlockDecision::InsufficientGuildHumans;
        return result;
    }

    // This is ONLY a domain proposal. A future adapter must atomically
    // persist a generation-qualified unlock + unique kill receipt.
    result.Decision = TrophyUnlockDecision::ProposalOnly;
    result.Owner = guild.Owner;
    result.UnlockKey = std::string(trophy->Key);
    result.EventReceipt = kill.EventReceipt;
    return result;
}

bool IsDraftTrophySlot(std::string_view slot)
{
    return slot == "hall_trophy_1" || slot == "hall_trophy_2" ||
           slot == "hall_trophy_3" || slot == "hall_trophy_4" ||
           slot == "hall_trophy_5";
}

TrophyPlacementProposal ProposeTrophyPlacement(
    Trophy const* trophy, TrophyPlacementRequest const& request)
{
    TrophyPlacementProposal result;
    if (!request.FeatureEnabled)
        return result;
    if (!trophy || FindTrophy(trophy->Key) != trophy)
    {
        result.Decision = TrophyPlacementDecision::UnknownTrophy;
        return result;
    }
    if (!request.IdentityVerified || !request.Owner.GuildId ||
        !request.Owner.CreatedAt)
    {
        result.Decision = TrophyPlacementDecision::GuildUnverified;
        return result;
    }
    if (!request.PropertySnapshotVerified)
    {
        result.Decision = TrophyPlacementDecision::PropertyUnverified;
        return result;
    }
    LifecycleDecision life =
        CheckPropertyLifetime(request.Property, request.Owner, true);
    if (life != LifecycleDecision::Allowed)
    {
        result.Decision = FromPlacementLifetime(life);
        return result;
    }
    if (!request.UnlockLookupVerified)
    {
        result.Decision = TrophyPlacementDecision::UnlockUnverified;
        return result;
    }
    if (!request.TrophyUnlockedForOriginalGeneration)
    {
        result.Decision = TrophyPlacementDecision::NotUnlocked;
        return result;
    }
    if (!IsDraftTrophySlot(request.SlotKey))
    {
        result.Decision = TrophyPlacementDecision::InvalidSlot;
        return result;
    }
    if (!request.SlotLookupVerified)
    {
        result.Decision = TrophyPlacementDecision::SlotLookupUnverified;
        return result;
    }
    if (request.SlotOccupied)
    {
        result.Decision = TrophyPlacementDecision::SlotOccupied;
        return result;
    }
    if (!request.DecorationAssetReviewed || !request.ReviewedObjectEntry)
    {
        result.Decision = TrophyPlacementDecision::UnapprovedAsset;
        return result;
    }
    result.Decision = TrophyPlacementDecision::ProposalOnly;
    result.TrophyKey = std::string(trophy->Key);
    result.SlotKey = request.SlotKey;
    return result;
}
}
