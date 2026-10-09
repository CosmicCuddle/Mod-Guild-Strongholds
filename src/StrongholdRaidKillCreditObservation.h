#ifndef NAXX_GUILD_STRONGHOLDS_RAID_KILL_CREDIT_OBSERVATION_H
#define NAXX_GUILD_STRONGHOLDS_RAID_KILL_CREDIT_OBSERVATION_H

#include <cstdint>

namespace NaxxGuildStrongholds
{
struct RaidKillCreditObservation
{
    bool ExplicitStagingConfig = false;
    bool CallbackReceived = false;
    bool HasPlayerAndCreature = false;
    bool RaidMap = false;
    bool DungeonBossFlag = false;
    bool SameNonzeroInstance = false;
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    std::uint32_t CreatureEntry = 0;
};

enum class RaidKillCreditDecision : std::uint8_t
{
    Disabled,
    UnverifiedSource,
    NotRaidBossCredit,
    NotSameInstance,
    InvalidIdentifiers,
    CreditCandidateOnly
};

struct RaidKillCreditResult
{
    RaidKillCreditDecision Decision = RaidKillCreditDecision::Disabled;
    // Receiving an AzerothCore creature kill credit event is NOT independent
    // confirmation of boss defeat, player participation or human control.
    bool HumanVerified = false;
    bool EncounterContributionVerified = false;
    bool TrophyGranted = false;
};

RaidKillCreditResult ReviewRaidKillCredit(RaidKillCreditObservation const& evidence);
}
#endif
