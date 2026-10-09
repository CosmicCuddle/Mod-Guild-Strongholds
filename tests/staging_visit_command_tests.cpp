#include "Chat.h"
#include "Config.h"
#include "Player.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "StrongholdStagingVisitAdapter.h"

#include <iostream>
#include <string>

FakeDiagnosticsConfig configStorage;
FakeDiagnosticsConfig* sConfigMgr = &configStorage;
CommandScript* gRegisteredDiagnostics = nullptr;

namespace NaxxGuildStrongholds
{
static int gTicketReadCalls = 0;
static Player const* gLastTicketActor = nullptr;

// Fake-only reader: tests command wiring, not SQL or trusted game identity.
VisitTicketReadReview ReadStagingVisitTicket(Player const* self)
{
    ++gTicketReadCalls;
    gLastTicketActor = self;
    return {VisitTicketReadStatus::Returning, false, false, false};
}
void AddStagingDiagnosticsScripts();
}

int main()
{
    unsigned checks = 0, failed = 0;
    auto test=[&](bool value, char const* label)
    {
        ++checks;
        if (!value) { ++failed; std::cerr << "FAIL: " << label << '\n'; }
    };

    NaxxGuildStrongholds::AddStagingDiagnosticsScripts();
    test(gRegisteredDiagnostics != nullptr,"Command registered only on explicit mock opt-in");
    if (!gRegisteredDiagnostics) return 1;
    auto commands = gRegisteredDiagnostics->GetCommands();
    test(commands.size() == 1 && commands[0].Name == "naxxgs",
         "Single top-level namespace");
    test(commands[0].Children.size() == 2,
         "Visit is a separate subcommand only under visit-read compile flag");
    if (commands[0].Children.size() != 2) return 1;
    auto const& snapshot = commands[0].Children[0];
    auto const& visit = commands[0].Children[1];
    test(snapshot.Name == "snapshot" && visit.Name == "visit",
         "Existing snapshot command preserved and visit added");
    test(visit.RequiredPermission == rbac::RBAC_PERM_COMMAND_DEBUG_INFO &&
         visit.ConsoleMode == Acore::ChatCommands::Console::No,
         "Visits restricted by RBAC and no console");

    Player player;
    player.GuildId = 101;
    player.MapId = 1;
    player.InstanceId = 45;
    player.PhaseMask = 3;
    ChatHandler handler;
    handler.PlayerValue = &player;
    test(visit.Handler(&handler),"Disabled report processed");
    test(handler.Messages.size() == 1 &&
         handler.Messages.back().find("diagnostic OFF") != std::string::npos,
         "Master diagnostic switch is independent barrier");
    test(NaxxGuildStrongholds::gTicketReadCalls == 0,
         "No underlying ticket query when diagnostic is disabled");

    configStorage.Enabled = true;
    test(visit.Handler(&handler),"GM-self visit command processed");
    test(NaxxGuildStrongholds::gTicketReadCalls == 1 &&
         NaxxGuildStrongholds::gLastTicketActor == &player,
         "Reader receives ONLY invoking player pointer");
    test(handler.Messages.size() == 2 &&
         handler.Messages.back().find("RETURNING") != std::string::npos &&
         handler.Messages.back().find("NOT verified") != std::string::npos,
         "Status never claims arrival verified");
    test(handler.Messages.back().find("guild=") == std::string::npos &&
         handler.Messages.back().find("phaseMask=") == std::string::npos,
         "Visit status does not reveal unrelated player details");

    handler.PlayerValue = nullptr;
    test(!visit.Handler(&handler),"Console or missing current character denied");
    test(NaxxGuildStrongholds::gTicketReadCalls == 1,
         "Missing player cannot cause a ticket query");
    test(!visit.Handler(nullptr),"Null handler refused");
    configStorage.Enabled = false;
    handler.PlayerValue = &player;
    test(visit.Handler(&handler) &&
         NaxxGuildStrongholds::gTicketReadCalls == 1,
         "Disabling config immediately blocks further ticket reads");
    test(player.GuildId == 101 && player.InstanceId == 45 &&
         player.PhaseMask == 3,"No player or phase mutation");
    delete gRegisteredDiagnostics;
    gRegisteredDiagnostics = nullptr;

    if (failed) return 1;
    std::cout<<"PASS: "<<checks<<" staging-only GM-self visit command tests\n";
    return 0;
}
