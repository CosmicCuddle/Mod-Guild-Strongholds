# Naxxramas Guild Strongholds — project handover and working rules

**Canonical code:** [CosmicCuddle/Mod-Guild-Strongholds](https://github.com/CosmicCuddle/Mod-Guild-Strongholds)  
**Active development:** `feature/v0.1.0-foundations`, [Draft PR #1](https://github.com/CosmicCuddle/Mod-Guild-Strongholds/pull/1).  
**Production status:** **NOT DEPLOYABLE.** `main` intentionally remains the initial README; the development branch contains prototypes and opt-in read-only observers only.

Read this first when resuming after a long conversation. This document preserves product choices and delivery constraints; it does not replace the technical evidence in the linked documents.

## 1. Agreed gameplay direction

1. **Shared settlement, personal activities — Option B.** Each guild owns one shared stronghold. All eligible members see the same persistent settlement, constructed buildings, amenities and earned trophy displays. Quest availability, NPC services, interactions and rewards are gated by the **individual character's verified IP state**, including the server-wide progression cap; one advanced member never grants progression access to another.
2. **Faction and racial choice.** An Alliance guild can choose Human, Dwarf, Night Elf, Gnome or Draenei styling; Horde can choose Orc, Troll, Tauren, Undead or Blood Elf. The guild leader's race does not restrict choices within their faction. Human and Orc are the *logical* early prototypes; other architecture and coordinates are not implemented.
3. **Actual bespoke sites, not a pasted-over familiar town.** Do not tell players that a random permanent building in a recognisable existing town (e.g. Razor Hill) is their stronghold. Evaluate prepared/reserved areas and legitimate 3.3.5-compatible assets. World privacy and art/terrain/collision checks must precede spawns or teleportation.
4. **Guild-scale progression.** Seven planned guild development stages: Founding Camp, Established Outpost, Growing Village, Fortified Settlement, Guild Stronghold, Grand Stronghold, Legendary Stronghold. These are separate from IP's per-character eras/ranks.
5. **Construction that visibly evolves.** Logical plots permit approved buildings and staged appearance changes: empty site, supplies/scaffolding, workers, completed structure. Preserve ownership, contributions and placed trophy history through relocation/replacement where approved. Free-form arbitrary placement is *not* a first-release feature.
6. **Daily and weekly reasons to return.** Guild resource projects, small-guild-friendly incremental progress, tier-appropriate individual objectives, collaborative weeklies and eventually raids. No mandatory large roster or unlimited automated Playerbot farming.
7. **Raid trophies with real achievement provenance.** Onyxia's head, Ragnaros' flame, Nefarian banner, C'Thun relic, and Kel'Thuzad sigil are *concepts* pending verified boss/GO/DBC IDs. A server-verified relevant boss completion plus independently eligible participating **real human** members of the same original guild generation is required. Allow 40-player and mixed-guild raids, evaluate each guild separately. Bots may assist but do not inflate the proposed human minimum. The final minimum (range 2..40) requires owner sign-off.
8. **Historical legacy.** Permanent achievements, ledger, projects and decoration ownership survive normal disable/uninstall; a guild disband archives its property. Never award an old guild's trophies to a new guild with the same recycled numeric guild ID.
9. **Permissions and safety.** Server validates actual guild membership, leader/rank authorisation, faction, IP, progress, phase/map privacy and financial/event replay protections. Do not trust addon/client input as proof of raid completion or property rights.
10. **A way to undo everything.** Distinguish (A) disabling with data preserved, (B) code removal with data preserved, and (C) user-authorised destructive purge *after backups and safe evacuation*. Never automatically execute data purges or strand players.

## 2. Technical boundaries that must never be bypassed

- AzerothCore **3.3.5** separate module. Do not silently change upstream AzerothCore, Individual Progression, Playerbots, Mod-Naxxramas-Core or any other existing module.
- Use `NaxxGuildStrongholds.*` config prefix, `naxx_gs_*` characters tables, namespaced C++ code. All draft SQL remains **manual and unapplied**; no automatic migrations.
- Treat compatibility with **all actually deployed modules** as a release gate, including IP phasing/caps, Playerbots, custom Naxxramas Core, dungeon-completion logic, existing guild-house/phasing systems and other installed modules. Module repos alone are not proof of installation.
- Default-on gameplay remains prohibited: `StrongholdStartupGate.h` hard-blocks housing. Separate staging scripts require compile flags **and** default-off configs.
- A core compile against pinned public upstream is useful, but **not** proof for the owner's customised source/binary. Never claim tests passed while a CI job is queued/in progress.
- Private housing mechanism is UNRESOLVED. Ordinary instances follow group/player saves, not demonstrated guild ownership; general phase bits can collide with IP and have limited capacity and asymmetric visibility. No arbitrary routing shortcut, phase assignment or teleport in current module.
- Server-side character origins and durable safe-return tickets must precede entry. Guild disband, restart, logout, death, teleport failures and uninstall must never strand members.
- Playerbots read-only public source can positively identify an active registered **bot**, but absence of bot AI is **UNKNOWN**, never verified human. The installed fork needs its own reviewed source contract.
- `OnUnitDeath` and `OnPlayerCreatureKillCredit` can only produce read-only candidate observations. Neither proves authoritative encounter completion or a human guild contribution. The new `StrongholdEncounterContribution.*` event provenance model produces staging **candidates only**, not `EncounterParticipationVerified`, `VerifiedHuman` or trophy rewards.
- No active trophy unlock, construction payment, guild property acquisition, actual quest/NPC spawn, GameObject placement, guild-private map, IP mutation, player teleport or database write pathway is implemented.

## 3. What exists in development source

- Race/faction settlement catalogue, seven-stage design, initial Human/Orc logical plots and construction proposals.
- Guild/player access, tier-capped IP activity policy, original guild generation/lifecycle/archival policies, safe-return visit model, phase-lease/instance-routing feasibility models.
- Namespaced **draft** schema, mock/temporary MariaDB transactions for property, contribution, original guild generation, trophy receipts and rollback/replay protection. These DO NOT handle actual item inventory or live AzerothCore DB.
- Passive default loader, opt-in staging GM snapshot, opt-in read-only Steward gossip, verified current guild source, read-only property SELECT and read-only Grimfeather IP seam.
- Staging-only read-only raid death/kill-credit observers and positive-bot-only Playerbots inspection, with anonymous numerical observations and no persistent reward.
- Raid trophy concepts and pure proposal functions, participation/Playerbots policies, and attempt-bound event contribution *review candidates*.
- C++ test runner, static mutation guards, pinned-upstream AzerothCore compile/link workflow, pinned public IP/Playerbots source checks and disposable MariaDB tests.

The accurate files are under `src/`, `tests/`, `docs/`, `conf/`, `data/sql/manual/`, `.github/workflows/` and `uninstall/`. For feature and release-status details, read [ROADMAP.md](ROADMAP.md), [COMPATIBILITY.md](COMPATIBILITY.md), [RAID_PARTICIPATION.md](RAID_PARTICIPATION.md), [ENCOUNTER_CONTRIBUTION.md](ENCOUNTER_CONTRIBUTION.md), [ISOLATION_DECISION.md](ISOLATION_DECISION.md), [ROLLBACK.md](../ROLLBACK.md), and [UNINSTALL.md](../UNINSTALL.md).

## 4. Mandatory continuing-work method

1. Check this handover, feature branch head, open Draft PR #1, recent commits, outstanding GitHub issues and relevant docs/code **before** implementing the next step.
2. Prefer a self-contained, testable code batch over asking the owner to test every few minutes. Commit to the development branch with a descriptive message; keep `main` and production untouched.
3. Preserve rollback for every batch: specific commit link, list of new/edited files, tests, how to revert safely. Do not use destructive force push or automatic merge.
4. Run `bash tests/run-catalog-tests.sh`, relevant Python static checks and GitHub CI. If tests fail, fetch precise job logs, fix source and re-check. Treat a pending job as pending.
5. Use *read-only* research to audit actual public API behavior; do not claim the deployed fork matches public source. Record exact upstream commit references used in CI.
6. Before any **server** step, ask for the missing actual source revisions, module list and staging availability. Explicitly guide full character/world DB, configs, binaries and source backups and test restore. Do not ask for SSH, DB credentials or secret config values.
7. Before applying any future SQL or assigning IDs, list exactly what tables/rows/creature/GO/quest/DBC IDs will change, why, and a selective uninstall/rollback procedure. Never use unreviewed global IDs or auto-delete other-module data.
8. In owner-facing replies, explain steps in clear short sections and concrete copyable commands when actions are needed; use exact paths, git branch/commit and descriptive commit message; avoid unclear abstract instructions. No unnecessary emoji.
9. Prefer addressing unresolved privacy, safe exits, correct human/guild participation and actual deployed compatibility over rushing visible NPC/trophies into the world. Strongholds must not conflict with other server features.

## 5. Major remaining release blockers

| Blocker | Required proof |
| --- | --- |
| Guild-private space | Approved reserved site; two guilds cannot see/interact with one another; repeat with IP/Playerbots, restart, relog, pets, summons and all relevant objects |
| Actual deployed source stack | Core and all module revisions/patches, current build, fork-specific APIs; full staging build/link/run and compatibility regression |
| Safe return | Verified server-origin location, visit state and evacuation on login/disable/disband/uninstall |
| Actual character IP | Exact installed Grimfeather fork/API/cap semantics; lower tier cannot interact with locked tier features or gain restricted rewards |
| Construction resources | Authoritative character inventory debit and rollback-safe escrow + idempotent server receipts, never fake supplies that can duplicate items |
| Guild raid evidence | Source-authenticated boss encounter/attempt, original guild and member identity at time, positive human/bot classification, actual per-member combat contribution and raid group identity; 40-player mixed-guild test |
| Assets and placement | Reviewed 3.3.5 creature/GO/quest/DBC IDs, usable models and collision, reversible exact-object cleanup |
| Data migrations | Backup/restore, versioned migrations, idempotency, preserve-data uninstall and explicitly separate purge after evacuation |

**Next likely engineering step:** review exact deployed Playerbots and AzerothCore source to design a **positive human-control and event-time group participation adapter**. If those sources are unavailable, continue with isolated tests and research only; do not invent a successful proof.

## 6. No live action requested

At this stage the owner does **not** need to `git pull`, edit HeidiSQL, run MobaXterm commands, recompile or restart a realm to benefit from this development branch. Only after a clear, separate owner-approved staging plan should that change.

## 8. Source preflight extension

A new read-only helper (scripts/audit-raid-source-readiness.py) prepares a manual review of exact deployed AzerothCore, Playerbots and raid event APIs on a separate source copy. It *never* approves positive human control, boss completion, member contribution, guild generation or trophies. See [RAID_SOURCE_PREFLIGHT.md](RAID_SOURCE_PREFLIGHT.md). Running it is optional until a copied staging checkout is available.

## 9. Group/roster timeline development

The domain-only StrongholdRaidMembershipTimeline tests historic join/leave and original guild identity against a proposed raid attempt. It cannot prove real group or human status, cannot award trophies and requires the exact installed-fork source review. See [RAID_MEMBERSHIP_TIMELINE.md](RAID_MEMBERSHIP_TIMELINE.md).

## 10. Future privacy staging coverage

A pure StrongholdPrivacyMatrix requires both A/B directions in base, mixed IP, Playerbots, relog, restart and guild-change contexts. No observations can turn on the housing feature. See [PRIVACY_STAGING_MATRIX.md](PRIVACY_STAGING_MATRIX.md).

## 11. Interrupted visit after restart

ReviewInterruptedVisit now models Prepared, Inside and Returning ticket reconciliation after crash/relog, with no live teleport, SQL changes, or automatic ticket deletion. It rejects unknown visit stage enum values. See [RECOVERY_RECONNECT.md](RECOVERY_RECONNECT.md); actual full-stack recovery still blocked.

## 12. Safe code removal and full visit drain

The pure StrongholdUninstallDrain model now rejects uninstall review with any outstanding return tickets, in-map visitor, invalid stage counts, unfrozen entry, pending transfers, missing backup or missing recovery handler. Even an empty staging-data fixture never authorizes actual module removal. See [UNINSTALL_DRAIN.md](UNINSTALL_DRAIN.md).

## 13. Actual source-linked GM-self visit status

A distinct staging-only option now uses an authenticated staff Player GUID in a one-row SELECT against an already reviewed separate staging characters table; .naxxgs visit displays status only. It is disabled/absent from normal builds and cannot return players or delete tickets. See [STAGING_VISIT_READ.md](STAGING_VISIT_READ.md).
