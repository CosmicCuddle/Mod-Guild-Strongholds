#include "StrongholdPlayerbotsRead.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    unsigned checks = 0, failed = 0;
    auto check = [&](bool yes, char const* label) {
        ++checks;
        if (!yes) { ++failed; std::cerr << "FAILED: " << label << '\n'; }
    };
    PlayerbotsReadEvidence e;
    check(ClassifyPlayerbotsRead(e) == ParticipantControl::Unknown,
          "Default never guesses human or bot");

    e.ExplicitStagingEnabled = true;
    e.ReviewedSourceAttested = true;
    e.ModuleEnabled = true;
    e.CharacterInWorld = true;
    e.BotAIRegistryQueried = true;
    e.BotAIRegistryMatched = true;
    check(ClassifyPlayerbotsRead(e) == ParticipantControl::VerifiedPlayerbot,
          "Only positive reviewed registry match is a bot");

    // Every mandatory field must be true before any classification.
    PlayerbotsReadEvidence complete = e;
    for (int i = 0; i < 6; ++i)
    {
        e = complete;
        switch (i)
        {
            case 0: e.ExplicitStagingEnabled = false; break;
            case 1: e.ReviewedSourceAttested = false; break;
            case 2: e.ModuleEnabled = false; break;
            case 3: e.CharacterInWorld = false; break;
            case 4: e.BotAIRegistryQueried = false; break;
            case 5: e.BotAIRegistryMatched = false; break;
        }
        check(ClassifyPlayerbotsRead(e) == ParticipantControl::Unknown,
              "Missing/stale source, disabled module or null AI never proves human");
    }

    // Exhaustively verify 2^6 combinations: there is no human result.
    for (unsigned mask = 0; mask < 64; ++mask)
    {
        PlayerbotsReadEvidence variant;
        variant.ExplicitStagingEnabled = (mask & 1) != 0;
        variant.ReviewedSourceAttested = (mask & 2) != 0;
        variant.ModuleEnabled = (mask & 4) != 0;
        variant.CharacterInWorld = (mask & 8) != 0;
        variant.BotAIRegistryQueried = (mask & 16) != 0;
        variant.BotAIRegistryMatched = (mask & 32) != 0;
        auto const control = ClassifyPlayerbotsRead(variant);
        check(control != ParticipantControl::VerifiedHuman,
              "No combination of negative bot lookup ever means human");
        check(control == (mask == 63 ?
                ParticipantControl::VerifiedPlayerbot : ParticipantControl::Unknown),
              "Exactly one positive match with all gates is VerifiedPlayerbot");
    }

    if (failed) return 1;
    std::cout << "PASS: " << checks
              << " one-way positive Playerbots classification checks\n";
    return 0;
}
