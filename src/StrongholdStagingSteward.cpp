/*
 * STAGING-ONLY read-only Guild Steward (NOT gameplay).
 *
 * A normal worldserver build does NOT compile/register this script.
 * Even in a staging build it must be explicitly enabled in config,
 * invoked by a real staff/GM character in a guild, and bound manually
 * to a separately reviewed creature_template.ScriptName on staging.
 *
 * No production NPC entry, creature spawn, SQL, phase, map or teleport.
 */
#if defined(NAXX_GS_BUILD_STAGING_STEWARD)

#include "StrongholdStewardPreview.h"
#include "Config.h"
#include "Creature.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

#include <string>

namespace NaxxGuildStrongholds
{
namespace
{
constexpr std::uint32_t kActionOverview =
    GOSSIP_ACTION_INFO_DEF + static_cast<std::uint32_t>(StewardPreviewAction::Overview);
constexpr std::uint32_t kActionBuildings =
    GOSSIP_ACTION_INFO_DEF + static_cast<std::uint32_t>(StewardPreviewAction::FutureBuildings);
constexpr std::uint32_t kActionClose =
    GOSSIP_ACTION_INFO_DEF + static_cast<std::uint32_t>(StewardPreviewAction::Close);

StewardPreviewContext ReadContext(Player const* player)
{
    StewardPreviewContext ctx;
    ctx.ExplicitStagingSettingEnabled = sConfigMgr->GetOption<bool>(
        "NaxxGuildStrongholds.StagingSteward.Enabled", false);
    if (player)
    {
        ctx.StaffGameMaster = player->IsGameMaster();
        ctx.CurrentGuildId = player->GetGuildId();
    }
    return ctx;
}

class GuildStrongholdsStagingSteward final : public CreatureScript
{
public:
    GuildStrongholdsStagingSteward()
        : CreatureScript("npc_naxx_guild_steward_staging") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!player || !creature)
            return true;
        ClearGossipMenuFor(player);

        if (CheckStewardPreview(ReadContext(player)) !=
            StewardPreviewDecision::Allowed)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        // All options are informative; NO action changes the game world.
        AddGossipItemFor(player, GOSSIP_ICON_CHAT,
            "Guild " + std::to_string(player->GetGuildId()) +
            ": view development status",
            GOSSIP_SENDER_MAIN, kActionOverview);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT,
            "Planned guild buildings and raid trophies",
            GOSSIP_SENDER_MAIN, kActionBuildings);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT,
            "Close preview", GOSSIP_SENDER_MAIN, kActionClose);
        SendGossipMenuFor(player, player->GetGossipTextId(creature), creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature,
        std::uint32_t sender, std::uint32_t action) override
    {
        if (!player || !creature)
            return true;

        ClearGossipMenuFor(player);
        // SECURITY: sender and selection can be fabricated in client packets.
        // Validate *current* GM status, guild and config on each click.
        if (action < GOSSIP_ACTION_INFO_DEF ||
            CheckStewardSelection(ReadContext(player),
                sender == GOSSIP_SENDER_MAIN,
                action - GOSSIP_ACTION_INFO_DEF) != StewardPreviewDecision::Allowed)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        if (action == kActionClose)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        if (action == kActionOverview)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Development preview only: private housing is not available.",
                GOSSIP_SENDER_MAIN, kActionClose);
        }
        else if (action == kActionBuildings)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Future: guild buildings, raid trophies and decorations. Not unlocked.",
                GOSSIP_SENDER_MAIN, kActionClose);
        }
        else
        {
            CloseGossipMenuFor(player);
            return true;
        }

        SendGossipMenuFor(player, player->GetGossipTextId(creature), creature);
        return true;
    }
};
}

void AddStagingStewardScripts()
{
    new GuildStrongholdsStagingSteward();
}
}

#endif // NAXX_GS_BUILD_STAGING_STEWARD
