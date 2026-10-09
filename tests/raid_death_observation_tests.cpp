#include "StrongholdRaidDeathObservation.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    unsigned checked = 0, failures = 0;
    auto expect = [&](bool good, char const* label)
    {
        ++checked;
        if (!good)
        {
            ++failures;
            std::cerr << "FAILED: " << label << '\n';
        }
    };

    RaidDeathObservation event;
    event.UnitDeathCallbackReceived = true;
    event.HasCreature = true;
    event.IsRaidMap = true;
    event.DungeonBossFlag = true;
    event.MapId = 249;
    event.InstanceId = 2001;
    event.CreatureEntry = 10184; // synthetically supplied; NOT mapped to trophy
    auto test = [&](RaidDeathObservationDecision expected, char const* label)
    {
        auto result = InspectRaidDeath(event);
        expect(result.Decision == expected, label);
        expect(!result.BossEncounterVerified &&
            !result.GuildParticipationVerified && !result.GuildTrophyGranted,
            "Even a candidate never grants boss, guild or trophy proof");
    };

    test(RaidDeathObservationDecision::Disabled, "off by default");
    event.ExplicitStagingConfig = true;
    test(RaidDeathObservationDecision::CandidateOnly,
         "Source callback in a raid instance is only a candidate");
    event.UnitDeathCallbackReceived = false;
    test(RaidDeathObservationDecision::NotDeathCallback,
         "No valid death callback");
    event.UnitDeathCallbackReceived = true;
    event.HasCreature = false;
    test(RaidDeathObservationDecision::NotCreature,
         "Player/pet/non-creature death excluded");
    event.HasCreature = true;
    event.IsRaidMap = false;
    test(RaidDeathObservationDecision::NotRaid,
         "Dungeon/world-map boss excluded");
    event.IsRaidMap = true;
    event.DungeonBossFlag = false;
    test(RaidDeathObservationDecision::NotFlaggedRaidBoss,
         "Non-boss raid mob excluded");
    event.DungeonBossFlag = true;
    event.MapId = 0;
    test(RaidDeathObservationDecision::NoInstanceIdentity,
         "Missing map identity excluded");
    event.MapId = 249;
    event.InstanceId = 0;
    test(RaidDeathObservationDecision::NoInstanceIdentity,
         "Missing raid instance identity excluded");
    event.InstanceId = 2001;
    event.CreatureEntry = 0;
    test(RaidDeathObservationDecision::MissingCreatureIdentity,
         "Unknown creature entry excluded");
    event.CreatureEntry = 10184;
    test(RaidDeathObservationDecision::CandidateOnly,
         "A complete observation remains unverified until encounter review");

    if (failures)
        return 1;
    std::cout << "PASS: " << checked << " fail-closed raid death observation checks\n";
    return 0;
}
