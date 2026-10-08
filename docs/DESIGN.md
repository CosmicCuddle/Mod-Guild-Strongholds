# Gameplay Design — v0.3 (agreed direction)

## Guild property

One property per guild, chosen from prepared, private sites. The guild's faction determines eligible styles; the guild leader's race does not restrict the style. Any Alliance guild can select Human, Dwarf, Night Elf, Gnome or Draenei. Any Horde guild can select Orc, Troll, Tauren, Undead or Blood Elf.

Each property has a common set of functional plots but race-appropriate structures, NPC costumes and furniture. The first layout prototypes are **Human** and **Orc**; the other eight are planned.

A property may be relocated later without losing unlocked trophies, construction resources or its event history. Relocation is out of scope for the first implementation.

## Shared housing + personal Individual Progression (Option B)

All authorised guild visitors see the same shared, completed settlement. Every interaction that offers a quest, vendor, service, expedition or tier-specific reward must check the **interacting character's** actual Individual Progression eligibility.

A progressed guild member must not enable locked-tier gameplay for an unprogressed member. Guild trophies remain visible to visitors even when they cannot personally accept a related activity.

Do not use personal quest progression as the isolation mechanism for the guild itself. For the first playable build, use tier-gated gossip, quests and services; do not attempt separate per-character settlement scenery.

## Seven proposed construction stages

1. Founding Camp
2. Established Outpost
3. Growing Village
4. Fortified Settlement
5. Guild Stronghold
6. Grand Stronghold
7. Legendary Stronghold

Stages are **not** IP tiers. Settlement progress is guild-wide; IP eligibility is checked per character.

## Construction

A building plot selects one structure from its allowed group (for example workshop, barracks, gardens, trophy hall). Each construction project has explicit costs, progress and visual milestones: empty plot -> supplies/scaffolding -> workers -> completed structure. Preserve the previous decoration ownership when replacing or moving a building.

Free-form decoration placement is a later investigation. Start with named furniture/display slots and validated gameobject templates.

## Repeatable activity

- Daily: resource deliveries, crafting requests, appropriate open-world tasks and tier-eligible dungeon objectives.
- Weekly: collaborative construction projects, guild expeditions and raid milestones.
- Both: settlement supplies and development experience, with personal credit where appropriate.
- Small guilds must progress meaningfully without mandatory daily attendance.
- Playerbot contributions will be capped and require clear player-initiated eligibility to prevent automated farming.

## Raid trophies and historical ledger

Qualifying guild defeats unlock permanent cosmetic objects (example: Onyxia's head; Molten Core monument; Ahn'Qiraj relics). Keep achievement unlocks and chosen display positions as separate data. Record historical accomplishments, project completions and resource contributions.

Trophy credit must respect the server's raid eligibility and Individual Progression rules. Exact encounter hooks and playerbot participation thresholds remain to be designed.

## Backward compatibility

A disable should stop interactions while preserving module records. Re-enabling should restore them. A normal uninstall preserves data. A separate, explicitly requested purge deletes only tables owned by this module. See UNINSTALL.md and ROLLBACK.md.
