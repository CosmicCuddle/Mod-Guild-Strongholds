#include "StrongholdPropertyRead.h"

namespace NaxxGuildStrongholds
{
PropertyReadResult InspectPropertyRow(GuildIdentity const& actor,
    bool allowed, bool returned, PropertySqlRow const& row)
{
    PropertyReadResult result;
    if (!allowed)
        return result;
    if (!actor.GuildId || !actor.CreatedAt)
    {
        result.Decision = PropertyReadDecision::IdentityUnverified;
        return result;
    }
    if (!returned)
    {
        result.Decision = PropertyReadDecision::NotFoundOrQueryUnavailable;
        return result;
    }
    if (!row.GuildId || !row.GuildCreatedAt ||
        (row.Lifecycle != "active" && row.Lifecycle != "archived") ||
        row.DevelopmentLevel < 1 || row.DevelopmentLevel > 7)
    {
        result.Decision = PropertyReadDecision::MalformedRow;
        return result;
    }

    // Retain the trustworthy row identity even for a DIFFERENT generation,
    // so the existing lifecycle policy can report reuse without granting it.
    result.ValidRow = true;
    result.Property.OriginalGuild = {row.GuildId, row.GuildCreatedAt};
    result.Property.State = row.Lifecycle == "active" ?
        PropertyLifecycle::Active : PropertyLifecycle::Archived;
    result.Property.Version = row.LifecycleVersion;
    result.Level = static_cast<std::uint8_t>(row.DevelopmentLevel);

    if (row.GuildId != actor.GuildId)
        result.Decision = PropertyReadDecision::WrongGuild;
    else if (row.GuildCreatedAt != actor.CreatedAt)
        result.Decision = PropertyReadDecision::WrongGeneration;
    else
        result.Decision = PropertyReadDecision::Validated;
    return result;
}
}
