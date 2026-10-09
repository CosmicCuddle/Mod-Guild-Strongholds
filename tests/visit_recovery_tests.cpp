#include "StrongholdVisitRecovery.h"

#include <iostream>
#include <limits>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0;
    int failed = 0;
    auto test = [&](bool good, const char* why)
    {
        ++checks;
        if (!good)
        {
            ++failed;
            std::cerr << "FAILED: " << why << '\n';
        }
    };
    VisitContext c;
    c.ModuleEnabled = true;
    c.IsolationVerified = true; // TEST fixture only: no real isolation exists
    c.MembershipVerified = true;
    c.VisitorGuild = {70, 1790000100};
    c.Property = {{70, 1790000100}, PropertyLifecycle::Active, 3};

    SafeReturnPoint origin{0, 0, -8950.1f, 515.5f, 98.0f, 1.2f};
    auto prepared = PrepareVisit(c, 1234, "serverNonce_A", "human", origin, true, false);
    test(prepared.Decision == VisitRecoveryDecision::Allowed, "Valid simulated preparation");
    test(prepared.Proposed.Stage == VisitStage::Prepared, "Starts as Prepared");
    test(prepared.Proposed.OriginalGuild.CreatedAt == 1790000100, "Guild generation saved");
    test(prepared.Proposed.ReturnPoint.MapId == 0, "Origin map saved");
    test(prepared.Proposed.Version == 0, "Fresh visit starts at version zero");

    test(PrepareVisit(c, 1234, "n2", "human", origin, true, true).Decision ==
         VisitRecoveryDecision::OutstandingVisit, "Cannot overwrite a visit's return position");
    test(PrepareVisit(c, 1234, "n2", "human", origin, false, false).Decision ==
         VisitRecoveryDecision::InvalidReturnPoint, "Origin not verified by server");
    test(PrepareVisit(c, 0, "n2", "human", origin, true, false).Decision ==
         VisitRecoveryDecision::IdentityUnverified, "Cannot create ticket for unknown character");
    test(PrepareVisit(c, 1234, "bad token", "human", origin, true, false).Decision ==
         VisitRecoveryDecision::InvalidVisitRecord, "Invalid visit key blocked");

    c.IsolationVerified = false;
    test(PrepareVisit(c, 1234, "n3", "human", origin, true, false).Decision ==
         VisitRecoveryDecision::EntryDenied, "Privacy-unverified entry blocked");
    c.IsolationVerified = true;
    c.Property.State = PropertyLifecycle::Archived;
    test(PrepareVisit(c, 1234, "n4", "human", origin, true, false).Decision ==
         VisitRecoveryDecision::EntryDenied, "Archived settlement entry blocked");
    c.Property.State = PropertyLifecycle::Active;

    auto badOrigin = origin;
    badOrigin.InstanceId = 22;
    test(!IsStructurallyValidReturn(badOrigin), "Return from instance not approved");
    test(PrepareVisit(c, 1234, "n5", "human", badOrigin, true, false).Decision ==
         VisitRecoveryDecision::InvalidReturnPoint, "Instance origin cannot be saved");
    badOrigin = origin;
    badOrigin.X = std::numeric_limits<float>::infinity();
    test(!IsStructurallyValidReturn(badOrigin), "Infinite return coordinate denied");

    auto ticket = prepared.Proposed;
    test(ConfirmArrival(ticket, 1234, true, false).Decision ==
         VisitRecoveryDecision::TeleportNotConfirmed, "TeleportTo() call alone does not confirm");
    test(ConfirmArrival(ticket, 9999, true, true).Decision ==
         VisitRecoveryDecision::IncorrectCharacter, "Another character cannot claim ticket");
    test(ConfirmArrival(ticket, 1234, false, true).Decision ==
         VisitRecoveryDecision::UntrustedRecord, "Cannot trust unsaved ticket");

    auto arrive = ConfirmArrival(ticket, 1234, true, true);
    test(arrive.Decision == VisitRecoveryDecision::Allowed, "Verified arrival may mark Inside");
    test(arrive.ExpectedVersion == 0 && arrive.Proposed.Version == 1,
         "Arrival enforces version transition");
    test(arrive.Proposed.Stage == VisitStage::Inside, "Inside only after verified destination");
    test(ticket.Stage == VisitStage::Prepared, "Pure policy never mutates original");

    // Crucial: emergency exit ignores archived guild, former membership and
    // module enable state. Trusted return belongs to player, not current guild.
    c.ModuleEnabled = false;
    c.MembershipVerified = false;
    c.Property.State = PropertyLifecycle::Archived;
    test(PrepareReturn(arrive.Proposed, 1234, true, true).Decision ==
         VisitRecoveryDecision::Allowed, "Disband/disabled must never trap a visitor");
    test(PrepareReturn(arrive.Proposed, 1234, false, true).Decision ==
         VisitRecoveryDecision::UntrustedRecord, "Untrusted origin cannot be used");
    test(PrepareReturn(arrive.Proposed, 1234, true, false).Decision ==
         VisitRecoveryDecision::UnverifiedReturn, "Invalid current map cannot be used");
    test(PrepareReturn(arrive.Proposed, 9999, true, true).Decision ==
         VisitRecoveryDecision::IncorrectCharacter, "Foreign player cannot exit via another's origin");

    auto returning = PrepareReturn(arrive.Proposed, 1234, true, true);
    test(returning.Proposed.Stage == VisitStage::Returning, "Exit is persisted before teleport");
    test(returning.Proposed.Version == 2, "Exit transitions with optimistic version");
    test(PrepareReturn(returning.Proposed, 1234, true, true).Decision ==
         VisitRecoveryDecision::AlreadyReturning, "Repeated exit doesn't overwrite location");
    test(CanClearVisit(returning.Proposed, 1234, true, false) ==
         VisitRecoveryDecision::TeleportNotConfirmed, "Do not clear on failed return");
    test(CanClearVisit(arrive.Proposed, 1234, true, true) ==
         VisitRecoveryDecision::RecoveryRequired, "Cannot clear before return state");
    test(CanClearVisit(returning.Proposed, 1234, false, true) ==
         VisitRecoveryDecision::UntrustedRecord, "Untrusted ticket may not be cleared");
    test(CanClearVisit(returning.Proposed, 9999, true, true) ==
         VisitRecoveryDecision::IncorrectCharacter, "Wrong character may not clear ticket");
    test(CanClearVisit(returning.Proposed, 1234, true, true) ==
         VisitRecoveryDecision::Allowed, "Only verified completed return may clear");

    auto overflow = ticket;
    overflow.Version = std::numeric_limits<std::uint64_t>::max();
    test(ConfirmArrival(overflow, 1234, true, true).Decision ==
         VisitRecoveryDecision::VersionOverflow, "Arrival rejects version overflow");
    overflow.Stage = VisitStage::Inside;
    test(PrepareReturn(overflow, 1234, true, true).Decision ==
         VisitRecoveryDecision::VersionOverflow, "Exit rejects version overflow");


    // Worldserver crashed after Returning was committed but before arrival:
    // no new entry ticket, only a source-reviewed RECONCILIATION CANDIDATE.
    RecoveryLocationSnapshot atHome{0,0,-8950.1f,515.5f,98.0f,
                                    true,true,false,false};
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, atHome) ==
         RecoveryResumeDecision::CandidateReviewCompletedReturn,
         "Returning at saved origin can be reviewed after restart");
    test(ReviewInterruptedVisit(ticket, 1234, true, true, atHome) ==
         RecoveryResumeDecision::CandidateReconcileNeverLeft,
         "Prepared still at origin may never have entered");
    test(ReviewInterruptedVisit(arrive.Proposed, 1234, true, true, atHome) ==
         RecoveryResumeDecision::CandidatePersistReturning,
         "Inside but already at origin still needs state reconciliation");

    auto inside = atHome;
    inside.MapId = 1;
    inside.InsideOriginalPropertyVerified = true;
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, inside) ==
         RecoveryResumeDecision::CandidateRetryReturning,
         "Returning still inside after crash may retry old origin");
    test(ReviewInterruptedVisit(ticket, 1234, true, true, inside) ==
         RecoveryResumeDecision::CandidatePersistReturning,
         "Prepared yet inside requires durable Returning transition");
    test(ReviewInterruptedVisit(arrive.Proposed, 1234, true, true, inside) ==
         RecoveryResumeDecision::CandidatePersistReturning,
         "Inside after relog requires durable Returning transition");

    test(ReviewInterruptedVisit(returning.Proposed, 1234, false, true, inside) ==
         RecoveryResumeDecision::UntrustedTicket, "Caller-supplied row never trusted");
    test(ReviewInterruptedVisit(returning.Proposed, 9999, true, true, inside) ==
         RecoveryResumeDecision::IncorrectCharacter, "Other player cannot resume visit");
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, false, inside) ==
         RecoveryResumeDecision::UnsafeSavedReturn, "Saved origin must be revalidated");

    auto unstable = inside;
    unstable.PositionFromServer = false;
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, unstable) ==
         RecoveryResumeDecision::UnverifiedPosition, "No self-attested location");
    unstable = inside;
    unstable.PositionStable = false;
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, unstable) ==
         RecoveryResumeDecision::MovementPending, "Wait for position to stabilize");
    unstable = inside;
    unstable.TeleportPending = true;
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, unstable) ==
         RecoveryResumeDecision::MovementPending, "Do not duplicate pending teleport");
    unstable = inside;
    unstable.X = std::numeric_limits<float>::quiet_NaN();
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, unstable) ==
         RecoveryResumeDecision::UnverifiedPosition, "Corrupt coordinate never qualifies");

    unstable = atHome;
    unstable.InsideOriginalPropertyVerified = true;
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, unstable) ==
         RecoveryResumeDecision::ConflictingEvidence, "At origin and inside is inconsistent");
    unstable = atHome;
    unstable.MapId = 571;
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, unstable) ==
         RecoveryResumeDecision::ManualRecoveryRequired, "Unexpected third map requires manual recovery");
    unstable = atHome;
    unstable.X += 10.0f;
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, unstable) ==
         RecoveryResumeDecision::ManualRecoveryRequired, "Not near original location is not confirmed return");
    unstable = atHome;
    unstable.InstanceId = 44;
    test(ReviewInterruptedVisit(returning.Proposed, 1234, true, true, unstable) ==
         RecoveryResumeDecision::ManualRecoveryRequired, "Wrong instance cannot clear visit");

    auto invalidStage = returning.Proposed;
    invalidStage.Stage = static_cast<VisitStage>(250);
    test(ReviewInterruptedVisit(invalidStage, 1234, true, true, atHome) ==
         RecoveryResumeDecision::InvalidTicket, "Unknown visit stage rejected");
    test(CanClearVisit(invalidStage, 1234, true, true) ==
         VisitRecoveryDecision::InvalidVisitRecord, "Unknown stage cannot clear SQL");
    test(PrepareReturn(invalidStage, 1234, true, true).Decision ==
         VisitRecoveryDecision::InvalidVisitRecord, "Unknown stage cannot create transition");
    // The original guild may have disbanded: the old ticket still belongs
    // exclusively to this CHARACTER. Active guild membership is irrelevant.
    auto disbanded = returning.Proposed;
    disbanded.OriginalGuild.GuildId = 555;
    test(ReviewInterruptedVisit(disbanded, 1234, true, true, inside) ==
         RecoveryResumeDecision::CandidateRetryReturning,
         "Archived/changed guild never strands the old character");

    if (failed)
    {
        std::cerr << failed << " of " << checks << " visit-recovery tests failed\n";
        return 1;
    }
    std::cout << "PASS: " << checks << " visit preparation and safe-return policy checks\n";
    return 0;
}
