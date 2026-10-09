#include "Config.h"
#include "ScriptMgr.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

FakeConfigMgr gFakeConfig;
FakeConfigMgr* sConfigMgr = &gFakeConfig;
WorldScript* gFakeRegisteredScript = nullptr;

struct LogRecord
{
    std::string Severity;
    std::string Message;
};

std::vector<LogRecord> gLogs;

WorldScript::WorldScript(std::string const&, std::initializer_list<FakeWorldHook>)
{
    if (gFakeRegisteredScript)
    {
        std::cerr << "Multiple world scripts registered unexpectedly\n";
        std::abort();
    }
    gFakeRegisteredScript = this;
}

void FakeLog(std::string const& severity, std::string const& message)
{
    gLogs.push_back({severity, message});
}

void Addmod_guild_strongholdsScripts();
void AddMod_Guild_StrongholdsScripts();

int main()
{
    int checks = 0;
    int failures = 0;
    auto test = [&](bool ok, const char* reason)
    {
        ++checks;
        if (!ok)
        {
            ++failures;
            std::cerr << "FAILED: " << reason << '\n';
        }
    };

#if defined(NAXX_GS_TEST_UPPERCASE_LOADER)
    AddMod_Guild_StrongholdsScripts();
#else
    Addmod_guild_strongholdsScripts();
#endif
    test(gFakeRegisteredScript != nullptr, "Bootstrap WorldScript registered");

    gFakeRegisteredScript->OnBeforeConfigLoad(false);
    gFakeRegisteredScript->OnStartup();
    test(gLogs.size() == 1, "Disabled start logs just once");
    test(gLogs.back().Message.find("gameplay disabled") != std::string::npos,
         "Default is disabled");

    gLogs.clear();
    gFakeConfig.RequestedEnabled = true;
    gFakeRegisteredScript->OnBeforeConfigLoad(true);
    gFakeRegisteredScript->OnStartup();
    test(gLogs.size() == 2, "Requested enabled logs block and startup warnings");
    test(gLogs[0].Message.find("BLOCKED") != std::string::npos,
         "Enabled config is blocked");
    test(gLogs[1].Message.find("no gameplay") != std::string::npos,
         "No gameplay registered");

    gLogs.clear();
    gFakeConfig.RequestedEnabled = false;
    gFakeRegisteredScript->OnBeforeConfigLoad(true);
    gFakeRegisteredScript->OnStartup();
    test(gLogs.size() == 1 &&
         gLogs.back().Message.find("gameplay disabled") != std::string::npos,
         "Configuration reload restores disabled state");

    delete gFakeRegisteredScript;
    gFakeRegisteredScript = nullptr;

    if (failures)
        return 1;
    std::cout << "PASS: " << checks << " bootstrap/worldscript smoke checks (fake core)\n";
    return 0;
}
