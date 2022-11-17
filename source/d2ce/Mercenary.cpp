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
#include "Mercenary.h"
#include "MercenaryConstants.h"
#include "Character.h"
#include <sstream>
#include <random>
#include <rapidcsv/rapidcsv.h>
#include <helpers/ItemHelpers.h>

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint32_t CHAR_V109_MERC_DEAD_BYTE_OFFSET = 177i32;       // pos 177 (1.09+ only)
    constexpr std::uint32_t CHAR_V109_MERC_ID_BYTE_OFFSET = 179i32;         // pos 179 (1.09+ only)
    constexpr std::uint32_t CHAR_V109_MERC_NAME_BYTE_OFFSET = 183i32;       // pos 183 (1.09+ only)
    constexpr std::uint32_t CHAR_V109_MERC_TYPE_BYTE_OFFSET = 185i32;       // pos 185 (1.09+ only)
    constexpr std::uint32_t CHAR_V109_MERC_EXPERIENCE_BYTE_OFFSET = 187i32; // Experience: pos 187 (1.09+ only)

    // must be in the form of "<string><number>"
    bool ExtractPrefixAndNumber(const std::string& input, std::string& prefix, std::uint16_t& value)
    {
        prefix.clear();
        value = 0;
        std::stringstream ss;
        std::stringstream ssValue;
        bool bFoundNumber = false;
        for (auto& ch : input)
        {
            if (ch >= '0' && ch <= '9')
            {
                bFoundNumber = true;
                ssValue << ch;
            }
            else if (bFoundNumber)
            {
                return false;
            }
            else
            {
                ss << ch;
            }
        }

        prefix = ss.str();
        std::string valueStr = ssValue.str();
        if (valueStr.empty())
        {
            // no digits
            return false;
        }

        value = static_cast<std::uint16_t>(std::stol(valueStr));
        return true;
    }

    struct MercSkillInfo
    {
        std::string index;            // The ID pointer used to reference this skill
        std::uint16_t id = MAXUINT16; // The actual ID number of the skill, this is what the pointer actually points at, this must be a unique number
        std::uint16_t Level = 0;      // The sLvl of the Skill at Base lvl
        std::uint16_t LvlPerLvl = 0;  // Factor in improving slvl at lvl up (0=no increment).
    };

    struct MercStatInfo
    {
        std::uint32_t Level = 1;                             // rmlvl of the hireling (what level is required to 'upgrade' skills / stats)
        std::uint32_t ExpPerLevel = 0;                       // Factor used in the calculation of Exp required between mlvls. The threshold for the next level (L+1) from the current level (L) is (exp/lvl)*L*L*(L+1).
        std::uint32_t Life = 0;                              // Life at Base Lvl.
        std::uint32_t LifePerLvl = 0;                        // Amount of additional Life given at Lvl up.
        std::uint16_t Defense = 0;                           // DR (Defence Rating) at Base Lvl.
        std::uint16_t DefPerLvl = 0;                         // Amount of additional DR given at Lvl up.
        std::uint32_t Strength = 0;                          // Strength at Base Lvl.
        std::uint32_t StrPerLvl = 0;                         // Amount (in 8ths) of additional Strength given at Lvl up.
        std::uint32_t Dexterity = 0;                         // Dexterity at Base Lvl.
        std::uint32_t DexPerLvl = 0;                         // Amount (in 8ths) of additional Dexterity given at Lvl up.
        std::uint16_t AttackRating = 0;                      // Attack Rating at Base Lvl.
        std::uint16_t ARPerLvl = 0;                          // Amount of additional AR given at Lvl up.
        BaseDamage Damage;                                   // Range of Damage dealt at Base lvl.
        std::uint16_t DmgPerLvl = 0;                         // Amount (in 8ths) of additional Damage given at Lvl up.
        std::uint16_t Resist = 0;                            // All Resistance at Base Lvl.
        std::uint16_t ResistPerLvl = 0;                      // Amount (in 8ths) of additional All Resist given at lvl up.
        std::vector<MercSkillInfo> skills;                   // the skill properties
    };

    struct MercTypeInfo
    {
        std::uint32_t Version = 0;                           // the version for this hireling
        std::uint16_t Id = 0;                                // the type Id for this hireling
        EnumMercenaryClass Class = EnumMercenaryClass::None; // the class for the hireling
        std::string ClassName;                               // the class name as shown in-game for the hireling
        EnumDifficulty Difficulty = EnumDifficulty::Normal;  // the difficulty level for the hireling
        std::uint8_t AttributeId = 0;                        // the attribute index for the hireling
        std::string AttributeName;                           // the attribute name as shown in-game for the hireling info
        std::map<std::uint32_t, MercStatInfo> Levels;        // the stat attributes properties
        std::optional<EnumCharClass> EquivClass;
    };

    typedef std::map<std::uint16_t, MercTypeInfo> MercTypeInfoMap;
    std::map<std::uint32_t, MercTypeInfoMap> s_MercTypeInfo;
    typedef std::map<EnumDifficulty, std::map<std::uint8_t, std::vector<std::uint16_t>>> MercDifficultyMap;
    MercDifficultyMap s_LegacyBarbarianTypes;
    typedef std::map<EnumMercenaryClass, MercDifficultyMap> MercTypeMap;
    std::map<std::uint32_t, MercTypeMap> s_MercTypes;
    std::map<d2ce::EnumMercenaryClass, std::vector<std::string>> s_MercNamesMap;
    std::map<d2ce::EnumMercenaryClass, std::vector<std::string>> s_MercAttributesMap;
    std::vector<std::string> s_MercClassNames;
    void InitMercData(const ITxtReader& txtReader)
    {
        static const ITxtReader* pCurTextReader = nullptr;
        if (!s_MercTypeInfo.empty())
        {
            if(pCurTextReader == &txtReader)
            {
                // already initialized
                return;
            }

            s_MercNamesMap.clear();
            s_MercTypes.clear();
            s_LegacyBarbarianTypes.clear();
            s_MercTypeInfo.clear();
        }

        pCurTextReader = &txtReader;
        auto pDoc(txtReader.GetHirelingTxt());
        auto& doc = *pDoc;

        const std::array<std::string, 5> mercStringNames = { "", "Rogue Scout", "Desert Mercenary", "Eastern Sorceror", "Barbarian" };
        const std::map<d2ce::EnumMercenaryClass, std::string> mercClassNameCode =
          { { d2ce::EnumMercenaryClass::None, ""}, { d2ce::EnumMercenaryClass::RogueScout, "RogueScout"}, 
            { d2ce::EnumMercenaryClass::DesertMercenary, "DesertMerc"}, { d2ce::EnumMercenaryClass::IronWolf, "Iron Wolf" },
            { d2ce::EnumMercenaryClass::Barbarian, "Barbarian" } };
        const std::map<std::uint16_t, d2ce::EnumMercenaryClass> mercClassToEnumClass = {
            { 271ui16, d2ce::EnumMercenaryClass::RogueScout}, { 338ui16, d2ce::EnumMercenaryClass::DesertMercenary},
            { 359ui16, d2ce::EnumMercenaryClass::IronWolf }, { 560ui16, d2ce::EnumMercenaryClass::Barbarian },
            { 561ui16, d2ce::EnumMercenaryClass::Barbarian }
        };

        std::map<std::uint32_t, MercTypeInfoMap> mercTypeInfo;
        MercDifficultyMap legacyBarbarianTypes;
        std::map<std::uint32_t, MercTypeMap> mercTypes;
        std::map<d2ce::EnumMercenaryClass, std::vector<std::string>> mercNamesMap;
        std::map<d2ce::EnumMercenaryClass, std::vector<std::string>> mercAttributesMap;
        std::vector<std::string> mercClassNames;
        size_t numRows = doc.GetRowCount();
        const SSIZE_T versionColumnIdx = doc.GetColumnIdx("Version");
        const SSIZE_T classColumnIdx = doc.GetColumnIdx("Class");
        const SSIZE_T hirelingColumnIdx = doc.GetColumnIdx("Hireling");
        if (hirelingColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T idColumnIdx = doc.GetColumnIdx("Id");
        if (idColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T difficultyColumnIdx = doc.GetColumnIdx("Difficulty");
        if (difficultyColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T levelColumnIdx = doc.GetColumnIdx("Level");
        if (levelColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T expPerLevelColumnIdx = doc.GetColumnIdx("Exp/Lvl");
        if (expPerLevelColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T lifeColumnIdx = doc.GetColumnIdx("HP");
        if (lifeColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T lifePerLvlColumnIdx = doc.GetColumnIdx("HP/Lvl");
        if (lifePerLvlColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T defenseColumnIdx = doc.GetColumnIdx("Defense");
        if (defenseColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T defPerLvlColumnIdx = doc.GetColumnIdx("Def/Lvl");
        if (defPerLvlColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strengthColumnIdx = doc.GetColumnIdx("Str");
        if (strengthColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strPerLvlColumnIdx = doc.GetColumnIdx("Str/Lvl");
        if (strPerLvlColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T dexterityColumnIdx = doc.GetColumnIdx("Dex");
        if (dexterityColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T dexPerLvlColumnIdx = doc.GetColumnIdx("Dex/Lvl");
        if (dexPerLvlColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T attackRatingColumnIdx = doc.GetColumnIdx("AR");
        if (attackRatingColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T arPerLvlColumnIdx = doc.GetColumnIdx("AR/Lvl");
        if (arPerLvlColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T damageMinColumnIdx = doc.GetColumnIdx("Dmg-Min");
        if (damageMinColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T damageMaxColumnIdx = doc.GetColumnIdx("Dmg-Max");
        if (damageMaxColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T dmgPerLvlColumnIdx = doc.GetColumnIdx("Dmg/Lvl");
        if (dmgPerLvlColumnIdx < 0)
        {
            return;
        }

        SSIZE_T resistColumnIdx = doc.GetColumnIdx("ResistFire"); // all resits are equal so only fetch one
        if (resistColumnIdx < 0)
        {
            resistColumnIdx = doc.GetColumnIdx("Resist"); // support the alternate version of this file format
            if (resistColumnIdx < 0)
            {
                return;
            }
        }

        SSIZE_T resistPerLvlColumnIdx = doc.GetColumnIdx("ResistFire/Lvl"); // all resits are equal so only fetch one
        if (resistPerLvlColumnIdx < 0)
        {
            resistPerLvlColumnIdx = doc.GetColumnIdx("Resist/Lvl"); // support the alternate version of this file format
            if (resistPerLvlColumnIdx < 0)
            {
                return;
            }
        }

        const SSIZE_T nameFirstColumnIdx = doc.GetColumnIdx("NameFirst");
        if (nameFirstColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T namelastColumnIdx = doc.GetColumnIdx("NameLast");
        if (namelastColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T hireDescColumnIdx = doc.GetColumnIdx("HireDesc");
        if (hireDescColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T equivalentcharclassColumnIdx = doc.GetColumnIdx("equivalentcharclass"); // allowed not to exist

        size_t skillsParamSize = 6;
        std::vector<SSIZE_T> skillIndex(skillsParamSize, -1);
        std::vector<SSIZE_T> skillLevel(skillsParamSize, -1);
        std::vector<SSIZE_T> skillLvlPerLvl(skillsParamSize, -1);
        for (size_t idx = 1; idx <= skillsParamSize; ++idx)
        {
            skillIndex[idx - 1] = doc.GetColumnIdx("Skill" + std::to_string(idx));
            if (skillIndex[idx - 1] < 0)
            {
                skillsParamSize = idx - 1;
                skillIndex.resize(skillsParamSize);
                skillLevel.resize(skillsParamSize);
                skillLvlPerLvl.resize(skillsParamSize);
                break;
            }

            skillLevel[idx - 1] = doc.GetColumnIdx("Level" + std::to_string(idx));
            if (skillLevel[idx - 1] < 0)
            {
                skillsParamSize = idx - 1;
                skillIndex.resize(skillsParamSize);
                skillLevel.resize(skillsParamSize);
                skillLvlPerLvl.resize(skillsParamSize);
                break;
            }

            skillLvlPerLvl[idx - 1] = doc.GetColumnIdx("LvlPerLvl" + std::to_string(idx));
            if (skillLvlPerLvl[idx - 1] < 0)
            {
                skillsParamSize = idx - 1;
                skillIndex.resize(skillsParamSize);
                skillLevel.resize(skillsParamSize);
                skillLvlPerLvl.resize(skillsParamSize);
                break;
            }
        }

        std::uint32_t version = 0;
        std::uint16_t id = 0;
        std::uint32_t level = 0;
        std::string strValue;
        std::string hireDesc;
        std::string hireDescDefault;
        std::string hireling;
        for (size_t i = 0; i < numRows; ++i)
        {
            strValue = doc.GetCellString(idColumnIdx, i);
            if (strValue.empty())
            {
                // should not happen
                continue;
            }
            id = doc.GetCellUInt16(idColumnIdx, i);

            strValue = doc.GetCellString(levelColumnIdx, i);
            if (strValue.empty())
            {
                continue;
            }
            level = doc.GetCellUInt32(levelColumnIdx, i);

            version = 0;
            if (versionColumnIdx >= 0)
            {
                version = doc.GetCellUInt32(versionColumnIdx, i);
            }

            auto& mercTypeInfoMap = mercTypeInfo[version];
            auto iterMerType = mercTypeInfoMap.find(id);
            if (iterMerType == mercTypeInfoMap.end())
            {
                std::uint16_t classId = 0;
                std::uint8_t mercClassIdx = 0;
                hireling = doc.GetCellString(hirelingColumnIdx, i);
                d2ce::EnumMercenaryClass mercClass = d2ce::EnumMercenaryClass::None;
                if (classColumnIdx >= 0)
                {
                    strValue = doc.GetCellString(classColumnIdx, i);
                    if (!strValue.empty())
                    {
                        classId = doc.GetCellUInt16(classColumnIdx, i);
                        auto iterClass = mercClassToEnumClass.find(classId);
                        if (iterClass != mercClassToEnumClass.end())
                        {
                            mercClass = iterClass->second;
                            mercClassIdx = static_cast<std::underlying_type_t<d2ce::EnumMercenaryClass>>(mercClass);
                        }
                    }
                }

                if (mercClass == d2ce::EnumMercenaryClass::None)
                {
                    for (auto mercClassName : mercStringNames)
                    {
                        if (hireling.compare(mercClassName) == 0)
                        {
                            mercClass = static_cast<d2ce::EnumMercenaryClass>(mercClassIdx);
                            switch (mercClass)
                            {
                            case d2ce::EnumMercenaryClass::RogueScout:
                                classId = 271ui16;
                                break;

                            case d2ce::EnumMercenaryClass::DesertMercenary:
                                classId = 338ui16;
                                break;

                            case d2ce::EnumMercenaryClass::IronWolf:
                                classId = 359ui16;
                                break;

                            case d2ce::EnumMercenaryClass::Barbarian:
                                classId = 561ui16;
                                break;
                            }
                            break;
                        }
                        ++mercClassIdx;
                    }

                    if (mercClass == d2ce::EnumMercenaryClass::None)
                    {
                        // should not happen
                        continue;
                    }
                }

                // load merc names
                if (mercNamesMap.find(mercClass) == mercNamesMap.end())
                {
                    std::string prefix;
                    std::uint16_t startIdx = 0;
                    if (ExtractPrefixAndNumber(doc.GetCellString(nameFirstColumnIdx, i), prefix, startIdx))
                    {
                        std::string endPrefix;
                        std::uint16_t endIdx = 0;
                        if (ExtractPrefixAndNumber(doc.GetCellString(namelastColumnIdx, i), endPrefix, endIdx) &&
                            (endPrefix == prefix) && (endIdx >= startIdx))
                        {
                            auto& mercNames = mercNamesMap[mercClass];
                            for (auto mercIdx = startIdx; mercIdx <= endIdx; ++mercIdx)
                            {
                                std::stringstream ss;
                                ss << prefix;
                                ss << std::setw(2) << std::setfill('0') << mercIdx;
                                LocalizationHelpers::GetStringTxtValue(ss.str(), strValue);
                                mercNames.push_back(strValue);
                            }
                        }
                    }

                    LocalizationHelpers::GetStringTxtValue(mercClassNameCode.at(mercClass), strValue, hireling.c_str());
                    if (mercClassNames.size() <= mercClassIdx)
                    {
                        mercClassNames.resize(size_t(mercClassIdx) + 1);
                    }

                    mercClassNames[mercClassIdx] = strValue;
                }

                std::uint8_t attributeId = 0;
                std::string attributeName;
                hireDescDefault.clear();
                hireDesc = doc.GetCellString(hireDescColumnIdx, i);
                if (!hireDesc.empty())
                {
                    // support values from the alternative files
                    if ((hireDesc == "farw") || (hireDesc == "strhirespecial1"))
                    {
                        hireDescDefault = "Fire Arrow";
                        hireDesc = "strhirespecial1";
                    }
                    else if ((hireDesc == "carw") || (hireDesc == "strhirespecial2"))
                    {
                        hireDescDefault = "Cold Arrow";
                        hireDesc = "strhirespecial2";
                    }
                    else if ((hireDesc == "comb") || (hireDesc == "skillname103") || (hireDesc == "skillname99"))
                    {
                        hireDescDefault = "Combat";
                        hireDesc = "StrMercEx15";
                    }
                    else if ((hireDesc == "def") || (hireDesc == "skillname114") || (hireDesc == "skillname104"))
                    {
                        hireDescDefault = "Defensive";
                        hireDesc = "StrMercEx12";
                    }
                    else if ((hireDesc == "off") || (hireDesc == "skillname98") || (hireDesc == "skillname108"))
                    {
                        hireDescDefault = "Offensive";
                        hireDesc = "StrMercEx14";
                    }
                    else if ((hireDesc == "fire") || (hireDesc == "strhirespecial7"))
                    {
                        hireDescDefault = "Fire";
                        hireDesc = "strhirespecial7";
                    }
                    else if ((hireDesc == "cold") || (hireDesc == "strhirespecial6"))
                    {
                        hireDescDefault = "Cold";
                        hireDesc = "strhirespecial6";
                    }
                    else if ((hireDesc == "ltng") || (hireDesc == "strhirespecial5"))
                    {
                        hireDescDefault = "Lightning";
                        hireDesc = "strhirespecial5";
                    }

                    LocalizationHelpers::GetStringTxtValue(hireDesc, attributeName, hireDesc.empty() ? nullptr : hireDesc.c_str());
                    if (!attributeName.empty())
                    {
                        auto& attribs = mercAttributesMap[mercClass];
                        auto iterAttrib = std::find(attribs.begin(), attribs.end(), attributeName);
                        if (iterAttrib == attribs.end())
                        {
                            attributeId = std::uint8_t(attribs.size());
                            attribs.push_back(attributeName);
                        }
                        else
                        {
                            attributeId = std::uint8_t(std::distance(std::begin(attribs), iterAttrib));
                        }
                    }
                }
                else if (mercClass == EnumMercenaryClass::Barbarian)
                {
                    // Bash or frenzy
                    switch (classId)
                    {
                    case 560:
                        hireDescDefault = "Frenzy";
                        hireDesc = "skillname147";
                        break;

                    case 561:
                        hireDescDefault = "Bash";
                        hireDesc = "skillname126";
                        break;
                    }

                    if (!hireDesc.empty())
                    {
                        LocalizationHelpers::GetStringTxtValue(hireDesc, attributeName, hireDesc.empty() ? nullptr : hireDesc.c_str());
                        if (!attributeName.empty())
                        {
                            auto& attribs = mercAttributesMap[mercClass];
                            auto iterAttrib = std::find(attribs.begin(), attribs.end(), attributeName);
                            if (iterAttrib == attribs.end())
                            {
                                attributeId = std::uint8_t(attribs.size());
                                attribs.push_back(attributeName);
                            }
                            else
                            {
                                attributeId = std::uint8_t(std::distance(std::begin(attribs), iterAttrib));
                            }
                        }
                    }
                }

                strValue = doc.GetCellString(difficultyColumnIdx, i);
                if (strValue.empty())
                {
                    // should not happen
                    continue;
                }

                d2ce::EnumDifficulty diff = d2ce::EnumDifficulty::Normal;
                switch (doc.GetCellUInt16(difficultyColumnIdx, i))
                {
                case 1:
                    diff = d2ce::EnumDifficulty::Normal;
                    break;
                case 2:
                    diff = d2ce::EnumDifficulty::Nightmare;
                    break;
                case 3:
                    diff = d2ce::EnumDifficulty::Hell;
                    break;
                default:
                    // should not happen
                    continue;
                }

                auto& mercType = mercTypeInfoMap[id];
                mercType.Version = version;
                mercType.Id = id;
                mercType.Class = mercClass;
                mercType.ClassName = mercClassNames[mercClassIdx];
                mercType.Difficulty = diff;
                mercType.AttributeId = attributeId;
                mercType.AttributeName = attributeName;

                if (equivalentcharclassColumnIdx >= 0)
                {
                    strValue = doc.GetCellString(equivalentcharclassColumnIdx, i);
                    if (strValue == "ama")
                    {
                        mercType.EquivClass = EnumCharClass::Amazon;
                    }
                    else if (strValue == "bar")
                    {
                        mercType.EquivClass = EnumCharClass::Barbarian;
                    }
                }

                mercTypes[version][mercType.Class][mercType.Difficulty][mercType.AttributeId].push_back(mercType.Id);
                if ((mercType.Class == EnumMercenaryClass::Barbarian) && (mercType.AttributeId == 0))
                {
                    legacyBarbarianTypes[mercType.Difficulty][mercType.AttributeId].push_back(mercType.Id);
                }
            }

            auto& mercType = mercTypeInfoMap[id];
            if (mercType.Levels.find(level) != mercType.Levels.end())
            {
                // duplicates should not happend
                continue;
            }

            auto& mercLevel = mercType.Levels[level];
            mercLevel.Level = level;

            strValue = doc.GetCellString(expPerLevelColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.ExpPerLevel = doc.GetCellUInt32(expPerLevelColumnIdx, i);
            }

            strValue = doc.GetCellString(lifeColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.Life = doc.GetCellUInt32(lifeColumnIdx, i);
            }

            strValue = doc.GetCellString(lifePerLvlColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.LifePerLvl = doc.GetCellUInt32(lifePerLvlColumnIdx, i);
            }

            strValue = doc.GetCellString(defenseColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.Defense = doc.GetCellUInt16(defenseColumnIdx, i);
            }

            strValue = doc.GetCellString(defPerLvlColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.DefPerLvl = doc.GetCellUInt16(defPerLvlColumnIdx, i);
            }

            strValue = doc.GetCellString(strengthColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.Strength = doc.GetCellUInt32(strengthColumnIdx, i);
            }

            strValue = doc.GetCellString(strPerLvlColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.StrPerLvl = doc.GetCellUInt32(strPerLvlColumnIdx, i);
            }

            strValue = doc.GetCellString(dexterityColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.Dexterity = doc.GetCellUInt32(dexterityColumnIdx, i);
            }

            strValue = doc.GetCellString(dexPerLvlColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.DexPerLvl = doc.GetCellUInt32(dexPerLvlColumnIdx, i);
            }

            strValue = doc.GetCellString(attackRatingColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.AttackRating = doc.GetCellUInt16(attackRatingColumnIdx, i);
            }

            strValue = doc.GetCellString(arPerLvlColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.ARPerLvl = doc.GetCellUInt16(arPerLvlColumnIdx, i);
            }

            strValue = doc.GetCellString(damageMinColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.Damage.Min = doc.GetCellUInt16(damageMinColumnIdx, i);
            }

            strValue = doc.GetCellString(damageMaxColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.Damage.Max = doc.GetCellUInt16(damageMaxColumnIdx, i);
            }

            strValue = doc.GetCellString(dmgPerLvlColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.DmgPerLvl = doc.GetCellUInt16(dmgPerLvlColumnIdx, i);
            }

            strValue = doc.GetCellString(resistColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.Resist = doc.GetCellUInt16(resistColumnIdx, i);
            }

            strValue = doc.GetCellString(resistPerLvlColumnIdx, i);
            if (!strValue.empty())
            {
                mercLevel.ResistPerLvl = doc.GetCellUInt16(resistPerLvlColumnIdx, i);
            }

            for (size_t idx = 0; idx < skillsParamSize; ++idx)
            {
                strValue = doc.GetCellString(skillIndex[idx], i);
                if (strValue.empty())
                {
                    break;
                }

                mercLevel.skills.resize(mercLevel.skills.size() + 1);
                auto& skill = mercLevel.skills.back();
                skill.index = strValue;
                const auto& skillInfo = CharClassHelper::getSkillByIndex(skill.index);
                skill.id = skillInfo.id;

                strValue = doc.GetCellString(skillLevel[idx], i);
                if (!strValue.empty())
                {
                    skill.Level = doc.GetCellUInt16(skillLevel[idx], i);
                }

                strValue = doc.GetCellString(skillLvlPerLvl[idx], i);
                if (!strValue.empty())
                {
                    skill.LvlPerLvl = doc.GetCellUInt16(skillLvlPerLvl[idx], i);
                }
            }
        }

        s_MercTypeInfo.swap(mercTypeInfo);
        s_MercTypes.swap(mercTypes);
        s_LegacyBarbarianTypes.swap(legacyBarbarianTypes);
        s_MercNamesMap.swap(mercNamesMap);
        s_MercClassNames.swap(mercClassNames);
        s_MercAttributesMap.swap(mercAttributesMap);
    }

    // Find Merc Difficulty mapped to Type that matches difficulty and class requirements
    const std::map<EnumDifficulty, std::map<std::uint8_t, std::vector<std::uint16_t>>>& getMercDifficultyMap(EnumCharVersion version, d2ce::EnumMercenaryClass mercClass)
    {
        static std::map<EnumDifficulty, std::map<std::uint8_t, std::vector<std::uint16_t>>> badValue;
        std::uint32_t fileVersion = 0;
        if (version >= EnumCharVersion::v110)
        {
            fileVersion = 100;
        }

        auto typeMapIter = s_MercTypes.find(fileVersion);
        if (typeMapIter == s_MercTypes.end())
        {
            // should not happend
            typeMapIter = s_MercTypes.begin();
            if (typeMapIter == s_MercTypes.end())
            {
                return badValue;
            }
        }

        auto iterClass = typeMapIter->second.find(mercClass);
        if (iterClass == typeMapIter->second.end())
        {
            // should not happend
            return badValue;
        }

        switch (mercClass)
        {
        case EnumMercenaryClass::Barbarian:
            if (version < EnumCharVersion::v120)
            {
                return s_LegacyBarbarianTypes;
            }
            break;
        }

        return iterClass->second;
    }

    // Find Merc Attributes mapped to Type that matches difficulty and class requirements
    const std::map<std::uint8_t, std::vector<std::uint16_t>>& getMercAttribMap(EnumCharVersion version, EnumDifficulty difficulty, d2ce::EnumMercenaryClass mercClass)
    {
        auto& diffMap = getMercDifficultyMap(version, mercClass);
        auto iterDiff = diffMap.find(difficulty);
        if (iterDiff == diffMap.end())
        {
            // should not happend
            static std::map<std::uint8_t, std::vector<std::uint16_t>> badValue;
            return badValue;
        }

        return iterDiff->second;
    }

    // Find first Merc Id that matches difficulty and class requirements
    std::uint16_t getMercId(EnumCharVersion version, EnumDifficulty difficulty, d2ce::EnumMercenaryClass mercClass)
    {
        const auto& attribMap = getMercAttribMap(version, difficulty, mercClass);
        auto iterAttrib = attribMap.begin();
        if (iterAttrib == attribMap.end())
        {
            // should not happend
            return 0;
        }

        auto iterId = iterAttrib->second.begin();
        if (iterId == iterAttrib->second.end())
        {
            // should not happend
            return 0;
        }

        return *iterId;
    }
    
    const MercTypeInfo& getMercTypeInfo(std::uint16_t id, EnumCharVersion version)
    {
        std::uint32_t fileVersion = 0;
        if (version >= EnumCharVersion::v110)
        {
            fileVersion = 100;
        }

        static MercTypeInfo badValue;
        auto typeMapIter = s_MercTypeInfo.find(fileVersion);
        if (typeMapIter == s_MercTypeInfo.end())
        {
            // should not happend
            typeMapIter = s_MercTypeInfo.begin();
            if (typeMapIter == s_MercTypeInfo.end())
            {
                return badValue;
            }
        }

        auto iter = typeMapIter->second.find(id);
        if (iter == typeMapIter->second.end())
        {
            // should not happend
            return badValue;
        }

        return iter->second;
    }

    const std::map<std::uint32_t, MercStatInfo>& getMercStatInfo(std::uint16_t id, EnumCharVersion version)
    {
        const auto& mercTypeInfo = getMercTypeInfo(id, version);
        return mercTypeInfo.Levels;
    }

    const MercStatInfo& getMercStatInfoForLevel(std::uint32_t level, const std::map<std::uint32_t, MercStatInfo>& mercStats)
    {
        static MercStatInfo badStatInfo;
        if (mercStats.size() == 0)
        {
            return badStatInfo;
        }

        auto rIter = mercStats.rbegin();
        if (rIter != mercStats.rend())
        {
            if (rIter->first <= level)
            {
                // last Merc level is lower then our level
                return rIter->second;
            }
        }

        auto iter = mercStats.begin();
        if (iter != mercStats.end())
        {
            if (iter->first >= level)
            {
                // first Merc Level is higher then our level
                return iter->second;
            }
        }

        // find appropriate Merc stat
        auto iter_end = mercStats.end();
        auto prevIter = iter;
        for (; iter != iter_end; prevIter = iter, ++iter)
        {
            if (iter->first > level)
            {
                // return previous value
                return prevIter->second;
            }
        }

        // should not happend as all cases covered above
        if (rIter != mercStats.rend())
        {
            return rIter->second;
        }
        return badStatInfo;
    }

    const MercStatInfo& getMercStatInfoForLevel(std::uint32_t level, std::uint16_t id, EnumCharVersion version)
    {
        return getMercStatInfoForLevel(level, getMercStatInfo(id, version));
    }

    std::uint32_t getMinLevelForStrength(std::uint32_t strength, const MercStatInfo& statInfo)
    {
        if ((strength <= statInfo.Strength) || (statInfo.StrPerLvl == 0))
        {
            return statInfo.Level;
        }
        return (strength - statInfo.Strength + (statInfo.StrPerLvl - 1)) / statInfo.StrPerLvl;
    }

    std::uint32_t getMinLevelForDexterity(std::uint32_t dexterity, const MercStatInfo& statInfo)
    {
        if ((dexterity <= statInfo.Dexterity) || (statInfo.DexPerLvl == 0))
        {
            return statInfo.Level;
        }

        return (dexterity - statInfo.Dexterity + (statInfo.DexPerLvl - 1)) / statInfo.DexPerLvl;
    }

    std::uint32_t getMercExpPerLevel(std::uint16_t id, EnumCharVersion version)
    {
        // all levels have the same ExprPerLevel values, so get the first one
        const auto& statInfo = getMercStatInfoForLevel(1, id, version);
        if (statInfo.ExpPerLevel == 0)
        {
            // should not happend
            return 100;
        }

        return statInfo.ExpPerLevel;
    }

    std::uint32_t getMercExprForLevel(std::uint32_t level, std::uint32_t expPerLevel)
    {
        // Mercenary's max level is 98
        level = std::max(std::uint32_t(1), std::min(level, d2ce::MERC_NUM_OF_LEVELS));
        return expPerLevel * (level + 1) * level * level;
    }

    std::uint32_t getMercExprForLevel(std::uint32_t level, std::uint16_t id, EnumCharVersion version)
    {
        return getMercExprForLevel(level, getMercExpPerLevel(id, version));
    }

    std::uint32_t getMercLevelForExpr(std::uint32_t experience, std::uint16_t id, EnumCharVersion version)
    {
        // Mercenary's max level is 98
        const auto& mercStats = getMercStatInfo(id, version);
        const auto& statInfo = getMercStatInfoForLevel(1, mercStats); // get bottom lowest level
        std::uint32_t level = statInfo.Level;
        auto expPerLevel = statInfo.ExpPerLevel;
        while (experience >= getMercExprForLevel(level + 1, expPerLevel) && level < d2ce::MERC_NUM_OF_LEVELS)
        {
            ++level;
        }

        return level;
    }
}

//---------------------------------------------------------------------------
d2ce::Mercenary::Mercenary(Character& charInfo) : CharInfo(charInfo)
{
}
//---------------------------------------------------------------------------
d2ce::Mercenary::~Mercenary()
{
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::readInfo()
{
    if (getVersion() < EnumCharVersion::v109)
    {
        clear();
        return false;
    }

    fillMercInfo(Merc);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::readInfo(const Json::Value& root, bool bSerializedFormat)
{
    if (getVersion() < EnumCharVersion::v109)
    {
        clear();
        return false;
    }

    Json::Value mercRoot = root[bSerializedFormat ? "Mercenary" : "header"];
    if (!mercRoot.isNull())
    {
        Json::Value jsonValue = mercRoot[bSerializedFormat ? "IsDead" : "dead_merc"];
        if (!jsonValue.isNull())
        {
            Merc.Dead = std::uint16_t(jsonValue.asInt64());
        }
        
        jsonValue = mercRoot[bSerializedFormat ? "Id" : "merc_id"];
        if (!jsonValue.isNull())
        {
            Merc.Id = bSerializedFormat ? std::uint32_t(jsonValue.asInt64()) : std::uint32_t(std::stoul(jsonValue.asString(), nullptr, 16));
        }

        jsonValue = mercRoot[bSerializedFormat ? "NameId" : "merc_name_id"];
        if (!jsonValue.isNull())
        {
            Merc.NameId = std::uint16_t(jsonValue.asInt64());
        }

        jsonValue = mercRoot[bSerializedFormat ? "TypeId" : "merc_type"];
        if (!jsonValue.isNull())
        {
            Merc.Type = std::uint16_t(jsonValue.asInt64());
        }

        jsonValue = mercRoot[bSerializedFormat ? "Experience" : "merc_experience"];
        if (!jsonValue.isNull())
        {
            Merc.Experience = std::uint32_t(jsonValue.asInt64());
        }
    }


    size_t current_byte_offset = CHAR_V109_MERC_DEAD_BYTE_OFFSET;
    if (!CharInfo.setBytes(current_byte_offset, sizeof(Merc.Dead), Merc.Dead)) // pos 177 (1.09+ only)
    {
        return false;
    }

    if (!CharInfo.setBytes(current_byte_offset, sizeof(Merc.Id), Merc.Id)) // pos 179 (1.09 + only)
    {
        return false;
    }

    if (!CharInfo.setBytes(current_byte_offset, sizeof(Merc.NameId), Merc.NameId)) // pos 183 (1.09+ only)
    {
        return false;
    }

    if (!CharInfo.setBytes(current_byte_offset, sizeof(Merc.Type), Merc.Type)) // pos 185 (1.09+ only)
    {
        return false;
    }

    if (!CharInfo.setBytes(current_byte_offset, sizeof(Merc.Experience), Merc.Experience)) // pos 187 (1.09+ only)
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setTxtReader()
{
    auto& txtReader = ItemHelpers::getTxtReader();
    InitMercData(txtReader);
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::clear()
{
    Merc.Dead = 0;
    Merc.Id = 0;
    Merc.NameId = 0;
    Merc.Type = 0;
    Merc.Experience = 0;

    CharInfo.updateBytes(CHAR_V109_MERC_DEAD_BYTE_OFFSET, sizeof(Merc.Dead), Merc.Dead); // pos 177 (1.09+ only)
    CharInfo.updateBytes(CHAR_V109_MERC_ID_BYTE_OFFSET, sizeof(Merc.Id), Merc.Id); // pos 179 (1.09 + only)
    CharInfo.updateBytes(CHAR_V109_MERC_NAME_BYTE_OFFSET, sizeof(Merc.NameId), Merc.NameId); // pos 183 (1.09+ only)
    CharInfo.updateBytes(CHAR_V109_MERC_TYPE_BYTE_OFFSET, sizeof(Merc.Type), Merc.Type); // pos 185 (1.09+ only)
    CharInfo.updateBytes(CHAR_V109_MERC_EXPERIENCE_BYTE_OFFSET, sizeof(Merc.Experience), Merc.Experience); // pos 187 (1.09+ only)
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::fillMercInfo(MercInfo& merc) const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        merc.Dead = 0;
        merc.Id = 0;
        merc.NameId = 0;
        merc.Type = 0;
        merc.Experience = 0;
        return;
    }

    merc.Dead = std::uint16_t(CharInfo.readBytes(CHAR_V109_MERC_DEAD_BYTE_OFFSET, sizeof(merc.Dead))); // pos 177 (1.09+ only)
    merc.Id = CharInfo.readBytes(CHAR_V109_MERC_ID_BYTE_OFFSET, sizeof(merc.Id)); // pos 179 (1.09 + only)
    merc.NameId = std::uint16_t(CharInfo.readBytes(CHAR_V109_MERC_NAME_BYTE_OFFSET, sizeof(merc.NameId))); // pos 183 (1.09+ only);
    merc.Type = std::uint16_t(CharInfo.readBytes(CHAR_V109_MERC_TYPE_BYTE_OFFSET, sizeof(merc.Type))); // pos 185 (1.09+ only)
    merc.Experience = CharInfo.readBytes(CHAR_V109_MERC_EXPERIENCE_BYTE_OFFSET, sizeof(merc.Experience)); // pos 187 (1.09+ only)
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::updateMercInfo(MercInfo& merc)
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return;
    }

    auto bHasItems = !getItems().empty();
    if (isHired() && (merc.Id == 0) && bHasItems)
    {
        // can't unhire if we have items
        return;
    }

    auto oldMercClass = getClass();
    auto oldMercId = Merc.Id == 0 ? merc.Id : Merc.Id;
    auto oldMercName = Merc.NameId;
    std::memcpy(&Merc, &merc, sizeof(MercInfo));

    if (!isHired())
    {
        clear();
        return;
    }

    if (bHasItems && (oldMercClass != getClass()))
    {
        CharInfo.updateBytes(CHAR_V109_MERC_DEAD_BYTE_OFFSET, sizeof(Merc.Dead), Merc.Dead); // pos 177 (1.09+ only)
        CharInfo.updateBytes(CHAR_V109_MERC_EXPERIENCE_BYTE_OFFSET, sizeof(Merc.Experience), Merc.Experience); // pos 187 (1.09+ only)

        // Can't change type if we have items
        Merc.NameId = oldMercName;
        CharInfo.updateBytes(CHAR_V109_MERC_NAME_BYTE_OFFSET, sizeof(Merc.NameId), Merc.NameId); // pos 183 (1.09+ only)

        Merc.Id = oldMercId;
        CharInfo.updateBytes(CHAR_V109_MERC_ID_BYTE_OFFSET, sizeof(Merc.Id), Merc.Id); // pos 179 (1.09 + only)

        setClass(oldMercClass);
    }
    else
    {
        CharInfo.updateBytes(CHAR_V109_MERC_DEAD_BYTE_OFFSET, sizeof(Merc.Dead), Merc.Dead); // pos 177 (1.09+ only)
        CharInfo.updateBytes(CHAR_V109_MERC_ID_BYTE_OFFSET, sizeof(Merc.Id), Merc.Id); // pos 179 (1.09 + only)
        CharInfo.updateBytes(CHAR_V109_MERC_NAME_BYTE_OFFSET, sizeof(Merc.NameId), Merc.NameId); // pos 183 (1.09+ only)
        CharInfo.updateBytes(CHAR_V109_MERC_TYPE_BYTE_OFFSET, sizeof(Merc.Type), Merc.Type); // pos 185 (1.09+ only)
    }

    // Check NameId
    setNameId(Merc.NameId);

    // check experience
    setExperience(Merc.Experience);
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::fillMercStats(CharStats& cs) const
{
    cs.clear();
    if (!isHired())
    {
        return;
    }

    cs.Level = getLevel();
    cs.Experience = Merc.Experience;

    const auto& statInfo = getMercStatInfoForLevel(cs.Level, Merc.Type, getVersion());
    cs.Strength = statInfo.Strength;
    cs.Dexterity = statInfo.Dexterity;
    cs.CurLife = statInfo.Life;
    cs.MaxLife = cs.CurLife;
    if (cs.Level > statInfo.Level )
    {
        auto levelDiff = cs.Level - statInfo.Level;
        cs.Strength += (levelDiff * statInfo.StrPerLvl) / 8;
        cs.Dexterity += (levelDiff * statInfo.DexPerLvl) / 8;
        cs.CurLife += (levelDiff * statInfo.LifePerLvl);
        cs.MaxLife = cs.CurLife;
    }

    // Apply item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getItemBonuses(magicalAttributes))
    {
        ItemHelpers::applyNonMaxMagicalAttributes(cs, magicalAttributes);
        cs.MaxLife = cs.CurLife;
        ItemHelpers::applyMaxMagicalAttributes(cs, magicalAttributes);
        cs.CurLife = cs.MaxLife;
        cs.Energy = 0;
        cs.Vitality = 0;
        cs.MaxMana = 0;
        cs.MaxStamina = 0;
    }
}
//---------------------------------------------------------------------------
d2ce::EnumCharVersion d2ce::Mercenary::getVersion() const
{
    return CharInfo.getVersion();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getLevel() const
{
    if (!isHired())
    {
        return 1;
    }

    // Mercenary's max level is 98
    return getMercLevelForExpr(Merc.Experience, Merc.Type, getVersion());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getMinLevel() const
{
    if (!isHired())
    {
        return 1;
    }

    const auto& statInfo = getMercStatInfoForLevel(1, Merc.Type, getVersion());
    ItemRequirements req;
    std::uint32_t level = statInfo.Level;
    for (const auto& item : getItems())
    {
        item.getDisplayedRequirements(req, CharInfo.getLevel());
        if (req.Level != 0)
        {
            level = std::max(level, std::uint32_t(req.Level));
        }

        if (req.Strength != 0)
        {
            level = std::max(level, getMinLevelForStrength(req.Strength, statInfo));
        }

        if (req.Dexterity != 0)
        {
            level = std::max(level, getMinLevelForDexterity(req.Dexterity, statInfo));
        }
    }

    // Mercenary's max level is 98
    return std::min(level, d2ce::MERC_NUM_OF_LEVELS);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getMaxLevel() const
{
    if (!isHired())
    {
        return 1;
    }

    return std::min(CharInfo.getLevel(), d2ce::MERC_NUM_OF_LEVELS);
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setLevel(std::uint32_t level)
{
    if (!isHired())
    {
        return;
    }

    // Make sure we can still use our items, is not at a higher level then our character and is at least our base level
    level = std::max(std::min(level, getMaxLevel()), getMinLevel());
    if (getLevel() == level)
    {
        return;
    }

    Merc.Experience = getMercExprForLevel(level, Merc.Type, getVersion());
    CharInfo.updateBytes(CHAR_V109_MERC_EXPERIENCE_BYTE_OFFSET, sizeof(Merc.Experience), Merc.Experience); // pos 187 (1.09+ only)
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getExperience() const
{
    if (!isHired())
    {
        return 0;
    }

    return Merc.Experience;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getMinExperience() const
{
    if (!isHired())
    {
        return 0;
    }

    return getMercExprForLevel(getMinLevel(), Merc.Type, getVersion());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getMaxExperience() const
{
    if (!isHired())
    {
        return 0;
    }

    // The max experience is the max for the current character's level
    return getMercExprForLevel(std::min(CharInfo.getLevel() + 1, CharInfo.getMaxLevel()), Merc.Type, getVersion()) - 1;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setExperience(std::uint32_t experience)
{
    if (!isHired())
    {
        return;
    }

    // Make sure we can still use our items, is not at a higher level then our character and is at least our base level
    Merc.Experience = std::max(std::min(experience, getMaxExperience()), getMinExperience());
    CharInfo.updateBytes(CHAR_V109_MERC_EXPERIENCE_BYTE_OFFSET, sizeof(Merc.Experience), Merc.Experience); // pos 187 (1.09+ only)
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getStrength() const
{
    CharStats cs;
    fillMercStats(cs);
    return cs.Strength;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Mercenary::getDexterity() const
{
    CharStats cs;
    fillMercStats(cs);
    return cs.Dexterity;
}
//---------------------------------------------------------------------------
d2ce::EnumMercenaryClass d2ce::Mercenary::getClass() const
{
    if (!isHired())
    {
        return EnumMercenaryClass::None;
    }

    const auto& mercType = getMercTypeInfo(Merc.Type, getVersion());
    return mercType.Class;
}
//---------------------------------------------------------------------------
const std::string& d2ce::Mercenary::getClassName() const
{
    if (!isHired())
    {
        static std::string badValue;
        return badValue;
    }

    const auto& mercType = getMercTypeInfo(Merc.Type, getVersion());
    return mercType.ClassName;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setClass(EnumMercenaryClass mercClass)
{
    // Can only change type if we carry no items
    if (!isHired() || (getClass() == mercClass) || !getItems().empty())
    {
        return;
    }

    d2ce::MercInfo oldMerc;
    fillMercInfo(oldMerc);
    auto oldDifficulty = getDifficulty();

    std::uint16_t maxMercId = 0;
    if (mercClass != EnumMercenaryClass::None)
    {
        const auto& mercNames = s_MercNamesMap[mercClass];
        if (!mercNames.empty())
        {
            maxMercId = std::uint16_t(mercNames.size() - 1);
        }
    }
    
    if (mercClass == EnumMercenaryClass::None)
    {
        clear();
    }
    else
    {
        Merc.Type = getMercId(getVersion(), oldDifficulty, mercClass);
        CharInfo.updateBytes(CHAR_V109_MERC_TYPE_BYTE_OFFSET, sizeof(Merc.Type), Merc.Type); // pos 185 (1.09+ only)

        Merc.NameId = std::min(Merc.NameId, maxMercId);
        CharInfo.updateBytes(CHAR_V109_MERC_NAME_BYTE_OFFSET, sizeof(Merc.NameId), Merc.NameId); // pos 183 (1.09+ only)
    }

    // restore difficulty level
    setDifficulty(oldDifficulty);

    // ensure name is valid
    setNameId(Merc.NameId);

    if (getLevel() > CharInfo.getLevel())
    {
        // The new class has a level higher then our character which is not allowed
        updateMercInfo(oldMerc);
    }
}
//---------------------------------------------------------------------------
d2ce::EnumDifficulty d2ce::Mercenary::getDifficulty() const
{
    if (!isHired())
    {
        return EnumDifficulty::Normal;
    }

    const auto& mercType = getMercTypeInfo(Merc.Type, getVersion());
    return mercType.Difficulty;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setDifficulty(EnumDifficulty difficulty)
{
    if (!isHired())
    {
        return;
    }

    d2ce::MercInfo oldMerc;
    fillMercInfo(oldMerc);
    auto oldDifficulty = getDifficulty();
    if (oldDifficulty == difficulty)
    {
        return;
    }

    auto attributeId = getAttributeId();
    const auto& attribMap = getMercAttribMap(getVersion(), difficulty, getClass());
    auto iter = attribMap.find(attributeId);
    if ((iter == attribMap.end()) || iter->second.empty())
    {
        // should not happen
        return;
    }

    auto& idList = iter->second;
    auto iterType = idList.begin();
    if (idList.size() > 1)
    {
        const auto& oldAttribMap = getMercAttribMap(getVersion(), oldDifficulty, getClass());
        auto iterOld = oldAttribMap.find(attributeId);
        if (iterOld != oldAttribMap.end() && (iterOld->second.size() > 1))
        {
            auto& idListOld = iterOld->second;
            auto iterTypeOld = std::find(idListOld.begin(), idListOld.end(), oldMerc.Type);
            if (iterTypeOld != idListOld.end())
            {
                auto typeIdx = std::distance(idListOld.begin(), iterTypeOld);
                if ((ssize_t)idList.size() > typeIdx)
                {
                    std::advance(iterType, typeIdx);
                    if (iterType == idList.end())
                    {
                        iterType = idList.begin();
                    }
                }
            }
        }
    }

    Merc.Type = *iterType;
    CharInfo.updateBytes(CHAR_V109_MERC_TYPE_BYTE_OFFSET, sizeof(Merc.Type), Merc.Type); // pos 185 (1.09+ only)

    // check experience
    setExperience(Merc.Experience);

    // Make sure our level still makes sense
    if ((difficulty > oldDifficulty) && (getLevel() > CharInfo.getLevel()))
    {
        // this difficuly's level is too high, go back original difficulty
        updateMercInfo(oldMerc);
    }
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Mercenary::getAttributeId() const
{
    if (!isHired())
    {
        return 0;
    }

    const auto& mercType = getMercTypeInfo(Merc.Type, getVersion());
    return mercType.AttributeId;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setAttributeId(std::uint8_t attributeId)
{
    if (!isHired())
    {
        return;
    }

    d2ce::MercInfo oldMerc;
    fillMercInfo(oldMerc);
    auto oldAttributeId = getAttributeId();
    const auto& attribMap = getMercAttribMap(getVersion(), getDifficulty(), getClass());
    auto maxAttrib = attribMap.empty() ? 0ui8 : std::uint8_t(attribMap.size() - 1);
    attributeId = std::min(attributeId, maxAttrib);
    if (oldAttributeId == attributeId)
    {
        return;
    }

    auto iter = attribMap.find(attributeId);
    if ((iter == attribMap.end()) || iter->second.empty())
    {
        // should not happen
        return;
    }

    auto& idList = iter->second;
    auto iterType = idList.begin();
    if (idList.size() > 1)
    {
        auto iter2 = attribMap.find(oldAttributeId);
        if (iter2 != attribMap.end() && (iter2->second.size() > 1))
        {
            auto& idListOld = iter2->second;
            auto iterTypeOld = std::find(idListOld.begin(), idListOld.end(), oldMerc.Type);
            if (iterTypeOld != idListOld.end())
            {
                auto typeIdx = std::distance(idListOld.begin(), iterTypeOld);
                if ((ssize_t)idList.size() > typeIdx)
                {
                    std::advance(iterType, typeIdx);
                    if (iterType == idList.end())
                    {
                        iterType = idList.begin();
                    }
                }
            }
        }
    }

    Merc.Type = *iterType;
    CharInfo.updateBytes(CHAR_V109_MERC_TYPE_BYTE_OFFSET, sizeof(Merc.Type), Merc.Type); // pos 185 (1.09+ only)

    // check experience
    setExperience(Merc.Experience);

    if (getLevel() > CharInfo.getLevel())
    {
        // The new attribute type has a level higher then our character which is not allowed
        updateMercInfo(oldMerc);
    }
}
//---------------------------------------------------------------------------
std::string d2ce::Mercenary::getAttributeName() const
{
    if (!isHired())
    {
        return "";
    }

    auto attributeId = getAttributeId();

    const auto& attribs = Mercenary::getMercAttributes(getClass(), getVersion());
    if (attributeId >= attribs.size())
    {
        return "";
    }

    return attribs[attributeId];
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Mercenary::getNameId() const
{
    if (!isHired())
    {
        return 0;
    }

    return Merc.NameId;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setNameId(std::uint16_t nameId)
{
    if (!isHired())
    {
        return;
    }

    auto mercClass = getClass();
    std::uint16_t maxMercId = 0;
    if (mercClass != EnumMercenaryClass::None)
    {
        const auto& mercNames = s_MercNamesMap[mercClass];
        if (!mercNames.empty())
        {
            maxMercId = std::uint16_t(mercNames.size() - 1);
        }
    }

    Merc.NameId = std::min(nameId, maxMercId);
    CharInfo.updateBytes(CHAR_V109_MERC_NAME_BYTE_OFFSET, sizeof(Merc.NameId), Merc.NameId); // pos 183 (1.09+ only)
}
//---------------------------------------------------------------------------
std::string d2ce::Mercenary::getName() const
{
    if (!isHired())
    {
        return "";
    }

    auto mercClass = getClass();
    if (mercClass == EnumMercenaryClass::None)
    {
        // should not happen
        return "";
    }

    const auto& mercNames = s_MercNamesMap[mercClass];
    if (Merc.NameId < mercNames.size())
    {
        return mercNames[Merc.NameId];
    }

    return "";
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::getDamage(BaseDamage& damage) const
{
    damage.clear();
    if (!isHired())
    {
        return;
    }

    auto level = getLevel();

    const auto& statInfo = getMercStatInfoForLevel(level, Merc.Type, getVersion());
    std::uint16_t levelDiff = std::uint16_t(level - statInfo.Level);
    std::uint16_t dmgDiff = std::uint16_t((levelDiff * statInfo.DmgPerLvl) / 8);
    damage.Min = statInfo.Damage.Min + dmgDiff;
    damage.Max = statInfo.Damage.Max + dmgDiff;

    // Apply item bonuses
    BaseDamage bonus;
    if (CharInfo.getCombinedMercDamage(bonus))
    {
        damage.Min += bonus.Min;
        damage.Max += bonus.Max;
    }

    if (damage.Min > damage.Max)
    {
        damage.Max = damage.Min + 1;
    }
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Mercenary::getDefenseRating() const
{
    if (!isHired())
    {
        return 0;
    }

    auto level = getLevel();

    const auto& statInfo = getMercStatInfoForLevel(level, Merc.Type, getVersion());
    std::uint16_t levelDiff = std::uint16_t(level - statInfo.Level);
    std::uint16_t defenseRating = statInfo.Defense + levelDiff * statInfo.DefPerLvl;

    // Apply item bonuses
    return defenseRating + CharInfo.getCombinedMercDefenseRating();
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Mercenary::getAttackRating() const
{
    if (!isHired())
    {
        return 0;
    }

    auto level = getLevel();

    const auto& statInfo = getMercStatInfoForLevel(level, Merc.Type, getVersion());
    std::uint16_t levelDiff = std::uint16_t(level - statInfo.Level);
    std::uint16_t attackRating = statInfo.AttackRating + levelDiff * statInfo.ARPerLvl;

    // Apply item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getItemBonuses(magicalAttributes))
    {
        std::uint64_t eAr = 0;
        std::uint64_t ar = 0;
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib);
            if (stat.name == "tohit")
            {
                ar += std::uint32_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else if (stat.name == "item_tohit_percent")
            {
                eAr += std::uint32_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else
            {
                for (const auto& opStat : stat.opAttribs.op_stats)
                {
                    if (opStat == "tohit")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 2:
                            ar += std::uint32_t(Items::getMagicalAttributeValue(attrib, level, 0));
                            break;
                        }
                    }
                    else if (opStat == "item_tohit_percent")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 2:
                            eAr += std::uint32_t(Items::getMagicalAttributeValue(attrib, level, 0));
                            break;
                        }
                    }
                }
            }

            attackRating += std::uint16_t((attackRating * eAr) / 100 + ar);
        }
    }

    return attackRating;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::getResistance(BaseResistance& resist) const
{
    resist.clear();
    if (!isHired())
    {
        return;
    }

    auto level = getLevel();

    const auto& statInfo = getMercStatInfoForLevel(level, Merc.Type, getVersion());
    std::int16_t levelDiff = std::int16_t(level - statInfo.Level);
    std::int16_t baseResist = std::int16_t(statInfo.Resist + (levelDiff * statInfo.ResistPerLvl) / 4);
    resist.Cold = baseResist;
    resist.Fire = resist.Cold;
    resist.Lightning = resist.Cold;
    resist.Poison = resist.Cold;

    std::int16_t maxColdResist = 75;
    std::int16_t maxFireResist = 75;
    std::int16_t maxLightningResist = 75;
    std::int16_t maxPoisonResist = 75;

    // Apply item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getItemBonuses(magicalAttributes))
    {
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib);
            if (stat.name == "fireresist")
            {
                resist.Fire += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else if (stat.name == "maxfireresist")
            {
                maxFireResist += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else if (stat.name == "lightresist")
            {
                resist.Lightning += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else if (stat.name == "maxlightresist")
            {
                maxLightningResist += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else if (stat.name == "coldresist")
            {
                resist.Cold += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else if (stat.name == "maxcoldresist")
            {
                maxColdResist += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else if (stat.name == "poisonresist")
            {
                resist.Poison += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else if (stat.name == "maxpoisonresist")
            {
                maxPoisonResist += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
            }
            else
            {
                for (const auto& opStat : stat.opAttribs.op_stats)
                {
                    if (opStat == "fireresist")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 2:
                            resist.Fire += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
                            break;
                        }
                    }
                    else if (opStat == "lightresist")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 2:
                            resist.Lightning += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
                            break;
                        }
                    }
                    else if (opStat == "coldresist")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 2:
                            resist.Cold += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
                            break;
                        }
                    }
                    else if (opStat == "poisonresist")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 2:
                            resist.Poison += std::int16_t(Items::getMagicalAttributeValue(attrib, level, 0));
                            break;
                        }
                    }
                }
            }
        }
    }

    // Apply current difficulty penalty
    switch (CharInfo.getDifficultyLastPlayed())
    {
    case EnumDifficulty::Nightmare:
        resist.Cold -= 40;
        resist.Fire -= 40;
        resist.Lightning -= 40;
        resist.Poison -= 40;
        break;

    case EnumDifficulty::Hell:
        resist.Cold -= 100;
        resist.Fire -= 100;
        resist.Lightning -= 100;
        resist.Poison -= 100;
        break;
    }

    resist.Cold = std::min(std::max(std::int16_t(-100), resist.Cold), maxColdResist);
    resist.Fire = std::min(std::max(std::int16_t(-100), resist.Fire), maxFireResist);
    resist.Lightning = std::min(std::max(std::int16_t(-100), resist.Lightning), maxLightningResist);
    resist.Poison = std::min(std::max(std::int16_t(-100), resist.Poison), maxPoisonResist);

}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::isHired() const
{
    return Merc.Id != 0 ? true : false;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setIsHired(bool bIsHired)
{
    if ((isHired() == bIsHired) || (getVersion() < EnumCharVersion::v109))
    {
        return;
    }

    if (!bIsHired)
    {
        clear();
        return;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<std::uint32_t> spread(0, MAXUINT32);
    Merc.Id = std::uint32_t(spread(gen) | 0x10); // new id
    CharInfo.updateBytes(CHAR_V109_MERC_ID_BYTE_OFFSET, sizeof(Merc.Id), Merc.Id); // pos 179 (1.09 + only)

    // check experience
    setExperience(Merc.Experience);
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::isDead() const
{
    if (!isHired())
    {
        return false;
    }

    return Merc.Dead != 0 ? true : false;
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::setIsDead(bool bIsDead)
{
    if (!isHired())
    {
        return;
    }

    Merc.Dead = bIsDead ? 1 : 0;
    CharInfo.updateBytes(CHAR_V109_MERC_DEAD_BYTE_OFFSET, sizeof(Merc.Dead), Merc.Dead); // pos 177 (1.09+ only)
}
//---------------------------------------------------------------------------
const std::optional<d2ce::EnumCharClass>& d2ce::Mercenary::getEquivClass() const
{
    auto version = getVersion();
    if (version < EnumCharVersion::v120)
    {
        static std::optional<EnumCharClass> badValue;
        return badValue;
    }

    return getMercTypeInfo(Merc.Type, version).EquivClass;
}
//---------------------------------------------------------------------------
size_t d2ce::Mercenary::getNumberOfItems() const
{
    return CharInfo.getMercItems().size();
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& d2ce::Mercenary::getItems() const
{
    return CharInfo.getMercItems();
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::canEquipItem(const d2ce::Item& item, EnumEquippedId equipId) const
{
    equipId = verifyEquippedId(item, equipId);
    if (equipId == d2ce::EnumEquippedId::NONE)
    {
        return false;
    }

    auto& equivClass = getEquivClass();
    if (equivClass.has_value())
    {
        if (!item.canEquip(equipId, equivClass.value()))
        {
            return false;
        }
    }
    else
    {
        if (item.isClassSpecific())
        {
            return false;
        }

        if (!item.canEquip(equipId))
        {
            return false;
        }
    }

    if (item.isQuestItem())
    {
        return false;
    }

    ItemRequirements itemReq;
    if (item.getRequirements(itemReq))
    {
        CharStats cs;
        fillMercStats(cs);
        if (itemReq.Level > cs.Level)
        {
            return false;
        }

        if (itemReq.Strength > cs.Strength)
        {
            return false;
        }

        if (itemReq.Dexterity > cs.Dexterity)
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
d2ce::EnumEquippedId d2ce::Mercenary::verifyEquippedId(const d2ce::Item& item, EnumEquippedId equipId) const
{
    const auto& itemHelper = item.getItemTypeHelper();
    auto mercClass = getClass();
    switch (mercClass)
    {
    case EnumMercenaryClass::RogueScout:
        switch (equipId)
        {
        case d2ce::EnumEquippedId::HEAD:
        case d2ce::EnumEquippedId::TORSO:
            break;

        case d2ce::EnumEquippedId::RIGHT_ARM:
        case d2ce::EnumEquippedId::LEFT_ARM:
            if (!itemHelper.hasCategoryCode("bow"))
            {
                return d2ce::EnumEquippedId::NONE;
            }
            equipId = EnumEquippedId::RIGHT_ARM;
            break;

        default:
            return d2ce::EnumEquippedId::NONE;
        }
        break;

    case EnumMercenaryClass::DesertMercenary:
        switch (equipId)
        {
        case d2ce::EnumEquippedId::HEAD:
        case d2ce::EnumEquippedId::TORSO:
            break;

        case d2ce::EnumEquippedId::RIGHT_ARM:
        case d2ce::EnumEquippedId::LEFT_ARM:
            if (!itemHelper.hasCategoryCode("sppl"))
            {
                return d2ce::EnumEquippedId::NONE;
            }
            equipId = EnumEquippedId::RIGHT_ARM;
            break;

        default:
            return d2ce::EnumEquippedId::NONE;
        }
        break;

    case EnumMercenaryClass::IronWolf:
        switch (equipId)
        {
        case d2ce::EnumEquippedId::HEAD:
        case d2ce::EnumEquippedId::TORSO:
            break;

        case d2ce::EnumEquippedId::RIGHT_ARM:
            if (!itemHelper.hasCategoryCode("swor"))
            {
                return d2ce::EnumEquippedId::NONE;
            }
            break;

        case d2ce::EnumEquippedId::LEFT_ARM:
            if (!itemHelper.hasCategoryCode("shie"))
            {
                return d2ce::EnumEquippedId::NONE;
            }
            break;

        default:
            return d2ce::EnumEquippedId::NONE;
        }
        break;

    case EnumMercenaryClass::Barbarian:
        switch (equipId)
        {
        case d2ce::EnumEquippedId::HEAD:
        case d2ce::EnumEquippedId::TORSO:
            break;

        case d2ce::EnumEquippedId::RIGHT_ARM:
            if (!itemHelper.hasCategoryCode("swor"))
            {
                return d2ce::EnumEquippedId::NONE;
            }

            if (getAttributeId() != 0)
            {
                if (itemHelper.isTwoHandedWeapon())
                {
                    // make sure other hand is not holding a sword
                    for (const auto& equipItem : getItems())
                    {
                        if (equipItem.getEquippedId() == EnumEquippedId::LEFT_ARM)
                        {
                            return d2ce::EnumEquippedId::NONE;
                        }
                    }
                }
            }
            break;

        case d2ce::EnumEquippedId::LEFT_ARM:
            if (!itemHelper.hasCategoryCode("swor"))
            {
                return d2ce::EnumEquippedId::NONE;
            }

            if (getAttributeId() == 0)
            {
                equipId = EnumEquippedId::RIGHT_ARM;
            }
            else
            {
                // make sure we are not holding two swords
                bool hasRight = false;
                bool hasLeft = false;
                for (const auto& equipItem : getItems())
                {
                    switch (equipItem.getEquippedId())
                    {
                    case EnumEquippedId::LEFT_ARM:
                        hasLeft = true;
                        break;

                    case EnumEquippedId::RIGHT_ARM:
                        hasRight = true;
                        break;
                    }
                }

                if (itemHelper.isTwoHandedWeapon())
                {
                    if (hasRight && hasLeft)
                    {
                        return d2ce::EnumEquippedId::NONE;
                    }
                    equipId = EnumEquippedId::RIGHT_ARM;
                }
                else if (!hasRight)
                {
                    equipId = EnumEquippedId::RIGHT_ARM;
                }
            }
            break;

        default:
            return d2ce::EnumEquippedId::NONE;
        }
        break;

    default:
        // invalid state
        return d2ce::EnumEquippedId::NONE;
    }

    return equipId;
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::getItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    return CharInfo.getMercItemBonuses(attribs);
}
//---------------------------------------------------------------------------
bool d2ce::Mercenary::getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    return CharInfo.getDisplayedMercItemBonuses(attribs);
}
//---------------------------------------------------------------------------
void d2ce::Mercenary::asJson(Json::Value& parent, bool bSerializedFormat) const
{
    if (bSerializedFormat)
    {
        Json::Value mercenary;
        mercenary["IsDead"] = Merc.Dead;
        mercenary["Id"] = Merc.Id;
        mercenary["NameId"] = Merc.NameId;
        if (Merc.Id != 0)
        {
            mercenary["Name"] = getName();
        }

        mercenary["TypeId"] = Merc.Type;
        if (Merc.Id != 0)
        {
            mercenary["Class"] = getClassName();
            auto attribName = getAttributeName();
            if (!attribName.empty())
            {
                parent["Attribute"] = attribName;
            }
            mercenary["Level"] = getLevel();
        }

        mercenary["Experience"] = Merc.Experience;
        if (Merc.Id != 0)
        {
            switch (getDifficulty())
            {
            case EnumDifficulty::Normal:
                mercenary["Difficulty"] = "Normal";
                break;
            case EnumDifficulty::Nightmare:
                mercenary["Difficulty"] = "Nightmare";
                break;
            case EnumDifficulty::Hell:
                mercenary["Difficulty"] = "Hell";
                break;
            }
        }
        parent["Mercenary"] = mercenary;
    }
    else
    {
        parent["dead_merc"] = Merc.Dead;
        {
            std::stringstream ss;
            ss << std::hex << Merc.Id;
            parent["merc_id"] = ss.str();
        }
        parent["merc_name_id"] = Merc.NameId;
        if (Merc.Id != 0)
        {
            parent["merc_name"] = getName();
        }

        parent["merc_type"] = Merc.Type;
        if (Merc.Id != 0)
        {
            parent["merc_class"] = getClassName();
            auto attribName = getAttributeName();
            if (!attribName.empty())
            {
                parent["merc_attribute"] = attribName;
            }
            parent["merc_level"] = getLevel();
        }

        parent["merc_experience"] = Merc.Experience;
        if (Merc.Id != 0)
        {
            switch (getDifficulty())
            {
            case EnumDifficulty::Normal:
                parent["merc_difficulty"] = "Normal";
                break;
            case EnumDifficulty::Nightmare:
                parent["merc_difficulty"] = "Nightmare";
                break;
            case EnumDifficulty::Hell:
                parent["merc_difficulty"] = "Hell";
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------
const std::vector<std::string>& d2ce::Mercenary::getMercNames(EnumMercenaryClass mercClass)
{
    if (mercClass == EnumMercenaryClass::None)
    {
        static std::vector<std::string> badValue;
        return badValue;
    }

    return s_MercNamesMap[mercClass];
}
//---------------------------------------------------------------------------
const std::vector<std::string>& d2ce::Mercenary::getMercClassNames()
{
    return s_MercClassNames;
}
//---------------------------------------------------------------------------
const std::vector<std::string>& d2ce::Mercenary::getMercAttributes(EnumMercenaryClass mercClass, EnumCharVersion version)
{
    static std::vector<std::string> emptyValue;
    switch (mercClass)
    {
    case EnumMercenaryClass::None:
        return emptyValue;

    case EnumMercenaryClass::Barbarian:
        if (version < EnumCharVersion::v120)
        {
            return emptyValue;
        }
        break;
    }

    return s_MercAttributesMap[mercClass];
}
//---------------------------------------------------------------------------
const std::string& d2ce::Mercenary::getMercClassName(EnumMercenaryClass mercClass)
{
    auto idx = static_cast<std::underlying_type_t<d2ce::EnumMercenaryClass>>(mercClass);
    if (idx >= s_MercClassNames.size())
    {
        static std::string badValue;
        return badValue;
    }

    return s_MercClassNames[idx];
}
//---------------------------------------------------------------------------