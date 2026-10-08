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
