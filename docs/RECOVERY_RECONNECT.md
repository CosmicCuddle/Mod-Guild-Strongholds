# Crash, reconnect and interrupted stronghold return — policy review

**Runtime recovery is not implemented.** The existing durable ticket contract uses Prepared, Inside and Returning. A crash or disconnect can interrupt an outbound teleport after Returning is committed, so the row must survive and be reviewed without overwriting the original safe position or nonce.

The added pure C++ ReviewInterruptedVisit policy classifies:
- Returning and visibly back at the saved origin: a candidate for verified final reconciliation, never automatic row deletion.
- Returning but independently verified still inside the original private property: a candidate for retrying the ORIGINAL return point, not creating a new visit.
- Prepared at origin: a candidate to reconcile an entry teleport that never happened.
- Prepared/Inside still inside, or Inside already back at origin: a candidate to persist Returning before any exit.
- Unknown third map, false source, unvalidated original landing, mismatched player, moving/teleport-pending, contradictory ownership or invalid ticket: no automatic action, manual recovery or wait required.

The coordinate proximity tolerance (2 X/Y and 4 Z) is a **synthetic heuristic only**; it cannot prove the client's actual landing or map/instance safety. Likewise the InsideOriginalPropertyVerified Boolean represents hypothetical reviewed-source attestation, not working housing. No returned enum means permission to teleport or clear SQL.

A real recovery handler must run when gameplay is disabled and after relog/restart, derive the player and original ticket from server/characters DB, verify map collision, and use compare-and-swap before retries. It must support safe homebind/GM fallback when the original map is no longer valid. Before uninstall, all outstanding visits need verified evacuation, with backups and independently checked return completion. No actual fallback, hook, DB adapter, phased world or teleport exists in this milestone.

Tests now exercise interrupted Returning, never-left Prepared, Inside with manual movement, disbanded guild, unknown/malicious state, wrong instance, third map, pending transfer and corrupted stage. GitHub tests use synthetic C++ state only. To roll back, revert the development commit; no migration or live state exists.

**Do not install, apply SQL or restart your actual realm for this milestone.**
