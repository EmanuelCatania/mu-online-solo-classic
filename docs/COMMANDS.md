# MU Online Solo Classic (0.97k) - Server Commands Guide

This document contains a comprehensive manual for all in-game chat and console commands supported by the **MU Online Solo Classic (0.97k)** WebAssembly engine.

Commands are typed directly into the standard in-game chat input box (press `Enter` to open chat, type the command beginning with `/`, and press `Enter` to execute).

---

## Table of Contents

1. [Quick Reference Cheat Sheet](#1-quick-reference-cheat-sheet)
2. [General Rules & Mechanics](#2-general-rules--mechanics)
3. [Standard Player Commands](#3-standard-player-commands)
   - [Stat Point Allocation (`/str`, `/agi`, `/vit`, `/ene`)](#stat-point-allocation)
   - [Automated Stat Distribution (`auto`)](#automated-stat-distribution)
   - [Character Reset (`/reset`)](#character-reset)
   - [Automated Reset Loop (`/reset auto`)](#automated-reset-loop)
   - [Grand Reset (`/grandreset`)](#grand-reset)
   - [PK / Murderer Clear (`/pkclear`)](#pk--murderer-clear)
   - [Class Evolution (`/evo`)](#class-evolution)
   - [Multi-Vault Warehouse (`/ware`)](#multi-vault-warehouse)
   - [Inventory Cleaner (`/clearinv`)](#inventory-cleaner)
   - [Map Teleportation (`/move`)](#map-teleportation)
   - [Global Chat Broadcast (`/post`)](#global-chat-broadcast)
   - [Request Filters & Auto-Party (`/re`)](#request-filters--auto-party)
   - [Guild Warfare (`/war`, `/soccer`)](#guild-warfare)
   - [Online Status (`/onlines`)](#online-status)
4. [Game Master (GM) & Admin Commands](#4-game-master-gm--admin-commands)
   - [GM Authorization & Configuration](#gm-authorization--configuration)
   - [Zen Management (`/addmoney`, `/submoney`)](#zen-management)
   - [Moderation & Teleportation (`/gmove`, `/gmoveall`, `/chase`, `/bring`, `/disconnect`)](#moderation--teleportation)
   - [Item Creation & Spawning (`/make`, `/makeset`, `/drop`)](#item-creation--spawning)
   - [Monster Spawning & Morphing (`/makemob`, `/skin`)](#monster-spawning--morphing)
   - [Events & Celebrations (`/fireworks`, `/global`, `/tamastart`)](#events--celebrations)
5. [Appendix & Reference Tables](#5-appendix--reference-tables)
   - [Move Destinations Table](#move-destinations-table)
   - [Item Group IDs Table](#item-group-ids-table)
   - [Armor Set IDs Table](#armor-set-ids-table)
   - [Excellent Option Bitmask Table](#excellent-option-bitmask-table)
   - [Common Item Creation Examples](#common-item-creation-examples)
   - [Common Monster IDs Table](#common-monster-ids-table)
   - [Map Numbers Table](#map-numbers-table)
   - [Chat Channels Prefix Operators](#chat-channels-prefix-operators)

---

## 1. Quick Reference Cheat Sheet

### Player Commands

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `/str` | `<amount>` | Instantly add `<amount>` points to Strength. |
| `/str auto` | `<target_amount>` | Auto-add points into Strength as you level up (`0` disables). |
| `/agi` | `<amount>` | Instantly add `<amount>` points to Agility. |
| `/agi auto` | `<target_amount>` | Auto-add points into Agility as you level up (`0` disables). |
| `/vit` | `<amount>` | Instantly add `<amount>` points to Vitality. |
| `/vit auto` | `<target_amount>` | Auto-add points into Vitality as you level up (`0` disables). |
| `/ene` | `<amount>` | Instantly add `<amount>` points to Energy. |
| `/ene auto` | `<target_amount>` | Auto-add points into Energy as you level up (`0` disables). |
| `/reset` | *none* | Perform a character reset once required level is reached. |
| `/reset auto` | `<str> <agi> <vit> <ene>` | Enable automated reset upon reaching max level with auto-stat distribution (re-type to disable). |
| `/grandreset` | *none* | Perform a Grand Reset once reset milestone is reached. |
| `/pkclear` | *none* | Cleanse PK status (murderer / outlaw) back to normal citizen. |
| `/evo` | *none* | Instantly complete 2nd class change quest (DK -> BK, DW -> SM, Elf -> ME). |
| `/ware` | `<0..4>` | Switch active warehouse vault to store additional items. |
| `/clearinv` | *none* | Clears all unequipped items from your inventory bags (equipped gear is safe). |
| `/move` | `<MapName>` | Teleport to a designated city or dungeon (e.g. `/move Lorencia`). |
| `/post` | `<message>` | Broadcast a message across the entire server. |
| `/re` | `<on \| off \| auto [password]>` | Configure request acceptance (Trade/Party/Guild). |
| `/war` | `<GuildName>` | Declare a Guild War against an opposing guild. |
| `/soccer` | `<GuildName>` | Challenge an opposing guild to Battle Soccer in Arena. |
| `/onlines` | *none* | Display the current count of online characters. |

### Game Master (GM) Commands

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `/addmoney` | `<amount>` | Add Zen to character inventory (up to 2,000,000,000). |
| `/submoney` | `<amount>` | Deduct Zen from character inventory. |
| `/global` | `<message>` | Broadcast a yellow GM announcement across the server. |
| `/fireworks` | *none* | Spawn 15 colorful celebratory fireworks around the GM. |
| `/chase` | `<player>` | Teleport yourself directly to the player's current coordinates. |
| `/bring` | `<player>` | Teleport the target player directly to your coordinates. |
| `/gmove` | `<player> <map> <x> <y>` | Teleport a player to an exact map number and coordinate. |
| `/gmoveall` | `<map> <x> <y>` | Teleport all players on the server to an exact map coordinate. |
| `/disconnect` | `<player>` | Disconnect / kick a player from the server. |
| `/make` | `[-quant] <sec> <type> <lvl> <sk> <lk> <opt> <exc>` | Create item(s) directly into GM inventory (negative first param for quantity). |
| `/makeset` | `<type> <lvl> <sk> <lk> <opt> <exc>` | Create a complete 5-piece armor set (Helm, Armor, Pants, Gloves, Boots). |
| `/drop` | `<sec> <type> <lvl> <sk> <lk> <opt> <exc>` | Drop an item directly on the floor at your coordinates. |
| `/makemob` | `<MonsterID> [quantity]` | Spawn monster(s) around the GM (auto-despawns after 30 min). |
| `/skin` | `<player> <MonsterID>` | Change player model into a monster (`0` resets to normal). |
| `/tamastart` | *none* | Trigger the Tamachan invasion event (starts in ~2 minutes). |

---

## 2. General Rules & Mechanics

1. **Case-Insensitive Syntax**: Command names and sub-arguments are case-insensitive (e.g., `/reset AUTO`, `/Move lorencia`, `/STR 50` all work).
2. **Open Interfaces Restriction**: Commands cannot be executed while an interactive interface is active (NPC Shop, Player Trade, Vault Warehouse, or Chaos Machine). Close the window first before issuing commands.
3. **Account Levels (AL)**:
   - `AL0`: Free / Standard player
   - `AL1`: VIP Bronze
   - `AL2`: VIP Silver
   - `AL3`: VIP Gold
4. **Execution Delays**: Some commands have cooldowns configured in `Command.txt` to prevent spam.

---

## 3. Standard Player Commands

### Stat Point Allocation

Quickly allocate unspent level-up stat points without opening the Character Attributes (`C`) window or clicking buttons repeatedly.

```text
/str <amount>
/agi <amount>
/vit <amount>
/ene <amount>
```

- `<amount>`: Number of points to add to the stat. Must be greater than 0 and less than or equal to your current unallocated `LevelUpPoints`.
- Examples:
  - `/str 100` — Adds 100 points to Strength.
  - `/agi 50` — Adds 50 points to Agility.
  - `/vit 80` — Adds 80 points to Vitality.
  - `/ene 150` — Adds 150 points to Energy.

### Automated Stat Distribution

You can configure automatic stat point distribution as you level up. Whenever your character gains level-up points, the server automatically assigns them until reaching your defined quota.

```text
/<stat> auto <target_amount>
```

- Supported stats: `/str auto`, `/agi auto`, `/vit auto`, `/ene auto`.
- `<target_amount>`: How many points the server should allocate towards this stat.
- To disable auto-allocation for a specific stat, pass `0`:
  - `/str auto 0`
  - `/agi auto 0`
- Example configuration:
  ```text
  /str auto 300
  /agi auto 200
  /vit auto 100
  /ene auto 500
  ```
  Points gained from monster kills will automatically balance across the active goals until each target is fulfilled.

### Character Reset

Once your character reaches the realm reset level requirement, reset your level back to 1 to receive extra reset points.

```text
/reset
```

- **Level Requirement by Difficulty Realm**:
  - 🟢 **Easy Realm**: Level **300**
  - 🟡 **Medium Realm**: Level **400**
  - 🔴 **Hard Realm**: Level **400**
- **Requirements & Effects**:
  - Resets character Level to **1**.
  - Adds **+1** to character reset count.
  - Grants bonus stat points according to reset table and difficulty settings.
  - Teleports character back to starting safe-zone town (Lorencia, Noria, or Devias).
  - Equipped items and inventory items are preserved.

### Automated Reset Loop

Set up an automated grinding loop that automatically performs a reset the exact moment your character hits max level, and automatically assigns your reset stat points:

```text
/reset auto <str> <agi> <vit> <ene>
```

- `<str>`: Points automatically fed to Strength on each reset.
- `<agi>`: Points automatically fed to Agility on each reset.
- `<vit>`: Points automatically fed to Vitality on each reset.
- `<ene>`: Points automatically fed to Energy on each reset.
- **How it works**:
  - As soon as your character reaches the required reset level, the server automatically resets the character without disconnecting or opening menus, and immediately begins assigning points to the designated attributes.
- **To toggle OFF**:
  - Simply type `/reset auto` without arguments to disable the automated reset loop.

### Grand Reset

High-end milestone reset that trades a large batch of standard resets for exclusive rewards and Grand Reset points:

```text
/grandreset
```

- **Resets Milestone Requirement**:
  - 🟢 **Easy Realm**: **50 Resets**
  - 🟡 **Medium Realm**: **100 Resets**
  - 🔴 **Hard Realm**: **150 Resets**
- **Effects**:
  - Level resets to **1**.
  - Resets counter resets to **0** (or **1** depending on server setup).
  - Increments your character's `GrandReset` counter.
  - Grants Grand Reset stat points.

### PK / Murderer Clear

If your character has killed other players and obtained Outlaw / Murderer status (orange or red name), guards will attack on sight and you cannot warp via `/move`.

```text
/pkclear
```

- Instantly resets your status back to `Commoner` (normal name color).

### Class Evolution

Instantly completes the Sebina the Priestess quest line (Quest 1 and Quest 2) without needing to search for the *Scroll of Emperor*, *Broken Sword*, *Tear of Elf*, or *Soul Shard of Wizard*.

```text
/evo
```

- **Eligible Classes**:
  - Dark Knight ➔ **Blade Knight**
  - Dark Wizard ➔ **Soul Master**
  - Fairy Elf ➔ **Muse Elf**
- Magic Gladiators (MG) are already advanced classes and do not require this command.

### Multi-Vault Warehouse

MU Online Solo Classic provides up to **5 independent vaults** per character difficulty slot, allowing you to organize items, sets, and jewels.

```text
/ware <0..4>
```

- `<0..4>`: The vault index (0, 1, 2, 3, or 4).
- Must be used while the warehouse window is **closed**.
- Example:
  - `/ware 0` — Primary vault.
  - `/ware 1` — Secondary vault (e.g. for Jewels & Wings).
  - `/ware 2` — Third vault (e.g. for Ancient / Excellent Armor sets).

### Inventory Cleaner

Clear out accumulated junk drops from your inventory bag with one quick command:

```text
/clearinv
```

- Cleans all item slots inside your standard inventory bags.
- **Safety guarantee**: Your equipped gear (Helm, Armor, Pants, Gloves, Boots, Left/Right Hand Weapons, Shields, Wings, Pet/Mount, Rings, Pendant) is **never touched**.

### Map Teleportation

Teleport your character directly to any supported map or dungeon gateway:

```text
/move <MapName>
```

- Valid destination names:
  `Lorencia`, `Noria`, `Devias`, `Devias2`, `Devias3`, `Devias4`, `Dungeon`, `Dungeon2`, `Dungeon3`, `Atlans`, `Atlans2`, `Atlans3`, `LostTower`, `LostTower2`, `LostTower3`, `LostTower4`, `LostTower5`, `LostTower6`, `LostTower7`, `Tarkan`, `Tarkan2`, `Icarus`, `Stadium`.
- See the [Move Destinations Table](#move-destinations-table) for level and Zen costs.
- Special conditions:
  - Moving to `Icarus` requires equipped **Wings** or **Horn of Dinorant**.
  - Moving to `Atlans` cannot be performed while mounted on a **Horn of Uniria**.
  - Murderers (PK status) cannot use `/move`.

### Global Chat Broadcast

Send a server-wide announcement that is visible to every player currently in the world:

```text
/post <message>
```

- Example:
  ```text
  /post Buying Excellent Dragon Armor +Luck with Zen!
  ```

### Request Filters & Auto-Party

Control how your character responds to incoming Trade, Party, and Guild invitations:

```text
/re <on | off | auto [password]>
```

- `/re on`: Enable incoming requests (default).
- `/re off`: Automatically reject all incoming trade, party, and guild requests.
- `/re auto [password]`: Automatically accepts party requests. If an optional password is provided (e.g. `/re auto 1234`), only party invites from players who know the password will be accepted.

### Guild Warfare

Commands available to Guild Masters:

- `/war <GuildName>`: Issue a formal Guild War challenge to `<GuildName>`.
- `/soccer <GuildName>`: Challenge `<GuildName>` to a Battle Soccer match inside the Arena Stadium field.

### Online Status

```text
/onlines
```

- Displays the total number of connected players currently active in the realm.

---

## 4. Game Master (GM) & Admin Commands

Game Master commands require account authorization configured in the server's data files.

### GM Authorization & Configuration

To enable GM rights for an account or character, add an entry to [`src/server/Data/Util/GameMaster.txt`](file:///c:/Work/_Projects/mu-online-solo-classic/src/server/Data/Util/GameMaster.txt):

```text
//Account   Name        Level
"admin"     "GmTest"    2
"myaccount" "MyChar"    2
end
```

- `Level 1`: Game Master (can use general moderation, drop, and fireworks commands).
- `Level 2`: Administrator (unrestricted access to all GM commands).

---

### Zen Management

```text
/addmoney <amount>
/submoney <amount>
```

- `/addmoney 100000000` — Adds 100,000,000 Zen to your character (maximum wallet capacity is 2,000,000,000 Zen).
- `/submoney 50000000` — Deducts 50,000,000 Zen from your character.

---

### Moderation & Teleportation

#### Teleport Target Player
```text
/gmove <PlayerName> <MapNumber> <X> <Y>
```
- Transports the specified player to the target map and coordinates.
- Example: `/gmove TestPlayer 0 125 125` (moves player to Lorencia Bar).

#### Teleport All Connected Players
```text
/gmoveall <MapNumber> <X> <Y>
```
- Warps every active player on the server to the target location (useful for community events or boss fights).
- Example: `/gmoveall 6 64 116` (warps all players to Arena Stadium center).

#### Chase Player
```text
/chase <PlayerName>
```
- Teleports the GM directly to `<PlayerName>`'s current coordinates.

#### Bring Player
```text
/bring <PlayerName>
```
- Summons `<PlayerName>` directly to the GM's current location.

#### Disconnect / Kick Player
```text
/disconnect <PlayerName>
```
- Closes the connection for `<PlayerName>` within 6 seconds.

---

### Item Creation & Spawning

#### `/make` — Spawn Item Into Inventory

Creates one or more items directly into the GM's inventory bag:

```text
/make [-quantity] <Group> <Index> <Level> <Skill> <Luck> <Option> <Excel>
```

##### Parameter Breakdown:
- `[-quantity]`: *(Optional)* If the first number is negative, it specifies how many copies to create (e.g. `-10` creates 10 items). If omitted or positive, 1 item is created.
- `<Group>`: Item category (0 to 15, see [Item Groups Table](#item-group-ids-table)).
- `<Index>`: Item ID within that category (0 to 511).
- `<Level>`: Item level (from `0` to `11` in classic 0.97k).
- `<Skill>`: Weapon / Shield skill (`1` = Yes, `0` = No).
- `<Luck>`: Item luck (`1` = Yes [+25% critical damage rate / +25% jewel success], `0` = No).
- `<Option>`: Additional option level:
  - `0` = +0 (No option)
  - `1` = +4 (Attack/Defense +4)
  - `2` = +8 (Attack/Defense +8)
  - `3` = +12 (Attack/Defense +12)
  - `4` = +16 (Attack/Defense +16)
- `<Excel>`: Excellent option bitmask:
  - `0` = Normal item (not excellent)
  - `63` = Full Option (All 6 excellent options applied)
  - (See [Excellent Option Bitmask Table](#excellent-option-bitmask-table) for custom combinations)

##### Examples:
```text
// 1x Divine Sword of Archangel +11 +Skill +Luck +Option 16 +Full Excellent
/make 0 19 11 1 1 4 63

// 1x Wings of Dragon +11 +Luck +Option 16
/make 12 5 11 0 1 4 0

// 10x Jewel of Bless
/make -10 14 13 0 0 0 0 0

// 10x Jewel of Soul
/make -10 14 14 0 0 0 0 0

// 10x Jewel of Life
/make -10 14 16 0 0 0 0 0

// 10x Jewel of Chaos
/make -10 12 15 0 0 0 0 0
```

#### `/makeset` — Spawn Complete 5-Piece Armor Set

Generates an entire 5-piece armor set (Helm, Armor, Pants, Gloves, Boots) matching the given set type directly into your inventory:

```text
/makeset <SetIndex> <Level> <Skill> <Luck> <Option> <Excel>
```

- `<SetIndex>`: The set identifier (see [Armor Set IDs Table](#armor-set-ids-table)).
- Examples:
  ```text
  // Full Great Dragon Set +11 +Luck +Option 16 +Full Excellent
  /makeset 21 11 1 1 4 63

  // Full Dark Phoenix Set +11 +Luck +Option 16 +Full Excellent
  /makeset 17 11 1 1 4 63

  // Full Grand Soul Set +11 +Luck +Option 16 +Full Excellent
  /makeset 18 11 1 1 4 63

  // Full Divine Set +11 +Luck +Option 16 +Full Excellent
  /makeset 19 11 1 1 4 63

  // Full Thunder Hawk Set +11 +Luck +Option 16 +Full Excellent (for MG)
  /makeset 20 11 1 1 4 63
  ```

#### `/drop` — Drop Item On Ground

Drops an item onto the floor directly under the GM's coordinates:

```text
/drop <Group> <Index> <Level> <Skill> <Luck> <Option> <Excel>
```

- Takes identical parameters to `/make`, but places the item on the map floor instead of your inventory.
- Example:
  ```text
  /drop 0 16 11 1 1 4 63  // Drops a Sword of Destruction +11+Skill+Luck+16+FO on the ground
  ```

---

### Monster Spawning & Morphing

#### Spawn Monsters (`/makemob`)

```text
/makemob <MonsterID> [Quantity]
```

- `<MonsterID>`: Monster type ID from `Monster.txt` (see [Common Monster IDs Table](#common-monster-ids-table)).
- `[Quantity]`: Number of monsters to spawn (defaults to 1 if omitted).
- Spawns in random free tiles within 3 cells of the GM.
- Monsters have an automatic lifespan of **30 minutes** (1,800,000 ms) before disappearing if not slain.
- Examples:
  - `/makemob 38 1` — Spawns 1 Balrog.
  - `/makemob 44 2` — Spawns 2 Dragons.
  - `/makemob 53 5` — Spawns 5 Golden Titans.

#### Morph Character Model (`/skin`)

```text
/skin <PlayerName> <MonsterID>
```

- `<PlayerName>`: Target character name.
- `<MonsterID>`: Monster model ID to transform into.
- Use `0` to restore standard character appearance.
- Example:
  - `/skin GmTest 40` — Transforms into a Death Knight!
  - `/skin GmTest 0` — Reverts back to normal player model.

---

### Events & Celebrations

#### Celebratory Fireworks (`/fireworks`)
```text
/fireworks
```
- Detonates 15 dazzling firework burst effects in an area around the GM.

#### Yellow Server Broadcast (`/global`)
```text
/global <message>
```
- Broadcasts a yellow GM announcement visible to all players:
  `[ServerName][GM_Name] <message>`

#### Tamachan Invasion Trigger (`/tamastart`)
```text
/tamastart
```
- Manually initiates the custom "Tamachan" invasion event without waiting for the automatic event timer (begins in ~2 minutes).

---

## 5. Appendix & Reference Tables

### Move Destinations Table

All destinations supported by `/move <MapName>`:

| Destination Name | Map | Gate | Min Level | Zen Cost | Note |
| :--- | :---: | :---: | :---: | :---: | :--- |
| `Lorencia` | Lorencia | 17 | 10 | 1,000 | Main starter town |
| `Noria` | Noria | 27 | 10 | 1,000 | Fairy town & Chaos Goblin |
| `Devias` | Devias | 22 | 20 | 1,500 | Snow town |
| `Devias2` | Devias | 72 | 20 | 2,000 | Mid-field |
| `Devias3` | Devias | 73 | 20 | 2,500 | Elite Yeti area |
| `Devias4` | Devias | 74 | 20 | 2,500 | Church / Ice Queen entrance |
| `Dungeon` | Dungeon 1 | 2 | 30 | 3,000 | Dungeon floor 1 |
| `Dungeon2` | Dungeon 2 | 6 | 40 | 3,000 | Dungeon floor 2 |
| `Dungeon3` | Dungeon 3 | 10 | 50 | 3,500 | Gorgon's Lair floor 3 |
| `Atlans` | Atlans | 49 | 70 | 4,000 | Underwater realm entrance (No Uniria) |
| `Atlans2` | Atlans | 75 | 80 | 4,500 | Mid-ocean |
| `Atlans3` | Atlans | 76 | 90 | 5,000 | Hydra's domain |
| `LostTower` | Lost Tower 1 | 42 | 50 | 5,000 | Lost Tower entrance |
| `LostTower2` | Lost Tower 2 | 31 | 50 | 5,000 | Floor 2 |
| `LostTower3` | Lost Tower 3 | 33 | 50 | 5,500 | Floor 3 |
| `LostTower4` | Lost Tower 4 | 35 | 60 | 5,500 | Floor 4 |
| `LostTower5` | Lost Tower 5 | 37 | 60 | 5,500 | Floor 5 |
| `LostTower6` | Lost Tower 6 | 39 | 70 | 6,000 | Floor 6 |
| `LostTower7` | Lost Tower 7 | 41 | 70 | 6,500 | Balrog's Throne Room |
| `Tarkan` | Tarkan | 57 | 140 | 8,000 | Desert entrance |
| `Tarkan2` | Tarkan | 77 | 140 | 8,500 | Deep Desert / Zaikan spot |
| `Icarus` | Icarus | 63 | 170 | 9,000 | Sky Realm (Requires Wings/Dinorant) |
| `Stadium` | Arena | 50 | 255 | 3,000 | Battle Stadium |

---

### Item Group IDs Table

Used for the first parameter of `/make` and `/drop`:

| Group ID | Category | Examples |
| :---: | :--- | :--- |
| **0** | Swords & Daggers | Kris, Blade, Lightning Sword, Sword of Destruction, Knight Blade, Archangel Sword |
| **1** | Axes | Small Axe, Double Axe, Chaos Dragon Axe |
| **2** | Maces & Scepters | Morning Star, Crystal Morning Star, Chaos Lightning Staff |
| **3** | Spears & Polearms | Dragon Lance, Great Scythe, Serpent Spear |
| **4** | Bows & Crossbows | Tiger Bow, Silver Bow, Chaos Nature Bow, Celestial Bow, Arrow Viper Bow |
| **5** | Wizard Staffs | Skull Staff, Legendary Staff, Resurrection Staff, Chaos Staff, Kundun Staff |
| **6** | Shields | Round Shield, Dragon Slayer Shield, Legendary Shield, Grand Soul Shield |
| **7** | Helms | Dragon Helm, Legendary Helm, Dark Phoenix Helm, Great Dragon Helm |
| **8** | Armors | Dragon Armor, Legendary Armor, Dark Phoenix Armor, Great Dragon Armor |
| **9** | Pants | Dragon Pants, Legendary Pants, Dark Phoenix Pants, Great Dragon Pants |
| **10** | Gloves | Dragon Gloves, Legendary Gloves, Dark Phoenix Gloves, Great Dragon Gloves |
| **11** | Boots | Dragon Boots, Legendary Boots, Dark Phoenix Boots, Great Dragon Boots |
| **12** | Wings & Orbs | Wings of Heaven (1), Satan (2), Spirits (3), Soul (4), Dragon (5), Darkness (6), Jewel of Chaos (15) |
| **13** | Pets & Accessories | Guardian Angel (0), Imp (1), Horn of Uniria (2), Dinorant (3), Rings, Pendants |
| **14** | Jewels & Consumables| Jewel of Bless (13), Jewel of Soul (14), Jewel of Life (16), Jewel of Creation (22), Potions |
| **15** | Magic Spell Scrolls | Scroll of Evil Spirit (8), Hellfire (9), Aqua Beam (11), Soul Barrier (15) |

---

### Armor Set IDs Table

Used with `/makeset <SetIndex> ...`:

| Set Index | Set Name | Suitable Class |
| :---: | :--- | :--- |
| **0** | Bronze Set | Dark Knight (DK) / Magic Gladiator (MG) |
| **1** | Dragon Set | Dark Knight (DK) / Magic Gladiator (MG) |
| **2** | Pad Set | Dark Wizard (DW) / Magic Gladiator (MG) |
| **3** | Legendary Set | Dark Wizard (DW) / Magic Gladiator (MG) |
| **4** | Bone Set | Dark Wizard (DW) / Magic Gladiator (MG) |
| **5** | Leather Set | Dark Knight (DK) / Magic Gladiator (MG) |
| **6** | Scale Set | Dark Knight (DK) / Magic Gladiator (MG) |
| **7** | Sphinx Set | Dark Wizard (DW) / Magic Gladiator (MG) |
| **8** | Brass Set | Dark Knight (DK) / Magic Gladiator (MG) |
| **9** | Plate Set | Dark Knight (DK) / Magic Gladiator (MG) |
| **10** | Vine Set | Fairy Elf (FE) |
| **11** | Silk Set | Fairy Elf (FE) |
| **12** | Wind Set | Fairy Elf (FE) |
| **13** | Spirit Set | Fairy Elf (FE) |
| **14** | Guardian Set | Fairy Elf (FE) |
| **15** | Storm Crow Set | Magic Gladiator (MG) |
| **16** | Black Dragon Set | Blade Knight (BK) |
| **17** | Dark Phoenix Set | Blade Knight (BK) |
| **18** | Grand Soul Set | Soul Master (SM) |
| **19** | Divine Set | Muse Elf (ME) |
| **20** | Thunder Hawk Set | Magic Gladiator (MG) |
| **21** | Great Dragon Set | Blade Knight (BK) |

---

### Excellent Option Bitmask Table

To build custom Excellent items with `/make`, `/drop`, or `/makeset`, sum the bit values for the options you want:

| Bit | Value | Weapon / Pendant Effect | Armor / Shield / Ring Effect |
| :---: | :---: | :--- | :--- |
| **0** | **1** | Mana recovery after monster die +Mana/8 | Zen drop rate increase +40% |
| **1** | **2** | Life recovery after monster die +Life/8 | Defense success rate +10% |
| **2** | **4** | Attack / Wizardry increase +Level/20 | Reflect damage +5% |
| **3** | **8** | Attack / Wizardry increase +2% | Damage decrease -4% |
| **4** | **16**| Attack / Casting Speed +7 | Maximum Mana +4% |
| **5** | **32**| Excellent Damage Rate +10% | Maximum Life +4% |
| **ALL**| **63**| **All 6 Options (Full Option)** | **All 6 Options (Full Option)** |

#### Common Combinations:
- `32` = Excellent Damage Rate only (PVP basic).
- `48` (32 + 16) = Excellent Damage Rate + Attack Speed +7.
- `56` (32 + 16 + 8) = Excellent Damage Rate + Speed + 2% Damage.
- `63` (1 + 2 + 4 + 8 + 16 + 32) = Full Excellent (FO).
- `12` (4 + 8) = Reflec 5% + Damage Decrease 4% (PvP defense combo).
- `14` (2 + 4 + 8) = Rate 10% + Reflect 5% + Decrease 4% (Popular armor combo).

---

### Common Item Creation Examples

```text
// ==========================================
// JEWELS (Group 14 & 12)
// ==========================================
/make -20 14 13 0 0 0 0 0     // 20x Jewel of Bless
/make -20 14 14 0 0 0 0 0     // 20x Jewel of Soul
/make -20 14 16 0 0 0 0 0     // 20x Jewel of Life
/make -20 12 15 0 0 0 0 0     // 20x Jewel of Chaos
/make -10 14 22 0 0 0 0 0     // 10x Jewel of Creation

// ==========================================
// WINGS (Group 12) - All Level +11 +Luck +16 Option
// ==========================================
/make 12 1 11 0 1 4 0         // Wings of Heaven (DW)
/make 12 2 11 0 1 4 0         // Wings of Satan (DK)
/make 12 3 11 0 1 4 0         // Wings of Spirits (Elf)
/make 12 4 11 0 1 4 0         // Wings of Soul (SM)
/make 12 5 11 0 1 4 0         // Wings of Dragon (BK)
/make 12 6 11 0 1 4 0         // Wings of Darkness (MG)

// ==========================================
// TOP WEAPONS (Group 0, 4, 5) - +11 +Skill +Luck +16 +Full Excellent
// ==========================================
/make 0 19 11 1 1 4 63        // Divine Sword of Archangel
/make 0 20 11 1 1 4 63        // Knight Blade
/make 0 16 11 1 1 4 63        // Sword of Destruction
/make 0 18 11 1 1 4 63        // Thunder Blade (MG)
/make 4 18 11 1 1 4 63        // Arrow Viper Bow (Elf)
/make 4 19 11 1 1 4 63        // Celestial Bow (Elf)
/make 5 11 11 1 1 4 63        // Kundun Staff (SM)
/make 5 9 11 1 1 4 63         // Staff of Destruction (SM)

// ==========================================
// MOUNTS & ACCESSORIES (Group 13)
// ==========================================
/make 13 3 0 1 0 0 0          // Horn of Dinorant
/make 13 2 0 0 0 0 0          // Horn of Uniria
/make 13 0 0 0 0 0 0          // Guardian Angel
/make 13 1 0 0 0 0 0          // Imp
/make 13 8 0 0 0 0 63         // Ring of Ice (Full Option)
/make 13 9 0 0 0 0 63         // Ring of Poison (Full Option)
/make 13 12 0 0 0 0 63        // Pendant of Lightning (Full Option)
```

---

### Common Monster IDs Table

Used with `/makemob <MonsterID>` and `/skin <player> <MonsterID>`:

| Monster ID | Monster Name | Area / Event |
| :---: | :--- | :--- |
| **0** | Bull Fighter | Lorencia |
| **2** | Budge Dragon | Lorencia |
| **3** | Spider | Lorencia |
| **7** | Giant | Lorencia |
| **14** | Skeleton | Lorencia / Dungeon |
| **18** | Gorgon (Boss) | Dungeon 3 |
| **19** | Yeti | Devias |
| **20** | Elite Yeti | Devias 3 |
| **25** | Ice Queen (Boss) | Devias 4 |
| **26** | Goblin | Noria |
| **38** | Balrog (Boss) | Lost Tower 7 |
| **40** | Death Knight | Lost Tower 6-7 |
| **42** | Red Dragon | Dragon Invasion |
| **43** | Golden Budge Dragon | Golden Invasion |
| **44** | Dragon | Lorencia / Lost Tower |
| **49** | Hydra (Boss) | Atlans 3 |
| **53** | Golden Titan | Golden Invasion |
| **54** | Golden Soldier | Golden Invasion |
| **55** | Death King | Special Event |
| **57** | Iron Wheel | Tarkan |
| **59** | Zaikan (Boss) | Tarkan 2 |
| **77** | Phoenix of Darkness (Boss) | Icarus |

---

### Map Numbers Table

Used with `/gmove` and `/gmoveall`:

| Map Number | Map Name |
| :---: | :--- |
| **0** | Lorencia |
| **1** | Dungeon |
| **2** | Devias |
| **3** | Noria |
| **4** | Lost Tower |
| **5** | Exile |
| **6** | Arena / Stadium |
| **7** | Atlans |
| **8** | Tarkan |
| **9** | Devil Square |
| **10** | Icarus |
| **11 – 16** | Blood Castle (1 to 6) |

---

### Chat Channels Prefix Operators

In addition to slash (`/`) commands, standard chat communication supports prefix characters:

- **General Chat**: Type message normally (visible to nearby players in your viewport).
- `~<message>`: **Party Chat** (visible only to members of your current party).
- `@<message>`: **Guild Chat** (visible only to members of your guild).
- `@><message>`: **Guild Notice** (Guild Master only; broadcasts a notification to all guild members).
