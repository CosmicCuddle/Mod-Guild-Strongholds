# Read-only staging diagnostics — NOT a housing feature

**State: implemented in the development branch, mock-core tests only.**

## Why this is optional

To decide if a guild-private location can safely coexist with Individual Progression, Playerbots and every other installed module, we need the exact **deployed fork** to report players' actual map, instance and phase values. Upstream theory is insufficient.

The source file `src/StrongholdStagingDiagnostics.cpp` provides a staff-only `.naxxgs snapshot` command, but it is wrapped in the compile-time gate `NAXX_GS_BUILD_STAGING_DIAGNOSTICS`. **This flag is deliberately unset in normal builds.** When absent, the loader does not register the command.

If deliberately enabled on a staging build, the separate config `NaxxGuildStrongholds.Diagnostics.Enabled=1` must also be set; default is **0**. The command is restricted by `rbac::RBAC_PERM_COMMAND_DEBUG_INFO` and `Console::No`, following upstream AzerothCore's existing staff command patterns.

**Do not add this compile flag to production or change RBAC/database permissions just to enable the probe.** No normal production action is needed now.

## What it displays

Only data from the staff player who typed `.naxxgs snapshot`:

- guild ID (numeric)
- map and instance IDs
- zone and area IDs
- phase mask (numeric)
- an explicit warning that phase comparison mode is **UNKNOWN** and **NO PRIVACY APPROVAL** is implied

It never inspects another player's account, inventory, character name, IP tier or secrets. It does not log to an external service or write SQL. It does not teleport, modify phase masks, spawn gameobjects or create private instances.

## What it does NOT prove

Two characters reporting different phase masks does **not** prove privacy. The object may be in combined-bitmask or exact-value mode; some masks overlap, and GM/Playerbot/NPC visibility may differ. Real guild privacy also needs measured visibility in **both directions**, interactions with NPC/gameobjects, summons/auras, mixed Individual Progression tiers and safe return after logout/disband/restart.

The diagnostic is therefore never a way to change `DevelopmentCapabilities.PrivacyIsolation` from false.

## When we eventually test on an isolated staging realm

1. Make a full backup and identify the exact AzerothCore, Playerbots, IP and other module revisions.
2. Compare the command API and RBAC permission with the user's **deployed** fork. Compile/test the opt-in code **only on staging** if the real interfaces match.
3. Run with staff accounts, two unrelated guilds, and a reserved test area. Record each GM's snapshots before and after any **separately developed, reviewed** isolation test.
4. Independently record which players, creatures and gameobjects are visible and interactive for Guild A and Guild B, including both directions and mixed IP tiers.
5. Test emergency exit, logout, disband, reinstall and no cross-module gameplay changes.
6. Remove the compile flag and keep `Diagnostics.Enabled=0` outside the staging environment.

**Still missing:** real AzerothCore compilation, actual worldserver registration test, user fork compatibility audit, confirmed map/phase implementation and field-test evidence. No need for the user to run anything on the live server today.

## Undo

Remove the compile definition and rebuild the known-good staging binary; leave `NaxxGuildStrongholds.Diagnostics.Enabled=0`. This feature creates no persistent records or world objects. The rest of the module continues to be blocked from enabling housing.
