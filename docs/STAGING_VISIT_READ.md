# Staging-only GM visit ticket reader (read-only)

**Status:** separate opt-in C++ adapter targeting pinned real AzerothCore headers. No private housing, login recovery, SQL migration or in-game teleport.

The new optional GM command is .naxxgs visit. It queries only the issuing staff character's current ticket, using that character's server-side GUID and a single SELECT from the already-existing test table naxx_gs_visit. It shows only PREPARED, INSIDE, RETURNING, INVALID, OFF, or NO ROW OR DATABASE UNAVAILABLE. It never shows character identity, original guild ID/creation, return coordinates, the visit nonce or other secrets.

## Three independent barriers

1. An isolated staging build must set BOTH NAXX_GS_BUILD_STAGING_DIAGNOSTICS and NAXX_GS_BUILD_STAGING_VISIT_READ compiler definitions; neither exists in a normal build.
2. Both NaxxGuildStrongholds.Diagnostics.Enabled and NaxxGuildStrongholds.StagingVisitRead.Enabled must be explicitly set to 1 in the separate backed-up staging config. They are 0 by default.
3. The command uses the core GM RBAC debug permission and forbids console; the DB adapter independently requires the issuing player to be an in-world GM. No user GUID, guild or SQL is accepted as a chat parameter.

The backed-up test characters database must ALREADY contain the reviewed draft naxx_gs_visit table, applied separately with full backup and approval. The adapter never installs SQL. A missing result can mean NO row, missing schema or failed database; none permits a safe-return claim.

The row is parsed through an independent fail-closed policy, including actor match, original guild generation, stored safe origin, nonce syntax and valid visit stage. Even a valid Returning row does NOT certify arrival, authorise deleting tickets or issue a teleport. Another player's tickets cannot be queried.

## Validation and rollback

Standalone tests cover malformed fields, all stages, source uncertainty, identity mixups and never revealing the return position. The GitHub upstream workflow compiles the adapter in one explicit staging variant, checks it is excluded from the normal module, and links the ordinary worldserver unchanged. This validates a pinned PUBLIC AzerothCore source, not the actual custom realm.

To remove it from a staging test build, reset both settings to 0 and rebuild without the compile flags. It owns no schema/data, world objects, spawn or phase assignment, so no SQL uninstall is needed for this reader.

The eventual recovery handler, two-guild isolation, audited actual module source, full player evacuation and real character-DB compare-and-swap remain mandatory release blockers. Do not deploy this branch to the live server.

## Command-level negative smoke tests

A separate fake-AzerothCore test build compiles the diagnostic with BOTH staging flags and a mock ticket reader. It confirms the new visit subcommand is RBAC-gated, disabled by default, only passes the issuing character into the reader, reveals only a status and does not change player state. The ordinary mock test still confirms snapshot is the only command without the extra visit compile flag. The real public-core CI separately compiles the actual database adapter.
