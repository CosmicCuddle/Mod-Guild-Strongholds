#include "StrongholdStewardReadOnlyContent.h"

#include "StrongholdActivities.h"
#include "StrongholdCatalog.h"
#include "StrongholdConstruction.h"
#include "StrongholdTrophies.h"

#include <string>
#include <string_view>
#include <vector>

namespace NaxxGuildStrongholds
{
namespace
{
std::string BuildActivityRow(Activity const& item)
{
    return "[PLAN] " + std::string(item.Title) +
        " | Guild level " + std::to_string(item.MinimumSettlementLevel) +
        " | IP milestone " +
        std::to_string(static_cast<unsigned>(item.MinimumIpMilestone));
}

std::string BuildProjectRow(BuildingProject const& item)
{
    return "[PLAN] " + std::string(item.Title) +
        " | Level " + std::to_string(item.MinimumSettlementLevel) +
        " | S:" + std::to_string(item.Cost.Supplies) +
        " T:" + std::to_string(item.Cost.Timber) +
        " I:" + std::to_string(item.Cost.Iron);
}

std::vector<std::string> PlannedThemes(Faction side)
{
    std::vector<std::string> result;
    for (Theme const& theme : GetThemes())
        if (theme.Team == side)
            result.push_back("[PLAN] " + std::string(theme.DisplayName) +
                " - " + std::string(theme.Architecture));
    return result;
}

std::vector<std::string> PlannedBuildings(std::string_view theme)
{
    std::vector<std::string> result;
    for (BuildingProject const& item : GetBuildingProjects())
        if (item.ThemeKey == theme)
            result.push_back(BuildProjectRow(item));
    return result;
}

std::vector<std::string> PlannedTrophies()
{
    std::vector<std::string> result;
    for (Trophy const& item : GetTrophyCatalog())
        result.push_back("[PLAN] " + std::string(item.DisplayName) +
            " | " + std::string(item.DecorationConcept));
    return result;
}

std::vector<std::string> PlannedActivities(Cadence cadence)
{
    std::vector<std::string> result;
    for (Activity const& item : GetActivityCatalog())
        if (item.Reset == cadence)
            result.push_back(BuildActivityRow(item));
    return result;
}
}

std::vector<std::string> BuildStewardPreviewRows(StewardPreviewAction action)
{
    switch (action)
    {
        case StewardPreviewAction::Overview:
            return {
                "[PLAN] A shared guild settlement with personal IP-gated services.",
                "[LOCKED] Private map access has NOT been implemented.",
                "[LOCKED] No guild ownership, contribution, rewards or trophies are active.",
                "This NPC is a GM-only staging preview. Nothing shown is unlocked."
            };
        case StewardPreviewAction::AllianceThemes:
            return PlannedThemes(Faction::Alliance);
        case StewardPreviewAction::HordeThemes:
            return PlannedThemes(Faction::Horde);
        case StewardPreviewAction::HumanBuildings:
            return PlannedBuildings("human");
        case StewardPreviewAction::OrcBuildings:
            return PlannedBuildings("orc");
        case StewardPreviewAction::DailyActivities:
            return PlannedActivities(Cadence::Daily);
        case StewardPreviewAction::WeeklyActivities:
            return PlannedActivities(Cadence::Weekly);
        case StewardPreviewAction::Trophies:
            return PlannedTrophies();
        default:
            return {};
    }
}
}
