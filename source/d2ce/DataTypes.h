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

        void clear()
        {
            Strength = 0;
            Energy = 0;
            Dexterity = 0;
            Vitality = 0;
            StatsLeft = 0;
            SkillChoices = 0;
            CurLife = 0;
            MaxLife = 0;
            CurMana = 0;
            MaxMana = 0;
            CurStamina = 0;
            MaxStamina = 0;
            Level = 1;
            Experience = 0;
            GoldInBelt = 0;
            GoldInStash = 0;
        }

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
            std::uint32_t curLevel = std::max(1u, std::min(Level, NUM_OF_LEVELS));
            if (curLevel <= d2ce::NUM_OF_LEVELS)
            {
                return MinExpRequired[curLevel - 1];
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

        void asJson(std::stringstream& ss, const std::string& parentIndent, bool bSerializedFormat = false) const
        {
            std::string attribParentIndent = parentIndent + jsonIndentStr;
            std::string attribIndent = attribParentIndent + jsonIndentStr;
            if (bSerializedFormat)
            {
                std::string tempName;
                ss << "\n" << attribParentIndent << "{";
                ss << "\n" << attribIndent << "\"Id\": " << std::dec << Id;
                ss << ",\n" << attribIndent << "\"Stat\": \"" << Name << "\"";
                switch (Id)
                {
                case 17:
                case 48:
                case 50:
                case 52:
                    if (!Values.empty())
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[0];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }
                    ss << "\n" << attribParentIndent << "}";
                    ss << ",\n" << attribParentIndent << "{";
                    ss << "\n" << attribIndent << "\"Id\": " << std::dec << Id + 1;
                    switch (Id  +1)
                    {
                    case 18:
                        tempName = "item_mindamage_percent";
                        break;

                    case 49:
                        tempName = "firemaxdam";
                        break;

                    case 51:
                        tempName = "lightmaxdam";
                        break;

                    case 53:
                        tempName = "magicmaxdam";
                        break;
                    }
                    ss << ",\n" << attribIndent << "\"Stat\": \"" << tempName << "\"";
                    if (Values.size() > 1)
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[1];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }
                    break;

                case 54:
                case 57:
                    if (!Values.empty())
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[0];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }
                    ss << "\n" << attribParentIndent << "}";
                    ss << ",\n" << attribParentIndent << "{";
                    ss << "\n" << attribIndent << "\"Id\": " << std::dec << Id + 1;
                    switch (Id + 1)
                    {
                    case 55:
                        tempName = "coldmaxdam";
                        break;

                    case 58:
                        tempName = "poisonmaxdam";
                        break;
                    }
                    ss << ",\n" << attribIndent << "\"Stat\": \"" << tempName << "\"";
                    if (Values.size() > 1)
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[1];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }

                    ss << "\n" << attribParentIndent << "}";
                    ss << ",\n" << attribParentIndent << "{";
                    ss << "\n" << attribIndent << "\"Id\": " << std::dec << Id + 2;
                    switch (Id + 2)
                    {
                    case 56:
                        tempName = "coldlength";
                        break;

                    case 59:
                        tempName = "poisonlength";
                        break;
                    }
                    ss << ",\n" << attribIndent << "\"Stat\": \"" << tempName << "\"";
                    if (Values.size() > 1)
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[1];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }
                    break;

                case 188:
                    if (!Values.empty())
                    {
                        ss << ",\n" << attribIndent << "\"SkillTab \": " << std::dec << Values[0];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"SkillTab \": 0";
                    }

                    if (Values.size() > 1)
                    {
                        ss << ",\n" << attribIndent << "\"SkillLevel \": " << std::dec << Values[1];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"SkillLevel \": 0";
                    }

                    if (Values.size() > 2)
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[2];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }
                    break;

                case 195:
                case 196:
                case 197:
                case 198:
                case 199:
                case 201:
                    if (Values.size() > 1)
                    {
                        ss << ",\n" << attribIndent << "\"SkillId  \": " << std::dec << Values[1];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"SkillId  \": 0";
                    }

                    if (!Values.empty())
                    {
                        ss << ",\n" << attribIndent << "\"SkillLevel  \": " << std::dec << Values[0];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"SkillLevel  \": 0";
                    }

                    if (Values.size() > 2)
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[2];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }
                    break;

                case 204:
                    if (Values.size() > 1)
                    {
                        ss << ",\n" << attribIndent << "\"SkillId  \": " << std::dec << Values[1];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"SkillId  \": 0";
                    }

                    if (!Values.empty())
                    {
                        ss << ",\n" << attribIndent << "\"SkillLevel  \": " << std::dec << Values[0];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"SkillLevel  \": 0";
                    }

                    if (Values.size() > 2)
                    {
                        ss << ",\n" << attribIndent << "\"MaxCharges \": " << std::dec << Values[2];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"MaxCharges \": 0";
                    }

                    if (Values.size() > 3)
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[3];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }
                    break;

                default:
                    if (Values.size() >= 2)
                    {
                        ss << ",\n" << attribIndent << "\"Param\": " << std::dec << Values[0];
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[1];
                    }
                    else if (Values.size() == 1)
                    {
                        ss << ",\n" << attribIndent << "\"Value\": " << std::dec << Values[0];
                    }
                    else
                    {
                        ss << ",\n" << attribIndent << "\"Value\": 0";
                    }
                    break;
                }
                ss << "\n" << attribParentIndent << "}";
            }
            else
            {
                ss << "\n" << attribParentIndent << "{";
                ss << "\n" << attribIndent << "\"id\": " << std::dec << Id;
                ss << ",\n" << attribIndent << "\"values\": [";
                if (Values.empty())
                {
                    ss << "]";
                }
                else
                {
                    size_t idx = 0;
                    for (const auto& val : Values)
                    {
                        if (idx != 0)
                        {
                            ss << ",";
                        }

                        if (val == 0)
                        {
                            // Check if zero is a valid value
                            switch (idx)
                            {
                            case 0:
                                switch (Id)
                                {
                                case 83:
                                case 97:
                                case 107:
                                case 151:
                                case 188:
                                    ss << "\n" << attribIndent << jsonIndentStr << std::dec << val;
                                    break;

                                default:
                                    ss << "\n" << attribIndent << jsonIndentStr << "null";
                                    break;
                                }
                                break;

                            case 1:
                                switch (Id)
                                {
                                case 151:
                                case 188:
                                case 195:
                                case 196:
                                case 197:
                                case 198:
                                case 199:
                                case 201:
                                case 204:
                                    ss << "\n" << attribIndent << jsonIndentStr << std::dec << val;
                                    break;

                                default:
                                    ss << "\n" << attribIndent << jsonIndentStr << "null";
                                    break;
                                }
                                break;

                            default:
                                ss << "\n" << attribIndent << jsonIndentStr << "null";
                                break;
                            }
                        }
                        else
                        {
                            ss << "\n" << attribIndent << jsonIndentStr << std::dec << val;
                        }
                        ++idx;
                    }
                    ss << "\n" << attribIndent << "]";
                    ss << ",\n" << attribIndent << "\"name\": \"" << Name << "\"";
                    if (!OpStats.empty())
                    {
                        ss << ",\n" << attribIndent << "\"op_stats\": [";
                        bool bFirstItem = true;
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

                            ss << "\n" << attribIndent << jsonIndentStr << "\"" << val << "\"";
                        }
                        ss << "\n" << attribIndent << "]";
                        ss << ",\n" << attribIndent << "\"op_value\": " << std::dec << OpValue;
                    }

                    if (!Desc.empty())
                    {
                        ss << ",\n" << attribIndent << "\"description\": \"" << Desc << "\"";
                    }

                    if (!Visible)
                    {
                        ss << ",\n" << attribIndent << "\"visible\": false";
                    }
                }

                ss << "\n" << attribParentIndent << "}";
            }
        }

        static void attributesAsJsonArray(std::stringstream& ss, const std::string& parentIndent, const std::vector<MagicalAttribute>& attribs, bool bSerializedFormat = false)
        {
            std::string attribParentIndent = parentIndent;
            if (bSerializedFormat)
            {
                attribParentIndent += jsonIndentStr;
            }

            ss << "\n" << parentIndent;
            if (bSerializedFormat)
            {
                ss << "\"Stats\": ";
            }
            ss << "[";

            if (attribs.empty())
            {
                ss << "]";
            }
            else
            {
                bool bFirstItem = true;
                for (const auto& attrib : attribs)
                {
                    if (bFirstItem)
                    {
                        bFirstItem = false;
                    }
                    else
                    {
                        ss << ",";
                    }

                    attrib.asJson(ss, parentIndent, bSerializedFormat);
                }
                ss << "\n" << parentIndent << "]";
            }
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

        void asJson(std::stringstream& ss, const std::string& parentIndent, bool bSerializedFormat = false) const
        {
            ss << "\n" << parentIndent << "\"runeword_id\": " << std::dec << Id;
            ss << ",\n" << parentIndent << "\"runeword_name\": \"" << Name << "\"";
            ss << ",\n" << parentIndent << "\"runeword_attributes\": ";
            MagicalAttribute::attributesAsJsonArray(ss, parentIndent, MagicalAttributes, bSerializedFormat);
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

        void setAttributesAsJsonArray(std::stringstream& ss, const std::string& parentIndent, bool bSerializedFormat = false) const
        {
            std::string attribParentIndent = parentIndent + jsonIndentStr;
            if (bSerializedFormat)
            {
                if (SetAttributes.empty())
                {
                    return;
                }

                ss << "\n" << attribParentIndent << "{";
            }
            else
            {
                ss << ",\n" << parentIndent << "\"set_attributes\": [";
                if (SetAttributes.empty())
                {
                    ss << "\n" << parentIndent << "]";
                    return;
                }
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

                MagicalAttribute::attributesAsJsonArray(ss, attribParentIndent, attribs, bSerializedFormat);
            }

            if (bSerializedFormat)
            {
                ss << "\n" << attribParentIndent << "}";
            }
            else
            {
                ss << "\n" << parentIndent << "]";
            }
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent <<  "\"set_id\": " << std::dec << Id;
            ss << ",\n" << parentIndent << "\"set_name\": \"" << Name << "\"";
            ss << ",\n" << parentIndent << "\"set_list_count\": " << std::dec << SetAttributes.size();
            setAttributesAsJsonArray(ss, parentIndent);
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
        std::uint16_t InvWidth = 0;  // non-zero for belts and cube
        std::uint16_t InvHeight = 0; // non-zero for belts and cube

        void clear()
        {
            Width = 0;
            Height = 0;
            InvWidth = 0;
            InvHeight = 0;
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

    struct BaseResistance
    {
        std::int16_t Cold = 0;
        std::int16_t Fire = 0;
        std::int16_t Lightning = 0;
        std::int16_t Poison = 0;

        void clear()
        {
            Cold = 0;
            Fire = 0;
            Lightning = 0;
            Poison = 0;
        }
    };

    struct BaseDamage
    {
        std::uint16_t Min = 0;
        std::uint16_t Max = 0; 
        
        void clear()
        {
            Min = 0;
            Max = 0;
        }

        void add(const BaseDamage& other)
        {
            Min += other.Min;
            Max += other.Max;
        }
    };

    struct ItemDamage
    {
        BaseDamage OneHanded;
        bool bOneOrTwoHanded = false;
        bool bTwoHanded = false;
        BaseDamage TwoHanded;
        BaseDamage Missile;

        void clear()
        {
            OneHanded.clear();
            TwoHanded.clear();
            Missile.clear();
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent << "\"base_damage\": {";
            bool bFirstItem = true;
            if (OneHanded.Max != 0)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                ss << "\n" << parentIndent << jsonIndentStr << "\"mindam\": " << std::dec << OneHanded.Min;
                ss << ",\n" << parentIndent << jsonIndentStr << "\"maxdam\": " << std::dec << OneHanded.Max;
            }

            if (bTwoHanded)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                ss << "\n" << parentIndent << jsonIndentStr << "\"twohandmindam\": " << std::dec << TwoHanded.Min;
                ss << ",\n" << parentIndent << jsonIndentStr << "\"twohandmaxdam\": " << std::dec << TwoHanded.Max;
            }

            if (Missile.Max != 0)
            {
                if (bFirstItem)
                {
                    bFirstItem = false;
                }
                else
                {
                    ss << ",";
                }
                ss << "\n" << parentIndent << jsonIndentStr << "\"minmisdam\": " << std::dec << Missile.Min;
                ss << ",\n" << parentIndent << jsonIndentStr << "\"maxmisdam\": " << std::dec << Missile.Max;
            }
            ss << "\n" << parentIndent << "}";
        }
    };

    struct CorpseHeader
    {
        std::uint16_t IsDead = 0;  // non-0 if you are currently dead
        std::uint32_t Unknown = 0; // unknown
        std::uint32_t X = 0;       // X Locaton of corpse
        std::uint32_t Y = 0;       // Y Locaton of corpse

        void clear()
        {
            IsDead = 0;
            Unknown = 0;
            X = 0;
            Y = 0;
        }

        CorpseHeader& operator=(const CorpseHeader& other)
        {
            IsDead = other.IsDead;
            Unknown = other.Unknown;
            X = other.X;
            Y = other.Y;
            return *this;
        }

        void swap(CorpseHeader& other) noexcept
        {
            IsDead = std::exchange(other.IsDead, std::uint16_t(0));
            Unknown = std::exchange(other.Unknown, std::uint32_t(0));
            X = std::exchange(other.X, std::uint32_t(0));
            Y = std::exchange(other.Y, std::uint32_t(0));
        }

        CorpseHeader& operator=(CorpseHeader&& other) noexcept
        {
            swap(other);
            return *this;
        }

        void asJson(std::stringstream& ss, const std::string& parentIndent) const
        {
            ss << "\n" << parentIndent << "\"is_dead\": " << std::dec << (IsDead ? 1 : 0);
            if (IsDead)
            {
                ss << ",\n" << parentIndent << "\"corpse_location\": {";
                ss << "\n" << parentIndent << jsonIndentStr << "\"unknown\": " << std::dec << Unknown;
                ss << ",\n" << parentIndent << jsonIndentStr << "\"position_x\": " << std::dec << X;
                ss << ",\n" << parentIndent << jsonIndentStr << "\"position_y\": " << std::dec << Y;
                ss << "\n" << parentIndent << "}";
            }
        }
    };
}
//---------------------------------------------------------------------------
