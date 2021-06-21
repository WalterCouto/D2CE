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

#pragma once

#include "CharacterConstants.h"
#include "CharacterStatsConstants.h"
#include "Constants.h"

namespace d2ce
{
    struct BasicStats
    {
        EnumCharVersion Version;                                            // Version for Character file
        char Name[NAME_LENGTH];                                             // pos 8 (pre-1.09, otherwise pos 20),
                                                                            // name includes terminating NULL
        bitmask::bitmask<EnumCharStatus> Status = EnumCharStatus::NoDeaths; // pos 24 (pos 36, 1.09+ only), determines character status
        EnumCharClass Class = EnumCharClass::Amazon;                        // pos 34 (pre-1.09, otherwise pos 40),
        bitmask::bitmask<EnumCharTitle> Title = EnumCharTitle::None;
        EnumDifficulty DifficultyLastPlayed = EnumDifficulty::Normal;
        EnumAct StartingAct = EnumAct::I;

        BasicStats()
        {
            std::memset(Name, 0, sizeof(Name));
        }

        bool isExpansionCharacter() const
        {
            return (Status & EnumCharStatus::Expansion) != 0 ? true : false;
        }

        void setIsExpansionCharacter(bool flag)
        {
            if (flag)
            {
                Status |= EnumCharStatus::Expansion;
            }
            else
            {
                Status &= EnumCharStatus::Expansion;
            }
        }

        bool isLadderCharacter() const
        {
            return (Status & EnumCharStatus::Ladder) != 0 ? true : false;
        }

        void setIsLadderCharacter(bool flag)
        {
            if (Version < EnumCharVersion::v110)
            {
                // ladder is for 1.10 or higher
                flag = false;
            }

            if (flag)
            {
                Status |= EnumCharStatus::Ladder;
            }
            else
            {
                Status &= EnumCharStatus::Ladder;
            }
        }

        bool isHardcoreCharacter() const
        {
            return (Status & EnumCharStatus::Hardcore) != 0 ? true : false;
        }

        void setIsHardcoreCharacter(bool flag)
        {
            if (flag)
            {
                Status |= EnumCharStatus::Hardcore;
                Status &= EnumCharStatus::Resurrected; // can't be resurrected
            }
            else
            {
                Status &= EnumCharStatus::Hardcore;
            }
        }

        bool isResurrectedCharacter() const
        {
            return (Status & EnumCharStatus::Resurrected) != 0 ? true : false;
        }

        void setIsResurrectedCharacter(bool flag)
        {
            if (isHardcoreCharacter())
            {
                flag = false; // Hardcore character can't be resurrected
            }

            if (flag)
            {
                Status |= EnumCharStatus::Resurrected;
            }
            else
            {
                Status &= ~EnumCharStatus::Resurrected;
            }
        }

        bool isDeadCharacter() const
        {
            return (Status & EnumCharStatus::Dead) != 0 ? true : false;
        }

        void setIsDeadCharacter(bool flag)
        {
            if (flag)
            {
                Status |= EnumCharStatus::Dead;
            }
            else
            {
                Status &= ~EnumCharStatus::Dead;
            }
        }
    };

    struct CharStats
    {
        std::uint32_t Strength = 0;          // pos 565 (pre-1.09)
        std::uint32_t Energy = 0;            // pos 569 (pre-1.09)
        std::uint32_t Dexterity = 0;         // pos 573 (pre-1.09)
        std::uint32_t Vitality = 0;          // pos 577 (pre-1.09)
        std::uint32_t StatsLeft = 0;         // value of stats left
        std::uint32_t SkillChoices = 0;      // value of skill choices remaining
        std::uint32_t CurLife = 0;           // shift right 8 bits for actual value
        std::uint32_t MaxLife = 0;           // shift right 8 bits for actual value
        std::uint32_t CurMana = 0;           // shift right 8 bits for actual value
        std::uint32_t MaxMana = 0;           // shift right 8 bits for actual value
        std::uint32_t CurStamina = 0;        // shift right 8 bits for actual value
        std::uint32_t MaxStamina = 0;        // shift right 8 bits for actual value
        std::uint32_t Level = 1;
        std::uint32_t Experience = 0;
        std::uint32_t GoldInBelt = 0;
        std::uint32_t GoldInStash = 0;

        std::uint32_t getMaxGoldInBelt() const
        {
            std::uint32_t curLevel = std::max(1u, std::min(Level, NUM_OF_LEVELS));
            return curLevel * 10000;
        }

        std::uint32_t getMaxGoldInStash(EnumCharVersion version = APP_CHAR_VERSION) const
        {
            if (version >= EnumCharVersion::v110) // 1.10+ character
            {
                return d2ce::GOLD_IN_STASH_LIMIT;
            }

            std::uint32_t curLevel = std::max(1u, std::min(Level, NUM_OF_LEVELS));
            if (curLevel < 31) // 1.00 - 1.09 character
            {
                return (curLevel / 10 + 1) * 50000;
            }

            if (version >= EnumCharVersion::v107) // 1.07 - 1.09 character
            {
                return (curLevel / 2 + 1) * 50000;
            }

            // pre 1.07 character
            if (curLevel < 90)
            {
                return (curLevel / 10 + 1) * 50000;
            }

            return 2000000;
        }

        std::uint32_t getMinExperienceRequired() const
        {
            std::uint32_t curLevel = std::max(0u, std::min(Level, NUM_OF_LEVELS));
            if (curLevel <= d2ce::NUM_OF_LEVELS)
            {
                return MinExpRequired[curLevel];
            }

            return std::uint32_t(-1);
        }

        std::uint32_t getNextExperienceLevel() const
        {
            std::uint32_t nextLevel = std::max(1u, std::min(Level + 1, NUM_OF_LEVELS));
            if (nextLevel + 1 <= d2ce::NUM_OF_LEVELS)
            {
                return MinExpRequired[nextLevel];
            }

            return std::uint32_t(-1);
        }
    };

    constexpr std::uint32_t NUM_OF_QUESTS = 6;
    constexpr std::uint32_t NUM_OF_QUESTS_ACT_IV = 3;
    constexpr std::uint32_t NUM_OF_ACTS = 5;
}
//---------------------------------------------------------------------------
