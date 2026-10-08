#include "StrongholdGuildReadOnly.h"

#include <iostream>

using namespace NaxxGuildStrongholds;

int main()
{
    int tested = 0;
    int failed = 0;
    auto check = [&](bool ok, char const* name)
    {
        ++tested;
        if (!ok)
        {
            ++failed;
            std::cerr << "FAILED: " << name << '\n';
        }
    };

    GuildReadObservation o;
    check(VerifyGuildIdentity(o).Decision == GuildReadDecision::NoGuild,
          "No guild by default");
    o.PlayerGuildId = 41;
    check(VerifyGuildIdentity(o).Decision == GuildReadDecision::RegistryUnavailable,
          "Player field guild alone cannot prove registry exists");
    o.RegistryGuildFound = true;
    o.RegistryGuildId = 42;
    o.MembershipFound = true;
    o.RegistryCreatedAt = 1790000000;
    check(VerifyGuildIdentity(o).Decision == GuildReadDecision::GuildIdMismatch,
          "Stale or wrong guild object denied");
    o.RegistryGuildId = 41;
    o.MembershipFound = false;
    check(VerifyGuildIdentity(o).Decision == GuildReadDecision::MembershipUnverified,
          "Guild exists but actor GUID not a member");
    o.MembershipFound = true;
    o.RegistryCreatedAt = 0;
    check(VerifyGuildIdentity(o).Decision == GuildReadDecision::InvalidCreationDate,
          "Unknown guild generation rejected");
    o.RegistryCreatedAt = 1790000123;
    auto result = VerifyGuildIdentity(o);
    check(result.IsVerified() && result.Identity.GuildId == 41 &&
          result.Identity.CreatedAt == 1790000123,
          "Original creation date and registry member ID validated");
    o.RegistryGuildId = 50;
    result = VerifyGuildIdentity(o);
    check(!result.IsVerified() && result.Identity.GuildId == 0 &&
          result.Identity.CreatedAt == 0,
          "A failed read never leaks previous successful identity");
    o.RegistryGuildId = 41;
    o.PlayerGuildId = 0;
    check(VerifyGuildIdentity(o).Decision == GuildReadDecision::NoGuild,
          "Current guild leave takes priority over previous registry");
    if (failed)
        return 1;
    std::cout << "PASS: " << tested << " read-only guild generation checks\n";
    return 0;
}
