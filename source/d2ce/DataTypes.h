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
#include <sstream>

namespace d2ce
{
    struct BasicStats
    {
        EnumCharVersion Version;                                            // Version for Character file
        char Name[NAME_LENGTH];                                             // pos 20 (1.09+, otherwise pos 8),
                                                                            // name includes terminating NULL
        bitmask::bitmask<EnumCharStatus> Status = EnumCharStatus::NoDeaths; // pos 36 (1.09+, otherwise, pos 24), determines character status
        bitmask::bitmask<EnumCharTitle> Title = EnumCharTitle::None;        // pos 37 (1.09+, otherwise pos 25), character's title
        EnumCharClass Class = EnumCharClass::Amazon;                        // pos 40 (1.09+, otherwise pos 34),
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

    struct MercInfo // 1.09+ only
    {
        std::uint16_t Dead = 0;       // pos 177
        std::uint32_t Id = 0;         // pos 179
        std::uint16_t NameId = 0;     // pos 183
        std::uint16_t Type = 0;       // pos 185
        std::uint32_t Experience = 0; // pos 187
    };

    constexpr std::uint32_t NUM_OF_QUESTS = 6;
    constexpr std::uint32_t NUM_OF_QUESTS_ACT_IV = 3;
    constexpr std::uint32_t NUM_OF_ACTS = 5;

    struct EarAttributes
    {
        EnumCharClass Class = EnumCharClass::Amazon;
        std::uint32_t Level = 1;
        char Name[NAME_LENGTH];

        EarAttributes()
        {
            std::memset(Name, 0, sizeof(Name));
        }

        void clear()
        {
            Class = EnumCharClass::Amazon;
            Level = 1;
            std::memset(Name, 0, sizeof(Name));
        }

        d2ce::EnumCharClass getClass() const
        {
            return Class;
        }

        std::uint32_t getLevel() const
        {
            return Level;
        }

        std::string getClassName() const
        {
            auto idx = (std::uint8_t)getClass();
            if (idx < NUM_OF_CLASSES)
            {
                return ClassNames[idx];
            }

            return "";
        }

        const char(&getName() const)[NAME_LENGTH]
        {
            return Name;
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent << "\"ear_attributes\": {";
            ss << "\n" << parentIndent << jsonIndentStr << "\"class\": \"" << getClassName() << "\"";
            ss << ",\n" << parentIndent << jsonIndentStr << "\"level\": " << std::dec << getLevel();
            ss << ",\n" << parentIndent << jsonIndentStr << "\"name\": \"" << getName() << "\"";
            ss << "\n" << parentIndent << "}";
        }
    };
    
    struct MagicalAttribute
    {
        std::uint16_t Id = 0;
        std::string Name;
        std::string Desc;
        std::vector<std::int64_t> Values;
        std::int64_t OpValue = 0;
        std::vector<std::string> OpStats;
        bool Visible = true;

        void clear()
        {
            Id = 0;
            Name.clear();
            Desc.clear();
            Values.clear();
            OpValue = 0;
            OpStats.clear();
            Visible = true;
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            std::string attribParentIndent = parentIndent + jsonIndentStr + jsonIndentStr;
            ss << "\n" << parentIndent << jsonIndentStr << "{";
            ss << "\n" << attribParentIndent << "\"id\": " << std::dec << Id;
            ss << ",\n" << attribParentIndent << "\"values\": [";
            if (Values.empty())
            {
                ss << "]";
            }
            else
            {
                bool bFirstItem = true;
                for (const auto& val : Values)
                {
                    if (bFirstItem)
                    {
                        bFirstItem = false;
                    }
                    else
                    {
                        ss << ",";
                    }

                    if (val == 0)
                    {
                        ss << "\n" << attribParentIndent << jsonIndentStr << "null";
                    }
                    else
                    {
                        ss << "\n" << attribParentIndent << jsonIndentStr << std::dec << val;
                    }
                }
                ss << "\n" << attribParentIndent << "]";
                ss << ",\n" << attribParentIndent << "\"name\": \"" << Name << "\"";
                if (!OpStats.empty())
                {
                    ss << ",\n" << attribParentIndent << "\"op_stats\": [";
                    bFirstItem = true;
                    for (const auto& val : OpStats)
                    {
                        if (bFirstItem)
                        {
                            bFirstItem = false;
                        }
                        else
                        {
                            ss << ",";
                        }

                        ss << "\n" << attribParentIndent << jsonIndentStr << "\"" << val << "\"";
                    }
                    ss << "\n" << attribParentIndent << "]";
                    ss << ",\n" << attribParentIndent << "\"op_value\": " << std::dec << OpValue;
                }

                if (!Desc.empty())
                {
                    ss << ",\n" << attribParentIndent << "\"description\": \"" << Desc << "\"";
                }

                if (!Visible)
                {
                    ss << ",\n" << attribParentIndent << "\"visible\": false";
                }
            }

            ss << "\n" << parentIndent << jsonIndentStr << "}";
        }
    };

    struct RunewordAttributes
    {
        std::uint16_t Id = 0;
        std::string Name;
        std::vector<MagicalAttribute> MagicalAttributes;

        void clear()
        {
            Id = 0;
            Name.clear();
            MagicalAttributes.clear();
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent << "\"runeword_id\": " << std::dec << Id;
            ss << ",\n" << parentIndent << "\"runeword_name\": \"" << Name << "\"";
            ss << ",\n" << parentIndent << "\"runeword_attributes\": [";
            if (MagicalAttributes.empty())
            {
                ss << "]";
            }
            else
            {
                bool bFirstItem = true;
                for (auto& attrib : MagicalAttributes)
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
        }
    };

    struct SetAttributes
    {
        std::uint16_t Id = 0;
        std::string Name;
        std::vector<std::vector<MagicalAttribute>> SetAttributes;

        void clear()
        {
            Id = 0;
            Name.clear();
            SetAttributes.clear();
        }

        void setAttributesJsonArray(std::stringstream& ss, const std::string& parentIndent) const
        {
            if (SetAttributes.empty())
            {
                return;
            }

            bool bFirstItem = true;
            for (const auto& attribs : SetAttributes)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }

                ss << "\n" << parentIndent << "[";
                if (!attribs.empty())
                {
                    for (const auto& attrib : attribs)
                    {
                        attrib.asJson(ss, parentIndent);
                    }
                }
                ss << "\n" << parentIndent << "]";
            }
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent <<  "\"set_id\": " << std::dec << Id;
            ss << ",\n" << parentIndent << "\"set_name\": \"" << Name << "\"";
            ss << ",\n" << parentIndent << "\"set_list_count\": " << std::dec << SetAttributes.size();
            ss << ",\n" << parentIndent << "\"set_attributes\": [";
            std::string attribParentIndent = parentIndent + jsonIndentStr;
            setAttributesJsonArray(ss, attribParentIndent);
            ss << "\n" << parentIndent << "]";
        }
    };

    struct MagicalAffixes
    {
        std::uint16_t PrefixId = 0;
        std::string PrefixName;
        std::uint16_t SuffixId = 0;
        std::string SuffixName;

        void clear()
        {
            PrefixId = 0;
            PrefixName.clear();
            SuffixId = 0;
            SuffixName.clear();
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent << "\"magic_prefix\": " << std::dec << PrefixId;
            if (PrefixId != 0)
            {
                ss << ",\n" << parentIndent << "\"magic_prefix_name\": \"" << PrefixName << "\"";
            }

            ss << ",\n" << parentIndent << "\"magic_suffix\": " << std::dec << SuffixId;
            if (SuffixId != 0)
            {
                ss << ",\n" << parentIndent << "\"magic_suffix_name\": \"" << SuffixName << "\"";
            }
        }
    };

    struct RareAttributes
    {
        std::uint16_t Id = 0;
        std::string Name;
        std::uint16_t Id2 = 0;
        std::string Name2;
        std::vector<MagicalAffixes> Affixes;

        void clear()
        {
            Id = 0;
            Name.clear();
            Id2 = 0;
            Name2.clear();
            Affixes.clear();
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent << "\"rare_name_id\": " << std::dec << Id;
            ss << ",\n" << parentIndent << "\"rare_name\": \"" << Name << "\"";
            ss << ",\n" << parentIndent << "\"rare_name_id2\": " << std::dec << Id2;
            ss << ",\n" << parentIndent << "\"rare_name2\": \"" << Name2 << "\"";
            ss << ",\n" << parentIndent << "\"magical_name_ids\": [";
            if (Affixes.empty())
            {
                ss << "]";
            }
            else
            {
                bool bFirstItem = true;
                for (const auto& val : Affixes)
                {
                    if (bFirstItem)
                    {
                        bFirstItem = false;
                    }
                    else
                    {
                        ss << ",";
                    }
                    ss << "\n" << parentIndent << jsonIndentStr << std::dec << val.PrefixId;
                    ss << ",\n" << parentIndent << jsonIndentStr << std::dec << val.SuffixId;
                }
                ss << "\n" << parentIndent << "]";
            }
        }
    };

    struct UniqueAttributes
    {
        std::uint16_t Id = 0;
        std::string Name;

        void clear()
        {
            Id = 0;
            Name.clear();
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent << "\"unique_id\": " << std::dec << Id;
            ss << ",\n" << parentIndent << "\"unique_name\": \"" << Name << "\"";
        }
    }; 
    
    struct ItemRequirements
    {
        std::uint16_t Strength = 0;
        std::uint16_t Dexterity = 0;
        std::uint16_t Level = 0;

        void clear()
        {
            Strength = 0;
            Dexterity = 0;
            Level = 0;
        }
    };

    struct ItemDimensions
    {
        std::uint16_t Width = 0;
        std::uint16_t Height = 0;

        void clear()
        {
            Width = 0;
            Height = 0;
        }
    };

    struct ItemDurability
    {
        std::uint16_t Current = 0;
        std::uint16_t Max = 0;

        void clear()
        {
            Current = 0;
            Max = 0;
        }
    };

    struct ItemDamage
    {
        std::uint16_t Min = 0;
        std::uint16_t Max = 0;
        bool OneOrTwoHanded = false;
        bool TwoHanded = false;
        std::uint16_t Min2 = 0;
        std::uint16_t Max2 = 0;
        std::uint16_t MissleMin = 0;
        std::uint16_t MisselMax = 0;

        void clear()
        {
            Min = 0;
            Max = 0;
            OneOrTwoHanded = false;
            TwoHanded = false;
            Min2 = 0;
            Max2 = 0;
            MissleMin = 0;
            MisselMax = 0;
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent << "\"base_damage\": {";
            bool bFirstItem = true;
            if (Max != 0)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                ss << "\n" << parentIndent << jsonIndentStr << "\"mindam\": " << std::dec << Min;
                ss << ",\n" << parentIndent << jsonIndentStr << "\"maxdam\": " << std::dec << Max;
            }

            if (TwoHanded)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                ss << "\n" << parentIndent << jsonIndentStr << "\"twohandmindam\": " << std::dec << Min2;
                ss << ",\n" << parentIndent << jsonIndentStr << "\"twohandmaxdam\": " << std::dec << Max2;
            }

            if (MisselMax != 0)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                ss << "\n" << parentIndent << jsonIndentStr << "\"minmisdam\": " << std::dec << MissleMin;
                ss << ",\n" << parentIndent << jsonIndentStr << "\"maxmisdam\": " << std::dec << MisselMax;
            }
            ss << "\n" << parentIndent << "}";
        }
    };
}
//---------------------------------------------------------------------------
