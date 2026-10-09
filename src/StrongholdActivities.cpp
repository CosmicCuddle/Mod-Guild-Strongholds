#include "StrongholdActivities.h"

namespace NaxxGuildStrongholds
{
namespace
{
const std::array<Activity, 9> Activities{{
    {"daily_supply_run", "Supplies for the Settlement", Cadence::Daily, 1, IpMilestone::Start},
    {"daily_defend_roads", "Secure the Roads", Cadence::Daily, 2, IpMilestone::Start},
    {"daily_smithing", "A Smith's Request", Cadence::Daily, 2, IpMilestone::Start},
    {"weekly_workshop", "Build the Workshop", Cadence::Weekly, 2, IpMilestone::Start},
    {"weekly_vanilla_expedition", "A Guild Expedition", Cadence::Weekly, 3, IpMilestone::Start},
    {"weekly_outland_expedition", "Outland Guild Expedition", Cadence::Weekly, 3, IpMilestone::PreTbc},
    {"weekly_northrend_expedition", "Northrend Guild Expedition", Cadence::Weekly, 4, IpMilestone::PreWotlk},
    {"onyxia_trophy_request", "The Dragon's Legacy", Cadence::OneTime, 4, IpMilestone::OnyxiaComplete},
    {"icc_memorial_request", "Icecrown Memorial", Cadence::OneTime, 6, IpMilestone::WotlkTier3}
}};
}

const std::array<Activity, 9>& GetActivityCatalog()
{
    return Activities;
}

const Activity* FindActivity(std::string_view key)
{
    for (const Activity& activity : Activities)
        if (activity.Key == key)
            return &activity;
    return nullptr;
}

bool ValidateActivities()
{
    for (std::size_t i = 0; i < Activities.size(); ++i)
    {
        const Activity& activity = Activities[i];
        if (activity.Key.empty() || activity.Title.empty() ||
            activity.MinimumSettlementLevel < 1 || activity.MinimumSettlementLevel > 7 ||
            static_cast<std::uint8_t>(activity.MinimumIpMilestone) > 18)
            return false;
        for (std::size_t j = i + 1; j < Activities.size(); ++j)
            if (activity.Key == Activities[j].Key)
                return false;
    }
    return true;
}

ActivityGate CheckActivityEligibility(const Activity* activity, ActivityContext const& context)
{
    if (!context.Enabled)
        return ActivityGate::ModuleDisabled;
    if (!activity)
        return ActivityGate::UnknownActivity;
    if (context.OwnerGuildId == 0 || context.ActorGuildId == 0)
        return ActivityGate::GuildRequired;
    if (context.OwnerGuildId != context.ActorGuildId)
        return ActivityGate::OtherGuild;
    if (context.IsBot && !context.AllowBotContributions)
        return ActivityGate::BotDisabled;
    // Fail closed if the future IP adapter is absent or unverifiable.
    if (!context.HasVerifiedIpState)
        return ActivityGate::NoProgressionProof;
    if (context.SettlementLevel < activity->MinimumSettlementLevel ||
        context.SettlementLevel > 7)
        return ActivityGate::SettlementTooLow;
    if (context.VerifiedIpRank < static_cast<std::uint8_t>(activity->MinimumIpMilestone))
        return ActivityGate::ProgressionTooLow;
    return ActivityGate::Allowed;
}
}
