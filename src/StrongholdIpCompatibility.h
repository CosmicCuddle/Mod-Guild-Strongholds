#ifndef NAXX_GUILD_STRONGHOLD_IP_COMPATIBILITY_H
#define NAXX_GUILD_STRONGHOLD_IP_COMPATIBILITY_H

#include "StrongholdActivities.h"

#include <cstdint>

// Source-version-aware, read-only IP eligibility policy.
//
// The future AzerothCore glue must read IP's own authoritative public fields
// and GetPlayerProgressionFromQuests(Player*) without calling an IP mutator.
// The source contract must be matched to the deployed fork first.
// This code makes NO external calls and is not registered with worldserver.
namespace NaxxGuildStrongholds
{
enum class IpReadResult : std::uint8_t
{
    Verified,
    UnverifiedSource,
    ModuleDisabled,
    CharacterNotInWorld,
    InvalidRawStage,
    InvalidProgressionLimit
};

struct IpRuntimeSnapshot
{
    bool SourceContractVerified = false;
    bool ModuleEnabled = false;
    bool CharacterInWorld = false;
    std::uint32_t RawQuestStage = 0;
    int ProgressionLimit = 0;  // 0 means no cap, as in Grimfeather IP.
};

struct EffectiveIpStage
{
    IpReadResult Result = IpReadResult::UnverifiedSource;
    std::uint8_t Rank = 0;
    bool IsVerified() const { return Result == IpReadResult::Verified; }
};

// Stages 0..18 match the reviewed Grimfeather fork at its pinned commit.
// This function is NOT evidence that the runtime module is that fork.
EffectiveIpStage EvaluateIpStage(IpRuntimeSnapshot const& runtime);

// Intentionally replaces any previously populated/stale IP proof fields.
// A false SourceContractVerified MUST never reuse an old Granted status.
ActivityContext ApplyIpStage(ActivityContext const& base,
    IpRuntimeSnapshot const& runtime);
}

#endif
