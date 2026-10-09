# Changelog

## 0.1.0-foundations (development branch; unreleased)

- Created documented hybrid Guild Strongholds design.
- Added ten racial architectural themes and guild-wide/personal-IP rules.
- Added roadmap, rollback and uninstall policies.
- Added a disabled-by-default configuration template and inert C++ registration stub.
- Drafted isolated character-database tables and explicit destructive cleanup SQL.
- **No live gameplay implementation or compatibility testing yet.**

## 0.2.0-settlement-catalog (design/prototype; unreleased)

- Registered ten faction-appropriate racial architectural themes in a pure C++ catalogue.
- Added six logical construction plots each for the initial Human and Orc prototypes.
- Added no-client-input faction selection and fail-closed guild entry policy helpers; these do **not** yet enforce live server isolation.
- Added standalone C++ policy/catalogue regression tests and a review-branch GitHub Actions workflow.
- Documented stronghold-area isolation decision, asset research gates and a concrete two-guild test plan.
- No new SQL, creature/object IDs, spawn points or gameplay hooks. The module remains disabled by default.


## 0.2.0-activity-policy (unreleased)

- Added a nine-activity design catalogue for daily, weekly, and one-time guild settlement projects.
- Implemented C++ policy checks for ownership, settlement level, personal IP milestone and bot restrictions.
- Added standalone C++ regression coverage and expanded GitHub Actions test execution.
- Inspected upstream AzerothCore instance routing and current Guild House zone-specific phasing; documented technical isolation risks.
- Still no AzerothCore runtime hooks, database updates, quest entry IDs, map coordinates or live-server changes.

## 0.3.0-construction-domain groundwork (unreleased, October 2026)

- Twelve preliminary building-construction project templates for Human/Orc plots.
- Deterministic 4-stage construction visual-state selection.
- Fail-closed material-contribution proposal validator: guild permissions, bot opt-out, project eligibility, receipt checks, amount bounds and optimistic version.
- Updated draft (unapplied) character SQL with project balances and uniqueness for per-guild contribution receipts; updated explicit purge list.
- C++ policy tests extended to construction and integration boundaries documented.
- **No housing instances, world spawn IDs, inventory deductions or live database changes.**

## 0.3.0-mariadb-contract groundwork (unreleased)

- Added staging-only MariaDB 10.11 InnoDB transaction contract test for virtual Guild Supplies contributions.
- Added locked guild/project update, unique receipt, optimistic version, ledger and failure-injection rollback reference behavior.
- Added concurrent same-receipt/different-receipt and cross-guild isolation tests.
- CI uses `naxx_gs_ci_test`; test runner refuses any other database name and requires an explicit CI environment guard.
- Clarified that WoW player inventory debit, real server-side guild validation, C++ database adapter, IP integration and private property instancing remain incomplete.

## Additional staging recovery validation

- Extended MariaDB transaction failpoint coverage to receipt and ledger insertions.
- Added draft schema reinstallation checks preserving data for two guilds.
- Added an explicit selective-purge test in the *isolated CI database* with an unrelated sentinel table.
- Reaffirmed that production migration and recovery remain untested until the deployed AzerothCore fork is available.

## 0.1.x — passive startup bootstrap (development only)

- Replaced inert loader with one diagnostic-only AzerothCore `WorldScript` listening to config load/startup.
- Added fail-closed capability evaluation covering private guild isolation, persistence, safe exit and compatibility.
- Explicit warning and no gameplay activation when `NaxxGuildStrongholds.Enabled=1` is requested prematurely.
- Added pure C++ startup gate tests and mock-core WorldScript compile/config-reload smoke checks.
- No housing activity, SQL, NPC, teleport, phase or character hooks enabled.

## 0.2.x — guild property claim groundwork (development only)

- Added pure C++ guild property claim validator with guildleader, race/faction, verified identity and duplicate ownership rules.
- Human/Orc logical properties claimable by their respective guild factions in the domain model; the other eight themes remain draft-only.
- Added disposable MariaDB property-claim transaction contract for simultaneous claims, rollback after failure, separate faction/guild owners and saved-property preservation after schema reapplication.
- Continued startup fail-closed restriction: absolutely no in-game housing, NPC, map, phase, teleport or quest changes.


## 0.3.x Guild lifecycle protection (development only)

- Added guild generation ID + creation-date checks and active/archived status to the draft schema.
- Added C++ policy for disband archiving, replay resistance, versioning, identity mismatches and restricted restoration.
- Updated disposable MariaDB property/contribution tests and added separate archival and recovery transaction checks.
- Preserves progress and achievements by design; no live AzerothCore guild hooks, teleport, or database changes.


- Added `StrongholdVisitGate`, combining the older guild-ID entry logic with mandatory membership, property lifetime/generation and archive checks; standalone C++ regression tests prevent stale guild access.


## v0.2.x — private property visit recovery groundwork (unreleased)

- Added pure C++ visit preparation, arrival verification, emergency return and safe completion policy.
- Required persisted per-character return ticket before any future teleport and denied overwriting outstanding return locations.
- Added three-state `prepared` / `inside` / `returning` lifecycle, retry and version checks, and character-bound visit ID validation.
- Created draft manual `naxx_gs_visit` table in characters database and updated explicit opt-in purge.
- Added C++ tests and disposable MariaDB contract for interrupted arrival, guild disband, return failure rollback, outstanding tickets, same-schema preservation and ticket deletion only after verified return.
- Researched upstream MapMgr/MapInstanced/Player teleport routing; no production-safe guild-private isolation strategy selected.
- **No playable housing, teleport, phase overwrite, character database migration or installed-module compatibility claim.**

## v0.2.x — phase collision and module audit groundwork (unreleased)

- Added pure C++ visibility model for AzerothCore's combined-bitmask and exact-phase-value comparison, including directional cross-guild checks.
- Added regression tests for phase reuse, global/all-phase masks, cross-guild visibility, asymmetric modes, missing creatures or gameobjects, and correct instance separation within synthetic samples.
- Added read-only Python compatibility auditor that inventories local module Git revisions and scans phase/teleport/instance hooks without reading configuration or editing source; deliberately never declares compatibility.
- Added synthetic-checkout tests and isolation acceptance documentation.
- Kept runtime `DevelopmentCapabilities.PrivacyIsolation` **false**; no WorldObject phase overrides, core patches, in-world teleports, NPCs or actual housing isolation introduced.

## v0.2.x — opt-in map/phase observation command (unreleased)

- Added `StrongholdStagingDiagnostics.cpp`, a **read-only, administrator-permission** map/phase information command explicitly excluded from normal builds.
- Requires BOTH `NAXX_GS_BUILD_STAGING_DIAGNOSTICS` compiler define and `NaxxGuildStrongholds.Diagnostics.Enabled=1` config; its default remains off.
- Exposes the issuing player's guild ID, map ID, instance ID, zone, area and phase mask as staging evidence only. Does not guess internal comparison mode or certify guild isolation.
- Added mock-core C++ tests for command tree/RBAC, disabled default, opt-in observations and no player changes.
- No real player movement, world phase hook, SQL write, housing spawn or deployed-worldserver build.


## v0.5.0 read-only IP source-contract groundwork (unreleased)

- Reviewed the actual public Grimfeather IP fork (pinned Git commit `706740808fee328b8557607f87b0548cf961e047`), including its quest-reward progression reader, `progressionLimit`, enabled flag and area phasing hooks.
- Added effective IP stage calculator respecting the fork's global progression cap, not merely the character's rewarded hidden quests.
- Added fail-closed C++ activity gate integration with stale-proof clearing, disabled/unverified source denial and invalid stage/limit validation.
- Added a read-only fork source-contract checker, synthetic negative tests and a pinned-revision CI checkout, without modifying the fork or server.
- No live Individual Progression linkage, guild NPCs, quest rewards or phased housing behavior enabled.


## v0.2.x — bounded phase-lease feasibility (unreleased)

- Added a conservative C++ reservation proposal for a manually preapproved candidate phase-bit pool, requiring full module inventory, uniform combined phase comparison and server-verified property identity.
- Rejects externally used bits, malformed historical leases, guild ID reuse, duplicate leases and capacity exhaustion; no guild-ID modulo collision workaround.
- **Retired phase bits remain reserved** until a separately audited evacuation/reclamation process is implemented; they are not silently recycled.
- Added draft module-owned `naxx_gs_isolation_slot` SQL with unique guild and phase keys, plus test-only InnoDB concurrency/retry/rollback and archival checks.
- Kept all startup gameplay flags disabled; no phase manipulation or privacy verification performed on a running realm.

## v0.2.x — native instance routing preflight (unreleased)

- Reviewed upstream 8 October 2026 AzerothCore `MapInstanced`, `MapMgr`, `InstanceSaveMgr`, `Player::TeleportTo` behaviour.
- Added strict **non-operational** guild-owned map-instance preflight, accepting only externally verified, core-managed instance ID and saved lifetime.
- Added checks for safe player/group bindings, non-raid map/client asset provenance, guild-private routing, world isolation and recovery proofs.
- Added regression tests rejecting shared instance IDs (even across map IDs), guild identity reuse, retired instance reuse, missing evidence and wrong arrival destination.
- No SQL migrations, gameplay registration, instance creation, phase writes or live teleportation implemented.

## Core loader/registration preflight — October 2026 (development only)

- Verified the *actual upstream* AzerothCore CMake module loader naming and `src/`-only source collection conventions.
- Fixed folder case mismatch by exporting both `Addmod_guild_strongholdsScripts()` and `AddMod_Guild_StrongholdsScripts()`; the uppercase form delegates to one passive registration path.
- Added C++ fake-core smoke compilation and execution through both case-sensitive loader symbols.
- Added a read-only, pinned-upstream source contract checker for generated CMake loader naming, module source glob and WorldScript/CommandScript interfaces; negative tests reject incompatible upstream changes.
- Added a GitHub Actions job checking genuine upstream source (rather than relying only on mock headers).
- Actual deployed-core compile, complete module inventory, private guild isolation and playable housing are **not yet verified or implemented**.


## 0.2.x — isolated upstream module compilation check (unreleased)

- Added a separate GitHub Actions job using a fixed public AzerothCore source revision, the real upstream CMake/Ninja build and compiler rather than mocked script interfaces alone.
- The CI job copies only Strongholds source/config into an ephemeral AzerothCore `modules/mod-guild-strongholds` folder, builds target `modules` with real core headers, and checks both folder-case registration symbols.
- No database, game server, SQL migration, world object, DBC or live user data accessed. This is a **static module target compile**, not a full worldserver link nor a deployed-fork compatibility test.
- Housing gameplay remains disabled, unimplemented and blocked by incomplete privacy isolation.


## Upstream full-link and staging-command API validation (development)

- Confirmed initial passive module compiled with real upstream AzerothCore headers and both folder-case registration exports.
- Expanded ephemeral GitHub build into two variants: normal full-worldserver compile/link, and separate diagnostics-enabled module compilation using real AzerothCore headers.
- Added binary-level checks to ensure the normal build **does not include** the staging GM command and the explicitly opted-in build does.
- Neither CI mode starts a realm, applies SQL or touches live data. Full deployed-fork compatibility and guild privacy remain unverified.

## Upstream staging-command compilation fix (development)

- Real-header CI caught a function-pointer construction rejected by upstream `ChatCommandBuilder` when compiling `.naxxgs snapshot` with `NAXX_GS_BUILD_STAGING_DIAGNOSTICS`.
- Changed the command to pass `HandleSnapshot` by function reference, matching upstream command syntax.
- Tightened fake-core test header to use a handler-reference constructor, preventing previous mock-only false passes.
- This correction does not register the GM command in normal builds or change housing safety gates.


## Guild Steward staging-preview script (development only)

- Added a real AzerothCore `CreatureScript` with a simple GM-only gossip menu: read-only guild preview, proposed guild buildings/trophies, and close.
- Compile-time `NAXX_GS_BUILD_STAGING_STEWARD` and separate disabled-by-default config protect production. The script is not bound to any NPC entry or spawned.
- Current staff/GM and guild membership are revalidated on every gossip click; forged menu senders/actions fail closed.
- Added independent pure C++ steward permission/action tests and real upstream CI opt-in compilation; normal build must exclude both staging registrations.
- No property purchase, building placement, currency change, quest reward, map phase or teleport.

## Guild Steward catalogue-driven gossip pages (development only)

- Replaced two static teaser lines with navigable planning-only pages sourced from the existing 10 race themes, Human/Orc building projects, 3 daily, 4 weekly and 2 one-time activity definitions.
- Added planned resource cost, settlement level and Individual Progression milestone labels without claiming a player's actual progress.
- Added `Back`/Close navigation and only self-looping detail rows; no claim, unlock, donation, quest, teleport or reward actions exist.
- Rechecks staff status, guild membership and config for every gossip selection, including navigation. Unknown/forged selections fail closed.
- Added C++ regression tests checking catalogue-derived labels, planned-only wording, page size limits, all defined menu actions and revoked permissions.
- Real NPC remains staging-flag-only, config off by default, and has no creature template or spawn.


## Guild Steward property/IP evidence composition (unreleased)

- Added `StrongholdStewardEvidence.*`, a standalone read-only snapshot evaluator combining original guild generation, current property lifecycle, settlement-level proof, Grimfeather IP enabled/cap, and existing per-activity eligibility.
- Rejects stale guild IDs, foreign property, archived property, missing identity/settlement data, disabled/unknown IP and bots. Synthetic positive eligibility remains **preview-only**, never gameplay.
- Added a Guild/IP evidence menu page in the staging-only CreatureScript. In-world it reads only the current player's guild ID and explicitly marks property/IP state unverified; it never guesses missing data.
- Added C++ tests for foreign/reused/archived guilds, IP ranks capped below rewarded quests, missing IP, bot opt-out, no gameplay enablement, and bounded `[LOCKED]` preview rows.
- No SQL queries, server-guild generation adapter, active building system, quest handlers, player teleports or visible NPC spawn.


## Guild Steward: real core registry/member-generation read (development)

- Added a **source-isolated read-only** guild adapter using actual AzerothCore `Player::GetGuild()`, `Guild::GetId()`, `Guild::GetMember(player GUID)` and `Guild::GetCreatedDate()` APIs.
- Staging Steward now refuses menu access without a verified guild object, exact member GUID and positive original creation date, not just an untrusted numeric player guild ID; checks occur at menu open and each click.
- The status page now truthfully displays current verified member guild ID and creation timestamp while leaving property ownership, settlement level, IP state and housing **LOCKED**.
- Added C++ tests for missing registry, foreign/stale ID, removed guild member, invalid creation generation, revoked privileges and never exposing stale proof.
- Added source-only no-mutator guard to GitHub Actions and adjusted upstream-build concurrency so new commits do not repeatedly cancel long worldserver links; no live data touched.


## Staging module inventory safety refinement (development)

- Extended the existing read-only module inventory to record tracked-source Git modification state alongside exact AzerothCore and independently cloned module SHAs.
- Reports standalone modules with no independent Git history as `NO_INDEPENDENT_GIT`; never silently trusts their parent revision.
- Suppresses modified-file paths, patch contents, remotes and configuration secrets; explicitly warns that untracked sources and enabled/compiled modules remain unaudited.
- Added disposable Git fixture tests for clean core, dirty independent module, unversioned module, dirty core and rejected invalid directory.
- No server command has been executed, no source patch or database changed.


## Gated property ownership SELECT contract (development only)

- Added a narrow, real AzerothCore `CharacterDatabase.Query` adapter for **read-only** guild settlement state, gated by a new independent staging compiler flag and runtime config, both off in regular builds.
- Queries only `naxx_gs_settlement` with verified guild ID after registry/member/creation validation; no auto SQL migration, table creation, property purchase or state mutation.
- Validates persisted guild creation generation, lifecycle value, development level 1..7 and returns no proof for missing table/row, malformed state or invalid identity.
- The Guild Steward can report an independently checked property row while ALL private housing services, Individual Progression status, quests, rewards and phase routing remain locked.
- Added C++ row validation and policy composition tests, disposable MariaDB real SELECT tests and a static guard banning mutations; CI opt-in build compiles the actual adapter against pinned upstream AzerothCore game/database headers.


## Staging IP runtime source adapter (development only)

- Added an **extra opt-in**, GM/member-only, read-only `IndividualProgression.h` adapter to obtain enabled status, actual rewarded-quest stage and runtime `progressionLimit` from the linked Grimfeather fork.
- The existing effective-stage evaluator caps rank 18 characters at the configured lower limit and refuses disabled/off-world/invalid sources.
- Introduced `NAXX_GS_BUILD_STAGING_IP_READ` in addition to staging Steward compiler opt-in, and separate `NaxxGuildStrongholds.StagingIpRead.Enabled=0` runtime setting.
- Added static no-mutation checks and a **third** real-header compilation variant with pinned public Grimfeather source `706740808fee328b8557607f87b0548cf961e047` checked out beside the module.
- Normal build and property-read-only build exclude the new adapter. No installed-fork compatibility claim, phase change, quest progression update, reward or database operation.

## Guild raid trophies: five symbolic encounters and placement rules (development only)

- Added conceptual trophy catalogue for Onyxia's Head, Ragnaros' Flame, Nefarian's Banner, C'Thun Relic and Kel'Thuzad Sigil.
- Added source-independent guild raid eligibility requiring independently verified boss kill and encounter source, current original-guild ownership, replay-checked unique server receipt and distinct encounter-participating human guild members.
- Explicitly supports mixed-guild and 40-person raid rosters. Bots can assist but cannot satisfy human guild attendance; no hard-coded majority/full-guild restriction.
- Added per-generation placement policy verifying earned unlock, logical decoration slot, no slot conflict and reviewed gameobject entry. Neither proposal ever awards or spawns anything.
- Guild Steward optional staging trophy preview now shows five concepts from the source catalogue, each explicitly [PLAN].
- Expanded source-only no-world-mutation guard and extensive C++ regression tests for guild reuse, archiving, mixed raids, fake kills, bots, replay receipts, forged roster, 40 members and invalid object placement.
- Current draft `naxx_gs_unlock` is **NOT adequate** for generation-qualified trophy receipts and must not be used for live awards until a versioned schema and audit-transaction design are reviewed. No SQL migration made.


## 0.3.x — guild-generation-safe trophy storage contract (unreleased; DRAFT)

- Added two module-owned, **unapplied** characters tables: `naxx_gs_trophy_receipt` keyed to (guild ID, original creation timestamp, unique raid event receipt), and `naxx_gs_trophy_unlock` keyed to (guild ID, original creation timestamp, trophy key) with an additional first-receipt constraint.
- Deliberately preserved the legacy/general `naxx_gs_unlock` table; it is not trusted to prove generation-safe raid achievements.
- Added a disposable MariaDB transaction contract: lock the active matching settlement row, validate verified source/roster policy, ensure no duplicate event or trophy, then commit the receipt, unlock and audit-ledger row **together**.
- Added fault-injection rollback tests after receipt, unlock and ledger inserts; concurrent same-event and different-event/same-trophy races; mixed-guild receipt reuse with independent generation keys; disband/archive refusal with historical preservation; simulated guild-ID reuse; same-schema reinstall preservation.
- Updated SQL-to-purge name safety lint, 11-table count and CI MariaDB job. No production database or gameplay actions, C++ world hooks, GO IDs or client patches.


## Real upstream raid death callback: read-only staging observer (unreleased)

- Added an optional `UnitScript` implementing actual pinned-AzerothCore `OnUnitDeath(Unit*, Unit*)`, with the `UNITHOOK_ON_UNIT_DEATH` registration hook.
- Filters for raid map, creature classified as dungeon boss, nonzero map/instance ID and creature entry, and explicitly enabled staging config.
- Logs only a numerical **raid death candidate**, not raid completion. NO player names/GUIDs, group/Playerbots assumptions, guild credit, Individual Progression changes, SQL inserts or trophy gameobjects.
- Two independent opt-ins: `NAXX_GS_BUILD_STAGING_RAID_OBSERVER` compiler definition and `NaxxGuildStrongholds.StagingRaidObserver.Enabled=1` (off by default).
- Pure negative/positive death-observation C++ tests and static source guard assert no false named-boss, participating-guild or trophy-award proof.
- Pinned public upstream staging compiler variant includes the observer; normal full worldserver compilation must exclude its symbol.


## Raid kill-credit observation and Playerbots-aware participant policy (development only)

- Added second staging-only real AzerothCore event handler `OnPlayerCreatureKillCredit` alongside `OnUnitDeath`, recording only numeric raid instance/creature candidate metadata; no player/GUID or guild attribution in logs.
- Added fail-closed participant domain policy requiring positive, independently verified human-vs-Playerbot classification, original guild generation, unique GUID, verified raid membership, matching instance and per-person encounter contribution.
- Added 40-player/mixed-guild, duplicate GUID, unknown bots, mismatched guild generation and non-participation negative tests.
- Audited pinned public Playerbots source: `GetPlayerbotAI` returns null when module is disabled, so a null AI pointer cannot prove a character is human. Added immutable public source checkout/contract CI job.
- **No new trophy award path**: kill-credit callbacks never imply completed encounter contribution, no persistent raid roster, no actual Playerbots fork runtime dependency, no DB/world/quest writes.


## Opt-in Playerbots positive bot-classifier (staging source only)

- Added a fail-closed one-way classifier: a genuinely nonnull registered `PlayerbotAI` may confirm **bot**; all other cases remain **unknown**, never human.
- Added actual `PlayerbotMgr.h`/`PlayerbotAIConfig.h` opt-in Playerbots registry reader and anonymous kill-credit staging log classification, guarded by separate compile macro and two disabled-by-default settings (operator-reviewed-fork attestation is NOT a source fingerprint).
- Added exhaustive 64-case pure C++ truth-table and no-effect source regression checks; updated pinned Playerbots fork source-contract test.
- Added independent real-upstream header-compilation job with **source-pinned public Playerbots checkout**, no installed local module/build, real realm, SQL or trophy reward. Normal builds still omit all staging scripts.

## Encounter provenance review model (development only)

- Added source-independent attempt and per-member effective-action evidence policy with bounded/unique event IDs, event-time group and encounter binding, and raid-map/instance checks.
- Passive presence, kill credit, unrelated healing, zero-effective actions, stale/unverified source data and duplicate/foreign events never count as qualifying actions.
- Candidate review still asserts zero verified participation and zero trophy rewards. This is NOT positive human/Playerbots proof or a live combat data source.
- Added exhaustive negative C++ tests, no-mutation source guard, CI integration and explicit installed-fork review requirements. No schema, gameplay, module configuration, NPC or other module changes.

## Source inventory for raid and Playerbots (development only)

- Added non-mutating signature/count scanner for copied AzerothCore and module C++ source, without leaking configuration values or source excerpts.
- Hard-coded all human, guild-generation, named-boss and per-member participation evidence to UNVERIFIED, and trophy awards to denied.
- Added synthetic privacy/negative tests, CI wiring and future staging usage notes. No deployable game features, SQL, NPCs, Playerbots or IP changes.

## Offline raid group/guild timeline (development only)

- Pure C++ windows for encounter-bound raid join/leave membership with original guild generation, distinct groups and half-open time intervals.
- Negative tests for previous-wipe replay, group swap, overlapping windows, roster gaps, invalid source and 40-member mixed guilds.
- Never certifies a character human, actual guild membership or trophy grant; no AzerothCore runtime hook, SQL, IP, Playerbots or world changes.

## Offline two-guild privacy matrix milestone

- Added 132-case synthetic bidirectional privacy/interactivity review matrix across six lifecycle/IP/bot contexts.
- Hard rejection of cross-guild leaks, invalid reports, missing coverage and broken own-guild visibility.
- Never enables housing; no SQL, worldserver, Playerbots, IP or live changes.

## Fail-closed isolation probe enum hardening

- Rejects out-of-range synthetic visibility kinds before fixed-array indexing and unknown phase comparison modes before visibility decisions.
- Adds negative C++ tests. No player/GO phase hooks, SQL, instance routing or other module modifications.

## Interrupted visitor reconnect and safe-return review (offline)

- Added fail-closed recovery candidate decisions for persisted Prepared/Inside/Returning tickets after crashes, relog and interrupted teleports.
- Explicitly refuses unverified positions, wrong player, uncertain third locations and invalid state values; never clears a ticket or rewrites a safe origin.
- Added C++ negative tests and read-only source guard. No worldserver, SQL, Playerbots, IP or live changes.

## Pre-uninstall visitor evacuation/drain safety model

- Pure C++ preservation-first blocking policy for Prepared/Inside/Returning tickets, missing characters, corrupt DB counts, new-admission races, pending transfers and missing backups.
- Adds synthetic tests and source-level no-purge guard. Even the clean synthetic scenario only reaches manual operator review.
- No actual module uninstall, SQL deletion, teleport, Playerbots, IP or live world changes.
