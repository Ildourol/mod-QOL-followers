/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#ifndef UTILITY_FOLLOWER_CONFIG_H
#define UTILITY_FOLLOWER_CONFIG_H

#include "UtilityFollowerCommon.h"
#include <string>

class UtilityFollowerConfig
{
public:
    static UtilityFollowerConfig* instance();

    void Load();

    // Master switch
    bool Enable{true};

    // Follower enables
    bool BankerEnable{true};
    bool AuctioneerEnable{true};
    bool TrainerEnable{true};

    // Movement
    float FollowDistance{3.0f};
    float StartFollowingDistance{5.0f};
    float CatchUpDistance{30.0f};
    uint32 FollowUpdateInterval{500};
    bool TeleportWhenStuck{true};

    // Active limit & policy
    uint32 MaxActive{3};
    MaxActivePolicy ActivePolicy{MaxActivePolicy::Reject};

    // Environment restrictions
    bool AllowInWorld{true};
    bool AllowInDungeons{true};
    bool AllowInRaids{true};
    bool AllowInBattlegrounds{false};
    bool AllowInArenas{false};

    // Auto spawn
    bool AutoSpawn{false};

    // Banker Appearance
    uint32 BankerCreatureEntryHorde{10636};
    uint32 BankerCreatureEntryAlliance{16225};
    uint32 BankerHordeDisplayId{7933};
    uint32 BankerAllianceDisplayId{14546};
    float BankerScale{1.0f};

    // Auctioneer Appearance
    uint32 AuctioneerCreatureEntry{8661};
    uint32 AuctioneerDisplayId{7993};
    float AuctioneerScale{1.0f};

    // Trainer Appearance
    uint32 TrainerCreatureEntry{26904};
    uint32 TrainerDisplayId{28103};
    float TrainerScale{1.0f};

    // Banker Spell
    bool BankerSpellEnable{true};
    bool BankerSpellAutoLearn{true};
    uint8 BankerSpellLearnLevel{1};
    uint32 BankerSpellId{67368};

    // Auctioneer Spell
    bool AuctioneerSpellEnable{true};
    bool AuctioneerSpellAutoLearn{true};
    uint8 AuctioneerSpellLearnLevel{5};
    uint32 AuctioneerSpellId{54614};

    // Trainer Spell
    bool TrainerSpellEnable{true};
    bool TrainerSpellAutoLearn{true};
    uint8 TrainerSpellLearnLevel{10};
    uint32 TrainerSpellId{54270};

    // Spell sync & de-level handling
    bool SpellsSyncOnLogin{true};
    bool SpellsRemoveIfBelowLevel{false};

    // Trainer policy
    bool TrainerFreeTraining{false};
    bool TrainerIgnoreLevelRequirements{false};
    bool TrainerFreeTalentReset{false};
    std::string TrainerProfessionMode{"Trainer"};

private:
    UtilityFollowerConfig() = default;
};

#define sUtilityFollowerConfig UtilityFollowerConfig::instance()

#endif // UTILITY_FOLLOWER_CONFIG_H
