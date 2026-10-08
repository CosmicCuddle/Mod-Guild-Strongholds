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
}

void Addmod_guild_strongholdsScripts()
{
    NaxxGuildStrongholds::AddBootstrapScripts();
}
