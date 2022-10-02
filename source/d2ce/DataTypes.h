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

#include "CharacterConstants.h"
#include "CharacterStatsConstants.h"
#include "Constants.h"
#include "ItemConstants.h"
#include <json/json.h>

namespace d2ce
{
    struct BasicStats
    {
        EnumCharVersion Version;                                            // Version for Character file
        std::array<char, d2ce::NAME_LENGTH> Name = { 0 };                   // pos 267 (D2R 1.2+, pos 20 for 1.09 - 1.14d, otherwise pos 8),
                                                                            // name includes terminating NULL
        bitmask::bitmask<EnumCharStatus> Status = EnumCharStatus::NoDeaths; // pos 36 (1.09+, otherwise, pos 24), determines character status
        std::uint8_t Title = 0;                                             // pos 37 (1.09+, otherwise pos 25), character's title
        EnumCharClass Class = EnumCharClass::Amazon;                        // pos 40 (1.09+, otherwise pos 34),
        EnumDifficulty DifficultyLastPlayed = EnumDifficulty::Normal;
        EnumAct StartingAct = EnumAct::I;

        BasicStats()
        {
            Name.fill(0);
        }

        bool isExpansionCharacter() const
        {
            return (Status & EnumCharStatus::Expansion) == EnumCharStatus::Expansion ? true : false;
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
            return (Status & EnumCharStatus::Ladder) == EnumCharStatus::Ladder ? true : false;
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
            return (Status & EnumCharStatus::Hardcore) == EnumCharStatus::Hardcore ? true : false;
        }

        void setIsHardcoreCharacter(bool flag)
        {
            if (flag)
            {
                Status |= EnumCharStatus::Hardcore;
                Status &= EnumCharStatus::Died; // can't be resurrected
            }
            else
            {
                Status &= EnumCharStatus::Hardcore;
            }
        }

        bool isResurrectedCharacter() const
        {
            return (Status & EnumCharStatus::Died) == EnumCharStatus::Died ? true : false;
        }

        void setIsResurrectedCharacter(bool flag)
        {
            if (isHardcoreCharacter())
            {
                flag = false; // Hardcore character can't be resurrected
            }

            if (flag)
            {
                Status |= EnumCharStatus::Died;
            }
            else
            {
                Status &= ~EnumCharStatus::Died;
            }
        }

        bool isDeadCharacter() const
        {
            return (Status & EnumCharStatus::Dead) == EnumCharStatus::Dead ? true : false;
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

        bool isFemaleCharacter() const
        {
            switch (Class)
            {
            case d2ce::EnumCharClass::Amazon:
            case d2ce::EnumCharClass::Assassin:
            case d2ce::EnumCharClass::Sorceress:
                return true;
                break;

            default:
                return false;
            }
        }

        std::uint8_t getNumActs() const
        {
            return std::uint8_t(isExpansionCharacter() ? 5 : 4);
        }

        std::uint8_t getGameCompleteTitle() const
        {
            return std::uint8_t(getNumActs() * 3);
        }

        std::uint8_t getStartingActTitle() const
        {
            return std::uint8_t(static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(DifficultyLastPlayed) * getNumActs() + static_cast<std::underlying_type_t<d2ce::EnumAct>>(StartingAct));
        }

        bool isGameComplete() const
        {
            return (getGameCompleteTitle() <= Title) ? true : false;
        }

        EnumDifficulty getTitleDifficulty() const
        {
            if (isGameComplete())
            {
                return EnumDifficulty::Hell;
            }

            return static_cast<EnumDifficulty>(Title / getNumActs());
        }

        EnumCharTitle getTitleEnum() const
        {
            if (isGameComplete())
            {
                return EnumCharTitle::BaronBaroness;
            }

            switch (getTitleDifficulty())
            {
            case EnumDifficulty::Hell:
                return EnumCharTitle::LordLady;

            case EnumDifficulty::Nightmare:
                return EnumCharTitle::SirDame;
            }

            return EnumCharTitle::None;
        }

        EnumAct getTitleAct() const
        {
            if (isGameComplete())
            {
                return static_cast<EnumAct>(getNumActs() - 1);
            }

            return static_cast<EnumAct>(Title % getNumActs());
        }

        bool isDifficultyComplete(d2ce::EnumDifficulty diff) const
        {
            auto title = std::uint8_t(getNumActs() * (static_cast<std::underlying_type_t<EnumDifficulty>>(diff) + 1));
            return title <= Title ? true : false;
        }

        EnumAct getLastAct() const
        {
            return isExpansionCharacter() ? EnumAct::V : EnumAct::IV;
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
        std::uint32_t MaxLevel = 99ui32;
        std::uint32_t MinExperienceLevel = 0ui32;
        std::uint32_t NextExperienceLevel = 500ui32;
        std::uint32_t MaxExperience = 3600000000ui32;
        std::uint32_t MaxGoldInBelt = 10000ui32;
        std::uint32_t MaxGoldInStash = d2ce::GOLD_IN_STASH_LIMIT;

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
            MaxLevel = 99ui32;
            MinExperienceLevel = 0ui32;
            NextExperienceLevel = 500ui32;
            MaxExperience = 3600000000ui32;
            MaxGoldInBelt = 10000ui32;
            MaxGoldInStash = d2ce::GOLD_IN_STASH_LIMIT;
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
        std::array<char, NAME_LENGTH> Name;

        EarAttributes()
        {
            Name.fill(0);
        }

        void clear()
        {
            Class = EnumCharClass::Amazon;
            Level = 1;
            Name.fill(0);
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
            return CharClassHelper::getClassName(getClass());
        }

        const std::array<char, NAME_LENGTH>& getName() const
        {
            return Name;
        }

        void asJson(Json::Value& parent) const
        {
            Json::Value ear_attributes;
            ear_attributes["class"] = getClassName();
            ear_attributes["class_id"] = std::uint16_t(getClass());
            ear_attributes["level"] = getLevel();
            ear_attributes["name"] = getName().data();
            parent["ear_attributes"] = ear_attributes;
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
        EnumItemVersion Version = EnumItemVersion::v100;
        std::uint16_t GameVersion = 0;
        std::uint8_t DescPriority = 0;   // The higher this value is the further up in the item description this stat will be listed
        std::uint8_t encode = 0;

        void clear()
        {
            Id = 0;
            Name.clear();
            Desc.clear();
            Values.clear();
            OpValue = 0;
            OpStats.clear();
            Visible = true;
            Version = EnumItemVersion::v100;
            GameVersion = 0;
            encode = 0;
        }

        void asJson(Json::Value& parent, bool bSerializedFormat = false) const
        {
            if (!parent.isArray())
            {
                return;
            }

            Json::Value attrib;
            if (bSerializedFormat)
            {
                attrib["Id"] = Id;
                attrib["Stat"] = Name;
                switch (Id)
                {
                case 17:
                case 48:
                case 50:
                case 52:
                    if (!Values.empty())
                    {
                        attrib["Value"] = Values[0];
                    }
                    else
                    {
                        attrib["Value"] = 0;
                    }
                    parent.append(attrib);
                    attrib.clear();
                    attrib["Id"] = Id + 1;
                    switch (Id + 1)
                    {
                    case 18:
                        attrib["Stat"] = "item_mindamage_percent";
                        break;

                    case 49:
                        attrib["Stat"] = "firemaxdam";
                        break;

                    case 51:
                        attrib["Stat"] = "lightmaxdam";
                        break;

                    case 53:
                        attrib["Stat"] = "magicmaxdam";
                        break;
                    }

                    if (Values.size() > 1)
                    {
                        attrib["Value"] = Values[1];
                    }
                    else
                    {
                        attrib["Value"] = 0;
                    }
                    break;

                case 54:
                case 57:
                    if (!Values.empty())
                    {
                        attrib["Value"] = Values[0];
                    }
                    else
                    {
                        attrib["Value"] = 0;
                    }
                    parent.append(attrib);
                    attrib.clear();
                    attrib["Id"] = Id + 1;
                    switch (Id + 1)
                    {
                    case 55:
                        attrib["Stat"] = "coldmaxdam";
                        break;

                    case 58:
                        attrib["Stat"] = "poisonmaxdam";
                        break;
                    }
                    if (Values.size() > 1)
                    {
                        attrib["Value"] = Values[1];
                    }
                    else
                    {
                        attrib["Value"] = 0;
                    }
                    parent.append(attrib);
                    attrib.clear();
                    attrib["Id"] = Id + 2;
                    switch (Id + 2)
                    {
                    case 56:
                        attrib["Stat"] = "coldlength";
                        break;

                    case 59:
                        attrib["Stat"] = "poisonlength";
                        break;
                    }
                    if (Values.size() > 2)
                    {
                        attrib["Value"] = Values[2];
                    }
                    else
                    {
                        attrib["Value"] = 0;
                    }
                    break;

                case 188:
                    if (!Values.empty())
                    {
                        attrib["SkillTab"] = Values[0];
                    }
                    else
                    {
                        attrib["SkillTab"] = 0;
                    }

                    if (Values.size() > 1)
                    {
                        attrib["SkillLevel"] = Values[1];
                    }
                    else
                    {
                        attrib["SkillLevel"] = 0;
                    }

                    if (Values.size() > 2)
                    {
                        attrib["Value"] = Values[2];
                    }
                    else
                    {
                        attrib["Value"] = 0;
                    }
                    break;

                default:
                    if (encode == 2)
                    {
                        if (Values.size() > 1)
                        {
                            attrib["SkillId"] = Values[1];
                        }
                        else
                        {
                            attrib["SkillId"] = 0;
                        }

                        if (!Values.empty())
                        {
                            attrib["SkillLevel"] = Values[0];
                        }
                        else
                        {
                            attrib["SkillLevel"] = 0;
                        }

                        if (Values.size() > 2)
                        {
                            attrib["Value"] = Values[2];
                        }
                        else
                        {
                            attrib["Value"] = 0;
                        }
                    }
                    else if (encode == 3)
                    {
                        if (Values.size() > 1)
                        {
                            attrib["SkillId"] = Values[1];
                        }
                        else
                        {
                            attrib["SkillId"] = 0;
                        }

                        if (!Values.empty())
                        {
                            attrib["SkillLevel"] = Values[0];
                        }
                        else
                        {
                            attrib["SkillLevel"] = 0;
                        }

                        if (Values.size() > 2)
                        {
                            attrib["MaxCharges"] = Values[2];
                        }
                        else
                        {
                            attrib["MaxCharges"] = 0;
                        }

                        if (Values.size() > 3)
                        {
                            attrib["Value"] = Values[3];
                        }
                        else
                        {
                            attrib["Value"] = 0;
                        }
                    }
                    else if (encode == 4)
                    {
                        // time-based stats were never implemented, but we handle them
                        std::uint64_t value = 0;
                        switch (Values.size())
                        {
                        case 1:
                            value = Values[0];
                            break;

                        case 3:
                            value |= (Values[2] & 0x3FF) << 12;
                            if (Values.size() > 1)
                            {
                                value |= (Values[1] & 0x3FF) << 2;
                            }
                            value |= Values[0] & 0x3;
                            break;
                        }
                        attrib["Value"] = value;
                    }
                    else if (Values.size() >= 2)
                    {
                        attrib["Param"] = Values[0];
                        attrib["Value"] = Values[1];
                    }
                    else if (Values.size() == 1)
                    {
                        attrib["Value"] = Values[0];
                    }
                    else
                    {
                        attrib["Value"] = 0;
                    }
                    break;
                }
                parent.append(attrib);
            }
            else
            {
                size_t idx = 0;
                attrib["id"] = Id;
                Json::Value null_value;
                Json::Value values(Json::arrayValue);
                for (auto val : Values)
                {
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
                                values.append(val);
                                break;

                            default:
                                values.append(null_value);
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
                                values.append(val);
                                break;

                            default:
                                ++idx;
                                values.append(null_value);
                                break;
                            }
                            break;

                        default:
                            values.append(null_value);
                            break;
                        }
                    }
                    else
                    {
                        values.append(val);
                    }
                    ++idx;
                }
                attrib["values"] = values;
                attrib["name"] = Name;
                if (!OpStats.empty())
                {
                    Json::Value opStats(Json::arrayValue);
                    for (const auto& val : OpStats)
                    {
                        opStats.append(val);
                    }
                    attrib["op_stats"] = opStats;
                    attrib["op_value"] = OpValue;
                }
                if (!Desc.empty())
                {
                    attrib["description"] = Desc;
                }

                if (!Visible)
                {
                    attrib["visible"] = false;
                }
                parent.append(attrib);
            }
        }

        static void attributesAsJsonArray(Json::Value& parent, const std::vector<MagicalAttribute>& attribs, bool bSerializedFormat = false)
        {
            if (!parent.isArray())
            {
                return;
            }

            if (bSerializedFormat)
            {
                Json::Value attribElement;
                Json::Value stats(Json::arrayValue);
                for (const auto& attrib : attribs)
                {
                    attrib.asJson(stats, bSerializedFormat);
                }
                attribElement["Stats"] = stats;
                parent.append(attribElement);
            }
            else
            {
                for (const auto& attrib : attribs)
                {
                    attrib.asJson(parent, bSerializedFormat);
                }
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

        void asJson(Json::Value& parent, bool bSerializedFormat = false) const
        {
            parent["runeword_id"] = Id;
            parent["runeword_name"] = Name;
            Json::Value runewordAttributes(Json::arrayValue);
            MagicalAttribute::attributesAsJsonArray(runewordAttributes, MagicalAttributes, bSerializedFormat);
            parent["runeword_attributes"] = runewordAttributes;
        }
    };

    struct SetAttributes
    {
        std::uint16_t Id = 0;
        std::string Name;
        std::uint16_t ReqLevel = 0;
        std::vector<std::vector<MagicalAttribute>> SetAttributes;

        void clear()
        {
            Id = 0;
            Name.clear();
            ReqLevel = 0;
            SetAttributes.clear();
        }

        std::uint8_t getBonusBits()
        {
            std::bitset<5> bonusBits;
            for (size_t idx = 0; idx < std::min(SetAttributes.size(), bonusBits.size()); ++idx)
            {
                bonusBits[idx] = 1;
            }

            return std::uint8_t(bonusBits.to_ulong());
        }

        void setAttributesAsJsonArray(Json::Value& parent, bool bSerializedFormat = false) const
        {
            if (!parent.isArray() || SetAttributes.empty())
            {
                return;
            }

            if (bSerializedFormat)
            {
                for (const auto& attribs : SetAttributes)
                {
                    MagicalAttribute::attributesAsJsonArray(parent, attribs, bSerializedFormat);
                }
            }
            else
            {
                for (const auto& attribs : SetAttributes)
                {
                    Json::Value setAttribs(Json::arrayValue);
                    MagicalAttribute::attributesAsJsonArray(setAttribs, attribs, bSerializedFormat);
                    parent.append(setAttribs);
                }

            }
        }

        void asJson(Json::Value& parent) const
        {
            parent["set_id"] = Id;
            parent["set_name"] = Name;
            parent["set_list_count"] = SetAttributes.size();
            Json::Value setAttributes(Json::arrayValue);
            setAttributesAsJsonArray(setAttributes);
            parent["set_attributes"] = setAttributes;
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

        void asJson(Json::Value& parent) const
        {
            parent["magic_prefix"] = PrefixId;
            if (PrefixId != 0)
            {
                parent["magic_prefix_name"] = PrefixName;
            }

            parent["magic_suffix"] = SuffixId;
            if (SuffixId != 0)
            {
                parent["magic_suffix_name"] = SuffixName;
            }
        }
    };

    struct RareAttributes
    {
        std::uint16_t Id = 0;
        std::string Name;
        std::string Index;
        std::uint16_t Id2 = 0;
        std::string Name2;
        std::string Index2;
        std::vector<MagicalAffixes> Affixes;

        void clear()
        {
            Id = 0;
            Name.clear();
            Index.clear();
            Id2 = 0;
            Name2.clear();
            Index2.clear();
            Affixes.clear();
        }

        void asJson(Json::Value& parent) const
        {
            parent["rare_name_id"] = Id;
            parent["rare_name"] = Name;
            if (Name != Index)
            {
                parent["rare_index"] = Index;

            }

            parent["rare_name_id2"] = Id2;
            parent["rare_name2"] = Name2;
            if (Name2 != Index2)
            {
                parent["rare_index2"] = Index2;

            }

            Json::Value magicalNameIds(Json::arrayValue);
            for (const auto& val : Affixes)
            {
                magicalNameIds.append(val.PrefixId);
                magicalNameIds.append(val.SuffixId);
            }
            parent["magical_name_ids"] = magicalNameIds;
        }
    };

    struct UniqueAttributes
    {
        std::uint16_t Id = 0;
        std::string Name;
        std::uint16_t ReqLevel = 0;

        void clear()
        {
            Id = 0;
            Name.clear();
            ReqLevel = 0;
        }

        void asJson(Json::Value& parent) const
        {
            parent["unique_id"] = Id;
            parent["unique_name"] = Name;
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
        std::uint16_t Base = 0;
        bool CurrentBit9 = false; // for v1.10 and higher we have an unknown 9th bit for Current value

        void clear()
        {
            Current = 0;
            Max = 0;
            Base = 0;
            CurrentBit9 = false;
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
        BaseDamage OneHanded;         // Minimum/Maximum one-handed damage
        bool bOneOrTwoHanded = false; // Can weapon be carried in 1 or 2 hands by Barbarian?
        bool bTwoHanded = false;      // is this a two handed weapon?
        BaseDamage TwoHanded;         // Minimum/Maximum two-handed damage
        BaseDamage Missile;           // Minimum/Maximum ranged damage

        void clear()
        {
            OneHanded.clear();
            TwoHanded.clear();
            bOneOrTwoHanded = false;
            bTwoHanded = false;
            Missile.clear();
        }

        void asJson(Json::Value& parent) const
        {
            Json::Value baseDamage;
            if (OneHanded.Max != 0)
            {
                baseDamage["mindam"] = OneHanded.Min;
                baseDamage["maxdam"] = OneHanded.Max;
            }

            if (bTwoHanded)
            {
                baseDamage["twohandmindam"] = TwoHanded.Min;
                baseDamage["twohandmaxdam"] = TwoHanded.Max;
            }

            if (Missile.Max != 0)
            {
                baseDamage["minmisdam"] = Missile.Min;
                baseDamage["maxmisdam"] = Missile.Max;
            }
            parent["base_damage"] = baseDamage;
        }
    };

    struct ItemArmorClass
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

    struct ItemLevel
    {
        std::uint16_t Quality = 0; // Base Item Level (also known as Quality Level - qlvl)
        std::uint16_t Magic = 0;   // Additional Magic Level (also known as mlvl)

        void clear()
        {
            Quality = 0;
            Magic = 0;
        }
    };

    struct ItemStackable
    {
        std::uint32_t Min = 0;
        std::uint32_t Max = 0;

        void clear()
        {
            Min = 0;
            Max = 0;
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

        void asJson(Json::Value& parent) const
        {
            parent["is_dead"] = (IsDead ? 1 : 0);
            if (IsDead)
            {
                Json::Value corpseLocation;
                corpseLocation["unknown"] = Unknown;
                corpseLocation["position_x"] = X;
                corpseLocation["position_y"] = Y;
                parent["corpse_location"] = corpseLocation;
            }
        }
    };

    struct MagicalCachev100
    {
        MagicalAffixes Affixes;
        std::vector<MagicalAttribute> MagicalAttributes;

        MagicalCachev100()
        {
            Affixes.PrefixId = MAXUINT16;
            Affixes.SuffixId = MAXUINT16;
        }

        void clear()
        {
            Affixes.clear();
            MagicalAttributes.clear();
            Affixes.PrefixId = MAXUINT16;
            Affixes.SuffixId = MAXUINT16;
        }
    };

    struct RareOrCraftedCachev100
    {
        std::uint16_t Id = 0;
        std::string Name;
        std::string Index;
        std::uint16_t Id2 = 0;
        std::string Name2;
        std::string Index2;
        std::vector<MagicalCachev100> affixes;

        RareOrCraftedCachev100()
        {
            Id = MAXUINT16;
            Id2 = MAXUINT16;
        }

        void clear()
        {
            Id = MAXUINT16;
            Name.clear();
            Index.clear();
            Id2 = MAXUINT16;
            Name2.clear();
            Index2.clear();
            affixes.clear();
        }
    };

    //---------------------------------------------------------------------------
    struct OpAttributes
    {
        std::uint8_t op = 0;
        std::uint8_t op_param = 0;
        std::string op_base;
        std::vector<std::string> op_stats;
    };

    //---------------------------------------------------------------------------
    struct ItemStat
    {
        std::uint16_t id = 0;            // internal ID
        std::string name;                // An Id pointer used in other *.txt files (Properties.txt, Skills.txt, States.txt and also within ItemStatCost.txt) to refer to this stat, this field must be unique, otherwise you're looking into a world of glitches and bugs.
        std::uint16_t charSaveBits = 0;  // How many bits of data are allocated to the value of this stat within the *.d2s file stat section when the stat is saved
        std::uint8_t encode = 0;         // This field tells the game whenever to encode the stat (and in this case in what way to encode it) when it is saved to conserve space
        std::uint16_t saveBits = 0;      // These fields control how many bits of data are allocated to the value of this stat within the *.d2s fil
        std::int16_t saveAdd = 0;
        std::uint16_t saveParamBits = 0; // This field controls how many bits of data are allocated to the parameter of this stat when it is saved on items
        OpAttributes opAttribs;          // These fields control special forms of stat assignment used for example for stats that add a bonus depending on cLvl
        std::uint8_t descPriority = 0;   // The higher this value is the further up in the item description this stat will be listed
        std::uint8_t descFunc = 0;       // The function used for generating the description for this stat, this works like the descfuncs in SkillDesc.txt pretty much
        std::uint8_t descVal = 0;        // Controls whenever and if so in what way the stat value is shown, 0 = doesn't show the value of the stat, 1 = shows the value of the stat infront of the description, 2 = shows the value of the stat after the description.
        std::string desc;                // base description string
        std::string descNeg;             // The string used for the description when the stat value is negative.
        std::string descGrp;             // When all stats in this group are present on the same item and they all have the same value, replace their description with the associated group description.
        std::string descGrpNeg;          // The string used for the description when the stat value is negative for the group
        std::string descRange;           // The description used for range stats (i.e. fire damage)
        std::string descNoRange;         // The description used for ragne stats when the min/max value is the same
        std::uint16_t nextInChain = 0;   // stat that follows this one (i.e. firemaxdam follows firemindam)
        bool isRootInChain = false;      // is this the root of the chain?
    };

    //---------------------------------------------------------------------------
    struct SpellDescStruct
    {
        // 0 = don't use any description or calculation.
        // 1 = use description only
        // 2 = use description and calculation.
        std::uint8_t descFunc = 0;

        std::string desc;            // description string
        std::uint64_t descCalc = 0;  // value to use with description
    };

    //---------------------------------------------------------------------------
    class Item;
    struct ItemType
    {
        std::string name;

        // 0 = Classic D2, 100 = Expansion
        // An item set to 100 cannot be generated in Classic Diablo II.
        std::uint16_t version = 0;

        // This is related to the savegames. An item that is saved in a compact way takes less space
        // in the save file, but in counter-part it has no stats : no quantity, no automagic, no
        // affixes, no damage, no armor, no durability ...
        // This is used by items that don't need any stats to be stored, like Healing Potions or Gems.
        bool compactsave = false;

        // Minimumand Maximum Armor Class(AC) this item can have.
        ItemArmorClass ac;

        // min/max damage the weapon can cause
        ItemDamage dam;

        // The minimum level, strength and/or dexterity the Player must be to use that item.
        ItemRequirements req;

        // Base Durability
        // Max value of 0 means then item is indestructible, othewise same value as Base
        ItemDurability durability;

        // This is a 3-letter code
        std::string code;

        // v1.00 to 1.03 used an Id instad of a 3-letter code
        std::uint16_t code_v100 = MAXUINT16;

        // Base Item Level (also known as Quality Level - qlvl) and Additional Magic Level (also known as mlvl)
        ItemLevel level;

        // Class specific value
        std::uint16_t auto_prefix = 0;

        // width and height in inventory cells
        // For Belts and Cubes, also holds the width and height of those inventories
        ItemDimensions dimensions;

        // min/max amount of quantity (Max having value of 0 means the item is not stackable).
        ItemStackable stackable;

        // Maximumn Sockets for item mapped by item level threshold (default 1, 25 and 40)
        std::map<std::uint16_t, std::uint8_t> max_sockets;

        std::vector<std::string> categories;

        // gem effect
        // 0 = weapon
        // 1 = armor/helm
        // 2 = shield
        // If you use another value, your item won't be able to have sockets.
        std::uint8_t gemApplyType = 0;

        // item in the inventory
        std::string inv_file;

        // colormap to use for the inventory version.
        std::uint16_t inv_transform = 0;

        // can item be personalized?
        bool nameable = false;

        // can item go in the belt?
        bool beltable = false;

        // if true, skip adding the item’s base name in its title
        bool skipName = false;

        SpellDescStruct spellDesc; // For potions, the description and points applied
        
        std::vector<EnumEquippedId> bodyLocations;
        std::optional<std::string> quiverCode;
        std::vector<std::string> codes; // normal, uber, ultra

        virtual bool hasCategory(const std::string category) const;
        virtual bool hasCategoryCode(const std::string code) const;
        virtual bool isStackable() const;
        virtual bool isWeapon() const;
        virtual bool isThrownWeapon() const;
        virtual bool isMissileWeapon() const;
        virtual bool isMissile() const;
        virtual bool isTwoHandedWeapon() const;
        virtual bool isOneOrTwoHandedWeapon() const; // Can weapon be carried in 1 or 2 hands by Barbarian?
        virtual bool isShield() const;
        virtual bool isArmor() const;
        virtual bool isHelm() const;
        virtual bool isMiscellaneous() const;
        virtual bool isBook() const;
        virtual bool isPotion() const;
        virtual bool isJewel() const;
        virtual bool isGem() const;
        virtual bool isQuestItem() const;
        virtual bool isGoldItem() const;
        virtual bool isRejuvenationPotion() const;
        virtual bool isHealingPotion() const;
        virtual bool isManaPotion() const;
        virtual bool isSocketFiller() const;
        virtual bool isUpgradableGem() const;
        virtual bool isUpgradableRejuvenationPotion() const;
        virtual bool isUpgradablePotion() const;
        virtual bool isUpgradableItem() const;
        virtual bool isRune() const;
        virtual bool isCharm() const;
        virtual bool isBelt() const;
        virtual bool isBoots() const;
        virtual bool isBeltable() const;
        virtual bool isScroll() const;
        virtual bool isKey() const;
        virtual bool isHoradricCube() const;
        virtual bool isRing() const;
        virtual bool isAmulet() const;
        virtual bool isBodyPart() const;
        virtual bool isPlayerBodyPart() const;
        virtual bool isSimpleItem() const;
        virtual bool isUnusedItem() const;
        virtual bool isExpansionItem() const;
        virtual bool isSecondHand() const;
        virtual bool isClassSpecific() const;
        virtual bool isUniqueItem() const;
        virtual bool isSetItem() const;
        virtual std::uint16_t getId() const;
        virtual std::uint32_t getSetItemDWBCode() const;
        virtual std::uint16_t getSetBonusBits() const;
        std::optional<d2ce::EnumCharClass> getClass() const;
        virtual bool hasMultipleGraphics() const;
        virtual std::uint32_t getNumPictureIds() const;

        virtual bool hasUndeadBonus() const;

        virtual bool canHaveSockets() const;
        std::uint8_t getMaxSockets(std::uint8_t level) const;

        virtual bool canPersonalize() const;

        virtual bool canEquip(EnumEquippedId equipId) const;
        virtual bool canEquip(EnumEquippedId equipId, EnumCharClass charClass) const;

        virtual bool getSocketedMagicalAttributes(const d2ce::Item& item, std::vector<MagicalAttribute>& attribs, std::uint8_t parentGemApplyType = 0) const;
        virtual bool getRuneMagicalAttributes(const d2ce::Item& parentItem, std::vector<MagicalAttribute>& attribs) const;

        virtual std::string getPotionDesc(EnumCharClass charClass) const;

        virtual const std::string& getRuneLetter() const;
    };

    struct RunewordType
    {
        std::uint16_t id = 0; // id of the runeword
        std::string name;     // what string will be displayed in-game for this set item

        bool serverOnly = false; // Is this runeword restricted to the realms

        std::vector<std::string> included_categories; // what item types this runeword can appear on
        std::vector<std::string> excluded_categories; // what item types this runeword will never appear on

        std::vector<std::string> runeCodes; // What runes are required to make the runeword and in what order they are to be socketed.

        std::vector<MagicalAttribute> attribs; // The modifiers this runeword will give to items

        // The general level requirement that your character must meet before they can use this runeword.
        std::uint16_t levelreq = 0;
        d2ce::EnumItemVersion version = d2ce::EnumItemVersion::v107; // min required version
    };

    struct UniqueItemType
    {
        std::uint16_t id = 0; // index of the unique item
        std::string name; // what string will be displayed in-game for this unique item

        d2ce::ItemType& baseItemType; // the base type of this unique item

        std::vector<MagicalAttribute> attribs; // The modifiers this item has
    };
}
//---------------------------------------------------------------------------
