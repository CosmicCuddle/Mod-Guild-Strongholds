#include "StrongholdStewardEvidence.h"

#include <iostream>
#include <string>

using namespace NaxxGuildStrongholds;

int main()
{
    unsigned n = 0, failed = 0;
    auto check = [&](bool good, char const* what)
    {
        ++n;
        if (!good)
        {
            ++failed;
            std::cerr << "FAILED: " << what << '\n';
        }
    };

    StewardEvidenceInput context;
    context.ActorGuild.GuildId = 321;
    auto result = EvaluateStewardEvidence(context);
    check(result.Status == StewardEvidenceStatus::IdentityUnverified,
          "Visible guild ID alone never proves property ownership");
    check(!result.HousingAvailable, "No public housing from player guild ID");
    auto menu = BuildStewardEvidenceRows(context);
    check(menu.size() == 5 && menu[1].find("321") != std::string::npos &&
          menu[1].find("NOT verified") != std::string::npos,
          "Staging screen reports guild ID but not property ownership");
    check(menu[3].find("No verified deployed IP") != std::string::npos,
          "Unintegrated IP never invented from a character");

    context.ActorGuild.CreatedAt = 1790001234;
    context.GuildGenerationVerified = true;
    check(EvaluateStewardEvidence(context).Status ==
          StewardEvidenceStatus::PropertyNotLoaded,
          "Guild identity proof is insufficient without property DB");
    menu = BuildStewardEvidenceRows(context);
    check(menu[1].find("member verified; guild created 1790001234") != std::string::npos &&
        menu[1].find("property ownership NOT verified") != std::string::npos,
        "Verified registry membership never implies property ownership");
    context.PropertySnapshotLoaded = true;
    context.Property.OriginalGuild = {322, 1790001234};
    context.Property.State = PropertyLifecycle::Active;
    check(EvaluateStewardEvidence(context).Status == StewardEvidenceStatus::WrongGuild,
          "Other-guild ownership rejected");
    context.Property.OriginalGuild = {321, 1790001235};
    check(EvaluateStewardEvidence(context).Status ==
          StewardEvidenceStatus::WrongGeneration, "Recycled guild ID rejected");
    context.Property.OriginalGuild = context.ActorGuild;
    context.Property.State = PropertyLifecycle::Archived;
    check(EvaluateStewardEvidence(context).Status == StewardEvidenceStatus::Archived,
          "Archived guild can't show active property");
    context.Property.State = PropertyLifecycle::Active;
    check(EvaluateStewardEvidence(context).Status ==
          StewardEvidenceStatus::LevelUnverified,
          "No verified settlement level means no planning privileges");
    context.SettlementLevelVerified = true;
    context.SettlementLevel = 8;
    check(EvaluateStewardEvidence(context).Status ==
          StewardEvidenceStatus::LevelUnverified, "Invalid level blocked");
    context.SettlementLevel = 4;
    check(EvaluateStewardEvidence(context).Status ==
          StewardEvidenceStatus::IpUnverified,
          "Unverified IP cannot satisfy activity requirements");

    context.Ip = {true, true, true, 18, 7};
    result = EvaluateStewardEvidence(context);
    check(result.Status == StewardEvidenceStatus::PreviewOnly &&
          result.EffectiveIpRank == 7, "Real IP cap overrides rewarded rank");
    check(result.PlannedActivityGates[0] == ActivityGate::Allowed &&
          result.PlannedActivityGates[5] == ActivityGate::ProgressionTooLow &&
          result.PlannedActivityGates[6] == ActivityGate::ProgressionTooLow,
          "Planning activity eligibility follows IP and level caps");
    check(!result.HousingAvailable, "All-valid synthetic proof NEVER enables housing");

    context.Ip.ProgressionLimit = 8;
    result = EvaluateStewardEvidence(context);
    check(result.PlannedActivityGates[5] == ActivityGate::Allowed &&
          result.PlannedActivityGates[6] == ActivityGate::ProgressionTooLow,
          "Outland opens in preview only, Northrend remains locked");
    context.Ip.ProgressionLimit = 13;
    result = EvaluateStewardEvidence(context);
    check(result.PlannedActivityGates[6] == ActivityGate::Allowed,
          "Northrend tier planning gate uses verified stage");
    context.IsBot = true;
    result = EvaluateStewardEvidence(context);
    check(result.PlannedActivityGates[0] == ActivityGate::BotDisabled,
          "Bots don't receive planned activities");
    context.IsBot = false;
    context.Ip.ModuleEnabled = false;
    result = EvaluateStewardEvidence(context);
    check(result.Status == StewardEvidenceStatus::IpUnverified &&
          result.PlannedActivityGates[0] == ActivityGate::ModuleDisabled &&
          result.EffectiveIpRank == 0, "Disabled IP clears planning permissions");

    menu = BuildStewardEvidenceRows(context);
    check(menu.size() <= 7, "Audit preview fits gossip with navigation");
    for (std::string const& row : menu)
    {
        check(row.rfind("[LOCKED]", 0) == 0 && row.size() < 255,
              "Every line visibly locked and byte bounded");
        check(row.find("Available now") == std::string::npos &&
              row.find("Unlocked") == std::string::npos,
              "No runtime access implied");
    }
    if (failed)
        return 1;
    std::cout << "PASS: " << n << " Guild Steward verified-evidence preview checks\n";
    return 0;
}
