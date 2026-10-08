#include "StrongholdRaidDeathObservation.h"

namespace NaxxGuildStrongholds
{
RaidDeathObservationResult InspectRaidDeath(RaidDeathObservation const& event)
{
    RaidDeathObservationResult result;
    if (!event.ExplicitStagingConfig)
        return result;
    if (!event.UnitDeathCallbackReceived)
        result.Decision = RaidDeathObservationDecision::NotDeathCallback;
    else if (!event.HasCreature)
        result.Decision = RaidDeathObservationDecision::NotCreature;
    else if (!event.IsRaidMap)
        result.Decision = RaidDeathObservationDecision::NotRaid;
    else if (!event.DungeonBossFlag)
        result.Decision = RaidDeathObservationDecision::NotFlaggedRaidBoss;
    else if (!event.MapId || !event.InstanceId)
        result.Decision = RaidDeathObservationDecision::NoInstanceIdentity;
    else if (!event.CreatureEntry)
        result.Decision = RaidDeathObservationDecision::MissingCreatureIdentity;
    else
        result.Decision = RaidDeathObservationDecision::CandidateOnly;

    // A generic death notification MUST NEVER set any proof/reward flag.
    return result;
}
}
