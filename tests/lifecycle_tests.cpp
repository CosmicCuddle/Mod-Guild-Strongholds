#include "StrongholdLifecycle.h"

#include <iostream>
#include <limits>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0;
    int failures = 0;
    auto check = [&](bool truth, const char* label)
    {
        ++checks;
        if (!truth)
        {
            ++failures;
            std::cerr << "FAILED: " << label << '\n';
        }
    };

    PropertyLifetime first{{42, 1790000000}, PropertyLifecycle::Active, 5};
    GuildIdentity current{42, 1790000000};
    GuildIdentity recreated{42, 1791000000};
    GuildIdentity other{43, 1790000000};

    check(CheckPropertyLifetime(first, current, true) == LifecycleDecision::Allowed,
        "Active original generation can access");
    check(CheckPropertyLifetime(first, current, false) == LifecycleDecision::IdentityUnverified,
        "Unverified identity always denied");
    check(CheckPropertyLifetime(first, recreated, true) == LifecycleDecision::WrongGeneration,
        "Recreated guild may not inherit old property");
    check(CheckPropertyLifetime(first, other, true) == LifecycleDecision::DifferentGuild,
        "Another guild may not access");
    check(CheckPropertyLifetime(first, {42, 0}, true) == LifecycleDecision::InvalidIdentity,
        "Unknown creation date denied");
    check(CheckPropertyLifetime({{0, 10}, PropertyLifecycle::Active, 0},
                               {42, 10}, true) == LifecycleDecision::InvalidIdentity,
        "Broken owner record denied");

    auto unverified = PlanGuildDisbandArchive(first, current, false);
    check(unverified.Decision == LifecycleDecision::UnverifiedDisband,
        "Unverified disband may not archive");
    check(unverified.Proposed.State == PropertyLifecycle::Active,
        "Rejected disband leaves original state unchanged");
    check(PlanGuildDisbandArchive(first, recreated, true).Decision == LifecycleDecision::WrongGeneration,
        "New guild generation cannot archive the old one");
    check(PlanGuildDisbandArchive(first, other, true).Decision == LifecycleDecision::DifferentGuild,
        "Unrelated guild cannot archive property");

    auto archived = PlanGuildDisbandArchive(first, current, true);
    check(archived.Decision == LifecycleDecision::Allowed,
        "Matching verified disband can archive");
    check(archived.Proposed.State == PropertyLifecycle::Archived,
        "Archive marks property unavailable");
    check(archived.ExpectedVersion == 5 && archived.Proposed.Version == 6,
        "Archive requires optimistic version update");
    check(first.State == PropertyLifecycle::Active && first.Version == 5,
        "Pure planning never mutates existing records");
    check(CheckPropertyLifetime(archived.Proposed, current, true) == LifecycleDecision::Archived,
        "Even original guild cannot use archived property");
    check(CheckPropertyLifetime(archived.Proposed, recreated, true) == LifecycleDecision::WrongGeneration,
        "New guild generation still blocked");
    check(PlanGuildDisbandArchive(archived.Proposed, current, true).Decision ==
          LifecycleDecision::AlreadyArchived, "Double disband does not duplicate archives");

    check(PlanAdministrativeRestore(archived.Proposed, current, false).Decision ==
          LifecycleDecision::RestoreNotApproved, "Cannot restore without admin approval");
    check(PlanAdministrativeRestore(archived.Proposed, recreated, true).Decision ==
          LifecycleDecision::WrongGeneration, "Cannot restore to reused guild ID");
    check(PlanAdministrativeRestore(archived.Proposed, other, true).Decision ==
          LifecycleDecision::DifferentGuild, "Cannot restore to foreign guild");
    check(PlanAdministrativeRestore(first, current, true).Decision ==
          LifecycleDecision::NotArchived, "Already-active property cannot be restored again");

    auto restored = PlanAdministrativeRestore(archived.Proposed, current, true);
    check(restored.Decision == LifecycleDecision::Allowed,
        "Matching old guild restored by approved administrator");
    check(restored.Proposed.State == PropertyLifecycle::Active &&
          restored.Proposed.Version == 7, "Restore updates state and version");
    check(CheckPropertyLifetime(restored.Proposed, current, true) == LifecycleDecision::Allowed,
        "Restored guild can regain access");

    auto max = first;
    max.Version = std::numeric_limits<std::uint64_t>::max();
    check(PlanGuildDisbandArchive(max, current, true).Decision ==
          LifecycleDecision::VersionOverflow, "Disband rejects version wraparound");
    max.State = PropertyLifecycle::Archived;
    check(PlanAdministrativeRestore(max, current, true).Decision ==
          LifecycleDecision::VersionOverflow, "Restore rejects version wraparound");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks << " guild lifecycle and recovery policy checks\n";
    return 0;
}
