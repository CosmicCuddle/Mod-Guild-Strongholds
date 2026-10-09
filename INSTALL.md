# Installation policy (foundation branch)

**Do not install this foundation branch on a live server.** It registers no gameplay, has no NPC/world spawns and has not been compiled against the deployed AzerothCore fork.

## Before any future build or migration

1. Write down the AzerothCore commit hash, Playerbots revision and Individual Progression revision.
2. Record the currently working `worldserver` binary location and the currently deployed module revisions.
3. Stop `worldserver` before database schema migrations or module removal.
4. Take tested backups of the character and world databases and the deployed configuration and binaries. See `backup/README.md`.
5. Use a separate test environment where possible; do not install two guild privacy modules without a compatibility review.

## Planned module installation

After compatibility work and documented approval, the module will be cloned into AzerothCore's `modules` folder, CMake will be reconfigured and the worldserver rebuilt. `conf/mod_naxx_guild_strongholds.conf.dist` will be copied to the server config directory as a local, separately editable `.conf` file.

Settings default to disabled; only enable after successful isolation and safe-exit tests.

The draft SQL schema lives in `data/sql/manual/install_characters.sql`. It is **not** automatically applied from the normal AzerothCore updater directories. Review and back up the character database before applying an approved migration.

Do not run installation SQL on `acore_world` or `acore_auth`; it is intended for the character database only.

This document will be expanded with exact commands when the target AzerothCore and database configuration are known.

## Mandatory compatibility preflight

Before even a staging install, record **all modules actually deployed**, the AzerothCore commit, the Playerbots fork/revision, the IP module revision and custom core patches. Follow [COMPATIBILITY.md](docs/COMPATIBILITY.md). Do not remove, replace, or silently modify existing modules to make Strongholds build. A compile failure, phase conflict, SQL/entry ID collision or duplicated completion reward is a release blocker.

## Staging-only bootstrap

The earlier empty loader has become a **diagnostic-only** WorldScript, which logs disabled or blocked status during startup. Do not deploy this branch live. The test uses fake core headers; validate against your actual AzerothCore fork and complete installed-module inventory before any staging deployment. This does not permit real housing enablement.

## Source layout and module naming — staging only, not an install instruction yet

Upstream AzerothCore discovers a module when its folder under `modules/` has a `src/` directory. It recursively compiles `src/` (not standalone `tests/`), and its generated script loader calls a C++ registration name based on the **exact folder spelling**:

| Folder under modules | Generated C++ registration |
| --- | --- |
| `mod-guild-strongholds` (recommended) | `Addmod_guild_strongholdsScripts()` |
| `Mod-Guild-Strongholds` (GitHub repository name) | `AddMod_Guild_StrongholdsScripts()` |

Both symbols exist in `src/loader.cpp`; one delegates to the other. **Only one of them is called** by the generated loader in any given build. This prevents Linux case-sensitive linker problems without touching any other installed module.

The repository intentionally does not supply a separate module-root `CMakeLists.txt` because the reviewed upstream AzerothCore loader manages module source discovery. The module's test code is outside `src/`, and `StrongholdStagingDiagnostics.cpp` has an additional compile-time guard in normal builds.

**Do not clone/pull this draft into a live server yet.** Real staging compilation against the exact deployed core and all modules is still required. The new automated check reads the public upstream core loader source but does not compile/link the actual user's worldserver. See [CORE_BUILD_CONTRACT.md](docs/CORE_BUILD_CONTRACT.md).


## CI upstream compile is not an installation

The separate GitHub Actions upstream-build workflow uses only a temporary runner and builds a disabled gameplay module's C++ source against **public upstream** AzerothCore headers. It is safe to inspect without any change to the live server. It does not start or install worldserver, access any SQL database or create in-game objects. No user Git Pull, recompile, MobaXterm command or HeidiSQL operation is needed for this development milestone.

Before any future **staging install**, still identify the exact deployed AzerothCore/Playerbots/IP/module revisions, back up character/world databases and binaries, and prepare a separate rollback. Upstream CI success never authorises production installation.
