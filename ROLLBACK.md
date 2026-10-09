# Rollback and recovery

**Not yet tested against the user's live AzerothCore setup.**

## Preferred: targeted rollback

1. Stop the worldserver and back up the **current state**, even if the update failed.
2. Identify the last known-good module Git commit and matching core binary/config.
3. If a module version changed the SQL schema, use **that version's reviewed reverse migration**. Do not guess which columns to drop.
4. Rebuild the older compatible version (or restore the matched known-good compiled binary) and restore the old configuration.
5. Start the server and verify housing, unrelated characters/guilds, IP tiers and Playerbots.
6. Keep a copy of failed logs and new backup for diagnosis.

## Emergency: full restore

If targeted rollback cannot repair the deployment, restore the matching full database backup and matching core/module binaries and configuration.

**Warning:** A full backup restore may remove unrelated character progression and activity that occurred after the snapshot. Coordinate the outage and verify backup integrity before restoring.

## Migration rules

- Every future schema upgrade must ship with a forward migration and a tested reversal or a documented backup/restore prerequisite.
- Use dedicated `naxx_gs_*` tables where practical; never assume Git checkout reverses SQL.
- Keep recorded ownership for every module NPC and object, so only module-owned world changes are cleaned up.
- Never forcibly reset server-wide phase or character IP values during uninstall.
