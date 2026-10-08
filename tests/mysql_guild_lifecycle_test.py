#!/usr/bin/env python3
"""CI-only guild lifecycle transaction contract; NEVER run on characters DB."""
from __future__ import annotations

import pymysql
from mysql_property_claim_test import claim, clear_schema
from mysql_construction_test import connection, _deposit_supply

def state(guild_id):
    with connection() as c:
        with c.cursor() as cur:
            cur.execute(
                "SELECT guild_created_at, lifecycle_state, lifecycle_version, "
                "guild_supplies FROM naxx_gs_settlement WHERE guild_id=%s", (guild_id,)
            )
            record = cur.fetchone()
            cur.execute(
                "SELECT event_key FROM naxx_gs_ledger WHERE guild_id=%s ORDER BY id",
                (guild_id,)
            )
            events = tuple(row[0] for row in cur.fetchall())
            return record, events

def can_enter(guild_id, guild_created_at, membership_verified):
    """Simulates a future authoritative guild membership and age check."""
    if not membership_verified or not guild_id or not guild_created_at:
        return False
    record, _ = state(guild_id)
    return bool(record and record[0] == guild_created_at and record[1] == "active")

def archive(guild_id, guild_created_at, disband_verified, failpoint=None):
    if not disband_verified or not guild_id or not guild_created_at:
        return "unverified"
    c = connection()
    try:
        c.begin()
        with c.cursor() as cur:
            cur.execute(
                "SELECT guild_created_at, lifecycle_state, lifecycle_version "
                "FROM naxx_gs_settlement WHERE guild_id=%s FOR UPDATE", (guild_id,)
            )
            record = cur.fetchone()
            if record is None:
                c.rollback()
                return "missing"
            generation, status, version = record
            if generation != guild_created_at:
                c.rollback()
                return "wrong_generation"
            if status != "active":
                c.rollback()
                return "already_archived"
            cur.execute(
                "UPDATE naxx_gs_settlement "
                "SET lifecycle_state='archived', lifecycle_version=lifecycle_version+1 "
                "WHERE guild_id=%s AND guild_created_at=%s AND lifecycle_version=%s "
                "AND lifecycle_state='active'",
                (guild_id, guild_created_at, version)
            )
            if cur.rowcount != 1:
                raise RuntimeError("Unexpected version conflict")
            if failpoint == "after_update":
                raise RuntimeError("Simulated failure after archive state update")
            cur.execute(
                "INSERT INTO naxx_gs_ledger (guild_id, event_key, details) "
                "VALUES (%s, 'settlement_archived', 'guild disband')", (guild_id,)
            )
            if failpoint == "after_ledger":
                raise RuntimeError("Simulated failure after archive ledger insert")
        c.commit()
        return "archived"
    except Exception:
        c.rollback()
        raise
    finally:
        c.close()

def restore(guild_id, guild_created_at, admin_verified, failpoint=False):
    # In production restoration must ALSO prove the ORIGINAL guild identity
    # exists in AzerothCore. Never revive orphaned property to reused guild IDs.
    if not admin_verified or not guild_created_at:
        return "denied"
    c = connection()
    try:
        c.begin()
        with c.cursor() as cur:
            cur.execute(
                "SELECT guild_created_at, lifecycle_state, lifecycle_version "
                "FROM naxx_gs_settlement WHERE guild_id=%s FOR UPDATE", (guild_id,)
            )
            record = cur.fetchone()
            if not record:
                c.rollback()
                return "missing"
            generation, status, version = record
            if generation != guild_created_at:
                c.rollback()
                return "wrong_generation"
            if status != "archived":
                c.rollback()
                return "already_active"
            cur.execute(
                "UPDATE naxx_gs_settlement SET lifecycle_state='active', "
                "lifecycle_version=lifecycle_version+1 "
                "WHERE guild_id=%s AND guild_created_at=%s AND lifecycle_version=%s "
                "AND lifecycle_state='archived'",
                (guild_id, guild_created_at, version)
            )
            if cur.rowcount != 1:
                raise RuntimeError("Unexpected restoration version conflict")
            if failpoint:
                raise RuntimeError("Simulated failure during restoration")
            cur.execute(
                "INSERT INTO naxx_gs_ledger (guild_id, event_key, details) "
                "VALUES (%s, 'settlement_restored', 'approved recovery')", (guild_id,)
            )
        c.commit()
        return "restored"
    except Exception:
        c.rollback()
        raise
    finally:
        c.close()

def main():
    clear_schema()
    count=0
    def check(cond, message):
        nonlocal count
        count+=1
        if not cond:
            raise AssertionError(message)

    first=dict(guild_id=42, actor_guild_id=42, leader=True, guild_faction="alliance",
               requested_theme="human", guild_created_at=1790000000)
    second=dict(guild_id=84, actor_guild_id=84, leader=True, guild_faction="horde",
                requested_theme="orc", guild_created_at=1790000001)
    check(claim(**first)=="accepted", "First guild created")
    check(claim(**second)=="accepted", "Second guild created")
    check(can_enter(42,1790000000,True), "Original guild can enter active property")
    check(not can_enter(42,1790000000,False), "Former guild member denied")
    check(not can_enter(42,1799999999,True), "Recycled guild id denied")
    check(archive(42,1790000000,False)=="unverified", "Unverified event cannot archive")
    check(archive(42,1799999999,True)=="wrong_generation", "Wrong guild generation cannot archive")
    check(state(42)[0][1:3]==("active",0), "Unverified archive changed no data")

    for point in ("after_update","after_ledger"):
        try:
            archive(42,1790000000,True,failpoint=point)
        except RuntimeError:
            pass
        else:
            raise AssertionError("Expected injected rollback")
        check(state(42)[0][1:3]==("active",0), "Failed archive rolled back all state")
        check(state(42)[1]==("settlement_claimed",), "Failed archive didn't write ledger")
    check(archive(42,1790000000,True)=="archived", "Verified disband archives")
    check(state(42)[0][1:3]==("archived",1), "Archived state version persisted")
    check(not can_enter(42,1790000000,True), "Archived property inaccessible")
    check(archive(42,1790000000,True)=="already_archived", "Repeated disband idempotent")
    check(state(42)[1]==("settlement_claimed","settlement_archived"),
          "Exactly one archive ledger event")
    check(claim(**{**first,"guild_created_at":1799999999})=="already_owned",
          "New guild with recycled ID cannot overwrite archived property")
    check(_deposit_supply(42,42,True,"archived_try",10,
          actor_created_at=1790000000)=="property_archived",
          "Archived property forbids new construction transactions")
    check(can_enter(84,1790000001,True), "Second guild unaffected")
    check(state(84)[0][1:3]==("active",0), "Another guild's property remains unchanged")
    check(restore(42,1790000000,False)=="denied", "No restoration without approval")
    check(restore(42,1799999999,True)=="wrong_generation", "No restore to reused ID")
    try:
        restore(42,1790000000,True,failpoint=True)
    except RuntimeError:
        pass
    else:
        raise AssertionError("Expected restore failure")
    check(state(42)[0][1:3]==("archived",1), "Failed restore rolled back")
    check(restore(42,1790000000,True)=="restored", "Approved exact-identity restore")
    check(can_enter(42,1790000000,True), "Restored original can enter")
    check(restore(42,1790000000,True)=="already_active", "Double restore idempotent")
    check(state(42)[0][1:3]==("active",2), "Restore increments version")
    check(state(42)[1]==("settlement_claimed","settlement_archived","settlement_restored"),
          "Claim/archive/restore history preserved")
    print(f"PASS: {count} MariaDB disband/archive/recovery contract checks (CI only)")

if __name__=="__main__":
    main()
