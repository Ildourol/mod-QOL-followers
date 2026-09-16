/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#include "AllCreatureScript.h"
#include "Chat.h"
#include "CommandScript.h"
#include "Creature.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptedGossip.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "StringFormat.h"
#include "UtilityFollowerAI.h"
#include "UtilityFollowerCommon.h"
#include "UtilityFollowerConfig.h"
#include "UtilityFollowerMgr.h"
#include "World.h"
#include "WorldConfig.h"
#include "WorldScript.h"

using namespace Acore::ChatCommands;

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

        ClearGossipMenuFor(player);

        FollowerType type = sUtilityFollowerMgr->GetFollowerType(creature->GetGUID());
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

            // Class Training
            case ACTION_TRAINER_CLASS:
            {
                CloseGossipMenuFor(player);
                uint32 trainerEntry = 0;
                switch (player->getClass())
                {
                    case CLASS_WARRIOR:      trainerEntry = TRAINER_WARRIOR; break;
                    case CLASS_PALADIN:      trainerEntry = TRAINER_PALADIN; break;
                    case CLASS_HUNTER:       trainerEntry = TRAINER_HUNTER; break;
                    case CLASS_ROGUE:        trainerEntry = TRAINER_ROGUE; break;
                    case CLASS_PRIEST:       trainerEntry = TRAINER_PRIEST; break;
                    case CLASS_DEATH_KNIGHT: trainerEntry = TRAINER_DEATH_KNIGHT; break;
                    case CLASS_SHAMAN:       trainerEntry = TRAINER_SHAMAN; break;
                    case CLASS_MAGE:         trainerEntry = TRAINER_MAGE; break;
                    case CLASS_WARLOCK:      trainerEntry = TRAINER_WARLOCK; break;
                    case CLASS_DRUID:        trainerEntry = TRAINER_DRUID; break;
                    default: break;
                }

                if (trainerEntry)
                {
                    creature->UpdateEntry(trainerEntry, nullptr, false, false);
                    if (auto ai = dynamic_cast<UtilityFollowerAI*>(creature->AI()))
                        ai->RestoreAppearance();

                    creature->SetNpcFlag(UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_GOSSIP);
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

            // Profession Trainers (Dalaran Neutral Grand Masters)
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
                CloseGossipMenuFor(player);
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
                    if (auto ai = dynamic_cast<UtilityFollowerAI*>(creature->AI()))
                        ai->RestoreAppearance();

                    creature->SetNpcFlag(UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_GOSSIP);
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
        if (!sUtilityFollowerConfig->Enable || !player || !spell)
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
            { "trainer",    HandleSummonTrainerCommand,    SEC_PLAYER, Console::No },
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
        if (!player)
            return false;

        sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Banker);
        return true;
    }

    static bool HandleSummonAuctioneerCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
            return false;

        sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Auctioneer);
        return true;
    }

    static bool HandleSummonTrainerCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
            return false;

        sUtilityFollowerMgr->HandleSpellSummon(player, FollowerType::Trainer);
        return true;
    }

    static bool HandleDismissCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
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
        handler->SendSysMessage("  .utility trainer    - Toggle Trainer Book follower");
        handler->SendSysMessage("  .utility dismiss    - Dismiss all active followers");
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
    }
};

// Script registration entry point
void AddUtilityFollowerScripts()
{
    new UtilityFollowerCreatureScript();
    new UtilityFollowerPlayerScript();
    new UtilityFollowerCommandScript();
    new UtilityFollowerWorldScript();
}
