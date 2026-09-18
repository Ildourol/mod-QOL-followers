/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#include "UtilityFollowerMgr.h"
#include "Chat.h"
#include "Creature.h"
#include "GameTime.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "UtilityFollowerAI.h"
#include "UtilityFollowerConfig.h"
#include "WorldSession.h"

UtilityFollowerMgr* UtilityFollowerMgr::instance()
{
    static UtilityFollowerMgr instance;
    return &instance;
}

bool UtilityFollowerMgr::IsFollower(ObjectGuid const& creatureGuid) const
{
    std::lock_guard<std::mutex> lock(_lock);
    return _followerToOwner.find(creatureGuid) != _followerToOwner.end();
}

ObjectGuid UtilityFollowerMgr::GetFollowerOwner(ObjectGuid const& creatureGuid) const
{
    std::lock_guard<std::mutex> lock(_lock);
    auto itr = _followerToOwner.find(creatureGuid);
    if (itr != _followerToOwner.end())
        return itr->second.first;

    return ObjectGuid::Empty;
}

FollowerType UtilityFollowerMgr::GetFollowerType(ObjectGuid const& creatureGuid) const
{
    std::lock_guard<std::mutex> lock(_lock);
    auto itr = _followerToOwner.find(creatureGuid);
    if (itr != _followerToOwner.end())
        return itr->second.second;

    return FollowerType::Banker;
}

bool UtilityFollowerMgr::IsFollowerActive(Player const* player, FollowerType type) const
{
    std::lock_guard<std::mutex> lock(_lock);
    auto itr = _playerFollowers.find(player->GetGUID());
    if (itr == _playerFollowers.end())
        return false;

    for (auto const& record : itr->second)
        if (record.type == type)
            return true;

    return false;
}

Creature* UtilityFollowerMgr::GetFollower(Player const* player, FollowerType type) const
{
    ObjectGuid creatureGuid;
    {
        std::lock_guard<std::mutex> lock(_lock);
        auto itr = _playerFollowers.find(player->GetGUID());
        if (itr == _playerFollowers.end())
            return nullptr;

        for (auto const& record : itr->second)
        {
            if (record.type == type)
            {
                creatureGuid = record.creatureGuid;
                break;
            }
        }
    }

    if (!creatureGuid)
        return nullptr;

    return player->GetMap()->GetCreature(creatureGuid);
}

uint32 UtilityFollowerMgr::GetActiveFollowerCount(Player const* player) const
{
    std::lock_guard<std::mutex> lock(_lock);
    auto itr = _playerFollowers.find(player->GetGUID());
    if (itr == _playerFollowers.end())
        return 0;

    return static_cast<uint32>(itr->second.size());
}

bool UtilityFollowerMgr::IsMapAllowed(Map const* map) const
{
    if (!map)
        return false;

    if (map->IsBattleArena())
        return sUtilityFollowerConfig->AllowInArenas;

    if (map->IsBattleground())
        return sUtilityFollowerConfig->AllowInBattlegrounds;

    if (map->IsRaid())
        return sUtilityFollowerConfig->AllowInRaids;

    if (map->IsNonRaidDungeon())
        return sUtilityFollowerConfig->AllowInDungeons;

    if (map->IsWorldMap())
        return sUtilityFollowerConfig->AllowInWorld;

    return true;
}

bool UtilityFollowerMgr::CanSummon(Player const* player, FollowerType type, std::string& reason) const
{
    if (!player || !IsRealPlayer(player))
    {
        reason = "Only real players may summon utility followers.";
        return false;
    }

    if (!sUtilityFollowerConfig->Enable)
    {
        reason = "Utility followers module is disabled.";
        return false;
    }

    switch (type)
    {
        case FollowerType::Banker:
            if (!sUtilityFollowerConfig->BankerEnable)
            {
                reason = "Banker follower is currently disabled.";
                return false;
            }
            break;
        case FollowerType::Auctioneer:
            if (!sUtilityFollowerConfig->AuctioneerEnable)
            {
                reason = "Auctioneer follower is currently disabled.";
                return false;
            }
            break;
        case FollowerType::Trainer:
            if (!sUtilityFollowerConfig->TrainerEnable)
            {
                reason = "Trainer Book follower is currently disabled.";
                return false;
            }
            break;
        default:
            return false;
    }

    if (!player->IsAlive())
    {
        reason = "You cannot summon a follower while dead.";
        return false;
    }

    if (!IsMapAllowed(player->GetMap()))
    {
        reason = "Utility followers are not permitted in this area.";
        return false;
    }

    return true;
}

bool UtilityFollowerMgr::SummonFollower(Player* player, FollowerType type)
{
    if (!player || !IsRealPlayer(player))
        return false;

    std::string reason;
    if (!CanSummon(player, type, reason))
    {
        if (player->GetSession())
            ChatHandler(player->GetSession()).SendSysMessage(reason.c_str());
        return false;
    }

    // Check capacity and policy
    uint32 activeCount = GetActiveFollowerCount(player);
    if (activeCount >= sUtilityFollowerConfig->MaxActive)
    {
        if (sUtilityFollowerConfig->ActivePolicy == MaxActivePolicy::ReplaceOldest)
        {
            // Find oldest follower and despawn
            FollowerType oldestType = FollowerType::Banker;
            uint32 oldestTime = 0xFFFFFFFF;
            {
                std::lock_guard<std::mutex> lock(_lock);
                auto const& list = _playerFollowers[player->GetGUID()];
                for (auto const& rec : list)
                {
                    if (rec.spawnTime < oldestTime)
                    {
                        oldestTime = rec.spawnTime;
                        oldestType = rec.type;
                    }
                }
            }
            DespawnFollower(player, oldestType);
        }
        else
        {
            ChatHandler(player->GetSession()).SendSysMessage("Cannot summon follower: maximum active follower limit reached.");
            return false;
        }
    }

    // Determine appearance settings
    uint32 creatureEntry = 0;
    uint32 displayId = 0;
    float scale = 1.0f;
    uint32 npcFlags = UNIT_NPC_FLAG_GOSSIP;

    switch (type)
    {
        case FollowerType::Banker:
        {
            bool isAlliance = (player->GetTeamId() == TEAM_ALLIANCE);
            creatureEntry = isAlliance ? sUtilityFollowerConfig->BankerCreatureEntryAlliance : sUtilityFollowerConfig->BankerCreatureEntryHorde;
            displayId = isAlliance ? sUtilityFollowerConfig->BankerAllianceDisplayId : sUtilityFollowerConfig->BankerHordeDisplayId;
            scale = sUtilityFollowerConfig->BankerScale;
            npcFlags |= UNIT_NPC_FLAG_BANKER;
            break;
        }
        case FollowerType::Auctioneer:
        {
            creatureEntry = sUtilityFollowerConfig->AuctioneerCreatureEntry;
            displayId = sUtilityFollowerConfig->AuctioneerDisplayId;
            scale = sUtilityFollowerConfig->AuctioneerScale;
            npcFlags |= UNIT_NPC_FLAG_AUCTIONEER;
            break;
        }
        case FollowerType::Trainer:
        {
            creatureEntry = sUtilityFollowerConfig->TrainerCreatureEntry;
            displayId = sUtilityFollowerConfig->TrainerDisplayId;
            scale = sUtilityFollowerConfig->TrainerScale;
            npcFlags |= UNIT_NPC_FLAG_TRAINER;
            break;
        }
        default:
            return false;
    }

    // Summon temporary runtime creature
    Position const pos = player->GetPosition();
    TempSummon* summon = player->SummonCreature(creatureEntry, pos, TEMPSUMMON_MANUAL_DESPAWN);
    if (!summon)
    {
        LOG_WARN("module.qol_followers", "Failed to summon utility follower template entry {} for player {}", creatureEntry, player->GetName());
        return false;
    }

    // Configure follower attributes and appearance
    summon->SetFaction(player->GetFaction());
    summon->SetDisplayId(displayId);
    summon->SetNativeDisplayId(displayId);
    summon->SetFloatValue(OBJECT_FIELD_SCALE_X, scale);

    // Make strictly non-combat utility companion
    summon->SetUnitFlag(UnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED));
    summon->ReplaceAllNpcFlags(NPCFlags(npcFlags));

    // Attach custom follower AI
    summon->SetAI(new UtilityFollowerAI(summon, player->GetGUID(), type, displayId, scale));

    // Register active follower
    {
        std::lock_guard<std::mutex> lock(_lock);
        FollowerRecord record;
        record.type = type;
        record.creatureGuid = summon->GetGUID();
        record.spawnTime = static_cast<uint32>(GameTime::GetGameTimeMS().count());

        _playerFollowers[player->GetGUID()].push_back(record);
        _followerToOwner[summon->GetGUID()] = std::make_pair(player->GetGUID(), type);
    }

    // Initialize orientation towards owner
    summon->SetFacingToObject(player);

    return true;
}

void UtilityFollowerMgr::DespawnFollower(Player* player, FollowerType type)
{
    ObjectGuid creatureGuid;
    {
        std::lock_guard<std::mutex> lock(_lock);
        auto playerItr = _playerFollowers.find(player->GetGUID());
        if (playerItr == _playerFollowers.end())
            return;

        auto& list = playerItr->second;
        for (auto itr = list.begin(); itr != list.end(); ++itr)
        {
            if (itr->type == type)
            {
                creatureGuid = itr->creatureGuid;
                list.erase(itr);
                break;
            }
        }

        if (list.empty())
            _playerFollowers.erase(playerItr);

        if (creatureGuid)
            _followerToOwner.erase(creatureGuid);
    }

    if (creatureGuid)
    {
        if (Creature* creature = player->GetMap()->GetCreature(creatureGuid))
            creature->DespawnOrUnsummon();
    }
}

void UtilityFollowerMgr::DespawnAllFollowers(Player* player)
{
    std::vector<ObjectGuid> toDespawn;
    {
        std::lock_guard<std::mutex> lock(_lock);
        auto playerItr = _playerFollowers.find(player->GetGUID());
        if (playerItr == _playerFollowers.end())
            return;

        for (auto const& record : playerItr->second)
        {
            toDespawn.push_back(record.creatureGuid);
            _followerToOwner.erase(record.creatureGuid);
        }
        _playerFollowers.erase(playerItr);
    }

    for (auto const& guid : toDespawn)
    {
        if (Creature* creature = player->GetMap()->GetCreature(guid))
            creature->DespawnOrUnsummon();
    }
}

void UtilityFollowerMgr::HandleSpellSummon(Player* player, FollowerType type)
{
    // Toggle behavior: present -> dismiss, absent -> summon
    if (IsFollowerActive(player, type))
        DespawnFollower(player, type);
    else
        SummonFollower(player, type);
}

void UtilityFollowerMgr::TeachFollowerSpell(Player* player, uint32 spellId)
{
    if (!player || !spellId || !IsRealPlayer(player) || player->HasSpell(spellId))
        return;

    if (sUtilityFollowerConfig->PreventActionBarAutoAdd)
        MarkAutoLearningSpell(player->GetGUID(), spellId);

    player->learnSpell(spellId, false);

    if (sUtilityFollowerConfig->PreventActionBarAutoAdd)
    {
        // Immediate safeguard: clear any action button automatically populated by client/server
        bool changed = false;
        for (uint8 b = 0; b < MAX_ACTION_BUTTONS; ++b)
        {
            if (ActionButton const* ab = player->GetActionButton(b))
            {
                if (ab->GetAction() == spellId && ab->GetType() == ACTION_BUTTON_SPELL)
                {
                    player->removeActionButton(b);
                    changed = true;
                }
            }
        }
        if (changed)
            player->SendActionButtons(1);
    }
}

void UtilityFollowerMgr::SyncSpellsOnLogin(Player* player)
{
    if (!sUtilityFollowerConfig->Enable || !sUtilityFollowerConfig->SpellsSyncOnLogin || !IsRealPlayer(player))
        return;

    uint8 level = player->GetLevel();

    // Banker
    if (sUtilityFollowerConfig->BankerEnable &&
        sUtilityFollowerConfig->BankerSpellEnable &&
        sUtilityFollowerConfig->BankerSpellAutoLearn &&
        level >= sUtilityFollowerConfig->BankerSpellLearnLevel)
    {
        TeachFollowerSpell(player, sUtilityFollowerConfig->BankerSpellId);
    }

    // Auctioneer
    if (sUtilityFollowerConfig->AuctioneerEnable &&
        sUtilityFollowerConfig->AuctioneerSpellEnable &&
        sUtilityFollowerConfig->AuctioneerSpellAutoLearn &&
        level >= sUtilityFollowerConfig->AuctioneerSpellLearnLevel)
    {
        TeachFollowerSpell(player, sUtilityFollowerConfig->AuctioneerSpellId);
    }

    // Trainer
    if (sUtilityFollowerConfig->TrainerEnable &&
        sUtilityFollowerConfig->TrainerSpellEnable &&
        sUtilityFollowerConfig->TrainerSpellAutoLearn &&
        level >= sUtilityFollowerConfig->TrainerSpellLearnLevel)
    {
        TeachFollowerSpell(player, sUtilityFollowerConfig->TrainerSpellId);
    }

    // Auto-spawn restoration if enabled
    if (sUtilityFollowerConfig->AutoSpawn && IsMapAllowed(player->GetMap()))
    {
        if (sUtilityFollowerConfig->BankerEnable && !IsFollowerActive(player, FollowerType::Banker))
            SummonFollower(player, FollowerType::Banker);
        if (sUtilityFollowerConfig->AuctioneerEnable && !IsFollowerActive(player, FollowerType::Auctioneer))
            SummonFollower(player, FollowerType::Auctioneer);
        if (sUtilityFollowerConfig->TrainerEnable && !IsFollowerActive(player, FollowerType::Trainer))
            SummonFollower(player, FollowerType::Trainer);
    }
}

void UtilityFollowerMgr::CheckSpellsOnLevelChange(Player* player, uint8 oldLevel)
{
    if (!sUtilityFollowerConfig->Enable || !IsRealPlayer(player))
        return;

    uint8 newLevel = player->GetLevel();

    if (newLevel > oldLevel)
    {
        // Level-up: teach newly eligible spells once
        if (sUtilityFollowerConfig->BankerEnable &&
            sUtilityFollowerConfig->BankerSpellEnable &&
            sUtilityFollowerConfig->BankerSpellAutoLearn &&
            newLevel >= sUtilityFollowerConfig->BankerSpellLearnLevel &&
            oldLevel < sUtilityFollowerConfig->BankerSpellLearnLevel)
        {
            TeachFollowerSpell(player, sUtilityFollowerConfig->BankerSpellId);
        }

        if (sUtilityFollowerConfig->AuctioneerEnable &&
            sUtilityFollowerConfig->AuctioneerSpellEnable &&
            sUtilityFollowerConfig->AuctioneerSpellAutoLearn &&
            newLevel >= sUtilityFollowerConfig->AuctioneerSpellLearnLevel &&
            oldLevel < sUtilityFollowerConfig->AuctioneerSpellLearnLevel)
        {
            TeachFollowerSpell(player, sUtilityFollowerConfig->AuctioneerSpellId);
        }

        if (sUtilityFollowerConfig->TrainerEnable &&
            sUtilityFollowerConfig->TrainerSpellEnable &&
            sUtilityFollowerConfig->TrainerSpellAutoLearn &&
            newLevel >= sUtilityFollowerConfig->TrainerSpellLearnLevel &&
            oldLevel < sUtilityFollowerConfig->TrainerSpellLearnLevel)
        {
            TeachFollowerSpell(player, sUtilityFollowerConfig->TrainerSpellId);
        }
    }
    else if (newLevel < oldLevel && sUtilityFollowerConfig->SpellsRemoveIfBelowLevel)
    {
        // De-level: remove spells if configured
        if (newLevel < sUtilityFollowerConfig->BankerSpellLearnLevel && player->HasSpell(sUtilityFollowerConfig->BankerSpellId))
            player->removeSpell(sUtilityFollowerConfig->BankerSpellId, SPEC_MASK_ALL, false);

        if (newLevel < sUtilityFollowerConfig->AuctioneerSpellLearnLevel && player->HasSpell(sUtilityFollowerConfig->AuctioneerSpellId))
            player->removeSpell(sUtilityFollowerConfig->AuctioneerSpellId, SPEC_MASK_ALL, false);

        if (newLevel < sUtilityFollowerConfig->TrainerSpellLearnLevel && player->HasSpell(sUtilityFollowerConfig->TrainerSpellId))
            player->removeSpell(sUtilityFollowerConfig->TrainerSpellId, SPEC_MASK_ALL, false);
    }
}

void UtilityFollowerMgr::HandleMapChange(Player* player)
{
    std::vector<FollowerType> activeTypes;
    {
        std::lock_guard<std::mutex> lock(_lock);
        auto itr = _playerFollowers.find(player->GetGUID());
        if (itr != _playerFollowers.end())
        {
            for (auto const& rec : itr->second)
            {
                activeTypes.push_back(rec.type);
                _followerToOwner.erase(rec.creatureGuid);
            }
            _playerFollowers.erase(itr);
        }
    }

    // If new map is allowed, restore followers at player's location
    if (IsMapAllowed(player->GetMap()))
    {
        for (FollowerType type : activeTypes)
            SummonFollower(player, type);
    }
}

void UtilityFollowerMgr::UnregisterFollower(ObjectGuid const& creatureGuid)
{
    std::lock_guard<std::mutex> lock(_lock);
    auto ownerItr = _followerToOwner.find(creatureGuid);
    if (ownerItr == _followerToOwner.end())
        return;

    ObjectGuid ownerGuid = ownerItr->second.first;
    _followerToOwner.erase(ownerItr);

    auto playerItr = _playerFollowers.find(ownerGuid);
    if (playerItr != _playerFollowers.end())
    {
        auto& list = playerItr->second;
        for (auto itr = list.begin(); itr != list.end(); ++itr)
        {
            if (itr->creatureGuid == creatureGuid)
            {
                list.erase(itr);
                break;
            }
        }
        if (list.empty())
            _playerFollowers.erase(playerItr);
    }
}

bool UtilityFollowerMgr::IsFollowerSpell(uint32 spellId) const
{
    return spellId == sUtilityFollowerConfig->BankerSpellId ||
           spellId == sUtilityFollowerConfig->AuctioneerSpellId ||
           spellId == sUtilityFollowerConfig->TrainerSpellId;
}

bool UtilityFollowerMgr::IsAutoLearningSpell(ObjectGuid const& playerGuid, uint32 spellId) const
{
    std::lock_guard<std::mutex> lock(_lock);
    auto itr = _autoLearningSpells.find(playerGuid);
    if (itr != _autoLearningSpells.end())
        return itr->second.find(spellId) != itr->second.end();
    return false;
}

void UtilityFollowerMgr::MarkAutoLearningSpell(ObjectGuid const& playerGuid, uint32 spellId)
{
    std::lock_guard<std::mutex> lock(_lock);
    _autoLearningSpells[playerGuid].insert(spellId);
}

void UtilityFollowerMgr::ClearAutoLearningSpell(ObjectGuid const& playerGuid, uint32 spellId)
{
    std::lock_guard<std::mutex> lock(_lock);
    auto itr = _autoLearningSpells.find(playerGuid);
    if (itr != _autoLearningSpells.end())
    {
        itr->second.erase(spellId);
        if (itr->second.empty())
            _autoLearningSpells.erase(itr);
    }
}

void UtilityFollowerMgr::ApplySpellCorrections()
{
    std::vector<uint32> spellIds = {
        sUtilityFollowerConfig->TrainerSpellId,
        sUtilityFollowerConfig->AuctioneerSpellId,
        sUtilityFollowerConfig->BankerSpellId,
        67368, // Bank Errand (standard Blizzard WotLK spell)
        69046, // Pack Hobgoblin (standard Blizzard WotLK spell - Goblin icon)
        54614, // Steam-Powered Auctioneer (standard Blizzard WotLK spell)
        62978, // Summon Guardian (standard Blizzard WotLK spell)
        39339, // Hand of Medivh (standard Blizzard WotLK spell)
        62076, // Pack Mule (standard Blizzard WotLK spell)
        54270, // Argent Tome Book Spawn
        31114  // Medivh's Journal safeguard
    };

    for (uint32 id : spellIds)
    {
        if (!id)
            continue;

        if (SpellInfo* spellInfo = const_cast<SpellInfo*>(sSpellMgr->GetSpellInfo(id)))
        {
            spellInfo->RequiresSpellFocus = 0;
            spellInfo->AreaGroupId = 0;
            spellInfo->EquippedItemClass = -1;
            spellInfo->Effects[EFFECT_0].Effect = SPELL_EFFECT_DUMMY;
            spellInfo->Effects[EFFECT_0].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
            spellInfo->Effects[EFFECT_1].Effect = 0;
            spellInfo->Effects[EFFECT_2].Effect = 0;
        }
    }
}

bool UtilityFollowerMgr::IsRealPlayer(Player const* player)
{
    if (!player)
        return false;

    WorldSession const* session = player->GetSession();
    if (!session)
        return false;

    return !session->IsBot();
}

bool UtilityFollowerMgr::IsPlayerBot(Player const* player)
{
    return !IsRealPlayer(player);
}

