#ifndef NAXX_GUILD_STRONGHOLD_STAGING_PROPERTY_ADAPTER_H
#define NAXX_GUILD_STRONGHOLD_STAGING_PROPERTY_ADAPTER_H

#include "StrongholdPropertyRead.h"

// Opt-in to read from draft module-owned SQL on a SEPARATE staging realm.
// Normal builds contain no database query or reference to character DB.
#if defined(NAXX_GS_BUILD_STAGING_PROPERTY_READ) && \
    !defined(NAXX_GS_BUILD_STAGING_STEWARD)
#error "Staging property reads require the independently gated staging steward"
#endif

#if defined(NAXX_GS_BUILD_STAGING_PROPERTY_READ) && \
    defined(NAXX_GS_BUILD_STAGING_STEWARD)
namespace NaxxGuildStrongholds
{
PropertyReadResult ReadStagingProperty(GuildIdentity const& verifiedGuild);
}
#endif
#endif
