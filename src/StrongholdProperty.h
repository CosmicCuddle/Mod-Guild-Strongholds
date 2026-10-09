#ifndef NAXX_GUILD_STRONGHOLD_PROPERTY_H
#define NAXX_GUILD_STRONGHOLD_PROPERTY_H

#include "StrongholdCatalog.h"

#include <cstdint>
#include <string_view>

// Pure property-claim rules. No map selection, teleport or database writes.
// Real caller must obtain the actor's guild, faction and leadership from the
// SERVER (not player-supplied gossip arguments). A DB unique key must enforce
// one property per guild during simultaneous requests.
namespace NaxxGuildStrongholds
{
enum class PropertyClaimDecision : std::uint8_t
{
    Allowed,
    ModuleDisabled,
    IdentityUnverified,
    NoGuild,
    OtherGuild,
    GuildMasterRequired,
    BotNotAllowed,
    UnknownTheme,
    WrongFaction,
    ThemeNotAvailable,
    AlreadyOwnsProperty,
    GuildPropertyConflict
};

struct PropertyClaimContext
{
    bool Enabled = false;
    bool IdentityVerified = false;
    std::uint32_t OwnerGuildId = 0;
    std::uint32_t ActorGuildId = 0;
    bool IsGuildMaster = false;
    bool IsBot = false;
    Faction GuildFaction = Faction::Alliance;
    bool HasExistingProperty = false;
    std::string_view ExistingTheme;
    std::uint64_t GuildCreatedAt = 0; // server-verified guild creation time
};

PropertyClaimDecision CheckPropertyClaim(PropertyClaimContext const& context,
    std::string_view requestedTheme);
}
#endif
