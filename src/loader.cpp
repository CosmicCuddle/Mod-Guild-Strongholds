/*
 * Naxxramas Guild Strongholds — development-only AzerothCore entry.
 *
 * AzerothCore generates Add<module-folder-with-hyphens-replaced>Scripts().
 * Both common folder spellings below are supported:
 *   modules/mod-guild-strongholds       -> Addmod_guild_strongholdsScripts()
 *   modules/Mod-Guild-Strongholds       -> AddMod_Guild_StrongholdsScripts()
 *
 * Register only a passive WorldScript in a NORMAL build. The GM snapshot
 * script is compiled/registered solely via explicit STAGING compiler opt-in.
 * Neither entry point creates housing, spawns NPCs, changes phases or SQL.
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
    NaxxGuildStrongholds::AddStagingDiagnosticsScripts();
#endif
}

// Case-preserving alias for cloning the actual GitHub repository under
// modules/Mod-Guild-Strongholds. The core calls EXACTLY ONE entry point,
// according to the folder name; this alias does not register scripts twice.
void AddMod_Guild_StrongholdsScripts()
{
    Addmod_guild_strongholdsScripts();
}
