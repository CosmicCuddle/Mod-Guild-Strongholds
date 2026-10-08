# Phase-slot reservation prototype — strict staging-only scope

**This is NOT functioning guild housing, a privacy guarantee, or an approved design decision.**

We have one candidate worth testing: if a dedicated housing zone uses normal combined-phase bitmask comparison consistently, each guild could receive an independently reserved single-bit phase value. The prototype builds only the allocation and persistence contract; it DOES NOT touch AzerothCore WorldObjects.

## Design and limits

- AzerothCore's combined phase comparison treats masks as sets of bits. Two disjoint *single-bit* masks cannot intersect in the phase comparison model.
- All installed module phase spells, GM/all-phase objects, global phase overrides, IP area/zone effects, creatures, gameobjects and ordinary visibility logic must be checked **on the real server**.
- A `uint32` phase mask has a theoretical maximum of **32 single-bit positions in total**, many of which may already be reserved, overlap existing content or be otherwise unsafe. It is **not** a scalable unlimited-guild architecture.
- There are **no preselected bits** in configuration, no production defaults, no automatically safe phase namespaces and no known private map.
- For future test fixtures only, CI simulates a pool with bits 24 and 25 and an unrelated mask bit 0. Those assignments are NOT permission to use these bits on the actual realm.
- Existing allocations must be treated as permanently occupied, including after guild disband or module disable. No automated reclamation; even a departed guild member or lingering gameobject could otherwise cross into another guild's future phase.

## C++ allocator policy

`StrongholdPhaseLease.h/.cpp`:

1. Requires reviewed **actual deployed** source/module inventory and approved phase-comparison assumptions.
2. Requires independent verified guild identity and active property ownership.
3. Requires an explicit administrator-approved phase mask, not a guild-ID-based guess.
4. Refuses if any approved bit overlaps a bit already observed in other modules.
5. Rejects malformed or duplicated historical lease rows, guild ID reuse and already-retired owners.
6. Selects a single unused phase bit; rejects new guilds when the pool runs out.
7. Marks retirement only after a verified lifecycle/admin event; the bit is still permanently reserved.

An unverified or refused request returns no proposed bit. A proposal does not change a player or object.

## Disposable MariaDB reservation contract

`naxx_gs_isolation_slot` is a DRAFT ninth module-owned character table:
- `guild_id` PRIMARY KEY, with separately recorded `guild_created_at`;
- `phase_bit` UNIQUE, so concurrent claims cannot silently share one;
- `lease_state` held/retired, version for optimistic retirement.

`tests/mysql_phase_lease_test.py` runs only on protected disposable `naxx_gs_ci_test`:
- two simultaneous guild requests for two fake bits get distinct allocations;
- duplicate guild, recycled ID, exhausted pool, external conflict, unknown source or mode all refuse;
- failure injection after inserting the phase lease and its ledger rolls back;
- retirement requires archived guild, preserves the bit, and never permits automatic reuse;
- other guilds and same-version reinstalls retain their records.

This is Python **test-only code**. A production C++ database adapter does not yet exist, and this example does not demonstrate worldserver transaction safety.

## Release decision

Candidate B is viable *only if* a full staging copy proves:
- every available phase bit is really unclaimed and collision free;
- all players, NPCs, objects, summons, bots and relevant content use compatible phase comparison in the reserved zone;
- Individual Progression's phase spells and era-specific NPC/quest filtering remain untouched;
- two different guilds simultaneously occupy the same logical site with no A↔B visibility, collision, gossip or interaction leakage;
- guilds never lose safe exit on logout, restart, archive, owner transfer or uninstall;
- realistic expected guild count fits the finite reviewed pool without exhaustion.

If even one fails, **do not force the allocation system into production**. True per-guild map instances (candidate A) still require a separate fork-specific technical solution and remain an option.

No gameplay release may flip `DevelopmentCapabilities.PrivacyIsolation` based on these standalone tests alone.
