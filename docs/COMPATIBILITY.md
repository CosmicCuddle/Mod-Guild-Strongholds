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
