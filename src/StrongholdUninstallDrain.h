#ifndef NAXX_GUILD_STRONGHOLDS_UNINSTALL_DRAIN_H
#define NAXX_GUILD_STRONGHOLDS_UNINSTALL_DRAIN_H
#include <cstdint>

namespace NaxxGuildStrongholds
{
// PURE review-only totals, never live DB queries. All booleans are future
// verified staging assertions, not facts supplied by an addon or user.
struct UninstallDrainEvidence
{
    bool ExactInstalledSourceReviewed = false;
    bool CharacterDatabaseBackupVerified = false;
    bool EntryAdmissionDisabled = false;
    bool AdmissionStopDurable = false;
    bool SnapshotAfterAdmissionStopped = false;
    bool SnapshotCompleteAndAuthenticated = false;
    bool NoPendingTeleportsOrTransfersVerified = false;
    bool IndependentPropertyMapSweepVerified = false;
    bool RecoveryHandlerStillAvailable = false;

    std::uint64_t TotalOutstandingTickets = 0;
    std::uint64_t PreparedTickets = 0;
    std::uint64_t InsideTickets = 0;
    std::uint64_t ReturningTickets = 0;
    std::uint64_t UnknownStageTickets = 0;
    std::uint64_t CharactersInsideHousing = 0;
};
enum class UninstallDrainDecision : std::uint8_t
{
    SourceUnverified,
    NoVerifiedBackup,
    NewVisitsNotFrozen,
    SnapshotUntrusted,
    RecoveryHandlerUnavailable,
    TransfersStillPending,
    InvalidTicketCounts,
    OutstandingTickets,
    CharactersStillInside,
    UnsafeUnknownState,
    CandidateForManualOperatorReview
};
struct UninstallDrainReview
{
    UninstallDrainDecision Decision = UninstallDrainDecision::SourceUnverified;
    // No purely computed check can authorise deleting tables, source, hooks
    // or removing the live emergency-return handler.
    bool MayRemoveRecoveryHandler = false;
    bool MayPurgeVisitTickets = false;
    bool MayModifyLiveDatabase = false;
};
UninstallDrainReview ReviewUninstallDrain(
    UninstallDrainEvidence const& evidence);
}
#endif
