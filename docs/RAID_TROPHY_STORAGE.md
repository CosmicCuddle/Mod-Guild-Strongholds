# Guild raid trophy receipts and persistent unlocks — draft SQL transaction contract

**Status: designed and tested exclusively on a disposable `naxx_gs_ci_test` database. NO live database migration, C++ encounter hooks or trophy awards.**

## Why two separate tables?

The original, still-unapplied `naxx_gs_unlock` has a key of (`guild_id`, `unlock_key`) and no **original guild creation date**. That is insufficient for a permanent raid trophy: AzerothCore may reuse a numeric guild ID after disband, and a trophy must NEVER be inherited by an unrelated newer guild.

We therefore leave `naxx_gs_unlock` untouched and add two new explicitly namespaced tables **only to the manual draft characters schema**:

| Table | Primary key | Purpose |
| --- | --- | --- |
| `naxx_gs_trophy_receipt` | `guild_id, guild_created_at, event_receipt` | Proves an event receipt has been recorded once for that exact original guild generation |
| `naxx_gs_trophy_unlock` | `guild_id, guild_created_at, trophy_key` | One persistent trophy per guild generation; records first qualifying receipt |

`naxx_gs_trophy_unlock` also has a unique index on (`guild_id, guild_created_at, first_event_receipt`), providing another defence against assigning one receipt to two unlocks.

The receipt table stores symbolic `trophy_key`, raid map/instance IDs, verified human attendee count and a timestamp. These are **future server-generated evidence fields**: the CI fixture uses synthetic values, not real WoW boss identities. No GO display ID, trophy position, housing map or world SQL is included.

## Single atomic transaction contract

A future trusted C++ persistence adapter MUST:

1. First confirm the actual server-side raid kill, encounter ID, IP access rules, unique event identifier, verified human participation and current guild member/original creation generation. These inputs must never be taken from addon packets or player chat.
2. Lock the `naxx_gs_settlement` record by numeric guild ID using `SELECT ... FOR UPDATE`. Abort if missing, archived or whose `guild_created_at` differs.
3. Check generation-qualified event receipt and trophy unlock existence. These checks support useful errors; the database primary/unique keys remain the final race guard.
4. Insert **one** `naxx_gs_trophy_receipt`, **one** corresponding `naxx_gs_trophy_unlock` and one append-only `naxx_gs_ledger` event, all inside the **same** InnoDB transaction.
5. Commit all three together, or roll back all three on any failure. Never award a trophy/gameobject directly before the transaction is durably committed.
6. Preserve records on guild disband/archive, including older guild generations. Never automatically assign them to a reused guild ID; restoration must verify the original creation generation.

The transaction contract is deliberately **per-guild**: for a mixed-guild 40-player raid, the same server-generated kill receipt can be stored separately for two guilds if each meets its independent human-player participation policy. Playerbots can help kill a boss but are not counted as qualifying human guild members.

**Important:** this is a test-only Python implementation. It accepts deliberately simulated `source_verified=True` and `simulated_policy_approved=True` flags. These are *not* a safe real-world proof mechanism and MUST NOT be exposed to users, NPC gossip, addons or production code.

## Safety, backups and schema lifecycle

Both table declarations are added to `data/sql/manual/install_characters.sql`, which contains only `CREATE TABLE IF NOT EXISTS` statements. `uninstall/purge_characters.sql` includes the two new tables solely for a deliberate, destructive administrator action. **Ordinary uninstall must not execute that purge.** Current schema is NOT a versioned migration engine: a pre-existing table with a different shape will not be updated by `CREATE TABLE IF NOT EXISTS` and must be handled through a separately reviewed migration.

Before any future installation, create verified backups of the actual world/characters databases, the compiled worldserver, module source/SHAs and config; use a separate staging realm. No SQL should be applied to the live server now.

## Automated CI verification

`tests/mysql_trophy_receipts_test.py` is run under a GitHub Actions MariaDB 10.11 service with database name **exactly `naxx_gs_ci_test`** and the explicit environment guard `NAXX_GS_CI_ONLY=YES`. The shared `connection()` helper refuses any other database name.

It exercises:
- no boss-proof or activity-policy flags, malformed receipts and insufficient/out-of-range human raider counts;
- missing property, archived property and mismatched original guild creation timestamp;
- rollback after receipt insert, after unlock insert and after audit insert;
- repeated receipt, receipt reused for another trophy, another kill attempted for an already-unlocked trophy;
- two threads racing with the exact same receipt, and two threads with different receipts racing for the same trophy;
- mixed-guild independent credit from one raid and unchanged records for both original guild generations;
- retaining achievements during guild archive and reapplying the same draft schema;
- retaining old-generation history while a simulated replacement guild with a recycled numeric ID begins its own new progress;
- verifying the composite generation-qualified primary-key columns exist.

`tests/sql_safety_check.py` also asserts that both new table keys include `guild_created_at` and the explicit module-owned purge targets match the declared draft install tables.

Passing these tests is **not** evidence of in-game trophy drops, persistent privately phased GOs, correct raid encounters, the user's actual Playerbots participation or a successful full deployed-fork database migration.

## Next release blockers

Real AzerothCore raid kill source hooks; server-authoritative event IDs and member/Playerbot presence; per-era boss/map verification; durable C++ transaction implementation; trophy unlock read-only display; reserved reviewed 3.3.5 GO assets; genuine two-guild isolation and safe return; rollback/uninstall/live-stage backups; owner's desired human participation threshold.

Until those are independently tested, **the Guild Steward only shows `[PLAN]` trophy concepts; all rewards and placements remain disabled.**
