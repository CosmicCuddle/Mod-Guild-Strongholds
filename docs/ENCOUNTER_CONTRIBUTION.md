# Encounter contribution provenance — offline contract only

**Status: draft pure C++ model + synthetic tests. NOT a live combat observer, human classifier, guild entitlement or trophy system.**

## Goal

The existing opt-in death/kill-credit log only detects *possible* raid boss events. Kill credit can be shared with spectators, pets and bots, so it cannot establish the participation of a real human guild raider. This new model specifies what a future separately reviewed server-side source must demonstrate, without guessing from kill credit or standing in the raid.

## Inputs and strict boundaries

- An audited server-owned attempt token, map, instance and boss identity; independently verified attempt start and completion, using one server clock. No token may be supplied by an addon.
- A bounded event batch for a **single character** (up to 128 distinct event IDs, same attempt/map/instance, within the recorded encounter). A zero, replayed, foreign, outside-window or unverified event invalidates the entire batch.
- A reviewed, server-originated source must bind the actor to the raid group **at event time** and the action target to this encounter, not merely the group roster at final kill.
- Only effective damage, effective healing or effective mitigation with nonzero effect are presently reviewable. Presence, final-boss kill credit, overheal and unreviewed support actions never become participation proof. Support/tanking semantics must be audited against the actual installed fork before changing this.
- Two distinct qualifying events produce only `CandidateForStagingReview`; this count and the two-hour safety bound are conservative **test-model heuristics, not approved game-design thresholds**. A short successful run of a raid or a specialized support role must not be silently excluded in eventual gameplay.

## Absolute non-goals

This model does **not** prove encounter victory, original guild generation, raid eligibility, Individual Progression stage, source-verified Playerbots control or **human classification**. A bot can generate authentic combat contributions. `EncounterParticipationVerified` and `TrophyGranted` are **always false**, including in synthetic positive examples.

There is no runtime combat tap, no data persistence, no network/API calls, no logging of character GUIDs, no SQL, no NPC/GO/spell/phase modifications. The regular AzerothCore module may compile this extra pure file but executes no new registration or effect.

## Review before a real adapter

1. Record exact deployed AzerothCore, Playerbots and Individual Progression SHAs and local differences on a separate backed-up staging checkout.
2. Verify an authoritative boss *attempt* start/end API, completion/encounter identity and event-time group identity (including disband, joins and mixed-guild raids).
3. Prove correct attribution for player, pet/guardian and totem actions, meaningful effective heals, absorbs, threat and support roles; do not count self-buffs or irrelevant heals.
4. Separate original guild membership and positive human control proof from combat contribution. A null Playerbot AI pointer remains unknown.
5. Agree on boss-specific participation policy and 2–40 human guild-member minimum; test 40-person mixed-guild, relog, AFK, spectators, disconnect and boss reset/retries.
6. Only after the source adapter is proven: decide private stronghold routing, safe receipt persistence, backup/preservation-first uninstall, and staging restore. Do not send trophies to a live realm based on this model.

Run tests with `bash tests/run-catalog-tests.sh` and `python3 tests/encounter_contribution_source_tests.py`.

**Rollback:** reverting the single development commit deletes these pure policy and test files; it has no migration, runtime objects or live state to restore.
