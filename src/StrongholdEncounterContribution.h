#ifndef NAXX_GUILD_STRONGHOLDS_ENCOUNTER_CONTRIBUTION_H
#define NAXX_GUILD_STRONGHOLDS_ENCOUNTER_CONTRIBUTION_H

#include <cstdint>
#include <vector>

// Offline domain policy only. An event is never proof of a human player or
// a trophy award. A deployed-source-reviewed adapter does not exist yet.
namespace NaxxGuildStrongholds
{
enum class EncounterActionKind : std::uint8_t
{
    PresenceOnly,
    KillCreditOnly,
    EffectiveDamage,
    EffectiveHealing,
    EffectiveMitigation,
    UnreviewedSupport
};

struct EncounterAttemptEvidence
{
    std::uint64_t AttemptToken = 0; // Opaque server-owned attempt identity
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    std::uint32_t BossEntry = 0;
    std::uint64_t StartedAtMs = 0;
    std::uint64_t CompletedAtMs = 0;
    bool AttemptSourceVerified = false;
    bool EncounterIdentityVerified = false;
    bool CompletionSourceVerified = false;
};

struct EncounterContributionEvent
{
    std::uint64_t EventId = 0; // Unique within this reviewed server attempt
    std::uint64_t AttemptToken = 0;
    std::uint64_t ActorGuid = 0;
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    std::uint64_t ObservedAtMs = 0;
    EncounterActionKind Action = EncounterActionKind::PresenceOnly;
    std::uint64_t EffectiveAmount = 0;
    bool EventSourceVerified = false;
    bool ActorGroupAtEventVerified = false;
    bool TargetBoundToEncounterVerified = false;
};

enum class EncounterContributionDecision : std::uint8_t
{
    UnverifiedAttempt,
    InvalidAttemptWindow,
    InvalidEvidenceBatch,
    UnverifiedEvent,
    MismatchedEvent,
    DuplicateEvent,
    InsufficientEffectiveActivity,
    CandidateForStagingReview
};

struct EncounterContributionReview
{
    EncounterContributionDecision Decision =
        EncounterContributionDecision::UnverifiedAttempt;
    std::uint8_t EffectiveEventCount = 0;
    // MUST stay false. A candidate is not an authoritative positive
    // participation proof and must never be copied to raid eligibility.
    bool EncounterParticipationVerified = false;
    bool TrophyGranted = false;
};

// Consumes hypothetical SERVER-VERIFIED event evidence. No runtime collector.
// Two qualifying events are needed for a *review candidate* only; this is NOT
// a finalized activity threshold for guild trophy eligibility.
EncounterContributionReview ReviewEncounterContribution(
    EncounterAttemptEvidence const& attempt, std::uint64_t actorGuid,
    std::vector<EncounterContributionEvent> const& events);
}
#endif
