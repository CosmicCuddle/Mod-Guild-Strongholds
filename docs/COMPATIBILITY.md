# Module Compatibility Policy — mandatory release gate

**Status: policy only. No compatibility testing has been performed.**

Naxxramas Guild Strongholds **must not be installed on the live server until it has been tested against the user's *actual deployed* AzerothCore revision, module list, module versions and configurations.** We cannot truthfully promise zero conflict from source inspection alone.

## Known systems to check

| System | Main potential interaction | Status |
| --- | --- | --- |
| Individual Progression (IP) | Character-level content eligibility, phasing, quest availability, instance unlock state | NOT TESTED |
| Playerbots / player's specific fork | Bot guild membership, quest participation, group/raid credit, character/bot teleport behaviour | NOT TESTED |
| Mod-Naxxramas-Core | Custom systems, world/character scripts, event handling, future common NPCs or rewards | NOT TESTED |
| mod-dungeon-clear (if installed) | Dungeon completion hooks, repeatable reward eligibility and event double-counting | NOT TESTED |
| Existing Guild House/housing or custom phasing module (if present) | Private-area ownership and phasing collisions; potentially mutually exclusive | NOT TESTED |
| Any other deployed modules | Unknown hooks, commands, NPC IDs, SQL, core patches and map/teleport behaviour | INVENTORY REQUIRED |

This is **not an inventory of everything installed**. The inventory must be collected from the running server's source checkout and its module directory, not guessed from GitHub repositories or old conversations. A repository existing does not mean its module is deployed.

## Rules that block release

1. Keep code in a dedicated module. No changes to original AzerothCore, Playerbots, IP or Naxxramas Core files unless specifically reviewed and backed up.
2. Module settings must have the `NaxxGuildStrongholds.` prefix; C++ registrations and SQL tables must have project-specific names.
3. Do not use unreviewed `creature_template`, `gameobject_template`, quest, gossip, world-spawn, spell, or command IDs. Reserve and document every shared identifier before writing SQL.
4. Never alter IP quest-reward states, progression records, or per-character phasing. Determine IP eligibility through a version-matched read-only adapter.
5. Do not override server-wide phase comparison, global phase flags, or teleport/summon behaviour without a narrowly scoped, proven approach and explicit source review.
6. Do not assume the existing Guild House implementation and this module can safely coexist in the same location.
7. Every game-event hook must be idempotent: dungeon/raid rewards, boss kill awards and construction contributions cannot double-count when multiple installed modules listen for the same events.
8. Playerbots are opted out of construction, repeatable payouts and related events until an audited adapter and abuse limits are ready.
9. New schema stays under `naxx_gs_*`; record any module-owned world rows for later selective cleanup. Never remove another module's rows on uninstall.
10. If a dependency or version is unknown, **fail closed**: keep the feature disabled and refuse the live deployment.

## Compatibility test plan (staging copy)

1. Create a complete module inventory with exact paths and Git SHAs, including any fork-specific patches.
2. Record SQL table names and project-used gameobject/NPC/quest identifiers; run collision checks before allocating anything.
3. Configure/build core with all the **existing** modules enabled and Strongholds disabled. Confirm the baseline runs.
4. Configure/build with Strongholds enabled on staging, without removing the existing modules. Confirm clean startup and shutdown.
5. Test two guilds concurrently in the **same** property type: no player, NPC, gameobject or aura phase leakage; safe exit and login restore for each.
6. Test a guild with mixed IP tiers: identical shared scenery, different approved quest/NPC menus; no locked-tier progression granted to lower-tier characters.
7. Test Playerbots in and outside guilds: no unintended teleport loops, quest completions, duplicate dungeon/raid credit or bot-driven farming.
8. Test dungeon/raid event overlap with installed completion, honor and other custom modules; exactly-once credit for qualifying activities.
9. Test both factions, guild disbanding, guild leadership changes, reconnect, restart, off-hours resets and permissions.
10. Disable/re-enable; preserve data through uninstall/reinstall; verify no stray spawns and no character/phasing changes.
11. Record outcomes, date, revisions, and remediation in the release checklist. **Any unresolved conflict blocks release.**

## Read-only inventory helper

`scripts/check-module-inventory.sh` lists installed module directories and visible Git revisions from a specified AzerothCore checkout. It does not read credentials or change files.

Usage on the Linux machine (when ready to begin compatibility review):

```bash
bash modules/Mod-Guild-Strongholds/scripts/check-module-inventory.sh /path/to/azerothcore
```

The user must substitute the correct checkout path. This helper is **not a substitute** for checking compiled modules, applied core patches, database schemas or runtime settings. It does not need to be run during documentation-only development.

## Release sign-off

A release note must state the tested AzerothCore commit, Playerbots fork/ref, IP ref, all other installed module commits, compatible configuration and SQL migration versions, and results of the above tests. No inferred claims of compatibility.

## Logical catalogue smoke tests

`bash tests/run-catalog-tests.sh` must pass with a C++17 compiler in the review branch. Its 21 checks cover unique themes, faction-appropriate selection, initial plot layouts and fail-closed access-policy decisions. Passing does **not** replace AzerothCore CMake, staging realm, IP phasing or Playerbot integration tests.


## IP activity policy smoke tests

The standalone test runner now includes activity eligibility checks for mismatched guilds, unverified IP stages, lower-tier access to Outland/Northrend, disabled module access, bot opt-out and invalid settlement levels. These validate domain decisions only; they do **not** verify the installed IP fork, quest events, or database persistence.

## Construction contribution compatibility checks

The new standalone construction-policy tests verify project definitions, different construction stages, guild/rank access, bot opt-out, duplicate receipt checks, contribution limits and fail-closed invalid states. Before in-game activation, verify external-mod event deduplication, **transactional** receipt uniqueness, currency/inventory accounting, race conditions and repeated login/restart behavior. Standalone tests do not establish runtime compatibility.

## MariaDB contract coverage

CI now includes an isolated MariaDB service for draft-character-schema and **virtual** guild supply accounting tests. These verify basic InnoDB rollback and concurrent receipt protection but cannot certify safe AzerothCore in-memory character inventory handling, IP phasing, Playerbots, core versions, restart evacuation or long-term compatibility. A live server install remains blocked.

## Runtime diagnostic coverage

The new WorldScript subscribes to config load/startup only, with no access to the DB, phasing, character scripts or WorldObjects. Its compile-time readiness flags all remain false. CI compiles the source against **mock** Config/Log/ScriptMgr headers to exercise logging and reload; this is not a substitute for compiling/testing against the actual full module stack.

## Guild property ownership — test boundaries

`StrongholdProperty.*` deliberately does not include AzerothCore Guild/Player APIs, and does not register a gossip or command. Tests reject cross-faction and cross-guild access and a second property claim. A future production adapter must check faction/guildmaster status through the actual installed AzerothCore classes. The MariaDB test proves only namespaced ownership and concurrency. No compatibility with other installed guild/housing modules is claimed.


## Lifecycle and old-guild ID reuse

A numeric guild ID alone is insufficient to restore a saved property. The proposed source uses the original guild creation date as a defensive fingerprint. Archived settlements block contributions and entry. A restoration requires independent verified administration and proof of the *original* guild identity. This is not guaranteed unique under every data restore or migration; deployed-fork checks remain mandatory. No guild-disband hook is registered yet.


## Safe-return contract

The new safe-return policy and MariaDB test preserve a player's server-captured original position before a hypothetical teleport, and keep the ticket until verified return. Entry remains blocked if isolation or guild ownership is unverified; exit must work even after loss of guild membership or module disable. Runtime integration needs IP phase restoration, Playerbots/logout/death/disband checks, map-coordinate/terrain validation and original core API compatibility. This code currently performs no worldserver teleport or data writes.

## Read-only phase-hook inventory

The optional `scripts/audit-isolation-compatibility.py` scans the AzerothCore source checkout's existing modules for phase-mask assignments, global phase hooks, Individual Progression phase spells, teleport calls, instance routing and guild-lifecycle hooks. It prints module names, independent Git revisions (when available) and **pattern identifiers/counts**, never configuration values. It reads source only and does not modify or upload anything.

A match means **review needed**, not necessarily an actual conflict; lack of matches is **not proof of compatibility**. The output always says `REVIEW_REQUIRED`, even if no patterns are found. Current CI tests use an invented source fixture, not the user's live module inventory.

The `StrongholdIsolationProbe` C++ tests model directional phase comparison and synthetic 2-guild player/NPC/gameobject visibility. Such passing tests cannot satisfy the live two-guild acceptance gate or change `DevelopmentCapabilities.PrivacyIsolation`.

## Optional staging-only administrative snapshot

The prospective `.naxxgs snapshot` command is **not compiled into normal builds**, nor available without an independent diagnostics config opt-in and AzerothCore `RBAC_PERM_COMMAND_DEBUG_INFO` permission. It reports only the calling staff character's guild/map/instance/zone/area/phase numbers for future staging review. The command cannot reveal internal phase comparison mode, and any mismatch with installed command/RBAC APIs must be resolved before a staging build. It never teleports or assigns phase. **It does not certify privacy** and cannot mark housing ready.


## Fork-specific Individual Progression cap

The reviewed *public* user fork `Grimfeather/mod-individual-progression` (commit `706740808fee328b8557607f87b0548cf961e047`) exposes `enabled`, `progressionLimit` and `GetPlayerProgressionFromQuests`. Its `hasPassedProgression` checks the cap **in addition to rewarded quest rank**. This creates a crucial potential tier bypass if a Strongholds integration reads only completed hidden quests. Our new `StrongholdIpCompatibility.*` domain policy instead computes an effective rank and revokes stale/unknown evidence.

The `check-grimfeather-ip-contract.py` scanner checks actual pinned public fork source in a separate GitHub Actions job and rejects changed public API/enum/rank/cap semantics; it does not prove the *installed* fork matches, nor certify a full build or lack of phasing interference. See [IP_FORK_COMPATIBILITY.md](IP_FORK_COMPATIBILITY.md).


## Slot accounting versus real private-area compatibility

`StrongholdPhaseLease.*` and a dedicated disposable MariaDB test now cover double allocations, externally occupied bit rejection, guild-generation uniqueness, concurrent claims, no capacity and permanent historical holds. These **do not verify live phase occupancy**. The approved pool must be built only after the *actual deployed* Individual Progression, Playerbots, guild-house and all other modules have been audited, including DB-sourced phase masks and hidden IP spells.

The CI `1 << 24` and `1 << 25` masks are synthetic values. Never write them into your worldserver as presumed safe defaults. If the real server cannot reserve sufficient non-conflicting bits with uniform behavior, abandon this candidate instead of force-changing global phase comparison. All housing readiness flags remain false.

## Guild-private instance routing feasibility

The new C++ route preflight tests source assumptions for candidate A, but **does not register or create AzerothCore instances**. A proper adapter cannot simply generate and persist an instance ID; it must integrate with the actual core instance manager and prove that `PlayerGetDestinationInstanceId` and group/raid binds do not override ownership routing. Map DBC and instance templates must be valid, without repurposing Molten Core, Onyxia or any IP-gated progression dungeon.

The upstream reviewed core commit for this research was `7b2cecef92b271a468e39d89831b520b20ae06a8` (8 October 2026), **not a verified deployed server revision**. The code has tests for stale guild generations and client arrival mismatches; all other module compatibility remains unverified. See [INSTANCE_ROUTING_FEASIBILITY.md](INSTANCE_ROUTING_FEASIBILITY.md).

## Actual upstream module build contract (not full compile)

AzerothCore's upstream CMake loader uses the module folder's literal spelling, replacing `-` with `_` while keeping case to generate `Add<folder>Scripts()`. We now support both the conventional all-lowercase `mod-guild-strongholds` folder and the capitalized GitHub repository `Mod-Guild-Strongholds`; a single registration path avoids duplicate scripts. The upstream collector discovers `src/` recursively, so no `tests/` fake C++ code is included in worldserver.

The `scripts/check-azerothcore-module-loader.py` checker and separate CI job verify these contracts against the pinned public upstream AzerothCore commit `7b2cecef92b271a468e39d89831b520b20ae06a8`, including WorldScript and CommandScript signatures. Any drift fails the check. **It does not compile the user's actual customized core** and cannot certify the modules, staging GM command, housing or IP phasing.

The entire housing feature remains hard-blocked under `DevelopmentCapabilities`. See [CORE_BUILD_CONTRACT.md](CORE_BUILD_CONTRACT.md).


## Disposable pinned-upstream module compilation

A new GitHub Actions compile check runs the current Strongholds source in a **temporary pinned upstream AzerothCore** CMake build, with real game headers and no runtime server or databases. It verifies the disabled-by-default module's source compiles and expected loader symbols are exported. This goes beyond matching CMake text and compiling mock headers.

The deployed core may differ materially (Playerbots, Grimfeather IP, custom Naxxramas Core and other modules). Passing the public-upstream module target **cannot** replace a full separate staging build against the deployed complete sources, nor does it prove gameplay collision freedom, safe teleport recovery or privacy.


## Separate normal and staging compiler variants

The upstream GitHub build now tests that a normal `worldserver` can link against the module with no staging GM diagnostic, and separately compiles the opt-in diagnostics using real public upstream command API headers. A *successful* public-source full link cannot prove compatibility with your installed fork, Playerbots, custom Naxxramas-Core or Individual Progression. Their versions and coexistence remain release blockers.


## Guild Steward gossip API and staged safety

The new `StrongholdStagingSteward.cpp` is an upstream-AzerothCore `CreatureScript`, explicitly compiled only with the separate `NAXX_GS_BUILD_STAGING_STEWARD` definition. It uses `ScriptedGossip.h`, server-read `Player::IsGameMaster()` and `GetGuildId()`, `OnGossipHello` and `OnGossipSelect`. Every gossip selection is checked again for active config, GM mode, guild membership, exact sender and approved informational action.

There is deliberately **no** assigned `creature_template` row, SQL or spawn, nor housing service code. The optional script must be compiled and verified against the user's actual installed AzerothCore fork on a separate staging server before *any* bind is permitted. Existing Individual Progression quests, spells and Playerbots are untouched. See [STAGING_STEWARD.md](STAGING_STEWARD.md).

## Guild Steward script uses only read-only catalogue data

The opt-in staging `CreatureScript` now links the `StrongholdCatalog`, `StrongholdConstruction`, and `StrongholdActivities` definitions to generate gossip pages. It has no runtime IP header, DB adapter, Playerbots mutation, inventory, quest reward or phase APIs. The real-header upstream opt-in compilation is the next check; the deployed fork and in-game gossip remain untested. Normal server builds never register the staging script.
