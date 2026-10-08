#ifndef NAXX_GUILD_STRONGHOLD_PROPERTY_READ_H
#define NAXX_GUILD_STRONGHOLD_PROPERTY_READ_H

#include "StrongholdLifecycle.h"

#include <cstdint>
#include <string>

// Platform-independent validation for a READ-ONLY staging SQL observation.
// Never creates a property, modifies its lifetime or enables housing.
namespace NaxxGuildStrongholds
{
enum class PropertyReadDecision : std::uint8_t
{
    Disabled,
    IdentityUnverified,
    NotFoundOrQueryUnavailable,
    MalformedRow,
    WrongGuild,
    WrongGeneration,
    Validated
};

struct PropertySqlRow
{
    std::uint32_t GuildId = 0;
    std::uint64_t GuildCreatedAt = 0;
    std::string Lifecycle;
    std::uint64_t LifecycleVersion = 0;
    std::uint32_t DevelopmentLevel = 0;
};

struct PropertyReadResult
{
    PropertyReadDecision Decision = PropertyReadDecision::Disabled;
    bool ValidRow = false;
    PropertyLifetime Property;
    std::uint8_t Level = 0;

    bool CanDisplayRow() const { return ValidRow; }
    // A property SQL row is not a privacy proof or permission to enter.
    bool HousingAvailable = false;
};

PropertyReadResult InspectPropertyRow(GuildIdentity const& verifiedGuild,
    bool explicitStagingQueryAllowed, bool rowReturned, PropertySqlRow const& row);
}
#endif
