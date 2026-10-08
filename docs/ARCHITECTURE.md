# Technical architecture and unresolved research

## Scope

Separate AzerothCore 3.3.5 module: `mod-guild-strongholds`. Use namespaced C++ symbols, isolated configuration keys (`NaxxGuildStrongholds.*`) and dedicated character tables (`naxx_gs_*`). Avoid core-file patches unless an audited capability gap makes them unavoidable.

## Critical unsolved problem: private locations

The upstream `azerothcore/mod-guildhouse` has specialised guild phasing for a specific reserved zone, not a proven general-purpose solution for ten arbitrary properties. **Never assume ordinary 32-bit phase-mask values can represent unlimited private guilds.** A property preview is not proof of safe isolation.

Before production housing:
1. Determine the deployed AzerothCore revision, module registration convention and its available instance/phasing hooks.
2. Prototype two guilds using the same location; players, NPCs and objects must not leak across guild boundaries.
3. Confirm teleport entry/exit, summon behavior, logout/login and character restoration.
4. Verify that Individual Progression phasing remains unchanged outside the property and that other existing modules do not conflict.
5. Reject property deployment until isolation and safe-exit tests pass.

The module will **not** copy the upstream Guild House code without reviewing compatibility and licensing.

## Data ownership

The draft manual schema in `data/sql/manual/install_characters.sql` creates only `naxx_gs_*` tables in the character database. It does not change `guild`, `characters`, quest tables or IP-module tables.

- `naxx_gs_settlement`: selected theme, development level, guild resources.
- `naxx_gs_building`: chosen building for each plot, construction stage.
- `naxx_gs_unlock`: permanently earned cosmetic/achievement keys.
- `naxx_gs_decoration`: a guild-owned object assigned to a designated slot.
- `naxx_gs_contribution`: auditable guild-member contributions.
- `naxx_gs_ledger`: persistent history of milestones and changes.

No tables should be created merely because the repository was cloned during the foundation stage; the SQL is stored outside automatic AzerothCore DB updater directories on purpose.

## Integration boundaries

- Individual Progression: **read-only adapter**, with the exact installed API and stage identifiers determined from deployed source. Never infer tier from level alone.
- Playerbots: optional adapter, disabled until compatibility is demonstrated.
- Naxxramas Core: no direct dependency in the foundational stub.
- Gameobject templates: use validated 3.3.5 entries. Existing buildings embedded in the client map cannot be simply removed with SQL.
- NPC entry IDs and world spawns: must be reserved, tracked and removable; none allocated in this version.

## Security and permissions

- All mutations validate guild membership on the server, not client input.
- Construction permissions should be assigned to guild leader and explicitly selected ranks.
- Validate faction, plot ownership, cost balance, rate limits and IP eligibility before committing changes.
- Use transactional resource deduction and construction updates when implemented.
- Reject untrusted client requests to spawn arbitrary objects or teleport to arbitrary coordinates.

## Acceptance gates

**Foundation:** documents and inert loader reviewed. (No playable features.)

**First playable prototype:** one private test location, two-guild isolation, safe entrance/exit, isolated tables, restart persistence and fully tested disable/uninstall/reinstall.

**Later:** dual faction architecture, dailies/weeklies, raid hooks and Personal IP gating. Live installation remains prohibited until the necessary gates pass.
