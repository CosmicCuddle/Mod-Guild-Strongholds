# Backups — operator checklist

Before running any migration on the server:

1. Identify the *actual* character and world database names from the deployed configuration; do not assume they are both named `acore_*`.
2. Save the active AzerothCore commit, module commit, config files and a matching known-good compiled `worldserver`.
3. Stop worldserver for a consistent maintenance-window snapshot, unless a tested transactional backup method is in place.
4. Dump both affected databases with a correctly configured `mysqldump` account. Avoid putting passwords in command-line arguments or version-controlled files.
5. Verify backup file size/format and, ideally, restore the backup on a test database.
6. Keep a second backup copy away from the server.

Illustrative commands (replace DB names and use your local secured MySQL login configuration):

```bash
mkdir -p "$HOME/azerothcore-backups"
mysqldump --single-transaction --routines --triggers acore_characters > "$HOME/azerothcore-backups/characters-before-strongholds.sql"
mysqldump --single-transaction --routines --triggers acore_world > "$HOME/azerothcore-backups/world-before-strongholds.sql"
```

These are **examples only**, not commands to run blindly. The `--single-transaction` option assumes transactional tables; it does not guarantee consistency for non-transactional tables or cross-database changes. Never commit dumps, passwords, secret config files or player data to GitHub.
