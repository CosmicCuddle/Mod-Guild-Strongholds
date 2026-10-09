#include "StrongholdRaidMembershipTimeline.h"
#include <algorithm>
#include <unordered_map>

namespace NaxxGuildStrongholds
{
RaidMembershipReview ReviewRaidMembershipAtEvent(
    RaidMembershipRequest const& request)
{
    RaidMembershipReview result;
    auto const& a = request.Attempt;
    constexpr std::uint64_t MaxAttemptMs = 2ULL * 60 * 60 * 1000;
    if (!a.AttemptSourceVerified || !a.EncounterIdentityVerified ||
        !a.CompletionSourceVerified || !a.AttemptToken || !a.MapId ||
        !a.InstanceId || !a.BossEntry || !a.StartedAtMs ||
        a.CompletedAtMs <= a.StartedAtMs ||
        a.CompletedAtMs - a.StartedAtMs > MaxAttemptMs)
        return result;
    if (!request.CharacterGuid || !request.ExpectedGroupToken ||
        !request.ExpectedGuild.GuildId || !request.ExpectedGuild.CreatedAt ||
        request.EventAtMs < a.StartedAtMs || request.EventAtMs > a.CompletedAtMs)
    {
        result.Decision = RaidMembershipDecision::InvalidRequest;
        return result;
    }
    // 160 is only a test memory bound, NOT an approved in-game roster rule.
    if (request.Windows.empty() || request.Windows.size() > 160)
    {
        result.Decision = RaidMembershipDecision::InvalidTimeline;
        return result;
    }
    std::unordered_map<std::uint64_t, std::vector<RaidMembershipWindow const*>> roster;
    for (auto const& window : request.Windows)
    {
        if (!window.MembershipSourceVerified ||
            !window.GuildGenerationSourceVerified)
        {
            result.Decision = RaidMembershipDecision::UnverifiedWindow;
            return result;
        }
        if (!window.CharacterGuid || !window.ServerGroupToken ||
            !window.GuildAtWindow.GuildId || !window.GuildAtWindow.CreatedAt ||
            !window.JoinedAtMs || window.JoinedAtMs < a.StartedAtMs ||
            window.JoinedAtMs > a.CompletedAtMs ||
            (window.EndAtMs && window.EndAtMs <= window.JoinedAtMs))
        {
            result.Decision = RaidMembershipDecision::InvalidTimeline;
            return result;
        }
        if (window.AttemptToken != a.AttemptToken ||
            window.MapId != a.MapId || window.InstanceId != a.InstanceId)
        {
            result.Decision = RaidMembershipDecision::ForeignWindow;
            return result;
        }
        roster[window.CharacterGuid].push_back(&window);
    }
    // Even two overlapping windows unrelated to the selected actor poison
    // the claimed roster source, including two duplicate/open windows.
    for (auto& each : roster)
    {
        auto& windows = each.second;
        std::sort(windows.begin(), windows.end(),
                  [](auto const* l, auto const* r)
                  { return l->JoinedAtMs < r->JoinedAtMs; });
        for (std::size_t i = 1; i < windows.size(); ++i)
            if (!windows[i-1]->EndAtMs ||
                windows[i-1]->EndAtMs > windows[i]->JoinedAtMs)
            {
                result.Decision = RaidMembershipDecision::OverlappingWindow;
                return result;
            }
    }
    auto entry = roster.find(request.CharacterGuid);
    if (entry == roster.end())
    {
        result.Decision = RaidMembershipDecision::NotMemberAtEvent;
        return result;
    }
    for (auto const* window : entry->second)
    {
        // Half-open window: [join,leave). No credit at the departure boundary.
        if (request.EventAtMs < window->JoinedAtMs ||
            (window->EndAtMs && request.EventAtMs >= window->EndAtMs))
            continue;
        if (window->ServerGroupToken != request.ExpectedGroupToken)
        {
            result.Decision = RaidMembershipDecision::WrongGroupAtEvent;
            return result;
        }
        if (window->GuildAtWindow.GuildId != request.ExpectedGuild.GuildId ||
            window->GuildAtWindow.CreatedAt != request.ExpectedGuild.CreatedAt)
        {
            result.Decision = RaidMembershipDecision::WrongGuildGeneration;
            return result;
        }
        result.Decision = RaidMembershipDecision::CandidateForStagingReview;
        return result;
    }
    result.Decision = RaidMembershipDecision::NotMemberAtEvent;
    return result;
}
}
