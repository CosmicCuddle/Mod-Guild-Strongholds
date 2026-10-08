# Diagnostic-only AzerothCore bootstrap

**Not approved for production.** This development branch now registers a single `NaxxGuildStrongholdsBootstrap` WorldScript using `WORLDHOOK_ON_BEFORE_CONFIG_LOAD` and `WORLDHOOK_ON_STARTUP`. The expected callback signatures follow the user's public `Mod-Naxxramas-Core` HonorReset.cpp code, not a guaranteed match to the running fork.

## Startup behavior

| Configuration | Behavior |
| --- | --- |
| Missing master setting | Disabled and only a foundation-loaded log |
| `NaxxGuildStrongholds.Enabled=0` | Disabled and only a foundation-loaded log |
| `NaxxGuildStrongholds.Enabled=1` | Logs **BLOCKED** warnings; no gameplay |
| Reload 1 to 0 | Returns to disabled status |

All four development capability flags — privacy isolation, persistence adapter, safe exit and deployed-module compatibility — are **false**, and a compile-time assertion rejects accidental changes that would mark the development branch as ready.

No player or guild hooks, SQL, NPCs, quests, gameobjects, teleportation, phasing, inventory changes or Playerbot/IP integration are registered.

## Test limits

The tests compile the diagnostic code using simple mock `Config.h`, `Log.h` and `ScriptMgr.h`, then exercise registration, blocked requests and config reload. This shows the expected callback pattern is syntactically consistent with the test stubs, **not** that your installed AzerothCore revision compiles or behaves identically. The installed AzerothCore, Individual Progression, Playerbots and all other modules need staging compilation and checks.

## Recovery

The bootstrap modifies no game state. To disable it, retain `Enabled=0`. To remove compiled diagnostic hooks, remove module source, re-run CMake and rebuild to a known-good binary. SQL installation remains separate, draft-only and unapplied. Full uninstall and rollback policy remains documented in `UNINSTALL.md` and `ROLLBACK.md`.

## Folder-case registration compatibility

The generated upstream AzerothCore loader takes the module folder name and replaces hyphens with underscores without normalizing letter case. Thus `mod-guild-strongholds` calls `Addmod_guild_strongholdsScripts()`, but `Mod-Guild-Strongholds` calls `AddMod_Guild_StrongholdsScripts()`. We export both via a single canonical implementation and test each symbol separately with fake core interfaces. This eliminates a preventable case-sensitive link failure on Linux; it **does not prove** a full build against the deployed core.

A separate workflow now verifies the exact loader-generation and callback declarations in a pinned **public upstream** AzerothCore revision. No normal-build code enables housing or the GM snapshot command.
