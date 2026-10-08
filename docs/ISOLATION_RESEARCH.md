# Isolation research and go/no-go decision

**Current state: not yet proven.** The pure catalog and access helper do **not** implement private guild areas.

## What we know

- AzerothCore's `mod-guildhouse` is useful as design reference, but its implementation derives phase IDs from guild IDs and relies on reserved-area-specific behaviour. It cannot simply be transplanted into ten existing town zones.
- AzerothCore's Individual Progression module already changes visibility/content by character progress, so world phase interactions are potentially fragile.
- Ordinary client/world phase masks have limited capacity and cannot be assumed to yield arbitrarily many independent guild spaces.

## Candidate isolation mechanisms (for evaluation, not implementation yet)

1. **Dedicated reserved map/zone with audited guild separation:** closest to an existing guildhouse pattern but needs proof of correct object and player visibility and safe phase restoration for the deployed core/forks.
2. **Private map instances:** may provide clearer physical separation, but instance creation, persistence, group/guild routing, NPC/world persistence and teleport handling need investigation on this specific core.
3. **Purely separate plots on shared-world coordinates:** unacceptable for privacy if a visitor can enter another guild's area or see its objects; not a default solution.

The system must not set global player phases blindly or patch global map-visibility checks just to gain a fast prototype.

## Required executable demonstration

- Two test guilds in the same property type enter concurrently.
- Both see **only** their own guild players, NPCs and objects.
- Leaving, logging out and relogging must never strand the player or corrupt IP phases.
- Guildless and cross-guild entry attempts fail on the server.
- Other installed modules see normal combat, area, quest and teleport behaviour outside housing.
- Server restart restores guild housing ownership and optional data without leaking objects.
- Disabling/uninstalling the feature evacuates or safely recovers affected characters.

## Stop conditions

Do not enable the future Guild Steward, preview, purchase, teleport or building placement if isolation is uncertain. Compile-time C++ unit tests for a mock access policy are necessary but **not sufficient**. Staging worldserver testing is mandatory.

## Data/logic boundary

`src/StrongholdCatalog.*` contains logical themes and plot names only. `CheckGuildEntry` is a **pure policy function** accepting prevalidated IDs and a test-established isolation-status value; it is not called from AzerothCore hooks and it does not teleport or spawn anything.

Further code should introduce a server-only adapter to acquire authoritative guild membership, not trust any ID sent by the client. Real-world server hooks and map APIs must be checked against the exact deployed core revision before integrating this policy.
