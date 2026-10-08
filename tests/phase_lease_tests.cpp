#include "StrongholdPhaseLease.h"

#include <iostream>
#include <vector>

using namespace NaxxGuildStrongholds;

int main()
{
    int n = 0, failed = 0;
    auto expect = [&](bool good, char const* what)
    {
        ++n;
        if (!good)
        {
            ++failed;
            std::cerr << "FAILED: " << what << '\n';
        }
    };

    PhasePoolApproval approval{true, true, true, (1u << 24) | (1u << 25), 1u};
    std::vector<PhaseLease> leases;
    auto first = ProposePhaseLease(approval, 101, 1790000001, leases);
    expect(first.Decision == LeaseDecision::Proposed &&
        first.Proposed.PhaseBit == (1u << 24), "First approved unused bit");
    leases.push_back(first.Proposed);

    auto second = ProposePhaseLease(approval, 102, 1790000002, leases);
    expect(second.Decision == LeaseDecision::Proposed &&
        second.Proposed.PhaseBit == (1u << 25), "Different guild gets distinct bit");
    leases.push_back(second.Proposed);
    expect(ProposePhaseLease(approval, 103, 1790000003, leases).Decision ==
        LeaseDecision::NoCapacity, "Exhaustion denies new guild rather than sharing");
    expect(ProposePhaseLease(approval, 101, 1790000001, leases).Decision ==
        LeaseDecision::AlreadyHeld, "Existing guild cannot duplicate lease");
    expect(ProposePhaseLease(approval, 101, 1790000999, leases).Decision ==
        LeaseDecision::OtherGenerationOwnsId, "Recycled guild ID cannot inherit lease");

    approval.SourceAndModuleInventoryVerified = false;
    expect(ProposePhaseLease(approval, 103, 1790000003, leases).Decision ==
        LeaseDecision::InventoryUnverified, "Unknown installed phase use refuses allocation");
    approval.SourceAndModuleInventoryVerified = true;
    approval.CombinedPhaseComparisonVerified = false;
    expect(ProposePhaseLease(approval, 103, 1790000003, leases).Decision ==
        LeaseDecision::PhaseModeUnverified, "Exact/combined mismatch refuses");
    approval.CombinedPhaseComparisonVerified = true;
    approval.PropertyAndGuildVerified = false;
    expect(ProposePhaseLease(approval, 103, 1790000003, leases).Decision ==
        LeaseDecision::PropertyUnverified, "Unverified property denies lease");
    approval.PropertyAndGuildVerified = true;

    approval.ObservedExternalBits = 1u << 24;
    expect(ProposePhaseLease(approval, 103, 1790000003, leases).Decision ==
        LeaseDecision::ReservedBitConflict, "Previously approved bit now used externally blocks entire pool");
    approval.ObservedExternalBits = 1u;
    expect(ProposePhaseLease(approval, 0, 1790000003, leases).Decision ==
        LeaseDecision::InvalidGuildIdentity, "Guild zero denied");
    expect(ProposePhaseLease(approval, 103, 0, leases).Decision ==
        LeaseDecision::InvalidGuildIdentity, "Unknown guild generation denied");

    approval.ExplicitApprovedBits = 0;
    expect(ProposePhaseLease(approval, 103, 1790000003, leases).Decision ==
        LeaseDecision::NoCapacity, "No magic default bits");
    approval.ExplicitApprovedBits = (1u << 24) | (1u << 25);

    auto malformed = leases;
    malformed.push_back({105, 1790000005, 3, LeaseStatus::Held});
    expect(ProposePhaseLease(approval, 106, 1790000006, malformed).Decision ==
        LeaseDecision::InvalidExistingLease, "Combined-bit existing reservation corrupt");
    malformed.back().PhaseBit = leases[0].PhaseBit;
    expect(ProposePhaseLease(approval, 106, 1790000006, malformed).Decision ==
        LeaseDecision::InvalidExistingLease, "Duplicate phase lease corrupt");

    expect(CheckLeaseRetirement(leases[0], 101, 1790000001, false) ==
        LeaseDecision::PropertyUnverified, "Disband proof required to retire");
    expect(CheckLeaseRetirement(leases[0], 101, 1790000001, true) ==
        LeaseDecision::Proposed, "Verified original owner may retire lease");
    expect(CheckLeaseRetirement(leases[0], 101, 1790000002, true) ==
        LeaseDecision::OtherGenerationOwnsId, "Successor cannot retire original lease");

    leases[0].Status = LeaseStatus::Retired;
    expect(CheckLeaseRetirement(leases[0], 101, 1790000001, true) ==
        LeaseDecision::PreviouslyRetired, "Repeat retirement idempotent");
    expect(ProposePhaseLease(approval, 101, 1790000001, leases).Decision ==
        LeaseDecision::PreviouslyRetired, "Archived owner cannot reopen automatically");
    expect(ProposePhaseLease(approval, 103, 1790000003, leases).Decision ==
        LeaseDecision::NoCapacity, "Retired bit never silently recycled");

    if (failed)
    {
        std::cerr << failed << " of " << n << " phase lease checks failed\n";
        return 1;
    }
    std::cout << "PASS: " << n << " staging-only phase reservation domain checks\n";
    return 0;
}
