#include "StrongholdConstruction.h"
#include "StrongholdCatalog.h"

#include <limits>

namespace NaxxGuildStrongholds
{
namespace
{
const std::array<BuildingProject, 12> Projects{{
    {"human_hall", "human", "hall", "Royal Guild Hall", 1, {100, 40, 20}},
    {"human_military", "human", "military", "Training Yard", 2, {160, 70, 35}},
    {"human_crafting", "human", "crafting", "Workshop Court", 2, {200, 100, 50}},
    {"human_social", "human", "social", "Tavern Garden", 3, {180, 80, 40}},
    {"human_prestige", "human", "prestige", "Hall of Legends", 4, {350, 150, 100}},
    {"human_utility", "human", "utility", "Stables and Services", 5, {300, 120, 70}},
    {"orc_hall", "orc", "hall", "Warlord's Hall", 1, {100, 40, 20}},
    {"orc_military", "orc", "military", "Sparring Arena", 2, {160, 70, 35}},
    {"orc_crafting", "orc", "crafting", "Forge Camp", 2, {200, 100, 50}},
    {"orc_social", "orc", "social", "Feasting Circle", 3, {180, 80, 40}},
    {"orc_prestige", "orc", "prestige", "Victory Totems", 4, {350, 150, 100}},
    {"orc_utility", "orc", "utility", "Supply Grounds", 5, {300, 120, 70}}
}};

bool Within(Materials delivered, Materials cost)
{
    return delivered.Supplies <= cost.Supplies &&
        delivered.Timber <= cost.Timber &&
        delivered.Iron <= cost.Iron;
}

bool Meets(Materials delivered, Materials cost)
{
    return delivered.Supplies >= cost.Supplies &&
        delivered.Timber >= cost.Timber &&
        delivered.Iron >= cost.Iron;
}

bool Half(std::uint32_t amount, std::uint32_t total)
{
    return amount >= total / 2 + total % 2;
}

bool ReceiptKeyValid(std::string const& key)
{
    if (key.empty() || key.size() > 64)
        return false;
    for (char c : key)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-'))
            return false;
    return true;
}

bool SafeAdd(std::uint32_t current, std::uint32_t offered, std::uint32_t limit)
{
    return current <= limit && offered <= limit - current;
}
}

const std::array<BuildingProject, 12>& GetBuildingProjects()
{
    return Projects;
}

const BuildingProject* FindBuildingProject(std::string_view key)
{
    for (BuildingProject const& project : Projects)
        if (project.Key == key)
            return &project;
    return nullptr;
}

bool ValidateBuildingProjects()
{
    for (std::size_t i = 0; i < Projects.size(); ++i)
    {
        BuildingProject const& p = Projects[i];
        Layout const* layout = FindPrototypeLayout(p.ThemeKey);
        if (!layout || p.Key.empty() || p.Title.empty() || p.MinimumSettlementLevel < 1 ||
            p.MinimumSettlementLevel > 7 ||
            (p.Cost.Supplies == 0 && p.Cost.Timber == 0 && p.Cost.Iron == 0))
            return false;

        bool foundPlot = false;
        for (Plot const& plot : layout->Plots)
            if (plot.Key == p.PlotKey && p.MinimumSettlementLevel >= plot.RequiredSettlementLevel)
                foundPlot = true;
        if (!foundPlot)
            return false;
        for (std::size_t j = i + 1; j < Projects.size(); ++j)
            if (p.Key == Projects[j].Key ||
                (p.ThemeKey == Projects[j].ThemeKey && p.PlotKey == Projects[j].PlotKey))
                return false;
    }
    return true;
}

ConstructionStage DeriveConstructionStage(Materials delivered, Materials cost)
{
    if (Meets(delivered, cost))
        return ConstructionStage::Completed;
    if (delivered.Supplies == 0 && delivered.Timber == 0 && delivered.Iron == 0)
        return ConstructionStage::Reserved;
    if (Half(delivered.Supplies, cost.Supplies) &&
        Half(delivered.Timber, cost.Timber) && Half(delivered.Iron, cost.Iron))
        return ConstructionStage::Scaffolding;
    return ConstructionStage::Gathering;
}

ContributionPlan PlanContribution(ProjectSnapshot const& stored,
    ContributionRequest const& request, ContributionContext const& context)
{
    ContributionPlan plan;
    plan.Proposed = stored;
    plan.ExpectedVersion = stored.Version;

    auto reject = [&](ContributionStatus status) -> ContributionPlan
    {
        plan.Status = status;
        return plan;
    };

    if (!context.Enabled)
        return reject(ContributionStatus::Disabled);

    BuildingProject const* project = FindBuildingProject(stored.ProjectKey);
    if (!project)
        return reject(ContributionStatus::UnknownProject);
    if (!stored.GuildId || !Within(stored.Delivered, project->Cost))
        return reject(ContributionStatus::InvalidSnapshot);

    plan.Stage = DeriveConstructionStage(stored.Delivered, project->Cost);

    if (!context.IdentityVerified)
        return reject(ContributionStatus::IdentityUnverified);
    if (!context.ActorGuildId)
        return reject(ContributionStatus::NoGuild);
    if (context.ActorGuildId != stored.GuildId)
        return reject(ContributionStatus::OtherGuild);
    if (!context.Authorized)
        return reject(ContributionStatus::Unauthorized);
    if (context.IsBot && !context.AllowBots)
        return reject(ContributionStatus::BotDisabled);
    if (!context.ReceiptLookupVerified)
        return reject(ContributionStatus::UnverifiedReceipt);
    if (!ReceiptKeyValid(request.ReceiptKey))
        return reject(ContributionStatus::InvalidReceipt);
    if (context.ReceiptAlreadyCommitted)
        return reject(ContributionStatus::DuplicateReceipt);
    if (context.SettlementLevel < project->MinimumSettlementLevel ||
        context.SettlementLevel > 7)
        return reject(ContributionStatus::SettlementTooLow);
    if (plan.Stage == ConstructionStage::Completed)
        return reject(ContributionStatus::AlreadyCompleted);
    if (!request.Offered.Supplies && !request.Offered.Timber && !request.Offered.Iron)
        return reject(ContributionStatus::EmptyContribution);

    if (!SafeAdd(stored.Delivered.Supplies, request.Offered.Supplies, project->Cost.Supplies) ||
        !SafeAdd(stored.Delivered.Timber, request.Offered.Timber, project->Cost.Timber) ||
        !SafeAdd(stored.Delivered.Iron, request.Offered.Iron, project->Cost.Iron))
        return reject(ContributionStatus::ExcessMaterials);
    if (stored.Version == std::numeric_limits<std::uint64_t>::max())
        return reject(ContributionStatus::VersionOverflow);

    plan.Proposed.Delivered.Supplies += request.Offered.Supplies;
    plan.Proposed.Delivered.Timber += request.Offered.Timber;
    plan.Proposed.Delivered.Iron += request.Offered.Iron;
    ++plan.Proposed.Version;
    plan.Stage = DeriveConstructionStage(plan.Proposed.Delivered, project->Cost);
    plan.Status = ContributionStatus::Accepted;
    return plan;
}
}
