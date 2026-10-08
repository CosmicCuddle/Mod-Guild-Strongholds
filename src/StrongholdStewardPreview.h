#ifndef NAXX_GUILD_STRONGHOLD_STEWARD_PREVIEW_H
#define NAXX_GUILD_STRONGHOLD_STEWARD_PREVIEW_H

#include <cstdint>

// Pure, fail-closed staging-only NPC interaction policy.
// There is NO character DB access, teleport, phase write, property purchase,
// material deduction, raid credit, quest assignment, or housing activation.
namespace NaxxGuildStrongholds
{
struct StewardPreviewContext
{
    bool ExplicitStagingSettingEnabled = false;
    bool StaffGameMaster = false;
    std::uint32_t CurrentGuildId = 0;
};

enum class StewardPreviewDecision : std::uint8_t
{
    Allowed,
    Disabled,
    NotStaff,
    NoGuild,
    InvalidMenuSender,
    InvalidAction
};

// Menu action values are only local, read-only information requests.
enum class StewardPreviewAction : std::uint32_t
{
    Overview = 1,
    FutureBuildings = 2,
    Close = 3
};

StewardPreviewDecision CheckStewardPreview(StewardPreviewContext const& input);

StewardPreviewDecision CheckStewardSelection(
    StewardPreviewContext const& input, bool IsMainSender,
    std::uint32_t action);

bool IsInformationalAction(std::uint32_t action);
}
#endif
