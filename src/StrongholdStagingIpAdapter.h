#ifndef NAXX_GUILD_STRONGHOLD_STAGING_IP_ADAPTER_H
#define NAXX_GUILD_STRONGHOLD_STAGING_IP_ADAPTER_H

#include "StrongholdIpCompatibility.h"

// This adapter deliberately DEPENDS on a separately installed reviewed
// Grimfeather IP source/header. If missing or changed, fail the opt-in build
// rather than silently pretending to know character progression.
#if defined(NAXX_GS_BUILD_STAGING_IP_READ) && \
    !defined(NAXX_GS_BUILD_STAGING_STEWARD)
#error "IP reads require staging Steward to be explicitly compiled"
#endif

#if defined(NAXX_GS_BUILD_STAGING_IP_READ) && \
    defined(NAXX_GS_BUILD_STAGING_STEWARD)
class Player;
namespace NaxxGuildStrongholds
{
IpRuntimeSnapshot ReadStagingIpState(Player* player, bool guildVerified);
}
#endif
#endif
