# Diablo II Save File Format

**Orginal text at https://github.com/krisives/d2s-format**

Diablo II stores your game character on disk as a .d2s file. This
is a binary file format that encodes all of the stats, items, name,
and other pieces of data.

Integers are stored in [little endian](https://en.wikipedia.org/wiki/Endianness)
byte order, which is the native byte ordering on a x86 architecture Diablo II
is based on.

## Header

Each .d2s file starts with a 765 byte header, after which data
is of variable length.

|Byte | Length | Desc
|-----|--------|------------
|0    | 4      | Signature (0xaa55aa55)
|4    | 4      | [Version ID](#versions)
|8    | 4      | File size
|12   | 4      | [Checksum](#checksum)
|16   | 4      | [Active Weapon](#active-weapon)
|20   | 16     | [Character Name](#character-name)
|36   | 1      | [Character Status](#character-status)
|37   | 1      | [Character Progression](#Character-progression)
|38   | 2      | ?
|40   | 1      | [Character Class](#character-class)
|41   | 2      | ?
|43   | 1      | [Level](#level)
|44   | 4      | Created [unix timestamp](https://en.wikipedia.org/wiki/Unix_time)?
|48   | 4      | Last Played [unix timestamp](https://en.wikipedia.org/wiki/Unix_time)
|52   | 4      | ?
|56   | 64     | [Assigned Skills](#AssignedSkills)
|120  | 4      | Left Mouse
|124  | 4      | Right Mouse
|128  | 4      | Left Mouse (weapon switch)
|132  | 4      | Right Mouse (weapon switch)
|136  | 32     | [Character Menu Appearance](#character-menu-appearance)
|168  | 3      | [Difficulty](#difficulty)
|171  | 4      | Map ID
|175  | 2      | ?
|177  | 2      | Merc dead?
|179  | 4      | Merc seed?
|183  | 2      | Merc Name ID
|185  | 2      | Merc Type
|187  | 4      | Merc Experience
|191  | 144    | ?
|335  | 298    | [Quest](#quest)
|633  | 80     | [Waypoint](#waypoint)
|713  | 52     | [NPC](#npc)
|765  |        | [Attributes](#attributes)
|     | 30     | [Skills](#skills)
|     |        | [Items](#items)

### Versions

File version. The following values are known:

* `71` is 1.00 through v1.06
* `87` is 1.07 or Expansion Set v1.08
* `89` is standard game v1.08
* `92` is v1.09 (both the standard game and the Expansion Set.)
* `96` is v1.10 - v1.14d
* `96` is v1.15+ (D2R)

### Checksum

To calculate the checksum set the value of it in the .d2s data
to be zero and iterate through all the bytes in the data calculating
a 32-bit checksum:

<details><summary>code in C</summary>

```C
    sum = (sum << 1) + data[i];
```

</details>

<details><summary>code in JavaScript</summary>

source: https://github.com/krisives/d2s-format/issues/5

```js
const fs = require("fs");
const path = require("path");
const file = path.join(process.cwd(), "path_to_save.d2s");

function calculateSum(data) {
  let sum = 0;
  for (let i = 0; i < data.length; i++) {
    let ch = data[i];
    if (i >= 12 && i < 16) {
      ch = 0;
    }
    ch += sum < 0;
    sum = (sum << 1) + ch;
  }

  return sum;
}

function littleToBigEndian(number) {
  return new DataView(
    Int32Array.of(
      new DataView(Int32Array.of(number).buffer).getUint32(0, true)
    ).buffer
  );
}

function ashex(buffer) {
  return buffer.getUint32(0, false).toString(16);
}

async function readSafeFile() {
  return await new Promise((resolve, reject) => {
    fs.readFile(file, (err, data) => {
      if (err) return reject(err);
      return resolve(data);
    });
  });
}

async function writeCheckSumToSafeFile(data) {
  return await new Promise((resolve, reject) => {
    fs.writeFile(file, data, err => {
      if (err) reject(err);
      resolve();
    });
  });
}

readSafeFile().then(data => {
  const sum = calculateSum(data);
  const bufferSum = littleToBigEndian(sum);
  const hex = ashex(bufferSum);
  const newData = data;
  for (let i = 0; i < 4; i++) {
    newData[12 + i] = bufferSum.getInt8(i);
  }
  writeCheckSumToSafeFile(newData).then(() => console.log(hex));
});
```

</details>

<details><summary>code in <a href="https://golang.org">golang</a></summary>

source: https://github.com/gucio321/d2d2s/blob/66f91e2af7b3949ca7f279aae397bd8904519e2d/pkg/d2s/d2s.go#L397

```golang
// CalculateChecksum calculates a checksum and saves in a byte slice 
func CalculateChecksum(data *[]byte) {
        var sum uint32
        for i := range *data {
                sum = ((sum << 1) % math.MaxUint32) | (sum >> (int32Size*byteLen - 1))

                sum += uint32((*data)[i])
        }

        sumBytes := make([]byte, int32Size)
        binary.LittleEndian.PutUint32(sumBytes, sum)

        const (
                int32Size = 4
                checksumPosition = 12
        )
        for i := 0; i < int32Size; i++ {
                (*data)[checksumPosition+i] = sumBytes[i]
        }
}
```

</details>

If the checksum is invalid, Diablo II will not open the save file.

### Active Weapon

This value is either 0 or 1 indicating which weapons/shield/spell setup is in use 

### Character Name

Character names are store as an array of 16 characters which contain
a null terminated string padded with `0x00` for the remaining bytes.
Characters are stored as 8-bit ASCII, but remember that valid must
follow these rules:
 * Must be 2-15 in length
 * Must begin with a letter
 * May contain up to one hyphen (`-`) or underscore (`_`)
 * May contain letters

### Character Status

This is a 8-bit field:

Bit | Desc
----|------
0   | ?
1   | ?
2   | Hardcore
3   | Died (if hardcore, idicates character can't be played)
4   | ?
5   | Expansion
6   | Ladder
7   | ?

### Character Progression

This is a 8-bit field representing the Title your character gets after completing a difficulty.  Non-Expansion has upper 2 bits set (00, 01, 10 or 11), while Expansion has both the upper and lower 2 bits set to the same 2 bit value (00, 01, 10 or 11)

Value | Standard (Classic)                   | Hardcore (Classic)
----- |--------------------------------------|----------------------------
0x00  | No Title                             | No Title 
0x10  | Slayer (Sir/Dame)                    | Destroyer (Count/Countess)
0x01  | Champion(Lord/Lady)                  | Conqueror (Duke/Duchess)
0x11  | Patriarch/Matriarch (Baron/Baroness) | Guardian (King/Queen)

### Character Class

ID | Class
---|-------
0  | Amazon
1  | Sorceress
2  | Necromancer
3  | Paladin
4  | Barbarian
5  | Druid
6  | Assassin

### Level

This level value is visible only in character select screen
and must be the same as this in [Stats](#stats) section.

### Assigned skills
Assigned skills section is a an array of 16 skill ids mapped to a hotkey, each a `4 byte` integer (uint32). If no skill is assigned the value is `0x00`.

### Character Menu Appearance
32 byte structure which defines how the character looks in the menu
Does not change in-game look


### Difficulty
3 bytes of data that indicates which of the three difficulties the character has unlocked.
Each byte is representitive of one of the difficulties. In this order:
Normal, Nightmare, and Hell.
Each byte is a bitfield structured like this:

 7      | 6 | 5 | 4 | 3 | 2, 1, 0
--------|---|---|---|---|-----------
Active? |Unknown|Unknown|Unknown|Unknown|Which act (0-4)?



### Quest
The quests struct is `298 byte` section that describes all quests in the game but also contains data about act traveling and NPC introductions. Each quest is `2 byte` long.

#### Header
| Offset | Bytes | Content |
|--------|-------|---------|
| 335    | 4     | Woo!    |
| 339    | 6     | Unknown |

#### Quest structs
A quest is `2 byte` long, I've created a general `quest` struct that holds the most important data of a quest, if it's completed or not. Each quest has a lot of unique bits set depending on different milestones of the quest. For example if you've consumed the Scroll of Resistance from the quest "Prison of Ice" or not.

#### Important Bits 
| Bit | Description                       |
|-----|-----------------------------------|
| 0   | Quest completed                   |
| 1   | Requirements Met                  |
| 2   | Quest Given                       |
| 6   | Drank potion of Life (Act III)    |
| 8   | Read Scroll of Resistance (Act V) |
| 11  | Secret Cow Level Complete (Act I) |
| 12  | Quest Closed                      |
| 13  | Quest Completed in Current Game   |

#### Quest structure
This structure repeats it self 3 times, once for Normal, Nightmare and Hell. The offset is the offset into the quest struct.

| Offset | Bytes      | Description                                                                             |
|--------|------------|-----------------------------------------------------------------------------------------|
| 0      | `2`        | Set to `1` if you have been introduced to Warriv in Act I.                              |
| 2      | `[6]quest` | All six quests for Act I.                                                               |
| 14     | `2`        | Set to `1` if you have traveled to Act II.                                              |
| 16     | `2`        | Set to `1` if you have been introduced to Jerhyn.                                       |
| 18     | `[6]quest` | All six quests for Act II.                                                              |
| 30     | `2`        | Set to `1` if you have traveled to Act III.                                             |
| 32     | `2`        | Set to `1` if you have been introduced to Hratli.                                       |
| 34     | `[6]quest` | All six quests for Act III.                                                             |
| 46     | `2`        | Set to `1` if you have traveled to Act IV.                                              |
| 48     | `2`        | Set to `1` if you have been introduced to Act IV. (which you have if you have traveled) |
| 50     | `[6]quest` | Act IV only has 3 quests, so the struct has 6 empty bytes here.                         |
| 62     | `2`        | Set to `1` if you have traveled to Act V.                                               |
| 64     | `2`        | Seems to be set to 1 after completing Terror's End and talking to Cain in act IV.       |
| 66     | `4`        | Seems to be some kind of padding.                                                       |
| 70     | `[6]quest` | All six quests for Act V.                                                               |
| 82     | `1`        | Set to 1 if you went to Akara to reset your stats already                               |
| 83     | `1`        | Seems to be set to 0x80 after completing the Difficulty Level                           |
| 84     | `12`         | Some kind of padding after all the quest data.                                        |


### Waypoint

Waypoint data starts with 2 chars "WS" and
6 unknown bytes, always = {0x01, 0x00, 0x00, 0x00, 0x50, 0x00}

Three structures are in place for each difficulty,
at offsets 641, 665 and 689.

The contents of this structure are as follows

byte | bytesize | contents
-----|----------|---------
  0  | 2 bytes  | {0x02, 0x01} Unknown purpose
  2  | 5 bytes  | Waypoint bitfield in order of least significant bit
  7  | 17 bytes | unknown

In the waypoint bitfield, a bit value of 1 means that the waypoint is enabled
It is in an order from lowest to highest, so 0 is Rogue encampment (ACT I) etc.
The first waypoint in each difficulty is always activated.

### NPC

TODO


### Attributes
Following the header is the attributes section, this sections layout consists of an array of  `9 bit` attribute id, followed by a `n bit` length attribute value. The section is terminated by a `9 bit` value of `0x1ff`. It's worth mentioning that these fields are [bit reversed](bitreader.go#L69). Basically if you find the bits `00100111` they are reversed into `11100100`.  

#### Attribute IDs
| ID | Attribute       |
|----|-----------------|
| 0  | Strength        |
| 1  | Energy          |
| 2  | Dexterity       |
| 3  | Vitality        |
| 4  | Unused stats    |
| 5  | Unused skills   |
| 6  | Current HP      |
| 7  | Max HP          |
| 8  | Current mana    |
| 9  | Max mana        |
| 10 | Current stamina |
| 11 | Max stamina     |
| 12 | Level           |
| 13 | Experience      |
| 14 | Gold            |
| 15 | Stashed gold    |

#### Attribute bit lengths
| Bit length | Attribute       |
|------------|-----------------|
| 10         | Strength        |
| 10         | Energy          |
| 10         | Dexterity       |
| 10         | Vitality        |
| 10         | Unused stats    |
| 8          | Unused skills   |
| 21         | Current HP      |
| 21         | Max HP          |
| 21         | Current mana    |
| 21         | Max mana        |
| 21         | Current stamina |
| 21         | Max stamina     |
| 7          | Level           |
| 32         | Experience      |
| 25         | Gold            |
| 25         | Stashed gold    |

#### Example of the reading

```go
for {
    // 1. read 9 bits id. (reverse them)
    // 2. if the id is 0x1ff, terminate the loop
    // 3. read bit length from attribute map for that id.
    // 4. read bit length nr of bits. 
}
```
### Skills
Skills are a `32 byte` section containing a `2 byte` header with the value `if` and `30 byte` of skill data. Each class has 30 skills available to them, so each skill get `1 byte` each. The tricky part about the skill mapping is that each class has a different offset into the [skill map](skills.go#L29) where their class specific skills start, and then go 30 indexes into the map. So for example Assassin has an offset of `251`. Which means Assassin skills are  between the indexes of `251` and `281` which is exactly 30 indexes.

##### Layout
|  Type  | Bytes | Value                     |
|:------:|:-----:|---------------------------|
| Header | `2`   | `if`                      |
| Skills | `30`  | [[30]skill](skills.go#L29) |

##### Skill offset map
|    Class    | Offset |
|:-----------:|:------:|
| Amazon      | `6`    |
| Sorceress   | `36`   |
| Necromancer | `66`   |
| Paladin     | `96`   |
| Barbarian   | `126`  |
| Druid       | `221`  |
| Assassin    | `251`  |



### Items

Items are stored in lists described by this header:

Byte | Size | Desc
-----|------|-----
0    | 2    | "JM"
2    | 2    | Item Count

After this come N items. Each item starts with a basic 14-byte
structure. Many fields in this structure are not "byte-aligned"
and are described by their bit position and sizes.

Bit | Size | Desc
----|------|------
0   | 16   | "JM" (separate from the list header)
16  | 4    | ?
20  | 1    | Identified
21  | 6    | ?
27  | 1    | Socketed
28  | 1    | ?
29  | 1    | Picked up since last save
30  | 2    | ?
32  | 1    | Ear
33  | 1    | Starter Gear
34  | 3    | ?
37  | 1    | Compact
38  | 1    | Ethereal
39  | 1    | ?
40  | 1    | Personalized
41  | 1    | ?
42  | 1    | [Runeword](#runeword)
43  | 15   | ?
58  | 3    | [Parent](#parent)
61  | 4    | [Equipped](#equipped)
65  | 4    | Column
69  | 3    | Row
72  | 1    | ?
73  | 3    | [Stash](#parent)
76  | 4    | ?
80  | 24   | Type code (3 letters)
108 |      | [Extended Item Data](#extended-item-data)

### Extended Item Data

If the item is marked as `Compact` (bit 37 is set) no extended
item information will exist and the item is finished.

Items with extended information store bits based on information in the item header. For example, an item marked as `Socketed` will store an
extra 3-bit integer encoding how many sockets the item has.

|Bit | Size | Desc
----|------|-------
|108 |      | [Sockets](#sockets)
|    |      | [Custom Graphics](#custom-graphics)
|    |      | [Class Specific](#class-specific)
|    |      | [Quality](#quality)
|    |      | [Mods](#mods)

### Custom Graphics

Custom graphics are denoted by a single bit, which if
set means a 3-bit number for the graphic index follows. If the
bit is not set the 3-bits are not present.

Bit | Size | Desc
----|------|------
0   | 1    | Item has custom graphics
1   | 3    | Alternate graphic index

### Class Specific

Class items like Barbarian helms or Amazon bows have special
properties specific to those kinds of items. If the first bit
is empty the remaining 11 bits will not be present.

Bit | Size | Desc
----|------|------
0   | 1    | Item has class specific data
1   | 11   | Class specific bits

### Quality

Item quality is encoded as a 4-bit integer.

Value | Desc
------|------
1     | Inferior
2     | Normal
3     | Superior
4     | Magically Enhanced
5     | Part of a Set
6     | Rare
7     | Unique
8     | Craft
9     | Tempered

#### Low Quality


### Mods

After each item is a list of mods. The list is a series of key value
pairs where the key is a 9-bit number and the value depends on the key.
The list ends when key `511` (`0x1ff`) is found which is all 9-bits
being set.

Using the file `ItemStatCost.txt` as a tab-delimited CSV file you can
extract the `ID` column which maps to the 9-bit keys. The columns
`Save Bits` and `Param Bits` describe how large the mod is.

The only exception is min-max style modifiers which use the next row
in the CSV to store the "max" portion of the mod. The bit sizes of
these two can be different and you should sum them to get the total
size.

#### Runeword

TODO

#### Parent

All items are located somewhere and have a "parent" which
can be another item, such as when inserting a jewel.

Value | Desc
------|------
0     | Stored
1     | Equipped
2     | Belt
4     | Cursor
6     | Item

For items that are "stored" a 3-bit integer encoded starting at
bit 73 describes where to store the item:

Value | Desc
------|------
1     | Inventory
4     | Horadric Cube
5     | Stash

### Equipped

Items that are equipped describe their slot:

Value | Slot
------|-----
1     | Helmet
2     | Amulet
3     | Armor
4     | Weapon (Right)
5     | Weapon (Left)
6     | Ring (Right)
7     | Ring (Left)
8     | Belt
9     | Boots
10    | Gloves
11    | Alternate Weapon (Right)
12    | Alternate Weapon (Left)

### Sockets