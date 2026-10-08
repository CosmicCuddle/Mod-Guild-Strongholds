# Housing privacy decision gate

**Decision: NOT YET IMPLEMENTED / NOT APPROVED.**

## What the upstream code tells us

- AzerothCore `MapMgr::CreateMap(mapId, player)` forwards instance maps to `MapInstanced::CreateInstanceForPlayer`.
- For dungeons/raids, `MapInstanced::CreateInstanceForPlayer` consults `sInstanceSaveMgr->PlayerGetDestinationInstanceId`, then uses saved instance IDs or generates a new one. There is no proven guild-owned instance mapping in this call.
- `Player::TeleportTo` validates coordinates and map access. For far teleports it calls `PlayerCannotEnter` before creating/selecting an instance; a teleport request may fail or be deferred.
- Upstream `mod-guildhouse` uses a GM Island/area-specific guild phase scheme, including custom combined-phase behavior. Copying that directly into arbitrary races/settlements risks collisions with IP and 32-bit phase-mask limitations.

Source references:
- [MapMgr.cpp](https://github.com/azerothcore/azerothcore-wotlk/blob/master/src/server/game/Maps/MapMgr.cpp)
- [MapInstanced.cpp](https://github.com/azerothcore/azerothcore-wotlk/blob/master/src/server/game/Maps/MapInstanced.cpp)
- [Player.cpp](https://github.com/azerothcore/azerothcore-wotlk/blob/master/src/server/game/Entities/Player/Player.cpp)
- [mod-guildhouse](https://github.com/azerothcore/mod-guildhouse/blob/master/src/mod_guildhouse.cpp)

These are **upstream** source observations, not certification of the user's modified server core.

## Candidate A — reserved map instances

Promising because players/objects from different guilds would have true map-instance isolation, *if the installed core supports deterministic guild routing*. Challenges:
- No native guild-keyed `CreateInstanceForPlayer` route was found.
- Existing group/raid binds, difficulty, resets, instance saves and duplicate NPC spawns could interfere.
- Custom map metadata/client assets might be required for a wholly new housing map.
- A core patch might be necessary, which we should avoid unless the owner approves after a compatibility review.

## Candidate B — reserved-zone special phasing

Promising only if a narrow, version-matched namespace is possible and works with existing Individual Progression phase handling. Challenges:
- Finite phase masks and overlap across unknown numbers of guilds.
- Any direct player phase-mask overwrite may erase IP state; global WorldObject-phase overrides are dangerous.
- Guild membership changes, phase restoration and object ownership must work correctly on login/logout.
- Guild count beyond available masks must have a deterministic safe denial, not leaked shared visibility.

## Candidate C — nonoverlapping shared-world plots

Not private by itself. A user could see or reach another guild's construction, so **not approved** as the default solution.

## Required two-guild staging demonstration

1. Use a full copy of the actual deployed AzerothCore and all its modules.
2. Verify Guild A/B own independent property rows; stand in the *same logical site* simultaneously.
3. Check in both directions: player visibility, interactive NPCs, gameobjects, auras, collision, summons, Playerbots and gossip permissions.
4. Repeat with mixed IP tiers; no quest/vendor/raid lock bypass.
5. Test disconnect, logout in housing, worldserver restart, member kicked, guild disband and deleted property.
6. Disable/reinstall module and run safe return before removing compiled code; no player should be stranded.
7. Record exact core/module Git revisions and pass/fail evidence.

**Decision must remain unresolved until this demonstration is successful.** The new visit-record code supports future recovery but does not establish or simulate actual private map isolation.

## Upstream phase-comparison implementation — concrete risk

In current upstream `src/server/game/Entities/Object/Object.h`, `WorldObject::InSamePhase(uint32)` uses `m_useCombinedPhases ? (GetPhaseMask() & phasemask) : (GetPhaseMask() == phasemask)`. The initial value of `m_useCombinedPhases` is **true**, and `SetPhaseMask` dispatches `OnBeforeWorldObjectSetPhaseMask` before saving the new phase.

The upstream `mod-guildhouse` `GuildHouseGlobal` sets `useCombinedPhases=false` in zone 876 and true elsewhere, which changes visibility interpretation for that zone. The upstream Individual Progression module casts several named `IPP_PHASE` spells according to progression and region. This is a *potential interaction*, not proof the deployed forks conflict.

A combined mask permits at most **32 distinct single-bit slots**, and existing modules/auras can consume some or all of those; assigning `guildId % 32` guarantees collisions once sufficient guilds exist. The alternative exact-value mode can represent many phase IDs **only if every relevant object uses the same mode consistently** and all other installed scripts/auras cooperate. Even one object with a different comparison mode can produce one-way visibility. Do not use global phase overrides to solve guild property isolation without approval and staging evidence.

See [PHASE_COMPATIBILITY_AUDIT.md](PHASE_COMPATIBILITY_AUDIT.md) for the model and read-only audit tool. The **privacy isolation capability remains unverified/disabled**.

## Optional snapshot mechanism (not an isolation solution)

The development branch now contains a **compile-time opt-in**, GM-restricted, configuration-gated read-only snapshot command that reports only the issuing character's map ID, instance ID and phase mask alongside guild/area identifiers. It does not instantiate maps, change phases, grant property access or report another guild's data. See [STAGING_DIAGNOSTICS.md](STAGING_DIAGNOSTICS.md). This is evidence gathering infrastructure only, not real guild isolation.
