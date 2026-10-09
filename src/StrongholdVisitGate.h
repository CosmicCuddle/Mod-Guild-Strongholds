#ifndef NAXX_GUILD_STRONGHOLD_VISIT_GATE_H
#define NAXX_GUILD_STRONGHOLD_VISIT_GATE_H

#include "StrongholdLifecycle.h"

#include <cstdint>

// Single future entry gate combining module state, private-area proof,
// authoritative guild membership, and original ownership generation.
// No AzerothCore API, teleport, phasing, or SQL access in this policy layer.
namespace NaxxGuildStrongholds
{
enum class VisitDecision : std::uint8_t
{
    Allowed,
    ModuleDisabled,
    IsolationUnverified,
    MembershipUnverified,
    NoGuild,
    OtherGuild,
    InvalidIdentity,
    WrongGeneration,
    Archived
};

struct VisitContext
{
    bool ModuleEnabled = false;
    bool IsolationVerified = false;
    bool MembershipVerified = false;
    GuildIdentity VisitorGuild;
    PropertyLifetime Property;
};

VisitDecision CheckGuildVisit(VisitContext const& context);
}

#endif
