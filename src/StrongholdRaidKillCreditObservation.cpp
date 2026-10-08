#include "StrongholdRaidKillCreditObservation.h"

namespace NaxxGuildStrongholds
{
RaidKillCreditResult ReviewRaidKillCredit(RaidKillCreditObservation const& e)
{
    RaidKillCreditResult result;
    if (!e.ExplicitStagingConfig)
        return result;
    if (!e.CallbackReceived || !e.HasPlayerAndCreature)
        result.Decision = RaidKillCreditDecision::UnverifiedSource;
    else if (!e.RaidMap || !e.DungeonBossFlag)
        result.Decision = RaidKillCreditDecision::NotRaidBossCredit;
    else if (!e.SameNonzeroInstance)
        result.Decision = RaidKillCreditDecision::NotSameInstance;
    else if (!e.MapId || !e.InstanceId || !e.CreatureEntry)
        result.Decision = RaidKillCreditDecision::InvalidIdentifiers;
    else
        result.Decision = RaidKillCreditDecision::CreditCandidateOnly;
    return result;
}
}
