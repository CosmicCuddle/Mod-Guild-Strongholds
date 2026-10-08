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
