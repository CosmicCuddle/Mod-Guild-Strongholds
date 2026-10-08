#include "StrongholdProperty.h"

#include <iostream>
#include <string_view>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0;
    int failures = 0;
    auto check = [&](bool passes, std::string_view description)
    {
        ++checks;
        if (!passes)
        {
            ++failures;
            std::cerr << "FAILED: " << description << '\n';
        }
    };

    PropertyClaimContext ctx{true, true, 100, 100, true, false, Faction::Alliance, false, ""};

    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::Allowed,
        "Alliance guild master may claim Human property");
    check(CheckPropertyClaim(ctx, "orc") == PropertyClaimDecision::WrongFaction,
        "Alliance guild cannot claim Orc theme");
    check(CheckPropertyClaim(ctx, "dwarf") == PropertyClaimDecision::ThemeNotAvailable,
        "Unimplemented Dwarf layout cannot be claimed yet");
    check(CheckPropertyClaim(ctx, "not_real") == PropertyClaimDecision::UnknownTheme,
        "Unknown property style is rejected");
    ctx.HasExistingProperty = true;
    ctx.ExistingTheme = "human";
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::AlreadyOwnsProperty,
        "Same property can only be claimed once");
    ctx.ExistingTheme = "dwarf";
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::GuildPropertyConflict,
        "Other existing property blocks second purchase");
    ctx.HasExistingProperty = false;
    ctx.IsGuildMaster = false;
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::GuildMasterRequired,
        "Nonleaders cannot claim a guild property");
    ctx.IsGuildMaster = true;
    ctx.IsBot = true;
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::BotNotAllowed,
        "Bot accounts cannot purchase properties");
    ctx.IsBot = false;
    ctx.OwnerGuildId = 0;
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::NoGuild,
        "Guildless requester rejected");
    ctx.OwnerGuildId = 200;
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::OtherGuild,
        "Another guild's property cannot be claimed");
    ctx.OwnerGuildId = 100;
    ctx.IdentityVerified = false;
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::IdentityUnverified,
        "Unverified identity rejected");
    ctx.IdentityVerified = true;
    ctx.Enabled = false;
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::ModuleDisabled,
        "Disabled module rejects all property claims");
    ctx.Enabled = true;
    ctx.GuildFaction = Faction::Horde;
    check(CheckPropertyClaim(ctx, "orc") == PropertyClaimDecision::Allowed,
        "Horde guild master may claim Orc property");
    check(CheckPropertyClaim(ctx, "human") == PropertyClaimDecision::WrongFaction,
        "Horde guild cannot claim Human property");
    check(CheckPropertyClaim(ctx, "troll") == PropertyClaimDecision::ThemeNotAvailable,
        "Other Horde themes await tested layouts");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks << " guild-property claim policy checks\n";
    return 0;
}
