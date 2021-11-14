/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
    Copyright (C) 2021 Walter Couto

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------

#include "pch.h"
#include <map>
#include <bitset>
#include <sstream>
#include <map>
#include "Item.h"
#include "SkillConstants.h"

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint16_t MAX_TOME_QUANTITY = 20;     // max # of scrolls in tome
    constexpr std::uint16_t MAX_KEY_QUANTITY = 12;      // max # of keys stacked
    constexpr std::uint16_t MAX_GLD_QUANTITY = 4095;    // max gld amount
    constexpr std::uint16_t MAX_STACKED_QUANTITY = 511; // max # of items in a stack
    constexpr std::uint16_t MAX_DURABILITY = 0xFF;      // max durability of an item (0 is Indestructible)

    constexpr std::uint8_t ITEM_MARKER[] = { 0x4A, 0x4D };        // alternatively "JM"
    constexpr std::uint8_t MERC_ITEM_MARKER[] = { 0x6A, 0x66 };   // alternatively "jf"
    constexpr std::uint8_t GOLEM_ITEM_MARKER[] = { 0x6B, 0x66 };  // alternatively "jk"

    constexpr std::uint32_t MIN_START_STATS_POS = 641;

#define readtemp_bits(data,start,size) \
    ((*((std::uint64_t*) &(data)[(start) / 8]) >> ((start) & 7))& (((std::uint64_t)1 << (size)) - 1))

#define read_uint32_bits(start,size) \
    ((*((std::uint32_t *) &data[(start) / 8]) >> ((start) & 7))& (((std::uint32_t)1 << (size)) - 1))

#define read_uint64_bits(start,size) \
    ((*((std::uint64_t*) &data[(start) / 8]) >> ((start) & 7))& (((std::uint64_t)1 << (size)) - 1))

#define set_bit(start) \
    ((data[(start) / 8]) |= (std::uint8_t)(1ul << ((start) & 7)))

#define clear_bit(start) \
    ((data[(start) / 8]) &= ~(std::uint8_t)(1ul << ((start) & 7)))

    struct OpAttributes
    {
        std::uint8_t op = 0;
        std::uint8_t op_param = 0;
        std::string op_base;
        std::vector<std::string> op_stats;
    };

    struct ItemStat
    {
        std::uint16_t id = 0;
        std::uint8_t encode = 0;
        std::uint16_t saveBits = 0;
        std::int16_t saveAdd = 0;
        std::uint16_t saveParamBits = 0;
        std::uint16_t nextInChain = 0;
        std::uint16_t charSaveBits = 0;
        std::string desc;
        std::string name;
        OpAttributes opAttribs;
    };

    const std::vector<ItemStat> itemStats = {
        {  0, 0,  8,  32,  0,  0, 10,                                                     "+{0} to Strength",                        "strength"},
        {  1, 0,  7,  32,  0,  0, 10,                                                       "+{0} to Energy",                          "energy"},
        {  2, 0,  7,  32,  0,  0, 10,                                                    "+{0} to Dexterity",                       "dexterity"},
        {  3, 0,  7,  32,  0,  0, 10,                                                     "+{0} to Vitality",                        "vitality"},
        {  4, 0,  0,   0,  0,  0, 10,                                                                     "",                         "statpts"},
        {  5, 0,  0,   0,  0,  0,  8,                                                                     "",                       "newskills"},
        {  6, 0,  0,   0,  0,  0, 21,                                                                     "",                       "hitpoints"},
        {  7, 0,  9,  32,  0,  0, 21,                                                         "+{0} to Life",                           "maxhp"},
        {  8, 0,  0,   0,  0,  0, 21,                                                                     "",                            "mana"},
        {  9, 0,  8,  32,  0,  0, 21,                                                         "+{0} to Mana",                         "maxmana"},
        { 10, 0,  0,   0,  0,  0, 21,                                                                     "",                         "stamina"},
        { 11, 0,  8,  32,  0,  0, 21,                                              "+{0} to Maximum Stamina",                      "maxstamina"},
        { 12, 0,  0,   0,  0,  0,  7,                                                                     "",                           "level"},
        { 13, 0,  0,   0,  0,  0, 32,                                                                     "",                      "experience"},
        { 14, 0,  0,   0,  0,  0, 25,                                                                     "",                            "gold"},
        { 15, 0,  0,   0,  0,  0, 25,                                                                     "",                        "goldbank"},
        { 16, 0,  9,   0,  0,  0,  0,                                               "+{0}% Enhanced Defense",              "item_armor_percent"},
        { 17, 0,  9,   0,  0, 18,  0,                                                "+{0}% Enhanced Damage",          "item_maxdamage_percent"},
        { 18, 0,  9,   0,  0,  0,  0,                                                                     "",          "item_mindamage_percent"},
        { 19, 0, 10,   0,  0,  0,  0,                                                "+{0} to Attack Rating",                           "tohit"},
        { 20, 0,  6,   0,  0,  0,  0,                                   "+{0}% Increased Chance of Blocking",                         "toblock"},
        { 21, 0,  6,   0,  0,  0,  0,                                               "+{0} to Minimum Damage",                       "mindamage"},
        { 22, 0,  7,   0,  0,  0,  0,                                               "+{0} to Maximum Damage",                       "maxdamage"},
        { 23, 0,  6,   0,  0,  0,  0,                                               "+{0} to Minimum Damage",             "secondary_mindamage"},
        { 24, 0,  7,   0,  0,  0,  0,                                               "+{0} to Maximum Damage",             "secondary_maxdamage"},
        { 25, 0,  8,   0,  0,  0,  0,                                                                     "",                   "damagepercent"},
        { 26, 0,  8,   0,  0,  0,  0,                                                                     "",                    "manarecovery"},
        { 27, 0,  8,   0,  0,  0,  0,                                                 "Regenerate Mana {0}%",               "manarecoverybonus"},
        { 28, 0,  8,   0,  0,  0,  0,                                               "Heal Stamina Plus {0}%",            "staminarecoverybonus"},
        { 29, 0,  0,   0,  0,  0,  0,                                                                     "",                         "lastexp"},
        { 30, 0,  0,   0,  0,  0,  0,                                                                     "",                         "nextexp"},
        { 31, 0, 11,  10,  0,  0,  0,                                                         "+{0} Defense",                      "armorclass"},
        { 32, 0,  9,   0,  0,  0,  0,                                             "+{0} Defense vs. Missile",           "armorclass_vs_missile"},
        { 33, 0,  8,   0,  0,  0,  0,                                               "+{0} Defense vs. Melee",               "armorclass_vs_hth"},
        { 34, 0,  6,   0,  0,  0,  0,                                                "Damage Reduced by {0}",         "normal_damage_reduction"},
        { 35, 0,  6,   0,  0,  0,  0,                                          "Magic Damage Reduced by {0}",          "magic_damage_reduction"},
        { 36, 0,  8,   0,  0,  0,  0,                                               "Damage Reduced by {0}%",                    "damageresist"},
        { 37, 0,  8,   0,  0,  0,  0,                                                   "Magic Resist +{0}%",                     "magicresist"},
        { 38, 0,  5,   0,  0,  0,  0,                                        "+{0}% to Maximum Magic Resist",                  "maxmagicresist"},
        { 39, 0,  8,  50,  0,  0,  0,                                                    "Fire Resist +{0}%",                      "fireresist"},
        { 40, 0,  5,   0,  0,  0,  0,                                         "+{0}% to Maximum Fire Resist",                   "maxfireresist"},
        { 41, 0,  8,  50,  0,  0,  0,                                               "Lightning Resist +{0}%",                     "lightresist"},
        { 42, 0,  5,   0,  0,  0,  0,                                    "+{0}% to Maximum Lightning Resist",                  "maxlightresist"},
        { 43, 0,  8,  50,  0,  0,  0,                                                    "Cold Resist +{0}%",                      "coldresist"},
        { 44, 0,  5,   0,  0,  0,  0,                                         "+{0}% to Maximum Cold Resist",                   "maxcoldresist"},
        { 45, 0,  8,  50,  0,  0,  0,                                                  "Poison Resist +{0}%",                    "poisonresist"},
        { 46, 0,  5,   0,  0,  0,  0,                                       "+{0}% to Maximum Poison Resist",                 "maxpoisonresist"},
        { 47, 0,  0,   0,  0,  0,  0,                                                                     "",                      "damageaura"},
        { 48, 0,  8,   0,  0, 49,  0,                                             "Adds {0}-{1} Fire Damage",                      "firemindam"},
        { 49, 0,  9,   0,  0,  0,  0,                                                     "+{0} Fire Damage",                      "firemaxdam"},
        { 50, 0,  6,   0,  0, 51,  0,                                        "Adds {0}-{1} Lightning Damage",                     "lightmindam"},
        { 51, 0, 10,   0,  0,  0,  0,                                                "+{0} Lightning Damage",                     "lightmaxdam"},
        { 52, 0,  8,   0,  0, 53,  0,                                            "Adds {0}-{1} Magic Damage",                     "magicmindam"},
        { 53, 0,  9,   0,  0,  0,  0,                                                    "+{0} Magic Damage",                     "magicmaxdam"},
        { 54, 0,  8,   0,  0, 55,  0,                                             "Adds {0}-{1} Cold Damage",                      "coldmindam"},
        { 55, 0,  9,   0,  0, 56,  0,                                                     "+{0} Cold Damage",                      "coldmaxdam"},
        { 56, 0,  8,   0,  0,  0,  0,                                                                     "",                      "coldlength"},
        { 57, 0, 10,   0,  0, 58,  0,                          "Adds {0}-{1} Poison Damage over {2} Seconds",                    "poisonmindam"},
        { 58, 0, 10,   0,  0, 59,  0,                                  "+{0} Poison Damage over {1} Seconds",                    "poisonmaxdam"},
        { 59, 0,  9,   0,  0,  0,  0,                                                                     "",                    "poisonlength"},
        { 60, 0,  7,   0,  0,  0,  0,                                             "{0}% Life Stolen Per Hit",                 "lifedrainmindam"},
        { 61, 0,  0,   0,  0,  0,  0,                                                                     "",                 "lifedrainmaxdam"},
        { 62, 0,  7,   0,  0,  0,  0,                                             "{0}% Mana Stolen Per Hit",                 "manadrainmindam"},
        { 63, 0,  0,   0,  0,  0,  0,                                                                     "",                 "manadrainmaxdam"},
        { 64, 0,  0,   0,  0,  0,  0,                                                                     "",                 "stamdrainmindam"},
        { 65, 0,  0,   0,  0,  0,  0,                                                                     "",                 "stamdrainmaxdam"},
        { 66, 0,  0,   0,  0,  0,  0,                                                                     "",                      "stunlength"},
        { 67, 0,  7,  30,  0,  0,  0,                                                                     "",                 "velocitypercent"},
        { 68, 0,  7,  30,  0,  0,  0,                                                                     "",                      "attackrate"},
        { 69, 0,  0,   0,  0,  0,  0,                                                                     "",                  "other_animrate"},
        { 70, 0,  0,   0,  0,  0,  0,                                                                     "",                        "quantity"},
        { 71, 0,  8, 100,  0,  0,  0,                                                                     "",                           "value"},
        { 72, 0,  9,   0,  0,  0,  0,                                                                     "",                      "durability"},
        { 73, 0,  8,   0,  0,  0,  0,                                              "+{0} Maximum Durability",                   "maxdurability"},
        { 74, 0,  6,  30,  0,  0,  0,                                                  "Replenish Life +{0}",                         "hpregen"},
        { 75, 0,  7,  20,  0,  0,  0,                                     "Increase Maximum Durability {0}%",      "item_maxdurability_percent"},
        { 76, 0,  6,  10,  0,  0,  0,                                           "Increase Maximum Life {0}%",              "item_maxhp_percent"},
        { 77, 0,  6,  10,  0,  0,  0,                                           "Increase Maximum Mana {0}%",            "item_maxmana_percent"},
        { 78, 0,  7,   0,  0,  0,  0,                                         "Attacker Takes Damage of {0}",        "item_attackertakesdamage"},
        { 79, 0,  9, 100,  0,  0,  0,                                        "{0}% Extra Gold from Monsters",                  "item_goldbonus"},
        { 80, 0,  8, 100,  0,  0,  0,                            "{0}% Better Chance of Getting Magic Items",                 "item_magicbonus"},
        { 81, 0,  7,   0,  0,  0,  0,                                                            "Knockback",                  "item_knockback"},
        { 82, 0,  9,  20,  0,  0,  0,                                                                     "",               "item_timeduration"},
        { 83, 0,  3,   0,  3,  0,  0,                                             "+{1} to {0} Skill Levels",             "item_addclassskills"},
        { 84, 0,  0,   0,  0,  0,  0,                                                                     "",                    "unsentparam1"},
        { 85, 0,  9,  50,  0,  0,  0,                                            "{0}% to Experience Gained",              "item_addexperience"},
        { 86, 0,  7,   0,  0,  0,  0,                                            "+{0} Life After Each Kill",              "item_healafterkill"},
        { 87, 0,  7,   0,  0,  0,  0,                                                  "Reduces Prices {0}%",              "item_reducedprices"},
        { 88, 0,  1,   0,  0,  0,  0,                                                                     "",         "item_doubleherbduration"},
        { 89, 0,  4,   4,  0,  0,  0,                                                 "+{0} to Light Radius",                "item_lightradius"},
        { 90, 0, 24,   0,  0,  0,  0,                                                        "Ambient light",                 "item_lightcolor"},
        { 91, 0,  8, 100,  0,  0,  0,                                                    "Requirements {0}%",                "item_req_percent"},
        { 92, 0,  7,   0,  0,  0,  0,                                                                     "",                   "item_levelreq"},
        { 93, 0,  7,  20,  0,  0,  0,                                          "{0}% Increased Attack Speed",           "item_fasterattackrate"},
        { 94, 0,  7,  64,  0,  0,  0,                                                                     "",                "item_levelreqpct"},
        { 95, 0,  0,   0,  0,  0,  0,                                                                     "",                  "lastblockframe"},
        { 96, 0,  7,  20,  0,  0,  0,                                                 "{0}% Faster Run/Walk",         "item_fastermovevelocity"},
        { 97, 1,  6,   0,  9,  0,  0,                                                          "+{1} To {0}",              "item_nonclassskill"},
        { 98, 0,  1,   0,  8,  0,  0,                                                                     "",                           "state"},
        { 99, 0,  7,  20,  0,  0,  0,                                             "{0}% Faster Hit Recovery",           "item_fastergethitrate"},
        {100, 0,  0,   0,  0,  0,  0,                                                                     "",             "monster_playercount"},
        {101, 0,  0,   0,  0,  0,  0,                                                                     "",    "skill_poison_override_length"},
        {102, 0,  7,  20,  0,  0,  0,                                               "{0}% Faster Block Rate",            "item_fasterblockrate"},
        {103, 0,  0,   0,  0,  0,  0,                                                                     "",             "skill_bypass_undead"},
        {104, 0,  0,   0,  0,  0,  0,                                                                     "",             "skill_bypass_demons"},
        {105, 0,  7,  20,  0,  0,  0,                                                "{0}% Faster Cast Rate",             "item_fastercastrate"},
        {106, 0,  0,   0,  0,  0,  0,                                                                     "",             "skill_bypass_beasts"},
        {107, 1,  3,   0,  9,  0,  0,                                                          "+{1} To {0}",                "item_singleskill"},
        {108, 0,  1,   0,  0,  0,  0,                                         "Slain Monsters Rest in Peace",                "item_restinpeace"},
        {109, 0,  9,   0,  0,  0,  0,                                                                     "",                "curse_resistance"},
        {110, 0,  8,  20,  0,  0,  0,                                        "Poison Length Reduced by {0}%",         "item_poisonlengthresist"},
        {111, 0,  9,  20,  0,  0,  0,                                                          "Damage +{0}",               "item_normaldamage"},
        {112, 0,  7,  -1,  0,  0,  0,                                     "Hit Causes Monsters to Flee {0}%",                       "item_howl"},
        {113, 0,  7,   0,  0,  0,  0,                                               "Hit Blinds Target +{0}",                  "item_stupidity"},
        {114, 0,  6,   0,  0,  0,  0,                                       "{0}% Damage Taken Goes to Mana",               "item_damagetomana"},
        {115, 0,  1,   0,  0,  0,  0,                                                "Ignore Target Defense",             "item_ignoretargetac"},
        {116, 0,  7,   0,  0,  0,  0,                                                 "-{0}% Target Defense",         "item_fractionaltargetac"},
        {117, 0,  7,   0,  0,  0,  0,                                                 "Prevent Monster Heal",                "item_preventheal"},
        {118, 0,  1,   0,  0,  0,  0,                                                 "Half Freeze Duration",         "item_halffreezeduration"},
        {119, 0,  9,  20,  0,  0,  0,                                          "{0}% Bonus to Attack Rating",              "item_tohit_percent"},
        {120, 0,  7, 128,  0,  0,  0,                                       "{0} to Monster Defense Per Hit",             "item_damagetargetac"},
        {121, 0,  9,  20,  0,  0,  0,                                               "+{0}% Damage to Demons",        "item_demondamage_percent"},
        {122, 0,  9,  20,  0,  0,  0,                                               "+{0}% Damage to Undead",       "item_undeaddamage_percent"},
        {123, 0, 10, 128,  0,  0,  0,                                 "+{0} to Attack Rating against Demons",                "item_demon_tohit"},
        {124, 0, 10, 128,  0,  0,  0,                                 "+{0} to Attack Rating against Undead",               "item_undead_tohit"},
        {125, 0,  1,   0,  0,  0,  0,                                                            "Throwable",                  "item_throwable"},
        {126, 0,  3,   0,  3,  0,  0,                                                  "+{0} to Fire Skills",                  "item_elemskill"},
        {127, 0,  3,   0,  0,  0,  0,                                                   "+{0} to All Skills",                  "item_allskills"},
        {128, 0,  5,   0,  0,  0,  0,                               "Attacker Takes Lightning Damage of {0}",   "item_attackertakeslightdamage"},
        {129, 0,  0,   0,  0,  0,  0,                                                                     "",                "ironmaiden_level"},
        {130, 0,  0,   0,  0,  0,  0,                                                                     "",                   "lifetap_level"},
        {131, 0,  0,   0,  0,  0,  0,                                                                     "",                  "thorns_percent"},
        {132, 0,  0,   0,  0,  0,  0,                                                                     "",                       "bonearmor"},
        {133, 0,  0,   0,  0,  0,  0,                                                                     "",                    "bonearmormax"},
        {134, 0,  5,   0,  0,  0,  0,                                                  "Freezes Target +{0}",                     "item_freeze"},
        {135, 0,  7,   0,  0,  0,  0,                                           "{0}% Chance of Open Wounds",                 "item_openwounds"},
        {136, 0,  7,   0,  0,  0,  0,                                         "{0}% Chance of Crushing Blow",               "item_crushingblow"},
        {137, 0,  7,   0,  0,  0,  0,                                                     "+{0} Kick Damage",                 "item_kickdamage"},
        {138, 0,  7,   0,  0,  0,  0,                                         "+{0} to Mana after each Kill",              "item_manaafterkill"},
        {139, 0,  7,   0,  0,  0,  0,                                      "+{0} Life after each Demon Kill",         "item_healafterdemonkill"},
        {140, 0,  7,   0,  0,  0,  0,                                                                     "",                 "item_extrablood"},
        {141, 0,  7,   0,  0,  0,  0,                                                   "{0}% Deadly Strike",               "item_deadlystrike"},
        {142, 0,  7,   0,  0,  0,  0,                                                     "Fire Absorb {0}%",         "item_absorbfire_percent"},
        {143, 0,  7,   0,  0,  0,  0,                                                     "+{0} Fire Absorb",                 "item_absorbfire"},
        {144, 0,  7,   0,  0,  0,  0,                                                "Lightning Absorb {0}%",        "item_absorblight_percent"},
        {145, 0,  7,   0,  0,  0,  0,                                                "+{0} Lightning Absorb",                "item_absorblight"},
        {146, 0,  7,   0,  0,  0,  0,                                                    "Magic Absorb {0}%",        "item_absorbmagic_percent"},
        {147, 0,  7,   0,  0,  0,  0,                                                    "+{0} Magic Absorb",                "item_absorbmagic"},
        {148, 0,  7,   0,  0,  0,  0,                                                     "Cold Absorb {0}%",         "item_absorbcold_percent"},
        {149, 0,  7,   0,  0,  0,  0,                                                     "+{0} Cold Absorb",                 "item_absorbcold"},
        {150, 0,  7,   0,  0,  0,  0,                                                 "Slows Target by {0}%",                       "item_slow"},
        {151, 0,  5,   0,  9,  0,  0,                                         "Level +{1} {0} When Equipped",                       "item_aura"},
        {152, 0,  1,   0,  0,  0,  0,                                                       "Indestructible",            "item_indesctructible"},
        {153, 0,  1,   0,  0,  0,  0,                                                     "Cannot Be Frozen",             "item_cannotbefrozen"},
        {154, 0,  7,  20,  0,  0,  0,                                            "{0}% Slower Stamina Drain",            "item_staminadrainpct"},
        {155, 0,  7,   0, 10,  0,  0,                                      "{0}% Chance to Reanimate Target",                  "item_reanimate"},
        {156, 0,  7,   0,  0,  0,  0,                                                      "Piercing Attack",                     "item_pierce"},
        {157, 0,  7,   0,  0,  0,  0,                                                   "Fires Magic Arrows",                 "item_magicarrow"},
        {158, 0,  7,   0,  0,  0,  0,                                      "Fires Explosive Arrows or Bolts",             "item_explosivearrow"},
        {159, 0,  6,   0,  0,  0,  0,                                         "+{0} to Minimum Throw Damage",            "item_throw_mindamage"},
        {160, 0,  7,   0,  0,  0,  0,                                         "+{0} to Maximum Throw Damage",            "item_throw_maxdamage"},
        {161, 0,  0,   0,  0,  0,  0,                                                                     "",              "skill_handofathena"},
        {162, 0,  0,   0,  0,  0,  0,                                                                     "",            "skill_staminapercent"},
        {163, 0,  0,   0,  0,  0,  0,                                                                     "",    "skill_passive_staminapercent"},
        {164, 0,  0,   0,  0,  0,  0,                                                                     "",             "skill_concentration"},
        {165, 0,  0,   0,  0,  0,  0,                                                                     "",                   "skill_enchant"},
        {166, 0,  0,   0,  0,  0,  0,                                                                     "",                    "skill_pierce"},
        {167, 0,  0,   0,  0,  0,  0,                                                                     "",                "skill_conviction"},
        {168, 0,  0,   0,  0,  0,  0,                                                                     "",             "skill_chillingarmor"},
        {169, 0,  0,   0,  0,  0,  0,                                                                     "",                    "skill_frenzy"},
        {170, 0,  0,   0,  0,  0,  0,                                                                     "",                 "skill_decrepify"},
        {171, 0,  0,   0,  0,  0,  0,                                                                     "",             "skill_armor_percent"},
        {172, 0,  0,   0,  0,  0,  0,                                                                     "",                       "alignment"},
        {173, 0,  0,   0,  0,  0,  0,                                                                     "",                         "target0"},
        {174, 0,  0,   0,  0,  0,  0,                                                                     "",                         "target1"},
        {175, 0,  0,   0,  0,  0,  0,                                                                     "",                        "goldlost"},
        {176, 0,  0,   0,  0,  0,  0,                                                                     "",                "conversion_level"},
        {177, 0,  0,   0,  0,  0,  0,                                                                     "",                "conversion_maxhp"},
        {178, 0,  0,   0,  0,  0,  0,                                                                     "",                  "unit_dooverlay"},
        {179, 0,  9,   0, 10,  0,  0,                                                                     "",               "attack_vs_montype"},
        {180, 0,  9,   0, 10,  0,  0,                                                                     "",               "damage_vs_montype"},
        {181, 0,  3,   0,  0,  0,  0,                                                                     "",                            "fade"},
        {182, 0,  0,   0,  0,  0,  0,                                                                     "",          "armor_override_percent"},
        {183, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused183"},
        {184, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused184"},
        {185, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused185"},
        {186, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused186"},
        {187, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused187"},
        {188, 4,  3,   0, 16,  0,  0,                                        "+{2} to {0} Skills ({1} Only)",               "item_addskill_tab"},
        {189, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused189"},
        {190, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused190"},
        {191, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused191"},
        {192, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused192"},
        {193, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused193"},
        {194, 0,  4,   0,  0,  0,  0,                                   "Adds {0} extra sockets to the item",                 "item_numsockets"},
        {195, 2,  7,   0, 16,  0,  0,                          "{2}% Chance to cast level {0} {1} on attack",              "item_skillonattack"},
        {196, 2,  7,   0, 16,  0,  0,             "{2}% Chance to cast level {0} {1} when you Kill an Enemy",                "item_skillonkill"},
        {197, 2,  7,   0, 16,  0,  0,                       "{2}% Chance to cast level {0} {1} when you Die",               "item_skillondeath"},
        {198, 2,  7,   0, 16,  0,  0,                        "{2}% Chance to cast level {0} {1} on striking",                 "item_skillonhit"},
        {199, 2,  7,   0, 16,  0,  0,                  "{2}% Chance to cast level {0} {1} when you Level-Up",             "item_skillonlevelup"},
        {200, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused200"},
        {201, 2,  7,   0, 16,  0,  0,                        "{2}% Chance to cast level {0} {1} when struck",              "item_skillongethit"},
        {202, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused202"},
        {203, 0,  0,   0,  0,  0,  0,                                                                     "",                       "unused203"},
        {204, 3, 16,   0, 16,  0,  0,                                      "Level {0} {1} ({2}/{3} Charges)",              "item_charged_skill"},
        {205, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused204"},
        {206, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused205"},
        {207, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused206"},
        {208, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused207"},
        {209, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused208"},
        {210, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused209"},
        {211, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused210"},
        {212, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused211"},
        {213, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused212"},
        {214, 0,  6,   0,  0,  0,  0,                           "+{0} to Defense (Based on Character Level)",             "item_armor_perlevel", {4, 3, "level", {"armorclass"}}},
        {215, 0,  6,   0,  0,  0,  0,                     "{0}% Enhanced Defense (Based on Character Level)",      "item_armorpercent_perlevel", {5, 3, "level", {"item_armorpercent"}}},
        {216, 0,  6,   0,  0,  0,  0,                              "+{0} to Life (Based on Character Level)",                "item_hp_perlevel", {2, 3, "level", {"maxhp"}}},
        {217, 0,  6,   0,  0,  0,  0,                              "+{0} to Mana (Based on Character Level)",              "item_mana_perlevel", {2, 3, "level", {"maxmana"}}},
        {218, 0,  6,   0,  0,  0,  0,                    "+{0} to Maximum Damage (Based on Character Level)",         "item_maxdamage_perlevel", {4, 3, "level", {"maxdamage", "secondary_maxdamage","item_throw_maxdamage"}}},
        {219, 0,  6,   0,  0,  0,  0,              "{0}% Enhanced Maximum Damage (Based on Character Level)", "item_maxdamage_percent_perlevel", {5, 3, "level", {"item_maxdamage_percent", "secondary_maxdamage","item_throw_maxdamage"}}},
        {220, 0,  6,   0,  0,  0,  0,                          "+{0} to Strength (Based on Character Level)",          "item_strength_perlevel", {2, 3, "level", {"strength"}}},
        {221, 0,  6,   0,  0,  0,  0,                         "+{0} to Dexterity (Based on Character Level)",         "item_dexterity_perlevel", {2, 3, "level", {"dexterity"}}},
        {222, 0,  6,   0,  0,  0,  0,                            "+{0} to Energy (Based on Character Level)",            "item_energy_perlevel", {2, 3, "level", {"energy"}}},
        {223, 0,  6,   0,  0,  0,  0,                          "+{0} to Vitality (Based on Character Level)",          "item_vitality_perlevel", {2, 3, "level", {"vitality"}}},
        {224, 0,  6,   0,  0,  0,  0,                     "+{0} to Attack Rating (Based on Character Level)",             "item_tohit_perlevel", {2, 1, "level", {"tohit"}}},
        {225, 0,  6,   0,  0,  0,  0,               "{0}% Bonus to Attack Rating (Based on Character Level)",      "item_tohitpercent_perlevel", {2, 1, "level", {"item_tohit_percent"}}},
        {226, 0,  6,   0,  0,  0,  0,                          "+{0} Cold Damage (Based on Character Level)",    "item_cold_damagemax_perlevel", {2, 3, "level", {"coldmaxdam"}}},
        {227, 0,  6,   0,  0,  0,  0,                          "+{0} Fire Damage (Based on Character Level)",    "item_fire_damagemax_perlevel", {2, 3, "level", {"firemaxdam"}}},
        {228, 0,  6,   0,  0,  0,  0,                     "+{0} Lightning Damage (Based on Character Level)",    "item_ltng_damagemax_perlevel", {2, 3, "level", {"lightmaxdam"}}},
        {229, 0,  6,   0,  0,  0,  0,                        "+{0} Poison Damage (Based on Character Level)",    "item_pois_damagemax_perlevel", {2, 3, "level", {"poisonmaxdam"}}},
        {230, 0,  6,   0,  0,  0,  0,                         "Cold Resist +{0}% (Based on Character Level)",       "item_resist_cold_perlevel", {2, 3, "level", {"coldresist"}}},
        {231, 0,  6,   0,  0,  0,  0,                         "Fire Resist +{0}% (Based on Character Level)",       "item_resist_fire_perlevel", {2, 3, "level", {"fireresist"}}},
        {232, 0,  6,   0,  0,  0,  0,                    "Lightning Resist +{0}% (Based on Character Level)",       "item_resist_ltng_perlevel", {2, 3, "level", {"lightresist"}}},
        {233, 0,  6,   0,  0,  0,  0,                       "Poison Resist +{0}% (Based on Character Level)",       "item_resist_pois_perlevel", {2, 3, "level", {"poisonresist"}}},
        {234, 0,  6,   0,  0,  0,  0,                          "+{0} Cold Absorb (Based on Character Level)",       "item_absorb_cold_perlevel", {2, 3, "level", {"item_absorbcold"}}},
        {235, 0,  6,   0,  0,  0,  0,                          "+{0} Fire Absorb (Based on Character Level)",       "item_absorb_fire_perlevel", {2, 3, "level", {"item_absorbfire"}}},
        {236, 0,  6,   0,  0,  0,  0,                     "+{0} Lightning Absorb (Based on Character Level)",       "item_absorb_ltng_perlevel", {2, 3, "level", {"item_absorblight"}}},
        {237, 0,  6,   0,  0,  0,  0,                         "{0} Poison Absorb (Based on Character Level)",       "item_absorb_pois_perlevel", {2, 3, "level", {"item_absorbmagic"}}},
        {238, 0,  5,   0,  0,  0,  0,              "Attacker Takes Damage of {0} (Based on Character Level)",            "item_thorns_perlevel", {2, 3, "level", {"item_attackertakesdamage"}}},
        {239, 0,  6,   0,  0,  0,  0,              "{0}% Extra Gold from Monster (Based on Character Level)",         "item_find_gold_perlevel", {2, 3, "level", {"item_goldbonus"}}},
        {240, 0,  6,   0,  0,  0,  0, "{0}% Better Chance of Getting Magic Items (Based on Character Level)",        "item_find_magic_perlevel", {2, 3, "level", {"item_magicbonus"}}},
        {241, 0,  6,   0,  0,  0,  0,                    "Heal Stamina Plus {0}% (Based on Character Level)",      "item_regenstamina_perlevel", {2, 3, "level", {"staminarecoverybonus"}}},
        {242, 0,  6,   0,  0,  0,  0,                      "+{0} Maxmium Stamina (Based on Character Level)",           "item_stamina_perlevel", {2, 3, "level", {"maxstamina"}}},
        {243, 0,  6,   0,  0,  0,  0,                     "{0}% Damage to Demons (Based on Character Level)",      "item_damage_demon_perlevel", {2, 3, "level", {"item_demondamage_percent"}}},
        {244, 0,  6,   0,  0,  0,  0,                     "{0}% Damage to Undead (Based on Character Level)",     "item_damage_undead_perlevel", {2, 3, "level", {"item_undeaddamage_percent"}}},
        {245, 0,  6,   0,  0,  0,  0,      "+{0} to Attack Rating against Demons (Based on Character Level)",       "item_tohit_demon_perlevel", {2, 1, "level", {"item_demon_tohit"}}},
        {246, 0,  6,   0,  0,  0,  0,      "+{0} to Attack Rating against Undead (Based on Character Level)",      "item_tohit_undead_perlevel", {2, 1, "level", {"item_undead_tohit"}}},
        {247, 0,  6,   0,  0,  0,  0,              "{0}% Chance of Crushing Blow (Based on Character Level)",      "item_crushingblow_perlevel", {2, 3, "level", {"item_crushingblow"}}},
        {248, 0,  6,   0,  0,  0,  0,                "{0}% Chance of Open Wounds (Based on Character Level)",        "item_openwounds_perlevel", {2, 3, "level", {"item_openwounds"}}},
        {249, 0,  6,   0,  0,  0,  0,                          "+{0} Kick Damage (Based on Character Level)",       "item_kick_damage_perlevel", {2, 3, "level", {"item_kickdamage"}}},
        {250, 0,  6,   0,  0,  0,  0,                     "{0}% to Deadly Strike (Based on Character Level)",      "item_deadlystrike_perlevel", {2, 3, "level", {"item_deadlystrike"}}},
        {251, 0,  0,   0,  0,  0,  0,                                                                     "",         "item_find_gems_perlevel"},
        {252, 0,  6,   0,  0,  0,  0,                                  "Repairs 1 Durability in {0} Seconds",       "item_replenish_durability"},
        {253, 0,  6,   0,  0,  0,  0,                                                 "Replenishes quantity",         "item_replenish_quantity"},
        {254, 0,  8,   0,  0,  0,  0,                                                 "Increased Stack Size",                "item_extra_stack"},
        {255, 0,  0,   0,  0,  0,  0,                                                                     "",                  "item_find_item"},
        {256, 0,  0,   0,  0,  0,  0,                                                                     "",               "item_slash_damage"},
        {257, 0,  0,   0,  0,  0,  0,                                                                     "",       "item_slash_damage_percent"},
        {258, 0,  0,   0,  0,  0,  0,                                                                     "",               "item_crush_damage"},
        {259, 0,  0,   0,  0,  0,  0,                                                                     "",       "item_crush_damage_percent"},
        {260, 0,  0,   0,  0,  0,  0,                                                                     "",              "item_thrust_damage"},
        {261, 0,  0,   0,  0,  0,  0,                                                                     "",      "item_thrust_damage_percent"},
        {262, 0,  0,   0,  0,  0,  0,                                                                     "",               "item_absorb_slash"},
        {263, 0,  0,   0,  0,  0,  0,                                                                     "",               "item_absorb_crush"},
        {264, 0,  0,   0,  0,  0,  0,                                                                     "",              "item_absorb_thrust"},
        {265, 0,  0,   0,  0,  0,  0,                                                                     "",       "item_absorb_slash_percent"},
        {266, 0,  0,   0,  0,  0,  0,                                                                     "",       "item_absorb_crush_percent"},
        {267, 0,  0,   0,  0,  0,  0,                                                                     "",      "item_absorb_thrust_percent"},
        {268, 4, 22,   0,  0,  0,  0,                                                                     "",               "item_armor_bytime"},
        {269, 4, 22,   0,  0,  0,  0,                                                                     "",        "item_armorpercent_bytime"},
        {270, 4, 22,   0,  0,  0,  0,                                                                     "",                  "item_hp_bytime"},
        {271, 4, 22,   0,  0,  0,  0,                                                                     "",                "item_mana_bytime"},
        {272, 4, 22,   0,  0,  0,  0,                                                                     "",           "item_maxdamage_bytime"},
        {273, 4, 22,   0,  0,  0,  0,                                                                     "",   "item_maxdamage_percent_bytime"},
        {274, 4, 22,   0,  0,  0,  0,                                                                     "",            "item_strength_bytime"},
        {275, 4, 22,   0,  0,  0,  0,                                                                     "",           "item_dexterity_bytime"},
        {276, 4, 22,   0,  0,  0,  0,                                                                     "",              "item_energy_bytime"},
        {277, 4, 22,   0,  0,  0,  0,                                                                     "",            "item_vitality_bytime"},
        {278, 4, 22,   0,  0,  0,  0,                                                                     "",               "item_tohit_bytime"},
        {279, 4, 22,   0,  0,  0,  0,                                                                     "",        "item_tohitpercent_bytime"},
        {280, 4, 22,   0,  0,  0,  0,                                                                     "",      "item_cold_damagemax_bytime"},
        {281, 4, 22,   0,  0,  0,  0,                                                                     "",      "item_fire_damagemax_bytime"},
        {282, 4, 22,   0,  0,  0,  0,                                                                     "",      "item_ltng_damagemax_bytime"},
        {283, 4, 22,   0,  0,  0,  0,                                                                     "",      "item_pois_damagemax_bytime"},
        {284, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_resist_cold_bytime"},
        {285, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_resist_fire_bytime"},
        {286, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_resist_ltng_bytime"},
        {287, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_resist_pois_bytime"},
        {288, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_absorb_cold_bytime"},
        {289, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_absorb_fire_bytime"},
        {290, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_absorb_ltng_bytime"},
        {291, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_absorb_pois_bytime"},
        {292, 4, 22,   0,  0,  0,  0,                                                                     "",           "item_find_gold_bytime"},
        {293, 4, 22,   0,  0,  0,  0,                                                                     "",          "item_find_magic_bytime"},
        {294, 4, 22,   0,  0,  0,  0,                                                                     "",        "item_regenstamina_bytime"},
        {295, 4, 22,   0,  0,  0,  0,                                                                     "",             "item_stamina_bytime"},
        {296, 4, 22,   0,  0,  0,  0,                                                                     "",        "item_damage_demon_bytime"},
        {297, 4, 22,   0,  0,  0,  0,                                                                     "",       "item_damage_undead_bytime"},
        {298, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_tohit_demon_bytime"},
        {299, 4, 22,   0,  0,  0,  0,                                                                     "",        "item_tohit_undead_bytime"},
        {300, 4, 22,   0,  0,  0,  0,                                                                     "",        "item_crushingblow_bytime"},
        {301, 4, 22,   0,  0,  0,  0,                                                                     "",          "item_openwounds_bytime"},
        {302, 4, 22,   0,  0,  0,  0,                                                                     "",         "item_kick_damage_bytime"},
        {303, 4, 22,   0,  0,  0,  0,                                                                     "",        "item_deadlystrike_bytime"},
        {304, 4,  0,   0,  0,  0,  0,                                                                     "",           "item_find_gems_bytime"},
        {305, 0,  8,  50,  0,  0,  0,                                        "-{0} to Enemy Cold Resistance",                "item_pierce_cold"},
        {306, 0,  8,  50,  0,  0,  0,                                        "-{0} to Enemy Fire Resistance",                "item_pierce_fire"},
        {307, 0,  8,  50,  0,  0,  0,                                   "-{0} to Enemy Lightning Resistance",                "item_pierce_ltng"},
        {308, 0,  8,  50,  0,  0,  0,                                      "-{0} to Enemy Poison Resistance",                "item_pierce_pois"},
        {309, 0,  0,   0,  0,  0,  0,                                                                     "",          "item_damage_vs_monster"},
        {310, 0,  0,   0,  0,  0,  0,                                                                     "",  "item_damage_percent_vs_monster"},
        {311, 0,  0,   0,  0,  0,  0,                                                                     "",           "item_tohit_vs_monster"},
        {312, 0,  0,   0,  0,  0,  0,                                                                     "",   "item_tohit_percent_vs_monster"},
        {313, 0,  0,   0,  0,  0,  0,                                                                     "",              "item_ac_vs_monster"},
        {314, 0,  0,   0,  0,  0,  0,                                                                     "",      "item_ac_percent_vs_monster"},
        {315, 0,  0,   0,  0,  0,  0,                                                                     "",                      "firelength"},
        {316, 0,  0,   0,  0,  0,  0,                                                                     "",                      "burningmin"},
        {317, 0,  0,   0,  0,  0,  0,                                                                     "",                      "burningmax"},
        {318, 0,  0,   0,  0,  0,  0,                                                                     "",              "progressive_damage"},
        {319, 0,  0,   0,  0,  0,  0,                                                                     "",               "progressive_steal"},
        {320, 0,  0,   0,  0,  0,  0,                                                                     "",               "progressive_other"},
        {321, 0,  0,   0,  0,  0,  0,                                                                     "",                "progressive_fire"},
        {322, 0,  0,   0,  0,  0,  0,                                                                     "",                "progressive_cold"},
        {323, 0,  0,   0,  0,  0,  0,                                                                     "",           "progressive_lightning"},
        {324, 0,  6,   0,  0,  0,  0,                                                                     "",              "item_extra_charges"},
        {325, 0,  0,   0,  0,  0,  0,                                                                     "",               "progressive_tohit"},
        {326, 0,  0,   0,  0,  0,  0,                                                                     "",                    "poison_count"},
        {327, 0,  0,   0,  0,  0,  0,                                                                     "",                "damage_framerate"},
        {328, 0,  0,   0,  0,  0,  0,                                                                     "",                      "pierce_idx"},
        {329, 0,  9,  50,  0,  0,  0,                                            "{0}% to Fire Skill Damage",            "passive_fire_mastery"},
        {330, 0,  9,  50,  0,  0,  0,                                       "{0}% to Lightning Skill Damage",            "passive_ltng_mastery"},
        {331, 0,  9,  50,  0,  0,  0,                                            "{0}% to Cold Skill Damage",            "passive_cold_mastery"},
        {332, 0,  9,  50,  0,  0,  0,                                          "{0}% to Poison Skill Damage",            "passive_pois_mastery"},
        {333, 0,  8,   0,  0,  0,  0,                                       "-{0}% to Enemy Fire Resistance",             "passive_fire_pierce"},
        {334, 0,  8,   0,  0,  0,  0,                                  "-{0}% to Enemy Lightning Resistance",             "passive_ltng_pierce"},
        {335, 0,  8,   0,  0,  0,  0,                                       "-{0}% to Enemy Cold Resistance",             "passive_cold_pierce"},
        {336, 0,  8,   0,  0,  0,  0,                                     "-{0}% to Enemy Poison Resistance",             "passive_pois_pierce"},
        {337, 0,  8,   0,  0,  0,  0,                                                                     "",         "passive_critical_strike"},
        {338, 0,  7,   0,  0,  0,  0,                                                                     "",                   "passive_dodge"},
        {339, 0,  7,   0,  0,  0,  0,                                                                     "",                   "passive_avoid"},
        {340, 0,  7,   0,  0,  0,  0,                                                                     "",                   "passive_evade"},
        {341, 0,  8,   0,  0,  0,  0,                                                                     "",                  "passive_warmth"},
        {342, 0,  8,   0,  0,  0,  0,                                                                     "",        "passive_mastery_melee_th"},
        {343, 0,  8,   0,  0,  0,  0,                                                                     "",       "passive_mastery_melee_dmg"},
        {344, 0,  8,   0,  0,  0,  0,                                                                     "",      "passive_mastery_melee_crit"},
        {345, 0,  8,   0,  0,  0,  0,                                                                     "",        "passive_mastery_throw_th"},
        {346, 0,  8,   0,  0,  0,  0,                                                                     "",       "passive_mastery_throw_dmg"},
        {347, 0,  8,   0,  0,  0,  0,                                                                     "",      "passive_mastery_throw_crit"},
        {348, 0,  8,   0,  0,  0,  0,                                                                     "",             "passive_weaponblock"},
        {349, 0,  8,   0,  0,  0,  0,                                                                     "",           "passive_summon_resist"},
        {350, 0,  0,   0,  0,  0,  0,                                                                     "",              "modifierlist_skill"},
        {351, 0,  0,   0,  0,  0,  0,                                                                     "",              "modifierlist_level"},
        {352, 0,  0,   0,  0,  0,  0,                                                                     "",                "last_sent_hp_pct"},
        {353, 0,  0,   0,  0,  0,  0,                                                                     "",                "source_unit_type"},
        {354, 0,  0,   0,  0,  0,  0,                                                                     "",                  "source_unit_id"},
        {355, 0,  0,   0,  0,  0,  0,                                                                     "",                     "shortparam1"},
        {356, 0,  2,   0,  0,  0,  0,                                                                     "",             "questitemdifficulty"},
        {357, 0,  9,  50,  0,  0,  0,                                                                     "",             "passive_mag_mastery"},
        {358, 0,  8,   0,  0,  0,  0,                                                                     "",              "passive_mag_pierce"},
    };

    const std::map<std::string, std::uint8_t> huffmanDecodeMap = {
        {"111101000", '\0'}, {       "01", ' '}, {"11011111", '0'}, { "0011111", '1'},
        {   "001100",  '2'}, {  "1011011", '3'}, {"01011111", '4'}, {"01101000", '5'},
        {  "1111011",  '6'}, {    "11110", '7'}, {  "001000", '8'}, {   "01110", '9'},
        {    "01111",  'a'}, {     "1010", 'b'}, {   "00010", 'c'}, {  "100011", 'd'},
        {   "000011",  'e'}, {   "110010", 'f'}, {   "01011", 'g'}, {   "11000", 'h'},
        {  "0111111",  'i'}, {"011101000", 'j'}, {  "010010", 'k'}, {   "10111", 'l'},
        {    "10110",  'm'}, {   "101100", 'n'}, { "1111111", 'o'}, {   "10011", 'p'},
        { "10011011",  'q'}, {    "00111", 'r'}, {    "0100", 's'}, {   "00110", 't'},
        {    "10000",  'u'}, {  "0111011", 'v'}, {   "00000", 'w'}, {   "11100", 'x'},
        {  "0101000",  'y'}, { "00011011", 'z'},
    };

    // Retrieves the huffman encoded chracter
    std::uint8_t getEncodedChar(const std::vector<std::uint8_t>& data, std::uint64_t& startOffset)
    {
        std::string bitStr;
        size_t startRead = startOffset;
        size_t readOffset = startRead;
        while (bitStr.size() < 9)
        {
            readOffset = startRead;
            std::stringstream ss2;
            ss2 << std::bitset<9>(read_uint32_bits(readOffset, bitStr.size() + 1));
            ++startOffset;
            bitStr = ss2.str().substr(8 - bitStr.size());
            auto iter = huffmanDecodeMap.find(bitStr);
            if (iter != huffmanDecodeMap.end())
            {
                return iter->second;
            }
        }

        // something went wrong
        return std::uint8_t(0xFF);
    }

    std::uint8_t getItemCodev115(const std::vector<std::uint8_t>& data, size_t startOffset, std::uint8_t(&strcode)[4])
    {
        size_t offset = startOffset;
        for (size_t i = 0; i < 4; ++i)
        {
            strcode[i] = getEncodedChar(data, offset);
        }

        return std::uint8_t(offset - startOffset);
    }

    // Retrieves encoded ItemCode (return number of bits set)
    void encodeItemCodev115(const std::uint8_t(&strcode)[4], std::uint64_t& encodedVal, std::uint8_t& numBitsSet)
    {
        encodedVal = 0;
        numBitsSet = 0;
        static const std::map<std::uint8_t, std::vector<std::uint16_t> > huffmanEncodeMap = {
            {'0', {223, 8}}, { '1', { 31, 7}}, {'2', { 12, 6}}, {'3', { 91, 7}},
            {'4', { 95, 8}}, { '5', {104, 8}}, {'6', {123, 7}}, {'7', { 30, 5}},
            {'8', {  8, 6}}, { '9', { 14, 5}}, {' ', {  1, 2}}, {'a', { 15, 5}},
            {'b', { 10, 4}}, { 'c', {  2, 5}}, {'d', { 35, 6}}, {'e', {  3, 6}},
            {'f', { 50, 6}}, { 'g', { 11, 5}}, {'h', { 24, 5}}, {'i', { 63, 7}},
            {'j', {232, 9}}, { 'k', { 18, 6}}, {'l', { 23, 5}}, {'m', { 22, 5}},
            {'n', { 44, 6}}, { 'o', {127, 7}}, {'p', { 19, 5}}, {'q', {155, 8}},
            {'r', {  7, 5}}, { 's', {  4, 4}}, {'t', {  6, 5}}, {'u', { 16, 5}},
            {'v', { 59, 7}}, { 'w', {  0, 5}}, {'x', { 28, 5}}, {'y', { 40, 7}},
            {'z', { 27, 8}}, {'\0', {488, 9}},
        };

        for (std::uint8_t i = 4; i > 0; --i)
        {
            auto iter = huffmanEncodeMap.find(strcode[i - 1]);
            if (iter == huffmanEncodeMap.end())
            {
                // 0
                encodedVal <<= 9;
                encodedVal |= 488;
                numBitsSet += 9;
            }
            else
            {
                encodedVal <<= iter->second[1];
                encodedVal |= iter->second[0];
                numBitsSet += std::uint8_t(iter->second[1]);
            }
        }
    }

    struct ItemType
    {
        std::string name;
        ItemDamage dam;
        ItemRequirements req;
        ItemDimensions dimensions;
        bool stackable = 0;
        std::string inv_file;
        std::uint16_t inv_transform = 0;
        std::vector<std::string> categories;

        bool isStackable() const
        {
            return stackable;
        }

        bool isWeapon() const
        {
            return std::find(categories.begin(), categories.end(), "Weapon") != categories.end() ? true : false;
        }

        bool isTwoHandedWeopon() const
        {
            if (!isWeapon())
            {
                return false;
            }

            return dam.bTwoHanded;
        }

        bool isShield() const
        {
            return std::find(categories.begin(), categories.end(), "Any Shield") != categories.end() ? true : false;
        }

        bool isArmor() const
        {
            return std::find(categories.begin(), categories.end(), "Any Armor") != categories.end() ? true : false;
        }

        bool isHelm() const
        {
            return std::find(categories.begin(), categories.end(), "Helm") != categories.end() ? true : false;
        }

        bool isBook() const
        {
            return std::find(categories.begin(), categories.end(), "Book") != categories.end() ? true : false;
        }

        bool isPotion() const
        {
            return std::find(categories.begin(), categories.end(), "Potion") != categories.end() ? true : false;
        }

        bool isQuestItem() const
        {
            return std::find(categories.begin(), categories.end(), "Quest") != categories.end() ? true : false;
        }

        bool isGoldItem() const
        {
            return std::find(categories.begin(), categories.end(), "Gold") != categories.end() ? true : false;
        }

        bool isRejuvenationPotion() const
        {
            return std::find(categories.begin(), categories.end(), "Rejuv Potion") != categories.end() ? true : false;
        }

        bool isHealingPotion() const
        {
            return std::find(categories.begin(), categories.end(), "Healing Potion") != categories.end() ? !isRejuvenationPotion() : false;
        }

        bool isManaPotion() const
        {
            return std::find(categories.begin(), categories.end(), "Mana Potion") != categories.end() ? !isRejuvenationPotion() : false;
        }

        bool isSocketFiller() const
        {
            return std::find(categories.begin(), categories.end(), "Socket Filler") != categories.end() ? true : false;
        }

        bool isUpgradableGem() const
        {
            if (isSocketFiller())
            {
                return ((std::find(categories.begin(), categories.end(), "Gem") == categories.end()) ||
                    (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())) ? false : true;
            }

            return false;
        }

        bool isUpgradableRejuvenationPotion() const
        {
            if (isRejuvenationPotion())
            {
                return name.find("Full") == 0 ? false : true;
            }

            return false;
        }

        bool isUpgradablePotion() const
        {
            if (isRejuvenationPotion())
            {
                return isUpgradableRejuvenationPotion();
            }

            if (isPotion())
            {
                return name.find("Super") == 0 ? false : true;
            }

            return false;
        }

        bool isRune() const
        {
            return std::find(categories.begin(), categories.end(), "Rune") != categories.end() ? true : false;
        }

        bool isCharm() const
        {
            return std::find(categories.begin(), categories.end(), "Charm") != categories.end() ? true : false;
        }

        bool isBelt() const
        {
            return std::find(categories.begin(), categories.end(), "Belt") != categories.end() ? true : false;
        }

        EnumItemType getEnumItemType() const
        {
            if (isWeapon())
            {
                return EnumItemType::Weapon;
            }

            if (isShield())
            {
                return EnumItemType::Shield;
            }

            if (isArmor())
            {
                return EnumItemType::Armor;
            }

            return EnumItemType::Other;
        }

        std::uint8_t getBaseType() const
        {
            std::uint8_t result = 0;
            if (isStackable())
            {
                result |= 0x1;
            }

            if (isWeapon())
            {
                result |= 0x2;
            }

            if (isArmor())
            {
                result |= 0x4;
            }

            if (isBook())
            {
                result |= 0x8;
            }

            return result;
        }

        bool getSocketedMagicalAttributes(std::vector<MagicalAttribute>& attribs, EnumItemType parentItemType) const
        {
            attribs.clear();
            if (isSocketFiller())
            {
                std::vector<std::uint16_t> ids;
                std::vector<std::uint16_t> ids2;
                std::vector<std::int64_t> values;
                std::vector<std::int64_t> values2;
                if (std::find(categories.begin(), categories.end(), "Gem") != categories.end())
                {
                    if (std::find(categories.begin(), categories.end(), "Amethyst") != categories.end())
                    {
                        switch (parentItemType)
                        {
                        case EnumItemType::Weapon:
                            ids.push_back(19);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(40);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(60);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(80);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(100);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(150);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Shield:
                            ids.push_back(31);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(18);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(24);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(30);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Armor:
                            ids.push_back(0);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(3);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(4);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(6);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(10);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        default:
                            return false;
                        }
                    }
                    else if (std::find(categories.begin(), categories.end(), "Topaz") != categories.end())
                    {
                        switch (parentItemType)
                        {
                        case EnumItemType::Weapon:
                            ids.push_back(50);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(1);
                                values.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(1);
                                values.push_back(14);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(1);
                                values.push_back(22);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(1);
                                values.push_back(30);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(1);
                                values.push_back(40);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Shield:
                            ids.push_back(41);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(16);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(22);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(28);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(40);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Armor:
                            ids.push_back(80);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(9);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(13);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(16);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(20);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(24);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        default:
                            return false;
                        }
                    }
                    else if (std::find(categories.begin(), categories.end(), "Sapphire") != categories.end())
                    {
                        switch (parentItemType)
                        {
                        case EnumItemType::Weapon:
                            ids.push_back(54);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(1);
                                values.push_back(3);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(3);
                                values.push_back(5);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(4);
                                values.push_back(7);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(6);
                                values.push_back(10);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(10);
                                values.push_back(14);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Shield:
                            ids.push_back(43);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(16);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(22);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(28);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(40);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Armor:
                            ids.push_back(9);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(10);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(17);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(24);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(31);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(38);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        default:
                            return false;
                        }
                    }
                    else if (std::find(categories.begin(), categories.end(), "Emerald") != categories.end())
                    {
                        switch (parentItemType)
                        {
                        case EnumItemType::Weapon:
                            ids.push_back(57);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(10);
                                values.push_back(10);
                                values.push_back(3);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(20);
                                values.push_back(20);
                                values.push_back(4);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(40);
                                values.push_back(40);
                                values.push_back(5);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(60);
                                values.push_back(60);
                                values.push_back(6);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(100);
                                values.push_back(100);
                                values.push_back(7);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Shield:
                            ids.push_back(45);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(16);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(22);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(28);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(40);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Armor:
                            ids.push_back(2);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(3);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(4);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(6);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(10);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        default:
                            return false;
                        }
                    }
                    else if (std::find(categories.begin(), categories.end(), "Ruby") != categories.end())
                    {
                        switch (parentItemType)
                        {
                        case EnumItemType::Weapon:
                            ids.push_back(48);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(3);
                                values.push_back(4);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(5);
                                values.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(8);
                                values.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(10);
                                values.push_back(16);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(15);
                                values.push_back(20);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Shield:
                            ids.push_back(39);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(16);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(22);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(28);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(40);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Armor:
                            ids.push_back(7);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(10);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(17);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(24);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(31);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(38);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        default:
                            return false;
                        }
                    }
                    else if (std::find(categories.begin(), categories.end(), "Diamond") != categories.end())
                    {
                        switch (parentItemType)
                        {
                        case EnumItemType::Weapon:
                            ids.push_back(122);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(28);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(34);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(44);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(54);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(68);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Shield:
                            ids.push_back(43);
                            ids.push_back(41);
                            ids.push_back(39);
                            ids.push_back(45);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(6);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(11);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(14);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(19);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Armor:
                            ids.push_back(19);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(20);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(40);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(60);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(80);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(100);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        default:
                            return false;
                        }
                    }
                    else if (std::find(categories.begin(), categories.end(), "Skull") != categories.end())
                    {
                        switch (parentItemType)
                        {
                        case EnumItemType::Weapon:
                            ids.push_back(62);
                            ids2.push_back(60);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(2);
                                values2.push_back(1);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(2);
                                values2.push_back(2);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(3);
                                values2.push_back(2);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(3);
                                values2.push_back(3);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(4);
                                values2.push_back(3);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Shield:
                            ids.push_back(78);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(4);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(16);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(20);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        case EnumItemType::Armor:
                            ids.push_back(74);
                            ids2.push_back(27);
                            if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                            {
                                values.push_back(2);
                                values2.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                            {
                                values.push_back(3);
                                values2.push_back(8);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                            {
                                values.push_back(3);
                                values2.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                            {
                                values.push_back(4);
                                values2.push_back(12);
                            }
                            else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                            {
                                values.push_back(5);
                                values2.push_back(19);
                            }
                            else
                            {
                                return false;
                            }
                            break;

                        default:
                            return false;
                        }
                    }

                    MagicalAttribute attrib;
                    for (auto id : ids)
                    {
                        attrib.Id = id;
                        attrib.Values = values;

                        const auto& stat = itemStats[attrib.Id];
                        attrib.Name = stat.name;
                        attrib.Desc = stat.desc;
                        attribs.push_back(attrib);
                    }

                    for (auto id : ids2)
                    {
                        attrib.Id = id;
                        attrib.Values = values2;

                        const auto& stat = itemStats[attrib.Id];
                        attrib.Name = stat.name;
                        attrib.Desc = stat.desc;
                        attribs.push_back(attrib);
                    }

                    return true;
                }
            }

            return false;
        }

        std::uint16_t getPotionPoints(EnumCharClass charClass) const
        {
            static const std::map <EnumCharClass, std::vector<std::uint16_t>> manaPotionPoints = {
                {EnumCharClass::Amazon, {30, 60, 120, 225, 375}},
                {EnumCharClass::Sorceress, {40, 80, 160, 300, 500}},
                {EnumCharClass::Necromancer, {40, 80, 160, 300, 500}},
                {EnumCharClass::Paladin, {30, 60, 120, 225, 375}},
                {EnumCharClass::Barbarian, {20, 40, 80, 150, 250}},
                {EnumCharClass::Druid, {40, 80, 160, 300, 500}},
                {EnumCharClass::Assassin, {30, 60, 120, 225, 375}}
            };

            static const std::map <EnumCharClass, std::vector<std::uint16_t>> healingPotionPoints = {
                {EnumCharClass::Amazon, {45, 90, 150, 270, 480}},
                {EnumCharClass::Sorceress, {30, 60, 100, 180, 320}},
                {EnumCharClass::Necromancer, {30, 60, 100, 180, 320}},
                {EnumCharClass::Paladin, {45, 90, 150, 270, 480}},
                {EnumCharClass::Barbarian, {60, 120, 200, 360, 640}},
                {EnumCharClass::Druid, {30, 60, 100, 180, 320}},
                {EnumCharClass::Assassin, {45, 90, 150, 270, 480}}
            };

            if (isRejuvenationPotion())
            {
                if (isUpgradableRejuvenationPotion())
                {
                    return 35;
                }

                return 100;
            }

            bool bIsManaPotion = isManaPotion();
            if (!bIsManaPotion && !isHealingPotion())
            {
                return 0;
            }

            auto manaIter = bIsManaPotion ? manaPotionPoints.find(charClass) : manaPotionPoints.end();
            auto healingIter = bIsManaPotion ? healingPotionPoints.end() : healingPotionPoints.find(charClass);
            if ((manaIter == manaPotionPoints.end()) && (healingIter == healingPotionPoints.end()))
            {
                // should not happen
                return 0;
            }

            const auto& potionPoints = bIsManaPotion ? manaIter->second : healingIter->second;
            auto c = inv_file.back();
            switch (c)
            {
            case '1':
                return potionPoints[0];

            case '2':
                return potionPoints[1];

            case '3':
                return potionPoints[2];

            case '4':
                return potionPoints[3];

            case '5':
                return potionPoints[4];
            }

            // should not happen
            return 0;
        }
    };

    ItemType s_invalidItemType;
    const ItemType& getShieldItemType(const std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, ItemType> shieldBases = {
            {"buc", {"Buckler", {{ 0, 0 }, false, false, { 0, 0 }}, {12, 0}, {2, 2}, false, "invbuc", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"sml", {"Small Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {22, 0}, {2, 2}, false, "invsml", 5, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"lrg", {"Large Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {34, 0}, {2, 3}, false, "invlrg", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"kit", {"Kite Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {47, 0}, {2, 3}, false, "invkit", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"tow", {"Tower Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {75, 0}, {2, 3}, false, "invtow", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"gts", {"Gothic Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {60, 0}, {2, 4}, false, "invgts", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"bsh", {"Bone Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0}, {2, 3}, false, "invbsh", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"spk", {"Spiked Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {30, 0}, {2, 3}, false, "invspk", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"xuc", {"Defender", {{ 0, 0 }, false, false, { 0, 0 }}, {38, 0, 22}, {2, 2}, false, "invbuc", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"xml", {"Round Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {53, 0, 25}, {2, 2}, false, "invsml", 5, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"xrg", {"Scutum", {{ 0, 0 }, false, false, { 0, 0 }}, {71, 0, 25}, {2, 3}, false, "invlrg", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"xit", {"Dragon Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {91, 0, 25}, {2, 3}, false, "invkit", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"xow", {"Pavise", {{ 0, 0 }, false, false, { 0, 0 }}, {133, 0, 25}, {2, 3}, false, "invtow", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"xts", {"Ancient Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {110, 0, 25}, {2, 4}, false, "invgts", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"xsh", {"Grim Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 3}, false, "invbsh", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"xpk", {"Barbed Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 25}, {2, 3}, false, "invspk", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"uuc", {"Heater", {{ 0, 0 }, false, false, { 0, 0 }}, {77, 0, 43}, {2, 2}, false, "invbuc", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"uml", {"Luna", {{ 0, 0 }, false, false, { 0, 0 }}, {100, 0, 45}, {2, 2}, false, "invsml", 5, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"urg", {"Hyperion", {{ 0, 0 }, false, false, { 0, 0 }}, {127, 0, 48}, {2, 3}, false, "invlrg", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"uit", {"Monarch", {{ 0, 0 }, false, false, { 0, 0 }}, {156, 0, 54}, {2, 3}, false, "invkit", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"uow", {"Aegis", {{ 0, 0 }, false, false, { 0, 0 }}, {219, 0, 59}, {2, 3}, false, "invtow", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"uts", {"Ward", {{ 0, 0 }, false, false, { 0, 0 }}, {185, 0, 63}, {2, 5}, false, "invgts", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"ush", {"Troll Nest", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 57}, {2, 3}, false, "invbsh", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"upk", {"Blade Barrier", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 51}, {2, 3}, false, "invspk", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}}},
            {"pa1", {"Targe", {{ 0, 0 }, false, false, { 0, 0 }}, {16, 0, 3}, {2, 2}, false, "invpa1", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pa2", {"Rondache", {{ 0, 0 }, false, false, { 0, 0 }}, {26, 0, 6}, {2, 2}, false, "invpa2", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pa3", {"Heraldic Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {40, 0, 12}, {2, 4}, false, "invpa3", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pa4", {"Aerin Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 15}, {2, 4}, false,"invpa4", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pa5", {"Crown Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 18}, {2, 2}, false, "invpa5", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"ne1", {"Preserved Head", {{ 0, 0 }, false, false, { 0, 0 }}, {12, 0, 3}, {2, 2}, false, "invne1", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"ne2", {"Zombie Head", {{ 0, 0 }, false, false, { 0, 0 }}, {14, 0, 6}, {2, 2}, false, "invne2", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"ne3", {"Unraveller Head", {{ 0, 0 }, false, false, { 0, 0 }}, {18, 0, 12}, {2, 2}, false, "invne3", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"ne4", {"Gargoyle Head", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 15}, {2, 2}, false, "invne4", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"ne5", {"Demon Head", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0, 18}, {2, 2}, false, "invne5", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"pa6", {"Akaran Targe", {{ 0, 0 }, false, false, { 0, 0 }}, {44, 0, 26}, {2, 2}, false, "invpa1", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pa7", {"Akaran Rondache", {{ 0, 0 }, false, false, { 0, 0 }}, {59, 0, 30}, {2, 2}, false, "invpa2", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pa8", {"Protector Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {69, 0, 34}, {2, 4}, false, "invpa3", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pa9", {"Guilded Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {89, 0, 38}, {2, 4}, false, "invpa4", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"paa", {"Royal Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {114, 0, 41}, {2, 2}, false, "invpa5", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"ne6", {"Mummified Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {38, 0, 24}, {2, 2}, false, "invne1", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"ne7", {"Fetish Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {41, 0, 29}, {2, 2}, false, "invne2", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"ne8", {"Sexton Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {47, 0, 33}, {2, 2}, false, "invne3", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"ne9", {"Cantor Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 36}, {2, 2}, false, "invne4", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"nea", {"Heirophant Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 40}, {2, 2}, false, "invne5", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"pab", {"Sacred Targe", {{ 0, 0 }, false, false, { 0, 0 }}, {86, 0, 47}, {2, 2}, false, "invpa1", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pac", {"Sacred Rondache", {{ 0, 0 }, false, false, { 0, 0 }}, {109, 0, 52}, {2, 2}, false, "invpa2", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"pad", {"Ancient Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {124, 0, 55}, {2, 4}, false, "invpa3", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"paf", {"Zakarum Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {142, 0, 61}, {2, 4}, false, "invpa4", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"paa", {"Vortex Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {148, 0, 66}, {2, 2}, false, "invpa5", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}}},
            {"neb", {"Minion Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {77, 0, 44}, {2, 2}, false, "invne1", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"nec", {"Hellspawn Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {82, 0, 50}, {2, 2}, false, "invne2", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"ned", {"Overseer Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {91, 0, 49}, {2, 2}, false, "invne3", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"nee", {"Succubae Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {95, 0, 60}, {2, 2}, false, "invne4", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
            {"nef", {"Bloodlord Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 65}, {2, 2}, false, "invne5", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}}},
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = shieldBases.find(testStr);
        if (iter != shieldBases.end())
        {
            return iter->second;
        }

        return s_invalidItemType;
    }

    const ItemType& getArmorItemType(const std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, ItemType> armorBases = {
            {"cap", {"Cap", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invcap", 8, {"Helm", "Any Armor"}}},
            {"skp", {"Skull Cap", {{ 0, 0 }, false, false, { 0, 0 }}, {15, 0}, {2, 2}, false, "invskp", 8, {"Helm", "Any Armor"}}},
            {"hlm", {"Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {26, 0}, {2, 2}, false, "invhlm", 8, {"Helm", "Any Armor"}}},
            {"fhl", {"Full Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {41, 0}, {2, 2}, false, "invfhl", 8, {"Helm", "Any Armor"}}},
            {"ghm", {"Great Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {63, 0}, {2, 2}, false, "invghm", 8, {"Helm", "Any Armor"}}},
            {"crn", {"Crown", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0}, {2, 2}, false, "invcrn", 8, {"Helm", "Any Armor"}}},
            {"msk", {"Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {23, 0}, {2, 2}, false, "invmsk", 2, {"Helm", "Any Armor"}}},
            {"qui", {"Quilted Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {12, 0}, {2, 3}, false, "invqlt", 8, {"Armor", "Any Armor"}}},
            {"lea", {"Leather Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {15, 0}, {2, 3}, false, "invlea", 8, {"Armor", "Any Armor"}}},
            {"hla", {"Hard Leather Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0}, {2, 3}, false, "invhla", 8, {"Armor", "Any Armor"}}},
            {"stu", {"Studded Leather", {{ 0, 0 }, false, false, { 0, 0 }}, {27, 0}, {2, 3}, false, "invstu", 8, {"Armor", "Any Armor"}}},
            {"rng", {"Ring Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {36, 0}, {2, 3}, false, "invrng", 8, {"Armor", "Any Armor"}}},
            {"scl", {"Scale Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {44, 0}, {2, 3}, false, "invscl", 8, {"Armor", "Any Armor"}}},
            {"chn", {"Chain Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {48, 0}, {2, 3}, false, "invchn", 8, {"Armor", "Any Armor"}}},
            {"brs", {"Breast Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {30, 0}, {2, 3}, false, "invbrs", 8, {"Armor", "Any Armor"}}},
            {"spl", {"Splint Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {51, 0}, {2, 3}, false, "invspl", 8, {"Armor", "Any Armor"}}},
            {"plt", {"Plate Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0}, {2, 3}, false, "invplt", 8, {"Armor", "Any Armor"}}},
            {"fld", {"Field Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0}, {2, 3}, false, "invfld", 8, {"Armor", "Any Armor"}}},
            {"gth", {"Gothic Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {70, 0}, {2, 3}, false, "invgth", 8, {"Armor", "Any Armor"}}},
            {"ful", {"Full Plate Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {80, 0}, {2, 3}, false, "invful", 8, {"Armor", "Any Armor"}}},
            {"aar", {"Ancient Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {100, 0}, {2, 3}, false, "invaar", 8, {"Armor", "Any Armor"}}},
            {"ltp", {"Light Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {41, 0}, {2, 3}, false, "invltp", 8, {"Armor", "Any Armor"}}},
            {"lgl", {"Leather Gloves", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invlgl", 8, {"Gloves", "Any Armor"}}},
            {"vgl", {"Heavy Gloves", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invvgl", 8, {"Gloves", "Any Armor"}}},
            {"mgl", {"Chain Gloves", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0}, {2, 2}, false, "invmgl", 8, {"Gloves", "Any Armor"}}},
            {"tgl", {"Light Gauntlets", {{ 0, 0 }, false, false, { 0, 0 }}, {45, 0}, {2, 2}, false, "invtgl", 8, {"Gloves", "Any Armor"}}},
            {"hgl", {"Gaunlets", {{ 0, 0 }, false, false, { 0, 0 }}, {60, 0}, {2, 2}, false, "invhgl", 8, {"Gloves", "Any Armor"}}},
            {"lbt", {"Leather Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invlbt", 8, {"Boots", "Any Armor"}}},
            {"vbt", {"Heavy Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {18, 0}, {2, 2}, false, "invvbt", 8, {"Boots", "Any Armor"}}},
            {"mbt", {"Chain Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {30, 0}, {2, 2}, false, "invmbt", 8, {"Boots", "Any Armor"}}},
            {"tbt", {"Light Plate Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0}, {2, 2}, false, "invtbt", 8, {"Boots", "Any Armor"}}},
            {"hbt", {"Greaves", {{ 0, 0 }, false, false, { 0, 0 }}, {70, 0}, {2, 2}, false, "invhbt", 8, {"Boots", "Any Armor"}}},
            {"lbl", {"Sash", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 1, 4, 2}, false, "invlbl", 8, {"Belt", "Any Armor"}}},
            {"vbl", {"Light Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 1, 4, 2}, false, "invvbl", 8, {"Belt", "Any Armor"}}},
            {"mbl", {"Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0}, {2, 1, 4, 3}, false, "invmbl", 8, {"Belt", "Any Armor"}}},
            {"tbl", {"Heavy Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {45, 0}, {2, 1, 4, 3}, false, "invtbl", 8, {"Belt", "Any Armor"}}},
            {"hbl", {"Plated Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {60, 0}, {2, 1, 4, 3}, false, "invhbl", 8, {"Belt", "Any Armor"}}},
            {"bhm", {"Bone Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0}, {2, 2}, false, "invbhm", 8, {"Helm", "Any Armor"}}},
            {"xap", {"War Hat", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 22}, {2, 2}, false, "invcap", 8, {"Helm", "Any Armor"}}},
            {"xkp", {"Sallet", {{ 0, 0 }, false, false, { 0, 0 }}, {43, 0, 25}, {2, 2}, false, "invskp", 8, {"Helm", "Any Armor"}}},
            {"xlm", {"Casque", {{ 0, 0 }, false, false, { 0, 0 }}, {59, 0, 25}, {2, 2}, false, "invhlm", 8, {"Helm", "Any Armor"}}},
            {"xhl", {"Basinet", {{ 0, 0 }, false, false, { 0, 0 }}, {82, 0, 25}, {2, 2}, false, "invfhl", 8, {"Helm", "Any Armor"}}},
            {"xhm", {"Winged Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {115, 0, 25}, {2, 2}, false, "invghm", 8, {"Helm", "Any Armor"}}},
            {"xrn", {"Grand Crown", {{ 0, 0 }, false, false, { 0, 0 }}, {103, 0, 25}, {2, 2}, false, "invcrn", 8, {"Helm", "Any Armor"}}},
            {"xsk", {"Death Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0, 25}, {2, 2}, false, "invmsk", 2, {"Helm", "Any Armor"}}},
            {"xui", {"Ghost Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {38, 0, 22}, {2, 3}, false, "invqlt", 8, {"Armor", "Any Armor"}}},
            {"xea", {"Serpentskin Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {43, 0, 24}, {2, 3}, false, "invlea", 8, {"Armor", "Any Armor"}}},
            {"xla", {"Demonhide Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 25}, {2, 3}, false, "invhla", 8, {"Armor", "Any Armor"}}},
            {"xtu", {"Trellised Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {61, 0, 25}, {2, 3}, false, "invstu", 8, {"Armor", "Any Armor"}}},
            {"xng", {"Linked Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {74, 0, 25}, {2, 3}, false, "invrng", 8, {"Armor", "Any Armor"}}},
            {"xcl", {"Tigulated Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {86, 0, 25}, {2, 3}, false, "invscl", 8, {"Armor", "Any Armor"}}},
            {"xhn", {"Mesh Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {92, 0, 25}, {2, 3}, false, "invchn", 8, {"Armor", "Any Armor"}}},
            {"xrs", {"Cuirass", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 25}, {2, 3}, false, "invbrs", 8, {"Armor", "Any Armor"}}},
            {"xpl", {"Russet Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {97, 0, 25}, {2, 3}, false, "invspl", 8, {"Armor", "Any Armor"}}},
            {"xlt", {"Templar Coat", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 25}, {2, 3}, false, "invplt", 8, {"Armor", "Any Armor"}}},
            {"xld", {"Sharktooth Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {103, 0, 25}, {2, 3}, false, "invfld", 8, {"Armor", "Any Armor"}}},
            {"xth", {"Embossed Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {125, 0, 25}, {2, 3}, false, "invgth", 8, {"Armor", "Any Armor"}}},
            {"xul", {"Chaos Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {140, 0, 25}, {2, 3}, false, "invful", 8, {"Armor", "Any Armor"}}},
            {"xar", {"Ornate Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {170, 0, 25}, {2, 3}, false, "invaar", 8, {"Armor", "Any Armor"}}},
            {"xtp", {"Mage Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0, 25}, {2, 3}, false, "invltp", 8, {"Armor", "Any Armor"}}},
            {"xlg", {"Demonhide Gloves", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 21}, {2, 2}, false, "invlgl", 8, {"Gloves", "Any Armor"}}},
            {"xvg", {"Sharkskin Gloves", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 25}, {2, 2}, false, "invvgl", 8, {"Gloves", "Any Armor"}}},
            {"xmg", {"Heavy Bracers", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 2}, false, "invmgl", 8, {"Gloves", "Any Armor"}}},
            {"xtg", {"Battle Gauntlets", {{ 0, 0 }, false, false, { 0, 0 }}, {88, 0, 25}, {2, 2}, false, "invtgl", 8, {"Gloves", "Any Armor"}}},
            {"xhg", {"War Gauntlets", {{ 0, 0 }, false, false, { 0, 0 }}, {110, 0, 25}, {2, 2}, false, "invhgl", 8, {"Gloves", "Any Armor"}}},
            {"xlb", {"Demonhide Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 24}, {2, 2}, false, "invhgl", 8, {"Boots", "Any Armor"}}},
            {"xvb", {"Sharkskin Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {47, 0, 25}, {2, 2}, false, "invvbt", 8, {"Boots", "Any Armor"}}},
            {"xmb", {"Mesh Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 2}, false, "invmbt", 8, {"Boots", "Any Armor"}}},
            {"xtb", {"Battle Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {95, 0, 25}, {2, 2}, false, "invtbt", 8, {"Boots", "Any Armor"}}},
            {"xhb", {"War Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {125, 0, 25}, {2, 2}, false, "invhbt", 8, {"Boots", "Any Armor"}}},
            {"zlb", {"Demonhide Sash", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 24}, {2, 1, 4, 3}, false, "invlbl", 8, {"Belt", "Any Armor"}}},
            {"zvb", {"Sharkskin Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 25}, {2, 1, 4, 3}, false, "invvbl", 8, {"Belt", "Any Armor"}}},
            {"zmb", {"Mesh Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 1, 4, 3}, false, "invmbl", 8, {"Belt", "Any Armor"}}},
            {"ztb", {"Battle Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {88, 0, 25}, {2, 1, 4, 3}, false, "invtbl", 8, {"Belt", "Any Armor"}}},
            {"zhb", {"War Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {110, 0, 25}, {2, 1, 4, 3}, false, "invhbl", 8, {"Belt", "Any Armor"}}},
            {"xh9", {"Grim Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 2}, false, "invbhm", 8, {"Helm", "Any Armor"}}},
            {"dr1", {"Wolf Head", {{ 0, 0 }, false, false, { 0, 0 }}, {16, 0, 3}, {2, 2}, false, "invdr1", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dr2", {"Hawk Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 6}, {2, 2}, false, "invdr2", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dr3", {"Antlers", {{ 0, 0 }, false, false, { 0, 0 }}, {24, 0, 12}, {2, 2}, false, "invdr3", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dr4", {"Falcon Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {28, 0, 15}, {2, 2}, false, "invdr4", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dr5", {"Spirit Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {30, 0, 18}, {2, 2}, false, "invdr5", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"ba1", {"Jawbone Cap", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0, 3}, {2, 2}, false, "invba1", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"ba2", {"Fanged Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {35, 0, 6}, {2, 2}, false, "invba2", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"ba3", {"Horned Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {45, 0, 12}, {2, 2}, false, "invba3", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"ba4", {"Assault Helmet", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0, 15}, {2, 2}, false, "invba4", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"ba5", {"Avenger Guard", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 18}, {2, 2}, false, "invba5", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"ci0", {"Circlet", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 16}, {2, 2}, false, "invci0", 2, {"Circlet", "Helm", "Any Armor"}}},
            {"ci1", {"Coronet", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 39}, {2, 2}, false, "invci1", 2, {"Circlet", "Helm", "Any Armor"}}},
            {"ci2", {"Tiara", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 52}, {2, 2}, false, "invci2", 2, {"Circlet", "Helm", "Any Armor"}}},
            {"ci3", {"Diadem", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 64}, {2, 2}, false, "invci3", 2, {"Circlet", "Helm", "Any Armor"}}},
            {"uap", {"Shako", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 43}, {2, 2}, false, "invcap", 8, {"Helm", "Any Armor"}}},
            {"ukp", {"Hydraskull", {{ 0, 0 }, false, false, { 0, 0 }}, {84, 0, 47}, {2, 2}, false, "invskp", 8, {"Helm", "Any Armor"}}},
            {"ulm", {"Armet", {{ 0, 0 }, false, false, { 0, 0 }}, {109, 0, 51}, {2, 2}, false, "invhlm", 8, {"Helm", "Any Armor"}}},
            {"uhl", {"Giant Conch", {{ 0, 0 }, false, false, { 0, 0 }}, {142, 0, 40}, {2, 2}, false, "invfhl", 8, {"Helm", "Any Armor"}}},
            {"uhm", {"Spired Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {192, 0, 59}, {2, 2}, false, "invghm", 8, {"Helm", "Any Armor"}}},
            {"urn", {"Corona", {{ 0, 0 }, false, false, { 0, 0 }}, {174, 0, 66}, {2, 2}, false, "invcrn", 8, {"Helm", "Any Armor"}}},
            {"usk", {"Demonhead", {{ 0, 0 }, false, false, { 0, 0 }}, {102, 0, 55}, {2, 2}, false, "invmsk", 2, {"Helm", "Any Armor"}}},
            {"uui", {"Dusk Shroud", {{ 0, 0 }, false, false, { 0, 0 }}, {77, 0, 49}, {2, 3}, false, "invqlt", 8, {"Armor", "Any Armor"}}},
            {"uea", {"Wyrmhide", {{ 0, 0 }, false, false, { 0, 0 }}, {84, 0, 50}, {2, 3}, false, "invlea", 8, {"Armor", "Any Armor"}}},
            {"ula", {"Scarab Husk", {{ 0, 0 }, false, false, { 0, 0 }}, {95, 0, 51}, {2, 3}, false, "invhla", 8, {"Armor", "Any Armor"}}},
            {"utu", {"Wire Fleece", {{ 0, 0 }, false, false, { 0, 0 }}, {111, 0, 53}, {2, 3}, false, "invstu", 8, {"Armor", "Any Armor"}}},
            {"ung", {"Diamond Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {131, 0, 54}, {2, 3}, false, "invrng", 8, {"Armor", "Any Armor"}}},
            {"ucl", {"Loricated Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {149, 0, 55}, {2, 3}, false, "invscl", 8, {"Armor", "Any Armor"}}},
            {"uhn", {"Boneweave", {{ 0, 0 }, false, false, { 0, 0 }}, {158, 0, 47}, {2, 3}, false, "invchn", 8, {"Armor", "Any Armor"}}},
            {"urs", {"Great Hauberk", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 56}, {2, 3}, false, "invbrs", 8, {"Armor", "Any Armor"}}},
            {"upl", {"Balrog Skin", {{ 0, 0 }, false, false, { 0, 0 }}, {165, 0, 57}, {2, 3}, false, "invspl", 8, {"Armor", "Any Armor"}}},
            {"ult", {"Hellforged Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {196, 0, 59}, {2, 3}, false, "invplt", 8, {"Armor", "Any Armor"}}},
            {"uld", {"Kraken Shell", {{ 0, 0 }, false, false, { 0, 0 }}, {174, 0, 61}, {2, 3}, false, "invfld", 8, {"Armor", "Any Armor"}}},
            {"uth", {"Lacquered Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {208, 0, 62}, {2, 3}, false, "invgth", 8, {"Armor", "Any Armor"}}},
            {"uul", {"Shadow Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {230, 0, 64}, {2, 3}, false, "invful", 8, {"Armor", "Any Armor"}}},
            {"uar", {"Sacred Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {232, 0, 66}, {2, 3}, false, "invaar", 8, {"Armor", "Any Armor"}}},
            {"utp", {"Archon Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {103, 0, 63}, {2, 3}, false, "invltp", 8, {"Armor", "Any Armor"}}},
            {"ulg", {"Bramble Mitts", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 42}, {2, 2}, false, "invlgl", 8, {"Gloves", "Any Armor"}}},
            {"uvg", {"Vampirebone Gloves", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 47}, {2, 2}, false, "invvgl", 8, {"Gloves", "Any Armor"}}},
            {"umg", {"Vambraces", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 51}, {2, 2}, false, "invmgl", 8, {"Gloves", "Any Armor"}}},
            {"utg", {"Crusader Gauntlets", {{ 0, 0 }, false, false, { 0, 0 }}, {151, 0, 57}, {2, 2}, false, "invtgl", 8, {"Gloves", "Any Armor"}}},
            {"uhg", {"Ogre Gauntlets", {{ 0, 0 }, false, false, { 0, 0 }}, {185, 0, 64}, {2, 2}, false, "invhgl", 8, {"Gloves", "Any Armor"}}},
            {"ulb", {"Wyrmhide Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 45}, {2, 2}, false, "invlbt", 8, {"Boots", "Any Armor"}}},
            {"uvb", {"Scarabshell Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {91, 0, 49}, {2, 2}, false, "invvbt", 8, {"Boots", "Any Armor"}}},
            {"umb", {"Boneweave Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 54}, {2, 2}, false, "invmbt", 8, {"Boots", "Any Armor"}}},
            {"utb", {"Mirrored Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {163, 0, 60}, {2, 2}, false, "invtbt", 8, {"Boots", "Any Armor"}}},
            {"uhb", {"Myrmidon Greaves", {{ 0, 0 }, false, false, { 0, 0 }}, {208, 0, 65}, {2, 2}, false, "invhbt", 8, {"Boots", "Any Armor"}}},
            {"ulc", {"Spiderweb Sash", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 46}, {2, 1, 4, 3}, false, "invlbl", 8, {"Belt", "Any Armor"}}},
            {"uvc", {"Vampirefang Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 51}, {2, 1, 4, 3}, false, "invvbl", 8, {"Belt", "Any Armor"}}},
            {"umc", {"Mithril Coil", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 56}, {2, 1, 4, 3}, false, "invmbl", 8, {"Belt", "Any Armor"}}},
            {"utc", {"Troll Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {151, 0, 62}, {2, 1, 4, 3}, false, "invtbl", 8, {"Belt", "Any Armor"}}},
            {"uhc", {"Colossus Girdle", {{ 0, 0 }, false, false, { 0, 0 }}, {185, 0, 67}, {2, 1, 4, 3}, false, "invhbl", 8, {"Belt", "Any Armor"}}},
            {"uh9", {"Bone Visage", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 63}, {2, 2}, false, "invbhm", 8, {"Helm", "Any Armor"}}},
            {"dr6", {"Alpha Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {44, 0, 26}, {2, 2}, false, "invdr1", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dr7", {"Griffon Headress", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 30}, {2, 2}, false, "invdr2", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dr8", {"Hunter's Guise", {{ 0, 0 }, false, false, { 0, 0 }}, {56, 0, 29}, {2, 2}, false, "invdr3", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dr9", {"Sacred Feathers", {{ 0, 0 }, false, false, { 0, 0 }}, {62, 0, 32}, {2, 2}, false, "invdr4", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dra", {"Totemic Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 41}, {2, 2}, false, "invdr5", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"ba6", {"Jawbone Visor", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 2}, false, "invba1", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"ba7", {"Lion Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {73, 0, 29}, {2, 2}, false, "invba2", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"ba8", {"Rage Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {88, 0, 29}, {2, 2}, false, "invba3", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"ba9", {"Savage Helmet", {{ 0, 0 }, false, false, { 0, 0 }}, {103, 0, 32}, {2, 2}, false, "invba4", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"baa", {"Slayer Guard", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 40}, {2, 2}, false, "invba5", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"drb", {"Blood Spirt", {{ 0, 0 }, false, false, { 0, 0 }}, {86, 0, 46}, {2, 2}, false, "invdr1", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"drc", {"Sun Spirit", {{ 0, 0 }, false, false, { 0, 0 }}, {95, 0, 51}, {2, 2}, false, "invdr2", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"drd", {"Earth Spirit", {{ 0, 0 }, false, false, { 0, 0 }}, {104, 0, 57}, {2, 2}, false, "invdr3", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"dre", {"Sky Spirit", {{ 0, 0 }, false, false, { 0, 0 }}, {113, 0, 62}, {2, 2}, false, "invdr4", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"drf", {"Dream Spirit", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 66}, {2, 2}, false, "invdr5", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}}},
            {"bab", {"Carnage Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 45}, {2, 2}, false, "invba1", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"bac", {"Fury Visor", {{ 0, 0 }, false, false, { 0, 0 }}, {129, 0, 49}, {2, 2}, false, "invba2", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"bad", {"Destroyer Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {151, 0, 54}, {2, 2}, false, "invba3", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"bae", {"Conquerer Crown", {{ 0, 0 }, false, false, { 0, 0 }}, {174, 0, 60}, {2, 2}, false, "invba4", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
            {"baf", {"Guardian Crown", {{ 0, 0 }, false, false, { 0, 0 }}, {196, 0, 65}, {2, 2}, false, "invba5", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}}},
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = armorBases.find(testStr);
        if (iter != armorBases.end())
        {
            return iter->second;
        }

        // Maybe a Sheild
        return getShieldItemType(strcode);
    }

    const ItemType& getWeaponItemType(const std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, ItemType> weaponBases = {
            {"hax", {"Hand Axe", {{ 3, 6 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invhax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"axe", {"Axe", {{ 4, 11 }, false, false, { 0, 0 }}, {32, 0}, {2, 3}, false, "invaxe", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"2ax", {"Double Axe", {{ 5, 13 }, false, false, { 0, 0 }}, {43, 0}, {2, 3}, false, "inv2ax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"mpi", {"Military Pick", {{ 7, 11 }, false, false, { 0, 0 }}, {49, 33}, {2, 3}, false, "invmpi", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"wax", {"War Axe", {{ 10, 18 }, false, false, { 0, 0 }}, {67, 0}, {2, 3}, false, "invwax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"lax", {"Large Axe", {{ 0, 0 }, false, true, { 6, 13 }}, {35, 0}, {2, 3}, false, "invlax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"bax", {"Broad Axe", {{ 0, 0 }, false, true, { 10, 18 }}, {48, 0}, {2, 3}, false, "invbrx", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"btx", {"Battle Axe", {{ 0, 0 }, false, true, { 12, 32 }}, {54, 0}, {2, 3}, false, "invbtx", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"gax", {"Great Axe", {{ 0, 0 }, false, true, { 9, 30 }}, {63, 39}, {2, 4}, false, "invgax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"gix", {"Giant Axe", {{ 0, 0 }, false, true, { 22, 45 }}, {70, 0}, {2, 3}, false, "invgix", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"wnd", {"Wand", {{ 2, 4 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invwnd", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"ywn", {"Yew Wand", {{ 2, 8 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invywn", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"bwn", {"Bone Wand", {{ 3, 7 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invbwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"gwn", {"Grim Wand", {{ 5, 11 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invgwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"clb", {"Club", {{ 1, 6 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invclb", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}}},
            {"scp", {"Scepter", {{ 6, 11 }, false, false, { 0, 0 }}, {25, 0}, {1, 3}, false, "invscp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"gsc", {"Grand Scepter", {{ 8, 18 }, false, false, { 0, 0 }}, {37, 0}, {1, 3}, false, "invgsc", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"wsp", {"War Scepter", {{ 10, 17 }, false, false, { 0, 0 }}, {55, 0}, {1, 3}, false, "invwsp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"spc", {"Spiked Club", {{ 5, 8 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invspc", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}}},
            {"mac", {"Mace", {{ 3, 10 }, false, false, { 0, 0 }}, {27, 0}, {1, 3}, false, "invmac", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"mst", {"Morning Star", {{ 7, 16 }, false, false, { 0, 0 }}, {36, 0}, {1, 3}, false, "invmst", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"fla", {"Flail", {{ 1, 24 }, false, false, { 0, 0 }}, {41, 35}, {1, 3}, false, "invfla", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"whm", {"War Hammer", {{ 19, 29 }, false, false, { 0, 0 }}, {53, 0}, {2, 3}, false, "invwhm", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"mau", {"Maul", {{ 0, 0 }, false, true, { 30, 43 }}, {69, 0}, {2, 4}, false, "invmau", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"gma", {"Great Maul", {{ 0, 0 }, false, true, { 38, 58 }}, {99, 0}, {2, 3}, false, "invgma", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"ssd", {"Short Sword", {{ 2, 7 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invssd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"scm", {"Scimitar", {{ 2, 6 }, false, false, { 0, 0 }}, {0, 21}, {1, 3}, false, "invscm", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"sbr", {"Sabre", {{ 3, 8 }, false, false, { 0, 0 }}, {25, 25}, {1, 3}, false, "invsbr", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"flc", {"Falchion", {{ 9, 17 }, false, false, { 0, 0 }}, {33, 0}, {1, 3}, false, "invflc", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"crs", {"Crystal Sword", {{ 5, 15 }, false, false, { 0, 0 }}, {43, 0}, {1, 3}, false, "invcrs", 8, {"Sword", "Melee Weapon", "Weapon"}}},
            {"bsd", {"Broad Sword", {{ 7, 14 }, false, false, { 0, 0 }}, {48, 0}, {1, 3}, false, "invbsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"lsd", {"Long Sword", {{ 3, 19 }, false, false, { 0, 0 }}, {55, 39}, {1, 3}, false, "invlsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"wsd", {"War Sword", {{ 8, 20 }, false, false, { 0, 0 }}, {71, 45}, {1, 3}, false, "invwsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"2hs", {"Two Handed Sword", {{ 2, 9 }, true, true, { 8, 17 }}, {35, 27}, {1, 4}, false, "inv2hs", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"clm", {"Claymore", {{ 5, 12 }, true, true, { 13, 30 }}, {47, 0}, {1, 4}, false, "invclm", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"gis", {"Giant Sword", {{ 3, 16 }, true, true, { 9, 28 }}, {56, 34}, {1, 4}, false, "invgis", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"bsw", {"Bastard Sword", {{ 7, 19 }, true, true, { 20, 28 }}, {62, 0}, {1, 4}, false, "invbsw", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"flb", {"Flamberge", {{ 9, 15 }, true, true, { 13, 26 }}, {70, 49}, {1, 4}, false, "invflb", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"gsd", {"Great Sword", {{ 12, 20 }, true, true, { 25, 42 }}, {100, 60}, {1, 4}, false, "invgsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"dgr", {"Dagger", {{ 1, 4 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invdgr", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"dir", {"Dirk", {{ 3, 9 }, false, false, { 0, 0 }}, {0, 25}, {1, 2}, false, "invdir", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"kri", {"Kris", {{ 2, 11 }, false, false, { 0, 0 }}, {0, 45}, {1, 3}, false, "invkrs", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"bld", {"Blade", {{ 4, 15 }, false, false, { 0, 0 }}, {35, 51}, {1, 3}, false, "invbld", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"tkf", {"Throwing Knife", {{ 2, 3 }, false, false, { 0, 0 }, { 4, 9 }}, {0, 21}, {1, 2}, true, "invtkn", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"tax", {"Throwing Axe", {{ 4, 7 }, false, false, { 0, 0 }, { 8, 12 }}, {0, 40}, {1, 2}, true, "invtax", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"bkf", {"Balanced Knife", {{ 1, 8 }, false, false, { 0, 0 }, { 6, 11 }}, {0, 51}, {1, 2}, true, "invbkf", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"bal", {"Balanced Axe", {{ 5, 10 }, false, false, { 0, 0 }, { 12, 15 }}, {0, 57}, {2, 3}, true, "invbal", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"jav", {"Javelin", {{ 1, 5 }, false, false, { 0, 0 }, { 6, 14 }}, {0, 0}, {1, 3}, true, "invjav", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"pil", {"Pilum", {{ 4, 9 }, false, false, { 0, 0 }, { 7, 20 }}, {0, 45}, {1, 3}, true, "invpil", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"ssp", {"Short Spear", {{ 2, 13 }, false, false, { 0, 0 }, { 10, 22 }}, {40, 40}, {1, 3}, true, "invssp", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"glv", {"Glaive", {{ 5, 17 }, false, false, { 0, 0 }, { 16, 22 }}, {52, 35}, {1, 4}, true, "invglv", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"tsp", {"Throwing Spear", {{ 5, 15 }, false, false, { 0, 0 }, { 12, 30 }}, {0, 65}, {1, 4}, true, "invtsp", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"spr", {"Spear", {{ 0, 0 }, false, true, { 3, 15 }}, {0, 20}, {2, 4}, false, "invspr", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"tri", {"Trident", {{ 0, 0 }, false, true, { 9, 15 }}, {38, 24}, {2, 4}, false, "invtri", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"brn", {"Brandistock", {{ 0, 0 }, false, true, { 7, 17 }}, {40, 50}, {2, 4}, false, "invbrn", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"spt", {"Spetum", {{ 0, 0 }, false, true, { 15, 23 }}, {54, 35}, {2, 4}, false, "invspt", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"pik", {"Pike", {{ 0, 0 }, false, true, { 14, 63 }}, {60, 45}, {2, 4}, false, "invpik", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"bar", {"Bardiche", {{ 0, 0 }, false, true, { 1, 27 }}, {40, 0}, {2, 4}, false, "invbar", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"vou", {"Voulge", {{ 0, 0 }, false, true, { 6, 21 }}, {50, 0}, {2, 4}, false, "invvou", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"scy", {"Scythe", {{ 0, 0 }, false, true, { 8, 20 }}, {41, 41}, {2, 4}, false, "invscy", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"pax", {"Pole Axe", {{ 0, 0 }, false, true, { 18, 39 }}, {62, 0}, {2, 4}, false, "invpax", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"hal", {"Halberd", {{ 0, 0 }, false, true, { 12, 45 }}, {75, 47}, {2, 4}, false, "invhal", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"wsc", {"War Scythe", {{ 0, 0 }, false, true, { 15, 36 }}, {80, 80}, {2, 4}, false, "invwsc", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"sst", {"Short Staff", {{ 0, 0 }, false, true, { 1, 5 }}, {0, 0}, {1, 3}, false, "invsst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"lst", {"Long Staff", {{ 0, 0 }, false, true, { 2, 8 }}, {0, 0}, {1, 4}, false, "invlst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"cst", {"Gnarled Staff", {{ 0, 0 }, false, true, { 4, 12 }}, {0, 0}, {1, 4}, false, "invcst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"bst", {"Battle Staff", {{ 0, 0 }, false, true, { 6, 13 }}, {0, 0}, {1, 4}, false, "invbst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"wst", {"War Staff", {{ 0, 0 }, false, true, { 12, 28 }}, {0, 0}, {2, 4}, false, "invwst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"sbw", {"Short Bow", {{ 0, 0 }, false, true, { 1, 4 }}, {0, 15}, {2, 3}, false, "invsbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"hbw", {"Hunter's Bow", {{ 0, 0 }, false, true, { 2, 6 }}, {0, 28}, {2, 3}, false, "invhbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"lbw", {"Long Bow", {{ 0, 0 }, false, true, { 3, 10 }}, {22, 19}, {2, 4}, false, "invlbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"cbw", {"Composite Bow", {{ 0, 0 }, false, true, { 4, 8 }}, {25, 35}, {2, 3}, false, "invcbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"sbb", {"Short Battle Bow", {{ 0, 0 }, false, true, { 5, 11 }}, {30, 40}, {2, 3}, false, "invsbb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"lbb", {"Long Battle Bow", {{ 0, 0 }, false, true, { 3, 18 }}, {40, 50}, {2, 4}, false, "invlbb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"swb", {"Short War Bow", {{ 0, 0 }, false, true, { 6, 14 }}, {35, 55}, {2, 3}, false, "invswb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"lwb", {"Long War Bow", {{ 0, 0 }, false, true, { 3, 23 }}, {50, 65}, {2, 4}, false, "invlwb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"lxb", {"Light Crossbow", {{ 0, 0 }, false, true, { 6, 9 }}, {21, 27}, {2, 3}, false, "invlxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"mxb", {"Crossbow", {{ 0, 0 }, false, true, { 9, 16 }}, {40, 33}, {2, 3}, false, "invmxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"hxb", {"Heavy Crossbow", {{ 0, 0 }, false, true, { 14, 26 }}, {60, 40}, {2, 4}, false, "invhxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"rxb", {"Repeating Crossbow", {{ 0, 0 }, false, true, { 6, 12 }}, {40, 50}, {2, 3}, false, "invrxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"gps", {"Rancid Gas Potion", {{ 0, 1 }, false, false, { 0, 0 }}, {0, 0, 24}, {1, 1}, true, "invgpl", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"ops", {"Oil Potion", {{ 0, 1 }, false, false, { 0, 0 }}, {0, 0, 20}, {1, 1}, true, "invopl", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"gpm", {"Choking Gas Potion", {0, 1, false, false, 0, 0, 16}, {0, 0}, {1, 1}, true, "invgpm", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"opm", {"Exploding Potion", {{ 0, 1 }, false, false, { 0, 0 }}, {0, 0, 12}, {1, 1}, true, "invopm", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"gpl", {"Strangling Gas Potion", {{ 0, 1 }, false, false, { 0, 0 }}, {0, 0, 6}, {1, 1}, true, "invgps", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"opl", {"Fulminating Potion", {{ 0, 1 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, true, "invops", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"d33", {"Decoy Gidbinn", {{ 1, 2 }, false, false, { 0, 0 }}, {15, 20}, {1, 2}, false, "invd33", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"g33", {"The Gidbinn", {{ 3, 7 }, false, false, { 0, 0 }}, {15, 25}, {1, 2}, false, "invg33", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"leg", {"Wirt's Leg", {{ 2, 8 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invleg", 8, {"Club", "Blunt", "Melee Weapon", "Weapon"}}},
            {"hdm", {"Horadric Malus", {{ 6, 15 }, false, false, { 0, 0 }}, {15, 15}, {1, 2}, false, "invhmr", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"hfh", {"Hellforge Hammer", {{ 6, 15 }, false, false, { 0, 0 }}, {0, 0}, {2, 3}, false, "invhfh", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"hst", {"Horadric Staff", {{ 0, 0 }, false, true, { 12, 20 }}, {30, 0}, {1, 4}, false, "invhst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"msf", {"Staff of the Kings",{0, 0, false, true, 10, 15}, {25, 0}, {1, 3}, false, "invmsf", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9ha", {"Hatchet", {{ 10, 21 }, false, false, { 0, 0 }}, {25, 35, 19}, {1, 3}, false, "invhax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9ax", {"Cleaver", {{ 10, 33 }, false, false, { 0, 0 }}, {68, 0, 22}, {2, 3}, false, "invaxe", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"92a", {"Twin Axe", {{ 13, 38 }, false, false, { 0, 0 }}, {85, 0, 25}, {2, 3}, false, "inv2ax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9mp", {"Crowbill", {{ 14, 34 }, false, false, { 0, 0 }}, {94, 70, 25}, {2, 3}, false, "invmpi", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9wa", {"Naga", {{ 16, 45 }, false, false, { 0, 0 }}, {121, 0, 25}, {2, 3}, false, "invwax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9la", {"Military Axe", {{ 0, 0 }, false, true, { 14, 34 }}, {73, 0, 25}, {2, 3}, false, "invlax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9ba", {"Bearded Axe", {{ 0, 0 }, false, true, { 21, 49 }}, {92, 0, 25}, {2, 3}, false, "invbrx", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9bt", {"Tabar", {{ 0, 0 }, false, true, { 24, 77 }}, {101, 0, 25}, {2, 3}, false, "invbtx", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9ga", {"Gothic Axe", {{ 0, 0 }, false, true, { 18, 70 }}, {115, 79, 25}, {2, 4}, false, "invgax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9gi", {"Ancient Axe", {{ 0, 0 }, false, true, { 43, 85 }}, {125, 0, 25}, {2, 3}, false, "invgix", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"9wn", {"Burnt Wand", {{ 8, 18 }, false, false, { 0, 0 }}, {25, 0, 19}, {1, 2}, false, "invwnd", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9yw", {"Petrified Wand", {{ 8, 24 }, false, false, { 0, 0 }}, {25, 0, 25}, {1, 2}, false, "invywn", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9bw", {"Tomb Wand", {{ 10, 22 }, false, false, { 0, 0 }}, {25, 0, 25}, {1, 2}, false, "invbwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9gw", {"Grave Wand", {{ 13, 29 }, false, false, { 0, 0 }}, {25, 0, 25}, {1, 2}, false, "invgwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9cl", {"Cudgel", {{ 6, 21 }, false, false, { 0, 0 }}, {25, 0, 18}, {1, 3}, false, "invclb", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9sc", {"Rune Scepter", {{ 13, 24 }, false, false, { 0, 0 }}, {58, 0, 19}, {1, 3}, false, "invscp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9qs", {"Holy Water Sprinkler", {{ 14, 36 }, false, false, { 0, 0 }}, {76, 0, 25}, {1, 3}, false, "invgsc", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9ws", {"Divine Scepter", {{ 16, 38 }, false, false, { 0, 0 }}, {103, 0, 25}, {2, 3}, false, "invwsp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9sp", {"Barbed Club", {{ 13, 25 }, false, false, { 0, 0 }}, {30, 0, 20}, {1, 3}, false, "invspc", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9ma", {"Flanged Mace", {{ 15, 23 }, false, false, { 0, 0 }}, {61, 0, 23}, {1, 3}, false, "invmac", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9mt", {"Jagged Star", {{ 20, 31 }, false, false, { 0, 0 }}, {74, 0, 25}, {1, 3}, false, "invmst", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9fl", {"Knout", {{ 13, 35 }, false, false, { 0, 0 }}, {82, 73, 25}, {2, 3}, false, "invfla", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9wh", {"Battle Hammer", {{ 35, 58 }, false, false, { 0, 0 }}, {100, 0, 25}, {2, 3}, false, "invwhm", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9m9", {"War Club", {{ 0, 0 }, false, true, { 53, 78 }}, {124, 0, 25}, {2, 3}, false, "invmau", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9gm", {"Martel De Fer", {{ 0, 0 }, false, true, { 61, 99 }}, {169, 0, 25}, {2, 3}, false, "invgma", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"9ss", {"Gladius", {{ 8, 22 }, false, false, { 0, 0 }}, {25, 0, 18}, {1, 3}, false, "invssd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9sm", {"Cutlass", {{ 8, 21 }, false, false, { 0, 0 }}, {25, 52, 25}, {1, 3}, false, "invscm", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9sb", {"Shamshir", {{ 10, 24 }, false, false, { 0, 0 }}, {58, 58, 25}, {1, 3}, false, "invsbr", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9fc", {"Tulwar", {{ 16, 35 }, false, false, { 0, 0 }}, {70, 42, 25}, {1, 3}, false, "invflc", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9cr", {"Dimensional Blade", {{ 13, 35 }, false, false, { 0, 0 }}, {85, 60, 25}, {2, 3}, false, "invcrs", 8, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9bs", {"Battle Sword", {{ 16, 34 }, false, false, { 0, 0 }}, {92, 43, 25}, {2, 3}, false, "invbsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9ls", {"Rune Sword", {{ 10, 42 }, false, false, { 0, 0 }}, {103, 79, 25}, {2, 3}, false, "invlsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9wd", {"Ancient Sword",{18, 43, false, false, 0, 0}, {127, 88, 25}, {1, 3}, false, "invwsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"92h", {"Espandon", {{ 8, 26 }, true, true, { 18, 40 }}, {73, 61, 25}, {1, 4}, false, "inv2hs", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9cm", {"Dacian Falx", {{ 13, 30 }, true, true, { 26, 61 }}, {91, 20, 25}, {1, 4}, false, "invclm", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9gs", {"Tusk Sword", {{ 10, 37 }, true, true, { 19, 58 }}, {104, 71, 25}, {1, 4}, false, "invgis", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9b9", {"Gothic Sword", {{ 14, 40 }, true, true, { 39, 60 }}, {113, 20, 25}, {1, 4}, false, "invbsw", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9fb", {"Zweihander", {{ 19, 35 }, true, true, { 29, 54 }}, {125, 94, 25}, {2, 4}, false, "invflb", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9gd", {"Executioner Sword", {{ 24, 40 }, true, true, { 47, 80 }}, {170, 110, 25}, {2, 4}, false, "invgsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"9dg", {"Poignard", {{ 6, 18 }, false, false, { 0, 0 }}, {25, 0, 19}, {1, 2}, false, "invdgr", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"9di", {"Rondel", {{ 10, 26 }, false, false, { 0, 0 }}, {25, 58, 24}, {1, 2}, false, "invdir", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"9kr", {"Ciquedeas", {{ 15, 31 }, false, false, { 0, 0 }}, {25, 88, 25}, {1, 3}, false, "invkrs", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"9bl", {"Stiletto", {{ 19, 36 }, false, false, { 0, 0 }}, {47, 97, 25}, {1, 3}, false, "invbld", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"9tk", {"Battle Dart", {{ 8, 16 }, false, false, { 0, 0 }, { 11, 24 }}, {25, 52, 19}, {1, 2}, true, "invtkn", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"9ta", {"Francisca", {{ 11, 22 }, false, false, { 0, 0 }, { 18, 33 }}, {25, 80, 22}, {1, 2}, true, "invtax", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"9bk", {"War Dart", {{ 6, 24 }, false, false, { 0, 0 }, { 14, 27 }}, {25, 97, 25}, {1, 2}, true, "invbkf", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"9b8", {"Hurlbat", {{ 13, 27 }, false, false, { 0, 0 }, { 24, 38 }}, {25, 106, 25}, {2, 3}, true, "invbal", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"9ja", {"War Javelin", {{ 6, 19 }, false, false, { 0, 0 }, { 14, 32 }}, {25, 25, 18}, {1, 3}, true, "invjav", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9pi", {"Great Pilum", {{ 11, 26 }, false, false, { 0, 0 }, { 16, 42 }}, {25, 88, 25}, {1, 3}, true, "invpil", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9s9", {"Simbilan", {{ 8, 32 }, false, false, { 0, 0 }, { 27, 50 }}, {80, 80, 25}, {1, 3}, true, "invssp", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9gl", {"Spiculum", {{ 13, 38 }, false, false, { 0, 0 }, { 32, 60 }}, {98, 73, 25}, {1, 4}, true, "invglv", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9ts", {"Harpoon", {{ 13, 35 }, false, false, { 0, 0 }, { 18, 54 }}, {25, 118, 25}, {1, 4}, true, "invtsp", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9sr", {"War Spear", {{ 0, 0 }, false, true, { 10, 37 }}, {25, 25, 21}, {2, 4}, false, "invspr", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"9tr", {"Fuscina", {{ 0, 0 }, false, true, { 19, 37 }}, {77, 25, 24}, {2, 4}, false, "invtri", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"9br", {"War Fork", {{ 0, 0 }, false, true, { 16, 40 }}, {80, 95, 25}, {2, 4}, false, "invbrn", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"9st", {"Yari", {{ 0, 0 }, false, true, { 29, 59 }}, {101, 0, 25}, {2, 4}, false, "invspt", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"9p9", {"Lance", {{ 0, 0 }, false, true, { 27, 114 }}, {110, 88, 25}, {2, 4}, false, "invpik", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"9b7", {"Lochaber Axe", {{ 0, 0 }, false, true, { 6, 58 }}, {80, 0, 21}, {2, 4}, false, "invbar", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"9vo", {"Bill", {{ 0, 0 }, false, true, { 14, 53 }}, {95, 0, 25}, {2, 4}, false, "invvou", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"9s8", {"Battle Scythe", {0, 0, false, true, 18, 45, 25}, {82, 82}, {2, 4}, false, "invscy", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"9pa", {"Partizan", {{ 0, 0 }, false, true, { 34, 75 }}, {113, 67, 25}, {2, 4}, false, "invpax", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"9h9", {"Bec-De-Corbin", {{ 0, 0 }, false, true, { 13, 85 }}, {133, 91, 25}, {2, 4}, false, "invhal", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"9wc", {"Grim Scythe", {{ 0, 0 }, false, true, { 30, 70 }}, {140, 140, 25}, {2, 4}, false, "invwsc", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"8ss", {"Jo Staff", {{ 0, 0 }, false, true, { 6, 21 }}, {25, 0, 18}, {1, 3}, false, "invsst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"8ls", {"Quarterstaff", {{ 0, 0 }, false, true, { 8, 26 }}, {25, 0, 23}, {1, 4}, false, "invlst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"8cs", {"Cedar Staff", {{ 0, 0 }, false, true, { 11, 32 }}, {25, 0, 25}, {1, 4}, false, "invcst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"8bs", {"Gothic Staff", {{ 0, 0 }, false, true, { 14, 34 }}, {25, 0, 25}, {1, 4}, false, "invbst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"8ws", {"Rune Staff", {{ 0, 0 }, false, true, { 24, 58 }}, {25, 0, 25}, {1, 4}, false, "invwst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"8sb", {"Edge Bow", {{ 0, 0 }, false, true, { 6, 19 }}, {25, 53, 18}, {2, 3}, false, "invsbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"8hb", {"Razor Bow", {{ 0, 0 }, false, true, { 8, 22 }}, {25, 62, 21}, {2, 3}, false, "invhbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"8lb", {"Cedar Bow", {{ 0, 0 }, false, true, { 10, 29 }}, {53, 49, 23}, {2, 4}, false, "invlbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"8cb", {"Double Bow", {{ 0, 0 }, false, true, { 11, 26 }}, {58, 73, 25}, {2, 3}, false, "invcbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"8s8", {"Short Siege Bow", {{ 0, 0 }, false, true, { 13, 30 }}, {65, 80, 25}, {2, 3}, false, "invsbb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"8l8", {"Long Siege Bow", {{ 0, 0 }, false, true, { 10, 42 }}, {80, 95, 25}, {2, 4}, false, "invlbb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"8sw", {"Rune Bow", {{ 0, 0 }, false, true, { 14, 35 }}, {73, 103, 25}, {2, 3}, false, "invswb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"8lw", {"Gothic Bow", {{ 0, 0 }, false, true, { 10, 50 }}, {95, 118, 25}, {2, 4}, false, "invlwb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"8lx", {"Arbalest", {{ 0, 0 }, false, true, { 14, 27 }}, {52, 61, 22}, {2, 3}, false, "invlxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"8mx", {"Siege Crossbow", {{ 0, 0 }, false, true, { 20, 42 }}, {80, 70, 25}, {2, 3}, false, "invmxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"8hx", {"Ballista", {{ 0, 0 }, false, true, { 33, 55 }}, {110, 80, 25}, {2, 4}, false, "invhxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"8rx", {"Chu-Ko-Nu", {{ 0, 0 }, false, true, { 14, 32 }}, {80, 95, 25}, {2, 3}, false, "invrxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"qf1", {"Khalim's Flail", {{ 1, 15 }, false, false, { 0, 0 }}, {41, 35}, {2, 3}, false, "invqf1", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"qf2", {"Khalim's Will", {{ 1, 15 }, false, false, { 0, 0 }}, {0, 0}, {2, 3}, false, "invqf2", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"ktr", {"Katar", {{ 4, 7 }, false, false, { 0, 0 }}, {20, 20}, {1, 3}, false, "invktr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"wrb", {"Wrist Blade", {{ 5, 9 }, false, false, { 0, 0 }}, {33, 33}, {1, 3}, false, "invktr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"axf", {"Hatchet Hands", {{ 2, 15 }, false, false, { 0, 0 }}, {37, 37}, {1, 3}, false, "invaxf", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"ces", {"Cestus", {{ 7, 15 }, false, false, { 0, 0 }}, {42, 42}, {1, 3}, false, "invaxf", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"clw", {"Claws", {{ 8, 15 }, false, false, { 0, 0 }}, {46, 46}, {1, 3}, false, "invclw", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"btl", {"Blade Talons", {{ 10, 14 }, false, false, { 0, 0 }}, {50, 50}, {1, 3}, false, "invclw", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"skr", {"Scissors Katar", {{ 9, 17 }, false, false, { 0, 0 }}, {55, 55}, {1, 2}, false, "invskr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"9ar", {"Quhab", {{ 11, 24 }, false, false, { 0, 0 }}, {57, 57, 21}, {1, 2}, false, "invskr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"9wb", {"Wrist Spike", {{ 13, 27 }, false, false, { 0, 0 }}, {66, 66, 24}, {1, 3}, false, "invktr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"9xf", {"Fascia", {{ 8, 37 }, false, false, { 0, 0 }}, {69, 69, 27}, {1, 2}, false, "invktr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"9cs", {"Hand Scythe", {{ 16, 37 }, false, false, { 0, 0 }}, {73, 73, 30}, {1, 2}, false, "invaxf", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"9lw", {"Greater Claws", {{ 18, 37 }, false, false, { 0, 0 }}, {76, 76, 33}, {1, 2}, false, "invclw", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"9tw", {"Greater Talons", {{ 21, 35 }, false, false, { 0, 0 }}, {79, 79, 37}, {1, 3}, false, "invclw", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"9qr", {"Scissors Quhab", {{ 19, 40 }, false, false, { 0, 0 }}, {82, 82, 40}, {1, 3}, false, "invskr", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"7ar", {"Suwayyah", {{ 39, 52 }, false, false, { 0, 0 }}, {99, 99, 44}, {1, 3}, false, "invktr", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"7wb", {"Wrist Sword", {{ 34, 45 }, false, false, { 0, 0 }}, {105, 105, 46}, {1, 3}, false, "invktr", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"7xf", {"War Fist", {{ 44, 53 }, false, false, { 0, 0 }}, {108, 108, 51}, {1, 3}, false, "invaxf", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"7cs", {"Battle Cestus", {{ 36, 42 }, false, false, { 0, 0 }}, {110, 110, 54}, {1, 3}, false, "invaxf", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"7lw", {"Feral Claws", {{ 22, 53 }, false, false, { 0, 0 }}, {113, 113, 58}, {1, 3}, false, "invclw", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"7tw", {"Runic Talons", {{ 24, 44 }, false, false, { 0, 0 }}, {115, 115, 60}, {1, 3}, false, "invclw", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"7qr", {"Scissors Suwayh", {{ 40, 51 }, false, false, { 0, 0 }}, {118, 188, 64}, {1, 3}, false, "invskr", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}}},
            {"7ha", {"Tomahawk", {{ 33, 58 }, false, false, { 0, 0 }}, {125, 67, 40}, {1, 3}, false, "invhax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7ax", {"Small Crescent", {{ 38, 60 }, false, false, { 0, 0 }}, {115, 83, 45}, {2, 3}, false, "invaxe", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"72a", {"Ettin Axe", {{ 33, 66 }, false, false, { 0, 0 }}, {145, 45, 52}, {2, 3}, false, "inv2ax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7mp", {"War Spike", {{ 30, 48 }, false, false, { 0, 0 }}, {133, 54, 59}, {2, 3}, false, "invmpi", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7wa", {"Berserker Axe", {{ 24, 71 }, false, false, { 0, 0 }}, {138, 59, 64}, {2, 3}, false, "invwax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7la", {"Feral Axe", {{ 0, 0 }, false, true, { 25, 123 }}, {196, 0, 42}, {2, 3}, false, "invlax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7ba", {"Silver Edged Ax", {{ 0, 0 }, false, true, { 62, 110 }}, {166, 65, 48}, {2, 3}, false, "invbrx", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7bt", {"Decapitator", {{ 0, 0 }, false, true, { 49, 137 }}, {189, 33, 54}, {2, 3}, false, "invbtx", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7ga", {"Champion Axe", {{ 0, 0 }, false, true, { 59, 94 }}, {167, 59, 61}, {2, 4}, false, "invgax", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7gi", {"Glorious Axe", {{ 0, 0 }, false, true, { 60, 124 }}, {164, 55, 66}, {2, 3}, false, "invgix", 2, {"Axe", "Melee Weapon", "Weapon"}}},
            {"7wn", {"Polished Wand", {{ 18, 33 }, false, false, { 0, 0 }}, {25, 0, 41}, {1, 2}, false, "invwnd", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7yw", {"Ghost Wand", {{ 20, 40 }, false, false, { 0, 0 }}, {25, 0, 48}, {1, 2}, false, "invywn", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7bw", {"Lich Wand", {{ 10, 31 }, false, false, { 0, 0 }}, {25, 0, 56}, {1, 2}, false, "invbwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7gw", {"Unearthed Wand", {{ 22, 28 }, false, false, { 0, 0 }}, {25, 0, 64}, {1, 2}, false, "invgwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7cl", {"Truncheon", {{ 35, 43 }, false, false, { 0, 0 }}, {88, 43, 39}, {1, 3}, false, "invclb", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7sc", {"Mighty Sceptre", {{ 40, 52 }, false, false, { 0, 0 }}, {125, 65, 46}, {1, 3}, false, "invscp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7qs", {"Seraph Rod", {{ 45, 54 }, false, false, { 0, 0 }}, {108, 69, 57}, {1, 3}, false, "invgsc", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7ws", {"Caduceus", {{ 37, 43 }, false, false, { 0, 0 }}, {97, 70, 66}, {2, 3}, false, "invwsp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7sp", {"Tyrant Club", {{ 32, 58 }, false, false, { 0, 0 }}, {133, 0, 42}, {1, 3}, false, "invspc", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7ma", {"Reinforced Mace", {{ 41, 49 }, false, false, { 0, 0 }}, {145, 46, 47}, {1, 3}, false, "invmac", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7mt", {"Devil Star", {{ 43, 53 }, false, false, { 0, 0 }}, {153, 44, 52}, {1, 3}, false, "invmst", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7fl", {"Scourge", {{ 3, 80 }, false, false, { 0, 0 }}, {125, 77, 57}, {2, 3}, false, "invfla", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7wh", {"Legendary Mallt", {{ 50, 61 }, false, false, { 0, 0 }}, {189, 0, 61}, {2, 3}, false, "invwhm", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7m7", {"Ogre Maul", {{ 0, 0 }, false, true, { 77, 106 }}, {225, 0, 51}, {2, 4}, false, "invmau", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7gm", {"Thunder Maul", {{ 0, 0 }, false, true, { 33, 180 }}, {253, 0, 65}, {2, 3}, false, "invgma", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}}},
            {"7ss", {"Falcata", {{ 31, 59 }, false, false, { 0, 0 }}, {150, 88, 42}, {1, 3}, false, "invssd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7sm", {"Ataghan", {{ 26, 46 }, false, false, { 0, 0 }}, {138, 95, 45}, {1, 3}, false, "invscm", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7sb", {"Elegant Blade", {{ 33, 45 }, false, false, { 0, 0 }}, {109, 122, 47}, {1, 3}, false, "invsbr", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7fc", {"Hydra Edge", {{ 28, 68 }, false, false, { 0, 0 }}, {142, 105, 51}, {1, 3}, false, "invflc", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7cr", {"Phase Blade", {{ 31, 35 }, false, false, { 0, 0 }}, {25, 136, 54}, {2, 3}, false, "invcrs", 8, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7bs", {"Conquest Sword", {{ 37, 53 }, false, false, { 0, 0 }}, {142, 112, 58}, {2, 3}, false, "invbsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7ls", {"Cryptic Sword", {{ 5, 77 }, false, false, { 0, 0 }}, {99, 109, 61}, {2, 3}, false, "invlsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7wd", {"Mythical Sword", {{ 40, 50 }, false, false, { 0, 0 }}, {147, 124, 66}, {1, 3}, false, "invwsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"72h", {"Legend Sword", {{ 22, 56 }, true, true, { 50, 94 }}, {175, 100, 44}, {1, 4}, false, "inv2hs", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7cm", {"Highland Blade", {{ 22, 62 }, true, true, { 67, 96 }}, {171, 104, 49}, {1, 4}, false, "invclm", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7gs", {"Balrog Blade", {{ 15, 75 }, true, true, { 55, 118 }}, {185, 87, 53}, {1, 4}, false, "invgis", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7b7", {"Champion Sword", {{ 24, 54 }, true, true, { 71, 83 }}, {163, 103, 57}, {1, 4}, false, "invbsw", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7fb", {"Colossal Sword", {{ 26, 70 }, true, true, { 61, 121 }}, {182, 95, 60}, {2, 4}, false, "invflb", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7gd", {"Colossus Blade", {{ 25, 65 }, true, true, { 58, 115 }}, {189, 110, 63}, {2, 4}, false, "invgsd", 2, {"Sword", "Melee Weapon", "Weapon"}}},
            {"7dg", {"Bone Knife", {{ 23, 49 }, false, false, { 0, 0 }}, {38, 75, 43}, {1, 2}, false, "invdgr", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"7di", {"Mithral Point", {{ 37, 53 }, false, false, { 0, 0 }}, {55, 98, 52}, {1, 2}, false, "invdir", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"7kr", {"Fanged Knife", {{ 15, 57 }, false, false, { 0, 0 }}, {42, 86, 62}, {1, 3}, false, "invkrs", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"7bl", {"Legend Spike", {{ 31, 47 }, false, false, { 0, 0 }}, {65, 67, 66}, {1, 3}, false, "invbld", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"7tk", {"Flying Knife", {{ 23, 54 }, false, false, { 0, 0 }, { 23, 54 }}, {48, 141, 48}, {1, 2}, true, "invtkn", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"7ta", {"Flying Axe", {{ 17, 65 }, false, false, { 0, 0 }, { 15, 66 }}, {88, 108, 42}, {1, 2}, true, "invtax", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"7bk", {"Winged Knife", {{ 27, 35 }, false, false, { 0, 0 }, { 23, 39 }}, {45, 142, 57}, {1, 2}, true, "invbkf", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"7b8", {"Winged Axe", {{ 11, 56 }, false, false, { 0, 0 }, { 7, 60 }}, {96, 122, 60}, {2, 3}, true, "invbal", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"7ja", {"Hyperion Javeln", {{ 21, 57 }, false, false, { 0, 0 }, { 28, 55 }}, {98, 123, 40}, {1, 3}, true, "invjav", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7pi", {"Stygian Pilum", {{ 14, 64 }, false, false, { 0, 0 }, { 21, 75 }}, {118, 112, 46}, {1, 3}, true, "invpil", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7s7", {"Balrog Spear", {{ 33, 63 }, false, false, { 0, 0 }, { 40, 62 }}, {127, 95, 53}, {1, 3}, true, "invssp", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7gl", {"Ghost Glaive", {{ 19, 60 }, false, false, { 0, 0 }, { 30, 85 }}, {89, 137, 59}, {1, 4}, true, "invglv", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7ts", {"Winged Harpoon", {{ 27, 35 }, false, false, { 0, 0 }, { 11, 77 }}, {76, 145, 65}, {1, 4}, true, "invtsp", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7sr", {"Hyperion Spear", {{ 0, 0 }, false, true, { 35, 119 }}, {155, 120, 43}, {2, 4}, false, "invspr", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"7tr", {"Stygian Pike", {{ 0, 0 }, false, true, { 29, 144 }}, {168, 97, 49}, {2, 4}, false, "invtri", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"7br", {"Mancatcher", {{ 0, 0 }, false, true, { 42, 92 }}, {132, 134, 55}, {2, 4}, false, "invbrn", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"7st", {"Ghost Spear", {{ 0, 0 }, false, true, { 18, 155 }}, {122, 163, 62}, {2, 4}, false, "invspt", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"7p7", {"War Pike", {{ 0, 0 }, false, true, { 33, 178 }}, {165, 106, 66}, {2, 4}, false, "invpik", 8, {"Spear", "Melee Weapon", "Weapon"}}},
            {"7o7", {"Ogre Axe", {{ 0, 0 }, false, true, { 28, 145 }}, {195, 75, 45}, {2, 4}, false, "invbar", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"7vo", {"Colossus Voulge", {{ 0, 0 }, false, true, { 17, 165 }}, {210, 55, 48}, {2, 4}, false, "invvou", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"7s8", {"Thresher", {{ 0, 0 }, false, true, { 12, 141 }}, {152, 118, 53}, {2, 4}, false, "invscy", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"7pa", {"Cryptic Axe", {{ 0, 0 }, false, true, { 33, 150 }}, {165, 103, 59}, {2, 4}, false, "invpax", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"7h7", {"Great Poleaxe", {{ 0, 0 }, false, true, { 46, 127 }}, {179, 99, 63}, {2, 4}, false, "invhal", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"7wc", {"Giant Thresher", {{ 0, 0 }, false, true, { 40, 114 }}, {188, 140, 66}, {2, 4}, false, "invwsc", 2, {"Polearm", "Melee Weapon", "Weapon"}}},
            {"6ss", {"Walking Stick", {{ 0, 0 }, false, true, { 69, 85 }}, {25, 0, 43}, {1, 3}, false, "invsst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"6ls", {"Stalagmite", {{ 0, 0 }, false, true, { 75, 107 }}, {63, 35, 49}, {1, 4}, false, "invlst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"6cs", {"Elder Staff", {{ 0, 0 }, false, true, { 80, 93 }}, {44, 37, 55}, {1, 4}, false, "invcst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"6bs", {"Shillelagh", {{ 0, 0 }, false, true, { 65, 108 }}, {52, 27, 62}, {1, 4}, false, "invbst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"6ws", {"Archon Staff", {{ 0, 0 }, false, true, { 83, 99 }}, {34, 0, 66}, {2, 4}, false, "invwst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}}},
            {"6sb", {"Spider Bow", {{ 0, 0 }, false, true, { 23, 50 }}, {64, 143, 41}, {2, 3}, false, "invsbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"6hb", {"Blade Bow", {{ 0, 0 }, false, true, { 21, 41 }}, {76, 119, 45}, {2, 3}, false, "invhbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"6lb", {"Shadow Bow", {{ 0, 0 }, false, true, { 15, 59 }}, {52, 188, 47}, {2, 4}, false, "invlbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"6cb", {"Great Bow", {{ 0, 0 }, false, true, { 12, 52 }}, {121, 107, 51}, {2, 3}, false, "invcbw", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"6s7", {"Diamond Bow", {{ 0, 0 }, false, true, { 33, 40 }}, {89, 132, 54}, {2, 3}, false, "invsbb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"6l7", {"Crusader Bow", {{ 0, 0 }, false, true, { 15, 63 }}, {97, 121, 57}, {2, 4}, false, "invlbb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"6sw", {"Ward Bow", {{ 0, 0 }, false, true, { 20, 53 }}, {72, 146, 60}, {2, 3}, false, "invswb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"6lw", {"Hydra Bow", {{ 0, 0 }, false, true, { 10, 68 }}, {134, 160, 63}, {2, 4}, false, "invlwb", 8, {"Bow", "Missile Weapon", "Weapon"}}},
            {"6lx", {"Pellet Bow", {{ 0, 0 }, false, true, { 28, 73 }}, {83, 155, 42}, {2, 3}, false, "invlxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"6mx", {"Gorgon Crossbow", {{ 0, 0 }, false, true, { 25, 87 }}, {117, 105, 50}, {2, 3}, false, "invmxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"6hx", {"Colossus Crossbow", {{ 0, 0 }, false, true, { 32, 91 }}, {163, 77, 56}, {2, 4}, false, "invhxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"6rx", {"Demon Crossbow", {{ 0, 0 }, false, true, { 26, 40 }}, {141, 98, 63}, {2, 3}, false, "invrxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}}},
            {"ob1", {"Eagle Orb", {{ 2, 5 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invob1", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"ob2", {"Sacred Globe", {{ 3, 8 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invob2", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"ob3", {"Smoked Sphere", {{ 4, 10 }, false, false, { 0, 0 }}, {0, 0, 8}, {1, 2}, false, "invob3", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"ob4", {"Clasped Orb", {{ 5, 12 }, false, false, { 0, 0 }}, {0, 0, 13}, {1, 2}, false, "invob4", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"ob5", {"Dragon Stone", {{ 8, 18 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 3}, false, "invob5", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"am1", {"Stag Bow", {{ 0, 0 }, false, true, { 7, 12 }}, {30, 45, 14}, {2, 4}, false, "invam1", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"am2", {"Reflex Bow", {{ 0, 0 }, false, true, { 9, 19 }}, {35, 60, 20}, {2, 4}, false, "invam2", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"am3", {"Maiden Spear", {{ 0, 0 }, false, true, { 18, 24 }}, {54, 40, 14}, {2, 4}, false, "invam3", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"am4", {"Maiden Pike", {{ 0, 0 }, false, true, { 23, 55 }}, {63, 52, 20}, {2, 4}, false, "invam4", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"am5", {"Maiden Javelin", {{ 8, 14 }, false, false, { 0, 0 }, { 6, 22 }}, {33, 47, 17}, {1, 3}, true, "invam5", 2, {"Amazon Javelin", "Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"ob6", {"Glowing Orb", {{ 8, 21 }, false, false, { 0, 0 }}, {0, 0, 24}, {1, 2}, false, "invob1", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"ob7", {"Crystalline Glb", {{ 10, 26 }, false, false, { 0, 0 }}, {0, 0, 27}, {1, 2}, false, "invob2", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"ob8", {"Cloudy Sphere", {{ 11, 29 }, false, false, { 0, 0 }}, {0, 0, 30}, {1, 2}, false, "invob3", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"ob9", {"Sparkling Ball", {{ 13, 32 }, false, false, { 0, 0 }}, {0, 0, 34}, {1, 2}, false, "invob4", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"oba", {"Swirling Crystal", {{ 18, 42 }, false, false, { 0, 0 }}, {0, 0, 37}, {1, 3}, false, "invob5", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"am6", {"Ashwood Bow", {{ 0, 0 }, false, true, { 16, 29 }}, {56, 77, 29}, {2, 4}, false, "invam1", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"am7", {"Ceremonial Bow", {{ 0, 0 }, false, true, { 19, 41 }}, {73, 110, 35}, {2, 4}, false, "invam2", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"am8", {"Ceremonial Spear", {{ 0, 0 }, false, true, { 34, 51 }}, {101, 80, 32}, {2, 4}, false, "invam3", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"am9", {"Ceremonial Pike", {{ 0, 0 }, false, true, { 42, 101 }}, {115, 98, 38}, {2, 4}, false, "invam4", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"ama", {"Ceremonial Javelin", {{ 18, 35 }, false, false, { 0, 0 }, { 18, 54 }}, {25, 109, 26}, {1, 3}, true, "invam5", 2, {"Amazon Javelin", "Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"obb", {"Heavenly Stone", {{ 21, 46 }, false, false, { 0, 0 }}, {0, 0, 44}, {1, 2}, false, "invob1", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"obc", {"Eldritch Orb", {{ 18, 50 }, false, false, { 0, 0 }}, {0, 0, 50}, {1, 2}, false, "invob2", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"obd", {"Demon Heart", {{ 23, 55 }, false, false, { 0, 0 }}, {0, 0, 56}, {1, 2}, false, "invob3", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"obe", {"Vortex Orb", {{ 12, 66 }, false, false, { 0, 0 }}, {0, 0, 63}, {1, 2}, false, "invob4", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"obf", {"Dimensional Shard", {{ 30, 53 }, false, false, { 0, 0 }}, {0, 0, 66}, {1, 3}, false, "invob5", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}}},
            {"amb", {"Matriarchal Bow", {{ 0, 0 }, false, true, { 20, 47 }}, {87, 187, 39}, {2, 4}, false, "invam1", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"amc", {"Grand Matron Bow", {{ 0, 0 }, false, true, { 14, 72 }}, {108, 152, 58}, {2, 4}, false, "invam2", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"amd", {"Matriarchal Spear", {{ 0, 0 }, false, true, { 65, 95 }}, {114, 142, 45}, {2, 4}, false, "invam3", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"ame", {"Matriarchal Pike", {{ 0, 0 }, false, true, { 37, 153 }}, {132, 149, 60}, {2, 4}, false, "invam4", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"amf", {"Matriarchal Javelin", {{ 30, 54 }, false, false, { 0, 0 }, { 35, 66 }}, {107, 151, 48}, {1, 3}, true, "invam5", 2, {"Amazon Javelin", "Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = weaponBases.find(testStr);
        if (iter != weaponBases.end())
        {
            return iter->second;
        }

        return s_invalidItemType;
    }

    const ItemType& getTomeItemType(const std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, ItemType> tomeBases = {
            {"tbk", {"Tome of Town Portal", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, true, "invbbk", 0, {"Book", "Miscellaneous"}}},
            {"ibk", {"Tome of Identify", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, true, "invrbk", 0, {"Book", "Miscellaneous"}}},
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = tomeBases.find(testStr);
        if (iter != tomeBases.end())
        {
            return iter->second;
        }

        return s_invalidItemType;
    }

    const ItemType& getStackableItemType(const std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, ItemType> stackableBases = {
            {"gld", {"gold", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, true, "invgld", 0, {"Gold", "Miscellaneous"}}},
            {"aqv", {"Arrows", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, true, "invqvr", 0, {"Bow Quiver", "Missile", "Miscellaneous"}}},
            {"cqv", {"Bolts", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, true, "invcqv", 0, {"Crossbow Quiver", "Missile", "Miscellaneous"}}},
            {"key", {"Key", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, true, "invkey", 0, {"Key", "Miscellaneous"}}},
            {"rps", {"Small Red Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, true, "invrps", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"rpl", {"Large Red Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, true, "invrpl", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"bps", {"Small Blue Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, true, "invbps", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
            {"bpl", {"Large Blue Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, true, "invbpl", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = stackableBases.find(testStr);
        if (iter != stackableBases.end())
        {
            return iter->second;
        }

        // Could be a Tome
        {
            const auto& result = getTomeItemType(strcode);
            if (result.stackable)
            {
                return result;
            }
        }

        // Could be a weapon
        {
            const auto& result = getWeaponItemType(strcode);
            if (result.stackable)
            {
                return result;
            }
        }

        return s_invalidItemType;
    }

    const ItemType& getMiscItemType(const std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, ItemType> essenceBases = {
            {"ice", {"Potion of Thawing", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invxyz", 0, {"Quest"}}},
            {"0sc", {"Scroll", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invrsc", 0, {"Scroll", "Miscellaneous"}}},
            {"amu", {"Amulet", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invamu", 0, {"Amulet", "Miscellaneous"}}},
            {"tch", {"Torch", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invtrch", 0, {"Torch", "Miscellaneous"}}},
            {"std", {"Standard of Heroes", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 90}, {1, 1}, false, "invflag", 0, {"Quest"}}},
            {"toa", {"Token of Absolution", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invtoa", 0, {"Quest"}}},
            {"tr1", {"Scroll of Horadric", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invhscr", 0, {"Quest"}}},
            {"tr2", {"Scroll of Malah", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invscb", 0, {"Quest"}}},
            {"vip", {"Viper Amulet", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invvip", 0, {"Amulet", "Miscellaneous"}}},
            {"bey", {"Baal's Eye", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "inveye", 0, {"Quest"}}},
            {"bks", {"Scroll of Inifuss", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invscb", 0, {"Quest"}}},
            {"bkd", {"Key to the Cairn Stones", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invscb", 0, {"Quest"}}},
            {"dhn", {"Diablo's Horn", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invfang", 0, {"Quest"}}},
            {"g34", {"Gold Bird", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invgbi", 0, {"Quest"}}},
            {"hrb", {"Herb", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invhrb", 0, {"Herb", "Miscellaneous"}}},
            {"box", {"Horadric Cube", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2, 3, 4}, false, "invbox", 0, {"Quest"}}},
            {"pk3", {"Key of Destruction", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invmph", 0, {"Quest"}}},
            {"pk2", {"Key of Hate", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invmph", 0, {"Quest"}}},
            {"pk1", {"Key of Terror", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invmph", 0, {"Quest"}}},
            {"qbr", {"Khalim's Brain", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invbrnz", 0, {"Quest"}}},
            {"qey", {"Khalim's Eye", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "inveye", 0, {"Quest"}}},
            {"qhr", {"Khalim's Heart", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invhrt", 0, {"Quest"}}},
            {"bbb", {"Lam Esen's Tome", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invbbb", 0, {"Quest"}}},
            {"mbr", {"Mephisto's Brain", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invbrnz", 0, {"Quest"}}},
            {"luv", {"Mephisto Key", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invkey", 0, {"Key", "Miscellaneous"}}},
            {"mss", {"Mephisto Soul Stone", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invmss", 0, {"Quest"}}},
            {"ass", {"Book of Skill", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invsbk", 0, {"Quest"}}},
            {"rin", {"Ring", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invrin", 0, {"Ring", "Miscellaneous"}}},
            {"xyz", {"Potion of Life", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invxyz", 0, {"Quest"}}},
            {"tsc", {"Town Portal Scroll", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invbsc", 0, {"Scroll", "Miscellaneous"}}},
            {"isc", {"Identify Scroll", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invrsc", 0, {"Scroll", "Miscellaneous"}}},
            {"j34", {"Jade Figurine", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invjbi", 0, {"Quest"}}},
            {"tes", {"Twisted Essence of Suffering", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invtes", 0, {"Quest"}}},
            {"ceh", {"Charged Essense of Hatred", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invceh", 0, {"Quest"}}},
            {"bet", {"Burning Essence of Terror", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invbet", 0, {"Quest"}}},
            {"fed", {"Festering Essence of Destruction", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invfed", 0, {"Quest"}}},
            {"cm1", {"Small Charm", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invchm1", 0, {"Small Charm", "Charm", "Miscellaneous"}}},
            {"cm2", {"Large Charm", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invchm2", 0, {"Medium Charm", "Charm", "Miscellaneous"}}},
            {"cm3", {"Grand Charm", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invchm3", 0, {"Large Charm", "Charm", "Miscellaneous"}}},
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = essenceBases.find(testStr);
        if (iter != essenceBases.end())
        {
            return iter->second;
        }

        return s_invalidItemType;
    }

    const ItemType& getGPSItemType(const std::uint8_t(&strcode)[4])
    {
        static const std::map<std::string, ItemType> essenceBases = {
            {"elx", {"Elixer of Vitality", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invpot", 0, {"Elixir", "Miscellaneous"}}},
            {"hpo", {"Healing Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invrps", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"mpo", {"Mana Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invbps", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
            {"hpf", {"Full Healing Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invrpl", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"mpf", {"Full Mana Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invbpl", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
            {"vps", {"Stamina Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invwps", 0, {"Stamina Potion", "Potion", "Miscellaneous"}}},
            {"yps", {"Antidote Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invnps", 0, {"Antidote Potion", "Potion", "Miscellaneous"}}},
            {"rvs", {"Rejuvenation Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invvps", 0, {"Rejuv Potion", "Healing Potion", "Mana Potion", "Potion", "Miscellaneous"}}},
            {"rvl", {"Full Rejuvenation Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invvpl", 0, {"Rejuv Potion", "Healing Potion", "Mana Potion", "Potion", "Miscellaneous"}}},
            {"wms", {"Thawing Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invyps", 0, {"Thawing Potion", "Potion", "Miscellaneous"}}},
            {"gcv", {"Chipped Amethyst", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 1}, {1, 1}, false, "invgsva", 0, {"Amethyst", "Gem", "Socket Filler", "Miscellaneous", "Chipped Gem"}}},
            {"gfv", {"Flawed Amethyst", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 5}, {1, 1}, false, "invgsvb", 0, {"Amethyst", "Gem", "Socket Filler", "Miscellaneous", "Flawed Gem"}}},
            {"gsv", {"Amethyst", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 12}, {1, 1}, false, "invgsvc", 0, {"Amethyst", "Gem", "Socket Filler", "Miscellaneous", "Standard Gem"}}},
            {"gzv", {"Flawless Amethyst", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invgsvd", 0, {"Amethyst", "Gem", "Socket Filler", "Miscellaneous", "Flawless Gem"}}},
            {"gpv", {"Perfect Amethyst", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 1}, false, "invgsve", 0, {"Amethyst", "Gem", "Socket Filler", "Miscellaneous", "Perfect Gem"}}},
            {"gcy", {"Chipped Topaz", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 1}, {1, 1}, false, "invgsya", 0, {"Topaz", "Gem", "Socket Filler", "Miscellaneous", "Chipped Gem"}}},
            {"gfy", {"Flawed Topaz", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 5}, {1, 1}, false, "invgsyb", 0, {"Topaz", "Gem", "Socket Filler", "Miscellaneous", "Flawed Gem"}}},
            {"gsy", {"Topaz", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 12}, {1, 1}, false, "invgsyc", 0, {"Topaz", "Gem", "Socket Filler", "Miscellaneous", "Standard Gem"}}},
            {"gly", {"Flawless Topaz", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invgsyd", 0, {"Topaz", "Gem", "Socket Filler", "Miscellaneous", "Flawless Gem"}}},
            {"gpy", {"Perfect Topaz", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 1}, false, "invgsye", 0, {"Topaz", "Gem", "Socket Filler", "Miscellaneous", "Perfect Gem"}}},
            {"gcb", {"Chipped Sapphire", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 1}, {1, 1}, false, "invgsba", 0, {"Sapphire", "Gem", "Socket Filler", "Miscellaneous", "Chipped Gem"}}},
            {"gfb", {"Flawed Sapphire", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 5}, {1, 1}, false, "invgsbb", 0, {"Sapphire", "Gem", "Socket Filler", "Miscellaneous", "Flawed Gem"}}},
            {"gsb", {"Sapphire", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 12}, {1, 1}, false, "invgsbc", 0, {"Sapphire", "Gem", "Socket Filler", "Miscellaneous", "Standard Gem"}}},
            {"glb", {"Flawless Sapphire", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invgsbd", 0, {"Sapphire", "Gem", "Socket Filler", "Miscellaneous", "Flawless Gem"}}},
            {"gpb", {"Perfect Sapphire", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 1}, false, "invgsbe", 0, {"Sapphire", "Gem", "Socket Filler", "Miscellaneous", "Perfect Gem"}}},
            {"gcg", {"Chipped Emerald", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 1}, {1, 1}, false, "invgsga", 0, {"Emerald", "Gem", "Socket Filler", "Miscellaneous", "Chipped Gem"}}},
            {"gfg", {"Flawed Emerald", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 5}, {1, 1}, false, "invgsgb", 0, {"Emerald", "Gem", "Socket Filler", "Miscellaneous", "Flawed Gem"}}},
            {"gsg", {"Emerald", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 12}, {1, 1}, false, "invgsgc", 0, {"Emerald", "Gem", "Socket Filler", "Miscellaneous", "Standard Gem"}}},
            {"glg", {"Flawless Emerald", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invgsgd", 0, {"Emerald", "Gem", "Socket Filler", "Miscellaneous", "Flawless Gem"}}},
            {"gpg", {"Perfect Emerald", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 1}, false, "invgsge", 0, {"Emerald", "Gem", "Socket Filler", "Miscellaneous", "Perfect Gem"}}},
            {"gcr", {"Chipped Ruby", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 1}, {1, 1}, false, "invgsra", 0, {"Ruby", "Gem", "Socket Filler", "Miscellaneous", "Chipped Gem"}}},
            {"gfr", {"Flawed Ruby", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 5}, {1, 1}, false, "invgsrb", 0, {"Ruby", "Gem", "Socket Filler", "Miscellaneous", "Flawed Gem"}}},
            {"gsr", {"Ruby", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 12}, {1, 1}, false, "invgsrc", 0, {"Ruby", "Gem", "Socket Filler", "Miscellaneous", "Standard Gem"}}},
            {"glr", {"Flawless Ruby", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invgsrd", 0, {"Ruby", "Gem", "Socket Filler", "Miscellaneous", "Flawless Gem"}}},
            {"gpr", {"Perfect Ruby", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 1}, false, "invgsre", 0, {"Ruby", "Gem", "Socket Filler", "Miscellaneous", "Perfect Gem"}}},
            {"gcw", {"Chipped Diamond", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 1}, {1, 1}, false, "invgswa", 0, {"Diamond", "Gem", "Socket Filler", "Miscellaneous", "Chipped Gem"}}},
            {"gfw", {"Flawed Diamond", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 5}, {1, 1}, false, "invgswb", 0, {"Diamond", "Gem", "Socket Filler", "Miscellaneous", "Flawed Gem"}}},
            {"gsw", {"Diamond", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 12}, {1, 1}, false, "invgswc", 0, {"Diamond", "Gem", "Socket Filler", "Miscellaneous", "Standard Gem"}}},
            {"glw", {"Flawless Diamond", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invgswd", 0, {"Diamond", "Gem", "Socket Filler", "Miscellaneous", "Flawless Gem"}}},
            {"gpw", {"Perfect Diamond", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 1}, false, "invgswe", 0, {"Diamond", "Gem", "Socket Filler", "Miscellaneous", "Perfect Gem"}}},
            {"hp1", {"Minor Healing Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invhp1", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"hp2", {"Light Healing Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invhp2", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"hp3", {"Healing Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invhp3", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"hp4", {"Greater Healing Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invhp4", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"hp5", {"Super Healing Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invhp5", 0, {"Healing Potion", "Potion", "Miscellaneous"}}},
            {"mp1", {"Minor Mana Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invmp1", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
            {"mp2", {"Light Mana Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invmp2", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
            {"mp3", {"Mana Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invmp3", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
            {"mp4", {"Greater Mana Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invmp4", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
            {"mp5", {"Super Mana Potion", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invmp5", 0, {"Mana Potion", "Potion", "Miscellaneous"}}},
            {"skc", {"Chipped Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 1}, {1, 1}, false, "invskc", 0, {"Skull", "Gem", "Socket Filler", "Miscellaneous", "Chipped Gem"}}},
            {"skf", {"Flawed Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 5}, {1, 1}, false, "invskf", 0, {"Skull", "Gem", "Socket Filler", "Miscellaneous", "Flawed Gem"}}},
            {"sku", {"Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 12}, {1, 1}, false, "invsku", 0, {"Skull", "Gem", "Socket Filler", "Miscellaneous", "Standard Gem"}}},
            {"skl", {"Flawless Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invskl", 0, {"Skull", "Gem", "Socket Filler", "Miscellaneous", "Flawless Gem"}}},
            {"skz", {"Perfect Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 1}, false, "invskz", 0, {"Skull", "Gem", "Socket Filler", "Miscellaneous", "Perfect Gem"}}},
            {"r01", {"El Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 11}, {1, 1}, false, "invrEl", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r02", {"Eld Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 11}, {1, 1}, false, "invrEld", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r03", {"Tir Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 13}, {1, 1}, false, "invrTir", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r04", {"Nef Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 13}, {1, 1}, false, "invrNef", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r05", {"Eth Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invrEth", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r06", {"Ith Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 15}, {1, 1}, false, "invrIth", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r07", {"Tal Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 17}, {1, 1}, false, "invrTal", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r08", {"Ral Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 19}, {1, 1}, false, "invrRal", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r09", {"Ort Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 21}, {1, 1}, false, "invrOrt", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r10", {"Thul Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 23}, {1, 1}, false, "invrThul", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r11", {"Amn Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 25}, {1, 1}, false, "invrAmn", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r12", {"Sol Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 27}, {1, 1}, false, "invrSol", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r13", {"Shael Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 29}, {1, 1}, false, "invrShae", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r14", {"Dol Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 31}, {1, 1}, false, "invrDol", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r15", {"Hel Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invrHel", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r16", {"Io Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 35}, {1, 1}, false, "invrIo", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r17", {"Lum Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 37}, {1, 1}, false, "invrLum", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r18", {"Ko Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 39}, {1, 1}, false, "invrKo", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r19", {"Fal Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 41}, {1, 1}, false, "invrFal", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r20", {"Lem Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 43}, {1, 1}, false, "invrLem", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r21", {"Pul Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 45}, {1, 1}, false, "invrPul", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r22", {"Um Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 47}, {1, 1}, false, "invrUm", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r23", {"Mal Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 49}, {1, 1}, false, "invrMal", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r24", {"Ist Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 51}, {1, 1}, false, "invrIst", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r25", {"Gul Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 53}, {1, 1}, false, "invrGul", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r26", {"Vex Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 55}, {1, 1}, false, "invrVex", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r27", {"Ohm Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 57}, {1, 1}, false, "invrOhm", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r28", {"Lo Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 59}, {1, 1}, false, "invrLo", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r29", {"Sur Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 61}, {1, 1}, false, "invrSur", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r30", {"Ber Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 63}, {1, 1}, false, "invrBer", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r31", {"Jah Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 65}, {1, 1}, false, "invrJo", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r32", {"Cham Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 67}, {1, 1}, false, "invrCham", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"r33", {"Zod Rune", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 69}, {1, 1}, false, "invrZod", 0, {"Rune", "Socket Filler", "Miscellaneous"}}},
            {"jew", {"Jewel", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invjew", 0, {"Jewel", "Socket Filler", "Miscellaneous"}}},
        };

        std::string testStr("   ");
        testStr[0] = (char)strcode[0];
        testStr[1] = (char)strcode[1];
        testStr[2] = (char)strcode[2];
        auto iter = essenceBases.find(testStr);
        if (iter != essenceBases.end())
        {
            return iter->second;
        }

        return s_invalidItemType;
    }

    const ItemType& getItemTypeHelper(std::uint8_t(&strcode)[4])
    {
        // could be stackable
        {
            const auto& result = getStackableItemType(strcode);
            if (&result != &s_invalidItemType)
            {
                // It's a stackable item
                return result;
            }
        }

        // Could be armor (all tomes are stackable so covered by call to getStackableItemType)
        {
            const auto& result = getArmorItemType(strcode);
            if (&result != &s_invalidItemType)
            {
                return result;
            }
        }

        // Could be a weapon
        {
            const auto& result = getWeaponItemType(strcode);
            if (&result != &s_invalidItemType)
            {
                return result;
            }
        }

        // Could be a GPS
        {
            const auto& result = getGPSItemType(strcode);
            if (&result != &s_invalidItemType)
            {
                return result;
            }
        }

        // Could be a Misc.
        return getMiscItemType(strcode);
    }

    std::uint8_t getItemBase(std::uint8_t(&strcode)[4])
    {
        const auto& item = getItemTypeHelper(strcode);
        return item.getBaseType();
    }

    EnumItemType getEnumItemTypeFromCode(std::uint8_t(&strcode)[4])
    {
        const auto& item = getItemTypeHelper(strcode);
        return item.getEnumItemType();
    }

    std::string getRunewordNameFromId(std::uint16_t id)
    {
        switch (id)
        {
        case 27: return "Ancient's Pledge";
        case 30: return "Beast";
        case 32: return "Black";
        case 34: return "Bone";
        case 35: return "Bramble";
        case 36: return "Brand";
        case 37: return "Breath of the Dying";
        case 39: return "Call to Arms";
        case 40: return "Chains of Honor";
        case 42: return "Chaos";
        case 43: return "Crescent Moon";
        case 46: return "Death";
        case 51: return "Destruction";
        case 52: return "Doom";
        case 53: return "Dragon";
        case 55: return "Dream";
        case 56: return "Duress";
        case 57: return "Edge";
        case 59: return "Enigma";
        case 60: return "Enlightenment";
        case 62: return "Eternity";
        case 63: return "Exile";
        case 64: return "Faith";
        case 65: return "Famine";
        case 67: return "Fortitude";
        case 70: return "Fury";
        case 71: return "Gloom";
        case 73: return "Grief";
        case 74: return "Hand of Justice";
        case 75: return "Harmory";
        case 77: return "Heart of the Oak";
        case 80: return "Holy Thunder";
        case 81: return "Honor";
        case 85: return "Ice";
        case 86: return "Infinity";
        case 88: return "Insight";
        case 91: return "King's Grace";
        case 92: return "Kingslayer";
        case 95: return "Last Wish";
        case 97: return "Lawbringer";
        case 98: return "Leaf";
        case 100: return "Lionheart";
        case 101: return "Lore";
        case 106: return "Malice";
        case 107: return "Melody";
        case 108: return "Memory";
        case 112: return "Myth";
        case 113: return "Nadir";
        case 116: return "Oath";
        case 117: return "Obedience";
        case 120: return "Passion";
        case 123: return "Peace";
        case 124: return "Winter";
        case 128: return "Phoenix";
        case 131: return "Plague";
        case 134: return "Pride";
        case 135: return "Principle";
        case 137: return "Prudence";
        case 141: return "Radiance";
        case 142: return "Rain";
        case 145: return "Rhyme";
        case 146: return "Rift";
        case 147: return "Sanctuary";
        case 151: return "Silence";
        case 153: return "Smoke";
        case 155: return "Spirit";
        case 156: return "Splendor";
        case 158: return "Stealth";
        case 162: return "Stone";
        case 164: return "Strength";
        case 173: return "Treachery";
        case 179: return "Venom";
        case 185: return "Wealth";
        case 187: return "White";
        case 188: return "Wind";
        case 193: return "Wrath";
        case 195: return "Zephyr";
        case 2718: return "Delirium";
        }

        return "";
    }

    std::string getSetNameFromId(std::uint16_t id)
    {
        switch (id)
        {
        case 0: return "Civerb's Ward";
        case 1: return "Civerb's Icon";
        case 2: return "Civerb's Cudgel";
        case 3: return "Hsaru's Iron Heel";
        case 4: return "Hsaru's Iron Fist";
        case 5: return "Hsaru's Iron Stay";
        case 6: return "Cleglaw's Tooth";
        case 7: return "Cleglaw's Claw";
        case 8: return "Cleglaw's Pincers";
        case 9: return "Iratha's Collar";
        case 10: return "Iratha's Cuff";
        case 11: return "Iratha's Coil";
        case 12: return "Iratha's Cord";
        case 13: return "Isenhart's Lightbrand";
        case 14: return "Isenhart's Parry";
        case 15: return "Isenhart's Case";
        case 16: return "Isenhart's Horns";
        case 17: return "Vidala's Barb";
        case 18: return "Vidala's Fetlock";
        case 19: return "Vidala's Ambush";
        case 20: return "Vidala's Snare";
        case 21: return "Milabrega's Orb";
        case 22: return "Milabrega's Rod";
        case 23: return "Milabrega's Diadem";
        case 24: return "Mialbrega's Robe";
        case 25: return "Cathan's Rule";
        case 26: return "Cathan's Mesh";
        case 27: return "Cathan's Visage";
        case 28: return "Cathan's Sigil";
        case 29: return "Cathan's Seal";
        case 30: return "Tancred's Crowbill";
        case 31: return "Tancred's Spine";
        case 32: return "Tancred's Hobnails";
        case 33: return "Tancred's Weird";
        case 34: return "Tancred's Skull";
        case 35: return "Sigon's Gage";
        case 36: return "Sigon's Visor";
        case 37: return "Sigon's Shelter";
        case 38: return "Sigon's Sabot";
        case 39: return "Sigon's Wrap";
        case 40: return "Sigon's Guard";
        case 41: return "Infernal Cranium";
        case 42: return "Infernal Torch";
        case 43: return "Infernal Sign";
        case 44: return "Berserker's Headgear";
        case 45: return "Berserker's Hauberk";
        case 46: return "Berserker's Hatchet";
        case 47: return "Death's Hand";
        case 48: return "Death's Guard";
        case 49: return "Death's Touch";
        case 50: return "Angelic Sickle";
        case 51: return "Angelic Mantle";
        case 52: return "Angelic Halo";
        case 53: return "Angelic Wings";
        case 54: return "Arctic Horn";
        case 55: return "Arctic Furs";
        case 56: return "Arctic Binding";
        case 57: return "Arctic Mitts";
        case 58: return "Arcanna's Sign";
        case 59: return "Arcanna's Deathwand";
        case 60: return "Arcanna's Head";
        case 61: return "Arcanna's Flesh";
        case 62: return "Natalya's Totem";
        case 63: return "Natalya's Mark";
        case 64: return "Natalya's Shadow";
        case 65: return "Natalya's Soul";
        case 66: return "Aldur's Stony Gaze";
        case 67: return "Aldur's Deception";
        case 68: return "Aldur's Rhythm";
        case 69: return "Aldur's Advance";
        case 70: return "Immortal King's Will";
        case 71: return "Immortal King's Soul Cage";
        case 72: return "Immortal King's Detail";
        case 73: return "Immortal King's Forge";
        case 74: return "Immortal King's Pillar";
        case 75: return "Immortal King's Stone Crusher";
        case 76: return "Tal Rasha's Fine-Spun Cloth";
        case 77: return "Tal Rasha's Adjudication";
        case 78: return "Tal Rasha's Lidless Eye";
        case 79: return "Tal Rasha's Guardianship";
        case 80: return "Tal Rasha's Horadric Crest";
        case 81: return "Griswold's Valor";
        case 82: return "Griswold's Heart";
        case 83: return "Griswold's Redemption";
        case 84: return "Griswold's Honor";
        case 85: return "Trang-Oul's Guise";
        case 86: return "Trang-Oul's Scales";
        case 87: return "Trang-Oul's Wing";
        case 88: return "Trang-Oul's Claws";
        case 89: return "Trang-Oul's Girth";
        case 90: return "M'avina's True Sight";
        case 91: return "M'avina's Embrace";
        case 92: return "M'avina's Icy Clutch";
        case 93: return "M'avina's Tenet";
        case 94: return "M'avina's Caster";
        case 95: return "Telling of Beads";
        case 96: return "Laying of Hands";
        case 97: return "Rite of Passage";
        case 98: return "Dark Adherent";
        case 99: return "Credendum";
        case 100: return "Dangoon's Teaching";
        case 101: return "Taebaek's Glory";
        case 102: return "Haemosu's Adament";
        case 103: return "Ondal's Almighty";
        case 104: return "Guillaume's Face";
        case 105: return "Wilhelm's Pride";
        case 106: return "Magnus' Skin";
        case 107: return "Wihtstan's Guard";
        case 108: return "Hwanin's Splendor";
        case 109: return "Hwanin's Refuge";
        case 110: return "Hwanin's Blessing";
        case 111: return "Hwanin's Justice";
        case 112: return "Sazabi's Cobalt Redeemer";
        case 113: return "Sazabi's Ghost Liberator";
        case 114: return "Sazabi's Mental Sheath";
        case 115: return "Bul-Katho's Sacred Charge";
        case 116: return "Bul-Katho's Tribal Guardian";
        case 117: return "Cow King's Horns";
        case 118: return "Cow King's Hide";
        case 119: return "Cow King's Hooves";
        case 120: return "Naj's Puzzler";
        case 121: return "Naj's Light Plate";
        case 122: return "Naj's Circlet";
        case 123: return "Sander's Paragon";
        case 124: return "Sander's Riprap";
        case 125: return "Sander's Taboo";
        case 126: return "Sander's Superstition";
        }

        return "";
    }

    std::string getRareNameFromId(std::uint16_t id)
    {
        switch (id)
        {
        case 1: return "Bite";
        case 2: return "Scratch";
        case 3: return "Scalpel";
        case 4: return "Fang";
        case 5: return "Gutter";
        case 6: return "Thirst";
        case 7: return "Razor";
        case 8: return "Scythe";
        case 9: return "Edge";
        case 10: return "Saw";
        case 11: return "Splitter";
        case 12: return "Cleaver";
        case 13: return "Sever";
        case 14: return "Sunder";
        case 15: return "Rend";
        case 16: return "Mangler";
        case 17: return "Slayer";
        case 18: return "Reaver";
        case 19: return "Spawn";
        case 20: return "Gnash";
        case 21: return "Star";
        case 22: return "Blow";
        case 23: return "Smasher";
        case 24: return "Bane";
        case 25: return "Crusher";
        case 26: return "Breaker";
        case 27: return "Grinder";
        case 28: return "Crack";
        case 29: return "Mallet";
        case 30: return "Knell";
        case 31: return "Lance";
        case 32: return "Spike";
        case 33: return "Impaler";
        case 34: return "Skewer";
        case 35: return "Prod";
        case 36: return "Scourge";
        case 37: return "Wand";
        case 38: return "Wrack";
        case 39: return "Barb";
        case 40: return "Needle";
        case 41: return "Dart";
        case 42: return "Bolt";
        case 43: return "Quarrel";
        case 44: return "Fletch";
        case 45: return "Flight";
        case 46: return "Nock";
        case 47: return "Horn";
        case 48: return "Stinger";
        case 49: return "Quill";
        case 50: return "Goad";
        case 51: return "Branch";
        case 52: return "Spire";
        case 53: return "Song";
        case 54: return "Call";
        case 55: return "Cry";
        case 56: return "Spell";
        case 57: return "Chant";
        case 58: return "Weaver";
        case 59: return "Gnarl";
        case 60: return "Visage";
        case 61: return "Crest";
        case 62: return "Circlet";
        case 63: return "Veil";
        case 64: return "Hood";
        case 65: return "Mask";
        case 66: return "Brow";
        case 67: return "Casque";
        case 68: return "Visor";
        case 69: return "Cowl";
        case 70: return "Hide";
        case 71: return "Pelt";
        case 72: return "Carapace";
        case 73: return "Coat";
        case 74: return "Wrap";
        case 75: return "Suit";
        case 76: return "Cloak";
        case 77: return "Shroud";
        case 78: return "Jack";
        case 79: return "Mantle";
        case 80: return "Guard";
        case 81: return "Badge";
        case 82: return "Rock";
        case 83: return "Aegis";
        case 84: return "Ward";
        case 85: return "Tower";
        case 86: return "Shield";
        case 87: return "Wing";
        case 88: return "Mark";
        case 89: return "Emblem";
        case 90: return "Hand";
        case 91: return "Fist";
        case 92: return "Claw";
        case 93: return "Clutches";
        case 94: return "Grip";
        case 95: return "Grasp";
        case 96: return "Hold";
        case 97: return "Torch";
        case 98: return "Finger";
        case 99: return "Knuckle";
        case 100: return "Shank";
        case 101: return "Spur";
        case 102: return "Tread";
        case 103: return "Stalker";
        case 104: return "Greave";
        case 105: return "Blazer";
        case 106: return "Nails";
        case 107: return "Trample";
        case 108: return "Brogues";
        case 109: return "Track";
        case 110: return "Slippers";
        case 111: return "Clasp";
        case 112: return "Buckle";
        case 113: return "Harness";
        case 114: return "Lock";
        case 115: return "Fringe";
        case 116: return "Winding";
        case 117: return "Chain";
        case 118: return "Strap";
        case 119: return "Lash";
        case 120: return "Cord";
        case 121: return "Knot";
        case 122: return "Circle";
        case 123: return "Loop";
        case 124: return "Eye";
        case 125: return "Turn";
        case 126: return "Spiral";
        case 127: return "Coil";
        case 128: return "Gyre";
        case 129: return "Band";
        case 130: return "Whorl";
        case 131: return "Talisman";
        case 132: return "Heart";
        case 133: return "Noose";
        case 134: return "Necklace";
        case 135: return "Collar";
        case 136: return "Beads";
        case 137: return "Torc";
        case 138: return "Gorget";
        case 139: return "Scarab";
        case 140: return "Wood";
        case 141: return "Brand";
        case 142: return "Bludgeon";
        case 143: return "Cudgel";
        case 144: return "Loom";
        case 145: return "Harp";
        case 146: return "Master";
        case 147: return "Barl";
        case 148: return "Hew";
        case 149: return "Crook";
        case 150: return "Mar";
        case 151: return "Shell";
        case 152: return "Stake";
        case 153: return "Picket";
        case 154: return "Pale";
        case 155: return "Flange";
        case 156: return "Beast";
        case 157: return "Eagle";
        case 158: return "Raven";
        case 159: return "Viper";
        case 160: return "Ghoul";
        case 161: return "Skull";
        case 162: return "Blood";
        case 163: return "Dread";
        case 164: return "Doom";
        case 165: return "Grim";
        case 166: return "Bone";
        case 167: return "Death";
        case 168: return "Shadow";
        case 169: return "Storm";
        case 170: return "Rune";
        case 171: return "Plague";
        case 172: return "Stone";
        case 173: return "Wraith";
        case 174: return "Spirit";
        case 175: return "Storm";
        case 176: return "Demon";
        case 177: return "Cruel";
        case 178: return "Empyrion";
        case 179: return "Bramble";
        case 180: return "Pain";
        case 181: return "Loath";
        case 182: return "Glyph";
        case 183: return "Imp";
        case 184: return "Fiendra";
        case 185: return "Hailstone";
        case 186: return "Gale";
        case 187: return "Dire";
        case 188: return "Soul";
        case 189: return "Brimstone";
        case 190: return "Corpse";
        case 191: return "Carrion";
        case 192: return "Armageddon";
        case 193: return "Havoc";
        case 194: return "Bitter";
        case 195: return "Entropy";
        case 196: return "Chaos";
        case 197: return "Order";
        case 198: return "Rule";
        case 199: return "Warp";
        case 200: return "Rift";
        case 201: return "Corruption";
        }

        return "";
    }

    std::string getMagicalPrefixFromId(std::uint16_t id)
    {
        switch (id)
        {
        case 2:
        case 143:
        case 144:
            return "Sturdy";

        case 3:
        case 145:
            return "Strong";

        case 4:
        case 146:
            return "Glorious";

        case 5:
        case 147:
            return "Blessed";

        case 6:
        case 148:
            return "Saintly";

        case 7:
        case 149:
            return "Holy";

        case 8:
        case 151:
            return "Devious";

        case 9:
            return "Fortified";

        case 13:
        case 168:
        case 169:
        case 170:
        case 175:
        case 176:
        case 180:
        case 186:
        case 608:
        case 611:
        case 613:
            return "Jagged";

        case 14:
        case 187:
            return "Deadly";

        case 15:
        case 188:
            return "Vicious";

        case 16:
        case 189:
            return "Brutal";

        case 17:
        case 190:
            return "Massive";

        case 18:
        case 191:
            return "Savage";

        case 19:
        case 192:
            return "Merciless";

        case 20:
        case 199:
            return "Vulpine";

        case 25:
        case 201:
        case 202:
        case 662:
            return "Tireless";

        case 26:
        case 204:
        case 205:
        case 206:
        case 207:
        case 208:
        case 209:
        case 210:
        case 211:
        case 212:
        case 213:
        case 214:
            return "Rugged";

        case 27:
        case 218:
        case 219:
        case 220:
        case 227:
        case 228:
        case 229:
        case 234:
        case 235:
        case 238:
            return "Bronze";

        case 28:
        case 221:
        case 222:
        case 223:
        case 230:
        case 231:
        case 236:
        case 239:
            return "Iron";

        case 29:
        case 224:
        case 225:
        case 226:
        case 232:
        case 233:
        case 237:
        case 240:
            return "Steel";

        case 30:
        case 241:
        case 249:
            return "Silver";

        case 32:
        case 242:
            return "Gold";

        case 33:
        case 243:
            return "Platinum";

        case 34:
        case 244:
            return "Meteoric";

        case 35:
        case 253:
        case 255:
        case 257:
            return "Sharp";

        case 36:
        case 251:
        case 252:
        case 254:
        case 256:
        case 258:
            return "Fine";

        case 37:
        case 259:
            return "Warrior's";

        case 38:
        case 260:
            return "Soldier's";

        case 39:
        case 261:
            return "Knight's";

        case 40:
        case 262:
            return "Lord's";

        case 41:
        case 263:
            return "King's";

        case 42:
        case 270:
        case 273:
            return "Howling";

        case 43:
        case 282:
            return "Fortuitous";

        case 49:
        case 266:
            return "Glimmering";

        case 50:
        case 267:
        case 551:
        case 639:
        case 643:
        case 647:
            return "Glowing";

        case 53:
        case 284:
        case 285:
        case 286:
        case 293:
        case 294:
        case 295:
        case 300:
        case 301:
        case 304:
        case 663:
            return "Lizard's";

        case 55:
        case 287:
        case 288:
        case 289:
        case 296:
        case 297:
        case 302:
        case 305:
            return "Snake's";

        case 56:
        case 57:
        case 290:
        case 291:
        case 292:
        case 298:
        case 299:
        case 303:
        case 306:
        case 307:
            return "Serpent's";

        case 58:
        case 308:
            return "Drake's";

        case 59:
        case 60:
        case 309:
        case 310:
            return "Dragon's";

        case 61:
        case 311:
            return "Wyrm's";

        case 64:
        case 65:
        case 326:
        case 331:
            return "Prismatic";

        case 66:
        case 338:
        case 342:
        case 346:
        case 350:
        case 664:
            return "Azure";

        case 67:
        case 68:
        case 339:
        case 343:
        case 347:
        case 351:
        case 352:
            return "Lapis";

        case 69:
        case 70:
        case 340:
        case 344:
        case 348:
        case 353:
        case 354:
            return "Cobalt";

        case 72:
        case 341:
        case 345:
        case 349:
        case 355:
        case 357:
            return "Sapphire";

        case 75:
        case 167:
        case 358:
        case 362:
        case 366:
        case 601:
        case 665:
            return "Crimson";

        case 76:
        case 77:
            return "Burgundy";

        case 78:
        case 79:
        case 360:
        case 364:
        case 368:
        case 372:
        case 373:
        case 375:
            return "Garnet";

        case 81:
        case 198:
        case 361:
        case 365:
        case 369:
        case 374:
        case 376:
            return "Ruby";

        case 84:
        case 378:
        case 382:
        case 386:
        case 390:
        case 391:
            return "Ocher";

        case 85:
        case 86:
        case 377:
        case 381:
        case 385:
        case 389:
        case 666:
            return "Tangerine";

        case 87:
        case 88:
        case 379:
        case 383:
        case 387:
        case 392:
        case 393:
            return "Coral";

        case 90:
        case 380:
        case 384:
        case 388:
        case 394:
            return "Amber";

        case 93:
        case 397:
        case 401:
        case 405:
        case 409:
        case 415:
        case 667:
            return "Beryl";

        case 94:
        case 95:
        case 399:
        case 403:
        case 407:
        case 412:
        case 413:
        case 416:
            return "Jade";

        case 96:
        case 97:
        case 398:
        case 402:
        case 406:
        case 410:
        case 411:
            return "Viridian";

        case 99:
        case 283:
        case 400:
        case 404:
        case 408:
        case 414:
            return "Emerald";

        case 101:
        case 430:
        case 433:
            return "Fletcher's";

        case 102:
        case 103:
        case 435:
            return "Archer's";

        case 104:
        case 564:
        case 566:
        case 568:
            return "Monk's";

        case 105:
        case 106:
        case 565:
        case 567:
        case 569:
            return "Priest's";

        case 107:
        case 570:
        case 572:
            return "Summoner's";

        case 108:
        case 109:
        case 571:
        case 573:
            return "Necromancer's";

        case 110:
        case 574:
        case 576:
            return "Angel's";

        case 111:
        case 112:
        case 575:
        case 577:
            return "Arch-Angel's";

        case 113:
        case 578:
        case 580:
        case 582:
            return "Slayer's";

        case 114:
        case 115:
        case 579:
        case 581:
        case 583:
            return "Berserker's";

        case 118:
        case 417:
            return "Triumphant";

        case 119:
        case 120:
        case 121:
        case 128:
        case 129:
        case 130:
        case 135:
        case 136:
            return "Stout";

        case 122:
        case 123:
        case 124:
        case 131:
        case 132:
        case 137:
            return "Burly";

        case 125:
        case 126:
        case 127:
        case 133:
        case 134:
        case 138:
            return "Stalwart";

        case 139:
            return "Blanched";

        case 140:
            return "Eburin";

        case 141:
            return "Bone";

        case 142:
            return "Ivory";

        case 150:
        case 668:
            return "Godly";

        case 152:
            return "Blank";

        case 153:
            return "Null";

        case 154:
            return "Antimagic";

        case 155:
        case 156:
        case 160:
        case 163:
        case 602:
        case 605:
        case 607:
            return "Red";

        case 157:
        case 158:
        case 161:
        case 164:
        case 603:
        case 606:
            return "Sanguinary";

        case 159:
        case 162:
        case 165:
        case 604:
            return "Bloody";

        case 166:
            return "Scarlet";

        case 171:
        case 172:
        case 177:
        case 178:
        case 181:
        case 609:
        case 612:
            return "Forked";

        case 173:
        case 174:
        case 179:
        case 182:
        case 610:
            return "Serrated";

        case 183:
            return "Carbuncle";

        case 184:
            return "Carmine";

        case 185:
            return "Vermillion";

        case 193:
            return "Ferocious";

        case 194:
        case 669:
            return "Cruel";

        case 195:
            return "Cinnabar";

        case 196:
            return "Rusty";

        case 197:
            return "Realgar";

        case 200:
            return "Dun";

        case 203:
            return "Brown";

        case 215:
            return "Vigorous";

        case 216:
            return "Chestnut";

        case 217:
            return "Maroon";

        case 245:
            return "Strange";

        case 246:
            return "Weird";

        case 247:
            return "Nickel";

        case 248:
            return "Tin";

        case 250:
            return "Argent";

        case 264:
        case 483:
            return "Master's";

        case 265:
            return "Grandmaster's";

        case 268:
            return "Bright";

        case 269:
        case 272:
        case 531:
            return "Screaming";

        case 271:
        case 274:
            return "Wailing";

        case 275:
        case 276:
        case 277:
        case 278:
        case 279:
        case 280:
            return "Lucky";

        case 281:
            return "Felicitous";

        case 312:
            return "Great Wyrm's";

        case 313:
            return "Bahamut's";

        case 314:
            return "Zircon";

        case 315:
            return "Jacinth";

        case 316:
            return "Turquoise";

        case 317:
        case 318:
        case 319:
        case 320:
        case 321:
        case 322:
        case 323:
        case 328:
        case 333:
        case 336:
            return "Shimmering";

        case 324:
        case 329:
        case 334:
            return "Rainbow";

        case 325:
        case 330:
        case 335:
        case 337:
            return "Scintillating";

        case 327:
        case 332:
            return "Chromatic";

        case 356:
            return "Lapis Lazuli";

        case 359:
        case 363:
        case 367:
        case 370:
        case 371:
            return "Russet";

        case 395:
            return "Camphor";

        case 396:
            return "Ambergris";

        case 418:
            return "Victorious";

        case 419:
            return "Aureolin";

        case 420:
            return "Mechanic's";

        case 421:
            return "Artisan's";

        case 422:
            return "Jeweler's";

        case 423:
            return "Assamic";

        case 424:
            return "Arcadian";

        case 425:
            return "Unearthly";

        case 426:
            return "Astral";

        case 427:
            return "Elysian";

        case 428:
            return "Celestial";

        case 429:
            return "Diamond";

        case 431:
        case 436:
            return "Acrobat's";

        case 432:
        case 439:
            return "Harpoonist's";

        case 434:
            return "Bowyer's";

        case 437:
            return "Gymnast's";

        case 438:
            return "Athlete's";

        case 440:
            return "Spearmaiden's";

        case 441:
            return "Lancer's";

        case 442:
        case 445:
            return "Burning";

        case 443:
        case 448:
            return "Sparking";

        case 444:
        case 451:
            return "Chilling";

        case 446:
            return "Blazing";

        case 447:
            return "Volcanic";

        case 449:
            return "Charged";

        case 450:
            return "Powered";

        case 452:
            return "Freezing";

        case 453:
            return "Glacial";

        case 454:
        case 457:
            return "Hexing";

        case 455:
        case 460:
            return "Fungal";

        case 456:
        case 463:
            return "Graverobber's";

        case 458:
            return "Blighting";

        case 459:
            return "Cursing";

        case 461:
            return "Noxious";

        case 462:
            return "Venomous";

        case 464:
            return "Mojo";

        case 465:
            return "Golemlord's";

        case 466:
        case 469:
            return "Lion Branded";

        case 467:
        case 472:
            return "Captain's";

        case 468:
        case 475:
            return "Preserver's";

        case 470:
            return "Hawk Branded";

        case 471:
            return "Rose Branded";

        case 473:
            return "Commander's";

        case 474:
            return "Marshal's";

        case 476:
            return "Warden's";

        case 477:
            return "Guardian's";

        case 478:
        case 481:
            return "Expert's";

        case 479:
        case 484:
            return "Fanatic";

        case 480:
        case 487:
            return "Sounding";

        case 482:
            return "Veteran's";

        case 485:
            return "Raging";

        case 486:
            return "Furious";

        case 488:
            return "Resonant";

        case 489:
            return "Echoing";

        case 490:
        case 493:
            return "Trainer's";

        case 491:
        case 496:
            return "Spiritual";

        case 492:
        case 499:
            return "Natural";

        case 494:
            return "Caretaker's";

        case 495:
            return "Keeper's";

        case 497:
            return "Feral";

        case 498:
            return "Communal";

        case 500:
            return "Terrene";

        case 501:
            return "Gaean";

        case 502:
        case 505:
            return "Entrapping";

        case 503:
        case 508:
            return "Mentalist's";

        case 504:
        case 511:
            return "Shogukusha's";

        case 506:
            return "Trickster's";

        case 507:
            return "Cunning";

        case 509:
            return "Psychic";

        case 510:
            return "Shadow";

        case 512:
            return "Sensei's";

        case 513:
            return "Kenshi's";

        case 514:
        case 515:
            return "Miocene";

        case 516:
        case 517:
            return "Oligocene";

        case 518:
        case 519:
            return "Eocene";

        case 520:
        case 521:
            return "Paleocene";

        case 522:
            return "Knave's";

        case 523:
            return "Jack's";

        case 524:
            return "Jester's";

        case 525:
            return "Joker's";

        case 526:
            return "Trump";

        case 527:
            return "Loud";

        case 528:
            return "Calling";

        case 529:
            return "Yelling";

        case 530:
            return "Shouting";

        case 532:
        case 533:
            return "Paradox";

        case 534:
            return "Robineye";

        case 535:
            return "Sparroweye";

        case 536:
            return "Falconeye";

        case 537:
            return "Hawkeye";

        case 538:
            return "Eagleeye";

        case 539:
            return "Visionary";

        case 540:
            return "Mnemonic";

        case 541:
        case 614:
        case 618:
        case 622:
            return "Snowflake";

        case 542:
        case 615:
        case 619:
        case 623:
            return "Shivering";

        case 543:
        case 616:
        case 620:
        case 624:
            return "Boreal";

        case 544:
        case 617:
        case 621:
        case 625:
            return "Hibernal";

        case 545:
        case 626:
        case 630:
        case 634:
            return "Ember";

        case 546:
        case 627:
        case 631:
        case 635:
            return "Smoldering";

        case 547:
        case 628:
        case 632:
        case 636:
            return "Smoking";

        case 548:
        case 629:
        case 633:
        case 637:
            return "Flaming";

        case 549:
            return "Scorching";

        case 550:
        case 638:
        case 642:
        case 646:
            return "Static";

        case 552:
            return "Buzzing";

        case 553:
        case 640:
        case 644:
        case 648:
            return "Arcing";

        case 554:
        case 641:
        case 645:
        case 649:
            return "Shocking";

        case 555:
        case 650:
        case 654:
        case 658:
            return "Septic";

        case 556:
        case 651:
        case 655:
        case 659:
            return "Envenomed";

        case 557:
            return "Corosive";

        case 558:
        case 652:
        case 656:
        case 660:
            return "Toxic";

        case 559:
        case 653:
        case 657:
        case 661:
            return "Pestilent";

        case 560:
        case 562:
            return "Maiden's";

        case 561:
        case 563:
            return "Valkyrie's";

        case 584:
        case 586:
            return "Shaman's";

        case 585:
        case 587:
            return "Hierophant's";

        case 588:
        case 590:
            return "Magekiller's";

        case 589:
        case 591:
            return "Witch-hunter's";

        case 592:
            return "Compact";

        case 593:
            return "Thin";

        case 594:
            return "Dense";

        case 595:
            return "Consecrated";

        case 596:
            return "Pure";

        case 597:
            return "Sacred";

        case 598:
            return "Hallowed";

        case 599:
            return "Divine";

        case 600:
            return "Pearl";
        }

        return "";
    }

    std::string getMagicalSuffixFromId(std::uint16_t id)
    {
        switch (id)
        {
        case 1: return "of Health";
        case 2: return "of Protection";
        case 3: return "of Absorption";
        case 4: return "of Life";
        case 6: return "of Warding";
        case 7: return "of the Sentinel";
        case 8: return "of Guarding";
        case 9: return "of Negation";
        case 11: return "of Piercing";
        case 12: return "of Bashing";
        case 13: return "of Puncturing";
        case 14: return "of Thorns";
        case 15: return "of Spikes";
        case 16: return "of Readiness";
        case 17: return "of Alacrity";
        case 18: return "of Swiftness";
        case 19: return "of Quickness";
        case 20: return "of Blocking";
        case 21: return "of Deflecting";
        case 22: return "of the Apprentice";
        case 23: return "of the Magus";
        case 24: return "of Frost";
        case 25: return "of the Glacier";
        case 26: return "of Frost";
        case 27: return "of Warmth";
        case 28: return "of Flame";
        case 29: return "of Fire";
        case 30: return "of Burning";
        case 31: return "of Flame";
        case 32: return "of Shock";
        case 33: return "of Lightning";
        case 34: return "of Thunder";
        case 35: return "of Shock";
        case 36: return "of Craftsmanship";
        case 37: return "of Quality";
        case 38: return "of Maiming";
        case 39: return "of Slaying";
        case 40: return "of Gore";
        case 41: return "of Carnage";
        case 42: return "of Slaughter";
        case 43: return "of Maiming";
        case 44: return "of Worth";
        case 45: return "of Measure";
        case 46: return "of Excellence";
        case 47: return "of Performance";
        case 48: return "of Measure";
        case 49: return "of Blight";
        case 50: return "of Venom";
        case 51: return "of Pestilence";
        case 52: return "of Blight";
        case 53: return "of Dexterity";
        case 54: return "of Dexterity";
        case 55: return "of Skill";
        case 56: return "of Skill";
        case 57: return "of Accuracy";
        case 58: return "of Precision";
        case 59: return "of Precision";
        case 60: return "of Perfection";
        case 61: return "of Balance";
        case 62: return "of Stability";
        case 64: return "of Regeneration";
        case 65: return "of Regeneration";
        case 66: return "of Regeneration";
        case 67: return "of Regrowth";
        case 68: return "of Regrowth";
        case 69: return "of Vileness";
        case 71: return "of Greed";
        case 72: return "of Wealth";
        case 73: return "of Chance";
        case 74: return "of Fortune";
        case 75: return "of Energy";
        case 76: return "of Energy";
        case 77: return "of the Mind";
        case 78: return "of Brilliance";
        case 79: return "of Sorcery";
        case 80: return "of Wizardry";
        case 81: return "of the Bear";
        case 82: return "of Light";
        case 83: return "of Radiance";
        case 84: return "of the Sun";
        case 85: return "of Life";
        case 86: return "of the Jackal";
        case 87: return "of the Fox";
        case 88: return "of the Wolf";
        case 89: return "of the Wolf";
        case 90: return "of the Tiger";
        case 91: return "of the Mammoth";
        case 92: return "of the Mammoth";
        case 93: return "of the Colosuss";
        case 94: return "of the Leech";
        case 95: return "of the Locust";
        case 96: return "of the Bat";
        case 97: return "of the Vampire";
        case 98: return "of Defiance";
        case 99: return "of Amelioration";
        case 100: return "of Remedy";
        case 102: return "of Simplicity";
        case 103: return "of Ease";
        case 105: return "of Strength";
        case 106: return "of Might";
        case 107: return "of the Ox";
        case 108: return "of the Ox";
        case 109: return "of the Giant";
        case 110: return "of the Giant";
        case 111: return "of the Titan";
        case 112: return "of Pacing";
        case 113: return "of Haste";
        case 114: return "of Speed";
        case 115: return "of Health";
        case 116: return "of Protection";
        case 117: return "of Absorption";
        case 118: return "of Life";
        case 119: return "of Life Everlasting";
        case 120: return "of Protection";
        case 121: return "of Absorption";
        case 122: return "of Life";
        case 123: return "of Anima";
        case 124: return "of Warding";
        case 125: return "of the Sentinel";
        case 126: return "of Guarding";
        case 127: return "of Negation";
        case 128: return "of the Sentinel";
        case 129: return "of Guarding";
        case 130: return "of Negation";
        case 131: return "of Coolness";
        case 132: return "of Incombustibility";
        case 133: return "of Amianthus";
        case 134: return "of Fire Quenching";
        case 135: return "of Coolness";
        case 136: return "of Incombustibility";
        case 137: return "of Amianthus";
        case 138: return "of Fire Quenching";
        case 139: return "of Faith";
        case 140: return "of Resistance";
        case 141: return "of Insulation";
        case 142: return "of Grounding";
        case 143: return "of the Dynamo";
        case 144: return "of Resistance";
        case 145: return "of Insulation";
        case 146: return "of Grounding";
        case 147: return "of the Dynamo";
        case 148: return "of Stoicism";
        case 149: return "of Warming";
        case 150: return "of Thawing";
        case 151: return "of the Dunes";
        case 152: return "of the Sirocco";
        case 153: return "of Warming";
        case 154: return "of Thawing";
        case 155: return "of the Dunes";
        case 156: return "of the Sirocco";
        case 157: return "of Desire";
        case 158: return "of Piercing";
        case 159: return "of Bashing";
        case 160: return "of Puncturing";
        case 161: return "of Thorns";
        case 162: return "of Spikes";
        case 163: return "of Razors";
        case 164: return "of Swords";
        case 165: return "of Malice";
        case 166: return "of Readiness";
        case 167: return "of Alacrity";
        case 168: return "of Swiftness";
        case 169: return "of Quickness";
        case 170: return "of Alacrity";
        case 171: return "of Fervor";
        case 172: return "of Blocking";
        case 173: return "of Deflecting";
        case 174: return "of the Apprentice";
        case 175: return "of the Magus";
        case 176: return "of Frost";
        case 177: return "of the Icicle";
        case 178: return "of the Glacier";
        case 179: return "of Winter";
        case 180: return "of Frost";
        case 181: return "of Frigidity";
        case 182: return "of Warmth";
        case 183: return "of Flame";
        case 184: return "of Fire";
        case 185: return "of Burning";
        case 186: return "of Incineration";
        case 187: return "of Flame";
        case 188: return "of Passion";
        case 189: return "of Shock";
        case 190: return "of Lightning";
        case 191: return "of Thunder";
        case 192: return "of Storms";
        case 193: return "of Shock";
        case 194: return "of Ennui";
        case 195: return "of Craftsmanship";
        case 196: return "of Quality";
        case 197: return "of Maiming";
        case 198: return "of Slaying";
        case 199: return "of Gore";
        case 200: return "of Carnage";
        case 201: return "of Slaughter";
        case 202: return "of Butchery";
        case 203: return "of Evisceration";
        case 204: return "of Maiming";
        case 205: return "of Craftsmanship";
        case 206: return "of Craftsmanship";
        case 207: return "of Craftsmanship";
        case 208: return "of Quality";
        case 209: return "of Quality";
        case 210: return "of Maiming";
        case 211: return "of Maiming";
        case 212: return "of Craftsmanship";
        case 213: return "of Craftsmanship";
        case 214: return "of Quality";
        case 215: return "of Quality";
        case 216: return "of Maiming";
        case 217: return "of Craftsmanship";
        case 218: return "of Quality";
        case 219: return "of Maiming";
        case 220: return "of Ire";
        case 221: return "of Wrath";
        case 222: return "of Carnage";
        case 223: return "of Worth";
        case 224: return "of Measure";
        case 225: return "of Excellence";
        case 226: return "of Performance";
        case 227: return "of Transcendence";
        case 228: return "of Worth";
        case 229: return "of Measure";
        case 230: return "of Excellence";
        case 231: return "of Performance";
        case 232: return "of Joyfulness";
        case 233: return "of Bliss";
        case 234: return "of Blight";
        case 235: return "of Venom";
        case 236: return "of Pestilence";
        case 237: return "of Anthrax";
        case 238: return "of Blight";
        case 239: return "of Envy";
        case 240: return "of Dexterity";
        case 241: return "of Skill";
        case 242: return "of Accuracy";
        case 243: return "of Precision";
        case 244: return "of Perfection";
        case 245: return "of Nirvana";
        case 246: return "of Dexterity";
        case 247: return "of Skill";
        case 248: return "of Accuracy";
        case 249: return "of Precision";
        case 250: return "of Perfection";
        case 251: return "of Dexterity";
        case 252: return "of Skill";
        case 253: return "of Accuracy";
        case 254: return "of Precision";
        case 255: return "of Dexterity";
        case 256: return "of Dexterity";
        case 257: return "of Dexterity";
        case 258: return "of Dexterity";
        case 259: return "of Dexterity";
        case 260: return "of Dexterity";
        case 261: return "of Daring";
        case 262: return "of Balance";
        case 263: return "of Equilibrium";
        case 264: return "of Stability";
        case 265: return "of Balance";
        case 266: return "of Balance";
        case 267: return "of Balance";
        case 268: return "of Truth";
        case 269: return "of Regeneration";
        case 270: return "of Regeneration";
        case 271: return "of Regeneration";
        case 272: return "of Regrowth";
        case 273: return "of Regrowth";
        case 274: return "of Revivification";
        case 275: return "of Honor";
        case 276: return "of Vileness";
        case 277: return "of Greed";
        case 278: return "of Wealth";
        case 279: return "of Greed";
        case 280: return "of Greed";
        case 281: return "of Greed";
        case 282: return "of Greed";
        case 283: return "of Greed";
        case 284: return "of Greed";
        case 285: return "of Avarice";
        case 286: return "of Chance";
        case 287: return "of Fortune";
        case 288: return "of Fortune";
        case 289: return "of Luck";
        case 290: return "of Fortune";
        case 291: return "of Good Luck";
        case 292: return "of Prosperity";
        case 293: return "of Energy";
        case 294: return "of the Mind";
        case 295: return "of Brilliance";
        case 296: return "of Sorcery";
        case 297: return "of Wizardry";
        case 298: return "of Enlightenment";
        case 299: return "of Energy";
        case 300: return "of the Mind";
        case 301: return "of Brilliance";
        case 302: return "of Sorcery";
        case 303: return "of Wizardry";
        case 304: return "of Energy";
        case 305: return "of the Mind";
        case 306: return "of Brilliance";
        case 307: return "of Sorcery";
        case 308: return "of Knowledge";
        case 309: return "of the Bear";
        case 310: return "of Light";
        case 311: return "of Radiance";
        case 312: return "of the Sun";
        case 313: return "of the Jackal";
        case 314: return "of the Fox";
        case 315: return "of the Wolf";
        case 316: return "of the Tiger";
        case 317: return "of the Mammoth";
        case 318: return "of the Colosuss";
        case 319: return "of the Squid";
        case 320: return "of the Whale";
        case 321: return "of the Jackal";
        case 322: return "of the Fox";
        case 323: return "of the Wolf";
        case 324: return "of the Tiger";
        case 325: return "of the Mammoth";
        case 326: return "of the Colosuss";
        case 327: return "of the Jackal";
        case 328: return "of the Fox";
        case 329: return "of the Wolf";
        case 330: return "of the Tiger";
        case 331: return "of the Mammoth";
        case 332: return "of Life";
        case 333: return "of Life";
        case 334: return "of Life";
        case 335: return "of Substinence";
        case 336: return "of Substinence";
        case 337: return "of Substinence";
        case 338: return "of Vita";
        case 339: return "of Vita";
        case 340: return "of Vita";
        case 341: return "of Life";
        case 342: return "of Life";
        case 343: return "of Substinence";
        case 344: return "of Substinence";
        case 345: return "of Vita";
        case 346: return "of Vita";
        case 347: return "of Life";
        case 348: return "of Substinence";
        case 349: return "of Vita";
        case 350: return "of Spirit";
        case 351: return "of Hope";
        case 352: return "of the Leech";
        case 353: return "of the Locust";
        case 354: return "of the Lamprey";
        case 355: return "of the Leech";
        case 356: return "of the Locust";
        case 357: return "of the Lamprey";
        case 358: return "of the Leech";
        case 359: return "of the Bat";
        case 360: return "of the Wraith";
        case 361: return "of the Vampire";
        case 362: return "of the Bat";
        case 363: return "of the Wraith";
        case 364: return "of the Vampire";
        case 365: return "of the Bat";
        case 366: return "of Defiance";
        case 367: return "of Amelioration";
        case 368: return "of Remedy";
        case 369: return "of Simplicity";
        case 370: return "of Ease";
        case 371: return "of Freedom";
        case 372: return "of Strength";
        case 373: return "of Might";
        case 374: return "of the Ox";
        case 375: return "of the Giant";
        case 376: return "of the Titan";
        case 377: return "of Atlus";
        case 378: return "of Strength";
        case 379: return "of Might";
        case 380: return "of the Ox";
        case 381: return "of the Giant";
        case 382: return "of the Titan";
        case 383: return "of Strength";
        case 384: return "of Might";
        case 385: return "of the Ox";
        case 386: return "of the Giant";
        case 387: return "of Strength";
        case 388: return "of Strength";
        case 389: return "of Strength";
        case 390: return "of Strength";
        case 391: return "of Strength";
        case 392: return "of Strength";
        case 393: return "of Virility";
        case 394: return "of Pacing";
        case 395: return "of Haste";
        case 396: return "of Speed";
        case 397: return "of Traveling";
        case 398: return "of Acceleration";
        case 399: return "of Inertia";
        case 400: return "of Inertia";
        case 401: return "of Inertia";
        case 402: return "of Self-Repair";
        case 403: return "of Fast Repair";
        case 404: return "of Ages";
        case 405: return "of Replenishing";
        case 406: return "of Propogation";
        case 407: return "of the Kraken";
        case 408: return "of Memory";
        case 409: return "of the Elephant";
        case 410: return "of Power";
        case 411: return "of Grace";
        case 412: return "of Grace and Power";
        case 413: return "of the Yeti";
        case 414: return "of the Phoenix";
        case 415: return "of the Efreeti";
        case 416: return "of the Cobra";
        case 417: return "of the Elements";
        case 418: return "of Firebolts";
        case 419: return "of Firebolts";
        case 420: return "of Firebolts";
        case 421: return "of Charged Bolt";
        case 422: return "of Charged Bolt";
        case 423: return "of Charged Bolt";
        case 424: return "of Icebolt";
        case 425: return "of Frozen Armor";
        case 426: return "of Static Field";
        case 427: return "of Telekinesis";
        case 428: return "of Frost Shield";
        case 429: return "of Ice Blast";
        case 430: return "of Blaze";
        case 431: return "of Fire Ball";
        case 432: return "of Nova";
        case 433: return "of Nova";
        case 434: return "of Nova Shield";
        case 435: return "of Nova Shield";
        case 436: return "of Nova Shield";
        case 437: return "of Lightning";
        case 438: return "of Lightning";
        case 439: return "of Shiver Armor";
        case 440: return "of Fire Wall";
        case 441: return "of Enchant";
        case 442: return "of Chain Lightning";
        case 443: return "of Chain Lightning";
        case 444: return "of Chain Lightning";
        case 445: return "of Teleport Shield";
        case 446: return "of Teleport Shield";
        case 447: return "of Teleport Shield";
        case 448: return "of Glacial Spike";
        case 449: return "of Meteor";
        case 450: return "of Thunder Storm";
        case 451: return "of Energy Shield";
        case 452: return "of Blizzard";
        case 453: return "of Chilling Armor";
        case 454: return "of Hydra Shield";
        case 455: return "of Frozen Orb";
        case 456: return "of Dawn";
        case 457: return "of Sunlight";
        case 458: return "of Magic Arrows";
        case 459: return "of Magic Arrows";
        case 460: return "of Fire Arrows";
        case 461: return "of Fire Arrows";
        case 462: return "of Inner Sight";
        case 463: return "of Inner Sight";
        case 464: return "of Jabbing";
        case 465: return "of Jabbing";
        case 466: return "of Cold Arrows";
        case 467: return "of Cold Arrows";
        case 468: return "of Multiple Shot";
        case 469: return "of Multiple Shot";
        case 470: return "of Power Strike";
        case 471: return "of Power Strike";
        case 472: return "of Poison Jab";
        case 473: return "of Poison Jab";
        case 474: return "of Exploding Arrows";
        case 475: return "of Exploding Arrows";
        case 476: return "of Slow Missiles";
        case 477: return "of Slow Missiles";
        case 478: return "of Impaling Strike";
        case 479: return "of Impaling Strike";
        case 480: return "of Lightning Javelin";
        case 481: return "of Lightning Javelin";
        case 482: return "of Ice Arrows";
        case 483: return "of Ice Arrows";
        case 484: return "of Guided Arrows";
        case 485: return "of Guided Arrows";
        case 486: return "of Charged Strike";
        case 487: return "of Charged Strike";
        case 488: return "of Plague Jab";
        case 489: return "of Plague Jab";
        case 490: return "of Immolating Arrows";
        case 491: return "of Immolating Arrows";
        case 492: return "of Fending";
        case 493: return "of Fending";
        case 494: return "of Freezing Arrows";
        case 495: return "of Freezing Arrows";
        case 496: return "of Lightning Strike";
        case 497: return "of Lightning Strike";
        case 498: return "of Lightning Fury";
        case 499: return "of Lightning Fury";
        case 500: return "of Fire Bolts";
        case 501: return "of Fire Bolts";
        case 502: return "of Charged Bolts";
        case 503: return "of Charged Bolts";
        case 504: return "of Ice Bolts";
        case 505: return "of Ice Bolts";
        case 506: return "of Frozen Armor";
        case 507: return "of Frozen Armor";
        case 508: return "of Static Field";
        case 509: return "of Static Field";
        case 510: return "of Telekinesis";
        case 511: return "of Telekinesis";
        case 512: return "of Frost Novas";
        case 513: return "of Frost Novas";
        case 514: return "of Ice Blasts";
        case 515: return "of Ice Blasts";
        case 516: return "of Blazing";
        case 517: return "of Blazing";
        case 518: return "of Fire Balls";
        case 519: return "of Fire Balls";
        case 520: return "of Novas";
        case 521: return "of Novas";
        case 522: return "of Lightning";
        case 523: return "of Lightning";
        case 524: return "of Shiver Armor";
        case 525: return "of Shiver Armor";
        case 526: return "of Fire Walls";
        case 527: return "of Fire Walls";
        case 528: return "of Enchantment";
        case 529: return "of Enchantment";
        case 530: return "of Chain Lightning";
        case 531: return "of Chain Lightning";
        case 532: return "of Teleportation";
        case 533: return "of Teleportation";
        case 534: return "of Glacial Spikes";
        case 535: return "of Glacial Spikes";
        case 536: return "of Meteors";
        case 537: return "of Meteors";
        case 538: return "of Thunder Storm";
        case 539: return "of Thunder Storm";
        case 540: return "of Energy Shield";
        case 541: return "of Energy Shield";
        case 542: return "of Blizzards";
        case 543: return "of Blizzards";
        case 544: return "of Chilling Armor";
        case 545: return "of Chilling Armor";
        case 546: return "of Hydras";
        case 547: return "of Hydras";
        case 548: return "of Frozen Orbs";
        case 549: return "of Frozen Orbs";
        case 550: return "of Amplify Damage";
        case 551: return "of Amplify Damage";
        case 552: return "of Teeth";
        case 553: return "of Teeth";
        case 554: return "of Bone Armor";
        case 555: return "of Bone Armor";
        case 556: return "of Raise Skeletons";
        case 557: return "of Raise Skeletons";
        case 558: return "of Dim Vision";
        case 559: return "of Dim Vision";
        case 560: return "of Weaken";
        case 561: return "of Weaken";
        case 562: return "of Poison Dagger";
        case 563: return "of Poison Dagger";
        case 564: return "of Corpse Explosions";
        case 565: return "of Corpse Explosions";
        case 566: return "of Clay Golem Summoning";
        case 567: return "of Clay Golem Summoning";
        case 568: return "of Iron Maiden";
        case 569: return "of Iron Maiden";
        case 570: return "of Terror";
        case 571: return "of Terror";
        case 572: return "of Bone Walls";
        case 573: return "of Bone Walls";
        case 574: return "of Raise Skeletal Mages";
        case 575: return "of Raise Skeletal Mages";
        case 576: return "of Confusion";
        case 577: return "of Confusion";
        case 578: return "of Life Tap";
        case 579: return "of Life Tap";
        case 580: return "of Poison Explosion";
        case 581: return "of Poison Explosion";
        case 582: return "of Bone Spears";
        case 583: return "of Bone Spears";
        case 584: return "of Blood Golem Summoning";
        case 585: return "of Blood Golem Summoning";
        case 586: return "of Attraction";
        case 587: return "of Attraction";
        case 588: return "of Decrepification";
        case 589: return "of Decrepification";
        case 590: return "of Bone Imprisonment";
        case 591: return "of Bone Imprisonment";
        case 592: return "of Iron Golem Creation";
        case 593: return "of Iron Golem Creation";
        case 594: return "of Lower Resistance";
        case 595: return "of Lower Resistance";
        case 596: return "of Poison Novas";
        case 597: return "of Poison Novas";
        case 598: return "of Bone Spirits";
        case 599: return "of Bone Spirits";
        case 600: return "of Fire Golem Summoning";
        case 601: return "of Fire Golem Summoning";
        case 602: return "of Revivification";
        case 603: return "of Revivification";
        case 604: return "of Sacrifice";
        case 605: return "of Sacrifice";
        case 606: return "of Holy Bolts";
        case 607: return "of Holy Bolts";
        case 608: return "of Zeal";
        case 609: return "of Zeal";
        case 610: return "of Vengeance";
        case 611: return "of Vengeance";
        case 612: return "of Blessed Hammers";
        case 613: return "of Blessed Hammers";
        case 614: return "of Conversion";
        case 615: return "of Conversion";
        case 616: return "of Fist of the Heavens";
        case 617: return "of Fist of the Heavens";
        case 618: return "of Bashing";
        case 619: return "of Bashing";
        case 620: return "of Howling";
        case 621: return "of Howling";
        case 622: return "of Potion Finding";
        case 623: return "of Potion Finding";
        case 624: return "of Taunting";
        case 625: return "of Taunting";
        case 626: return "of Shouting";
        case 627: return "of Shouting";
        case 628: return "of Stunning";
        case 629: return "of Stunning";
        case 630: return "of Item Finding";
        case 631: return "of Item Finding";
        case 632: return "of Concentration";
        case 633: return "of Concentration";
        case 634: return "of Battle Cry";
        case 635: return "of Battle Cry";
        case 636: return "of Battle Orders";
        case 637: return "of Battle Orders";
        case 638: return "of Grim Ward";
        case 639: return "of Grim Ward";
        case 640: return "of War Cry";
        case 641: return "of War Cry";
        case 642: return "of Battle Command";
        case 643: return "of Battle Command";
        case 644: return "of Firestorms";
        case 645: return "of Firestorms";
        case 646: return "of Molten Boulders";
        case 647: return "of Molten Boulders";
        case 648: return "of Eruption";
        case 649: return "of Eruption";
        case 650: return "of Cyclone Armor";
        case 651: return "of Cyclone Armor";
        case 652: return "of Twister";
        case 653: return "of Twister";
        case 654: return "of Volcano";
        case 655: return "of Volcano";
        case 656: return "of Tornado";
        case 657: return "of Tornado";
        case 658: return "of Armageddon";
        case 659: return "of Armageddon";
        case 660: return "of Hurricane";
        case 661: return "of Hurricane";
        case 662: return "of Damage Amplification";
        case 663: return "of the Icicle";
        case 664: return "of the Glacier";
        case 665: return "of Fire";
        case 666: return "of Burning";
        case 667: return "of Lightning";
        case 668: return "of Thunder";
        case 669: return "of Daring";
        case 670: return "of Daring";
        case 671: return "of Knowledge";
        case 672: return "of Knowledge";
        case 673: return "of Virility";
        case 674: return "of Virility";
        case 675: return "of Readiness";
        case 676: return "of Craftsmanship";
        case 677: return "of Quality";
        case 678: return "of Maiming";
        case 679: return "of Craftsmanship";
        case 680: return "of Quality";
        case 681: return "of Craftsmanship";
        case 682: return "of Blight";
        case 683: return "of Venom";
        case 684: return "of Pestilence";
        case 685: return "of Anthrax";
        case 686: return "of Blight";
        case 687: return "of Venom";
        case 688: return "of Pestilence";
        case 689: return "of Anthrax";
        case 690: return "of Blight";
        case 691: return "of Venom";
        case 692: return "of Pestilence";
        case 693: return "of Anthrax";
        case 694: return "of Frost";
        case 695: return "of the Icicle";
        case 696: return "of the Glacier";
        case 697: return "of Winter";
        case 698: return "of Frost";
        case 699: return "of the Icicle";
        case 700: return "of the Glacier";
        case 701: return "of Winter";
        case 702: return "of Frost";
        case 703: return "of the Icicle";
        case 704: return "of the Glacier";
        case 705: return "of Winter";
        case 706: return "of Flame";
        case 707: return "of Fire";
        case 708: return "of Burning";
        case 709: return "of Incineration";
        case 710: return "of Flame";
        case 711: return "of Fire";
        case 712: return "of Burning";
        case 713: return "of Incineration";
        case 714: return "of Flame";
        case 715: return "of Fire";
        case 716: return "of Burning";
        case 717: return "of Incineration";
        case 718: return "of Shock";
        case 719: return "of Lightning";
        case 720: return "of Thunder";
        case 721: return "of Storms";
        case 722: return "of Shock";
        case 723: return "of Lightning";
        case 724: return "of Thunder";
        case 725: return "of Storms";
        case 726: return "of Shock";
        case 727: return "of Lightning";
        case 728: return "of Thunder";
        case 729: return "of Storms";
        case 730: return "of Dexterity";
        case 731: return "of Dexterity";
        case 732: return "of Strength";
        case 733: return "of Strength";
        case 734: return "of Thorns";
        case 735: return "of Frost";
        case 736: return "of Flame";
        case 737: return "of Blight";
        case 738: return "of Shock";
        case 739: return "of Regeneration";
        case 740: return "of Energy";
        case 741: return "of Light";
        case 742: return "of the Leech";
        case 743: return "of the Locust";
        case 744: return "of the Lamprey";
        case 745: return "of the Bat";
        case 746: return "of the Wraith";
        case 747: return "of the Vampire";
        }

        return "";
    }

    std::string getUniqueNameFromId(std::uint16_t id)
    {
        switch (id)
        {
        case 0: return "The Gnasher";
        case 1: return "Deathspade";
        case 2: return "Bladebone";
        case 3: return "Skull Splitter";
        case 4: return "Rakescar";
        case 5: return "Axe of Fechmar";
        case 6: return "Goreshovel";
        case 7: return "The Chiefthan";
        case 8: return "Brainhew";
        case 9: return "Humongous";
        case 10: return "Torch of Iros";
        case 11: return "Maelstorm";
        case 12: return "Gravenspine";
        case 13: return "Umes Lament";
        case 14: return "Felloak";
        case 15: return "Knell Striker";
        case 16: return "Rusthandle";
        case 17: return "Stormeye";
        case 18: return "Stoutnail";
        case 19: return "Crushflange";
        case 20: return "Bloodrise";
        case 21: return "The Generals Tan Do Li Ga";
        case 22: return "Ironstone";
        case 23: return "Bonesnap";
        case 24: return "Steeldriver";
        case 25: return "Rixot's Keen";
        case 26: return "Blood Crescent";
        case 27: return "Skewer of Krintiz";
        case 28: return "Gleamscythe";
        case 29: return "Azurewrath";
        case 30: return "Griswold's Edge";
        case 31: return "Hellplague";
        case 32: return "Culwens Point";
        case 33: return "Shadowfang";
        case 34: return "Soulflay";
        case 35: return "Kinemils Awl";
        case 36: return "Blacktongue";
        case 37: return "Ripsaw";
        case 38: return "The Patriarch";
        case 39: return "Gull";
        case 40: return "The Diggler";
        case 41: return "The Jade Tan Do";
        case 42: return "Spectral Shard";
        case 43: return "The Dragon Chang";
        case 44: return "Razortine";
        case 45: return "Bloodthief";
        case 46: return "Lance of Yaggai";
        case 47: return "The Tannr Gorerod";
        case 48: return "Dimoaks Hew";
        case 49: return "Steelgoad";
        case 50: return "Soul Harvest";
        case 51: return "The Battlebranch";
        case 52: return "Woestave";
        case 53: return "The Grim Reaper";
        case 54: return "Bane Ash";
        case 55: return "Serpent Lord";
        case 56: return "Spire of Lazarus";
        case 57: return "The Salamander";
        case 58: return "The Iron Jang Bong";
        case 59: return "Pluckeye";
        case 60: return "Witherstring";
        case 61: return "Raven Claw";
        case 62: return "Rogue's Bow";
        case 63: return "Stormstrike";
        case 64: return "Wizendraw";
        case 65: return "Hellclap";
        case 66: return "Blastbark";
        case 67: return "Leadcrow";
        case 68: return "Ichorsting";
        case 69: return "Hellcast";
        case 70: return "Doomslinger";
        case 71: return "Biggin's Bonnet";
        case 72: return "Tarnhelm";
        case 73: return "Coif of Glory";
        case 74: return "Duskdeep";
        case 75: return "Wormskull";
        case 76: return "Howltusk";
        case 77: return "Undead Crown";
        case 78: return "The Face of Horror";
        case 79: return "Greyform";
        case 80: return "Blinkbat's Form";
        case 81: return "The Centurion";
        case 82: return "Twitchthroe";
        case 83: return "Darkglow";
        case 84: return "Hawkmail";
        case 85: return "Sparking Mail";
        case 86: return "Venom Ward";
        case 87: return "Iceblink";
        case 88: return "Boneflesh";
        case 89: return "Rockfleece";
        case 90: return "Rattlecage";
        case 91: return "Goldskin";
        case 92: return "Victors Silk";
        case 93: return "Heavenly Garb";
        case 94: return "Pelta Lunata";
        case 95: return "Umbral Disk";
        case 96: return "Stormguild";
        case 97: return "Wall of the Eyeless";
        case 98: return "Swordback Hold";
        case 99: return "Steelclash";
        case 100: return "Bverrit Keep";
        case 101: return "The Ward";
        case 102: return "The Hand of Broc";
        case 103: return "Bloodfist";
        case 104: return "Chance Guards";
        case 105: return "Magefist";
        case 106: return "Frostburn";
        case 107: return "Hotspur";
        case 108: return "Gorefoot";
        case 109: return "Treads of Cthon";
        case 110: return "Goblin Toe";
        case 111: return "Tearhaunch";
        case 112: return "Lenymo";
        case 113: return "Snakecord";
        case 114: return "Nightsmoke";
        case 115: return "Goldwrap";
        case 116: return "Bladebuckle";
        case 117: return "Nokozan Relic";
        case 118: return "The Eye of Etlich";
        case 119: return "The Mahim-Oak Curio";
        case 120: return "Nagelring";
        case 121: return "Manald Heal";
        case 122: return "The Stone of Jordan";
        case 123: return "Amulet of the Viper";
        case 124: return "Staff of Kings";
        case 125: return "Horadric Staff";
        case 126: return "Hell Forge Hammer";
        case 127: return "Khalim's Flail";
        case 128: return "Super Khalim's Flail";
        case 129: return "Coldkill";
        case 130: return "Butcher's Pupil";
        case 131: return "Islestrike";
        case 132: return "Pompe's Wrath";
        case 133: return "Guardian Naga";
        case 134: return "Warlord's Trust";
        case 135: return "Spellsteel";
        case 136: return "Stormrider";
        case 137: return "Boneslayer Blade";
        case 138: return "The Minataur";
        case 139: return "Suicide Branch";
        case 140: return "Carin Shard";
        case 141: return "Arm of King Leoric";
        case 142: return "Blackhand Key";
        case 143: return "Dark Clan Crusher";
        case 144: return "Zakarum's Hand";
        case 145: return "The Fetid Sprinkler";
        case 146: return "Hand of Blessed Light";
        case 147: return "Fleshrender";
        case 148: return "Sureshrill Frost";
        case 149: return "Moonfall";
        case 150: return "Baezil's Vortex";
        case 151: return "Earthshaker";
        case 152: return "Bloodtree Stump";
        case 153: return "The Gavel of Pain";
        case 154: return "Bloodletter";
        case 155: return "Coldsteel Eye";
        case 156: return "Hexfire";
        case 157: return "Blade of Ali Baba";
        case 158: return "Ginther's Rift";
        case 159: return "Headstriker";
        case 160: return "Plague Bearer";
        case 161: return "The Atlantian";
        case 162: return "Crainte Vomir";
        case 163: return "Bing Sz Wang";
        case 164: return "The Vile Husk";
        case 165: return "Cloudcrack";
        case 166: return "Todesfaelle Flamme";
        case 167: return "Swordguard";
        case 168: return "Spineripper";
        case 169: return "Heart Carver";
        case 170: return "Blackbog's Sharp";
        case 171: return "Stormspike";
        case 172: return "The Impaler";
        case 173: return "Kelpie Snare";
        case 174: return "Soulfeast Tine";
        case 175: return "Hone Sundan";
        case 176: return "Spire of Honor";
        case 177: return "The Meat Scraper";
        case 178: return "Blackleach Blade";
        case 179: return "Athena's Wrath";
        case 180: return "Pierre Tombale Couant";
        case 181: return "Husoldal Evo";
        case 182: return "Grim's Burning Dead";
        case 183: return "Razorswitch";
        case 184: return "Ribcracker";
        case 185: return "Chromatic Ire";
        case 186: return "Warpspear";
        case 187: return "Skullcollector";
        case 188: return "Skystrike";
        case 189: return "Riphook";
        case 190: return "Kuko Shakaku";
        case 191: return "Endlesshail";
        case 192: return "Whichwild String";
        case 193: return "Cliffkiller";
        case 194: return "Magewrath";
        case 195: return "Godstrike Arch";
        case 196: return "Langer Briser";
        case 197: return "Pus Spiter";
        case 198: return "Buriza-Do Kyanon";
        case 199: return "Demon Machine";
        case 200: return "Armor (Unknown)";
        case 201: return "Peasent Crown";
        case 202: return "Rockstopper";
        case 203: return "Stealskull";
        case 204: return "Darksight Helm";
        case 205: return "Valkyrie Wing";
        case 206: return "Crown of Thieves";
        case 207: return "Blckhorn's Face";
        case 208: return "Vampire Gaze";
        case 209: return "The Spirit Shroud";
        case 210: return "Skin of the Vipermagi";
        case 211: return "Skin of the Flayed One";
        case 212: return "Ironpelt";
        case 213: return "Spiritforge";
        case 214: return "Crow Caw";
        case 215: return "Shaftstop";
        case 216: return "Duriel's Shell";
        case 217: return "Skullder's Ire";
        case 218: return "Guardian Angel";
        case 219: return "Toothrow";
        case 220: return "Atma's Wail";
        case 221: return "Black Hades";
        case 222: return "Corpsemourn";
        case 223: return "Que-Hegan's Wisdom";
        case 224: return "Visceratuant";
        case 225: return "Mosers Blessed Circle";
        case 226: return "Stormchaser";
        case 227: return "Tiamat's Rebuke";
        case 228: return "Gerke's Sanctuary";
        case 229: return "Radimant's Sphere";
        case 230: return "Lidless Wall";
        case 231: return "Lance Guard";
        case 232: return "Venom Grip";
        case 233: return "Gravepalm";
        case 234: return "Ghoulhide";
        case 235: return "Lavagout";
        case 236: return "Hellmouth";
        case 237: return "Infernostride";
        case 238: return "Waterwalk";
        case 239: return "Silkweave";
        case 240: return "Wartraveler";
        case 241: return "Gorerider";
        case 242: return "String of Ears";
        case 243: return "Razortail";
        case 244: return "Gloomstrap";
        case 245: return "Snowclash";
        case 246: return "Thundergod's Vigor";
        case 247: return "Elite unique";
        case 248: return "Harlequin Crest";
        case 249: return "Veil of Steel";
        case 250: return "The Gladiator's Bane";
        case 251: return "Arkaine's Valor";
        case 252: return "Blackoak Shield";
        case 253: return "Stormshield";
        case 254: return "Hellslayer";
        case 255: return "Messerschmidt's Reaver";
        case 256: return "Baranar's Star";
        case 257: return "Schaefer's Hammer";
        case 258: return "The Cranium Basher";
        case 259: return "Lightsabre";
        case 260: return "Doombringer";
        case 261: return "The Grandfather";
        case 262: return "Wizardspike";
        case 263: return "Constricting Ring";
        case 264: return "Stormspire";
        case 265: return "Eaglehorn";
        case 266: return "Windforce";
        case 267: return "Ring";
        case 268: return "Bul Katho's Wedding Band";
        case 269: return "The Cat's Eye";
        case 270: return "The Rising Sun";
        case 271: return "Crescent Moon";
        case 272: return "Mara's Kaleidoscope";
        case 273: return "Atma's Scarab";
        case 274: return "Dwarf Star";
        case 275: return "Raven Frost";
        case 276: return "Highlord's Wrath";
        case 277: return "Saracen's Chance";
        case 278: return "Class specific";
        case 279: return "Arreat's Face";
        case 280: return "Homunculus";
        case 281: return "Titan's Revenge";
        case 282: return "Lycander's Aim";
        case 283: return "Lycander's Flank";
        case 284: return "The Oculus";
        case 285: return "Herald of Zakarum";
        case 286: return "Bartuc's Cut-Throat";
        case 287: return "Jalal's Mane";
        case 288: return "The Scalper";
        case 289: return "Bloodmoon";
        case 290: return "Djinnslayer";
        case 291: return "Deathbit";
        case 292: return "Warshrike";
        case 293: return "Gutsiphon";
        case 294: return "Razoredge";
        case 295: return "Gore Ripper";
        case 296: return "Demon Limb";
        case 297: return "Steel Shade";
        case 298: return "Tomb Reaver";
        case 299: return "Death's Web";
        case 300: return "Nature's Peace";
        case 301: return "Azurewrath";
        case 302: return "Seraph's Hymn";
        case 303: return "Zakarum's Salvation";
        case 304: return "Fleshripper";
        case 305: return "Odium";
        case 306: return "Horizon's Tornado";
        case 307: return "Stone Crusher";
        case 308: return "Jade Talon";
        case 309: return "Shadow Dancer";
        case 310: return "Cerebus' Bite";
        case 311: return "Tyrael's Might";
        case 312: return "Soul Drainer";
        case 313: return "Rune Master";
        case 314: return "Death Cleaver";
        case 315: return "Executioner's Justice";
        case 316: return "Stoneraven";
        case 317: return "Leviathan";
        case 318: return "Larzuk's Champion";
        case 319: return "Wisp Projector";
        case 320: return "Gargoyle's Bite";
        case 321: return "Lacerator";
        case 322: return "Mang Song's Lesson";
        case 323: return "Viperfork";
        case 324: return "Ethereal Edge";
        case 325: return "Demonhorn's Edge";
        case 326: return "The Reaper's Toll";
        case 327: return "Spiritkeeper";
        case 328: return "Hellrack";
        case 329: return "Alma Negra";
        case 330: return "Darkforce Spawn";
        case 331: return "Widowmaker";
        case 332: return "Bloodraven's Charge";
        case 333: return "Ghostflame";
        case 334: return "Shadowkiller";
        case 335: return "Gimmershred";
        case 336: return "Griffon's Eye";
        case 337: return "Windhammer";
        case 338: return "Thunderstroke";
        case 339: return "Giant Maimer";
        case 340: return "Demon's Arch";
        case 341: return "Boneflame";
        case 342: return "Steelpillar";
        case 343: return "Nightwing's Veil";
        case 344: return "Crown of Ages";
        case 345: return "Andariel's Visage";
        case 346: return "Darkfear";
        case 347: return "Dragonscale";
        case 348: return "Steel Carapice";
        case 349: return "Medusa's Gaze";
        case 350: return "Ravenlore";
        case 351: return "Boneshade";
        case 352: return "Nethercrow";
        case 353: return "Flamebellow";
        case 354: return "Fathom";
        case 355: return "Wolfhowl";
        case 356: return "Spirit Ward";
        case 357: return "Kira's Guardian";
        case 358: return "Ormus Robes";
        case 359: return "Gheed's Fortune";
        case 360: return "Stormlash";
        case 361: return "Halaberd's Reign";
        case 362: return "Warriv's Warder";
        case 363: return "Spike Thorn";
        case 364: return "Dracul's Grasp";
        case 365: return "Frostwind";
        case 366: return "Templar's Might";
        case 367: return "Eschuta's Temper";
        case 368: return "Firelizard's Talons";
        case 369: return "Sandstorm Trek";
        case 370: return "Marrowwalk";
        case 371: return "Heaven's Light";
        case 372: return "Merman's Speed";
        case 373: return "Arachnid Mesh";
        case 374: return "Nosferatu's Coil";
        case 375: return "Metalgrid";
        case 376: return "Verdugo's Hearty Cord";
        case 377: return "Sigurd's Staunch";
        case 378: return "Carrion Wind";
        case 379: return "Giantskull";
        case 380: return "Ironward";
        case 381: return "Annihilus";
        case 382: return "Arioc's Needle";
        case 383: return "Cranebeak";
        case 384: return "Nord's Tenderizer";
        case 385: return "Earthshifter";
        case 386: return "Wraithflight";
        case 387: return "Bonehew";
        case 388: return "Ondal's Wisdom";
        case 389: return "The Reedeemer";
        case 390: return "Headhunter's Glory";
        case 391: return "Steelrend";
        case 392: return "Rainbow Facet";
        case 393: return "Rainbow Facet";
        case 394: return "Rainbow Facet";
        case 395: return "Rainbow Facet";
        case 396: return "Rainbow Facet";
        case 397: return "Rainbow Facet";
        case 398: return "Rainbow Facet";
        case 399: return "Rainbow Facet";
        case 400: return "Hellfire Torch";
        }

        return "";
    }

    bool magicalAttributeSorter(const MagicalAttribute& left, const MagicalAttribute& right)
    {
        static std::vector<size_t> attribSortPriority = { 67, 61, 65, 63, 0, 0, 0, 59, 0, 55, 0, 51, 0, 0, 0, 0, 74, 129, 130, 115, 134, 127,
            126, 124, 123, 0, 0, 52, 48, 0, 0, 71, 69, 70, 22, 21, 22, 41, 46, 36, 42, 38, 43, 40, 44, 34, 45, 0, 102, 101, 99, 98, 104, 103,
            96, 95, 0, 92, 91, 0, 88, 0, 89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 3, 58, 54, 13, 10, 8, 76, 0, 150, 0, 11, 16, 8, 0, 6, 0, 0, 0,
            145, 0, 0, 148, 81, 0, 139, 0, 0, 136, 0, 0, 142, 0, 81, 81, 0, 18, 122, 79, 80, 11, 119, 118, 81, 19, 117, 75, 112, 108, 110, 106,
            5, 157, 158, 14, 0, 0, 0, 0, 0, 78, 83, 87, 121, 16, 15, 0, 85, 23, 27, 24, 29, 26, 33, 25, 31, 77, 159, 160, 20, 49, 17, 132, 131,
            133, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 108, 106, 0, 0, 0, 0, 0, 0, 0, 151, 0, 0, 0, 0, 0, 0, 160, 160,
            160, 160, 160, 0, 160, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 72, 73, 57, 53, 125, 128, 66, 64, 60, 62, 114, 116, 94, 100, 97, 90, 39,
            35, 37, 33, 32, 28, 30, 0, 12, 9, 7, 47, 50, 111, 107, 109, 105, 86, 82, 120, 84, 0, 1, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 0, 180, 180, 180,
            180, 180, 180, 180, 180, 180, 180, 180, 180, 0, 88, 88, 88, 88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 88, 88,
            88, 88, 88, 88, 88, 88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        if (left.Id > attribSortPriority.size() || right.Id > attribSortPriority.size())
        {
            return (left.Id < right.Id);
        }

        const auto& leftStat = attribSortPriority[left.Id];
        const auto& rightStat = attribSortPriority[right.Id];
        if (leftStat == 0)
        {
            if (rightStat == 0)
            {
                return (left.Id < right.Id);
            }

            return false;
        }

        if (rightStat == 0)
        {
            return true;
        }

        if (leftStat == rightStat)
        {
            return (left.Id < right.Id);
        }

        return (leftStat > rightStat);
    }

    void checkForRelatedMagicalAttributes(std::vector<MagicalAttribute>& attribs)
    {
        // check for the "all" cases
        std::map<std::uint16_t, std::reference_wrapper<MagicalAttribute>> relatedPropIdMap;
        std::int64_t numRelatedProps[2] = { 0, 0 };
        std::int64_t relatedPropValues[2] = { 0, 0 };
        bool validProps[2] = { true, true };
        for (auto& attrib : attribs)
        {
            if (!validProps[0] && !validProps[1])
            {
                return;
            }

            switch (attrib.Id)
            {
            case 0:
            case 1:
            case 2:
            case 3:
                if (validProps[0])
                {
                    if (relatedPropIdMap.find(attrib.Id) == relatedPropIdMap.end())
                    {
                        relatedPropIdMap.emplace(attrib.Id, std::ref(attrib));
                        if ((numRelatedProps[0] == 0) || (attrib.Values[0] == relatedPropValues[0]))
                        {
                            ++numRelatedProps[0];
                            relatedPropValues[0] = attrib.Values[0];
                        }
                        else
                        {
                            // non matching case
                            validProps[0] = false;
                            numRelatedProps[0] = 0;
                        }
                    }
                    else
                    {
                        // more than one poperty of the same type
                        validProps[0] = false;
                        numRelatedProps[0] = 0;
                    }
                }
                break;

            case 39:
            case 41:
            case 43:
            case 45:
                if (validProps[1])
                {
                    if (relatedPropIdMap.find(attrib.Id) == relatedPropIdMap.end())
                    {
                        relatedPropIdMap.emplace(attrib.Id, std::ref(attrib));
                        if ((numRelatedProps[1] == 0) || (attrib.Values[0] == relatedPropValues[1]))
                        {
                            ++numRelatedProps[1];
                            relatedPropValues[1] = attrib.Values[0];
                        }
                        else
                        {
                            // non matching case
                            validProps[1] = false;
                            numRelatedProps[0] = 0;
                        }
                    }
                    else
                    {
                        // more than one poperty of the same type
                        validProps[1] = false;
                        numRelatedProps[1] = 0;
                    }
                }
                break;
            }
        }

        for (size_t i = 0; i < 2; ++i)
        {
            if (validProps[i] && (numRelatedProps[i] != 4))
            {
                validProps[i] = false;
            }
        }

        if (!validProps[0] && !validProps[1])
        {
            return;
        }

        for (auto& prop : relatedPropIdMap)
        {
            auto& attrib = prop.second.get();
            switch (attrib.Id)
            {
            case 0:
            case 1:
            case 2:
                if (validProps[0])
                {
                    attrib.Desc = "+{0} to All Attributes";
                    attrib.Visible = false;
                }
                break;

            case 3:
                if (validProps[0])
                {
                    attrib.Desc = "+{0} to All Attributes";
                }
                break;

            case 39:
            case 41:
            case 43:
                if (validProps[1])
                {
                    attrib.Desc = "All Resistances +{0}";
                    attrib.Visible = false;
                }
                break;

            case 45:
                if (validProps[1])
                {
                    attrib.Desc = "All Resistances +{0}";
                }
                break;
            }
        }
    }

    std::int64_t getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx, const ItemStat& stat)
    {
        if (idx >= attrib.Values.size())
        {
            return 0;
        }

        auto value = attrib.Values[idx];
        std::stringstream ssValue;
        switch (attrib.Id)
        {
            // values at index 0 and 1 are 256 * the posion damage over the duration of damage
        case 57:
            switch (idx)
            {
            case 0:
            case 1:
                return (value * attrib.Values[2] + 128) / 256;

                // value at index 3 is 25 * duration of postion damage
            case 2:
                return value / 25;
            }
            break;

            // value at index 0 with range 0 to 128 must be converted to a percentage
        case 112:
            switch (idx)
            {
            case 0:
                return  (value * 100) / 128;
            }
            break;

            // value at index 0 is durability units per 100 seconds
        case 252:
            switch (idx)
            {
            case 0:
                return 100 / value;
            }
            break;

        default:
            if ((idx == 0) && (stat.opAttribs.op_base == "level"))
            {
                // stats based on character level
                switch (stat.opAttribs.op_param)
                {
                case 1:
                    attrib.OpValue = (charLevel * attrib.Values[0]) / 100;
                    attrib.OpStats = stat.opAttribs.op_stats;
                    return  attrib.OpValue;

                case 2:
                case 3:
                case 4:
                case 5:
                    attrib.OpValue = (charLevel * attrib.Values[0]) / (2 * std::int64_t(stat.opAttribs.op));
                    attrib.OpStats = stat.opAttribs.op_stats;
                    return attrib.OpValue;
                }
            }
            break;
        }

        return value;
    }

    std::string formatMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx, const ItemStat& stat)
    {
        if (idx >= attrib.Values.size())
        {
            return "";
        }

        auto value = getMagicalAttributeValue(attrib, charLevel, idx, stat);
        std::stringstream ssValue;
        switch (attrib.Id)
        {
            // Class name is index 0
        case 83:
            switch (idx)
            {
            case 0:
                if (value < NUM_OF_CLASSES)
                {
                    ssValue << ClassNames[value];
                }
                else
                {
                    ssValue << std::dec << value;
                }
                break;

            default:
                ssValue << std::dec << value;
                break;
            }
            break;

            // skill name is index 0
        case 97:
            switch (idx)
            {
            case 0:
            {
                auto iter = GenericSkillNames.find(value);
                if (iter != GenericSkillNames.end())
                {
                    ssValue << iter->second;
                }
                else
                {
                    size_t classIdx = (value - START_SKILL_ID) / NUM_OF_SKILLS;
                    if (classIdx < NUM_OF_CLASSES)
                    {
                        ssValue << SkillsNames[std::uint8_t(classIdx)][(value - START_SKILL_ID) % NUM_OF_SKILLS];
                    }
                    else
                    {
                        ssValue << std::dec << value;
                    }
                }
                break;
            }

            default:
                ssValue << std::dec << value;
                break;
            }
            break;

            // Character specific Skill index is index 0
        case 107:
            switch (idx)
            {
            case 0:
            {
                auto iter = GenericSkillNames.find(value);
                if (iter != GenericSkillNames.end())
                {
                    ssValue << iter->second;
                }
                else
                {
                    size_t classIdx = (value - START_SKILL_ID) / NUM_OF_SKILLS;
                    if (classIdx < NUM_OF_CLASSES)
                    {
                        ssValue << SkillsNames[std::uint8_t(classIdx)][(value - START_SKILL_ID) % NUM_OF_SKILLS] << " (" << ClassNames[classIdx] << " Only)";
                    }
                    else
                    {
                        ssValue << std::dec << value;
                    }
                }
                break;
            }

            default:
                ssValue << std::dec << value;
                break;
            }
            break;

            // Skill name is index 0 and class name is index 1
        case 151:
        case 188:
            switch (idx)
            {
            case 0:
                if ((value < NUM_OF_SKILLS) && (attrib.Values[1] < NUM_OF_CLASSES))
                {
                    ssValue << SkillsNames[attrib.Values[1]][value];
                }
                else
                {
                    ssValue << std::dec << value;
                }
                break;

            case 1:
                if (value < NUM_OF_CLASSES)
                {
                    ssValue << ClassNames[value];
                }
                else
                {
                    ssValue << std::dec << value;
                }
                break;

            default:
                ssValue << std::dec << value;
                break;
            }
            break;

            // Skill name is index 1, all others are raw values
        case 195:
        case 196:
        case 197:
        case 198:
        case 199:
        case 201:
        case 204:
            switch (idx)
            {
            case 1:
            {
                auto iter = GenericSkillNames.find(value);
                if (iter != GenericSkillNames.end())
                {
                    ssValue << iter->second;
                }
                else
                {
                    size_t classIdx = (value - START_SKILL_ID) / NUM_OF_SKILLS;
                    if (classIdx < NUM_OF_CLASSES)
                    {
                        ssValue << SkillsNames[std::uint8_t(classIdx)][(value - START_SKILL_ID) % NUM_OF_SKILLS];
                    }
                    else
                    {
                        ssValue << std::dec << value;
                    }
                }
                break;
            }

            default:
                ssValue << std::dec << value;
                break;
            }
            break;

        default:
            ssValue << std::dec << value;
            break;
        }

        return ssValue.str();
    }

    bool formatDisplayedMagicalAttribute(MagicalAttribute& attrib, std::uint32_t charLevel)
    {
        if (attrib.Desc.empty() || attrib.Values.empty())
        {
            return false;
        }

        // Check special cases
        size_t repIdx = 0;
        std::string replaceStr;
        switch (attrib.Id)
        {
            // Check for range having min and max the same value
        case 48:
        case 50:
        case 52:
        case 54:
        case 57:
            if (attrib.Values[0] == attrib.Values[1])
            {
                replaceStr = "Adds {0}-{1}";
                repIdx = attrib.Desc.find(replaceStr);
                if (repIdx != attrib.Desc.npos)
                {
                    attrib.Desc.replace(repIdx, replaceStr.size(), "+{1}");
                }
            }
            break;
        }

        const auto& stat = itemStats[attrib.Id];
        for (size_t idx = 0; idx < attrib.Values.size(); ++idx)
        {
            std::stringstream ssReplace;
            ssReplace << "{";
            ssReplace << idx;
            ssReplace << "}";
            replaceStr = ssReplace.str();

            repIdx = attrib.Desc.find(replaceStr);
            if (repIdx == attrib.Desc.npos)
            {
                continue;
            }

            attrib.Desc.replace(repIdx, replaceStr.size(), formatMagicalAttributeValue(attrib, charLevel, idx, stat));
        }

        return true;
    }

    void combineMagicalAttribute(std::multimap<size_t, size_t>& itemIndexMap, const std::vector<MagicalAttribute>& newAttribs, std::vector<MagicalAttribute>& attribs)
    {
        for (const auto& attrib : newAttribs)
        {
            auto iter = itemIndexMap.lower_bound(attrib.Id);
            auto iterEnd = itemIndexMap.upper_bound(attrib.Id);
            if (iter == iterEnd)
            {
                itemIndexMap.insert(std::make_pair(attrib.Id, attribs.size()));
                attribs.push_back(attrib);
            }
            else
            {
                bool notMatched = true;
                for (; notMatched && iter != iterEnd; ++iter)
                {
                    auto& existing = attribs.at(iter->second);
                    if (existing.Values.empty())
                    {
                        continue;
                    }

                    // Check to see if we are a match to merge
                    switch (attrib.Id)
                    {
                    case 48:
                    case 50:
                    case 52:
                    case 54:
                    case 57:
                        existing.Values[0] += attrib.Values[0];
                        existing.Values[1] += attrib.Values[1];
                        notMatched = false;
                        break;

                    default:
                    {
                        const ItemStat* stat = &itemStats[attrib.Id];
                        bool goodMatch = true;
                        size_t numMatchValue = (stat->encode == 3) ? 2 : 1;
                        for (size_t idx = 0; idx < existing.Values.size() - numMatchValue; ++idx)
                        {
                            if (existing.Values[idx] != attrib.Values[idx])
                            {
                                goodMatch = false;
                                break;
                            }
                        }

                        if (goodMatch)
                        {
                            notMatched = false;
                            for (std::int64_t idx = (std::int64_t)existing.Values.size() - 1; idx >= (std::int64_t)existing.Values.size() - (std::int64_t)numMatchValue; --idx)
                            {
                                existing.Values[idx] += attrib.Values[idx];
                            }
                        }
                        break;
                    }
                    }

                    if (notMatched)
                    {
                        itemIndexMap.insert(std::make_pair(attrib.Id, attribs.size()));
                        attribs.push_back(attrib);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
d2ce::Item::Item()
{
}
//---------------------------------------------------------------------------
d2ce::Item::Item(size_t itemsize) : data(itemsize)
{
}
d2ce::Item::Item(const Item& other)
{
    *this = other;
}
//---------------------------------------------------------------------------
d2ce::Item::~Item()
{
}
//---------------------------------------------------------------------------
d2ce::Item& d2ce::Item::operator=(const Item& other)
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    data = other.data;
    SocketedItems = other.SocketedItems;

    FileVersion = other.FileVersion;
    start_bit_offset = other.start_bit_offset;
    is_potion_bit_offset = other.is_potion_bit_offset;
    location_bit_offset = other.location_bit_offset;
    equipped_id_offset = other.equipped_id_offset;
    position_offset = other.position_offset;
    alt_position_id_offset = other.alt_position_id_offset;
    type_code_offset = other.type_code_offset;
    extended_data_offset = other.extended_data_offset;
    quest_difficulty_offset = other.quest_difficulty_offset;
    nr_of_items_in_sockets_offset = other.nr_of_items_in_sockets_offset;
    nr_of_items_in_sockets_bits = other.nr_of_items_in_sockets_bits;
    item_id_bit_offset = other.item_id_bit_offset;
    item_level_bit_offset = other.item_level_bit_offset;
    quality_bit_offset = other.quality_bit_offset;
    multi_graphic_bit_offset = other.multi_graphic_bit_offset;
    autoAffix_bit_offset = other.autoAffix_bit_offset;
    quality_attrib_bit_offset = other.quality_attrib_bit_offset;
    runeword_id_bit_offset = other.runeword_id_bit_offset;
    personalized_bit_offset = other.personalized_bit_offset;
    realm_bit_offset = other.realm_bit_offset;
    defense_rating_bit_offset = other.defense_rating_bit_offset;
    durability_bit_offset = other.durability_bit_offset;
    stackable_bit_offset = other.stackable_bit_offset;
    gld_stackable_bit_offset = other.gld_stackable_bit_offset;
    socket_count_bit_offset = other.socket_count_bit_offset;
    bonus_bits_bit_offset = other.bonus_bits_bit_offset;
    magical_props_bit_offset = other.magical_props_bit_offset;
    set_bonus_props_bit_offset = other.set_bonus_props_bit_offset;
    runeword_props_bit_offset = other.runeword_props_bit_offset;
    item_end_bit_offset = other.item_end_bit_offset;
    return *this;
}
d2ce::Item& d2ce::Item::operator=(Item&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    data.swap(other.data);
    other.data.clear();
    SocketedItems.swap(other.SocketedItems);
    other.SocketedItems.clear();
    FileVersion = std::exchange(other.FileVersion, APP_CHAR_VERSION);
    start_bit_offset = std::exchange(other.start_bit_offset, 16);
    is_potion_bit_offset = std::exchange(other.is_potion_bit_offset, 36);
    location_bit_offset = std::exchange(other.location_bit_offset, 58);
    equipped_id_offset = std::exchange(other.equipped_id_offset, 61);
    position_offset = std::exchange(other.position_offset, 65);
    alt_position_id_offset = std::exchange(other.alt_position_id_offset, 73);
    type_code_offset = std::exchange(other.type_code_offset, 76);
    extended_data_offset = std::exchange(other.extended_data_offset, 108);
    quest_difficulty_offset = std::exchange(other.quest_difficulty_offset, 0);
    nr_of_items_in_sockets_offset = std::exchange(other.nr_of_items_in_sockets_offset, 0);
    nr_of_items_in_sockets_bits = std::exchange(other.nr_of_items_in_sockets_bits, 3);
    item_id_bit_offset = std::exchange(other.item_id_bit_offset, 0);
    item_level_bit_offset = std::exchange(other.item_level_bit_offset, 0);
    quality_bit_offset = std::exchange(other.quality_bit_offset, 0);
    multi_graphic_bit_offset = std::exchange(other.multi_graphic_bit_offset, 0);
    autoAffix_bit_offset = std::exchange(other.autoAffix_bit_offset, 0);
    quality_attrib_bit_offset = std::exchange(other.quality_attrib_bit_offset, 0);
    runeword_id_bit_offset = std::exchange(other.runeword_id_bit_offset, 0);
    personalized_bit_offset = std::exchange(other.personalized_bit_offset, 0);
    realm_bit_offset = std::exchange(other.realm_bit_offset, 0);
    defense_rating_bit_offset = std::exchange(other.defense_rating_bit_offset, 0);
    durability_bit_offset = std::exchange(other.durability_bit_offset, 0);
    stackable_bit_offset = std::exchange(other.stackable_bit_offset, 0);
    gld_stackable_bit_offset = std::exchange(other.gld_stackable_bit_offset, 0);
    socket_count_bit_offset = std::exchange(other.socket_count_bit_offset, 0);
    bonus_bits_bit_offset = std::exchange(other.bonus_bits_bit_offset, 0);
    magical_props_bit_offset = std::exchange(other.magical_props_bit_offset, 0);
    set_bonus_props_bit_offset = std::exchange(other.set_bonus_props_bit_offset, 0);
    runeword_props_bit_offset = std::exchange(other.runeword_props_bit_offset, 0);
    item_end_bit_offset = std::exchange(other.item_end_bit_offset, 0);
    return *this;
}
//---------------------------------------------------------------------------
void d2ce::Item::swap(Item& other)
{
    std::swap(*this, other);
}
//---------------------------------------------------------------------------
std::uint8_t& d2ce::Item::operator [](size_t position) const
{
    return data[position];
}
//---------------------------------------------------------------------------
// returns the number of bytes not including socketed items
size_t d2ce::Item::size() const
{
    return data.size();
}
//---------------------------------------------------------------------------
// returns the number of bytes to store the item, including socketed items
size_t d2ce::Item::getFullSize() const
{
    size_t byteSize = data.size();
    for (auto& item : SocketedItems)
    {
        byteSize += item.getFullSize();
    }

    return byteSize;
}
//---------------------------------------------------------------------------
// itemsize is the number of bytes not including socketed items
void d2ce::Item::resize(size_t itemsize)
{
    data.resize(itemsize);
}
//---------------------------------------------------------------------------
// itemsize is the number of bytes not including socketed items
void d2ce::Item::reserve(size_t itemsize)
{
    data.reserve(itemsize);
}
//---------------------------------------------------------------------------
void d2ce::Item::clear()
{
    *this = Item();
}
//---------------------------------------------------------------------------
void d2ce::Item::push_back(const std::uint8_t& value)
{
    data.push_back(value);
}
//---------------------------------------------------------------------------
d2ce::EnumItemVersion d2ce::Item::Version() const
{
    std::uint16_t value = getRawVersion();
    if (FileVersion < EnumCharVersion::v115) // pre-1.15 character file
    {
        switch (value)
        {
        case 0: // pre-1.08
            if (FileVersion < EnumCharVersion::v107) // pre-1.07 character file
            {
                if (data.size() == 27)
                {
                    return EnumItemVersion::v100; // v1.00 - v1.03
                }

                return EnumItemVersion::v104;     // v1.04 - v1.06 item
            }

            return EnumItemVersion::v107;         // v1.07 item

        case 1:
        case 100:
            return EnumItemVersion::v108;         // v1.08/1.09 normal or expansion

        case 2:
        case 101:
            return EnumItemVersion::v110;         // v1.10 normal or expansion
        }

        return EnumItemVersion::v110;             // v1.10 normal or expansion
    }

    // TODO: do we do anything with the raw version value?
    return EnumItemVersion::v115;                 // v1.15 Diable II: Resurrected
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getRawVersion() const
{
    return (std::uint16_t)read_uint32_bits(start_bit_offset + 32, (FileVersion < EnumCharVersion::v115 ? 8 : 3));
}
//---------------------------------------------------------------------------
bool d2ce::Item::isIdentified() const
{
    return read_uint32_bits(start_bit_offset + 4, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPotion() const
{
    if (is_potion_bit_offset == 0)
    {
        is_potion_bit_offset = start_bit_offset + 10;
    }

    return read_uint32_bits(is_potion_bit_offset, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isSocketed() const
{
    if (isSimpleItem())
    {
        return false;
    }

    return read_uint32_bits(start_bit_offset + 11, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isNew() const
{
    return read_uint32_bits(start_bit_offset + 13, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isEar() const
{
    return read_uint32_bits(start_bit_offset + 16, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isStarterItem() const
{
    return read_uint32_bits(start_bit_offset + 17, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
/*
   Returns true if this item is a simple item (i.e. 14 byte item)
*/
bool d2ce::Item::isSimpleItem() const
{
    if (FileVersion < EnumCharVersion::v107 && Version() == EnumItemVersion::v100)
    {
        // v1.00 - v1.03
        return true;
    }

    return read_uint32_bits(start_bit_offset + 21, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isEthereal() const
{
    return read_uint32_bits(start_bit_offset + 22, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPersonalized() const
{
    return read_uint32_bits(start_bit_offset + 24, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRuneword() const
{
    return read_uint32_bits(start_bit_offset + 26, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
d2ce::EnumItemLocation d2ce::Item::getLocation() const
{
    uint8_t loc = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        loc = (std::uint8_t)read_uint32_bits(location_bit_offset, 2);
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06
        loc = (std::uint8_t)read_uint32_bits(location_bit_offset, 8);
        if (loc == 0xFF)
        {
            return EnumItemLocation::SOCKET;
        }

        if (loc == 0x7F)
        {
            return EnumItemLocation::BELT;
        }

        return EnumItemLocation::STORED;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II Resurrected
        loc = (std::uint8_t)read_uint32_bits(location_bit_offset, 3);
        break;
    }

    if (loc > static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::SOCKET))
    {
        return EnumItemLocation::STORED;
    }

    return static_cast<EnumItemLocation>(loc);
}
//---------------------------------------------------------------------------
d2ce::EnumEquippedId d2ce::Item::getEquippedId() const
{
    auto value = (std::uint8_t)read_uint32_bits(equipped_id_offset, 4);
    if (value > static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_HAND_LEFT))
    {
        return EnumEquippedId::NONE;
    }

    return static_cast<EnumEquippedId>(value);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPositionX() const
{
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        if (getLocation() == EnumItemLocation::BELT)
        {
            return (std::uint8_t)read_uint32_bits(position_offset, 2);
        }

        return (std::uint8_t)read_uint32_bits(position_offset, 5);

    case EnumItemVersion::v104: // v1.04 - v1.06
        return (std::uint8_t)read_uint32_bits(position_offset, 5);

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        return (std::uint8_t)read_uint32_bits(position_offset, 4);
    }

    return 0;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPositionY() const
{
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        if (getLocation() == EnumItemLocation::BELT)
        {
            return 3 - (std::uint8_t)read_uint32_bits(position_offset + 2, 2);
        }

        return (std::uint8_t)read_uint32_bits(position_offset + 5, 2);

    case EnumItemVersion::v104: // v1.04 - v1.06
        return (std::uint8_t)read_uint32_bits(position_offset + 5, 2);

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        return (std::uint8_t)read_uint32_bits(position_offset + 4, 4);
    }

    return 0;
}
//---------------------------------------------------------------------------
d2ce::EnumAltItemLocation d2ce::Item::getAltPositionId() const
{
    std::uint8_t loc = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        loc = (std::uint8_t)read_uint32_bits(alt_position_id_offset, 4);
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        loc = (std::uint8_t)read_uint32_bits(alt_position_id_offset, 3);
        break;
    }

    if (loc > static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::STASH))
    {
        return EnumAltItemLocation::UKNOWN;
    }

    return static_cast<EnumAltItemLocation>(loc);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getItemCode(std::uint8_t(&strcode)[4]) const
{
    std::memset(strcode, 0, sizeof(strcode));
    if (isEar())
    {
        return false;
    }

    std::uint64_t code = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        code = std::uint64_t(read_uint32_bits(type_code_offset, 10));
        switch (code)
        {
            // Weapon Axe
        case 0x000: // Hand Axe
            strcode[0] = 'h';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x001: // Axe
            strcode[0] = 'a';
            strcode[1] = 'x';
            strcode[2] = 'e';
            strcode[3] = 0x20;
            return true;
        case 0x002: // Double Axe
            strcode[0] = '2';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x003: // Military Pick
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = 'i';
            strcode[3] = 0x20;
            return true;
        case 0x004: // War Axe
            strcode[0] = 'w';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x005: // Large Axe
            strcode[0] = 'l';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x006: // Broad Axe
            strcode[0] = 'b';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x007: // Battle Axe
            strcode[0] = 'b';
            strcode[1] = 't';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x008: // Great Axe
            strcode[0] = 'g';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x009: // Giant Axe
            strcode[0] = 'g';
            strcode[1] = 'i';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x05D: // Hatchet
            strcode[0] = '9';
            strcode[1] = 'h';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x05E: // Cleaver
            strcode[0] = '9';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x05F: // Twin Axe
            strcode[0] = '9';
            strcode[1] = '2';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x060: // Crowbill
            strcode[0] = '9';
            strcode[1] = 'm';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x061: // Naga
            strcode[0] = '9';
            strcode[1] = 'w';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x062: // Military Axe
            strcode[0] = '9';
            strcode[1] = 'l';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x063: // Bearded Axe
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x064: // Tabar
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x065: // Gothic Axe
            strcode[0] = '9';
            strcode[1] = 'g';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x066: // Ancient Axe
            strcode[0] = '9';
            strcode[1] = 'g';
            strcode[2] = 'i';
            strcode[3] = 0x20;
            return true;

            // Weapon Wand
        case 0x00A: // Wand
            strcode[0] = 'w';
            strcode[1] = 'n';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x00B: // Yew Wand
            strcode[0] = 'y';
            strcode[1] = 'w';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x00C: // Bone Wand
            strcode[0] = 'b';
            strcode[1] = 'w';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x00D: // Grim Wand
            strcode[0] = 'g';
            strcode[1] = 'w';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x067: // Burnt Wand
            strcode[0] = '9';
            strcode[1] = 'w';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x068: // Petrified Wand
            strcode[0] = '9';
            strcode[1] = 'y';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x069: // Tomb Wand
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x06A: // Grave Wand
            strcode[0] = '9';
            strcode[1] = 'g';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;

            // Weapon Scepter
        case 0x00F: // Scepter
            strcode[0] = 's';
            strcode[1] = 'c';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x010: // Grand Scepter
            strcode[0] = 'g';
            strcode[1] = 's';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x011: // War Scepter
            strcode[0] = 'w';
            strcode[1] = 's';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x06C: // Rune Scepter
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x06D: // Holy Water Sprinkler
            strcode[0] = '9';
            strcode[1] = 'q';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x06E: // Divine Scepter
            strcode[0] = '9';
            strcode[1] = 'w';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

            // Weapon Mace
        case 0x00E: // Club
            strcode[0] = 'c';
            strcode[1] = 'l';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x012: // Spiked Club
            strcode[0] = 's';
            strcode[1] = 'p';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x013: // Mace
            strcode[0] = 'm';
            strcode[1] = 'a';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x014: // Morning Star
            strcode[0] = 'm';
            strcode[1] = 's';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x015: // Flail
            strcode[0] = 'f';
            strcode[1] = 'l';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x016: // War Hammer
            strcode[0] = 'w';
            strcode[1] = 'h';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x017: // Maul
            strcode[0] = 'm';
            strcode[1] = 'a';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x018: // Great Maul
            strcode[0] = 'g';
            strcode[1] = 'm';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x06B: // Cudgel
            strcode[0] = '9';
            strcode[1] = 'c';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x06F: // Barbed Club
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x070: // Flanged Mace
            strcode[0] = '9';
            strcode[1] = 'm';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x071: // Jagged Star
            strcode[0] = '9';
            strcode[1] = 'm';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x072: // Knout
            strcode[0] = '9';
            strcode[1] = 'f';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x073: // Battle Hammer
            strcode[0] = '9';
            strcode[1] = 'w';
            strcode[2] = 'h';
            strcode[3] = 0x20;
            return true;
        case 0x074: // War Club
            strcode[0] = '9';
            strcode[1] = 'm';
            strcode[2] = '9';
            strcode[3] = 0x20;
            return true;
        case 0x075: // Martel De Fer
            strcode[0] = '9';
            strcode[1] = 'g';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;

            // Weapon Sword
        case 0x019: // Short Sword
            strcode[0] = 's';
            strcode[1] = 's';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x01A: // Scimitar
            strcode[0] = 's';
            strcode[1] = 'c';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x01B: // Sabre
            strcode[0] = 's';
            strcode[1] = 'b';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x01C: // Falchion
            strcode[0] = 'f';
            strcode[1] = 'l';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x01D: // Crystal Sword
            strcode[0] = 'c';
            strcode[1] = 'r';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x01E: // Broad Sword
            strcode[0] = 'b';
            strcode[1] = 's';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x01F: // Long Sword
            strcode[0] = 'l';
            strcode[1] = 's';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x020: // War Sword
            strcode[0] = 'w';
            strcode[1] = 's';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x021: // Two Handed Sword
            strcode[0] = '2';
            strcode[1] = 'h';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x022: // Claymore
            strcode[0] = 'c';
            strcode[1] = 'l';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x023: // Giant Sword
            strcode[0] = 'g';
            strcode[1] = 'i';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x024: // Bastard Sword
            strcode[0] = 'b';
            strcode[1] = 's';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x025: // Flamberge
            strcode[0] = 'f';
            strcode[1] = 'l';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x026: // Great Sword
            strcode[0] = 'g';
            strcode[1] = 's';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x076: // Gladius
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x077: // Cutlass
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x078: // Shamshir
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x079: // Tulwar
            strcode[0] = '9';
            strcode[1] = 'f';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x07A: // Dimensional Blade
            strcode[0] = '9';
            strcode[1] = 'c';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x07B: // Battle Sword
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x07C: // Rune Sword
            strcode[0] = '9';
            strcode[1] = 'l';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x07D: // Ancient Sword
            strcode[0] = '9';
            strcode[1] = 'w';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x07E: // Espandon
            strcode[0] = '9';
            strcode[1] = '2';
            strcode[2] = 'h';
            strcode[3] = 0x20;
            return true;
        case 0x07F: // Dacian Falx
            strcode[0] = '9';
            strcode[1] = 'c';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x080: // Tusk Sword
            strcode[0] = '9';
            strcode[1] = 'g';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x081: // Gothic Sword
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = '9';
            strcode[3] = 0x20;
            return true;
        case 0x082: // Zweihander
            strcode[0] = '9';
            strcode[1] = 'f';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x083: // Executioner Sword
            strcode[0] = '9';
            strcode[1] = 'g';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;

            // Weapon Dagger
        case 0x027: // Dagger
            strcode[0] = 'd';
            strcode[1] = 'g';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x028: // Dirk
            strcode[0] = 'd';
            strcode[1] = 'i';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x029: // Kris
            strcode[0] = 'k';
            strcode[1] = 'r';
            strcode[2] = 'i';
            strcode[3] = 0x20;
            return true;
        case 0x02A: // Blade
            strcode[0] = 'b';
            strcode[1] = 'l';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x084: // Poignard
            strcode[0] = '9';
            strcode[1] = 'd';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x085: // Rondel
            strcode[0] = '9';
            strcode[1] = 'd';
            strcode[2] = 'i';
            strcode[3] = 0x20;
            return true;
        case 0x086: // Ciquedeas
            strcode[0] = '9';
            strcode[1] = 'k';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x087: // Stiletto
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;

            // Weapon Throwing
        case 0x02B: // Throwing Knife
            strcode[0] = 't';
            strcode[1] = 'k';
            strcode[2] = 'f';
            strcode[3] = 0x20;
            return true;
        case 0x02C: // Throwing Axe
            strcode[0] = 't';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x02D: // Balanced Knife
            strcode[0] = 'b';
            strcode[1] = 'k';
            strcode[2] = 'f';
            strcode[3] = 0x20;
            return true;
        case 0x02E: // Balanced Axe
            strcode[0] = 'b';
            strcode[1] = 'a';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x088: // Battle Dart
            strcode[0] = '9';
            strcode[1] = 't';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;
        case 0x089: // Francisca
            strcode[0] = '9';
            strcode[1] = 't';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x08A: // War Dart
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;
        case 0x08B: // Hurlbat
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = '8';
            strcode[3] = 0x20;
            return true;

            // Weapon Javelin
        case 0x02F: // Javelin
            strcode[0] = 'j';
            strcode[1] = 'a';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x030: // Pilum
            strcode[0] = 'p';
            strcode[1] = 'i';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x031: // Short Spear
            strcode[0] = 's';
            strcode[1] = 's';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x032: // Glaive
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x033: // Throwing Spear
            strcode[0] = 't';
            strcode[1] = 's';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x034: // Spear
            strcode[0] = 's';
            strcode[1] = 'p';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x035: // Trident
            strcode[0] = 't';
            strcode[1] = 'r';
            strcode[2] = 'i';
            strcode[3] = 0x20;
            return true;
        case 0x036: // Brandistock
            strcode[0] = 'b';
            strcode[1] = 'r';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x037: // Spetum
            strcode[0] = 's';
            strcode[1] = 'p';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x038: // Pike
            strcode[0] = 'p';
            strcode[1] = 'i';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;
        case 0x08C: // War Javelin
            strcode[0] = '9';
            strcode[1] = 'j';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x08D: // Great Pilum
            strcode[0] = '9';
            strcode[1] = 'p';
            strcode[2] = 'i';
            strcode[3] = 0x20;
            return true;
        case 0x08E: // Simbilan
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = '9';
            strcode[3] = 0x20;
            return true;
        case 0x08F: // Spiculum
            strcode[0] = '9';
            strcode[1] = 'g';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x090: // Harpoon
            strcode[0] = '9';
            strcode[1] = 't';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x091: // War Spear
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x092: // Fuscina
            strcode[0] = '9';
            strcode[1] = 't';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x093: // War Fork
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x094: // Yari
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x095: // Lance
            strcode[0] = '9';
            strcode[1] = 'p';
            strcode[2] = '9';
            strcode[3] = 0x20;
            return true;

            // Weapon Polearm
        case 0x039: // Bardiche
            strcode[0] = 'b';
            strcode[1] = 'a';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x03A: // Voulge
            strcode[0] = 'v';
            strcode[1] = 'o';
            strcode[2] = 'u';
            strcode[3] = 0x20;
            return true;
        case 0x03B: // Scythe
            strcode[0] = 's';
            strcode[1] = 'c';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x03C: // Pole Axe
            strcode[0] = 'p';
            strcode[1] = 'a';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x03D: // Halberd
            strcode[0] = 'h';
            strcode[1] = 'a';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x03E: // War Scythe
            strcode[0] = 'w';
            strcode[1] = 's';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x096: // Lochaber Axe
            strcode[0] = '9';
            strcode[1] = 'b';
            strcode[2] = '7';
            strcode[3] = 0x20;
            return true;
        case 0x097: // Bill
            strcode[0] = '9';
            strcode[1] = 'v';
            strcode[2] = 'o';
            strcode[3] = 0x20;
            return true;
        case 0x098: // Battle Scythe
            strcode[0] = '9';
            strcode[1] = 's';
            strcode[2] = '8';
            strcode[3] = 0x20;
            return true;
        case 0x099: // Partizan
            strcode[0] = '9';
            strcode[1] = 'p';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x09A: // Bec-De-Corbin
            strcode[0] = '9';
            strcode[1] = 'h';
            strcode[2] = '9';
            strcode[3] = 0x20;
            return true;
        case 0x09B: // Grim Scythe
            strcode[0] = '9';
            strcode[1] = 'w';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;

            // Weapon Staff
        case 0x03F: // Short Staff
            strcode[0] = 's';
            strcode[1] = 's';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x040: // Long Staff
            strcode[0] = 'l';
            strcode[1] = 's';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x041: // Gnarled Staff
            strcode[0] = 'c';
            strcode[1] = 's';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x042: // Battle Staff
            strcode[0] = 'b';
            strcode[1] = 's';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x043: // War Staff
            strcode[0] = 'w';
            strcode[1] = 's';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x09C: // Jo Staff
            strcode[0] = '8';
            strcode[1] = 's';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x09D: // Quarterstaff
            strcode[0] = '8';
            strcode[1] = 'l';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x09E: // Cedar Staff
            strcode[0] = '8';
            strcode[1] = 'c';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x09F: // Gothic Staff
            strcode[0] = '8';
            strcode[1] = 'b';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x0A0: // Rune Staff
            strcode[0] = '8';
            strcode[1] = 'w';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

            // Weapon Bow
        case 0x044: // Short Bow
            strcode[0] = 's';
            strcode[1] = 'b';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x045: // Hunter's Bow
            strcode[0] = 'h';
            strcode[1] = 'b';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x046: // Long Bow
            strcode[0] = 'l';
            strcode[1] = 'b';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x047: // Composite Bow
            strcode[0] = 'c';
            strcode[1] = 'b';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x048: // Short Battle Bow
            strcode[0] = 's';
            strcode[1] = 'b';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x049: // Long Battle Bow
            strcode[0] = 'l';
            strcode[1] = 'b';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x04A: // Short War Bow
            strcode[0] = 's';
            strcode[1] = 'w';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x04B: // Long War Bow
            strcode[0] = 'l';
            strcode[1] = 'w';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x0A1: // Edge Bow
            strcode[0] = '8';
            strcode[1] = 's';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x0A2: // Razor Bow
            strcode[0] = '8';
            strcode[1] = 'h';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x0A3: // Cedar Bow
            strcode[0] = '8';
            strcode[1] = 'l';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x0A4: // Double Bow
            strcode[0] = '8';
            strcode[1] = 'c';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x0A5: // Short Siege Bow
            strcode[0] = '8';
            strcode[1] = 's';
            strcode[2] = '8';
            strcode[3] = 0x20;
            return true;
        case 0x0A6: // Long Siege Bow
            strcode[0] = '8';
            strcode[1] = 'l';
            strcode[2] = '8';
            strcode[3] = 0x20;
            return true;
        case 0x0A7: // Rune Bow
            strcode[0] = '8';
            strcode[1] = 's';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x0A8: // Gothic Bow
            strcode[0] = '8';
            strcode[1] = 'l';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;

            // Weapon Crossbow
        case 0x04C: // Light Crossbow
            strcode[0] = 'l';
            strcode[1] = 'x';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x04D: // Crossbow
            strcode[0] = 'm';
            strcode[1] = 'x';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x04E: // Heavy Crossbow
            strcode[0] = 'h';
            strcode[1] = 'x';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x04F: // Repeating Crossbow
            strcode[0] = 'r';
            strcode[1] = 'x';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x0A9: // Arbalest
            strcode[0] = '8';
            strcode[1] = 'l';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x0AA: // Siege Crossbow
            strcode[0] = '8';
            strcode[1] = 'm';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x0AB: // Ballista
            strcode[0] = '8';
            strcode[1] = 'h';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x0AC: // Chu-Ko-Nu
            strcode[0] = '8';
            strcode[1] = 'r';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;

            // Potion Throwing
        case 0x050: // Rancid Gas Potion
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x051: // Oil Potion
            strcode[0] = 'o';
            strcode[1] = 'p';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x052: // Choking Gas Potion
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x053: // Exploding Potion
            strcode[0] = 'o';
            strcode[1] = 'p';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x054: // Strangling Gas Potion
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x055: // Fulminating Potion
            strcode[0] = 'o';
            strcode[1] = 'p';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;

            // Special Weapon
        case 0x056: // Decoy Gidbinn
            strcode[0] = 'd';
            strcode[1] = '3';
            strcode[2] = '3';
            strcode[3] = 0x20;
            return true;
        case 0x057: // The Gidbinn
            strcode[0] = 'g';
            strcode[1] = '3';
            strcode[2] = '3';
            strcode[3] = 0x20;
            return true;
        case 0x058: // Wirt's Leg
            strcode[0] = 'l';
            strcode[1] = 'e';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x059: // Horadric Malus
            strcode[0] = 'h';
            strcode[1] = 'd';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x05A: // Hellforge Hammer
            strcode[0] = 'h';
            strcode[1] = 'f';
            strcode[2] = 'h';
            strcode[3] = 0x20;
            return true;
        case 0x05B: // Horadric Staff
            strcode[0] = 'h';
            strcode[1] = 's';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x05C: // Shaft of the Horadric Staff
            strcode[0] = 'm';
            strcode[1] = 's';
            strcode[2] = 'f';
            strcode[3] = 0x20;
            return true;
        case 0x0AD: // Khalim's Flail
            strcode[0] = 'q';
            strcode[1] = 'f';
            strcode[2] = '1';
            strcode[3] = 0x20;
            return true;
        case 0x0AE: // Khalim's Will
            strcode[0] = 'q';
            strcode[1] = 'f';
            strcode[2] = '2';
            strcode[3] = 0x20;
            return true;

            // Helm
        case 0x0AF: // Cap
            strcode[0] = 'c';
            strcode[1] = 'a';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x0B0: // Skull Cap
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x0B1: // Helm
            strcode[0] = 'h';
            strcode[1] = 'l';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x0B2: // Full Helm
            strcode[0] = 'f';
            strcode[1] = 'h';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0B3: // Great Helm
            strcode[0] = 'g';
            strcode[1] = 'h';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x0B4: // Crown
            strcode[0] = 'c';
            strcode[1] = 'r';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x0B5: // Mask
            strcode[0] = 'm';
            strcode[1] = 's';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;
        case 0x0DA: // Bone Helm
            strcode[0] = 'b';
            strcode[1] = 'h';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x0DD: // War Hat
            strcode[0] = 'x';
            strcode[1] = 'a';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x0DE: // Sallet
            strcode[0] = 'x';
            strcode[1] = 'k';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x0DF: // Casque
            strcode[0] = 'x';
            strcode[1] = 'l';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x0E0: // Basinet
            strcode[0] = 'x';
            strcode[1] = 'h';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0E1: // Winged Helm
            strcode[0] = 'x';
            strcode[1] = 'h';
            strcode[2] = 'm';
            strcode[3] = 0x20;
            return true;
        case 0x0E2: // Grand Crown
            strcode[0] = 'x';
            strcode[1] = 'r';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x0E3: // Death Mask
            strcode[0] = 'x';
            strcode[1] = 's';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;
        case 0x108: // Grim Helm
            strcode[0] = 'x';
            strcode[1] = 'h';
            strcode[2] = '9';
            strcode[3] = 0x20;
            return true;

            // Armor
        case 0x0B6: // Quilted Armor
            strcode[0] = 'q';
            strcode[1] = 'u';
            strcode[2] = 'i';
            strcode[3] = 0x20;
            return true;
        case 0x0B7: // Leather Armor
            strcode[0] = 'l';
            strcode[1] = 'e';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x0B8: // Hard Leather Armor
            strcode[0] = 'h';
            strcode[1] = 'l';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x0B9: // Studded Leather
            strcode[0] = 's';
            strcode[1] = 't';
            strcode[2] = 'u';
            strcode[3] = 0x20;
            return true;
        case 0x0BA: // Ring Mail
            strcode[0] = 'r';
            strcode[1] = 'n';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x0BB: // Scale Mail
            strcode[0] = 's';
            strcode[1] = 'c';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0BC: // Chain Mail
            strcode[0] = 'c';
            strcode[1] = 'h';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x0BD: // Breast Plate
            strcode[0] = 'b';
            strcode[1] = 'r';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x0BE: // Splint Mail
            strcode[0] = 's';
            strcode[1] = 'p';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0BF: // Plate Mail
            strcode[0] = 'p';
            strcode[1] = 'l';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0C0: // Field Plate
            strcode[0] = 'f';
            strcode[1] = 'l';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x0C1: // Gothic Plate
            strcode[0] = 'g';
            strcode[1] = 't';
            strcode[2] = 'h';
            strcode[3] = 0x20;
            return true;
        case 0x0C2: // Full Plate
            strcode[0] = 'f';
            strcode[1] = 'u';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0C3: // Ancient Armor
            strcode[0] = 'a';
            strcode[1] = 'a';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x0C4: // Light Plate
            strcode[0] = 'l';
            strcode[1] = 't';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;
        case 0x0E4: // Ghost Armor
            strcode[0] = 'x';
            strcode[1] = 'u';
            strcode[2] = 'i';
            strcode[3] = 0x20;
            return true;
        case 0x0E5: // Serpent Skin Armor
            strcode[0] = 'x';
            strcode[1] = 'e';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x0E6: // Demonhide Armor
            strcode[0] = 'x';
            strcode[1] = 'l';
            strcode[2] = 'a';
            strcode[3] = 0x20;
            return true;
        case 0x0E7: // Trellised Armor
            strcode[0] = 'x';
            strcode[1] = 't';
            strcode[2] = 'u';
            strcode[3] = 0x20;
            return true;
        case 0x0E8: // Linked Mail
            strcode[0] = 'x';
            strcode[1] = 'n';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x0E9: // Tigulated Mail
            strcode[0] = 'x';
            strcode[1] = 'c';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0EA: // Mesh Armor
            strcode[0] = 'x';
            strcode[1] = 'h';
            strcode[2] = 'n';
            strcode[3] = 0x20;
            return true;
        case 0x0EB: // Cuirass
            strcode[0] = 'x';
            strcode[1] = 'r';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x0EC: // Russet Armor
            strcode[0] = 'x';
            strcode[1] = 'p';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0ED: // Templar Coat
            strcode[0] = 'x';
            strcode[1] = 'l';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0EE: // Sharktooth Armor
            strcode[0] = 'x';
            strcode[1] = 'l';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;
        case 0x0EF: // Embossed Plate
            strcode[0] = 'x';
            strcode[1] = 't';
            strcode[2] = 'h';
            strcode[3] = 0x20;
            return true;
        case 0x0F0: // Chaos Armor
            strcode[0] = 'x';
            strcode[1] = 'u';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0F1: // Ornate Plate
            strcode[0] = 'x';
            strcode[1] = 'a';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x0F2: // Mage Plate
            strcode[0] = 'x';
            strcode[1] = 't';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;

            // Shield
        case 0x0C5: // Buckler
            strcode[0] = 'b';
            strcode[1] = 'u';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x0C6: // Small Shield
            strcode[0] = 's';
            strcode[1] = 'm';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0C7: // Large Shield
            strcode[0] = 'l';
            strcode[1] = 'r';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x0C8: // Kite Shield
            strcode[0] = 'k';
            strcode[1] = 'i';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0C9: // Tower Shield
            strcode[0] = 't';
            strcode[1] = 'o';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x0CA: // Gothic Shield
            strcode[0] = 'g';
            strcode[1] = 't';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x0DB: // Bone Shield
            strcode[0] = 'b';
            strcode[1] = 's';
            strcode[2] = 'h';
            strcode[3] = 0x20;
            return true;
        case 0x0DC: // Spiked Shield
            strcode[0] = 's';
            strcode[1] = 'p';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;
        case 0x0F3: // Defender
            strcode[0] = 'x';
            strcode[1] = 'u';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x0F4: // Round Shield
            strcode[0] = 'x';
            strcode[1] = 'm';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0F5: // Scutum
            strcode[0] = 'x';
            strcode[1] = 'r';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x0F6: // Dragon Shield
            strcode[0] = 'x';
            strcode[1] = 'i';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0F7: // Pavise
            strcode[0] = 'x';
            strcode[1] = 'o';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x0F8: // Ancient Shield
            strcode[0] = 'x';
            strcode[1] = 't';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x109: // Grim Shield
            strcode[0] = 'x';
            strcode[1] = 's';
            strcode[2] = 'h';
            strcode[3] = 0x20;
            return true;
        case 0x10A: // Barbed Shield
            strcode[0] = 'x';
            strcode[1] = 'p';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;

            // Gloves
        case 0x0CB: // Leather Gloves
            strcode[0] = 'l';
            strcode[1] = 'g';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0CC: // Heavy Gloves
            strcode[0] = 'v';
            strcode[1] = 'g';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0CD: // Chain Glove
            strcode[0] = 'm';
            strcode[1] = 'g';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0CE: // Light Gauntlet
            strcode[0] = 't';
            strcode[1] = 'g';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0CF: // Gauntlets
            strcode[0] = 'h';
            strcode[1] = 'g';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0F9: // Demonhide Gloves
            strcode[0] = 'x';
            strcode[1] = 'l';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x0FA: // Sharkskin Gloves
            strcode[0] = 'x';
            strcode[1] = 'v';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x0FB: // Heavy Bracers
            strcode[0] = 'x';
            strcode[1] = 'm';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x0FC: // Battle Gauntlets
            strcode[0] = 'x';
            strcode[1] = 't';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x0FD: // War Gauntlet
            strcode[0] = 'x';
            strcode[1] = 'h';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;

            // Boots
        case 0x0D0: // Boots
            strcode[0] = 'l';
            strcode[1] = 'b';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0D1: // Heavy Boots
            strcode[0] = 'v';
            strcode[1] = 'b';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0D2: // Chain Boots
            strcode[0] = 'm';
            strcode[1] = 'b';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0D3: // Light Plate
            strcode[0] = 't';
            strcode[1] = 'b';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0D4: // Greaves
            strcode[0] = 'h';
            strcode[1] = 'b';
            strcode[2] = 't';
            strcode[3] = 0x20;
            return true;
        case 0x0FE: // Demonhide Boots
            strcode[0] = 'x';
            strcode[1] = 'l';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x0FF: // Sharkskin Boots
            strcode[0] = 'x';
            strcode[1] = 'v';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x100: // Mesh Boots
            strcode[0] = 'x';
            strcode[1] = 'm';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x101: // Battle Boots
            strcode[0] = 'x';
            strcode[1] = 't';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x102: // War Boots
            strcode[0] = 'x';
            strcode[1] = 'h';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;

            // Belt
        case 0x0D5: // Sash
            strcode[0] = 'l';
            strcode[1] = 'b';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0D6: // Light Belt
            strcode[0] = 'v';
            strcode[1] = 'b';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0D7: // Belt
            strcode[0] = 'm';
            strcode[1] = 'b';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0D8: // Heavy Belt
            strcode[0] = 't';
            strcode[1] = 'b';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x0D9: // Plated Belt
            strcode[0] = 'h';
            strcode[1] = 'b';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;
        case 0x103: // Demonhide Sash
            strcode[0] = 'z';
            strcode[1] = 'l';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x104: // Sharkskin Belt
            strcode[0] = 'z';
            strcode[1] = 'v';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x105: // Mesh Belt
            strcode[0] = 'z';
            strcode[1] = 'm';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x106: // Battle Belt
            strcode[0] = 'z';
            strcode[1] = 't';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x107: // War Belt
            strcode[0] = 'z';
            strcode[1] = 'h';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;

        case 0x10B: // Elixer of Vitality
            strcode[0] = 'e';
            strcode[1] = 'l';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;

        case 0x110: // Stamina Potion
            strcode[0] = 'v';
            strcode[1] = 'p';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

        case 0x111: // Antidote Potion
            strcode[0] = 'y';
            strcode[1] = 'p';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

        case 0x112: // regular rejuvenation potion
            strcode[0] = 'r';
            strcode[1] = 'v';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

        case 0x113: // full rejuvenation potion
            strcode[0] = 'r';
            strcode[1] = 'v';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            return true;

        case 0x114: // thawing potion
            strcode[0] = 'w';
            strcode[1] = 'm';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;

        case 0x115: // Tome of Town Portal
            strcode[0] = 't';
            strcode[1] = 'b';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;
        case 0x116: // Tome of Identify
            strcode[0] = 't';
            strcode[1] = 'b';
            strcode[2] = 'k';
            strcode[3] = 0x20;
            return true;

        case 0x117: // Amulet
            strcode[0] = 'a';
            strcode[1] = 'm';
            strcode[2] = 'u';
            strcode[3] = 0x20;
            return true;

        case 0x118: // Viper Amulet
            strcode[0] = 'v';
            strcode[1] = 'i';
            strcode[2] = 'p';
            strcode[3] = 0x20;
            return true;

        case 0x119: // Ring
            strcode[0] = 'r';
            strcode[1] = 'i';
            strcode[2] = 'n';
            strcode[3] = 0x20;

        case 0x11B: // Scroll of Inifuss
            strcode[0] = 'b';
            strcode[1] = 'k';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x11C: // Key to the Cairn Stones
            strcode[0] = 'b';
            strcode[1] = 'k';
            strcode[2] = 'd';
            strcode[3] = 0x20;
            return true;

        case 0x11D: // Arrows
            strcode[0] = 'a';
            strcode[1] = 'q';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x11E: // Torch
            strcode[0] = 't';
            strcode[1] = 'c';
            strcode[2] = 'h';
            strcode[3] = 0x20;
            return true;
        case 0x11F: // Bolts
            strcode[0] = 'c';
            strcode[1] = 'q';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;

        case 0x120: // Scroll of Town Portal
            strcode[0] = 't';
            strcode[1] = 's';
            strcode[2] = 'c';
            strcode[3] = 0x20;
        case 0x121: // Scroll of Identify
            strcode[0] = 'i';
            strcode[1] = 's';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;

        case 0x12E: // Key
            strcode[0] = 'k';
            strcode[1] = 'e';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;

        case 0x130: // Potion of Life
            strcode[0] = 'x';
            strcode[1] = 'y';
            strcode[2] = 'z';
            strcode[3] = 0x20;
            return true;
        case 0x131: // Jade Figurine
            strcode[0] = 'j';
            strcode[1] = '3';
            strcode[2] = '4';
            strcode[3] = 0x20;
            return true;
        case 0x132: // The Golden Bird
            strcode[0] = 'g';
            strcode[1] = '3';
            strcode[2] = '4';
            strcode[3] = 0x20;
            return true;
        case 0x133: // Lam Esen's Tome
            strcode[0] = 'b';
            strcode[1] = 'b';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x134: // Horadric Cube
            strcode[0] = 'b';
            strcode[1] = '0';
            strcode[2] = 'x';
            strcode[3] = 0x20;
            return true;
        case 0x135: // Horadric Scroll
            strcode[0] = 't';
            strcode[1] = 'r';
            strcode[2] = '1';
            strcode[3] = 0x20;
            return true;
        case 0x136: // Mephisto's Soulstone
            strcode[0] = 'm';
            strcode[1] = 's';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x137: // Book of Skill
            strcode[0] = 'a';
            strcode[1] = 's';
            strcode[2] = 's';
            strcode[3] = 0x20;
            return true;
        case 0x138: // Khalim's Eye
            strcode[0] = 'q';
            strcode[1] = 'e';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x139: // Khalim's Heart
            strcode[0] = 'q';
            strcode[1] = 'h';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x13A: // Khalim's Brain
            strcode[0] = 'q';
            strcode[1] = 'b';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x13B: // Ear
            strcode[0] = 'e';
            strcode[1] = 'a';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;

        case 0x13C: // chipped amethyst
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x13D: // flawed amethyst
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x113E: // regular amethyst
            strcode[0] = 'g';
            strcode[1] = 's';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;
        case 0x113F: // flawless amethyst
            strcode[0] = 'g';
            strcode[1] = 'z';
            strcode[2] = 'v';
            return true;
        case 0x140: // perfect amethyst
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'v';
            strcode[3] = 0x20;
            return true;

        case 0x141: // chipped topaz
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x142: // flawed topaz
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x143: // regular topaz
            strcode[0] = 'g';
            strcode[1] = 's';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x144: // flawless topaz
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;
        case 0x145: // perfect topaz
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'y';
            strcode[3] = 0x20;
            return true;

        case 0x146: // chipped sapphire
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x147: // flawed sapphire
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x148: // regular sapphire
            strcode[0] = 'g';
            strcode[1] = 's';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;
        case 0x149: // flawless sapphire
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            break;
        case 0x114A: // perfect sapphire
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'b';
            strcode[3] = 0x20;
            return true;

        case 0x114B: // chipped emerald
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            break;
        case 0x114C: // flawed emerald
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x114D: // regular emerald
            strcode[0] = 'g';
            strcode[1] = 's';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x114E: // flawless emerald
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;
        case 0x114F: // perfect emerald
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'g';
            strcode[3] = 0x20;
            return true;

        case 0x150: // chipped ruby
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;

        case 0x151: // flawed ruby
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x152: // regular ruby
            strcode[0] = 'g';
            strcode[1] = 's';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x153: // flawless ruby
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;
        case 0x154: // perfect ruby
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'r';
            strcode[3] = 0x20;
            return true;

        case 0x155: // chipped diamond
            strcode[0] = 'g';
            strcode[1] = 'c';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x156: // flawed diamond
            strcode[0] = 'g';
            strcode[1] = 'f';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x157: // regular diamond
            strcode[0] = 'g';
            strcode[1] = 's';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x158: // flawless diamond
            strcode[0] = 'g';
            strcode[1] = 'l';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;
        case 0x159: // perfect diamond
            strcode[0] = 'g';
            strcode[1] = 'p';
            strcode[2] = 'w';
            strcode[3] = 0x20;
            return true;

        case 0x115A: // minor healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '1';
            strcode[3] = 0x20;
            return true;
        case 0x115B: // light healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '2';
            strcode[3] = 0x20;
            break;
        case 0x115C: // regular healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '3';
            strcode[3] = 0x20;
            return true;
        case 0x115D: // Greater healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '4';
            strcode[3] = 0x20;
            return true;
        case 0x115E: // Super healing potion
            strcode[0] = 'h';
            strcode[1] = 'p';
            strcode[2] = '5';
            return true;

        case 0x115F: // minor mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '1';
            strcode[3] = 0x20;
            break;
        case 0x160: // light mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '2';
            strcode[3] = 0x20;
            return true;
        case 0x161: // regular mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '3';
            strcode[3] = 0x20;
            return true;
        case 0x162: // Greater mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '4';
            strcode[3] = 0x20;
            return true;
        case 0x163: // Super mana potion
            strcode[0] = 'm';
            strcode[1] = 'p';
            strcode[2] = '5';
            strcode[3] = 0x20;
            return true;

        case 0x164: // chipped skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'c';
            strcode[3] = 0x20;
            return true;
        case 0x165: // flawed skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'f';
            strcode[3] = 0x20;
            return true;
        case 0x166: // regular skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'u';
            strcode[3] = 0x20;
            return true;
        case 0x167: // flawless skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'l';
            strcode[3] = 0x20;
            break;
        case 0x168: // perfect skull
            strcode[0] = 's';
            strcode[1] = 'k';
            strcode[2] = 'z';
            strcode[3] = 0x20;
            return true;
        }  // end switch
        return false;

    case EnumItemVersion::v104: // v1.04 - v1.06
        strcode[0] = std::uint8_t(read_uint32_bits(type_code_offset, 8));
        strcode[1] = std::uint8_t(read_uint32_bits(type_code_offset + 8, 8));
        strcode[2] = std::uint8_t(read_uint32_bits(type_code_offset + 16, 8));
        strcode[3] = std::uint8_t(read_uint32_bits(type_code_offset + 24, 6));
        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
        strcode[0] = std::uint8_t(read_uint32_bits(type_code_offset, 8));
        strcode[1] = std::uint8_t(read_uint32_bits(type_code_offset + 8, 8));
        strcode[2] = std::uint8_t(read_uint32_bits(type_code_offset + 16, 8));
        strcode[3] = std::uint8_t(read_uint32_bits(type_code_offset + 24, 8));
        return true;

    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        getItemCodev115(data, type_code_offset, strcode);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
d2ce::EnumItemType d2ce::Item::getItemType() const
{
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        return getEnumItemTypeFromCode(strcode);
    }

    return EnumItemType::Other;

}
//---------------------------------------------------------------------------
std::string d2ce::Item::getItemTypeName() const
{
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        return result.name;
    }

    return "";
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateGem(const std::uint8_t(&newgem)[4])
{
    if (isEar())
    {
        return false;
    }

    // Check for compatible location for update
    bool isPotion = false;
    const std::uint8_t& gem = newgem[0];
    const std::uint8_t& gemCondition = newgem[1];
    const std::uint8_t& gemColour = newgem[2];
    switch (gem)
    {
    case 'g': // gem
        if (getLocation() == EnumItemLocation::BELT)
        {
            // can't put gem in belt
            return false;
        }
        break;

    case 's': // skull
        if (gemCondition == 'k')
        {
            if (getLocation() == EnumItemLocation::BELT)
            {
                // can't put skull in belt
                return false;
            }
        }
        break;

    case 'h': // healing
    case 'm': // mana
        if (gemCondition == 'p')
        {
            isPotion = true;
            if (getLocation() == EnumItemLocation::SOCKET)
            {
                // can't put potion in socket
                return false;
            }
        }
        break;

    case 'r': // rejuvenation potion
        if (gemCondition == 'v')
        {
            isPotion = true;
            if (getLocation() == EnumItemLocation::SOCKET)
            {
                // can't put potion in socket
                return false;
            }
        }
        break;

    case 'y': // antidote potion
    case 'v': //stamina potion
        if (gemCondition == 'p' && gemColour == 's')
        {
            // these guys are potion but not marked as such
            if (getLocation() == EnumItemLocation::SOCKET)
            {
                // can't put potion in socket
                return false;
            }
        }
        break;

    case 'w': // thawing potion
        if (gemCondition == 'm' && gemColour == 's')
        {
            // this guy is a potion but not marked as such
            if (getLocation() == EnumItemLocation::SOCKET)
            {
                // can't put potion in socket
                return false;
            }
        }
        break;
    }

    std::uint8_t gemValue = 0;
    std::uint64_t code = 0;
    std::uint8_t numBitsSet = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        switch (gem)
        {
        case 'g': // gem
            switch (gemColour)
            {
            case 'v': // amethyst
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x3C;
                    break;
                case 'f': // flawed
                    gemValue = 0x3D;
                    break;
                case 'u': // regular
                    gemValue = 0x3E;
                    break;
                case 'z': // flawless
                    gemValue = 0x3F;
                    break;
                default: // perfect
                    gemValue = 0x40;
                    break;
                }
                break;
            case 'y': // topaz
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x41;
                    break;
                case 'f': // flawed
                    gemValue = 0x42;
                    break;
                case 'u': // regular
                    gemValue = 0x43;
                    break;
                case 'l': // flawless
                    gemValue = 0x44;
                    break;
                default: // perfect
                    gemValue = 0x45;
                    break;
                }
                break;
            case 'b': // sapphire
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x46;
                    break;
                case 'f': // flawed
                    gemValue = 0x47;
                    break;
                case 'u': // regular
                    gemValue = 0x48;
                    break;
                case 'l': // flawless
                    gemValue = 0x49;
                    break;
                default: // perfect
                    gemValue = 0x4A;
                    break;
                }
                break;
            case 'g': // emerald
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x4B;
                    break;
                case 'f': // flawed
                    gemValue = 0x4C;
                    break;
                case 'u': // regular
                    gemValue = 0x4D;
                    break;
                case 'l': // flawless
                    gemValue = 0x4E;
                    break;
                default: // perfect
                    gemValue = 0x4F;
                    break;
                }
                break;
            case 'r': // ruby
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x50;
                    break;
                case 'f': // flawed
                    gemValue = 0x51;
                    break;
                case 'u': // regular
                    gemValue = 0x52;
                    break;
                case 'l': // flawless
                    gemValue = 0x53;
                    break;
                default: // perfect
                    gemValue = 0x54;
                    break;
                }
                break;
            case 'w': // diamond
                switch (gemCondition)
                {
                case 'c': // chipped
                    gemValue = 0x55;
                    break;
                case 'f': // flawed
                    gemValue = 0x56;
                    break;
                case 'u': // regular
                    gemValue = 0x57;
                    break;
                case 'l': // flawless
                    gemValue = 0x58;
                    break;
                default: // perfect
                    gemValue = 0x59;
                    break;
                }
                break;
            }
            break;

        case 's': // skull
            if (gemCondition == 'k')
            {
                switch (gemColour)
                {
                case 'c': // chipped
                    gemValue = 0x64;
                    break;
                case 'f': // flawed
                    gemValue = 0x65;
                    break;
                case 'u': // regular
                    gemValue = 0x66;
                    break;
                case 'l': // flawless
                    gemValue = 0x67;
                    break;
                default: // perfect
                    gemValue = 0x68;
                    break;
                }
            }
            break;

        case 'h': // healing
            if (gemCondition == 'p')
            {
                switch (gemColour)
                {
                case '1': // Minor
                    gemValue = 0x5A;
                    break;
                case '2': // Light
                    gemValue = 0x5B;
                    break;
                case '3': // Regular
                    gemValue = 0x5C;
                    break;
                case '4': // Greater
                    gemValue = 0x5D;
                    break;
                default: // Super
                    gemValue = 0x5E;
                    break;
                }
            }
            break;

        case 'm': // mana
            if (gemCondition == 'p')
            {
                switch (gemColour)
                {
                case '1': // Minor
                    gemValue = 0x5F;
                    break;
                case '2': // Light
                    gemValue = 0x60;
                    break;
                case '3': // Regular
                    gemValue = 0x61;
                    break;
                case '4': // Greater
                    gemValue = 0x62;
                    break;
                default: // Super
                    gemValue = 0x63;
                    break;
                }
            }
            break;

        case 'r': // rejuvenation potion
            if (gemCondition == 'v')
            {
                switch (gemColour)
                {
                case 's': // Regular
                    gemValue = 0x12;
                    break;
                default: // Full
                    gemValue = 0x13;
                    break;
                }
            }
            break;

        case 'y': // antidote potion
            if (gemCondition == 'p' && gemColour == 's')
            {
                gemValue = 0x11;
            }
            break;

        case 'v': //stamina potion
            if (gemCondition == 'p' && gemColour == 's')
            {
                gemValue = 0x10;
            }
            break;

        case 'w': // thawing potion
            if (gemCondition == 'm' && gemColour == 's')
            {
                gemValue = 0x14;
            }
            break;
        }

        if (gemValue != 0)
        {
            data[8] |= (gemValue << 4);
            data[9] |= (gemValue >> 4 & 0x0F) | 0x10;
            return true;
        }

        return false;

    case EnumItemVersion::v104: // v1.04 - v1.06
        code = *((std::uint32_t*)newgem);
        updateBits64(type_code_offset, 30, code);
        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
        code = *((std::uint32_t*)newgem);
        updateBits64(type_code_offset, 32, code);
        if (isPotion)
        {
            set_bit(is_potion_bit_offset);
        }
        else
        {
            clear_bit(is_potion_bit_offset);
        }
        return true;

    case EnumItemVersion::v115: // v1.15 normal or expansion
        encodeItemCodev115(newgem, code, numBitsSet);
        updateItemCodev115(code, numBitsSet);
        if (isPotion)
        {
            set_bit(is_potion_bit_offset);
        }
        else
        {
            clear_bit(is_potion_bit_offset);
        }
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getQuestDifficulty() const
{
    if (quest_difficulty_offset == 0)
    {
        return 0;
    }

    return (std::uint8_t)read_uint32_bits(quest_difficulty_offset, 2);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::socketedItemCount() const
{
    if (nr_of_items_in_sockets_offset == 0)
    {
        nr_of_items_in_sockets_offset = extended_data_offset;
        nr_of_items_in_sockets_bits = isSimpleItem() ? 1 : 3;
    }

    return (std::uint8_t)read_uint32_bits(nr_of_items_in_sockets_offset, nr_of_items_in_sockets_bits);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getEarAttributes(d2ce::EarAttributes& attrib) const
{
    attrib.clear();
    if (!isEar())
    {
        return false;
    }

    size_t currentOffset = type_code_offset;
    attrib.Class = static_cast<EnumCharClass>(read_uint32_bits(currentOffset, 3));
    currentOffset += 3;
    attrib.Level = std::uint32_t(read_uint32_bits(currentOffset, 7));
    currentOffset += 7;

    // up to 15 7 bit characters
    std::memset(attrib.Name, 0, sizeof(attrib.Name));
    char c = 0;
    for (std::uint8_t idx = 0; idx < 15; ++idx)
    {
        c = (char)read_uint32_bits(currentOffset, 7);
        if (c == 0)
        {
            break;
        }
        attrib.Name[idx] = c;
        currentOffset += 7;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRequirements(ItemRequirements& req) const
{
    req.clear();
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        req = result.req;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getCategories(std::vector<std::string>& categories) const
{
    categories.clear();
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        categories = result.categories;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDimensions(ItemDimensions& dimensions) const
{
    dimensions.clear();
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        dimensions = result.dimensions;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::getTotalItemSlots() const
{
    ItemDimensions dimensions;
    if (!getDimensions(dimensions))
    {
        return 0;
    }

    return dimensions.InvHeight * dimensions.InvWidth;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getInvFile() const
{
    if (isEar())
    {
        return "invear";
    }

    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        return result.inv_file;
    }

    return "";
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::getId() const
{
    if (isSimpleItem())
    {
        return 0;
    }

    if (item_id_bit_offset == 0)
    {
        item_id_bit_offset = start_bit_offset + 95;
    }

    return (std::uint32_t)read_uint64_bits(item_id_bit_offset, 32);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getLevel() const
{
    if (isSimpleItem())
    {
        return 0;
    }

    if (item_level_bit_offset == 0)
    {
        item_level_bit_offset = start_bit_offset + 127;
    }

    return (std::uint8_t)read_uint32_bits(item_level_bit_offset, 7);
}
//---------------------------------------------------------------------------
d2ce::EnumItemQuality d2ce::Item::getQuality() const
{
    if (isSimpleItem())
    {
        return EnumItemQuality::UKNOWN;
    }

    if (quality_bit_offset == 0)
    {
        quality_bit_offset = start_bit_offset + 134;
    }

    auto value = read_uint32_bits(quality_bit_offset, 4);
    if (value > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
    {
        return EnumItemQuality::UKNOWN;
    }

    return static_cast<EnumItemQuality>(value);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAffixes(MagicalAffixes& affixes) const
{
    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }

    affixes.clear();
    switch (getQuality())
    {
    case EnumItemQuality::MAGIC:
        break;

    default:
        return false;
    }

    size_t current_bit_offset = quality_attrib_bit_offset; // must copy value as readPropertyList will modify value
    affixes.PrefixId = (std::uint16_t)read_uint32_bits(current_bit_offset, 11);
    current_bit_offset += 11;
    affixes.PrefixName = getMagicalPrefixFromId(affixes.PrefixId);

    affixes.SuffixId = (std::uint16_t)read_uint32_bits(current_bit_offset, 11);
    current_bit_offset += 11;
    affixes.SuffixName = getMagicalSuffixFromId(affixes.SuffixId);

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRunewordAttributes(d2ce::RunewordAttributes& attrib) const
{
    attrib.clear();
    if (!isRuneword() || (runeword_id_bit_offset == 0))
    {
        return false;
    }

    attrib.Id = (std::uint16_t)read_uint32_bits(runeword_id_bit_offset, 12);
    attrib.Name = getRunewordNameFromId(attrib.Id);
    size_t current_bit_offset = runeword_props_bit_offset; // must copy value as readPropertyList will modify value
    if (!readPropertyList(current_bit_offset, attrib.MagicalAttributes))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getPersonalizedName() const
{
    if (!isPersonalized() || (personalized_bit_offset == 0))
    {
        return std::string();
    }

    // up to 15 7 bit characters
    size_t currentOffset = personalized_bit_offset;
    char name[NAME_LENGTH];
    std::memset(name, 0, sizeof(name));
    char c = 0;
    for (std::uint8_t idx = 0; idx < 15; ++idx)
    {
        c = (char)read_uint32_bits(currentOffset, 7);
        if (c == 0)
        {
            break;
        }
        name[idx] = c;
        currentOffset += 7;
    }

    return name;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getSetAttributes(SetAttributes& attrib) const
{
    attrib.clear();
    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::SET:
        break;

    default:
        return false;
    }

    attrib.Id = (std::uint16_t)read_uint32_bits(quality_attrib_bit_offset, 5);
    attrib.Name = getSetNameFromId(attrib.Id);
    if (bonus_bits_bit_offset == 0)
    {
        return false;
    }

    std::vector<MagicalAttribute> setAttribs;
    std::uint16_t setBonusBits = (std::uint16_t)read_uint32_bits(bonus_bits_bit_offset, 5);
    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        size_t current_bit_offset = set_bonus_props_bit_offset; // must copy value as readPropertyList will modify value
        for (size_t i = 0; i < 5 && setBonusBits > 0; ++i, setBonusBits >>= 1)
        {
            if ((setBonusBits & 0x01) != 0)
            {
                if (!readPropertyList(current_bit_offset, setAttribs))
                {
                    return false;
                }

                attrib.SetAttributes.emplace_back(setAttribs);
                setAttribs.clear();
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRareOrCraftedAttributes(RareAttributes& attrib) const
{
    attrib.clear();
    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        break;

    default:
        return false;
    }

    size_t current_bit_offset = quality_attrib_bit_offset; // must copy value as readPropertyList will modify value
    attrib.Id = (std::uint16_t)read_uint32_bits(current_bit_offset, 8);
    current_bit_offset += 8;
    attrib.Name = getRareNameFromId(attrib.Id);

    attrib.Id2 = (std::uint16_t)read_uint32_bits(current_bit_offset, 8);
    current_bit_offset += 8;
    attrib.Name2 = getRareNameFromId(attrib.Id2);

    // Following the name IDs, we got 6 possible magical affixes, the pattern
    // is 1 bit id, 11 bit value... But the value will only exist if the prefix
    // is 1. So we'll read the id first and check it against 1.
    std::uint8_t prefix = 0;
    for (size_t i = 3; i > 0; --i)
    {
        MagicalAffixes affixes;
        prefix = (std::uint8_t)read_uint32_bits(current_bit_offset, 1);
        current_bit_offset += 1;
        if (prefix != 0)
        {
            affixes.PrefixId = (std::uint16_t)read_uint32_bits(current_bit_offset, 11);
            current_bit_offset += 11;
            affixes.PrefixName = getMagicalPrefixFromId(affixes.PrefixId);
        }

        prefix = (std::uint8_t)read_uint32_bits(current_bit_offset, 1);
        current_bit_offset += 1;
        if (prefix != 0)
        {
            affixes.SuffixId = (std::uint16_t)read_uint32_bits(current_bit_offset, 11);
            current_bit_offset += 11;
            affixes.SuffixName = getMagicalSuffixFromId(affixes.SuffixId);
        }

        attrib.Affixes.emplace_back(affixes);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getUniqueAttributes(UniqueAttributes& attrib) const
{
    attrib.clear();
    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::UNIQUE:
        break;

    default:
        return false;
    }

    size_t current_bit_offset = quality_attrib_bit_offset; // must copy value as readPropertyList will modify value
    attrib.Id = (std::uint16_t)read_uint32_bits(current_bit_offset, 12);
    current_bit_offset += 12;
    attrib.Name = getUniqueNameFromId(attrib.Id);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAttributes(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();
    if (isSimpleItem() || (magical_props_bit_offset == 0))
    {
        if (!socketedMagicalAttributes.empty())
        {
            attribs.insert(attribs.end(), socketedMagicalAttributes.begin(), socketedMagicalAttributes.end());
            return true;
        }

        return false;
    }

    size_t current_bit_offset = magical_props_bit_offset;
    if (!readPropertyList(current_bit_offset, attribs))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();

    std::vector<MagicalAttribute> tempAttribs;
    if (!getMagicalAttributes(tempAttribs))
    {
        return false;
    }

    std::multimap<size_t, size_t> itemIndexMap;
    combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);

    RunewordAttributes runeAttrib;
    if (getRunewordAttributes(runeAttrib))
    {
        combineMagicalAttribute(itemIndexMap, runeAttrib.MagicalAttributes, attribs);
    }

    for (const auto& item : SocketedItems)
    {
        if (item.getMagicalAttributes(tempAttribs))
        {
            combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::hasMultipleGraphics() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (multi_graphic_bit_offset == 0)
    {
        multi_graphic_bit_offset = start_bit_offset + 138;
    }

    return read_uint32_bits(multi_graphic_bit_offset, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPictureId() const
{
    if (!hasMultipleGraphics())
    {
        return 0;
    }

    return (std::uint8_t)read_uint32_bits(multi_graphic_bit_offset + 1, 3);
}
//---------------------------------------------------------------------------
bool d2ce::Item::isAutoAffix() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (autoAffix_bit_offset == 0)
    {
        autoAffix_bit_offset = start_bit_offset + 139;
        if (hasMultipleGraphics())
        {
            autoAffix_bit_offset += 3;
        }
    }

    return read_uint32_bits(autoAffix_bit_offset, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getInferiorQualityId() const
{
    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::INFERIOR:
        break;

    default:
        return 0;
    }

    return (std::uint8_t)read_uint32_bits(quality_attrib_bit_offset, 3);
}
//---------------------------------------------------------------------------
bool d2ce::Item::isArmor() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    return (base & 4) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    return (base & 2) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isTome() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    return (base & 8) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isStackable() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        base = getItemBase(strcode);
    }

    return (base & 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRune() const
{
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        return result.isRune();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isCharm() const
{
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        return result.isCharm();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isBelt() const
{
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        return result.isBelt();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isHoradricCube() const
{
    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        if (strcode[0] == 'b' && strcode[1] == 'o' && strcode[2] == 'x')
        {
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::totalNumberOfSockets() const
{
    if (isSimpleItem() || (socket_count_bit_offset == 0))
    {
        return 0;
    }

    return (std::uint8_t)read_uint32_bits(socket_count_bit_offset, 4);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getQuantity() const
{
    if (!isStackable() || (stackable_bit_offset == 0 && gld_stackable_bit_offset == 0))
    {
        return 0;
    }

    if (gld_stackable_bit_offset != 0)
    {
        return (std::uint8_t)read_uint32_bits(gld_stackable_bit_offset, 11);
    }

    return (std::uint8_t)read_uint32_bits(stackable_bit_offset, 9);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setQuantity(std::uint16_t quantity)
{
    if (!isStackable() || (stackable_bit_offset == 0 && gld_stackable_bit_offset == 0))
    {
        return false;
    }

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    getItemCode(strcode);
    switch (gem)
    {
    case 't': // Town portal book
    case 'i': // Identify scroll book
        if (gemcondition == 'b' && gemcolour == 'k')
        {
            // can hold up to 20 scrolls
            quantity = std::min(quantity, MAX_TOME_QUANTITY);
        }
        break;
    case 'k':
        if (gemcondition == 'e' && gemcolour == 'y')
        {
            // only 12 keys can be sacked at need at least 1
            quantity = std::max(std::uint16_t(1), std::min(quantity, MAX_KEY_QUANTITY));
        }
        break;
    case 'g':
        if (gemcondition == 'l' && gemcolour == 'd')
        {
            // can hold up to 4095 gold pieces
            quantity = std::min(quantity, MAX_GLD_QUANTITY);
        }
        break;

    default:
        quantity = std::min(quantity, MAX_STACKED_QUANTITY);
        break;

    }  // end switch

    if (getQuantity() == quantity)
    {
        // nothing to do
        return false;
    }

    if (gld_stackable_bit_offset != 0)
    {
        return updateBits(gld_stackable_bit_offset, 12, quantity);
    }

    return updateBits(stackable_bit_offset, 9, quantity);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMaxQuantity()
{
    if (!isStackable() || (stackable_bit_offset == 0 && gld_stackable_bit_offset == 0))
    {
        return false;
    }

    std::uint16_t quantity = gld_stackable_bit_offset != 0 ? MAX_GLD_QUANTITY : MAX_STACKED_QUANTITY;
    return setQuantity(quantity);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getDefenseRating() const
{
    if (defense_rating_bit_offset == 0)
    {
        return 0;
    }

    static const auto& stat = itemStats[31];
    return (std::uint16_t)(read_uint32_bits(defense_rating_bit_offset, 11) - stat.saveAdd);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDurability(ItemDurability& durability) const
{
    durability.clear();
    if (durability_bit_offset == 0)
    {
        return false;
    }

    durability.Max = (std::uint16_t)read_uint32_bits(durability_bit_offset, 8);
    if (durability.Max == 0)
    {
        return false;
    }

    durability.Current = (std::uint16_t)read_uint32_bits(durability_bit_offset + 8, 8);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setDurability(const ItemDurability& attrib)
{
    ItemDurability oldAttrib;
    if (attrib.Max == 0 || !getDurability(oldAttrib))
    {
        return false;
    }
    
    if (attrib.Current == oldAttrib.Current &&
        attrib.Max == oldAttrib.Max)
    {
        // nothing to do
        return false;
    }

    if (!updateBits(durability_bit_offset, 8, attrib.Max))
    {
        return false;
    }

    if (!updateBits(durability_bit_offset + 8, 8, attrib.Current))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::fixDurability()
{
    ItemDurability attrib;
    if (!getDurability(attrib))
    {
        return false;
    }

    if (attrib.Current == attrib.Max)
    {
        // nothing to do
        return false;
    }

    attrib.Current = attrib.Max;
    return setDurability(attrib);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMaxDurability()
{
    static ItemDurability maxDurability = { MAX_DURABILITY, MAX_DURABILITY };
    return setDurability(maxDurability);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDamage(ItemDamage& damage) const
{
    damage.clear();

    std::uint8_t strcode[4] = { 0 };
    if (getItemCode(strcode))
    {
        const auto& result = getItemTypeHelper(strcode);
        if (!result.isWeapon())
        {
            return false;
        }

        damage = result.dam;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRealmDataFlag() const
{
    if (isSimpleItem() || (realm_bit_offset == 0))
    {
        return false;
    }

    return read_uint32_bits(realm_bit_offset, 1) == 0 ? false : true;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedItemName() const
{
    std::stringstream ss;
    EarAttributes earAttrib;
    if (getEarAttributes(earAttrib))
    {
        ss << earAttrib.getName() << "'s Ear\n" << earAttrib.getClassName() << "\nLevel " << std::dec << earAttrib.getLevel();
        return ss.str();
    }

    if (!isIdentified())
    {
        ss << getItemTypeName() << "\nUnidentified";
        return ss.str();
    }

    d2ce::RunewordAttributes runewordAttrib;
    if (getRunewordAttributes(runewordAttrib) && !runewordAttrib.Name.empty())
    {
        ss << runewordAttrib.Name << "\n";
    }

    std::string personalizedName = getPersonalizedName();
    if (!personalizedName.empty())
    {
        ss << personalizedName << "'s ";
    }

    MagicalAffixes affixes;
    d2ce::RareAttributes rareAttrib;
    d2ce::UniqueAttributes uniqueAttrib;
    d2ce::SetAttributes setAttrib;
    auto quality = getQuality();
    switch (quality)
    {
    case EnumItemQuality::INFERIOR:
        switch (getInferiorQualityId())
        {
        case 0:
            ss << "Crude ";
            break;

        case 1:
            ss << "Cracked ";
            break;

        case 2:
            ss << "Damaged ";
            break;

        case 3:
            ss << "Low Quality ";
            break;
        }
        ss << getItemTypeName();
        break;

    case EnumItemQuality::NORMAL:
        if (socketedItemCount() > 0)
        {
            ss << "Gemmed ";
        }
        ss << getItemTypeName();
        break;

    case EnumItemQuality::SUPERIOR:
        ss << "Superior " << getItemTypeName();
        break;

    case EnumItemQuality::MAGIC:
        if (getMagicalAffixes(affixes))
        {
            if (!affixes.PrefixName.empty())
            {
                ss << affixes.PrefixName << " ";
            }

            ss << getItemTypeName();

            if (!affixes.SuffixName.empty())
            {
                ss << " " << affixes.SuffixName;
            }
        }
        else
        {
            // should not happen
            ss << getItemTypeName();
        }
        break;

    case EnumItemQuality::SET:
        if (getSetAttributes(setAttrib) && !setAttrib.Name.empty())
        {
            ss << setAttrib.Name << '\n' << getItemTypeName();
        }
        else
        {
            // should not happen
            ss << getItemTypeName();
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        if (getRareOrCraftedAttributes(rareAttrib))
        {
            if (!rareAttrib.Name.empty())
            {
                ss << rareAttrib.Name << " ";
            }

            if (!rareAttrib.Name2.empty())
            {
                ss << rareAttrib.Name2;
            }

            ss << '\n' << getItemTypeName();
        }
        else
        {
            // should not happen
            ss << getItemTypeName();
        }
        break;

    case EnumItemQuality::UNIQUE:
        if (getUniqueAttributes(uniqueAttrib) && !uniqueAttrib.Name.empty())
        {
            ss << uniqueAttrib.Name << '\n' << getItemTypeName();
        }
        else
        {
            // should not happen
            ss << getItemTypeName();
        }
        break;

    default:
        // should not happen
        ss << getItemTypeName();
        break;
    }

    return ss.str();
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedSocketedRunes() const
{
    std::string name;
    std::stringstream ss;
    bool bFirstItem = true;
    for (const auto& item : SocketedItems)
    {
        if (item.isRune())
        {
            if (bFirstItem)
            {
                ss << "'";
            }

            name = item.getItemTypeName();
            ss << name.substr(0, name.find(" "));
        }
    }

    if (!bFirstItem)
    {
        ss << "'";
    }

    return ss.str();
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getDisplayedDefenseRating(std::uint32_t charLevel) const
{
    if (defense_rating_bit_offset == 0)
    {
        return 0;
    }

    auto defenseRating = getDefenseRating();

    // Calculate item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        checkForRelatedMagicalAttributes(magicalAttributes);

        std::uint64_t eDef = 0;
        std::uint64_t def = 0;
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = itemStats[attrib.Id];
            switch (attrib.Id)
            {
            case 16:
            case 215:
                eDef += getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 31:
            case 214:
                def += getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;
            }
        }

        defenseRating += std::uint16_t((defenseRating * eDef) / 100 + def);
    }

    return defenseRating;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedDurability(ItemDurability& durability, std::uint32_t charLevel) const
{
    if (!getDurability(durability))
    {
        return false;
    }

    // Calculate item bonus
    std::uint64_t eDur = 0;
    std::uint64_t dur = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        checkForRelatedMagicalAttributes(magicalAttributes);
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = itemStats[attrib.Id];
            switch (attrib.Id)
            {
            case 73:
                dur += getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 75:
                eDur += getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;
            }
        }

        durability.Max += std::uint16_t((durability.Max * eDur) / 100 + dur);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedDamage(ItemDamage& damage, std::uint32_t charLevel) const
{
    if (!getDamage(damage))
    {
        return false;
    }

    // Calculate item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        checkForRelatedMagicalAttributes(magicalAttributes);

        // Calculate item bonus
        std::uint64_t eDmg = 0;
        std::uint64_t eDmgMax = 0;
        std::uint64_t dmgMin = 0;
        std::uint64_t dmgMax = 0;
        std::uint64_t dmgMin2 = 0;
        std::uint64_t dmgMax2 = 0;
        std::uint64_t dmgMinThrow = 0;
        std::uint64_t dmgMaxThrow = 0;
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = itemStats[attrib.Id];
            switch (attrib.Id)
            {
            case 17:
                eDmg += getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 21:
                dmgMin += getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 22:
                dmgMax += getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 23:
                dmgMin2 += getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMin
                break;

            case 24:
                dmgMax2 += getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMax
                break;

            case 219:
                eDmgMax += getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 159:
                dmgMinThrow += getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMin
                break;

            case 160:
                dmgMaxThrow += getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMax
                break;
            }
        }

        if (dmgMin == 0)
        {
            dmgMin = std::max(dmgMinThrow, dmgMin2);
        }

        if (dmgMax == 0)
        {
            dmgMax = std::max(dmgMaxThrow, dmgMax2);
        }

        if (isEthereal())
        {
            if (damage.OneHanded.Max != 0)
            {
                damage.OneHanded.Min += std::uint16_t((std::uint64_t(damage.OneHanded.Min) * 50) / 100);
                damage.OneHanded.Max += std::uint16_t((std::uint64_t(damage.OneHanded.Max) * 50) / 100);
            }

            if (damage.bTwoHanded)
            {
                damage.TwoHanded.Min += std::uint16_t((std::uint64_t(damage.TwoHanded.Min) * 50) / 100);
                damage.TwoHanded.Max += std::uint16_t((std::uint64_t(damage.TwoHanded.Max) * 50) / 100);
            }

            if (damage.Missile.Max != 0)
            {
                damage.Missile.Min += std::uint16_t((std::uint64_t(damage.Missile.Min) * 50) / 100);
                damage.Missile.Max += std::uint16_t((std::uint64_t(damage.Missile.Max) * 50) / 100);
            }
        }

        if (damage.OneHanded.Max != 0)
        {
            damage.OneHanded.Min += std::uint16_t((damage.OneHanded.Min * eDmg) / 100 + dmgMin);
            damage.OneHanded.Max += std::uint16_t((damage.OneHanded.Max * (eDmg + eDmgMax)) / 100 + dmgMax);
            if (damage.OneHanded.Min > damage.OneHanded.Max)
            {
                damage.OneHanded.Max = damage.OneHanded.Min + 1;
            }
        }

        if (damage.bTwoHanded)
        {
            damage.TwoHanded.Min += std::uint16_t((damage.TwoHanded.Min * eDmg) / 100 + dmgMin);
            damage.TwoHanded.Max += std::uint16_t((damage.TwoHanded.Max * (eDmg + eDmgMax)) / 100 + dmgMax);
            if (damage.TwoHanded.Min > damage.TwoHanded.Max)
            {
                damage.TwoHanded.Max = damage.TwoHanded.Min + 1;
            }
        }

        if (damage.Missile.Max != 0)
        {
            damage.Missile.Min += std::uint16_t((damage.Missile.Min * eDmg) / 100 + dmgMin);
            damage.Missile.Max += std::uint16_t((damage.Missile.Max * (eDmg + eDmgMax)) / 100 + dmgMax);
            if (damage.Missile.Min > damage.Missile.Max)
            {
                damage.Missile.Max = damage.Missile.Min + 1;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedItemAttributes(EnumCharClass charClass, std::uint32_t charLevel) const
{
    if (isEar())
    {
        return "";
    }

    std::uint8_t strcode[4] = { 0 };
    getItemCode(strcode);
    const auto& itemType = getItemTypeHelper(strcode);
    if (itemType.isPotion())
    {
        auto points = itemType.getPotionPoints(charClass);
        if (points == 0)
        {
            return "";
        }

        std::stringstream ss;
        if (itemType.isRejuvenationPotion())
        {
            ss << "Heals " << std::dec << points << "% Life and Mana";
        }
        else
        {
            ss << "Points: " << std::dec << points;
        }

        return ss.str();
    }

    bool bFirst = true;
    std::stringstream ss;
    if (itemType.isStackable())
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Quantity: " << std::dec << getQuantity();
    }

    auto defenseRating = getDisplayedDefenseRating(charLevel);
    if (defenseRating > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }

        ss << "Defense: " << std::dec << defenseRating;
    }

    if (itemType.isBelt())
    {
        ItemDimensions dimensions;
        if (getDimensions(dimensions))
        {
            std::uint16_t numSlots = dimensions.InvWidth * dimensions.InvHeight - 4;
            if (numSlots > 0)
            {
                if (bFirst)
                {
                    bFirst = false;
                }
                else
                {
                    ss << "\n";
                }

                ss << "Belt Size: +" << std::dec << numSlots << " Slots";
            }
        }
    }

    if (itemType.isWeapon())
    {
        ItemDamage dam;
        getDisplayedDamage(dam, charLevel);
        if (dam.OneHanded.Max != 0)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            ss << "One-Hand Damage: " << std::dec << dam.OneHanded.Min << " to " << dam.OneHanded.Max;
        }

        if (dam.bTwoHanded)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            ss << "Two-Hand Damage: " << std::dec << dam.TwoHanded.Min << " to " << dam.TwoHanded.Max;
        }

        if (dam.Missile.Max != 0)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            ss << "Throw Damage: " << std::dec << dam.Missile.Min << " to " << std::dec << dam.Missile.Max;
        }
    }

    ItemDurability durability;
    if (getDisplayedDurability(durability, charLevel))
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }

        ss << "Durability: " << std::dec << durability.Current << " of " << std::dec << durability.Max;
    }

    if (itemType.req.Dexterity > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Required Dexterity: " << std::dec << itemType.req.Dexterity;
    }

    if (itemType.req.Strength > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Required Strength: " << std::dec << itemType.req.Strength;
    }

    if (itemType.req.Level > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Required Level: " << std::dec << itemType.req.Level;
    }

    return ss.str();
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getMagicalAttributes(attribs))
    {
        return false;
    }

    // check for the "all" cases
    checkForRelatedMagicalAttributes(attribs);

    bool bFormatted = false;
    for (auto& attrib : attribs)
    {
        bFormatted |= formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.begin(), attribs.end(), magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedRunewordAttributes(RunewordAttributes& attribs, std::uint32_t charLevel) const
{
    if (!getRunewordAttributes(attribs))
    {
        return false;
    }

    // check for the "all" cases
    checkForRelatedMagicalAttributes(attribs.MagicalAttributes);

    bool bFormatted = false;
    for (auto& attrib : attribs.MagicalAttributes)
    {
        bFormatted |= formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.MagicalAttributes.begin(), attribs.MagicalAttributes.end(), magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getCombinedMagicalAttributes(attribs))
    {
        return false;
    }

    // check for the "all" cases
    checkForRelatedMagicalAttributes(attribs);

    bool bFormatted = false;
    for (auto& attrib : attribs)
    {
        bFormatted |= formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.begin(), attribs.end(), magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::Item::readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
{
    size_t readOffset = current_bit_offset;

    // Ensure we read enough
    if (!skipBits(charfile, current_bit_offset, bits))
    {
        return 0;
    }

    if (bits > 64)
    {
        return 0;
    }

    return read_uint64_bits(readOffset, bits);
}
//---------------------------------------------------------------------------
bool d2ce::Item::skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
{
    if (feof(charfile))
    {
        return false;
    }

    size_t bytesRequired = (current_bit_offset + bits + 7) / 8;
    std::uint8_t value = 0;
    while (data.size() < bytesRequired)
    {
        if (feof(charfile))
        {
            return false;
        }

        std::fread(&value, sizeof(value), 1, charfile);
        data.push_back(value);
    }

    current_bit_offset += bits;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readItem(EnumCharVersion version, std::FILE* charfile)
{
    FileVersion = version;
    data.clear();
    SocketedItems.clear();

    // reserve enough space to reduce chance of reallocation (haven't seen an item size bigger then 80
    data.reserve(80);

    start_bit_offset = 0;
    size_t current_bit_offset = start_bit_offset;
    if (FileVersion < EnumCharVersion::v115)
    {
        std::uint8_t value = (std::uint8_t)readBits(charfile, current_bit_offset, 8);
        if (value != ITEM_MARKER[0])
        {
            // corrupt file
            return false;
        }

        value = (std::uint8_t)readBits(charfile, current_bit_offset, 8);
        if (value != ITEM_MARKER[1])
        {
            // corrupt file
            return false;
        }

        start_bit_offset = current_bit_offset;

        // At least the Simple/Compact size is required
        if (!skipBits(charfile, current_bit_offset, 95))
        {
            return false;
        }

        is_potion_bit_offset = 26;
        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
            alt_position_id_offset = 30;
            equipped_id_offset = 48;
            type_code_offset = 68;
            location_bit_offset = 78;
            position_offset = 129;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06
            location_bit_offset = 66;
            equipped_id_offset = 74;
            type_code_offset = 82;
            alt_position_id_offset = 90;
            position_offset = 137;
            break;

        default: // v1.07 - v1.14d
            location_bit_offset = 58;
            equipped_id_offset = 61;
            position_offset = 65;
            alt_position_id_offset = 73;
            type_code_offset = 76;
            break;
        }

        extended_data_offset = 108;
    }
    else
    {
        // flags
        if (!skipBits(charfile, current_bit_offset, 32))
        {
            return false;
        }

        is_potion_bit_offset = 10;

        // version
        if (!skipBits(charfile, current_bit_offset, 3))
        {
            return false;
        }

        // Location
        location_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 3))
        {
            return false;
        }

        // equipped_id
        equipped_id_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 4))
        {
            return false;
        }

        // position x/y
        position_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 8))
        {
            return false;
        }

        // position x/y
        alt_position_id_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 3))
        {
            return false;
        }

        type_code_offset = current_bit_offset;
        if (isEar())
        {
            if (!skipBits(charfile, current_bit_offset, 10))
            {
                return false;
            }

            // up to 15 7 bit characters
            for (std::uint8_t idx = 0; !feof(charfile) && idx < 15 && readBits(charfile, current_bit_offset, 7) != 0; ++idx);
            item_end_bit_offset = current_bit_offset;
            return true;
        }
        else
        {
            for (size_t i = 0; i < 4; ++i)
            {
                if (getEncodedChar(charfile, current_bit_offset) == 0xFF)
                {
                    return false;
                }
            }

            extended_data_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, isSimpleItem() ? 1 : 3))
            {
                return false;
            }
        }
    }

    quest_difficulty_offset = 0;
    gld_stackable_bit_offset = 0;
    std::uint8_t strcode[4] = { 0 };
    getItemCode(strcode);
    const auto& itemType = getItemTypeHelper(strcode);
    if (isSimpleItem())
    {
        if (itemType.isGoldItem())
        {
            // Is this correct for gld items? It's not currently used, so is it even needed?
            gld_stackable_bit_offset = extended_data_offset + 1;
            if (!skipBits(charfile, extended_data_offset, 12))
            {
                return false;
            }

            nr_of_items_in_sockets_offset = current_bit_offset;
            nr_of_items_in_sockets_bits = 1;
        }
        else
        {
            nr_of_items_in_sockets_offset = extended_data_offset;
            nr_of_items_in_sockets_bits = 1;
            if (itemType.isQuestItem())
            {
                quest_difficulty_offset = extended_data_offset;
                nr_of_items_in_sockets_offset = quest_difficulty_offset + 2;
                nr_of_items_in_sockets_bits = 1;
                if (!skipBits(charfile, current_bit_offset, 2))
                {
                    return false;
                }
            }
        }

        item_end_bit_offset = current_bit_offset;
        return true;
    }
    else if (itemType.isQuestItem())
    {
        quest_difficulty_offset = extended_data_offset;
        nr_of_items_in_sockets_offset = quest_difficulty_offset + 2;
        nr_of_items_in_sockets_bits = 1;
    }
    else
    {
        nr_of_items_in_sockets_offset = extended_data_offset;
        nr_of_items_in_sockets_bits = 3;
    }

    item_id_bit_offset = current_bit_offset;
    if (!skipBits(charfile, current_bit_offset, 32))
    {
        return false;
    }

    item_level_bit_offset = current_bit_offset;
    if (!skipBits(charfile, current_bit_offset, 7))
    {
        return false;
    }

    quality_bit_offset = current_bit_offset;
    auto qualityval = readBits(charfile, current_bit_offset, 4);
    if (qualityval > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
    {
        return false;
    }

    auto quality = static_cast<EnumItemQuality>(qualityval);

    // If this is TRUE, it means the item has more than one picture associated with it.
    multi_graphic_bit_offset = current_bit_offset;
    if (readBits(charfile, current_bit_offset, 1) != 0)
    {
        // The next 3 bits contain the picture ID
        if (!skipBits(charfile, current_bit_offset, 3))
        {
            return false;
        }
    }

    // If this is TRUE, it means the item is class specific.
    autoAffix_bit_offset = current_bit_offset;
    if (readBits(charfile, current_bit_offset, 1) != 0)
    {
        // If the item is class specific, the next 11 bits will
        // contain the class specific data.
        if (!skipBits(charfile, current_bit_offset, 11))
        {
            return false;
        }
    }

    quality_attrib_bit_offset = 0;
    switch (quality)
    {
    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::SUPERIOR:
        quality_attrib_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 3))
        {
            return false;
        }
        break;

    case EnumItemQuality::MAGIC:
        quality_attrib_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 22))
        {
            return false;
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        quality_attrib_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 16))
        {
            return false;
        }

        // Following the name IDs, we got 6 possible magical affixes, the pattern
        // is 1 bit id, 11 bit value... But the value will only exist if the prefix
        // is 1. So we'll read the id first and check it against 1.
        for (size_t i = 3; i > 0; --i)
        {
            if (readBits(charfile, current_bit_offset, 1) != 0)
            {
                if (!skipBits(charfile, current_bit_offset, 11))
                {
                    return false;
                }
            }

            if (feof(charfile))
            {
                return false;
            }

            if (readBits(charfile, current_bit_offset, 1) != 0)
            {
                if (!skipBits(charfile, current_bit_offset, 11))
                {
                    return false;
                }
            }

            if (feof(charfile))
            {
                return false;
            }
        }
        break;

    case EnumItemQuality::SET:
    case EnumItemQuality::UNIQUE:
        quality_attrib_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 12))
        {
            return false;
        }
        break;
    }

    if (isRuneword())
    {
        runeword_id_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 16))
        {
            return false;
        }
    }

    if (isPersonalized())
    {
        personalized_bit_offset = current_bit_offset;

        // up to 15 7 bit characters
        for (std::uint8_t idx = 0; !feof(charfile) && idx < 15 && readBits(charfile, current_bit_offset, 7) != 0; ++idx);
    }

    std::uint8_t base = itemType.getBaseType();
    bool isArmor = (base & 4) != 0 ? true : false;
    bool isWeapon = (base & 2) != 0 ? true : false;
    bool isTome = (base & 8) != 0 ? true : false;
    bool isStackable = (base & 1) != 0 ? true : false;

    if (isTome)
    {
        // If the item is a tome, it will contain 5 extra bits, we're not
        // interested in these bits, the value is usually 1, but not sure
        // what it is.
        if (!skipBits(charfile, current_bit_offset, 5))
        {
            return false;
        }
    }

    // Realm Data Flag
    realm_bit_offset = current_bit_offset;
    if (readBits(charfile, current_bit_offset, 1) != 0)
    {
        if (!skipBits(charfile, current_bit_offset, 96))
        {
            return false;
        }
    }

    if (feof(charfile))
    {
        return false;
    }

    if (isArmor || isWeapon)
    {
        if (isArmor)
        {
            // Defense rating
            defense_rating_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, ((FileVersion >= EnumCharVersion::v110) ? 11 : 10)))
            {
                return false;
            }
        }

        // Some armor/weapons like phase blades don't have durability
        durability_bit_offset = current_bit_offset;
        if (readBits(charfile, current_bit_offset, 8) > 0)
        {
            // current durability value (8 bits + unknown single bit)
            if (!skipBits(charfile, current_bit_offset, ((FileVersion >= EnumCharVersion::v110) ? 9 : 8)))
            {
                return false;
            }
        }

        if (feof(charfile))
        {
            return false;
        }
    }

    if (isStackable)
    {
        // If the item is a stacked item, e.g. a javelin or something, these 9
        // bits will contain the quantity.
        stackable_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 9))
        {
            return false;
        }
    }

    if (isSocketed())
    {
        // If the item is socketed, it will contain 4 bits of data which are the
        // number of total sockets the item have, regardless of how many are occupied
        // by an item.
        socket_count_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 4))
        {
            return false;
        }
    }

    // If the item is part of a set, these bit will tell us how many lists
    // of magical properties follow the one regular magical property list.
    std::uint8_t setBonusBits = 0;
    if (quality == EnumItemQuality::SET)
    {
        bonus_bits_bit_offset = current_bit_offset;
        setBonusBits = (std::uint8_t)readBits(charfile, current_bit_offset, 5);
        if (feof(charfile))
        {
            return false;
        }
    }

    // magical properties
    magical_props_bit_offset = current_bit_offset;
    if (!parsePropertyList(charfile, current_bit_offset))
    {
        return false;
    }

    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        set_bonus_props_bit_offset = current_bit_offset;
        for (size_t i = 0; i < 5 && setBonusBits > 0; ++i, setBonusBits >>= 1)
        {
            if ((setBonusBits & 0x01) != 0)
            {
                if (!parsePropertyList(charfile, current_bit_offset))
                {
                    return false;
                }
            }
        }
    }

    if (isRuneword())
    {
        // runewords have their own list of magical properties
        runeword_props_bit_offset = current_bit_offset;
        if (!parsePropertyList(charfile, current_bit_offset))
        {
            return false;
        }
    }

    auto numSocketed = socketedItemCount();
    if (numSocketed > 0)
    {
        for (std::uint8_t i = 0; !feof(charfile) && i < numSocketed; ++i)
        {
            SocketedItems.resize(SocketedItems.size() + 1);
            auto& childItem = SocketedItems.back();
            if (!childItem.readItem(FileVersion, charfile))
            {
                return false;
            }

            if (childItem.getLocation() != EnumItemLocation::SOCKET)
            {
                // corrupt file
                return false;
            }

            // resolve magical properties of socketed gem
            childItem.getItemCode(strcode);
            std::vector<MagicalAttribute> attribs;
            itemType.getSocketedMagicalAttributes(childItem.socketedMagicalAttributes, getItemType());
        }
    }

    item_end_bit_offset = current_bit_offset;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::writeItem(std::FILE* charfile)
{
    if (data.empty())
    {
        return true;
    }

    if (std::fwrite(&data[0], data.size(), 1, charfile) != 1)
    {
        std::fflush(charfile);
        return false;
    }

    // now write child items
    for (auto& item : SocketedItems)
    {
        if (!item.writeItem(charfile))
        {
            std::fflush(charfile);
            return false;
        }
    }

    std::fflush(charfile);
    return true;
}
//---------------------------------------------------------------------------
void d2ce::Item::asJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel, bool isListItem /*= true*/) const
{
    std::vector<MagicalAttribute> magicalAttributes;
    d2ce::RunewordAttributes runewordAttrib;
    std::string braceIndent = parentIndent;
    std::string itemPropIndent = parentIndent + jsonIndentStr;
    if (isListItem)
    {
        itemPropIndent += jsonIndentStr;
        braceIndent += jsonIndentStr;
        ss << "\n" << braceIndent << "{";
    }

    unknownAsJson(ss, itemPropIndent);
    ss << ",\n" << itemPropIndent << "\"identified\": " << std::dec << (isIdentified() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"socketed\": " << std::dec << (isSocketed() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"new\": " << std::dec << (isNew() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"is_ear\": " << std::dec << (isEar() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"starter_item\": " << std::dec << (isStarterItem() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"simple_item\": " << std::dec << (isSimpleItem() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"ethereal\": " << std::dec << (isEthereal() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"personalized\": " << std::dec << (isPersonalized() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"given_runeword\": " << std::dec << (isRuneword() ? 1 : 0);
    ss << ",\n" << itemPropIndent << "\"version\": " << getRawVersion();
    ss << ",\n" << itemPropIndent << "\"location_id\": " << std::dec << std::uint16_t(getLocation());
    ss << ",\n" << itemPropIndent << "\"equipped_id\": " << std::dec << std::uint16_t(getEquippedId());
    ss << ",\n" << itemPropIndent << "\"position_x\": " << std::dec << std::uint16_t(getPositionX());
    ss << ",\n" << itemPropIndent << "\"position_y\": " << std::dec << std::uint16_t(getPositionY());
    ss << ",\n" << itemPropIndent << "\"alt_position_id\": " << std::dec << std::uint16_t(getAltPositionId());

    if (isEar())
    {
        EarAttributes earAttrib;
        getEarAttributes(earAttrib);
        ss << ",";
        earAttrib.asJson(ss, itemPropIndent);
        return;
    }

    std::uint8_t strcode[4] = { 0 };
    getItemCode(strcode);
    const auto& itemType = getItemTypeHelper(strcode);
    strcode[3] = 0;
    ss << ",\n" << itemPropIndent << "\"type\": \"" << strcode << "\"";
    ss << ",\n" << itemPropIndent << "\"categories\": [";
    if (itemType.categories.empty())
    {
        ss << "]";
    }
    else
    {
        bool bFirstItem = true;
        for (auto& category : itemType.categories)
        {
            if (bFirstItem)
            {
                bFirstItem = false;
            }
            else
            {
                ss << ",";
            }
            ss << "\n" << itemPropIndent << jsonIndentStr << "\"" << category << "\"";
        }
        ss << "\n" << itemPropIndent << "]";
    }
    ss << ",\n" << itemPropIndent << "\"type_id\": " << std::dec << std::uint16_t(itemType.getEnumItemType());
    if (itemType.isQuestItem())
    {
        ss << ",\n" << itemPropIndent << "\"quest_difficulty\": " << std::dec << std::uint16_t(getQuestDifficulty());
    }
    ss << ",\n" << itemPropIndent << "\"nr_of_items_in_sockets\": " << std::dec << std::uint16_t(socketedItemCount());

    if (isSimpleItem())
    {
        getMagicalAttributes(magicalAttributes);
        if (!magicalAttributes.empty())
        {
            ss << ",\n" << itemPropIndent << "\"magic_attributes\": [";
            if (magicalAttributes.empty())
            {
                ss << "]";
            }
            else
            {
                bool bFirstItem = true;
                for (auto& attrib : magicalAttributes)
                {
                    if (bFirstItem)
                    {
                        bFirstItem = false;
                    }
                    else
                    {
                        ss << ",";
                    }
                    attrib.asJson(ss, itemPropIndent);
                }
                ss << "\n" << itemPropIndent << "]";
            }
        }
    }
    else
    {
        ss << ",\n" << itemPropIndent << "\"id\": " << std::dec << getId();
        ss << ",\n" << itemPropIndent << "\"level\": " << std::dec << std::uint16_t(getLevel());

        auto quality = getQuality();
        ss << ",\n" << itemPropIndent << "\"quality\": " << std::dec << std::uint16_t(quality);
        ss << ",\n" << itemPropIndent << "\"multiple_pictures\": " << std::dec << (hasMultipleGraphics() ? 1 : 0);
        if (hasMultipleGraphics())
        {
            ss << ",\n" << itemPropIndent << "\"picture_id\": " << std::dec << std::uint16_t(getPictureId());
        }
        ss << ",\n" << itemPropIndent << "\"class_specific\": " << std::dec << (isAutoAffix() ? 1 : 0);
        if (quality == EnumItemQuality::INFERIOR)
        {
            ss << ",\n" << itemPropIndent << "\"low_quality_id\": " << std::dec << std::uint16_t(getInferiorQualityId());
        }

        if (quality == EnumItemQuality::MAGIC)
        {
            d2ce::MagicalAffixes magicalAffixes;
            getMagicalAffixes(magicalAffixes);
            ss << ",";
            magicalAffixes.asJson(ss, itemPropIndent);
        }

        if (isRuneword())
        {
            getRunewordAttributes(runewordAttrib);
            ss << ",";
            runewordAttrib.asJson(ss, itemPropIndent);
        }

        switch (quality)
        {
        case EnumItemQuality::SET:
        {
            d2ce::SetAttributes setAttrib;
            getSetAttributes(setAttrib);
            ss << ",";
            setAttrib.asJson(ss, itemPropIndent);
            break;
        }

        case EnumItemQuality::RARE:
        case EnumItemQuality::CRAFT:
        case EnumItemQuality::TEMPERED:
        {
            d2ce::RareAttributes rareAttrib;
            getRareOrCraftedAttributes(rareAttrib);
            ss << ",";
            rareAttrib.asJson(ss, itemPropIndent);
            break;
        }

        case EnumItemQuality::UNIQUE:
        {
            d2ce::UniqueAttributes uniqueAttrib;
            getUniqueAttributes(uniqueAttrib);
            ss << ",";
            uniqueAttrib.asJson(ss, itemPropIndent);
            break;
        }
        }

        if (isPersonalized())
        {
            ss << ",\n" << itemPropIndent << "\"personalized_name\": \"" << getPersonalizedName() << "\"";
        }

        ss << ",\n" << itemPropIndent << "\"timestamp\": " << std::dec << (getRealmDataFlag() ? 1 : 0);
        if (isStackable())
        {
            ss << ",\n" << itemPropIndent << "\"quantity\": " << std::dec << getQuantity();
        }

        auto defenseRating = getDefenseRating();
        if (defenseRating > 0)
        {
            ss << ",\n" << itemPropIndent << "\"defense_rating\": " << std::dec << defenseRating;
        }

        ItemDurability durability;
        if (getDurability(durability))
        {
            ss << ",\n" << itemPropIndent << "\"max_durability\": " << std::dec << durability.Max;
            ss << ",\n" << itemPropIndent << "\"current_durability\": " << std::dec << durability.Current;
        }

        if (isSocketed())
        {
            ss << ",\n" << itemPropIndent << "\"total_nr_of_sockets\": " << std::dec << std::uint16_t(totalNumberOfSockets());
        }

        getMagicalAttributes(magicalAttributes);
        ss << ",\n" << itemPropIndent << "\"magic_attributes\": [";
        if (magicalAttributes.empty())
        {
            ss << "]";
        }
        else
        {
            bool bFirstItem = true;
            for (auto& attrib : magicalAttributes)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                attrib.asJson(ss, itemPropIndent);
            }
            ss << "\n" << itemPropIndent << "]";
        }

        if (itemType.isWeapon())
        {
            ss << ",";
            itemType.dam.asJson(ss, itemPropIndent);
        }

        if (isSocketed())
        {
            // Socketed items
            ss << ",\n" << itemPropIndent << "\"socketed_items\": [";
            if (SocketedItems.empty())
            {
                ss << "]";
            }
            else
            {
                bool bFirstItem = true;
                std::string socketedIndent = parentIndent + jsonIndentStr + jsonIndentStr;
                for (auto& item : SocketedItems)
                {
                    if (bFirstItem)
                    {
                        bFirstItem = false;
                    }
                    else
                    {
                        ss << ",";
                    }
                    item.asJson(ss, socketedIndent, charLevel);
                }
                ss << "\n" << itemPropIndent << "]";
            }
        }
    }

    ss << ",\n" << itemPropIndent << "\"type_name\": \"" << itemType.name << "\"";
    if (itemType.req.Strength != 0)
    {
        ss << ",\n" << itemPropIndent << "\"reqstr\": " << std::dec << itemType.req.Strength;
    }

    if (itemType.req.Dexterity != 0)
    {
        ss << ",\n" << itemPropIndent << "\"reqdex\": " << std::dec << itemType.req.Dexterity;
    }

    if (itemType.req.Level != 0)
    {
        ss << ",\n" << itemPropIndent << "\"levelreq\": " << std::dec << itemType.req.Level;
    }

    ss << ",\n" << itemPropIndent << "\"inv_file\": \"" << itemType.inv_file << "\"";
    ss << ",\n" << itemPropIndent << "\"inv_height\": " << std::dec << itemType.dimensions.Height;
    ss << ",\n" << itemPropIndent << "\"inv_width\": " << std::dec << itemType.dimensions.Width;

    if (itemType.inv_transform != 0)
    {
        ss << ",\n" << itemPropIndent << "\"inv_transform\": " << std::dec << itemType.inv_transform;
    }

    if (!isSimpleItem() || !magicalAttributes.empty())
    {
        // For efficiency reasons we do the formatting using the existing list 
        // instead of building it again
        checkForRelatedMagicalAttributes(magicalAttributes);
        for (auto& attrib : magicalAttributes)
        {
            formatDisplayedMagicalAttribute(attrib, charLevel);
        }

        // Sort display items in proper order
        std::sort(magicalAttributes.begin(), magicalAttributes.end(), magicalAttributeSorter);

        ss << ",\n" << itemPropIndent << "\"displayed_magic_attributes\": [";
        if (magicalAttributes.empty())
        {
            ss << "]";
        }
        else
        {
            bool bFirstItem = true;
            for (auto& attrib : magicalAttributes)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                attrib.asJson(ss, itemPropIndent);
            }
            ss << "\n" << itemPropIndent << "]";
        }

        ss << ",\n" << itemPropIndent << "\"displayed_runeword_attributes\": [";
        if (runewordAttrib.MagicalAttributes.empty())
        {
            ss << "]";
        }
        else
        {
            checkForRelatedMagicalAttributes(runewordAttrib.MagicalAttributes);
            for (auto& attrib : runewordAttrib.MagicalAttributes)
            {
                formatDisplayedMagicalAttribute(attrib, charLevel);
            }

            // Sort display items in proper order
            std::sort(runewordAttrib.MagicalAttributes.begin(), runewordAttrib.MagicalAttributes.end(), magicalAttributeSorter);

            bool bFirstItem = true;
            for (auto& attrib : runewordAttrib.MagicalAttributes)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                attrib.asJson(ss, itemPropIndent);
            }
            ss << "\n" << itemPropIndent << "]";
        }

        getCombinedMagicalAttributes(magicalAttributes);
        ss << ",\n" << itemPropIndent << "\"combined_magic_attributes\": [";
        if (magicalAttributes.empty())
        {
            ss << "]";
        }
        else
        {
            bool bFirstItem = true;
            for (auto& attrib : magicalAttributes)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                attrib.asJson(ss, itemPropIndent);
            }
            ss << "\n" << itemPropIndent << "]";
        }

        // For efficiency reasons we do the formatting using the existing list 
        // instead of building it again
        checkForRelatedMagicalAttributes(magicalAttributes);
        for (auto& attrib : magicalAttributes)
        {
            formatDisplayedMagicalAttribute(attrib, charLevel);
        }

        // Sort display items in proper order
        std::sort(magicalAttributes.begin(), magicalAttributes.end(), magicalAttributeSorter);

        ss << ",\n" << itemPropIndent << "\"displayed_combined_magic_attributes\": [";
        if (magicalAttributes.empty())
        {
            ss << "]";
        }
        else
        {
            bool bFirstItem = true;
            for (auto& attrib : magicalAttributes)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                attrib.asJson(ss, itemPropIndent);
            }
            ss << "\n" << itemPropIndent << "]";
        }
    }

    ss << "\n" << braceIndent << "}";
}
//---------------------------------------------------------------------------
void d2ce::Item::unknownAsJson(std::stringstream& ss, const std::string& parentIndent) const
{
    struct byteRange
    {
        size_t startIdx = 0;
        size_t endIdx = 0;
    };
    static std::vector<byteRange> unknowns = { {0,3},{5,10},{12,12},{14,15},{18,20},{23,23},{25,25},{27,31} };

    ss << "\n" << parentIndent << "\"_unknown_data\": {";
    std::string bytesParentIndent = parentIndent + jsonIndentStr;
    bool bFirstItem = true;
    for (const auto& byteInfo : unknowns)
    {
        if (bFirstItem)
        {
            bFirstItem = false;
        }
        else
        {
            ss << ",";
        }

        byteRangeAsJson(ss, bytesParentIndent, byteInfo.startIdx, byteInfo.endIdx);
    }
    ss << "\n" << parentIndent << "}";
}
//---------------------------------------------------------------------------
void d2ce::Item::byteRangeAsJson(std::stringstream& ss, const std::string& parentIndent, size_t startByte, size_t endByte) const
{
    if (endByte < startByte)
    {
        return;
    }

    ss << "\n" << parentIndent << "\"b" << std::dec << startByte;
    if (endByte > startByte)
    {
        ss << "_" << std::dec << endByte;
    }
    ss << "\": {";

    startByte += start_bit_offset;
    endByte += start_bit_offset;

    bool bFirstItem = true;
    for (size_t idx = startByte; idx <= endByte; ++idx)
    {
        if (bFirstItem)
        {
            bFirstItem = false;
        }
        else
        {
            ss << ",";
        }

        ss << "\n" << parentIndent << jsonIndentStr << "\"" << std::dec << (idx - startByte) << "\": " << std::dec << read_uint32_bits(idx, 1);
    }
    ss << "\n" << parentIndent << "}";
}
//---------------------------------------------------------------------------
bool d2ce::Item::parsePropertyList(std::FILE* charfile, size_t& current_bit_offset)
{
    std::uint16_t id = (std::uint16_t)readBits(charfile, current_bit_offset, 9);
    if (feof(charfile))
    {
        return false;
    }

    while (id != 0x1FF)
    {
        if (id >= itemStats.size())
        {
            // corrupt file
            return false;
        }

        const ItemStat* stat = &itemStats[id];

        // saveBits being zero or >= 64 is unrecoverably bad, and
        if (stat->saveBits == 0 || stat->saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        size_t numParms = 0;
        if (stat->encode == 2)
        {
            if (stat->saveBits != 7 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 3;
        }
        else if (stat->encode == 3)
        {
            if (stat->saveBits != 16 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 4;
        }
        else if (stat->encode == 4)
        {
            if (stat->saveBits != 3 || stat->saveParamBits != 16)
            {
                // time-based stats were never implemented, so it's a corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 3;
        }
        else if (stat->saveParamBits > 0)
        {
            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 2;
        }
        else
        {
            if (!skipBits(charfile, current_bit_offset, stat->saveBits))
            {
                // corrupt file
                return false;
            }

            numParms = 1;
        }

        while (stat->nextInChain && numParms < 4)
        {
            if (stat->nextInChain >= itemStats.size())
            {
                // corrupt file
                return false;
            }

            stat = &itemStats[stat->nextInChain];
            if (stat->saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, stat->saveBits))
            {
                // corrupt file
                return false;
            }
            ++numParms;
        }

        id = (std::uint16_t)readBits(charfile, current_bit_offset, 9);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readPropertyList(size_t& current_bit_offset, std::vector<MagicalAttribute>& attrib) const
{
    attrib.clear();
    if (current_bit_offset == 0)
    {
        return false;
    }

    std::uint16_t id = (std::uint16_t)read_uint32_bits(current_bit_offset, 9);
    current_bit_offset += 9;

    while (id != 0x1FF)
    {
        MagicalAttribute magicalAttrib;
        if (id >= itemStats.size())
        {
            // corrupt file
            return false;
        }

        magicalAttrib.Id = id;

        const ItemStat* stat = &itemStats[id];
        magicalAttrib.Name = stat->name;
        magicalAttrib.Desc = stat->desc;

        // saveBits being zero or >= 64 is unrecoverably bad, and
        // encode type 4 is only used by stats that were never implemented (time-based stats)
        if (stat->saveBits == 0 || stat->saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        if (stat->encode == 2)
        {
            if (stat->saveBits != 7 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 6) - stat->saveAdd);
            current_bit_offset += 6;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 10) - stat->saveAdd);
            current_bit_offset += 10;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }
        else if (stat->encode == 3)
        {
            if (stat->saveBits != 16 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 6) - stat->saveAdd);
            current_bit_offset += 6;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 10) - stat->saveAdd);
            current_bit_offset += 10;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 8) - stat->saveAdd);
            current_bit_offset += 8;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 8) - stat->saveAdd);
            current_bit_offset += 8;
        }
        else if (stat->encode == 4)
        {
            if (stat->saveBits != 3 || stat->saveParamBits != 16)
            {
                // time-based stats were never implemented, so it's a corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 3) - stat->saveAdd);
            current_bit_offset += 3;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 13) - stat->saveAdd);
            current_bit_offset += 13;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }
        else if (stat->saveParamBits > 0)
        {
            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveParamBits) - stat->saveAdd);
            current_bit_offset += stat->saveParamBits;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }
        else
        {
            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }

        while (stat->nextInChain && magicalAttrib.Values.size() < 4)
        {
            if (stat->nextInChain >= itemStats.size())
            {
                // corrupt file
                return false;
            }

            stat = &itemStats[stat->nextInChain];
            if (stat->saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }

        attrib.push_back(magicalAttrib);
        id = (std::uint16_t)read_uint32_bits(current_bit_offset, 9);
        current_bit_offset += 9;
    }

    return true;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getEncodedChar(std::FILE* charfile, size_t& current_bit_offset)
{
    std::string bitStr;
    size_t startRead = current_bit_offset;
    size_t readOffset = startRead;
    while (bitStr.size() < 9)
    {
        readOffset = startRead;
        std::stringstream ss2;
        ss2 << std::bitset<9>(readBits(charfile, readOffset, bitStr.size() + 1));
        current_bit_offset = readOffset;
        bitStr = ss2.str().substr(8 - bitStr.size());
        auto iter = huffmanDecodeMap.find(bitStr);
        if (iter != huffmanDecodeMap.end())
        {
            return iter->second;
        }
    }

    // something went wrong
    return 0xFF;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateBits(size_t start, std::uint8_t size, std::uint32_t value)
{
    size_t startIdx = start / 8;
    size_t endIdx = (start + size) / 8;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    size_t startBit = start % 8;
    size_t numBytes = (endIdx - startIdx) + 1;
    if (numBytes > 5)
    {
        // 32 bit value can't consume more then this many bytes
        return false;
    }

    std::uint64_t mask = (((std::uint64_t)1 << size) - 1) << (startBit);
    std::uint64_t dest = 0;
    std::memcpy((std::uint8_t*)&dest, &data[startIdx], numBytes);
    dest = (dest & ~mask) | (((std::uint64_t)value << (startBit)) & mask);
    std::memcpy(&data[startIdx], (std::uint8_t*)&dest, numBytes);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateBits64(size_t start, std::uint8_t size, std::uint64_t value)
{
    size_t startIdx = start / 8;
    size_t endIdx = (start + size) / 8;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    size_t startBit = start % 8;
    size_t numBytes = (endIdx - startIdx) + 1;
    if (numBytes > 8)
    {
        // value is too big for us to handle
        return false;
    }

    std::uint64_t mask = (((std::uint64_t)1 << size) - 1) << (startBit);
    std::uint64_t dest = 0;
    std::memcpy((std::uint8_t*)&dest, &data[startIdx], numBytes);
    dest = (dest & ~mask) | (((std::uint64_t)value << (startBit)) & mask);
    std::memcpy(&data[startIdx], (std::uint8_t*)&dest, numBytes);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateItemCodev115(std::uint64_t code, std::uint8_t numBitsSet)
{
    auto oldNumBitsSet = std::uint8_t(std::int64_t(extended_data_offset) - std::int64_t(type_code_offset));
    if (oldNumBitsSet == numBitsSet)
    {
        // easy change
        return updateBits64(type_code_offset, numBitsSet, code);
    }

    // complex change
    size_t old_current_bit_offset = extended_data_offset;
    size_t bitsToCopy = item_end_bit_offset - extended_data_offset;

    std::int64_t diff = std::int64_t(numBitsSet) - std::int64_t(oldNumBitsSet);
    extended_data_offset += diff;
    item_end_bit_offset += diff;
    size_t current_bit_offset = extended_data_offset;

    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // truncate the original
    size_t newSize = (item_end_bit_offset + 7) / 8;
    data.resize(newSize, 0);
    updateBits64(type_code_offset, numBitsSet, code); // update the bits

    // now copy the remaining bits
    std::uint32_t value = 0;
    std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBits(old_current_bit_offset, bits, value);
        current_bit_offset += bits;
        bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        value = 0;
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBits(old_current_bit_offset, bits, 0);
    }

    if (quest_difficulty_offset != 0)
    {
        quest_difficulty_offset += diff;
    }

    if (nr_of_items_in_sockets_offset != 0)
    {
        nr_of_items_in_sockets_offset += diff;
    }

    // update offsets after the extended_data_offset
    if (!isSimpleItem())
    {
        if (item_id_bit_offset != 0)
        {
            item_id_bit_offset += diff;
        }

        if (item_level_bit_offset != 0)
        {
            item_level_bit_offset += diff;
        }

        if (quality_bit_offset != 0)
        {
            quality_bit_offset += diff;
        }

        if (multi_graphic_bit_offset != 0)
        {
            multi_graphic_bit_offset += diff;
        }

        if (autoAffix_bit_offset != 0)
        {
            autoAffix_bit_offset += diff;
        }

        if (quality_attrib_bit_offset != 0)
        {
            quality_attrib_bit_offset += diff;
        }

        if (runeword_id_bit_offset != 0)
        {
            runeword_id_bit_offset += diff;
        }

        if (personalized_bit_offset != 0)
        {
            personalized_bit_offset += diff;
        }

        if (realm_bit_offset != 0)
        {
            realm_bit_offset += diff;
        }

        if (defense_rating_bit_offset != 0)
        {
            defense_rating_bit_offset += diff;
        }

        if (durability_bit_offset != 0)
        {
            durability_bit_offset += diff;
        }

        if (stackable_bit_offset != 0)
        {
            stackable_bit_offset += diff;
        }

        if (gld_stackable_bit_offset != 0)
        {
            gld_stackable_bit_offset += diff;
        }

        if (socket_count_bit_offset != 0)
        {
            socket_count_bit_offset += diff;
        }

        if (bonus_bits_bit_offset != 0)
        {
            bonus_bits_bit_offset += diff;
        }

        if (magical_props_bit_offset != 0)
        {
            magical_props_bit_offset += diff;
        }

        if (set_bonus_props_bit_offset != 0)
        {
            set_bonus_props_bit_offset += diff;
        }

        if (runeword_props_bit_offset != 0)
        {
            runeword_props_bit_offset += diff;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::findItems()
{
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    ItemLocationReference.clear();
    ItemAltLocationReference.clear();
    HasBeltEquipped = false;
    HasHoradricCube = false;
    EquippedBeltSlots = 0;

    d2ce::EnumItemLocation itemLocation = d2ce::EnumItemLocation::BUFFER;
    d2ce::EnumAltItemLocation itemAltLocation = d2ce::EnumAltItemLocation::UKNOWN;
    std::uint8_t strcode[4] = { 0 };
    for (auto& item : Inventory)
    {
        item.getItemCode(strcode);
        const auto& itemType = getItemTypeHelper(strcode);
        if (itemType.isStackable())
        {
            Stackables.push_back(item);

            if (itemType.isWeapon())
            {
                Weapons.push_back(item);
            }
        }
        else if (itemType.isWeapon())
        {
            Weapons.push_back(item);
        }
        else if (itemType.isArmor())
        {
            Armor.push_back(item);
        }
        else if (itemType.isPotion() || itemType.isSocketFiller())
        {
            GPSs.push_back(item);
        }

        itemLocation = item.getLocation();
        switch (item.getLocation())
        {
        case d2ce::EnumItemLocation::BELT:
            HasBeltEquipped = true;
            ItemLocationReference[itemLocation].push_back(const_cast<d2ce::Item&>(item));
            break;

        case d2ce::EnumItemLocation::EQUIPPED:
            ItemLocationReference[itemLocation].push_back(const_cast<d2ce::Item&>(item));
            if (!HasBeltEquipped)
            {
                if (item.isBelt())
                {
                    HasBeltEquipped = true;
                    EquippedBeltSlots = item.getTotalItemSlots();
                }
            }
            break;

        case d2ce::EnumItemLocation::STORED:
            itemAltLocation = item.getAltPositionId();
            switch (itemAltLocation)
            {
            case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
                HasHoradricCube = true;
                ItemAltLocationReference[itemAltLocation].push_back(const_cast<d2ce::Item&>(item));
                break;

            case d2ce::EnumAltItemLocation::INVENTORY:
            case d2ce::EnumAltItemLocation::STASH:
                ItemAltLocationReference[itemAltLocation].push_back(const_cast<d2ce::Item&>(item));
                if (!HasHoradricCube)
                {
                    HasHoradricCube = item.isHoradricCube();
                }
                break;
            }
        }
    } // end for
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::list<d2ce::Item>& items)
{
    numItems = 0;
    items.clear();
    if (update_locations)
    {
        // find items location
        location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)MIN_START_STATS_POS)
        {
            cur_pos = MIN_START_STATS_POS;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[1])
            {
                continue;
            }

            // found item marker
            std::fread(&numItems, sizeof(numItems), 1, charfile);
            location = std::ftell(charfile);
            break;
        }

        if (location == 0)
        {
            return false;
        }
    }
    else
    {
        if (location == 0)
        {
            return false;
        }

        std::fseek(charfile, location - sizeof(numItems), SEEK_SET);
        std::fread(&numItems, sizeof(numItems), 1, charfile);
    }

    if (numItems > 0)
    {
        if (!fillItemsArray(charfile, location, numItems, items))
        {
            // Corrupt file
            if (update_locations)
            {
                location = 0;
            }
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::fillItemsArray(std::FILE* charfile, std::uint32_t location, std::uint16_t numItems, std::list<d2ce::Item>& items)
{
    std::fseek(charfile, location, SEEK_SET);
    while (items.size() < numItems)
    {
        if (feof(charfile))
        {
            return false;
        }

        items.resize(items.size() + 1);
        auto& item = items.back();
        if (!item.readItem(Version, charfile))
        {
            // corrupt file
            items.pop_back();
            return false;
        }
    }  // end while

    return numItems == items.size() ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readCorpseItems(std::FILE* charfile)
{
    CorpseInfo.clear();
    if (update_locations)
    {
        // find items location
        corpse_location = 0;
        corpse_item_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)MIN_START_STATS_POS)
        {
            cur_pos = MIN_START_STATS_POS;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[1])
            {
                continue;
            }

            // found item marker
            std::fread(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
            if (CorpseInfo.IsDead > 1)
            {
                return false;
            }

            corpse_location = std::ftell(charfile);
            break;
        }

        if (corpse_location == 0)
        {
            return false;
        }
    }
    else
    {
        if (corpse_location == 0)
        {
            return false;
        }

        std::fseek(charfile, corpse_location - sizeof(CorpseInfo.IsDead), SEEK_SET);
        std::fread(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
        if (CorpseInfo.IsDead > 1)
        {
            return false;
        }
    }

    if (CorpseInfo.IsDead > 0)
    {
        std::fread(&CorpseInfo.Unk_b0_4, sizeof(CorpseInfo.Unk_b0_4), 1, charfile);
        std::fread(&CorpseInfo.X, sizeof(CorpseInfo.X), 1, charfile);
        std::fread(&CorpseInfo.Y, sizeof(CorpseInfo.Y), 1, charfile);
        if (!readItems(charfile, corpse_item_location, NumOfCorpseItems, CorpseItems))
        {
            if (update_locations)
            {
                corpse_location = 0;
            }

            return false;
        }
    }
    else
    {
        corpse_item_location = 0;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::readMercItems(std::FILE* charfile)
{
    if (update_locations)
    {
        bool bFoundMercMarker = false;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)MIN_START_STATS_POS)
        {
            cur_pos = MIN_START_STATS_POS;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != MERC_ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != MERC_ITEM_MARKER[1])
            {
                continue;
            }

            // found merc Item marker
            bFoundMercMarker = true;
            break;
        }

        if (!bFoundMercMarker)
        {
            return;
        }
    }

    readItems(charfile, merc_location, NumOfMercItems, MercItems);
    if (merc_location == 0)
    {
        return;
    }

    readGolemItem(charfile);
}
//---------------------------------------------------------------------------
void d2ce::Items::readGolemItem(std::FILE* charfile)
{
    HasGolem = 0;
    GolemItem.clear();
    if (update_locations)
    {
        // find items location
        golem_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)items_location)
        {
            cur_pos = items_location;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != GOLEM_ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != GOLEM_ITEM_MARKER[1])
            {
                continue;
            }

            // found golem item marker (0x464B). 
            std::fread(&HasGolem, sizeof(HasGolem), 1, charfile);
            golem_location = std::ftell(charfile);
            break;
        }

        if (golem_location == 0)
        {
            return;
        }
    }
    else
    {
        if (golem_location == 0)
        {
            return;
        }

        std::fseek(charfile, golem_location - sizeof(HasGolem), SEEK_SET);
        std::fread(&HasGolem, sizeof(HasGolem), 1, charfile);
    }

    if (HasGolem != 0)
    {
        if (!GolemItem.readItem(Version, charfile))
        {
            // Corrupt file
            if (update_locations)
            {
                golem_location = 0;
            }

            GolemItem.clear();
            HasGolem = false;
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeCorpseItems(std::FILE* charfile)
{
    std::fwrite(ITEM_MARKER, sizeof(ITEM_MARKER), 1, charfile);
    if (CorpseInfo.IsDead != 1 || CorpseItems.empty())
    {
        CorpseInfo.clear();
        NumOfCorpseItems = 0;
        CorpseItems.clear();
        corpse_item_location = 0;
    }

    std::fwrite(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
    corpse_location = std::ftell(charfile);

    if (CorpseInfo.IsDead > 0)
    {
        std::fwrite(&CorpseInfo.Unk_b0_4, sizeof(CorpseInfo.Unk_b0_4), 1, charfile);
        std::fwrite(&CorpseInfo.X, sizeof(CorpseInfo.X), 1, charfile);
        std::fwrite(&CorpseInfo.Y, sizeof(CorpseInfo.Y), 1, charfile);

        std::fwrite(ITEM_MARKER, sizeof(ITEM_MARKER), 1, charfile);
        NumOfCorpseItems = (std::uint16_t)CorpseItems.size();
        std::fwrite(&NumOfCorpseItems, sizeof(NumOfCorpseItems), 1, charfile);
        corpse_item_location = std::ftell(charfile);
        for (auto& item : CorpseItems)
        {
            if (!item.writeItem(charfile))
            {
                return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeMercItems(std::FILE* charfile)
{
    std::fwrite(MERC_ITEM_MARKER, sizeof(MERC_ITEM_MARKER), 1, charfile);
    std::fwrite(ITEM_MARKER, sizeof(ITEM_MARKER), 1, charfile);
    NumOfMercItems = (std::uint16_t)MercItems.size();
    std::fwrite(&NumOfMercItems, sizeof(NumOfMercItems), 1, charfile);
    merc_location = std::ftell(charfile);
    for (auto& item : MercItems)
    {
        if (!item.writeItem(charfile))
        {
            return false;
        }
    }

    return writeGolemItem(charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeGolemItem(std::FILE* charfile)
{
    std::fwrite(GOLEM_ITEM_MARKER, sizeof(GOLEM_ITEM_MARKER), 1, charfile);
    HasGolem = GolemItem.size() > 0 ? 1 : 0;
    std::fwrite(&HasGolem, sizeof(HasGolem), 1, charfile);
    golem_location = std::ftell(charfile);
    if (!GolemItem.writeItem(charfile))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(EnumCharVersion version, std::FILE* charfile, bool isExpansion)
{
    Version = version;
    update_locations = items_location == 0 ? true : false;
    isFileExpansionCharacter = isExpansion;
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    if (!readItems(charfile, items_location, NumOfItems, Inventory) || items_location == 0)
    {
        return false;
    }

    if (!readCorpseItems(charfile) || corpse_location == 0)
    {
        return false;
    }

    if (isExpansion)
    {
        readMercItems(charfile);
    }

    update_locations = false;
    findItems();
    return true;
}
//---------------------------------------------------------------------------
// write items in place at offset saved from reasding
bool d2ce::Items::writeItems(std::FILE* charfile, bool isExpansion)
{
    // Write Items
    std::fwrite(ITEM_MARKER, sizeof(ITEM_MARKER), 1, charfile);
    NumOfItems = (std::uint16_t)Inventory.size();
    std::fwrite(&NumOfItems, sizeof(NumOfItems), 1, charfile);
    for (auto& item : Inventory)
    {
        if (!item.writeItem(charfile))
        {
            return false;
        }
    }

    // Write Corpse Items
    if (!writeCorpseItems(charfile))
    {
        return false;
    }

    if (isExpansion)
    {
        if (!writeMercItems(charfile))
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::itemsAsJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const
{
    ss << "\n" << parentIndent << "\"items\": [";
    if (Inventory.empty())
    {
        ss << "]";
    }
    else
    {
        bool bFirstItem = true;
        for (auto& item : Inventory)
        {
            if (bFirstItem)
            {
                bFirstItem = false;
            }
            else
            {
                ss << ",";
            }
            item.asJson(ss, parentIndent, charLevel);
        }
        ss << "\n" << parentIndent << "]";
    }
}
//---------------------------------------------------------------------------
void d2ce::Items::corpseItemsAsJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const
{
    ss << "\n" << parentIndent << "\"corpse_items\": [";
    if (CorpseItems.empty())
    {
        ss << "]";
    }
    else
    {
        bool bFirstItem = true;
        for (auto& item : CorpseItems)
        {
            if (bFirstItem)
            {
                bFirstItem = false;
            }
            else
            {
                ss << ",";
            }
            item.asJson(ss, parentIndent, charLevel);
        }
        ss << "\n" << parentIndent << "]";
    }
}
//---------------------------------------------------------------------------
bool d2ce::Items::mercItemsAsJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const
{
    if (!isFileExpansionCharacter)
    {
        return false;
    }

    ss << "\n" << parentIndent << "\"merc_items\": [";
    if (MercItems.empty())
    {
        ss << "]";
    }
    else
    {
        bool bFirstItem = true;
        for (auto& item : MercItems)
        {
            if (bFirstItem)
            {
                bFirstItem = false;
            }
            else
            {
                ss << ",";
            }
            item.asJson(ss, parentIndent, charLevel);
        }
        ss << "\n" << parentIndent << "]";
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::golemItemAsJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const
{
    if (!HasGolem)
    {
        return false;
    }

    ss << "\n" << parentIndent << "\"golem_item\": {";
    GolemItem.asJson(ss, parentIndent, charLevel, false);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::itemBonusesAsJson(std::stringstream& ss, const std::string& parentIndent) const
{
    std::vector<MagicalAttribute> attribs;
    if (!getItemBonuses(attribs))
    {
        return false;
    }

    ss << "\n" << parentIndent << "\"item_bonuses\": [";
    if (attribs.empty())
    {
        ss << "]";
    }
    else
    {
        bool bFirstItem = true;
        for (auto& attrib : attribs)
        {
            if (bFirstItem)
            {
                bFirstItem = false;
            }
            else
            {
                ss << ",";
            }
            attrib.asJson(ss, parentIndent);
        }
        ss << "\n" << parentIndent << "]";
    }
    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::asJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const
{
    itemsAsJson(ss, parentIndent, charLevel);
    ss << ",";
    corpseItemsAsJson(ss, parentIndent, charLevel);
    ss << ",\n" << parentIndent << "\"is_dead\": " << std::dec << CorpseItems.size();

    {
        std::stringstream ss2;
        if (mercItemsAsJson(ss2, parentIndent, charLevel))
        {
            ss << "," << ss2.str();
        }
    }

    {
        std::stringstream ss2;
        if (golemItemAsJson(ss2, parentIndent, charLevel))
        {
            ss << "," << ss2.str();
        }
    }

    {
        std::stringstream ss2;
        if (itemBonusesAsJson(ss2, parentIndent))
        {
            ss << "," << ss2.str();
        }
    }
}
//---------------------------------------------------------------------------
d2ce::Items::Items()
{
}
//---------------------------------------------------------------------------
d2ce::Items::Items(const Items& other)
{
    *this = other;
}
//---------------------------------------------------------------------------
d2ce::Items::~Items()
{
}
//---------------------------------------------------------------------------
d2ce::Items& d2ce::Items::operator=(const Items& other)
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    Version = other.Version;

    items_location = other.items_location;
    corpse_location = other.corpse_location;
    corpse_item_location = other.corpse_item_location;
    merc_location = other.merc_location;
    golem_location = other.golem_location;

    NumOfItems = other.NumOfItems;
    Inventory = other.Inventory;

    CorpseInfo = other.CorpseInfo;
    NumOfCorpseItems = other.NumOfCorpseItems;
    CorpseItems = other.CorpseItems;

    NumOfMercItems = other.NumOfMercItems;
    MercItems = other.MercItems;

    HasGolem = other.HasGolem;
    GolemItem = other.GolemItem;

    update_locations = other.update_locations;
    isFileExpansionCharacter = other.isFileExpansionCharacter;

    // refetch references to items
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    findItems();
    return *this;
}
d2ce::Items& d2ce::Items::operator=(Items&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    Version = std::exchange(other.Version, APP_CHAR_VERSION);

    items_location = std::exchange(other.items_location, 0);
    corpse_location = std::exchange(other.corpse_location, 0);
    corpse_item_location = std::exchange(other.corpse_item_location, 0);
    merc_location = std::exchange(other.merc_location, 0);
    golem_location = std::exchange(other.golem_location, 0);

    // copy reference to items
    GPSs.swap(other.GPSs);
    other.Inventory.clear();
    Stackables.swap(other.Stackables);
    other.Stackables.clear();
    Armor.swap(other.Armor);
    other.Armor.clear();
    Weapons.swap(other.Weapons);
    other.Weapons.clear();

    NumOfItems = std::exchange(other.NumOfItems, std::uint16_t(0));
    Inventory.swap(other.Inventory);
    other.Inventory.clear();

    CorpseInfo.swap(other.CorpseInfo);
    NumOfCorpseItems = std::exchange(other.NumOfCorpseItems, std::uint16_t(0));
    CorpseItems.swap(other.CorpseItems);
    other.CorpseItems.clear();

    NumOfMercItems = std::exchange(other.NumOfMercItems, std::uint16_t(0));
    MercItems.swap(other.MercItems);
    other.MercItems.clear();

    HasGolem = std::exchange(other.HasGolem, std::uint8_t(0));
    GolemItem.swap(other.GolemItem);
    other.GolemItem.clear();

    update_locations = std::exchange(other.update_locations, true);
    isFileExpansionCharacter = std::exchange(other.isFileExpansionCharacter, false);
    return *this;
}
//---------------------------------------------------------------------------
void d2ce::Items::swap(Items& other)
{
    std::swap(*this, other);
}
//---------------------------------------------------------------------------
void d2ce::Items::clear()
{
    *this = Items();
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-perfect gems.
   Returns false if no gems or all gems are perfect.
*/
bool d2ce::Items::anyUpgradableGems() const
{
    if (GPSs.empty())
    {
        return 0;
    }

    std::uint8_t strcode[4] = { 0 };
    for (auto& item : GPSs)
    {
        item.get().getItemCode(strcode);
        const auto& itemType = getItemTypeHelper(strcode);
        if (itemType.isUpgradableGem())
        {
            return true;
        }
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-Super Health/Mana potions or
                non-Full Rejuvenation potions.
   Returns false if no potions or all potions are the top kind.
*/
bool d2ce::Items::anyUpgradablePotions() const
{
    if (GPSs.empty())
    {
        return false;
    }

    std::uint8_t strcode[4] = { 0 };
    for (auto& item : GPSs)
    {
        item.get().getItemCode(strcode);
        const auto& itemType = getItemTypeHelper(strcode);
        if (itemType.isUpgradablePotion())
        {
            return true;
        }
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns true if there are any non-Full Rejuvenation potions.
   Returns false if no potions or all potions Full Rejuvenation potions.
*/
bool d2ce::Items::anyUpgradableRejuvenations() const
{
    if (GPSs.empty())
    {
        return false;
    }

    std::uint8_t strcode[4] = { 0 };
    for (auto& item : GPSs)
    {
        item.get().getItemCode(strcode);
        const auto& itemType = getItemTypeHelper(strcode);
        if (itemType.isUpgradableRejuvenationPotion() || (itemType.isPotion() && !itemType.isRejuvenationPotion()))
        {
            return true;
        }
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns the number of items in character's inventory.
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Items::getNumberOfItems() const
{
    return Inventory.size();
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfEquippedItems() const
{
    return getEquippedItems().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getEquippedItems() const
{
    return ItemLocationReference[d2ce::EnumItemLocation::EQUIPPED];
}
//---------------------------------------------------------------------------
bool d2ce::Items::getHasBeltEquipped() const
{
    return HasBeltEquipped;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getMaxNumberOfItemsInBelt() const
{
    if (!getHasBeltEquipped())
    {
        return 4;
    }

    return EquippedBeltSlots;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInBelt() const
{
    return getItemsInBelt().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInBelt() const
{
    return ItemLocationReference[d2ce::EnumItemLocation::BELT];
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInInventory() const
{
    return getItemsInInventory().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInInventory() const
{
    return ItemAltLocationReference[d2ce::EnumAltItemLocation::INVENTORY];
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInStash() const
{
    return getItemsInStash().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInStash() const
{
    return ItemAltLocationReference[d2ce::EnumAltItemLocation::STASH];
}
//---------------------------------------------------------------------------
bool d2ce::Items::getHasHoradricCube() const
{
    return HasHoradricCube;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInHoradricCube() const
{
    if (!getHasHoradricCube())
    {
        return 0;
    }

    return getItemsInHoradricCube().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInHoradricCube() const
{
    return ItemAltLocationReference[d2ce::EnumAltItemLocation::HORADRIC_CUBE];
}
//---------------------------------------------------------------------------
// number of bytes to store all item sections
size_t d2ce::Items::getByteSize() const
{
    size_t byteSize = GolemItem.getFullSize();
    for (auto& item : Inventory)
    {
        byteSize += item.getFullSize();
    }

    for (auto& item : CorpseItems)
    {
        byteSize += item.getFullSize();
    }

    for (auto& item : MercItems)
    {
        byteSize += item.getFullSize();
    }

    return byteSize;
}
//---------------------------------------------------------------------------
/*
   Returns the number of Gems, Potions or Skulls in character's inventory.
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Items::getNumberOfGPSs() const
{
    return GPSs.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getGPSs() const
{
    return GPSs;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfStackables() const
{
    return Stackables.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getStackables() const
{
    return Stackables;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfArmor() const
{
    return Armor.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getArmor() const
{
    return Armor;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfWeapons() const
{
    return Weapons.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getWeapons() const
{
    return Weapons;
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& d2ce::Items::getMercItems() const
{
    return MercItems;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getMercItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();

    // Combine all equipped magical properties
    std::vector<MagicalAttribute> tempAttribs;
    std::multimap<size_t, size_t> itemIndexMap;
    for (auto& item : MercItems)
    {
        if (item.getCombinedMagicalAttributes(tempAttribs))
        {
            combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getDisplayedMercItemBonuses(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getMercItemBonuses(attribs))
    {
        return false;
    }

    // check for the "all" cases
    checkForRelatedMagicalAttributes(attribs);

    bool bFormatted = false;
    for (auto& attrib : attribs)
    {
        bFormatted |= formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.begin(), attribs.end(), magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Items::getCombinedMercDefenseRating(std::uint32_t charLevel) const
{
    // Combine all equipped defense ratings
    EnumItemLocation location_id = EnumItemLocation::STORED;
    std::uint64_t eDef = 0;
    std::uint64_t defenseRating = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : MercItems)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            defenseRating += item.getDefenseRating();
            if (item.getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
            {
                for (auto& attrib : magicalAttributes)
                {
                    switch (attrib.Id)
                    {
                    case 16:
                    case 215:
                        eDef += Items::getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 31:
                    case 214:
                        defenseRating += Items::getMagicalAttributeValue(attrib, charLevel, 0);
                        break;
                    }
                }
            }
        }
    }

    defenseRating += (defenseRating * eDef) / 100;
    return std::uint16_t(defenseRating);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getCombinedMercDamage(BaseDamage& damage, std::uint32_t charLevel) const
{
    ItemDamage itemDamage;
    damage.clear();

    EnumItemLocation location_id = EnumItemLocation::STORED;
    std::uint64_t eDmg = 0;
    std::uint64_t eDmgMax = 0;
    std::uint64_t dmgMin = 0;
    std::uint64_t dmgMax = 0;
    std::uint64_t dmgMin2 = 0;
    std::uint64_t dmgMax2 = 0;
    std::uint64_t dmgMinThrow = 0;
    std::uint64_t dmgMaxThrow = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : MercItems)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            if (item.getDamage(itemDamage))
            {
                if (item.isEthereal())
                {
                    itemDamage.OneHanded.Min += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Min) * 50) / 100);
                    itemDamage.OneHanded.Max += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Max) * 50) / 100);
                }
                damage.add(itemDamage.OneHanded);
            }

            if (item.getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
            {
                for (auto& attrib : magicalAttributes)
                {
                    switch (attrib.Id)
                    {
                    case 17:
                        eDmg += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 21:
                        dmgMin += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 22:
                        dmgMax += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 23:
                        dmgMin2 += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMin
                        break;

                    case 24:
                        dmgMax2 += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMax
                        break;

                    case 219:
                        eDmgMax += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 159:
                        dmgMinThrow += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMin
                        break;

                    case 160:
                        dmgMaxThrow += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMax
                        break;
                    }
                }

                if (dmgMin == 0)
                {
                    dmgMin = std::max(dmgMinThrow, dmgMin2);
                }

                if (dmgMax == 0)
                {
                    dmgMax = std::max(dmgMaxThrow, dmgMax2);
                }

                if (dmgMin != 0)
                {
                    damage.Min += std::uint16_t(((dmgMin * eDmg) / 100) + dmgMin);
                }

                if (dmgMax != 0)
                {
                    damage.Max += std::uint16_t( ((dmgMax * (eDmg + eDmgMax)) / 100) + dmgMax);
                }

                if (damage.Min > damage.Max)
                {
                    damage.Max = damage.Min + 1;
                }
            }
        }
    }

    return damage.Max != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Items::hasCorpse() const
{
    return CorpseInfo.IsDead == 1 ? true : false;
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& d2ce::Items::getCorpseItems() const
{
    return CorpseItems;
}
//---------------------------------------------------------------------------
bool d2ce::Items::hasGolem() const
{
    return HasGolem != 0 ? true : false;
}
//---------------------------------------------------------------------------
const d2ce::Item& d2ce::Items::getGolemItem() const
{
    return GolemItem;
}
//---------------------------------------------------------------------------
/*
   Converts the all gems to their perfect state
   Returns the number of gems converted.
*/
size_t d2ce::Items::upgradeGems()
{
    if (GPSs.empty())
    {
        return 0;
    }

    size_t gemsconverted = 0;
    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    for (auto& item : GPSs)
    {
        item.get().getItemCode(strcode);
        switch (gem)
        {
        case 'g':
            switch (gemcondition)
            {
            case 'c': // chipped
            case 'f': // flawed
            case 's': // regular
            case 'l': // flawless
                switch (gemcolour)
                {
                case 'v': // amethyst
                    if (gemcondition != 'l')
                    {
                        gemcondition = 'p'; // perfect
                        if (item.get().updateGem(strcode))
                        {
                            ++gemsconverted;
                        }
                    }
                    break;

                case 'w': // diamond
                case 'g': // emerald
                case 'r': // ruby
                case 'b': // sapphire
                case 'y': // topaz
                    gemcondition = 'p'; // perfect
                    if (item.get().updateGem(strcode))
                    {
                        ++gemsconverted;
                    }
                    break;
                }
                break;

            case 'z': // flawless amethyst
                if (gemcolour == 'v')
                {
                    gemcondition = 'p'; // perfect
                    if (item.get().updateGem(strcode))
                    {
                        ++gemsconverted;
                    }
                }
                break;
            }
            break;

        case 's': // skulls
            if (gemcondition == 'k')
            {
                switch (gemcolour)
                {
                case 'c': // chipped
                case 'f': // flawed
                case 'u': // regular
                case 'l': // flawless
                    gemcolour = 'z'; // perfect
                    if (item.get().updateGem(strcode))
                    {
                        ++gemsconverted;
                    }
                    break;
                }
            }
            break;
        }  // end switch
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to their highest quiality.
   Returns the number of potions converted.
*/
size_t d2ce::Items::upgradePotions()
{
    if (GPSs.empty())
    {
        return 0;
    }

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        item.get().getItemCode(strcode);
        switch (gem)
        {
        case 'r': // rejuvenation potions
            if (gemcondition == 'v' && gemcolour == 's')
            {
                gemcolour = 'l';
                if (item.get().updateGem(strcode))
                {
                    ++gemsconverted;
                }
            }
            break;

        case 'h': // healing potions
        case 'm': // mana potions
            if (gemcondition == 'p' &&
                gemcolour >= '1' && gemcolour <= '4')
            {
                gemcolour = '5';
                if (item.get().updateGem(strcode))
                {
                    ++gemsconverted;
                }
            }
            break;
        }  // end switch
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to Full Rejuvenation potions.
   Returns the number of potions converted.
*/
size_t d2ce::Items::upgradeRejuvenationPotions()
{
    if (GPSs.empty())
    {
        return 0;
    }

    std::uint8_t strcode[4] = { 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        item.get().getItemCode(strcode);
        switch (gem)
        {
        case 'r': // rejuvenation potions
            if (gemcondition == 'v' && gemcolour == 's')
            {
                gemcolour = 'l';
                if (item.get().updateGem(strcode))
                {
                    ++gemsconverted;
                }
            }
            break;

        case 'h': // healing potions
        case 'm': // mana potions
            if (gemcondition == 'p' &&
                gemcolour >= '1' && gemcolour <= '5')
            {
                gem = 'r';
                gemcondition = 'v';
                gemcolour = 'l';
                if (item.get().updateGem(strcode))
                {
                    ++gemsconverted;
                }
            }
            break;
        }  // end switch
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems, potions or skulls to the specified
   final gem, potion or skull.
   Returns the number of gems converted.
*/
size_t d2ce::Items::convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4])
{
    if (GPSs.empty())
    {
        return 0;
    }

    const std::uint8_t& oldgem = existingGem[0];
    const std::uint8_t& oldgemcondition = existingGem[1];
    const std::uint8_t& oldgemcolour = existingGem[2];

    std::uint8_t temp[4] = { 0 };
    std::uint8_t& currentgem = temp[0];
    std::uint8_t& currentgemcondition = temp[1];
    std::uint8_t& currentgemcolour = temp[2];

    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        item.get().getItemCode(temp);
        if (oldgem == currentgem && oldgemcondition == currentgemcondition && oldgemcolour == currentgemcolour)
        {
            // found a match, try to update it
            if (item.get().updateGem(desiredGem))
            {
                ++gemsconverted;
            }
        }
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::fillAllStackables()
{
    size_t itemsFilled = 0;
    for (auto& item : Stackables)
    {
        if (item.get().setMaxQuantity())
        {
            ++itemsFilled;
        }
    }  // end for

    return itemsFilled;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::fixAllItems()
{
    size_t itemsFixed = 0;
    for (auto& item : Armor)
    {
        if (item.get().fixDurability())
        {
            ++itemsFixed;
        }
    }

    for (auto& item : Weapons)
    {
        if (item.get().fixDurability())
        {
            ++itemsFixed;
        }
    }

    for (auto& item : MercItems)
    {
        if (item.fixDurability())
        {
            ++itemsFixed;
        }
    }

    if (HasGolem)
    {
        if (GolemItem.fixDurability())
        {
            ++itemsFixed;
        }
    }

    return itemsFixed;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::maxDurabilityAllItems()
{
    size_t itemsFixed = 0;
    for (auto& item : Armor)
    {
        if (item.get().setMaxDurability())
        {
            ++itemsFixed;
        }
    }

    for (auto& item : Weapons)
    {
        if (item.get().setMaxDurability())
        {
            ++itemsFixed;
        }
    }

    for (auto& item : MercItems)
    {
        if (item.setMaxDurability())
        {
            ++itemsFixed;
        }
    }

    if (HasGolem)
    {
        if (GolemItem.setMaxDurability())
        {
            ++itemsFixed;
        }
    }

    return itemsFixed;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();

    // Combine all equipped magical properties
    std::vector<MagicalAttribute> tempAttribs;
    std::multimap<size_t, size_t> itemIndexMap;
    EnumItemLocation location_id = EnumItemLocation::STORED;
    for (auto& item : Inventory)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            if (item.getCombinedMagicalAttributes(tempAttribs))
            {
                combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getCharmBonuses(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();

    // Combine all charm magical properties
    std::vector<MagicalAttribute> tempAttribs;
    std::multimap<size_t, size_t> itemIndexMap;
    EnumItemLocation location_id = EnumItemLocation::STORED;
    EnumAltItemLocation altLocation_id = EnumAltItemLocation::UKNOWN;
    for (auto& item : Inventory)
    {
        if (!item.isCharm())
        {
            continue;
        }

        location_id = item.getLocation();
        if (location_id != EnumItemLocation::STORED)
        {
            continue;
        }

        altLocation_id = item.getAltPositionId();
        if (altLocation_id == EnumAltItemLocation::INVENTORY)
        {
            if (item.getCombinedMagicalAttributes(tempAttribs))
            {
                combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getItemBonuses(attribs))
    {
        return false;
    }

    // check for the "all" cases
    checkForRelatedMagicalAttributes(attribs);

    bool bFormatted = false;
    for (auto& attrib : attribs)
    {
        bFormatted |= formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.begin(), attribs.end(), magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Items::getCombinedDefenseRating(std::uint32_t charLevel) const
{
    // Combine all equipped defense ratings
    EnumItemLocation location_id = EnumItemLocation::STORED;
    std::uint64_t eDef = 0;
    std::uint64_t defenseRating = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : Inventory)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            defenseRating += item.getDefenseRating();
            if (item.getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
            {
                for (auto& attrib : magicalAttributes)
                {
                    switch (attrib.Id)
                    {
                    case 16:
                    case 215:
                        eDef += Items::getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 31:
                    case 214:
                        defenseRating += Items::getMagicalAttributeValue(attrib, charLevel, 0);
                        break;
                    }
                }
            }
        }
    }

    defenseRating += (defenseRating * eDef) / 100;
    return std::uint16_t(defenseRating);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getCombinedDamage(BaseDamage& damage, std::uint32_t charLevel) const
{
    ItemDamage itemDamage;
    damage.clear();

    EnumItemLocation location_id = EnumItemLocation::STORED;
    std::uint64_t eDmg = 0;
    std::uint64_t eDmgMax = 0;
    std::uint64_t dmgMin = 0;
    std::uint64_t dmgMax = 0;
    std::uint64_t dmgMin2 = 0;
    std::uint64_t dmgMax2 = 0;
    std::uint64_t dmgMinThrow = 0;
    std::uint64_t dmgMaxThrow = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : Inventory)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            if (item.getDamage(itemDamage))
            {
                if (item.isEthereal())
                {
                    itemDamage.OneHanded.Min += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Min) * 50) / 100);
                    itemDamage.OneHanded.Max += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Max) * 50) / 100);
                }
                damage.add(itemDamage.OneHanded);
            }

            if (item.getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
            {
                for (auto& attrib : magicalAttributes)
                {
                    switch (attrib.Id)
                    {
                    case 17:
                        eDmg += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 21:
                        dmgMin += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 22:
                        dmgMax += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 23:
                        dmgMin2 += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMin
                        break;

                    case 24:
                        dmgMax2 += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMax
                        break;

                    case 219:
                        eDmgMax += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 159:
                        dmgMinThrow += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMin
                        break;

                    case 160:
                        dmgMaxThrow += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMax
                        break;
                    }
                }

                if (dmgMin == 0)
                {
                    dmgMin = std::max(dmgMinThrow, dmgMin2);
                }

                if (dmgMax == 0)
                {
                    dmgMax = std::max(dmgMaxThrow, dmgMax2);
                }

                if (dmgMin != 0)
                {
                    damage.Min += std::uint16_t(((dmgMin * eDmg) / 100) + dmgMin);
                }

                if (dmgMax != 0)
                {
                    damage.Max += std::uint16_t(((dmgMax * (eDmg + eDmgMax)) / 100) + dmgMax);
                }

                if (damage.Min > damage.Max)
                {
                    damage.Max = damage.Min + 1;
                }
            }
        }
    }

    return damage.Max != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::int64_t d2ce::Items::getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx)
{
    const auto& stat = itemStats[attrib.Id];
    return d2ce::getMagicalAttributeValue(attrib, charLevel, idx, stat);
}
//---------------------------------------------------------------------------