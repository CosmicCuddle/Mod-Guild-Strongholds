/*
 * Extra-optional STAGING-only GM-self read of the module-owned DRAFT
 * character database ticket table. No write, teleport, or secret output.
 */
#if defined(NAXX_GS_BUILD_STAGING_VISIT_READ) && \
    defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
#include "StrongholdStagingVisitAdapter.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "QueryResult.h"
#include <cstdint>
#include <string>

namespace NaxxGuildStrongholds
{
VisitTicketReadReview ReadStagingVisitTicket(Player const* self)
{
    VisitRecord row;
    bool const optedIn = sConfigMgr->GetOption<bool>(
        "NaxxGuildStrongholds.StagingVisitRead.Enabled", false);
    if (!optedIn || !self || !self->IsGameMaster() || !self->IsInWorld())
        return ReviewVisitTicketRow(0, false, false, row);

    std::uint32_t const selfGuid = self->GetGUID().GetCounter();
    if (!selfGuid)
        return ReviewVisitTicketRow(0, true, false, row);

    QueryResult query = CharacterDatabase.Query(
        "SELECT player_guid, guild_id, guild_created_at, visit_nonce, "
        "property_key, visit_state, return_map, return_instance, "
        "return_x, return_y, return_z, return_o, version "
        "FROM naxx_gs_visit WHERE player_guid = {} LIMIT 1",
        selfGuid);
    if (!query)
        return ReviewVisitTicketRow(selfGuid, true, false, row);
    Field* f = query->Fetch();
    if (!f)
        return ReviewVisitTicketRow(selfGuid, true, false, row);
    row.CharacterGuid = f[0].Get<std::uint32_t>();
    row.OriginalGuild.GuildId = f[1].Get<std::uint32_t>();
    row.OriginalGuild.CreatedAt = f[2].Get<std::uint64_t>();
    row.SessionKey = f[3].Get<std::string>();
    row.PropertyKey = f[4].Get<std::string>();
    std::string const stage = f[5].Get<std::string>();
    if (stage == "prepared")
        row.Stage = VisitStage::Prepared;
    else if (stage == "inside")
        row.Stage = VisitStage::Inside;
    else if (stage == "returning")
        row.Stage = VisitStage::Returning;
    else
        return {VisitTicketReadStatus::InvalidRow, false, false, false};
    row.ReturnPoint.MapId = f[6].Get<std::uint32_t>();
    row.ReturnPoint.InstanceId = f[7].Get<std::uint32_t>();
    row.ReturnPoint.X = f[8].Get<float>();
    row.ReturnPoint.Y = f[9].Get<float>();
    row.ReturnPoint.Z = f[10].Get<float>();
    row.ReturnPoint.Orientation = f[11].Get<float>();
    row.Version = f[12].Get<std::uint64_t>();
    return ReviewVisitTicketRow(selfGuid, true, true, row);
}
}
#endif
