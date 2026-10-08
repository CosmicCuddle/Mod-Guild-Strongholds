# Direct Individual Progression runtime read — staging only

**Not live housing. Not enabled in normal builds. No actual deployed Naxxramas server files, DBC or database modified.**

## Why direct module state matters

The Grimfeather reviewed IP fork stores earned stage as the *highest rewarded progression quest*. But `hasPassedProgression` also checks a separate `enabled` flag and **global `progressionLimit`**. Reading only character quests (or separately parsing a setting) could overstate what a character can access. We must consult the **actual linked IP object** for consistency.

Reviewed public fork: `Grimfeather/mod-individual-progression` commit `706740808fee328b8557607f87b0548cf961e047`. Public API reviewed:
- `sIndividualProgression` singleton macro and public `enabled` flag.
- `GetPlayerProgressionFromQuests(Player*)`, **read-only** quest stage 0–18.
- `progressionLimit`, with 0 meaning no cap; stage value above limit is not available to the player.

## Gates, all required

1. Compile `NAXX_GS_BUILD_STAGING_STEWARD` for a disposable/test realm.
2. Separately compile `NAXX_GS_BUILD_STAGING_IP_READ`; the code deliberately includes `IndividualProgression.h` and **requires** the reviewed installed fork's source to be available and linked. Missing source must cause a build error, not a silent fallback.
3. Set **`NaxxGuildStrongholds.StagingSteward.Enabled=1`** and **`NaxxGuildStrongholds.StagingIpRead.Enabled=1`** on that separately backed-up staging realm. Both default to 0.
4. The player must be a real in-world GM, with verified AzerothCore Guild ID, exact member GUID and original guild creation date.

On the locked Steward evidence page, the adapter reads **only** `IndividualProgression::enabled`, `GetPlayerProgressionFromQuests(player)` and `progressionLimit`. It feeds `EvaluateIpStage`, which refuses disabled/off-world/unexpected data and correctly caps the rewarded stage to the effective stage. No `UpdateProgressionState`, `ForceUpdateProgressionState`, spell phase changes, quest awards, configuration writes or SQL operations are performed.

**Do not use this in the live server.** A successful CI build against reviewed *public* IP source is not evidence that your installed fork's headers and runtime match this commit. The full deployed source SHA and uncommitted local modifications must be checked before any future staging trial.

## GitHub CI evidence

The upstream workflow has three independent paths:
- Passive normal build (both staging adapters absent), attempts full public upstream worldserver link.
- GM Steward and property-read staging build with no IP dependency.
- **NEW:** GM Steward, property-read and IP-read staging build with the pinned public Grimfeather IP fork checked out into the ephemeral upstream core's `modules/mod-individual-progression`. CMake builds the real `modules` target with IP's header directory explicitly included.

No game realm is started and no IP quest is changed. Static tests disallow known mutator calls in the IP adapter. C++ effective-stage tests already cover cap 0, 7, 8, 13, 17, high rewarded rank, disabled source and off-world. None of these tests bypass the hard-coded `HousingAvailable=false` rule or the private-isolation release gate.

## Remaining release blockers

Exact installed fork revision and full deployed-module compile, real private settlement isolation, NPC/world assets, safe teleport and logout recovery, quest and construction transactions, Playerbots, disband and uninstall tests are still pending.
