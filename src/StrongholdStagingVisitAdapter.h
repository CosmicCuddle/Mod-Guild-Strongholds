#ifndef NAXX_GS_STAGING_VISIT_ADAPTER_H
#define NAXX_GS_STAGING_VISIT_ADAPTER_H
#include "StrongholdVisitTicketRead.h"
#if defined(NAXX_GS_BUILD_STAGING_VISIT_READ) && \
    !defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
#error "Visit read requires the staging diagnostics build flag"
#endif
#if defined(NAXX_GS_BUILD_STAGING_VISIT_READ) && \
    defined(NAXX_GS_BUILD_STAGING_DIAGNOSTICS)
class Player;
namespace NaxxGuildStrongholds
{
VisitTicketReadReview ReadStagingVisitTicket(Player const* self);
}
#endif
#endif
