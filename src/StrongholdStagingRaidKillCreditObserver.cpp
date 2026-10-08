/*
 * Extra staging-only AzerothCore PlayerScript (NO guild credit/rewards).
 * The core's creature kill-credit callback may credit a pet owner, totem
 * owner or tapped creature. It is therefore not proof of a human player's
 * actual boss contribution, nor of all members of a 40-player raid.
 *
 * Read-only numeric metadata only, no player GUID/name logged. No DB, IP,
 * Playerbots, quest, or world-object operations. Normal build omits it.
 */
#if defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)

#include "StrongholdRaidKillCreditObservation.h"

#include "Config.h"
#include "Creature.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"

namespace NaxxGuildStrongholds
{
namespace
{
class StagingRaidKillCreditObserver final : public PlayerScript
{
public:
    StagingRaidKillCreditObserver()
        : PlayerScript("NaxxGuildStrongholdsStagingRaidKillCreditObserver",
            { PLAYERHOOK_ON_CREATURE_KILL_CREDIT })
    {
    }

    void OnPlayerCreatureKillCredit(Player* player, Creature* killed) override
    {
        if (!sConfigMgr->GetOption<bool>(
            "NaxxGuildStrongholds.StagingRaidObserver.Enabled", false) ||
            !player || !killed)
            return;

        Map const* map = killed->GetMap();
        if (!map)
            return;

        RaidKillCreditObservation observed;
        observed.ExplicitStagingConfig = true;
        observed.CallbackReceived = true;
        observed.HasPlayerAndCreature = true;
        observed.RaidMap = map->IsRaid();
        observed.DungeonBossFlag = killed->IsDungeonBoss();
        observed.SameNonzeroInstance =
            player->GetMapId() == killed->GetMapId() &&
            player->GetInstanceId() != 0 &&
            player->GetInstanceId() == killed->GetInstanceId();
        observed.MapId = killed->GetMapId();
        observed.InstanceId = killed->GetInstanceId();
        observed.CreatureEntry = killed->GetEntry();

        if (ReviewRaidKillCredit(observed).Decision !=
            RaidKillCreditDecision::CreditCandidateOnly)
            return;

        LOG_INFO("module",
            "NaxxGS STAGING creature kill-credit CANDIDATE: map={}, instance={}, "
            "creature={}; human/playerbot status and encounter participation "
            "UNVERIFIED. No trophy, quest, SQL or gameobject changed.",
            observed.MapId, observed.InstanceId, observed.CreatureEntry);
    }
};
}

void AddStagingRaidKillCreditObserverScripts()
{
    new StagingRaidKillCreditObserver();
}
}
#endif
