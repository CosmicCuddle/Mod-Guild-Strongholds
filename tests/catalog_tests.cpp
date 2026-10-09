#include "StrongholdCatalog.h"

#include <iostream>
#include <string_view>

using namespace NaxxGuildStrongholds;

int main()
{
    int failures = 0;
    const auto expect = [&](bool condition, std::string_view message)
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << message << '\n';
            ++failures;
        }
    };

    expect(ValidateCatalog(), "Catalog structure and uniqueness");
    expect(GetThemes().size() == 10, "Ten racial themes exist");
    expect(FindTheme("human") != nullptr, "Human theme exists");
    expect(FindTheme("orc") != nullptr, "Orc theme exists");
    expect(FindTheme("unlisted") == nullptr, "Unknown themes are rejected");
    expect(CanChooseTheme(Faction::Alliance, "human"), "Alliance can select Human");
    expect(CanChooseTheme(Faction::Horde, "orc"), "Horde can select Orc");
    expect(CanChooseTheme(Faction::Alliance, "draenei"), "Alliance can select Draenei");
    expect(CanChooseTheme(Faction::Horde, "blood_elf"), "Horde can select Blood Elf");
    expect(!CanChooseTheme(Faction::Alliance, "orc"), "Alliance cannot select Orc");
    expect(!CanChooseTheme(Faction::Horde, "human"), "Horde cannot select Human");
    expect(!CanChooseTheme(Faction::Alliance, ""), "Empty theme denied");

    const Layout* human = FindPrototypeLayout("human");
    const Layout* orc = FindPrototypeLayout("orc");
    expect(human && human->Plots.size() == 6, "Six Human building plots");
    expect(orc && orc->Plots.size() == 6, "Six Orc building plots");
    expect(FindPrototypeLayout("gnome") == nullptr, "Unimplemented Gnome plot layout is unavailable");

    expect(CheckGuildEntry(false, true, 10, 10) == AccessDecision::Disabled,
        "Disabled module must deny access");
    expect(CheckGuildEntry(true, false, 10, 10) == AccessDecision::IsolationUnverified,
        "Unverified isolation must deny access");
    expect(CheckGuildEntry(true, true, 0, 10) == AccessDecision::NoGuild,
        "Guildless or invalid ownership must deny access");
    expect(CheckGuildEntry(true, true, 10, 0) == AccessDecision::NoGuild,
        "Guildless visitors must be denied");
    expect(CheckGuildEntry(true, true, 10, 11) == AccessDecision::OtherGuild,
        "Other guilds must be denied");
    expect(CheckGuildEntry(true, true, 10, 10) == AccessDecision::Granted,
        "Same guild admitted only after prerequisites");

    if (failures)
    {
        std::cerr << failures << " catalog test(s) failed\n";
        return 1;
    }
    std::cout << "PASS: all 21 settlement catalog and access-policy assertions\n";
    return 0;
}
