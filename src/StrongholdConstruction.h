#ifndef NAXX_GUILD_STRONGHOLD_CONSTRUCTION_H
#define NAXX_GUILD_STRONGHOLD_CONSTRUCTION_H

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

// Standalone domain logic, NOT a live AzerothCore script.
// Calling code must fetch authoritative identity, balances and receipt status.
// The persistence adapter must commit the proposal, material deduction,
// unique receipt and event ledger in ONE database transaction.
namespace NaxxGuildStrongholds
{
struct Materials
{
    std::uint32_t Supplies = 0;
    std::uint32_t Timber = 0;
    std::uint32_t Iron = 0;
};

struct BuildingProject
{
    std::string_view Key;
    std::string_view ThemeKey;
    std::string_view PlotKey;
    std::string_view Title;
    std::uint8_t MinimumSettlementLevel;
    Materials Cost;
};

enum class ConstructionStage : std::uint8_t
{
    Reserved,     // No materials
    Gathering,    // Partially supplied
    Scaffolding,  // Every required resource at least half supplied
    Completed     // All resources supplied
};

struct ProjectSnapshot
{
    std::uint32_t GuildId = 0;
    std::string ProjectKey;
    std::string PlotKey;
    std::uint64_t Version = 0;
    Materials Delivered;
};

struct ContributionRequest
{
    std::string ReceiptKey;
    Materials Offered;
};

struct ContributionContext
{
    bool Enabled = false;
    bool IdentityVerified = false;
    std::uint32_t ActorGuildId = 0;
    bool Authorized = false;
    bool IsBot = false;
    bool AllowBots = false;
    bool ReceiptLookupVerified = false;
    bool ReceiptAlreadyCommitted = false;
    std::uint8_t SettlementLevel = 0;
    std::string_view SettlementThemeKey;  // Server-verified selected theme
};

enum class ContributionStatus : std::uint8_t
{
    Accepted,
    Disabled,
    UnknownProject,
    InvalidSnapshot,
    IdentityUnverified,
    NoGuild,
    OtherGuild,
    Unauthorized,
    BotDisabled,
    UnverifiedReceipt,
    DuplicateReceipt,
    InvalidReceipt,
    AlreadyCompleted,
    EmptyContribution,
    ExcessMaterials,
    VersionOverflow,
    SettlementTooLow
};

struct ContributionPlan
{
    ContributionStatus Status = ContributionStatus::InvalidSnapshot;
    ProjectSnapshot Proposed;
    ConstructionStage Stage = ConstructionStage::Reserved;
    std::uint64_t ExpectedVersion = 0;
};

const std::array<BuildingProject, 12>& GetBuildingProjects();
const BuildingProject* FindBuildingProject(std::string_view key);
bool ValidateBuildingProjects();
ConstructionStage DeriveConstructionStage(Materials delivered, Materials cost);

// Stateless validation and proposal only. NEVER deduct resources here.
// Even Accepted is NOT durable: call-site must transact everything atomically.
ContributionPlan PlanContribution(ProjectSnapshot const& stored,
    ContributionRequest const& request, ContributionContext const& context);
}

#endif
