#ifndef NAXX_GUILD_STRONGHOLD_ISOLATION_PROBE_H
#define NAXX_GUILD_STRONGHOLD_ISOLATION_PROBE_H

#include <cstdint>
#include <vector>

// Models observed AzerothCore WorldObject phase comparison behaviour.
// Pure *diagnostic* logic; does NOT phase objects or prove real server privacy.
// Never use passing synthetic samples to set DevelopmentCapabilities.PrivacyIsolation.
namespace NaxxGuildStrongholds
{
enum class VisibilityMode : std::uint8_t
{
    CombinedBits,
    ExactValue
};

enum class ObservationKind : std::uint8_t
{
    Player,
    Creature,
    GameObject
};

struct IsolationSample
{
    std::uint32_t GuildId;
    std::uint32_t MapId;
    std::uint32_t InstanceId;
    std::uint32_t PhaseMask;
    VisibilityMode Mode;
    ObservationKind Kind;
};

enum class ProbeVerdict : std::uint8_t
{
    NoSamples,
    MalformedSample,
    OnlyOneGuild,
    MissingRequiredKinds,
    OwnGuildHidden,
    OtherGuildVisible,
    PairwiseSeparatedInSamples
};

// Models directional AzerothCore WorldObject::InSamePhase only.
// Different map or instance is intentionally treated as not visible.
bool CanSampleSee(IsolationSample const& observer,
    IsolationSample const& target);

// A bitmask phase needs exactly one distinct bit for each isolated guild.
// All-phases (0xffffffff), 0, combined masks and reused phase bits fail.
bool IsSingleBitPhase(std::uint32_t mask);

ProbeVerdict EvaluateIsolationSamples(
    std::vector<IsolationSample> const& samples);
}

#endif
