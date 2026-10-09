/*
 * Guild generation/member validation from real public AzerothCore APIs.
 * NOT a guild housing property lookup or playable feature.
 */
#if defined(NAXX_GS_BUILD_STAGING_STEWARD)

#include "StrongholdStagingGuildAdapter.h"

#include "Guild.h"
#include "Player.h"

#include <cstdint>

namespace NaxxGuildStrongholds
{
GuildReadResult ReadCurrentGuildIdentity(Player const* player)
{
    GuildReadObservation observed;
    if (!player)
        return VerifyGuildIdentity(observed);

    observed.PlayerGuildId = player->GetGuildId();
    if (!observed.PlayerGuildId)
        return VerifyGuildIdentity(observed);

    Guild const* guild = player->GetGuild();
    if (!guild)
        return VerifyGuildIdentity(observed);

    observed.RegistryGuildFound = true;
    observed.RegistryGuildId = guild->GetId();
    observed.MembershipFound = guild->GetMember(player->GetGUID()) != nullptr;

    auto const createdAt = guild->GetCreatedDate();
    if (createdAt > 0)
        observed.RegistryCreatedAt = static_cast<std::uint64_t>(createdAt);

    return VerifyGuildIdentity(observed);
}
}

#endif
