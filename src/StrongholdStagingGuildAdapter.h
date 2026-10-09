#ifndef NAXX_GUILD_STRONGHOLD_STAGING_GUILD_ADAPTER_H
#define NAXX_GUILD_STRONGHOLD_STAGING_GUILD_ADAPTER_H

#include "StrongholdGuildReadOnly.h"

#if defined(NAXX_GS_BUILD_STAGING_STEWARD)
class Player;

namespace NaxxGuildStrongholds
{
// Read-only AzerothCore adapter. Uses only Player::GetGuild(),
// Guild::GetMember(GUID), GetId and GetCreatedDate. No mutators or SQL.
// No code is available in normal module builds.
GuildReadResult ReadCurrentGuildIdentity(Player const* player);
}
#endif

#endif
