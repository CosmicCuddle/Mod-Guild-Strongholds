#include "StrongholdVisitTicketRead.h"
#include <cmath>
namespace NaxxGuildStrongholds
{
namespace
{
bool ValidKey(std::string const& text, std::size_t maximum)
{
    if (text.empty() || text.size() > maximum)
        return false;
    for (char c : text)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_'))
            return false;
    return true;
}
}
VisitTicketReadReview ReviewVisitTicketRow(
    std::uint32_t selfGuid, bool optIn, bool rowReturned, VisitRecord const& row)
{
    VisitTicketReadReview output;
    if (!optIn)
        return output;
    if (!selfGuid)
    {
        output.Status = VisitTicketReadStatus::UnknownCharacter;
        return output;
    }
    if (!rowReturned)
    {
        // Null QueryResult also means missing table or failed database.
        output.Status = VisitTicketReadStatus::NoRowOrDatabaseUnavailable;
        return output;
    }
    if (!row.CharacterGuid || !row.OriginalGuild.GuildId ||
        !row.OriginalGuild.CreatedAt ||
        !ValidKey(row.SessionKey, 64) ||
        !ValidKey(row.PropertyKey, 32) ||
        row.ReturnPoint.InstanceId != 0 ||
        !std::isfinite(row.ReturnPoint.X) ||
        !std::isfinite(row.ReturnPoint.Y) ||
        !std::isfinite(row.ReturnPoint.Z) ||
        !std::isfinite(row.ReturnPoint.Orientation))
    {
        output.Status = VisitTicketReadStatus::InvalidRow;
        return output;
    }
    if (row.CharacterGuid != selfGuid)
    {
        output.Status = VisitTicketReadStatus::WrongCharacter;
        return output;
    }
    switch (row.Stage)
    {
        case VisitStage::Prepared:
            output.Status = VisitTicketReadStatus::Prepared;
            break;
        case VisitStage::Inside:
            output.Status = VisitTicketReadStatus::Inside;
            break;
        case VisitStage::Returning:
            output.Status = VisitTicketReadStatus::Returning;
            break;
        default:
            output.Status = VisitTicketReadStatus::InvalidRow;
    }
    return output;
}
char const* VisitTicketStatusText(VisitTicketReadStatus status)
{
    switch (status)
    {
        case VisitTicketReadStatus::Disabled:
            return "Visit ticket reader OFF.";
        case VisitTicketReadStatus::UnknownCharacter:
            return "Visit ticket: character identity UNVERIFIED.";
        case VisitTicketReadStatus::NoRowOrDatabaseUnavailable:
            return "Visit ticket: NO ROW OR DATABASE UNAVAILABLE. NOT a safe-return proof.";
        case VisitTicketReadStatus::InvalidRow:
            return "Visit ticket: INVALID; manual staging review.";
        case VisitTicketReadStatus::WrongCharacter:
            return "Visit ticket: wrong character; read refused.";
        case VisitTicketReadStatus::Prepared:
            return "Visit ticket: PREPARED; entry NOT verified.";
        case VisitTicketReadStatus::Inside:
            return "Visit ticket: INSIDE; safe exit NOT verified.";
        case VisitTicketReadStatus::Returning:
            return "Visit ticket: RETURNING; final arrival NOT verified.";
    }
    return "Visit ticket: UNKNOWN; manual staging review.";
}
}
