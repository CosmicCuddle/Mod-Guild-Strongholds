#!/usr/bin/env python3
"""DISPOSABLE CI database contract: prototype guild phase-slot reservations.

No AzerothCore/world phase code is called. Does not certify phase bits safe
on any deployed realm. connection() refuses names other than naxx_gs_ci_test.
"""
from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor
import pymysql

from mysql_construction_test import connection, INSTALL_SQL, PURGE_SQL, statements

POOL = (1 << 24) | (1 << 25)  # TEST-ONLY fake admin-approved bit range
BASELINE = 1                 # TEST-ONLY observed external mask


def reset():
    with connection() as c:
        with c.cursor() as cur:
            for sql in statements(PURGE_SQL):
                cur.execute(sql)
            for sql in statements(INSTALL_SQL):
                cur.execute(sql)
            for guild, generation, theme in (
                (101, 1790000001, "human"),
                (102, 1790000002, "orc"),
                (103, 1790000003, "human"),
                (104, 1790000004, "orc"),
            ):
                cur.execute(
                    "INSERT INTO naxx_gs_settlement "
                    "(guild_id,guild_created_at,theme_key) VALUES (%s,%s,%s)",
                    (guild, generation, theme),
                )
        c.commit()


def read(guild):
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "SELECT guild_created_at,phase_bit,lease_state,version "
                "FROM naxx_gs_isolation_slot WHERE guild_id=%s", (guild,)
            )
            row = cur.fetchone()
            cur.execute(
                "SELECT COUNT(*) FROM naxx_gs_ledger "
                "WHERE guild_id=%s AND event_key='phase_slot_reserved'",
                (guild,)
            )
            return row, cur.fetchone()[0]


def reserve(guild, generation, *,
            source_verified=True, phase_mode_verified=True,
            outside_mask=BASELINE, approved_mask=POOL,
            fault=None):
    # The booleans are MOCK staging fixtures. A real server must not use
    # client-supplied booleans or infer safe masks from empty SQL tables.
    if not source_verified or not phase_mode_verified:
        return "unverified"
    if not guild or not generation or not approved_mask:
        return "invalid"
    if approved_mask & outside_mask:
        return "external_collision"

    for attempt in range(8):
        c = connection()
        try:
            # Fresh read-committed transaction on each collision retry;
            # unique DB keys, not a pre-read, provide the final guarantee.
            with c.cursor() as cur:
                cur.execute("SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED")
            c.begin()
            with c.cursor() as cur:
                cur.execute(
                    "SELECT guild_created_at,lifecycle_state "
                    "FROM naxx_gs_settlement WHERE guild_id=%s FOR UPDATE",
                    (guild,),
                )
                owner = cur.fetchone()
                if not owner or owner[0] != generation or owner[1] != "active":
                    c.rollback()
                    return "invalid_property"
                cur.execute(
                    "SELECT guild_created_at,lease_state "
                    "FROM naxx_gs_isolation_slot WHERE guild_id=%s",
                    (guild,),
                )
                current = cur.fetchone()
                if current:
                    c.rollback()
                    return "already_owned" if current[0] == generation else "wrong_generation"

                cur.execute("SELECT phase_bit FROM naxx_gs_isolation_slot")
                used = 0
                for (mask,) in cur.fetchall():
                    if not mask or mask & (mask - 1):
                        c.rollback()
                        return "corrupt_slot"
                    if used & mask:
                        c.rollback()
                        return "corrupt_slot"
                    used |= mask

                candidate = approved_mask & ~outside_mask & ~used & 0xffffffff
                if not candidate:
                    c.rollback()
                    return "no_capacity"
                bit = candidate & -candidate
                cur.execute(
                    "INSERT INTO naxx_gs_isolation_slot "
                    "(guild_id,guild_created_at,phase_bit,lease_state) "
                    "VALUES (%s,%s,%s,'held')",
                    (guild, generation, bit),
                )
                if fault == "after_insert":
                    raise RuntimeError("SIMULATED lease insert failure")
                cur.execute(
                    "INSERT INTO naxx_gs_ledger "
                    "(guild_id,event_key,details) "
                    "VALUES (%s,'phase_slot_reserved',%s)",
                    (guild, str(bit)),
                )
                if fault == "after_ledger":
                    raise RuntimeError("SIMULATED lease ledger failure")
            c.commit()
            return "reserved"
        except pymysql.err.IntegrityError as exc:
            c.rollback()
            if exc.args[0] == 1062:
                continue  # Collision or duplicate guild, recheck fresh row.
            raise
        except pymysql.err.OperationalError as exc:
            c.rollback()
            if exc.args[0] in (1205, 1213):  # timeout / deadlock
                continue
            raise
        except Exception:
            c.rollback()
            raise
        finally:
            c.close()

    return "retry_exhausted"


def retire(guild, generation, *, event_verified, fault=False):
    """Mark a slot retired but NEVER delete or recycle it."""
    if not event_verified:
        return "unverified"
    c = connection()
    try:
        c.begin()
        with c.cursor() as cur:
            cur.execute(
                "SELECT guild_created_at,lifecycle_state "
                "FROM naxx_gs_settlement WHERE guild_id=%s FOR UPDATE",
                (guild,),
            )
            owner = cur.fetchone()
            if not owner or owner[0] != generation:
                c.rollback()
                return "wrong_generation"
            if owner[1] != "archived":
                c.rollback()
                return "property_not_archived"
            cur.execute(
                "SELECT guild_created_at,lease_state,version "
                "FROM naxx_gs_isolation_slot WHERE guild_id=%s FOR UPDATE",
                (guild,),
            )
            row = cur.fetchone()
            if not row or row[0] != generation:
                c.rollback()
                return "no_matching_lease"
            if row[1] == "retired":
                c.rollback()
                return "already_retired"
            cur.execute(
                "UPDATE naxx_gs_isolation_slot "
                "SET lease_state='retired',version=version+1 "
                "WHERE guild_id=%s AND guild_created_at=%s "
                "AND version=%s AND lease_state='held'",
                (guild, generation, row[2]),
            )
            if cur.rowcount != 1:
                raise RuntimeError("Unexpected version conflict")
            cur.execute(
                "INSERT INTO naxx_gs_ledger (guild_id,event_key) "
                "VALUES (%s,'phase_slot_retired')", (guild,),
            )
            if fault:
                raise RuntimeError("SIMULATED retirement failure")
        c.commit()
        return "retired"
    except Exception:
        c.rollback()
        raise
    finally:
        c.close()


def main():
    checks = 0

    def check(condition, message):
        nonlocal checks
        checks += 1
        if not condition:
            raise AssertionError(message)

    reset()
    check(reserve(101, 1790000001, source_verified=False) == "unverified",
          "No source review means no allocations")
    check(reserve(101, 1790000001, phase_mode_verified=False) == "unverified",
          "Unverified combined comparison refuses allocation")
    check(reserve(101, 1790000001, approved_mask=0) == "invalid",
          "No default phase pool")
    check(reserve(101, 1790000001, outside_mask=1 << 24) == "external_collision",
          "External module collision rejects whole approved pool")
    check(reserve(101, 1791000001) == "invalid_property",
          "Guild reuse does not inherit the original property")
    check(read(101) == (None, 0), "Rejected leases have no effects")

    for point in ("after_insert", "after_ledger"):
        try:
            reserve(101, 1790000001, fault=point)
        except RuntimeError:
            pass
        else:
            raise AssertionError("Missing failure injection")
        check(read(101) == (None, 0), "Transaction rolls back lease and audit")

    # Same and different guilds race to reserve from only TWO sample bits.
    with ThreadPoolExecutor(max_workers=2) as executor:
        results = list(executor.map(
            lambda args: reserve(*args),
            ((101, 1790000001), (102, 1790000002)),
        ))
    check(results.count("reserved") == 2, "Two simultaneous guilds each reserve")
    slot_a = read(101)[0]
    slot_b = read(102)[0]
    check(bool(slot_a and slot_b and slot_a[1] != slot_b[1]),
          "Database prevents cross-guild phase collisions")
    check({slot_a[1], slot_b[1]} == {1 << 24, 1 << 25},
          "Both test-approved bits are used, neither shared")
    check(read(101)[1] == 1 and read(102)[1] == 1,
          "Exactly one reservation audit event for each guild")
    check(reserve(101, 1790000001) == "already_owned",
          "Retry does not duplicate same guild lease")
    check(reserve(103, 1790000003) == "no_capacity",
          "Third guild refused instead of sharing phase")
    check(reserve(101, 1797777777) == "invalid_property",
          "Recreated guild still cannot override original lease")

    check(retire(101, 1790000001, event_verified=False) == "unverified",
          "Cannot retire without verified event")
    check(retire(101, 1790000001, event_verified=True) == "property_not_archived",
          "Live guild cannot retire and free a slot")
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "UPDATE naxx_gs_settlement SET lifecycle_state='archived' "
                "WHERE guild_id=101"
            )
        c.commit()
    try:
        retire(101, 1790000001, event_verified=True, fault=True)
    except RuntimeError:
        pass
    else:
        raise AssertionError("Missing failure injection on retirement")
    check(read(101)[0][2:] == ("held", 0),
          "Failure during retirement preserves original slot")
    check(retire(101, 1790000001, event_verified=True) == "retired",
          "Archived guild can retire without deleting slot")
    check(read(101)[0][2:] == ("retired", 1),
          "Retired lease still holds unique phase bit")
    check(retire(101, 1790000001, event_verified=True) == "already_retired",
          "Repeat disband/retire is idempotent")
    check(retire(101, 1791111111, event_verified=True) == "wrong_generation",
          "Foreign guild generation cannot retire owner's lease")
    check(reserve(103, 1790000003) == "no_capacity",
          "Retired bit is never silently reallocated")
    check(read(102)[0][2:] == ("held", 0),
          "Unrelated guild lease is not changed by retirement")

    with connection() as c:
        with c.cursor() as cur:
            for sql in statements(INSTALL_SQL):
                cur.execute(sql)
        c.commit()
    check(read(101)[0][2:] == ("retired", 1),
          "Same-version reinstall preserves retired bit")
    check(read(102)[0][2:] == ("held", 0),
          "Same-version reinstall preserves active bit")

    print(f"PASS: {checks} MariaDB phase-slot reservation and retirement checks (CI only)")


if __name__ == "__main__":
    main()
