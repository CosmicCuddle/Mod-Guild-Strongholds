# Settlement catalogue — preliminary v0.2.0 design

**Current status:** logical definitions and pure C++ tests only. There are no map coordinates, NPCs, gameobjects, client patches, teleports, quests or live construction systems.

## Ten racial themes (3.3.5)

| Theme key | Faction | Working name | Visual reference |
| --- | --- | --- | --- |
| `human` | Alliance | Royal Stronghold | Stone keep, courtyard and barracks |
| `dwarf` | Alliance | Mountain Hold | Heavy stone hall, smithy, forge |
| `night_elf` | Alliance | Moonwood Sanctuary | Forest, moonwell and raised walkways |
| `gnome` | Alliance | Mechanist Enclave | Machines, gears and workshops |
| `draenei` | Alliance | Crystal Refuge | Crystal buildings and blue glows |
| `orc` | Horde | Warlord's Fortress | Timber palisade and training ring |
| `troll` | Horde | Darkspear Village | Tiki masks, thatch and jungle wood |
| `tauren` | Horde | Ancestral Encampment | Plains tents, leather and totems |
| `undead` | Horde | Forsaken Bastion | Gothic stone, crypts and apothecary |
| `blood_elf` | Horde | Sunspire Estate | Red-gold towers and arcane gardens |

A guild may choose a racial theme within **its faction**, regardless of guild leader's race. This does not give the guild's lower-IP-tier members additional expansion gameplay; services and quests stay tier-gated.

## First two logical plot layouts

The `human` and `orc` prototypes each define exactly six plot identifiers, which are stable storage keys. The IDs are design-level keys, **not world DB spawn IDs**.

| Plot ID | Unlock at settlement level | Human proposal | Orc proposal |
| --- | ---: | --- | --- |
| `hall` | 1 | Stone guild hall | Orcish war hall |
| `military` | 2 | Training yard | Sparring arena |
| `crafting` | 2 | Workshop court | Forge camp |
| `social` | 3 | Tavern garden | Feasting circle |
| `prestige` | 4 | Hall of legends | Victory totems |
| `utility` | 5 | Stables and services | Supply grounds |

The remaining settlement levels 6 and 7 would deepen buildings and decorations rather than introduce additional base plots automatically.

## Required build-object research before implementation

For each race and stage, identify actual 3.3.5 client-visible gameobject or creature display IDs, physical collision, map geometry, permitted map coordinates and an appropriately sized unused footprint.

Do not use existing quest hubs as if private. Do not place shared-world spawns into occupied public areas. Do not invent 3.3.5 display or spawn IDs; verify each asset from a clean test client and DB.

## Construction sequence

1. Blank reserved plot.
2. Supplies and scaffolding model where available.
3. Worker NPCs and construction progress.
4. Finished race-styled building; later decorative upgrades.
5. Decorations are owned by guild and recoverable after moving/replacing their display slots.

The `FindPrototypeLayout` function currently returns layouts only for `human` and `orc`. Other themes are registered in the catalogue for planning but remain non-playable until tested layouts exist.

## Individual Progression contract

Option B is final: a **single common settlement scene** for all authorised guild members, with the services, quest offerings and activities decided from **each character's** IP tier. Read IP status through a verified, read-only adapter from the actually deployed mod-individual-progression fork. **No personal tier-specific visual phasing in v0.2.0.**

## Next feasibility decision

Research, then prove **real privacy**, safe teleport in/out and overlap-free construction. Do not wire an entry NPC or a player-facing preview before a secure location instance/phasing design is implemented and tested against existing modules.
