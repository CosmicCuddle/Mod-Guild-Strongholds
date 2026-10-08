# Real raid-death observer — staged source milestone, NOT trophy credit

**Status:** read-only C++ `UnitScript` candidate detection in the optional staging build. No installed live worldserver changes, no hooks on production, no event receipts, no saved trophies.

## 1. Verified upstream interface

The source-pinned AzerothCore `7b2cecef92b271a468e39d89831b520b20ae06a8` exposes:
- `src/server/game/Scripting/ScriptDefines/UnitScript.h`: `UnitScript::OnUnitDeath(Unit*, Unit*)` and explicit `UNITHOOK_ON_UNIT_DEATH`;
- `Creature::IsDungeonBoss()` for the current boss-flag classification;
- `Map::IsRaid()` and `Map::GetInstanceId()` for raid map/instance identity.

These methods can report the **occurrence of a death callback**, *not* that the event matches one of our five Classic trophy encounters or that any player qualified. A different raid difficulty, bugged flags, reused spawn, GM-killed NPC or custom encounter script can change meaning. The installed fork must be reviewed separately.

## 2. Double opt-in and safe output

The code is excluded from normal compilation unless **`NAXX_GS_BUILD_STAGING_RAID_OBSERVER`** is explicitly supplied. Even when compiled it remains silent unless **`NaxxGuildStrongholds.StagingRaidObserver.Enabled=1`** (default **0**) is independently enabled on a backed-up isolated staging realm.

When a creature dies, it filters:
1. actual `OnUnitDeath` callback and non-null `Creature`;
2. a raid map according to the current core;
3. a creature marked `IsDungeonBoss()`;
4. nonzero map ID, instance ID, and creature entry.

If everything matches it logs a message prefixed **`NaxxGS STAGING raid death CANDIDATE`** and the three numeric identifiers. No character name, GUID, guild ID, location coordinates, chat, addon payload, inventory or private data is printed. It does not write SQL or generate durable kill receipts.

Logging a candidate must NOT be treated as permission to award, teleport, spawn, decorate or claim anything. We do not yet have a verified boss/map/difficulty allowlist on the owner's custom server.

## 3. Why guild participation stays unverified

**Crucial:** A killing blow is not a 40-player participation record. Being in the instance or in the raid roster does not prove encounter participation, raid completion/kill credit or that the character was a **real human rather than a Playerbot**. The observer therefore never sets any of the prototype `TrophyKillProof` flags or calls the trophy proposal/persistence tests.

Before even a *dry-run* guild credit can be constructed from actual players, we need:
- an approved encounter-specific map/instance/difficulty/boss mapping for this installed fork, separating Classic from Wrath Onyxia;
- trusted group/instance membership snapshots, distinct player GUIDs, membership at kill time, original guild creation generations;
- validated human-versus-Playerbot identity using that installed Playerbots fork (no guesses from session presence);
- server-side encounter contribution/credit evidence, not simply in-zone proximity, guild ID, killing blow, loot or addon messages;
- bounded unique event IDs, duplicate-event rejection, exact boss event ordering, and negative tests for restarts/retries/GM kills.

This should be designed jointly with the actual Playerbots source and custom raid/progression rules before a game-facing reward handler is proposed.

## 4. Automated validation

`tests/raid_death_observation_tests.cpp` exercises disabled config, player/noncreature death, non-raid map, nonboss creature, zero instance/map/creature identifiers, valid candidate and **permanent no-proof/no-reward** flags.

`tests/raid_death_observer_source_tests.py` checks explicit compile/config guards and refuses forbidden world/guild/SQL/quest action calls. GitHub CI compiles the actual staging observer against pinned public AzerothCore headers in the separate staging variant; a normal worldserver build rejects the staging observer symbol.

This is NOT an in-game acceptance test. No server starts, no raid boss is spawned, no raid kill is simulated in-game, and the hook's behavior on the deployed custom fork remains unverified.

## 5. Current safety boundary

Live housing, original guild-private map, player entrance/exit, safe return, Playerbots isolation, player IP progression, trophy unlock persistence (real C++), decorations, quests and World DB assets remain blocked. No Git Pull or database changes on production are appropriate at this stage.
