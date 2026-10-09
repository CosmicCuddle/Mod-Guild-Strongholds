#include "StrongholdPrivacyMatrix.h"
#include <array>
namespace NaxxGuildStrongholds
{
PrivacyReviewResult ReviewPrivacyMatrix(PrivacyMatrixInput const& input)
{
    PrivacyReviewResult result;
    if (!input.ExactInstalledSourceReviewed || !input.StagingBuildVerified ||
        !input.OriginalGuildIdentitiesVerified)
        return result;
    if (!input.LogicalSite || !input.GuildA.GuildId || !input.GuildB.GuildId ||
        !input.GuildA.CreatedAt || !input.GuildB.CreatedAt ||
        (input.GuildA.GuildId == input.GuildB.GuildId &&
         input.GuildA.CreatedAt == input.GuildB.CreatedAt))
    {
        result.Decision = PrivacyReviewDecision::InvalidGuildIdentity;
        return result;
    }
    constexpr std::size_t C = static_cast<std::size_t>(PrivacyContext::Count);
    constexpr std::size_t S = static_cast<std::size_t>(PrivacySurface::Count);
    constexpr std::size_t Required = C * (2*S + 6);
    if (input.Observations.empty() || input.Observations.size() > Required)
    {
        result.Decision = PrivacyReviewDecision::InvalidEvidence;
        return result;
    }
    std::array<bool, C*S*4> seen{};
    bool crossLeak = false, ownHidden = false;
    for (auto const& o : input.Observations)
    {
        auto c = static_cast<std::size_t>(o.Context);
        auto s = static_cast<std::size_t>(o.Surface);
        if (c >= C || s >= S || o.ObserverGuild > 1 || o.TargetGuild > 1 ||
            !o.ServerObservationAttested || !o.ObserverMap ||
            !o.TargetMap || o.ObserverMap != o.TargetMap ||
            (o.ObserverGuild == o.TargetGuild && s > 2))
        {
            result.Decision = PrivacyReviewDecision::InvalidEvidence;
            return result;
        }
        auto key = ((c*S+s)*2+o.ObserverGuild)*2+o.TargetGuild;
        if (seen[key])
        {
            result.Decision = PrivacyReviewDecision::DuplicateEvidence;
            return result;
        }
        seen[key] = true;
        if (o.ObserverGuild != o.TargetGuild && o.VisibleOrInteractive)
            crossLeak = true;
        if (o.ObserverGuild == o.TargetGuild && !o.VisibleOrInteractive)
            ownHidden = true;
    }
    if (crossLeak)
    {
        result.Decision = PrivacyReviewDecision::CrossGuildLeak;
        return result;
    }
    if (ownHidden)
    {
        result.Decision = PrivacyReviewDecision::OwnGuildHidden;
        return result;
    }
    for (std::size_t c = 0; c < C; ++c)
        for (std::size_t s = 0; s < S; ++s)
            for (std::size_t from = 0; from < 2; ++from)
                for (std::size_t to = 0; to < 2; ++to)
                {
                    // Same-guild gossip/aura/combat can legitimately be
                    // blocked by personal IP, rank or combat permissions.
                    if (from == to && s > 2) continue;
                    if (!seen[((c*S+s)*2+from)*2+to])
                    {
                        result.Decision = PrivacyReviewDecision::MissingCoverage;
                        return result;
                    }
                }
    // Operator-entered reports can be forged or incomplete in ways the
    // model cannot detect. Never turn this candidate into runtime approval.
    result.Decision = PrivacyReviewDecision::CandidateForManualStagingReview;
    return result;
}
}
