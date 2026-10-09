#ifndef NAXX_GUILD_STRONGHOLD_STEWARD_EVIDENCE_H
#define NAXX_GUILD_STRONGHOLD_STEWARD_EVIDENCE_H

#include "StrongholdActivities.h"
#include "StrongholdIpCompatibility.h"
#include "StrongholdLifecycle.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// ONLY hypothetical read-only stage preview; no production guild/IP/DB
// adapter exists. This policy may report a *planning* gate as Allowed,
// but a hard-coded runtime flag prevents treating that as gameplay access.
namespace NaxxGuildStrongholds
{
enum class StewardEvidenceStatus : std::uint8_t
{
    NoGuild,
    IdentityUnverified,
    PropertyNotLoaded,
    WrongGuild,
    WrongGeneration,
    Archived,
    InvalidProperty,
    LevelUnverified,
    IpUnverified,
    PreviewOnly
};

struct StewardEvidenceInput
{
    GuildIdentity ActorGuild;
    bool GuildGenerationVerified = false;
    bool PropertySnapshotLoaded = false;
    PropertyLifetime Property;
    bool SettlementLevelVerified = false;
    std::uint8_t SettlementLevel = 0;
    IpRuntimeSnapshot Ip;
    bool IsBot = false;
};

struct StewardEvidenceReport
{
    StewardEvidenceStatus Status = StewardEvidenceStatus::NoGuild;
    IpReadResult IpStatus = IpReadResult::UnverifiedSource;
    std::uint8_t EffectiveIpRank = 0;
    std::array<ActivityGate, 9> PlannedActivityGates{};
    // NEVER true in development, even with all synthetic proofs accepted.
    bool HousingAvailable = false;
};

StewardEvidenceReport EvaluateStewardEvidence(StewardEvidenceInput const& input);

// A bounded, honest menu page. Do not display synthetic proof as live status.
std::vector<std::string> BuildStewardEvidenceRows(
    StewardEvidenceInput const& input);
}

#endif
