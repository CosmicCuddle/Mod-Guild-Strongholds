#include "StrongholdInstanceRoute.h"

#include <set>

namespace NaxxGuildStrongholds
{
namespace
{
InstanceRoutePlan Refuse(InstanceRouteDecision decision)
{
    InstanceRoutePlan result;
    result.Decision = decision;
    return result;
}

bool Valid(InstanceRoute const& route)
{
    return route.GuildId != 0 && route.GuildCreatedAt != 0 &&
        route.MapId != 0 && route.CoreInstanceId != 0;
}
}

InstanceRoutePlan EvaluateInstanceRoute(InstanceRoute candidate,
    InstanceRouteEvidence const& evidence,
    std::vector<InstanceRoute> const& previous)
{
    if (!evidence.InstalledCoreAndModulesVerified)
        return Refuse(InstanceRouteDecision::UnverifiedCore);
    if (!evidence.DbcAndMapTemplateVerified)
        return Refuse(InstanceRouteDecision::UnverifiedMapAssets);
    if (!evidence.DedicatedNonProgressionMap)
        return Refuse(InstanceRouteDecision::ProgressionMapConflict);
    if (!evidence.InstanceIdIssuedByCore)
        return Refuse(InstanceRouteDecision::UnverifiedCoreInstance);
    if (!evidence.InstanceSaveLifetimeVerified)
        return Refuse(InstanceRouteDecision::UnverifiedInstanceLifetime);
    if (!evidence.PlayerAndGroupBindingSafe)
        return Refuse(InstanceRouteDecision::PlayerOrGroupBindConflict);
    if (!evidence.DeterministicGuildRoutingVerified)
        return Refuse(InstanceRouteDecision::NoGuildRoutingMechanism);
    if (!evidence.SafeReturnAndRecoveryVerified)
        return Refuse(InstanceRouteDecision::UnverifiedSafeReturn);
    if (!evidence.TwoGuildIsolationTestPassed)
        return Refuse(InstanceRouteDecision::UnverifiedWorldIsolation);
    if (!evidence.PropertyAndGuildIdentityVerified)
        return Refuse(InstanceRouteDecision::UnverifiedGuildIdentity);

    if (!Valid(candidate) || candidate.Retired)
        return Refuse(InstanceRouteDecision::InvalidIdentityOrMap);

    std::set<std::uint32_t> occupiedCoreIds;
    std::set<std::uint32_t> knownGuildIds;
    for (InstanceRoute const& prior : previous)
    {
        if (!Valid(prior) || !occupiedCoreIds.insert(prior.CoreInstanceId).second ||
            !knownGuildIds.insert(prior.GuildId).second)
            return Refuse(InstanceRouteDecision::InvalidExistingRoute);
        if (prior.GuildId == candidate.GuildId)
        {
            if (prior.GuildCreatedAt != candidate.GuildCreatedAt)
                return Refuse(InstanceRouteDecision::GuildIdReused);
            // Existing and even retired routes cannot silently be replaced.
            return Refuse(InstanceRouteDecision::ExistingGuildRoute);
        }
        if (prior.CoreInstanceId == candidate.CoreInstanceId)
            return Refuse(InstanceRouteDecision::InstanceAlreadyAssigned);
    }

    InstanceRoutePlan result;
    result.Decision = InstanceRouteDecision::ProposedOnly;
    result.Proposed = candidate;
    return result;
}

bool InstanceArrivalMatches(InstanceRoute const& registered,
    std::uint32_t characterGuild, std::uint64_t guildCreatedAt,
    std::uint32_t actualMap, std::uint32_t actualCoreInstance,
    bool serverVerifiedMember, bool routeProofStillValid)
{
    return Valid(registered) && !registered.Retired &&
        serverVerifiedMember && routeProofStillValid &&
        registered.GuildId == characterGuild &&
        registered.GuildCreatedAt == guildCreatedAt &&
        registered.MapId == actualMap &&
        registered.CoreInstanceId == actualCoreInstance;
}
}
