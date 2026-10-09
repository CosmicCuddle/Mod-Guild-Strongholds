#include "StrongholdTrophies.h"

#include <iostream>
#include <string>
#include <vector>

using namespace NaxxGuildStrongholds;

int main()
{
    unsigned checks = 0;
    unsigned failures = 0;
    auto check = [&](bool yes, char const* reason) {
        ++checks;
        if (!yes) {
            ++failures;
            std::cerr << "FAILED: " << reason << '\n';
        }
    };

    check(ValidateTrophyCatalog(), "Unique nonempty encounter and trophy catalogue");
    check(GetTrophyCatalog().size() == 5, "Five symbolic raid trophies");
    for (Trophy const& trophy : GetTrophyCatalog())
        check(FindTrophy(trophy.Key) == &trophy, "Catalogue lookup uses owned definition");
    check(!FindTrophy("future_fake_encounter"), "Unknown encounter is not credited");

    Trophy const* onyxia = FindTrophy("onyxia_head");
    TrophyGuildEligibility guild;
    guild.Owner = {21, 1777000000};
    guild.GuildIdentityVerified = true;
    guild.PropertySnapshotVerified = true;
    guild.Property.OriginalGuild = guild.Owner;
    guild.Property.State = PropertyLifecycle::Active;
    guild.ExistingReceiptLookupVerified = true;
    guild.ExistingUnlockLookupVerified = true;
    guild.MinimumGuildHumans = 2;

    TrophyKillProof kill;
    kill.EncounterKey = "classic_onyxia";
    kill.RaidMapId = 249;
    kill.RaidInstanceId = 123;
    kill.EventReceipt = "raid:onyxia:kill:instance123:boss1";
    kill.ServerBossKillConfirmed = true;
    kill.EncounterIdentityVerified = true;
    kill.RaidRosterVerified = true;
    kill.UniqueServerEventVerified = true;
    kill.Roster = {
        {1001, {21, 1777000000}, true, true, false},
        {1002, {21, 1777000000}, true, true, false},
        {1003, {44, 1777000001}, true, true, false},
        {1004, {44, 1777000001}, true, true, false},
        {1005, {21, 1777000000}, true, true, true}
    };

    auto result = ProposeTrophyUnlock(onyxia, guild, kill);
    check(result.Decision == TrophyUnlockDecision::Disabled,
          "Feature permanently closed by default");

    kill.FeatureEnabled = true;
    result = ProposeTrophyUnlock(onyxia, guild, kill);
    check(result.Decision == TrophyUnlockDecision::ProposalOnly &&
          result.VerifiedHumanGuildParticipants == 2,
          "Verified human members credit proposal, bot excluded");
    check(result.UnlockKey == "onyxia_head" &&
          result.EventReceipt == kill.EventReceipt &&
          result.Owner.CreatedAt == guild.Owner.CreatedAt && !result.RewardGranted,
          "Proposal carries provenance but grants absolutely nothing");

    auto otherGuild = guild;
    otherGuild.Owner = {44, 1777000001};
    otherGuild.Property.OriginalGuild = otherGuild.Owner;
    auto other = ProposeTrophyUnlock(onyxia, otherGuild, kill);
    check(other.Decision == TrophyUnlockDecision::ProposalOnly &&
          other.VerifiedHumanGuildParticipants == 2,
          "Mixed guild raid can independently qualify two guilds");

    auto missing = guild;
    missing.MinimumGuildHumans = 3;
    check(ProposeTrophyUnlock(onyxia, missing, kill).Decision ==
          TrophyUnlockDecision::InsufficientGuildHumans,
          "Guild threshold counts humans, not bots or other guilds");
    missing.MinimumGuildHumans = 0;
    check(ProposeTrophyUnlock(onyxia, missing, kill).Decision ==
          TrophyUnlockDecision::InvalidParticipationPolicy,
          "No implicit or zero participant threshold");
    missing.MinimumGuildHumans = 41;
    check(ProposeTrophyUnlock(onyxia, missing, kill).Decision ==
          TrophyUnlockDecision::InvalidParticipationPolicy,
          "Maximum raid size is 40");

    auto wrongGeneration = guild;
    wrongGeneration.Owner.CreatedAt += 1;
    check(ProposeTrophyUnlock(onyxia, wrongGeneration, kill).Decision ==
          TrophyUnlockDecision::WrongGeneration,
          "Reused guild ID cannot inherit old property or trophy");
    wrongGeneration = guild;
    wrongGeneration.Property.OriginalGuild.GuildId = 999;
    check(ProposeTrophyUnlock(onyxia, wrongGeneration, kill).Decision ==
          TrophyUnlockDecision::WrongGuild,
          "Another guild's property rejected");
    wrongGeneration = guild;
    wrongGeneration.Property.State = PropertyLifecycle::Archived;
    check(ProposeTrophyUnlock(onyxia, wrongGeneration, kill).Decision ==
          TrophyUnlockDecision::Archived,
          "Archived guild cannot earn trophies");

    auto noProof = guild;
    noProof.GuildIdentityVerified = false;
    check(ProposeTrophyUnlock(onyxia, noProof, kill).Decision ==
          TrophyUnlockDecision::GuildUnverified, "Unverified guild rejected");
    noProof = guild;
    noProof.PropertySnapshotVerified = false;
    check(ProposeTrophyUnlock(onyxia, noProof, kill).Decision ==
          TrophyUnlockDecision::PropertyUnverified, "Missing property lookup rejected");
    noProof = guild;
    noProof.ExistingUnlockLookupVerified = false;
    check(ProposeTrophyUnlock(onyxia, noProof, kill).Decision ==
          TrophyUnlockDecision::UnlockLookupUnverified, "Unverified uniqueness lookup blocked");
    noProof = guild;
    noProof.ExistingReceiptLookupVerified = false;
    check(ProposeTrophyUnlock(onyxia, noProof, kill).Decision ==
          TrophyUnlockDecision::UnlockLookupUnverified, "Unverified receipt lookup blocked");
    noProof = guild;
    noProof.AlreadyUnlocked = true;
    check(ProposeTrophyUnlock(onyxia, noProof, kill).Decision ==
          TrophyUnlockDecision::AlreadyUnlocked, "One guild gets one trophy unlock");
    noProof = guild;
    noProof.ReceiptAlreadyCommitted = true;
    check(ProposeTrophyUnlock(onyxia, noProof, kill).Decision ==
          TrophyUnlockDecision::DuplicateEventReceipt, "Replayed kill receipt blocked");

    auto bad = kill;
    bad.ServerBossKillConfirmed = false;
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::KillUnverified, "Fake kill denied");
    bad = kill;
    bad.EncounterIdentityVerified = false;
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::KillUnverified, "Unverified boss identity denied");
    bad = kill;
    bad.EncounterKey = "wotlk_onyxia";
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::EncounterMismatch,
          "WotLK Onyxia cannot become vanilla Onyxia trophy");
    bad = kill;
    bad.RaidInstanceId = 0;
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InvalidRaidInstance,
          "Unverified instance zero denied");
    bad = kill;
    bad.EventReceipt = "player_input";
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InvalidEventReceipt,
          "Short player-origin receipt denied");
    bad = kill;
    bad.EventReceipt += ";DROP TABLE";
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InvalidEventReceipt,
          "Unsafe event token characters denied");
    bad = kill;
    bad.Roster[0].CharacterGuid = bad.Roster[1].CharacterGuid;
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InvalidRoster, "Duplicate raid character GUID denied");
    bad = kill;
    bad.Roster[0].EncounterParticipationVerified = false;
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InsufficientGuildHumans,
          "Guildmates not credited for absent encounter participation");
    bad = kill;
    bad.Roster[0].Guild.CreatedAt = 1777000777;
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InsufficientGuildHumans,
          "Recycled guild-generation raid member not credited");
    bad = kill;
    bad.Roster[0].GuildMembershipVerified = false;
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InsufficientGuildHumans,
          "Unverified member excluded from proof");
    bad = kill;
    bad.Roster.resize(41);
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InvalidRoster, "Raid roster cannot exceed 40");
    bad = kill;
    bad.Roster.clear();
    check(ProposeTrophyUnlock(onyxia, guild, bad).Decision ==
          TrophyUnlockDecision::InvalidRoster, "Empty raid has no guild kill proof");

    TrophyKillProof forty = kill;
    forty.Roster.clear();
    for (std::uint64_t i = 1; i <= 40; ++i)
        forty.Roster.push_back({i, i <= 3 ? guild.Owner : GuildIdentity{44, 1777000001},
                               true, true, i == 3});
    auto fortyResult = ProposeTrophyUnlock(onyxia, guild, forty);
    check(fortyResult.Decision == TrophyUnlockDecision::ProposalOnly &&
          fortyResult.VerifiedHumanGuildParticipants == 2,
          "Forty-player raid supports mixed guilds and bots");

    TrophyPlacementRequest place;
    place.Owner = guild.Owner;
    place.IdentityVerified = true;
    place.PropertySnapshotVerified = true;
    place.Property = guild.Property;
    place.UnlockLookupVerified = true;
    place.TrophyUnlockedForOriginalGeneration = true;
    place.SlotLookupVerified = true;
    place.DecorationAssetReviewed = true;
    place.ReviewedObjectEntry = 1; // synthetic test entry, NEVER a real asset
    place.SlotKey = "hall_trophy_1";
    check(ProposeTrophyPlacement(onyxia, place).Decision ==
          TrophyPlacementDecision::Disabled, "Placement disabled by default");
    place.FeatureEnabled = true;
    auto p = ProposeTrophyPlacement(onyxia, place);
    check(p.Decision == TrophyPlacementDecision::ProposalOnly &&
          p.TrophyKey == "onyxia_head" && p.SlotKey == place.SlotKey &&
          !p.ObjectSpawned,
          "Approved synthetic placement is still NOT a spawned object");
    place.UnlockLookupVerified = false;
    check(ProposeTrophyPlacement(onyxia, place).Decision ==
          TrophyPlacementDecision::UnlockUnverified, "Unknown earned trophy blocked");
    place.UnlockLookupVerified = true;
    place.TrophyUnlockedForOriginalGeneration = false;
    check(ProposeTrophyPlacement(onyxia, place).Decision ==
          TrophyPlacementDecision::NotUnlocked, "Unawarded trophy cannot be placed");
    place.TrophyUnlockedForOriginalGeneration = true;
    place.SlotKey = "guild_bank_slot";
    check(ProposeTrophyPlacement(onyxia, place).Decision ==
          TrophyPlacementDecision::InvalidSlot, "Non-trophy plot denied");
    place.SlotKey = "hall_trophy_2";
    place.SlotOccupied = true;
    check(ProposeTrophyPlacement(onyxia, place).Decision ==
          TrophyPlacementDecision::SlotOccupied, "Occupied trophy slot protected");
    place.SlotOccupied = false;
    place.DecorationAssetReviewed = false;
    check(ProposeTrophyPlacement(onyxia, place).Decision ==
          TrophyPlacementDecision::UnapprovedAsset, "No unreviewed GO entries allowed");
    place.DecorationAssetReviewed = true;
    place.Property.State = PropertyLifecycle::Archived;
    check(ProposeTrophyPlacement(onyxia, place).Decision ==
          TrophyPlacementDecision::Archived, "Archive revokes decoration placement");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks
              << " guild raid trophy unlock/placement policy checks\n";
    return 0;
}
