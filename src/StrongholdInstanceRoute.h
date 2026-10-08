#ifndef NAXX_GUILD_STRONGHOLD_INSTANCE_ROUTE_H
#define NAXX_GUILD_STRONGHOLD_INSTANCE_ROUTE_H

#include <cstdint>
#include <vector>

// Pure instance-route feasibility policy. This NEVER creates an instance,
// changes player/group instance binds, reserves core IDs or teleports anyone.
// A real adapter has NOT been implemented in the user's AzerothCore fork.
namespace NaxxGuildStrongholds
{
struct InstanceRoute
{
    std::uint32_t GuildId = 0;
    std::uint64_t GuildCreatedAt = 0;
    std::uint32_t MapId = 0;
    std::uint32_t CoreInstanceId = 0;
    bool Retired = false;
};

struct InstanceRouteEvidence
{
    bool InstalledCoreAndModulesVerified = false;
    bool DbcAndMapTemplateVerified = false;
    bool DedicatedNonProgressionMap = false;
    bool InstanceIdIssuedByCore = false;
    bool InstanceSaveLifetimeVerified = false;
    bool PlayerAndGroupBindingSafe = false;
    bool DeterministicGuildRoutingVerified = false;
    bool SafeReturnAndRecoveryVerified = false;
    bool TwoGuildIsolationTestPassed = false;
    bool PropertyAndGuildIdentityVerified = false;
};

enum class InstanceRouteDecision : std::uint8_t
{
    ProposedOnly,             // NOT a live allocation or privacy approval
    UnverifiedCore,
    UnverifiedMapAssets,
    ProgressionMapConflict,
    UnverifiedCoreInstance,
    UnverifiedInstanceLifetime,
    PlayerOrGroupBindConflict,
    NoGuildRoutingMechanism,
    UnverifiedSafeReturn,
    UnverifiedWorldIsolation,
    UnverifiedGuildIdentity,
    InvalidIdentityOrMap,
    InvalidExistingRoute,
    ExistingGuildRoute,
    GuildIdReused,
    InstanceAlreadyAssigned
};

struct InstanceRoutePlan
{
    InstanceRouteDecision Decision = InstanceRouteDecision::UnverifiedCore;
    InstanceRoute Proposed;
};

// The candidate's CoreInstanceId MUST come from a functioning, documented
// core integration. Do not call MapMgr::GenerateInstanceId from this module;
// merely getting an ID does not make routing safe or persistent.
InstanceRoutePlan EvaluateInstanceRoute(InstanceRoute candidate,
    InstanceRouteEvidence const& evidence,
    std::vector<InstanceRoute> const& previous);

// This is a POST-ARRIVAL assertion for an already confirmed, separate
// Worldserver teleport. Never treats TeleportTo() returning true as arrival.
bool InstanceArrivalMatches(InstanceRoute const& registered,
    std::uint32_t characterGuild, std::uint64_t guildCreatedAt,
    std::uint32_t actualMap, std::uint32_t actualCoreInstance,
    bool serverVerifiedMember, bool routeProofStillValid);
}

#endif
