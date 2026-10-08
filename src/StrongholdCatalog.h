#ifndef NAXX_GUILD_STRONGHOLD_CATALOG_H
#define NAXX_GUILD_STRONGHOLD_CATALOG_H

#include <array>
#include <cstdint>
#include <string_view>

// Pure game-design data. This header does not read or modify AzerothCore state.
namespace NaxxGuildStrongholds
{
enum class Faction : std::uint8_t { Alliance, Horde };
enum class PlotKind : std::uint8_t { Hall, Military, Crafting, Social, Prestige, Utility };

enum class AccessDecision : std::uint8_t
{
    Disabled,
    IsolationUnverified,
    NoGuild,
    OtherGuild,
    Granted
};

struct Theme
{
    std::string_view Key;
    std::string_view DisplayName;
    Faction Team;
    std::string_view Architecture;
};

struct Plot
{
    std::string_view Key;
    PlotKind Kind;
    std::uint8_t RequiredSettlementLevel;
    std::string_view Description;
};

struct Layout
{
    std::string_view ThemeKey;
    std::array<Plot, 6> Plots;
};

const std::array<Theme, 10>& GetThemes();
const Theme* FindTheme(std::string_view key);
bool CanChooseTheme(Faction guildFaction, std::string_view themeKey);

// Only Human and Orc have initial plot prototypes. Returns nullptr otherwise.
const Layout* FindPrototypeLayout(std::string_view themeKey);
bool ValidateCatalog();

// Pure access-policy helper. Callers must supply SERVER-VERIFIED guild IDs
// and an isolation status established by runtime integration/tests.
// This is NOT a complete implementation of guild isolation or teleportation.
AccessDecision CheckGuildEntry(bool enabled, bool isolationVerified,
    std::uint32_t ownerGuildId, std::uint32_t visitorGuildId);
}

#endif
