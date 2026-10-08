# Read-only settlement SQL staging adapter

**Current state:** source implemented and draft schema SELECT covered by disposable CI; never deployed to a real server. Absolutely no housing gameplay is enabled.

## What it reads

`StrongholdStagingPropertyAdapter.cpp` uses an indexed single-row query on our **own** draft `naxx_gs_settlement` characters table:

```sql
SELECT guild_id, guild_created_at, lifecycle_state,
       lifecycle_version, development_level
FROM naxx_gs_settlement
WHERE guild_id = <server_verified_guild_id> LIMIT 1;
```

The C++ code substitutes the **server-verified numeric guild ID**, not client-provided gossip strings. `PropertySqlRow` and `InspectPropertyRow` check that guild ID/generation are nonzero, lifecycle state is precisely `active` or `archived`, and the settlement level is in 1–7. An existing row from a **different original guild generation** is reported as a mismatch and must never be silently inherited. No row or unavailable table returns *no ownership proof*. The actual `StewardEvidence` layer separately checks current original guild generation and archived status.

## Triple opt-in (and normal-build exclusion)

1. Define `NAXX_GS_BUILD_STAGING_STEWARD` in a **staging build only**.
2. Define `NAXX_GS_BUILD_STAGING_PROPERTY_READ` as a **separate**, explicit staging build option. The new adapter rejects being compiled without the steward flag.
3. Set `NaxxGuildStrongholds.StagingPropertyRead.Enabled = 1` (defaults to **0**). The Guild Steward config itself must also be on, requiring a GM in a verifiably registered guild.

Without *all* of these, the adapter cannot run the query. A normal build does not contain a property database integration. There is no SQL install/update code in the module. The draft schema would have to be separately reviewed and backed up, then approved for a disposable/staging characters database; **we have not done that**.

## Runtime boundaries and player experience

A test GM clicking the Steward's `Guild and IP Evidence` option could see that a saved property record was **checked** against the current guild generation, with explicit lock status. The query is not executed when opening the menu or selecting unrelated theme/building pages. If a guild is kicked/disbanded or no longer matches, the menu refuses access and a property lookup is not attempted.

Even if the property row is valid and active, **no housing access** is granted. There is no verified private guild map, teleport, server-side claim service, source-matched Individual Progression adapter, playerbot isolation or quest reward integration. `HousingAvailable` is always false in the combined evidence report.

## Automated evidence and limits

- `tests/property_read_tests.cpp`: explicit opt-in, no-query, missing row, wrong guild generation, archived row, malformed lifecycle, invalid guild/level, housing flag always off.
- `tests/property_read_evidence_tests.cpp`: composes SELECT proof with original-guild lifetime and IP fail-closed evidence; proves a valid row alone remains locked.
- `tests/mysql_property_read_test.py`: disposable protected `naxx_gs_ci_test` MariaDB real SELECT (not production), active/archived results, missing rows, creation-time changes, repeat reads, preservation across the same draft schema reinstall.
- `tests/staging_guild_adapter_source_tests.py`: source-only enforcement of exactly one gated `CharacterDatabase.Query` call and lack of INSERT/UPDATE/DELETE/CREATE/ALTER/DROP in that query.

The actual pinned upstream *compilation* job checks against public AzerothCore `DatabaseEnv`/Field types. It is **not** a real server run or proof that the user's deployed fork has compatible DB API/schema.

**Do not run any of the manual draft SQL on your live database or enable this optional adapter on production.**
