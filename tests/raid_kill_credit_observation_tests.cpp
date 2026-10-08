#include "StrongholdRaidKillCreditObservation.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0, failed = 0;
    auto check = [&](bool good, char const* name) {
        ++checks;
        if (!good) { ++failed; std::cerr << "FAILED: " << name << '\n'; }
    };

    RaidKillCreditObservation input;
    input.CallbackReceived = true;
    input.HasPlayerAndCreature = true;
    input.RaidMap = true;
    input.DungeonBossFlag = true;
    input.SameNonzeroInstance = true;
    input.MapId = 249;
    input.InstanceId = 2001;
    input.CreatureEntry = 10184; // synthetic test ID, not a mapped trophy
    auto run = [&](RaidKillCreditDecision expected, char const* reason) {
        auto r = ReviewRaidKillCredit(input);
        check(r.Decision == expected, reason);
        check(!r.HumanVerified && !r.EncounterContributionVerified &&
              !r.TrophyGranted, "Credit never proves human or grants trophy");
    };
    run(RaidKillCreditDecision::Disabled, "Off by default");
    input.ExplicitStagingConfig = true;
    run(RaidKillCreditDecision::CreditCandidateOnly, "Candidate only");
    input.CallbackReceived = false;
    run(RaidKillCreditDecision::UnverifiedSource, "No callback");
    input.CallbackReceived = true;
    input.HasPlayerAndCreature = false;
    run(RaidKillCreditDecision::UnverifiedSource, "Missing player or creature");
    input.HasPlayerAndCreature = true;
    input.RaidMap = false;
    run(RaidKillCreditDecision::NotRaidBossCredit, "Not raid");
    input.RaidMap = true;
    input.DungeonBossFlag = false;
    run(RaidKillCreditDecision::NotRaidBossCredit, "Nonboss");
    input.DungeonBossFlag = true;
    input.SameNonzeroInstance = false;
    run(RaidKillCreditDecision::NotSameInstance, "Another instance");
    input.SameNonzeroInstance = true;
    input.MapId = 0;
    run(RaidKillCreditDecision::InvalidIdentifiers, "No map ID");
    input.MapId = 249;
    input.InstanceId = 0;
    run(RaidKillCreditDecision::InvalidIdentifiers, "No instance ID");
    input.InstanceId = 2001;
    input.CreatureEntry = 0;
    run(RaidKillCreditDecision::InvalidIdentifiers, "No boss entry");
    if (failed) return 1;
    std::cout << "PASS: " << checks << " source-only kill credit observation checks\n";
    return 0;
}
