#include "StrongholdStewardEvidence.h"

#include <string>

namespace NaxxGuildStrongholds
{
namespace
{
StewardEvidenceStatus MapLifecycle(LifecycleDecision decision)
{
    switch (decision)
    {
        case LifecycleDecision::DifferentGuild:
            return StewardEvidenceStatus::WrongGuild;
        case LifecycleDecision::WrongGeneration:
            return StewardEvidenceStatus::WrongGeneration;
        case LifecycleDecision::Archived:
            return StewardEvidenceStatus::Archived;
        default:
            return StewardEvidenceStatus::InvalidProperty;
    }
}

std::string StatusDescription(StewardEvidenceStatus status)
{
    switch (status)
    {
        case StewardEvidenceStatus::NoGuild:
            return "No guild membership found.";
        case StewardEvidenceStatus::IdentityUnverified:
            return "Guild creation generation NOT verified.";
        case StewardEvidenceStatus::PropertyNotLoaded:
            return "No authoritative property adapter or loaded ownership.";
        case StewardEvidenceStatus::WrongGuild:
            return "The property belongs to another guild.";
        case StewardEvidenceStatus::WrongGeneration:
            return "Guild ID reused: original owner generation differs.";
        case StewardEvidenceStatus::Archived:
            return "Original property is archived, not active.";
        case StewardEvidenceStatus::InvalidProperty:
            return "Property identity is invalid or unverified.";
        case StewardEvidenceStatus::LevelUnverified:
            return "Settlement level not authoritatively verified.";
        case StewardEvidenceStatus::IpUnverified:
            return "IP state, module availability or limits unverified.";
        case StewardEvidenceStatus::PreviewOnly:
            return "Requirements reviewed in memory, not activated.";
    }
    return "Unknown policy result.";
}

std::string IpDescription(IpReadResult status, std::uint8_t level)
{
    switch (status)
    {
        case IpReadResult::Verified:
            return "Source-verified effective IP rank " + std::to_string(level) +
                " (still no housing).";
        case IpReadResult::UnverifiedSource:
            return "No verified deployed IP source adapter.";
        case IpReadResult::ModuleDisabled:
            return "Individual Progression module is disabled.";
        case IpReadResult::CharacterNotInWorld:
            return "Character not in world; IP status unavailable.";
        case IpReadResult::InvalidRawStage:
            return "Unexpected IP progression stage; review required.";
        case IpReadResult::InvalidProgressionLimit:
            return "Unexpected IP cap; review required.";
    }
    return "IP result unknown.";
}
}

StewardEvidenceReport EvaluateStewardEvidence(StewardEvidenceInput const& input)
{
    StewardEvidenceReport result;
    // Evaluate unknown/disabled IP separately for transparent reporting.
    EffectiveIpStage const ip = EvaluateIpStage(input.Ip);
    result.IpStatus = ip.Result;
    result.EffectiveIpRank = ip.IsVerified() ? ip.Rank : 0;
    result.PlannedActivityGates.fill(ActivityGate::ModuleDisabled);

    if (!input.ActorGuild.GuildId)
        return result;
    if (!input.GuildGenerationVerified || !input.ActorGuild.CreatedAt)
    {
        result.Status = StewardEvidenceStatus::IdentityUnverified;
        return result;
    }
    if (!input.PropertySnapshotLoaded)
    {
        result.Status = StewardEvidenceStatus::PropertyNotLoaded;
        return result;
    }

    LifecycleDecision const life = CheckPropertyLifetime(
        input.Property, input.ActorGuild, true);
    if (life != LifecycleDecision::Allowed)
    {
        result.Status = MapLifecycle(life);
        return result;
    }
    if (!input.SettlementLevelVerified ||
        input.SettlementLevel < 1 || input.SettlementLevel > 7)
    {
        result.Status = StewardEvidenceStatus::LevelUnverified;
        return result;
    }
    if (!ip.IsVerified())
    {
        result.Status = StewardEvidenceStatus::IpUnverified;
        return result;
    }

    result.Status = StewardEvidenceStatus::PreviewOnly;
    ActivityContext context{
        true, input.IsBot, false, true, ip.Rank, input.SettlementLevel,
        input.Property.OriginalGuild.GuildId, input.ActorGuild.GuildId
    };
    auto const& activities = GetActivityCatalog();
    for (std::size_t i = 0; i < activities.size(); ++i)
        result.PlannedActivityGates[i] =
            CheckActivityEligibility(&activities[i], context);

    // NEVER set HousingAvailable, even if a test injects all "true" fields.
    return result;
}

std::vector<std::string> BuildStewardEvidenceRows(
    StewardEvidenceInput const& input)
{
    StewardEvidenceReport const result = EvaluateStewardEvidence(input);
    return {
        "[LOCKED] Housing is not enabled; real isolation is unverified.",
        "[LOCKED] Guild " + std::to_string(input.ActorGuild.GuildId) +
            (input.GuildGenerationVerified && input.ActorGuild.CreatedAt ?
                " member verified; guild created " +
                    std::to_string(input.ActorGuild.CreatedAt) +
                    "; property ownership NOT verified." :
                " member/creation unverified; property ownership NOT verified."),
        "[LOCKED] Property: " + StatusDescription(result.Status),
        "[LOCKED] IP: " + IpDescription(result.IpStatus, result.EffectiveIpRank),
        "[LOCKED] All guild activities, quests and trophies remain inactive."
    };
}
}
