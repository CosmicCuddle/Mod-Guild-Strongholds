#ifndef NAXX_GS_RAID_MEMBERSHIP_TIMELINE_H
#define NAXX_GS_RAID_MEMBERSHIP_TIMELINE_H
#include "StrongholdEncounterContribution.h"
#include "StrongholdLifecycle.h"
#include <cstdint>
#include <vector>

namespace NaxxGuildStrongholds
{
// SYNTHETIC source events only; EndAtMs=0 means still in group.
struct RaidMembershipWindow
{
    std::uint64_t AttemptToken = 0;
    std::uint64_t CharacterGuid = 0;
    std::uint64_t ServerGroupToken = 0;
    GuildIdentity GuildAtWindow;
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    std::uint64_t JoinedAtMs = 0;
    std::uint64_t EndAtMs = 0;
    bool MembershipSourceVerified = false;
    bool GuildGenerationSourceVerified = false;
};
struct RaidMembershipRequest
{
    EncounterAttemptEvidence Attempt;
    std::uint64_t CharacterGuid = 0;
    std::uint64_t ExpectedGroupToken = 0;
    GuildIdentity ExpectedGuild;
    std::uint64_t EventAtMs = 0;
    std::vector<RaidMembershipWindow> Windows;
};
enum class RaidMembershipDecision : std::uint8_t
{
    UnverifiedAttempt,
    InvalidRequest,
    InvalidTimeline,
    UnverifiedWindow,
    ForeignWindow,
    OverlappingWindow,
    NotMemberAtEvent,
    WrongGroupAtEvent,
    WrongGuildGeneration,
    CandidateForStagingReview
};
struct RaidMembershipReview
{
    RaidMembershipDecision Decision = RaidMembershipDecision::UnverifiedAttempt;
    // NEVER set true: synthetic time windows are not source-of-truth adapters.
    bool MemberAtEventConfirmed = false;
    bool HumanControlConfirmed = false;
    bool TrophyGranted = false;
};
RaidMembershipReview ReviewRaidMembershipAtEvent(
    RaidMembershipRequest const& request);
}
#endif
