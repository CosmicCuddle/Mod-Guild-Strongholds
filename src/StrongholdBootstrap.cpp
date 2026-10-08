/*
 * Naxxramas Guild Strongholds — passive worldserver bootstrap only.
 *
 * Registers exactly one WorldScript for configuration and startup logging.
 * Does NOT enable property ownership, spawning, phasing, teleports, quests,
 * player hooks, SQL updates, inventory debits, or IP/Playerbot integration.
 * Do not deploy the development branch to a live server.
 */

#include "StrongholdStartupGate.h"

#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"

namespace NaxxGuildStrongholds
{
namespace
{
class StrongholdBootstrap final : public WorldScript
{
public:
    StrongholdBootstrap()
        : WorldScript("NaxxGuildStrongholdsBootstrap",
            { WORLDHOOK_ON_BEFORE_CONFIG_LOAD, WORLDHOOK_ON_STARTUP })
    {
    }

    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        const bool requested = sConfigMgr->GetOption<bool>(
            "NaxxGuildStrongholds.Enabled", false);
        _decision = EvaluateStartup(requested, DevelopmentCapabilities);

        if (_decision == StartupDecision::BlockedByUnverifiedDependencies)
        {
            LOG_WARN("module",
                "Guild Strongholds: Enabled=1 was requested, but housing is BLOCKED "
                "because isolation, persistence, safe exit and module compatibility "
                "have not been approved. No gameplay scripts are active.");
        }
    }

    void OnStartup() override
    {
        if (_decision == StartupDecision::DisabledByConfiguration)
        {
            LOG_INFO("module", "Guild Strongholds: foundation loaded, gameplay disabled.");
        }
        else if (_decision == StartupDecision::BlockedByUnverifiedDependencies)
        {
            LOG_WARN("module",
                "Guild Strongholds: startup BLOCKED. The development module makes "
                "no gameplay or database changes.");
        }
        else
        {
            // This branch cannot be reached with DevelopmentCapabilities.
            // It does NOT register gameplay even if capabilities change.
            LOG_WARN("module",
                "Guild Strongholds: startup checks passed, but no housing gameplay "
                "is registered in this development build.");
        }
    }

private:
    StartupDecision _decision = StartupDecision::DisabledByConfiguration;
};
}

void AddBootstrapScripts()
{
    new StrongholdBootstrap();
}
}
