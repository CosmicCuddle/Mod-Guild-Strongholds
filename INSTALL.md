# Installation policy (foundation branch)

**Do not install this foundation branch on a live server.** It registers no gameplay, has no NPC/world spawns and has not been compiled against the deployed AzerothCore fork.

## Before any future build or migration

1. Write down the AzerothCore commit hash, Playerbots revision and Individual Progression revision.
2. Record the currently working `worldserver` binary location and the currently deployed module revisions.
3. Stop `worldserver` before database schema migrations or module removal.
4. Take tested backups of the character and world databases and the deployed configuration and binaries. See `backup/README.md`.
5. Use a separate test environment where possible; do not install two guild privacy modules without a compatibility review.

## Planned module installation

After compatibility work and documented approval, the module will be cloned into AzerothCore's `modules` folder, CMake will be reconfigured and the worldserver rebuilt. `conf/mod_naxx_guild_strongholds.conf.dist` will be copied to the server config directory as a local, separately editable `.conf` file.

Settings default to disabled; only enable after successful isolation and safe-exit tests.

The draft SQL schema lives in `data/sql/manual/install_characters.sql`. It is **not** automatically applied from the normal AzerothCore updater directories. Review and back up the character database before applying an approved migration.

Do not run installation SQL on `acore_world` or `acore_auth`; it is intended for the character database only.

This document will be expanded with exact commands when the target AzerothCore and database configuration are known.

## Mandatory compatibility preflight

Before even a staging install, record **all modules actually deployed**, the AzerothCore commit, the Playerbots fork/revision, the IP module revision and custom core patches. Follow [COMPATIBILITY.md](docs/COMPATIBILITY.md). Do not remove, replace, or silently modify existing modules to make Strongholds build. A compile failure, phase conflict, SQL/entry ID collision or duplicated completion reward is a release blocker.
