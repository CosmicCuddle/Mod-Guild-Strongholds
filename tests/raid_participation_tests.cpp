#include "StrongholdRaidParticipation.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0, failed = 0;
    auto check = [&](bool good, char const* name) {
        ++checks;
        if (!good) { ++failed; std::cerr << "FAILED: " << name << '\n'; }
    };

    RaidParticipationInput raid;
    raid.ClaimingGuild = {21, 1777000000};
    raid.GuildGenerationVerified = true;
    raid.ServerRaidInstanceVerified = true;
    raid.ServerBossEncounterVerified = true;
    raid.RosterSourceVerified = true;
    raid.PlayerbotsSourceVerified = true;
    raid.MapId = 249;
    raid.InstanceId = 4001;
    raid.RequiredHumanGuildMembers = 2;
    RaidMemberEvidence human1{1001, raid.ClaimingGuild, true, true, true, true,
                              ParticipantControl::VerifiedHuman, true};
    RaidMemberEvidence human2 = human1;
    human2.CharacterGuid = 1002;
    RaidMemberEvidence bot = human1;
    bot.CharacterGuid = 1003;
    bot.Control = ParticipantControl::VerifiedPlayerbot;
    RaidMemberEvidence unknown = human1;
    unknown.CharacterGuid = 1004;
    unknown.Control = ParticipantControl::Unknown;
    raid.Members = {human1, human2, bot, unknown};
    auto result = ReviewRaidParticipation(raid);
    check(result.Decision == RaidParticipationDecision::ProposalOnly &&
          result.QualifiedHumanGuildMembers == 2 && result.ExcludedBots == 1 &&
          result.UnknownHumanStatus == 1 && !result.TrophyGranted,
          "Two proven humans qualify only hypothetical planning, bot/unknown excluded");

    raid.PlayerbotsSourceVerified = false;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::UnverifiedPlayerbots,
          "Installed-fork bot classifier must be verified");
    raid.PlayerbotsSourceVerified = true;
    raid.RosterSourceVerified = false;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::UnverifiedRoster, "No trusted roster means no proposal");
    raid.RosterSourceVerified = true;
    raid.ServerBossEncounterVerified = false;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::UnverifiedEncounter,
          "Generic death/kill-credit callback is not named encounter proof");
    raid.ServerBossEncounterVerified = true;
    raid.GuildGenerationVerified = false;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::UnverifiedGuild,
          "Guild ID alone is insufficient");
    raid.GuildGenerationVerified = true;

    raid.Members[1].EncounterParticipationVerified = false;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InsufficientHumanGuildMembers,
          "Map/roster presence alone does not qualify");
    raid.Members[1].EncounterParticipationVerified = true;
    raid.Members[1].ControlClassificationSourceVerified = false;
    result = ReviewRaidParticipation(raid);
    check(result.Decision == RaidParticipationDecision::InsufficientHumanGuildMembers &&
          result.UnknownHumanStatus == 2,
          "Unverified human classifier must be treated as unknown");
    raid.Members[1].ControlClassificationSourceVerified = true;
    raid.Members[1].CurrentGuild.CreatedAt++;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InsufficientHumanGuildMembers,
          "Guild ID reuse does not credit former member");
    raid.Members[1].CurrentGuild = raid.ClaimingGuild;
    raid.Members[1].RaidGroupMembershipVerified = false;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InsufficientHumanGuildMembers,
          "Unverified group membership cannot count");
    raid.Members[1].RaidGroupMembershipVerified = true;
    raid.Members[1].SameInstanceAtEncounterVerified = false;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InsufficientHumanGuildMembers,
          "Different instance cannot count");
    raid.Members[1].SameInstanceAtEncounterVerified = true;
    raid.Members[1].GuildGenerationAndMembershipVerified = false;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InsufficientHumanGuildMembers,
          "Member lookup must be verified for that guild generation");
    raid.Members[1].GuildGenerationAndMembershipVerified = true;

    raid.Members[1].CharacterGuid = raid.Members[0].CharacterGuid;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InvalidRoster,
          "Duplicate GUID makes the entire roster untrustworthy");
    raid.Members[1].CharacterGuid = 1002;
    raid.RequiredHumanGuildMembers = 0;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InvalidThreshold, "Zero threshold is disallowed");
    raid.RequiredHumanGuildMembers = 41;
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InvalidThreshold, "Too-large threshold rejected");
    raid.RequiredHumanGuildMembers = 2;

    // 40-person mixed-guild raid where only two humans of THIS guild
    // contributed. Bots and other-guild players do not inflate count.
    raid.Members.clear();
    for (std::uint64_t i = 1; i <= 40; ++i)
    {
        auto m = human1;
        m.CharacterGuid = i;
        if (i == 3) m.Control = ParticipantControl::VerifiedPlayerbot;
        if (i > 3) m.CurrentGuild = {44, 1777000001};
        raid.Members.push_back(m);
    }
    result = ReviewRaidParticipation(raid);
    check(result.Decision == RaidParticipationDecision::ProposalOnly &&
          result.QualifiedHumanGuildMembers == 2 && !result.TrophyGranted,
          "40-raider mixed-guild scenario counts only two verified humans");
    raid.Members.push_back(human1);
    check(ReviewRaidParticipation(raid).Decision ==
          RaidParticipationDecision::InvalidRoster, "More than 40 members invalid");

    if (failed) return 1;
    std::cout << "PASS: " << checks
              << " secure raid participation and Playerbots-unknown checks\n";
    return 0;
}
