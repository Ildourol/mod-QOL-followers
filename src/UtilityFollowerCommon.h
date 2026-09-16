/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#ifndef UTILITY_FOLLOWER_COMMON_H
#define UTILITY_FOLLOWER_COMMON_H

#include "Common.h"
#include "SharedDefines.h"
#include <cmath>

enum class FollowerType : uint8
{
    Banker     = 0,
    Auctioneer = 1,
    Trainer    = 2,
    Max        = 3
};

enum class MaxActivePolicy : uint8
{
    Reject        = 0,
    ReplaceOldest = 1
};

// Gossip Senders
enum FollowerGossipSender : uint32
{
    SENDER_FOLLOWER_MAIN          = 100,
    SENDER_FOLLOWER_BANKER        = 101,
    SENDER_FOLLOWER_AUCTIONEER    = 102,
    SENDER_FOLLOWER_TRAINER       = 103,
    SENDER_FOLLOWER_PROF_PRIMARY  = 104,
    SENDER_FOLLOWER_PROF_SECOND   = 105,
    SENDER_FOLLOWER_TALENTS       = 106,
    SENDER_FOLLOWER_DUAL_SPEC     = 107
};

// Gossip Actions
enum FollowerGossipAction : uint32
{
    // General
    ACTION_FOLLOWER_DISMISS             = 1,
    ACTION_FOLLOWER_BACK_MAIN           = 2,

    // Banker
    ACTION_BANKER_OPEN                  = 10,

    // Auctioneer
    ACTION_AUCTIONEER_OPEN              = 20,

    // Trainer Menu Navigation
    ACTION_TRAINER_CLASS                = 30,
    ACTION_TRAINER_PROF_MAIN            = 31,
    ACTION_TRAINER_PROF_PRIMARY_MENU    = 32,
    ACTION_TRAINER_PROF_SECONDARY_MENU  = 33,
    ACTION_TRAINER_TALENT_SERVICES      = 34,
    ACTION_TRAINER_DUAL_SPEC_MENU       = 35,

    // Talent Actions
    ACTION_TALENT_RESET_CONFIRM         = 40,
    ACTION_TALENT_RESET_EXECUTE         = 41,
    ACTION_TALENT_RESET_PET             = 42,

    // Dual Spec Actions
    ACTION_DUAL_SPEC_EXECUTE            = 45,

    // Primary Professions (Base offset 100)
    ACTION_PROF_ALCHEMY                 = 101,
    ACTION_PROF_BLACKSMITHING           = 102,
    ACTION_PROF_ENCHANTING              = 103,
    ACTION_PROF_ENGINEERING             = 104,
    ACTION_PROF_HERBALISM               = 105,
    ACTION_PROF_INSCRIPTION             = 106,
    ACTION_PROF_JEWELCRAFTING           = 107,
    ACTION_PROF_LEATHERWORKING          = 108,
    ACTION_PROF_MINING                  = 109,
    ACTION_PROF_SKINNING                = 110,
    ACTION_PROF_TAILORING               = 111,

    // Secondary Professions (Base offset 200)
    ACTION_PROF_COOKING                 = 201,
    ACTION_PROF_FIRST_AID               = 202,
    ACTION_PROF_FISHING                 = 203
};

// Verified WotLK Class Trainer Template IDs
enum ClassTrainerTemplate : uint32
{
    TRAINER_WARRIOR      = 913,   // Lyrok
    TRAINER_PALADIN      = 927,   // Brother Wilhelm
    TRAINER_HUNTER       = 987,   // Ogromm
    TRAINER_ROGUE        = 917,   // Gest
    TRAINER_PRIEST       = 376,   // High Priestess Laurena
    TRAINER_DEATH_KNIGHT = 28471, // Lady Alistra
    TRAINER_SHAMAN       = 986,   // Kardris Dreamseeker
    TRAINER_MAGE         = 328,   // Khelden Bremen
    TRAINER_WARLOCK      = 461,   // Demisette Clochar
    TRAINER_DRUID        = 3033   // Turak Runetotem
};

// Verified WotLK Grand Master Profession Trainer Template IDs (Dalaran neutral masters)
enum ProfessionTrainerTemplate : uint32
{
    // Primary
    TRAINER_ALCHEMY        = 26903, // Lanolis Dewdrop
    TRAINER_BLACKSMITHING  = 26904, // Rosina Rivet
    TRAINER_ENCHANTING     = 26906, // Elizabeth Jackson
    TRAINER_ENGINEERING    = 26907, // Tisha Longbridge
    TRAINER_HERBALISM      = 26910, // Fayin Whisperleaf
    TRAINER_INSCRIPTION    = 26916, // Mindri Dinkles
    TRAINER_JEWELCRAFTING  = 26915, // Ounhulo
    TRAINER_LEATHERWORKING = 26996, // Awan Iceborn
    TRAINER_MINING         = 26912, // Grumbol Stoutpick
    TRAINER_SKINNING       = 26913, // Frederic Burrhus
    TRAINER_TAILORING      = 26914, // Benjamin Clegg

    // Secondary
    TRAINER_COOKING        = 26905, // Brom Brewbaster
    TRAINER_FIRST_AID      = 26956, // Sally Tompkins
    TRAINER_FISHING        = 26909  // Byron Welwick
};

// Distinct follow angles relative to player facing so multiple followers form up neatly behind player
inline float GetFollowerAngle(FollowerType type)
{
    switch (type)
    {
        case FollowerType::Banker:
            return static_cast<float>(M_PI * 0.75); // behind-left (~135 deg)
        case FollowerType::Auctioneer:
            return static_cast<float>(M_PI * 1.25); // behind-right (~225 deg)
        case FollowerType::Trainer:
        default:
            return static_cast<float>(M_PI);        // directly behind (~180 deg)
    }
}

#endif // UTILITY_FOLLOWER_COMMON_H
