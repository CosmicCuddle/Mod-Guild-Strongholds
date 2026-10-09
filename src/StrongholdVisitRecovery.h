#ifndef NAXX_GUILD_STRONGHOLD_VISIT_RECOVERY_H
#define NAXX_GUILD_STRONGHOLD_VISIT_RECOVERY_H

#include "StrongholdVisitGate.h"
#include <cstdint>
#include <string>

namespace NaxxGuildStrongholds
{
// Server-originated, validated surface position only. No raid/BG/instance
// return points until handling lockouts, resets and saved instances is proven.
struct SafeReturnPoint
{
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float Orientation = 0.0f;
};

enum class VisitStage : std::uint8_t
{
    Prepared,       // Persisted BEFORE any entry teleport attempt
    Inside,         // Arrival confirmed on the actual private map
    Returning       // Exit requested; persist until return is verified
};

struct VisitRecord
{
    std::uint32_t CharacterGuid = 0;
    GuildIdentity OriginalGuild;
    std::string SessionKey;
    std::string PropertyKey;
    SafeReturnPoint ReturnPoint;
    VisitStage Stage = VisitStage::Prepared;
    std::uint64_t Version = 0;
};

enum class VisitRecoveryDecision : std::uint8_t
{
    Allowed,
    EntryDenied,
    IdentityUnverified,
    InvalidReturnPoint,
    InvalidVisitRecord,
    OutstandingVisit,
    IncorrectCharacter,
    UntrustedRecord,
    TeleportNotConfirmed,
    UnverifiedReturn,
    RecoveryRequired,
    AlreadyReturning,
    VersionOverflow
};

struct VisitRecoveryPlan
{
    VisitRecoveryDecision Decision = VisitRecoveryDecision::EntryDenied;
    VisitRecord Proposed;
    std::uint64_t ExpectedVersion = 0;
};

// Future Worldserver adapter must run MapMgr::IsValidMapCoord and verify
// that origin is a safe non-instance map, in addition to these basic checks.
bool IsStructurallyValidReturn(SafeReturnPoint const& location);

// Builds a DRAFT. The adapter MUST persist it durably under a unique
// character-guid constraint BEFORE requesting teleport. No side effects.
VisitRecoveryPlan PrepareVisit(VisitContext const& access,
    std::uint32_t verifiedCharacterGuid, std::string sessionKey,
    std::string propertyKey, SafeReturnPoint returnPoint,
    bool returnCapturedAndVerifiedByServer, bool hasOutstandingVisit);

// Arrival is NOT confirmed merely because TeleportTo() returned true.
// The runtime adapter must also verify final map / instance / guild privacy.
VisitRecoveryPlan ConfirmArrival(VisitRecord const& saved,
    std::uint32_t verifiedCharacterGuid, bool persistedRecordTrusted,
    bool destinationAndIsolationVerified);

// Emergency exit deliberately does NOT require guild membership, active
// guild ownership or module Enabled=1. This avoids trapping former members
// or players whose guild disbanded. Only the original player can use a
// trusted, server-validated return record.
VisitRecoveryPlan PrepareReturn(VisitRecord const& saved,
    std::uint32_t verifiedCharacterGuid, bool persistedRecordTrusted,
    bool returnMapRevalidatedByServer);


// Pure, read-only review of a persisted visit after crash, relog or restart.
// A candidate result never teleports a player, updates SQL, clears a ticket
// or approves removing the recovery handler from a live realm.
struct RecoveryLocationSnapshot
{
    std::uint32_t MapId = 0;
    std::uint32_t InstanceId = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    bool PositionFromServer = false;
    bool PositionStable = false;
    bool TeleportPending = false;
    bool InsideOriginalPropertyVerified = false;
};

enum class RecoveryResumeDecision : std::uint8_t
{
    UntrustedTicket,
    InvalidTicket,
    IncorrectCharacter,
    UnsafeSavedReturn,
    UnverifiedPosition,
    MovementPending,
    ConflictingEvidence,
    ManualRecoveryRequired,
    CandidateReconcileNeverLeft,
    CandidatePersistReturning,
    CandidateRetryReturning,
    CandidateReviewCompletedReturn
};

RecoveryResumeDecision ReviewInterruptedVisit(
    VisitRecord const& saved, std::uint32_t verifiedCharacterGuid,
    bool persistedRecordTrusted, bool savedReturnRevalidatedByServer,
    RecoveryLocationSnapshot const& current);

// No deletion is allowed unless a trusted return teleport was VERIFIED.
// Even if the game client disconnects, the row must survive for recovery.
VisitRecoveryDecision CanClearVisit(VisitRecord const& saved,
    std::uint32_t verifiedCharacterGuid, bool persistedRecordTrusted,
    bool playerConfirmedAtSafeReturn);
}
#endif
