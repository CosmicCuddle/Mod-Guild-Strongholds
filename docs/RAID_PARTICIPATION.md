# Guild raid participation and Playerbots source contract — development only

**Status:** independently testable policy and actual upstream **candidate-only** staging callback. Neither observer grants trophy eligibility, quest rewards or housing access.

## Two real read-only AzerothCore event sources

The staging build compiles `StrongholdStagingRaidObserver.cpp` for `UnitScript::OnUnitDeath(Unit*, Unit*)`, and now `StrongholdStagingRaidKillCreditObserver.cpp` for `PlayerScript::OnPlayerCreatureKillCredit(Player*, Creature*)`. Both are registered **only with** `NAXX_GS_BUILD_STAGING_RAID_OBSERVER`; both are silent unless `NaxxGuildStrongholds.StagingRaidObserver.Enabled=1` (default 0).

The death callback reports a flagged raid boss's candidate death. The kill-credit callback reports a **candidate recipient**, but intentionally **does not log player name, GUID, guild or Playerbots state**. Neither hook alone proves a specific Classic raid encounter was legitimately completed or that all raid members earned a guild trophy.

AzerothCore explicitly documents kill-credit callbacks for tapped creatures and those killed by pets or totems. Therefore a kill-credit callback, a player in the map, an entry in the group roster or the killing blow are not sufficient by themselves to claim independently verified boss participation.

## StrongholdRaidParticipation domain policy

A **synthetic data-driven** `ReviewRaidParticipation` now tests the exact evidence a future installed-fork adapter must provide:
1. Exact claiming guild ID and original creation date, both server-verified.
2. Authentic server raid map/instance and independently identified boss encounter.
3. Authoritatively sourced, unique roster of up to 40 distinct character GUIDs.
4. For each qualifying human: independently verified current guild membership/generation, group membership, same-instance participation window and genuine per-character encounter contribution.
5. Positive, audited human-versus-bot classification, including the exact version of the deployed Playerbots module. `Unknown` or not-source-verified always means **not counted**.
6. Explicit 2–40 qualifying human member minimum per guild. Mixed-guild raids may independently qualify two original guild generations; Playerbots and outsiders do not inflate the human count.

Even with all synthetic evidence set to true, the returned result is **ProposalOnly** and `TrophyGranted=false`. It is not hooked to `StrongholdTrophies`, a database write, world object, quest or reward. No runtime function currently creates a verified successful participation proof.

## The Playerbots trap discovered in public source

In public `mod-playerbots/mod-playerbots` commit `037c01418b5d01506917a3db9b44fd56ac5f965c`, the implementation of `PlayerbotsMgr::GetPlayerbotAI(Player*)` first checks `sPlayerbotAIConfig.enabled`. It returns **nullptr if the module is disabled**, or if a player has no AI in the bot map.

Consequently, `GetPlayerbotAI(player) == nullptr` is **not a positive proof of being human**. The reviewed fork also references `GetSession()->IsHeadless()`, but no combination has been verified as a sufficient positive human test across all bots, session types and custom patches on the owner's server. Do not silently treat non-headless or a null AI pointer as sufficient proof without a source audit and negative staging tests.

The CI `playerbots-fork-contract` job checks out that exact public Git commit and examines the expected class/lookup semantics. This only identifies the public API risk; **it does not certify the owner's installed fork** or human classification.

## Tests added

- `tests/raid_participation_tests.cpp`: 40-person mixed guild, Playerbots, unknown control, malicious duplicate GUIDs, recycled original guild ID, outsider guild, missing group/instance/encounter contribution, disabled bot source and threshold errors.
- `tests/raid_kill_credit_observation_tests.cpp`: disabled hook, raid-vs-nonraid, boss flag, same-instance checks, invalid IDs and permanent zero evidence/reward flags.
- `tests/raid_participation_source_tests.py`: scans the staging observer and policies for forbidden reward, persistence and guessed bot/classification functions.
- `tests/playerbots_source_contract_tests.py`: pins the public Playerbots implementation and validates that a null AI pointer is ambiguous.
- Pinned upstream real-header CI separately compiles the new PlayerScript observer only under the staging build option and verifies its symbol is absent in normal builds.

No real server was started or edited. The User's customised AzerothCore and Playerbots forks still require an audited complete staging compile and actual raid test.

## Release blockers

Source-verified map/difficulty/boss IDs for the **custom** Classic raids, named encounter-completion/credit evidence, valid real participation window, trusted positive human/bot determination, original guild generation at encounter time, bounded durable event/roster proof, owner-selected guild human minimum, actual private housing and verified Playerbots/IP/coexistence. Backups and rollback are required before staging world edits. Nothing should be installed in production now.


## Positive bot recognition in the optional staging observer

An additional opt-in **read-only** `StrongholdStagingPlayerbotsRead.cpp` now uses exactly the reviewed public fork's `sPlayerbotsMgr.GetPlayerbotAI(player)` and `sPlayerbotAIConfig.enabled`. This is not a robust human classifier. **Non-null, source-matched bot AI means a positively detected Playerbot. Null means UNKNOWN**, including when the bot module is off, disabled, the character is not registered, or custom server state differs.

Three conditions are all necessary before the staging bot read runs: explicitly compile `NAXX_GS_BUILD_STAGING_RAID_OBSERVER` AND `NAXX_GS_BUILD_STAGING_PLAYERBOTS_READ`, opt in to the raid observer config, and separately enable `StagingPlayerbotsRead.Enabled` **and** `StagingPlayerbotsRead.ReviewedSource` (both default 0). The reviewed-source switch is an operator attestation, **not** proof that the installed fork matches the pinned public checkout.

The staging `OnPlayerCreatureKillCredit` logger may then emit anonymous `BOT_CONFIRMED` or `UNKNOWN_NOT_HUMAN_PROOF` beside map/instance/creature numeric IDs. No character names/GUIDs, guild info or IP tiers are logged. This observation **never** produces `ParticipantControl::VerifiedHuman`, does not fill actual encounter participation evidence and cannot change rewards, database or game objects.

The new CI variant compiles the optional interface against public pinned Playerbots source (source not linked into game binary). Real deployed fork with all installed modules, human account/session validation, raid presence/contribution audit and positive human proof are still unverified. See [STAGING_PLAYERBOTS_READ.md](STAGING_PLAYERBOTS_READ.md).

## Attempt-bound combat event review model

The new independent `StrongholdEncounterContribution.*` pure policy illustrates an **offline candidate evidence envelope**: a server-authenticated encounter attempt, bounded event times, per-character matching actor/map/instance, distinct server event IDs, group membership at event time and target-encounter binding. It rejects spectator-only, shared kill-credit and zero-effective actions; permits independently evidenced damage, heals or mitigation as staging *candidates*. There is still no deployed-fork adapter and every candidate retains `EncounterParticipationVerified=false` and `TrophyGranted=false`. See [ENCOUNTER_CONTRIBUTION.md](ENCOUNTER_CONTRIBUTION.md). Do not set the existing per-member participation Boolean from a candidate.

## Source-signature preflight for exact installed forks

The scanner described in [RAID_SOURCE_PREFLIGHT.md](RAID_SOURCE_PREFLIGHT.md) searches C++ symbol occurrences for bot/session, boss lifecycle, group/guild and effective combat entrypoints. It has no runtime hooks or proof of real participation; all four provenance states stay UNVERIFIED. It is a manual code-review aid only, not a human classifier or a reward pathway.
