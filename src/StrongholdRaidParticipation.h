#ifndef NAXX_GUILD_STRONGHOLDS_RAID_PARTICIPATION_H
#define NAXX_GUILD_STRONGHOLDS_RAID_PARTICIPATION_H

#include "StrongholdLifecycle.h"

#include <cstdint>
#include <vector>

// Pure policy only. All observations require a future trusted server-side
// adapter. The live death and creature-kill-credit callbacks NEVER assert
// membership, actual encounter contribution or human status on their own.
namespace NaxxGuildStrongholds
{
enum class ParticipantControl : std::uint8_t
{
    Unknown,
    VerifiedHuman,
    VerifiedPlayerbot
};

struct RaidMemberEvidence
{
    std::uint64_t CharacterGuid = 0;
    GuildIdentity CurrentGuild;
    bool GuildGenerationAndMembershipVerified = false;
    bool RaidGroupMembershipVerified = false;
    bool SameInstanceAtEncounterVerified = false;
    // Must originate from an audited per-character encounter contribution
    // source, not only map presence, final blow or group reward.
    bool EncounterParticipationVerified = false;
    ParticipantControl Control = ParticipantControl::Unknown;
    bool ControlClassificationSourceVerified = false;
};

struct RaidParticipationInput
{
    GuildIdentity ClaimingGuild;
    bool GuildGenerationVerified = false;
    bool ServerRaidInstanceVerified = false;
    bool ServerBossEncounterVerified = false;
    bool RosterSourceVerified = false;
    bool PlayerbotsSourceVerified = false;
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    std::uint8_t RequiredHumanGuildMembers = 0; // explicit 2..40 only
    std::vector<RaidMemberEvidence> Members;
};

enum class RaidParticipationDecision : std::uint8_t
{
    UnverifiedGuild,
    UnverifiedEncounter,
    UnverifiedRoster,
    UnverifiedPlayerbots,
    InvalidThreshold,
    InvalidRoster,
    InsufficientHumanGuildMembers,
    ProposalOnly
};

struct RaidParticipationResult
{
    RaidParticipationDecision Decision = RaidParticipationDecision::UnverifiedGuild;
    std::uint8_t QualifiedHumanGuildMembers = 0;
    std::uint8_t ExcludedBots = 0;
    std::uint8_t UnknownHumanStatus = 0;
    // Never a persistent unlock, achievement or gameplay permission.
    bool TrophyGranted = false;
};

RaidParticipationResult ReviewRaidParticipation(RaidParticipationInput const& input);
}
#endif
