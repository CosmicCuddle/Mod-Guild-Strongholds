#include "StrongholdGuildReadOnly.h"

namespace NaxxGuildStrongholds
{
GuildReadResult VerifyGuildIdentity(GuildReadObservation const& observed)
{
    GuildReadResult out;
    if (!observed.PlayerGuildId)
        out.Decision = GuildReadDecision::NoGuild;
    else if (!observed.RegistryGuildFound)
        out.Decision = GuildReadDecision::RegistryUnavailable;
    else if (observed.PlayerGuildId != observed.RegistryGuildId)
        out.Decision = GuildReadDecision::GuildIdMismatch;
    else if (!observed.MembershipFound)
        out.Decision = GuildReadDecision::MembershipUnverified;
    else if (!observed.RegistryCreatedAt)
        out.Decision = GuildReadDecision::InvalidCreationDate;
    else
    {
        out.Decision = GuildReadDecision::Verified;
        out.Identity = {observed.RegistryGuildId, observed.RegistryCreatedAt};
    }
    // Failed checks never expose stale guild identity/generation as verified.
    return out;
}
}
