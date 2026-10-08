#ifndef NAXX_GUILD_STRONGHOLD_LIFECYCLE_H
#define NAXX_GUILD_STRONGHOLD_LIFECYCLE_H

#include <cstdint>

// Domain-only guild lifecycle policy. No live AzerothCore hooks, DB writes,
// phasing or player mutations. A guild-ID alone is never proof of ownership.
namespace NaxxGuildStrongholds
{
enum class PropertyLifecycle : std::uint8_t
{
    Active,
    Archived
};

struct GuildIdentity
{
    std::uint32_t GuildId = 0;
    std::uint64_t CreatedAt = 0;  // verified server guild creation timestamp
};

struct PropertyLifetime
{
    GuildIdentity OriginalGuild;
    PropertyLifecycle State = PropertyLifecycle::Archived;
    std::uint64_t Version = 0;
};

enum class LifecycleDecision : std::uint8_t
{
    Allowed,
    IdentityUnverified,
    InvalidIdentity,
    DifferentGuild,
    WrongGeneration,
    Archived,
    AlreadyArchived,
    NotArchived,
    UnverifiedDisband,
    RestoreNotApproved,
    VersionOverflow
};

struct LifecyclePlan
{
    LifecycleDecision Decision = LifecycleDecision::IdentityUnverified;
    PropertyLifetime Proposed;
    std::uint64_t ExpectedVersion = 0;
};

// View/interaction check. Must run before any persistent guild action.
// GuildId + CreatedAt are not a cryptographic token; the final adapter
// must also inspect authoritative Guild membership/permissions.
LifecycleDecision CheckPropertyLifetime(PropertyLifetime const& property,
    GuildIdentity const& actingGuild, bool identityVerified);

// An archive preserves EVERY owned record and requires a verified guild
// disband event. Live hook/DB transaction support is not implemented.
LifecyclePlan PlanGuildDisbandArchive(PropertyLifetime const& property,
    GuildIdentity const& disbandedGuild, bool eventVerified);

// No automatic rebinding to a replacement guild. A *matching original*
// guild generation and explicit, independently verified admin recovery
// permission are mandatory.
LifecyclePlan PlanAdministrativeRestore(PropertyLifetime const& property,
    GuildIdentity const& restoredGuild, bool adminApproved);
}

#endif
