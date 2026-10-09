#include "StrongholdPropertyRead.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    int checks = 0, failed = 0;
    auto expect = [&](bool ok, char const* label)
    {
        ++checks;
        if (!ok)
        {
            ++failed;
            std::cerr << "FAILED: " << label << '\n';
        }
    };

    GuildIdentity actor{52, 1790000500};
    PropertySqlRow row{52, 1790000500, "active", 2, 4};
    auto read = InspectPropertyRow(actor, false, true, row);
    expect(read.Decision == PropertyReadDecision::Disabled &&
        !read.ValidRow && !read.HousingAvailable, "Runtime opt-in defaults closed");
    read = InspectPropertyRow(actor, true, false, row);
    expect(read.Decision == PropertyReadDecision::NotFoundOrQueryUnavailable &&
        !read.ValidRow, "Missing table/row cannot imply ownership");
    read = InspectPropertyRow({52, 0}, true, true, row);
    expect(read.Decision == PropertyReadDecision::IdentityUnverified,
        "Actor must have verified creation time");

    read = InspectPropertyRow(actor, true, true, row);
    expect(read.Decision == PropertyReadDecision::Validated &&
        read.ValidRow && read.Level == 4 &&
        read.Property.OriginalGuild.CreatedAt == actor.CreatedAt &&
        read.Property.State == PropertyLifecycle::Active,
        "Valid active property can only be displayed");
    expect(!read.HousingAvailable, "Successful read never enables housing");

    row.GuildCreatedAt += 10;
    read = InspectPropertyRow(actor, true, true, row);
    expect(read.Decision == PropertyReadDecision::WrongGeneration &&
        read.ValidRow && !read.HousingAvailable,
        "Recycled guild ID remains visible as wrong generation, not owner");
    row.GuildCreatedAt = actor.CreatedAt;
    row.GuildId = 71;
    read = InspectPropertyRow(actor, true, true, row);
    expect(read.Decision == PropertyReadDecision::WrongGuild &&
        read.ValidRow, "Unexpected foreign row never inherits property");
    row.GuildId = actor.GuildId;

    row.Lifecycle = "archived";
    read = InspectPropertyRow(actor, true, true, row);
    expect(read.Decision == PropertyReadDecision::Validated &&
        read.Property.State == PropertyLifecycle::Archived && !read.HousingAvailable,
        "Archived properties remain archived in presentation");
    for (char const* invalid : {"", "pending", "ACTIVE", "deleted", "active "})
    {
        row.Lifecycle = invalid;
        expect(InspectPropertyRow(actor, true, true, row).Decision ==
            PropertyReadDecision::MalformedRow, "Unexpected lifecycle fails closed");
    }
    row.Lifecycle = "active";
    for (std::uint32_t level : {0u, 8u, 255u})
    {
        row.DevelopmentLevel = level;
        expect(InspectPropertyRow(actor, true, true, row).Decision ==
            PropertyReadDecision::MalformedRow, "Invalid settlement level denied");
    }
    row.DevelopmentLevel = 7;
    row.GuildCreatedAt = 0;
    expect(InspectPropertyRow(actor, true, true, row).Decision ==
        PropertyReadDecision::MalformedRow, "Absent persisted generation refused");
    row.GuildCreatedAt = actor.CreatedAt;
    row.GuildId = 0;
    expect(InspectPropertyRow(actor, true, true, row).Decision ==
        PropertyReadDecision::MalformedRow, "Guild zero refused");

    if (failed)
        return 1;
    std::cout << "PASS: " << checks << " staging SQL property read policy checks\n";
    return 0;
}
