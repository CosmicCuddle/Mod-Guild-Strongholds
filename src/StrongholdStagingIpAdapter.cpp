/*
 * EXTRA staging opt-in: direct read of reviewed Grimfeather IP module.
 * Does not change player quests, spells, ranks, phase, or IP configuration.
 * Requires matching installed IP header in CMake include path and linked
 * source module; refuses a missing header/build contract instead of fallback.
 */
#if defined(NAXX_GS_BUILD_STAGING_IP_READ) && \
    defined(NAXX_GS_BUILD_STAGING_STEWARD)

#include "StrongholdStagingIpAdapter.h"

#include "Config.h"
#include "IndividualProgression.h"
#include "Player.h"

namespace NaxxGuildStrongholds
{
IpRuntimeSnapshot ReadStagingIpState(Player* player, bool guildVerified)
{
    IpRuntimeSnapshot snapshot;
    if (!sConfigMgr->GetOption<bool>(
            "NaxxGuildStrongholds.StagingIpRead.Enabled", false) ||
        !player || !guildVerified || !player->IsGameMaster() ||
        !player->IsInWorld())
        return snapshot;

    // Directly consult the independently built/linked installed fork,
    // not client-reported progress, hidden quest ID guesses, or config
    // copies that might drift from IP's actual runtime limit.
    IndividualProgression* ip = sIndividualProgression;
    if (!ip)
        return snapshot;
    snapshot.SourceContractVerified = true;
    snapshot.ModuleEnabled = ip->enabled;
    snapshot.CharacterInWorld = player->IsInWorld();
    snapshot.RawQuestStage = ip->GetPlayerProgressionFromQuests(player);
    snapshot.ProgressionLimit = ip->progressionLimit;
    return snapshot;
}
}

#endif
