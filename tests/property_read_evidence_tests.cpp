#include "StrongholdPropertyRead.h"
#include "StrongholdStewardEvidence.h"

#include <iostream>
#include <string>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0;
    int failures = 0;
    auto check = [&](bool ok, char const* reason)
    {
        ++checks;
        if (!ok)
        {
            ++failures;
            std::cerr << "FAILED: " << reason << '\n';
        }
    };

    GuildIdentity actor{52, 1790000500};
    PropertySqlRow row{52, 1790000500, "active", 2, 4};
    StewardEvidenceInput view;
    view.ActorGuild = actor;
    view.GuildGenerationVerified = true;

    // Query disabled, missing row and error must NEVER simulate ownership.
    for (auto read : {
        InspectPropertyRow(actor, false, true, row),
        InspectPropertyRow(actor, true, false, row)
    })
    {
        if (read.CanDisplayRow())
        {
            view.PropertySnapshotLoaded = true;
            view.Property = read.Property;
        }
        check(EvaluateStewardEvidence(view).Status ==
              StewardEvidenceStatus::PropertyNotLoaded,
              "Missing disabled/unavailable query gives no property proof");
    }

    auto read = InspectPropertyRow(actor, true, true, row);
    check(read.CanDisplayRow(), "Validated stage row available for policy");
    view.PropertySnapshotLoaded = read.CanDisplayRow();
    view.Property = read.Property;
    view.SettlementLevelVerified = read.CanDisplayRow();
    view.SettlementLevel = read.Level;
    check(EvaluateStewardEvidence(view).Status == StewardEvidenceStatus::IpUnverified,
          "Valid persisted property plus unverified IP remains locked");
    check(!EvaluateStewardEvidence(view).HousingAvailable,
          "Even verified property is NOT an open private map");
    auto display = BuildStewardEvidenceRows(view);
    check(display[1].find("saved property row checked") != std::string::npos,
          "Gossip labels inspected row without claiming housing");

    row.GuildCreatedAt += 1;
    read = InspectPropertyRow(actor, true, true, row);
    view.Property = read.Property;
    check(EvaluateStewardEvidence(view).Status ==
          StewardEvidenceStatus::WrongGeneration,
          "Same numeric guild ID, different creation generation refused");
    row.GuildCreatedAt = actor.CreatedAt;
    row.Lifecycle = "archived";
    read = InspectPropertyRow(actor, true, true, row);
    view.Property = read.Property;
    check(EvaluateStewardEvidence(view).Status == StewardEvidenceStatus::Archived,
          "Archived property not treated as active housing");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks << " property SELECT-to-gossip proof checks\n";
    return 0;
}
