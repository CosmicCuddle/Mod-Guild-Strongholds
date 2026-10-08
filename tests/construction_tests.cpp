#include "StrongholdConstruction.h"

#include <iostream>
#include <limits>
#include <string_view>

using namespace NaxxGuildStrongholds;

int main()
{
    int failures = 0;
    int checks = 0;
    auto expect = [&](bool condition, std::string_view text)
    {
        ++checks;
        if (!condition)
        {
            ++failures;
            std::cerr << "FAILED: " << text << '\n';
        }
    };
    auto snapshot = ProjectSnapshot{42, "human_crafting", 7, {0, 0, 0}};
    auto context = ContributionContext{true, true, 42, true, false, false, true, false, 2};
    auto request = ContributionRequest{"craft-receipt_0001", {100, 50, 25}};

    expect(ValidateBuildingProjects(), "Project catalog matches theme layouts");
    expect(GetBuildingProjects().size() == 12, "Twelve project templates");
    expect(FindBuildingProject("human_crafting") != nullptr, "Known project");
    expect(FindBuildingProject("orc_crafting") != nullptr, "Orc project");
    expect(FindBuildingProject("nonexistent") == nullptr, "No arbitrary project keys");
    expect(DeriveConstructionStage({0, 0, 0}, {200, 100, 50}) == ConstructionStage::Reserved, "Reserved stage");
    expect(DeriveConstructionStage({1, 0, 0}, {200, 100, 50}) == ConstructionStage::Gathering, "Gathering stage");
    expect(DeriveConstructionStage({100, 50, 25}, {200, 100, 50}) == ConstructionStage::Scaffolding, "Scaffolding stage");
    expect(DeriveConstructionStage({200, 100, 50}, {200, 100, 50}) == ConstructionStage::Completed, "Completed stage");
    expect(DeriveConstructionStage({1, 0, 0}, {1, 0, 0}) == ConstructionStage::Completed, "Zero-requirement materials respected");

    auto first = PlanContribution(snapshot, request, context);
    expect(first.Status == ContributionStatus::Accepted, "Valid contribution accepted");
    expect(first.Proposed.Delivered.Supplies == 100, "Supplies included");
    expect(first.Proposed.Delivered.Timber == 50, "Timber included");
    expect(first.Proposed.Delivered.Iron == 25, "Iron included");
    expect(first.Stage == ConstructionStage::Scaffolding, "Scaffolding unlocked");
    expect(first.ExpectedVersion == 7 && first.Proposed.Version == 8, "Optimistic version proposal");
    expect(snapshot.Delivered.Supplies == 0 && snapshot.Version == 7, "Original snapshot unchanged");

    context.ReceiptAlreadyCommitted = true;
    auto duplicate = PlanContribution(first.Proposed, request, context);
    expect(duplicate.Status == ContributionStatus::DuplicateReceipt, "Already used receipt denied");
    expect(duplicate.Proposed.Delivered.Supplies == 100, "Duplicate proposal doesn't alter progress");
    context.ReceiptAlreadyCommitted = false;
    auto finishing = PlanContribution(first.Proposed, request, context);
    expect(finishing.Status == ContributionStatus::Accepted && finishing.Stage == ConstructionStage::Completed,
        "Second distinct delivery would complete if unique receipt");
    auto finished = finishing.Proposed;
    expect(PlanContribution(finished, request, context).Status == ContributionStatus::AlreadyCompleted, "Completed project locked");

    context.Enabled = false;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::Disabled, "Master switch");
    context.Enabled = true;
    context.IdentityVerified = false;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::IdentityUnverified, "Server identity must be verified");
    context.IdentityVerified = true;
    context.ActorGuildId = 0;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::NoGuild, "Guildless denied");
    context.ActorGuildId = 43;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::OtherGuild, "Wrong guild denied");
    context.ActorGuildId = 42;
    context.Authorized = false;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::Unauthorized, "Construction permissions checked");
    context.Authorized = true;
    context.IsBot = true;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::BotDisabled, "Bots off by default");
    context.IsBot = false;
    context.ReceiptLookupVerified = false;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::UnverifiedReceipt, "Unverified receipt store fails closed");
    context.ReceiptLookupVerified = true;
    request.ReceiptKey = "";
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::InvalidReceipt, "Empty receipt key denied");
    request.ReceiptKey = "unsafe spaces";
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::InvalidReceipt, "Invalid receipt characters denied");
    request.ReceiptKey = std::string(65, 'x');
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::InvalidReceipt, "Oversize receipt denied");
    request.ReceiptKey = "craft-receipt_0002";
    context.SettlementLevel = 1;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::SettlementTooLow, "Locked plot denied");
    context.SettlementLevel = 8;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::SettlementTooLow, "Invalid level denied");
    context.SettlementLevel = 2;
    request.Offered = {0, 0, 0};
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::EmptyContribution, "Zero donation denied");
    request.Offered = {201, 0, 0};
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::ExcessMaterials, "Overdelivery denied without losing resources");
    request.Offered = {std::numeric_limits<std::uint32_t>::max(), 0, 0};
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::ExcessMaterials, "Integer overflow guarded");
    request.Offered = {100, 50, 25};

    snapshot.Delivered = {201, 0, 0};
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::InvalidSnapshot, "Corrupt overfunded snapshot rejected");
    snapshot.Delivered = {0, 0, 0};
    snapshot.GuildId = 0;
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::InvalidSnapshot, "Missing owning guild rejected");
    snapshot.GuildId = 42;
    snapshot.ProjectKey = "unlisted";
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::UnknownProject, "Unknown snapshot project denied");
    snapshot.ProjectKey = "human_crafting";
    snapshot.Version = std::numeric_limits<std::uint64_t>::max();
    expect(PlanContribution(snapshot, request, context).Status == ContributionStatus::VersionOverflow, "Version overflow guarded");

    if (failures)
    {
        std::cerr << failures << " of " << checks << " construction checks failed\n";
        return 1;
    }
    std::cout << "PASS: " << checks << " construction catalogue and contribution checks\n";
    return 0;
}
