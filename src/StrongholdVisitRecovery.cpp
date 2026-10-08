#include "StrongholdVisitRecovery.h"

#include <cmath>
#include <limits>
#include <utility>

namespace NaxxGuildStrongholds
{
namespace
{
bool KeyValid(std::string const& key, std::size_t maximum)
{
    if (key.empty() || key.size() > maximum)
        return false;
    for (char c : key)
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') || c == '_' || c == '-'))
            return false;
    return true;
}

bool ValidVisitRecord(VisitRecord const& visit)
{
    return visit.CharacterGuid != 0 &&
        visit.OriginalGuild.GuildId != 0 &&
        visit.OriginalGuild.CreatedAt != 0 &&
        KeyValid(visit.SessionKey, 64) &&
        KeyValid(visit.PropertyKey, 32) &&
        IsStructurallyValidReturn(visit.ReturnPoint);
}

VisitRecoveryPlan CopyPlan(VisitRecord const& record)
{
    VisitRecoveryPlan plan;
    plan.Proposed = record;
    plan.ExpectedVersion = record.Version;
    return plan;
}

VisitRecoveryPlan Refuse(VisitRecord const& record, VisitRecoveryDecision reason)
{
    VisitRecoveryPlan plan = CopyPlan(record);
    plan.Decision = reason;
    return plan;
}
}

bool IsStructurallyValidReturn(SafeReturnPoint const& location)
{
    return location.InstanceId == 0 &&
        std::isfinite(location.X) && std::isfinite(location.Y) &&
        std::isfinite(location.Z) && std::isfinite(location.Orientation);
}

VisitRecoveryPlan PrepareVisit(VisitContext const& access,
    std::uint32_t verifiedCharacterGuid, std::string sessionKey,
    std::string propertyKey, SafeReturnPoint returnPoint,
    bool returnCapturedAndVerifiedByServer, bool hasOutstandingVisit)
{
    VisitRecoveryPlan plan;
    if (CheckGuildVisit(access) != VisitDecision::Allowed)
        plan.Decision = VisitRecoveryDecision::EntryDenied;
    else if (!verifiedCharacterGuid)
        plan.Decision = VisitRecoveryDecision::IdentityUnverified;
    else if (hasOutstandingVisit)
        plan.Decision = VisitRecoveryDecision::OutstandingVisit;
    else if (!returnCapturedAndVerifiedByServer ||
             !IsStructurallyValidReturn(returnPoint))
        plan.Decision = VisitRecoveryDecision::InvalidReturnPoint;
    else if (!KeyValid(sessionKey, 64) || !KeyValid(propertyKey, 32))
        plan.Decision = VisitRecoveryDecision::InvalidVisitRecord;
    else
    {
        plan.Decision = VisitRecoveryDecision::Allowed;
        plan.Proposed.CharacterGuid = verifiedCharacterGuid;
        plan.Proposed.OriginalGuild = access.Property.OriginalGuild;
        plan.Proposed.SessionKey = std::move(sessionKey);
        plan.Proposed.PropertyKey = std::move(propertyKey);
        plan.Proposed.ReturnPoint = returnPoint;
        plan.Proposed.Stage = VisitStage::Prepared;
        plan.Proposed.Version = 0;
    }
    return plan;
}

VisitRecoveryPlan ConfirmArrival(VisitRecord const& saved,
    std::uint32_t verifiedCharacterGuid, bool persistedRecordTrusted,
    bool destinationAndIsolationVerified)
{
    if (!persistedRecordTrusted)
        return Refuse(saved, VisitRecoveryDecision::UntrustedRecord);
    if (!ValidVisitRecord(saved))
        return Refuse(saved, VisitRecoveryDecision::InvalidVisitRecord);
    if (!verifiedCharacterGuid || saved.CharacterGuid != verifiedCharacterGuid)
        return Refuse(saved, VisitRecoveryDecision::IncorrectCharacter);
    if (saved.Stage != VisitStage::Prepared)
        return Refuse(saved, VisitRecoveryDecision::RecoveryRequired);
    if (!destinationAndIsolationVerified)
        return Refuse(saved, VisitRecoveryDecision::TeleportNotConfirmed);
    if (saved.Version == std::numeric_limits<std::uint64_t>::max())
        return Refuse(saved, VisitRecoveryDecision::VersionOverflow);

    VisitRecoveryPlan plan = CopyPlan(saved);
    plan.Decision = VisitRecoveryDecision::Allowed;
    plan.Proposed.Stage = VisitStage::Inside;
    ++plan.Proposed.Version;
    return plan;
}

VisitRecoveryPlan PrepareReturn(VisitRecord const& saved,
    std::uint32_t verifiedCharacterGuid, bool persistedRecordTrusted,
    bool returnMapRevalidatedByServer)
{
    if (!persistedRecordTrusted)
        return Refuse(saved, VisitRecoveryDecision::UntrustedRecord);
    if (!ValidVisitRecord(saved))
        return Refuse(saved, VisitRecoveryDecision::InvalidVisitRecord);
    if (!verifiedCharacterGuid || saved.CharacterGuid != verifiedCharacterGuid)
        return Refuse(saved, VisitRecoveryDecision::IncorrectCharacter);
    if (!returnMapRevalidatedByServer)
        return Refuse(saved, VisitRecoveryDecision::UnverifiedReturn);
    if (saved.Stage == VisitStage::Returning)
        return Refuse(saved, VisitRecoveryDecision::AlreadyReturning);
    if (saved.Version == std::numeric_limits<std::uint64_t>::max())
        return Refuse(saved, VisitRecoveryDecision::VersionOverflow);

    VisitRecoveryPlan plan = CopyPlan(saved);
    plan.Decision = VisitRecoveryDecision::Allowed;
    plan.Proposed.Stage = VisitStage::Returning;
    ++plan.Proposed.Version;
    return plan;
}

VisitRecoveryDecision CanClearVisit(VisitRecord const& saved,
    std::uint32_t verifiedCharacterGuid, bool persistedRecordTrusted,
    bool playerConfirmedAtSafeReturn)
{
    if (!persistedRecordTrusted)
        return VisitRecoveryDecision::UntrustedRecord;
    if (!ValidVisitRecord(saved))
        return VisitRecoveryDecision::InvalidVisitRecord;
    if (!verifiedCharacterGuid || saved.CharacterGuid != verifiedCharacterGuid)
        return VisitRecoveryDecision::IncorrectCharacter;
    if (saved.Stage != VisitStage::Returning)
        return VisitRecoveryDecision::RecoveryRequired;
    if (!playerConfirmedAtSafeReturn)
        return VisitRecoveryDecision::TeleportNotConfirmed;
    return VisitRecoveryDecision::Allowed;
}
}
