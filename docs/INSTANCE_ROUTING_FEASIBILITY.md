# Candidate A — Guild-private map instances: feasibility and hard blockers

**Status: NON-OPERATIONAL.** Upstream source review and independent C++ preflight tests. No instance has been allocated, no map loaded, no Playerbot moved, and no private guild zone entered.

## Source reviewed

Upstream `azerothcore/azerothcore-wotlk` commit `7b2cecef92b271a468e39d89831b520b20ae06a8` (8 October 2026):

- [MapMgr.cpp](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/src/server/game/Maps/MapMgr.cpp): `CreateMap(id, player)` forwards instanceable maps to `CreateInstanceForPlayer`; `GenerateInstanceId()` operates on a **global** instance ID allocator.
- [MapInstanced.cpp](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/src/server/game/Maps/MapInstanced.cpp): dungeon selection uses `InstanceSaveMgr::PlayerGetDestinationInstanceId(player, map, difficulty)`. If none exists, the core generates a new ID. `CreateInstance` requires an existing map entry and instance template, registers an `InstanceMap`, and adds an instance save when creating a new dungeon.
- [InstanceSaveMgr.h](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/src/server/game/Instances/InstanceSaveMgr.h): player/group bound saves, difficulty and reset semantics must be respected.
- [Player.cpp](https://github.com/azerothcore/azerothcore-wotlk/blob/7b2cecef92b271a468e39d89831b520b20ae06a8/src/server/game/Entities/Player/Player.cpp): teleport checks map/client, restrictions, and may reject before arrival.

This is a **review of public upstream code**, not proof that your custom AzerothCore/Playerbots fork has the same APIs.

## Why normal AzerothCore instance IDs are not a housing solution by themselves

1. A GUID/instance ID obtained from `GenerateInstanceId()` does **not** create a map or force that map's player destination binding. Our module must **not call this allocator speculatively**.
2. The normal dungeon destination depends on player/group binds and difficulty, not guild ownership. A guildmate entering while in a different raid group could be routed elsewhere unless a validated integration changes or bypasses this safely.
3. The created dungeon instance is subject to instance save/reset and potentially raid encounter/collision constraints. A guild's settlement must not reset after a raid reset, award raid credit, or consume any dungeon/raid instance lockout.
4. A stable map ID needs valid client DBC/map data, instance metadata, terrain, gameobjects and safe entrance/exit coordinates. An invented map ID is not playable.
5. A core patch or carefully tested extension may ultimately be required. **Do not modify existing AzerothCore, IP or Playerbots sources without the owner's explicit approval and a rollback plan.**

## New C++ source (tests only)

`StrongholdInstanceRoute.h/.cpp` defines a **non-mutating proposed registry**. It refuses a candidate unless evidence covers:
- installed core/modules verified;
- client DBC map and server instance template;
- a dedicated non-progression map;
- instance ID **issued by the core**, with durable save/restart behaviour;
- safe player and group bind semantics;
- a *demonstrated guild-specific destination resolver*;
- verified safe return and cross-guild A/B world isolation;
- verified original guild generation/property identity.

The test suite exercises every missing-evidence condition, duplicate/retired instance ID, a later guild using an old ID, and *actual-arrival* comparison (map + instance ID + membership/generation). All "successful" proofs in the C++ tests are **synthetic fixture flags**, not real observed evidence.

This layer creates **NO** SQL tables yet because it cannot responsibly reserve a core instance ID outside the core instance lifetime manager. If the deployed fork cannot implement guild routing without core changes, candidate A remains blocked.

## Stage-gate when an actual isolated test world is ready

1. Back up the configured source and database and record exact deployed AzerothCore, Grimfeather IP, Playerbots and other module revisions.
2. Verify an approved non-raid, non-IP progression **client-visible map** and valid server instance template.
3. Prototype guild-specific map creation/routing **on staging only** without conflicting with player/group/raid bindings. Confirm instance restart/lifetime persistency through the core, not only our registry.
4. Confirm Guild A and Guild B can both enter the same logical housing map with **different core instance IDs**, including group/un-grouped/bot cases.
5. Confirm player, creature and gameobject visibility and interactions in **both directions**, including Individual Progression phase effects and guild-only authorisation.
6. Test failed entry, login from inside, crash, restart, guild disband, kick, leave and emergency return with the durable return-ticket contract.
7. Test saved world object state, guild trophy placement, instance reset/update schedules, save/reload and module disable/uninstall.
8. Record and sign off exact test evidence before allowing any housing startup capabilities.

**Decision not made:** Candidate A could scale more naturally than one phase bit per guild, but the necessary persistent guild-keyed core routing API is not established on the current source. Candidate B phase slots are separately bounded and not approved either. All module housing gameplay readiness remains false.
