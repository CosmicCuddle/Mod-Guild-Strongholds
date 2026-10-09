#include "StrongholdPlayerbotsRead.h"

namespace NaxxGuildStrongholds
{
ParticipantControl ClassifyPlayerbotsRead(PlayerbotsReadEvidence const& evidence)
{
    if (!evidence.ExplicitStagingEnabled ||
        !evidence.ReviewedSourceAttested ||
        !evidence.ModuleEnabled ||
        !evidence.CharacterInWorld ||
        !evidence.BotAIRegistryQueried ||
        !evidence.BotAIRegistryMatched)
        return ParticipantControl::Unknown;

    // Critical: this API NEVER returns VerifiedHuman. Even a non-headless
    // session or a null PlayerbotAI pointer cannot prove human control.
    return ParticipantControl::VerifiedPlayerbot;
}
}
