# Guild isolation compatibility audit — staging preparation

**Development status:** Only independent C++ visibility-model tests and a read-only Python source scanner exist. **No private guild map/phase implementation**, and zero deployment compatibility tests have occurred.

## Confirmed upstream behaviour

The current upstream AzerothCore implementation exposes two phase-comparison modes:

```cpp
// WorldObject::InSamePhase(uint32 phasemask), simplified:
return m_useCombinedPhases ? (GetPhaseMask() & phasemask) != 0
                           : GetPhaseMask() == phasemask;
```

It is **directional**: it uses the *observer's* mode when comparing against another mask. An object with exact mode and phase value 3 cannot see an object in combined mode with mask 1; the second object **can** see the first (1 & 3). This is a potential one-way visibility leak. Any test must verify both A→B and B→A for player, creature and gameobject observations.

Upstream source references:
- [Object.h](https://github.com/azerothcore/azerothcore-wotlk/blob/master/src/server/game/Entities/Object/Object.h)
- [Object.cpp — SetPhaseMask](https://github.com/azerothcore/azerothcore-wotlk/blob/master/src/server/game/Entities/Object/Object.cpp)
- [mod-guildhouse — GuildHouseGlobal](https://github.com/azerothcore/mod-guildhouse/blob/master/src/mod_guildhouse.cpp)
- [Individual Progression](https://github.com/ZhengPeiRu21/mod-individual-progression/blob/main/src/IndividualProgression.cpp)

### Limits to consider

- Standard combined bitmasks support a maximum of **32 disjoint one-bit slots** across *all* consumers, including existing modules and gameplay.
- Simple `guildId` hashing/modulo 32 would **reuse** a bit for multiple guilds and is not private.
- `PHASEMASK_ANYWHERE` (all bits) can intersect multiple guild masks; privileged/GM visibility needs explicit tests.
- Exact phase values offer a different namespace, but need consistent object-specific interpretation and auditing of global phase hooks.
- True different map instance IDs separate world objects in the model; actually routing a guild reliably to the right persistent map instance is still unsolved.

## New synthetic C++ tests

`src/StrongholdIsolationProbe.h/.cpp` models directional phase checks and same-map/instance locality. `tests/isolation_probe_tests.cpp` covers:
- 2 guilds, each with player, creature and gameobject, with mutually visible guildmates and invisible outsiders;
- same-bit collision, all-phase mask and missing observations;
- misconfigured NPC phase and different instance ID;
- **asymmetric** phase modes, including one-way visibility;
- exact-value mode and combined-bitmask mode.

These prove only that our expectations and phase math are internally consistent. They **cannot** be used to set the runtime `PrivacyIsolation` startup flag.

## Optional read-only module source audit (do not run yet)

When ready to check a *staging copy* or source checkout of the real server, use:

```bash
python3 modules/Mod-Guild-Strongholds/scripts/audit-isolation-compatibility.py /path/to/azerothcore
```

Replace both paths to reflect actual checkout and module location. The command prints a concise list of **module folder names, independent Git SHAs, and counts of relevant source-code patterns**. No credentials, `.conf` content, character data or module source text is printed; no files are changed or transmitted.

To inspect example file paths instead:

```bash
python3 modules/Mod-Guild-Strongholds/scripts/audit-isolation-compatibility.py /path/to/azerothcore --json
```

The scanner detects potential uses of `OnBeforeWorldObjectSetPhaseMask`, `SetPhaseMask`, `IPP_PHASE` constants, `TeleportTo`, instance routing and guild-disband hooks. It deliberately emits **`REVIEW_REQUIRED` every time**, even if all counts are zero. Results can be incomplete (CMake selection, aliases, inherited methods, phase spells, nonstandard source paths, custom core patches and DB-only changes). It is **not** a conflict detector or automatic compatibility approval.

No need to run this on the user's server during documentation/source development.

## Acceptance record (MUST be filled from actual staging test)

| Requirement | Evidence | Result |
| --- | --- | --- |
| Exact core SHA and deployed module SHA inventory | To be recorded | NOT TESTED |
| Verified private guild routing, A/B same logical plot | To be recorded | NOT TESTED |
| Players, creatures and objects A→B and B→A invisible | To be recorded | NOT TESTED |
| All within-guild visibility functional | To be recorded | NOT TESTED |
| IP phasing and per-character gated NPC/quest unaffected | To be recorded | NOT TESTED |
| Playerbots/GM/combat/summons/death/logout/restart | To be recorded | NOT TESTED |
| Safe return for failed teleport and member removed/disbanded | To be recorded | NOT TESTED |
| Disable, evacuation, uninstall and reinstall | To be recorded | NOT TESTED |

Any failure or missing evidence blocks the live release. See [ISOLATION_DECISION.md](ISOLATION_DECISION.md) and [COMPATIBILITY.md](COMPATIBILITY.md).
