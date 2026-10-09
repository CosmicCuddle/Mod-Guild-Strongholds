#include "StrongholdUninstallDrain.h"
#include <iostream>
#include <limits>
using namespace NaxxGuildStrongholds;
namespace
{
UninstallDrainEvidence ReadyFixture()
{
    UninstallDrainEvidence e;
    e.ExactInstalledSourceReviewed = true;
    e.CharacterDatabaseBackupVerified = true;
    e.EntryAdmissionDisabled = true;
    e.AdmissionStopDurable = true;
    e.SnapshotAfterAdmissionStopped = true;
    e.SnapshotCompleteAndAuthenticated = true;
    e.NoPendingTeleportsOrTransfersVerified = true;
    e.IndependentPropertyMapSweepVerified = true;
    e.RecoveryHandlerStillAvailable = true;
    return e;
}
}
int main()
{
    unsigned checks=0,failures=0;
    auto test=[&](bool good,char const* why){
        ++checks;
        if(!good){++failures;std::cerr<<"FAIL: "<<why<<'\n';}
    };
    auto e=ReadyFixture();
    auto result=ReviewUninstallDrain(e);
    test(result.Decision==UninstallDrainDecision::CandidateForManualOperatorReview,
         "Empty externally asserted inventory is only a manual review candidate");
    test(!result.MayRemoveRecoveryHandler && !result.MayPurgeVisitTickets &&
         !result.MayModifyLiveDatabase,
         "Even perfect-looking evidence never authorises uninstall");

    auto x=e;x.ExactInstalledSourceReviewed=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::SourceUnverified,
         "Installed fork unreviewed");
    x=e;x.CharacterDatabaseBackupVerified=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::NoVerifiedBackup,
         "Never uninstall without verified backup");
    x=e;x.EntryAdmissionDisabled=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::NewVisitsNotFrozen,
         "New admission races refuse scan");
    x=e;x.AdmissionStopDurable=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::NewVisitsNotFrozen,
         "In-memory pause is insufficient");
    x=e;x.SnapshotAfterAdmissionStopped=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::NewVisitsNotFrozen,
         "Old snapshot may miss new visitors");
    x=e;x.SnapshotCompleteAndAuthenticated=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::SnapshotUntrusted,
         "Unknown DB completeness");
    x=e;x.IndependentPropertyMapSweepVerified=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::SnapshotUntrusted,
         "DB alone cannot prove no stranded players");
    x=e;x.RecoveryHandlerStillAvailable=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::RecoveryHandlerUnavailable,
         "Do not remove the only emergency handler");
    x=e;x.NoPendingTeleportsOrTransfersVerified=false;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::TransfersStillPending,
         "Pending transfer may strand characters");

    x=e;x.TotalOutstandingTickets=1;x.PreparedTickets=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::OutstandingTickets,
         "Prepared ticket prevents uninstall");
    x=e;x.TotalOutstandingTickets=1;x.InsideTickets=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::OutstandingTickets,
         "Inside ticket prevents uninstall");
    x=e;x.TotalOutstandingTickets=1;x.ReturningTickets=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::OutstandingTickets,
         "Returning ticket prevents uninstall despite apparent exit");
    x=e;x.CharactersInsideHousing=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::CharactersStillInside,
         "Untracked in-map player prevents uninstall");
    x=e;x.TotalOutstandingTickets=1;x.UnknownStageTickets=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::UnsafeUnknownState,
         "Unknown DB stage cannot be silently ignored");

    x=e;x.TotalOutstandingTickets=2;x.PreparedTickets=2;x.InsideTickets=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::InvalidTicketCounts,
         "Contradictory stage counts denied");
    x=e;x.TotalOutstandingTickets=1;x.UnknownStageTickets=2;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::InvalidTicketCounts,
         "Unknown count cannot exceed total");
    x=e;x.TotalOutstandingTickets=1;x.PreparedTickets=1;x.ReturningTickets=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::InvalidTicketCounts,
         "Cannot double count one ticket");
    x=e;x.TotalOutstandingTickets=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::InvalidTicketCounts,
         "Unclassified ticket never falls through");
    x=e;x.TotalOutstandingTickets=std::numeric_limits<std::uint64_t>::max();
    x.PreparedTickets=std::numeric_limits<std::uint64_t>::max();
    x.InsideTickets=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::InvalidTicketCounts,
         "Huge untrusted sums cannot overflow into zero");
    x=e;x.TotalOutstandingTickets=std::numeric_limits<std::uint64_t>::max();
    x.ReturningTickets=std::numeric_limits<std::uint64_t>::max();
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::OutstandingTickets,
         "Large consistent nonzero backlog still blocks");
    x=e;x.TotalOutstandingTickets=2;x.InsideTickets=1;x.ReturningTickets=1;
    x.CharactersInsideHousing=1;
    test(ReviewUninstallDrain(x).Decision==UninstallDrainDecision::OutstandingTickets,
         "Tickets must be cleared even if some players still inside");
    if(failures)return 1;
    std::cout<<"PASS: "<<checks<<" preservation-first uninstall drain safety checks\n";
    return 0;
}
