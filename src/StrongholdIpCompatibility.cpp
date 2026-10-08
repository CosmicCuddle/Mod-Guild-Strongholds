#include "StrongholdIpCompatibility.h"

namespace NaxxGuildStrongholds
{
EffectiveIpStage EvaluateIpStage(IpRuntimeSnapshot const& runtime)
{
    if (!runtime.SourceContractVerified)
        return {IpReadResult::UnverifiedSource, 0};
    if (!runtime.ModuleEnabled)
        return {IpReadResult::ModuleDisabled, 0};
    if (!runtime.CharacterInWorld)
        return {IpReadResult::CharacterNotInWorld, 0};
    if (runtime.RawQuestStage > 18)
        return {IpReadResult::InvalidRawStage, 0};
    if (runtime.ProgressionLimit < 0)
        return {IpReadResult::InvalidProgressionLimit, 0};

    std::uint32_t effective = runtime.RawQuestStage;
    // The fork's hasPassedProgression checks progressionLimit before
    // comparing rewarded quests. A raw quest stage alone bypasses the cap.
    if (runtime.ProgressionLimit != 0 &&
        effective > static_cast<std::uint32_t>(runtime.ProgressionLimit))
        effective = static_cast<std::uint32_t>(runtime.ProgressionLimit);
    return {IpReadResult::Verified, static_cast<std::uint8_t>(effective)};
}

ActivityContext ApplyIpStage(ActivityContext const& base,
    IpRuntimeSnapshot const& runtime)
{
    ActivityContext adjusted = base;
    EffectiveIpStage const verified = EvaluateIpStage(runtime);
    adjusted.HasVerifiedIpState = verified.IsVerified();
    adjusted.VerifiedIpRank = verified.IsVerified() ? verified.Rank : 0;
    return adjusted;
}
}
