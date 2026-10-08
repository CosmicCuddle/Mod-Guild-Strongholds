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
    return action >= static_cast<std::uint32_t>(StewardPreviewAction::Overview) &&
        (action <= static_cast<std::uint32_t>(StewardPreviewAction::Trophies) ||
         action == static_cast<std::uint32_t>(StewardPreviewAction::EvidenceReview));
}

StewardPreviewDecision CheckStewardSelection(
    StewardPreviewContext const& input, bool isMainSender,
    std::uint32_t action)
{
    StewardPreviewDecision const base = CheckStewardPreview(input);
    if (base != StewardPreviewDecision::Allowed)
        return base;
    if (!isMainSender)
        return StewardPreviewDecision::InvalidMenuSender;
    if (!IsInformationalAction(action) &&
        action != static_cast<std::uint32_t>(StewardPreviewAction::Back) &&
        action != static_cast<std::uint32_t>(StewardPreviewAction::Close))
        return StewardPreviewDecision::InvalidAction;
    return StewardPreviewDecision::Allowed;
}
}
