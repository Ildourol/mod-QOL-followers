/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#include "AllCreatureScript.h"
#include "Chat.h"
#include "CommandScript.h"
#include "Creature.h"
#include "Opcodes.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptedGossip.h"
#include "ServerScript.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "StringFormat.h"
#include "Trainer.h"
#include "UtilityFollowerAI.h"
#include "UtilityFollowerCommon.h"
#include "UtilityFollowerConfig.h"
#include "UtilityFollowerMgr.h"
#include "World.h"
#include "WorldConfig.h"
#include "WorldPacket.h"
#include "WorldScript.h"
#include "WorldSession.h"

using namespace Acore::ChatCommands;

// ============================================================================
// Comprehensive Teleport Destinations (Major Cities, All Vanilla/TBC/WotLK Dungeons & Raids)
// ============================================================================
static constexpr TeleportLocation kTeleportLocations[] =
{
    // Major Neutral Cities & World Hubs
    { ACTION_TELEPORT_DALARAN,             "Dalaran",                      571, 5807.98f,   588.487f,  660.940f,  1.66594f },
    { ACTION_TELEPORT_SHATTRATH,           "Shattrath City",               530, -1838.16f,  5301.79f,  -12.428f,  5.95170f },
    { ACTION_TELEPORT_STORMWIND,           "Stormwind City",               0,   -8833.38f,  628.628f,  94.0066f,  1.06535f },
    { ACTION_TELEPORT_IRONFORGE,           "Ironforge",                    0,   -4918.88f, -940.406f,  501.564f,  5.42347f },
    { ACTION_TELEPORT_DARNASSUS,           "Darnassus",                    1,    9949.56f,  2284.21f,  1341.40f,  1.59587f },
    { ACTION_TELEPORT_EXODAR,              "The Exodar",                   530, -3965.70f, -11653.6f, -138.844f,  0.85215f },
    { ACTION_TELEPORT_ORGRIMMAR,           "Orgrimmar",                    1,    1629.85f, -4373.64f,  31.5573f,  3.69762f },
    { ACTION_TELEPORT_UNDERCITY,           "Undercity",                    0,    1584.14f,  240.308f, -52.1534f,  0.04179f },
    { ACTION_TELEPORT_THUNDER_BLUFF,       "Thunder Bluff",                1,   -1277.37f,  124.804f,  131.287f,  5.22274f },
    { ACTION_TELEPORT_SILVERMOON,          "Silvermoon City",              530,  9487.69f, -7279.20f,  14.2866f,  6.16478f },
    { ACTION_TELEPORT_KARAZHAN,            "Karazhan",                     0,   -11118.9f, -2010.33f,  47.0819f,  0.64989f },
    { ACTION_TELEPORT_CAVERNS_OF_TIME,     "Caverns of Time",              1,   -8204.88f, -4495.25f,  9.00910f,  4.72574f },
    { ACTION_TELEPORT_GADGETZAN,           "Gadgetzan",                    1,   -7177.15f, -3785.34f,  8.36981f,  6.10237f },
    { ACTION_TELEPORT_BOOTY_BAY,           "Booty Bay",                    0,   -14297.2f,  530.993f,  8.77916f,  3.98863f },

    // Vanilla Dungeons (Outside Entrances)
    { ACTION_TELEPORT_RFC,                 "Ragefire Chasm",               1,    1811.78f, -4410.50f, -18.4704f,  5.20165f },
    { ACTION_TELEPORT_VC,                  "The Deadmines",                0,   -11208.7f,  1673.52f,  24.6361f,  1.51067f },
    { ACTION_TELEPORT_WC,                  "Wailing Caverns",              1,   -731.607f, -2218.39f,  17.0281f,  2.78486f },
    { ACTION_TELEPORT_SFK,                 "Shadowfang Keep",              0,   -234.675f,  1561.63f,  76.8921f,  1.24031f },
    { ACTION_TELEPORT_BFD,                 "Blackfathom Deeps",            1,    4249.99f,  740.102f, -25.6710f,  1.34062f },
    { ACTION_TELEPORT_STOCKADE,            "The Stockade",                 0,   -8779.90f,  834.349f,  94.6801f,  0.65301f },
    { ACTION_TELEPORT_GNOMER,              "Gnomeregan",                   0,   -5163.54f,  925.423f,  257.181f,  1.57423f },
    { ACTION_TELEPORT_RFK,                 "Razorfen Kraul",               1,   -4470.28f, -1677.77f,  81.3925f,  1.16302f },
    { ACTION_TELEPORT_SM,                  "Scarlet Monastery",            0,    2872.60f, -764.398f,  160.332f,  5.05735f },
    { ACTION_TELEPORT_RFD,                 "Razorfen Downs",               1,   -4657.30f, -2519.35f,  81.0529f,  4.54808f },
    { ACTION_TELEPORT_ULDA,                "Uldaman",                      0,   -6071.37f, -2955.16f,  209.782f,  0.01570f },
    { ACTION_TELEPORT_ZF,                  "Zul'Farrak",                   1,   -6801.19f, -2893.02f,  9.00388f,  0.15863f },
    { ACTION_TELEPORT_MARA,                "Maraudon",                     1,   -1419.13f,  2908.14f,  137.464f,  1.57366f },
    { ACTION_TELEPORT_ST,                  "The Sunken Temple",            0,   -10177.9f, -3994.90f, -111.239f,  6.01885f },
    { ACTION_TELEPORT_BRD,                 "Blackrock Depths",             0,   -7179.34f, -921.212f,  165.821f,  5.09599f },
    { ACTION_TELEPORT_LBRS,                "Lower Blackrock Spire",        0,   -7527.05f, -1226.77f,  285.732f,  5.29626f },
    { ACTION_TELEPORT_UBRS,                "Upper Blackrock Spire",        0,   -7535.40f, -1212.30f,  285.450f,  5.25000f },
    { ACTION_TELEPORT_DM,                  "Dire Maul",                    1,   -3980.80f,  789.005f,  161.007f,  4.71945f },
    { ACTION_TELEPORT_SCHOLO,              "Scholomance",                  0,    1269.64f, -2556.21f,  93.6088f,  0.62062f },
    { ACTION_TELEPORT_STRAT,               "Stratholme",                   0,    3352.92f, -3379.03f,  144.782f,  6.25978f },

    // Vanilla Raids (Outside Entrances)
    { ACTION_TELEPORT_MC,                  "Molten Core",                  0,   -7538.51f, -1063.45f,  180.981f,  0.03409f },
    { ACTION_TELEPORT_BWL,                 "Blackwing Lair",               0,   -7515.40f, -1045.60f,  182.300f,  0.10000f },
    { ACTION_TELEPORT_AQ20,                "Ruins of Ahn'Qiraj",           1,   -8409.82f,  1499.06f,  27.7179f,  2.51868f },
    { ACTION_TELEPORT_AQ40,                "Temple of Ahn'Qiraj",          1,   -8240.09f,  1991.32f,  129.072f,  0.94160f },
    { ACTION_TELEPORT_ONYXIA,              "Onyxia's Lair",                1,   -4708.27f, -3727.64f,  54.5589f,  3.72786f },
    { ACTION_TELEPORT_ZG,                  "Zul'Gurub",                    0,   -11916.7f, -1215.72f,  92.2890f,  4.72454f },

    // TBC Dungeons (Outside Entrances)
    { ACTION_TELEPORT_RAMPARTS,            "Hellfire Ramparts",            530, -360.671f,  3071.90f, -15.0977f,  1.89389f },
    { ACTION_TELEPORT_BLOOD_FURNACE,       "The Blood Furnace",            530, -291.324f,  3149.10f,  31.5541f,  2.27147f },
    { ACTION_TELEPORT_SHATTERED_HALLS,     "The Shattered Halls",          530, -305.790f,  3061.63f, -2.53847f,  1.88888f },
    { ACTION_TELEPORT_SLAVE_PENS,          "The Slave Pens",               530,  717.282f,  6979.87f, -73.0281f,  1.50287f },
    { ACTION_TELEPORT_UNDERBOG,            "The Underbog",                 530,  763.307f,  6767.81f, -67.7695f,  5.99726f },
    { ACTION_TELEPORT_STEAMVAULT,          "The Steamvault",               530,  794.537f,  6927.81f, -80.4757f,  0.15908f },
    { ACTION_TELEPORT_MANA_TOMBS,          "Mana-Tombs",                   530, -3104.18f,  4945.52f, -101.507f,  6.22344f },
    { ACTION_TELEPORT_AUCHENAI_CRYPTS,     "Auchenai Crypts",              530, -3362.04f,  5209.85f, -101.050f,  1.60924f },
    { ACTION_TELEPORT_SETHEKK_HALLS,       "Sethekk Halls",                530, -3362.20f,  4664.12f, -101.049f,  4.66050f },
    { ACTION_TELEPORT_SHADOW_LABYRINTH,    "Shadow Labyrinth",             530, -3627.90f,  4941.98f, -101.049f,  3.16039f },
    { ACTION_TELEPORT_DURNHOLDE,           "Old Hillsbrad Foothills",      1,   -8404.30f, -4070.62f, -208.586f,  0.23703f },
    { ACTION_TELEPORT_BLACK_MORASS,        "The Black Morass",             1,   -8734.30f, -4230.11f, -209.500f,  2.16212f },
    { ACTION_TELEPORT_BOTANICA,            "The Botanica",                 530,  3407.11f,  1488.48f,  182.838f,  5.59559f },
    { ACTION_TELEPORT_MECHANAR,            "The Mechanar",                 530,  2867.12f,  1549.42f,  252.159f,  3.82218f },
    { ACTION_TELEPORT_ARCATRAZ,            "The Arcatraz",                 530,  3308.92f,  1340.72f,  505.560f,  4.94686f },
    { ACTION_TELEPORT_MAGISTERS_TERRACE,   "Magisters' Terrace",           530,  12884.6f, -7317.69f,  65.5023f,  4.79900f },

    // TBC Raids (Outside Entrances)
    { ACTION_TELEPORT_KARA_RAID,           "Karazhan",                     0,   -11118.9f, -2010.33f,  47.0819f,  0.64989f },
    { ACTION_TELEPORT_GRUUL,               "Gruul's Lair",                 530,  3530.06f,  5104.08f,  3.50861f,  5.51117f },
    { ACTION_TELEPORT_MAGTHERIDON,         "Magtheridon's Lair",           530, -312.700f,  3087.26f, -116.520f,  5.19026f },
    { ACTION_TELEPORT_SSC,                 "Serpentshrine Cavern",         530,  820.025f,  6864.93f, -66.7556f,  6.28127f },
    { ACTION_TELEPORT_THE_EYE,             "The Eye (Tempest Keep)",       530,  3088.49f,  1381.57f,  184.863f,  4.61973f },
    { ACTION_TELEPORT_HYJAL,               "Battle for Mount Hyjal",       1,   -8177.89f, -4181.23f, -167.552f,  0.91333f },
    { ACTION_TELEPORT_BLACK_TEMPLE,        "Black Temple",                 530, -3649.92f,  317.469f,  35.2827f,  2.94285f },
    { ACTION_TELEPORT_SUNWELL,             "Sunwell Plateau",              530,  12574.1f, -6774.81f,  15.0904f,  3.13788f },
    { ACTION_TELEPORT_ZULAMAN,             "Zul'Aman",                     530,  6851.78f, -7972.57f,  179.242f,  4.64691f },

    // WotLK Dungeons (Outside Entrances)
    { ACTION_TELEPORT_UTGARDE_KEEP,        "Utgarde Keep",                 571,  1219.72f, -4865.28f,  41.2479f,  0.31322f },
    { ACTION_TELEPORT_THE_NEXUS,           "The Nexus",                    571,  3893.51f,  6985.33f,  69.4877f,  6.27898f },
    { ACTION_TELEPORT_AZJOL_NERUB,         "Azjol-Nerub",                  571,  3677.53f,  2166.70f,  35.8080f,  2.30108f },
    { ACTION_TELEPORT_AHN_KAHET,           "Ahn'kahet: The Old Kingdom",   571,  3643.31f,  2036.51f,  1.78742f,  4.33919f },
    { ACTION_TELEPORT_DRAK_THARON,         "Drak'Tharon Keep",             571,  4774.60f, -2032.92f,  229.150f,  1.59000f },
    { ACTION_TELEPORT_VIOLET_HOLD,         "The Violet Hold",              571,  5685.50f,  493.516f,  652.593f,  4.03351f },
    { ACTION_TELEPORT_GUNDRAK,             "Gundrak",                      571,  6952.30f, -4419.98f,  450.078f,  0.80751f },
    { ACTION_TELEPORT_HALLS_OF_STONE,      "Halls of Stone",               571,  8921.91f, -993.503f,  1039.41f,  1.55263f },
    { ACTION_TELEPORT_HALLS_OF_LIGHTNING,  "Halls of Lightning",           571,  9182.92f, -1384.82f,  1110.21f,  5.57779f },
    { ACTION_TELEPORT_THE_OCULUS,          "The Oculus",                   571,  3879.96f,  6984.62f,  106.312f,  3.19669f },
    { ACTION_TELEPORT_UTGARDE_PINNACLE,    "Utgarde Pinnacle",             571,  1259.33f, -4852.02f,  215.763f,  3.48293f },
    { ACTION_TELEPORT_CULLING_STRATHOLME,  "The Culling of Stratholme",    1,   -8750.76f, -4442.20f, -199.260f,  4.37694f },
    { ACTION_TELEPORT_TRIAL_OF_CHAMPION,   "Trial of the Champion",        571,  8588.42f,  791.888f,  558.236f,  3.23819f },
    { ACTION_TELEPORT_FORGE_OF_SOULS,      "The Forge of Souls",           571,  5666.25f,  2009.20f,  798.041f,  5.43184f },
    { ACTION_TELEPORT_PIT_OF_SARON,        "Pit of Saron",                 571,  5598.74f,  2015.85f,  798.042f,  3.81001f },
    { ACTION_TELEPORT_HALLS_OF_REFLECTION, "Halls of Reflection",          571,  5630.44f,  1994.01f,  798.059f,  4.58756f },

    // WotLK Raids (Outside Entrances)
    { ACTION_TELEPORT_NAXXRAMAS,           "Naxxramas",                    571,  3668.72f, -1262.46f,  243.622f,  4.78500f },
    { ACTION_TELEPORT_OBSIDIAN_SANCTUM,    "The Obsidian Sanctum",         571,  3457.11f,  262.394f, -113.819f,  3.28258f },
    { ACTION_TELEPORT_EYE_OF_ETERNITY,     "The Eye of Eternity",          571,  3859.44f,  6989.85f,  152.041f,  5.79635f },
    { ACTION_TELEPORT_VAULT_OF_ARCHAVON,   "Vault of Archavon",            571,  5453.72f,  2840.79f,  421.280f,  0.00000f },
    { ACTION_TELEPORT_ULDUAR,              "Ulduar",                       571,  9327.25f, -1114.64f,  1245.15f,  0.00231f },
    { ACTION_TELEPORT_TRIAL_OF_CRUSADER,   "Trial of the Crusader",        571,  8515.68f,  716.982f,  558.248f,  1.57315f },
    { ACTION_TELEPORT_ICECROWN_CITADEL,    "Icecrown Citadel",             571,  5873.82f,  2110.98f,  636.011f,  3.55230f },
    { ACTION_TELEPORT_RUBY_SANCTUM,        "The Ruby Sanctum",             571,  3600.50f,  197.340f, -113.760f,  5.29905f }
};

static TeleportLocation const* FindTeleportLocation(uint32 action)
{
    for (auto const& loc : kTeleportLocations)
        if (loc.action == action)
            return &loc;
    return nullptr;
}

// ============================================================================
// AllCreatureScript: Follower Gossip Handling & Security
// ============================================================================
class UtilityFollowerCreatureScript : public AllCreatureScript
{
public:
    UtilityFollowerCreatureScript() : AllCreatureScript("UtilityFollowerCreatureScript") { }

    bool CanCreatureGossipHello(Player* player, Creature* creature) override
    {
        if (!sUtilityFollowerMgr->IsFollower(creature->GetGUID()))
            return false;

        // Security check: Only the owning player may interact with the follower
        if (sUtilityFollowerMgr->GetFollowerOwner(creature->GetGUID()) != player->GetGUID())
        {
            ChatHandler(player->GetSession()).SendSysMessage("This follower belongs to another adventurer.");
            CloseGossipMenuFor(player);
            return true;
        }

        FollowerType type = sUtilityFollowerMgr->GetFollowerType(creature->GetGUID());

        // If Medivh was temporarily switched to a class/profession trainer entry, restore his native entry and appearance
        if (type == FollowerType::Trainer && creature->GetEntry() != sUtilityFollowerConfig->TrainerCreatureEntry)
        {
            creature->UpdateEntry(sUtilityFollowerConfig->TrainerCreatureEntry, nullptr, false, false);
            creature->SetFaction(player->GetFaction());
            creature->SetDisplayId(sUtilityFollowerConfig->TrainerDisplayId);
            creature->SetNativeDisplayId(sUtilityFollowerConfig->TrainerDisplayId);
            creature->SetFloatValue(OBJECT_FIELD_SCALE_X, sUtilityFollowerConfig->TrainerScale);
            creature->SetUnitFlag(UnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED));
            if (auto ai = dynamic_cast<UtilityFollowerAI*>(creature->AI()))
                ai->RestoreAppearance();
        }

        // Always ensure NPC gossip and trainer flags are active
        creature->ReplaceAllNpcFlags(UNIT_NPC_FLAG_GOSSIP | (type == FollowerType::Trainer ? UNIT_NPC_FLAG_TRAINER : UNIT_NPC_FLAG_NONE));
        ClearGossipMenuFor(player);

        switch (type)
        {
            case FollowerType::Banker:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Open Bank", SENDER_FOLLOWER_BANKER, ACTION_BANKER_OPEN);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Dismiss", SENDER_FOLLOWER_MAIN, ACTION_FOLLOWER_DISMISS);
                break;

            case FollowerType::Auctioneer:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Open Auction House", SENDER_FOLLOWER_AUCTIONEER, ACTION_AUCTIONEER_OPEN);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Dismiss", SENDER_FOLLOWER_MAIN, ACTION_FOLLOWER_DISMISS);
                break;

            case FollowerType::Trainer:
                if (sUtilityFollowerConfig->TrainerTeleportEnable)
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Teleportation", SENDER_FOLLOWER_TELEPORT, ACTION_TRAINER_TELEPORT_MENU);

                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Class Training", SENDER_FOLLOWER_TRAINER, ACTION_TRAINER_CLASS);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Professions", SENDER_FOLLOWER_TRAINER, ACTION_TRAINER_PROF_MAIN);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Talent Services", SENDER_FOLLOWER_TRAINER, ACTION_TRAINER_TALENT_SERVICES);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Dual Specialization", SENDER_FOLLOWER_TRAINER, ACTION_TRAINER_DUAL_SPEC_MENU);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Dismiss", SENDER_FOLLOWER_MAIN, ACTION_FOLLOWER_DISMISS);
                break;

            default:
                break;
        }

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    static bool DoTeleport(Player* player, Creature* creature, uint32 mapId, float x, float y, float z, float o, std::string const& destName)
    {
        CloseGossipMenuFor(player);

        if (!sUtilityFollowerConfig->TrainerTeleportEnable)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Teleportation is currently disabled.");
            return true;
        }

        if (sUtilityFollowerConfig->TrainerTeleportCombatCheck && player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("You cannot teleport while in combat.");
            return true;
        }

        if (!player->IsAlive())
        {
            ChatHandler(player->GetSession()).SendSysMessage("You cannot teleport while dead.");
            return true;
        }

        if (player->GetLevel() < sUtilityFollowerConfig->TrainerTeleportMinLevel)
        {
            ChatHandler(player->GetSession()).SendSysMessage(
                Acore::StringFormat("You must be at least level {} to use teleportation.", sUtilityFollowerConfig->TrainerTeleportMinLevel));
            return true;
        }

        if (mapId == 571 && player->GetLevel() < sUtilityFollowerConfig->TrainerTeleportDalaranMinLevel)
        {
            ChatHandler(player->GetSession()).SendSysMessage(
                Acore::StringFormat("You must be at least level {} to teleport to Dalaran.", sUtilityFollowerConfig->TrainerTeleportDalaranMinLevel));
            return true;
        }

        if (sUtilityFollowerConfig->TrainerTeleportCost > 0)
        {
            if (!player->HasEnoughMoney(sUtilityFollowerConfig->TrainerTeleportCost))
            {
                player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
                return true;
            }
            player->ModifyMoney(-static_cast<int32>(sUtilityFollowerConfig->TrainerTeleportCost));
        }

        // Medivh casts arcane teleport visual on the player
        creature->CastSpell(player, 35517 /* SPELL_TELEPORT_VISUAL */, true);

        player->TeleportTo(mapId, x, y, z, o);
        ChatHandler(player->GetSession()).SendSysMessage(Acore::StringFormat("Medivh opens a portal to {}...", destName));
        return true;
    }

    bool CanCreatureGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!sUtilityFollowerMgr->IsFollower(creature->GetGUID()))
            return false;

        // Security check: Verify ownership server-side for every action packet
        if (sUtilityFollowerMgr->GetFollowerOwner(creature->GetGUID()) != player->GetGUID())
        {
            ChatHandler(player->GetSession()).SendSysMessage("This follower belongs to another adventurer.");
            CloseGossipMenuFor(player);
            return true;
        }

        if (TeleportLocation const* loc = FindTeleportLocation(action))
            return DoTeleport(player, creature, loc->mapId, loc->x, loc->y, loc->z, loc->o, loc->name);

        switch (action)
        {
            // General
            case ACTION_FOLLOWER_DISMISS:
            {
                FollowerType type = sUtilityFollowerMgr->GetFollowerType(creature->GetGUID());
                CloseGossipMenuFor(player);
                sUtilityFollowerMgr->DespawnFollower(player, type);
                return true;
            }

            case ACTION_FOLLOWER_BACK_MAIN:
                return CanCreatureGossipHello(player, creature);

            // Banker
            case ACTION_BANKER_OPEN:
                CloseGossipMenuFor(player);
                creature->SetNpcFlag(UNIT_NPC_FLAG_BANKER);
                player->GetSession()->SendShowBank(creature->GetGUID());
                return true;

            // Auctioneer
            case ACTION_AUCTIONEER_OPEN:
                CloseGossipMenuFor(player);
                creature->SetFaction(player->GetFaction());
                creature->SetNpcFlag(UNIT_NPC_FLAG_AUCTIONEER);
                player->GetSession()->SendAuctionHello(creature->GetGUID(), creature);
                return true;

            // Teleportation Main Menu
            case ACTION_TRAINER_TELEPORT_MENU:
            {
                ClearGossipMenuFor(player);
                if (sUtilityFollowerConfig->TrainerTeleportEnableDungeons)
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Dungeons", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_CAT_DUNGEONS);
                if (sUtilityFollowerConfig->TrainerTeleportEnableRaids)
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Raids", SENDER_FOLLOWER_TELEPORT_RAIDS, ACTION_TELEPORT_CAT_RAIDS);

                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Dalaran", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_DALARAN);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Shattrath City", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_SHATTRATH);

                if (player->GetTeamId() == TEAM_ALLIANCE)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Stormwind City", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_STORMWIND);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Ironforge", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_IRONFORGE);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Darnassus", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_DARNASSUS);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Exodar", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_EXODAR);
                }
                else
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Orgrimmar", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_ORGRIMMAR);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Undercity", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_UNDERCITY);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Thunder Bluff", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_THUNDER_BLUFF);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Silvermoon City", SENDER_FOLLOWER_TELEPORT, ACTION_TELEPORT_SILVERMOON);
                }

                if (sUtilityFollowerConfig->TrainerTeleportEnableExtraLocations)
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Other Destinations", SENDER_FOLLOWER_TELEPORT, ACTION_TRAINER_TELEPORT_EXTRA_MENU);

                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TRAINER, ACTION_FOLLOWER_BACK_MAIN);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Other Destinations (World Hubs & POIs)
            case ACTION_TRAINER_TELEPORT_EXTRA_MENU:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Karazhan", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_KARAZHAN);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Caverns of Time", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_CAVERNS_OF_TIME);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Gadgetzan", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_GADGETZAN);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Booty Bay", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BOOTY_BAY);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT, ACTION_TRAINER_TELEPORT_MENU);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Dungeons Category Submenu
            case ACTION_TELEPORT_CAT_DUNGEONS:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Classic Dungeons", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_CLASSIC_P1);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Burning Crusade Dungeons", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_TBC_P1);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Wrath of the Lich King Dungeons", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_WRATH_P1);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT, ACTION_TRAINER_TELEPORT_MENU);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Raids Category Submenu
            case ACTION_TELEPORT_CAT_RAIDS:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Classic Raids", SENDER_FOLLOWER_TELEPORT_RAIDS, ACTION_TELEPORT_RAIDS_CLASSIC);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Burning Crusade Raids", SENDER_FOLLOWER_TELEPORT_RAIDS, ACTION_TELEPORT_RAIDS_TBC);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Wrath of the Lich King Raids", SENDER_FOLLOWER_TELEPORT_RAIDS, ACTION_TELEPORT_RAIDS_WRATH);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT, ACTION_TRAINER_TELEPORT_MENU);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // World Hubs Submenu
            case ACTION_TELEPORT_CAT_HUBS:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Karazhan", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_KARAZHAN);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Caverns of Time", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_CAVERNS_OF_TIME);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Gadgetzan", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_GADGETZAN);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Booty Bay", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BOOTY_BAY);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT, ACTION_TRAINER_TELEPORT_MENU);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Classic Dungeons - Page 1
            case ACTION_TELEPORT_DUNGEONS_CLASSIC_P1:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Ragefire Chasm (13-18)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_RFC);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Deadmines (15-23)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_VC);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Wailing Caverns (15-25)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_WC);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Shadowfang Keep (18-25)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SFK);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Blackfathom Deeps (20-30)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BFD);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Stockade (22-30)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_STOCKADE);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Gnomeregan (24-34)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_GNOMER);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Razorfen Kraul (25-35)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_RFK);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Scarlet Monastery (30-45)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SM);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Razorfen Downs (35-45)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_RFD);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Next Page ->", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_CLASSIC_P2);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_CAT_DUNGEONS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Classic Dungeons - Page 2
            case ACTION_TELEPORT_DUNGEONS_CLASSIC_P2:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Uldaman (35-45)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ULDA);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Zul'Farrak (42-46)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ZF);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Maraudon (40-52)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_MARA);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Sunken Temple (45-55)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ST);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Blackrock Depths (48-60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BRD);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Lower Blackrock Spire (52-60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_LBRS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Upper Blackrock Spire (55-60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_UBRS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Dire Maul (54-60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_DM);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Scholomance (56-60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SCHOLO);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Stratholme (56-60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_STRAT);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- Previous Page", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_CLASSIC_P1);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_CAT_DUNGEONS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // TBC Dungeons - Page 1
            case ACTION_TELEPORT_DUNGEONS_TBC_P1:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Hellfire Ramparts (60-62)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_RAMPARTS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Blood Furnace (61-63)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BLOOD_FURNACE);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Shattered Halls (68-70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SHATTERED_HALLS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Slave Pens (62-64)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SLAVE_PENS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Underbog (63-65)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_UNDERBOG);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Steamvault (68-70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_STEAMVAULT);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Mana-Tombs (64-66)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_MANA_TOMBS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Auchenai Crypts (65-67)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_AUCHENAI_CRYPTS);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Next Page ->", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_TBC_P2);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_CAT_DUNGEONS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // TBC Dungeons - Page 2
            case ACTION_TELEPORT_DUNGEONS_TBC_P2:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Sethekk Halls (67-69)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SETHEKK_HALLS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Shadow Labyrinth (68-70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SHADOW_LABYRINTH);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Old Hillsbrad Foothills (66-68)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_DURNHOLDE);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Black Morass (69-70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BLACK_MORASS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Botanica (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BOTANICA);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Mechanar (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_MECHANAR);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Arcatraz (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ARCATRAZ);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Magisters' Terrace (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_MAGISTERS_TERRACE);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- Previous Page", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_TBC_P1);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_CAT_DUNGEONS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Wrath Dungeons - Page 1
            case ACTION_TELEPORT_DUNGEONS_WRATH_P1:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Utgarde Keep (70-72)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_UTGARDE_KEEP);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Nexus (71-73)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_THE_NEXUS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Azjol-Nerub (72-74)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_AZJOL_NERUB);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Ahn'kahet: The Old Kingdom (73-75)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_AHN_KAHET);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Drak'Tharon Keep (74-76)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_DRAK_THARON);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Violet Hold (75-77)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_VIOLET_HOLD);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Gundrak (76-78)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_GUNDRAK);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Halls of Stone (77-79)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_HALLS_OF_STONE);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Next Page ->", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_WRATH_P2);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_CAT_DUNGEONS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Wrath Dungeons - Page 2
            case ACTION_TELEPORT_DUNGEONS_WRATH_P2:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Halls of Lightning (78-80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_HALLS_OF_LIGHTNING);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Oculus (79-80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_THE_OCULUS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Utgarde Pinnacle (79-80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_UTGARDE_PINNACLE);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Culling of Stratholme (79-80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_CULLING_STRATHOLME);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Trial of the Champion (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_TRIAL_OF_CHAMPION);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Forge of Souls (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_FORGE_OF_SOULS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Pit of Saron (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_PIT_OF_SARON);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Halls of Reflection (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_HALLS_OF_REFLECTION);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- Previous Page", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_DUNGEONS_WRATH_P1);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_DUNGEONS, ACTION_TELEPORT_CAT_DUNGEONS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Classic Raids
            case ACTION_TELEPORT_RAIDS_CLASSIC:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Molten Core (60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_MC);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Blackwing Lair (60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BWL);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Ruins of Ahn'Qiraj - AQ20 (60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_AQ20);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Temple of Ahn'Qiraj - AQ40 (60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_AQ40);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Onyxia's Lair (60/80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ONYXIA);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Zul'Gurub (60)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ZG);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_RAIDS, ACTION_TELEPORT_CAT_RAIDS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // TBC Raids
            case ACTION_TELEPORT_RAIDS_TBC:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Karazhan (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_KARA_RAID);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Gruul's Lair (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_GRUUL);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Magtheridon's Lair (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_MAGTHERIDON);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Serpentshrine Cavern (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SSC);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Eye - Tempest Keep (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_THE_EYE);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Battle for Mount Hyjal (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_HYJAL);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Black Temple (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_BLACK_TEMPLE);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Sunwell Plateau (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_SUNWELL);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Zul'Aman (70)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ZULAMAN);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_RAIDS, ACTION_TELEPORT_CAT_RAIDS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Wrath Raids
            case ACTION_TELEPORT_RAIDS_WRATH:
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Naxxramas (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_NAXXRAMAS);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Obsidian Sanctum (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_OBSIDIAN_SANCTUM);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Eye of Eternity (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_EYE_OF_ETERNITY);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Vault of Archavon (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_VAULT_OF_ARCHAVON);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Ulduar (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ULDUAR);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Trial of the Crusader (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_TRIAL_OF_CRUSADER);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Icecrown Citadel (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_ICECROWN_CITADEL);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "The Ruby Sanctum (80)", SENDER_FOLLOWER_TELEPORT_DEST, ACTION_TELEPORT_RUBY_SANCTUM);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TELEPORT_RAIDS, ACTION_TELEPORT_CAT_RAIDS);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Class Training (Direct Native Trainer Window)
            case ACTION_TRAINER_CLASS:
            {
                uint32 trainerEntry = 0;
                bool isAlliance = (player->GetTeamId() == TEAM_ALLIANCE);

                switch (player->getClass())
                {
                    case CLASS_WARRIOR:      trainerEntry = isAlliance ? TRAINER_ALLIANCE_WARRIOR : TRAINER_HORDE_WARRIOR; break;
                    case CLASS_PALADIN:      trainerEntry = isAlliance ? TRAINER_ALLIANCE_PALADIN : TRAINER_HORDE_PALADIN; break;
                    case CLASS_HUNTER:       trainerEntry = isAlliance ? TRAINER_ALLIANCE_HUNTER  : TRAINER_HORDE_HUNTER; break;
                    case CLASS_ROGUE:        trainerEntry = isAlliance ? TRAINER_ALLIANCE_ROGUE   : TRAINER_HORDE_ROGUE; break;
                    case CLASS_PRIEST:       trainerEntry = isAlliance ? TRAINER_ALLIANCE_PRIEST  : TRAINER_HORDE_PRIEST; break;
                    case CLASS_DEATH_KNIGHT: trainerEntry = isAlliance ? TRAINER_ALLIANCE_DK      : TRAINER_HORDE_DK; break;
                    case CLASS_SHAMAN:       trainerEntry = isAlliance ? TRAINER_ALLIANCE_SHAMAN  : TRAINER_HORDE_SHAMAN; break;
                    case CLASS_MAGE:         trainerEntry = isAlliance ? TRAINER_ALLIANCE_MAGE    : TRAINER_HORDE_MAGE; break;
                    case CLASS_WARLOCK:      trainerEntry = isAlliance ? TRAINER_ALLIANCE_WARLOCK : TRAINER_HORDE_WARLOCK; break;
                    case CLASS_DRUID:        trainerEntry = isAlliance ? TRAINER_ALLIANCE_DRUID   : TRAINER_HORDE_DRUID; break;
                    default: break;
                }

                if (trainerEntry)
                {
                    creature->UpdateEntry(trainerEntry, nullptr, false, false);
                    creature->SetFaction(player->GetFaction());
                    creature->SetDisplayId(sUtilityFollowerConfig->TrainerDisplayId);
                    creature->SetNativeDisplayId(sUtilityFollowerConfig->TrainerDisplayId);
                    creature->SetFloatValue(OBJECT_FIELD_SCALE_X, sUtilityFollowerConfig->TrainerScale);
                    creature->SetUnitFlag(UnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED));
                    creature->SetNpcFlag(UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_GOSSIP);
                    creature->PauseMovementForInteraction();

                    player->GetSession()->SendTrainerList(creature);
                }
                return true;
            }

            // Profession Main Menu
            case ACTION_TRAINER_PROF_MAIN:
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Primary Professions", SENDER_FOLLOWER_TRAINER, ACTION_TRAINER_PROF_PRIMARY_MENU);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Secondary Professions", SENDER_FOLLOWER_TRAINER, ACTION_TRAINER_PROF_SECONDARY_MENU);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TRAINER, ACTION_FOLLOWER_BACK_MAIN);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;

            // Primary Professions Menu
            case ACTION_TRAINER_PROF_PRIMARY_MENU:
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Alchemy", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_ALCHEMY);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Blacksmithing", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_BLACKSMITHING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Enchanting", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_ENCHANTING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Engineering", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_ENGINEERING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Herbalism", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_HERBALISM);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Inscription", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_INSCRIPTION);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Jewelcrafting", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_JEWELCRAFTING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Leatherworking", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_LEATHERWORKING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Mining", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_MINING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Skinning", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_SKINNING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Tailoring", SENDER_FOLLOWER_PROF_PRIMARY, ACTION_PROF_TAILORING);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TRAINER, ACTION_TRAINER_PROF_MAIN);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;

            // Secondary Professions Menu
            case ACTION_TRAINER_PROF_SECONDARY_MENU:
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Cooking", SENDER_FOLLOWER_PROF_SECOND, ACTION_PROF_COOKING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "First Aid", SENDER_FOLLOWER_PROF_SECOND, ACTION_PROF_FIRST_AID);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Fishing", SENDER_FOLLOWER_PROF_SECOND, ACTION_PROF_FISHING);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TRAINER, ACTION_TRAINER_PROF_MAIN);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;

            // Profession Trainers (Dalaran Neutral Grand Masters with guaranteed non-hostility)
            case ACTION_PROF_ALCHEMY:
            case ACTION_PROF_BLACKSMITHING:
            case ACTION_PROF_ENCHANTING:
            case ACTION_PROF_ENGINEERING:
            case ACTION_PROF_HERBALISM:
            case ACTION_PROF_INSCRIPTION:
            case ACTION_PROF_JEWELCRAFTING:
            case ACTION_PROF_LEATHERWORKING:
            case ACTION_PROF_MINING:
            case ACTION_PROF_SKINNING:
            case ACTION_PROF_TAILORING:
            case ACTION_PROF_COOKING:
            case ACTION_PROF_FIRST_AID:
            case ACTION_PROF_FISHING:
            {
                uint32 profEntry = 0;
                switch (action)
                {
                    case ACTION_PROF_ALCHEMY:        profEntry = TRAINER_ALCHEMY; break;
                    case ACTION_PROF_BLACKSMITHING:  profEntry = TRAINER_BLACKSMITHING; break;
                    case ACTION_PROF_ENCHANTING:     profEntry = TRAINER_ENCHANTING; break;
                    case ACTION_PROF_ENGINEERING:    profEntry = TRAINER_ENGINEERING; break;
                    case ACTION_PROF_HERBALISM:      profEntry = TRAINER_HERBALISM; break;
                    case ACTION_PROF_INSCRIPTION:    profEntry = TRAINER_INSCRIPTION; break;
                    case ACTION_PROF_JEWELCRAFTING:  profEntry = TRAINER_JEWELCRAFTING; break;
                    case ACTION_PROF_LEATHERWORKING: profEntry = TRAINER_LEATHERWORKING; break;
                    case ACTION_PROF_MINING:         profEntry = TRAINER_MINING; break;
                    case ACTION_PROF_SKINNING:       profEntry = TRAINER_SKINNING; break;
                    case ACTION_PROF_TAILORING:      profEntry = TRAINER_TAILORING; break;
                    case ACTION_PROF_COOKING:        profEntry = TRAINER_COOKING; break;
                    case ACTION_PROF_FIRST_AID:      profEntry = TRAINER_FIRST_AID; break;
                    case ACTION_PROF_FISHING:        profEntry = TRAINER_FISHING; break;
                    default: break;
                }

                if (profEntry)
                {
                    creature->UpdateEntry(profEntry, nullptr, false, false);
                    creature->SetFaction(player->GetFaction());
                    creature->SetDisplayId(sUtilityFollowerConfig->TrainerDisplayId);
                    creature->SetNativeDisplayId(sUtilityFollowerConfig->TrainerDisplayId);
                    creature->SetFloatValue(OBJECT_FIELD_SCALE_X, sUtilityFollowerConfig->TrainerScale);
                    creature->SetUnitFlag(UnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED));
                    creature->SetNpcFlag(UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_GOSSIP);
                    creature->PauseMovementForInteraction();

                    player->GetSession()->SendTrainerList(creature);
                }
                return true;
            }

            // Talent Services Menu
            case ACTION_TRAINER_TALENT_SERVICES:
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Reset Class Talents", SENDER_FOLLOWER_TALENTS, ACTION_TALENT_RESET_CONFIRM);
                if (player->GetPet() && player->GetPet()->getPetType() == HUNTER_PET)
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Reset Pet Talents", SENDER_FOLLOWER_TALENTS, ACTION_TALENT_RESET_PET);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TRAINER, ACTION_FOLLOWER_BACK_MAIN);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;

            // Talent Reset Confirmation
            case ACTION_TALENT_RESET_CONFIRM:
            {
                ClearGossipMenuFor(player);
                uint32 cost = sUtilityFollowerConfig->TrainerFreeTalentReset ? 0 : player->resetTalentsCost();
                std::string msg = Acore::StringFormat("Reset talents now? Cost: {} gold.", cost / GOLD);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, msg, SENDER_FOLLOWER_TALENTS, ACTION_TALENT_RESET_EXECUTE);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Cancel", SENDER_FOLLOWER_TALENTS, ACTION_TRAINER_TALENT_SERVICES);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;
            }

            // Execute Talent Reset
            case ACTION_TALENT_RESET_EXECUTE:
            {
                CloseGossipMenuFor(player);
                if (player->GetLevel() < 10)
                {
                    ChatHandler(player->GetSession()).SendSysMessage("You must be at least level 10 to reset talents.");
                    return true;
                }

                bool free = sUtilityFollowerConfig->TrainerFreeTalentReset;
                if (!free && !sWorld->getBoolConfig(CONFIG_NO_RESET_TALENT_COST))
                {
                    uint32 cost = player->resetTalentsCost();
                    if (!player->HasEnoughMoney(cost))
                    {
                        player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
                        return true;
                    }
                }

                if (player->resetTalents(free))
                {
                    player->SendTalentsInfoData(false);
                    creature->CastSpell(player, 14867, true); // Untalent Visual Effect
                    ChatHandler(player->GetSession()).SendSysMessage("Your talents have been reset.");
                }
                else
                    ChatHandler(player->GetSession()).SendSysMessage("You have no talent points to reset.");

                return true;
            }

            // Pet Talent Reset
            case ACTION_TALENT_RESET_PET:
                CloseGossipMenuFor(player);
                if (player->GetPet())
                {
                    player->ResetPetTalents();
                    ChatHandler(player->GetSession()).SendSysMessage("Your pet's talents have been reset.");
                }
                return true;

            // Dual Specialization Menu
            case ACTION_TRAINER_DUAL_SPEC_MENU:
                ClearGossipMenuFor(player);
                if (player->GetSpecsCount() > 1)
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "You already possess Dual Specialization.", SENDER_FOLLOWER_TRAINER, ACTION_FOLLOWER_BACK_MAIN);
                else if (player->GetLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL))
                {
                    std::string msg = Acore::StringFormat("You must be at least level {} to learn Dual Specialization.",
                        sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL));
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, msg, SENDER_FOLLOWER_TRAINER, ACTION_FOLLOWER_BACK_MAIN);
                }
                else
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Purchase Dual Specialization (Cost: 1,000 gold)", SENDER_FOLLOWER_DUAL_SPEC, ACTION_DUAL_SPEC_EXECUTE);

                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "< Back", SENDER_FOLLOWER_TRAINER, ACTION_FOLLOWER_BACK_MAIN);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                return true;

            // Execute Dual Specialization
            case ACTION_DUAL_SPEC_EXECUTE:
            {
                CloseGossipMenuFor(player);
                if (player->GetSpecsCount() > 1 || player->GetLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL))
                    return true;

                uint32 const cost = 1000 * GOLD;
                if (!player->HasEnoughMoney(cost))
                {
                    player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
                    return true;
                }

                player->ModifyMoney(-static_cast<int32>(cost));
                player->CastSpell(player, 63680, true, nullptr, nullptr, player->GetGUID());
                player->CastSpell(player, 63624, true, nullptr, nullptr, player->GetGUID());
                ChatHandler(player->GetSession()).SendSysMessage("Congratulations! You have learned Dual Specialization.");
                return true;
            }

            default:
                break;
        }

        return false;
    }
};

// ============================================================================
// PlayerScript: Lifecycle, Spells, and Map Transitions
// ============================================================================
class UtilityFollowerPlayerScript : public PlayerScript
{
public:
    UtilityFollowerPlayerScript() : PlayerScript("UtilityFollowerPlayerScript") { }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!sUtilityFollowerConfig->Enable || !player || !spell || !UtilityFollowerMgr::IsRealPlayer(player))
            return;

        uint32 spellId = spell->GetSpellInfo()->Id;

        if (sUtilityFollowerConfig->BankerSpellEnable && spellId == sUtilityFollowerConfig->BankerSpellId)
        {
            sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Banker);
            player->RemoveAura(spellId);
        }
        else if (sUtilityFollowerConfig->AuctioneerSpellEnable && spellId == sUtilityFollowerConfig->AuctioneerSpellId)
        {
            sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Auctioneer);
            player->RemoveAura(spellId);
        }
        else if (sUtilityFollowerConfig->TrainerSpellEnable && spellId == sUtilityFollowerConfig->TrainerSpellId)
        {
            sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Trainer);
            player->RemoveAura(spellId);
        }
    }

    void OnPlayerLogin(Player* player) override
    {
        sUtilityFollowerMgr->SyncSpellsOnLogin(player);
    }

    void OnPlayerLevelChanged(Player* player, uint8 oldLevel) override
    {
        sUtilityFollowerMgr->CheckSpellsOnLevelChange(player, oldLevel);
    }

    void OnPlayerLogout(Player* player) override
    {
        sUtilityFollowerMgr->DespawnAllFollowers(player);
    }

    void OnPlayerMapChanged(Player* player) override
    {
        sUtilityFollowerMgr->HandleMapChange(player);
    }
};

// ============================================================================
// CommandScript: Optional Player Chat Commands (.utility)
// ============================================================================
class UtilityFollowerCommandScript : public CommandScript
{
public:
    UtilityFollowerCommandScript() : CommandScript("UtilityFollowerCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable utilityTable =
        {
            { "banker",     HandleSummonBankerCommand,     SEC_PLAYER, Console::No },
            { "auctioneer", HandleSummonAuctioneerCommand, SEC_PLAYER, Console::No },
            { "medivh",     HandleSummonTrainerCommand,    SEC_PLAYER, Console::No },
            { "dismiss",    HandleDismissCommand,          SEC_PLAYER, Console::No },
            { "",           HandleHelpCommand,             SEC_PLAYER, Console::No }
        };

        static ChatCommandTable commandTable =
        {
            { "utility", utilityTable }
        };

        return commandTable;
    }

    static bool HandleSummonBankerCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player || !UtilityFollowerMgr::IsRealPlayer(player))
            return false;

        sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Banker);
        return true;
    }

    static bool HandleSummonAuctioneerCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player || !UtilityFollowerMgr::IsRealPlayer(player))
            return false;

        sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Auctioneer);
        return true;
    }

    static bool HandleSummonTrainerCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player || !UtilityFollowerMgr::IsRealPlayer(player))
            return false;

        sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Trainer);
        return true;
    }

    static bool HandleDismissCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player || !UtilityFollowerMgr::IsRealPlayer(player))
            return false;

        sUtilityFollowerMgr->DespawnAllFollowers(player);
        handler->SendSysMessage("All utility followers dismissed.");
        return true;
    }

    static bool HandleHelpCommand(ChatHandler* handler)
    {
        handler->SendSysMessage("Utility Follower Commands:");
        handler->SendSysMessage("  .utility banker     - Toggle Banker follower");
        handler->SendSysMessage("  .utility auctioneer - Toggle Auctioneer follower");
        handler->SendSysMessage("  .utility medivh     - Toggle Medivh follower (Trainer & Teleporter)");
        handler->SendSysMessage("  .utility dismiss    - Dismiss all active followers");
        return true;
    }
};

// ============================================================================
// ServerScript: Prevent Auto-Placement of Follower Spells on Action Bar
// ============================================================================
class UtilityFollowerServerScript : public ServerScript
{
public:
    UtilityFollowerServerScript() : ServerScript("UtilityFollowerServerScript") { }

    bool CanPacketReceive(WorldSession* session, WorldPacket const& packet) override
    {
        if (!sUtilityFollowerConfig->Enable || !sUtilityFollowerConfig->PreventActionBarAutoAdd || !session)
            return true;

        if (packet.GetOpcode() == CMSG_SET_ACTION_BUTTON)
        {
            WorldPacket copy = packet;
            uint8 button;
            uint32 packetData;
            copy >> button >> packetData;

            uint32 action = ACTION_BUTTON_ACTION(packetData);
            uint8 type = ACTION_BUTTON_TYPE(packetData);

            if (type == ACTION_BUTTON_SPELL && sUtilityFollowerMgr->IsFollowerSpell(action))
            {
                if (Player* player = session->GetPlayer())
                {
                    if (sUtilityFollowerMgr->IsAutoLearningSpell(player->GetGUID(), action))
                    {
                        sUtilityFollowerMgr->ClearAutoLearningSpell(player->GetGUID(), action);
                        player->SendActionButtons(1);
                        return false;
                    }
                }
            }
        }

        return true;
    }
};

// ============================================================================
// WorldScript: Configuration Reload Hook
// ============================================================================
class UtilityFollowerWorldScript : public WorldScript
{
public:
    UtilityFollowerWorldScript() : WorldScript("UtilityFollowerWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sUtilityFollowerConfig->Load();
        sUtilityFollowerMgr->ApplySpellCorrections();
    }
};

// Script registration entry point
void AddUtilityFollowerScripts()
{
    new UtilityFollowerCreatureScript();
    new UtilityFollowerPlayerScript();
    new UtilityFollowerCommandScript();
    new UtilityFollowerWorldScript();
    new UtilityFollowerServerScript();
}
