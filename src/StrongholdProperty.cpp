#include "StrongholdProperty.h"

namespace NaxxGuildStrongholds
{
PropertyClaimDecision CheckPropertyClaim(PropertyClaimContext const& context,
    std::string_view requestedTheme)
{
    if (!context.Enabled)
        return PropertyClaimDecision::ModuleDisabled;
    if (!context.IdentityVerified || !context.GuildCreatedAt)
        return PropertyClaimDecision::IdentityUnverified;
    if (!context.OwnerGuildId || !context.ActorGuildId)
        return PropertyClaimDecision::NoGuild;
    if (context.OwnerGuildId != context.ActorGuildId)
        return PropertyClaimDecision::OtherGuild;
    if (!context.IsGuildMaster)
        return PropertyClaimDecision::GuildMasterRequired;
    if (context.IsBot)
        return PropertyClaimDecision::BotNotAllowed;

    Theme const* theme = FindTheme(requestedTheme);
    if (!theme)
        return PropertyClaimDecision::UnknownTheme;
    if (theme->Team != context.GuildFaction)
        return PropertyClaimDecision::WrongFaction;
    if (!FindPrototypeLayout(requestedTheme))
        return PropertyClaimDecision::ThemeNotAvailable;

    if (context.HasExistingProperty)
        return context.ExistingTheme == requestedTheme ?
            PropertyClaimDecision::AlreadyOwnsProperty :
            PropertyClaimDecision::GuildPropertyConflict;

    return PropertyClaimDecision::Allowed;
}
}
