/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#include "UtilityFollowerConfig.h"
#include "Config.h"
#include "Log.h"

UtilityFollowerConfig* UtilityFollowerConfig::instance()
{
    static UtilityFollowerConfig instance;
    return &instance;
}

void UtilityFollowerConfig::Load()
{
    LOG_INFO("server.loading", "Loading Utility Followers configuration...");

    Enable = sConfigMgr->GetOption<bool>("UtilityFollowers.Enable", true);

    BankerEnable = sConfigMgr->GetOption<bool>("UtilityFollowers.Banker.Enable", true);
    AuctioneerEnable = sConfigMgr->GetOption<bool>("UtilityFollowers.Auctioneer.Enable", true);
    TrainerEnable = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.Enable", true);

    FollowDistance = sConfigMgr->GetOption<float>("UtilityFollowers.FollowDistance", 3.5f);
    StartFollowingDistance = sConfigMgr->GetOption<float>("UtilityFollowers.StartFollowingDistance", 7.0f);
    CatchUpDistance = sConfigMgr->GetOption<float>("UtilityFollowers.CatchUpDistance", 30.0f);
    FollowUpdateInterval = sConfigMgr->GetOption<uint32>("UtilityFollowers.FollowUpdateInterval", 300);
    TeleportWhenStuck = sConfigMgr->GetOption<bool>("UtilityFollowers.TeleportWhenStuck", true);

    MaxActive = sConfigMgr->GetOption<uint32>("UtilityFollowers.MaxActive", 1);
    std::string policyStr = sConfigMgr->GetOption<std::string>("UtilityFollowers.MaxActivePolicy", "ReplaceOldest");

    AllowInWorld = sConfigMgr->GetOption<bool>("UtilityFollowers.AllowInWorld", true);
    AllowInDungeons = sConfigMgr->GetOption<bool>("UtilityFollowers.AllowInDungeons", true);
    AllowInRaids = sConfigMgr->GetOption<bool>("UtilityFollowers.AllowInRaids", true);
    AllowInBattlegrounds = sConfigMgr->GetOption<bool>("UtilityFollowers.AllowInBattlegrounds", false);
    AllowInArenas = sConfigMgr->GetOption<bool>("UtilityFollowers.AllowInArenas", false);

    AutoSpawn = sConfigMgr->GetOption<bool>("UtilityFollowers.AutoSpawn", false);

    // Banker Appearance
    BankerCreatureEntryHorde = sConfigMgr->GetOption<uint32>("UtilityFollowers.Banker.CreatureEntry.Horde", 10636);
    BankerCreatureEntryAlliance = sConfigMgr->GetOption<uint32>("UtilityFollowers.Banker.CreatureEntry.Alliance", 16225);
    BankerHordeDisplayId = sConfigMgr->GetOption<uint32>("UtilityFollowers.Banker.Horde.DisplayId", 7933);
    BankerAllianceDisplayId = sConfigMgr->GetOption<uint32>("UtilityFollowers.Banker.Alliance.DisplayId", 14546);
    BankerScale = sConfigMgr->GetOption<float>("UtilityFollowers.Banker.Scale", 1.0f);

    // Auctioneer Appearance
    AuctioneerCreatureEntry = sConfigMgr->GetOption<uint32>("UtilityFollowers.Auctioneer.CreatureEntry", 8661);
    AuctioneerDisplayId = sConfigMgr->GetOption<uint32>("UtilityFollowers.Auctioneer.DisplayId", 7993);
    AuctioneerScale = sConfigMgr->GetOption<float>("UtilityFollowers.Auctioneer.Scale", 1.0f);

    // Medivh (Trainer & Teleporter) Appearance
    TrainerCreatureEntry = sConfigMgr->GetOption<uint32>("UtilityFollowers.Trainer.CreatureEntry", 15608);
    TrainerDisplayId = sConfigMgr->GetOption<uint32>("UtilityFollowers.Trainer.DisplayId", 18718);
    TrainerScale = sConfigMgr->GetOption<float>("UtilityFollowers.Trainer.Scale", 1.0f);

    // Global Spell Defaults
    DefaultSpellLearnLevel = sConfigMgr->GetOption<uint8>("UtilityFollowers.Spells.DefaultLearnLevel", 10);

    // Banker Spell
    BankerSpellEnable = sConfigMgr->GetOption<bool>("UtilityFollowers.Banker.Spell.Enable", true);
    BankerSpellAutoLearn = sConfigMgr->GetOption<bool>("UtilityFollowers.Banker.Spell.AutoLearn", true);
    BankerSpellLearnLevel = sConfigMgr->GetOption<uint8>("UtilityFollowers.Banker.Spell.LearnLevel", DefaultSpellLearnLevel);
    BankerSpellId = sConfigMgr->GetOption<uint32>("UtilityFollowers.Banker.Spell.SpellId", 67368);

    // Auctioneer Spell
    AuctioneerSpellEnable = sConfigMgr->GetOption<bool>("UtilityFollowers.Auctioneer.Spell.Enable", true);
    AuctioneerSpellAutoLearn = sConfigMgr->GetOption<bool>("UtilityFollowers.Auctioneer.Spell.AutoLearn", true);
    AuctioneerSpellLearnLevel = sConfigMgr->GetOption<uint8>("UtilityFollowers.Auctioneer.Spell.LearnLevel", DefaultSpellLearnLevel);
    AuctioneerSpellId = sConfigMgr->GetOption<uint32>("UtilityFollowers.Auctioneer.Spell.SpellId", 54614);

    // Trainer / Medivh Spell
    TrainerSpellEnable = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.Spell.Enable", true);
    TrainerSpellAutoLearn = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.Spell.AutoLearn", true);
    TrainerSpellLearnLevel = sConfigMgr->GetOption<uint8>("UtilityFollowers.Trainer.Spell.LearnLevel", DefaultSpellLearnLevel);
    TrainerSpellId = sConfigMgr->GetOption<uint32>("UtilityFollowers.Trainer.Spell.SpellId", 62978);

    // Sync & De-level
    SpellsSyncOnLogin = sConfigMgr->GetOption<bool>("UtilityFollowers.Spells.SyncOnLogin", true);
    SpellsRemoveIfBelowLevel = sConfigMgr->GetOption<bool>("UtilityFollowers.Spells.RemoveIfBelowLevel", false);
    PreventActionBarAutoAdd = sConfigMgr->GetOption<bool>("UtilityFollowers.Spells.PreventActionBarAutoAdd", true);

    // Trainer Services
    TrainerFreeTraining = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.FreeTraining", false);
    TrainerIgnoreLevelRequirements = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.IgnoreLevelRequirements", false);
    TrainerFreeTalentReset = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.FreeTalentReset", false);
    TrainerProfessionMode = sConfigMgr->GetOption<std::string>("UtilityFollowers.Trainer.ProfessionMode", "Trainer");

    // Medivh Teleportation Services
    TrainerTeleportEnable = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.Teleport.Enable", true);
    TrainerTeleportCombatCheck = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.Teleport.CombatCheck", true);
    TrainerTeleportCost = sConfigMgr->GetOption<uint32>("UtilityFollowers.Trainer.Teleport.Cost", 0);
    TrainerTeleportMinLevel = sConfigMgr->GetOption<uint8>("UtilityFollowers.Trainer.Teleport.MinLevel", 1);
    TrainerTeleportDalaranMinLevel = sConfigMgr->GetOption<uint8>("UtilityFollowers.Trainer.Teleport.DalaranMinLevel", 68);
    TrainerTeleportEnableExtraLocations = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.Teleport.EnableExtraLocations", true);
    TrainerTeleportEnableDungeons = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.Teleport.EnableDungeons", true);
    TrainerTeleportEnableRaids = sConfigMgr->GetOption<bool>("UtilityFollowers.Trainer.Teleport.EnableRaids", true);

    // =========================================================================
    // Validation and Safe Fallbacks
    // =========================================================================
    if (FollowDistance <= 0.0f)
    {
        LOG_WARN("module.qol_followers", "Invalid UtilityFollowers.FollowDistance ({}), resetting to default 2.0", FollowDistance);
        FollowDistance = 2.0f;
    }

    if (StartFollowingDistance <= FollowDistance)
    {
        LOG_WARN("module.qol_followers", "UtilityFollowers.StartFollowingDistance ({}) must be greater than FollowDistance ({}), resetting to {}",
            StartFollowingDistance, FollowDistance, FollowDistance + 1.0f);
        StartFollowingDistance = FollowDistance + 1.0f;
    }

    if (CatchUpDistance <= StartFollowingDistance)
    {
        LOG_WARN("module.qol_followers", "UtilityFollowers.CatchUpDistance ({}) must be greater than StartFollowingDistance ({}), resetting to 30.0",
            CatchUpDistance, StartFollowingDistance);
        CatchUpDistance = 30.0f;
    }

    if (FollowUpdateInterval < 50)
    {
        LOG_WARN("module.qol_followers", "UtilityFollowers.FollowUpdateInterval ({}) too low, setting to minimum safe value 100 ms", FollowUpdateInterval);
        FollowUpdateInterval = 100;
    }

    if (MaxActive == 0)
    {
        LOG_WARN("module.qol_followers", "UtilityFollowers.MaxActive cannot be 0, resetting to 1");
        MaxActive = 1;
    }

    if (policyStr == "ReplaceOldest")
        ActivePolicy = MaxActivePolicy::ReplaceOldest;
    else if (policyStr == "Reject")
        ActivePolicy = MaxActivePolicy::Reject;
    else
    {
        LOG_WARN("module.qol_followers", "Unknown UtilityFollowers.MaxActivePolicy '{}', falling back to 'Reject'", policyStr);
        ActivePolicy = MaxActivePolicy::Reject;
    }

    if (BankerScale <= 0.0f)
    {
        LOG_WARN("module.qol_followers", "Invalid UtilityFollowers.Banker.Scale ({}), falling back to 1.0", BankerScale);
        BankerScale = 1.0f;
    }

    if (AuctioneerScale <= 0.0f)
    {
        LOG_WARN("module.qol_followers", "Invalid UtilityFollowers.Auctioneer.Scale ({}), falling back to 1.0", AuctioneerScale);
        AuctioneerScale = 1.0f;
    }

    if (TrainerScale <= 0.0f)
    {
        LOG_WARN("module.qol_followers", "Invalid UtilityFollowers.Trainer.Scale ({}), falling back to 1.0", TrainerScale);
        TrainerScale = 1.0f;
    }

    LOG_INFO("server.loading", "Utility Followers configuration loaded successfully. [Master Enable: {}]", Enable ? "YES" : "NO");
}
