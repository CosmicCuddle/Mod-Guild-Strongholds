# Guild Steward — verified evidence before any service

**State: implemented domain checks plus a locked, staging-only gossip page. No live guild property or IP adapter.**

## Principle: do not confuse identity with ownership

The player's own server-visible numeric `GetGuildId()` is not sufficient to prove which original guild generation owns a permanent settlement. Guild IDs may be recycled and properties archived after guild disband. The client may also have a high completed Individual Progression quest stage while the server has a lower `progressionLimit`.

The `StewardEvidenceInput` separates:

- current guild ID **and separately verified guild creation time**;
- authoritative property record loaded **and** checked against that original guild generation;
- active/archived property lifecycle;
- authoritative settlement level in 1..7;
- source-verified Individual Progression enable state, player-in-world stage and actual configured cap;
- actor bot flag (bots are disabled for preview activity eligibility).

`EvaluateStewardEvidence` reuses `CheckPropertyLifetime`, `EvaluateIpStage` and `CheckActivityEligibility` in that order. Any missing, invalid, other-guild or archived evidence fails closed. Only with independently verified inputs does it compute hypothetical **planning** activity gates. The result's `HousingAvailable` is explicitly ALWAYS false in this release, even if a synthetic test supplies complete evidence.

## What a future staging GM sees

The opt-in `Guild and IP evidence [LOCKED]` page reads **only** the player's visible guild ID. It does not use `CharacterDatabase`, `WorldDatabase`, unverified individual-progression APIs or other modules. The page states: guild ID not property proof, no authoritative original generation, no loaded settlement property and no verified IP state. Every row begins `[LOCKED]`.

This is deliberately less impressive than fabricated `Level 7` / `IP Tier 18` values, but is safe and trustworthy. The other Guild Steward catalogue pages remain **design plans**, not live achievements.

## Regression acceptance

`tests/steward_evidence_tests.cpp` checks:
- guild ID without verified original creation time;
- other guild ownership, recycled guild ID and archived property;
- unloaded property and invalid settlement levels;
- disabled/unknown IP state and old WotLK quest progress capped to Vanilla/TBC;
- planned Outland/Northrend gates and Playerbot denial;
- zero production housing access even in a fully verified synthetic snapshot;
- page length, visible `[LOCKED]` status and no false unlock statements.

## Still blocked

A separate, owner-approved staging build must first provide authoritative read-only API glue for original guild generation, property state and versioned persistence, IP state on the installed fork, and independently confirmed private settlement isolation. No real property, daily quest, reward, trophy, map entry or teleport should be wired to a gossip menu until those tests and backup/uninstall plans have passed.
