# Delivery roadmap (provisional)

These are target milestones, not released functionality.

| Version | Scope | Release gate |
| --- | --- | --- |
| 0.1.0 | Safe module skeleton, config, schema proposal, documentation | Review source/module compatibility before building |
| 0.2.0 | Single reserved-location prototype, ownership, entry/exit, secure guild isolation | Two guilds do not see each other's players or objects |
| 0.3.0 | Building plots, construction, persistence and resource transactions | Restart + uninstall/reinstall tests |
| 0.4.0 | Guild daily tasks and weekly projects | Quest resets and abuse prevention verified |
| 0.5.0 | Read-only Individual Progression integration | Correct per-character quest/service gates |
| 0.6.0 | Raid trophy unlocks and event ledger | Boss attribution and no duplicate rewards |
| 0.7.0 | Ten racial style layouts and housing assets | Model/terrain collision and accessibility review |
| 0.8.0 | Controlled Playerbot activity and guild visitors | Permissions, exploits and bot caps tested |
| 0.9.0 | Relocation, seasonal decoration and polish | No lost owned decor or progress |
| 1.0.0 | Stability, backups, uninstall and reinstall hardening | Production acceptance checklist completed |

Each milestone requires a clean revert path, schema upgrade plan, test checklist and changelog entry. Do not bundle experimental core patches into a normal release.

## Compatibility gate for every milestone

Before each playable milestone is accepted: confirm the full existing module inventory and run the relevant tests in [COMPATIBILITY.md](COMPATIBILITY.md). Additions to module-related hooks, phasing, SQL gameobject IDs, IP, Playerbots or dungeon-completion logic require focused regression tests. A clean build by itself does not prove runtime compatibility.

## Progress in the development branch

- v0.1.0 foundations: inert loader, draft SQL, configuration and safety/compatibility documents.
- v0.2.0 settlement catalogue (non-playable substep): ten defined race themes, Human/Orc plot definitions and pure access policy tests.
- **Still outstanding for v0.2.0 proper:** evaluated private-area mechanism, approved map/asset coordinates, server-side NPC and teleport implementation, isolation tests with the *actual* IP and Playerbots configuration.


## Subsequent research checkpoint

Upstream MapMgr and MapInstanced source indicates that standard dungeon/raid instance routing is based on player/group instance saves, **not guild ownership**. No production-safe guild private-space mechanism has been selected yet.

Per-character activity gate code is now available for testing in `src/StrongholdActivities.*`. Runtime compatibility and IP adapter are still outstanding.

## v0.3.0 prototype substep — construction domain model

Pure C++ `StrongholdConstruction.*` now defines twelve Human/Orc building projects and four visual-stage transitions. This is an **early design substep**, not the completion of v0.3.0. The actual persistence adapter, resource debit, unique receipt transaction, objects, privacy and rollback testing are still outstanding.

See [CONSTRUCTION_ENGINE.md](CONSTRUCTION_ENGINE.md) for strict transaction/rollback requirements.

## MariaDB persistence contract experiment

A separate CI job now applies the draft schema to a throwaway MariaDB database and tests supply-only InnoDB transaction semantics with receipt uniqueness, rollback and concurrent deposits. See [PERSISTENCE_CONTRACT.md](PERSISTENCE_CONTRACT.md). It is **not** an AzerothCore runtime adapter or live migration. Item/timber/iron consumption stays prohibited until transaction/escrow design and deployed-core testing.

## Diagnostic bootstrap checkpoint

The loader now registers a passive **startup/config logging** WorldScript only; no gameplay can be activated in the current development build. Mock-core compilation and gate tests are available; a full build against the actual AzerothCore fork and module stack remains a release blocker.

## Property ownership domain milestone

The pure C++ property selection policy and MariaDB claim transaction **test-only contract** now exist. The underlying `naxx_gs_settlement.guild_id` primary key prevents multiple simultaneous guild property claims in the isolated CI database. This does not imply that guild member/faction auth, game location isolation or actual purchase menus have been integrated into worldserver. See [PROPERTY_CLAIMS.md](PROPERTY_CLAIMS.md).


## Ownership recovery checkpoint

Property lifecycle logic now addresses guild leadership changes, disband, stale guild IDs, archiving and possible restoration. This is a source-only / temporary MariaDB contract, not operational on the live server. In-game GuildScript registration, real membership verification, safe evacuation, migration and staging compatibility remain blockers. See [GUILD_LIFECYCLE.md](GUILD_LIFECYCLE.md).


## v0.2.x Safe Return — development progress

`StrongholdVisitRecovery.*` and the disposable `naxx_gs_visit` database test now model a three-step entry/return process and recovery after a failed teleport or disband. **This is groundwork, not a completed v0.2.0.** Actual guild-private map/phase routing remains the release blocker. Read [SAFE_RETURN_CONTRACT.md](SAFE_RETURN_CONTRACT.md) and [ISOLATION_DECISION.md](ISOLATION_DECISION.md) before adding teleport scripts.

## Isolation feasibility progress — source audit & model tests

The development branch now contains a read-only source inventory scanner and C++ tests modelling both phase-comparison modes. In particular, two players can have **asymmetric** visibility if one uses combined phase bits and the other exact matching. These tests help specify what a staging experiment must catch; they cannot prove the chosen isolation mechanism works on the deployed server.

Next gate is collecting the actual module fork versions, selecting a reserved private-area mechanism and completing a **real two-guild** staged visibility/teleport test. No production approach has been approved.
