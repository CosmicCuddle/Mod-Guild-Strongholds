#include "StrongholdStewardReadOnlyContent.h"
#include "StrongholdActivities.h"
#include "StrongholdCatalog.h"
#include "StrongholdConstruction.h"

#include <iostream>
#include <string>
#include <vector>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0;
    int failed = 0;
    auto test = [&](bool okay, char const* label)
    {
        ++checks;
        if (!okay)
        {
            ++failed;
            std::cerr << "FAILED: " << label << '\n';
        }
    };
    test(ValidateCatalog() && ValidateBuildingProjects() && ValidateActivities(),
        "Referenced source catalogues are valid");

    const auto overview = BuildStewardPreviewRows(StewardPreviewAction::Overview);
    test(overview.size() == 4, "Overview provides four explicitly locked statements");
    test(overview[1].find("NOT been implemented") != std::string::npos,
        "Preview does not imply real map isolation");

    const auto alliance = BuildStewardPreviewRows(StewardPreviewAction::AllianceThemes);
    const auto horde = BuildStewardPreviewRows(StewardPreviewAction::HordeThemes);
    test(alliance.size() == 5 && horde.size() == 5, "Five themes per faction");
    test(alliance[0].find("Royal Stronghold") != std::string::npos,
        "Human reference style from catalogue");
    test(horde[0].find("Warlord") != std::string::npos,
        "Orc reference style from catalogue");

    const auto human = BuildStewardPreviewRows(StewardPreviewAction::HumanBuildings);
    const auto orc = BuildStewardPreviewRows(StewardPreviewAction::OrcBuildings);
    test(human.size() == 6 && orc.size() == 6,
        "Six planned buildings for each prototype theme");
    test(human[0].find("Royal Guild Hall") != std::string::npos &&
        human[0].find("S:100 T:40 I:20") != std::string::npos,
        "Building list uses real catalogue and resource costs");
    test(orc[5].find("Supply Grounds") != std::string::npos,
        "Last Orc building derives from catalogue");

    const auto daily = BuildStewardPreviewRows(StewardPreviewAction::DailyActivities);
    const auto weekly = BuildStewardPreviewRows(StewardPreviewAction::WeeklyActivities);
    const auto trophies = BuildStewardPreviewRows(StewardPreviewAction::Trophies);
    test(daily.size() == 3 && weekly.size() == 4 && trophies.size() == 5,
        "Dailies/weeklies and five independent trophy concepts from catalogues");
    test(weekly[2].find("Outland") != std::string::npos &&
        weekly[2].find("IP milestone 8") != std::string::npos,
        "Outland activity advertises required IP milestone, not unlocked status");
    test(weekly[3].find("IP milestone 13") != std::string::npos,
        "Northrend reference milestone remains capped");
    test(trophies[0].find("Onyxia") != std::string::npos &&
        trophies[4].find("Kel") != std::string::npos &&
        trophies[0].find("[PLAN]") == 0,
        "Five raid trophies derive from real symbolic concepts, not rewards");
    test(BuildStewardPreviewRows(StewardPreviewAction::Back).empty() &&
        BuildStewardPreviewRows(StewardPreviewAction::Close).empty() &&
        BuildStewardPreviewRows(StewardPreviewAction::EvidenceReview).empty(),
        "Navigation actions do not generate a plan page");

    for (StewardPreviewAction action : {
        StewardPreviewAction::Overview, StewardPreviewAction::AllianceThemes,
        StewardPreviewAction::HordeThemes, StewardPreviewAction::HumanBuildings,
        StewardPreviewAction::OrcBuildings, StewardPreviewAction::DailyActivities,
        StewardPreviewAction::WeeklyActivities, StewardPreviewAction::Trophies})
    {
        auto rows = BuildStewardPreviewRows(action);
        test(rows.size() <= 7, "Page fits WotLK gossip page with navigation");
        for (auto const& row : rows)
        {
            test(!row.empty() && row.size() < 255 &&
                (row.find("[PLAN]") == 0 || row.find("[LOCKED]") == 0 ||
                 row.find("This NPC is a GM-only") == 0),
                "Every preview row discloses planning-only status");
            test(row.find("Unlocked") == std::string::npos &&
                row.find("Completed") == std::string::npos &&
                row.find("Claim now") == std::string::npos,
                "No catalogue row falsely claims player progress");
        }
    }

    if (failed)
        return 1;
    std::cout << "PASS: " << checks
              << " sourced staging Guild Steward preview content checks\n";
    return 0;
}
