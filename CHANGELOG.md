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
