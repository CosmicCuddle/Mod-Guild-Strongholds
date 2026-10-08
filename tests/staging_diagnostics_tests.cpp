#include "Chat.h"
#include "Config.h"
#include "Player.h"
#include "RBAC.h"
#include "ScriptMgr.h"

#include <iostream>
#include <string>

FakeDiagnosticsConfig configStorage;
FakeDiagnosticsConfig* sConfigMgr = &configStorage;
CommandScript* gRegisteredDiagnostics = nullptr;

void Addmod_guild_strongholdsScripts();

namespace NaxxGuildStrongholds
{
void AddStagingDiagnosticsScripts();
}

int main()
{
    int tests = 0;
    int failures = 0;
    auto check = [&](bool okay, char const* reason)
    {
        ++tests;
        if (!okay)
        {
            ++failures;
            std::cerr << "FAILED: " << reason << '\n';
        }
    };

    // The opt-in command is only explicitly registered in this test;
    // the production loader has an independent compile-time guard.
    NaxxGuildStrongholds::AddStagingDiagnosticsScripts();
    check(gRegisteredDiagnostics != nullptr, "Registered test command script");
    if (!gRegisteredDiagnostics)
        return 1;
    const auto commands = gRegisteredDiagnostics->GetCommands();
    check(commands.size() == 1 && commands[0].Name == "naxxgs",
          "Unique command namespace");
    check(commands[0].Children.size() == 1 &&
          commands[0].Children[0].Name == "snapshot",
          "Snapshot is the only staging command");
    if (commands[0].Children.empty())
        return 1;
    const auto snapshot = commands[0].Children[0];
    check(snapshot.RequiredPermission == rbac::RBAC_PERM_COMMAND_DEBUG_INFO &&
          snapshot.ConsoleMode == Acore::ChatCommands::Console::No,
          "GM-only RBAC and no console");
    Player p;
    p.GuildId = 431;
    p.MapId = 1;
    p.InstanceId = 17;
    p.ZoneId = 876;
    p.AreaId = 876;
    p.PhaseMask = 3;
    ChatHandler handler;
    handler.PlayerValue = &p;
    check(snapshot.Handler(&handler), "Command disabled response");
    check(handler.Messages.size() == 1 &&
          handler.Messages[0].find("diagnostic OFF") != std::string::npos,
          "Default config denies observation");

    configStorage.Enabled = true;
    check(snapshot.Handler(&handler), "Enabled observation succeeds");
    check(handler.Messages.size() == 2 &&
          handler.Messages[1].find("guild=431") != std::string::npos &&
          handler.Messages[1].find("instance=17") != std::string::npos &&
          handler.Messages[1].find("phaseMask=3") != std::string::npos,
          "Reads map, instance, guild and phase from current player only");
    check(handler.Messages[1].find("UNKNOWN") != std::string::npos &&
          handler.Messages[1].find("NO PRIVACY APPROVAL") != std::string::npos,
          "Cannot claim private isolation or phase-comparison mode");
    configStorage.Enabled = false;
    check(snapshot.Handler(&handler) &&
          handler.Messages.size() == 3 &&
          handler.Messages.back().find("diagnostic OFF") != std::string::npos,
          "Runtime config disables observations again");

    // Deliberately simulate no current character. No state mutation.
    configStorage.Enabled = true;
    handler.PlayerValue = nullptr;
    check(!snapshot.Handler(&handler), "No character => refuse");
    check(!snapshot.Handler(nullptr), "Null ChatHandler => refuse");
    check(p.GuildId == 431 && p.PhaseMask == 3 && p.InstanceId == 17,
          "Observed player was not modified");
    delete gRegisteredDiagnostics;
    gRegisteredDiagnostics = nullptr;

    if (failures)
        return 1;
    std::cout << "PASS: " << tests << " read-only staging diagnostic smoke checks\n";
    return 0;
}
