#ifndef NAXX_GUILD_STRONGHOLD_STEWARD_READ_ONLY_CONTENT_H
#define NAXX_GUILD_STRONGHOLD_STEWARD_READ_ONLY_CONTENT_H

#include "StrongholdStewardPreview.h"

#include <string>
#include <vector>

// Source-only preview of planned content, NOT server guild or IP state.
// These lines MUST NEVER assert anything is built, owned, unlocked or earned.
namespace NaxxGuildStrongholds
{
std::vector<std::string> BuildStewardPreviewRows(StewardPreviewAction action);
}

#endif
