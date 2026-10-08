# Disable, uninstall or purge

There are **three different operations**. Back up the relevant databases before the second or third.

## A. Disable (data retained)

1. Arrange for players to exit the property; after a fully implemented release, verify automatic safe evacuation.
2. Set `NaxxGuildStrongholds.Enabled = 0` in the module config.
3. Restart the server.
4. Verify that guild stronghold interactions are unavailable, while the `naxx_gs_*` records still exist.

## B. Remove executable module, preserve data (preferred uninstall)

1. Stop the worldserver; ensure no characters are stranded in module-only locations.
2. Back up the character DB, world DB, binaries and configs.
3. Remove the module source directory from AzerothCore's active `modules` tree.
4. Reconfigure CMake **and** rebuild the server without it.
5. Remove or archive the module's local configuration file.
6. If future versions create NPCs or world spawns, use that version's **specific recorded-ownership cleanup script**, not blanket DELETE statements.
7. Verify character login, existing progression, Playerbots and normal zone phasing.
8. Leave `naxx_gs_*` tables intact so a later reinstall may restore progress.

Removing a module's folder without recompiling does not remove compiled code.

## C. Permanently delete module data (destructive, optional)

Only if no longer needed: stop the server, confirm an external backup, and review `uninstall/purge_characters.sql`. That SQL drops only the namespaced character-database tables from this module. **It permanently deletes owned settlement progress.** It is never run automatically by uninstall.

Use the correct character database explicitly, not `acore_world` or `acore_auth`.

## Before declaring uninstall supported

The playable release must test disable/re-enable, no stranded players, no leaked spawns, compile without module, preserving data through reinstall, and no impact on IP/Playerbots. These are **future tests**, not current guarantees.

## Construction-project tables (draft schema update)

The isolated, unapplied schema now includes `naxx_gs_project` and uniquely identified `naxx_gs_contribution` rows. Preserve-data uninstall leaves these tables untouched. Only optional `uninstall/purge_characters.sql` explicitly drops them **after backups and consent**; do not delete guild supplies or character inventory in order to remove code.


## Archived settlements

A normal disable or data-preserving uninstall leaves both active and archived properties, all decorative trophy and building records and the audit ledger intact. Guild disband does **not** trigger destructive purge. The optional SQL purge is separate, manual and backup-dependent. Never restore an archived property to a newly created guild simply because it reuses an old numeric ID. Actual lifecycle hooks remain unimplemented.


## Critical: outstanding `naxx_gs_visit` records

The draft module includes durable return tickets for potential private-area visitors. A future production uninstall **must evacuate and verify every player is safely returned before removing the compiled recovery hooks**, and preserve tickets until success. Do not purge `naxx_gs_visit` when players may still be inside housing. The optional destructive SQL purge is never automatically run, and a database backup alone cannot teleport a stranded character. The diagnostic development branch is not housing-capable and has no actual return hooks yet.
