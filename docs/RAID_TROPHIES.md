# Guild raid trophies — safe progression design and pure-domain prototype

**Status: game rules and staging preview only. No boss kills are tracked and no trophy has been awarded or spawned.**

This implements the early structure of the guild-achievement system requested for a future **shared guild settlement**: the guild defeats a major raid boss, earns proof of victory, and may later place a visual trophy in its own stronghold.

## Trophy catalogue

| Trophy key | Symbolic encounter key | Decoration concept |
| --- | --- | --- |
| `onyxia_head` | `classic_onyxia` | Mounted Onyxia head |
| `ragnaros_flame` | `classic_ragnaros` | Molten Core brazier |
| `nefarian_banner` | `classic_nefarian` | Blackwing victory banner |
| `cthun_relic` | `classic_cthun` | Sealed Ahn'Qiraj relic |
| `kelthuzad_sigil` | `classic_kelthuzad` | Naxxramas memorial sigil |

These are **symbolic** identifiers, not established boss creature IDs, real 3.3.5 GO display IDs, spells, DBC entries or map spawns. *Classic Onyxia* is deliberately distinct from any later Wrath Onyxia variant. Confirm exact encounter/map/DBC identity from the owner's server fork before wiring kill credit. No unfamiliar models or invented gameobject IDs are distributed.

## Verified raid completion — proposed credit, not an award

`ProposeTrophyUnlock` accepts only a separately validated **server-side** kill record, never player-generated messages or addon submissions. Inputs must prove:

1. The feature was explicitly permitted by a future audited runtime gate (default **false**).
2. Exact known trophy and matching verified **encounter identity**, confirmed real boss kill, raid roster and nonzero raid map/instance.
3. Server-generated, unique, replay-checked **event receipt**, with bounded safe token. An event token typed by a player or copied from an addon can never be a valid proof by itself.
4. Verified **original guild ID plus its original creation timestamp**, with the same **active** guild property; do not inherit progress when a guild ID is reused.
5. Authoritative duplicate unlock and event receipt lookups; if unknown, refuse the proposed credit.
6. An **explicit** minimum of 2–40 **distinct human players** who belonged to that original guild **and** had encounter participation verified by the server. No implicit default or one-player solo-guild credit.

### Mixed-guild and 40-player raids

A raid can contain **different guilds, unaffiliated raiders and Playerbots**. Neither all-players-from-one-guild nor a raid majority is assumed. Each guild is evaluated **independently** against its verified members; if two guilds meet the configured minimum, each may receive a **proposed** credit for the same kill, keyed to its own original guild generation. Bot raiders can help defeat the boss but **do not count towards the distinct human-guild minimum**. The prototype accepts up to 40 unique raid member GUIDs.

This is a proposed participation policy, not a final release rule. An eventual admin setting may set the minimum per encounter/raid size, but it must be explicitly reviewed; no setting is installed today. The owner can decide how strict actual guild participation should be before the hook goes live.

**Individual Progression distinction:** A guild trophy would be based on genuine guild raid completion, not simply one character's high IP tier. The installed IP module must still enforce access/attunements for each participant. Trophy decoration *visibility* in a private home must not bypass those personal IP restrictions.

## Replay, data persistence and guild-generation safety

A successful C++ `TrophyUnlockDecision::ProposalOnly` does **not** write anything. A later durable schema must atomically save both:

- an unlock unique to **(guild ID, original creation timestamp, trophy key)**;
- a boss event receipt unique to **(guild ID, original creation timestamp, server kill receipt)**.

The existing **draft** `naxx_gs_unlock(guild_id,unlock_key)` table is **insufficient by itself**: it lacks original generation and kill-receipt provenance. Do **not** repurpose it to grant these trophies or write SQL to it on the live server. Before introducing real awards, design reversible, namespaced migration SQL, preservation-first uninstall, append-only audit receipts and concurrency/rollback tests. No migration or live database change is included in this batch.

## Placement — independent authorization

`ProposeTrophyPlacement` requires all of:

- an active property matching the player's original guild generation;
- an independently verified earned unlock tied to that generation;
- one of five **purely logical** prototype trophy wall slots `hall_trophy_1` through `hall_trophy_5`, with slot availability verified;
- a separately reviewed/approved **real** gameobject entry and visual asset.

These slot names are **not** world coordinates or proof of client models. Even when a synthetic test supplies an object entry and every flag, the result remains **ProposalOnly**, with `ObjectSpawned=false`. No placed object, rotation, phasing, guild visitor permission or worldstate hook exists.

## Automated tests

`tests/trophy_tests.cpp` checks real-mixed-guild/40-member scenarios, bot exclusion, missed boss participation, stale/recycled guild generation, archived property, unknown boss, fake/unverified kills, WotLK-versus-Classic encounter mismatch, duplicate receipts, foreign/guildless roster, duplicate member GUIDs, invalid event strings, invalid threshold and denied placement. It also proves success remains non-operative.

`tests/trophy_no_effects_tests.py` ensures the trophy policy and staging view do not acquire SQL queries, quest changes, world object summons, teleport or phasing calls. GitHub CI compiles the pure C++ tests and compiles the module with real upstream headers. **These are not tests of actual raid kill events on the user's server.**

## Before any live development feature is enabled

- Inspect the exact deployed AzerothCore raid boss kill callbacks, group/Playerbots and per-guild member participation rules, and the custom Individual Progression era gates.
- Verify boss NPC/map/encounter identity for each **classic** raid and how 40-player raid participation is represented.
- Decide the desired per-guild minimum human raider requirement for Onyxia, Molten Core and other raids, including mixed guild raids.
- Implement an authoritative, transactional receipt and unlock schema with backups, repeat-event prevention, disband/archive and guild ID reuse protections.
- Choose tested gameobject assets for the guild's chosen racial architectural style.
- Prove guild-private world and safe entrance/exit, both-direction visibility, Playerbots, mixed IP tiers, logout/disband/restart, rollback and full uninstall.

**Do not apply draft SQL or spawn NPC/GO entries on production. All gameplay startup capabilities remain false.**


## Draft permanent storage and event receipts — October 2026 extension

The prototype now includes `naxx_gs_trophy_receipt` and `naxx_gs_trophy_unlock` in the **unapplied draft** characters SQL, rather than writing to the older/general `naxx_gs_unlock` whose guild-ID-only primary key does not protect original guild generation. Each new table contains `guild_id` AND `guild_created_at` in its primary key. An event receipt is unique for a given guild generation; an unlock is unique per (guild generation, trophy key). Same encounter can credit two separately qualified guilds without crossing ownership or inventing duplicates.

The **disposable CI-only** InnoDB transaction simulates future persistence by taking the matching `naxx_gs_settlement` row `FOR UPDATE`, rejecting missing/recycled/archived guilds, checking replay and existing unlock, then writing a unique kill receipt, matching trophy unlock and ledger audit in ONE transaction. Injected errors after any insert roll all three back; concurrent races commit only one. Test data is fabricated and does not prove a real kill, bot roster, map instance, IP phase or housing location. This provides the persistence **contract**, not a deployable C++ reward handler. Full detail: [RAID_TROPHY_STORAGE.md](RAID_TROPHY_STORAGE.md).

The previous 'before introducing real awards' storage requirement is partly satisfied at **draft schema and isolated DB test** level only. Actual migration validation, real server-side encounter evidence, trusted runtime C++ adapter, genuine models and secure placement remain pending.


## Source-verified passive boss-death callback observation

Added `StrongholdStagingRaidObserver.cpp`, a **separately opt-in real AzerothCore `UnitScript`** that can listen to generic `OnUnitDeath` callbacks and log a raid boss-like creature's **map ID, instance ID and creature entry**. This is *candidate telemetry*, not trusted Classic boss completion. There is deliberately no hard-coded boss/raid ID mapping and no player/Playerbot/Guild or individual-progression access.

This new staging observer **never calls `ProposeTrophyUnlock`**, the test-only Python receipt recorder or any database API. Its pure-core-independent policy explicitly keeps `BossEncounterVerified`, `GuildParticipationVerified` and `GuildTrophyGranted` false. User-approved actual boss/instance validation and a safely sourced human guild participation audit are mandatory before event-derived credit. See [STAGING_RAID_OBSERVER.md](STAGING_RAID_OBSERVER.md).
