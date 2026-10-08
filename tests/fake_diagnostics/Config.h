#pragma once
#include <string>

struct FakeDiagnosticsConfig
{
    bool Enabled = false;
    template<class T>
    T GetOption(std::string const& key, T fallback) const
    {
        if (key == "NaxxGuildStrongholds.Diagnostics.Enabled")
            return static_cast<T>(Enabled);
        return fallback;
    }
};
extern FakeDiagnosticsConfig* sConfigMgr;
