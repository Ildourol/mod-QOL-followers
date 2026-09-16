# mod-QOL-followers for AzerothCore (WotLK 3.3.5a)

<p align="center">
  <img src="assets/banner.png" alt="mod-QOL-followers Banner" width="850">
</p>

<p align="center">
  <a href="https://github.com/azerothcore/azerothcore-wotlk"><img src="https://img.shields.io/badge/AzerothCore-WotLK%203.3.5a-blue.svg" alt="AzerothCore"></a>
  <a href="https://github.com/Ildourol/mod-QOL-followers/blob/master/conf/mod_qol_followers.conf.dist"><img src="https://img.shields.io/badge/Configuration-Fully%20Configurable-brightgreen.svg" alt="Configurable"></a>
  <a href="https://github.com/Ildourol/mod-QOL-followers/blob/master/acore-module.json"><img src="https://img.shields.io/badge/Compatibility-v1.0.0-orange.svg" alt="Module Version"></a>
</p>

---

## Description

**mod-QOL-followers** is a standalone AzerothCore C++ module providing player-owned utility NPC companions for World of Warcraft: Wrath of the Lich King (3.3.5a).

Followers physically accompany their master across the world with smooth movement hysteresis and collision avoidance, offering essential town services through intuitive gossip dialogues. Each follower can be summoned individually via dedicated player spells or chat commands.

---

## Companions

### 1. Banker Follower
- **Horde Appearance**: Pack Kodo (Display ID `7933`, Creature Entry `10636`)
- **Alliance Appearance**: Pack Mule (Display ID `14546`, Creature Entry `16225`)
- **Services**:
  - `Open Bank` - Access your personal bank vault anywhere across the open world.
  - `Dismiss` - Safely unsummons the banker.

### 2. Auctioneer Follower
- **Appearance**: Goblin Auctioneer (Display ID `7993`, Creature Entry `8661`)
- **Faction Correctness**: Automatically inherits the player's faction, opening Alliance, Horde, or Neutral Auction House dialogues appropriately.
- **Services**:
  - `Open Auction House` - Browse, bid, and list auctions directly in the field.
  - `Dismiss` - Safely unsummons the auctioneer.

### 3. Trainer Tome Follower
- **Appearance**: Floating Arcane Master Tome (Display ID `28103`, Creature Entry `26904`)
- **Hierarchical Services**:
  - **Class Training**: Automatically detects the player's class and provides full master class training up to level 80.
  - **Professions**: Submenus for all 11 Primary Professions and 3 Secondary Professions, backed by neutral Dalaran Grand Master trainers up to rank 450.
  - **Talent Services**: Reset class talents (with progressive cost scaling) and reset pet talents for hunters.
  - **Dual Specialization**: Learn dual specialization at level 40+ with standard 1,000 gold requirement.
  - **Dismiss**: Safely unsummons the trainer book.

---

## Movement and Following Mechanics

Followers utilize native AzerothCore `MotionMaster::MoveFollow` with tuned hysteresis to prevent stop/start jitter:
- **`FollowDistance`** (`3.0` yards): Preferred steady-state following distance.
- **`StartFollowingDistance`** (`5.0` yards): Hysteresis threshold - follower stays idle until the player exceeds this distance.
- **`CatchUpDistance`** (`30.0` yards): If the player mounts or sprints, the follower smoothly teleports nearby without runaway pathfinding.
- **Formation Offsets**: Each companion follows at a dedicated angle relative to the player's facing direction, allowing all three followers to be active simultaneously without clipping.

---

## Dedicated Summon Spells

Each follower can be summoned and dismissed via its own dedicated spell:
- **Summon Banker**: Spell `67368` (*Bank Errand*, Icon: `inv_misc_coin_02`)
- **Summon Auctioneer**: Spell `54614` (*Steam-Powered Auctioneer*, Icon: `trade_engineering`)
- **Summon Trainer Book**: Spell `54270` (*Argent Tome Book Spawn*, Icon: `inv_misc_book_13`)

### Toggle Behavior
- When absent: Casting the spell summons the corresponding follower.
- When present: Casting the spell dismisses that follower.
- Spells can be dragged from the General tab of the spellbook to action bars.

---

## Chat Commands

Optional player chat commands (`SEC_PLAYER`):
```text
.utility banker     - Toggle Banker follower
.utility auctioneer - Toggle Auctioneer follower
.utility trainer    - Toggle Trainer Book follower
.utility dismiss    - Dismiss all active utility followers
.utility            - Show command list and help
```

---

## Directory Structure

```
mod-QOL-followers/
├── conf/
│   └── mod_qol_followers.conf.dist      # Complete module configuration template
├── src/
│   ├── UtilityFollowerAI.cpp            # Companion AI, movement, and hysteresis
│   ├── UtilityFollowerAI.h
│   ├── UtilityFollowerCommon.h          # Constants and data models
│   ├── UtilityFollowerConfig.cpp        # Configuration loader
│   ├── UtilityFollowerConfig.h
│   ├── UtilityFollowerMgr.cpp           # Summoning and lifecycle manager
│   ├── UtilityFollowerMgr.h
│   ├── UtilityFollowerScripts.cpp       # Gossip menus, spell hooks, and commands
│   └── mod_QOL_followers_loader.cpp     # Script registry entry point
├── assets/                              # Documentation media
├── acore-module.json                    # Module metadata
├── CMakeLists.txt                       # Build script
└── include.sh                           # Build hook
```

---

## Installation

1. Navigate to your AzerothCore `modules/` directory and clone this repository:
   ```bash
   cd azerothcore-wotlk/modules
   git clone https://github.com/Ildourol/mod-QOL-followers.git
   ```

2. Re-generate CMake and compile `worldserver`:
   ```bash
   cd azerothcore-wotlk/build
   cmake ../ -DCMAKE_INSTALL_PREFIX=/path/to/server
   make -j $(nproc)
   make install
   ```

3. Configure the module:
   ```bash
   cp ../modules/mod-QOL-followers/conf/mod_qol_followers.conf.dist /path/to/server/etc/mod_qol_followers.conf
   ```

4. Database Setup:
   - This module requires **no database modifications**. All NPC entries, display IDs, gossip menus, and trainer links are resolved dynamically in C++.

---

## License

This module is released under the GNU General Public License v2 (or at your option any later version) in accordance with AzerothCore licensing.
