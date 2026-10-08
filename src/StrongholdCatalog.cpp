#include "StrongholdCatalog.h"

namespace NaxxGuildStrongholds
{
namespace
{
const std::array<Theme, 10> Themes{{
    {"human", "Royal Stronghold", Faction::Alliance, "Human stone keep and courtyard"},
    {"dwarf", "Mountain Hold", Faction::Alliance, "Dwarven stone halls and forges"},
    {"night_elf", "Moonwood Sanctuary", Faction::Alliance, "Night Elf forest sanctuary"},
    {"gnome", "Mechanist Enclave", Faction::Alliance, "Gnomish engineering enclave"},
    {"draenei", "Crystal Refuge", Faction::Alliance, "Draenei crystal refuge"},
    {"orc", "Warlord's Fortress", Faction::Horde, "Orcish palisade and war hall"},
    {"troll", "Darkspear Village", Faction::Horde, "Troll jungle village"},
    {"tauren", "Ancestral Encampment", Faction::Horde, "Tauren plains encampment"},
    {"undead", "Forsaken Bastion", Faction::Horde, "Forsaken gothic bastion"},
    {"blood_elf", "Sunspire Estate", Faction::Horde, "Blood Elf arcane estate"}
}};

const std::array<Layout, 2> PrototypeLayouts{{
    {"human", {{{"hall", PlotKind::Hall, 1, "Stone guild hall"},
                 {"military", PlotKind::Military, 2, "Training yard"},
                 {"crafting", PlotKind::Crafting, 2, "Workshop court"},
                 {"social", PlotKind::Social, 3, "Tavern garden"},
                 {"prestige", PlotKind::Prestige, 4, "Hall of legends"},
                 {"utility", PlotKind::Utility, 5, "Stables and services"}}}},
    {"orc", {{{"hall", PlotKind::Hall, 1, "Orcish war hall"},
               {"military", PlotKind::Military, 2, "Sparring arena"},
               {"crafting", PlotKind::Crafting, 2, "Forge camp"},
               {"social", PlotKind::Social, 3, "Feasting circle"},
               {"prestige", PlotKind::Prestige, 4, "Victory totems"},
               {"utility", PlotKind::Utility, 5, "Supply grounds"}}}}
}};
}

const std::array<Theme, 10>& GetThemes()
{
    return Themes;
}

const Theme* FindTheme(std::string_view key)
{
    for (const Theme& theme : Themes)
        if (theme.Key == key)
            return &theme;
    return nullptr;
}

bool CanChooseTheme(Faction guildFaction, std::string_view themeKey)
{
    const Theme* theme = FindTheme(themeKey);
    return theme && theme->Team == guildFaction;
}

const Layout* FindPrototypeLayout(std::string_view themeKey)
{
    for (const Layout& layout : PrototypeLayouts)
        if (layout.ThemeKey == themeKey)
            return &layout;
    return nullptr;
}

bool ValidateCatalog()
{
    unsigned alliance = 0;
    unsigned horde = 0;
    for (std::size_t i = 0; i < Themes.size(); ++i)
    {
        const Theme& item = Themes[i];
        if (item.Key.empty() || item.DisplayName.empty() || item.Architecture.empty())
            return false;
        if (item.Team == Faction::Alliance)
            ++alliance;
        else if (item.Team == Faction::Horde)
            ++horde;
        else
            return false;
        for (std::size_t j = i + 1; j < Themes.size(); ++j)
            if (item.Key == Themes[j].Key)
                return false;
    }
    if (alliance != 5 || horde != 5)
        return false;

    for (std::size_t i = 0; i < PrototypeLayouts.size(); ++i)
    {
        const Layout& layout = PrototypeLayouts[i];
        if (!FindTheme(layout.ThemeKey))
            return false;
        for (std::size_t j = i + 1; j < PrototypeLayouts.size(); ++j)
            if (layout.ThemeKey == PrototypeLayouts[j].ThemeKey)
                return false;
        for (std::size_t j = 0; j < layout.Plots.size(); ++j)
        {
            const Plot& plot = layout.Plots[j];
            if (plot.Key.empty() || plot.Description.empty() || plot.RequiredSettlementLevel < 1 ||
                plot.RequiredSettlementLevel > 7)
                return false;
            for (std::size_t k = j + 1; k < layout.Plots.size(); ++k)
                if (plot.Key == layout.Plots[k].Key)
                    return false;
        }
    }
    return true;
}

AccessDecision CheckGuildEntry(bool enabled, bool isolationVerified,
    std::uint32_t ownerGuildId, std::uint32_t visitorGuildId)
{
    if (!enabled)
        return AccessDecision::Disabled;
    if (!isolationVerified)
        return AccessDecision::IsolationUnverified;
    if (!ownerGuildId || !visitorGuildId)
        return AccessDecision::NoGuild;
    if (ownerGuildId != visitorGuildId)
        return AccessDecision::OtherGuild;
    return AccessDecision::Granted;
}
}
