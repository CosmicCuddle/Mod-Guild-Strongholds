#ifndef NAXX_GS_VISIT_TICKET_READ_H
#define NAXX_GS_VISIT_TICKET_READ_H
#include "StrongholdVisitRecovery.h"
#include <cstdint>

namespace NaxxGuildStrongholds
{
// Pure GM diagnostic display policy. No state change or return confirmation.
enum class VisitTicketReadStatus : std::uint8_t
{
    Disabled, UnknownCharacter, NoRowOrDatabaseUnavailable,
    InvalidRow, WrongCharacter, Prepared, Inside, Returning
};
struct VisitTicketReadReview
{
    VisitTicketReadStatus Status = VisitTicketReadStatus::Disabled;
    bool AuthorisesTeleport = false;
    bool AuthorisesClear = false;
    bool ProvesSafeReturn = false;
};
VisitTicketReadReview ReviewVisitTicketRow(
    std::uint32_t selfGuid, bool explicitStaffOptIn,
    bool rowReturned, VisitRecord const& row);
char const* VisitTicketStatusText(VisitTicketReadStatus status);
}
#endif
