#!/usr/bin/env python3
"""Disposable MariaDB contract for durable housing return tickets.

TEST ONLY. Guarded by mysql_construction_test.connection(), which refuses
any database other than naxx_gs_ci_test with NAXX_GS_CI_ONLY=YES.
NO real AzerothCore teleportation or player maps.
"""
from __future__ import annotations

import pymysql
from mysql_construction_test import INSTALL_SQL, PURGE_SQL, connection, statements

GUILD_ID = 70
GUILD_GENERATION = 1790000100
CHARACTER = 1234
OTHER_CHARACTER = 5678
ORIGIN = (0, 0, -8950.1, 515.5, 98.0, 1.2)


def reset():
    with connection() as c:
        with c.cursor() as cur:
            for sql in statements(PURGE_SQL):
                cur.execute(sql)
            for sql in statements(INSTALL_SQL):
                cur.execute(sql)
            cur.execute(
                "INSERT INTO naxx_gs_settlement "
                "(guild_id,guild_created_at,theme_key) VALUES (%s,%s,'human')",
                (GUILD_ID, GUILD_GENERATION)
            )
        c.commit()


def read(guid=CHARACTER):
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "SELECT visit_nonce, visit_state, version, guild_id, guild_created_at, "
                "return_map, return_instance, return_x, return_y, return_z, return_o "
                "FROM naxx_gs_visit WHERE player_guid=%s", (guid,)
            )
            return cur.fetchone()


def prepare(guid, nonce, *, server_verified, no_outstanding, failpoint=False):
    """Simulated adapter contract: validate source before any future teleport."""
    if not server_verified or not no_outstanding or guid <= 0 or not nonce:
        return "denied"
    c = connection()
    try:
        c.begin()
        with c.cursor() as cur:
            try:
                cur.execute(
                    "INSERT INTO naxx_gs_visit "
                    "(player_guid,guild_id,guild_created_at,visit_nonce,property_key,"
                    "visit_state,return_map,return_instance,return_x,return_y,return_z,return_o) "
                    "VALUES (%s,%s,%s,%s,'human','prepared',%s,%s,%s,%s,%s,%s)",
                    (guid, GUILD_ID, GUILD_GENERATION, nonce, *ORIGIN)
                )
            except pymysql.err.IntegrityError as ex:
                if ex.args[0] != 1062:
                    raise
                c.rollback()
                return "outstanding_or_duplicate"
            if failpoint:
                raise RuntimeError("Simulated crash before committing prepared return")
        c.commit()
        return "prepared"
    except Exception:
        c.rollback()
        raise
    finally:
        c.close()


def transition(guid, nonce, expected_state, next_state,
               *, verified, destination_ok, failpoint=False):
    """State changes must be a versioned CAS; player identity must be trusted."""
    if not verified or not destination_ok:
        return "unverified"
    if (expected_state, next_state) not in (
            ("prepared", "inside"), ("inside", "returning"),
            ("prepared", "returning")):
        return "invalid_transition"
    c = connection()
    try:
        c.begin()
        with c.cursor() as cur:
            cur.execute(
                "SELECT visit_state,version FROM naxx_gs_visit "
                "WHERE player_guid=%s AND visit_nonce=%s FOR UPDATE",
                (guid, nonce)
            )
            saved = cur.fetchone()
            if not saved or saved[0] != expected_state:
                c.rollback()
                return "stale_or_unknown"
            cur.execute(
                "UPDATE naxx_gs_visit SET visit_state=%s,version=version+1 "
                "WHERE player_guid=%s AND visit_nonce=%s "
                "AND visit_state=%s AND version=%s",
                (next_state, guid, nonce, expected_state, saved[1])
            )
            if cur.rowcount != 1:
                raise RuntimeError("Unexpected version race")
            if failpoint:
                raise RuntimeError("Simulated crash between state update and commit")
        c.commit()
        return "updated"
    except Exception:
        c.rollback()
        raise
    finally:
        c.close()


def clear(guid, nonce, *, player_at_verified_return):
    """Do NOT delete without confirmation that the original player got home."""
    if not player_at_verified_return:
        return "not_confirmed"
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "DELETE FROM naxx_gs_visit WHERE player_guid=%s "
                "AND visit_nonce=%s AND visit_state='returning'", (guid, nonce)
            )
            count = cur.rowcount
        c.commit()
        return "cleared" if count == 1 else "not_ready"


def main():
    checks = 0

    def verify(condition, message):
        nonlocal checks
        checks += 1
        if not condition:
            raise AssertionError(message)

    reset()
    verify(read() is None, "No return ticket at beginning")
    verify(prepare(CHARACTER, "nonce-A", server_verified=False, no_outstanding=True) ==
           "denied", "Unverified server origin cannot prepare")
    verify(read() is None, "Rejected visit left no record")
    try:
        prepare(CHARACTER, "nonce-crash", server_verified=True,
                no_outstanding=True, failpoint=True)
    except RuntimeError:
        pass
    else:
        raise AssertionError("Missing prepare failure injection")
    verify(read() is None, "Prepared ticket insertion rolled back before commit")

    verify(prepare(CHARACTER, "nonce-A", server_verified=True,
                   no_outstanding=True) == "prepared",
           "Durable ticket prepared")
    verify(read()[1:3] == ("prepared", 0), "Ticket persisted before entry")
    verify(read()[5:7] == (0, 0), "Saved origin map is non-instance")
    verify(prepare(CHARACTER, "nonce-B", server_verified=True,
                   no_outstanding=True) == "outstanding_or_duplicate",
           "Second visit for same character cannot overwrite return point")
    verify(prepare(OTHER_CHARACTER, "nonce-A", server_verified=True,
                   no_outstanding=True) == "outstanding_or_duplicate",
           "Unique nonce cannot be shared by another player")
    verify(read()[0] == "nonce-A", "Original return ticket retained")

    verify(transition(CHARACTER, "nonce-A", "prepared", "inside",
                      verified=True, destination_ok=False) == "unverified",
           "Unverified destination cannot mark inside")
    verify(read()[1] == "prepared", "Failed teleport leaves recovery ticket")
    verify(transition(OTHER_CHARACTER, "nonce-A", "prepared", "inside",
                      verified=True, destination_ok=True) == "stale_or_unknown",
           "Wrong character cannot change session")
    verify(transition(CHARACTER, "nonce-A", "prepared", "inside",
                      verified=True, destination_ok=True) == "updated",
           "Verified arrival transitions to inside")
    verify(read()[1:3] == ("inside", 1), "Inside state persisted")

    # Guild disband may occur while player is inside. Emergency return must
    # not require active property ownership or guild membership.
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "UPDATE naxx_gs_settlement SET lifecycle_state='archived' "
                "WHERE guild_id=%s", (GUILD_ID,)
            )
        c.commit()
    verify(read()[1] == "inside", "Disband does not erase visit ticket")
    try:
        transition(CHARACTER, "nonce-A", "inside", "returning",
                   verified=True, destination_ok=True, failpoint=True)
    except RuntimeError:
        pass
    else:
        raise AssertionError("Missing return failure injection")
    verify(read()[1:3] == ("inside", 1), "Return failure rolls back, ticket survives")

    verify(transition(CHARACTER, "nonce-A", "inside", "returning",
                      verified=True, destination_ok=True) == "updated",
           "Recovery return can begin even after guild disband")
    verify(read()[1:3] == ("returning", 2), "Returning persisted before teleport")
    verify(clear(CHARACTER, "nonce-A", player_at_verified_return=False) ==
           "not_confirmed", "Unconfirmed teleport cannot delete return ticket")
    verify(read() is not None, "Return ticket still exists across connections")
    verify(clear(OTHER_CHARACTER, "nonce-A", player_at_verified_return=True) ==
           "not_ready", "Other character cannot clear ticket")
    verify(clear(CHARACTER, "invalid", player_at_verified_return=True) ==
           "not_ready", "Wrong nonce cannot clear ticket")
    verify(read()[1] == "returning", "Invalid clear attempts leave return available")

    with connection() as c:
        with c.cursor() as cur:
            for stmt in statements(INSTALL_SQL):
                cur.execute(stmt)
        c.commit()
    verify(read()[1:3] == ("returning", 2), "Same-version reinstallation preserves return ticket")
    verify(clear(CHARACTER, "nonce-A", player_at_verified_return=True) ==
           "cleared", "Confirmed safe arrival permits clearing ticket")
    verify(read() is None, "Return ticket removed after success")

    verify(prepare(CHARACTER, "nonce-new", server_verified=True,
                   no_outstanding=True) == "prepared", "New independent visit can be staged")
    verify(transition(CHARACTER, "nonce-new", "prepared", "returning",
                      verified=True, destination_ok=True) == "updated",
           "Failed-before-arrival case can be recovered without inside state")
    verify(clear(CHARACTER, "nonce-new", player_at_verified_return=True) ==
           "cleared", "Recovered pre-entry ticket safely closed")

    print(f"PASS: {checks} durable return-ticket MariaDB contract checks (isolated CI database)")


if __name__ == "__main__":
    main()
