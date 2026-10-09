#ifndef NAXX_GUILD_STRONGHOLD_STARTUP_GATE_H
#define NAXX_GUILD_STRONGHOLD_STARTUP_GATE_H

// Pure policy. Does not depend on AzerothCore, SQL, or Playerbots.
// Only verified capabilities may be set true by an audited runtime integration.
namespace NaxxGuildStrongholds
{
struct ValidatedCapabilities
{
    bool PrivacyIsolation = false;
    bool DatabaseAdapter = false;
    bool SafeExit = false;
    bool ModuleCompatibility = false;
};

enum class StartupDecision
{
    DisabledByConfiguration,
    BlockedByUnverifiedDependencies,
    Ready
};

constexpr StartupDecision EvaluateStartup(bool requested,
    ValidatedCapabilities const& approved) noexcept
{
    if (!requested)
        return StartupDecision::DisabledByConfiguration;

    if (!(approved.PrivacyIsolation && approved.DatabaseAdapter &&
          approved.SafeExit && approved.ModuleCompatibility))
        return StartupDecision::BlockedByUnverifiedDependencies;

    return StartupDecision::Ready;
}

// v0 development branch: NONE of these capabilities have passed a
// deployed-core staging review. These constants must remain false until
// documented, version-specific integration evidence is available.
constexpr ValidatedCapabilities DevelopmentCapabilities{};
static_assert(EvaluateStartup(true, DevelopmentCapabilities) ==
              StartupDecision::BlockedByUnverifiedDependencies,
              "Unverified development builds must never enable housing.");
}
#endif
