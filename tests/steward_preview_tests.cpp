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
    check(CheckStewardPreview(c) == StewardPreviewDecision::Allowed,
          "staff member of a guild may inspect staging preview");

    check(CheckStewardSelection(c, true, 1) == StewardPreviewDecision::Allowed,
          "overview is informational");
    check(CheckStewardSelection(c, true, 2) == StewardPreviewDecision::Allowed,
          "future buildings are informational");
    check(CheckStewardSelection(c, true, 3) == StewardPreviewDecision::Allowed,
          "close is allowed");
    check(CheckStewardSelection(c, true, 0) == StewardPreviewDecision::InvalidAction,
          "zero action is not accepted");
    check(CheckStewardSelection(c, true, 4) == StewardPreviewDecision::InvalidAction,
          "unknown client action is not accepted");
    check(CheckStewardSelection(c, false, 1) ==
          StewardPreviewDecision::InvalidMenuSender,
          "forged sender rejected");
    check(CheckStewardSelection(c, false, 3) ==
          StewardPreviewDecision::InvalidMenuSender,
          "forged close sender rejected");
    check(IsInformationalAction(1) && IsInformationalAction(2) &&
          !IsInformationalAction(3) && !IsInformationalAction(999),
          "only exactly two information actions available");

    c.StaffGameMaster = false;
    check(CheckStewardSelection(c, true, 1) == StewardPreviewDecision::NotStaff,
          "GM rights revoked between hello and selection");
    c.StaffGameMaster = true;
    c.CurrentGuildId = 0;
    check(CheckStewardSelection(c, true, 1) == StewardPreviewDecision::NoGuild,
          "guild left between hello and selection");
    c.CurrentGuildId = 17;
    c.ExplicitStagingSettingEnabled = false;
    check(CheckStewardSelection(c, true, 1) == StewardPreviewDecision::Disabled,
          "runtime config disabled between hello and selection");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks
              << " staging steward access and forged selection checks\n";
    return 0;
}
