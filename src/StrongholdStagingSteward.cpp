/*
 * STAGING-ONLY read-only Guild Steward (NOT housing gameplay).
 *
 * Not compiled/registered in normal builds. Requires a separate compile
 * flag, disabled-by-default config, GM character in guild, and a manually
 * authorised staging NPC ScriptName binding. No SQL or world changes.
 */
#if defined(NAXX_GS_BUILD_STAGING_STEWARD)

#include "StrongholdStewardPreview.h"
#include "StrongholdStewardReadOnlyContent.h"
#include "StrongholdStewardEvidence.h"
#include "StrongholdStagingGuildAdapter.h"
#if defined(NAXX_GS_BUILD_STAGING_IP_READ)
#include "StrongholdStagingIpAdapter.h"
#endif
#if defined(NAXX_GS_BUILD_STAGING_PROPERTY_READ)
#include "StrongholdStagingPropertyAdapter.h"
#endif
#include "Config.h"
#include "Creature.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace NaxxGuildStrongholds
{
namespace
{
constexpr std::uint32_t kActionBase = GOSSIP_ACTION_INFO_DEF;

std::uint32_t EncodeAction(StewardPreviewAction action)
{
    return kActionBase + static_cast<std::uint32_t>(action);
}

StewardPreviewContext ReadContext(Player const* player)
{
    StewardPreviewContext ctx;
    ctx.ExplicitStagingSettingEnabled = sConfigMgr->GetOption<bool>(
        "NaxxGuildStrongholds.StagingSteward.Enabled", false);
    if (player)
    {
        ctx.StaffGameMaster = player->IsGameMaster();
        ctx.CurrentGuildId = player->GetGuildId();
        // Registry ID, member GUID and actual creation date must ALL match.
        ctx.VerifiedActiveMembershipAndGeneration =
            ReadCurrentGuildIdentity(player).IsVerified();
    }
    return ctx;
}

void AddNavigation(Player* player)
{
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back to overview",
        GOSSIP_SENDER_MAIN, EncodeAction(StewardPreviewAction::Back));
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Close staging preview",
        GOSSIP_SENDER_MAIN, EncodeAction(StewardPreviewAction::Close));
}

void ShowMainMenu(Player* player, Creature* creature)
{
    ClearGossipMenuFor(player);
    // Explicit PREVIEW wording: never imply these styles, buildings,
    // guild levels or quests have been earned or made available.
    for (std::pair<StewardPreviewAction, char const*> const& item : {
        std::pair{StewardPreviewAction::Overview, "Development overview [LOCKED]"},
        std::pair{StewardPreviewAction::AllianceThemes, "Alliance racial themes [PLAN]"},
        std::pair{StewardPreviewAction::HordeThemes, "Horde racial themes [PLAN]"},
        std::pair{StewardPreviewAction::HumanBuildings, "Human buildings [PLAN]"},
        std::pair{StewardPreviewAction::OrcBuildings, "Orc buildings [PLAN]"},
        std::pair{StewardPreviewAction::DailyActivities, "Daily activity ideas [PLAN]"},
        std::pair{StewardPreviewAction::WeeklyActivities, "Weekly activity ideas [PLAN]"},
        std::pair{StewardPreviewAction::Trophies, "Raid trophies [PLAN]"},
        std::pair{StewardPreviewAction::EvidenceReview, "Guild and IP evidence [LOCKED]"}
    })
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, item.second,
            GOSSIP_SENDER_MAIN, EncodeAction(item.first));

    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Close staging preview",
        GOSSIP_SENDER_MAIN, EncodeAction(StewardPreviewAction::Close));
    SendGossipMenuFor(player, player->GetGossipTextId(creature), creature);
}

void ShowDetailPage(Player* player, Creature* creature, StewardPreviewAction page)
{
    ClearGossipMenuFor(player);
    // Server guild/member/creation are independently verified.
    // A separate *extra opt-in* staging adapter may SELECT draft property
    // fields. IP rank and private-area isolation remain unverified/locked.
    std::vector<std::string> rows;
    if (page == StewardPreviewAction::EvidenceReview)
    {
        StewardEvidenceInput evidence;
        GuildReadResult const guild = ReadCurrentGuildIdentity(player);
        evidence.ActorGuild.GuildId = player->GetGuildId();
        if (guild.IsVerified())
        {
            evidence.ActorGuild = guild.Identity;
            evidence.GuildGenerationVerified = true;
        }
#if defined(NAXX_GS_BUILD_STAGING_PROPERTY_READ)
        // EXTRA independent staging compile/config opt-in. A SELECT on our
        // own DRAFT table, never auto-install SQL or presume a missing row
        // grants ownership. Same original guild generation is mandatory.
        if (guild.IsVerified())
        {
            PropertyReadResult const property = ReadStagingProperty(guild.Identity);
            if (property.CanDisplayRow())
            {
                evidence.PropertySnapshotLoaded = property.CanDisplayRow();
                evidence.Property = property.Property;
                evidence.SettlementLevelVerified = true;
                evidence.SettlementLevel = property.Level;
            }
        }
#endif
#if defined(NAXX_GS_BUILD_STAGING_IP_READ)
        // Separate opt-in and compile-time dependency on reviewed IP header.
        // The actual module's enabled flag, quest rank and cap are READ only.
        // Even valid guild/property/IP proof NEVER enables private housing.
        evidence.Ip = ReadStagingIpState(player, guild.IsVerified());
#endif
        rows = BuildStewardEvidenceRows(evidence);
    }
    else
        rows = BuildStewardPreviewRows(page);
    // Defensive cap: never flood client with an unbounded source catalogue.
    // Every detail line is itself a self-loop; it cannot trigger gameplay.
    std::size_t rendered = 0;
    for (std::string const& row : rows)
    {
        if (rendered++ >= 7)
            break;
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, row,
            GOSSIP_SENDER_MAIN, EncodeAction(page));
    }
    AddNavigation(player);
    SendGossipMenuFor(player, player->GetGossipTextId(creature), creature);
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
        if (CheckStewardPreview(ReadContext(player)) != StewardPreviewDecision::Allowed)
        {
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);
            return true;
        }
        ShowMainMenu(player, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature,
        std::uint32_t sender, std::uint32_t action) override
    {
        if (!player || !creature)
            return true;

        // SECURITY: client-supplied sender and action can be forged.
        // Validate current staff, guild and config before EVERY selection.
        if (action < kActionBase ||
            CheckStewardSelection(ReadContext(player),
                sender == GOSSIP_SENDER_MAIN,
                action - kActionBase) != StewardPreviewDecision::Allowed)
        {
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);
            return true;
        }

        StewardPreviewAction const choice =
            static_cast<StewardPreviewAction>(action - kActionBase);
        if (choice == StewardPreviewAction::Close)
        {
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);
        }
        else if (choice == StewardPreviewAction::Back)
            ShowMainMenu(player, creature);
        else
            ShowDetailPage(player, creature, choice);
        return true;
    }
};
}

void AddStagingStewardScripts()
{
    new GuildStrongholdsStagingSteward();
}
}

#endif
