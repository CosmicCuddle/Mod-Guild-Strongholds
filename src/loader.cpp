/*
 * Naxxramas Guild Strongholds — development-only AzerothCore module entry.
 *
 * Active registration is restricted to the passive diagnostic WorldScript.
 * No housing, guild, player, phase, spawn, teleport or database hooks.
 * Release-blocking compatibility testing is still required.
 */
namespace NaxxGuildStrongholds
{
void AddBootstrapScripts();
#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
void AddStagingDiagnosticsScripts();
#endif
}

void Addmod_guild_strongholdsScripts()
{
    NaxxGuildStrongholds::AddBootstrapScripts();
#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
    // Explicit compile-time staging opt-in; ABSENT from normal module builds.
    // The command still requires GM RBAC and Diagnostics.Enabled=1.
    NaxxGuildStrongholds::AddStagingDiagnosticsScripts();
#endif
}
