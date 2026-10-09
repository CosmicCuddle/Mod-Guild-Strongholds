#include "StrongholdActivities.h"

#include <iostream>
#include <string_view>

using namespace NaxxGuildStrongholds;

int main()
{
    int failures = 0;
    int total = 0;
    auto expect = [&](bool condition, std::string_view message)
    {
        ++total;
        if (!condition)
        {
            std::cerr << "FAILED: " << message << '\n';
            ++failures;
        }
    };

    ActivityContext context{true, false, false, true, 0, 1, 123, 123};

    expect(ValidateActivities(), "Catalogue contains valid activities");
    expect(GetActivityCatalog().size() == 9, "Nine prototype activities");
    expect(FindActivity("missing") == nullptr, "Unknown key denied");
    expect(CheckActivityEligibility(FindActivity("daily_supply_run"), context) == ActivityGate::Allowed,
        "Earliest Vanilla activity is accessible to guild members");
    expect(CheckActivityEligibility(FindActivity("weekly_outland_expedition"), context) == ActivityGate::SettlementTooLow,
        "Settlement level checked before IP milestone");
    context.SettlementLevel = 3;
    expect(CheckActivityEligibility(FindActivity("weekly_outland_expedition"), context) == ActivityGate::ProgressionTooLow,
        "Vanilla character cannot access Outland activity");
    context.VerifiedIpRank = 7;
    expect(CheckActivityEligibility(FindActivity("weekly_outland_expedition"), context) == ActivityGate::ProgressionTooLow,
        "Naxxramas-40 complete does not by itself unlock Outland");
    context.VerifiedIpRank = 8;
    expect(CheckActivityEligibility(FindActivity("weekly_outland_expedition"), context) == ActivityGate::Allowed,
        "Verified Outland milestone unlocks activity");
    expect(CheckActivityEligibility(FindActivity("weekly_northrend_expedition"), context) == ActivityGate::SettlementTooLow,
        "Northrend requires a developed settlement");
    context.SettlementLevel = 4;
    expect(CheckActivityEligibility(FindActivity("weekly_northrend_expedition"), context) == ActivityGate::ProgressionTooLow,
        "Outland character cannot access Northrend activity");
    context.VerifiedIpRank = 13;
    expect(CheckActivityEligibility(FindActivity("weekly_northrend_expedition"), context) == ActivityGate::Allowed,
        "Verified pre-Wrath milestone unlocks Northrend activity");

    context.VerifiedIpRank = 0;
    expect(CheckActivityEligibility(FindActivity("onyxia_trophy_request"), context) == ActivityGate::ProgressionTooLow,
        "Onyxia cosmetic request requires stage eligibility");
    context.VerifiedIpRank = 2;
    expect(CheckActivityEligibility(FindActivity("onyxia_trophy_request"), context) == ActivityGate::Allowed,
        "Onyxia stage eligibility allows requesting reward verification");

    context.ActorGuildId = 999;
    expect(CheckActivityEligibility(FindActivity("daily_supply_run"), context) == ActivityGate::OtherGuild,
        "Cross-guild request denied");
    context.ActorGuildId = 0;
    expect(CheckActivityEligibility(FindActivity("daily_supply_run"), context) == ActivityGate::GuildRequired,
        "Guildless request denied");
    context.ActorGuildId = context.OwnerGuildId;
    context.IsBot = true;
    expect(CheckActivityEligibility(FindActivity("daily_supply_run"), context) == ActivityGate::BotDisabled,
        "Bot contributions disabled by default");
    context.IsBot = false;
    context.HasVerifiedIpState = false;
    expect(CheckActivityEligibility(FindActivity("daily_supply_run"), context) == ActivityGate::NoProgressionProof,
        "Unknown IP state fails closed");
    context.HasVerifiedIpState = true;
    context.Enabled = false;
    expect(CheckActivityEligibility(FindActivity("daily_supply_run"), context) == ActivityGate::ModuleDisabled,
        "Disabled system denies all activities");
    context.Enabled = true;
    expect(CheckActivityEligibility(nullptr, context) == ActivityGate::UnknownActivity,
        "Nonexistent activity denied");
    context.SettlementLevel = 8;
    expect(CheckActivityEligibility(FindActivity("daily_supply_run"), context) == ActivityGate::SettlementTooLow,
        "Out-of-range settlement level fails closed");

    if (failures != 0)
    {
        std::cerr << failures << " out of " << total << " activity tests failed\n";
        return 1;
    }
    std::cout << "PASS: " << total << " activity eligibility assertions\n";
    return 0;
}
