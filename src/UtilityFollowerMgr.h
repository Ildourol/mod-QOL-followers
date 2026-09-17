/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#ifndef UTILITY_FOLLOWER_MGR_H
#define UTILITY_FOLLOWER_MGR_H

#include "UtilityFollowerCommon.h"
#include "ObjectGuid.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Creature;
class Map;
class Player;

struct FollowerRecord
{
    FollowerType type;
    ObjectGuid creatureGuid;
    uint32 spawnTime;
};

class UtilityFollowerMgr
{
public:
    static UtilityFollowerMgr* instance();

    bool IsFollower(ObjectGuid const& creatureGuid) const;
    ObjectGuid GetFollowerOwner(ObjectGuid const& creatureGuid) const;
    FollowerType GetFollowerType(ObjectGuid const& creatureGuid) const;

    bool IsFollowerActive(Player const* player, FollowerType type) const;
    Creature* GetFollower(Player const* player, FollowerType type) const;
    uint32 GetActiveFollowerCount(Player const* player) const;

    bool CanSummon(Player const* player, FollowerType type, std::string& reason) const;
    bool SummonFollower(Player* player, FollowerType type);
    void DespawnFollower(Player* player, FollowerType type);
    void DespawnAllFollowers(Player* player);

    void HandleSpellSummon(Player* player, FollowerType type);
    void SyncSpellsOnLogin(Player* player);
    void CheckSpellsOnLevelChange(Player* player, uint8 oldLevel);
    void TeachFollowerSpell(Player* player, uint32 spellId);
    void HandleMapChange(Player* player);

    bool IsFollowerSpell(uint32 spellId) const;
    bool IsAutoLearningSpell(ObjectGuid const& playerGuid, uint32 spellId) const;
    void MarkAutoLearningSpell(ObjectGuid const& playerGuid, uint32 spellId);
    void ClearAutoLearningSpell(ObjectGuid const& playerGuid, uint32 spellId);

    void UnregisterFollower(ObjectGuid const& creatureGuid);
    bool IsMapAllowed(Map const* map) const;
    void ApplySpellCorrections();

private:
    UtilityFollowerMgr() = default;

    mutable std::mutex _lock;
    std::unordered_map<ObjectGuid, std::vector<FollowerRecord>> _playerFollowers;
    std::unordered_map<ObjectGuid, std::pair<ObjectGuid, FollowerType>> _followerToOwner;
    std::unordered_map<ObjectGuid, std::unordered_set<uint32>> _autoLearningSpells;
};

#define sUtilityFollowerMgr UtilityFollowerMgr::instance()

#endif // UTILITY_FOLLOWER_MGR_H
