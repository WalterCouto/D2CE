/*
    Diablo II Character Editor
    Copyright (C) 2021-2022 Walter Couto

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
#include <random>
#include "ItemHelpers.h"
#include "Item.h"
#include "SkillConstants.h"

//---------------------------------------------------------------------------
namespace d2ce
{
#define read_uint32_bits(start,size) \
    ((*((std::uint32_t *) &data[(start) / 8]) >> ((start) & 7))& (((std::uint32_t)1 << (size)) - 1))

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
        { 49, 0,  9,   0,  0,  0,  0,                                          "+{0} to Maximum Fire Damage",                      "firemaxdam"},
        { 50, 0,  6,   0,  0, 51,  0,                                        "Adds {0}-{1} Lightning Damage",                     "lightmindam"},
        { 51, 0, 10,   0,  0,  0,  0,                                     "+{0} to Maximum Lightning Damage",                     "lightmaxdam"},
        { 52, 0,  8,   0,  0, 53,  0,                                            "Adds {0}-{1} Magic Damage",                     "magicmindam"},
        { 53, 0,  9,   0,  0,  0,  0,                                         "+{0} to Maximum Magic Damage",                     "magicmaxdam"},
        { 54, 0,  8,   0,  0, 55,  0,                                             "Adds {0}-{1} Cold Damage",                      "coldmindam"},
        { 55, 0,  9,   0,  0, 56,  0,                                          "+{0} to Maximum Cold Damage",                      "coldmaxdam"},
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
        {205, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused205"},
        {206, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused206"},
        {207, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused207"},
        {208, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused208"},
        {209, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused209"},
        {210, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused210"},
        {211, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused211"},
        {212, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused212"},
        {213, 3,  0,   0,  0,  0,  0,                                                                     "",                       "unused213"},
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

    ItemType s_invalidItemType;
    const ItemType& getShieldItemType(const std::array<std::uint8_t, 4>& strcode)
    {
        static const std::map<std::string, ItemType> shieldBases = {
            {"buc", {"Buckler", {{ 0, 0 }, false, false, { 0, 0 }}, {12, 0}, {2, 2}, false, "invbuc", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {1}}},
            {"sml", {"Small Shield", {{0, 0}, false, false, {0, 0}}, {22, 0}, {2, 2}, false, "invsml", 5, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {2}}},
            {"lrg", {"Large Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {34, 0}, {2, 3}, false, "invlrg", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"kit", {"Kite Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {47, 0}, {2, 3}, false, "invkit", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"tow", {"Tower Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {75, 0}, {2, 3}, false, "invtow", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"gts", {"Gothic Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {60, 0}, {2, 4}, false, "invgts", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"bsh", {"Bone Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0}, {2, 3}, false, "invbsh", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {2}}},
            {"spk", {"Spiked Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {30, 0}, {2, 3}, false, "invspk", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {2}}},
            {"xuc", {"Defender", {{ 0, 0 }, false, false, { 0, 0 }}, {38, 0, 22}, {2, 2}, false, "invbuc", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {1}}},
            {"xml", {"Round Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {53, 0, 25}, {2, 2}, false, "invsml", 5, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {2}}},
            {"xrg", {"Scutum", {{ 0, 0 }, false, false, { 0, 0 }}, {71, 0, 25}, {2, 3}, false, "invlrg", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"xit", {"Dragon Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {91, 0, 25}, {2, 3}, false, "invkit", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"xow", {"Pavise", {{ 0, 0 }, false, false, { 0, 0 }}, {133, 0, 25}, {2, 3}, false, "invtow", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"xts", {"Ancient Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {110, 0, 25}, {2, 4}, false, "invgts", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"xsh", {"Grim Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 3}, false, "invbsh", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {2}}},
            {"xpk", {"Barbed Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 25}, {2, 3}, false, "invspk", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {2}}},
            {"uuc", {"Heater", {{ 0, 0 }, false, false, { 0, 0 }}, {77, 0, 43}, {2, 2}, false, "invbuc", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {2}}},
            {"uml", {"Luna", {{ 0, 0 }, false, false, { 0, 0 }}, {100, 0, 45}, {2, 2}, false, "invsml", 5, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {2}}},
            {"urg", {"Hyperion", {{ 0, 0 }, false, false, { 0, 0 }}, {127, 0, 48}, {2, 3}, false, "invlrg", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"uit", {"Monarch", {{ 0, 0 }, false, false, { 0, 0 }}, {156, 0, 54}, {2, 3}, false, "invkit", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3, 3, 4}}},
            {"uow", {"Aegis", {{ 0, 0 }, false, false, { 0, 0 }}, {219, 0, 59}, {2, 3}, false, "invtow", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3, 3, 4}}},
            {"uts", {"Ward", {{ 0, 0 }, false, false, { 0, 0 }}, {185, 0, 63}, {2, 5}, false, "invgts", 2, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3, 3, 4}}},
            {"ush", {"Troll Nest", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 57}, {2, 3}, false, "invbsh", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"upk", {"Blade Barrier", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 51}, {2, 3}, false, "invspk", 8, {"Shield", "Any Shield", "Any Armor", "Second Hand"}, {3}}},
            {"pa1", {"Targe", {{ 0, 0 }, false, false, { 0, 0 }}, {16, 0, 3}, {2, 2}, false, "invpa1", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"pa2", {"Rondache", {{ 0, 0 }, false, false, { 0, 0 }}, {26, 0, 6}, {2, 2}, false, "invpa2", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"pa3", {"Heraldic Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {40, 0, 12}, {2, 4}, false, "invpa3", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"pa4", {"Aerin Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 15}, {2, 4}, false,"invpa4", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"pa5", {"Crown Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 18}, {2, 2}, false, "invpa5", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"ne1", {"Preserved Head", {{ 0, 0 }, false, false, { 0, 0 }}, {12, 0, 3}, {2, 2}, false, "invne1", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"ne2", {"Zombie Head", {{ 0, 0 }, false, false, { 0, 0 }}, {14, 0, 6}, {2, 2}, false, "invne2", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"ne3", {"Unraveller Head", {{ 0, 0 }, false, false, { 0, 0 }}, {18, 0, 12}, {2, 2}, false, "invne3", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"ne4", {"Gargoyle Head", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 15}, {2, 2}, false, "invne4", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"ne5", {"Demon Head", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0, 18}, {2, 2}, false, "invne5", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"pa6", {"Akaran Targe", {{ 0, 0 }, false, false, { 0, 0 }}, {44, 0, 26}, {2, 2}, false, "invpa1", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4, 4}}},
            {"pa7", {"Akaran Rondache", {{ 0, 0 }, false, false, { 0, 0 }}, {59, 0, 30}, {2, 2}, false, "invpa2", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"pa8", {"Protector Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {69, 0, 34}, {2, 4}, false, "invpa3", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"pa9", {"Guilded Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {89, 0, 38}, {2, 4}, false, "invpa4", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"paa", {"Royal Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {114, 0, 41}, {2, 2}, false, "invpa5", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"ne6", {"Mummified Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {38, 0, 24}, {2, 2}, false, "invne1", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"ne7", {"Fetish Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {41, 0, 29}, {2, 2}, false, "invne2", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"ne8", {"Sexton Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {47, 0, 33}, {2, 2}, false, "invne3", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"ne9", {"Cantor Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 36}, {2, 2}, false, "invne4", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"nea", {"Heirophant Trophy", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 40}, {2, 2}, false, "invne5", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"pab", {"Sacred Targe", {{ 0, 0 }, false, false, { 0, 0 }}, {86, 0, 47}, {2, 2}, false, "invpa1", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"pac", {"Sacred Rondache", {{ 0, 0 }, false, false, { 0, 0 }}, {109, 0, 52}, {2, 2}, false, "invpa2", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"pad", {"Ancient Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {124, 0, 55}, {2, 4}, false, "invpa3", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3}}},
            {"pae", {"Zakarum Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {142, 0, 61}, {2, 4}, false, "invpa4", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"paf", {"Vortex Shield", {{ 0, 0 }, false, false, { 0, 0 }}, {148, 0, 66}, {2, 2}, false, "invpa5", 0, {"Auric Shields", "Any Shield", "Any Armor", "Second Hand", "Paladin Item", "Class Specific"}, {3, 4}}},
            {"neb", {"Minion Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {77, 0, 44}, {2, 2}, false, "invne1", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"nec", {"Hellspawn Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {82, 0, 50}, {2, 2}, false, "invne2", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"ned", {"Overseer Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {91, 0, 49}, {2, 2}, false, "invne3", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"nee", {"Succubae Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {95, 0, 60}, {2, 2}, false, "invne4", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
            {"nef", {"Bloodlord Skull", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 65}, {2, 2}, false, "invne5", 8,{"Voodoo Heads", "Any Shield", "Any Armor", "Second Hand", "Necromancer Item", "Class Specific"}, {2}}},
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

    const ItemType& getArmorItemType(const std::array<std::uint8_t, 4>& strcode)
    {
        static const std::map<std::string, ItemType> armorBases = {
            {"cap", {"Cap", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {2, 2}, false, "invcap", 8, {"Helm", "Any Armor"}, {2}}},
            {"skp", {"Skull Cap", {{ 0, 0 }, false, false, { 0, 0 }}, {15, 0}, {2, 2}, false, "invskp", 8, {"Helm", "Any Armor"}, {2}}},
            {"hlm", {"Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {26, 0}, {2, 2}, false, "invhlm", 8, {"Helm", "Any Armor"}, {2}}},
            {"fhl", {"Full Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {41, 0}, {2, 2}, false, "invfhl", 8, {"Helm", "Any Armor"}, {2}}},
            {"ghm", {"Great Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {63, 0}, {2, 2}, false, "invghm", 8, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"crn", {"Crown", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0}, {2, 2}, false, "invcrn", 8, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"msk", {"Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {23, 0}, {2, 2}, false, "invmsk", 2, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"qui", {"Quilted Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {12, 0}, {2, 3}, false, "invqlt", 8, {"Armor", "Any Armor"}, {2}}},
            {"lea", {"Leather Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {15, 0}, {2, 3}, false, "invlea", 8, {"Armor", "Any Armor"}, {2}}},
            {"hla", {"Hard Leather Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0}, {2, 3}, false, "invhla", 8, {"Armor", "Any Armor"}, {2}}},
            {"stu", {"Studded Leather", {{ 0, 0 }, false, false, { 0, 0 }}, {27, 0}, {2, 3}, false, "invstu", 8, {"Armor", "Any Armor"}, {2}}},
            {"rng", {"Ring Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {36, 0}, {2, 3}, false, "invrng", 8, {"Armor", "Any Armor"}, {3}}},
            {"scl", {"Scale Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {44, 0}, {2, 3}, false, "invscl", 8, {"Armor", "Any Armor"}, {2}}},
            {"chn", {"Chain Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {48, 0}, {2, 3}, false, "invchn", 8, {"Armor", "Any Armor"}, {2}}},
            {"brs", {"Breast Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {30, 0}, {2, 3}, false, "invbrs", 8, {"Armor", "Any Armor"}, {3}}},
            {"spl", {"Splint Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {51, 0}, {2, 3}, false, "invspl", 8, {"Armor", "Any Armor"}, {2}}},
            {"plt", {"Plate Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0}, {2, 3}, false, "invplt", 8, {"Armor", "Any Armor"}, {2}}},
            {"fld", {"Field Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0}, {2, 3}, false, "invfld", 8, {"Armor", "Any Armor"}, {2}}},
            {"gth", {"Gothic Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {70, 0}, {2, 3}, false, "invgth", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"ful", {"Full Plate Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {80, 0}, {2, 3}, false, "invful", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"aar", {"Ancient Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {100, 0}, {2, 3}, false, "invaar", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"ltp", {"Light Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {41, 0}, {2, 3}, false, "invltp", 8, {"Armor", "Any Armor"}, {3}}},
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
            {"bhm", {"Bone Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0}, {2, 2}, false, "invbhm", 8, {"Helm", "Any Armor"}, {2}}},
            {"xap", {"War Hat", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 22}, {2, 2}, false, "invcap", 8, {"Helm", "Any Armor"}, {2}}},
            {"xkp", {"Sallet", {{ 0, 0 }, false, false, { 0, 0 }}, {43, 0, 25}, {2, 2}, false, "invskp", 8, {"Helm", "Any Armor"}, {2}}},
            {"xlm", {"Casque", {{ 0, 0 }, false, false, { 0, 0 }}, {59, 0, 25}, {2, 2}, false, "invhlm", 8, {"Helm", "Any Armor"}, {2}}},
            {"xhl", {"Basinet", {{ 0, 0 }, false, false, { 0, 0 }}, {82, 0, 25}, {2, 2}, false, "invfhl", 8, {"Helm", "Any Armor"}, {2}}},
            {"xhm", {"Winged Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {115, 0, 25}, {2, 2}, false, "invghm", 8, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"xrn", {"Grand Crown", {{ 0, 0 }, false, false, { 0, 0 }}, {103, 0, 25}, {2, 2}, false, "invcrn", 8, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"xsk", {"Death Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0, 25}, {2, 2}, false, "invmsk", 2, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"xui", {"Ghost Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {38, 0, 22}, {2, 3}, false, "invqlt", 8, {"Armor", "Any Armor"}, {2}}},
            {"xea", {"Serpentskin Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {43, 0, 24}, {2, 3}, false, "invlea", 8, {"Armor", "Any Armor"}, {2}}},
            {"xla", {"Demonhide Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 25}, {2, 3}, false, "invhla", 8, {"Armor", "Any Armor"}, {2}}},
            {"xtu", {"Trellised Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {61, 0, 25}, {2, 3}, false, "invstu", 8, {"Armor", "Any Armor"}, {2}}},
            {"xng", {"Linked Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {74, 0, 25}, {2, 3}, false, "invrng", 8, {"Armor", "Any Armor"}, {3}}},
            {"xcl", {"Tigulated Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {86, 0, 25}, {2, 3}, false, "invscl", 8, {"Armor", "Any Armor"}, {3}}},
            {"xhn", {"Mesh Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {92, 0, 25}, {2, 3}, false, "invchn", 8, {"Armor", "Any Armor"}, {3}}},
            {"xrs", {"Cuirass", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 25}, {2, 3}, false, "invbrs", 8, {"Armor", "Any Armor"}, {3}}},
            {"xpl", {"Russet Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {97, 0, 25}, {2, 3}, false, "invspl", 8, {"Armor", "Any Armor"}, {3}}},
            {"xlt", {"Templar Coat", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 25}, {2, 3}, false, "invplt", 8, {"Armor", "Any Armor"}, {3}}},
            {"xld", {"Sharktooth Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {103, 0, 25}, {2, 3}, false, "invfld", 8, {"Armor", "Any Armor"}, {3}}},
            {"xth", {"Embossed Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {125, 0, 25}, {2, 3}, false, "invgth", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"xul", {"Chaos Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {140, 0, 25}, {2, 3}, false, "invful", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"xar", {"Ornate Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {170, 0, 25}, {2, 3}, false, "invaar", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"xtp", {"Mage Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0, 25}, {2, 3}, false, "invltp", 8, {"Armor", "Any Armor"}, {3}}},
            {"xlg", {"Demonhide Gloves", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 21}, {2, 2}, false, "invlgl", 8, {"Gloves", "Any Armor"}}},
            {"xvg", {"Sharkskin Gloves", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 25}, {2, 2}, false, "invvgl", 8, {"Gloves", "Any Armor"}}},
            {"xmg", {"Heavy Bracers", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 2}, false, "invmgl", 8, {"Gloves", "Any Armor"}}},
            {"xtg", {"Battle Gauntlets", {{ 0, 0 }, false, false, { 0, 0 }}, {88, 0, 25}, {2, 2}, false, "invtgl", 8, {"Gloves", "Any Armor"}}},
            {"xhg", {"War Gauntlets", {{ 0, 0 }, false, false, { 0, 0 }}, {110, 0, 25}, {2, 2}, false, "invhgl", 8, {"Gloves", "Any Armor"}}},
            {"xlb", {"Demonhide Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 24}, {2, 2}, false, "invlbt", 8, {"Boots", "Any Armor"}}},
            {"xvb", {"Sharkskin Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {47, 0, 25}, {2, 2}, false, "invvbt", 8, {"Boots", "Any Armor"}}},
            {"xmb", {"Mesh Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 2}, false, "invmbt", 8, {"Boots", "Any Armor"}}},
            {"xtb", {"Battle Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {95, 0, 25}, {2, 2}, false, "invtbt", 8, {"Boots", "Any Armor"}}},
            {"xhb", {"War Boots", {{ 0, 0 }, false, false, { 0, 0 }}, {125, 0, 25}, {2, 2}, false, "invhbt", 8, {"Boots", "Any Armor"}}},
            {"zlb", {"Demonhide Sash", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 24}, {2, 1, 4, 3}, false, "invlbl", 8, {"Belt", "Any Armor"}}},
            {"zvb", {"Sharkskin Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 25}, {2, 1, 4, 3}, false, "invvbl", 8, {"Belt", "Any Armor"}}},
            {"zmb", {"Mesh Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 1, 4, 3}, false, "invmbl", 8, {"Belt", "Any Armor"}}},
            {"ztb", {"Battle Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {88, 0, 25}, {2, 1, 4, 3}, false, "invtbl", 8, {"Belt", "Any Armor"}}},
            {"zhb", {"War Belt", {{ 0, 0 }, false, false, { 0, 0 }}, {110, 0, 25}, {2, 1, 4, 3}, false, "invhbl", 8, {"Belt", "Any Armor"}}},
            {"xh9", {"Grim Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 2}, false, "invbhm", 8, {"Helm", "Any Armor"}, {2}}},
            {"dr1", {"Wolf Head", {{ 0, 0 }, false, false, { 0, 0 }}, {16, 0, 3}, {2, 2}, false, "invdr1", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dr2", {"Hawk Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {20, 0, 6}, {2, 2}, false, "invdr2", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dr3", {"Antlers", {{ 0, 0 }, false, false, { 0, 0 }}, {24, 0, 12}, {2, 2}, false, "invdr3", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dr4", {"Falcon Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {28, 0, 15}, {2, 2}, false, "invdr4", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dr5", {"Spirit Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {30, 0, 18}, {2, 2}, false, "invdr5", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"ba1", {"Jawbone Cap", {{ 0, 0 }, false, false, { 0, 0 }}, {25, 0, 3}, {2, 2}, false, "invba1", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"ba2", {"Fanged Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {35, 0, 6}, {2, 2}, false, "invba2", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"ba3", {"Horned Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {45, 0, 12}, {2, 2}, false, "invba3", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"ba4", {"Assault Helmet", {{ 0, 0 }, false, false, { 0, 0 }}, {55, 0, 15}, {2, 2}, false, "invba4", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"ba5", {"Avenger Guard", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 18}, {2, 2}, false, "invba5", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"ci0", {"Circlet", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 16}, {2, 2}, false, "invci0", 2, {"Circlet", "Helm", "Any Armor"}, {1, 2}}},
            {"ci1", {"Coronet", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 39}, {2, 2}, false, "invci1", 2, {"Circlet", "Helm", "Any Armor"}, {1, 2}}},
            {"ci2", {"Tiara", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 52}, {2, 2}, false, "invci2", 2, {"Circlet", "Helm", "Any Armor"}, {1, 2, 3}}},
            {"ci3", {"Diadem", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0, 64}, {2, 2}, false, "invci3", 2, {"Circlet", "Helm", "Any Armor"}, {1, 2, 3}}},
            {"uap", {"Shako", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 43}, {2, 2}, false, "invcap", 8, {"Helm", "Any Armor"}, {2}}},
            {"ukp", {"Hydraskull", {{ 0, 0 }, false, false, { 0, 0 }}, {84, 0, 47}, {2, 2}, false, "invskp", 8, {"Helm", "Any Armor"}, {2}}},
            {"ulm", {"Armet", {{ 0, 0 }, false, false, { 0, 0 }}, {109, 0, 51}, {2, 2}, false, "invhlm", 8, {"Helm", "Any Armor"}, {2}}},
            {"uhl", {"Giant Conch", {{ 0, 0 }, false, false, { 0, 0 }}, {142, 0, 40}, {2, 2}, false, "invfhl", 8, {"Helm", "Any Armor"}, {2}}},
            {"uhm", {"Spired Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {192, 0, 59}, {2, 2}, false, "invghm", 8, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"urn", {"Corona", {{ 0, 0 }, false, false, { 0, 0 }}, {174, 0, 66}, {2, 2}, false, "invcrn", 8, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"usk", {"Demonhead", {{ 0, 0 }, false, false, { 0, 0 }}, {102, 0, 55}, {2, 2}, false, "invmsk", 2, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"uui", {"Dusk Shroud", {{ 0, 0 }, false, false, { 0, 0 }}, {77, 0, 49}, {2, 3}, false, "invqlt", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"uea", {"Wyrmhide", {{ 0, 0 }, false, false, { 0, 0 }}, {84, 0, 50}, {2, 3}, false, "invlea", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"ula", {"Scarab Husk", {{ 0, 0 }, false, false, { 0, 0 }}, {95, 0, 51}, {2, 3}, false, "invhla", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"utu", {"Wire Fleece", {{ 0, 0 }, false, false, { 0, 0 }}, {111, 0, 53}, {2, 3}, false, "invstu", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"ung", {"Diamond Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {131, 0, 54}, {2, 3}, false, "invrng", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"ucl", {"Loricated Mail", {{ 0, 0 }, false, false, { 0, 0 }}, {149, 0, 55}, {2, 3}, false, "invscl", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"uhn", {"Boneweave", {{ 0, 0 }, false, false, { 0, 0 }}, {158, 0, 47}, {2, 3}, false, "invchn", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"urs", {"Great Hauberk", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 56}, {2, 3}, false, "invbrs", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"upl", {"Balrog Skin", {{ 0, 0 }, false, false, { 0, 0 }}, {165, 0, 57}, {2, 3}, false, "invspl", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"ult", {"Hellforged Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {196, 0, 59}, {2, 3}, false, "invplt", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"uld", {"Kraken Shell", {{ 0, 0 }, false, false, { 0, 0 }}, {174, 0, 61}, {2, 3}, false, "invfld", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"uth", {"Lacquered Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {208, 0, 62}, {2, 3}, false, "invgth", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"uul", {"Shadow Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {230, 0, 64}, {2, 3}, false, "invful", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"uar", {"Sacred Armor", {{ 0, 0 }, false, false, { 0, 0 }}, {232, 0, 66}, {2, 3}, false, "invaar", 8, {"Armor", "Any Armor"}, {3, 4}}},
            {"utp", {"Archon Plate", {{ 0, 0 }, false, false, { 0, 0 }}, {103, 0, 63}, {2, 3}, false, "invltp", 8, {"Armor", "Any Armor"}, {3, 4}}},
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
            {"uh9", {"Bone Visage", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 63}, {2, 2}, false, "invbhm", 8, {"Helm", "Any Armor"}, {2, 2, 3}}},
            {"dr6", {"Alpha Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {44, 0, 26}, {2, 2}, false, "invdr1", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dr7", {"Griffon Headress", {{ 0, 0 }, false, false, { 0, 0 }}, {50, 0, 30}, {2, 2}, false, "invdr2", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dr8", {"Hunter's Guise", {{ 0, 0 }, false, false, { 0, 0 }}, {56, 0, 29}, {2, 2}, false, "invdr3", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dr9", {"Sacred Feathers", {{ 0, 0 }, false, false, { 0, 0 }}, {62, 0, 32}, {2, 2}, false, "invdr4", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dra", {"Totemic Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {65, 0, 41}, {2, 2}, false, "invdr5", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"ba6", {"Jawbone Visor", {{ 0, 0 }, false, false, { 0, 0 }}, {58, 0, 25}, {2, 2}, false, "invba1", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"ba7", {"Lion Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {73, 0, 29}, {2, 2}, false, "invba2", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"ba8", {"Rage Mask", {{ 0, 0 }, false, false, { 0, 0 }}, {88, 0, 29}, {2, 2}, false, "invba3", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"ba9", {"Savage Helmet", {{ 0, 0 }, false, false, { 0, 0 }}, {103, 0, 32}, {2, 2}, false, "invba4", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"baa", {"Slayer Guard", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 40}, {2, 2}, false, "invba5", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"drb", {"Blood Spirt", {{ 0, 0 }, false, false, { 0, 0 }}, {86, 0, 46}, {2, 2}, false, "invdr1", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"drc", {"Sun Spirit", {{ 0, 0 }, false, false, { 0, 0 }}, {95, 0, 51}, {2, 2}, false, "invdr2", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"drd", {"Earth Spirit", {{ 0, 0 }, false, false, { 0, 0 }}, {104, 0, 57}, {2, 2}, false, "invdr3", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"dre", {"Sky Spirit", {{ 0, 0 }, false, false, { 0, 0 }}, {113, 0, 62}, {2, 2}, false, "invdr4", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"drf", {"Dream Spirit", {{ 0, 0 }, false, false, { 0, 0 }}, {118, 0, 66}, {2, 2}, false, "invdr5", 8, {"Pelt", "Helm", "Any Armor", "Druid Item", "Class Specific"}, {2, 3}}},
            {"bab", {"Carnage Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {106, 0, 45}, {2, 2}, false, "invba1", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"bac", {"Fury Visor", {{ 0, 0 }, false, false, { 0, 0 }}, {129, 0, 49}, {2, 2}, false, "invba2", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"bad", {"Destroyer Helm", {{ 0, 0 }, false, false, { 0, 0 }}, {151, 0, 54}, {2, 2}, false, "invba3", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"bae", {"Conquerer Crown", {{ 0, 0 }, false, false, { 0, 0 }}, {174, 0, 60}, {2, 2}, false, "invba4", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
            {"baf", {"Guardian Crown", {{ 0, 0 }, false, false, { 0, 0 }}, {196, 0, 65}, {2, 2}, false, "invba5", 8, {"Primal Helm", "Helm", "Any Armor", "Barbarian Item", "Class Specific"}, {2, 3}}},
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

    const ItemType& getWeaponItemType(const std::array<std::uint8_t, 4>& strcode)
    {
        static const std::map<std::string, ItemType> weaponBases = {
            {"hax", {"Hand Axe", {{ 3, 6 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invhax", 2, {"Axe", "Melee Weapon", "Weapon"}, {2}}},
            {"axe", {"Axe", {{ 4, 11 }, false, false, { 0, 0 }}, {32, 0}, {2, 3}, false, "invaxe", 2, {"Axe", "Melee Weapon", "Weapon"}, {4}}},
            {"2ax", {"Double Axe", {{ 5, 13 }, false, false, { 0, 0 }}, {43, 0}, {2, 3}, false, "inv2ax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"mpi", {"Military Pick", {{ 7, 11 }, false, false, { 0, 0 }}, {49, 33}, {2, 3}, false, "invmpi", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"wax", {"War Axe", {{ 10, 18 }, false, false, { 0, 0 }}, {67, 0}, {2, 3}, false, "invwax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"lax", {"Large Axe", {{ 0, 0 }, false, true, { 6, 13 }}, {35, 0}, {2, 3}, false, "invlax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4}}},
            {"bax", {"Broad Axe", {{ 0, 0 }, false, true, { 10, 18 }}, {48, 0}, {2, 3}, false, "invbrx", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"btx", {"Battle Axe", {{ 0, 0 }, false, true, { 12, 32 }}, {54, 0}, {2, 3}, false, "invbtx", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"gax", {"Great Axe", {{ 0, 0 }, false, true, { 9, 30 }}, {63, 39}, {2, 4}, false, "invgax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"gix", {"Giant Axe", {{ 0, 0 }, false, true, { 22, 45 }}, {70, 0}, {2, 3}, false, "invgix", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"wnd", {"Wand", {{ 2, 4 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invwnd", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {1}}},
            {"ywn", {"Yew Wand", {{ 2, 8 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invywn", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {1}}},
            {"bwn", {"Bone Wand", {{ 3, 7 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invbwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"gwn", {"Grim Wand", {{ 5, 11 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invgwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"clb", {"Club", {{ 1, 6 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invclb", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"scp", {"Scepter", {{ 6, 11 }, false, false, { 0, 0 }}, {25, 0}, {1, 3}, false, "invscp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"gsc", {"Grand Scepter", {{ 8, 18 }, false, false, { 0, 0 }}, {37, 0}, {1, 3}, false, "invgsc", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"wsp", {"War Scepter", {{ 10, 17 }, false, false, { 0, 0 }}, {55, 0}, {1, 3}, false, "invwsp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3, 5}}},
            {"spc", {"Spiked Club", {{ 5, 8 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invspc", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"mac", {"Mace", {{ 3, 10 }, false, false, { 0, 0 }}, {27, 0}, {1, 3}, false, "invmac", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"mst", {"Morning Star", {{ 7, 16 }, false, false, { 0, 0 }}, {36, 0}, {1, 3}, false, "invmst", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"fla", {"Flail", {{ 1, 24 }, false, false, { 0, 0 }}, {41, 35}, {2, 3}, false, "invfla", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"whm", {"War Hammer", {{ 19, 29 }, false, false, { 0, 0 }}, {53, 0}, {2, 3}, false, "invwhm", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"mau", {"Maul", {{ 0, 0 }, false, true, { 30, 43 }}, {69, 0}, {2, 4}, false, "invmau", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"gma", {"Great Maul", {{ 0, 0 }, false, true, { 38, 58 }}, {99, 0}, {2, 3}, false, "invgma", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"ssd", {"Short Sword", {{ 2, 7 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invssd", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"scm", {"Scimitar", {{ 2, 6 }, false, false, { 0, 0 }}, {0, 21}, {1, 3}, false, "invscm", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"sbr", {"Saber", {{ 3, 8 }, false, false, { 0, 0 }}, {25, 25}, {1, 3}, false, "invsbr", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"flc", {"Falchion", {{ 9, 17 }, false, false, { 0, 0 }}, {33, 0}, {1, 3}, false, "invflc", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"crs", {"Crystal Sword", {{ 5, 15 }, false, false, { 0, 0 }}, {43, 0}, {2, 3}, false, "invcrs", 8, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"bsd", {"Broad Sword", {{ 7, 14 }, false, false, { 0, 0 }}, {48, 0}, {2, 3}, false, "invbsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"lsd", {"Long Sword", {{ 3, 19 }, false, false, { 0, 0 }}, {55, 39}, {2, 3}, false, "invlsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"wsd", {"War Sword", {{ 8, 20 }, false, false, { 0, 0 }}, {71, 45}, {1, 3}, false, "invwsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3}}},
            {"2hs", {"Two-Handed Sword", {{ 2, 9 }, true, true, { 8, 17 }}, {35, 27}, {1, 4}, false, "inv2hs", 2, {"Sword", "Melee Weapon", "Weapon"}, {3}}},
            {"clm", {"Claymore", {{ 5, 12 }, true, true, { 13, 30 }}, {47, 0}, {1, 4}, false, "invclm", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"gis", {"Giant Sword", {{ 3, 16 }, true, true, { 9, 28 }}, {56, 34}, {1, 4}, false, "invgis", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"bsw", {"Bastard Sword", {{ 7, 19 }, true, true, { 20, 28 }}, {62, 0}, {1, 4}, false, "invbsw", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"flb", {"Flamberge", {{ 9, 15 }, true, true, { 13, 26 }}, {70, 49}, {2, 4}, false, "invflb", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"gsd", {"Great Sword", {{ 12, 20 }, true, true, { 25, 42 }}, {100, 60}, {2, 4}, false, "invgsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"dgr", {"Dagger", {{ 1, 4 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invdgr", 2, {"Knife", "Melee Weapon", "Weapon"}, {1}}},
            {"dir", {"Dirk", {{ 3, 9 }, false, false, { 0, 0 }}, {0, 25}, {1, 2}, false, "invdir", 2, {"Knife", "Melee Weapon", "Weapon"}, {1}}},
            {"kri", {"Kriss", {{ 2, 11 }, false, false, { 0, 0 }}, {0, 45}, {1, 3}, false, "invkrs", 2, {"Knife", "Melee Weapon", "Weapon"}, {2, 3}}},
            {"bld", {"Blade", {{ 4, 15 }, false, false, { 0, 0 }}, {35, 51}, {1, 3}, false, "invbld", 2, {"Knife", "Melee Weapon", "Weapon"}, {2}}},
            {"tkf", {"Throwing Knife", {{ 2, 3 }, false, false, { 0, 0 }, { 4, 9 }}, {0, 21}, {1, 2}, true, "invtkn", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"tax", {"Throwing Axe", {{ 4, 7 }, false, false, { 0, 0 }, { 8, 12 }}, {0, 40}, {1, 2}, true, "invtax", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"bkf", {"Balanced Knife", {{ 1, 8 }, false, false, { 0, 0 }, { 6, 11 }}, {0, 51}, {1, 2}, true, "invbkf", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"bal", {"Balanced Axe", {{ 5, 10 }, false, false, { 0, 0 }, { 12, 15 }}, {0, 57}, {2, 3}, true, "invbal", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"jav", {"Javelin", {{ 1, 5 }, false, false, { 0, 0 }, { 6, 14 }}, {0, 0}, {1, 3}, true, "invjav", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"pil", {"Pilum", {{ 4, 9 }, false, false, { 0, 0 }, { 7, 20 }}, {0, 45}, {1, 3}, true, "invpil", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"ssp", {"Short Spear", {{ 2, 13 }, false, false, { 0, 0 }, { 10, 22 }}, {40, 40}, {1, 3}, true, "invssp", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"glv", {"Glaive", {{ 5, 17 }, false, false, { 0, 0 }, { 16, 22 }}, {52, 35}, {1, 4}, true, "invglv", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"tsp", {"Throwing Spear", {{ 5, 15 }, false, false, { 0, 0 }, { 12, 30 }}, {0, 65}, {1, 4}, true, "invtsp", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"spr", {"Spear", {{ 0, 0 }, false, true, { 3, 15 }}, {0, 20}, {2, 4}, false, "invspr", 8, {"Spear", "Melee Weapon", "Weapon"}, {3}}},
            {"tri", {"Trident", {{ 0, 0 }, false, true, { 9, 15 }}, {38, 24}, {2, 4}, false, "invtri", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"brn", {"Brandistock", {{ 0, 0 }, false, true, { 7, 17 }}, {40, 50}, {2, 4}, false, "invbrn", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"spt", {"Spetum", {{ 0, 0 }, false, true, { 15, 23 }}, {54, 35}, {2, 4}, false, "invspt", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"pik", {"Pike", {{ 0, 0 }, false, true, { 14, 63 }}, {60, 45}, {2, 4}, false, "invpik", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"bar", {"Bardiche", {{ 0, 0 }, false, true, { 1, 27 }}, {40, 0}, {2, 4}, false, "invbar", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3}}},
            {"vou", {"Voulge", {{ 0, 0 }, false, true, { 6, 21 }}, {50, 0}, {2, 4}, false, "invvou", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"scy", {"Scythe", {{ 0, 0 }, false, true, { 8, 20 }}, {41, 41}, {2, 4}, false, "invscy", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"pax", {"Poleaxe", {{ 0, 0 }, false, true, { 18, 39 }}, {62, 0}, {2, 4}, false, "invpax", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"hal", {"Halberd", {{ 0, 0 }, false, true, { 12, 45 }}, {75, 47}, {2, 4}, false, "invhal", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"wsc", {"War Scythe", {{ 0, 0 }, false, true, { 15, 36 }}, {80, 80}, {2, 4}, false, "invwsc", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"sst", {"Short Staff", {{ 0, 0 }, false, true, { 1, 5 }}, {0, 0}, {1, 3}, false, "invsst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"lst", {"Long Staff", {{ 0, 0 }, false, true, { 2, 8 }}, {0, 0}, {1, 4}, false, "invlst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"cst", {"Gnarled Staff", {{ 0, 0 }, false, true, { 4, 12 }}, {0, 0}, {1, 4}, false, "invcst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {4}}},
            {"bst", {"Battle Staff", {{ 0, 0 }, false, true, { 6, 13 }}, {0, 0}, {1, 4}, false, "invbst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {4}}},
            {"wst", {"War Staff", {{ 0, 0 }, false, true, { 12, 28 }}, {0, 0}, {2, 4}, false, "invwst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {5, 6}}},
            {"sbw", {"Short Bow", {{ 0, 0 }, false, true, { 1, 4 }}, {0, 15}, {2, 3}, false, "invsbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3}}},
            {"hbw", {"Hunter's Bow", {{ 0, 0 }, false, true, { 2, 6 }}, {0, 28}, {2, 3}, false, "invhbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"lbw", {"Long Bow", {{ 0, 0 }, false, true, { 3, 10 }}, {22, 19}, {2, 4}, false, "invlbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"cbw", {"Composite Bow", {{ 0, 0 }, false, true, { 4, 8 }}, {25, 35}, {2, 3}, false, "invcbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"sbb", {"Short Battle Bow", {{ 0, 0 }, false, true, { 5, 11 }}, {30, 40}, {2, 3}, false, "invsbb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"lbb", {"Long Battle Bow", {{ 0, 0 }, false, true, { 3, 18 }}, {40, 50}, {2, 4}, false, "invlbb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"swb", {"Short War Bow", {{ 0, 0 }, false, true, { 6, 14 }}, {35, 55}, {2, 3}, false, "invswb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"lwb", {"Long War Bow", {{ 0, 0 }, false, true, { 3, 23 }}, {50, 65}, {2, 4}, false, "invlwb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"lxb", {"Light Crossbow", {{ 0, 0 }, false, true, { 6, 9 }}, {21, 27}, {2, 3}, false, "invlxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3}}},
            {"mxb", {"Crossbow", {{ 0, 0 }, false, true, { 9, 16 }}, {40, 33}, {2, 3}, false, "invmxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"hxb", {"Heavy Crossbow", {{ 0, 0 }, false, true, { 14, 26 }}, {60, 40}, {2, 4}, false, "invhxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"rxb", {"Repeating Crossbow", {{ 0, 0 }, false, true, { 6, 12 }}, {40, 50}, {2, 3}, false, "invrxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"gps", {"Rancid Gas Potion", {{ 0, 1 }, false, false, { 0, 0 }, {168, 168}}, {0, 0, 24}, {1, 1}, true, "invgpl", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"ops", {"Oil Potion", {{ 0, 1 }, false, false, { 0, 0 }, {28, 64}}, {0, 0, 20}, {1, 1}, true, "invopl", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"gpm", {"Choking Gas Potion", {{ 0, 1 }, false, false, { 0, 0 }, {72, 72}}, {0, 0, 16}, {1, 1}, true, "invgpm", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"opm", {"Exploding Potion", {{ 0, 1 }, false, false, { 0, 0 }, {16, 30}}, {0, 0, 12}, {1, 1}, true, "invopm", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"gpl", {"Strangling Gas Potion", {{ 0, 1 }, false, false, { 0, 0 }, {24, 24}}, {0, 0, 6}, {1, 1}, true, "invgps", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"opl", {"Fulminating Potion", {{ 0, 1 }, false, false, { 0, 0 }, {5, 15}}, {0, 0}, {1, 1}, true, "invops", 0, {"Missile Potion", "Thrown Weapon", "Weapon"}}},
            {"d33", {"Decoy Gidbinn", {{ 1, 2 }, false, false, { 0, 0 }}, {15, 20}, {1, 2}, false, "invd33", 2, {"Knife", "Melee Weapon", "Weapon"}}},
            {"g33", {"The Gidbinn", {{ 3, 7 }, false, false, { 0, 0 }}, {15, 25}, {1, 2}, false, "invg33", 2, {"Knife", "Melee Weapon", "Weapon", "Quest"}}},
            {"leg", {"Wirt's Leg", {{ 2, 8 }, false, false, { 0, 0 }}, {0, 0}, {1, 3}, false, "invleg", 8, {"Club", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"hdm", {"Horadric Malus", {{ 6, 15 }, false, false, { 0, 0 }}, {15, 15}, {1, 2}, false, "invhmr", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon", "Quest"}}},
            {"hfh", {"Hellforge Hammer", {{ 6, 15 }, false, false, { 0, 0 }}, {0, 0}, {2, 3}, false, "invhfh", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon", "Quest"}, {3, 4, 5}}},
            {"hst", {"Horadric Staff", {{ 0, 0 }, false, true, { 12, 20 }}, {30, 0}, {1, 4}, false, "invhst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon", "Quest"}}},
            {"msf", {"Staff of the Kings", {{0, 0}, false, true, {10, 15}}, {25, 0}, {1, 3}, false, "invmsf", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon", "Quest"}}},
            {"9ha", {"Hatchet", {{ 10, 21 }, false, false, { 0, 0 }}, {25, 35, 19}, {1, 3}, false, "invhax", 2, {"Axe", "Melee Weapon", "Weapon"}, {2}}},
            {"9ax", {"Cleaver", {{ 10, 33 }, false, false, { 0, 0 }}, {68, 0, 22}, {2, 3}, false, "invaxe", 2, {"Axe", "Melee Weapon", "Weapon"}, {4}}},
            {"92a", {"Twin Axe", {{ 13, 38 }, false, false, { 0, 0 }}, {85, 0, 25}, {2, 3}, false, "inv2ax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"9mp", {"Crowbill", {{ 14, 34 }, false, false, { 0, 0 }}, {94, 70, 25}, {2, 3}, false, "invmpi", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"9wa", {"Naga", {{ 16, 45 }, false, false, { 0, 0 }}, {121, 0, 25}, {2, 3}, false, "invwax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"9la", {"Military Axe", {{ 0, 0 }, false, true, { 14, 34 }}, {73, 0, 22}, {2, 3}, false, "invlax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4}}},
            {"9ba", {"Bearded Axe", {{ 0, 0 }, false, true, { 21, 49 }}, {92, 0, 25}, {2, 3}, false, "invbrx", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"9bt", {"Tabar", {{ 0, 0 }, false, true, { 24, 77 }}, {101, 0, 25}, {2, 3}, false, "invbtx", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"9ga", {"Gothic Axe", {{ 0, 0 }, false, true, { 18, 70 }}, {115, 79, 25}, {2, 4}, false, "invgax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"9gi", {"Ancient Axe", {{ 0, 0 }, false, true, { 43, 85 }}, {125, 0, 25}, {2, 3}, false, "invgix", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"9wn", {"Burnt Wand", {{ 8, 18 }, false, false, { 0, 0 }}, {25, 0, 19}, {1, 2}, false, "invwnd", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {1}}},
            {"9yw", {"Petrified Wand", {{ 8, 24 }, false, false, { 0, 0 }}, {25, 0, 25}, {1, 2}, false, "invywn", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"9bw", {"Tomb Wand", {{ 10, 22 }, false, false, { 0, 0 }}, {25, 0, 25}, {1, 2}, false, "invbwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"9gw", {"Grave Wand", {{ 13, 29 }, false, false, { 0, 0 }}, {25, 0, 25}, {1, 2}, false, "invgwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"9cl", {"Cudgel", {{ 6, 21 }, false, false, { 0, 0 }}, {25, 0, 18}, {1, 3}, false, "invclb", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"9sc", {"Rune Scepter", {{ 13, 24 }, false, false, { 0, 0 }}, {58, 0, 19}, {1, 3}, false, "invscp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"9qs", {"Holy Water Sprinkler", {{ 14, 36 }, false, false, { 0, 0 }}, {76, 0, 25}, {1, 3}, false, "invgsc", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"9ws", {"Divine Scepter", {{ 16, 38 }, false, false, { 0, 0 }}, {103, 0, 25}, {2, 3}, false, "invwsp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3, 5}}},
            {"9sp", {"Barbed Club", {{ 13, 25 }, false, false, { 0, 0 }}, {30, 0, 20}, {1, 3}, false, "invspc", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"9ma", {"Flanged Mace", {{ 15, 23 }, false, false, { 0, 0 }}, {61, 0, 23}, {1, 3}, false, "invmac", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"9mt", {"Jagged Star", {{ 20, 31 }, false, false, { 0, 0 }}, {74, 0, 25}, {1, 3}, false, "invmst", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"9fl", {"Knout", {{ 13, 35 }, false, false, { 0, 0 }}, {82, 73, 25}, {2, 3}, false, "invfla", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"9wh", {"Battle Hammer", {{ 35, 58 }, false, false, { 0, 0 }}, {100, 0, 25}, {2, 3}, false, "invwhm", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"9m9", {"War Club", {{ 0, 0 }, false, true, { 53, 78 }}, {124, 0, 25}, {2, 3}, false, "invmau", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"9gm", {"Martel De Fer", {{ 0, 0 }, false, true, { 61, 99 }}, {169, 0, 25}, {2, 3}, false, "invgma", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"9ss", {"Gladius", {{ 8, 22 }, false, false, { 0, 0 }}, {25, 0, 18}, {1, 3}, false, "invssd", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"9sm", {"Cutlass", {{ 8, 21 }, false, false, { 0, 0 }}, {25, 52, 25}, {1, 3}, false, "invscm", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"9sb", {"Shamshir", {{ 10, 24 }, false, false, { 0, 0 }}, {58, 58, 25}, {1, 3}, false, "invsbr", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"9fc", {"Tulwar", {{ 16, 35 }, false, false, { 0, 0 }}, {70, 42, 25}, {1, 3}, false, "invflc", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"9cr", {"Dimensional Blade", {{ 13, 35 }, false, false, { 0, 0 }}, {85, 60, 25}, {2, 3}, false, "invcrs", 8, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"9bs", {"Battle Sword", {{ 16, 34 }, false, false, { 0, 0 }}, {92, 43, 25}, {2, 3}, false, "invbsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"9ls", {"Rune Sword", {{ 10, 42 }, false, false, { 0, 0 }}, {103, 79, 25}, {2, 3}, false, "invlsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"9wd", {"Ancient Sword",{18, 43, false, false, 0, 0}, {127, 88, 25}, {1, 3}, false, "invwsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3}}},
            {"92h", {"Espadon", {{ 8, 26 }, true, true, { 18, 40 }}, {73, 61, 25}, {1, 4}, false, "inv2hs", 2, {"Sword", "Melee Weapon", "Weapon"}, {3}}},
            {"9cm", {"Dacian Falx", {{ 13, 30 }, true, true, { 26, 61 }}, {91, 20, 25}, {1, 4}, false, "invclm", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"9gs", {"Tusk Sword", {{ 10, 37 }, true, true, { 19, 58 }}, {104, 71, 25}, {1, 4}, false, "invgis", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"9b9", {"Gothic Sword", {{ 14, 40 }, true, true, { 39, 60 }}, {113, 20, 25}, {1, 4}, false, "invbsw", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"9fb", {"Zweihander", {{ 19, 35 }, true, true, { 29, 54 }}, {125, 94, 25}, {2, 4}, false, "invflb", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"9gd", {"Executioner Sword", {{ 24, 40 }, true, true, { 47, 80 }}, {170, 110, 25}, {2, 4}, false, "invgsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"9dg", {"Poignard", {{ 6, 18 }, false, false, { 0, 0 }}, {25, 0, 19}, {1, 2}, false, "invdgr", 2, {"Knife", "Melee Weapon", "Weapon"}, {1}}},
            {"9di", {"Rondel", {{ 10, 26 }, false, false, { 0, 0 }}, {25, 58, 24}, {1, 2}, false, "invdir", 2, {"Knife", "Melee Weapon", "Weapon"}, {1}}},
            {"9kr", {"Cinquedeas", {{ 15, 31 }, false, false, { 0, 0 }}, {25, 88, 25}, {1, 3}, false, "invkrs", 2, {"Knife", "Melee Weapon", "Weapon"}, {2, 3}}},
            {"9bl", {"Stilleto", {{ 19, 36 }, false, false, { 0, 0 }}, {47, 97, 25}, {1, 3}, false, "invbld", 2, {"Knife", "Melee Weapon", "Weapon"}, {2}}},
            {"9tk", {"Battle Dart", {{ 8, 16 }, false, false, { 0, 0 }, { 11, 24 }}, {25, 52, 19}, {1, 2}, true, "invtkn", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"9ta", {"Francisca", {{ 11, 22 }, false, false, { 0, 0 }, { 18, 33 }}, {25, 80, 22}, {1, 2}, true, "invtax", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"9bk", {"War Dart", {{ 6, 24 }, false, false, { 0, 0 }, { 14, 27 }}, {25, 97, 25}, {1, 2}, true, "invbkf", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"9b8", {"Hurlbat", {{ 13, 27 }, false, false, { 0, 0 }, { 24, 38 }}, {25, 106, 25}, {2, 3}, true, "invbal", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"9ja", {"War Javelin", {{ 6, 19 }, false, false, { 0, 0 }, { 14, 32 }}, {25, 25, 18}, {1, 3}, true, "invjav", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9pi", {"Great Pilum", {{ 11, 26 }, false, false, { 0, 0 }, { 16, 42 }}, {25, 88, 25}, {1, 3}, true, "invpil", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9s9", {"Simbilan", {{ 8, 32 }, false, false, { 0, 0 }, { 27, 50 }}, {80, 80, 25}, {1, 3}, true, "invssp", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9gl", {"Spiculum", {{ 13, 38 }, false, false, { 0, 0 }, { 32, 60 }}, {98, 73, 25}, {1, 4}, true, "invglv", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9ts", {"Harpoon", {{ 13, 35 }, false, false, { 0, 0 }, { 18, 54 }}, {25, 118, 25}, {1, 4}, true, "invtsp", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"9sr", {"War Spear", {{ 0, 0 }, false, true, { 10, 37 }}, {25, 25, 21}, {2, 4}, false, "invspr", 8, {"Spear", "Melee Weapon", "Weapon"}, {3}}},
            {"9tr", {"Fuscina", {{ 0, 0 }, false, true, { 19, 37 }}, {77, 25, 24}, {2, 4}, false, "invtri", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"9br", {"War Fork", {{ 0, 0 }, false, true, { 16, 40 }}, {80, 95, 25}, {2, 4}, false, "invbrn", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"9st", {"Yari", {{ 0, 0 }, false, true, { 29, 59 }}, {101, 0, 25}, {2, 4}, false, "invspt", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"9p9", {"Lance", {{ 0, 0 }, false, true, { 27, 114 }}, {110, 88, 25}, {2, 4}, false, "invpik", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"9b7", {"Lochaber Axe", {{ 0, 0 }, false, true, { 6, 58 }}, {80, 0, 21}, {2, 4}, false, "invbar", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3}}},
            {"9vo", {"Bill", {{ 0, 0 }, false, true, { 14, 53 }}, {95, 0, 25}, {2, 4}, false, "invvou", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"9s8", {"Battle Scythe", {0, 0, false, true, 18, 45, 25}, {82, 82}, {2, 4}, false, "invscy", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"9pa", {"Partizan", {{ 0, 0 }, false, true, { 34, 75 }}, {113, 67, 25}, {2, 4}, false, "invpax", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"9h9", {"Bec-De-Corbin", {{ 0, 0 }, false, true, { 13, 85 }}, {133, 91, 25}, {2, 4}, false, "invhal", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"9wc", {"Grim Scythe", {{ 0, 0 }, false, true, { 30, 70 }}, {140, 140, 25}, {2, 4}, false, "invwsc", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"8ss", {"Jo Staff", {{ 0, 0 }, false, true, { 6, 21 }}, {25, 0, 18}, {1, 3}, false, "invsst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"8ls", {"Quarterstaff", {{ 0, 0 }, false, true, { 8, 26 }}, {25, 0, 23}, {1, 4}, false, "invlst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"8cs", {"Cedar Staff", {{ 0, 0 }, false, true, { 11, 32 }}, {25, 0, 25}, {1, 4}, false, "invcst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {4}}},
            {"8bs", {"Gothic Staff", {{ 0, 0 }, false, true, { 14, 34 }}, {25, 0, 25}, {1, 4}, false, "invbst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {4}}},
            {"8ws", {"Rune Staff", {{ 0, 0 }, false, true, { 24, 58 }}, {25, 0, 25}, {1, 4}, false, "invwst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {5, 6}}},
            {"8sb", {"Edge Bow", {{ 0, 0 }, false, true, { 6, 19 }}, {25, 53, 18}, {2, 3}, false, "invsbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3}}},
            {"8hb", {"Razor Bow", {{ 0, 0 }, false, true, { 8, 22 }}, {25, 62, 21}, {2, 3}, false, "invhbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"8lb", {"Cedar Bow", {{ 0, 0 }, false, true, { 10, 29 }}, {53, 49, 23}, {2, 4}, false, "invlbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"8cb", {"Double Bow", {{ 0, 0 }, false, true, { 11, 26 }}, {58, 73, 25}, {2, 3}, false, "invcbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"8s8", {"Short Siege Bow", {{ 0, 0 }, false, true, { 13, 30 }}, {65, 80, 25}, {2, 3}, false, "invsbb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"8l8", {"Long Siege Bow", {{ 0, 0 }, false, true, { 10, 42 }}, {80, 95, 25}, {2, 4}, false, "invlbb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"8sw", {"Rune Bow", {{ 0, 0 }, false, true, { 14, 35 }}, {73, 103, 25}, {2, 3}, false, "invswb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"8lw", {"Gothic Bow", {{ 0, 0 }, false, true, { 10, 50 }}, {95, 118, 25}, {2, 4}, false, "invlwb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"8lx", {"Arbalest", {{ 0, 0 }, false, true, { 14, 27 }}, {52, 61, 22}, {2, 3}, false, "invlxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3}}},
            {"8mx", {"Siege Crossbow", {{ 0, 0 }, false, true, { 20, 42 }}, {80, 70, 25}, {2, 3}, false, "invmxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"8hx", {"Balista", {{ 0, 0 }, false, true, { 33, 55 }}, {110, 80, 25}, {2, 4}, false, "invhxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"8rx", {"Chu-Ko-Nu", {{ 0, 0 }, false, true, { 14, 32 }}, {80, 95, 25}, {2, 3}, false, "invrxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"qf1", {"Khalim's Flail", {{ 1, 15 }, false, false, { 0, 0 }}, {41, 35}, {2, 3}, false, "invqf1", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon", "Quest"}}},
            {"qf2", {"Khalim's Will", {{ 1, 15 }, false, false, { 0, 0 }}, {0, 0}, {2, 3}, false, "invqf2", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon", "Quest"}}},
            {"ktr", {"Katar", {{ 4, 7 }, false, false, { 0, 0 }}, {20, 20}, {1, 3}, false, "invktr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"wrb", {"Wrist Blade", {{ 5, 9 }, false, false, { 0, 0 }}, {33, 33}, {1, 3}, false, "invktr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"axf", {"Hatchet Hands", {{ 2, 15 }, false, false, { 0, 0 }}, {37, 37}, {1, 3}, false, "invaxf", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"ces", {"Cestus", {{ 7, 15 }, false, false, { 0, 0 }}, {42, 42}, {1, 3}, false, "invaxf", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"clw", {"Claws", {{ 8, 15 }, false, false, { 0, 0 }}, {46, 46}, {1, 3}, false, "invclw", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"btl", {"Blade Talons", {{ 10, 14 }, false, false, { 0, 0 }}, {50, 50}, {1, 3}, false, "invclw", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"skr", {"Scissors Katar", {{ 9, 17 }, false, false, { 0, 0 }}, {55, 55}, {1, 2}, false, "invskr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"9ar", {"Quhab", {{ 11, 24 }, false, false, { 0, 0 }}, {57, 57, 21}, {1, 2}, false, "invskr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"9wb", {"Wrist Spike", {{ 13, 27 }, false, false, { 0, 0 }}, {66, 66, 24}, {1, 3}, false, "invktr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"9xf", {"Fascia", {{ 8, 37 }, false, false, { 0, 0 }}, {69, 69, 27}, {1, 2}, false, "invktr", 2, {"Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"9cs", {"Hand Scythe", {{ 16, 37 }, false, false, { 0, 0 }}, {73, 73, 30}, {1, 2}, false, "invaxf", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"9lw", {"Greater Claws", {{ 18, 37 }, false, false, { 0, 0 }}, {76, 76, 33}, {1, 2}, false, "invclw", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"9tw", {"Greater Talons", {{ 21, 35 }, false, false, { 0, 0 }}, {79, 79, 37}, {1, 3}, false, "invclw", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"9qr", {"Scissors Quhab", {{ 19, 40 }, false, false, { 0, 0 }}, {82, 82, 40}, {1, 3}, false, "invskr", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"7ar", {"Suwayyah", {{ 39, 52 }, false, false, { 0, 0 }}, {99, 99, 44}, {1, 3}, false, "invktr", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"7wb", {"Wrist Sword", {{ 34, 45 }, false, false, { 0, 0 }}, {105, 105, 46}, {1, 3}, false, "invktr", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"7xf", {"War Fist", {{ 44, 53 }, false, false, { 0, 0 }}, {108, 108, 51}, {1, 3}, false, "invaxf", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"7cs", {"Battle Cestus", {{ 36, 42 }, false, false, { 0, 0 }}, {110, 110, 54}, {1, 3}, false, "invaxf", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2}}},
            {"7lw", {"Feral Claws", {{ 22, 53 }, false, false, { 0, 0 }}, {113, 113, 58}, {1, 3}, false, "invclw", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"7tw", {"Runic Talons", {{ 24, 44 }, false, false, { 0, 0 }}, {115, 115, 60}, {1, 3}, false, "invclw", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"7qr", {"Scissors Suwayyah", {{ 40, 51 }, false, false, { 0, 0 }}, {118, 188, 64}, {1, 3}, false, "invskr", 2, {"Hand to Hand 2", "Hand to Hand", "Melee Weapon", "Weapon", "Assassin Item", "Class Specific"}, {2, 3}}},
            {"7ha", {"Tomahawk", {{ 33, 58 }, false, false, { 0, 0 }}, {125, 67, 40}, {1, 3}, false, "invhax", 2, {"Axe", "Melee Weapon", "Weapon"}, {2}}},
            {"7ax", {"Small Crescent", {{ 38, 60 }, false, false, { 0, 0 }}, {115, 83, 45}, {2, 3}, false, "invaxe", 2, {"Axe", "Melee Weapon", "Weapon"}, {4}}},
            {"72a", {"Ettin Axe", {{ 33, 66 }, false, false, { 0, 0 }}, {145, 45, 52}, {2, 3}, false, "inv2ax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"7mp", {"War Spike", {{ 30, 48 }, false, false, { 0, 0 }}, {133, 54, 59}, {2, 3}, false, "invmpi", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"7wa", {"Berserker Axe", {{ 24, 71 }, false, false, { 0, 0 }}, {138, 59, 64}, {2, 3}, false, "invwax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"7la", {"Feral Axe", {{ 0, 0 }, false, true, { 25, 123 }}, {196, 0, 42}, {2, 3}, false, "invlax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4}}},
            {"7ba", {"Silver Edged Axe", {{ 0, 0 }, false, true, { 62, 110 }}, {166, 65, 48}, {2, 3}, false, "invbrx", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"7bt", {"Decapitator", {{ 0, 0 }, false, true, { 49, 137 }}, {189, 33, 54}, {2, 3}, false, "invbtx", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5}}},
            {"7ga", {"Champion Axe", {{ 0, 0 }, false, true, { 59, 94 }}, {167, 59, 61}, {2, 4}, false, "invgax", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"7gi", {"Glorious Axe", {{ 0, 0 }, false, true, { 60, 124 }}, {164, 55, 66}, {2, 3}, false, "invgix", 2, {"Axe", "Melee Weapon", "Weapon"}, {4, 5, 6}}},
            {"7wn", {"Polished Wand", {{ 18, 33 }, false, false, { 0, 0 }}, {25, 0, 41}, {1, 2}, false, "invwnd", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"7yw", {"Ghost Wand", {{ 20, 40 }, false, false, { 0, 0 }}, {25, 0, 48}, {1, 2}, false, "invywn", 8, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"7bw", {"Lich Wand", {{ 10, 31 }, false, false, { 0, 0 }}, {25, 0, 56}, {1, 2}, false, "invbwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"7gw", {"Unearthed Wand", {{ 22, 28 }, false, false, { 0, 0 }}, {25, 0, 64}, {1, 2}, false, "invgwn", 2, {"Wand", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"7cl", {"Truncheon", {{ 35, 43 }, false, false, { 0, 0 }}, {88, 43, 39}, {1, 3}, false, "invclb", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"7sc", {"Mighty Scepter", {{ 40, 52 }, false, false, { 0, 0 }}, {125, 65, 46}, {1, 3}, false, "invscp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"7qs", {"Seraph Rod", {{ 45, 54 }, false, false, { 0, 0 }}, {108, 69, 57}, {1, 3}, false, "invgsc", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"7ws", {"Caduceus", {{ 37, 43 }, false, false, { 0, 0 }}, {97, 70, 66}, {2, 3}, false, "invwsp", 2, {"Scepter", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3, 5}}},
            {"7sp", {"Tyrant Club", {{ 32, 58 }, false, false, { 0, 0 }}, {133, 0, 42}, {1, 3}, false, "invspc", 2, {"Club", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"7ma", {"Reinforced Mace", {{ 41, 49 }, false, false, { 0, 0 }}, {145, 46, 47}, {1, 3}, false, "invmac", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"7mt", {"Devil Star", {{ 43, 53 }, false, false, { 0, 0 }}, {153, 44, 52}, {1, 3}, false, "invmst", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"7fl", {"Scourge", {{ 3, 80 }, false, false, { 0, 0 }}, {125, 77, 57}, {2, 3}, false, "invfla", 2, {"Mace", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"7wh", {"Legendary Mallet", {{ 50, 61 }, false, false, { 0, 0 }}, {189, 0, 61}, {2, 3}, false, "invwhm", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"7m7", {"Ogre Maul", {{ 0, 0 }, false, true, { 77, 106 }}, {225, 0, 51}, {2, 4}, false, "invmau", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"7gm", {"Thunder Maul", {{ 0, 0 }, false, true, { 33, 180 }}, {253, 0, 65}, {2, 3}, false, "invgma", 2, {"Hammer", "Blunt", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"7ss", {"Falcata", {{ 31, 59 }, false, false, { 0, 0 }}, {150, 88, 42}, {1, 3}, false, "invssd", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"7sm", {"Ataghan", {{ 26, 46 }, false, false, { 0, 0 }}, {138, 95, 45}, {1, 3}, false, "invscm", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"7sb", {"Elegant Blade", {{ 33, 45 }, false, false, { 0, 0 }}, {109, 122, 47}, {1, 3}, false, "invsbr", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"7fc", {"Hydra Edge", {{ 28, 68 }, false, false, { 0, 0 }}, {142, 105, 51}, {1, 3}, false, "invflc", 2, {"Sword", "Melee Weapon", "Weapon"}, {2}}},
            {"7cr", {"Phase Blade", {{ 31, 35 }, false, false, { 0, 0 }}, {25, 136, 54}, {2, 3}, false, "invcrs", 8, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"7bs", {"Conquest Sword", {{ 37, 53 }, false, false, { 0, 0 }}, {142, 112, 58}, {2, 3}, false, "invbsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"7ls", {"Cryptic Sword", {{ 5, 77 }, false, false, { 0, 0 }}, {99, 109, 61}, {2, 3}, false, "invlsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"7wd", {"Mythical Sword", {{ 40, 50 }, false, false, { 0, 0 }}, {147, 124, 66}, {1, 3}, false, "invwsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3}}},
            {"72h", {"Legend Sword", {{ 22, 56 }, true, true, { 50, 94 }}, {175, 100, 44}, {1, 4}, false, "inv2hs", 2, {"Sword", "Melee Weapon", "Weapon"}, {3}}},
            {"7cm", {"Highland Blade", {{ 22, 62 }, true, true, { 67, 96 }}, {171, 104, 49}, {1, 4}, false, "invclm", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"7gs", {"Balrog Blade", {{ 15, 75 }, true, true, { 55, 118 }}, {185, 87, 53}, {1, 4}, false, "invgis", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"7b7", {"Champion Sword", {{ 24, 54 }, true, true, { 71, 83 }}, {163, 103, 57}, {1, 4}, false, "invbsw", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"7fb", {"Colossal Sword", {{ 26, 70 }, true, true, { 61, 121 }}, {182, 95, 60}, {2, 4}, false, "invflb", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"7gd", {"Colossus Blade", {{ 25, 65 }, true, true, { 58, 115 }}, {189, 110, 63}, {2, 4}, false, "invgsd", 2, {"Sword", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"7dg", {"Bone Knife", {{ 23, 49 }, false, false, { 0, 0 }}, {38, 75, 43}, {1, 2}, false, "invdgr", 2, {"Knife", "Melee Weapon", "Weapon"}, {1}}},
            {"7di", {"Mithril Point", {{ 37, 53 }, false, false, { 0, 0 }}, {55, 98, 52}, {1, 2}, false, "invdir", 2, {"Knife", "Melee Weapon", "Weapon"}, {1}}},
            {"7kr", {"Fanged Knife", {{ 15, 57 }, false, false, { 0, 0 }}, {42, 86, 62}, {1, 3}, false, "invkrs", 2, {"Knife", "Melee Weapon", "Weapon"}, {2, 3}}},
            {"7bl", {"Legend Spike", {{ 31, 47 }, false, false, { 0, 0 }}, {65, 67, 66}, {1, 3}, false, "invbld", 2, {"Knife", "Melee Weapon", "Weapon"}, {2}}},
            {"7tk", {"Flying Knife", {{ 23, 54 }, false, false, { 0, 0 }, { 23, 54 }}, {48, 141, 48}, {1, 2}, true, "invtkn", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"7ta", {"Flying Axe", {{ 17, 65 }, false, false, { 0, 0 }, { 15, 66 }}, {88, 108, 42}, {1, 2}, true, "invtax", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"7bk", {"Winged Knife", {{ 27, 35 }, false, false, { 0, 0 }, { 23, 39 }}, {45, 142, 57}, {1, 2}, true, "invbkf", 2, {"Throwing Knife", "Combo Weapon", "Knife", "Melee Weapon", "Weapon"}}},
            {"7b8", {"Winged Axe", {{ 11, 56 }, false, false, { 0, 0 }, { 7, 60 }}, {96, 122, 60}, {2, 3}, true, "invbal", 2, {"Throwing Axe", "Combo Weapon", "Axe", "Melee Weapon", "Weapon"}}},
            {"7ja", {"Hyperion Javeln", {{ 21, 57 }, false, false, { 0, 0 }, { 28, 55 }}, {98, 123, 40}, {1, 3}, true, "invjav", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7pi", {"Stygian Pilum", {{ 14, 64 }, false, false, { 0, 0 }, { 21, 75 }}, {118, 112, 46}, {1, 3}, true, "invpil", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7s7", {"Balrog Spear", {{ 33, 63 }, false, false, { 0, 0 }, { 40, 62 }}, {127, 95, 53}, {1, 3}, true, "invssp", 8, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7gl", {"Ghost Glaive", {{ 19, 60 }, false, false, { 0, 0 }, { 30, 85 }}, {89, 137, 59}, {1, 4}, true, "invglv", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7ts", {"Winged Harpoon", {{ 27, 35 }, false, false, { 0, 0 }, { 11, 77 }}, {76, 145, 65}, {1, 4}, true, "invtsp", 2, {"Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon"}}},
            {"7sr", {"Hyperion Spear", {{ 0, 0 }, false, true, { 35, 119 }}, {155, 120, 43}, {2, 4}, false, "invspr", 8, {"Spear", "Melee Weapon", "Weapon"}, {3}}},
            {"7tr", {"Stygian Pike", {{ 0, 0 }, false, true, { 29, 144 }}, {168, 97, 49}, {2, 4}, false, "invtri", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"7br", {"Mancatcher", {{ 0, 0 }, false, true, { 42, 92 }}, {132, 134, 55}, {2, 4}, false, "invbrn", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"7st", {"Ghost Spear", {{ 0, 0 }, false, true, { 18, 155 }}, {122, 163, 62}, {2, 4}, false, "invspt", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"7p7", {"War Pike", {{ 0, 0 }, false, true, { 33, 178 }}, {165, 106, 66}, {2, 4}, false, "invpik", 8, {"Spear", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"7o7", {"Ogre Axe", {{ 0, 0 }, false, true, { 28, 145 }}, {195, 75, 45}, {2, 4}, false, "invbar", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3}}},
            {"7vo", {"Colossus Voulge", {{ 0, 0 }, false, true, { 17, 165 }}, {210, 55, 48}, {2, 4}, false, "invvou", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4}}},
            {"7s8", {"Thresher", {{ 0, 0 }, false, true, { 12, 141 }}, {152, 118, 53}, {2, 4}, false, "invscy", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"7pa", {"Cryptic Axe", {{ 0, 0 }, false, true, { 33, 150 }}, {165, 103, 59}, {2, 4}, false, "invpax", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 5}}},
            {"7h7", {"Great Poleaxe", {{ 0, 0 }, false, true, { 46, 127 }}, {179, 99, 63}, {2, 4}, false, "invhal", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"7wc", {"Giant Thresher", {{ 0, 0 }, false, true, { 40, 114 }}, {188, 140, 66}, {2, 4}, false, "invwsc", 2, {"Polearm", "Melee Weapon", "Weapon"}, {3, 4, 6}}},
            {"6ss", {"Walking Stick", {{ 0, 0 }, false, true, { 69, 85 }}, {25, 0, 43}, {1, 3}, false, "invsst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {2}}},
            {"6ls", {"Stalagmite", {{ 0, 0 }, false, true, { 75, 107 }}, {63, 35, 49}, {1, 4}, false, "invlst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {3}}},
            {"6cs", {"Elder Staff", {{ 0, 0 }, false, true, { 80, 93 }}, {44, 37, 55}, {1, 4}, false, "invcst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {4}}},
            {"6bs", {"Shillelagh", {{ 0, 0 }, false, true, { 65, 108 }}, {52, 27, 62}, {1, 4}, false, "invbst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {4}}},
            {"6ws", {"Archon Staff", {{ 0, 0 }, false, true, { 83, 99 }}, {34, 0, 66}, {2, 4}, false, "invwst", 8, {"Staff", "Staves And Rods", "Blunt", "Melee Weapon", "Weapon"}, {5, 6}}},
            {"6sb", {"Spider Bow", {{ 0, 0 }, false, true, { 23, 50 }}, {64, 143, 41}, {2, 3}, false, "invsbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3}}},
            {"6hb", {"Blade Bow", {{ 0, 0 }, false, true, { 21, 41 }}, {76, 119, 45}, {2, 3}, false, "invhbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"6lb", {"Shadow Bow", {{ 0, 0 }, false, true, { 15, 59 }}, {52, 188, 47}, {2, 4}, false, "invlbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"6cb", {"Great Bow", {{ 0, 0 }, false, true, { 12, 52 }}, {121, 107, 51}, {2, 3}, false, "invcbw", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"6s7", {"Diamond Bow", {{ 0, 0 }, false, true, { 33, 40 }}, {89, 132, 54}, {2, 3}, false, "invsbb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"6l7", {"Crusader Bow", {{ 0, 0 }, false, true, { 15, 63 }}, {97, 121, 57}, {2, 4}, false, "invlbb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"6sw", {"Ward Bow", {{ 0, 0 }, false, true, { 20, 53 }}, {72, 146, 60}, {2, 3}, false, "invswb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"6lw", {"Hydra Bow", {{ 0, 0 }, false, true, { 10, 68 }}, {134, 160, 63}, {2, 4}, false, "invlwb", 8, {"Bow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"6lx", {"Pellet Bow", {{ 0, 0 }, false, true, { 28, 73 }}, {83, 155, 42}, {2, 3}, false, "invlxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3}}},
            {"6mx", {"Gorgon Crossbow", {{ 0, 0 }, false, true, { 25, 87 }}, {117, 105, 50}, {2, 3}, false, "invmxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4}}},
            {"6hx", {"Colossus Crossbow", {{ 0, 0 }, false, true, { 32, 91 }}, {163, 77, 56}, {2, 4}, false, "invhxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4, 6}}},
            {"6rx", {"Demon Crossbow", {{ 0, 0 }, false, true, { 26, 40 }}, {141, 98, 63}, {2, 3}, false, "invrxb", 8, {"Crossbow", "Missile Weapon", "Weapon"}, {3, 4, 5}}},
            {"ob1", {"Eagle Orb", {{ 2, 5 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invob1", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"ob2", {"Sacred Globe", {{ 3, 8 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invob2", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"ob3", {"Smoked Sphere", {{ 4, 10 }, false, false, { 0, 0 }}, {0, 0, 8}, {1, 2}, false, "invob3", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"ob4", {"Clasped Orb", {{ 5, 12 }, false, false, { 0, 0 }}, {0, 0, 13}, {1, 2}, false, "invob4", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"ob5", {"Dragon Stone", {{ 8, 18 }, false, false, { 0, 0 }}, {0, 0, 18}, {1, 3}, false, "invob5", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"am1", {"Stag Bow", {{ 0, 0 }, false, true, { 7, 12 }}, {30, 45, 14}, {2, 4}, false, "invam1", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 5}}},
            {"am2", {"Reflex Bow", {{ 0, 0 }, false, true, { 9, 19 }}, {35, 60, 20}, {2, 4}, false, "invam2", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 5}}},
            {"am3", {"Maiden Spear", {{ 0, 0 }, false, true, { 18, 24 }}, {54, 40, 14}, {2, 4}, false, "invam3", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 6}}},
            {"am4", {"Maiden Pike", {{ 0, 0 }, false, true, { 23, 55 }}, {63, 52, 20}, {2, 4}, false, "invam4", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 6}}},
            {"am5", {"Maiden Javelin", {{ 8, 14 }, false, false, { 0, 0 }, { 6, 22 }}, {33, 47, 17}, {1, 3}, true, "invam5", 2, {"Amazon Javelin", "Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"ob6", {"Glowing Orb", {{ 8, 21 }, false, false, { 0, 0 }}, {0, 0, 24}, {1, 2}, false, "invob1", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"ob7", {"Crystalline Globe", {{ 10, 26 }, false, false, { 0, 0 }}, {0, 0, 27}, {1, 2}, false, "invob2", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"ob8", {"Cloudy Sphere", {{ 11, 29 }, false, false, { 0, 0 }}, {0, 0, 30}, {1, 2}, false, "invob3", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"ob9", {"Sparkling Ball", {{ 13, 32 }, false, false, { 0, 0 }}, {0, 0, 34}, {1, 2}, false, "invob4", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"oba", {"Swirling Crystal", {{ 18, 42 }, false, false, { 0, 0 }}, {0, 0, 37}, {1, 3}, false, "invob5", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"am6", {"Ashwood Bow", {{ 0, 0 }, false, true, { 16, 29 }}, {56, 77, 29}, {2, 4}, false, "invam1", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 5}}},
            {"am7", {"Ceremonial Bow", {{ 0, 0 }, false, true, { 19, 41 }}, {73, 110, 35}, {2, 4}, false, "invam2", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 5}}},
            {"am8", {"Ceremonial Spear", {{ 0, 0 }, false, true, { 34, 51 }}, {101, 80, 32}, {2, 4}, false, "invam3", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 6}}},
            {"am9", {"Ceremonial Pike", {{ 0, 0 }, false, true, { 42, 101 }}, {115, 98, 38}, {2, 4}, false, "invam4", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 6}}},
            {"ama", {"Ceremonial Javelin", {{ 18, 35 }, false, false, { 0, 0 }, { 18, 54 }}, {25, 109, 26}, {1, 3}, true, "invam5", 2, {"Amazon Javelin", "Javelin", "Combo Weapon", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}}},
            {"obb", {"Heavenly Stone", {{ 21, 46 }, false, false, { 0, 0 }}, {0, 0, 44}, {1, 2}, false, "invob1", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"obc", {"Eldritch Orb", {{ 18, 50 }, false, false, { 0, 0 }}, {0, 0, 50}, {1, 2}, false, "invob2", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"obd", {"Demon Heart", {{ 23, 55 }, false, false, { 0, 0 }}, {0, 0, 56}, {1, 2}, false, "invob3", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"obe", {"Vortex Orb", {{ 12, 66 }, false, false, { 0, 0 }}, {0, 0, 63}, {1, 2}, false, "invob4", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"obf", {"Dimensional Shard", {{ 30, 53 }, false, false, { 0, 0 }}, {0, 0, 66}, {1, 3}, false, "invob5", 8, {"Orb", "Weapon", "Sorceress Item", "Class Specific"}, {2, 3}}},
            {"amb", {"Matriarchal Bow", {{ 0, 0 }, false, true, { 20, 47 }}, {87, 187, 39}, {2, 4}, false, "invam1", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 5}}},
            {"amc", {"Grand Matron Bow", {{ 0, 0 }, false, true, { 14, 72 }}, {108, 152, 58}, {2, 4}, false, "invam2", 8, {"Amazon Bow", "Bow", "Missile Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 5}}},
            {"amd", {"Matriarchal Spear", {{ 0, 0 }, false, true, { 65, 95 }}, {114, 142, 45}, {2, 4}, false, "invam3", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 6}}},
            {"ame", {"Matriarchal Pike", {{ 0, 0 }, false, true, { 37, 153 }}, {132, 149, 60}, {2, 4}, false, "invam4", 8, {"Amazon Spear", "Spear", "Melee Weapon", "Weapon", "Amazon Item", "Class Specific"}, {3, 4, 6}}},
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

    const ItemType& getTomeItemType(const std::array<std::uint8_t, 4>& strcode)
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

    const ItemType& getStackableItemType(const std::array<std::uint8_t, 4>& strcode)
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

    const ItemType& getMiscItemType(const std::array<std::uint8_t, 4>& strcode)
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
            {"vip", {"Viper Amulet", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 1}, false, "invvip", 0, {"Amulet", "Miscellaneous", "Quest"}}},
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
            {"j34", {"Jade Figurine", {{ 0, 0 }, false, false, { 0, 0 }}, {0, 0}, {1, 2}, false, "invjbi", 0, {"Quest"}}},
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

    const ItemType& getGPSItemType(const std::array<std::uint8_t, 4>& strcode)
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

    struct SetItemV100
    {
        std::uint16_t code;
        std::uint16_t id;
        std::vector<MagicalAttribute> attribs;
    };

    const std::vector<SetItemV100> setItemV100Info = {
        { 0,  0, {{ 31, "", "",   {15}}, { 20, "", "",   {15}}}},
        { 0,  1, {{ 27, "", "",   {40}}, { 74, "", "",    {4}}}},
        { 0,  2, {{ 19, "", "",   {75}}, { 24, "", "",   {17}}}},
        { 1,  3, {{ 39, "", "",   {25}}, { 96, "", "",   {10}}}},
        { 1,  4, {{ 34, "", "",    {2}}, {  0, "", "",   {10}}}},
        { 1,  5, {{ 43, "", "",   {20}}, {  7, "", "",   {20}}}},
        { 2,  6, {{119, "", "",   {30}}, {141, "", "",   {50}}}},
        { 2,  7, {{ 31, "", "",   {17}}, {110, "", "",   {75}}}},
        { 2,  8, {{ 81, "", "",    {1}}, {150, "", "",   {25}}}},
        { 3,  9, {{ 45, "", "",   {30}}, {110, "", "",   {75}}}},
        { 3, 10, {{ 43, "", "",   {30}}, {118, "", "",    {1}}}},
        { 3, 11, {{ 39, "", "",   {30}}, { 41, "", "",   {30}}}},
        { 3, 12, {{ 31, "", "",   {25}}, { 23, "", "",    {5}}}},
        { 4, 13, {{ 23, "", "",   {10}}, { 89, "", "",    {3}}}},
        { 4, 14, {{ 31, "", "",   {40}}, {128, "", "",    {4}}}},
        { 4, 15, {{ 31, "", "",   {40}}, { 35, "", "",    {2}}}},
        { 4, 16, {{  2, "", "",    {6}}, { 34, "", "",    {2}}}},
        { 5, 17, {{ 50, "", "", {1, 20}}}},
        { 5, 18, {{ 11, "", "",  {150}}, { 96, "", "",   {30}}}},
        { 5, 19, {{ 31, "", "",   {50}}, {  2, "", "",   {11}}}},
        { 5, 20, {{  7, "", "",   {15}}, { 43, "", "",   {20}}}},
        { 6, 21, {{ 80, "", "",   {20}}, { 31, "", "",   {25}}}},
        { 6, 22, {{ 83, "", "", {3, 1}}, { 17, "", "",   {50}}}},
        { 6, 23, {{  7, "", "",   {15}}, {  9, "", "",   {15}}}},
        { 6, 24, {{ 78, "", "",    {3}}, { 34, "", "",    {2}}}},
        { 7, 25, {{126, "", "",    {1}}, { 49, "", "",   {10}}}},
        { 7, 26, {{ 31, "", "",   {15}}, { 91, "", "",  {-50}}}},
        { 7, 27, {{  9, "", "",   {20}}, { 43, "", "",   {25}}}},
        { 7, 28, {{ 99, "", "",   {10}}, {128, "", "",    {5}}}},
        { 7, 29, {{ 60, "", "",    {6}}, { 34, "", "",    {2}}}},
        { 8, 30, {{ 19, "", "",   {75}}, { 17, "", "",   {80}}}},
        { 8, 31, {{  7, "", "",   {40}}, {  0, "", "",   {15}}}},
        { 8, 32, {{ 28, "", "",   {25}}, {  2, "", "",   {10}}}},
        { 8, 33, {{ 34, "", "",    {2}}, { 35, "", "",    {1}}}},
        { 8, 34, {{ 17, "", "",   {10}}, { 19, "", "",   {40}}}},
        { 9, 35, {{  9, "", "",   {30}}, { 89, "", "",    {3}}}},
        { 9, 36, {{ 16, "", "",   {25}}, { 41, "", "",   {30}}}},
        { 9, 37, {{  0, "", "",   {10}}, { 19, "", "",   {20}}}},
        { 9, 38, {{ 96, "", "",   {20}}, { 43, "", "",   {40}}}},
        { 9, 39, {{ 39, "", "",   {20}}, {  7, "", "",   {20}}}},
        { 9, 40, {{127, "", "",    {1}}, { 20, "", "",   {20}}}},
        {10, 41, {{ 39, "", "",   {10}}, { 41, "", "",   {10}}, { 43, "", "",   {10}}, { 45, "", "",   {10}}, {114, "", "",  {20}}}},
        {10, 42, {{ 23, "", "",    {8}}, { 83, "", "", {2, 1}}}},
        {10, 43, {{ 31, "", "",   {25}}, {  7, "", "",   {20}}}},
        {11, 44, {{ 31, "", "",   {15}}, { 39, "", "",   {25}}}},
        {11, 45, {{ 35, "", "",    {2}}, { 83, "", "", {4, 1}}}},
        {11, 46, {{119, "", "",   {30}}, { 62, "", "",    {5}}}},
        {12, 47, {{ 45, "", "",   {50}}, {110, "", "",   {75}}}},
        {12, 48, {{ 31, "", "",   {20}}, {153, "", "",    {1}}}},
        {12, 49, {{ 17, "", "",   {25}}, { 60, "", "",    {4}}}},
        {13, 50, {{ 19, "", "",   {75}}, {122, "", "",  {250}}}},
        {13, 51, {{ 34, "", "",    {3}}, { 16, "", "",   {40}}}},
        {13, 52, {{ 74, "", "",    {6}}, {  7, "", "",   {20}}}},
        {13, 53, {{ 89, "", "",    {3}}, {114, "", "",   {20}}}},
        {14, 54, {{119, "", "",   {20}}, { 17, "", "",   {20}}}},
        {14, 55, {{ 16, "", "",  {275}}, { 39, "", "",   {10}}, { 41, "", "",   {10}}, { 43, "", "",   {10}}, { 45, "", "",   {10}}}},
        {14, 56, {{ 43, "", "",   {40}}, { 31, "", "",   {30}}}},
        {14, 57, {{  7, "", "",   {20}}, { 93, "", "",   {10}}}},
        {15, 58, {{  9, "", "",   {15}}, { 27, "", "",   {20}}}},
        {15, 59, {{ 83, "", "", {1, 1}}, {141, "", "",   {25}}}},
        {15, 60, {{ 74, "", "",    {4}}, { 78, "", "",    {2}}}},
        {15, 61, {{ 89, "", "",    {2}}, { 34, "", "",    {3}}}},
    }; 
    
    const std::vector<std::vector<MagicalAttribute>> uniqueItemV100Info = {
        {{ 19, "", "",            {30}}, {135, "", "",            {50}}, {136, "", "",            {20}}},
        {{113, "", "",             {1}}, {21, "", "",             {8}}, {119, "", "",            {15}}, {138, "", "",             {2}}},
        {{122, "", "",           {100}}, {124, "", "",            {40}}, { 19, "", "",            {20}}, { 31, "", "",            {20}}, { 48, "", "",      {  4,   7}}},
        {{113, "", "",            {60}}, { 27, "", "",            {20}}, { 50, "", "",      {  1,  12}}, { 19, "", "",            {50}}},
        {{ 57, "", "", { 64,  96,  75}}, { 19, "", "",            {50}}, { 45, "", "",            {50}}},
        {{ 17, "", "",            {50}}, {134, "", "",             {1}}, { 43, "", "",            {30}}, { 89, "", "",             {2}}},
        {{119, "", "",             {1}}, {  0, "", "",            {25}}, {135, "", "",            {60}}},
        {{ 17, "", "",           {100}}, { 39, "", "",            {10}}, { 41, "", "",            {10}}, { 43, "", "",            {10}}, { 45, "", "",            {10}}, { 45, "", "", {20}}, {119, "", "",            {10}}},
        {{ 21, "", "",            {14}}, {  9, "", "",            {25}}, { 89, "", "",             {4}}, { 62, "", "",            {10}}},                         
        {{  0, "", "",            {10}}, { 21, "", "",             {8}}, { 22, "", "",            {15}}, {136, "", "",            {33}}, { 91, "", "",            {20}}},
        {{ 83, "", "",      {  2,   1}}, { 60, "", "",             {6}}, { 48, "", "",     {  5,   9}}},                                                          
        {{ 50, "", "",      {  1,   9}}, { 41, "", "",            {40}}, {118, "", "",             {1}}, {105, "", "",            {20}}, {  9, "", "",            {13}}},
        {{  0, "", "",            {10}}, {  2, "", "",            {10}}, { 54, "", "", {  4,   8,  75}}, { 62, "", "",             {5}}},                         
        {{ 83, "", "",      {  2,   1}}, {  9, "", "",            {40}}, {105, "", "",            {20}}, {112, "", "",            {50}}},                         
        {{ 41, "", "",            {60}}, { 39, "", "",            {20}}, { 81, "", "",             {1}}, { 48, "", "",      {  6,   8}}},                         
        {{136, "", "",            {25}}, { 39, "", "",            {20}}, { 45, "", "",            {20}}, {  9, "", "",            {15}}, { 19, "", "",            {35}}},
        {{ 83, "", "",      {  3,   1}}, { 21, "", "",             {3}}, { 22, "", "",             {7}}, { 35, "", "",             {1}}, { 60, "", "",             {8}}},
        {{ 50, "", "",      {  1,   6}}, { 54, "", "", {  3,   5,  75}}, { 74, "", "",            {10}}},                                                         
        {{ 78, "", "",             {1}}, { 17, "", "",           {100}}, {  7, "", "",            {15}}, { 35, "", "",             {2}}},                         
        {{  0, "", "",            {15}}, { 81, "", "",             {1}}, { 89, "", "",             {2}}, { 39, "", "",            {50}}},                         
        {{ 17, "", "",           {100}}, {119, "", "",            {50}}, { 89, "", "",             {2}}, {119, "", "",            {10}}},                         
        {{ 21, "", "",             {1}}, { 22, "", "",            {20}}, {150, "", "",            {50}}, { 31, "", "",            {25}}, { 62, "", "",             {5}}},
        {{ 19, "", "",            {80}}, { 17, "", "",            {60}}, { 50, "", "",      {  1,  10}}},                         
        {{ 17, "", "",           {200}}, {136, "", "",            {40}}, { 39, "", "",            {30}}, { 43, "", "",            {30}}},                         
        {{ 91, "", "",           {-50}}, {119, "", "",            {30}}, { 28, "", "",            {25}}},                                                         
        {{ 21, "", "",             {5}}, {119, "", "",            {20}}, { 89, "", "",             {2}}, {136, "", "",            {25}}, { 31, "", "",            {25}}},
        {{ 39, "", "",            {15}}, { 41, "", "",            {15}}, { 43, "", "",            {15}}, { 45, "", "",            {15}}, { 17, "", "",            {25}}, {  7, "", "", {15}}, { 89, "", "",             {4}}},
        {{115, "", "",             {1}}, {  0, "", "",            {10}}, {  2, "", "",            {10}}, { 62, "", "",             {3}}, { 17, "", "",            {50}}},
        {{ 89, "", "",             {3}}, {  9, "", "",            {30}}, { 31, "", "",            {20}}, { 19, "", "",            {20}}},                         
        {{141, "", "",            {30}}, { 80, "", "",            {10}}, { 54, "", "", {  3,   6, 100}}},                                                         
        {{ 48, "", "",      {  8,  12}}, { 19, "", "",            {40}}, { 93, "", "",            {10}}, { 81, "", "",             {1}}},
        {{ 62, "", "",             {5}}, { 60, "", "",             {5}}, { 57, "", "", { 48,  96, 150}}},                                                         
        {{127, "", "",             {1}}, {110, "", "",            {50}}, { 99, "", "",            {20}}, {102, "", "",            {20}}, { 19, "", "",            {60}}},
        {{ 62, "", "",             {5}}, { 43, "", "",            {20}}, { 89, "", "",            {-2}}, { 54, "", "", {  5,  10,  75}}},                         
        {{ 62, "", "",             {4}}, { 60, "", "",             {4}}, { 17, "", "",            {30}}, { 39, "", "",             {5}}, { 41, "", "",             {5}}, { 43, "", "",  {5}}, { 45, "", "",             {5}}},
        {{ 19, "", "",            {45}}, {  9, "", "",            {20}}, { 48, "", "",      {  6,  12}}},                                                         
        {{ 57, "", "", { 48,  64,  75}}, {117, "", "",             {1}}, { 45, "", "",            {50}}},                         
        {{135, "", "",            {50}}, { 22, "", "",            {15}}, { 62, "", "",             {6}}},                                                         
        {{ 34, "", "",             {3}}, { 35, "", "",             {3}}, {113, "", "",            {50}}, { 79, "", "",           {100}}},
        {{ 21, "", "",             {1}}, { 22, "", "",            {15}}, { 80, "", "",            {50}}, {  9, "", "",            {-5}}},                         
        {{  2, "", "",            {10}}, { 17, "", "",            {50}}, {119, "", "",             {1}}, { 43, "", "",            {25}}, { 39, "", "",            {25}}},
        {{ 19, "", "",            {75}}, {153, "", "",             {1}}, { 57, "", "", { 24,  48,  75}}},                                                         
        {{105, "", "",            {20}}, {  9, "", "",            {50}}, { 19, "", "",            {55}}, { 39, "", "",            {10}}, { 41, "", "",            {10}}, { 43, "", "", {10}}, { 45, "", "",            {10}}},
        {{ 19, "", "",            {35}}, { 21, "", "",            {10}}, { 89, "", "",             {2}}, {122, "", "",           {100}}},                         
        {{150, "", "",            {25}}, {116, "", "",            {50}}, {  0, "", "",            {15}}, {  2, "", "",             {8}}, {119, "", "",            {30}}},
        {{135, "", "",            {35}}, {  0, "", "",            {10}}, { 60, "", "",             {8}}, {  7, "", "",            {26}}},                         
        {{ 78, "", "",             {8}}, { 50, "", "",      {  1,  40}}, { 39, "", "",            {15}}, { 41, "", "",            {15}}, { 43, "", "",            {15}}, { 45, "", "", {15}}},
        {{ 48, "", "",      { 12,  18}}, { 40, "", "",            {15}}, {  7, "", "",            {30}}, { 19, "", "",            {60}}, { 89, "", "",             {3}}, { 39, "", "", {15}}},
        {{  2, "", "",            {15}}, { 17, "", "",           {100}}, { 19, "", "",            {20}}, { 31, "", "",            {-8}}},
        {{112, "", "",            {75}}, {141, "", "",            {30}}, { 19, "", "",            {30}}, { 39, "", "",             {5}}, { 41, "", "",             {5}}, { 43, "", "",  {5}}, { 45, "", "",             {5}}},
        {{135, "", "",            {30}}, { 19, "", "",            {45}}, { 39, "", "",            {20}}, { 41, "", "",            {20}}, { 43, "", "",            {20}}, { 45, "", "", {20}}, { 57, "", "", { 32,  48, 125}}},
        {{119, "", "",            {30}}, {  2, "", "",            {10}}, { 17, "", "",            {50}}, { 19, "", "",            {40}}, { 60, "", "",             {7}}},
        {{ 57, "", "", { 24,  96,  76}}, {150, "", "",            {50}}, { 46, "", "",            {15}}, { 45, "", "",            {15}}, { 89, "", "",             {2}}},
        {{141, "", "",            {50}}, {117, "", "",             {1}}, { 62, "", "",             {5}}},                         
        {{ 48, "", "",      {  4,   6}}, { 39, "", "",            {50}}, {  9, "", "",            {30}}, { 19, "", "",            {20}}},                         
        {{ 57, "", "", {  8, 128,  75}}, { 45, "", "",            {50}}, { 89, "", "",            {-1}}, {  9, "", "",            {10}}},                         
        {{ 39, "", "",            {30}}, { 41, "", "",            {30}}, { 43, "", "",            {30}}, { 45, "", "",            {30}}, { 35, "", "",             {3}}, {  9, "", "", {20}}, { 19, "", "",            {20}}, {  0, "", "",  {9}}, { 48, "", "", {  7,   12}}},
        {{126, "", "",             {2}}, { 48, "", "",      {  1,  10}}, { 39, "", "",            {20}}, { 19, "", "",            {50}}},                         
        {{ 50, "", "",      {  1,  25}}, {119, "", "",            {50}}, { 31, "", "",            {25}}, { 83, "", "",      {  1,   1}}, {105, "", "",            {20}}},
        {{ 19, "", "",            {28}}, { 17, "", "",           {100}}, {  7, "", "",            {10}}, { 89, "", "",             {2}}, { 62, "", "",             {3}}},
        {{119, "", "",             {1}}, { 21, "", "",             {1}}, { 22, "", "",             {3}}, { 19, "", "",            {50}}, {157, "", "",            {35}}},
        {{119, "", "",            {50}}, {  2, "", "",             {3}}, {158, "", "",            {30}}, {  0, "", "",             {3}}},                         
        {{ 39, "", "",            {10}}, { 41, "", "",            {10}}, { 43, "", "",            {10}}, { 45, "", "",            {10}}, {141, "", "",            {30}}, { 19, "", "", {60}}, {122, "", "",           {100}}},
        {{ 50, "", "",      {  1,  10}}, {  0, "", "",             {8}}, { 19, "", "",            {28}}, {156, "", "",            {10}}, { 41, "", "",            {25}}},
        {{157, "", "",             {1}}, {  9, "", "",            {30}}, { 19, "", "",            {20}}, { 43, "", "",            {26}}, { 19, "", "",            {36}}},
        {{119, "", "",            {10}}, { 48, "", "",      {  6,   9}}, { 19, "", "",            {20}}, { 39, "", "",            {40}}, {  2, "", "",            {12}}},
        {{ 17, "", "",            {50}}, {  0, "", "",             {5}}, { 83, "", "",      {  0,   1}}, { 62, "", "",             {3}}},                         
        {{  2, "", "",            {10}}, {  7, "", "",            {10}}, { 17, "", "",            {50}}, { 45, "", "",            {30}}, {141, "", "",            {25}}, { 19, "", "", {40}}},
        {{ 57, "", "", { 24,  32,  75}}, {  2, "", "",            {20}}, {156, "", "",            {50}}, { 19, "", "",            {50}}},                         
        {{158, "", "",             {1}}, { 40, "", "",            {15}}, { 39, "", "",            {15}}, { 19, "", "",            {70}}, { 19, "", "",            {20}}},
        {{ 83, "", "",      {  0,   1}}, {156, "", "",            {35}}, {119, "", "",            {30}}, {  7, "", "",            {15}}},
        {{  7, "", "",            {15}}, { 19, "", "",            {30}}, { 17, "", "",            {40}}, {  9, "", "",            {15}}},
        {{ 79, "", "",            {75}}, { 80, "", "",            {25}}, {127, "", "",             {1}}},
        {{128, "", "",             {4}}, {113, "", "",             {1}}, { 41, "", "",            {15}}, { 32, "", "",            {25}}},
        {{ 89, "", "",            {-2}}, { 39, "", "",             {5}}, { 41, "", "",             {5}}, { 43, "", "",             {5}}, { 45, "", "",             {5}}, { 36, "", "",  {3}}, { 22, "", "",             {8}}},
        {{ 83, "", "",       {  2,  1}}, { 60, "", "",             {5}}, {  9, "", "",            {10}}, { 45, "", "",            {25}}},
        {{ 35, "", "",             {2}}, { 78, "", "",             {3}}, { 16, "", "",            {40}}, {114, "", "",            {35}}},
        {{ 60, "", "",             {4}}, { 31, "", "",             {8}}, { 45, "", "",            {50}}, {118, "", "",             {1}}},
        {{112, "", "",             {1}}, {  0, "", "",            {20}}, { 39, "", "",             {5}}, { 41, "", "",             {5}}, { 43, "", "",             {5}}, { 45, "", "",  {5}}, {122, "", "",            {50}}},
        {{ 35, "", "",             {3}}, { 43, "", "",            {20}}, { 39, "", "",            {20}}, {  2, "", "",            {10}}, { 60, "", "",             {5}}},
        {{ 32, "", "",            {16}}, { 96, "", "",             {1}}, { 31, "", "",            {25}}, { 48, "", "",      {  3,   6}}},
        {{ 31, "", "",            {30}}, { 19, "", "",            {25}}, { 36, "", "",             {2}}, {  7, "", "",            {15}}, {  9, "", "",            {15}}, { 11, "", "", {15}}, { 74, "", "",             {3}}},
        {{119, "", "",             {1}}, {  2, "", "",            {10}}, { 20, "", "",            {25}}, { 31, "", "",            {15}}, {  0, "", "",            {10}}},
        {{ 19, "", "",            {20}}, { 40, "", "",             {5}}, { 42, "", "",             {5}}, { 44, "", "",             {5}}, { 46, "", "",             {5}}, { 89, "", "",  {3}}, { 33, "", "",            {25}}, { 39, "", "", {10}}, { 41, "", "",        {10}}, { 43, "", "", {10}}, { 45, "", "", {10}}},
        {{ 31, "", "",            {12}}, { 44, "", "",            {15}}, { 43, "", "",            {15}}, {153, "", "",             {1}}},
        {{ 31, "", "",            {40}}, { 50, "", "",      {  1,  10}}, {128, "", "",             {1}}},               
        {{ 57, "", "", { 24,  32,  75}}, { 46, "", "",            {15}}, { 89, "", "",             {2}}, { 17, "", "",            {50}}, { 45, "", "",            {15}}},
        {{134, "", "",             {1}}, { 43, "", "",            {30}}, { 89, "", "",             {4}}, { 35, "", "",             {1}}},
        {{ 60, "", "",             {5}}, { 31, "", "",            {35}}, { 19, "", "",            {35}}},               
        {{ 91, "", "",           {-10}}, { 31, "", "",            {25}}, { 45, "", "",            {50}}, { 36, "", "",             {3}}, {  0, "", "",             {5}}},
        {{112, "", "",             {1}}, { 19, "", "",            {45}}, {136, "", "",            {25}}, { 31, "", "",            {45}}},
        {{ 31, "", "",            {75}}, { 39, "", "",            {30}}, { 41, "", "",            {30}}, { 43, "", "",            {30}}, { 45, "", "",            {30}}, { 78, "", "",  {1}}, { 89, "", "",   {2}}},
        {{ 62, "", "",             {5}}, {127, "", "",             {1}}, { 89, "", "",             {2}}},
        {{ 16, "", "",           {100}}, { 39, "", "",            {10}}, { 41, "", "",            {10}}, { 43, "", "",            {10}}, { 45, "", "",            {10}}, { 27, "", "", {25}}},
        {{ 31, "", "",            {40}}, {  7, "", "",            {10}}, {  0, "", "",             {2}}, {  9, "", "",            {10}}},
        {{113, "", "",             {1}}, {  2, "", "",            {10}}, { 31, "", "",            {18}}, {  7, "", "",            {20}}, { 89, "", "",            {-2}}},
        {{ 35, "", "",             {1}}, { 41, "", "",            {25}}, { 31, "", "",            {15}}, { 50, "", "",      {  1,   6}}},
        {{138, "", "",             {5}}, { 62, "", "",             {3}}, {105, "", "",            {20}}, { 45, "", "",            {20}}},
        {{ 78, "", "",             {5}}, { 20, "", "",            {20}}, {135, "", "",            {50}}},
        {{ 20, "", "",            {25}}, { 83, "", "",      {  3,   1}}, { 36, "", "",             {3}}, { 89, "", "",             {3}}},
        {{ 31, "", "",            {20}}, { 39, "", "",            {25}}, {  0, "", "",             {5}}, { 35, "", "",             {4}}},
        {{31, "", "",             {40}}, {35, "", "",              {2}}, {0, "", "",              {10}}, {20, "", "",             {10}}},
        {{ 62, "", "",             {3}}, { 60, "", "",             {3}}, { 45, "", "",            {10}}, {  9, "", "",            {20}}},
        {{ 21, "", "",             {5}}, {  7, "", "",            {40}}, { 99, "", "",            {30}}},
        {{ 79, "", "",           {200}}, { 80, "", "",            {24}}, { 19, "", "",            {25}}, { 31, "", "",            {15}}, { 89, "", "",             {2}}},
        {{105, "", "",            {20}}, { 27, "", "",            {25}}, {126, "", "",             {1}}, { 48, "", "",      {  1,   5}}},
        {{ 31, "", "",            {30}}, { 17, "", "",            {85}}, { 77, "", "",            {40}}, { 54, "", "", {  1,   6,  50}}},
        {{231, "", "",            {15}}, {  7, "", "",            {15}}, { 48, "", "",      {  3,   6}}},
        {{ 96, "", "",            {10}}, { 62, "", "",             {2}}, { 78, "", "",             {2}}},
        {{ 96, "", "",             {1}}, { 32, "", "",            {50}}, {154, "", "",             {50}}, {  7, "", "",           {10}}},
        {{136, "", "",            {25}}, { 36, "", "",             {1}}, { 35, "", "",             {1}}, { 31, "", "",             {5}}, { 89, "", "",            {-1}}},
        {{ 31, "", "",            {10}}, {  0, "", "",             {5}}, {  2, "", "",             {5}}, { 96, "", "",             {1}}, { 39, "", "",            {10}}, { 41, "", "", {10}}, { 43, "", "",            {10}}, { 45, "", "", {10}} },
        {{  9, "", "",            {15}}, { 27, "", "",            {30}}, { 39, "", "",             {5}}, { 41, "", "",             {5}}, { 43, "", "",             {5}}, { 45, "", "",  {5}}, { 89, "", "",             {1}}},
        {{ 57, "", "", { 16,  24,  75}}, { 45, "", "",            {25}}, { 31, "", "",            {11}}, { 74, "", "",             {5}}},
        {{ 39, "", "",            {10}}, { 41, "", "",            {10}}, { 43, "", "",            {10}}, { 45, "", "",            {10}}, {114, "", "",            {50}}, {  9, "", "", {20}}, { 36, "", "",             {2}}},
        {{ 80, "", "",            {30}}, { 89, "", "",             {2}}, { 31, "", "",            {40}}, {119, "", "",            {10}}},
        {{ 78, "", "",             {8}}, { 31, "", "",            {20}}, { 36, "", "",             {3}}, {  0, "", "",             {5}}, {  2, "", "",            {10}}},
        {{ 48, "", "",      {  3,   6}}, { 39, "", "",            {10}}, { 89, "", "",             {3}}, { 40, "", "",            {10}}},
        {{ 32, "", "",            {10}}, { 89, "", "",             {1}}, {127, "", "",             {1}}, { 60, "", "",             {3}}, { 54, "", "", {  1,   3,  50}}},
        {{  2, "", "",             {5}}, {  9, "", "",            {20}}, {  0, "", "",             {5}}, {  7, "", "",            {20}}, { 31, "", "",            {10}}, { 19, "", "", {40}}},
        {{ 35, "", "",             {2}}, { 78, "", "",             {3}}, { 19, "", "",            {18}}, { 80, "", "",            {15}}},
        {{ 62, "", "",             {4}}, { 74, "", "",             {5}}, {  7, "", "",            {20}}},
        {{ 9, "", "",             {20}}, { 77, "", "",            {25}}, { 50, "", "",      {  1,  12}}, { 127, "", "",            {1}}},
        {{  9, "", "",            {10}}, { 45, "", "",            {25}}, {  7, "", "",            {10}}},
        {{ 39, "", "",            {10}}, { 41, "", "",            {10}}, { 43, "", "",            {10}}, { 45, "", "",            {10}}, {119, "", "",            {50}}},
        {{  9, "", "",            {10}}, { 45, "", "",            {25}}, {  7, "", "",            {10}}, { 39, "", "",            {10}}, { 41, "", "",            {10}}, { 43, "", "", {10}}, { 45, "", "",            {10}}, {119, "", "", {50}}},
        {{ 48, "", "",      {  5,  20}}, { 39, "", "",            {40}}, { 31, "", "",            {35}}},
        {{ 50, "", "",      {  1,  20}}, {119, "", "",            {50}}, { 19, "", "",            {40}}},
        {{ 50, "", "",      {  1,  40}}, {119, "", "",            {50}}, { 19, "", "",            {40}}, { 62, "", "",             {6}}, { 60, "", "",             {6}}}
    };

    struct ItemV100Info
    {
        char Quality = ' ';
        char ItemSearch = ' ';
        int UniqueCode = 0;
        int DWBCode = 0;
        int MagicMask = 0;
        int RareMask = 0;
        int QualityMask = 0;
    };

    const std::map<int, std::vector<ItemV100Info>> itemV100Info = {
        {0x000, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000B}, {'U', 'U', 0x0000, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x001, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000B}, {'U', 'U', 0x0001, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x002, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000F}, {'S', 'S', 0x000B, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0002, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x003, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000F}, {'S', 'S', 0x0008, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0003, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x004, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000B}, {'U', 'U', 0x0004, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x005, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000B}, {'U', 'U', 0x0005, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x006, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000B}, {'U', 'U', 0x0006, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x007, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000B}, {'U', 'U', 0x0007, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x008, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000B}, {'U', 'U', 0x0008, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x009, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x000B}, {'U', 'U', 0x0009, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x00A, {{'N', 'N', 0x0000, 0x0000, 0x0020, 0x0100, 0x000B}, {'U', 'U', 0x000A, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x00B, {{'N', 'N', 0x0000, 0x0000, 0x0020, 0x0100, 0x000B}, {'U', 'U', 0x000B, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x00C, {{'N', 'N', 0x0000, 0x0000, 0x0020, 0x0100, 0x000B}, {'U', 'U', 0x000C, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x00D, {{'N', 'N', 0x0000, 0x0000, 0x0020, 0x0100, 0x000F}, {'S', 'S', 0x000A, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x000D, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x00E, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x000B}, {'U', 'U', 0x000E, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x00F, {{'N', 'N', 0x0000, 0x0000, 0x0010, 0x0080, 0x000B}, {'U', 'U', 0x000F, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x010, {{'N', 'N', 0x0000, 0x0000, 0x0010, 0x0080, 0x000F}, {'S', 'S', 0x0000, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0010, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x011, {{'N', 'N', 0x0000, 0x0000, 0x0010, 0x0080, 0x000F}, {'S', 'S', 0x0006, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0011, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x012, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x000B}, {'U', 'U', 0x0012, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x013, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x000B}, {'U', 'U', 0x0013, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x014, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x000B}, {'U', 'U', 0x0014, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x015, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x000B}, {'U', 'U', 0x0015, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x016, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x000B}, {'U', 'U', 0x0016, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x017, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x000B}, {'U', 'U', 0x0017, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x018, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x000B}, {'U', 'U', 0x0018, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x019, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0019, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x01A, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x001A, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x01B, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000F}, {'S', 'S', 0x000D, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x001B, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x01C, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x001C, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x01D, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x001D, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x01E, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000F}, {'S', 'S', 0x0004, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x001E, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x01F, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000F}, {'S', 'S', 0x0002, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x001F, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x020, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000F}, {'S', 'S', 0x000C, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0020, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x021, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0021, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x022, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0022, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x023, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0023, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x024, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0024, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x025, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0025, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x026, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0026, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x027, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0027, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x028, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0028, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x029, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x0029, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x02A, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x000B}, {'U', 'U', 0x002A, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x02B, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x02C, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x02D, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x02E, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x02F, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x030, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x031, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x032, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x033, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x034, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x002B, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x035, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x002C, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x036, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x002D, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x037, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x002E, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x038, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x002F, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x039, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x0030, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x03A, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x0031, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x03B, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x0032, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x03C, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x0033, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x03D, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x0034, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x03E, {{'N', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x000B}, {'U', 'U', 0x0035, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x03F, {{'N', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x000B}, {'U', 'U', 0x0036, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x040, {{'N', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x000B}, {'U', 'U', 0x0037, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x041, {{'N', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x000B}, {'U', 'U', 0x0038, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x042, {{'N', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x000F}, {'S', 'S', 0x0007, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0039, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x043, {{'N', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x000F}, {'S', 'S', 0x000F, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x003A, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x044, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x003B, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x045, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x003C, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x046, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x003D, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x047, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x003E, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x048, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x003F, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x049, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000F}, {'S', 'S', 0x0005, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0040, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x04A, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000F}, {'S', 'S', 0x000E, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0041, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x04B, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x0042, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x04C, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x0043, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x04D, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x0044, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x04E, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x0045, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x04F, {{'N', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x000B}, {'U', 'U', 0x0046, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x050, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x051, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x052, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x053, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x054, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x055, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x056, {{'Q', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x057, {{'Q', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x058, {{'Q', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x059, {{'Q', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x05A, {{'U', 'U', 0x007E, 0x0000, 0x0002, 0x0010, 0x0008}}},
        {0x05B, {{'U', 'U', 0x007D, 0x0000, 0x0040, 0x0200, 0x0008}}},
        {0x05C, {{'U', 'U', 0x007C, 0x0000, 0x0040, 0x0200, 0x0008}}},
        {0x05D, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x05E, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x05F, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x060, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x061, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x062, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x063, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x064, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x065, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x066, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0010, 0x0003}}},
        {0x067, {{'E', 'N', 0x0000, 0x0000, 0x0020, 0x0100, 0x0003}}},
        {0x068, {{'E', 'N', 0x0000, 0x0000, 0x0020, 0x0100, 0x0003}}},
        {0x069, {{'E', 'N', 0x0000, 0x0000, 0x0020, 0x0100, 0x0003}}},
        {0x06A, {{'E', 'N', 0x0000, 0x0000, 0x0020, 0x0100, 0x0003}}},
        {0x06B, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x06C, {{'E', 'N', 0x0000, 0x0000, 0x0010, 0x0080, 0x0003}}},
        {0x06D, {{'E', 'N', 0x0000, 0x0000, 0x0010, 0x0080, 0x0003}}},
        {0x06E, {{'E', 'N', 0x0000, 0x0000, 0x0010, 0x0080, 0x0003}}},
        {0x06F, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x070, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x071, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x072, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x073, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x074, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x075, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x076, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x077, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x078, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x079, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x07A, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x07B, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x07C, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x07D, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x07E, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x07F, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x080, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x081, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x082, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x083, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x084, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x085, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x086, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x087, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0008, 0x0003}}},
        {0x088, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x089, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x08A, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x08B, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x08C, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x08D, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x08E, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x08F, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x090, {{'E', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x091, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0001}}},
        {0x092, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x093, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x094, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x095, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x096, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x097, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x098, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x099, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x09A, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x09B, {{'E', 'N', 0x0000, 0x0000, 0x0002, 0x0040, 0x0003}}},
        {0x09C, {{'E', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x0003}}},
        {0x09D, {{'E', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x0003}}},
        {0x09E, {{'E', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x0003}}},
        {0x09F, {{'E', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x0003}}},
        {0x0A0, {{'E', 'N', 0x0000, 0x0000, 0x0040, 0x0200, 0x0003}}},
        {0x0A1, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0A2, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0A3, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0A4, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0A5, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0A6, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0A7, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0A8, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0A9, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0AA, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0AB, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0AC, {{'E', 'N', 0x0000, 0x0000, 0x0080, 0x0400, 0x0003}}},
        {0x0AD, {{'Q', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x0AE, {{'Q', 'N', 0x0000, 0x0000, 0x0002, 0x0020, 0x0003}}},
        {0x0AF, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x000F}, {'S', 'S', 0x000A, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0047, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B0, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x000F}, {'S', 'S', 0x000F, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0048, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B1, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x000F}, {'S', 'S', 0x000B, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0049, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B2, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x000F}, {'S', 'S', 0x0004, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x004A, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B3, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x000F}, {'S', 'S', 0x0009, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x004C, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B4, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x000F}, {'S', 'S', 0x0003, 0x0070, 0x0000, 0x0000, 0x000F}, {'S', 'S', 0x0006, 0xFF8F, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x004D, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B5, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x000F}, {'S', 'S', 0x0007, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x004E, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B6, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x000E, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x004F, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B7, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x0005, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0050, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0B8, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'U', 'U', 0x0051, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x0B9, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000B}, {'U', 'U', 0x0052, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0BA, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x000D, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0053, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0BB, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'U', 'U', 0x0054, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0BC, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x0007, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0055, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0BD, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x0004, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0056, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0BE, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x000B, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0057, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0BF, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000B}, {'U', 'U', 0x0058, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x0C0, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000B}, {'U', 'U', 0x0059, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x0C1, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x0009, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x005A, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0C2, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x0008, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x005B, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0C3, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x0006, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x005C, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0C4, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x000F}, {'S', 'S', 0x000F, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x005D, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0C5, {{'N', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x000F}, {'S', 'S', 0x0001, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x005E, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0C6, {{'N', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x000F}, {'S', 'S', 0x0002, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x005F, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0C7, {{'N', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x000F}, {'S', 'S', 0x0000, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0060, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0C8, {{'N', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x000F}, {'S', 'S', 0x0006, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0063, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0C9, {{'N', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x000F}, {'S', 'S', 0x0009, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0064, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0CA, {{'N', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x000F}, {'S', 'S', 0x0004, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0065, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0CB, {{'N', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x000F}, {'S', 'S', 0x000C, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0066, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0CC, {{'N', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x000B}, {'U', 'U', 0x0067, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x0CD, {{'N', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x000F}, {'S', 'S', 0x0002, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0068, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0CE, {{'N', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x000F}, {'S', 'S', 0x0003, 0x7FF0, 0x0000, 0x0000, 0x000F}, {'S', 'S', 0x000E, 0x800F, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0069, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0CF, {{'N', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x000F}, {'S', 'S', 0x0009, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x006A, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D0, {{'N', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x000F}, {'S', 'S', 0x0008, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x006B, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D1, {{'N', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x000B}, {'U', 'U', 0x006C, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x0D2, {{'N', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x000F}, {'S', 'S', 0x0001, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x006D, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D3, {{'N', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x000F}, {'S', 'S', 0x0005, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x006E, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D4, {{'N', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x000F}, {'S', 'S', 0x0009, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x006F, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D5, {{'N', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x000F}, {'S', 'S', 0x000C, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0070, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D6, {{'N', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x000F}, {'S', 'S', 0x000E, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0071, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D7, {{'N', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x000F}, {'S', 'S', 0x0001, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0072, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D8, {{'N', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x000F}, {'S', 'S', 0x0003, 0x07F0, 0x0000, 0x0000, 0x000F}, {'S', 'S', 0x000A, 0xF80F, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0073, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0D9, {{'N', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x000F}, {'S', 'S', 0x0009, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x0074, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0DA, {{'N', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x000F}, {'S', 'S', 0x0008, 0x0000, 0x0000, 0x0000, 0x000F}, {'U', 'U', 0x004B, 0x0000, 0x0000, 0x0000, 0x000F}}},
        {0x0DB, {{'N', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x000B}, {'U', 'U', 0x0061, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x0DC, {{'N', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x000B}, {'U', 'U', 0x0062, 0x0000, 0x0000, 0x0000, 0x000B}}},
        {0x0DD, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x0003}}},
        {0x0DE, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x0003}}},
        {0x0DF, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x0003}}},
        {0x0E0, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x0003}}},
        {0x0E1, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x0003}}},
        {0x0E2, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x0003}}},
        {0x0E3, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x0003}}},
        {0x0E4, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0E5, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0E6, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0E7, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0E8, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0E9, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0EA, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0EB, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0EC, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0ED, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0EE, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0EF, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0F0, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0F1, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0F2, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0001, 0x0003}}},
        {0x0F3, {{'E', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x0003}}},
        {0x0F4, {{'E', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x0003}}},
        {0x0F5, {{'E', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x0003}}},
        {0x0F6, {{'E', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x0003}}},
        {0x0F7, {{'E', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x0003}}},
        {0x0F8, {{'E', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x0003}}},
        {0x0F9, {{'E', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x0003}}},
        {0x0FA, {{'E', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x0003}}},
        {0x0FB, {{'E', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x0003}}},
        {0x0FC, {{'E', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x0003}}},
        {0x0FD, {{'E', 'N', 0x0000, 0x0000, 0x0200, 0x1000, 0x0003}}},
        {0x0FE, {{'E', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x0003}}},
        {0x0FF, {{'E', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x0003}}},
        {0x100, {{'E', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x0003}}},
        {0x101, {{'E', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x0003}}},
        {0x102, {{'E', 'N', 0x0000, 0x0000, 0x0100, 0x0800, 0x0003}}},
        {0x103, {{'E', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x0003}}},
        {0x104, {{'E', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x0003}}},
        {0x105, {{'E', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x0003}}},
        {0x106, {{'E', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x0003}}},
        {0x107, {{'E', 'N', 0x0000, 0x0000, 0x0400, 0x2000, 0x0003}}},
        {0x108, {{'E', 'N', 0x0000, 0x0000, 0x0001, 0x0002, 0x0003}}},
        {0x109, {{'E', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x0003}}},
        {0x10A, {{'E', 'N', 0x0000, 0x0000, 0x0004, 0x0004, 0x0003}}},
        {0x10B, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x10C, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x10D, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x10E, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x10F, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x110, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x111, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x112, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x113, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x114, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x115, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x116, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x117, {{'M', 'M', 0x0000, 0x0000, 0x1000, 0x8000, 0x000E}, {'S', 'S', 0x0000, 0x000E, 0x0000, 0x0000, 0x000E}, {'S', 'S', 0x0003, 0x0030, 0x0000, 0x0000, 0x000E}, {'S', 'S', 0x0005, 0x00C0, 0x0000, 0x0000, 0x000E}, {'S', 'S', 0x0007, 0x0100, 0x0000, 0x0000, 0x000E}, {'S', 'S', 0x0008, 0x3E00, 0x0000, 0x0000, 0x000E}, {'S', 'S', 0x000D, 0xC000, 0x0000, 0x0000, 0x000E}, {'S', 'S', 0x000F, 0x0001, 0x0000, 0x0000, 0x000E}, {'U', 'U', 0x0075, 0x0000, 0x0000, 0x0000, 0x000E}, {'U', 'U', 0x0076, 0x0000, 0x0000, 0x0000, 0x000E}, {'U', 'U', 0x0077, 0x0000, 0x0000, 0x0000, 0x000E}, {'U', 'U', 0x007B, 0x0000, 0x0000, 0x0000, 0x000E}}},
        {0x118, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0001}}},
        {0x119, {{'M', 'M', 0x0000, 0x0000, 0x0800, 0x4000, 0x000E}, {'S', 'S', 0x0007, 0x3F00, 0x0000, 0x0000, 0x000E}, {'S', 'S', 0x000D, 0xC0FF, 0x0000, 0x0000, 0x000E}, {'U', 'U', 0x0078, 0x0000, 0x0000, 0x0000, 0x000E}, {'U', 'U', 0x0079, 0x0000, 0x0000, 0x0000, 0x000E}, {'U', 'U', 0x007A, 0x0000, 0x0000, 0x0000, 0x000E}}},
        {0x11A, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x11B, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x11C, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x11D, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x11E, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x11F, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x120, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x121, {{'N', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x122, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x123, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x124, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x125, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x126, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x127, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x128, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x129, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x12A, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x12B, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x12C, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x12D, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x12E, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x12F, {{'X', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x130, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x131, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x132, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x133, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x134, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x135, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x136, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x137, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x138, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x139, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x13A, {{'Q', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x13B, {{'R', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x13C, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x13D, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x13E, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x13F, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x140, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x141, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x142, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x143, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x144, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x145, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x146, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x147, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x148, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x149, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x14A, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x14B, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x14C, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x14D, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x14E, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x14F, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x150, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x151, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x152, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x153, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x154, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x155, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x156, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x157, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x158, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x159, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x15A, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x15B, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x15C, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x15D, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x15E, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x15F, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x160, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x161, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x162, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x163, {{'A', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x164, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x165, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x166, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x167, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}},
        {0x168, {{'G', 'N', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}}}
    };

    const std::vector<ItemV100Info>& getItemV100Info(std::uint16_t typeCode)
    {
        static std::vector<ItemV100Info> invalidItem;

        auto iter = itemV100Info.find(typeCode);
        if (iter != itemV100Info.end())
        {
            return iter->second;
        }

        return invalidItem;
    }
            
    std::uint32_t getSetDWBCodev100(std::uint16_t id, const std::array<std::uint8_t, 4>& strcode)
    {
        auto typeCode = ItemHelpers::getTypeCodeV100(strcode);
        if (typeCode >= UINT16_MAX)
        {
            return 0;
        }

        const auto& itemInfo = getItemV100Info(typeCode);
        for (const auto& info : itemInfo)
        {
            if ((info.Quality == 'S') && (info.UniqueCode == id))
            {
                return info.DWBCode;
            }
        }

        return 0;
    }

    struct ItemRandStruct
    {
        std::uint32_t seed = 0;
        std::uint32_t carry = 0;
    };

    uint32_t generateRandomv100(ItemRandStruct& rnd)
    {
        std::uint64_t x = rnd.seed;
        x *= 0x6AC690C5;
        x += rnd.carry;
        rnd.seed = std::uint32_t(x);
        rnd.carry = std::uint32_t(x >> 32);
        return rnd.seed;
    }

    std::uint32_t initalizeItemRandomizationv100(std::uint32_t dwb, std::uint16_t magicLevel, EnumItemQuality quality, ItemRandStruct& rnd)
    {
        // Intialize random algorithm
        rnd.seed = dwb;
        rnd.carry = 666;

        static std::int64_t usualAdd[] = { 1000, 200, 125,  30, 12, 4 };
        static std::int64_t usualDiv[] = { 1,   1,   1,  16, 16, 8 };
        static std::int64_t excepAdd[] = { 600, 120, 100,   3,  4, 1 };
        static std::int64_t excepDiv[] = { 1,   1,   1, 100, 16, 8 };

        auto& add = quality == EnumItemQuality::SUPERIOR ? excepAdd : usualAdd;
        auto& div = quality == EnumItemQuality::SUPERIOR ? excepDiv : usualDiv;

        std::uint32_t rands = 0;
        for (size_t z = 0; z < 6; ++z)
        {
            std::uint64_t modulo = std::max(1ui64, std::uint64_t(add[z] - (magicLevel / div[z])));
            ++rands;
            if ((generateRandomv100(rnd) % modulo) == 0)
            {
                break;
            }
        }

        return rands;
    }
    
}

//---------------------------------------------------------------------------
bool d2ce::ItemType::isStackable() const
{
    return stackable;
}

bool d2ce::ItemType::isWeapon() const
{
    return std::find(categories.begin(), categories.end(), "Weapon") != categories.end() ? true : false;
}

bool d2ce::ItemType::isMissileWeapon() const
{
    return std::find(categories.begin(), categories.end(), "Missile Weapon") != categories.end() ? true : false;
}

bool d2ce::ItemType::isTwoHandedWeapon() const
{
    if (!isWeapon())
    {
        return false;
    }

    return dam.bTwoHanded;
}

bool d2ce::ItemType::isShield() const
{
    return std::find(categories.begin(), categories.end(), "Any Shield") != categories.end() ? true : false;
}

bool d2ce::ItemType::isArmor() const
{
    return std::find(categories.begin(), categories.end(), "Any Armor") != categories.end() ? true : false;
}

bool d2ce::ItemType::isHelm() const
{
    return std::find(categories.begin(), categories.end(), "Helm") != categories.end() ? true : false;
}

bool d2ce::ItemType::isBook() const
{
    return std::find(categories.begin(), categories.end(), "Book") != categories.end() ? true : false;
}

bool d2ce::ItemType::isPotion() const
{
    return std::find(categories.begin(), categories.end(), "Potion") != categories.end() ? true : false;
}

bool d2ce::ItemType::isJewel() const
{
    return std::find(categories.begin(), categories.end(), "Jewel") != categories.end() ? true : false;
}

bool d2ce::ItemType::isGem() const
{
    return std::find(categories.begin(), categories.end(), "Gem") != categories.end() ? true : false;
}

bool d2ce::ItemType::isQuestItem() const
{
    return std::find(categories.begin(), categories.end(), "Quest") != categories.end() ? true : false;
}

bool d2ce::ItemType::isGoldItem() const
{
    return std::find(categories.begin(), categories.end(), "Gold") != categories.end() ? true : false;
}

bool d2ce::ItemType::isRejuvenationPotion() const
{
    return std::find(categories.begin(), categories.end(), "Rejuv Potion") != categories.end() ? true : false;
}

bool d2ce::ItemType::isHealingPotion() const
{
    return std::find(categories.begin(), categories.end(), "Healing Potion") != categories.end() ? !isRejuvenationPotion() : false;
}

bool d2ce::ItemType::isManaPotion() const
{
    return std::find(categories.begin(), categories.end(), "Mana Potion") != categories.end() ? !isRejuvenationPotion() : false;
}

bool d2ce::ItemType::isSocketFiller() const
{
    return std::find(categories.begin(), categories.end(), "Socket Filler") != categories.end() ? true : false;
}

bool d2ce::ItemType::isUpgradableGem() const
{
    if (isSocketFiller())
    {
        return ((std::find(categories.begin(), categories.end(), "Gem") == categories.end()) ||
            (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())) ? false : true;
    }

    return false;
}

bool d2ce::ItemType::isUpgradableRejuvenationPotion() const
{
    if (isRejuvenationPotion())
    {
        return name.find("Full") == 0 ? false : true;
    }

    return false;
}

bool d2ce::ItemType::isUpgradablePotion() const
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

bool d2ce::ItemType::isRune() const
{
    return std::find(categories.begin(), categories.end(), "Rune") != categories.end() ? true : false;
}

bool d2ce::ItemType::isCharm() const
{
    return std::find(categories.begin(), categories.end(), "Charm") != categories.end() ? true : false;
}

bool d2ce::ItemType::isBelt() const
{
    return std::find(categories.begin(), categories.end(), "Belt") != categories.end() ? true : false;
}

bool d2ce::ItemType::hasUndeadBonus() const
{
    if (!isWeapon())
    {
        return false;
    }
    return std::find(categories.begin(), categories.end(), "Blunt") != categories.end() ? true : false;
}

d2ce::EnumItemType d2ce::ItemType::getEnumItemType() const
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

bool d2ce::ItemType::canHaveSockets() const
{
    return max_sockets.empty() ? false : true;
}

std::uint8_t d2ce::ItemType::getMaxSockets(std::uint8_t level) const
{
    if (max_sockets.empty())
    {
        return 0;
    }

    size_t index = level > 40 ? 3 : (level > 25 ? 1 : 0);
    return index >= max_sockets.size() ? max_sockets.back() : max_sockets.at(index);
}

std::uint8_t d2ce::ItemType::getBaseType() const
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

bool d2ce::ItemType::getSocketedMagicalAttributes(const d2ce::Item& item, std::vector<MagicalAttribute>& attribs, EnumItemType parentItemType) const
{
    attribs.clear();
    if (isSocketFiller())
    {
        if (isJewel())
        {
            return item.getMagicalAttributes(attribs);
        }

        std::vector<std::uint16_t> ids;
        std::vector<std::uint16_t> ids2;
        std::vector<std::int64_t> values;
        std::vector<std::int64_t> values2;
        if (isGem())
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
                        values.push_back(25); // TODO: duration seconds * 25?
                    }
                    else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                    {
                        values.push_back(3);
                        values.push_back(5);
                        values.push_back(35); // TODO: duration seconds * 25?
                    }
                    else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                    {
                        values.push_back(4);
                        values.push_back(7);
                        values.push_back(50); // TODO: duration seconds * 25?
                    }
                    else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                    {
                        values.push_back(6);
                        values.push_back(10);
                        values.push_back(60); // TODO: duration seconds * 25?
                    }
                    else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                    {
                        values.push_back(10);
                        values.push_back(14);
                        values.push_back(75); // TODO: duration seconds * 25?
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
                        values.push_back(35);
                        values.push_back(35);
                        values.push_back(75);
                    }
                    else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                    {
                        values.push_back(52);
                        values.push_back(52);
                        values.push_back(100);
                    }
                    else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                    {
                        values.push_back(82);
                        values.push_back(82);
                        values.push_back(125);
                    }
                    else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                    {
                        values.push_back(36);
                        values.push_back(36);
                        values.push_back(150);
                    }
                    else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                    {
                        values.push_back(147);
                        values.push_back(147);
                        values.push_back(175);
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
                    ids.push_back(60);
                    ids2.push_back(62);
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
                    ids.push_back(27);
                    ids2.push_back(74);
                    if (std::find(categories.begin(), categories.end(), "Chipped Gem") != categories.end())
                    {
                        values.push_back(8);
                        values2.push_back(2);
                    }
                    else if (std::find(categories.begin(), categories.end(), "Flawed Gem") != categories.end())
                    {
                        values.push_back(8);
                        values2.push_back(3);
                    }
                    else if (std::find(categories.begin(), categories.end(), "Standard Gem") != categories.end())
                    {
                        values.push_back(12);
                        values2.push_back(3);
                    }
                    else if (std::find(categories.begin(), categories.end(), "Flawless Gem") != categories.end())
                    {
                        values.push_back(12);
                        values2.push_back(4);
                    }
                    else if (std::find(categories.begin(), categories.end(), "Perfect Gem") != categories.end())
                    {
                        values.push_back(19);
                        values2.push_back(5);
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
        }
        else if (isRune())
        {
            if (name.find("El ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(19);
                    ids2.push_back(89);
                    values.push_back(50);
                    values2.push_back(1);
                    break;

                default:
                    ids.push_back(31);
                    ids2.push_back(89);
                    values.push_back(15);
                    values2.push_back(1);
                    break;
                }
            }
            else if (name.find("Eld ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(122);
                    ids2.push_back(124);
                    values.push_back(75);
                    values2.push_back(50);
                    break;

                case EnumItemType::Shield:
                    ids.push_back(20);
                    values.push_back(7);
                    break;

                default:
                    ids.push_back(154);
                    values.push_back(15);
                    break;
                }
            }
            else if (name.find("Tir ") == 0)
            {
                ids.push_back(138);
                values.push_back(2);
            }
            else if (name.find("Nef ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(81);
                    values.push_back(1); // TODO: is this correct?
                    break;

                default:
                    ids.push_back(32);
                    values.push_back(30);
                    break;
                }
            }
            else if (name.find("Eth ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(116);
                    values.push_back(25);
                    break;

                default:
                    ids.push_back(27);
                    values.push_back(15);
                    break;
                }
            }
            else if (name.find("Ith ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(22);
                    values.push_back(9);
                    break;

                default:
                    ids.push_back(114);
                    values.push_back(15);
                    break;
                }
            }
            else if (name.find("Tal ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(57);
                    values.push_back(154);
                    values.push_back(154);
                    values.push_back(125);
                    break;

                case EnumItemType::Shield:
                    ids.push_back(45);
                    values.push_back(35);
                    break;

                default:
                    ids.push_back(45);
                    values.push_back(30);
                    break;
                }
            }
            else if (name.find("Ral ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(48);
                    values.push_back(5);
                    values.push_back(30);
                    break;

                case EnumItemType::Shield:
                    ids.push_back(39);
                    values.push_back(35);
                    break;

                default:
                    ids.push_back(39);
                    values.push_back(30);
                    break;
                }
            }
            else if (name.find("Ort ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(50);
                    values.push_back(1);
                    values.push_back(50);
                    break;

                case EnumItemType::Shield:
                    ids.push_back(41);
                    values.push_back(35);
                    break;

                default:
                    ids.push_back(41);
                    values.push_back(30);
                    break;
                }
            }
            else if (name.find("Thul ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(54);
                    values.push_back(3);
                    values.push_back(14);
                    break;

                case EnumItemType::Shield:
                    ids.push_back(43);
                    values.push_back(35);
                    break;

                default:
                    ids.push_back(43);
                    values.push_back(30);
                    break;
                }
            }
            else if (name.find("Amn ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(60);
                    values.push_back(7);
                    break;

                default:
                    ids.push_back(78);
                    values.push_back(14);
                    break;
                }
            }
            else if (name.find("Sol ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(21);
                    values.push_back(9);
                    break;

                default:
                    ids.push_back(34);
                    values.push_back(7);
                    break;
                }
            }
            else if (name.find("Shael ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(93);
                    values.push_back(20);
                    break;

                case EnumItemType::Shield:
                    ids.push_back(102);
                    values.push_back(20);
                    break;

                default:
                    ids.push_back(99);
                    values.push_back(20);
                    break;
                }
            }
            else if (name.find("Dol ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(112);
                    values.push_back(32); // percent * 1.28
                    break;

                default:
                    ids.push_back(74);
                    values.push_back(7);
                    break;
                }
            }
            else if (name.find("Hel ") == 0)
            {
                ids.push_back(91);
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    values.push_back(-20);
                    break;

                default:
                    values.push_back(-15);
                    break;
                }
            }
            else if (name.find("Io ") == 0)
            {
                ids.push_back(3);
                values.push_back(10);
            }
            else if (name.find("Lum ") == 0)
            {
                ids.push_back(1);
                values.push_back(10);
            }
            else if (name.find("Ko ") == 0)
            {
                ids.push_back(2);
                values.push_back(10);
            }
            else if (name.find("Fal ") == 0)
            {
                ids.push_back(0);
                values.push_back(10);
            }
            else if (name.find("Lem ") == 0)
            {
                ids.push_back(79);
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    values.push_back(75);
                    break;

                default:
                    values.push_back(50);
                    break;
                }
            }
            else if (name.find("Pul ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(121);
                    ids2.push_back(123);
                    values.push_back(75);
                    values2.push_back(100);
                    break;

                default:
                    ids.push_back(16);
                    values.push_back(30);
                    break;
                }
            }
            else if (name.find("Um ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(135);
                    values.push_back(25);
                    break;

                case EnumItemType::Shield:
                    ids.push_back(39);
                    ids.push_back(41);
                    ids.push_back(43);
                    ids.push_back(45);
                    values.push_back(22);
                    break;

                default:
                    ids.push_back(39);
                    ids.push_back(41);
                    ids.push_back(43);
                    ids.push_back(45);
                    values.push_back(15);
                    break;
                }
            }
            else if (name.find("Mal ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(117);
                    values.push_back(1); // TODO: is this correct?
                    break;

                default:
                    ids.push_back(35);
                    values.push_back(7);
                    break;
                }
            }
            else if (name.find("Ist ") == 0)
            {
                ids.push_back(80);
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    values.push_back(30);
                    break;

                default:
                    values.push_back(25);
                    break;
                }
            }
            else if (name.find("Gul ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(119);
                    values.push_back(20);
                    break;

                default:
                    ids.push_back(46);
                    values.push_back(5);
                    break;
                }
            }
            else if (name.find("Vex ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(62);
                    values.push_back(7);
                    break;

                default:
                    ids.push_back(40);
                    values.push_back(5);
                    break;
                }
            }
            else if (name.find("Ohm ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(17);
                    values.push_back(50);
                    break;

                default:
                    ids.push_back(44);
                    values.push_back(5);
                    break;
                }
            }
            else if (name.find("Lo ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(141);
                    values.push_back(20);
                    break;

                default:
                    ids.push_back(42);
                    values.push_back(5);
                    break;
                }
            }
            else if (name.find("Sur ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(113);
                    values.push_back(1); // TODO: is this correct?
                    break;

                case EnumItemType::Shield:
                    ids.push_back(9);
                    values.push_back(50);
                    break;

                default:
                    ids.push_back(77);
                    values.push_back(5);
                    break;
                }
            }
            else if (name.find("Ber ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(136);
                    values.push_back(20);
                    break;

                default:
                    ids.push_back(34);
                    values.push_back(8);
                    break;
                }
            }
            else if (name.find("Jah ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(115);
                    values.push_back(1); // TODO: is this correct?
                    break;

                case EnumItemType::Shield:
                    ids.push_back(7);
                    values.push_back(50);
                    break;

                default:
                    ids.push_back(76);
                    values.push_back(5);
                    break;
                }
            }
            else if (name.find("Cham ") == 0)
            {
                switch (parentItemType)
                {
                case EnumItemType::Weapon:
                    ids.push_back(134);
                    values.push_back(3);
                    break;

                default:
                    ids.push_back(153);
                    values.push_back(1);  // TODO: is this correct?
                    break;
                }
            }
            else if (name.find("Zod ") == 0)
            {
                ids.push_back(152);
                values.push_back(1);  // TODO: is this correct?
            }
        }

        MagicalAttribute attrib;
        for (auto id : ids)
        {
            attrib.Id = id;
            attrib.Values = values;

            const auto& stat = ItemHelpers::getItemStat(attrib.Id);
            attrib.Name = stat.name;
            attrib.Desc = stat.desc;
            attribs.push_back(attrib);
        }

        for (auto id : ids2)
        {
            attrib.Id = id;
            attrib.Values = values2;

            const auto& stat = ItemHelpers::getItemStat(attrib.Id);
            attrib.Name = stat.name;
            attrib.Desc = stat.desc;
            attribs.push_back(attrib);
        }

        return true;
    }

    return false;
}

std::uint16_t d2ce::ItemType::getPotionPoints(d2ce::EnumCharClass charClass) const
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
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
const d2ce::ItemStat& d2ce::ItemHelpers::getItemStat(size_t idx)
{
    static d2ce::ItemStat dummy;
    if (idx >= itemStats.size())
    {
        return dummy;
    }

    return itemStats[idx];
}
//---------------------------------------------------------------------------
size_t d2ce::ItemHelpers::getItemStatSize()
{
    return itemStats.size();
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ItemHelpers::getTypeCodeV100(const std::array<std::uint8_t, 4>& strcode)
{
    static const std::map<std::string, int> itemCodes = {
        {"hax", 0x000}, {"axe", 0x001}, {"2ax", 0x002}, {"mpi", 0x003}, {"wax", 0x004}, {"lax", 0x005}, {"bax", 0x006}, {"btx", 0x007},
        {"gax", 0x008}, {"gix", 0x009}, {"wnd", 0x00A}, {"ywn", 0x00B}, {"bwn", 0x00C}, {"gwn", 0x00D}, {"clb", 0x00E}, {"scp", 0x00F},
        {"gsc", 0x010}, {"wsp", 0x011}, {"spc", 0x012}, {"mac", 0x013}, {"mst", 0x014}, {"fla", 0x015}, {"whm", 0x016}, {"mau", 0x017},
        {"gma", 0x018}, {"ssd", 0x019}, {"scm", 0x01A}, {"sbr", 0x01B}, {"flc", 0x01C}, {"crs", 0x01D}, {"bsd", 0x01E}, {"lsd", 0x01F},
        {"wsd", 0x020}, {"2hs", 0x021}, {"clm", 0x022}, {"gis", 0x023}, {"bsw", 0x024}, {"flb", 0x025}, {"gsd", 0x026}, {"dgr", 0x027},
        {"dir", 0x028}, {"kri", 0x029}, {"bld", 0x02A}, {"tkf", 0x02B}, {"tax", 0x02C}, {"bkf", 0x02D}, {"bal", 0x02E}, {"jav", 0x02F},
        {"pil", 0x030}, {"ssp", 0x031}, {"glv", 0x032}, {"tsp", 0x033}, {"spr", 0x034}, {"tri", 0x035}, {"brn", 0x036}, {"spt", 0x037},
        {"pik", 0x038}, {"bar", 0x039}, {"vou", 0x03A}, {"scy", 0x03B}, {"pax", 0x03C}, {"hal", 0x03D}, {"wsc", 0x03E}, {"sst", 0x03F},
        {"lst", 0x040}, {"cst", 0x041}, {"bst", 0x042}, {"wst", 0x043}, {"sbw", 0x044}, {"hbw", 0x045}, {"lbw", 0x046}, {"cbw", 0x047},
        {"sbb", 0x048}, {"lbb", 0x049}, {"swb", 0x04A}, {"lwb", 0x04B}, {"lxb", 0x04C}, {"mxb", 0x04D}, {"hxb", 0x04E}, {"rxb", 0x04F},
        {"gps", 0x050}, {"ops", 0x051}, {"gpm", 0x052}, {"opm", 0x053}, {"gpl", 0x054}, {"opl", 0x055}, {"d33", 0x056}, {"g33", 0x057},
        {"leg", 0x058}, {"hdm", 0x059}, {"hfh", 0x05A}, {"hst", 0x05B}, {"msf", 0x05C}, {"9ha", 0x05D}, {"9ax", 0x05E}, {"92a", 0x05F},
        {"9mp", 0x060}, {"9wa", 0x061}, {"9la", 0x062}, {"9ba", 0x063}, {"9bt", 0x064}, {"9ga", 0x065}, {"9gi", 0x066}, {"9wn", 0x067},
        {"9yw", 0x068}, {"9bw", 0x069}, {"9gw", 0x06A}, {"9cl", 0x06B}, {"9sc", 0x06C}, {"9qs", 0x06D}, {"9ws", 0x06E}, {"9sp", 0x06F},
        {"9ma", 0x070}, {"9mt", 0x071}, {"9fl", 0x072}, {"9wh", 0x073}, {"9m9", 0x074}, {"9gm", 0x075}, {"9ss", 0x076}, {"9sm", 0x077},
        {"9sb", 0x078}, {"9fc", 0x079}, {"9cr", 0x07A}, {"9bs", 0x07B}, {"9ls", 0x07C}, {"9wd", 0x07D}, {"92h", 0x07E}, {"9cm", 0x07F},
        {"9gs", 0x080}, {"9b9", 0x081}, {"9fb", 0x082}, {"9gd", 0x083}, {"9dg", 0x084}, {"9di", 0x085}, {"9kr", 0x086}, {"9bl", 0x087},
        {"9tk", 0x088}, {"9ta", 0x089}, {"9bk", 0x08A}, {"9b8", 0x08B}, {"9ja", 0x08C}, {"9pi", 0x08D}, {"9s9", 0x08E}, {"9gl", 0x08F},
        {"9ts", 0x090}, {"9sr", 0x091}, {"9tr", 0x092}, {"9br", 0x093}, {"9st", 0x094}, {"9p9", 0x095}, {"9b7", 0x096}, {"9vo", 0x097},
        {"9s8", 0x098}, {"9pa", 0x099}, {"9h9", 0x09A}, {"9wc", 0x09B}, {"8ss", 0x09C}, {"8ls", 0x09D}, {"8cs", 0x09E}, {"8bs", 0x09F},
        {"8ws", 0x0A0}, {"8sb", 0x0A1}, {"8hb", 0x0A2}, {"8lb", 0x0A3}, {"8cb", 0x0A4}, {"8s8", 0x0A5}, {"8l8", 0x0A6}, {"8sw", 0x0A7},
        {"8lw", 0x0A8}, {"8lx", 0x0A9}, {"8mx", 0x0AA}, {"8hx", 0x0AB}, {"8rx", 0x0AC}, {"qf1", 0x0AD}, {"qf2", 0x0AE}, {"cap", 0x0AF},
        {"skp", 0x0B0}, {"hlm", 0x0B1}, {"fhl", 0x0B2}, {"ghm", 0x0B3}, {"crn", 0x0B4}, {"msk", 0x0B5}, {"qui", 0x0B6}, {"lea", 0x0B7},
        {"hla", 0x0B8}, {"stu", 0x0B9}, {"rng", 0x0BA}, {"scl", 0x0BB}, {"chn", 0x0BC}, {"brs", 0x0BD}, {"spl", 0x0BE}, {"plt", 0x0BF},
        {"fld", 0x0C0}, {"gth", 0x0C1}, {"ful", 0x0C2}, {"aar", 0x0C3}, {"ltp", 0x0C4}, {"buc", 0x0C5}, {"sml", 0x0C6}, {"lrg", 0x0C7},
        {"kit", 0x0C8}, {"tow", 0x0C9}, {"gts", 0x0CA}, {"lgl", 0x0CB}, {"vgl", 0x0CC}, {"mgl", 0x0CD}, {"tgl", 0x0CE}, {"hgl", 0x0CF},
        {"lbt", 0x0D0}, {"vbt", 0x0D1}, {"mbt", 0x0D2}, {"tbt", 0x0D3}, {"hbt", 0x0D4}, {"lbl", 0x0D5}, {"vbl", 0x0D6}, {"mbl", 0x0D7},
        {"tbl", 0x0D8}, {"hbl", 0x0D9}, {"bhm", 0x0DA}, {"bsh", 0x0DB}, {"spk", 0x0DC}, {"xap", 0x0DD}, {"xkp", 0x0DE}, {"xlm", 0x0DF},
        {"xhl", 0x0E0}, {"xhm", 0x0E1}, {"xrn", 0x0E2}, {"xsk", 0x0E3}, {"xui", 0x0E4}, {"xea", 0x0E5}, {"xla", 0x0E6}, {"xtu", 0x0E7},
        {"xng", 0x0E8}, {"xcl", 0x0E9}, {"xhn", 0x0EA}, {"xrs", 0x0EB}, {"xpl", 0x0EC}, {"xlt", 0x0ED}, {"xld", 0x0EE}, {"xth", 0x0EF},
        {"xul", 0x0F0}, {"xar", 0x0F1}, {"xtp", 0x0F2}, {"xuc", 0x0F3}, {"xml", 0x0F4}, {"xrg", 0x0F5}, {"xit", 0x0F6}, {"xow", 0x0F7},
        {"xts", 0x0F8}, {"xlg", 0x0F9}, {"xvg", 0x0FA}, {"xmg", 0x0FB}, {"xtg", 0x0FC}, {"xhg", 0x0FD}, {"xlb", 0x0FE}, {"xvb", 0x0FF},
        {"xmb", 0x100}, {"xtb", 0x101}, {"xhb", 0x102}, {"zlb", 0x103}, {"zvb", 0x104}, {"zmb", 0x105}, {"ztb", 0x106}, {"zhb", 0x107},
        {"xh9", 0x108}, {"xsh", 0x109}, {"xpk", 0x10A}, {"elx", 0x10B}, {"hpo", 0x10C}, {"mpo", 0x10D}, {"hpf", 0x10E}, {"mpf", 0x10F},
        {"vps", 0x110}, {"yps", 0x111}, {"rvs", 0x112}, {"rvl", 0x113}, {"wms", 0x114}, {"tbk", 0x115}, {"ibk", 0x116}, {"amu", 0x117},
        {"vip", 0x118}, {"rin", 0x119}, {"gld", 0x11A}, {"bks", 0x11B}, {"bkd", 0x11C}, {"aqv", 0x11D}, {"tch", 0x11E}, {"cqv", 0x11F},
        {"tsc", 0x120}, {"isc", 0x121},
                                                                                                        {"key", 0x12E},
        {"xyz", 0x130}, {"j34", 0x131}, {"g34", 0x132}, {"bbb", 0x133}, {"box", 0x134}, {"trl", 0x135}, {"mss", 0x136}, {"ass", 0x137},
        {"qey", 0x138}, {"qhr", 0x139}, {"qbr", 0x13A}, {"ear", 0x13B}, {"gcv", 0x13C}, {"gfv", 0x13D}, {"gsv", 0x13E}, {"gzv", 0x13F},
        {"gpv", 0x140}, {"gcy", 0x141}, {"gfy", 0x142}, {"gsy", 0x143}, {"gly", 0x144}, {"gpy", 0x145}, {"gcb", 0x146}, {"gfb", 0x147},
        {"gsb", 0x148}, {"glb", 0x149}, {"gpb", 0x14A}, {"gcg", 0x14B}, {"gfg", 0x14C}, {"gsg", 0x14D}, {"glg", 0x14E}, {"gpg", 0x14F},
        {"gcr", 0x150}, {"gfr", 0x151}, {"gsr", 0x152}, {"glr", 0x153}, {"gpr", 0x154}, {"gcw", 0x155}, {"gfw", 0x156}, {"gsw", 0x157},
        {"glw", 0x158}, {"gpw", 0x159}, {"hp1", 0x15A}, {"hp2", 0x15B}, {"hp3", 0x15C}, {"hp4", 0x15D}, {"hp5", 0x15E}, {"mp1", 0x15F},
        {"mp2", 0x160}, {"mp3", 0x161}, {"mp4", 0x162}, {"mp5", 0x163}, {"skc", 0x164}, {"skf", 0x165}, {"sku", 0x166}, {"skl", 0x167},
        {"skz", 0x168}
    };

    std::string testStr("   ");
    testStr[0] = (char)strcode[0];
    testStr[1] = (char)strcode[1];
    testStr[2] = (char)strcode[2];
    auto iter = itemCodes.find(testStr);
    if (iter != itemCodes.end())
    {
        return std::uint16_t(iter->second);
    }

    return UINT16_MAX;
}
//---------------------------------------------------------------------------
bool d2ce::ItemHelpers::getItemCodev100(std::uint16_t code, std::array<std::uint8_t, 4>& strcode)
{
    static const std::map<int, std::string> typeCodes = {
        {0x000, "hax"}, {0x001, "axe"}, {0x002, "2ax"}, {0x003, "mpi"}, {0x004, "wax"}, {0x005, "lax"}, {0x006, "bax"}, {0x007, "btx"},
        {0x008, "gax"}, {0x009, "gix"}, {0x00A, "wnd"}, {0x00B, "ywn"}, {0x00C, "bwn"}, {0x00D, "gwn"}, {0x00E, "clb"}, {0x00F, "scp"},
        {0x010, "gsc"}, {0x011, "wsp"}, {0x012, "spc"}, {0x013, "mac"}, {0x014, "mst"}, {0x015, "fla"}, {0x016, "whm"}, {0x017, "mau"},
        {0x018, "gma"}, {0x019, "ssd"}, {0x01A, "scm"}, {0x01B, "sbr"}, {0x01C, "flc"}, {0x01D, "crs"}, {0x01E, "bsd"}, {0x01F, "lsd"},
        {0x020, "wsd"}, {0x021, "2hs"}, {0x022, "clm"}, {0x023, "gis"}, {0x024, "bsw"}, {0x025, "flb"}, {0x026, "gsd"}, {0x027, "dgr"},
        {0x028, "dir"}, {0x029, "kri"}, {0x02A, "bld"}, {0x02B, "tkf"}, {0x02C, "tax"}, {0x02D, "bkf"}, {0x02E, "bal"}, {0x02F, "jav"},
        {0x030, "pil"}, {0x031, "ssp"}, {0x032, "glv"}, {0x033, "tsp"}, {0x034, "spr"}, {0x035, "tri"}, {0x036, "brn"}, {0x037, "spt"},
        {0x038, "pik"}, {0x039, "bar"}, {0x03A, "vou"}, {0x03B, "scy"}, {0x03C, "pax"}, {0x03D, "hal"}, {0x03E, "wsc"}, {0x03F, "sst"},
        {0x040, "lst"}, {0x041, "cst"}, {0x042, "bst"}, {0x043, "wst"}, {0x044, "sbw"}, {0x045, "hbw"}, {0x046, "lbw"}, {0x047, "cbw"},
        {0x048, "sbb"}, {0x049, "lbb"}, {0x04A, "swb"}, {0x04B, "lwb"}, {0x04C, "lxb"}, {0x04D, "mxb"}, {0x04E, "hxb"}, {0x04F, "rxb"},
        {0x050, "gps"}, {0x051, "ops"}, {0x052, "gpm"}, {0x053, "opm"}, {0x054, "gpl"}, {0x055, "opl"}, {0x056, "d33"}, {0x057, "g33"},
        {0x058, "leg"}, {0x059, "hdm"}, {0x05A, "hfh"}, {0x05B, "hst"}, {0x05C, "msf"}, {0x05D, "9ha"}, {0x05E, "9ax"}, {0x05F, "92a"},
        {0x060, "9mp"}, {0x061, "9wa"}, {0x062, "9la"}, {0x063, "9ba"}, {0x064, "9bt"}, {0x065, "9ga"}, {0x066, "9gi"}, {0x067, "9wn"},
        {0x068, "9yw"}, {0x069, "9bw"}, {0x06A, "9gw"}, {0x06B, "9cl"}, {0x06C, "9sc"}, {0x06D, "9qs"}, {0x06E, "9ws"}, {0x06F, "9sp"},
        {0x070, "9ma"}, {0x071, "9mt"}, {0x072, "9fl"}, {0x073, "9wh"}, {0x074, "9m9"}, {0x075, "9gm"}, {0x076, "9ss"}, {0x077, "9sm"},
        {0x078, "9sb"}, {0x079, "9fc"}, {0x07A, "9cr"}, {0x07B, "9bs"}, {0x07C, "9ls"}, {0x07D, "9wd"}, {0x07E, "92h"}, {0x07F, "9cm"},
        {0x080, "9gs"}, {0x081, "9b9"}, {0x082, "9fb"}, {0x083, "9gd"}, {0x084, "9dg"}, {0x085, "9di"}, {0x086, "9kr"}, {0x087, "9bl"},
        {0x088, "9tk"}, {0x089, "9ta"}, {0x08A, "9bk"}, {0x08B, "9b8"}, {0x08C, "9ja"}, {0x08D, "9pi"}, {0x08E, "9s9"}, {0x08F, "9gl"},
        {0x090, "9ts"}, {0x091, "9sr"}, {0x092, "9tr"}, {0x093, "9br"}, {0x094, "9st"}, {0x095, "9p9"}, {0x096, "9b7"}, {0x097, "9vo"},
        {0x098, "9s8"}, {0x099, "9pa"}, {0x09A, "9h9"}, {0x09B, "9wc"}, {0x09C, "8ss"}, {0x09D, "8ls"}, {0x09E, "8cs"}, {0x09F, "8bs"},
        {0x0A0, "8ws"}, {0x0A1, "8sb"}, {0x0A2, "8hb"}, {0x0A3, "8lb"}, {0x0A4, "8cb"}, {0x0A5, "8s8"}, {0x0A6, "8l8"}, {0x0A7, "8sw"},
        {0x0A8, "8lw"}, {0x0A9, "8lx"}, {0x0AA, "8mx"}, {0x0AB, "8hx"}, {0x0AC, "8rx"}, {0x0AD, "qf1"}, {0x0AE, "qf2"}, {0x0AF, "cap"},
        {0x0B0, "skp"}, {0x0B1, "hlm"}, {0x0B2, "fhl"}, {0x0B3, "ghm"}, {0x0B4, "crn"}, {0x0B5, "msk"}, {0x0B6, "qui"}, {0x0B7, "lea"},
        {0x0B8, "hla"}, {0x0B9, "stu"}, {0x0BA, "rng"}, {0x0BB, "scl"}, {0x0BC, "chn"}, {0x0BD, "brs"}, {0x0BE, "spl"}, {0x0BF, "plt"},
        {0x0C0, "fld"}, {0x0C1, "gth"}, {0x0C2, "ful"}, {0x0C3, "aar"}, {0x0C4, "ltp"}, {0x0C5, "buc"}, {0x0C6, "sml"}, {0x0C7, "lrg"},
        {0x0C8, "kit"}, {0x0C9, "tow"}, {0x0CA, "gts"}, {0x0CB, "lgl"}, {0x0CC, "vgl"}, {0x0CD, "mgl"}, {0x0CE, "tgl"}, {0x0CF, "hgl"},
        {0x0D0, "lbt"}, {0x0D1, "vbt"}, {0x0D2, "mbt"}, {0x0D3, "tbt"}, {0x0D4, "hbt"}, {0x0D5, "lbl"}, {0x0D6, "vbl"}, {0x0D7, "mbl"},
        {0x0D8, "tbl"}, {0x0D9, "hbl"}, {0x0DA, "bhm"}, {0x0DB, "bsh"}, {0x0DC, "spk"}, {0x0DD, "xap"}, {0x0DE, "xkp"}, {0x0DF, "xlm"},
        {0x0E0, "xhl"}, {0x0E1, "xhm"}, {0x0E2, "xrn"}, {0x0E3, "xsk"}, {0x0E4, "xui"}, {0x0E5, "xea"}, {0x0E6, "xla"}, {0x0E7, "xtu"},
        {0x0E8, "xng"}, {0x0E9, "xcl"}, {0x0EA, "xhn"}, {0x0EB, "xrs"}, {0x0EC, "xpl"}, {0x0ED, "xlt"}, {0x0EE, "xld"}, {0x0EF, "xth"},
        {0x0F0, "xul"}, {0x0F1, "xar"}, {0x0F2, "xtp"}, {0x0F3, "xuc"}, {0x0F4, "xml"}, {0x0F5, "xrg"}, {0x0F6, "xit"}, {0x0F7, "xow"},
        {0x0F8, "xts"}, {0x0F9, "xlg"}, {0x0FA, "xvg"}, {0x0FB, "xmg"}, {0x0FC, "xtg"}, {0x0FD, "xhg"}, {0x0FE, "xlb"}, {0x0FF, "xvb"},
        {0x100, "xmb"}, {0x101, "xtb"}, {0x102, "xhb"}, {0x103, "zlb"}, {0x104, "zvb"}, {0x105, "zmb"}, {0x106, "ztb"}, {0x107, "zhb"},
        {0x108, "xh9"}, {0x109, "xsh"}, {0x10A, "xpk"}, {0x10B, "elx"}, {0x10C, "hpo"}, {0x10D, "mpo"}, {0x10E, "hpf"}, {0x10F, "mpf"},
        {0x110, "vps"}, {0x111, "yps"}, {0x112, "rvs"}, {0x113, "rvl"}, {0x114, "wms"}, {0x115, "tbk"}, {0x116, "ibk"}, {0x117, "amu"},
        {0x118, "vip"}, {0x119, "rin"}, {0x11A, "gld"}, {0x11B, "bks"}, {0x11C, "bkd"}, {0x11D, "aqv"}, {0x11E, "tch"}, {0x11F, "cqv"},
        {0x120, "tsc"}, {0x121, "isc"},
                                                                                                        {0x12E, "key"},
        {0x130, "xyz"}, {0x131, "j34"}, {0x132, "g34"}, {0x133, "bbb"}, {0x134, "box"}, {0x135, "trl"}, {0x136, "mss"}, {0x137, "ass"},
        {0x138, "qey"}, {0x139, "qhr"}, {0x13A, "qbr"}, {0x13B, "ear"}, {0x13C, "gcv"}, {0x13D, "gfv"}, {0x13E, "gsv"}, {0x13F, "gzv"},
        {0x140, "gpv"}, {0x141, "gcy"}, {0x142, "gfy"}, {0x143, "gsy"}, {0x144, "gly"}, {0x145, "gpy"}, {0x146, "gcb"}, {0x147, "gfb"},
        {0x148, "gsb"}, {0x149, "glb"}, {0x14A, "gpb"}, {0x14B, "gcg"}, {0x14C, "gfg"}, {0x14D, "gsg"}, {0x14E, "glg"}, {0x14F, "gpg"},
        {0x150, "gcr"}, {0x151, "gfr"}, {0x152, "gsr"}, {0x153, "glr"}, {0x154, "gpr"}, {0x155, "gcw"}, {0x156, "gfw"}, {0x157, "gsw"},
        {0x158, "glw"}, {0x159, "gpw"}, {0x15A, "hp1"}, {0x15B, "hp2"}, {0x15C, "hp3"}, {0x15D, "hp4"}, {0x15E, "hp5"}, {0x15F, "mp1"},
        {0x160, "mp2"}, {0x161, "mp3"}, {0x162, "mp4"}, {0x163, "mp5"}, {0x164, "skc"}, {0x165, "skf"}, {0x166, "sku"}, {0x167, "skl"},
        {0x168, "skz"}
    };

    auto iter = typeCodes.find(code);
    if (iter != typeCodes.end())
    {
        for (size_t i = 0; i < 3; ++i)
        {
            strcode[i] = iter->second[i];
        }
        strcode[3] = 0x20;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::ItemHelpers::getItemCodev115(const std::vector<std::uint8_t>& data, size_t startOffset, std::array<std::uint8_t, 4>& strcode)
{
    size_t offset = startOffset;
    for (size_t i = 0; i < 4; ++i)
    {
        strcode[i] = getEncodedChar(data, offset);
    }

    return std::uint8_t(offset - startOffset);
}
//---------------------------------------------------------------------------
// Retrieves encoded ItemCode (return number of bits set)
void d2ce::ItemHelpers::encodeItemCodev115(const std::array<std::uint8_t, 4>& strcode, std::uint64_t& encodedVal, std::uint8_t& numBitsSet)
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
//---------------------------------------------------------------------------
std::uint8_t d2ce::ItemHelpers::HuffmanDecodeBitString(const std::string& bitstr)
{
    auto iter = huffmanDecodeMap.find(bitstr);
    if (iter != huffmanDecodeMap.end())
    {
        return iter->second;
    }

    // something went wrong
    return UINT8_MAX;
}
//---------------------------------------------------------------------------
const d2ce::ItemType& d2ce::ItemHelpers::getItemTypeHelper(const std::array<std::uint8_t, 4>& strcode)
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
//---------------------------------------------------------------------------
std::uint8_t d2ce::ItemHelpers::getItemBase(const std::array<std::uint8_t, 4>& strcode)
{
    const auto& item = getItemTypeHelper(strcode);
    return item.getBaseType();
}
//---------------------------------------------------------------------------
d2ce::EnumItemType d2ce::ItemHelpers::getEnumItemTypeFromCode(std::array<std::uint8_t, 4>& strcode)
{
    const auto& item = getItemTypeHelper(strcode);
    return item.getEnumItemType();
}
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getRunewordNameFromId(std::uint16_t id)
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
//---------------------------------------------------------------------------
std::uint16_t d2ce::ItemHelpers::getSetIDv100(std::uint16_t id, const std::array<std::uint8_t, 4>& strcode)
{
    auto typeCode = getTypeCodeV100(strcode);
    if (typeCode >= UINT16_MAX)
    {
        return UINT16_MAX;
    }

    switch (id)
    {
    case 0: // Civerb's Vestments
        switch (typeCode)
        {
        case 0x0C7: // Civerb's Ward
            return 0;

        case 0x117: // Civerb's Icon
            return 1;

        case 0x010: // Civerb's Cudgel
            return 2;
        }
        break;

    case 1: // Hsarus' Trim
        switch (typeCode)
        {
        case 0x0D2: // Hsarus' Iron Heel
            return 3;

        case 0x0C5: // Hsarus' Iron Fist
            return 4;

        case 0x0D7: // Hsarus' Iron Stay
            return 5;
        }
        break;

    case 2: // Cleglaw's Brace
        switch (typeCode)
        {
        case 0x01F: // Cleglaw's Tooth
            return 6;

        case 0x0C6: // Cleglaw's Claw
            return 7;

        case 0x0CD: // Cleglaw's Pincers
            return 8;
        }
        break;

    case 3: // Iratha's Finery
        switch (typeCode)
        {
        case 0x117: // Iratha's Collar
            return 9;

        case 0x0CE: // Iratha's Cuff
            return 10;

        case 0x0B4: // Iratha's Coil
            return 11;

        case 0x0D8: // Iratha's Cord
            return 12;
        }
        break;

    case 4: // Isenhart's Armory
        switch (typeCode)
        {
        case 0x01E: // Isenhart's Lightbrand
            return 13;

        case 0x0CA: // Isenhart's Parry
            return 14;

        case 0x0BD: // Isenhart's Case
            return 15;

        case 0x0B2: // Isenhart's Horns
            return 16;
        }
        break;

    case 5: // Vidala's Rig
        switch (typeCode)
        {
        case 0x049: // Vidala's Barb
            return 17;

        case 0x0D3: // Vidala's Fetlock
            return 18;

        case 0x0B7: // Vidala's Ambush
            return 19;

        case 0x117: // Vidala's Snare
            return 20;
        }
        break;

    case 6: // Milabrega's Regalia
        switch (typeCode)
        {
        case 0x0C8: // Milabrega's Orb
            return 21;

        case 0x011: // Milabrega's Rod
            return 22;

        case 0x0B4: // Milabrega's Diadem
            return 23;

        case 0x0C3: // Mialbrega's Robe
            return 24;
        }
        break;

    case 7: // Cathan's Traps
        switch (typeCode)
        {
        case 0x042: // Cathan's Rule
            return 25;

        case 0x0BC: // Cathan's Mesh
            return 26;

        case 0x0B5: // Cathan's Visage
            return 27;

        case 0x117: // Cathan's Sigil
            return 28;

        case 0x119: // Cathan's Seal
            return 29;
        }
        break;

    case 8: // Tancred's Battlegear
        switch (typeCode)
        {
        case 0x003: // Tancred's Crowbill
            return 30;

        case 0x0C2: // Tancred's Spine
            return 31;

        case 0x0D0: // Tancred's Hobnails
            return 32;

        case 0x117: // Tancred's Weird
            return 33;

        case 0x0DA: // Tancred's Skull
            return 34;
        }
        break;

    case 9: // Sigon's Complete Steel
        switch (typeCode)
        {
        case 0x0B3: // Sigon's Gage
            return 35;

        case 0x0C1: // Sigon's Visor
            return 36;

        case 0x0CF: // Sigon's Shelter
            return 37;

        case 0x0D4: // Sigon's Sabot
            return 38;

        case 0x0D9: // Sigon's Wrap
            return 39;

        case 0x0C9: // Sigon's Guard
            return 40;
        }
        break;

    case 10: // Infernal Tools
        switch (typeCode)
        {
        case 0x0AF: // Infernal Cranium
            return 41;

        case 0x00D: // Infernal Torch
            return 42;

        case 0x0D8: // Infernal Sign
            return 43;
        }
        break;

    case 11: // Berserker's Garb
        switch (typeCode)
        {
        case 0x0B1: // Berserker's Headgear
            return 44;

        case 0x0BE: // Berserker's Hauberk
            return 45;

        case 0x002: // Berserker's Hatchet
            return 46;
        }
        break;

    case 12: // Death's Disguise
        switch (typeCode)
        {
        case 0x0CB: // Death's Hand
            return 47;

        case 0x0D5: // Death's Guard
            return 48;

        case 0x020: // Death's Touch
            return 49;
        }
        break;

    case 13: // Angelical Raiment
        switch (typeCode)
        {
        case 0x01B: // Angelic Sickle
            return 50;

        case 0x0BA: // Angelic Mantle
            return 51;

        case 0x119: // Angelic Halo
            return 52;

        case 0x117: // Angelic Wings
            return 53;
        }
        break;

    case 14: // Arctic Gear
        switch (typeCode)
        {
        case 0x04A: // Arctic Horn
            return 54;

        case 0x0B6: // Arctic Furs
            return 55;

        case 0x0D6: // Arctic Binding
            return 56;

        case 0x0CE: // Arctic Mitts
            return 57;
        }
        break;

    case 15: // Arcanna's Tricks
        switch (typeCode)
        {
        case 0x117: // Arcanna's Sign
            return 58;

        case 0x043: // Arcanna's Deathwand
            return 59;

        case 0x0B0: // Arcanna's Head
            return 60;

        case 0x0C4: // Arcanna's Flesh
            return 61;
        }
        break;
    }

    return UINT16_MAX;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ItemHelpers::getSetCodev100(std::uint16_t id)
{
    if (id >= setItemV100Info.size())
    {
        return UINT16_MAX;
    }

    return setItemV100Info[id].id;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::ItemHelpers::generateSetDWBCodev100(std::uint16_t id, const std::array<std::uint8_t, 4>& strcode, std::uint32_t dwb, std::uint16_t magicLevel)
{
    auto itemDwbCode = getSetDWBCodev100(id, strcode);
    if (itemDwbCode == 0)
    {
        return dwb;
    }

    for (int z = 0; z < 20000; z++)
    {
        ItemRandStruct rnd = { dwb, 666 };
        initalizeItemRandomizationv100(dwb, magicLevel, EnumItemQuality::SET, rnd);

        std::uint32_t offset = generateRandomv100(rnd) % 0x10;
        if ((itemDwbCode & (1 << offset)) != 0)
        {
            break;
        }

        dwb = ItemHelpers::generarateRandomDW();
    }

    return dwb;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::ItemHelpers::generateInferiorQualityIdv100(std::uint32_t dwb, std::uint16_t magicLevel)
{
    ItemRandStruct rnd = { dwb, 666 };
    initalizeItemRandomizationv100(dwb, magicLevel, EnumItemQuality::INFERIOR, rnd);
    return std::uint8_t(generateRandomv100(rnd) % 4);
}
//---------------------------------------------------------------------------
bool d2ce::ItemHelpers::generateMagicalAffixesv100(std::uint16_t typeCode, std::uint32_t dwb, std::uint16_t magicLevel, MagicalCachev100& cache)
{
    struct ModStruct
    {
        int code = 0;
        int min = 0;
        int max = 0;
        int secondary = 0;
    };

    struct PreSuffixStruct
    {
        int code = 0;
        int level = 0;
        int MagicMask = 0;
        std::vector<ModStruct> mod;
    };

    int magicMask = 0;
    const auto& itemInfo = getItemV100Info(typeCode);
    for (const auto& info : itemInfo)
    {
        switch (info.ItemSearch)
        {
        case 'U':
        case 'S':
            continue;
        }

        magicMask = info.MagicMask;
        break;
    }

    static const std::vector<PreSuffixStruct> prefixInfo = {
            {  2,  4,  1797, {{ 16,  20,  30}}},
            {  3,  9,  1797, {{ 16,  31,  40}}},
            {  4, 19,  1797, {{ 16,  41,  50}}},
            {  5, 25,  1797, {{ 16,  51,  65}}},
            {  6, 31,  1797, {{ 16,  66,  80}}},
            {  7, 36,  1797, {{ 16,  81, 100}}},
            {  8,  7,     4, {{ 35,   1,   1}}},
            {  9, 14,     4, {{ 35,   2,   2}}},
            { 13,  1,   242, {{ 17,  10,  20}}},
            { 14,  5,  8434, {{ 17,  21,  30}}},
            { 15,  8,   242, {{ 17,  31,  40}}},
            { 16, 14,   242, {{ 17,  41,  50}}},
            { 17, 20,   242, {{ 17,  51,  65}}},
            { 18, 26,   146, {{ 17,  66,  80}}},
            { 19, 32,   146, {{ 17,  81, 100}}},
            { 20,  9,  4100, {{114,  10,  10}}},
            { 25, 14,   256, {{ 28,  50,  50}}},
            { 26, 14, 16128, {{ 11,   5,  10}}},
            { 27,  1,  6898, {{ 19,  10,  20}}},
            { 28,  4,  2290, {{ 19,  21,  40}}},
            { 29,  8,  2290, {{ 19,  41,  60}}},
            { 30, 12,  2290, {{ 19,  61,  80}}},
            { 32, 17,   242, {{ 19,  81, 100}}},
            { 33, 22,  2290, {{ 19, 101, 120}}},
            { 34, 27,  2194, {{ 19, 121, 150}}},
            { 35,  5,   242, {{ 19,  10,  20}, {17, 10,  20}}},
            { 36,  9,   242, {{ 19,  21,  40}, {17, 21,  30}}},
            { 37, 15,   242, {{ 19,  41,  60}, {17, 31,  40}}},
            { 38, 21,   242, {{ 19,  61,  80}, {17, 41,  50}}},
            { 39, 25,   242, {{ 19,  81, 100}, {17, 51,  65}}},
            { 40, 30,   146, {{ 19, 101, 120}, {17, 66,  80}}},
            { 41, 35,   146, {{ 19, 121, 150}, {17, 81, 100}}},
            { 42, 16,   114, {{112,   1,   1}}},
            { 43,  5, 14336, {{ 79,  10,  15}}},
            { 49,  1, 32613, {{ 89,   1,   1}}},
            { 50,  6, 32613, {{ 89,   2,   2}}},
            { 53,  3, 31861, {{  9,   1,   5}}},
            { 55,  6, 23668, {{  9,   5,  10}}},
            { 56, 14, 23668, {{  9,  11,  20}}},
            { 57, 37,   771, {{  9,  11,  20}}},
            { 58, 20, 23664, {{  9,  21,  30}}},
            { 59, 24, 22640, {{  9,  31,  40}}},
            { 60, 52,   769, {{  9,  31,  40}}},
            { 61, 30, 22640, {{  9,  41,  60}}},
            { 64, 27,  4096, {{ 39,  15,  25}, {41, 15,  25}, {43, 15, 25}, {45, 15, 25}}},
            { 65, 62,  2048, {{ 39,  15,  15}, {41, 15,  15}, {43, 15, 15}, {45, 15, 15}}},
            { 66,  5, 32757, {{ 43,   5,  10}}},
            { 67, 12, 24437, {{ 43,  11,  20}}},
            { 68, 35,   642, {{ 43,  11,  20}}},
            { 69, 18, 23925, {{ 43,  21,  30}}},
            { 70, 55,   642, {{ 43,  21,  30}}},
            { 72, 25, 31200, {{ 43,  31,  50}}},
            { 75,  5, 24565, {{ 39,   5,  10}}},
            { 76, 12, 24437, {{ 39,  11,  20}}},
            { 77, 35,   130, {{ 39,  11,  20}}},
            { 78, 18, 23925, {{ 39,  21,  30}}},
            { 79, 55,   642, {{ 39,  21,  30}}},
            { 81, 25, 22880, {{ 39,  31,  50}}},
            { 84,  5, 24565, {{ 41,   5,  10}}},
            { 85, 12, 24437, {{ 41,  11,  20}}},
            { 86, 35,   130, {{ 41,  11,  20}}},
            { 87, 18, 23925, {{ 41,  21,  30}}},
            { 88, 55,   642, {{ 41,  21,  30}}},
            { 90, 25, 23905, {{ 41,  31,  50}}},
            { 93,  5,  8149, {{ 45,   5,  10}}},
            { 94, 12,  8021, {{ 45,  11,  20}}},
            { 95, 35,  2210, {{ 45,  11,  20}}},
            { 96, 18,  6229, {{ 45,  21,  30}}},
            { 97, 55,  1954, {{ 45,  21,  30}}},
            { 99, 25,  6160, {{ 45,  31,  50}}},
            {101, 30,  4224, {{ 83,   1,   1, 0}}},
            {102, 40,   128, {{ 83,   2,   2, 0}}},
            {103, 90,  4096, {{ 83,   2,   2, 0}}},
            {104, 30,  4112, {{ 83,   1,   1, 3}}},
            {105, 40,    16, {{ 83,   2,   2, 3}}},
            {106, 90,  4096, {{ 83,   2,   2, 3}}},
            {107, 30,  4128, {{ 83,   1,   1, 2}}},
            {108, 40,    32, {{ 83,   2,   2, 2}}},
            {109, 90,  4096, {{ 83,   2,   2, 2}}},
            {110, 30, 20544, {{ 83,   1,   1, 1}}},
            {111, 40,    64, {{ 83,   2,   2, 1}}},
            {112, 90, 20480, {{ 83,   2,   2, 1}}},
            {113, 30,  4098, {{ 83,   1,   1, 4}}},
            {114, 40,     2, {{ 83,   2,   2, 4}}},
            {115, 90,  4096, {{ 83,   2,   2, 4}}},
            {118,  3,  2162, {{138,   1,   1}}}
    };


    static const std::vector<PreSuffixStruct> suffixInfo = {
        {  1,  7,  6149, {{ 34,   1,   1}}},
        {  2, 18,  6144, {{ 34,   2,   2}}},
        {  3, 26,  4096, {{ 34,   3,   3}}},
        {  4, 35, 12288, {{ 34,   4,   4}}},
        {  5, 35,     0, {{ 34,   4,   4}}},
        {  6,  7, 14341, {{ 35,   1,   1}}},
        {  7, 18,  6144, {{ 35,   2,   2}}},
        {  8, 26,  4096, {{ 35,   3,   3}}},
        {  9, 35,  4096, {{ 35,   4,   4}}},
        { 11, 25,   112, {{115,   1,   1}}},
        { 12, 16,    18, {{120, -25, -40}}},
        { 13,  6,    18, {{120, -10, -20}}},
        { 14, 14,  1029, {{ 78,   1,   3}}},
        { 15, 21,  1029, {{ 78,   2,   6}}},
        { 16,  1,   754, {{ 19,  10,  10}}},
        { 17,  8,   242, {{ 19,  20,  20}}},
        { 18, 19,    18, {{ 19,  30,  30}}},
        { 19, 25,    18, {{ 19,  40,  40}}},
        { 20,  1,     4, {{ 20,  10,  10}}},
        { 21, 11,     4, {{ 20,  20,  20}}},
        { 22,  5, 22640, {{105,  10,  10}}},
        { 23, 17, 16496, {{105,  20,  20}}},
        { 24,  4,   242, {{ 54,   1,   1}, {55,  1,  4}, {56, 125, 125}}},
        { 25, 18,   114, {{ 54,   4,   4}, {55,  4, 12}, {56, 150, 150}}},
        { 26, 55,  5120, {{ 54,   1,   1}, {55,  1,  4}, {56, 125, 125}}},
        { 27, 10, 23300, {{118,   1,   1}}},
        { 28,  4,   242, {{ 48,   1,   1}, {49,  2,  6}}},
        { 29, 15,   242, {{ 48,   2,   2}, {49,  6, 11}}},
        { 30, 25,   242, {{ 48,  10,  10}, {49, 10, 20}}},
        { 31, 40,  6656, {{ 48,   1,   1}, {49,  2,  6}}},
        { 32,  4,   242, {{ 50,   1,   1}, {51,  8,  8}}},
        { 33, 15,   242, {{ 50,   1,   1}, {51, 16, 16}}},
        { 34, 25,   242, {{ 50,   1,   1}, {51, 32, 32}}},
        { 35, 50,  6402, {{ 50,   1,   1}, {51,  8,  8}}},
        { 36,  1,  6386, {{ 22,   1,   2}}},
        { 37,  4,   242, {{ 22,   2,   2}}},
        { 38,  7,   242, {{ 22,   3,   4}}},
        { 39, 11,   242, {{ 22,   5,   7}}},
        { 40, 14,   242, {{ 22,   8,  10}}},
        { 41, 19,   146, {{ 22,  11,  14}}},
        { 42, 25,   146, {{ 22,  15,  20}}},
        { 43, 42,  6148, {{ 22,   3,   4}}},
        { 44,  2,  8322, {{ 21,   1,   1}}},
        { 45,  6,   242, {{ 21,   2,   2}}},
        { 46, 12,  6386, {{ 21,   3,   3}}},
        { 47, 18,   242, {{ 21,   4,   5}}},
        { 48, 37,  6145, {{ 21,   2,   2}}},
        { 49,  5,   226, {{ 57,   8,   8}, {58, 24, 24}, {59,  75,  75}}},
        { 50, 15,   226, {{ 57,  16,  16}, {58, 48, 48}, {59,  75,  75}}},
        { 51, 25,   226, {{ 57,  32,  32}, {58, 72, 72}, {59, 100, 100}}},
        { 52, 45,  6144, {{ 57,   8,   8}, {58, 24, 24}, {59,  75,  75}}},
        { 53,  4, 14336, {{  2,   1,   1}}},
        { 54,  5,  7056, {{  2,   1,   3}}},
        { 55, 10,  7056, {{  2,   4,   6}}},
        { 56, 45,  1029, {{  2,   4,   6}}},
        { 57, 18,  7056, {{  2,   7,  10}}},
        { 58, 22,  4240, {{  2,  11,  15}}},
        { 59, 60,  3845, {{  2,  11,  15}}},
        { 60, 30,  4736, {{  2,  16,  20}}},
        { 61,  5,  9477, {{ 99,  10,  10}}},
        { 62,  9,  1285, {{ 99,  20,  20}}},
        { 64, 10,  7184, {{ 74,   3,   3}}},
        { 65, 40,   516, {{ 74,   3,   3}}},
        { 66, 70,   483, {{ 74,   3,   3}}},
        { 67, 17,  4112, {{ 74,   5,   5}}},
        { 68, 55,  3072, {{ 74,   5,   5}}},
        { 69,  9,    98, {{117,   1,   1}}},
        { 71,  2,  6144, {{ 79,  40,  60}}},
        { 72, 17,  5888, {{ 79,  80, 120}}},
        { 73, 12, 30720, {{ 80,  10,  19}}},
        { 74, 16, 13056, {{ 80,  20,  35}}},
        { 75,  4, 30720, {{  9,   1,   1}}},
        { 76,  5, 22640, {{  9,   1,   5}}},
        { 77, 10, 22640, {{  9,   6,  10}}},
        { 78, 18, 22640, {{  9,  11,  15}}},
        { 79, 22, 20592, {{  9,  16,  20}}},
        { 80, 30, 22640, {{  9,  21,  30}}},
        { 81,  8,   114, {{ 81,   1,   1}}},
        { 82,  6, 32757, {{ 89,   1,   1}}},
        { 83, 15, 22768, {{ 89,   3,   3}}},
        { 84, 17, 31472, {{ 89,   5,   5}}},
        { 85, 23, 13317, {{  7,   5,   5}}},
        { 86,  3, 16245, {{  7,   1,   5}}},
        { 87,  6,  8053, {{  7,   5,  10}}},
        { 88, 14,  8053, {{  7,  11,  20}}},
        { 89, 45,   130, {{  7,  11,  20}}},
        { 90, 20,  7941, {{  7,  21,  30}}},
        { 91, 24,  6149, {{  7,  31,  40}}},
        { 92, 60,  1538, {{  7,  31,  40}}},
        { 93, 30, 13317, {{  7,  41,  60}}},
        { 94,  6,  6386, {{ 60,   4,   7}}},
        { 95, 20,   114, {{ 60,   8,  10}}},
        { 96,  6,  6386, {{ 62,   4,   8}}},
        { 97, 20,   114, {{ 62,   9,  12}}},
        { 98, 25,  4101, {{110,  75,  75}}},
        { 99, 18,  4101, {{110,  50,  50}}},
        {100,  7,  7941, {{110,  25,  25}}},
        {102, 25,   135, {{ 91, -40, -40}}},
        {103, 15,   135, {{ 91, -20, -20}}},
        {105,  5,  7168, {{  0,   1,   3}}},
        {106, 10, 14358, {{  0,   4,   6}}},
        {107, 18,  7186, {{  0,   7,  10}}},
        {108, 47,   772, {{  0,   7,  10}}},
        {109, 22,  5138, {{  0,  11,  15}}},
        {110, 62,  2820, {{  0,  11,  15}}},
        {111, 30,  5376, {{  0,  16,  20}}},
        {112,  2,   256, {{ 96,  10,  10}}},
        {113, 12,   256, {{ 96,  20,  20}}},
        {114, 22,   256, {{ 96,  30,  30}}}
    };

    std::vector<PreSuffixStruct> prefixes;

    ItemRandStruct rnd = { dwb, 666 };
    initalizeItemRandomizationv100(dwb, magicLevel, EnumItemQuality::MAGIC, rnd);

    for (auto& prefix : prefixInfo)
    {
        if (prefix.level - 2 > magicLevel)
        {
            continue;
        }

        if ((prefix.MagicMask & magicMask) == 0)
        {
            continue;
        }

        prefixes.push_back(prefix);
    }

    // Do we have a prefix?
    cache.Affixes.PrefixId = 0;
    if ((generateRandomv100(rnd) % 2) == 1)
    {
        if (prefixes.empty())
        {
            return false;
        }

        auto& prefix = prefixes[generateRandomv100(rnd) % prefixes.size()];
        cache.Affixes.PrefixId = std::uint16_t(prefix.code);
        cache.Affixes.PrefixName = ItemHelpers::getMagicalPrefixFromId(cache.Affixes.PrefixId);

        std::uint16_t nextInChain = 0;
        size_t numRndCalls = prefix.mod.size();
        for (auto& mod : prefix.mod)
        {
            MagicalAttribute attrib;
            attrib.Id = std::uint16_t(mod.code);
            if (mod.secondary != 0)
            {
                attrib.Values.push_back(mod.secondary);
            }
            
            if (mod.max > mod.min)
            {
                attrib.Values.push_back(std::uint16_t(generateRandomv100(rnd) % (mod.max - mod.min) + mod.min));
                --numRndCalls;
            }
            else
            {
                attrib.Values.push_back(std::uint16_t(mod.min));
            }

            if (nextInChain != 0 && attrib.Id == nextInChain)
            {
                cache.MagicalAttributes.back().Values.push_back(attrib.Values.front());
                nextInChain = 0;
                if (attrib.Id < itemStats.size())
                {
                    auto& stat = itemStats[attrib.Id];
                    nextInChain = stat.nextInChain;
                }
            }
            else
            {
                nextInChain = 0;
                if (attrib.Id < itemStats.size())
                {
                    auto& stat = itemStats[attrib.Id];
                    attrib.Name = stat.name;
                    attrib.Desc = stat.desc;
                    nextInChain = stat.nextInChain;
                }

                cache.MagicalAttributes.push_back(attrib);
            }
        }

        for (size_t i = 0; i < numRndCalls; ++i)
        {
            generateRandomv100(rnd);
        }
    }

    std::vector<PreSuffixStruct> suffixes;
    for (auto& suffix : suffixInfo)
    {
        if (suffix.level - 2 > magicLevel)
        {
            continue;
        }

        if ((suffix.MagicMask & magicMask) == 0)
        {
            continue;
        }

        suffixes.push_back(suffix);
    }

    // Do we have a suffix
    cache.Affixes.SuffixId = 0;
    if ((generateRandomv100(rnd) % 2) == 1 || (cache.Affixes.PrefixId == 0))
    {
        if (suffixes.empty())
        {
            return false;
        }

        auto& suffix = suffixes[generateRandomv100(rnd) % prefixes.size()];
        cache.Affixes.SuffixId = std::uint16_t(suffix.code);
        cache.Affixes.SuffixName = ItemHelpers::getMagicalSuffixFromId(cache.Affixes.SuffixId);

        std::uint16_t nextInChain = 0;
        size_t numRndCalls = suffix.mod.size();
        for (auto& mod : suffix.mod)
        {
            MagicalAttribute attrib;
            attrib.Id = std::uint16_t(mod.code);
            if (mod.secondary != 0)
            {
                attrib.Values.push_back(mod.secondary);
            }

            if (mod.max > mod.min)
            {
                attrib.Values.push_back(std::uint16_t(generateRandomv100(rnd) % (mod.max - mod.min) + mod.min));
                --numRndCalls;
            }
            else
            {
                attrib.Values.push_back(std::uint16_t(mod.min));
            }

            if (nextInChain != 0 && attrib.Id == nextInChain)
            {
                cache.MagicalAttributes.back().Values.push_back(attrib.Values.front());
                nextInChain = 0;
                if (attrib.Id < itemStats.size())
                {
                    auto& stat = itemStats[attrib.Id];
                    nextInChain = stat.nextInChain;
                }
            }
            else
            {
                nextInChain = 0;
                if (attrib.Id < itemStats.size())
                {
                    auto& stat = itemStats[attrib.Id];
                    attrib.Name = stat.name;
                    attrib.Desc = stat.desc;
                    nextInChain = stat.nextInChain;
                }

                cache.MagicalAttributes.push_back(attrib);
            }
        }

        for (size_t i = 0; i < numRndCalls; ++i)
        {
            generateRandomv100(rnd);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::ItemHelpers::generarateRandomDW()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<std::uint32_t> spread(0, MAXUINT32);
    return std::uint32_t(spread(gen) + (spread(gen) << 16));
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::ItemHelpers::generarateRandomMagicLevel()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<std::uint32_t> spread(0, MAXUINT32);
    return std::uint32_t((spread(gen) % 90) + 1);
}
//---------------------------------------------------------------------------
bool d2ce::ItemHelpers::getSetMagicAttribsV100(std::uint16_t id, std::vector<MagicalAttribute>& attribs)
{
    attribs.clear();
    if (id >= setItemV100Info.size())
    {
        return false;
    }

    const SetItemV100& setInfo = setItemV100Info[id];
    attribs = setInfo.attribs;
    for (auto& attrib : attribs)
    {
        if (attrib.Id >= itemStats.size())
        {
            return false;
        }

        auto& stat = itemStats[attrib.Id];
        attrib.Name = stat.name;
        attrib.Desc = stat.desc;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ItemHelpers::getUniqueMagicAttribsV100(std::uint16_t id, std::vector<MagicalAttribute>& attribs)
{
    attribs.clear();
    if (id > uniqueItemV100Info.size())
    {
        return false;
    }

    attribs = uniqueItemV100Info[id];
    for (auto& attrib : attribs)
    {
        if (attrib.Id >= itemStats.size())
        {
            return false;
        }

        auto& stat = itemStats[attrib.Id];
        attrib.Name = stat.name;
        attrib.Desc = stat.desc;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ItemHelpers::getUniqueQuestMagicAttribsV100(const std::array<std::uint8_t, 4>& strcode, std::vector<MagicalAttribute>& attribs)
{
    attribs.clear();
    std::uint16_t id = 0;
    switch (getTypeCodeV100(strcode))
    {
    case 0x118: // Viper Amulet
        id = 123;
        break;

    case 0x05C: // Staff of the Kings
        id = 124;
        break;

    case 0x05B: // Horadric Staff
        id = 125;
        break;

    case 0x05A: // Hellforge Hammer
        id = 126;
        break;

    case 0x0AD: // Hellforge Hammer
        id = 127;
        break;

    case 0x0AE: // Hellforge Hammer
        id = 128;
        break;

    default:
        return false;
    }

    return getUniqueMagicAttribsV100(id, attribs);
}
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getSetNameFromId(std::uint16_t id)
{
    switch (id)
    {
    case 0: return "Civerb's Ward";
    case 1: return "Civerb's Icon";
    case 2: return "Civerb's Cudgel";
    case 3: return "Hsarus' Iron Heel";
    case 4: return "Hsarus' Iron Fist";
    case 5: return "Hsarus' Iron Stay";
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
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getSetTCFromId(std::uint16_t id)
{
    switch (id)
    {
    case 0:
    case 1:
    case 2:
    case 41:
    case 42:
    case 43:
        return "lyel";

    case 3:
    case 4:
    case 5:
    case 44:
    case 45:
    case 46:
        return "dred";

    case 6:
    case 7:
    case 8:
    case 47:
    case 48:
    case 49:
        return "lred";

    case 9:
    case 10:
    case 11:
    case 12:
    case 50:
    case 51:
    case 52:
    case 53:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 104:
    case 105:
    case 106:
    case 107:
        return "lgry";

    case 13:
    case 14:
    case 15:
    case 16:
    case 54:
    case 55:
    case 56:
    case 57:
        return "lgld";

    case 17:
    case 18:
    case 19:
    case 20:
    case 58:
    case 59:
    case 60:
    case 61:
        return "blac";

    case 21:
    case 22:
    case 23:
    case 24:
    case 112:
    case 113:
    case 114:
        return "dblu";

    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 115:
    case 116:
        return "dgrn";

    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 81:
    case 82:
    case 83:
    case 84:
        return "dgld";

    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
        return "whit";

    case 62:
    case 63:
    case 64:
    case 65:
        return "dgry";

    case 66:
    case 67:
    case 68:
    case 69:
        return "oran";

    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
        return "dpur";

    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
        return "dyel";

    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
        return "lblu";

    case 123:
    case 124:
    case 125:
    case 126:
        return "lpur";
    }

    return "";
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ItemHelpers::getSetLevelReqFromId(std::uint16_t id)
{
    switch (id)
    {
    case 0:
    case 1:
    case 2:
        return 9;

    case 3:
    case 4:
    case 5:
        return 3;

    case 6:
    case 7:
    case 8:
        return 4;

    case 9:
    case 10:
    case 11:
    case 12:
        return 15;

    case 13:
    case 14:
    case 15:
    case 16:
        return 8;

    case 17:
    case 18:
    case 19:
    case 20:
        return 14;

    case 21:
    case 22:
    case 23:
    case 24:
        return 17;

    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
        return 11;

    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
        return 20;

    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
        return 6;

    case 41:
    case 42:
    case 43:
        return 5;

    case 44:
    case 45:
    case 46:
        return 3;

    case 47:
    case 48:
    case 49:
        return 6;

    case 50:
    case 51:
    case 52:
    case 53:
        return 12;

    case 54:
    case 55:
    case 56:
    case 57:
        return 2;

    case 58:
    case 59:
    case 60:
    case 61:
        return 15;

    case 62:
    case 63:
    case 64:
    case 65:
        return 59;

    case 66: return 36;
    case 67: return 76;
    case 68: return 42;
    case 69: return 45;

    case 70: return 47;
    case 71: return 76;
    case 72: return 29;
    case 73: return 30;
    case 74: return 31;
    case 75: return 76;

    case 76: return 53;
    case 77: return 67;
    case 78: return 65;
    case 79: return 71;
    case 80: return 66;

    case 81: return 69;
    case 82: return 45;
    case 83: return 66;
    case 84: return 68;

    case 85: return 65;
    case 86: return 49;
    case 87: return 54;
    case 88: return 45;
    case 89: return 62;

    case 90: return 64;
    case 91: return 70;
    case 92: return 32;
    case 93: return 45;
    case 94: return 70;

    case 95: return 30;
    case 96: return 63;
    case 97: return 29;
    case 98: return 49;
    case 99: return 65;

    case 100: return 68;
    case 101: return 81;
    case 102: return 44;
    case 103: return 69;

    case 104: return 34;
    case 105: return 42;
    case 106: return 37;
    case 107: return 29;

    case 108: return 45;
    case 109: return 30;
    case 110: return 35;
    case 111: return 28;

    case 112: return 73;
    case 113: return 67;
    case 114: return 43;

    case 115: return 63;
    case 116: return 66;

    case 117: return 25;
    case 118: return 18;
    case 119: return 13;

    case 120: return 78;
    case 121: return 71;
    case 122: return 28;

    case 123: return 25;
    case 124: return 20;
    case 125: return 28;
    case 126: return 25;
    }

    return 0;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ItemHelpers::getIdFromRareName(const std::string& rareName)
{
    static const std::map<std::string, int> rareNames = {
            {"Bite", 0x001}, {"Scratch", 0x002}, {"Scalpel", 0x003}, {"Fang", 0x004}, {"Gutter", 0x005}, {"Thirst", 0x006}, {"Razor", 0x007},
            {"Scythe", 0x008}, {"Edge", 0x009}, {"Saw", 0x00A}, {"Splitter", 0x00B}, {"Cleaver", 0x00C}, {"Sever", 0x00D}, {"Sunder", 0x00E}, {"Rend", 0x00F},
            {"Mangler", 0x010}, {"Slayer", 0x011}, {"Reaver", 0x012}, {"Spawn", 0x013}, {"Gnash", 0x014}, {"Star", 0x015}, {"Blow", 0x016}, {"Smasher", 0x017},
            {"Bane", 0x018}, {"Crusher", 0x019}, {"Breaker", 0x01A}, {"Grinder", 0x01B}, {"Crack", 0x01C}, {"Mallet", 0x01D}, {"Knell", 0x01E}, {"Lance", 0x01F},
            {"Spike", 0x020}, {"Impaler", 0x021}, {"Skewer", 0x022}, {"Prod", 0x023}, {"Scourge", 0x024}, {"Wand", 0x025}, {"Wrack", 0x026}, {"Barb", 0x027},
            {"Needle", 0x028}, {"Dart", 0x029}, {"Bolt", 0x02A}, {"Quarrel", 0x02B}, {"Fletch", 0x02C}, {"Flight", 0x02D}, {"Nock", 0x02E}, {"Horn", 0x02F},
            {"Stinger", 0x030}, {"Quill", 0x031}, {"Goad", 0x032}, {"Branch", 0x033}, {"Spire", 0x034}, {"Song", 0x035}, {"Call", 0x036}, {"Cry", 0x037},
            {"Spell", 0x038}, {"Chant", 0x039}, {"Weaver", 0x03A}, {"Gnarl", 0x03B}, {"Visage", 0x03C}, {"Crest", 0x03D}, {"Circlet", 0x03E}, {"Veil", 0x03F},
            {"Hood", 0x040}, {"Mask", 0x041}, {"Brow", 0x042}, {"Casque", 0x043}, {"Visor", 0x044}, {"Cowl", 0x045}, {"Hide", 0x046}, {"Pelt", 0x047},
            {"Carapace", 0x048}, {"Coat", 0x049}, {"Wrap", 0x04A}, {"Suit", 0x04B}, {"Cloak", 0x04C}, {"Shroud", 0x04D}, {"Jack", 0x04E}, {"Mantle", 0x04F},
            {"Guard", 0x050}, {"Badge", 0x051}, {"Rock", 0x052}, {"Aegis", 0x053}, {"Ward", 0x054}, {"Tower", 0x055}, {"Shield", 0x056}, {"Wing", 0x057},
            {"Mark", 0x058}, {"Emblem", 0x059}, {"Hand", 0x05A}, {"Fist", 0x05B}, {"Claw", 0x05C}, {"Clutches", 0x05D}, {"Grip", 0x05E}, {"Grasp", 0x05F},
            {"Hold", 0x060}, {"Torch", 0x061}, {"Finger", 0x062}, {"Knuckle", 0x063}, {"Shank", 0x064}, {"Spur", 0x065}, {"Tread", 0x066}, {"Stalker", 0x067},
            {"Greave", 0x068}, {"Blazer", 0x069}, {"Nails", 0x06A}, {"Trample", 0x06B}, {"Brogues", 0x06C}, {"Track", 0x06D}, {"Slippers", 0x06E}, {"Clasp", 0x06F},
            {"Buckle", 0x070}, {"Harness", 0x071}, {"Lock", 0x072}, {"Fringe", 0x073}, {"Winding", 0x074}, {"Chain", 0x075}, {"Strap", 0x076}, {"Lash", 0x077},
            {"Cord", 0x078}, {"Knot", 0x079}, {"Circle", 0x07A}, {"Loop", 0x07B}, {"Eye", 0x07C}, {"Turn", 0x07D}, {"Spiral", 0x07E}, {"Coil", 0x07F},
            {"Gyre", 0x080}, {"Band", 0x081}, {"Whorl", 0x082}, {"Talisman", 0x083}, {"Heart", 0x084}, {"Noose", 0x085}, {"Necklace", 0x086}, {"Collar", 0x087},
            {"Beads", 0x088}, {"Torc", 0x089}, {"Gorget", 0x08A}, {"Scarab", 0x08B}, {"Wood", 0x08C}, {"Brand", 0x08D}, {"Bludgeon", 0x08E}, {"Cudgel", 0x08F},
            {"Loom", 0x090}, {"Harp", 0x091}, {"Master", 0x092}, {"Barl", 0x093}, {"Hew", 0x094}, {"Crook", 0x095}, {"Mar", 0x096}, {"Shell", 0x097},
            {"Stake", 0x098}, {"Picket", 0x099}, {"Pale", 0x09A}, {"Flange", 0x09B}, {"Beast", 0x09C}, {"Eagle", 0x09D}, {"Raven", 0x09E}, {"Viper", 0x09F},
            {"Ghoul", 0x0A0}, {"Skull", 0x0A1}, {"Blood", 0x0A2}, {"Dread", 0x0A3}, {"Doom", 0x0A4}, {"Grim", 0x0A5}, {"Bone", 0x0A6}, {"Death", 0x0A7},
            {"Shadow", 0x0A8}, {"Storm", 0x0A9}, {"Rune", 0x0AA}, {"Plague", 0x0AB}, {"Stone", 0x0AC}, {"Wraith", 0x0AD}, {"Spirit", 0x0AE}, {"Storm", 0x0AF},
            {"Demon", 0x0B0}, {"Cruel", 0x0B1}, {"Empyrion", 0x0B2}, {"Bramble", 0x0B3}, {"Pain", 0x0B4}, {"Loath", 0x0B5}, {"Glyph", 0x0B6}, {"Imp", 0x0B7},
            {"Fiendra", 0x0B8}, {"Hailstone", 0x0B9}, {"Gale", 0x0BA}, {"Dire", 0x0BB}, {"Soul", 0x0BC}, {"Brimstone", 0x0BD}, {"Corpse", 0x0BE}, {"Carrion", 0x0BF},
            {"Armageddon", 0x0C0}, {"Havoc", 0x0C1}, {"Bitter", 0x0C2}, {"Entropy", 0x0C3}, {"Chaos", 0x0C4}, {"Order", 0x0C5}, {"Rule", 0x0C6}, {"Warp", 0x0C7},
            {"Rift", 0x0C8}, {"Corruption", 0x0C9}
    };

    auto iter = rareNames.find(rareName);
    if (iter != rareNames.end())
    {
        return std::uint16_t(iter->second);
    }

    return std::uint16_t(0);
}
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getRareNameFromId(std::uint16_t id)
{
    switch (id)
    {
    case   1: return "Bite";
    case   2: return "Scratch";
    case   3: return "Scalpel";
    case   4: return "Fang";
    case   5: return "Gutter";
    case   6: return "Thirst";
    case   7: return "Razor";
    case   8: return "Scythe";
    case   9: return "Edge";
    case  10: return "Saw";
    case  11: return "Splitter";
    case  12: return "Cleaver";
    case  13: return "Sever";
    case  14: return "Sunder";
    case  15: return "Rend";
    case  16: return "Mangler";
    case  17: return "Slayer";
    case  18: return "Reaver";
    case  19: return "Spawn";
    case  20: return "Gnash";
    case  21: return "Star";
    case  22: return "Blow";
    case  23: return "Smasher";
    case  24: return "Bane";
    case  25: return "Crusher";
    case  26: return "Breaker";
    case  27: return "Grinder";
    case  28: return "Crack";
    case  29: return "Mallet";
    case  30: return "Knell";
    case  31: return "Lance";
    case  32: return "Spike";
    case  33: return "Impaler";
    case  34: return "Skewer";
    case  35: return "Prod";
    case  36: return "Scourge";
    case  37: return "Wand";
    case  38: return "Wrack";
    case  39: return "Barb";
    case  40: return "Needle";
    case  41: return "Dart";
    case  42: return "Bolt";
    case  43: return "Quarrel";
    case  44: return "Fletch";
    case  45: return "Flight";
    case  46: return "Nock";
    case  47: return "Horn";
    case  48: return "Stinger";
    case  49: return "Quill";
    case  50: return "Goad";
    case  51: return "Branch";
    case  52: return "Spire";
    case  53: return "Song";
    case  54: return "Call";
    case  55: return "Cry";
    case  56: return "Spell";
    case  57: return "Chant";
    case  58: return "Weaver";
    case  59: return "Gnarl";
    case  60: return "Visage";
    case  61: return "Crest";
    case  62: return "Circlet";
    case  63: return "Veil";
    case  64: return "Hood";
    case  65: return "Mask";
    case  66: return "Brow";
    case  67: return "Casque";
    case  68: return "Visor";
    case  69: return "Cowl";
    case  70: return "Hide";
    case  71: return "Pelt";
    case  72: return "Carapace";
    case  73: return "Coat";
    case  74: return "Wrap";
    case  75: return "Suit";
    case  76: return "Cloak";
    case  77: return "Shroud";
    case  78: return "Jack";
    case  79: return "Mantle";
    case  80: return "Guard";
    case  81: return "Badge";
    case  82: return "Rock";
    case  83: return "Aegis";
    case  84: return "Ward";
    case  85: return "Tower";
    case  86: return "Shield";
    case  87: return "Wing";
    case  88: return "Mark";
    case  89: return "Emblem";
    case  90: return "Hand";
    case  91: return "Fist";
    case  92: return "Claw";
    case  93: return "Clutches";
    case  94: return "Grip";
    case  95: return "Grasp";
    case  96: return "Hold";
    case  97: return "Torch";
    case  98: return "Finger";
    case  99: return "Knuckle";
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
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getMagicalPrefixFromId(std::uint16_t id)
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
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getMagicalPrefixTCFromId(std::uint16_t id)
{
    switch (id)
    {
    case 6:
    case 7:
    case 17:
    case 18:
    case 19:
    case 39:
    case 40:
    case 41:
    case 148:
    case 149:
    case 150:
    case 190:
    case 191:
    case 192:
    case 261:
    case 262:
    case 263:
    case 264:
    case 265:
    case 427:
    case 428:
    case 483:
    case 668:
        return "dgld";

    case 8:
    case 9:
    case 151:
    case 153:
    case 154:
        return "dblu";

    case 13:
    case 168:
    case 169:
    case 170:
    case 171:
    case 172:
    case 173:
    case 174:
    case 175:
    case 176:
    case 177:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 186:
    case 193:
    case 194:
    case 608:
    case 609:
    case 610:
    case 611:
    case 612:
    case 613:
    case 669:
        return "blac";

    case 20:
    case 58:
    case 59:
    case 60:
    case 61:
    case 199:
    case 308:
    case 309:
    case 310:
    case 311:
    case 312:
    case 313:
    case 314:
    case 315:
    case 316:
    case 540:
        return "cblu";

    case 32:
    case 33:
    case 34:
    case 242:
    case 243:
    case 244:
    case 245:
    case 246:
    case 434:
    case 437:
    case 438:
    case 440:
    case 441:
    case 446:
    case 447:
    case 449:
    case 450:
    case 452:
    case 453:
    case 458:
    case 459:
    case 461:
    case 462:
    case 464:
    case 465:
    case 470:
    case 471:
    case 473:
    case 474:
    case 476:
    case 477:
    case 482:
    case 485:
    case 486:
    case 488:
    case 489:
    case 494:
    case 495:
    case 497:
    case 498:
    case 500:
    case 501:
    case 506:
    case 507:
    case 509:
    case 510:
    case 512:
    case 513:
    case 596:
    case 597:
    case 598:
    case 599:
        return "lgld";

    case 42:
    case 269:
    case 272:
    case 270:
    case 271:
    case 273:
    case 274:
    case 531:
        return "oran";

    case 50:
    case 87:
    case 88:
    case 90:
    case 267:
    case 379:
    case 380:
    case 383:
    case 384:
    case 387:
    case 388:
    case 392:
    case 393:
    case 394:
    case 395:
    case 396:
    case 550:
    case 551:
    case 552:
    case 553:
    case 554:
    case 638:
    case 639:
    case 640:
    case 641:
    case 642:
    case 643:
    case 644:
    case 645:
    case 646:
    case 647:
    case 648:
    case 649:
        return "lyel";

    case 64:
    case 65:
    case 317:
    case 318:
    case 319:
    case 320:
    case 321:
    case 322:
    case 323:
    case 324:
    case 325:
    case 326:
    case 327:
    case 328:
    case 329:
    case 330:
    case 331:
    case 332:
    case 333:
    case 334:
    case 335:
    case 336:
    case 337:
        return "lpur";

    case 69:
    case 70:
    case 72:
    case 340:
    case 341:
    case 344:
    case 345:
    case 348:
    case 349:
    case 353:
    case 354:
    case 355:
    case 356:
    case 357:
    case 541:
    case 542:
    case 543:
    case 544:
    case 614:
    case 615:
    case 616:
    case 617:
    case 618:
    case 619:
    case 620:
    case 621:
    case 622:
    case 623:
    case 624:
    case 625:
        return "lblu";

    case 75:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
    case 183:
    case 184:
    case 185:
    case 195:
    case 196:
    case 197:
    case 358:
    case 362:
    case 366:
    case 545:
    case 546:
    case 547:
    case 548:
    case 549:
    case 601:
    case 602:
    case 603:
    case 604:
    case 605:
    case 606:
    case 607:
    case 626:
    case 627:
    case 628:
    case 629:
    case 630:
    case 631:
    case 632:
    case 633:
    case 634:
    case 635:
    case 636:
    case 637:
    case 665:
        return "dred";

    case 78:
    case 79:
    case 81:
    case 198:
    case 359:
    case 360:
    case 361:
    case 363:
    case 364:
    case 365:
    case 367:
    case 368:
    case 369:
    case 370:
    case 371:
    case 372:
    case 373:
    case 374:
    case 375:
    case 376:
        return "lred";

    case 93:
    case 94:
    case 95:
    case 102:
    case 103:
    case 105:
    case 106:
    case 108:
    case 109:
    case 111:
    case 112:
    case 114:
    case 115:
    case 397:
    case 399:
    case 401:
    case 403:
    case 405:
    case 407:
    case 409:
    case 412:
    case 413:
    case 415:
    case 416:
    case 435:
    case 561:
    case 563:
    case 565:
    case 567:
    case 569:
    case 571:
    case 573:
    case 575:
    case 577:
    case 579:
    case 581:
    case 583:
    case 585:
    case 587:
    case 589:
    case 591:
    case 667:
        return "cgrn";

    case 99:
    case 283:
    case 400:
    case 404:
    case 408:
    case 414:
        return "lgrn";

    case 139:
    case 140:
    case 141:
    case 142:
    case 522:
    case 523:
    case 524:
    case 525:
    case 528:
    case 529:
    case 532:
    case 533:
    case 534:
    case 535:
    case 536:
    case 537:
    case 538:
    case 539:
        return "whit";

    case 555:
    case 556:
    case 557:
    case 558:
    case 559:
    case 650:
    case 651:
    case 652:
    case 653:
    case 654:
    case 655:
    case 656:
    case 657:
    case 658:
    case 659:
    case 660:
    case 661:
        return "dgrn";
    }

    return "";
}
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getMagicalSuffixFromId(std::uint16_t id)
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
    case 27: return "of Thawing";
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
    case 93: return "of the Colossus";
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
    case 123: return "of Amicae";
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
    case 182: return "of Thawing";
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
    case 318: return "of the Colossus";
    case 319: return "of the Squid";
    case 320: return "of the Whale";
    case 321: return "of the Jackal";
    case 322: return "of the Fox";
    case 323: return "of the Wolf";
    case 324: return "of the Tiger";
    case 325: return "of the Mammoth";
    case 326: return "of the Colossus";
    case 327: return "of the Jackal";
    case 328: return "of the Fox";
    case 329: return "of the Wolf";
    case 330: return "of the Tiger";
    case 331: return "of the Mammoth";
    case 332: return "of Life";
    case 333: return "of Life";
    case 334: return "of Life";
    case 335: return "of Sustenance";
    case 336: return "of Sustenance";
    case 337: return "of Sustenance";
    case 338: return "of Vita";
    case 339: return "of Vita";
    case 340: return "of Vita";
    case 341: return "of Life";
    case 342: return "of Life";
    case 343: return "of Sustenance";
    case 344: return "of Sustenance";
    case 345: return "of Vita";
    case 346: return "of Vita";
    case 347: return "of Life";
    case 348: return "of Sustenance";
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
    case 377: return "of Atlas";
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
    case 403: return "of Restoration";
    case 404: return "of Ages";
    case 405: return "of Replenishing";
    case 406: return "of Propogation";
    case 407: return "of the Centaur";
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
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getMagicalSuffixTCFromId(std::uint16_t id)
{
    switch (id)
    {
    case 4:
    case 85:
    case 118:
    case 122:
    case 179:
    case 181:
    case 332:
    case 333:
    case 334:
    case 341:
    case 342:
    case 347:
    case 697:
    case 701:
    case 705:
        return "dblu";

    case 15:
    case 162:
    case 163:
    case 164:
        return "oran";

    case 19:
    case 34:
    case 62:
    case 169:
    case 191:
    case 192:
    case 194:
    case 264:
    case 416:
    case 450:
    case 668:
    case 720:
    case 721:
    case 724:
    case 725:
    case 728:
    case 729:
        return "dyel";

    case 30:
    case 98:
    case 185:
    case 186:
    case 188:
    case 366:
    case 666:
    case 708:
    case 709:
    case 712:
    case 713:
    case 716:
    case 717:
        return "dred";

    case 41:
    case 42:
    case 47:
    case 200:
    case 201:
    case 202:
    case 203:
    case 221:
    case 222:
    case 226:
    case 227:
    case 231:
        return "blac";

    case 51:
    case 236:
    case 237:
    case 239:
    case 684:
    case 685:
    case 688:
    case 689:
    case 692:
    case 693:
        return "dgrn";

    case 60:
    case 79:
    case 80:
    case 109:
    case 110:
    case 111:
    case 244:
    case 245:
    case 250:
    case 296:
    case 297:
    case 298:
    case 302:
    case 303:
    case 307:
    case 375:
    case 376:
    case 377:
    case 381:
    case 382:
    case 386:
    case 410:
    case 411:
    case 412:
    case 417:
        return "dgld";

    case 67:
    case 68:
    case 91:
    case 92:
    case 93:
    case 95:
    case 272:
    case 273:
    case 274:
    case 317:
    case 318:
    case 319:
    case 320:
    case 325:
    case 326:
    case 331:
    case 353:
    case 354:
    case 356:
    case 357:
    case 360:
    case 363:
    case 407:
    case 408:
    case 409:
    case 414:
    case 454:
    case 743:
    case 744:
    case 746:
        return "cred";

    case 72:
    case 74:
    case 278:
    case 287:
    case 288:
    case 289:
    case 290:
        return "lgld";

    case 97:
    case 361:
    case 364:
    case 415:
    case 432:
    case 433:
    case 434:
    case 435:
    case 436:
    case 747:
        return "cblu";

    case 232:
    case 233:
    case 445:
    case 446:
    case 447:
        return "whit";

    case 413:
    case 424:
    case 428:
    case 504:
    case 505:
        return "lblu";
    }

    return "";
}
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getUniqueNameFromId(std::uint16_t id)
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
    case 7: return "The Chieftain";
    case 8: return "Brainhew";
    case 9: return "Humongous";
    case 10: return "Torch of Iros";
    case 11: return "Maelstorm";
    case 12: return "Gravenspine";
    case 13: return "Ume's Lament";
    case 14: return "Felloak";
    case 15: return "Knell Striker";
    case 16: return "Rusthandle";
    case 17: return "Stormeye";
    case 18: return "Stoutnail";
    case 19: return "Crushflange";
    case 20: return "Bloodrise";
    case 21: return "The General's Tan Do Li Ga";
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
    case 32: return "Culwen's Point";
    case 33: return "Shadowfang";
    case 34: return "Soulflay";
    case 35: return "Kinemil's Awl";
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
    case 48: return "Dimoak's Hew";
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
    case 92: return "Silks of the Victor";
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
    case 128: return "Khalim's Will";
    case 129: return "Coldkill";
    case 130: return "Butcher's Pupil";
    case 131: return "Islestrike";
    case 132: return "Pompeii's Wrath";
    case 133: return "Guardian Naga";
    case 134: return "Warlord's Trust";
    case 135: return "Spellsteel";
    case 136: return "Stormrider";
    case 137: return "Boneslayer Blade";
    case 138: return "The Minotaur";
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
    case 161: return "The Atlantean";
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
    case 187: return "Skull Collector";
    case 188: return "Skystrike";
    case 189: return "Riphook";
    case 190: return "Kuko Shakaku";
    case 191: return "Endlesshail";
    case 192: return "Witchwild String";
    case 193: return "Cliffkiller";
    case 194: return "Magewrath";
    case 195: return "Goldstrike Arch";
    case 196: return "Langer Briser";
    case 197: return "Pus Spitter";
    case 198: return "Buriza-Do Kyanon";
    case 199: return "Demon Machine";
    case 200: return "Armor (Unknown)";
    case 201: return "Peasant Crownn";
    case 202: return "Rockstopper";
    case 203: return "Stealskull";
    case 204: return "Darksight Helm";
    case 205: return "Valkyrie Wing";
    case 206: return "Crown of Thieves";
    case 207: return "Blackhorn's Face";
    case 208: return "Vampire Gaze";
    case 209: return "The Spirit Shroud";
    case 210: return "Skin of the Vipermagi";
    case 211: return "Skin of the Flayed One";
    case 212: return "Iron Pelt";
    case 213: return "Spirit Forge";
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
    case 225: return "Moser's Blessed Circle";
    case 226: return "Stormchaser";
    case 227: return "Tiamat's Rebuke";
    case 228: return "Gerke's Sanctuary";
    case 229: return "Radament's Sphere";
    case 230: return "Lidless Wall";
    case 231: return "Lance Guard";
    case 232: return "Venom Grip";
    case 233: return "Gravepalm";
    case 234: return "Ghoulhide";
    case 235: return "Lava Gout";
    case 236: return "Hellmouth";
    case 237: return "Infernostride";
    case 238: return "Waterwalk";
    case 239: return "Silkweave";
    case 240: return "War Traveler";
    case 241: return "Gore Rider";
    case 242: return "String of Ears";
    case 243: return "Razortail";
    case 244: return "Gloom's Trap";
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
    case 268: return "Bul-Kathos' Wedding Band";
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
    case 290: return "Djinn Slayer";
    case 291: return "Deathbit";
    case 292: return "Warshrike";
    case 293: return "Gut Siphon";
    case 294: return "Razor's Edge";
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
//---------------------------------------------------------------------------
std::uint16_t d2ce::ItemHelpers::getUniqueLevelReqFromId(std::uint16_t id)
{
    switch (id)
    {
    case   0: return  5;
    case   1: return  9;
    case   2: return 15;
    case   3: return 21;
    case   4: return 27;
    case   5: return  8;
    case   6: return 14;
    case   7: return 19;
    case   8: return 25;
    case   9: return 29;
    case  10: return  5;
    case  11: return 14;
    case  12: return 20;
    case  13: return 28;
    case  14: return  3;
    case  15: return  5;
    case  16: return 18;
    case  17: return 30;
    case  18: return  5;
    case  19: return  9;
    case  20: return 15;
    case  21: return 21;
    case  22: return 27;
    case  23: return 24;
    case  24: return 29;
    case  25: return  2;
    case  26: return  7;
    case  27: return 10;
    case  28: return 13;
    case  29: return 85;
    case  30: return 17;
    case  31: return 22;
    case  32: return 29;
    case  33: return 12;
    case  34: return 19;
    case  35: return 23;
    case  36: return 26;
    case  37: return 26;
    case  38: return 29;
    case  39: return  4;
    case  40: return 11;
    case  41: return 19;
    case  42: return 25;
    case  43: return  8;
    case  44: return 12;
    case  45: return 17;
    case  46: return 22;
    case  47: return 27;
    case  48: return  8;
    case  49: return 14;
    case  50: return 19;
    case  51: return 25;
    case  52: return 28;
    case  53: return 29;
    case  54: return  5;
    case  55: return  9;
    case  56: return 18;
    case  57: return 21;
    case  58: return 28;
    case  59: return  7;
    case  60: return 13;
    case  61: return 15;
    case  62: return 20;
    case  63: return 25;
    case  64: return 26;
    case  65: return 27;
    case  66: return 28;
    case  67: return  9;
    case  68: return 18;
    case  69: return 27;
    case  70: return 28;
    case  71: return  3;
    case  72: return 15;
    case  73: return 14;
    case  74: return 17;
    case  75: return 21;
    case  76: return 25;
    case  77: return 29;
    case  78: return 20;
    case  79: return  7;
    case  80: return 12;
    case  81: return 14;
    case  82: return 16;
    case  83: return 14;
    case  84: return 15;
    case  85: return 17;
    case  86: return 20;
    case  87: return 22;
    case  88: return 26;
    case  89: return 28;
    case  90: return 29;
    case  91: return 28;
    case  92: return 28;
    case  93: return 29;
    case  94: return  2;
    case  95: return  9;
    case  96: return 13;
    case  97: return 20;
    case  98: return 15;
    case  99: return 17;
    case 100: return 19;
    case 101: return 26;
    case 102: return  5;
    case 103: return  9;
    case 104: return 15;
    case 105: return 23;
    case 106: return 29;
    case 107: return  5;
    case 108: return  9;
    case 109: return 15;
    case 110: return 22;
    case 111: return 29;
    case 112: return  7;
    case 113: return 12;
    case 114: return 20;
    case 115: return 27;
    case 116: return 29;
    case 117: return 10;
    case 118: return 15;
    case 119: return 25;
    case 120: return  7;
    case 121: return 15;
    case 122: return 29;
    case 129: return 36;
    case 130: return 39;
    case 131: return 43;
    case 132: return 45;
    case 133: return 48;
    case 134: return 35;
    case 135: return 39;
    case 136: return 41;
    case 137: return 42;
    case 138: return 45;
    case 139: return 33;
    case 140: return 35;
    case 141: return 36;
    case 142: return 41;
    case 143: return 34;
    case 144: return 37;
    case 145: return 38;
    case 146: return 42;
    case 147: return 38;
    case 148: return 39;
    case 149: return 42;
    case 150: return 45;
    case 151: return 43;
    case 152: return 48;
    case 153: return 45;
    case 154: return 30;
    case 155: return 31;
    case 156: return 33;
    case 157: return 35;
    case 158: return 37;
    case 159: return 39;
    case 160: return 41;
    case 161: return 42;
    case 162: return 42;
    case 163: return 43;
    case 164: return 44;
    case 165: return 45;
    case 166: return 46;
    case 167: return 48;
    case 168: return 32;
    case 169: return 36;
    case 170: return 38;
    case 171: return 41;
    case 172: return 31;
    case 173: return 33;
    case 174: return 35;
    case 175: return 37;
    case 176: return 39;
    case 177: return 41;
    case 178: return 42;
    case 179: return 42;
    case 180: return 43;
    case 181: return 44;
    case 182: return 45;
    case 183: return 28;
    case 184: return 31;
    case 185: return 35;
    case 186: return 39;
    case 187: return 41;
    case 188: return 28;
    case 189: return 31;
    case 190: return 33;
    case 191: return 36;
    case 192: return 39;
    case 193: return 41;
    case 194: return 43;
    case 195: return 46;
    case 196: return 32;
    case 197: return 36;
    case 198: return 41;
    case 199: return 49;
    case 201: return 28;
    case 202: return 31;
    case 203: return 35;
    case 204: return 38;
    case 205: return 44;
    case 206: return 49;
    case 207: return 41;
    case 208: return 41;
    case 209: return 28;
    case 210: return 29;
    case 211: return 31;
    case 212: return 33;
    case 213: return 35;
    case 214: return 37;
    case 215: return 38;
    case 216: return 41;
    case 217: return 42;
    case 218: return 45;
    case 219: return 48;
    case 220: return 51;
    case 221: return 53;
    case 222: return 55;
    case 223: return 51;
    case 224: return 28;
    case 225: return 31;
    case 226: return 35;
    case 227: return 38;
    case 228: return 44;
    case 229: return 50;
    case 230: return 41;
    case 231: return 35;
    case 232: return 29;
    case 233: return 32;
    case 234: return 36;
    case 235: return 42;
    case 236: return 47;
    case 237: return 29;
    case 238: return 32;
    case 239: return 36;
    case 240: return 42;
    case 241: return 47;
    case 242: return 29;
    case 243: return 32;
    case 244: return 36;
    case 245: return 42;
    case 246: return 47;
    case 248: return 62;
    case 249: return 73;
    case 250: return 85;
    case 251: return 85;
    case 252: return 61;
    case 253: return 73;
    case 254: return 66;
    case 255: return 70;
    case 256: return 65;
    case 257: return 79;
    case 258: return 87;
    case 259: return 58;
    case 260: return 69;
    case 261: return 81;
    case 262: return 61;
    case 263: return 95;
    case 264: return 70;
    case 265: return 69;
    case 266: return 73;
    case 268: return 58;
    case 269: return 50;
    case 270: return 65;
    case 271: return 50;
    case 272: return 67;
    case 273: return 60;
    case 274: return 45;
    case 275: return 45;
    case 276: return 65;
    case 277: return 47;
    case 279: return 42;
    case 280: return 42;
    case 281: return 42;
    case 282: return 42;
    case 283: return 42;
    case 284: return 42;
    case 285: return 42;
    case 286: return 42;
    case 287: return 42;
    case 288: return 57;
    case 289: return 61;
    case 290: return 65;
    case 291: return 44;
    case 292: return 75;
    case 293: return 71;
    case 294: return 67;
    case 296: return 63;
    case 297: return 62;
    case 298: return 84;
    case 299: return 66;
    case 300: return 69;
    case 301: return 85;
    case 302: return 65;
    case 304: return 68;
    case 306: return 64;
    case 307: return 68;
    case 308: return 66;
    case 309: return 71;
    case 310: return 63;
    case 311: return 84;
    case 312: return 74;
    case 313: return 72;
    case 314: return 70;
    case 315: return 75;
    case 316: return 64;
    case 317: return 65;
    case 319: return 76;
    case 320: return 70;
    case 321: return 68;
    case 322: return 82;
    case 323: return 71;
    case 324: return 74;
    case 325: return 61;
    case 326: return 75;
    case 327: return 67;
    case 328: return 76;
    case 329: return 77;
    case 330: return 64;
    case 331: return 65;
    case 332: return 71;
    case 333: return 62;
    case 334: return 78;
    case 335: return 70;
    case 336: return 76;
    case 337: return 68;
    case 338: return 69;
    case 340: return 68;
    case 341: return 72;
    case 342: return 69;
    case 343: return 67;
    case 344: return 82;
    case 345: return 83;
    case 347: return 80;
    case 348: return 66;
    case 349: return 76;
    case 350: return 74;
    case 351: return 79;
    case 353: return 71;
    case 354: return 73;
    case 355: return 79;
    case 356: return 68;
    case 357: return 77;
    case 358: return 75;
    case 359: return 62;
    case 360: return 82;
    case 361: return 77;
    case 363: return 70;
    case 364: return 76;
    case 365: return 70;
    case 366: return 74;
    case 367: return 72;
    case 368: return 67;
    case 369: return 64;
    case 370: return 66;
    case 371: return 61;
    case 373: return 80;
    case 374: return 51;
    case 375: return 81;
    case 376: return 63;
    case 378: return 60;
    case 379: return 65;
    case 380: return 60;
    case 381: return 70;
    case 382: return 81;
    case 383: return 63;
    case 384: return 68;
    case 385: return 69;
    case 386: return 76;
    case 387: return 64;
    case 388: return 66;
    case 389: return 72;
    case 390: return 75;
    case 391: return 70;
    case 392: return 49;
    case 393: return 49;
    case 394: return 49;
    case 395: return 49;
    case 396: return 49;
    case 397: return 49;
    case 398: return 49;
    case 399: return 49;
    case 400: return 75;
    }

    return 0;
}
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::getUniqueTCFromId(std::uint16_t id)
{
    switch (id)
    {
    case 2:
    case 39:
    case 47:
    case 79:
    case 105:
    case 176:
    case 195:
    case 218:
    case 227:
    case 249:
    case 250:
    case 262:
    case 379:
        return "lgry";

    case 4:
    case 40:
    case 76:
    case 89:
    case 110:
    case 164:
    case 212:
    case 274:
    case 325:
    case 348:
    case 351:
    case 360:
        return "dgry";

    case 5:
    case 53:
    case 144:
    case 190:
        return "lpur";

    case 6:
    case 43:
    case 90:
    case 268:
    case 277:
    case 290:
    case 306:
        return "dpur";

    case 9:
    case 19:
    case 25:
    case 48:
    case 77:
    case 113:
    case 142:
    case 178:
    case 187:
    case 199:
    case 204:
    case 207:
    case 222:
    case 255:
    case 258:
    case 297:
    case 309:
    case 315:
    case 321:
    case 329:
    case 357:
    case 358:
    case 373:
    case 376:
    case 380:
        return "blac";

    case 11:
    case 21:
    case 42:
    case 52:
    case 108:
    case 150:
    case 210:
    case 264:
    case 311:
    case 338:
    case 356:
    case 386:
        return "dblu";

    case 13:
    case 51:
    case 78:
    case 115:
    case 161:
    case 184:
    case 192:
    case 257:
    case 271:
    case 313:
        return "lblu";

    case 15:
    case 57:
    case 61:
    case 80:
    case 106:
    case 177:
    case 254:
    case 260:
    case 295:
    case 304:
    case 312:
    case 331:
    case 341:
    case 345:
    case 364:
        return "dred";

    case 16:
    case 87:
    case 180:
    case 270:
        return "lgld";

    case 17:
    case 81:
    case 107:
    case 132:
    case 154:
    case 157:
    case 182:
    case 189:
    case 289:
    case 330:
    case 339:
    case 340:
    case 353:
    case 355:
        return "cred";

    case 22:
    case 37:
    case 59:
    case 93:
    case 102:
    case 129:
    case 130:
    case 140:
    case 171:
    case 186:
    case 226:
    case 263:
    case 275:
    case 333:
    case 337:
    case 343:
    case 352:
    case 365:
    case 371:
        return "cblu";

    case 24:
    case 49:
    case 55:
    case 84:
    case 112:
    case 197:
    case 208:
    case 248:
    case 273:
    case 308:
    case 317:
    case 320:
    case 366:
        return "cgrn";

    case 32:
    case 45:
    case 134:
    case 135:
    case 219:
    case 324:
        return "whit";

    case 34:
    case 64:
    case 83:
    case 111:
    case 296:
    case 300:
    case 323:
    case 347:
        return "dgrn";

    case 46:
    case 60:
    case 104:
    case 136:
    case 172:
    case 211:
    case 251:
    case 256:
    case 349:
        return "lred";

    case 54:
    case 82:
    case 109:
    case 228:
    case 293:
        return "lgrn";

    case 58:
    case 86:
    case 116:
    case 266:
    case 363:
        return "dyel";

    case 66:
    case 85:
    case 114:
    case 174:
    case 261:
        return "lyel";

    case 72:
    case 103:
    case 216:
    case 269:
    case 272:
        return "oran";

    case 88:
    case 143:
    case 206:
    case 230:
    case 265:
    case 322:
    case 332:
    case 344:
    case 350:
        return "dgld";

    case 159:
    case 167:
    case 276:
    case 292:
    case 299:
    case 302:
    case 310:
    case 319:
    case 387:
        return "bwht";
    }

    return "";
}
//---------------------------------------------------------------------------
bool d2ce::ItemHelpers::magicalAttributeSorter(const MagicalAttribute& left, const MagicalAttribute& right)
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
        if (left.Id == right.Id)
        {
            if (left.Values.empty())
            {
                return right.Values.empty();
            }

            if (right.Values.empty())
            {
                return true;
            }

            return left.Values[0] > right.Values[0];
        }

        return (left.Id < right.Id);
    }

    return (leftStat > rightStat);
}
//---------------------------------------------------------------------------
void d2ce::ItemHelpers::checkForRelatedMagicalAttributes(std::vector<MagicalAttribute>& attribs)
{
    // check for the "all" cases
    std::map<std::uint16_t, std::reference_wrapper<MagicalAttribute>> relatedPropIdMap;
    std::int64_t numRelatedProps[4] = { 0, 0, 0, 0 };
    std::int64_t relatedPropValues[4] = { 0, 0, 0, 9 };
    bool validProps[4] = { true, true, true, true };
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
                        numRelatedProps[1] = 0;
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

        case 21:  // mindamage
        case 23:  // secondary_mindamage
        case 159: // item_throw_mindamage
            if (validProps[2])
            {
                if (relatedPropIdMap.find(attrib.Id) == relatedPropIdMap.end())
                {
                    relatedPropIdMap.emplace(attrib.Id, std::ref(attrib));
                    if ((numRelatedProps[2] == 0) || (attrib.Values[0] == relatedPropValues[2]))
                    {
                        ++numRelatedProps[2];
                        relatedPropValues[2] = attrib.Values[0];
                    }
                    else
                    {
                        // non matching case
                        validProps[2] = false;
                        numRelatedProps[2] = 0;
                    }
                }
                else
                {
                    // more than one poperty of the same type
                    validProps[2] = false;
                    numRelatedProps[2] = 0;
                }
            }
            break;

        case 22:  // maxdamage
        case 24:  // secondary_maxdamage
        case 160: // item_throw_maxdamage
            if (validProps[3])
            {
                if (relatedPropIdMap.find(attrib.Id) == relatedPropIdMap.end())
                {
                    relatedPropIdMap.emplace(attrib.Id, std::ref(attrib));
                    if ((numRelatedProps[3] == 0) || (attrib.Values[0] == relatedPropValues[3]))
                    {
                        ++numRelatedProps[3];
                        relatedPropValues[3] = attrib.Values[0];
                    }
                    else
                    {
                        // non matching case
                        validProps[3] = false;
                        numRelatedProps[3] = 0;
                    }
                }
                else
                {
                    // more than one poperty of the same type
                    validProps[3] = false;
                    numRelatedProps[3] = 0;
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

    for (size_t i = 2; i < 4; ++i)
    {
        if (validProps[i] && (numRelatedProps[i] == 1))
        {
            validProps[i] = false;
        }
    }

    if (!validProps[0] && !validProps[1] && !validProps[2] && !validProps[3])
    {
        return;
    }

    bool minDamageVisible = true;
    bool maxDamageVisible = true;
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

        case 21:  // mindamage
        case 23:  // secondary_mindamage
        case 159: // item_throw_mindamage
            if (validProps[2])
            {
                if (!minDamageVisible)
                {
                    attrib.Visible = false;
                }
                minDamageVisible = false;
            }
            break;

        case 22:  // maxdamage
        case 24:  // secondary_maxdamage
        case 160: // item_throw_maxdamage
            if (validProps[2])
            {
                if (!maxDamageVisible)
                {
                    attrib.Visible = false;
                }
                maxDamageVisible = false;
            }
            break;
        }
    }
}
//---------------------------------------------------------------------------
std::int64_t d2ce::ItemHelpers::getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx, const ItemStat& stat)
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

        // value at index 0 with range 1 to 128 must be converted to a percentage
    case 112:
        switch (idx)
        {
        case 0:
            return  (value * 100 + 64) / 128;
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
//---------------------------------------------------------------------------
std::string d2ce::ItemHelpers::formatMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx, const ItemStat& stat)
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
    case 151:
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
                size_t classIdx = NUM_OF_CLASSES;
                size_t skillIdx = NUM_OF_SKILLS;
                if (value <= END_SKILL_ID)
                {
                    classIdx = (value - START_SKILL_ID) / NUM_OF_SKILLS;
                    skillIdx = (value - START_SKILL_ID) % NUM_OF_SKILLS;
                }
                else if (value >= EXPANSION_START_SKILL_ID && value <= EXPANSION_END_SKILL_ID)
                {
                    classIdx = 5 + (value - EXPANSION_START_SKILL_ID) / NUM_OF_SKILLS;
                    skillIdx = (value - EXPANSION_START_SKILL_ID) % NUM_OF_SKILLS;
                }

                if (classIdx < NUM_OF_CLASSES)
                {
                    ssValue << SkillsNames[std::uint8_t(classIdx)][skillIdx];
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
                size_t classIdx = NUM_OF_CLASSES;
                size_t skillIdx = NUM_OF_SKILLS;
                if (value <= END_SKILL_ID)
                {
                    classIdx = (value - START_SKILL_ID) / NUM_OF_SKILLS;
                    skillIdx = (value - START_SKILL_ID) % NUM_OF_SKILLS;
                }
                else if (value >= EXPANSION_START_SKILL_ID && value <= EXPANSION_END_SKILL_ID)
                {
                    classIdx = 5 + (value - EXPANSION_START_SKILL_ID) / NUM_OF_SKILLS;
                    skillIdx = (value - EXPANSION_START_SKILL_ID) % NUM_OF_SKILLS;
                }

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
                size_t classIdx = NUM_OF_CLASSES;
                size_t skillIdx = NUM_OF_SKILLS;
                if (value <= END_SKILL_ID)
                {
                    classIdx = (value - START_SKILL_ID) / NUM_OF_SKILLS;
                    skillIdx = (value - START_SKILL_ID) % NUM_OF_SKILLS;
                }
                else if (value >= EXPANSION_START_SKILL_ID && value <= EXPANSION_END_SKILL_ID)
                {
                    classIdx = 5 + (value - EXPANSION_START_SKILL_ID) / NUM_OF_SKILLS;
                    skillIdx = (value - EXPANSION_START_SKILL_ID) % NUM_OF_SKILLS;
                }

                if (classIdx < NUM_OF_CLASSES)
                {
                    ssValue << SkillsNames[classIdx][skillIdx];
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
//---------------------------------------------------------------------------
bool d2ce::ItemHelpers::formatDisplayedMagicalAttribute(MagicalAttribute& attrib, std::uint32_t charLevel)
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
        if (attrib.Values.size() >= 2 && attrib.Values[0] == attrib.Values[1])
        {
            replaceStr = "Adds {0}-{1}";
            repIdx = attrib.Desc.find(replaceStr);
            if (repIdx != attrib.Desc.npos)
            {
                attrib.Desc.replace(repIdx, replaceStr.size(), "+{1}");
            }
        }
        break;

   default:
        if (attrib.Values.size() == 1 && attrib.Values[0] < 0)
        {
            replaceStr = "+{0}";
            repIdx = attrib.Desc.find(replaceStr);
            if (repIdx != attrib.Desc.npos)
            {
                attrib.Desc.replace(repIdx, replaceStr.size(), "{0}");
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
//---------------------------------------------------------------------------
void d2ce::ItemHelpers::combineMagicalAttribute(std::multimap<size_t, size_t>& itemIndexMap, const std::vector<MagicalAttribute>& newAttribs, std::vector<MagicalAttribute>& attribs)
{
    size_t numPoisonAttribs = 0;
    size_t numPoisonTimeSum = 0;
    size_t numColdAttribs = 0;
    size_t numColdTimeSum = 0;
    for (const auto& attrib : newAttribs)
    {
        auto iter = itemIndexMap.lower_bound(attrib.Id);
        auto iterEnd = itemIndexMap.upper_bound(attrib.Id);
        if (iter == iterEnd)
        {
            itemIndexMap.insert(std::make_pair(attrib.Id, attribs.size()));
            attribs.push_back(attrib);
            switch (attrib.Id)
            {
            case 52:
                numColdAttribs = 1;
                numColdTimeSum = attrib.Values[2];
                break;

            case 57:
                numPoisonAttribs = 1;
                numPoisonTimeSum = attrib.Values[2];
                break;
            }
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
                case 17:
                case 48:
                case 50:
                case 52:
                    existing.Values[0] += attrib.Values[0];
                    existing.Values[1] += attrib.Values[1];
                    notMatched = false;
                    break;

                case 54:
                    ++numColdAttribs;
                    numColdTimeSum += existing.Values[2];
                    existing.Values[0] += attrib.Values[0];
                    existing.Values[1] += attrib.Values[1];
                    existing.Values[2] = numColdTimeSum / numColdAttribs; // average
                    notMatched = false;
                    break;

                case 57:
                    ++numPoisonAttribs;
                    numPoisonTimeSum += existing.Values[2];
                    existing.Values[0] += attrib.Values[0];
                    existing.Values[1] += attrib.Values[1];
                    existing.Values[2] = numPoisonTimeSum / numPoisonAttribs; // average
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
            }

            if (notMatched)
            {
                itemIndexMap.insert(std::make_pair(attrib.Id, attribs.size()));
                attribs.push_back(attrib);
            }
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::ItemHelpers::ProcessNameNode(const Json::Value& node, std::array<char, NAME_LENGTH>& name)
{
    if (node.isNull())
    {
        return false;
    }

    {
        // Check Name
        // Remove any invalid characters from the number
        std::string curName(node.asString());
        std::string strNewText;
        for (size_t iPos = 0, numberOfUnderscores = 0, nLen = curName.size(); iPos < nLen; ++iPos)
        {
            char c = curName[iPos];
            if (std::isalpha(c))
            {
                strNewText += c;
            }
            else if ((c == '_' || c == '-') && strNewText.size() != 0 && numberOfUnderscores < 1)
            {
                strNewText += c;
                ++numberOfUnderscores;
            }
        }

        // trim bad characters
        if (strNewText.size() > 15)
        {
            strNewText.resize(15);
        }

        strNewText.erase(strNewText.find_last_not_of("_-") + 1);
        if (strNewText.size() < 2)
        {
            return false;
        }

        name.fill(0);
        strcpy_s(name.data(), strNewText.length() + 1, strNewText.c_str());
        name[15] = 0; // must be zero
        return true;
    }
}
//---------------------------------------------------------------------------