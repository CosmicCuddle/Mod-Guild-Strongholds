#pragma once
#include <string>

struct FakeConfigMgr
{
    bool RequestedEnabled = false;

    template<class T>
    T GetOption(std::string const& key, T fallback) const
    {
        if (key == "NaxxGuildStrongholds.Enabled")
            return static_cast<T>(RequestedEnabled);
        return fallback;
    }
};

extern FakeConfigMgr* sConfigMgr;
