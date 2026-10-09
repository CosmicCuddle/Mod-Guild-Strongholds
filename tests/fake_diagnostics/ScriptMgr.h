#pragma once
#include "Chat.h"
#include <string>

struct CommandScript;
extern CommandScript* gRegisteredDiagnostics;

struct CommandScript
{
    explicit CommandScript(std::string)
    {
        gRegisteredDiagnostics = this;
    }
    virtual ~CommandScript() = default;
    virtual Acore::ChatCommands::ChatCommandTable GetCommands() const = 0;
};
