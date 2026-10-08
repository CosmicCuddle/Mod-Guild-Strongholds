# Individual Progression activity policy — first prototype

**Implementation state:** standalone C++ activity catalogue and eligibility rules, with no NPCs, quests, rewards, SQL writes or AzerothCore hook registration. This is **not an operational daily/weekly quest system**.

## Why this is per-character

The settlement and its buildings belong to the guild, but each visitor has their own Individual Progression state. A high-tier member cannot grant an unprogressed character access to Northrend or Outland service content simply by upgrading the guild hall.

The current upstream ZhengPeiRu21/mod-individual-progression header defines `PROGRESSION_ONYXIA=2`, `PROGRESSION_NAXX40=7`, `PROGRESSION_PRE_TBC=8`, `PROGRESSION_TBC_TIER_5=13` (opens WotLK raid tier) and `PROGRESSION_WOTLK_TIER_3=16`. These are only research reference numbers. The user's installed version may have modified definitions.

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
