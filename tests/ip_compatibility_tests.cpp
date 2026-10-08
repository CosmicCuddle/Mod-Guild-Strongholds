#include "StrongholdIpCompatibility.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0;
    int failed = 0;
    auto expect = [&](bool pass, char const* name)
    {
        ++checks;
        if (!pass)
        {
            ++failed;
            std::cerr << "FAILED: " << name << '\n';
        }
    };

    IpRuntimeSnapshot source{true, true, true, 18, 0};
    expect(EvaluateIpStage(source).IsVerified(), "Reviewed and enabled IP module");
    expect(EvaluateIpStage(source).Rank == 18, "No cap keeps actual quest rank");

    source.ProgressionLimit = 8;
    expect(EvaluateIpStage(source).Rank == 8, "Vanilla-era cap bounds completed TBC/WotLK quests");
    source.ProgressionLimit = 7;
    expect(EvaluateIpStage(source).Rank == 7, "Before-TBC capped players cannot use Outland");
    source.ProgressionLimit = 13;
    expect(EvaluateIpStage(source).Rank == 13, "Before-Wrath cap permits pre-Wrath rank only");
    source.ProgressionLimit = 17;
    expect(EvaluateIpStage(source).Rank == 17, "Upper cap still respected");

    source.RawQuestStage = 2;
    source.ProgressionLimit = 13;
    expect(EvaluateIpStage(source).Rank == 2, "Upper server limit cannot grant missing quests");
    source.RawQuestStage = 0;
    expect(EvaluateIpStage(source).IsVerified() && EvaluateIpStage(source).Rank == 0,
        "Legitimate fresh character may have verified rank zero");
    source.RawQuestStage = 19;
    expect(EvaluateIpStage(source).Result == IpReadResult::InvalidRawStage,
        "Unexpected stage enum requires review");
    source.RawQuestStage = 0;
    source.ProgressionLimit = -1;
    expect(EvaluateIpStage(source).Result == IpReadResult::InvalidProgressionLimit,
        "Unusual negative progression setting fails closed");
    source.ProgressionLimit = 0;
    source.SourceContractVerified = false;
    expect(EvaluateIpStage(source).Result == IpReadResult::UnverifiedSource,
        "Unreviewed fork must not unlock activities");
    source.SourceContractVerified = true;
    source.ModuleEnabled = false;
    expect(EvaluateIpStage(source).Result == IpReadResult::ModuleDisabled,
        "Disabled IP cannot grant any activities");
    source.ModuleEnabled = true;
    source.CharacterInWorld = false;
    expect(EvaluateIpStage(source).Result == IpReadResult::CharacterNotInWorld,
        "No reliable IP result during loading/offline");
    source.CharacterInWorld = true;

    ActivityContext normal{true, false, false, true, 18, 4, 100, 100};
    source.RawQuestStage = 18;
    source.ProgressionLimit = 7;
    ActivityContext capped = ApplyIpStage(normal, source);
    expect(capped.HasVerifiedIpState && capped.VerifiedIpRank == 7,
        "Stale high-tier proof is overwritten by effective cap");
    expect(CheckActivityEligibility(FindActivity("weekly_outland_expedition"), capped)
           == ActivityGate::ProgressionTooLow, "High quest rank cannot bypass Vanilla cap");

    source.ProgressionLimit = 8;
    ActivityContext outland = ApplyIpStage(normal, source);
    expect(CheckActivityEligibility(FindActivity("weekly_outland_expedition"), outland)
           == ActivityGate::Allowed, "Verified pre-TBC milestone can take Outland project");
    expect(CheckActivityEligibility(FindActivity("weekly_northrend_expedition"), outland)
           == ActivityGate::ProgressionTooLow, "TBC stage cannot see Northrend");

    source.ProgressionLimit = 13;
    ActivityContext northrend = ApplyIpStage(normal, source);
    expect(CheckActivityEligibility(FindActivity("weekly_northrend_expedition"), northrend)
           == ActivityGate::Allowed, "Verified pre-Wrath milestone can take Northrend");
    source.ModuleEnabled = false;
    ActivityContext disabled = ApplyIpStage(normal, source);
    expect(!disabled.HasVerifiedIpState && disabled.VerifiedIpRank == 0,
        "IP disabled clears previously trusted proof");
    expect(CheckActivityEligibility(FindActivity("daily_supply_run"), disabled)
           == ActivityGate::NoProgressionProof, "Even Vanilla guild quest fails closed when IP disabled");

    source.ModuleEnabled = true;
    source.SourceContractVerified = false;
    ActivityContext modified = ApplyIpStage(normal, source);
    expect(!modified.HasVerifiedIpState && modified.VerifiedIpRank == 0,
        "Unreviewed source revokes cached IP rank");
    expect(CheckActivityEligibility(FindActivity("onyxia_trophy_request"), modified)
           == ActivityGate::NoProgressionProof, "Unknown fork blocks trophy activity");

    if (failed)
        return 1;
    std::cout << "PASS: " << checks << " Grimfeather fork-aware IP eligibility checks\n";
    return 0;
}
