#include "StrongholdIsolationProbe.h"

#include <iostream>
#include <vector>

using namespace NaxxGuildStrongholds;

namespace
{
std::vector<IsolationSample> MakeSamples(std::uint32_t firstMask,
    std::uint32_t secondMask, VisibilityMode mode = VisibilityMode::CombinedBits)
{
    return {
        {100, 1, 0, firstMask, mode, ObservationKind::Player},
        {100, 1, 0, firstMask, mode, ObservationKind::Creature},
        {100, 1, 0, firstMask, mode, ObservationKind::GameObject},
        {200, 1, 0, secondMask, mode, ObservationKind::Player},
        {200, 1, 0, secondMask, mode, ObservationKind::Creature},
        {200, 1, 0, secondMask, mode, ObservationKind::GameObject}
    };
}
}

int main()
{
    int checks = 0;
    int failures = 0;
    auto test = [&](bool passes, char const* message)
    {
        ++checks;
        if (!passes)
        {
            ++failures;
            std::cerr << "FAILED: " << message << '\n';
        }
    };

    test(!IsSingleBitPhase(0), "Zero is not a single phase");
    test(IsSingleBitPhase(1), "Bit 0 allowed");
    test(IsSingleBitPhase(0x80000000u), "Bit 31 allowed");
    test(!IsSingleBitPhase(3), "Combined two-phase flag not exclusive");
    test(!IsSingleBitPhase(0xffffffffu), "All-phase mask cannot isolate");

    auto safe = MakeSamples(1, 2);
    test(EvaluateIsolationSamples({}) == ProbeVerdict::NoSamples,
        "Empty observation list is not proof");
    test(EvaluateIsolationSamples({safe[0]}) == ProbeVerdict::OnlyOneGuild,
        "One guild cannot prove separation");
    test(EvaluateIsolationSamples(safe) == ProbeVerdict::PairwiseSeparatedInSamples,
        "Two guilds with disjoint masks are separated in synthetic model");
    auto missing = safe;
    missing.pop_back();
    test(EvaluateIsolationSamples(missing) == ProbeVerdict::MissingRequiredKinds,
        "Need player, creature and gameobject samples for each guild");

    auto sameMask = MakeSamples(1, 1);
    test(EvaluateIsolationSamples(sameMask) == ProbeVerdict::OtherGuildVisible,
        "Phase bit collision leaks cross-guild data");

    auto allPhase = MakeSamples(1, 2);
    allPhase[3].PhaseMask = 0xffffffffu;
    test(EvaluateIsolationSamples(allPhase) == ProbeVerdict::OtherGuildVisible,
        "All-phases actor can see the other guild");
    auto normal = MakeSamples(1, 2);
    normal[3].PhaseMask = 0;
    test(EvaluateIsolationSamples(normal) == ProbeVerdict::MalformedSample,
        "Zero phase is rejected");

    auto exact = MakeSamples(1000, 2000, VisibilityMode::ExactValue);
    test(EvaluateIsolationSamples(exact) == ProbeVerdict::PairwiseSeparatedInSamples,
        "Exact mode distinct values separate guilds within given samples");
    // Critical danger: asymmetric visibility. Exclusive mask '3' compares
    // unequal to 1, but combined mask '1' intersects 3 and sees that object.
    IsolationSample a{100, 1, 0, 3, VisibilityMode::ExactValue, ObservationKind::Player};
    IsolationSample b{200, 1, 0, 1, VisibilityMode::CombinedBits, ObservationKind::Player};
    test(!CanSampleSee(a, b) && CanSampleSee(b, a),
        "Mixed phase-comparison modes can produce one-way visibility");

    auto mixed = MakeSamples(3, 1, VisibilityMode::ExactValue);
    mixed[3].Mode = VisibilityMode::CombinedBits;
    test(EvaluateIsolationSamples(mixed) == ProbeVerdict::OtherGuildVisible,
        "One-way cross-guild visibility must still be blocked");

    auto brokenOwn = MakeSamples(1, 2);
    brokenOwn[1].PhaseMask = 4;
    test(EvaluateIsolationSamples(brokenOwn) == ProbeVerdict::OwnGuildHidden,
        "Guildmates must see guild NPCs");
    auto separateInstances = MakeSamples(1, 1);
    for (std::size_t i = 3; i < separateInstances.size(); ++i)
        separateInstances[i].InstanceId = 77;
    test(EvaluateIsolationSamples(separateInstances) ==
         ProbeVerdict::PairwiseSeparatedInSamples,
        "Distinct true instance IDs separate same-phase properties in model");
    auto mismatchedOwn = MakeSamples(1, 2);
    mismatchedOwn[1].InstanceId = 1;
    test(EvaluateIsolationSamples(mismatchedOwn) == ProbeVerdict::OwnGuildHidden,
        "Guild object on wrong map instance is not visible");
    auto badGuild = MakeSamples(1, 2);
    badGuild[0].GuildId = 0;
    test(EvaluateIsolationSamples(badGuild) == ProbeVerdict::MalformedSample,
        "Unknown owning guild is not proof");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks << " bidirectional phase and instance model checks\n";
    return 0;
}
