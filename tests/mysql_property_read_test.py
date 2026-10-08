#!/usr/bin/env python3
"""Read-only property SELECT contract on the CI-ONLY disposable MariaDB.

This fixture inserts records solely into protected naxx_gs_ci_test, then
checks the exact SELECT columns the guarded AzerothCore adapter will read.
Never runs on a live DB; connection() rejects any other DB name.
"""
from __future__ import annotations

from mysql_construction_test import INSTALL_SQL, PURGE_SQL, connection, statements

SQL = (
    "SELECT guild_id, guild_created_at, lifecycle_state, "
    "lifecycle_version, development_level "
    "FROM naxx_gs_settlement WHERE guild_id = %s LIMIT 1"
)


def read(guild_id):
    with connection() as db:
        with db.cursor() as cur:
            cur.execute(SQL, (guild_id,))
            return cur.fetchone()


def main():
    checked = 0

    def check(ok, name):
        nonlocal checked
        checked += 1
        if not ok:
            raise AssertionError(name)

    with connection() as db:
        with db.cursor() as cur:
            for cmd in statements(PURGE_SQL):
                cur.execute(cmd)
            for cmd in statements(INSTALL_SQL):
                cur.execute(cmd)
            cur.execute(
                "INSERT INTO naxx_gs_settlement "
                "(guild_id, guild_created_at, lifecycle_state, "
                "lifecycle_version, theme_key, development_level) "
                "VALUES (52, 1790000500, 'active', 2, 'human', 4)"
            )
            cur.execute(
                "INSERT INTO naxx_gs_settlement "
                "(guild_id, guild_created_at, lifecycle_state, "
                "lifecycle_version, theme_key, development_level) "
                "VALUES (53, 1790000501, 'archived', 7, 'orc', 6)"
            )
        db.commit()

    check(read(52) == (52, 1790000500, "active", 2, 4),
          "Exact active row columns return in expected order")
    check(read(53) == (53, 1790000501, "archived", 7, 6),
          "Archived row is reported rather than invisibly recreated")
    check(read(54) is None, "Missing guild cannot claim another guild's row")
    check(read(0) is None, "Guild zero has no property")

    with connection() as db:
        with db.cursor() as cur:
            cur.execute(
                "UPDATE naxx_gs_settlement "
                "SET guild_created_at=1790000600 WHERE guild_id=52"
            )
        db.commit()
    check(read(52)[1] == 1790000600,
          "Persisted guild generation is returned for validation, not inferred")

    before = read(53)
    for _ in range(3):
        check(read(53) == before, "Repeated SELECT does not mutate property")
    with connection() as db:
        with db.cursor() as cur:
            for cmd in statements(INSTALL_SQL):
                cur.execute(cmd)
        db.commit()
    check(read(53) == before, "Reinstall preserves archived record")

    print(f"PASS: {checked} disposable read-only settlement SELECT checks")


if __name__ == "__main__":
    main()
