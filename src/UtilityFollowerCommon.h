/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#ifndef UTILITY_FOLLOWER_COMMON_H
#define UTILITY_FOLLOWER_COMMON_H

#include "Common.h"
#include "SharedDefines.h"
#include "Random.h"
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
    SENDER_FOLLOWER_MAIN              = 100,
    SENDER_FOLLOWER_BANKER            = 101,
    SENDER_FOLLOWER_AUCTIONEER        = 102,
    SENDER_FOLLOWER_TRAINER           = 103,
    SENDER_FOLLOWER_PROF_PRIMARY      = 104,
    SENDER_FOLLOWER_PROF_SECOND       = 105,
    SENDER_FOLLOWER_TALENTS           = 106,
    SENDER_FOLLOWER_DUAL_SPEC         = 107,
    SENDER_FOLLOWER_TELEPORT          = 108,
    SENDER_FOLLOWER_TELEPORT_EXTRA    = 109,
    SENDER_FOLLOWER_TELEPORT_DUNGEONS = 110,
    SENDER_FOLLOWER_TELEPORT_RAIDS    = 111,
    SENDER_FOLLOWER_TELEPORT_DEST     = 112
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

    // Trainer / Medivh Menu Navigation
    ACTION_TRAINER_CLASS                = 30,
    ACTION_TRAINER_PROF_MAIN            = 31,
    ACTION_TRAINER_PROF_PRIMARY_MENU    = 32,
    ACTION_TRAINER_PROF_SECONDARY_MENU  = 33,
    ACTION_TRAINER_TALENT_SERVICES      = 34,
    ACTION_TRAINER_DUAL_SPEC_MENU       = 35,
    ACTION_TRAINER_TELEPORT_MENU        = 36,
    ACTION_TRAINER_TELEPORT_EXTRA_MENU  = 37,

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
    ACTION_PROF_FISHING                 = 203,

    // Major Neutral Cities
    ACTION_TELEPORT_DALARAN             = 301,
    ACTION_TELEPORT_SHATTRATH           = 302,

    // Alliance Capitals
    ACTION_TELEPORT_STORMWIND           = 311,
    ACTION_TELEPORT_IRONFORGE           = 312,
    ACTION_TELEPORT_DARNASSUS           = 313,
    ACTION_TELEPORT_EXODAR              = 314,

    // Horde Capitals
    ACTION_TELEPORT_ORGRIMMAR           = 321,
    ACTION_TELEPORT_UNDERCITY           = 322,
    ACTION_TELEPORT_THUNDER_BLUFF       = 323,
    ACTION_TELEPORT_SILVERMOON          = 324,

    // Category Menus under "Other Destinations"
    ACTION_TELEPORT_CAT_DUNGEONS        = 340,
    ACTION_TELEPORT_CAT_RAIDS           = 341,
    ACTION_TELEPORT_CAT_HUBS            = 342,

    // Dungeon Menus & Pages
    ACTION_TELEPORT_DUNGEONS_CLASSIC_P1 = 350,
    ACTION_TELEPORT_DUNGEONS_CLASSIC_P2 = 351,
    ACTION_TELEPORT_DUNGEONS_TBC_P1     = 352,
    ACTION_TELEPORT_DUNGEONS_TBC_P2     = 353,
    ACTION_TELEPORT_DUNGEONS_WRATH_P1   = 354,
    ACTION_TELEPORT_DUNGEONS_WRATH_P2   = 355,

    // Raid Menus
    ACTION_TELEPORT_RAIDS_CLASSIC       = 360,
    ACTION_TELEPORT_RAIDS_TBC           = 361,
    ACTION_TELEPORT_RAIDS_WRATH         = 362,

    // World Hubs (Thematic Destinations)
    ACTION_TELEPORT_KARAZHAN            = 370,
    ACTION_TELEPORT_CAVERNS_OF_TIME     = 371,
    ACTION_TELEPORT_GADGETZAN           = 372,
    ACTION_TELEPORT_BOOTY_BAY           = 373,

    // Vanilla Dungeons (400 - 449)
    ACTION_TELEPORT_RFC                 = 401,
    ACTION_TELEPORT_VC                  = 402,
    ACTION_TELEPORT_WC                  = 403,
    ACTION_TELEPORT_SFK                 = 404,
    ACTION_TELEPORT_BFD                 = 405,
    ACTION_TELEPORT_STOCKADE            = 406,
    ACTION_TELEPORT_GNOMER              = 407,
    ACTION_TELEPORT_RFK                 = 408,
    ACTION_TELEPORT_SM                  = 409,
    ACTION_TELEPORT_RFD                 = 410,
    ACTION_TELEPORT_ULDA                = 411,
    ACTION_TELEPORT_ZF                  = 412,
    ACTION_TELEPORT_MARA                = 413,
    ACTION_TELEPORT_ST                  = 414,
    ACTION_TELEPORT_BRD                 = 415,
    ACTION_TELEPORT_LBRS                = 416,
    ACTION_TELEPORT_UBRS                = 417,
    ACTION_TELEPORT_DM                  = 418,
    ACTION_TELEPORT_SCHOLO              = 419,
    ACTION_TELEPORT_STRAT               = 420,

    // Vanilla Raids (450 - 469)
    ACTION_TELEPORT_MC                  = 451,
    ACTION_TELEPORT_BWL                 = 452,
    ACTION_TELEPORT_AQ20                = 453,
    ACTION_TELEPORT_AQ40                = 454,
    ACTION_TELEPORT_ONYXIA              = 455,
    ACTION_TELEPORT_ZG                  = 456,

    // TBC Dungeons (500 - 549)
    ACTION_TELEPORT_RAMPARTS            = 501,
    ACTION_TELEPORT_BLOOD_FURNACE       = 502,
    ACTION_TELEPORT_SHATTERED_HALLS     = 503,
    ACTION_TELEPORT_SLAVE_PENS          = 504,
    ACTION_TELEPORT_UNDERBOG            = 505,
    ACTION_TELEPORT_STEAMVAULT          = 506,
    ACTION_TELEPORT_MANA_TOMBS          = 507,
    ACTION_TELEPORT_AUCHENAI_CRYPTS     = 508,
    ACTION_TELEPORT_SETHEKK_HALLS       = 509,
    ACTION_TELEPORT_SHADOW_LABYRINTH    = 510,
    ACTION_TELEPORT_DURNHOLDE           = 511,
    ACTION_TELEPORT_BLACK_MORASS        = 512,
    ACTION_TELEPORT_BOTANICA            = 513,
    ACTION_TELEPORT_MECHANAR            = 514,
    ACTION_TELEPORT_ARCATRAZ            = 515,
    ACTION_TELEPORT_MAGISTERS_TERRACE   = 516,

    // TBC Raids (550 - 569)
    ACTION_TELEPORT_KARA_RAID           = 551,
    ACTION_TELEPORT_GRUUL               = 552,
    ACTION_TELEPORT_MAGTHERIDON         = 553,
    ACTION_TELEPORT_SSC                 = 554,
    ACTION_TELEPORT_THE_EYE             = 555,
    ACTION_TELEPORT_HYJAL               = 556,
    ACTION_TELEPORT_BLACK_TEMPLE        = 557,
    ACTION_TELEPORT_SUNWELL             = 558,
    ACTION_TELEPORT_ZULAMAN             = 559,

    // WotLK Dungeons (600 - 649)
    ACTION_TELEPORT_UTGARDE_KEEP        = 601,
    ACTION_TELEPORT_THE_NEXUS           = 602,
    ACTION_TELEPORT_AZJOL_NERUB         = 603,
    ACTION_TELEPORT_AHN_KAHET           = 604,
    ACTION_TELEPORT_DRAK_THARON         = 605,
    ACTION_TELEPORT_VIOLET_HOLD         = 606,
    ACTION_TELEPORT_GUNDRAK             = 607,
    ACTION_TELEPORT_HALLS_OF_STONE      = 608,
    ACTION_TELEPORT_HALLS_OF_LIGHTNING  = 609,
    ACTION_TELEPORT_THE_OCULUS          = 610,
    ACTION_TELEPORT_UTGARDE_PINNACLE    = 611,
    ACTION_TELEPORT_CULLING_STRATHOLME  = 612,
    ACTION_TELEPORT_TRIAL_OF_CHAMPION   = 613,
    ACTION_TELEPORT_FORGE_OF_SOULS      = 614,
    ACTION_TELEPORT_PIT_OF_SARON        = 615,
    ACTION_TELEPORT_HALLS_OF_REFLECTION = 616,

    // WotLK Raids (650 - 669)
    ACTION_TELEPORT_NAXXRAMAS           = 651,
    ACTION_TELEPORT_OBSIDIAN_SANCTUM    = 652,
    ACTION_TELEPORT_EYE_OF_ETERNITY     = 653,
    ACTION_TELEPORT_VAULT_OF_ARCHAVON   = 654,
    ACTION_TELEPORT_ULDUAR              = 655,
    ACTION_TELEPORT_TRIAL_OF_CRUSADER   = 656,
    ACTION_TELEPORT_ICECROWN_CITADEL    = 657,
    ACTION_TELEPORT_RUBY_SANCTUM        = 658
};

// Verified WotLK Class Trainer Template IDs - Corrected for accurate classes and non-hostility
enum AllianceClassTrainerTemplate : uint32
{
    TRAINER_ALLIANCE_WARRIOR      = 914,   // Ander Germaine (Stormwind - Warrior Trainer)
    TRAINER_ALLIANCE_PALADIN      = 5492,  // Katherine the Pure (Stormwind - Paladin Trainer)
    TRAINER_ALLIANCE_HUNTER       = 5515,  // Einris Brightspear (Stormwind - Hunter Trainer)
    TRAINER_ALLIANCE_ROGUE        = 918,   // Osborne the Night Man (Stormwind - Rogue Trainer)
    TRAINER_ALLIANCE_PRIEST       = 11401, // Priestess Alathea (Stormwind - Priest Trainer)
    TRAINER_ALLIANCE_DK           = 28474, // Amal'thazad (Acherus - Death Knight Trainer)
    TRAINER_ALLIANCE_SHAMAN       = 20407, // Farseer Umbrua (Exodar / Stormwind - Shaman Trainer)
    TRAINER_ALLIANCE_MAGE         = 328,   // Zaldimar Wefhellt (Stormwind - Mage Trainer)
    TRAINER_ALLIANCE_WARLOCK      = 5495,  // Ursula Deline (Stormwind - Warlock Trainer)
    TRAINER_ALLIANCE_DRUID        = 4217   // Mathrengyl Bearwalker (Darnassus - Druid Trainer)
};

enum HordeClassTrainerTemplate : uint32
{
    TRAINER_HORDE_WARRIOR         = 4593,  // Christoph Walker (Undercity - Warrior Trainer)
    TRAINER_HORDE_PALADIN         = 16681, // Champion Bachi (Silvermoon - Paladin Trainer)
    TRAINER_HORDE_HUNTER          = 3406,  // Xor'juul (Orgrimmar - Hunter Trainer)
    TRAINER_HORDE_ROGUE           = 3328,  // Ormok (Orgrimmar - Rogue Trainer)
    TRAINER_HORDE_PRIEST          = 6018,  // Ur'kyo (Orgrimmar - Priest Trainer)
    TRAINER_HORDE_DK              = 28472, // Lord Thorval (Acherus - Death Knight Trainer)
    TRAINER_HORDE_SHAMAN          = 3032,  // Beram Skychaser (Thunder Bluff - Shaman Trainer)
    TRAINER_HORDE_MAGE            = 5885,  // Deino (Orgrimmar - Mage Trainer)
    TRAINER_HORDE_WARLOCK         = 3324,  // Grol'dar (Orgrimmar - Warlock Trainer)
    TRAINER_HORDE_DRUID           = 3033   // Turak Runetotem (Thunder Bluff - Druid Trainer)
};

// Verified WotLK Grand Master Profession Trainer Template IDs (Dalaran neutral masters)
enum ProfessionTrainerTemplate : uint32
{
    // Primary
    TRAINER_ALCHEMY        = 26953, // Linzy Blackbolt
    TRAINER_BLACKSMITHING  = 26952, // Alard Schmied
    TRAINER_ENCHANTING     = 26906, // Alexis Marlow
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

// Teleport Destination Representation
struct TeleportLocation
{
    uint32 action;
    char const* name;
    uint32 mapId;
    float x, y, z, o;
};

// Point identifier for custom unanchored follower approach
constexpr uint32 POINT_FOLLOWER_APPROACH = 1001;

#endif // UTILITY_FOLLOWER_COMMON_H
