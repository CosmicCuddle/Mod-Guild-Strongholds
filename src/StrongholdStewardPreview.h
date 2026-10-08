#ifndef NAXX_GUILD_STRONGHOLD_STEWARD_PREVIEW_H
#define NAXX_GUILD_STRONGHOLD_STEWARD_PREVIEW_H

#include <cstdint>

// Pure, fail-closed staging-only NPC interaction policy.
// No DB access, teleport, phase write, property purchase, material
// deduction, raid credit, quest assignment, or housing activation.
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

enum class StewardPreviewAction : std::uint32_t
{
    Overview = 1,
    AllianceThemes = 2,
    HordeThemes = 3,
    HumanBuildings = 4,
    OrcBuildings = 5,
    DailyActivities = 6,
    WeeklyActivities = 7,
    Trophies = 8,
    Back = 9,
    Close = 10,
    EvidenceReview = 11
};

StewardPreviewDecision CheckStewardPreview(StewardPreviewContext const& input);
StewardPreviewDecision CheckStewardSelection(StewardPreviewContext const& input,
    bool isMainSender, std::uint32_t action);
bool IsInformationalAction(std::uint32_t action);
}
#endif
