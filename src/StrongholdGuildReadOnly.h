#ifndef NAXX_GUILD_STRONGHOLD_GUILD_READ_ONLY_H
#define NAXX_GUILD_STRONGHOLD_GUILD_READ_ONLY_H

#include "StrongholdLifecycle.h"

#include <cstdint>

// Domain-only validation for a single authoritative AzerothCore Guild snapshot.
// No database, guild state mutation, commands or Playerbot operations.
namespace NaxxGuildStrongholds
{
enum class GuildReadDecision : std::uint8_t
{
    Verified,
    NoGuild,
    RegistryUnavailable,
    GuildIdMismatch,
    MembershipUnverified,
    InvalidCreationDate
};

struct GuildReadObservation
{
    std::uint32_t PlayerGuildId = 0;
    bool RegistryGuildFound = false;
    std::uint32_t RegistryGuildId = 0;
    std::uint64_t RegistryCreatedAt = 0;
    bool MembershipFound = false;
};

struct GuildReadResult
{
    GuildReadDecision Decision = GuildReadDecision::NoGuild;
    GuildIdentity Identity;

    bool IsVerified() const { return Decision == GuildReadDecision::Verified; }
};

// A numeric character guild ID is NEVER sufficient to establish ownership.
// The adapter must verify registry ID, exact member GUID and creation date.
// Result is a current membership snapshot, NOT a property/ownership proof.
GuildReadResult VerifyGuildIdentity(GuildReadObservation const& observed);
}
#endif
