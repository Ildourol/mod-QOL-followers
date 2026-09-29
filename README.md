# mod-QOL-followers

**mod-QOL-followers** is a standalone AzerothCore WotLK module that gives real players summonable utility companions for services that are normally tied to cities and NPC hubs.

The module currently provides a **Banker**, an **Auctioneer**, and **Medivh** as a combined trainer, profession helper, talent-service NPC, dual-spec trainer, and teleporter. Followers physically accompany their owner, use configurable movement hysteresis, and can safely catch up when the player gets too far away.

No custom client patch or SQL import is required.

## Followers

| Follower | Main services |
| --- | --- |
| **Banker** | Opens the player's bank and can be dismissed from gossip. Uses faction-specific Pack Kodo / Pack Mule appearances by default. |
| **Auctioneer** | Opens the Auction House and inherits the owner's faction so the correct auction house is used. |
| **Medivh** | Class training, profession trainers, talent reset services, pet talent reset for hunters, dual specialization, and teleportation. |

### Medivh teleportation

Medivh can teleport players to:

- Dalaran and Shattrath
- Alliance and Horde capitals
- Classic, TBC, and WotLK dungeon entrances
- Classic, TBC, and WotLK raid entrances
- Extra world destinations such as Karazhan, Caverns of Time, Gadgetzan, and Booty Bay

Teleport use, cost, minimum level, Dalaran level requirement, dungeon/raid menus, and extra destinations are configurable.

## Summoning

By default, eligible real players learn three native WotLK spells at level 10:

| Follower | Spell |
| --- | --- |
| Banker | `67368` — Bank Errand |
| Auctioneer | `54614` — Steam-Powered Auctioneer |
| Medivh | `62978` — Summon Guardian |

Casting a follower spell toggles that follower:

- not active → summon
- already active → dismiss

The module can prevent automatically learned follower spells from being placed on the action bar. Players can still drag them from the spellbook manually.

## Chat commands

Player commands are also available:

```text
.utility banker
.utility auctioneer
.utility medivh
.utility dismiss
.utility
```

The first three commands toggle their matching follower, `.utility dismiss` removes all active utility followers, and `.utility` displays command help.

## Movement behavior

The follower AI is designed to avoid constant micro-adjustments around the player.

Default movement settings:

| Setting | Default | Purpose |
| --- | ---: | --- |
| `UtilityFollowers.FollowDistance` | `3.5` | Preferred resting distance from the owner |
| `UtilityFollowers.StartFollowingDistance` | `7.0` | Distance that starts follow movement |
| `UtilityFollowers.CatchUpDistance` | `30.0` | Distance that triggers safe catch-up/teleport behavior |
| `UtilityFollowers.FollowUpdateInterval` | `300` ms | Movement update cadence |
| `UtilityFollowers.TeleportWhenStuck` | `1` | Allows safe catch-up when stuck or too far away |

The default active-follower limit is one:

```ini
UtilityFollowers.MaxActive = 1
UtilityFollowers.MaxActivePolicy = "ReplaceOldest"
```

Both the capacity and replacement policy are configurable.

## Environment controls

Followers can be enabled or disabled independently for different content:

```ini
UtilityFollowers.AllowInWorld = 1
UtilityFollowers.AllowInDungeons = 1
UtilityFollowers.AllowInRaids = 1
UtilityFollowers.AllowInBattlegrounds = 0
UtilityFollowers.AllowInArenas = 0
```

Battlegrounds and arenas are disabled by default.

## Installation

Clone the module into your AzerothCore `modules/` directory:

```bash
cd ~/azerothcore-wotlk/modules
git clone https://github.com/Ildourol/mod-QOL-followers.git
```

Then rebuild/install AzerothCore using your normal build workflow. For the mod-playerbots AzerothCore fork, for example:

```bash
cd ~/azerothcore-wotlk
./acore.sh compiler all
```

After installation, the distributed configuration is installed with the module configs. Create the live config if your setup does not already do this automatically:

```bash
cd ~/azerothcore-wotlk/env/dist/etc/modules
cp mod_qol_followers.conf.dist mod_qol_followers.conf
```

Restart `worldserver` after rebuilding or changing module configuration.

## Configuration

The full configuration template is:

```text
conf/mod_qol_followers.conf.dist
```

Notable configuration groups include:

- master enable/disable
- per-follower enable switches
- movement distances and update interval
- maximum active follower count and replacement policy
- world / dungeon / raid / PvP restrictions
- follower creature entries, display IDs, and scale
- summon spell IDs and auto-learning
- trainer costs and level restrictions
- profession-training mode
- teleport cost, combat restriction, level requirements, and destination menus

## Compatibility

The module metadata declares AzerothCore `^3.0.0` compatibility for both the standard `master` branch and the mod-playerbots `Playerbot` branch.

This module is self-contained and does not require database schema changes or custom client files.

## Project layout

```text
mod-QOL-followers/
├── conf/
│   └── mod_qol_followers.conf.dist
├── src/
│   ├── UtilityFollowerAI.cpp
│   ├── UtilityFollowerAI.h
│   ├── UtilityFollowerCommon.h
│   ├── UtilityFollowerConfig.cpp
│   ├── UtilityFollowerConfig.h
│   ├── UtilityFollowerMgr.cpp
│   ├── UtilityFollowerMgr.h
│   ├── UtilityFollowerScripts.cpp
│   └── mod_QOL_followers_loader.cpp
├── acore-module.json
├── CMakeLists.txt
└── include.sh
```

## License

The module source files currently carry **GNU AGPL v3** license headers.

---

Built for AzerothCore WotLK 3.3.5a.
