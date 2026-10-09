#include "StrongholdVisitGate.h"
#include "StrongholdCatalog.h"

namespace NaxxGuildStrongholds
{
VisitDecision CheckGuildVisit(VisitContext const& context)
{
    const AccessDecision base = CheckGuildEntry(context.ModuleEnabled,
        context.IsolationVerified, context.Property.OriginalGuild.GuildId,
        context.VisitorGuild.GuildId);

    switch (base)
    {
        case AccessDecision::Disabled: return VisitDecision::ModuleDisabled;
        case AccessDecision::IsolationUnverified: return VisitDecision::IsolationUnverified;
        case AccessDecision::NoGuild: return VisitDecision::NoGuild;
        case AccessDecision::OtherGuild: return VisitDecision::OtherGuild;
        case AccessDecision::Granted: break;
    }

    // A guild-ID match is insufficient: the visitor must be a currently
    // verified member of the ORIGINAL owner generation.
    if (!context.MembershipVerified)
        return VisitDecision::MembershipUnverified;

    switch (CheckPropertyLifetime(context.Property, context.VisitorGuild, true))
    {
        case LifecycleDecision::Allowed: return VisitDecision::Allowed;
        case LifecycleDecision::InvalidIdentity: return VisitDecision::InvalidIdentity;
        case LifecycleDecision::DifferentGuild: return VisitDecision::OtherGuild;
        case LifecycleDecision::WrongGeneration: return VisitDecision::WrongGeneration;
        case LifecycleDecision::Archived: return VisitDecision::Archived;
        default: return VisitDecision::InvalidIdentity; // fail closed
    }
}
}
