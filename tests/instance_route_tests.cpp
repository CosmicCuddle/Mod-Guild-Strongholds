#include "StrongholdInstanceRoute.h"

#include <iostream>
#include <vector>

using namespace NaxxGuildStrongholds;

int main()
{
    unsigned checks = 0, errors = 0;
    auto check = [&](bool good, char const* reason)
    {
        ++checks;
        if (!good)
        {
            ++errors;
            std::cerr << "FAILED: " << reason << '\n';
        }
    };

    // All 'true' below are SYNTHETIC ONLY. The actual upstream core has no
    // verified guild-specific instance router; production would fail closed.
    InstanceRouteEvidence all{true,true,true,true,true,true,true,true,true,true};
    InstanceRoute a{51, 1790001000, 995, 41001, false};
    InstanceRoute b{52, 1790002000, 995, 41002, false};
    check(EvaluateInstanceRoute(a, {}, {}).Decision ==
          InstanceRouteDecision::UnverifiedCore, "No real deployment proof denies");
    check(EvaluateInstanceRoute(a, all, {}).Decision ==
          InstanceRouteDecision::ProposedOnly, "Complete simulated proof proposes only");

    auto absent = all;
    absent.DbcAndMapTemplateVerified = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::UnverifiedMapAssets, "Client/instance template required");
    absent = all;
    absent.DedicatedNonProgressionMap = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::ProgressionMapConflict, "Cannot repurpose raid map");
    absent = all;
    absent.InstanceIdIssuedByCore = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::UnverifiedCoreInstance, "Never invent instance ID");
    absent = all;
    absent.InstanceSaveLifetimeVerified = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::UnverifiedInstanceLifetime, "Restart retention required");
    absent = all;
    absent.PlayerAndGroupBindingSafe = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::PlayerOrGroupBindConflict, "Player/raid binds cannot be affected");
    absent = all;
    absent.DeterministicGuildRoutingVerified = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::NoGuildRoutingMechanism, "Map alone not a guild route");
    absent = all;
    absent.SafeReturnAndRecoveryVerified = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::UnverifiedSafeReturn, "Safe emergency return mandatory");
    absent = all;
    absent.TwoGuildIsolationTestPassed = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::UnverifiedWorldIsolation, "Real in-world proof mandatory");
    absent = all;
    absent.PropertyAndGuildIdentityVerified = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::UnverifiedGuildIdentity, "Only verified guild owners");
    absent = all;
    absent.InstalledCoreAndModulesVerified = false;
    check(EvaluateInstanceRoute(a, absent, {}).Decision ==
          InstanceRouteDecision::UnverifiedCore, "Unknown installed fork rejects");

    auto invalid = a;
    invalid.CoreInstanceId = 0;
    check(EvaluateInstanceRoute(invalid, all, {}).Decision ==
          InstanceRouteDecision::InvalidIdentityOrMap, "Instance zero isn't private");
    invalid = a;
    invalid.MapId = 0;
    check(EvaluateInstanceRoute(invalid, all, {}).Decision ==
          InstanceRouteDecision::InvalidIdentityOrMap, "No assumed map");
    invalid = a;
    invalid.GuildCreatedAt = 0;
    check(EvaluateInstanceRoute(invalid, all, {}).Decision ==
          InstanceRouteDecision::InvalidIdentityOrMap, "Missing guild generation fails closed");
    invalid = a;
    invalid.Retired = true;
    check(EvaluateInstanceRoute(invalid, all, {}).Decision ==
          InstanceRouteDecision::InvalidIdentityOrMap, "Cannot propose already-retired route");

    std::vector<InstanceRoute> routes{a};
    check(EvaluateInstanceRoute(a, all, routes).Decision ==
          InstanceRouteDecision::ExistingGuildRoute, "One route per existing guild");
    auto recycled = a;
    recycled.GuildCreatedAt += 1;
    recycled.CoreInstanceId = 41003;
    check(EvaluateInstanceRoute(recycled, all, routes).Decision ==
          InstanceRouteDecision::GuildIdReused, "Guild ID reuse cannot acquire old route");
    check(EvaluateInstanceRoute(b, all, routes).Decision ==
          InstanceRouteDecision::ProposedOnly, "Another guild can propose separate core-issued ID");
    routes.push_back(b);
    InstanceRoute c{53, 1790003000, 996, 41002, false};
    check(EvaluateInstanceRoute(c, all, routes).Decision ==
          InstanceRouteDecision::InstanceAlreadyAssigned,
          "Core instance IDs unique even if map IDs differ");
    routes.front().Retired = true;
    c.CoreInstanceId = 41001;
    check(EvaluateInstanceRoute(c, all, routes).Decision ==
          InstanceRouteDecision::InstanceAlreadyAssigned,
          "Retired instances not silently recycled");
    routes.push_back(b);
    c.CoreInstanceId = 41004;
    check(EvaluateInstanceRoute(c, all, routes).Decision ==
          InstanceRouteDecision::InvalidExistingRoute, "Corrupt duplicate existing routes deny");
    routes.pop_back();

    check(InstanceArrivalMatches(a, 51, 1790001000, 995, 41001, true, true),
          "Server-confirmed member arrived in intended map and instance");
    check(!InstanceArrivalMatches(a, 51, 1790001000, 995, 41002, true, true),
          "Same map, other guild instance rejected");
    check(!InstanceArrivalMatches(a, 52, 1790001000, 995, 41001, true, true),
          "Other guild cannot use route");
    check(!InstanceArrivalMatches(a, 51, 1790001001, 995, 41001, true, true),
          "Recreated guild cannot use saved route");
    check(!InstanceArrivalMatches(a, 51, 1790001000, 995, 41001, false, true),
          "Former member denied");
    check(!InstanceArrivalMatches(a, 51, 1790001000, 995, 41001, true, false),
          "Expired/unproven routing denies admission");
    check(!InstanceArrivalMatches(routes.front(), 51, 1790001000, 995, 41001, true, true),
          "Retired route denies arrival");

    if (errors)
        return 1;
    std::cout << "PASS: " << checks << " core-instance routing preflight checks (synthetic only)\n";
    return 0;
}
