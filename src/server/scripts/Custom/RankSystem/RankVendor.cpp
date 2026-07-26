#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

#include <sstream>

namespace
{
    constexpr uint32 RANK_VENDOR_ACTION_BASE = 44199;

    std::string GetVendorGreeting(Player* player, char const* ruText, char const* enText)
    {
        std::ostringstream out;
        out << (player->GetSession()->GetSessionDbLocaleIndex() == LOCALE_ruRU ? ruText : enText);
        return out.str();
    }

    void AddRankVendorItems(
        Player* player,
        uint32 minRank,
        uint32 maxRank)
    {
        uint32 playerRank = static_cast<uint32>(player->GetRankByExp());
        for (uint32 rank = minRank; rank <= maxRank; ++rank)
        {
            if (playerRank >= rank)
            {
                AddGossipItemFor(
                    player,
                    GOSSIP_ICON_BATTLE,
                    "|TInterface\\icons\\Ability_warrior_rampage:20:20:0:0|t > Rank " + std::to_string(rank),
                    GOSSIP_SENDER_MAIN,
                    RANK_VENDOR_ACTION_BASE + rank);
            }
        }
    }
}

class npc_pvp_rang_10 : public CreatureScript
{
public:
    npc_pvp_rang_10() : CreatureScript("npc_pvp_rang_10") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PlayerTalkClass->ClearMenus();
        uint32 playerRank = static_cast<uint32>(player->GetRankByExp());

        AddRankVendorItems(player, 1, 10);
        if (playerRank == 0)
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Вам нужно повысить ранг, чтобы открыть продавца.", GOSSIP_SENDER_MAIN, 0);

        player->PlayerTalkClass->SendGossipMenu(
            GetVendorGreeting(
                player,
                "Уважаемый игрок. Здесь продавцы ранга 1-10.",
                "Dear player. Rank vendors 1-10 are available here."),
            creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!action)
            return true;

        uint32 const rankRequired = action - RANK_VENDOR_ACTION_BASE;
        if (static_cast<uint32>(player->GetRankByExp()) >= rankRequired)
            player->GetSession()->SendListInventory(creature->GetGUID(), action);

        return true;
    }
};

class npc_pvp_rang_20 : public CreatureScript
{
public:
    npc_pvp_rang_20() : CreatureScript("npc_pvp_rang_20") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PlayerTalkClass->ClearMenus();
        uint32 playerRank = static_cast<uint32>(player->GetRankByExp());
        AddRankVendorItems(player, 11, 20);

        if (playerRank < 11)
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Требуется минимум 11 ранг.", GOSSIP_SENDER_MAIN, 0);

        player->PlayerTalkClass->SendGossipMenu(
            GetVendorGreeting(
                player,
                "Уважаемый игрок. Здесь продавцы ранга 11-20.",
                "Dear player. Rank vendors 11-20 are available here."),
            creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!action)
            return true;

        uint32 const rankRequired = action - RANK_VENDOR_ACTION_BASE;
        if (static_cast<uint32>(player->GetRankByExp()) >= rankRequired)
            player->GetSession()->SendListInventory(creature->GetGUID(), action);

        return true;
    }
};

class npc_pvp_rang_30 : public CreatureScript
{
public:
    npc_pvp_rang_30() : CreatureScript("npc_pvp_rang_30") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PlayerTalkClass->ClearMenus();
        uint32 playerRank = static_cast<uint32>(player->GetRankByExp());
        AddRankVendorItems(player, 21, 30);

        if (playerRank < 21)
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Требуется минимум 21 ранг.", GOSSIP_SENDER_MAIN, 0);

        player->PlayerTalkClass->SendGossipMenu(
            GetVendorGreeting(
                player,
                "Уважаемый игрок. Здесь продавцы ранга 21-30.",
                "Dear player. Rank vendors 21-30 are available here."),
            creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!action)
            return true;

        uint32 const rankRequired = action - RANK_VENDOR_ACTION_BASE;
        if (static_cast<uint32>(player->GetRankByExp()) >= rankRequired)
            player->GetSession()->SendListInventory(creature->GetGUID(), action);

        return true;
    }
};

class npc_pvp_rang_40 : public CreatureScript
{
public:
    npc_pvp_rang_40() : CreatureScript("npc_pvp_rang_40") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PlayerTalkClass->ClearMenus();
        uint32 playerRank = static_cast<uint32>(player->GetRankByExp());
        AddRankVendorItems(player, 31, 40);

        if (playerRank < 31)
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Требуется минимум 31 ранг.", GOSSIP_SENDER_MAIN, 0);

        player->PlayerTalkClass->SendGossipMenu(
            GetVendorGreeting(
                player,
                "Уважаемый игрок. Здесь продавцы ранга 31-40.",
                "Dear player. Rank vendors 31-40 are available here."),
            creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!action)
            return true;

        uint32 const rankRequired = action - RANK_VENDOR_ACTION_BASE;
        if (static_cast<uint32>(player->GetRankByExp()) >= rankRequired)
            player->GetSession()->SendListInventory(creature->GetGUID(), action);

        return true;
    }
};

class npc_pvp_rang_50 : public CreatureScript
{
public:
    npc_pvp_rang_50() : CreatureScript("npc_pvp_rang_50") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PlayerTalkClass->ClearMenus();
        uint32 playerRank = static_cast<uint32>(player->GetRankByExp());
        AddRankVendorItems(player, 41, 50);

        if (playerRank < 41)
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Требуется минимум 41 ранг.", GOSSIP_SENDER_MAIN, 0);

        player->PlayerTalkClass->SendGossipMenu(
            GetVendorGreeting(
                player,
                "Уважаемый игрок. Здесь продавцы ранга 41-50.",
                "Dear player. Rank vendors 41-50 are available here."),
            creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!action)
            return true;

        uint32 const rankRequired = action - RANK_VENDOR_ACTION_BASE;
        if (static_cast<uint32>(player->GetRankByExp()) >= rankRequired)
            player->GetSession()->SendListInventory(creature->GetGUID(), action);

        return true;
    }
};

class npc_pvp_rang_info : public CreatureScript
{
public:
    npc_pvp_rang_info() : CreatureScript("npc_pvp_rang_info") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PlayerTalkClass->ClearMenus();

        std::ostringstream text;
        if (player->GetSession()->GetSessionDbLocaleIndex() == LOCALE_ruRU)
        {
            text << "Ваш текущий ранг: " << player->GetRankByExp() << "\n"
                 << "У вас: " << player->GetRankPoints() << " опыта\n"
                 << "До следующего ранга: " << player->PointsUntilNextRank() << " опыта";
        }
        else
        {
            text << "Your current rank: " << player->GetRankByExp() << "\n"
                 << "You have: " << player->GetRankPoints() << " experience\n"
                 << "Until next rank: " << player->PointsUntilNextRank() << " experience";
        }

        AddGossipItemFor(
            player,
            GOSSIP_ICON_BATTLE,
            "Обновить меню",
            GOSSIP_SENDER_MAIN,
            1);

        player->PlayerTalkClass->SendGossipMenu(text.str(), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (sender == GOSSIP_SENDER_MAIN && action == 1)
            return OnGossipHello(player, creature);

        return true;
    }
};

void AddSC_NPC_RANK_VENDOR()
{
    new npc_pvp_rang_10();
    new npc_pvp_rang_20();
    new npc_pvp_rang_30();
    new npc_pvp_rang_40();
    new npc_pvp_rang_50();
    new npc_pvp_rang_info();
}
