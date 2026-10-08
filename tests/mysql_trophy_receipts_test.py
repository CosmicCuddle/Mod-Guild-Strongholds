#!/usr/bin/env python3
"""Disposable MariaDB ONLY: atomic, generation-qualified guild trophy receipts.

This is a TEST FIXTURE, never imported/linked from AzerothCore. Simulated
server kill/roster confirmations are NOT proof of real boss completion.
Requires mysql_construction_test.connection(), which refuses any DB other
than naxx_gs_ci_test and insists on NAXX_GS_CI_ONLY=YES.
"""
from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor

from mysql_construction_test import (
    INSTALL_SQL, PURGE_SQL, connection, statements, Rejected,
)

TROPHIES = {"onyxia_head", "ragnaros_flame", "nefarian_banner",
            "cthun_relic", "kelthuzad_sigil"}


def reset():
    with connection() as db:
        with db.cursor() as cur:
            for sql in statements(PURGE_SQL):
                cur.execute(sql)
            for sql in statements(INSTALL_SQL):
                cur.execute(sql)
            cur.execute(
                "INSERT INTO naxx_gs_settlement "
                "(guild_id, guild_created_at, lifecycle_state, theme_key) "
                "VALUES (21, 1777000000, 'active', 'human'), "
                "(44, 1777000001, 'active', 'orc'), "
                "(77, 1777000003, 'archived', 'human')"
            )
        db.commit()


def state(guild_id, generation):
    with connection() as db:
        with db.cursor() as cur:
            cur.execute(
                "SELECT trophy_key, first_event_receipt "
                "FROM naxx_gs_trophy_unlock WHERE guild_id=%s "
                "AND guild_created_at=%s ORDER BY trophy_key",
                (guild_id, generation),
            )
            unlocks = tuple(cur.fetchall())
            cur.execute(
                "SELECT event_receipt, trophy_key FROM naxx_gs_trophy_receipt "
                "WHERE guild_id=%s AND guild_created_at=%s ORDER BY event_receipt",
                (guild_id, generation),
            )
            receipts = tuple(cur.fetchall())
            cur.execute(
                "SELECT details FROM naxx_gs_ledger WHERE guild_id=%s "
                "AND event_key='trophy_unlocked' AND details LIKE %s ORDER BY id",
                (guild_id, f"gen={generation};%"),
            )
            events = tuple(r[0] for r in cur.fetchall())
            return unlocks, receipts, events


def _valid_receipt(value):
    return (
        isinstance(value, str) and 16 <= len(value) <= 80 and
        all("a" <= c <= "z" or "A" <= c <= "Z" or
            "0" <= c <= "9" or c in "_-:" for c in value)
    )


def record_trophy(guild_id, generation, trophy, receipt, *,
                  map_id=249, instance_id=100, humans=2,
                  source_verified=False, simulated_policy_approved=False,
                  failpoint=None):
    """Model future ONE-transaction persistence, under the guild row lock.

    No real boss kill hook or C++ persistence adapter exists. The test caller
    explicitly fakes source_verified and policy approval. Production MUST
    derive these from authenticated server encounter and roster callbacks.
    """
    if (not source_verified or not simulated_policy_approved or
        isinstance(guild_id, bool) or not isinstance(guild_id, int) or
        guild_id <= 0 or isinstance(generation, bool) or
        not isinstance(generation, int) or generation <= 0):
        return "unverified"
    if trophy not in TROPHIES or not _valid_receipt(receipt) or (
        not isinstance(map_id, int) or not isinstance(instance_id, int) or
        map_id <= 0 or instance_id <= 0 or not isinstance(humans, int) or
        isinstance(humans, bool) or not 2 <= humans <= 40
    ):
        return "invalid_proof"

    db = connection()
    try:
        db.begin()
        with db.cursor() as cur:
            # Serialize award attempts and disband/archive on the same row.
            # This must be the FIRST DB lock for any proposed credit.
            cur.execute(
                "SELECT guild_created_at, lifecycle_state "
                "FROM naxx_gs_settlement WHERE guild_id=%s FOR UPDATE",
                (guild_id,),
            )
            owner = cur.fetchone()
            if not owner:
                raise Rejected("missing_property")
            if owner[0] != generation:
                raise Rejected("wrong_generation")
            if owner[1] != "active":
                raise Rejected("archived")

            cur.execute(
                "SELECT event_receipt FROM naxx_gs_trophy_receipt "
                "WHERE guild_id=%s AND guild_created_at=%s "
                "AND event_receipt=%s", (guild_id, generation, receipt),
            )
            if cur.fetchone():
                raise Rejected("duplicate_event")
            cur.execute(
                "SELECT first_event_receipt FROM naxx_gs_trophy_unlock "
                "WHERE guild_id=%s AND guild_created_at=%s "
                "AND trophy_key=%s", (guild_id, generation, trophy),
            )
            if cur.fetchone():
                raise Rejected("already_unlocked")

            cur.execute(
                "INSERT INTO naxx_gs_trophy_receipt "
                "(guild_id, guild_created_at, event_receipt, trophy_key, "
                "raid_map_id, raid_instance_id, verified_human_count) "
                "VALUES (%s, %s, %s, %s, %s, %s, %s)",
                (guild_id, generation, receipt, trophy, map_id, instance_id, humans),
            )
            if failpoint == "after_receipt":
                raise RuntimeError("CI injected failure after unique event receipt")

            cur.execute(
                "INSERT INTO naxx_gs_trophy_unlock "
                "(guild_id, guild_created_at, trophy_key, first_event_receipt) "
                "VALUES (%s, %s, %s, %s)",
                (guild_id, generation, trophy, receipt),
            )
            if failpoint == "after_unlock":
                raise RuntimeError("CI injected failure after saved trophy unlock")

            details = f"gen={generation};trophy={trophy};event={receipt}"
            cur.execute(
                "INSERT INTO naxx_gs_ledger (guild_id, event_key, details) "
                "VALUES (%s, 'trophy_unlocked', %s)",
                (guild_id, details),
            )
            if failpoint == "after_audit":
                raise RuntimeError("CI injected failure after append-only audit")
        db.commit()
        return "recorded"
    except Rejected as exc:
        db.rollback()
        return str(exc)
    except Exception:
        db.rollback()
        raise
    finally:
        db.close()


def main():
    checks = 0

    def check(ok, label):
        nonlocal checks
        checks += 1
        if not ok:
            raise AssertionError(label)

    reset()
    empty = ((), (), ())
    original = (21, 1777000000, "onyxia_head", "raid:onyxia:inst100:kill001")
    another = (44, 1777000001, "onyxia_head", "raid:onyxia:inst100:kill001")
    kwargs = {"source_verified": True, "simulated_policy_approved": True}
    check(state(21, 1777000000) == empty, "fresh schema no trophies")
    check(record_trophy(*original) == "unverified",
          "Source proof must not silently default true")
    check(record_trophy(*original, source_verified=True) == "unverified",
          "Pure-domain participation approval required")
    check(record_trophy(21, 1777000999, original[2], original[3], **kwargs)
          == "wrong_generation", "Reused guild ID cannot claim original settlement")
    check(record_trophy(77, 1777000003, original[2], original[3], **kwargs)
          == "archived", "Disbanded guild cannot receive trophies")
    check(record_trophy(55, 1777000500, original[2], original[3], **kwargs)
          == "missing_property", "No guild property is not a valid trophy owner")
    check(record_trophy(*original, humans=1, **kwargs) == "invalid_proof",
          "Solo participant is not accepted by current policy")
    check(record_trophy(*original, humans=41, **kwargs) == "invalid_proof",
          "More than 40 humans cannot be claimed")
    check(record_trophy(21, 1777000000, "wotlk_onyxia", original[3], **kwargs)
          == "invalid_proof", "Unknown/incorrect-era trophy denied")
    check(record_trophy(21, 1777000000, original[2],
                        "raid:onyxia:drop table;", **kwargs) == "invalid_proof",
          "Unsafe event token denied")
    check(state(21, 1777000000) == empty, "Every rejected proof has zero writes")

    for at in ("after_receipt", "after_unlock", "after_audit"):
        try:
            record_trophy(*original, failpoint=at, **kwargs)
        except RuntimeError:
            pass
        else:
            raise AssertionError(f"Expected rollback failpoint: {at}")
        check(state(21, 1777000000) == empty,
              f"Atomic rollback at {at}, including ledger")

    check(record_trophy(*original, **kwargs) == "recorded",
          "Verified simulated first raid trophy transaction commits")
    check(len(state(21, 1777000000)[0]) == 1 and
          len(state(21, 1777000000)[1]) == 1 and
          len(state(21, 1777000000)[2]) == 1,
          "Award receipt, unlock and audit ledger committed together")
    check(record_trophy(*original, **kwargs) == "duplicate_event",
          "Same kill event replay prevented")
    same_event_other_trophy = (21, 1777000000, "nefarian_banner", original[3])
    check(record_trophy(*same_event_other_trophy, **kwargs) == "duplicate_event",
          "One kill event cannot be reused for different boss trophies")
    check(record_trophy(21, 1777000000, "onyxia_head",
                        "raid:onyxia:inst111:kill002", **kwargs) == "already_unlocked",
          "Another Onyxia kill cannot award a duplicate guild trophy")
    check(record_trophy(*another, **kwargs) == "recorded",
          "Mixed-guild raid can credit a second independently qualified guild")
    check(len(state(44, 1777000001)[0]) == 1,
          "Mixed-guild second credit stays separate")

    # Concurrent same new receipt: ONE atomic award, no duplicate ledger.
    nefarian = (21, 1777000000, "nefarian_banner", "raid:nefarian:inst200:kill01")
    with ThreadPoolExecutor(max_workers=2) as pool:
        replies = list(pool.map(lambda _: record_trophy(*nefarian, **kwargs),
                                range(2)))
    check(sorted(replies) == ["duplicate_event", "recorded"],
          "Concurrent same event exactly-once")
    check(len(state(21, 1777000000)[0]) == 2 and
          len(state(21, 1777000000)[1]) == 2 and
          len(state(21, 1777000000)[2]) == 2,
          "Race does not create a second receipt or audit")

    # Different event receipts but SAME new trophy: unique unlock wins.
    ragnaros_a = (21, 1777000000, "ragnaros_flame",
                  "raid:ragnaros:inst300:kill01")
    ragnaros_b = (21, 1777000000, "ragnaros_flame",
                  "raid:ragnaros:inst301:kill02")
    with ThreadPoolExecutor(max_workers=2) as pool:
        replies = list(pool.map(
            lambda x: record_trophy(*x, **kwargs),
            (ragnaros_a, ragnaros_b),
        ))
    check(sorted(replies) == ["already_unlocked", "recorded"],
          "Concurrent distinct kills can award same trophy only once")
    check(len(state(21, 1777000000)[0]) == 3 and
          len(state(21, 1777000000)[1]) == 3 and
          len(state(21, 1777000000)[2]) == 3,
          "Only one atomic event/unlock/audit per trophy")

    # Genuine archived original keeps historical progress, but cannot add.
    with connection() as db:
        with db.cursor() as cur:
            cur.execute("UPDATE naxx_gs_settlement SET lifecycle_state='archived' "
                        "WHERE guild_id=21")
        db.commit()
    historic = state(21, 1777000000)
    check(record_trophy(21, 1777000000, "cthun_relic",
                        "raid:cthun:inst400:kill001", **kwargs) == "archived",
          "Archived settlements cannot earn more")
    check(state(21, 1777000000) == historic,
          "Archival preserves trophies and historical receipt audit")

    # Reinstall same DRAFT schema is non-destructive; not a migration engine.
    with connection() as db:
        with db.cursor() as cur:
            for command in statements(INSTALL_SQL):
                cur.execute(command)
        db.commit()
    check(state(21, 1777000000) == historic,
          "Reapply exact draft CREATE preserves old-generation trophy history")
    check(len(state(44, 1777000001)[0]) == 1,
          "Second guild survives schema reinstall")

    # A former ID is reused after the old property is archived: historic
    # trophy data must remain associated with the OLD creation timestamp.
    with connection() as db:
        with db.cursor() as cur:
            cur.execute("UPDATE naxx_gs_settlement SET guild_created_at=%s,"
                        " lifecycle_state='active' WHERE guild_id=21",
                        (1777999999,))
        db.commit()
    check(record_trophy(21, 1777000000, "cthun_relic",
                        "raid:cthun:inst400:kill001", **kwargs) == "wrong_generation",
          "Old guild generation cannot claim new guild's current property")
    check(state(21, 1777000000) == historic,
          "Old original-guild trophies preserved; never silently transferred")
    check(state(21, 1777999999) == empty,
          "New guild generation gets no inherited trophies")
    check(record_trophy(21, 1777999999, "cthun_relic",
                        "raid:cthun:inst400:kill001", **kwargs) == "recorded",
          "New generation may independently earn its own distinct trophy")
    check(len(state(21, 1777999999)[0]) == 1 and
          state(21, 1777000000) == historic,
          "New generation and old history remain independently keyed")

    with connection() as db:
        with db.cursor() as cur:
            for tbl in ("naxx_gs_trophy_receipt", "naxx_gs_trophy_unlock"):
                cur.execute(
                    "SELECT COUNT(*) FROM information_schema.statistics "
                    "WHERE table_schema=DATABASE() AND table_name=%s "
                    "AND index_name='PRIMARY'", (tbl,)
                )
                check(cur.fetchone()[0] == 3,
                      f"{tbl} has a generation-qualified three-column primary key")
    print(f"PASS: {checks} disposable MariaDB trophy receipt, replay, "
          "concurrency, rollback, archive and generation checks")


if __name__ == "__main__":
    main()
