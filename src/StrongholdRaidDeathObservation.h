#ifndef NAXX_GUILD_STRONGHOLD_RAID_DEATH_OBSERVATION_H
#define NAXX_GUILD_STRONGHOLD_RAID_DEATH_OBSERVATION_H

#include <cstdint>

namespace NaxxGuildStrongholds
{
// OBSERVATION ONLY. An OnUnitDeath callback is not enough to identify a
// particular raid encounter, validate all participating players, classify
// Playerbots or authorize a persistent raid trophy.
struct RaidDeathObservation
{
    bool ExplicitStagingConfig = false;
    bool UnitDeathCallbackReceived = false;
    bool HasCreature = false;
    bool IsRaidMap = false;
    bool DungeonBossFlag = false;
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    std::uint32_t CreatureEntry = 0;
};

enum class RaidDeathObservationDecision : std::uint8_t
{
    Disabled,
    NotDeathCallback,
    NotCreature,
    NotRaid,
    NotFlaggedRaidBoss,
    NoInstanceIdentity,
    MissingCreatureIdentity,
    CandidateOnly
};

struct RaidDeathObservationResult
{
    RaidDeathObservationDecision Decision = RaidDeathObservationDecision::Disabled;
    // These can NEVER become true in this observation policy.
    bool BossEncounterVerified = false;
    bool GuildParticipationVerified = false;
    bool GuildTrophyGranted = false;
};

RaidDeathObservationResult InspectRaidDeath(RaidDeathObservation const& observation);
}
#endif
