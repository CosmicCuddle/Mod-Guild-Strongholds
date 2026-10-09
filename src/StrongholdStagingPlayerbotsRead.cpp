/*
 * Extra opt-in Playerbots registry READ, only used by read-only staging logs.
 *
 * Requires pinned public Playerbots headers/compiled fork compatibility.
 * PlayerbotsMgr::GetPlayerbotAI(Player*) returns nullptr when module is off,
 * player is absent, OR not currently in bot registry; never label human!
 */
#if defined(NAXX_GS_BUILD_STAGING_PLAYERBOTS_READ) && \
    defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)

#include "StrongholdStagingPlayerbotsRead.h"

#include "Config.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotMgr.h"

namespace NaxxGuildStrongholds
{
ParticipantControl ReadStagingPlayerbotsControl(Player* player)
{
    PlayerbotsReadEvidence evidence;
    evidence.ExplicitStagingEnabled = sConfigMgr->GetOption<bool>(
        "NaxxGuildStrongholds.StagingPlayerbotsRead.Enabled", false);
    // An explicit staging administrator approval flag is NOT a source
    // fingerprint and never proves human status or encounter participation.
    evidence.ReviewedSourceAttested = sConfigMgr->GetOption<bool>(
        "NaxxGuildStrongholds.StagingPlayerbotsRead.ReviewedSource", false);

    if (!evidence.ExplicitStagingEnabled ||
        !evidence.ReviewedSourceAttested ||
        !player || !player->IsInWorld())
        return ParticipantControl::Unknown;

    evidence.CharacterInWorld = true;
    evidence.ModuleEnabled = sPlayerbotAIConfig.enabled;
    if (!evidence.ModuleEnabled)
        return ParticipantControl::Unknown;

    evidence.BotAIRegistryQueried = true;
    evidence.BotAIRegistryMatched = sPlayerbotsMgr.GetPlayerbotAI(player) != nullptr;
    return ClassifyPlayerbotsRead(evidence);
}
}
#endif
