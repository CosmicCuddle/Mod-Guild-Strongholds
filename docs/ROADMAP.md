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
