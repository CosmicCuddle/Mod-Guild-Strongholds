#include "StrongholdRaidMembershipTimeline.h"
#include <iostream>
using namespace NaxxGuildStrongholds;
int main()
{
    unsigned checks = 0, failures = 0;
    auto test = [&](bool ok, char const* name) {
        ++checks;
        if (!ok) { ++failures; std::cerr << "FAIL " << name << '\n'; }
    };
    RaidMembershipRequest input;
    input.Attempt = {321, 249, 700, 10184, 1000, 30000, true, true, true};
    input.CharacterGuid = 42;
    input.ExpectedGroupToken = 456;
    input.ExpectedGuild = {18, 1800000000};
    input.EventAtMs = 3500;
    RaidMembershipWindow w;
    w.AttemptToken = 321;
    w.CharacterGuid = 42;
    w.ServerGroupToken = 456;
    w.GuildAtWindow = input.ExpectedGuild;
    w.MapId = 249;
    w.InstanceId = 700;
    w.JoinedAtMs = 1500;
    w.EndAtMs = 4000;
    w.MembershipSourceVerified = true;
    w.GuildGenerationSourceVerified = true;
    input.Windows = {w};

    auto result = ReviewRaidMembershipAtEvent(input);
    test(result.Decision == RaidMembershipDecision::CandidateForStagingReview,
         "Positive synthetic window review");
    test(!result.MemberAtEventConfirmed && !result.HumanControlConfirmed &&
         !result.TrophyGranted, "No synthetic review can certify gameplay");

    auto q = input; q.Attempt.CompletionSourceVerified = false;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::UnverifiedAttempt, "Unverified boss end");
    q = input; q.Attempt.AttemptSourceVerified = false;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::UnverifiedAttempt, "Unverified attempt");
    q = input; q.Attempt.BossEntry = 0;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::UnverifiedAttempt, "Unknown boss entry");
    q = input; q.Attempt.CompletedAtMs = q.Attempt.StartedAtMs;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::UnverifiedAttempt, "Zero attempt duration");
    q = input; q.CharacterGuid = 0;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidRequest, "No member GUID");
    q = input; q.ExpectedGroupToken = 0;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidRequest, "No server group token");
    q = input; q.ExpectedGuild.CreatedAt = 0;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidRequest, "No original guild generation");
    q = input; q.EventAtMs = 999;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidRequest, "Before pull");
    q = input; q.EventAtMs = 30001;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidRequest, "After kill");
    q = input; q.Windows.clear();
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidTimeline, "No history");
    q = input; q.Windows.assign(161, w);
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidTimeline, "Bounded history");

    auto changed = w;
    changed.MembershipSourceVerified = false;
    q = input; q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::UnverifiedWindow, "Untrusted group record");
    changed = w; changed.GuildGenerationSourceVerified = false;
    q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::UnverifiedWindow, "Untrusted guild record");
    changed = w; changed.JoinedAtMs = 31000;
    q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidTimeline, "After boss completed");
    changed = w; changed.EndAtMs = changed.JoinedAtMs;
    q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::InvalidTimeline, "Zero membership duration");
    changed = w; changed.AttemptToken = 999;
    q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::ForeignWindow, "Prior wipe refused");
    changed = w; changed.InstanceId++;
    q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::ForeignWindow, "Wrong instance refused");

    q = input; q.CharacterGuid = 99;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::NotMemberAtEvent, "Outside roster");
    q = input; q.EventAtMs = w.EndAtMs;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::NotMemberAtEvent, "Leave boundary excluded");
    q.EventAtMs = w.JoinedAtMs;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::CandidateForStagingReview, "Join boundary included");

    changed = w; changed.ServerGroupToken++;
    q = input; q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::WrongGroupAtEvent, "Group switch");
    changed = w; changed.GuildAtWindow.CreatedAt++;
    q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::WrongGuildGeneration, "Recycled guild ID");
    changed = w; changed.GuildAtWindow.GuildId++;
    q.Windows = {changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::WrongGuildGeneration, "Other guild");
    changed = w; changed.JoinedAtMs = 3000; changed.EndAtMs = 4500;
    q.Windows = {w, changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::OverlappingWindow, "Overlapping records");

    changed = w; changed.JoinedAtMs = 4500; changed.EndAtMs = 0;
    q = input; q.Windows = {w, changed}; q.EventAtMs = 4200;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::NotMemberAtEvent, "Gap after leave");
    q.EventAtMs = 4700;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::CandidateForStagingReview, "Rejoin review candidate");
    changed.JoinedAtMs = w.EndAtMs;
    q.Windows = {changed, w}; q.EventAtMs = 4000;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::CandidateForStagingReview, "Adjacent intervals accepted");
    changed.GuildAtWindow.CreatedAt++;
    q.Windows = {w, changed};
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::WrongGuildGeneration, "Guild changed at join");

    // 40-person mixed-guild raid, no bot/human inference and no rewards.
    q = input; q.Windows.clear();
    for (std::uint64_t i = 1; i <= 40; ++i)
    {
        auto slot = w;
        slot.CharacterGuid = i;
        if (i > 2) slot.GuildAtWindow = {27, 1800000001};
        q.Windows.push_back(slot);
    }
    q.CharacterGuid = 2;
    result = ReviewRaidMembershipAtEvent(q);
    test(result.Decision == RaidMembershipDecision::CandidateForStagingReview &&
         !result.MemberAtEventConfirmed && !result.HumanControlConfirmed &&
         !result.TrophyGranted, "40-member mixed guild candidate only");
    q.CharacterGuid = 3;
    test(ReviewRaidMembershipAtEvent(q).Decision ==
         RaidMembershipDecision::WrongGuildGeneration, "Mixed guild isolation");

    if (failures) return 1;
    std::cout << "PASS: " << checks << " event-time raid membership policy checks\n";
    return 0;
}
