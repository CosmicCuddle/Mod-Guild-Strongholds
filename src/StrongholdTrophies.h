#ifndef NAXX_GUILD_STRONGHOLD_TROPHIES_H
#define NAXX_GUILD_STRONGHOLD_TROPHIES_H

#include "StrongholdLifecycle.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// GAMEPLAY DOMAIN ONLY. No AzerothCore kill hooks, SQL, GO ids or spawns.
// A proposed trophy is NOT earned or placeable until authoritative raid
// receipts, private guild areas, and persistence adapters exist.
namespace NaxxGuildStrongholds
{
struct Trophy
{
    std::string_view Key;
    std::string_view DisplayName;
    std::string_view EncounterKey;  // symbolic until core map/boss IDs audited
    std::string_view DecorationConcept;
};

std::array<Trophy, 5> const& GetTrophyCatalog();
Trophy const* FindTrophy(std::string_view key);
bool ValidateTrophyCatalog();

struct TrophyParticipant
{
    std::uint64_t CharacterGuid = 0;
    GuildIdentity Guild;
    bool GuildMembershipVerified = false;
    bool EncounterParticipationVerified = false;
    bool IsPlayerbot = false;
};

struct TrophyKillProof
{
    bool FeatureEnabled = false; // Always false in current runtime
    bool ServerBossKillConfirmed = false;
    bool EncounterIdentityVerified = false;
    bool RaidRosterVerified = false;
    bool UniqueServerEventVerified = false;
    std::uint32_t RaidMapId = 0;
    std::uint32_t RaidInstanceId = 0;
    std::string EncounterKey;
    std::string EventReceipt;
    std::vector<TrophyParticipant> Roster;
};

struct TrophyGuildEligibility
{
    GuildIdentity Owner;
    bool GuildIdentityVerified = false;
    bool PropertySnapshotVerified = false;
    PropertyLifetime Property;
    bool ExistingUnlockLookupVerified = false;
    bool AlreadyUnlocked = false;
    bool ExistingReceiptLookupVerified = false;
    bool ReceiptAlreadyCommitted = false;
    std::uint8_t MinimumGuildHumans = 0; // Explicit 2..40 policy, no implicit default
};

enum class TrophyUnlockDecision : std::uint8_t
{
    Disabled,
    UnknownTrophy,
    GuildUnverified,
    WrongGuild,
    WrongGeneration,
    Archived,
    PropertyUnverified,
    KillUnverified,
    EncounterMismatch,
    InvalidRaidInstance,
    InvalidEventReceipt,
    DuplicateEventReceipt,
    UnlockLookupUnverified,
    AlreadyUnlocked,
    InvalidRoster,
    InvalidParticipationPolicy,
    InsufficientGuildHumans,
    ProposalOnly
};

struct TrophyUnlockProposal
{
    TrophyUnlockDecision Decision = TrophyUnlockDecision::Disabled;
    std::string UnlockKey;
    std::string EventReceipt;
    std::uint8_t VerifiedHumanGuildParticipants = 0;
    GuildIdentity Owner;
    // No proposal ever transfers rewards or authorizes a world spawn.
    bool RewardGranted = false;
};

TrophyUnlockProposal ProposeTrophyUnlock(
    Trophy const* trophy, TrophyGuildEligibility const& guild,
    TrophyKillProof const& kill);

// Logical locations only. There is NO known-safe client GO or housing map ID.
bool IsDraftTrophySlot(std::string_view slot);

enum class TrophyPlacementDecision : std::uint8_t
{
    Disabled,
    UnknownTrophy,
    GuildUnverified,
    PropertyUnverified,
    WrongGuild,
    WrongGeneration,
    Archived,
    UnlockUnverified,
    NotUnlocked,
    InvalidSlot,
    SlotLookupUnverified,
    SlotOccupied,
    UnapprovedAsset,
    ProposalOnly
};

struct TrophyPlacementRequest
{
    bool FeatureEnabled = false;
    GuildIdentity Owner;
    bool IdentityVerified = false;
    bool PropertySnapshotVerified = false;
    PropertyLifetime Property;
    bool UnlockLookupVerified = false;
    bool TrophyUnlockedForOriginalGeneration = false;
    bool SlotLookupVerified = false;
    bool SlotOccupied = false;
    bool DecorationAssetReviewed = false;
    std::uint32_t ReviewedObjectEntry = 0; // Never populated by this module
    std::string SlotKey;
};

struct TrophyPlacementProposal
{
    TrophyPlacementDecision Decision = TrophyPlacementDecision::Disabled;
    std::string TrophyKey;
    std::string SlotKey;
    // The only successful result is STILL a plan; no GO is spawned.
    bool ObjectSpawned = false;
};

TrophyPlacementProposal ProposeTrophyPlacement(
    Trophy const* trophy, TrophyPlacementRequest const& request);
}
#endif
