#include "StrongholdStartupGate.h"
#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0;
    int failures = 0;
    auto verify = [&](bool ok, const char* reason)
    {
        ++checks;
        if (!ok)
        {
            ++failures;
            std::cerr << "FAILED: " << reason << '\n';
        }
    };

    verify(EvaluateStartup(false, {}) == StartupDecision::DisabledByConfiguration,
        "Disabled is default");
    verify(EvaluateStartup(true, DevelopmentCapabilities) ==
               StartupDecision::BlockedByUnverifiedDependencies,
        "Development build cannot enable housing");

    const ValidatedCapabilities all{true, true, true, true};
    verify(EvaluateStartup(false, all) == StartupDecision::DisabledByConfiguration,
        "Admin config off takes priority");
    verify(EvaluateStartup(true, all) == StartupDecision::Ready,
        "Only four verified capabilities permit readiness");
    verify(EvaluateStartup(true, {false, true, true, true}) ==
               StartupDecision::BlockedByUnverifiedDependencies,
        "Isolation is mandatory");
    verify(EvaluateStartup(true, {true, false, true, true}) ==
               StartupDecision::BlockedByUnverifiedDependencies,
        "Database adapter is mandatory");
    verify(EvaluateStartup(true, {true, true, false, true}) ==
               StartupDecision::BlockedByUnverifiedDependencies,
        "Safe exit is mandatory");
    verify(EvaluateStartup(true, {true, true, true, false}) ==
               StartupDecision::BlockedByUnverifiedDependencies,
        "Existing module compatibility is mandatory");
    verify(EvaluateStartup(false, {false, false, false, false}) ==
               StartupDecision::DisabledByConfiguration,
        "Config 0 always wins");

    if (failures)
        return 1;
    std::cout << "PASS: " << checks << " startup gate assertions\n";
    return 0;
}
