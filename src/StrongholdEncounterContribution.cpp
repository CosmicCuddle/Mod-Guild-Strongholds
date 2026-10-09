#include "StrongholdEncounterContribution.h"

#include <unordered_set>

namespace NaxxGuildStrongholds
{
EncounterContributionReview ReviewEncounterContribution(
    EncounterAttemptEvidence const& attempt, std::uint64_t actorGuid,
    std::vector<EncounterContributionEvent> const& events)
{
    EncounterContributionReview review;
    if (!attempt.AttemptSourceVerified || !attempt.EncounterIdentityVerified ||
        !attempt.CompletionSourceVerified || !attempt.AttemptToken ||
        !attempt.MapId || !attempt.InstanceId || !attempt.BossEntry || !actorGuid)
        return review;

    // Reject reversed, zero-length or implausibly long encounter records.
    // This is a safety bound for a proposal, not an approved raid mechanic.
    constexpr std::uint64_t MaxAttemptDurationMs = 2ULL * 60 * 60 * 1000;
    if (!attempt.StartedAtMs || attempt.CompletedAtMs <= attempt.StartedAtMs ||
        attempt.CompletedAtMs - attempt.StartedAtMs > MaxAttemptDurationMs)
    {
        review.Decision = EncounterContributionDecision::InvalidAttemptWindow;
        return review;
    }

    constexpr std::size_t MaxEventsPerActor = 128;
    if (events.empty() || events.size() > MaxEventsPerActor)
    {
        review.Decision = EncounterContributionDecision::InvalidEvidenceBatch;
        return review;
    }

    std::unordered_set<std::uint64_t> eventIds;
    for (EncounterContributionEvent const& event : events)
    {
        if (!event.EventSourceVerified || !event.EventId)
        {
            review.Decision = EncounterContributionDecision::UnverifiedEvent;
            review.EffectiveEventCount = 0;
            return review;
        }
        if (event.AttemptToken != attempt.AttemptToken ||
            event.ActorGuid != actorGuid || event.MapId != attempt.MapId ||
            event.InstanceId != attempt.InstanceId ||
            event.ObservedAtMs < attempt.StartedAtMs ||
            event.ObservedAtMs > attempt.CompletedAtMs)
        {
            review.Decision = EncounterContributionDecision::MismatchedEvent;
            review.EffectiveEventCount = 0;
            return review;
        }
        if (!eventIds.insert(event.EventId).second)
        {
            review.Decision = EncounterContributionDecision::DuplicateEvent;
            review.EffectiveEventCount = 0;
            return review;
        }

        // Presence, group kill-credit, over-heal, unreviewed support and
        // effects outside the encounter NEVER establish activity.
        bool const reviewedEffectiveAction =
            event.Action == EncounterActionKind::EffectiveDamage ||
            event.Action == EncounterActionKind::EffectiveHealing ||
            event.Action == EncounterActionKind::EffectiveMitigation;
        if (reviewedEffectiveAction && event.EffectiveAmount &&
            event.ActorGroupAtEventVerified &&
            event.TargetBoundToEncounterVerified)
            ++review.EffectiveEventCount;
    }

    review.Decision = review.EffectiveEventCount >= 2 ?
        EncounterContributionDecision::CandidateForStagingReview :
        EncounterContributionDecision::InsufficientEffectiveActivity;

    // Deliberately NO success-to-reward or success-to-verified-human path.
    return review;
}
} // namespace NaxxGuildStrongholds
