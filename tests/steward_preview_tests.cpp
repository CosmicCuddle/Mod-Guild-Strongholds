#include "StrongholdStewardPreview.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    unsigned checks = 0, failures = 0;
    auto check = [&](bool ok, char const* message)
    {
        ++checks;
        if (!ok)
        {
            ++failures;
            std::cerr << "FAILED: " << message << '\n';
        }
    };

    StewardPreviewContext c;
    check(CheckStewardPreview(c) == StewardPreviewDecision::Disabled,
        "default no steward NPC access");
    c.ExplicitStagingSettingEnabled = true;
    check(CheckStewardPreview(c) == StewardPreviewDecision::NotStaff,
        "config alone does not grant access");
    c.StaffGameMaster = true;
    check(CheckStewardPreview(c) == StewardPreviewDecision::NoGuild,
        "no-guild staff cannot view guild property preview");
    c.CurrentGuildId = 17;
    check(CheckStewardPreview(c) == StewardPreviewDecision::UnverifiedGuild,
        "GM with numeric guild ID alone cannot access preview");
    c.VerifiedActiveMembershipAndGeneration = true;
    check(CheckStewardPreview(c) == StewardPreviewDecision::Allowed,
        "GM with registry and membership proof can inspect preview");

    for (std::uint32_t action = 1; action <= 11; ++action)
    {
        check(CheckStewardSelection(c, true, action) == StewardPreviewDecision::Allowed,
            "only defined read-only actions accepted");
        check(CheckStewardSelection(c, false, action) ==
            StewardPreviewDecision::InvalidMenuSender,
            "forged sender rejected for each action");
    }
    check(!IsInformationalAction(0) && !IsInformationalAction(9) &&
        !IsInformationalAction(10) && IsInformationalAction(8) &&
        IsInformationalAction(11),
        "navigation never misclassified as informational content");
    for (std::uint32_t forged : {0u, 12u, 100u, 0xffffffffu})
        check(CheckStewardSelection(c, true, forged) ==
            StewardPreviewDecision::InvalidAction, "unknown client action denied");

    c.StaffGameMaster = false;
    check(CheckStewardSelection(c, true, 1) == StewardPreviewDecision::NotStaff,
        "GM rights revoked between hello and selection");
    c.StaffGameMaster = true;
    c.CurrentGuildId = 0;
    check(CheckStewardSelection(c, true, 1) == StewardPreviewDecision::NoGuild,
        "guild left between hello and selection");
    c.CurrentGuildId = 17;
    c.VerifiedActiveMembershipAndGeneration = false;
    check(CheckStewardSelection(c, true, 1) ==
        StewardPreviewDecision::UnverifiedGuild,
        "Guild registry/member proof lost after menu opened");
    c.VerifiedActiveMembershipAndGeneration = true;
    c.ExplicitStagingSettingEnabled = false;
    check(CheckStewardSelection(c, true, 1) == StewardPreviewDecision::Disabled,
        "runtime config disabled between hello and selection");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks
              << " staging steward access and forged selection checks\n";
    return 0;
}
