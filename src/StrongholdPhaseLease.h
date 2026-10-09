#ifndef NAXX_GUILD_STRONGHOLD_PHASE_LEASE_H
#define NAXX_GUILD_STRONGHOLD_PHASE_LEASE_H

#include <cstdint>
#include <vector>

// NON-PLAYABLE phase-slot reservation proposal.
// Never writes WorldObject phase masks, spawns objects or enables housing.
// Only designed for a specifically reserved staging zone in COMBINED bit mode.
// The pool is supplied by a *verified administrator inventory*, not guessed.
namespace NaxxGuildStrongholds
{
enum class LeaseStatus : std::uint8_t { Held, Retired };

struct PhaseLease
{
    std::uint32_t GuildId = 0;
    std::uint64_t GuildCreatedAt = 0;
    std::uint32_t PhaseBit = 0;
    LeaseStatus Status = LeaseStatus::Held;
};

struct PhasePoolApproval
{
    bool SourceAndModuleInventoryVerified = false;
    bool CombinedPhaseComparisonVerified = false;
    bool PropertyAndGuildVerified = false;
    std::uint32_t ExplicitApprovedBits = 0;
    std::uint32_t ObservedExternalBits = 0;
};

enum class LeaseDecision : std::uint8_t
{
    Proposed,
    InventoryUnverified,
    PhaseModeUnverified,
    PropertyUnverified,
    InvalidGuildIdentity,
    InvalidExistingLease,
    OtherGenerationOwnsId,
    AlreadyHeld,
    PreviouslyRetired,
    ReservedBitConflict,
    NoCapacity
};

struct LeasePlan
{
    LeaseDecision Decision = LeaseDecision::InventoryUnverified;
    PhaseLease Proposed;
    // The database must independently enforce UNIQUE(guild_id) and
    // UNIQUE(phase_bit). This domain plan does NOT commit a lease.
};

// Retired leases deliberately remain reserved to prevent unsafe reuse.
LeasePlan ProposePhaseLease(PhasePoolApproval const& approval,
    std::uint32_t guildId, std::uint64_t guildCreatedAt,
    std::vector<PhaseLease> const& existing);

LeaseDecision CheckLeaseRetirement(PhaseLease const& lease,
    std::uint32_t guildId, std::uint64_t guildCreatedAt,
    bool disbandOrAdminVerified);

}

#endif
