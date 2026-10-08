# Guild property ownership — non-playable development foundation

## Scope

One registered Guild Stronghold property per guild. The chosen architectural theme is stored in `naxx_gs_settlement` with `guild_id` as its **primary key**. Guild leader/rank is validated by the authoritative AzerothCore server before any future claim.

### First two test properties
- Alliance, Human: `human` theme / Royal Stronghold.
- Horde, Orc: `orc` theme / Warlord's Fortress.

Ten themes exist in the catalogue, but only **Human** and **Orc** have initial logical layouts. Dwarf, Night Elf, Gnome, Draenei, Troll, Tauren, Forsaken, and Blood Elf property claims remain rejected until their layouts/assets are implemented. Guild leader race is irrelevant; guild faction restricts available themes.

## C++ domain code

`StrongholdProperty.*` provides `CheckPropertyClaim`. It denies disabled modules, missing server verification, guildless/cross-guild requests, non-guildmasters, bots, unknown/wrong-faction themes, unavailable layouts, and second property claims.

The pure C++ function cannot itself establish a guild's membership or prevent a database race. The **future runtime adapter** must read guild membership, actual guild faction and permissions from server-owned data rather than client selections.

## MariaDB contract tests

`tests/mysql_property_claim_test.py` is an **isolated CI-only** test:
- runs solely in the guarded `naxx_gs_ci_test` MariaDB database;
- uses the namespaced draft character-table schema;
- checks simultaneous claims to the same guild (exactly one succeeds because of the database primary key);
- makes the guild history record part of the *same transaction* as the property claim;
- injects failure after insertion to show no partial owner or history remains;
- tests separate Horde and Alliance guilds and same-schema reinstall persistence.

No real AzerothCore guild, guild leader, fee, spawn, map, phasing, portal or NPC exists in this proof. It only verifies the requested **storage and domain constraints**.

## Design rules

- Do not take over pre-existing public faction hubs.
- Private housing still requires a secure map/phase mechanism, not yet implemented.
- Property selection must never grant new IP unlocks.
- Initial ownership should be guild-wide and durable, even if the module is disabled.
- Relocation, guild disbanding and ownership transfer need explicit separate lifecycle rules and tests before being enabled.
- Safe uninstall preserves property records by default; only separately requested purge deletes them.

Do not run the schema/claim tests against the live characters database.


## Owner generation

A new `guild_created_at` field is required in the draft initial schema. Claim tests reject unknown creation dates and prevent a new guild with a recycled numeric ID from overwriting an existing record. The future adapter should query this from the actual Guild record (`Guild::GetCreatedDate()` exists upstream), not from the client. It is a defensive fingerprint rather than proof of globally unique guild identity. See [GUILD_LIFECYCLE.md](GUILD_LIFECYCLE.md).
