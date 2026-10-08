# Naxxramas Guild Strongholds

A planned AzerothCore 3.3.5 module for private, customisable guild settlements.

**Status:** v0.1.0 foundation proposal, **not gameplay-ready**. No housing, phasing, quests, NPCs or object spawning are implemented yet. The C++ loader is intentionally inert and the feature is disabled by default.

## Planned gameplay

- **Hybrid housing:** a guild selects one prepared property and customises building plots and decoration slots.
- **Ten racial themes:** Human, Dwarf, Night Elf, Gnome, Draenei; Orc, Troll, Tauren, Undead, Blood Elf. A guild may choose a theme within its faction.
- **Shared world:** all eligible guild members see the same buildings and earned trophies.
- **Individual Progression (Option B):** each character's IP state controls quests, NPC services and activities; the shared settlement remains visible.
- **Long-term progression:** seven proposed settlement stages, daily assignments, weekly guild projects, raid trophies and a permanent guild ledger.
- **Playerbots:** future controlled participation, not unlimited automated currency generation.

## Safety-first development

1. Never deploy unreviewed code or SQL to the live server.
2. Maintain a reversible install, a data-preserving uninstall and a separate destructive purge.
3. Do not overwrite base AzerothCore or Individual Progression tables.
4. Test guild separation, logouts, relogs, teleport returns, restarts and progression checks before allowing property purchases.
5. Stop and back up the databases before any schema installation or upgrade.

Read **[DESIGN.md](docs/DESIGN.md)**, **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** and **[ROADMAP.md](docs/ROADMAP.md)**.

## Current repository contents

- `src/loader.cpp`: compile-time registration stub only.
- `conf/mod_naxx_guild_strongholds.conf.dist`: disabled-by-default settings.
- `data/sql/manual/install_characters.sql`: optional, **manual** isolated schema creation; do not execute yet.
- `INSTALL.md`, `UNINSTALL.md`, `ROLLBACK.md`: installation and recovery policy.
- `uninstall/purge_characters.sql`: explicitly destructive module-data purge, never automatic.
- `backup/README.md`: backup checklist.
- `CHANGELOG.md`: version history.

**There is no reason to git-pull or recompile this on the live server yet.**

## Compatibility still to verify

AzerothCore revision, C++ module loader convention on the deployed core, `mod-individual-progression` API and Playerbots fork must be checked before the first executable feature is added.

## Mandatory module compatibility gate

Guild Strongholds must coexist with the server's **actually installed** modules, not only upstream AzerothCore. Compatibility is a **release blocker**, not an assumption. See [COMPATIBILITY.md](docs/COMPATIBILITY.md) for the required inventory, IP/Playerbots/Naxxramas Core interaction matrix and staged testing plan. No live installation until verified.

## Next development batch — logical settlement catalogue

The `src/StrongholdCatalog.h/.cpp` pair defines all ten racial themes and two *logical* Human/Orc building layouts, with pure faction and guild access-policy checks. These checks do **not** create housing instances or teleport players. Read [SETTLEMENT_CATALOG.md](docs/SETTLEMENT_CATALOG.md) and [ISOLATION_RESEARCH.md](docs/ISOLATION_RESEARCH.md).

A standalone C++ regression test is provided at `bash tests/run-catalog-tests.sh` (requires a C++17 compiler). It tests the data and fail-closed policy, **not** AzerothCore integration or compatibility with deployed modules.


## Milestone: per-character activity eligibility groundwork

`src/StrongholdActivities.h/.cpp` now defines nine sample daily, weekly and one-time settlement activities and fail-closed access-policy checks for personal IP milestones, settlement development, guild ownership and Playerbots. This is **domain logic**, not live quest registrations or reward processing. See [IP_ACTIVITY_RULES.md](docs/IP_ACTIVITY_RULES.md) for the concrete gating rules and the requirement for a version-matched, read-only IP adapter.

`bash tests/run-catalog-tests.sh` tests the racial catalogue **and** activity eligibility without installing or changing anything in AzerothCore.

## New groundwork: construction milestones and contribution safety

The standalone `src/StrongholdConstruction.*` catalogue includes twelve **proposed** Human/Orc construction projects, four visual construction states and a non-mutating contribution validator with receipt/version protections. This is not an in-game building mechanic. See [CONSTRUCTION_ENGINE.md](docs/CONSTRUCTION_ENGINE.md).

The **draft, unapplied** characters SQL now includes a project-balance table and a unique receipt key to plan for idempotency. Database writes, item escrow, race-safe transactions, world spawning, uninstall/reinstall and compatibility testing are future mandatory milestones.

## Staging MariaDB persistence contract

The project now tests real InnoDB transaction behavior on a **temporary, throwaway MariaDB database in GitHub Actions**. The runner simulates *virtual Guild Supplies* (not player inventory), checks same-guild concurrency, unique receipts, rollback after injected exceptions, guild separation and reopening the connection after commit. See [PERSISTENCE_CONTRACT.md](docs/PERSISTENCE_CONTRACT.md).

This is a **test-only Python reference implementation**; AzerothCore C++ persistence and inventory integrations are not yet written. It does not make Strongholds playable.

The MariaDB contract now also checks same-schema reinstall preservation and that the **explicit optional purge** leaves a simulated unrelated module table untouched (again only in the throwaway CI test database). This does **not** exercise a live uninstall.

## Diagnostic-only AzerothCore bootstrap

`src/StrongholdBootstrap.cpp` registers a **passive WorldScript** (startup/config logging only). It reads `NaxxGuildStrongholds.Enabled` but **cannot activate housing**, even if an administrator sets it to 1: all four necessary development capabilities (privacy, persistence, safe exit and compatibility) remain explicitly unverified. No character, player, NPC, quest, map, phase, or database hooks are registered.

A mock-core compilation test checks registration and startup behavior; this is **not** proof of compatibility with the user's installed AzerothCore/Playerbots/IP revision. See [RUNTIME_BOOTSTRAP.md](docs/RUNTIME_BOOTSTRAP.md).
