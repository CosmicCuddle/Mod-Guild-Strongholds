#!/usr/bin/env python3
"""Static guardrail for DRAFT Guild Strongholds SQL.

Not a SQL parser, not a database test, and not a replacement for backups.
Checks that template CREATEs and permanent-purge DROP targets match exactly.
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INSTALL = ROOT / "data/sql/manual/install_characters.sql"
PURGE = ROOT / "uninstall/purge_characters.sql"
IDENT = r"`?([a-zA-Z_][a-zA-Z0-9_]*)`?"


def statements(text: str) -> list[str]:
    without_comments = "\n".join(
        line for line in text.splitlines() if not line.lstrip().startswith("--")
    )
    return [s.strip() for s in without_comments.split(";") if s.strip()]


def validate_sql(install_sql: str, purge_sql: str) -> list[str]:
    created: list[str] = []
    for stmt in statements(install_sql):
        match = re.fullmatch(
            r"CREATE\s+TABLE\s+IF\s+NOT\s+EXISTS\s+"
            + IDENT + r"\s*\(.*\)\s*ENGINE\s*=\s*InnoDB\s+DEFAULT\s+CHARSET\s*=\s*utf8mb4",
            stmt, flags=re.IGNORECASE | re.DOTALL,
        )
        if not match:
            raise ValueError("Unrecognised or unsafe install statement")
        created.append(match.group(1))

    removed: list[str] = []
    for stmt in statements(purge_sql):
        match = re.fullmatch(
            r"DROP\s+TABLE\s+IF\s+EXISTS\s+" + IDENT,
            stmt, flags=re.IGNORECASE,
        )
        if not match:
            raise ValueError("Unrecognised or unsafe purge statement")
        removed.append(match.group(1))

    if not created or len(set(created)) != len(created):
        raise ValueError("Install tables empty or duplicated")
    if len(set(removed)) != len(removed):
        raise ValueError("Purge tables duplicated")
    if any(not name.startswith("naxx_gs_") for name in created + removed):
        raise ValueError("Found non-module-owned table")
    if set(created) != set(removed):
        raise ValueError("Install and purge tables do not match")
    return created


def run_checks() -> None:
    install = INSTALL.read_text(encoding="utf-8")
    purge = PURGE.read_text(encoding="utf-8")
    tables = validate_sql(install, purge)

    # Trophy receipt and unlock MUST remain generation-qualified. The legacy
    # naxx_gs_unlock intentionally remains separate/untrusted for trophies.
    for name in ("naxx_gs_trophy_receipt", "naxx_gs_trophy_unlock"):
        match = re.search(
            r"CREATE\s+TABLE\s+IF\s+NOT\s+EXISTS\s+`"
            + name + r"`\s*\((.*?)\)\s*ENGINE",
            install, flags=re.IGNORECASE | re.DOTALL,
        )
        if not match:
            raise AssertionError(f"Missing generation-qualified trophy table: {name}")
        body = match.group(1)
        if "`guild_created_at` BIGINT UNSIGNED NOT NULL" not in body:
            raise AssertionError(f"{name}: missing original guild generation")
        if not re.search(r"PRIMARY KEY\s*\(`guild_id`\s*,\s*`guild_created_at`", body):
            raise AssertionError(f"{name}: primary key is not generation-qualified")
    if "PRIMARY KEY (`guild_id`, `guild_created_at`, `event_receipt`)" not in install:
        raise AssertionError("Trophy receipt replay identity must be unique per generation")
    if "PRIMARY KEY (`guild_id`, `guild_created_at`, `trophy_key`)" not in install:
        raise AssertionError("Trophy unlock must be unique per original guild and trophy")
    # Simple negative tests for protection against accidental schema expansion.
    def must_fail(i: str, p: str) -> None:
        try:
            validate_sql(i, p)
        except ValueError:
            return
        raise AssertionError("Expected unsafe or mismatched SQL to be rejected")

    must_fail(install, purge + "\nDROP TABLE IF EXISTS `characters`;")
    must_fail(install + "\nDELETE FROM `characters`;", purge)
    must_fail(install, purge.replace("DROP TABLE IF EXISTS `naxx_gs_project`;", ""))
    must_fail(install, purge.replace("DROP TABLE IF EXISTS `naxx_gs_project`;",
                                      "DROP TABLE IF EXISTS `naxx_gs_project`;\nDROP TABLE IF EXISTS `naxx_gs_project`;"))

    print(f"PASS: {len(tables)} module-owned SQL tables match explicit purge targets (static lint)")


if __name__ == "__main__":
    run_checks()
