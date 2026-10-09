# Raid and Playerbots source preflight — development only

Status: **READ-ONLY SOURCE SIGNALS, NOT A HUMAN OR RAID VERIFIER.**

## Purpose

The previous encounter-contribution prototype can identify suspicious synthetic evidence, but it does not connect to AzerothCore combat events. Before doing so, we must verify the exact custom AzerothCore, Playerbots, Individual Progression and related module source. An upstream Playerbots bot-registry lookup returning null is **not proof of a human**. Boss death and shared kill credit are **not proof that a member actively completed an encounter**.

The new scanner is a safe way to locate candidate API *names* in a separate copy of source code, without printing source excerpts, SQL, configuration values, character names or Git patch contents. Matches may occur in comments, disabled code, obsolete functions, or entirely unrelated implementations.

## Future staging usage (not on the live realm)

When a backed-up staging source copy exists, from the Strongholds repository root:

    python3 scripts/audit-raid-source-readiness.py /path/to/STAGING/azerothcore

Optional counts/status-only JSON output:

    python3 scripts/audit-raid-source-readiness.py /path/to/STAGING/azerothcore --json

It scans C++ files only under core game/scripts and module src directories. Symlinks, config/SQL, large files, binary/invalid UTF-8 and build/test directories are skipped. Warnings report unreadable/truncated source. It never reads player records or performs writes. No SSH password, DB credentials or tokens are needed.

## What to investigate afterward

1. **Positive human account control.** Audit the actual installed Playerbots fork, headless and ordinary sessions, random bots, bots controlled by a player, account handoff and module-disabled behavior. Neither a null bot AI nor not-headless is enough. A source-linked, genuinely positive human attestation is required.
2. **Boss lifecycle.** Identify an authoritative named Classic encounter start, completion and reset. Do not conflate Wrath raid variants with Classic encounters. A wipe, respawn or duplicated callback must not produce a trophy receipt.
3. **Raid and guild membership at the time.** Verify the original guild generation, group, matching map and instance during the encounter. Final-group membership cannot establish who contributed earlier.
4. **Actual contribution.** Audit source of effective damage, healing, shielding/absorb and tank mitigation, including player-controlled pets/guardians/totems. Never count unrelated healing, buffs, kill credit alone or standing beside the boss.
5. **Coexistence.** Check Individual Progression phase restrictions and Naxxramas Core, dungeon clear and all other actually installed modules to avoid duplicate rewards.
6. **Safe future implementation.** Only after proof from a staged full worldserver build and human approval may a read-only real adapter be proposed. Permanent awards still require approved private guild space, repeat-safe SQL, backups, selective rollback and uninstall.

## Interpretation

Every report must say:
- decision = REVIEW_REQUIRED
- human_control = UNVERIFIED
- boss_completion = UNVERIFIED
- member_contribution = UNVERIFIED
- original_guild_generation = UNVERIFIED
- trophy_award_allowed = false

Even a positive count for *every* signal does not change any of these fields. A missing signal does not establish that an API is missing.

## Validation and rollback

The synthetic, fixture-only runner is:

    python3 tests/raid_source_readiness_tests.py

GitHub CI executes it without contacting the user's server or database. A revert of the development commit removes the scanner/tests/docs with no DB, objects, maps, phase or server code to clean up.

**Do not git pull, recompile, apply manual SQL or restart the live server for this development milestone.**
