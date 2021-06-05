/*
    Diablo 2 Character Editor
    Copyright (C) 2000-2003  Burton Tsang

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef SkillConstantsH
#define SkillConstantsH

// skill names in original array position as found in character file
const AnsiString SkillsNames[NUM_OF_CLASSES][NUM_OF_SKILLS] =
         {{"Magic Arrow", "Fire Arrow", "Inner Sight", "Critical Strike", "Jab",
           "Cold Arrow", "Multiple Shot", "Dodge", "Power Strike",
           "Poison Javelin", "Exploding Arrow", "Slow Missiles", "Avoid",
           "Impale", "Lightning Bolt", "Ice Arrow", "Guided Arrow", "Penetrate",
           "Charged Strike", "Plague Javelin", "Strafe", "Immolation Arrow",
           "Decoy", "Evade", "Fend", "Freezing Arrow", "Valkyrie", "Pierce",
           "Lightning Strike", "Lightning Fury"},
          {"Firebolt", "Warmth", "Charged Bolt", "Ice Bolt", "Frozen Armor",
           "Inferno", "Static Field", "Telekinesis", "Frost Nova", "Ice Blast",
           "Blaze", "Fire Ball", "Nova", "Lightning", "Shiver Armor",
           "Fire Wall", "Enchant", "Chain Lightning", "Teleport",
           "Glacial Spike", "Meteor", "Thunder Storm", "Energy Shield",
           "Blizzard", "Chilling Armor", "Fire Mastery", "Hydra",
           "Lightning Mastery", "Frozen Orb", "Cold Mastery"},
          {"Amplify", "Teeth", "Bone Armor", "Skeleton Mastery",
           "Raise Skeleton", "Dim Vision", "Weaken", "Poison Dagger",
           "Corpse Explosion", "Clay Golem", "Iron Maiden", "Terror",
           "Bone Wall", "Golem Mastery", "Raise Skeletal Mage", "Confuse",
           "Life Tap", "Poison Explosion", "Bone Spear", "Blood Golem",
           "Attract", "Decrepify", "Bone Prison", "Summon Resist", "Iron Golem",
           "Lower Resist", "Poison Nova", "Bone Spirit", "Fire Golem", "Revive"},
          {"Sacrifice", "Smite", "Might", "Prayer", "Resist Fire", "Holy Bolt",
           "Holy Fire", "Thorns", "Defiance", "Resist Cold", "Zeal", "Charge",
           "Blessed Aim", "Cleansing", "Resist Lightning", "Vengeance",
           "Blessed Hammer", "Concentration", "Holy Freeze", "Vigor",
           "Conversion", "Holy Shield", "Holy Shock", "Sanctuary", "Meditation",
           "Fist Of The Heavens", "Fanaticism", "Conviction", "Redemption",
           "Salvation"},
          {"Bash", "Sword Mastery", "Axe Mastery", "Mace Mastery", "Howl",
           "Find Potion", "Leap", "Double Swing", "Pole Arm Mastery",
           "Throwing Mastery", "Spear Mastery", "Taunt", "Shout", "Stun",
           "Double Throw", "Increased Stamina", "Find Item", "Leap Attack",
           "Concentrate", "Iron Skin", "Battle Cry", "Frenzy", "Increased Speed",
           "Battle Orders", "Grim Ward", "Whirlwind", "Beserk",
           "Natural Resistance", "War Cry", "Battle Command"},
          {"Raven", "Poison Creeper", "Werewolf", "Lycanthropy", "Firestorm",
           "Oak Sage", "Summon Spirit Wolf", "Werebear", "Molten Boulder",
           "Arctic Blast", "Carrion Vine", "Feral Rage", "Maul", "Eruption",
           "Cyclone Armor", "Heart of Wolverine", "Summon Dire Wolf", "Rabies",
           "Fire Claws", "Twister", "Solar Creeper", "Hunger", "Shock Wave",
           "Volcano", "Tornado", "Spirit of Barbs", "Summon Grizzly", "Fury",
           "Armageddon", "Hurricane"},
          {"Fire Blast", "Claw Mastery", "Psychic Hammer", "Tiger Strike",
           "Dragon Talon", "Shock Web", "Blade Sentinel", "Burst of Speed",
           "Fists of Fire", "Dragon Claw", "Charged Bolt Sentry",
           "Wake of Fire", "Weapon Block", "Cloak of Shadows",
           "Cobra Strike", "Blade Fury", "Fade", "Shadow Warrior",
           "Claws of Thunder", "Dragon Tail", "Lightning Sentry",
           "Wake of Inferno", "Mind Blast", "Blades of Ice", "Dragon Flight",
           "Death Sentry", "Blade Shield", "Venom", "Shadow Master",
           "Phoenix Strike"}};
// skill tab names sorted by ascending tab number
const AnsiString TabNames[NUM_OF_CLASSES][3] =
         {{"Javelin and Spear Skills", "Passive and Magic Skills",
           "Bow and Crossbow Skills"},
          {"Cold Spells", "Lightning Spells", "Fire Spells"},
          {"Summon Spells", "Poison and Bone Spells", "Curses"},
          {"Defensive Auras", "Offensive Auras", "Combat Skills"},
          {"Warcries", "Combat Masteries", "Combat Skills"},
          {"Elemental", "Shape Shifting", "Summoning"},
          {"Martial Arts", "Shadow Disciplines", "Traps"}};
// skill names sorted by ascending tab number
const AnsiString TabSkills[NUM_OF_CLASSES][NUM_OF_SKILLS] =
         {{"Jab", "Power Strike", "Poison Javelin", "Impale", "Lightning Bolt",
           "Charged Strike", "Plague Javelin", "Fend", "Lightning Strike",
           "Lightning Fury", "Inner Sight", "Critical Strike", "Dodge",
           "Slow Missiles", "Avoid", "Penetrate", "Decoy", "Evade", "Valkyrie",
           "Pierce", "Magic Arrow", "Fire Arrow", "Cold Arrow", "Multiple Shot",
           "Exploding Arrow", "Ice Arrow", "Guided Arrow", "Strafe",
           "Immolation Arrow", "Freezing Arrow"},
          {"Ice Bolt", "Frozen Armor", "Frost Nova", "Ice Blast", "Shiver Armor",
           "Glacial Spike", "Blizzard", "Chilling Armor", "Frozen Orb",
           "Cold Mastery", "Charged Bolt", "Static Field", "Telekinesis", "Nova",
           "Lightning", "Chain Lightning", "Teleport", "Thunder Storm",
           "Energy Shield", "Lightning Mastery", "Firebolt", "Warmth", "Inferno",
           "Blaze", "Fire Ball", "Fire Wall", "Enchant", "Meteor",
           "Fire Mastery", "Hydra"},
          {"Skeleton Mastery", "Raise Skeleton", "Clay Golem", "Golem Mastery",
           "Raise Skeletal Mage", "Blood Golem", "Summon Resist", "Iron Golem",
           "Fire Golem", "Revive", "Teeth", "Bone Armor", "Poison Dagger",
           "Corpse Explosion", "Bone Wall", "Poison Explosion", "Bone Spear",
           "Bone Prison", "Poison Nova", "Bone Spirit", "Amplify Damage",
           "Dim Vision", "Weaken", "Iron Maiden", "Terror", "Confuse",
           "Life Tap", "Attract", "Decrepify", "Lower Resist"},
          {"Prayer", "Resist Fire", "Defiance", "Resist Cold", "Cleansing",
           "Resist Lightning", "Vigor", "Meditation", "Redemption", "Salvation",
           "Might", "Holy Fire", "Thorns", "Blessed Aim", "Concentration",
           "Holy Freeze", "Holy Shock", "Sanctuary", "Fanaticism", "Conviction",
           "Sacrifice", "Smite", "Holy Bolt", "Zeal", "Charge", "Vengeance",
           "Blessed Hammer", "Conversion", "Holy Shield", "Fist Of The Heavens"},
          {"Howl", "Find Potion", "Taunt", "Shout", "Find Item", "Battle Cry",
           "Battle Orders", "Grim Ward", "War Cry", "Battle Command",
           "Sword Mastery", "Axe Mastery", "Mace Mastery", "Pole Arm Mastery",
           "Throwing Mastery", "Spear Mastery", "Increased Stamina", "Iron Skin",
           "Increased Speed", "Natural Resistance", "Bash", "Leap",
           "Double Swing", "Stun", "Double Throw", "Leap Attack", "Concentrate",
           "Frenzy", "Whirlwind", "Beserk"},
          {"Firestorm", "Molten Boulder", "Arctic Blast", "Fissure",
           "Cyclone Armor", "Twister", "Volcano", "Tornado", "Armageddon",
           "Hurricane", "Werewolf", "Lycanthropy", "Werebear", "Feral Rage",
           "Maul", "Rabies", "Fire Claws", "Hunger", "Shock Wave", "Fury",
           "Raven", "Poison Creeper", "Oak Sage", "Summon Spirit Wolf",
           "Carrion Vine", "Heart of Wolverine", "Summon Dire Wolf",
           "Solar Creeper", "Spirit of Barbs", "Summon Grizzly"},
          {"Tiger Strike", "Dragon Talon", "Fists of Fire", "Dragon Claw",
           "Cobra Strike", "Claws of Thunder", "Dragon Tail", "Blades of Ice",
           "Dragon Flight", "Phoenix Strike", "Claw Mastery", "Psychic Hammer",
           "Burst of Speed", "Weapon Block", "Cloak of Shadow", "Fade",
           "Shadow Warrior", "Mind Blast", "Venom", "Shadow Master",
           "Fire Blast", "Shock Web", "Blade Sentinel", "Charged Bolt Sentry",
           "Wake of Fire", "Blade Fury", "Lightning Sentry", "Wake of Inferno",
           "Death Sentry", "Blade Shield"}};
// position skill is found in the Skill array found in the D2MainForm.h file
// sorted by ascending tab number as shown in the game
const unsigned char TabSkillPos[NUM_OF_CLASSES][NUM_OF_SKILLS] =
                                         {{4, 8, 9, 13, 14, 18, 19, 24, 28, 29,
                                           2, 3, 7, 11, 12, 17, 22, 23, 26, 27,
                                           0, 1, 5, 6, 10, 15, 16, 20, 21, 25},
                                          {3, 4, 8, 9, 14, 19, 23, 24, 28, 29,
                                           2, 6, 7, 12, 13, 17, 18, 21, 22, 27,
                                           0, 1, 5, 10, 11, 15, 16, 20, 25, 26},
                                          {3, 4, 9, 13, 14, 19, 23, 24, 28, 29,
                                           1, 2, 7, 8, 12, 17, 18, 22, 26, 27,
                                           0, 5, 6, 10, 11, 15, 16, 20, 21, 25},
                                          {3, 4, 8, 9, 13, 14, 19, 24, 28, 29,
                                           2, 6, 7, 12, 17, 18, 22, 23, 26, 27,
                                           0, 1, 5, 10, 11, 15, 16, 20, 21, 25},
                                          {4, 5, 11, 12, 16, 20, 23, 24, 28, 29,
                                           1, 2, 3, 8, 9, 10, 15, 19, 22, 27,
                                           0, 6, 7, 13, 14, 17, 18, 21, 25, 26},
                                          {4, 8, 9, 13, 14, 19, 23, 24, 28, 29,
                                           2, 3, 7, 11, 12, 17, 18, 21, 22, 27,
                                           0, 1, 5, 6, 10, 15, 16, 20, 25, 26},
                                          {3, 4, 8, 9, 14, 18, 19, 23, 24, 29,
                                           1, 2, 7, 12, 13, 16, 17, 22, 27, 28,
                                           0, 5, 6, 10, 11, 15, 20, 21, 25, 26}};

#endif

