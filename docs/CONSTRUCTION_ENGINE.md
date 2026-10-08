# Guild construction engine — v0.3.0 groundwork (not gameplay-ready)

## Implemented as independent C++ logic

`StrongholdConstruction.h/.cpp` holds **12 proposed building projects** (six Human and six Orc) and a *pure*, non-mutating `PlanContribution` function. It validates the project key, settlement level, server-verified identity, guild membership, contribution permission, Playerbot opt-out, receipt status, amount limits, state integrity, and project version.

A proposal identifies the next resource balances and visual state, but it **does not deduct items, award guild supplies, write the database, spawn NPCs, or build structures**.

## Visual states

| State | Rule |
| --- | --- |
| Reserved | Nothing contributed |
| Gathering | Some resources, but not all resource categories half complete |
| Scaffolding | Every resource category at least half complete |
| Completed | All resource requirements met |

A completed project is not automatically a spawned gameobject. World appearance will need a tested loader/unloader compatible with guild isolation and valid client models.

## Contribution example

For a Human Workshop Court the prototype cost is:
- 200 Guild Supplies
- 100 Timber units
- 50 Iron units

Contributing 100 / 50 / 25 moves the project from Reserved to Scaffolding. A *new* accepted delivery of 100 / 50 / 25 finishes it. These are temporary balancing values, not chosen item IDs or live quests.

**Over-deliveries are fully rejected**. The caller must offer the correct amount; the engine will not silently consume and discard the excess.

## Transaction requirement (not yet built)

Only a future staging-tested persistence adapter may mark a contribution durable. The proposed schema adds a `naxx_gs_project` table with project balances and a version, plus a unique `receipt_key` scoped to the guild in `naxx_gs_contribution`.

The adapter must, in **one InnoDB transaction**:
1. Obtain server-authoritative guild identity and current resource ownership; verify rank/permissions and bot policy.
2. Lock the project row and read its current version and balances (or use a tested compare-and-swap update).
3. Check that the unique receipt has not already been used for that guild.
4. Evaluate the domain contribution plan against this **locked** snapshot.
5. Remove player items and guild supplies **without risking consumption on a failed transaction**. If player inventory updates cannot be transactionally coupled to SQL in this AzerothCore fork, design a separate secure escrow/compensation strategy and test it before enabling item contributions.
6. Insert the receipt and contribution audit record, update project resources and version, and append the guild ledger entry.
7. Commit; only then confirm success to the player. On error, roll back without awarding credit.

**A pre-read `ReceiptAlreadyCommitted=false` is not a race-safe uniqueness guarantee.** The database unique constraint and locking/serialization are required to block simultaneous duplicate submissions. The function is deterministic policy logic, **not persistence or concurrency safety by itself**.

Guild Supplies are currently also recorded on `naxx_gs_settlement`, so the future adapter must define a single accountable debit path and consistent balances; do not credit the same supply transfer twice.

## Undo and recovery

- Disabling must leave project rows and receipts untouched.
- A data-preserving uninstall removes code but retains records for reinstallation.
- Full purge SQL may drop only `naxx_gs_*` tables, now including `naxx_gs_project`, and must be intentionally invoked after backup.
- Existing player guild/character, IP and other module data must remain untouched.
- Draft SQL is **not** auto-applied or approved for the live server; the schema must be versioned and tested in staging first.

## Release gates

The GitHub unit test suite checks domain rules. Before declaring playable: test storage concurrency, rollback/restart, actual world objects, inventory accounting, all installed modules and safe housing privacy.
