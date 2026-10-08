/*
 * EXTRA STAGING-ONLY read-only raid death observer.
 *
 * Registers NO raid boss script, loot hook, guild credit, DB write or
 * Playerbot logic. Does not inspect a player's GUID or name, group or IP.
 *
 * UnitScript::OnUnitDeath is source-verified in pinned AzerothCore.
 * A generic boss-death callback is CANDIDATE TELEMETRY, NOT earned trophies.
 * On a live/normal build this entire translation unit is inert.
 */
#if defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)

#include "StrongholdRaidDeathObservation.h"

#include "Config.h"
#include "Creature.h"
#include "Log.h"
#include "Map.h"
#include "ScriptMgr.h"
#include "Unit.h"

namespace NaxxGuildStrongholds
{
namespace
{
class StagingRaidDeathObserver final : public UnitScript
{
public:
    StagingRaidDeathObserver()
        : UnitScript("NaxxGuildStrongholdsStagingRaidDeathObserver", true,
            { UNITHOOK_ON_UNIT_DEATH })
    {
    }

    void OnUnitDeath(Unit* unit, Unit* /*killer*/) override
    {
        // No player, guild, roster, kill-credit or Playerbots assumptions.
        if (!unit)
            return;
        Creature* creature = unit->ToCreature();
        if (!creature)
            return;

        Map const* map = creature->GetMap();
        if (!map)
            return;

        RaidDeathObservation observation;
        observation.ExplicitStagingConfig = sConfigMgr->GetOption<bool>(
            "NaxxGuildStrongholds.StagingRaidObserver.Enabled", false);
        if (!observation.ExplicitStagingConfig)
            return;

        observation.UnitDeathCallbackReceived = true;
        observation.HasCreature = true;
        observation.IsRaidMap = map->IsRaid();
        observation.DungeonBossFlag = creature->IsDungeonBoss();
        observation.MapId = creature->GetMapId();
        observation.InstanceId = creature->GetInstanceId();
        observation.CreatureEntry = creature->GetEntry();

        if (InspectRaidDeath(observation).Decision !=
            RaidDeathObservationDecision::CandidateOnly)
            return;

        // No member GUIDs or names in logs, and NO proof of a specific
        // named encounter. Boss entries need explicit per-fork review.
        LOG_INFO("module",
            "NaxxGS STAGING raid death CANDIDATE: map={}, instance={}, creature={}; "
            "boss mapping, raid roster and IP participation NOT VERIFIED. "
            "No trophy, quest, SQL or gameobject was changed.",
            observation.MapId, observation.InstanceId, observation.CreatureEntry);
    }
};
}

void AddStagingRaidObserverScripts()
{
    new StagingRaidDeathObserver();
}
}

#endif // NAXX_GS_BUILD_STAGING_RAID_OBSERVER
