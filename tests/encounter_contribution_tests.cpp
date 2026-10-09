#include "StrongholdEncounterContribution.h"

#include <iostream>
#include <vector>

using namespace NaxxGuildStrongholds;

int main()
{
    unsigned checks = 0, failed = 0;
    auto check = [&](bool okay, char const* label) {
        ++checks;
        if (!okay) { ++failed; std::cerr << "FAILED: " << label << '\n'; }
    };

    EncounterAttemptEvidence attempt{901, 249, 321, 10184, 100000, 220000,
                                     true, true, true};
    EncounterContributionEvent damage{1, 901, 42, 249, 321, 110000,
        EncounterActionKind::EffectiveDamage, 150, true, true, true};
    EncounterContributionEvent heal = damage;
    heal.EventId = 2;
    heal.ObservedAtMs = 150000;
    heal.Action = EncounterActionKind::EffectiveHealing;
    heal.EffectiveAmount = 75;
    auto review = ReviewEncounterContribution(attempt, 42, {damage, heal});
    check(review.Decision == EncounterContributionDecision::CandidateForStagingReview &&
          review.EffectiveEventCount == 2, "Two independently logged encounter effects are review candidates");
    check(!review.EncounterParticipationVerified && !review.TrophyGranted,
          "Even positive synthetic samples grant nothing and verify nobody");

    auto a = attempt;
    a.AttemptSourceVerified = false;
    check(ReviewEncounterContribution(a, 42, {damage, heal}).Decision ==
          EncounterContributionDecision::UnverifiedAttempt, "Unreviewed source refused");
    a = attempt; a.EncounterIdentityVerified = false;
    check(ReviewEncounterContribution(a, 42, {damage, heal}).Decision ==
          EncounterContributionDecision::UnverifiedAttempt, "Boss identity not proven");
    a = attempt; a.CompletionSourceVerified = false;
    check(ReviewEncounterContribution(a, 42, {damage, heal}).Decision ==
          EncounterContributionDecision::UnverifiedAttempt, "Generic boss death not completion proof");
    a = attempt; a.AttemptToken = 0;
    check(ReviewEncounterContribution(a, 42, {damage, heal}).Decision ==
          EncounterContributionDecision::UnverifiedAttempt, "Zero attempt refused");
    a = attempt; a.BossEntry = 0;
    check(ReviewEncounterContribution(a, 42, {damage, heal}).Decision ==
          EncounterContributionDecision::UnverifiedAttempt, "Unknown boss refused");
    check(ReviewEncounterContribution(attempt, 0, {damage, heal}).Decision ==
          EncounterContributionDecision::UnverifiedAttempt, "Zero actor refused");

    a = attempt; a.CompletedAtMs = a.StartedAtMs;
    check(ReviewEncounterContribution(a, 42, {damage, heal}).Decision ==
          EncounterContributionDecision::InvalidAttemptWindow, "Zero duration refused");
    a = attempt; a.CompletedAtMs = a.StartedAtMs - 1;
    check(ReviewEncounterContribution(a, 42, {damage, heal}).Decision ==
          EncounterContributionDecision::InvalidAttemptWindow, "Reverse duration refused");
    a = attempt; a.CompletedAtMs += 8000000;
    check(ReviewEncounterContribution(a, 42, {damage, heal}).Decision ==
          EncounterContributionDecision::InvalidAttemptWindow, "Unbounded attempt refused");
    check(ReviewEncounterContribution(attempt, 42, {}).Decision ==
          EncounterContributionDecision::InvalidEvidenceBatch, "Empty source batch refused");
    check(ReviewEncounterContribution(attempt, 42,
          std::vector<EncounterContributionEvent>(129, damage)).Decision ==
          EncounterContributionDecision::InvalidEvidenceBatch, "Excessive batch refused");

    auto e = heal; e.EventSourceVerified = false;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::UnverifiedEvent, "Forged source event poisons batch");
    e = heal; e.EventId = 0;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::UnverifiedEvent, "Zero event ID refused");
    e = heal; e.AttemptToken++;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::MismatchedEvent, "Other boss attempt refused");
    e = heal; e.ActorGuid = 43;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::MismatchedEvent, "Another raid member refused");
    e = heal; e.MapId++;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::MismatchedEvent, "Wrong map refused");
    e = heal; e.InstanceId++;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::MismatchedEvent, "Wrong instance refused");
    e = heal; e.ObservedAtMs = attempt.StartedAtMs - 1;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::MismatchedEvent, "Pre-pull buff refused");
    e = heal; e.ObservedAtMs = attempt.CompletedAtMs + 1;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::MismatchedEvent, "Post-kill event refused");
    e = heal; e.EventId = damage.EventId;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::DuplicateEvent, "Duplicate event replay refused");

    e = heal; e.Action = EncounterActionKind::PresenceOnly;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::InsufficientEffectiveActivity, "Spectators not counted");
    e = heal; e.Action = EncounterActionKind::KillCreditOnly;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::InsufficientEffectiveActivity, "Pet/totem group kill credit not enough");
    e = heal; e.Action = EncounterActionKind::UnreviewedSupport;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::InsufficientEffectiveActivity, "Unreviewed buff not counted");
    e = heal; e.EffectiveAmount = 0;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::InsufficientEffectiveActivity, "Overheal or zero effect not counted");
    e = heal; e.ActorGroupAtEventVerified = false;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::InsufficientEffectiveActivity, "Group identity must be event-time verified");
    e = heal; e.TargetBoundToEncounterVerified = false;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::InsufficientEffectiveActivity, "Healing unrelated player cannot count");
    check(ReviewEncounterContribution(attempt, 42, {damage}).Decision ==
          EncounterContributionDecision::InsufficientEffectiveActivity, "One tag not enough");

    e = heal; e.Action = EncounterActionKind::EffectiveMitigation; e.EventId = 3;
    check(ReviewEncounterContribution(attempt, 42, {damage, e}).Decision ==
          EncounterContributionDecision::CandidateForStagingReview, "Tank mitigation can contribute");
    check(ReviewEncounterContribution(attempt, 42, {heal, e}).Decision ==
          EncounterContributionDecision::CandidateForStagingReview, "Healing plus mitigation can contribute");

    if (failed) return 1;
    std::cout << "PASS: " << checks
              << " attempt-bound contribution provenance tests (proposal-only)\n";
    return 0;
}
