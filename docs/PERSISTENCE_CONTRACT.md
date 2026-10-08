# Persistence contract — staging proof, NOT connected to worldserver

## Scope

This repository has a standalone MariaDB transaction-contract test at `tests/mysql_construction_test.py`, using the actual **draft** `data/sql/manual/install_characters.sql` against an isolated `naxx_gs_ci_test` database in GitHub Actions.

This test is an **executable storage specification** for future AzerothCore integration. It is NOT a ready-to-run production SQL script or proof that Worldserver can safely debit a player's inventory.

The test refuses to start unless `NAXX_GS_CI_ONLY=YES` and the database name is exactly `naxx_gs_ci_test`. It drops/recreates module-owned tables **only in that dedicated test database**.

## What can be safely transacted within the character database

The first integration target is **virtual Guild Supplies**, represented by `naxx_gs_settlement.guild_supplies`. An accepted *supply-only* deposit in one InnoDB transaction changes four module-owned records:

1. Lock the guild's settlement row with `SELECT ... FOR UPDATE`.
2. Lock the selected building project row. Check theme, project, level, bounds and remaining balance.
3. Check receipt ID, then **debit** the guild's supply balance.
4. Update the project progress and version, requiring the expected version.
5. Insert the uniquely keyed contribution receipt and ledger history.
6. Commit, or rollback the entire transaction on any error.

The unique index `uk_guild_receipt (guild_id, receipt_key)` is still required. A precheck alone does not prevent retries/races. The test exercises both same-receipt and distinct-receipt concurrent requests.

The simulated routine accepts actor identity/permissions as arguments to **test** the expected policy. A real C++ adapter must obtain them authoritatively from the connected player's server-side guild and guild rank; **never** from client-provided IDs.

## What is NOT safe or implemented

- Physical timber/iron/other inventory items are **not** deducted by the test. The AzerothCore object inventory must not be separately charged before this module's character DB transaction commits without a proven rollback/escrow strategy. Do **not** enable physical material deposits in-game until this problem is solved on the installed core.
- No actual guild supply earning, NPC menu, quest/raid event, IP adapter, Playerbot contribution or persistent private map implementation exists.
- There is no C++ AzerothCore persistence adapter yet. Python code in `tests/` is **test-only** and must never be called from worldserver.
- Completing a supply portion does NOT complete a building if timber or iron requirements remain.
- Race conditions and tests in an isolated CI MariaDB are useful but do not prove production compatibility, durability on host crash, cross-database atomicity, or safe player inventory accounting.

## Forward compatibility and rollback

- The draft tables are namespaced `naxx_gs_*` and stored outside AzerothCore's automated migration directory.
- No live migrations have been executed.
- A data-preserving uninstall leaves all tables, receipts, and project history.
- Destructive purge remains opt-in, requires backup, and drops **only** owned tables.
- Before any schema evolves in production, provide explicit schema version tracking and tested migration/rollback scripts; `CREATE TABLE IF NOT EXISTS` is **not** a safe schema upgrade mechanism.
- If an unexpected crash or DB exception occurs before commit, the test expects all changes to roll back without losing supplies or awarding duplicate project credit.

## How to see the checks

Open `.github/workflows/catalog-tests.yml` and the GitHub Actions tab. The `mysql-construction` job starts a temporary MariaDB service, creates a dedicated test-only database, applies draft schema, and checks atomicity, rollback, concurrent receipts and guild separation.

Do not run the test against a live AzerothCore characters DB. The hard-coded guards are intentionally strict.

## Additional reversible-install checks

The MariaDB contract suite also repeats all draft `CREATE TABLE IF NOT EXISTS` statements **after** sample contributions and verifies both guilds' committed data survives. This tests an identical-schema reinstall, **not** a versioned schema upgrade.

A deliberately unrelated sentinel table is created **inside the CI-only database**, then the optional module purge is executed. The suite verifies the sentinel survives and the seven module-owned tables are gone. It then cleans up the sentinel. None of these test statements run on the live server, and this does not replace a verified production uninstall plan.

Injected failures at four transaction boundaries (after debit, project update, receipt insertion, and ledger insertion) must roll back every change, including the unique receipt. This tests recoverable SQL failures, **not** a process kill or physical disk-crash durability.


## Archived owner safety

The **revised initial, unapplied** `naxx_gs_settlement` table now stores the guild creation date plus lifecycle state/version. Deposits must verify the origin and active state **inside the same guild row lock** used for accounting. An archive updates lifecycle status and appends an audit event within one transaction. Real in-game escrow/authoritative guild verification remain to be implemented. This revised initial schema is **not a migration** for already deployed database tables.
