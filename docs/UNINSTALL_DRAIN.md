# Safe uninstall: drain and evacuation review (offline only)

**Never run a destructive purge or remove the only recovery handler while visitors could remain in a private area.** Our v0.1 code has no actual private area or live visitors, but this release requirement must be proven before a playable release.

The pure StrongholdUninstallDrain policy models the mandatory operator checks prior to a data-preserving code removal or a separately approved destructive purge. It never reads a live DB, teleports someone, removes a module, grants removal permission or deletes SQL.

## Preconditions for an operator review candidate

1. Audit the exact deployed AzerothCore and every installed module, and take/verify a restorable characters database backup.
2. Disable new stronghold entry **durably**, then re-scan after that barrier; an old snapshot is not sufficient.
3. Preserve the emergency return/recovery handler throughout evacuation, including when normal gameplay is disabled.
4. Confirm no pending teleports/transfers remain; run both an authenticated full characters-DB visit-ticket inventory and an independent original-property map/instance visitor sweep.
5. Check exact counts of Prepared, Inside, Returning and unknown-state tickets; malformed, overlapping or unclassified counts fail closed.
6. If ANY visit row remains, or any player still occupies housing despite no ticket, halt uninstall and recover the actual character first.
7. Re-check all gates after evacuation. Even all-zero synthetic counts yield CandidateForManualOperatorReview ONLY, never MayRemoveRecoveryHandler or MayPurgeVisitTickets.

A database query reporting zero tickets is not enough: crashed/outdated binaries, missing rows, long teleport loads and manual world positioning could still leave a player inside. Also test logout/offline characters; a logged-out character inside must be evacuated on login before removing recovery code. Snapshot source flags in the test fixture are claims, NOT real verification.

After passing this policy on real staging, a separate, written administrator sign-off, restore-from-backup test, correct module removal sequence and real AzerothCore character-recovery demonstration are required. Destructive purge must remain opt-in and specifically backed up. Unknown players/areas block removal.

The C++ tests run only synthetic counts. No live database connection or worldserver is touched. Revert the development commit to roll back this policy-only addition.
