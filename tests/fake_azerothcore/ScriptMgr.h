#pragma once
#include <initializer_list>
#include <string>

enum FakeWorldHook { WORLDHOOK_ON_BEFORE_CONFIG_LOAD, WORLDHOOK_ON_STARTUP };

struct WorldScript
{
    WorldScript(std::string const&, std::initializer_list<FakeWorldHook>);
    virtual ~WorldScript() = default;
    virtual void OnBeforeConfigLoad(bool) {}
    virtual void OnStartup() {}
};

extern WorldScript* gFakeRegisteredScript;
