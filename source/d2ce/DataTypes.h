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
#include <json/json.h>

namespace d2ce
{
    struct BasicStats
    {
        EnumCharVersion Version;                                            // Version for Character file
        std::array<char, d2ce::NAME_LENGTH> Name = { 0 };                   // pos 20 (1.09+, otherwise pos 8),
                                                                            // name includes terminating NULL
        bitmask::bitmask<EnumCharStatus> Status = EnumCharStatus::NoDeaths; // pos 36 (1.09+, otherwise, pos 24), determines character status
        bitmask::bitmask<EnumCharTitle> Title = EnumCharTitle::None;        // pos 37 (1.09+, otherwise pos 25), character's title
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

        EnumDifficulty getTitleDifficulty() const
        {
            std::uint8_t titlePos = (Title.bits() & 0x0C) >> 2;
            auto progression = EnumDifficulty::Hell;
            switch (titlePos)
            {
            case 0:
                progression = EnumDifficulty::Normal;
                break;

            case 1:
                progression = EnumDifficulty::Nightmare;
                break;
            }

            return progression;
        }

        bool isGameComplete() const
        {
            return (((Title.bits() & 0x0C) >> 2) == 3) ? true : false;
        }

        bool isDifficultyComplete(d2ce::EnumDifficulty diff) const
        {
            auto progression = getTitleDifficulty();
            if (diff > progression)
            {
                return false;
            }

            if (progression > diff)
            {
                return true;
            }

            // If we have progressed to Hell, we need to check if we completed the game
            if (diff != EnumDifficulty::Hell)
            {
                return false;
            }

            return isGameComplete();
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
            auto idx = (std::uint8_t)getClass();
            if (idx < NUM_OF_CLASSES)
            {
                return ClassNames[idx];
            }

            return "";
        }

        const std::array<char, NAME_LENGTH>& getName() const
        {
            return Name;
        }

        void asJson(Json::Value& parent) const
        {
            Json::Value ear_attributes;
            ear_attributes["class"] = getClassName();
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
                    if (Values.size() > 1)
                    {
                        attrib["Value"] = Values[1];
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

                case 195:
                case 196:
                case 197:
                case 198:
                case 199:
                case 201:
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
                    break;

                case 204:
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
                    break;

                default:
                    if (Values.size() >= 2)
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

        void asJson(Json::Value& parent) const
        {
            parent["rare_name_id"] = Id;
            parent["rare_name"] = Name;
            parent["rare_name_id2"] = Id2;
            parent["rare_name2"] = Name2;

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
}
//---------------------------------------------------------------------------
