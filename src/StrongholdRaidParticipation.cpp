#include "StrongholdRaidParticipation.h"

#include <unordered_set>

namespace NaxxGuildStrongholds
{
RaidParticipationResult ReviewRaidParticipation(RaidParticipationInput const& input)
{
    RaidParticipationResult result;
    if (!input.GuildGenerationVerified || !input.ClaimingGuild.GuildId ||
        !input.ClaimingGuild.CreatedAt)
        return result;
    if (!input.ServerRaidInstanceVerified ||
        !input.ServerBossEncounterVerified || !input.MapId || !input.InstanceId)
    {
        result.Decision = RaidParticipationDecision::UnverifiedEncounter;
        return result;
    }
    if (!input.RosterSourceVerified)
    {
        result.Decision = RaidParticipationDecision::UnverifiedRoster;
        return result;
    }
    if (!input.PlayerbotsSourceVerified)
    {
        result.Decision = RaidParticipationDecision::UnverifiedPlayerbots;
        return result;
    }
    if (input.RequiredHumanGuildMembers < 2 ||
        input.RequiredHumanGuildMembers > 40)
    {
        result.Decision = RaidParticipationDecision::InvalidThreshold;
        return result;
    }
    if (input.Members.empty() || input.Members.size() > 40)
    {
        result.Decision = RaidParticipationDecision::InvalidRoster;
        return result;
    }

    std::unordered_set<std::uint64_t> distinct;
    for (RaidMemberEvidence const& member : input.Members)
    {
        // A duplicated or empty GUID invalidates the ENTIRE source record.
        if (!member.CharacterGuid || !distinct.insert(member.CharacterGuid).second)
        {
            result.Decision = RaidParticipationDecision::InvalidRoster;
            return result;
        }
    }

    for (RaidMemberEvidence const& member : input.Members)
    {
        if (!member.GuildGenerationAndMembershipVerified ||
            member.CurrentGuild.GuildId != input.ClaimingGuild.GuildId ||
            member.CurrentGuild.CreatedAt != input.ClaimingGuild.CreatedAt ||
            !member.RaidGroupMembershipVerified ||
            !member.SameInstanceAtEncounterVerified ||
            !member.EncounterParticipationVerified)
            continue;

        // Not finding a bot AI is NOT sufficient to classify a player as a
        // human. The explicit human check must be source-verified.
        if (!member.ControlClassificationSourceVerified ||
            member.Control == ParticipantControl::Unknown)
        {
            ++result.UnknownHumanStatus;
            continue;
        }
        if (member.Control == ParticipantControl::VerifiedPlayerbot)
        {
            ++result.ExcludedBots;
            continue;
        }
        if (member.Control == ParticipantControl::VerifiedHuman)
            ++result.QualifiedHumanGuildMembers;
    }

    result.Decision =
        result.QualifiedHumanGuildMembers >= input.RequiredHumanGuildMembers ?
        RaidParticipationDecision::ProposalOnly :
        RaidParticipationDecision::InsufficientHumanGuildMembers;
    return result;
}
}
