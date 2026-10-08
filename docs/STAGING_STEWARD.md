# Guild Steward staging preview — read-only CreatureScript

**Status: source prototype only. Not installed, spawned or bound in game.**

This is the first actual AzerothCore `CreatureScript` created for Guild Strongholds. It prepares a future NPC interaction while keeping all housing gameplay permanently blocked in development.

## Two deliberate gates

1. Compile-time macro `NAXX_GS_BUILD_STAGING_STEWARD` must be explicitly present. It is **absent from normal worldserver builds**. When absent, there is no steward script registration.
2. Config `NaxxGuildStrongholds.StagingSteward.Enabled=1`, default `0`, must be enabled in a **separate staging realm**. Merely setting this config in a normal build does nothing.

Further restrictions: a staff-controlled player must have **GM mode** enabled and currently belong to a guild. The creature must have the explicitly reviewed `creature_template.ScriptName = 'npc_naxx_guild_steward_staging'` binding. No numeric creature ID is allocated or supplied in this repo. This is a test script name only; it is never attached to an existing server NPC automatically.

## Read-only interaction

The script exposes exactly:
- An overview stating the guild's ID and that housing is unavailable.
- A preview line about future guild buildings and raid trophies.
- Close.

It only sends gossip menus. It NEVER:
- claims/archives property, alters guild membership, or edits DB records;
- debits inventory, money, supplies or consumables;
- teaches spells or grants quests/trophies/rewards;
- sets phase masks, edits Individual Progression auras or changes maps;
- teleports characters or adds creatures/gameobjects.

`OnGossipSelect` revalidates GM mode, guild membership and config and rejects unexpected senders/actions, including fabricated packets. This is defensive even though no actionable gameplay exists yet.

## Developer testing

`StrongholdStewardPreview.h/.cpp` holds a pure deterministic permission/action gate. `tests/steward_preview_tests.cpp` covers config off by default, non-GMs, guildless users, approved actions, forged senders, unknown choices, and privileges revoked between gossip opening and selecting.

The CI matrix deliberately builds the real upstream AzerothCore `modules` target with `NAXX_GS_BUILD_STAGING_STEWARD` set only in a separate opt-in job, alongside the existing optional GM diagnostic. A normal build must have **neither** staging script registration symbol. This test compiles source; it does not execute a gossip conversation on a game realm.

## Real staging acceptance (not completed)

1. Back up the actual server code, characters/world SQL and binaries; use an isolated staging instance, not production.
2. Verify the deployed fork `CreatureScript`, `ScriptedGossip`, player/GM and NPC binding interfaces and available creature ID range.
3. Compile the stage opt-in code on that fork only. The feature master switch remains off.
4. With explicit permission, use an unused staging creature template and bind only this script name, using reviewed reversible SQL. Never repurpose an NPC or clone a production world table.
5. Check GM in guild, GM outside guild, normal guildmate, bot player, forged/unknown gossip selection, GM losing guild/GM mode after opening the menu, disable config, and empty menu.
6. Confirm zero housing, quests, phasing, DB writes, teleport or access-control changes. Remove the NPC and its binding, revert staged binary/config and verify backups.

**There are no provided NPC IDs, SQL statements, coordinates or actual world spawns. Do not install this draft on your live realm.**

## Rich catalogue-based preview — still read-only

The optional script now presents a main menu with Development Overview, Alliance Themes, Horde Themes, Human Buildings, Orc Buildings, Daily Activities, Weekly Activities, Raid Trophies, and Close. A detail page displays the relevant **compiled source catalogue**, never live property state:

- 5 Alliance and 5 Horde theme names/architecture descriptions.
- 6 Human and 6 Orc building plans, minimum settlement level and *planned* supply/timber/iron costs.
- 3 proposed daily activities, 4 weekly activities and 2 future one-time trophy requests, each labelled with planned guild level and IP milestone.
- Back and Close navigation; preview detail lines reopen only the same page and cannot perform any gameplay action.

**Every content entry explicitly says `[PLAN]` or `[LOCKED]`.** This is not a live IP adapter, real guild construction view, quest giver, vendor or trophy distributor. The implementation intentionally does **not** read current guild property rows, IP progression, quest flags or live inventory, and avoids falsely implying an achievement was earned. Future services must use the server-authoritative adapters and private-space gates before becoming accessible.

`tests/steward_readonly_content_tests.cpp` verifies catalogue linkage, 5/5 racial theme counts, six buildings per prototype race, three/four/two activity counts, stated minimum levels/required IP milestones, bounded gossip page size and planning-only wording. The existing selection-policy tests now cover all ten defined read-only menu actions, unknown/forged actions and revoked privileges.

This remains only an **explicitly compiled, staff-only staging script** with no NPC template, spawn, world SQL or live gameplay changes.
