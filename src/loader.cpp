/*
 * Naxxramas Guild Strongholds — development-only AzerothCore entry.
 *
 * AzerothCore generates Add<module-folder-with-hyphens-replaced>Scripts().
 * Both common folder spellings below are supported:
 *   modules/mod-guild-strongholds       -> Addmod_guild_strongholdsScripts()
 *   modules/Mod-Guild-Strongholds       -> AddMod_Guild_StrongholdsScripts()
 *
 * Register only a passive WorldScript in NORMAL builds. Read-only GM snapshot
 * and informational CreatureScript are separate explicit STAGING opt-ins.
 * Neither entry point spawns an NPC, opens housing, changes phases or SQL.
 */
namespace NaxxGuildStrongholds
{
void AddBootstrapScripts();
#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
void AddStagingDiagnosticsScripts();
#endif
#if defined(NAXX_GS_BUILD_STAGING_STEWARD)
void AddStagingStewardScripts();
#endif
#if defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)
void AddStagingRaidObserverScripts();
#endif
}

void Addmod_guild_strongholdsScripts()
{
    NaxxGuildStrongholds::AddBootstrapScripts();
#if defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
    NaxxGuildStrongholds::AddStagingDiagnosticsScripts();
#endif
#if defined(NAXX_GS_BUILD_STAGING_STEWARD)
    NaxxGuildStrongholds::AddStagingStewardScripts();
#endif
#if defined(NAXX_GS_BUILD_STAGING_RAID_OBSERVER)
    NaxxGuildStrongholds::AddStagingRaidObserverScripts();
#endif
}

// Case-preserving alias for cloning the actual GitHub repository under
// modules/Mod-Guild-Strongholds. The core calls EXACTLY ONE entry point,
// according to the folder name; this alias does not register scripts twice.
void AddMod_Guild_StrongholdsScripts()
{
    Addmod_guild_strongholdsScripts();
}
