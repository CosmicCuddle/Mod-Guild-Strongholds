#include "StrongholdVisitGate.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0;
    int failures = 0;
    auto expect = [&](bool good, const char* name)
    {
        ++checks;
        if (!good)
        {
            ++failures;
            std::cerr << "FAILED: " << name << '\n';
        }
    };

    VisitContext c;
    c.Property = {{51, 1790000100}, PropertyLifecycle::Active, 9};
    c.VisitorGuild = {51, 1790000100};
    c.ModuleEnabled = true;
    c.IsolationVerified = true;
    c.MembershipVerified = true;

    expect(CheckGuildVisit(c) == VisitDecision::Allowed, "Verified active owner can visit");
    c.ModuleEnabled = false;
    expect(CheckGuildVisit(c) == VisitDecision::ModuleDisabled, "Disabled wins");
    c.ModuleEnabled = true;
    c.IsolationVerified = false;
    expect(CheckGuildVisit(c) == VisitDecision::IsolationUnverified, "Unverified isolation denied");
    c.IsolationVerified = true;
    c.MembershipVerified = false;
    expect(CheckGuildVisit(c) == VisitDecision::MembershipUnverified, "Former guild member denied");
    c.MembershipVerified = true;
    c.VisitorGuild = {0, 0};
    expect(CheckGuildVisit(c) == VisitDecision::NoGuild, "Guildless visitor denied");
    c.VisitorGuild = {52, 1790000100};
    expect(CheckGuildVisit(c) == VisitDecision::OtherGuild, "Other guild denied");
    c.VisitorGuild = {51, 1790000200};
    expect(CheckGuildVisit(c) == VisitDecision::WrongGeneration, "Recycled ID denied");
    c.VisitorGuild = {51, 0};
    expect(CheckGuildVisit(c) == VisitDecision::InvalidIdentity, "Unverified creation date denied");
    c.VisitorGuild = {51, 1790000100};
    c.Property.State = PropertyLifecycle::Archived;
    expect(CheckGuildVisit(c) == VisitDecision::Archived, "Archived property denied");
    c.Property.State = PropertyLifecycle::Active;
    c.Property.OriginalGuild = {51, 0};
    expect(CheckGuildVisit(c) == VisitDecision::InvalidIdentity, "Damaged owner identity denied");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks << " combined private-guild visit checks\n";
    return 0;
}
