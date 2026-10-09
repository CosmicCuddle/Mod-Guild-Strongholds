#!/usr/bin/env python3
"""CI-only guild-property ownership contract tests on disposable MariaDB.

This is not the worldserver production adapter or client guild verification.
"""
from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor

import pymysql

from mysql_construction_test import INSTALL_SQL, PURGE_SQL, connection, statements

THEMES = {"human": "alliance", "orc": "horde"}


def clear_schema():
    with connection() as c:
        with c.cursor() as cur:
            for sql in statements(PURGE_SQL):
                cur.execute(sql)
            for sql in statements(INSTALL_SQL):
                cur.execute(sql)
        c.commit()


def claim(*, guild_id: int, actor_guild_id: int, leader: bool,
          guild_faction: str, requested_theme: str, guild_created_at: int,
          bot: bool = False,
          failpoint: bool = False):
    # These values are test fixtures. Live adapter MUST read faction,
    # guild membership/rank and bot status from the server's Player/Guild.
    if guild_id <= 0 or actor_guild_id != guild_id or not leader or bot or guild_created_at <= 0:
        return "unauthorized"
    if THEMES.get(requested_theme) != guild_faction:
        return "invalid_theme"

    c = connection()
    try:
        c.begin()
        with c.cursor() as cur:
            # Primary key on settlement.guild_id is the final protection
            # against two simultaneous property claims for one guild.
            try:
                cur.execute(
                    "INSERT INTO naxx_gs_settlement "
                    "(guild_id, guild_created_at, theme_key, development_level, guild_supplies) "
                    "VALUES (%s, %s, %s, 1, 0)",
                    (guild_id, guild_created_at, requested_theme)
                )
            except pymysql.err.IntegrityError as exc:
                if exc.args[0] != 1062:
                    raise
                c.rollback()
                return "already_owned"

            if failpoint:
                raise RuntimeError("SIMULATED FAILURE after property claim")

            cur.execute(
                "INSERT INTO naxx_gs_ledger (guild_id, event_key, details) "
                "VALUES (%s, 'settlement_claimed', %s)",
                (guild_id, requested_theme)
            )
        c.commit()
        return "accepted"
    except Exception:
        c.rollback()
        raise
    finally:
        c.close()


def read_claim(guild_id: int):
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "SELECT theme_key FROM naxx_gs_settlement WHERE guild_id=%s",
                (guild_id,)
            )
            result = cur.fetchone()
            cur.execute(
                "SELECT COUNT(*) FROM naxx_gs_ledger "
                "WHERE guild_id=%s AND event_key='settlement_claimed'",
                (guild_id,)
            )
            ledger = cur.fetchone()[0]
            return (result[0] if result else None, ledger)


def main():
    clear_schema()
    checks = 0

    def expect(condition, reason):
        nonlocal checks
        checks += 1
        if not condition:
            raise AssertionError(reason)

    parameters = dict(guild_id=100, actor_guild_id=100, leader=True,
                      guild_faction="alliance", requested_theme="human", guild_created_at=1790000000)

    expect(claim(**{**parameters, "guild_created_at": 0}) == "unauthorized",
           "unverified guild date is rejected")
    expect(claim(**{**parameters, "leader": False}) == "unauthorized",
           "guildmaster required")
    expect(claim(**{**parameters, "bot": True}) == "unauthorized",
           "bot cannot claim property")
    expect(claim(**{**parameters, "actor_guild_id": 101}) == "unauthorized",
           "foreign guild cannot claim property")
    expect(claim(**{**parameters, "requested_theme": "orc"}) == "invalid_theme",
           "cross-faction theme denied")
    expect(read_claim(100) == (None, 0), "rejected claims had no side effects")

    try:
        claim(**parameters, failpoint=True)
    except RuntimeError:
        pass
    else:
        raise AssertionError("Injected failure didn't happen")
    expect(read_claim(100) == (None, 0), "failed claim and ledger must roll back")

    with ThreadPoolExecutor(max_workers=2) as pool:
        results = list(pool.map(lambda _: claim(**parameters), range(2)))
    expect(sorted(results) == ["accepted", "already_owned"],
           "two simultaneous claims produce one owner")
    expect(read_claim(100) == ("human", 1), "one property and one history entry")
    expect(claim(**parameters) == "already_owned", "repeated claim cannot double-spend")
    expect(claim(**{**parameters, "guild_created_at": 1791111111}) == "already_owned",
           "guild ID reuse cannot overwrite saved property")
    expect(read_claim(100) == ("human", 1), "repeated claim cannot double-log")

    horde = dict(guild_id=200, actor_guild_id=200, leader=True,
                 guild_faction="horde", requested_theme="orc", guild_created_at=1790000001)
    expect(claim(**horde) == "accepted", "Horde guild can claim separate property")
    expect(read_claim(200) == ("orc", 1), "Horde guild independent")
    expect(read_claim(100) == ("human", 1), "Alliance guild unaffected")

    with connection() as c:
        with c.cursor() as cur:
            for sql in statements(INSTALL_SQL):
                cur.execute(sql)
        c.commit()
    expect(read_claim(100) == ("human", 1), "same-schema reinstall preserves Alliance property")
    expect(read_claim(200) == ("orc", 1), "same-schema reinstall preserves Horde property")

    # Test database is disposable, so leaving two example records is safe.
    print(f"PASS: {checks} MariaDB guild property ownership contract checks (CI only)")


if __name__ == "__main__":
    main()
