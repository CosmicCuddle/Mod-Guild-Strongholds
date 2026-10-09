#include "StrongholdPhaseLease.h"
#include "StrongholdIsolationProbe.h"

namespace NaxxGuildStrongholds
{
namespace
{
LeasePlan Reject(LeaseDecision status)
{
    LeasePlan plan;
    plan.Decision = status;
    return plan;
}
}

LeasePlan ProposePhaseLease(PhasePoolApproval const& approval,
    std::uint32_t guildId, std::uint64_t guildCreatedAt,
    std::vector<PhaseLease> const& existing)
{
    if (!approval.SourceAndModuleInventoryVerified)
        return Reject(LeaseDecision::InventoryUnverified);
    if (!approval.CombinedPhaseComparisonVerified)
        return Reject(LeaseDecision::PhaseModeUnverified);
    if (!approval.PropertyAndGuildVerified)
        return Reject(LeaseDecision::PropertyUnverified);
    if (!guildId || !guildCreatedAt)
        return Reject(LeaseDecision::InvalidGuildIdentity);
    if (!approval.ExplicitApprovedBits)
        return Reject(LeaseDecision::NoCapacity);
    if (approval.ExplicitApprovedBits & approval.ObservedExternalBits)
        return Reject(LeaseDecision::ReservedBitConflict);

    std::uint32_t used = 0;
    for (PhaseLease const& lease : existing)
    {
        if (!lease.GuildId || !lease.GuildCreatedAt ||
            !IsSingleBitPhase(lease.PhaseBit) || (used & lease.PhaseBit))
            return Reject(LeaseDecision::InvalidExistingLease);
        used |= lease.PhaseBit;
        if (lease.GuildId == guildId)
        {
            if (lease.GuildCreatedAt != guildCreatedAt)
                return Reject(LeaseDecision::OtherGenerationOwnsId);
            return Reject(lease.Status == LeaseStatus::Held ?
                LeaseDecision::AlreadyHeld : LeaseDecision::PreviouslyRetired);
        }
    }

    std::uint32_t available =
        approval.ExplicitApprovedBits & ~approval.ObservedExternalBits & ~used;
    if (!available)
        return Reject(LeaseDecision::NoCapacity);

    // Lowest available bit: stable, bounded selection; no modulo hashing
    // based on guild ID, so the same phase can never be offered twice.
    std::uint32_t chosen = available & (~available + 1u);
    LeasePlan plan;
    plan.Decision = LeaseDecision::Proposed;
    plan.Proposed = {guildId, guildCreatedAt, chosen, LeaseStatus::Held};
    return plan;
}

LeaseDecision CheckLeaseRetirement(PhaseLease const& lease,
    std::uint32_t guildId, std::uint64_t guildCreatedAt,
    bool disbandOrAdminVerified)
{
    if (!disbandOrAdminVerified)
        return LeaseDecision::PropertyUnverified;
    if (!guildId || !guildCreatedAt || !lease.GuildId ||
        !lease.GuildCreatedAt || !IsSingleBitPhase(lease.PhaseBit))
        return LeaseDecision::InvalidGuildIdentity;
    if (lease.GuildId != guildId || lease.GuildCreatedAt != guildCreatedAt)
        return LeaseDecision::OtherGenerationOwnsId;
    return lease.Status == LeaseStatus::Retired ?
        LeaseDecision::PreviouslyRetired : LeaseDecision::Proposed;
}
}
