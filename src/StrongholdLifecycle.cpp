#include "StrongholdLifecycle.h"

#include <limits>

namespace NaxxGuildStrongholds
{
namespace
{
bool Valid(GuildIdentity const& id)
{
    return id.GuildId != 0 && id.CreatedAt != 0;
}

LifecycleDecision IdentityCheck(PropertyLifetime const& property,
    GuildIdentity const& guild)
{
    if (!Valid(property.OriginalGuild) || !Valid(guild))
        return LifecycleDecision::InvalidIdentity;
    if (property.OriginalGuild.GuildId != guild.GuildId)
        return LifecycleDecision::DifferentGuild;
    if (property.OriginalGuild.CreatedAt != guild.CreatedAt)
        return LifecycleDecision::WrongGeneration;
    return LifecycleDecision::Allowed;
}

LifecyclePlan MakePlan(PropertyLifetime const& property)
{
    LifecyclePlan result;
    result.Proposed = property;
    result.ExpectedVersion = property.Version;
    return result;
}
}

LifecycleDecision CheckPropertyLifetime(PropertyLifetime const& property,
    GuildIdentity const& actingGuild, bool identityVerified)
{
    if (!identityVerified)
        return LifecycleDecision::IdentityUnverified;
    LifecycleDecision check = IdentityCheck(property, actingGuild);
    if (check != LifecycleDecision::Allowed)
        return check;
    return property.State == PropertyLifecycle::Active ?
        LifecycleDecision::Allowed : LifecycleDecision::Archived;
}

LifecyclePlan PlanGuildDisbandArchive(PropertyLifetime const& property,
    GuildIdentity const& disbandedGuild, bool eventVerified)
{
    LifecyclePlan result = MakePlan(property);
    if (!eventVerified)
        result.Decision = LifecycleDecision::UnverifiedDisband;
    else
        result.Decision = IdentityCheck(property, disbandedGuild);

    if (result.Decision != LifecycleDecision::Allowed)
        return result;
    if (property.State == PropertyLifecycle::Archived)
    {
        result.Decision = LifecycleDecision::AlreadyArchived;
        return result;
    }
    if (property.Version == std::numeric_limits<std::uint64_t>::max())
    {
        result.Decision = LifecycleDecision::VersionOverflow;
        return result;
    }
    result.Proposed.State = PropertyLifecycle::Archived;
    ++result.Proposed.Version;
    return result;
}

LifecyclePlan PlanAdministrativeRestore(PropertyLifetime const& property,
    GuildIdentity const& restoredGuild, bool adminApproved)
{
    LifecyclePlan result = MakePlan(property);
    if (!adminApproved)
    {
        result.Decision = LifecycleDecision::RestoreNotApproved;
        return result;
    }
    result.Decision = IdentityCheck(property, restoredGuild);
    if (result.Decision != LifecycleDecision::Allowed)
        return result;
    if (property.State != PropertyLifecycle::Archived)
    {
        result.Decision = LifecycleDecision::NotArchived;
        return result;
    }
    if (property.Version == std::numeric_limits<std::uint64_t>::max())
    {
        result.Decision = LifecycleDecision::VersionOverflow;
        return result;
    }
    result.Proposed.State = PropertyLifecycle::Active;
    ++result.Proposed.Version;
    return result;
}
}
