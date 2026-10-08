#pragma once
#include <string>
#include <utility>
#include <vector>

struct Player;
struct ChatHandler
{
    Player* PlayerValue = nullptr;
    std::vector<std::string> Messages;
    Player* GetPlayer() const { return PlayerValue; }
    void SendSysMessage(std::string const& message) { Messages.push_back(message); }
};

namespace Acore::ChatCommands
{
enum class Console { No, Yes };
struct ChatCommandEntry
{
    std::string Name;
    bool (*Handler)(ChatHandler*) = nullptr;
    std::vector<ChatCommandEntry> Children;
    Console ConsoleMode = Console::No;
    int RequiredPermission = 0;

    // Match upstream ChatCommandBuilder\x27s lvalue function-handler requirement.\n    // Passing &HandleSnapshot produces a temporary pointer and fails this check.\n    ChatCommandEntry(char const* name, bool (&handler)(ChatHandler*), int permission, Console mode)
        : Name(name), Handler(handler), ConsoleMode(mode), RequiredPermission(permission) {}

    ChatCommandEntry(char const* name, std::vector<ChatCommandEntry> children)
        : Name(name), Children(std::move(children)) {}
};
using ChatCommandTable = std::vector<ChatCommandEntry>;
}
