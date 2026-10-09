#ifndef NAXX_GUILD_STRONGHOLDS_STAGING_PLAYERBOTS_READ_H
#define NAXX_GUILD_STRONGHOLDS_STAGING_PLAYERBOTS_READ_H

#include "StrongholdPlayerbotsRead.h"

#if defined(NAXX_GS_BUILD_STAGING_PLAYERBOTS_READ) && \
    !defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)
#error "Optional Playerbots read requires staging raid observer"
#endif

#if defined(NAXX_GS_BUILD_STAGING_PLAYERBOTS_READ) && \
    defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)
class Player;
namespace NaxxGuildStrongholds
{
// The only guaranteed positive classification is VerifiedPlayerbot.
// It does not establish who truly participated in an encounter.
ParticipantControl ReadStagingPlayerbotsControl(Player* player);
}
#endif
#endif
