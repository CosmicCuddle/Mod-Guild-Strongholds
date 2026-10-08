/*
 * EXTRA OPT-IN read-only character-DB adapter; no migration or mutations.
 * Requires a separately reviewed, backed-up STAGING database that already
 * contains the draft naxx_gs_settlement table. Disabled by default.
 */
#if defined(NAXX_GS_BUILD_STAGING_PROPERTY_READ) && \
    defined(NAXX_GS_BUILD_STAGING_STEWARD)

#include "StrongholdStagingPropertyAdapter.h"
#include "Config.h"
#include "DatabaseEnv.h"

#include <cstdint>
#include <string>

namespace NaxxGuildStrongholds
{
PropertyReadResult ReadStagingProperty(GuildIdentity const& verifiedGuild)
{
    bool const enabled = sConfigMgr->GetOption<bool>(
        "NaxxGuildStrongholds.StagingPropertyRead.Enabled", false);
    PropertySqlRow row;
    // Do not even attempt the query without BOTH verified original guild
    // generation and explicit staging config. No implicit SQL install.
    if (!enabled || !verifiedGuild.GuildId || !verifiedGuild.CreatedAt)
        return InspectPropertyRow(verifiedGuild, enabled, false, row);

    QueryResult result = CharacterDatabase.Query(
        "SELECT guild_id, guild_created_at, lifecycle_state, "
        "lifecycle_version, development_level "
        "FROM naxx_gs_settlement WHERE guild_id = {} LIMIT 1",
        verifiedGuild.GuildId);
    // QueryResult null may mean no row OR missing/unavailable staging table.
    // Both cases must remain fail-closed without guessing ownership.
    if (!result)
        return InspectPropertyRow(verifiedGuild, true, false, row);

    Field* fields = result->Fetch();
    if (!fields)
        return InspectPropertyRow(verifiedGuild, true, false, row);

    row.GuildId = fields[0].Get<std::uint32_t>();
    row.GuildCreatedAt = fields[1].Get<std::uint64_t>();
    row.Lifecycle = fields[2].Get<std::string>();
    row.LifecycleVersion = fields[3].Get<std::uint64_t>();
    row.DevelopmentLevel = fields[4].Get<std::uint8_t>();
    return InspectPropertyRow(verifiedGuild, true, true, row);
}
}
#endif
