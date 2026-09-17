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

    // Movement (smooth unanchored hysteresis: 3.5 steady state, starts following at 7.0)
    float FollowDistance{3.5f};
    float StartFollowingDistance{7.0f};
    float CatchUpDistance{30.0f};
    uint32 FollowUpdateInterval{300};
    bool TeleportWhenStuck{true};

    // Active limit & policy
    uint32 MaxActive{1};
    MaxActivePolicy ActivePolicy{MaxActivePolicy::ReplaceOldest};

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

    // Medivh (Trainer & Teleporter) Appearance
    uint32 TrainerCreatureEntry{15608};
    uint32 TrainerDisplayId{18718};
    float TrainerScale{1.0f};

    // Banker Spell
    bool BankerSpellEnable{true};
    bool BankerSpellAutoLearn{true};
    uint8 BankerSpellLearnLevel{10};
    uint32 BankerSpellId{87094};

    // Auctioneer Spell
    bool AuctioneerSpellEnable{true};
    bool AuctioneerSpellAutoLearn{true};
    uint8 AuctioneerSpellLearnLevel{10};
    uint32 AuctioneerSpellId{87093};

    // Trainer / Medivh Spell
    bool TrainerSpellEnable{true};
    bool TrainerSpellAutoLearn{true};
    uint8 TrainerSpellLearnLevel{10};
    uint32 TrainerSpellId{87092};

    // Global default spell learn level
    uint8 DefaultSpellLearnLevel{10};

    // Spell sync & de-level handling
    bool SpellsSyncOnLogin{true};
    bool SpellsRemoveIfBelowLevel{false};
    bool PreventActionBarAutoAdd{true};

    // Trainer policy
    bool TrainerFreeTraining{false};
    bool TrainerIgnoreLevelRequirements{false};
    bool TrainerFreeTalentReset{false};
    std::string TrainerProfessionMode{"Trainer"};

    // Medivh Teleportation policy
    bool TrainerTeleportEnable{true};
    bool TrainerTeleportCombatCheck{true};
    uint32 TrainerTeleportCost{0};
    uint8 TrainerTeleportMinLevel{1};
    uint8 TrainerTeleportDalaranMinLevel{68};
    bool TrainerTeleportEnableExtraLocations{true};
    bool TrainerTeleportEnableDungeons{true};
    bool TrainerTeleportEnableRaids{true};

private:
    UtilityFollowerConfig() = default;
};

#define sUtilityFollowerConfig UtilityFollowerConfig::instance()

#endif // UTILITY_FOLLOWER_CONFIG_H
