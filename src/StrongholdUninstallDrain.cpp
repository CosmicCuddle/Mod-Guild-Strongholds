#include "StrongholdUninstallDrain.h"

namespace NaxxGuildStrongholds
{
UninstallDrainReview ReviewUninstallDrain(UninstallDrainEvidence const& e)
{
    UninstallDrainReview result;
    if (!e.ExactInstalledSourceReviewed)
        return result;
    if (!e.CharacterDatabaseBackupVerified)
    {
        result.Decision = UninstallDrainDecision::NoVerifiedBackup;
        return result;
    }
    if (!e.EntryAdmissionDisabled || !e.AdmissionStopDurable ||
        !e.SnapshotAfterAdmissionStopped)
    {
        result.Decision = UninstallDrainDecision::NewVisitsNotFrozen;
        return result;
    }
    if (!e.SnapshotCompleteAndAuthenticated ||
        !e.IndependentPropertyMapSweepVerified)
    {
        result.Decision = UninstallDrainDecision::SnapshotUntrusted;
        return result;
    }
    if (!e.RecoveryHandlerStillAvailable)
    {
        result.Decision = UninstallDrainDecision::RecoveryHandlerUnavailable;
        return result;
    }
    if (!e.NoPendingTeleportsOrTransfersVerified)
    {
        result.Decision = UninstallDrainDecision::TransfersStillPending;
        return result;
    }

    // Avoid count overflow from corrupt untrusted / incomplete staging
    // exports; UnknownStageTickets may never silently be ignored.
    if (e.UnknownStageTickets > e.TotalOutstandingTickets ||
        e.PreparedTickets > e.TotalOutstandingTickets ||
        e.InsideTickets > e.TotalOutstandingTickets ||
        e.ReturningTickets > e.TotalOutstandingTickets)
    {
        result.Decision = UninstallDrainDecision::InvalidTicketCounts;
        return result;
    }
    auto remaining = e.TotalOutstandingTickets - e.UnknownStageTickets;
    if (e.PreparedTickets > remaining)
    {
        result.Decision = UninstallDrainDecision::InvalidTicketCounts;
        return result;
    }
    remaining -= e.PreparedTickets;
    if (e.InsideTickets > remaining)
    {
        result.Decision = UninstallDrainDecision::InvalidTicketCounts;
        return result;
    }
    remaining -= e.InsideTickets;
    if (e.ReturningTickets != remaining)
    {
        result.Decision = UninstallDrainDecision::InvalidTicketCounts;
        return result;
    }

    if (e.UnknownStageTickets)
    {
        result.Decision = UninstallDrainDecision::UnsafeUnknownState;
        return result;
    }
    if (e.TotalOutstandingTickets)
    {
        result.Decision = UninstallDrainDecision::OutstandingTickets;
        return result;
    }
    if (e.CharactersInsideHousing)
    {
        result.Decision = UninstallDrainDecision::CharactersStillInside;
        return result;
    }

    // Still no release authorization: source-backed real-world evacuation,
    // backups, rollback and GM/operator acceptance are independent gates.
    result.Decision = UninstallDrainDecision::CandidateForManualOperatorReview;
    return result;
}
}
