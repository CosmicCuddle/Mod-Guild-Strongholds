# Guild lifecycle safety — archive and recovery

**Development-only C++ policy and isolated MariaDB tests. No actual GuildScript, PlayerScript or teleport hooks are attached.**

## Guiding rule

Guild housing belongs to a verified **guild generation**, not just its numeric `guild_id`. The draft character table stores `guild_created_at`, `lifecycle_state` (`active` or `archived`), and `lifecycle_version`. Numeric ID and creation date are a defensive pair, **not a cryptographic unique identifier**. If the deployed fork or migration can reuse both, a more robust stable generation token must be designed before deployment.

## Lifecycle decisions

| Event | Expected rule |
| --- | --- |
| Guild leader transfer | Guild retains all settlement progress; permissions read live Guild ranks |
| Guild member leaves/kicked | Deny housing interaction immediately by checking membership on every action |
| Guild disbands | Archive property, *preserve* projects, supplies, decorations, raid trophies, contribution and audit records |
| Second disband callback | Remain archived without new ledger entry |
| New guild has same numeric ID | Deny access, contribution and property overwrite because original creation date differs |
| Server restarts | Archived state remains and prevents access |
| Original guild restored in validated DB restore | Administrator may explicitly reactivate after verifying **exact original guild identity** |
| Module disabled/uninstalled | All active and archived records preserved |
| Explicit permanent purge | Optional module-only deletion, requires an external backup |

## Upstream code evidence

In upstream AzerothCore `Guild::Disband()`, `sScriptMgr->OnGuildDisband(this)` is called before the guild's members and guild tables are deleted. Its Guild.h also offers `GetCreatedDate()` and `GetId()`. This means the relevant data is **potentially available**, but a module's SQL archive write must be completed safely before any dependent live cleanup. An asynchronous archive event could race guild deletion. The deployed core's actual hooks and transaction sequencing remain to be verified.

## Technical contract

- Only a **server-verified** guild ID and original creation date can authorise actions. Client gossip responses cannot supply authority.
- On every entry and every guild supply/material mutation: check membership, ownership generation, `lifecycle_state='active'`, permissions and bot/IP restrictions; perform the critical checks under database row lock.
- Archiving must atomically update `lifecycle_state`, increment `lifecycle_version` and insert an audit ledger entry, or roll back every change.
- Archiving is non-destructive and never deletes or resets any other mod's guild, character, IP or Playerbot records.
- Restoration must require administrator approval, exact original identity and proof of actual original guild restoration; do not auto-transfer to a successor guild.
- Leaving, teleporting, logout/login, death, guild disband and module uninstall must not strand players in a private area. Real eviction remains unsolved until isolation exists.
- Preserve the `guild_created_at` value when upgrading a guild's settlement. A change of leader does **not** mean a new guild generation.
- The revised draft CREATE script is an **initial schema**; no automatic upgrade or ALTER script exists.

## Implemented independent checks

- `StrongholdLifecycle.h/.cpp` and `tests/lifecycle_tests.cpp`: origin validation, archive, restore, replay, overflow, no data mutation on reject.
- `tests/mysql_guild_lifecycle_test.py`: disposable MariaDB transaction contract for archive/restore, rolled-back failure injection, repeated disband, generation mismatch, other-guild unaffected and archived contribution rejection.
- `tests/mysql_property_claim_test.py`: guild generation required for new claims, primary-key persistence.
- `tests/mysql_construction_test.py`: guild generation and archived-state check under settlement row lock.

**Not implemented:** production AzerothCore C++ database adapter, guild event listener, Playerbot/IP fork compatibility, quest NPCs, actual private housing map or evacuation. Never run this code's draft SQL on the live server.


## Unified gate for future teleport

`StrongholdVisitGate.cpp` now combines the original `CheckGuildEntry` policy **with** original-generation and active-property checks. A valid guild ID alone cannot permit entry if the visitor left, privacy is unverified, the settlement is archived or the ID belongs to a different guild generation. This helper remains pure testable C++: no teleportation and no guarantee that a future core adapter has yet supplied the authoritative inputs.
