/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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

#pragma once

#include "Constants.h"
#include <string>
#include <array>

namespace d2ce
{
    constexpr std::uint16_t START_SKILL_ID = 6;
    constexpr std::uint16_t END_SKILL_ID = 155;
    constexpr std::uint16_t EXPANSION_START_SKILL_ID = 221;
    constexpr std::uint16_t EXPANSION_END_SKILL_ID = 280;
    constexpr std::uint32_t NUM_OF_SKILLS = 30;
    constexpr std::uint8_t MAX_SKILL_VALUE = 20;

    const std::map<size_t, std::string> GenericSkillNames = {
        {0, "Attack"}, {1, "Kick"}, {2, "Throw"}, {3, "Unsummon"}, {4, "Left Hand Throw"}, {5, "Left Hand Swing"},
        {217, "Scroll of Identify"}, {218, "Tome of Identify"}, {219, "Scroll of Town Portal"}, {220, "Tome of Town Portal"}
    };

    // skill names in original array position as found in character file
    const std::array<std::array<std::string, NUM_OF_SKILLS>, NUM_OF_CLASSES> SkillsNames =
             {{{"Magic Arrow", "Fire Arrow", "Inner Sight", "Critical Strike", "Jab",
               "Cold Arrow", "Multiple Shot", "Dodge", "Power Strike",
               "Poison Javelin", "Exploding Arrow", "Slow Missiles", "Avoid",
               "Impale", "Lightning Bolt", "Ice Arrow", "Guided Arrow", "Penetrate",
               "Charged Strike", "Plague Javelin", "Strafe", "Immolation Arrow",
               "Decoy", "Evade", "Fend", "Freezing Arrow", "Valkyrie", "Pierce",
               "Lightning Strike", "Lightning Fury"},
              {"Fire Bolt", "Warmth", "Charged Bolt", "Ice Bolt", "Frozen Armor",
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
               "Phoenix Strike"}}};
    // skill tab names sorted by ascending tab number
    const std::array<std::array<std::string, 3>, NUM_OF_CLASSES> TabNames =
             {{{"Javelin and Spear Skills", "Passive and Magic Skills",
               "Bow and Crossbow Skills"},
              {"Cold Spells", "Lightning Spells", "Fire Spells"},
              {"Summon Spells", "Poison and Bone Spells", "Curses"},
              {"Defensive Auras", "Offensive Auras", "Combat Skills"},
              {"Warcries", "Combat Masteries", "Combat Skills"},
              {"Elemental", "Shape Shifting", "Summoning"},
              {"Martial Arts", "Shadow Disciplines", "Traps"}}};
    // position skill is found in the Skill array found in the D2MainForm.h file
    // sorted by ascending tab number as shown in the game
    constexpr std::array<std::array<std::uint8_t, NUM_OF_SKILLS>, NUM_OF_CLASSES> TabSkillPos =
                                             {{{4, 8, 9, 13, 14, 18, 19, 24, 28, 29,
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
                                               0, 5, 6, 10, 11, 15, 20, 21, 25, 26}}};
}
//---------------------------------------------------------------------------
