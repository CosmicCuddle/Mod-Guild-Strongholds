#include "StrongholdIsolationProbe.h"

#include <array>
#include <set>

namespace NaxxGuildStrongholds
{
bool CanSampleSee(IsolationSample const& observer,
    IsolationSample const& target)
{
    if (observer.MapId != target.MapId ||
        observer.InstanceId != target.InstanceId)
        return false;

    // AzerothCore Object.h WorldObject::InSamePhase(uint32):
    // m_useCombinedPhases ? GetPhaseMask() & phasemask
    //                    : GetPhaseMask() == phasemask
    if (observer.Mode == VisibilityMode::CombinedBits)
        return (observer.PhaseMask & target.PhaseMask) != 0;
    return observer.PhaseMask == target.PhaseMask;
}

bool IsSingleBitPhase(std::uint32_t mask)
{
    return mask != 0 && (mask & (mask - 1)) == 0;
}

ProbeVerdict EvaluateIsolationSamples(
    std::vector<IsolationSample> const& samples)
{
    if (samples.empty())
        return ProbeVerdict::NoSamples;

    std::set<std::uint32_t> guildIds;
    for (IsolationSample const& item : samples)
    {
        if (item.GuildId == 0 || item.PhaseMask == 0)
            return ProbeVerdict::MalformedSample;
        guildIds.insert(item.GuildId);
    }
    if (guildIds.size() < 2)
        return ProbeVerdict::OnlyOneGuild;

    for (std::uint32_t guildId : guildIds)
    {
        std::array<bool, 3> found{};
        for (IsolationSample const& item : samples)
            if (item.GuildId == guildId)
                found[static_cast<std::size_t>(item.Kind)] = true;
        for (bool entry : found)
            if (!entry)
                return ProbeVerdict::MissingRequiredKinds;
    }

    // ALL observations are checked in both directions. This matters when
    // different modules select opposite phase-comparison modes.
    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        for (std::size_t j = i + 1; j < samples.size(); ++j)
        {
            IsolationSample const& first = samples[i];
            IsolationSample const& second = samples[j];

            // Two observations of the same guild should mutually see
            // each other; any cross-guild visibility is a privacy failure.
            const bool firstSeesSecond = CanSampleSee(first, second);
            const bool secondSeesFirst = CanSampleSee(second, first);
            if (first.GuildId == second.GuildId)
            {
                if (!firstSeesSecond || !secondSeesFirst)
                    return ProbeVerdict::OwnGuildHidden;
            }
            else if (firstSeesSecond || secondSeesFirst)
                return ProbeVerdict::OtherGuildVisible;
        }
    }
    return ProbeVerdict::PairwiseSeparatedInSamples;
}
}
