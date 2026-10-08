# Delivery roadmap (provisional)

These are target milestones, not released functionality.

| Version | Scope | Release gate |
| --- | --- | --- |
| 0.1.0 | Safe module skeleton, config, schema proposal, documentation | Review source/module compatibility before building |
| 0.2.0 | Single reserved-location prototype, ownership, entry/exit, secure guild isolation | Two guilds do not see each other's players or objects |
| 0.3.0 | Building plots, construction, persistence and resource transactions | Restart + uninstall/reinstall tests |
| 0.4.0 | Guild daily tasks and weekly projects | Quest resets and abuse prevention verified |
| 0.5.0 | Read-only Individual Progression integration | Correct per-character quest/service gates |
| 0.6.0 | Raid trophy unlocks and event ledger | Boss attribution and no duplicate rewards |
| 0.7.0 | Ten racial style layouts and housing assets | Model/terrain collision and accessibility review |
| 0.8.0 | Controlled Playerbot activity and guild visitors | Permissions, exploits and bot caps tested |
| 0.9.0 | Relocation, seasonal decoration and polish | No lost owned decor or progress |
| 1.0.0 | Stability, backups, uninstall and reinstall hardening | Production acceptance checklist completed |

Each milestone requires a clean revert path, schema upgrade plan, test checklist and changelog entry. Do not bundle experimental core patches into a normal release.

## Compatibility gate for every milestone

Before each playable milestone is accepted: confirm the full existing module inventory and run the relevant tests in [COMPATIBILITY.md](COMPATIBILITY.md). Additions to module-related hooks, phasing, SQL gameobject IDs, IP, Playerbots or dungeon-completion logic require focused regression tests. A clean build by itself does not prove runtime compatibility.

## Progress in the development branch

- v0.1.0 foundations: inert loader, draft SQL, configuration and safety/compatibility documents.
- v0.2.0 settlement catalogue (non-playable substep): ten defined race themes, Human/Orc plot definitions and pure access policy tests.
- **Still outstanding for v0.2.0 proper:** evaluated private-area mechanism, approved map/asset coordinates, server-side NPC and teleport implementation, isolation tests with the *actual* IP and Playerbots configuration.


## Subsequent research checkpoint

Upstream MapMgr and MapInstanced source indicates that standard dungeon/raid instance routing is based on player/group instance saves, **not guild ownership**. No production-safe guild private-space mechanism has been selected yet.

Per-character activity gate code is now available for testing in `src/StrongholdActivities.*`. Runtime compatibility and IP adapter are still outstanding.

## v0.3.0 prototype substep — construction domain model

Pure C++ `StrongholdConstruction.*` now defines twelve Human/Orc building projects and four visual-stage transitions. This is an **early design substep**, not the completion of v0.3.0. The actual persistence adapter, resource debit, unique receipt transaction, objects, privacy and rollback testing are still outstanding.

See [CONSTRUCTION_ENGINE.md](CONSTRUCTION_ENGINE.md) for strict transaction/rollback requirements.

## MariaDB persistence contract experiment

A separate CI job now applies the draft schema to a throwaway MariaDB database and tests supply-only InnoDB transaction semantics with receipt uniqueness, rollback and concurrent deposits. See [PERSISTENCE_CONTRACT.md](PERSISTENCE_CONTRACT.md). It is **not** an AzerothCore runtime adapter or live migration. Item/timber/iron consumption stays prohibited until transaction/escrow design and deployed-core testing.

## Diagnostic bootstrap checkpoint

The loader now registers a passive **startup/config logging** WorldScript only; no gameplay can be activated in the current development build. Mock-core compilation and gate tests are available; a full build against the actual AzerothCore fork and module stack remains a release blocker.

## Property ownership domain milestone

The pure C++ property selection policy and MariaDB claim transaction **test-only contract** now exist. The underlying `naxx_gs_settlement.guild_id` primary key prevents multiple simultaneous guild property claims in the isolated CI database. This does not imply that guild member/faction auth, game location isolation or actual purchase menus have been integrated into worldserver. See [PROPERTY_CLAIMS.md](PROPERTY_CLAIMS.md).


## Ownership recovery checkpoint

Property lifecycle logic now addresses guild leadership changes, disband, stale guild IDs, archiving and possible restoration. This is a source-only / temporary MariaDB contract, not operational on the live server. In-game GuildScript registration, real membership verification, safe evacuation, migration and staging compatibility remain blockers. See [GUILD_LIFECYCLE.md](GUILD_LIFECYCLE.md).


## v0.2.x Safe Return — development progress

`StrongholdVisitRecovery.*` and the disposable `naxx_gs_visit` database test now model a three-step entry/return process and recovery after a failed teleport or disband. **This is groundwork, not a completed v0.2.0.** Actual guild-private map/phase routing remains the release blocker. Read [SAFE_RETURN_CONTRACT.md](SAFE_RETURN_CONTRACT.md) and [ISOLATION_DECISION.md](ISOLATION_DECISION.md) before adding teleport scripts.

## Isolation feasibility progress — source audit & model tests

The development branch now contains a read-only source inventory scanner and C++ tests modelling both phase-comparison modes. In particular, two players can have **asymmetric** visibility if one uses combined phase bits and the other exact matching. These tests help specify what a staging experiment must catch; they cannot prove the chosen isolation mechanism works on the deployed server.

Next gate is collecting the actual module fork versions, selecting a reserved private-area mechanism and completing a **real two-guild** staged visibility/teleport test. No production approach has been approved.

## v0.2.x — optional in-game observational diagnostic code

An opt-in, disabled-by-default `.naxxgs snapshot` command now exists in source solely to collect future **staging** measurements of an administrator character's current map/instance/phase/guild. It needs both a compile flag and a runtime config flag; no housing subsystem is enabled by it. Mock headers tests compile it, but the deployed AzerothCore fork and RBAC permissions have **not** been tested. See [STAGING_DIAGNOSTICS.md](STAGING_DIAGNOSTICS.md).

Next step remains a real, owner-approved two-guild isolation feasibility spike. The probe and offline code tests cannot prove map isolation.


## User-fork IP contract milestone

Reviewed the actual public `Grimfeather/mod-individual-progression` fork, not merely the upstream parent. The fork's `progressionLimit` means quest-state rank cannot alone determine guild activity eligibility. Added C++ effective-stage policy, integration with the existing pure activity gate, negative tests and pinned-fork source-contract CI checks. **No runtime AzerothCore/Individual Progression adapter exists or has been approved yet.** See [IP_FORK_COMPATIBILITY.md](IP_FORK_COMPATIBILITY.md).


## Phase-slot allocator feasibility — test-only foundation

Implemented a source-only lease allocator and disposable MariaDB reservation contract for a *hypothetically approved* dedicated-zone pool of phase bits. The C++ model rejects collisions and capacity exhaustion, and the SQL unique constraints protect against concurrent assignments. **This does not yet satisfy v0.2.0**: an actual phase pool must be reserved and proven unclaimed on the real server; mixed IP/Playerbots visibility, in-zone spawns, safe returns, scale limits and staged removal remain unknown.

Review [PHASE_LEASE_FEASIBILITY.md](PHASE_LEASE_FEASIBILITY.md) before considering any live phase assignment.

## Candidate A — Guild-private core instance routing feasibility

We now have a test-only C++ instance route preflight and server-arrival comparison. **The current upstream MapInstanced source does not offer a demonstrated guild-owned destination hook**: player/group binding controls dungeon and raid instance selection. This remains a major blocker, not a ready alternative to the finite phase leases. Candidate A requires a compatibility-reviewed core-supported route plus persistent custom instance save semantics without modifying existing raid lockouts. See [INSTANCE_ROUTING_FEASIBILITY.md](INSTANCE_ROUTING_FEASIBILITY.md).

## Upstream module loader integration milestone

The repository now exports both folder-case registration symbols and automatically checks them against upstream AzerothCore's **real** module loader CMake generator and WorldScript/CommandScript interface declarations (pinned revision). Tests under `tests/` are outside the source tree collected by the actual upstream module build, avoiding accidental compilation of fake-core tests into the worldserver.

Next non-negotiable validation: compile against the **user's actual deployed AzerothCore/Playerbots/IP fork** with all modules in a separate staging environment. This upstream-source check **does not** permit live installation or housing gameplay; privacy remains unverified. See [CORE_BUILD_CONTRACT.md](CORE_BUILD_CONTRACT.md).


## Integration progress: actual upstream module build target

Added a separate, opt-in-capable GitHub workflow that attempts to compile the **actual upstream** AzerothCore `modules` target with this development code using Clang, CMake and Ninja. This closes a gap between prior fake-header smoke tests and a genuine build of C++ code against actual upstream core headers.

The job does not build/link the whole user's customised worldserver, start any realm, interact with databases or certify coexistence with deployed modules. A successful run would improve *upstream source compatibility* only. A full staging build on the **user's version-matched complete module stack** and two-guild privacy validation remain release blockers.


## Follow-on upstream build gate

The initial passive upstream module compilation passed. The next automated gate now attempts a **complete public-upstream worldserver compile and link** with the disabled module, plus a second **diagnostics-enabled** module compile checking real upstream command interfaces. These are isolated CI-only jobs and intentionally exclude user's deployed modules and real databases. Neither result can certify private guild routing or game features.


## First gameplay-facing staging interface — Guild Steward preview

The first actual AzerothCore `CreatureScript` now exists: an explicitly opt-in read-only Guild Steward information menu with GM/guild requirements. It is not spawned, does not bind to any live creature template and cannot perform housing operations. The upstream CI checks that the opt-in script compiles and is absent from normal builds. This prepares real gossip integration while protecting players and existing modules.

Next step is **full upstream/staging build validation**, then decide a safe guild-private location and real server-side persistence; only after a complete staged compatibility review should an approved dedicated Guild Steward NPC template/SQL be drafted. See [STAGING_STEWARD.md](STAGING_STEWARD.md).

## Staging Guild Steward menu expansion

The actual opt-in `CreatureScript` preview now reuses existing C++ source catalogues rather than maintaining disconnected placeholder text. Its read-only gossip pages expose prospective architecture, Human/Orc building costs and minimum guild/IP activity thresholds. Everything is shown as planned and locked; **no player/guild status is loaded and no gameplay action exists**. The separate C++ content tests and real-header opt-in compile provide a foundation for eventual version-verified data-backed NPC interactions.


## Steward proof-composition milestone

The development-only Steward now has a structured, fail-closed guild/IP evidence policy and read-only gossip page. Unlike displaying generic planned thresholds, it can independently detect lifecycle-generation mismatch, archived property and high rewarded-IP-rank with a lower global server cap in tests. The **actual in-game page intentionally shows these as UNVERIFIED**, not unlocked; production-readonly property/IP adapters and real private-space routing remain missing. See [STEWARD_EVIDENCE_CONTRACT.md](STEWARD_EVIDENCE_CONTRACT.md).


## First actual AzerothCore guild identity seam

The staging Steward can now query a current player, registry guild object, member GUID and original guild creation timestamp with actual AzerothCore APIs. Previously the preview only knew the player's numeric guild ID. This closes a concrete ownership-spoofing gap in our read-only UI and supplies a verified `GuildIdentity` to existing lifecycle policies, without claiming property ownership.

Remaining: version-matched deployed core, authoritative **property DB adapter**, Individual Progression runtime state and actual private-world isolation. All services remain locked. See [STAGING_GUILD_IDENTITY.md](STAGING_GUILD_IDENTITY.md).


## Read-only settlement ownership lookup milestone

Following real guild-member and original-generation verification, the Steward now has a second **independent opt-in staging-only** data seam for reading the draft module-owned `naxx_gs_settlement` row. It uses the actual AzerothCore characters database **SELECT** API but cannot run in a default build, and is configured off even for a staging-compiled build. Missing/untrusted rows, generation reuse and malformed lifecycle data fail closed; archived property remains archived.

No real table is installed, no actual worldserver database was touched, and no ownership changes are supported. A successful SELECT is evidence that a module-owned row exists; it is **not** isolation proof, IP progress permission, or a claim to playable housing. Next stages require correct installed-fork IP and actual per-guild isolated destination, then real entry/exit recovery. [STAGING_PROPERTY_READ.md](STAGING_PROPERTY_READ.md).
