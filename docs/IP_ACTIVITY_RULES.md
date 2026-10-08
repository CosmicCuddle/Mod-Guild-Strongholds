# Individual Progression activity policy — first prototype

**Implementation state:** standalone C++ activity catalogue and eligibility rules, with no NPCs, quests, rewards, SQL writes or AzerothCore hook registration. This is **not an operational daily/weekly quest system**.

## Why this is per-character

The settlement and its buildings belong to the guild, but each visitor has their own Individual Progression state. A high-tier member cannot grant an unprogressed character access to Northrend or Outland service content simply by upgrading the guild hall.

The reviewed public Grimfeather/mod-individual-progression header (commit `706740808fee328b8557607f87b0548cf961e047`) defines `PROGRESSION_ONYXIA=2`, `PROGRESSION_NAXX40=7`, `PROGRESSION_PRE_TBC=8`, `PROGRESSION_TBC_TIER_5=13` (pre-Wrath gate) and `PROGRESSION_WOTLK_TIER_3=16`. The deployed checkout may still differ and must be verified before any live integration.

**Do not substitute character level or account level for real IP stage.** The future read-only adapter must retrieve and verify the installed module's progression data before supplying a monotonic rank to this code. If the adapter cannot verify compatibility, all progression-gated activities are locked.

## Initial prototype catalogue

| Key | Cadence | Settlement level | Upstream named IP milestone |
| --- | --- | ---: | --- |
| daily_supply_run | Daily | 1 | START |
| daily_defend_roads | Daily | 2 | START |
| daily_smithing | Daily | 2 | START |
| weekly_workshop | Weekly | 2 | START |
| weekly_vanilla_expedition | Weekly | 3 | START |
| weekly_outland_expedition | Weekly | 3 | PRE_TBC |
| weekly_northrend_expedition | Weekly | 4 | TBC_TIER_5 |
| onyxia_trophy_request | One-time | 4 | ONYXIA |
| icc_memorial_request | One-time | 6 | WOTLK_TIER_3 |

These are eligibility demonstrations, not final quest IDs, payout values or in-game content. A passed trophy-request eligibility gate **does not prove a guild killed the boss**. Boss credit needs an independent, idempotent authoritative encounter event and a permanent guild unlock record.

## Order of checks (fail closed)

1. Master feature enabled.
2. Known activity key.
3. Guild exists, and actor's server-verified guild ID matches the owning guild.
4. Bot contributions off by default until safely integrated.
5. Personal IP state known and source verified.
6. Settlement level valid and meets the activity requirement.
7. Personal IP milestone reached.

A separate **persistent quest and guild-contribution layer** must handle resets, project resource deduplication, material consumption, audit trails, account/bot limits and guild permissions before these activities become playable.

## Scope and compatibility

- `StrongholdActivities.h/.cpp` are pure code; no direct includes from IP or AzerothCore.
- The future glue layer must be compiled against the user's **actual installed** core and IP fork, rather than copying unknown enum offsets.
- Guild-scoped construction costs cannot be paid twice on event retries or login/reload.
- The shared settlement view stays the same for all guild members: Option B.
- Do not put housing changes into `Spell.dbc` or the existing IP database.


## Reviewed actual server IP fork — October 2026

The project's current source reference is **Grimfeather/mod-individual-progression**, not just its parent ZhengPeiRu21 repo. The reviewed public Git commit is `706740808fee328b8557607f87b0548cf961e047`. The user's *actually deployed* checkout version remains unknown until source inventory.

In that fork, `GetPlayerProgressionFromQuests(Player*)` finds the **highest rewarded hidden quest** at `66000 + stage`; it returns zero if the player isn't in the world. Importantly, `hasPassedProgression` also refuses progression when `enabled=false` or `progressionLimit` is configured below a requested milestone. A raw quest rank alone can therefore **overstate permitted access**. IP also responds to player zone/area changes and casts its own phase spells; Strongholds must never overwrite these auras or masks.

`StrongholdIpCompatibility.*` adds a **standalone read-only policy** that computes an effective stage from a verified fork contract, IP enabled flag, player-in-world status, rewarded-quest rank and `progressionLimit` (zero means uncapped). It resets old activity credentials if IP is disabled, unknown, changed or invalid; negative limits or unknown enum ranks are denied.

**Example:** a character retains WotLK quest progress (rank 18), but the server sets `progressionLimit = 7`. A guild expedition requiring `PROGRESSION_PRE_TBC=8` must still be unavailable.

The [fork contract review](IP_FORK_COMPATIBILITY.md) includes a pinned-source CI check and negative tests. A successful source check is *not* a tested, deployed read-only IP adapter. All actual-world activities remain disabled.
