#!/usr/bin/env python3
"""Staging-only MariaDB transaction contract tests.

No AzerothCore integration, player inventory, actual housing, or live DB writes.
The runner REFUSES databases except naxx_gs_ci_test.
Install: pip install PyMySQL==1.1.1
"""
from __future__ import annotations

import os
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import pymysql

ROOT = Path(__file__).resolve().parents[1]
TEST_DATABASE = "naxx_gs_ci_test"
INSTALL_SQL = ROOT / "data/sql/manual/install_characters.sql"
PURGE_SQL = ROOT / "uninstall/purge_characters.sql"


class Rejected(Exception):
    pass


def connection():
    if os.environ.get("NAXX_GS_CI_ONLY") != "YES":
        raise RuntimeError("Refusing SQL tests without NAXX_GS_CI_ONLY=YES")
    name = os.environ.get("NAXX_GS_TEST_DB", "")
    if name != TEST_DATABASE:
        raise RuntimeError(f"Refusing SQL tests against database {name!r}")
    c = pymysql.connect(
        host=os.environ.get("NAXX_GS_TEST_HOST", "127.0.0.1"),
        port=int(os.environ.get("NAXX_GS_TEST_PORT", "3306")),
        user=os.environ.get("NAXX_GS_TEST_USER", "root"),
        password=os.environ.get("NAXX_GS_TEST_PASSWORD", ""),
        database=name,
        charset="utf8mb4",
        autocommit=False,
        connect_timeout=10,
    )
    with c.cursor() as cur:
        cur.execute("SELECT DATABASE()")
        if cur.fetchone()[0] != TEST_DATABASE:
            c.close()
            raise RuntimeError("Wrong database selected")
    return c


def statements(path: Path):
    """Only for the controlled draft DDL, not arbitrary SQL parsing."""
    return [
        fragment.strip()
        for fragment in "\n".join(
            line for line in path.read_text(encoding="utf-8").splitlines()
            if not line.lstrip().startswith("--")
        ).split(";")
        if fragment.strip()
    ]


def reset_fixtures():
    with connection() as c:
        with c.cursor() as cur:
            for sql in statements(PURGE_SQL):
                cur.execute(sql)
            for sql in statements(INSTALL_SQL):
                cur.execute(sql)

            cur.execute(
                "INSERT INTO naxx_gs_settlement (guild_id, guild_created_at, theme_key, development_level, guild_supplies) "
                "VALUES (42, 1790000000, 'human', 2, 250), (84, 1790000000, 'orc', 2, 150)"
            )
            cur.execute(
                "INSERT INTO naxx_gs_project (guild_id, plot_key, project_key) "
                "VALUES (42, 'crafting', 'human_crafting'), (84, 'crafting', 'orc_crafting')"
            )
        c.commit()


def read_state(guild_id):
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "SELECT s.guild_supplies, p.supplies_contributed, p.version "
                "FROM naxx_gs_settlement AS s "
                "JOIN naxx_gs_project AS p ON p.guild_id = s.guild_id "
                "WHERE s.guild_id = %s AND p.plot_key = 'crafting'", (guild_id,)
            )
            state = cur.fetchone()
            cur.execute(
                "SELECT COUNT(*) FROM naxx_gs_contribution WHERE guild_id = %s", (guild_id,)
            )
            receipts = cur.fetchone()[0]
            cur.execute("SELECT COUNT(*) FROM naxx_gs_ledger WHERE guild_id = %s", (guild_id,))
            ledger = cur.fetchone()[0]
            return (*state, receipts, ledger)


def _deposit_supply(guild_id, actor_guild_id, actor_authorized, receipt, units,
                   *, actor_created_at=None, failpoint=None, is_bot=False):
    """Illustrate the REQUIRED atomic DB contract for virtual Guild Supplies.

    Caller must provide *server-verified* guild ID and permission (faked in tests).
    This is NOT a production API and CANNOT charge WoW character inventory.
    """
    if not actor_authorized or actor_guild_id != guild_id or is_bot or not actor_created_at:
        return "unauthorized"
    if (not isinstance(units, int) or isinstance(units, bool) or units < 1
            or units > 200):
        return "invalid_units"
    if (not isinstance(receipt, str) or not 1 <= len(receipt) <= 64
            or not all(x.isascii() and (x.isalnum() or x in "-_") for x in receipt)):
        return "invalid_receipt"

    c = connection()
    try:
        c.begin()
        with c.cursor() as cur:
            # A single guild-level row lock serializes all guild-supply debits.
            cur.execute(
                "SELECT theme_key, development_level, guild_supplies, guild_created_at, lifecycle_state "
                "FROM naxx_gs_settlement WHERE guild_id = %s FOR UPDATE",
                (guild_id,)
            )
            owner = cur.fetchone()
            if owner is None:
                raise Rejected("unknown_guild")
            theme, level, balance, owner_created_at, lifecycle_state = owner
            if owner_created_at != actor_created_at:
                raise Rejected("wrong_generation")
            if lifecycle_state != "active":
                raise Rejected("property_archived")
            cur.execute(
                "SELECT project_key, supplies_contributed, version "
                "FROM naxx_gs_project "
                "WHERE guild_id = %s AND plot_key = 'crafting' FOR UPDATE",
                (guild_id,)
            )
            project = cur.fetchone()
            if project is None:
                raise Rejected("unknown_project")
            project_key, delivered, version = project
            expected = theme + "_crafting"
            if level < 2 or project_key != expected or delivered > 200:
                raise Rejected("invalid_project")
            # The lookup helps classify retries. Uniqueness on (guild_id, receipt_key)
            # is the definitive constraint, not this preliminary query.
            cur.execute(
                "SELECT id FROM naxx_gs_contribution "
                "WHERE guild_id = %s AND receipt_key = %s", (guild_id, receipt)
            )
            if cur.fetchone():
                raise Rejected("duplicate")
            if delivered + units > 200:
                raise Rejected("overdelivery")
            if balance < units:
                raise Rejected("insufficient_supplies")

            cur.execute(
                "UPDATE naxx_gs_settlement SET guild_supplies = guild_supplies - %s "
                "WHERE guild_id = %s AND guild_supplies >= %s",
                (units, guild_id, units)
            )
            if cur.rowcount != 1:
                raise Rejected("insufficient_supplies")
            if failpoint == "after_debit":
                raise RuntimeError("SIMULATED FAILURE after debit")

            cur.execute(
                "UPDATE naxx_gs_project SET supplies_contributed = %s, version = version + 1 "
                "WHERE guild_id = %s AND plot_key = 'crafting' AND version = %s",
                (delivered + units, guild_id, version)
            )
            if cur.rowcount != 1:
                raise Rejected("version_conflict")
            if failpoint == "after_project_update":
                raise RuntimeError("SIMULATED FAILURE after project update")

            # Production must replace these fake actor IDs with authoritative GUIDs.
            cur.execute(
                "INSERT INTO naxx_gs_contribution "
                "(guild_id, player_guid, activity_key, receipt_key, units) "
                "VALUES (%s, 12345, %s, %s, %s)",
                (guild_id, project_key, receipt, units)
            )
            if failpoint == "after_receipt_insert":
                raise RuntimeError("SIMULATED FAILURE after receipt insert")
            cur.execute(
                "INSERT INTO naxx_gs_ledger (guild_id, event_key, actor_guid, details) "
                "VALUES (%s, 'construction_supply_deposit', 12345, %s)",
                (guild_id, receipt)
            )
            if failpoint == "after_ledger_insert":
                raise RuntimeError("SIMULATED FAILURE after ledger insert")
        c.commit()
        return "accepted"
    except Rejected as exc:
        c.rollback()
        return str(exc)
    except Exception:
        c.rollback()
        raise
    finally:
        c.close()


def run_tests():
    tests = 0

    def check(condition, description):
        nonlocal tests
        tests += 1
        if not condition:
            raise AssertionError(description)

    reset_fixtures()
    # Helper inserts explicit TEST-only server identity into every simulated request.
    def deposit_supply(*args, **kwargs):
        kwargs.setdefault("actor_created_at", 1790000000)
        return _deposit_supply(*args, **kwargs)

    check(read_state(42) == (250, 0, 0, 0, 0), "initial state")
    check(deposit_supply(42, 99, True, "foreign", 10) == "unauthorized", "foreign guild rejected")
    check(deposit_supply(42, 42, True, "reused", 10, actor_created_at=1791111111) ==
          "wrong_generation", "new guild with same ID cannot donate")
    check(deposit_supply(42, 42, False, "norank", 10) == "unauthorized", "unauthorized guild rank rejected")
    check(deposit_supply(42, 42, True, "bot", 10, is_bot=True) == "unauthorized", "bot rejected")
    check(deposit_supply(42, 42, True, "invalid", -1) == "invalid_units", "invalid amount rejected")
    check(deposit_supply(42, 42, True, "has spaces", 5) == "invalid_receipt", "invalid receipt rejected")
    check(read_state(42) == (250, 0, 0, 0, 0), "rejections have no side effects")

    for at in ("after_debit", "after_project_update",
               "after_receipt_insert", "after_ledger_insert"):
        try:
            deposit_supply(42, 42, True, at, 40, failpoint=at)
        except RuntimeError:
            pass
        else:
            raise AssertionError("Failure injection should throw")
        check(read_state(42) == (250, 0, 0, 0, 0), f"atomic rollback at {at}")

    check(deposit_supply(42, 42, True, "once", 100) == "accepted", "first transaction commits")
    check(read_state(42) == (150, 100, 1, 1, 1), "debit, progress, version, receipt and audit commit together")
    check(deposit_supply(42, 42, True, "once", 100) == "duplicate", "idempotent replay")
    check(read_state(42) == (150, 100, 1, 1, 1), "replayed receipt has no side effects")

    # Concurrency: one unique receipt reused by simultaneous submitters.
    with ThreadPoolExecutor(max_workers=2) as pool:
        results = list(pool.map(
            lambda _: deposit_supply(42, 42, True, "race_same", 40), range(2)
        ))
    check(sorted(results) == ["accepted", "duplicate"], "same-receipt race credits once")
    check(read_state(42) == (110, 140, 2, 2, 2), "same-receipt race stayed atomic")

    # Distinct requests serialize and both can commit when affordable.
    with ThreadPoolExecutor(max_workers=2) as pool:
        results = list(pool.map(
            lambda n: deposit_supply(42, 42, True, f"race_{n}", 25), range(2)
        ))
    check(results.count("accepted") == 2, "distinct receipts both complete")
    check(read_state(42) == (60, 190, 4, 4, 4), "concurrent deposits correctly serialized")
    check(deposit_supply(42, 42, True, "too_much", 20) == "overdelivery", "overdelivery denied")
    check(read_state(42) == (60, 190, 4, 4, 4), "overdelivery unchanged")

    check(deposit_supply(84, 84, True, "orc_first", 40) == "accepted", "separate guild contributes")
    check(read_state(84) == (110, 40, 1, 1, 1), "other guild's progress remains separate")
    check(read_state(42) == (60, 190, 4, 4, 4), "first guild remains unchanged")
    check(deposit_supply(42, 42, True, "final", 10) == "accepted", "final supply deposit")
    check(read_state(42) == (50, 200, 5, 5, 5), "supply portion fully funded")

    # A second DB connection observes the committed state (restart analogue).
    c = connection()
    try:
        with c.cursor() as cur:
            cur.execute(
                "SELECT COUNT(*) FROM information_schema.tables "
                "WHERE table_schema = DATABASE() AND table_name LIKE 'naxx\\_gs\\_%'"
            )
            check(cur.fetchone()[0] == 7, "exactly seven isolated module tables")
            cur.execute(
                "SELECT COUNT(*) FROM information_schema.statistics "
                "WHERE table_schema = DATABASE() AND table_name='naxx_gs_contribution' "
                "AND index_name='uk_guild_receipt'"
            )
            check(cur.fetchone()[0] == 2, "two-column unique receipt index exists")
    finally:
        c.close()

    # Reapplying draft CREATE IF NOT EXISTS is a *reinstall* smoke check, NOT
    # a schema-migration strategy. It must never erase existing saved progress.
    with connection() as c:
        with c.cursor() as cur:
            for sql in statements(INSTALL_SQL):
                cur.execute(sql)
        c.commit()
    check(read_state(42) == (50, 200, 5, 5, 5),
          "reapplying identical schema preserves already-committed progress")
    check(read_state(84) == (110, 40, 1, 1, 1),
          "reapplying identical schema preserves other guild")

    # A non-module table in this throwaway DB is a sentinel for selective
    # purge. The only destructive operation here is deliberately confined to
    # naxx_gs_ci_test and is never performed against player data.
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "CREATE TABLE IF NOT EXISTS ci_other_module_sentinel "
                "(id INT PRIMARY KEY, value VARCHAR(20) NOT NULL) ENGINE=InnoDB"
            )
            cur.execute(
                "INSERT INTO ci_other_module_sentinel (id, value) VALUES (1, 'preserve') "
                "ON DUPLICATE KEY UPDATE value=VALUES(value)"
            )
        c.commit()
    with connection() as c:
        with c.cursor() as cur:
            for sql in statements(PURGE_SQL):
                cur.execute(sql)
            cur.execute("SELECT value FROM ci_other_module_sentinel WHERE id=1")
            check(cur.fetchone()[0] == "preserve",
                  "module purge leaves unrelated data untouched")
            cur.execute(
                "SELECT COUNT(*) FROM information_schema.tables "
                "WHERE table_schema=DATABASE() AND table_name LIKE 'naxx\\\\_gs\\\\_%'"
            )
            check(cur.fetchone()[0] == 0,
                  "deliberate destructive purge removed only module tables")
        c.commit()
    # Keep CI database disposable, and remove our unrelated sentinel too.
    with connection() as c:
        with c.cursor() as cur:
            cur.execute("DROP TABLE ci_other_module_sentinel")
        c.commit()

    print(f"PASS: {tests} MariaDB construction, reinstallation and selective-purge checks (isolated test DB)")


if __name__ == "__main__":
    run_tests()
