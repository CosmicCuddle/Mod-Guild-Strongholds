# Naxxramas Guild Strongholds

A planned AzerothCore 3.3.5 module for private, customisable guild settlements.

**Status:** development-only, **not gameplay-ready**. No housing, phasing, quests, NPCs or object spawning are implemented. The C++ loader registers only passive diagnostics; actual housing remains hard-blocked even if the master enable switch is requested.

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

- `src/loader.cpp`: passive diagnostic-only entry; supports both common module-folder spellings.
- `conf/mod_naxx_guild_strongholds.conf.dist`: disabled-by-default settings.
- `data/sql/manual/install_characters.sql`: optional, **manual** isolated schema creation; do not execute yet.
- `INSTALL.md`, `UNINSTALL.md`, `ROLLBACK.md`: installation and recovery policy.
- `uninstall/purge_characters.sql`: explicitly destructive module-data purge, never automatic.
- `backup/README.md`: backup checklist.
- `CHANGELOG.md`: version history.

**There is no reason to git-pull or recompile this on the live server yet.**

## Compatibility still to verify

Upstream AzerothCore's CMake loader convention is now checked automatically at a pinned public revision. The **deployed** core loader/build, Individual Progression and Playerbots forks still need a real staging build before gameplay is enabled.

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

## Guild property selection prototype — separate from world maps

`src/StrongholdProperty.*` now implements safe, pure C++ rules for one property per guild, server-verified ownership, guildmaster-only claiming and faction-appropriate theme selection. Until in-world isolation is solved, only the initial Human and Orc logical layouts are eligible; all other themes await assets and privacy tests.

GitHub Actions also tests the **existing** `naxx_gs_settlement` primary-key ownership constraint on a disposable MariaDB: two simultaneous claims cannot produce two properties, a simulated SQL failure rolls back the claim and ledger, and an identical-schema reinstall preserves the saved property. See [PROPERTY_CLAIMS.md](docs/PROPERTY_CLAIMS.md).

These are **not** real character property purchases or in-game housing.


## New guild lifecycle safety foundation

The draft settlement owner now uses a verified guild creation date alongside its numeric ID, so a reused ID must not inherit an earlier guild's property. Disbanding is designed to **archive** property while retaining buildings, supplies, trophies and contribution history. Recovery is administrator-controlled and allowed only for the original guild generation. Standalone C++ policy and isolated MariaDB contract tests cover failure rollback and denial of access after archival. **No live guild event hooks are registered.** See [GUILD_LIFECYCLE.md](docs/GUILD_LIFECYCLE.md).


## Unified property visit check

A new `StrongholdVisitGate` combines master enable, proven isolation, server-verified membership, original guild generation and active property status into a single fail-closed decision. This makes it harder for a future entry NPC to accidentally check only the numeric guild ID. It is a **policy helper, not actual guild privacy or a teleport**.


## Durable safe-return tickets — development prototype

A new `StrongholdVisitRecovery` C++ policy layer requires a **server-captured**, structurally valid origin point and a unique outstanding visit record before a hypothetical guild-property entry teleport. Three ticket states — `prepared`, `inside`, and `returning` — support crash recovery and emergency return even if the guild disbands or housing is disabled. This is a **domain model**, not live teleport handling.

The draft character SQL now includes a module-owned `naxx_gs_visit` table. GitHub Actions tests durable records and rollback in an isolated MariaDB DB; no map/phase modifications exist. See [SAFE_RETURN_CONTRACT.md](docs/SAFE_RETURN_CONTRACT.md) and [ISOLATION_DECISION.md](docs/ISOLATION_DECISION.md).

## Phase/instance isolation diagnostic — non-playable research

A new **bidirectional** phase visibility model reflects the upstream AzerothCore `WorldObject::InSamePhase` comparison: normal bitmask intersection versus an explicit exact-phase-value mode. This model demonstrates dangerous one-way visibility when modules disagree about phase mode, phase-mask collisions and the limits of a 32-bit phase mask. It does **not** set phases, select maps or prove in-game housing isolation.

A read-only source audit helper `scripts/audit-isolation-compatibility.py` can later examine the server's *actual* local AzerothCore source checkout and module folders for relevant phase, teleport and instance APIs. It produces **REVIEW_REQUIRED**, never a compatibility certificate. No command needs to be run yet. See [PHASE_COMPATIBILITY_AUDIT.md](docs/PHASE_COMPATIBILITY_AUDIT.md).

## Optional, two-key staging diagnostic (not available in normal builds)

The source now contains a **read-only** administrator command, `.naxxgs snapshot`, compiled and registered **only when** `NAXX_GS_BUILD_STAGING_DIAGNOSTICS` is explicitly set, and answering only when `NaxxGuildStrongholds.Diagnostics.Enabled=1`. The command requires a GM RBAC permission and reports **only the issuing player's own** map, instance, zone, area, guild ID and phase-mask values. It never changes phase, moves a player, spawns objects or reads SQL. This provides future staging evidence without pretending to prove privacy.

Normal builds do not register the command, and the housing startup guard remains blocked. The command is only mock-compiled so far—not built or exercised on the user's deployed core. See [STAGING_DIAGNOSTICS.md](docs/STAGING_DIAGNOSTICS.md). **There is nothing to install on the live server.**


## Actual Individual Progression fork — reviewed source contract

We inspected the user's public [Grimfeather Individual Progression fork](https://github.com/Grimfeather/mod-individual-progression) at commit `706740808fee328b8557607f87b0548cf961e047`. The fork derives raw character stage from rewarded hidden quests, **but separately restricts access using `enabled` and `progressionLimit`**.

A standalone `StrongholdIpCompatibility.*` policy now computes the capped effective stage and refuses to use stale/unverified stage evidence. An automated read-only source-contract checker tests the pinned public fork in CI and reports mismatches instead of assuming upstream APIs are unchanged. This is not compiled against or installed on the live server. Read [IP_FORK_COMPATIBILITY.md](docs/IP_FORK_COMPATIBILITY.md).


## Staging phase-slot reservations — NOT live phasing

The new `StrongholdPhaseLease.*` and `tests/mysql_phase_lease_test.py` implement a **prototype reservation ledger** for a candidate dedicated-housing-zone phase pool. An administrator must first verify the installed modules, combined-mode phase behavior and **explicitly approve** which phase bits are safe. Until then, no phase slot may be offered.

Tests ensure a phase bit is unique per guild, reused guild IDs cannot acquire old allocations, retired bits remain reserved, simultaneous requests cannot claim the same bit, and insufficient capacity returns **no capacity** instead of sharing space. The draft characters SQL includes `naxx_gs_isolation_slot`, which is not auto-installed. **No player's phase is changed; no housing instance, teleport, creature or gameobject is created.**

This prototype does not prove that finite phase bits can support the desired scale or coexist with Individual Progression and every installed module. True map instances remain an alternative. See [PHASE_LEASE_FEASIBILITY.md](docs/PHASE_LEASE_FEASIBILITY.md).

## Candidate A: guild-owned map instances (feasibility preflight)

The project now has a **test-only** core-instance route preflight in `src/StrongholdInstanceRoute.*`. It rejects unverified map/client assets, mismatched player/group instance bindings, unsafe reset lifetime, lack of deterministic per-guild routing and missing two-guild privacy evidence. Existing and even retired instance IDs may not silently be given to another guild. The code **does not call** AzerothCore `GenerateInstanceId`, `CreateInstance` or `TeleportTo` and does not store or assign any real instance ID.

Upstream AzerothCore's current dungeon instance routing uses player/group instance saves, not a verified guild owner mapping. True private instances remain a **candidate requiring a custom integration and staged proof**, not an implemented housing option. No new database tables were added in this batch. Read [INSTANCE_ROUTING_FEASIBILITY.md](docs/INSTANCE_ROUTING_FEASIBILITY.md).

## Upstream build-loader integration — now checked in CI

We found and fixed a genuine integration gap: AzerothCore derives the module C++ registration symbol **from the folder name, preserving case**. Both `modules/mod-guild-strongholds` (preferred) and `modules/Mod-Guild-Strongholds` (the GitHub repo's name) now have compatible loader symbols, and the two variants are mock-linked and smoke-tested.

A separate GitHub Actions job checks pinned **real upstream** AzerothCore CMake module-source collection, generated registration symbols and WorldScript/CommandScript interfaces. It confirms AzerothCore collects sources from this module's **`src/` directory only**, leaving standalone `tests/` outside the worldserver build. No extra `CMakeLists.txt` is required in this module for that upstream default discovery.

**This is not a full AzerothCore build** and does not establish compatibility with the user's customized core/Playerbots. No production code or database was installed. Read [CORE_BUILD_CONTRACT.md](docs/CORE_BUILD_CONTRACT.md).


## Real upstream C++ module compile check (isolated CI)

GitHub Actions now has a **separate, isolated module compilation workflow** at `.github/workflows/upstream-compile.yml`. It checks out the *pinned public* AzerothCore upstream source and our **draft branch** into temporary GitHub runner directories, copies only the module's `src/` and `conf/` under a lowercase module folder, configures upstream CMake, and builds the genuine `modules` C++ target using real game headers. It also checks that both expected loader entry points are in the compiled archive.

The job does **not** run `worldserver`, connect to MySQL, install to a server, apply SQL, or enable any gameplay. It is not a complete `worldserver` link or a compile against your customised AzerothCore/Playerbots/IP fork. Do not treat even a successful upstream compilation as permission to deploy housing. See [CORE_BUILD_CONTRACT.md](docs/CORE_BUILD_CONTRACT.md).


## Expanded upstream integration: full link + opt-in diagnostics

The isolated GitHub Actions build now checks **two independent compiler configurations** against the immutable public upstream AzerothCore source:

1. **Normal game configuration:** build the real `modules` C++ target, check both case-sensitive loader symbols, verify the staging GM command is absent and attempt to compile **and link the full `worldserver` executable**. No realm is started.
2. **Staging diagnostics opt-in:** explicitly enable `NAXX_GS_BUILD_STAGING_DIAGNOSTICS` in the temporary CI compiler flags and build the real upstream module target to check that the GM command compiles against actual `ChatHandler`, `Player`, `CommandScript` and RBAC headers. Does not link or run a realm.

Both paths use a disposable GitHub Actions runner with no real player data, SQL updates or user server access. **Neither verifies the user's custom deployed AzerothCore/Grimfeather IP/Playerbots stack**, nor private housing functionality.

This expands the workflow after the initial module-target compilation succeeded. See [CORE_BUILD_CONTRACT.md](docs/CORE_BUILD_CONTRACT.md).


## First AzerothCore Guild Steward — optional staging-only preview

The module now contains `src/StrongholdStagingSteward.cpp`, a real AzerothCore `CreatureScript` implementing **read-only gossip menus** for the proposed Guild Steward. This moves beyond conceptual menu mock-ups, but **it does not spawn an NPC**.

A normal build omits the script. On an approved *separate staging* build it requires BOTH `NAXX_GS_BUILD_STAGING_STEWARD` and `NaxxGuildStrongholds.StagingSteward.Enabled=1`. Only a staff character in GM mode who belongs to a guild can view it. It rechecks permissions on every gossip selection and accepts only overview, future-building information, and close. It explicitly says housing is unavailable.

No creature template ID, spawn, world SQL, guild property purchase, teleport, inventory debit, quest progress, decoration or phase operation is included. A staging admin would have to supply a separately approved, backed-up test NPC template and script binding after compatibility testing. See [STAGING_STEWARD.md](docs/STAGING_STEWARD.md).

## Catalogue-driven Guild Steward preview (development only)

The staging-only Guild Steward now has browsable pages for **all ten race-theme plans**, **twelve Human/Orc building projects**, **daily/weekly IP-milestone activity plans**, and **two planned raid trophy requests**. Every preview row is built from the project's actual C++ catalogues, not duplicated string placeholders.

All rows say `[PLAN]` or `[LOCKED]`, include only planned minimum settlement levels, suggested resource costs and IP milestone numbers, and never imply that the guild owns or can use anything. A menu click can only change the read-only gossip page, go back or close. Separate C++ tests verify catalogue linkage, page sizes and no accidental unlocked/reward descriptions.

No character IP rank, guild settlement level, claim status, completion receipt or trophy ownership is read from the server yet. Production builds omit the preview CreatureScript; no NPC has been spawned or bound. See [STAGING_STEWARD.md](docs/STAGING_STEWARD.md).


## Read-only Guild Steward evidence review (staging-only)

The Guild Steward now has a **Guild and IP evidence [LOCKED]** page, driven by `StrongholdStewardEvidence.*`. That policy composes existing guild-generation/lifecycle checks with the reviewed Grimfeather IP effective-stage cap and the activity catalogue. It explicitly distinguishes missing proof, another guild's property, recycled guild IDs, archived property, missing settlement level, unknown/disabled IP and hypothetical planning eligibility.

**Real staging gossip reads only the current player's guild ID via AzerothCore.** We have deliberately **not** fabricated a guild creation timestamp, property SQL record, settlement level, Individual Progression stage or verified private map. The live-preview row explicitly says those fields cannot yet be verified. Every row says `[LOCKED]`, and `HousingAvailable` remains false even if all inputs are true in a synthetic test.

This work provides a safe, composable seam for future **read-only** deployed-fork adapters but does not activate any quests, awards, buildings or NPC spawns. See [STEWARD_EVIDENCE_CONTRACT.md](docs/STEWARD_EVIDENCE_CONTRACT.md).


## First actual, read-only core guild-identity adapter (staging)

The optional Guild Steward now reads **actual AzerothCore guild registry state** when compiled for staging. Instead of trusting `Player::GetGuildId()` alone, `StrongholdStagingGuildAdapter.cpp` checks `Player::GetGuild()`, a matching `Guild::GetId()`, `Guild::GetMember(player->GetGUID())`, and the **original** `Guild::GetCreatedDate()` timestamp. The preview closes if the player is no longer a verified guild member or the guild generation is unavailable. Every menu interaction checks again.

The Guild/IP Evidence page can now honestly show the **verified member guild ID and original creation timestamp**. It still marks **property ownership, settlement level, Individual Progression stage and all housing gameplay LOCKED**: none of those server adapters exist. The new domain test suite rejects stale/recycled guild ID data, lost membership and missing creation timestamps; a source-only safety test rejects game/database mutator APIs.

**Staging-only:** direct guild API adapter is compiled only under `NAXX_GS_BUILD_STAGING_STEWARD`. Normal builds contain neither staging gossip nor guild API glue. Real upstream compilation will verify signatures; the deployed custom fork still needs a separate build. See [STAGING_GUILD_IDENTITY.md](docs/STAGING_GUILD_IDENTITY.md).


## Safe future staging module inventory — local changes matter

`scripts/check-module-inventory.sh` has been strengthened to report the core Git SHA, each independently versioned module SHA, and whether **tracked source files are locally modified**. It also labels modules without their own Git checkout as `NO_INDEPENDENT_GIT` rather than guessing a revision. It never prints configuration data, local diff contents, Git remote credentials or source paths of modified files.

This is a **read-only aid for a future separate staging checkout**, not something you need to run on your live server now. A clean Git SHA is not enough to prove full compatibility, because untracked source files, custom binaries and installed config may still differ. CI uses synthetic local Git repositories to check dirty-state reporting and refusal of invalid source directories. See [STAGING_SOURCE_INVENTORY.md](docs/STAGING_SOURCE_INVENTORY.md).


## Staging-only property SELECT (extra opt-in; **NOT live housing**)

A narrow **read-only CharacterDatabase adapter** now exists at `StrongholdStagingPropertyAdapter.cpp`, but it is compiled **only** when `NAXX_GS_BUILD_STAGING_STEWARD` **and** `NAXX_GS_BUILD_STAGING_PROPERTY_READ` are explicitly set. It also requires a **separate disabled-by-default** setting `NaxxGuildStrongholds.StagingPropertyRead.Enabled=1` on an approved, backed-up staging realm that already contains the *draft* `naxx_gs_settlement` table. Normal production builds have **no property DB query**.

On a Guild Steward evidence click, it first requires **actual AzerothCore guild membership and original creation time**. It then SELECTs only `guild_id, guild_created_at, lifecycle_state, lifecycle_version, development_level` for that guild. Missing SQL/table returns no proof, invalid rows fail closed and a recycled guild ID with a different creation timestamp is explicitly denied. Valid active or archived rows are reported as **read-only property-record evidence**, never private housing access.

The SQL adapter never creates, updates or drops any table, never installs draft SQL or obtains Individual Progression status. **No staging or live database was queried in this development change**; only disposable GitHub MariaDB fixtures test the SELECT. See [STAGING_PROPERTY_READ.md](docs/STAGING_PROPERTY_READ.md).
