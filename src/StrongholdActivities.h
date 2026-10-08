#ifndef NAXX_GUILD_STRONGHOLD_ACTIVITIES_H
#define NAXX_GUILD_STRONGHOLD_ACTIVITIES_H

#include <array>
#include <cstdint>
#include <string_view>

// Domain-only catalogue. No DB, AzerothCore Player, IP or bot calls here.
// The future live integration MUST map stages from its installed IP module.
namespace NaxxGuildStrongholds
{
enum class Cadence : std::uint8_t { Daily, Weekly, OneTime };

enum class ActivityGate : std::uint8_t
{
    Allowed,
    ModuleDisabled,
    GuildRequired,
    OtherGuild,
    BotDisabled,
    NoProgressionProof,
    SettlementTooLow,
    ProgressionTooLow,
    UnknownActivity
};

// These named stages were reviewed against public Grimfeather IP commit
// 706740808fee328b8557607f87b0548cf961e047. The DEPLOYED server may
// differ. Future adapters MUST verify actual installed source before
// passing a rank to this independent catalogue; no direct quest writes.
enum class IpMilestone : std::uint8_t
{
    Start = 0,
    MoltenCoreComplete = 1,
    OnyxiaComplete = 2,
    BlackwingLairComplete = 3,
    Naxxramas40Complete = 7,
    PreTbc = 8,
    TbcTier1 = 9,
    PreWotlk = 13,
    WotlkTier1 = 14,
    WotlkTier3 = 16,
    WotlkTier4 = 17
};

struct Activity
{
    std::string_view Key;
    std::string_view Title;
    Cadence Reset;
    std::uint8_t MinimumSettlementLevel;
    IpMilestone MinimumIpMilestone;
};

struct ActivityContext
{
    bool Enabled;
    bool IsBot;
    bool AllowBotContributions;
    bool HasVerifiedIpState;
    std::uint8_t VerifiedIpRank;  // monotonic rank mapped by version-matched adapter
    std::uint8_t SettlementLevel;
    std::uint32_t OwnerGuildId;
    std::uint32_t ActorGuildId;
};

const std::array<Activity, 9>& GetActivityCatalog();
const Activity* FindActivity(std::string_view key);
bool ValidateActivities();
ActivityGate CheckActivityEligibility(const Activity* activity, ActivityContext const& context);
}

#endif
