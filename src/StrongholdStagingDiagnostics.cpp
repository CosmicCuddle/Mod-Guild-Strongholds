/*
 * STAGING-ONLY read-only diagnostic, excluded from normal builds.
 * Enabling requires an explicit build define AND config key AND GM RBAC.
 *
 * NO teleport, phase writes, world spawn, SQL or other module mutations.
 * Only logs information about the invoking administrator's own player.
 */
#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)

#include "Chat.h"
#include "Config.h"
#include "Player.h"
#include "RBAC.h"
#include "ScriptMgr.h"

#include <string>

namespace NaxxGuildStrongholds
{
namespace
{
class StagingDiagnosticsCommands final : public CommandScript
{
public:
    StagingDiagnosticsCommands() : CommandScript("NaxxGuildStrongholdsStagingDiagnostics") {}

    Acore::ChatCommands::ChatCommandTable GetCommands() const override
    {
        using namespace Acore::ChatCommands;
        static ChatCommandTable snapshotTable =
        {
            { "snapshot", HandleSnapshot, rbac::RBAC_PERM_COMMAND_DEBUG_INFO, Console::No }
        };
        static ChatCommandTable commandTable =
        {
            { "naxxgs", snapshotTable }
        };
        return commandTable;
    }

    static bool HandleSnapshot(ChatHandler* handler)
    {
        if (!handler)
            return false;

        // This is independent of NaxxGuildStrongholds.Enabled: housing stays
        // disabled and the staff diagnostic can gather baseline evidence.
        if (!sConfigMgr->GetOption<bool>(
            "NaxxGuildStrongholds.Diagnostics.Enabled", false))
        {
            handler->SendSysMessage("Guild Strongholds diagnostic OFF. "
                "Enable only in an authorised staging worldserver.");
            return true;
        }

        Player const* player = handler->GetPlayer();
        if (!player)
            return false;

        // Report observations, NEVER infer or set exact/combined phase mode:
        // WorldObject's comparison mode is not exposed by this interface.
        std::string const line =
            "NaxxGS staging observation: guild=" + std::to_string(player->GetGuildId()) +
            " map=" + std::to_string(player->GetMapId()) +
            " instance=" + std::to_string(player->GetInstanceId()) +
            " zone=" + std::to_string(player->GetZoneId()) +
            " area=" + std::to_string(player->GetAreaId()) +
            " phaseMask=" + std::to_string(player->GetPhaseMask()) +
            " (comparison mode UNKNOWN). NO PRIVACY APPROVAL.";
        handler->SendSysMessage(line);
        return true;
    }
};
}

void AddStagingDiagnosticsScripts()
{
    new StagingDiagnosticsCommands();
}
}

#endif // NAXX_GS_BUILD_STAGING_DIAGNOSTICS
