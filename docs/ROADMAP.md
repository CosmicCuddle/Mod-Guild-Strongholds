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


## Direct runtime IP state source seam

The optional staging Guild Steward now has a guarded integration with the reviewed Grimfeather source, reading IP's **actual** enabled flag, quest-progression API and global limit; existing activity policies consume the capped result. The new public upstream build job checks out the *exact pinned* reviewed fork source to validate the integration together, separate from normal upstream-only builds. This is still a **non-operational housing** review page and not installed on the custom server.

The next outstanding blockers remain source auditing of the actual deployed fork, real protected private guild isolation, durable entrance/exit, replay-safe construction/raid rewards and an explicit rollback plan.

## Guild raid trophy gameplay design milestone

The source now provides five specifically named **raid trophy concepts** with strict no-side-effect unlock/placement proposal policies. A verified guild raid requires authoritative server kill/encounter proof, unique receipt, original guild generation, an active stronghold and separately verified real human guild participants. Mixed guilds can independently qualify; Playerbots do not count as qualifying human guild players; any proposal is still not a reward.

The trophy records cannot be connected to the current draft `naxx_gs_unlock` table without schema hardening for **original guild generation and idempotent encounter receipts**. The future kill hook must be audited against the exact deployed AzerothCore/Playerbots modules, the boss/map IDs verified for *classic* versus 3.3.5 Onyxia variants, and a transaction adapter must persist unlocks with audit receipts. No loot or gameobject IDs have been allocated. See [RAID_TROPHIES.md](RAID_TROPHIES.md).


## Raid trophy database milestone: transactional receipts (CI-only)

Added generation-qualified trophy receipt and trophy unlock **draft** tables without replacing any existing general unlock records. In a disposable MariaDB test database, a simulated validated server kill is recorded under an InnoDB settlement row lock with a unique receipt, one-time trophy unlock and audit entry in one transaction. Rollback, concurrency, mixed-guild raids, guild ID reuse, archival and schema-reapplication tests prevent duplicate or partially committed history.

A production C++ event listener, authentic boss/map/Playerbots checks and an approved migration path **do not exist**. Next: decide guild raid participation requirements with the owner; inspect the installed AzerothCore kill/group hooks and actual Classic raid IDs; design read-only earned-trophy display and vetted 3.3.5 GO models, only after the real guild-private destination is safe. See [RAID_TROPHY_STORAGE.md](RAID_TROPHY_STORAGE.md).


## Actual AzerothCore death signal milestone (not yet boss achievement credit)

A staging-only `UnitScript` now registers the **real upstream** `UNITHOOK_ON_UNIT_DEATH` and can observe when a `Creature::IsDungeonBoss()` dies on `Map::IsRaid()` with a nonzero instance identity. The module logs metadata only and never calls the trophy unlock/persistence proposal. This proves a possible safe event-integration seam without guessing which map/boss difficulty corresponds to original Classic Onyxia, Naxxramas or Wrath variants.

**Still required before any trophy credit:** source-verified real boss/raid difficulty allowlist on the owner's custom server; verified instance completion; encounter-participation records not just being in the map; exact member GUID/guild generation and a trustworthy Playerbots classification. A killing blow, map presence, loot tag or NPC flag is NOT enough to award anything. See [STAGING_RAID_OBSERVER.md](STAGING_RAID_OBSERVER.md).


## Stronghold raid participation evidence seam

Added a real `PlayerScript::OnPlayerCreatureKillCredit` staging observer as a second candidate-only source of raid boss signals. Domain tests now require positive Playerbots-aware human classification, unique GUID, guild creation generation, trusted raid roster, same-instance proof and independently verified contribution to the same boss encounter before even returning a hypothetical qualifying result.

Public `mod-playerbots` source revealed a critical semantic trap: `GetPlayerbotAI()==nullptr` is ambiguous if the module is disabled. The new pinned-fork CI audit confirms this; a real human check must be specifically designed against **the owner's installed Playerbots fork**, not guessed from that pointer.

No actual encounter-specific participation adapter or reward hook has been added; all server logs remain candidate-only and all housing/trophy gameplay blocked. See [RAID_PARTICIPATION.md](RAID_PARTICIPATION.md).


## Staging bot recognition without false human credit

An opt-in Playerbots source adapter has been added to the anonymous kill-credit observer. It uses the **real public Playerbots bot-AI lookup** only to make the *positive* statement "this character is a bot" when a registered AI exists. All negative lookups, inactive/unreviewed source or missing observations remain unknown and **never** establish human guild eligibility. This closes the dangerous shortcut of treating `nullptr` or normal-looking sessions as human.

Next blockers: source-audit the user's actual deployed Playerbots fork, implement a **positively verified human** controller classification, real per-encounter contribution provenance and safe guild roster/instance snapshot, all still entirely blocked from gameplay/trophies. Source compilation and synthetic tests cannot replace full staging/live-fork integration.

## Encounter contribution evidence prototype — still offline

A pure C++ attempt-bound event policy now rejects foreign instance/actor evidence, duplicate/unverified events, passive kill credit, spectator-only attendance and out-of-window actions. Effective damage, healing and mitigation can generate **review candidates only**, never positive human status, encounter participation proof or trophy rewards. The genuine custom-fork event source, authoritative encounter-completion adapter, 40-player mixed-guild behavior and Playerbots human identity audit remain blocking. See [ENCOUNTER_CONTRIBUTION.md](ENCOUNTER_CONTRIBUTION.md).

## Read-only deployed raid-source inventory preparation

The optional Python source preflight scans only bounded C++ API name occurrences in a separate source copy. No actual installed AzerothCore/Playerbots human classifier, boss completion, raid participation or guild routing is verified. See [RAID_SOURCE_PREFLIGHT.md](RAID_SOURCE_PREFLIGHT.md); gameplay and awards remain disabled.

## Offline event-time group and guild-history model

A bounded synthetic roster window evaluator now rejects previous-wipe event tokens, reused guild IDs, overlapping windows and action during join/leave gaps, including 40-player mixed guild simulations. All results remain review candidates with no actual verification or rewards. See [RAID_MEMBERSHIP_TIMELINE.md](RAID_MEMBERSHIP_TIMELINE.md).

## Bidirectional private housing scenario coverage model (unreleased)

Offline six-context, eight-surface A/B matrix with 132 synthetic observations, cross-guild interaction denial and own-guild controls. No actual visibility hook or gameplay changes. See [PRIVACY_STAGING_MATRIX.md](PRIVACY_STAGING_MATRIX.md).

## Preservation-first pre-uninstall drain verification (offline)

Added a fail-closed pure C++ model of a trusted complete DB ticket count plus independent housing-world sweep after blocking new visits, with the emergency handler still present. It cannot permit uninstall or SQL purge, and is not deployed. See [UNINSTALL_DRAIN.md](UNINSTALL_DRAIN.md).
