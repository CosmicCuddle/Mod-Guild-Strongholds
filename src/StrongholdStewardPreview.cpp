#include "StrongholdStewardPreview.h"

namespace NaxxGuildStrongholds
{
StewardPreviewDecision CheckStewardPreview(StewardPreviewContext const& input)
{
    if (!input.ExplicitStagingSettingEnabled)
        return StewardPreviewDecision::Disabled;
    if (!input.StaffGameMaster)
        return StewardPreviewDecision::NotStaff;
    if (!input.CurrentGuildId)
        return StewardPreviewDecision::NoGuild;
    return StewardPreviewDecision::Allowed;
}

bool IsInformationalAction(std::uint32_t action)
{
    return action == static_cast<std::uint32_t>(StewardPreviewAction::Overview) ||
           action == static_cast<std::uint32_t>(StewardPreviewAction::FutureBuildings);
}

StewardPreviewDecision CheckStewardSelection(
    StewardPreviewContext const& input, bool IsMainSender,
    std::uint32_t action)
{
    StewardPreviewDecision const base = CheckStewardPreview(input);
    if (base != StewardPreviewDecision::Allowed)
        return base;
    if (!IsMainSender)
        return StewardPreviewDecision::InvalidMenuSender;
    if (!IsInformationalAction(action) &&
        action != static_cast<std::uint32_t>(StewardPreviewAction::Close))
        return StewardPreviewDecision::InvalidAction;
    return StewardPreviewDecision::Allowed;
}
}
