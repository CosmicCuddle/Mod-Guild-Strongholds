# AzerothCore module build integration — pinned source contract

**Status: upstream source inspected and two loader aliases smoke-linked against fake headers. No full worldserver build has been performed.**

## Issue corrected

AzerothCore's module loader automatically chooses a C++ function named `Add<module-directory-name-with-hyphens-changed-to-underscores>Scripts()`. The upstream CMake code does **not** normalize filename case. Linux treats these as different symbols:

| Actual module folder | Loader function |
| --- | --- |
| `modules/mod-guild-strongholds` | `Addmod_guild_strongholdsScripts()` |
| `modules/Mod-Guild-Strongholds` | `AddMod_Guild_StrongholdsScripts()` |

Our original `src/loader.cpp` only exported the lowercase one, despite the GitHub repository name containing capitals. It could therefore fail to link when cloned into `modules/Mod-Guild-Strongholds`.

**Fix:** `src/loader.cpp` now exports both symbols. The uppercase alias calls the lowercase implementation. A generated loader selects **one** based on the folder name, so this does not install scripts twice. Both code paths are smoke-compiled and tested.

## Real upstream build mechanics reviewed

Pinned upstream AzerothCore: `7b2cecef92b271a468e39d89831b520b20ae06a8` (8 October 2026).

- [`modules/CMakeLists.txt`](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/modules/CMakeLists.txt) builds module registration names, sets static/dynamic linking, and collects source files.
- [`ConfigureModules.cmake`](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/src/cmake/macros/ConfigureModules.cmake) defines each module's source directory as `modules/<folder>/src`.
- [`AutoCollect.cmake`](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/src/cmake/macros/AutoCollect.cmake) recursively collects `.cpp/.h` files **under `src/`**.
- [`ModulesLoader.cpp.in.cmake`](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/modules/ModulesLoader.cpp.in.cmake) invokes the computed registration functions.
- [`WorldScript.h`](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/src/server/game/Scripting/ScriptDefines/WorldScript.h) declares the passive startup hooks.
- [`CommandScript.h`](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/src/server/game/Scripting/ScriptDefines/CommandScript.h) declares the command interface used **only for optional staging diagnostics**.

**No separate module-level `CMakeLists.txt` is required under this upstream CMake setup.** Adding one would not substitute for the core's normal discovery. Our `tests/` and fake headers are outside `src/` and will not be compiled into worldserver through that default collector.

## CI checks

1. `bash tests/run-catalog-tests.sh` compiles the passive WorldScript twice with fake core headers: once invoking the lowercase module symbol and once the case-preserving alias.
2. `python3 tests/module_loader_contract_tests.py` includes negative cases for changed CMake macros, missing function exports, broken alias, removed default-disable flag or accidentally unguarded staging diagnostic.
3. `python3 scripts/check-azerothcore-module-loader.py reviewed-azerothcore` checks source contracts against a pinned, **real** upstream AzerothCore checkout in an independent GitHub Actions job.
4. It always labels the **deployed customized core** as unverified and does not edit or upload core files, call any worldserver API or touch SQL.

## Remaining required staging test

- Obtain the actual installed AzerothCore revision, module folder names, IP, Playerbots and other module revisions first.
- Confirm the custom fork's module generator and ScriptMgr/WorldScript/CommandScript signatures.
- Compile the **actual** `mod-guild-strongholds` source against a backed-up staging copy of the complete server with every other installed module.
- Confirm only one passive bootstrap registers and that `NaxxGuildStrongholds.Enabled=0` is inert; setting it to 1 still **must block housing**.
- Check for name collisions, duplicate scripts, module config location, Linux compiler differences, and dynamic/static linkage.
- Revert staging binaries/configs to the known-good build after testing. Do not change production until all private-area, IP, Playerbots and rollback gates have been satisfied.

**No live server install, recompile, SQL or map change is authorized by these tests.**


## Isolated upstream compile workflow

**New:** `.github/workflows/upstream-compile.yml` is a separate workflow which can run on changes to `src/`, `conf/` or its own workflow file on the development branch, or by manual dispatch. It:
1. Fetches the exact public AzerothCore commit `7b2cecef92b271a468e39d89831b520b20ae06a8`, in a **disposable GitHub runner**, separate from the live server.
2. Copies the current Strongholds `src/` and `conf/` (not SQL, other module folders, fake headers or scripts) into `core/modules/mod-guild-strongholds`.
3. Installs upstream Clang/CMake/Ninja build dependencies and configures the real AzerothCore CMake project with static modules, tools disabled and housing config default off.
4. Builds **only the actual `modules` target**, using real upstream game headers and static module source discovery.
5. Verifies the resulting static archive contains both module-folder-name entry symbols.

It never runs `worldserver`, installs anything to the user’s server, contacts MySQL, modifies database records, spawns NPCs or changes player phases. Source checkout and compilation take place exclusively on the hosted CI runner. It also avoids downloading any DBC/game-client assets.

**Important test boundary:** compiling a static `modules` library is **not** equivalent to linking the entire `worldserver`. It is not proof of any live module stack, persistent private instances, phasing, safe return, playerbots, or actual NPC/quest functionality. If upstream builds change, this check must fail visibly and we must not infer deployed-fork compatibility. A separate real **staging worldserver build** will still be required before deployment.

**Normal module behaviour:** the passive WorldScript remains disabled by default and `NAXX_GS_BUILD_STAGING_DIAGNOSTICS` is not provided. All `DevelopmentCapabilities` are false.
