# Isolation research and go/no-go decision

**Current state: not yet proven.** The pure catalog and access helper do **not** implement private guild areas.

## What we know

- AzerothCore's `mod-guildhouse` is useful as design reference, but its implementation derives phase IDs from guild IDs and relies on reserved-area-specific behaviour. It cannot simply be transplanted into ten existing town zones.
- AzerothCore's Individual Progression module already changes visibility/content by character progress, so world phase interactions are potentially fragile.
- Ordinary client/world phase masks have limited capacity and cannot be assumed to yield arbitrarily many independent guild spaces.

## Candidate isolation mechanisms (for evaluation, not implementation yet)

1. **Dedicated reserved map/zone with audited guild separation:** closest to an existing guildhouse pattern but needs proof of correct object and player visibility and safe phase restoration for the deployed core/forks.
2. **Private map instances:** may provide clearer physical separation, but instance creation, persistence, group/guild routing, NPC/world persistence and teleport handling need investigation on this specific core.
3. **Purely separate plots on shared-world coordinates:** unacceptable for privacy if a visitor can enter another guild's area or see its objects; not a default solution.

The system must not set global player phases blindly or patch global map-visibility checks just to gain a fast prototype.

## Required executable demonstration

- Two test guilds in the same property type enter concurrently.
- Both see **only** their own guild players, NPCs and objects.
- Leaving, logging out and relogging must never strand the player or corrupt IP phases.
- Guildless and cross-guild entry attempts fail on the server.
- Other installed modules see normal combat, area, quest and teleport behaviour outside housing.
- Server restart restores guild housing ownership and optional data without leaking objects.
- Disabling/uninstalling the feature evacuates or safely recovers affected characters.

## Stop conditions

Do not enable the future Guild Steward, preview, purchase, teleport or building placement if isolation is uncertain. Compile-time C++ unit tests for a mock access policy are necessary but **not sufficient**. Staging worldserver testing is mandatory.

## Data/logic boundary

`src/StrongholdCatalog.*` contains logical themes and plot names only. `CheckGuildEntry` is a **pure policy function** accepting prevalidated IDs and a test-established isolation-status value; it is not called from AzerothCore hooks and it does not teleport or spawn anything.

Further code should introduce a server-only adapter to acquire authoritative guild membership, not trust any ID sent by the client. Real-world server hooks and map APIs must be checked against the exact deployed core revision before integrating this policy.


## Source inspection checkpoint — 8 October 2026

Research targets inspected (upstream code, **not** the user's deployed fork):

- [AzerothCore MapMgr.cpp](https://github.com/azerothcore/azerothcore-wotlk/blob/master/src/server/game/Maps/MapMgr.cpp): `CreateMap(id, player)` delegates instanced-map selection to `MapInstanced::CreateInstanceForPlayer` when appropriate. `PlayerCannotEnter` enforces dungeon/raid entry requirements (including scripts and raid groups).
- [AzerothCore MapInstanced.cpp](https://github.com/azerothcore/azerothcore-wotlk/blob/master/src/server/game/Maps/MapInstanced.cpp): `CreateInstanceForPlayer` resolves destination instance IDs from `sInstanceSaveMgr->PlayerGetDestinationInstanceId` or generates one. It is **player/group/instance-save oriented**, not keyed by guild property ownership.
- [Guild House mod_guildhouse.cpp](https://github.com/azerothcore/mod-guildhouse/blob/master/src/mod_guildhouse.cpp): its player script assigns the guild's saved phase in GM Island (zone/area 876); its `OnBeforeWorldObjectSetPhaseMask` override changes combined-phase handling in that zone. It also has a code comment worrying about players in the wrong phase.
- [Individual Progression header](https://github.com/ZhengPeiRu21/mod-individual-progression/blob/master/src/IndividualProgression.h): individual progression is derived from quest state and exposes `GetPlayerProgressionFromQuests` and `hasPassedProgression`; its own area/phase code needs separate conflict testing.

### Consequences for implementation

**Dungeon/raid instance reuse:** do not assume `TeleportTo(mapId,...)` routes guildmates to the same persistent private housing instance. Ordinary dungeon/raid instance saving, group leader routing, raid restrictions, lockouts, resets and world/DB spawn persistence may interfere. Would require a carefully designed adapter plus staging tests, possibly core changes. We have **not** implemented that adapter.

**Existing Guild House phasing:** do not take its `guildId + 10` phase convention and deploy it across arbitrary zones. It relies on zone-specific behavior and may override other systems' phase masks. This does **not** prove compatibility with Individual Progression or multiple simultaneous Guild House modules.

**Recommendation for first practical spike:** a test-only privacy feasibility prototype, disabled by default, that records/map-logs the intended owner guild, verified client guild ID, selected property key and safe return location; it must **not teleport or change phases** until we can prove one of the isolation methods on the target core. Follow with the documented two-guild in-world test, not a guessed permanent phase allocation.

**No production approach chosen yet.** A feasibility decision is blocked on inspecting the actual server fork/modules and a physical test world. Both options remain candidates; neither is safe to promise as production-ready.


## Updated decision evidence

The **upstream** `Player::TeleportTo` validates map coordinates and can deny entry for map restrictions, and dungeon/raid instance assignment relies on player/group instance saves. This reinforces that a successful teleport request is not evidence of correct guild-private arrival. The new recovery ticket contract is documented separately in [SAFE_RETURN_CONTRACT.md](SAFE_RETURN_CONTRACT.md); the candidate map/phase approaches and their stop conditions are in [ISOLATION_DECISION.md](ISOLATION_DECISION.md). No private-site solution has yet been implemented.
