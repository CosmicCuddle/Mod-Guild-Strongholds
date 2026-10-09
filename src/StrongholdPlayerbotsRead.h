#ifndef NAXX_GUILD_STRONGHOLDS_PLAYERBOTS_READ_H
#define NAXX_GUILD_STRONGHOLDS_PLAYERBOTS_READ_H

#include "StrongholdRaidParticipation.h"

namespace NaxxGuildStrongholds
{
// One-way proof only: the reviewed public Playerbots registry can positively
// identify bot AI, but nullptr cannot establish a real human. A missing,
// disabled, unreviewed or uncertain source ALWAYS remains Unknown.
struct PlayerbotsReadEvidence
{
    bool ExplicitStagingEnabled = false;
    bool ReviewedSourceAttested = false;
    bool ModuleEnabled = false;
    bool CharacterInWorld = false;
    bool BotAIRegistryQueried = false;
    bool BotAIRegistryMatched = false;
};

ParticipantControl ClassifyPlayerbotsRead(PlayerbotsReadEvidence const& evidence);
}
#endif
