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
#include "CharacterStats.h"
#include "Character.h"
#include "SkillConstants.h"
#include "ItemConstants.h"
#include <helpers/ItemHelpers.h>

//---------------------------------------------------------------------------
namespace d2ce
{

    constexpr std::array<std::uint8_t, 2> STATS_MARKER = { 0x67, 0x66 };              // alternatively "gf"
    constexpr std::uint16_t STAT_MAX = 16;
    constexpr std::uint16_t STAT_END_MARKER = 0x1FF;
    constexpr size_t STAT_BITS = 9;
    constexpr std::array<size_t, 16> V110_BITS_PER_STAT = { 10,10,10,10,10,8,21,21,21,21,21,21,7,32,25,25 };

    constexpr std::uint32_t MIN_START_STATS_POS = 765;
    constexpr std::uint32_t MIN_START_STATS_POS_v100 = 560;

    constexpr std::array<std::uint8_t, 2> SKILLS_MARKER = { 0x69, 0x66 };             // alternatively "if"

    namespace ItemHelpers
    {
        void initRunewordData();
    }

    std::map<d2ce::EnumCharClass, std::vector<std::uint32_t>> s_MinExpRequired;
    void InitExperienceData(const ITxtReader& txtReader)
    {
        static const ITxtReader* pCurTextReader = nullptr;
        if (!s_MinExpRequired.empty())
        {
            if (pCurTextReader == &txtReader)
            {
                // already initialized
                return;
            }

            s_MinExpRequired.clear();
        }

        pCurTextReader = &txtReader;
        auto pDoc(txtReader.GetExperienceTxt());
        auto& doc = *pDoc;
        std::map<d2ce::EnumCharClass, std::vector<std::uint32_t>> minExpRequired;
        size_t numRows = doc.GetRowCount();
        const SSIZE_T levelColumnIdx = doc.GetColumnIdx("Level");
        if (levelColumnIdx < 0)
        {
            return;
        }

        std::map<d2ce::EnumCharClass, SSIZE_T> classColumnIndex;
        classColumnIndex[d2ce::EnumCharClass::Amazon] = doc.GetColumnIdx("Amazon");
        if (classColumnIndex[d2ce::EnumCharClass::Amazon] < 0)
        {
            return;
        }

        classColumnIndex[d2ce::EnumCharClass::Sorceress] = doc.GetColumnIdx("Sorceress");
        if (classColumnIndex[d2ce::EnumCharClass::Sorceress] < 0)
        {
            return;
        }

        classColumnIndex[d2ce::EnumCharClass::Necromancer] = doc.GetColumnIdx("Necromancer");
        if (classColumnIndex[d2ce::EnumCharClass::Necromancer] < 0)
        {
            return;
        }

        classColumnIndex[d2ce::EnumCharClass::Paladin] = doc.GetColumnIdx("Paladin");
        if (classColumnIndex[d2ce::EnumCharClass::Paladin] < 0)
        {
            return;
        }

        classColumnIndex[d2ce::EnumCharClass::Barbarian] = doc.GetColumnIdx("Barbarian");
        if (classColumnIndex[d2ce::EnumCharClass::Barbarian] < 0)
        {
            return;
        }

        classColumnIndex[d2ce::EnumCharClass::Druid] = doc.GetColumnIdx("Druid");
        if (classColumnIndex[d2ce::EnumCharClass::Druid] < 0)
        {
            return;
        }

        classColumnIndex[d2ce::EnumCharClass::Assassin] = doc.GetColumnIdx("Assassin");
        if (classColumnIndex[d2ce::EnumCharClass::Assassin] < 0)
        {
            return;
        }

        std::string strValue;
        std::uint32_t exp = 0;
        std::uint32_t level = 0;
        for (size_t i = 0; i < numRows; ++i)
        {
            strValue = doc.GetCellString(levelColumnIdx, i);
            if (strValue.empty())
            {
                // skip
                continue;
            }

            if (strValue == "MaxLvl")
            {
                // first row is the MaxLvl row, so initialize vector to fit size
                for (auto colIdx : classColumnIndex)
                {
                    level = d2ce::MAX_NUM_LEVELS;
                    strValue = doc.GetCellString(colIdx.second, i);
                    if (!strValue.empty())
                    {
                        level = std::min(doc.GetCellUInt32(colIdx.second, i), 127ui32);
                    }

                    minExpRequired[colIdx.first].resize(level + 1, 0ui32);
                }
                continue;
            }

            for (auto colIdx : classColumnIndex)
            {
                exp = 0;
                strValue = doc.GetCellString(colIdx.second, i);
                if (!strValue.empty())
                {
                    exp = doc.GetCellUInt32(colIdx.second, i);
                }

                auto& expReqList = minExpRequired[colIdx.first];
                if (expReqList.size() >= i)
                {
                    expReqList[i - 1] = exp;
                }
            }
        }

        s_MinExpRequired.swap(minExpRequired);
    }

    struct CharacterInfoType
    {
        std::string ClassIndex; // The character class this line refers to (this is just a reference field, you can't actually change this).
        std::string ClassName;  // The localized name for the class
        d2ce::EnumCharClass ClassEnum = d2ce::EnumCharClass::Amazon;
        std::string Code;       // code used to reference this class in other files
        std::uint32_t Version = 0;

        std::uint32_t Strength = 0;   // The amount of strength this character class will start with.
        std::uint32_t Dexterity = 0;  // The amount of dexterity this character class will start with.
        std::uint32_t Energy = 0;     // The amount of energy this character class will start with.
        std::uint32_t Vitality = 0;   // The amount of vitality this character class will start with.
        std::uint32_t TotalStats = 0; // The total amount of the above starting stats
        std::uint32_t Stamina = 0;    // The amount of stamina this character class will start with.

        std::uint32_t HpAdd = 0;           // The amount of life added to the amount of life granted by the vit column.
        std::uint32_t LifePerLevel = 0;    // Amount of life earned for each level up. This value is in fourths, thus the lowest bonus possible is 64/256 (on quarter of one on-screen point, the fractional value is used by the game however).
        std::uint32_t StaminaPerLevel = 0; // Amount of stamina earned for each level up. This value is in fourths, thus the lowest bonus possible is 64/256 (on quarter of one on-screen point, the fractional value is used by the game however).
        std::uint32_t ManaPerLevel = 0;    // Amount of mana earned for each level up. This value is in fourths, thus the lowest bonus possible is 64/256 (on quarter of one on-screen point, the fractional value is used by the game however).
        std::uint32_t StatPerLevel = 0;    // Amount of stat points earned at each level up.

        std::uint32_t LifePerVitality = 0;    // Amount of life earned for each point invested in vitality. This value is in fourths, thus the lowest bonus possible is 64/256 (on quarter of one on-screen point, the fractional value is used by the game however).
        std::uint32_t StaminaPerVitality = 0; // Amount of stamina earned for each point invested in vitality. This value is in fourths, thus the lowest bonus possible is 64/256 (on quarter of one on-screen point, the fractional value is used by the game however).

        std::uint32_t ManaPerMagic = 0; // Amount of mana earned for each point invested in energy. This value is in fourths, thus the lowest bonus possible is 64/256 (on quarter of one on-screen point, the fractional value is used by the game however).

        std::string StrAllSkills; // This field tells the game what string to display for the bonus to all class skills (ex: +1 to all Amazon skills).
        std::string StrSkillTab1; // This field tells the game what string to display for the bonus to all skills of the first skill tab (ex: +1 to all Bow and Crossbow skills).
        std::string StrSkillTab2; // This field tells the game what string to display for the bonus to all skills of the second skill tab (ex: +1 to all Passive and Magic skills).
        std::string StrSkillTab3; // This field tells the game what string to display for the bonus to all skills of the third skill tab (ex: +1 to all Javelin and Spear skills).
        std::string StrClassOnly; // This field tells the game what string to display for class specific items (and class specific skill bonus) (ex: Amazon Only).

        std::string StrSklTreeTab1; // The string to display for the first skill tab
        std::string StrSklTreeTab2; // the string to display for the second skill tab
        std::string StrSklTreeTab3; // the string to display for the third skill tab

        std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>> SklTreeTab1; // The skill ids for the first skill tab, organized by row and column
        std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>> SklTreeTab2; // The skill ids for the second skill tab, organized by row and column
        std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>> SklTreeTab3; // The skill ids for the third skill tab, organized by row and column

        std::vector<std::uint16_t> Skills; // skill ids in original array position as found in character file
    };

    std::map<std::string, d2ce::EnumCharClass> s_CharClassEnumNameMap = { {"Amazon", d2ce::EnumCharClass::Amazon},
        {"Sorceress", d2ce::EnumCharClass::Sorceress}, {"Necromancer", d2ce::EnumCharClass::Necromancer},
        {"Paladin", d2ce::EnumCharClass::Paladin}, {"Barbarian", d2ce::EnumCharClass::Barbarian},
        {"Druid", d2ce::EnumCharClass::Druid}, {"Assassin", d2ce::EnumCharClass::Assassin} };
    std::map<std::string, std::uint16_t> s_CharClassNameMap;
    std::map<d2ce::EnumCharClass, std::uint16_t> s_CharClassEnumMap;
    std::map<std::uint16_t, CharacterInfoType> s_CharClassInfo;
    void InitCharStatsData(const ITxtReader& txtReader)
    {
        static const ITxtReader* pCurTextReader = nullptr;
        if (!s_CharClassInfo.empty())
        {
            if (pCurTextReader == &txtReader)
            {
                // already initialized
                return;
            }

            s_CharClassNameMap.clear();
            s_CharClassEnumMap.clear();
            s_CharClassInfo.clear();
        }

        InitExperienceData(txtReader);
        pCurTextReader = &txtReader;
        auto pDoc(txtReader.GetCharStatsTxt());
        auto& doc = *pDoc;
        std::map<std::string, std::uint16_t> charClassNameMap;
        std::map<d2ce::EnumCharClass, std::uint16_t> charClassEnumMap;
        std::map<std::uint16_t, CharacterInfoType> charClassInfo;
        size_t numRows = doc.GetRowCount();
        const SSIZE_T classColumnIdx = doc.GetColumnIdx("class");
        if (classColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strColumnIdx = doc.GetColumnIdx("str");
        if (strColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T dexColumnIdx = doc.GetColumnIdx("dex");
        if (dexColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T intColumnIdx = doc.GetColumnIdx("int");
        if (intColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T vitColumnIdx = doc.GetColumnIdx("vit");
        if (vitColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T staminaColumnIdx = doc.GetColumnIdx("stamina");
        if (staminaColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T hpaddColumnIdx = doc.GetColumnIdx("hpadd");
        if (hpaddColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T lifePerLevelColumnIdx = doc.GetColumnIdx("LifePerLevel");
        if (lifePerLevelColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T staminaPerLevelColumnIdx = doc.GetColumnIdx("StaminaPerLevel");
        if (staminaPerLevelColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T manaPerLevelColumnIdx = doc.GetColumnIdx("ManaPerLevel");
        if (manaPerLevelColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T lifePerVitalityColumnIdx = doc.GetColumnIdx("LifePerVitality");
        if (lifePerVitalityColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T staminaPerVitalityColumnIdx = doc.GetColumnIdx("StaminaPerVitality");
        if (staminaPerVitalityColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T manaPerMagicColumnIdx = doc.GetColumnIdx("ManaPerMagic");
        if (manaPerMagicColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T statPerLevelColumnIdx = doc.GetColumnIdx("StatPerLevel");
        if (statPerLevelColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strAllSkillsColumnIdx = doc.GetColumnIdx("StrAllSkills");
        if (strAllSkillsColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strSkillTab1ColumnIdx = doc.GetColumnIdx("StrSkillTab1");
        if (strSkillTab1ColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strSkillTab2ColumnIdx = doc.GetColumnIdx("StrSkillTab2");
        if (strSkillTab2ColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strSkillTab3ColumnIdx = doc.GetColumnIdx("StrSkillTab3");
        if (strSkillTab3ColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strClassOnlyColumnIdx = doc.GetColumnIdx("StrClassOnly");
        if (strClassOnlyColumnIdx < 0)
        {
            return;
        }

        std::uint16_t version = 0;
        std::string strValue;
        std::string index;
        std::uint16_t idx = 0;
        for (size_t i = 0; i < numRows; ++i)
        {
            index = doc.GetCellString(classColumnIdx, i);
            if (index.empty())
            {
                // skip
                continue;
            }

            if (index == "Expansion")
            {
                // skip
                version = 100;
                continue;
            }

            auto iter = s_CharClassEnumNameMap.find(index);
            if (iter != s_CharClassEnumNameMap.end())
            {
                charClassEnumMap[iter->second] = idx;
            }

            LocalizationHelpers::GetStringTxtValue(index, strValue);
            charClassNameMap[strValue] = idx;
            auto& item = charClassInfo[idx];
            ++idx;
            item.ClassIndex = index;
            item.Version = version;
            item.ClassEnum = iter->second;
            item.ClassName = strValue;

            strValue = doc.GetCellString(strColumnIdx, i);
            if (!strValue.empty())
            {
                item.Strength = doc.GetCellUInt32(strColumnIdx, i);
            }

            strValue = doc.GetCellString(dexColumnIdx, i);
            if (!strValue.empty())
            {
                item.Dexterity = doc.GetCellUInt32(dexColumnIdx, i);
            }

            strValue = doc.GetCellString(intColumnIdx, i);
            if (!strValue.empty())
            {
                item.Energy = doc.GetCellUInt32(intColumnIdx, i);
            }

            strValue = doc.GetCellString(vitColumnIdx, i);
            if (!strValue.empty())
            {
                item.Vitality = doc.GetCellUInt32(vitColumnIdx, i);
            }
            item.TotalStats = item.Strength + item.Dexterity + item.Energy + item.Vitality;

            strValue = doc.GetCellString(staminaColumnIdx, i);
            if (!strValue.empty())
            {
                item.Stamina = doc.GetCellUInt32(staminaColumnIdx, i);
            }

            strValue = doc.GetCellString(hpaddColumnIdx, i);
            if (!strValue.empty())
            {
                item.HpAdd = doc.GetCellUInt32(hpaddColumnIdx, i);
            }

            strValue = doc.GetCellString(lifePerLevelColumnIdx, i);
            if (!strValue.empty())
            {
                item.LifePerLevel = doc.GetCellUInt32(lifePerLevelColumnIdx, i);
            }

            strValue = doc.GetCellString(staminaPerLevelColumnIdx, i);
            if (!strValue.empty())
            {
                item.StaminaPerLevel = doc.GetCellUInt32(staminaPerLevelColumnIdx, i);
            }

            strValue = doc.GetCellString(manaPerLevelColumnIdx, i);
            if (!strValue.empty())
            {
                item.ManaPerLevel = doc.GetCellUInt32(manaPerLevelColumnIdx, i);
            }

            strValue = doc.GetCellString(lifePerVitalityColumnIdx, i);
            if (!strValue.empty())
            {
                item.LifePerVitality = doc.GetCellUInt32(lifePerVitalityColumnIdx, i);
            }

            strValue = doc.GetCellString(staminaPerVitalityColumnIdx, i);
            if (!strValue.empty())
            {
                item.StaminaPerVitality = doc.GetCellUInt32(staminaPerVitalityColumnIdx, i);
            }

            strValue = doc.GetCellString(manaPerMagicColumnIdx, i);
            if (!strValue.empty())
            {
                item.ManaPerMagic = doc.GetCellUInt32(manaPerMagicColumnIdx, i);
            }

            strValue = doc.GetCellString(statPerLevelColumnIdx, i);
            if (!strValue.empty())
            {
                item.StatPerLevel = doc.GetCellUInt32(statPerLevelColumnIdx, i);
            }

            strValue = doc.GetCellString(strAllSkillsColumnIdx, i);
            if (!LocalizationHelpers::GetStringTxtValue(strValue, item.StrAllSkills))
            {
                std::stringstream ss;
                ss << "%+d to ";
                ss << item.ClassIndex;
                ss << " Skill Levels";
                item.StrAllSkills = ss.str();
            }
            
            std::string strFind = "%+d";
            auto strPos = item.StrAllSkills.find(strFind);
            if (strPos != item.StrAllSkills.npos)
            {
                item.StrAllSkills.replace(strPos, strFind.size(), "+{1}");
            }
            else
            {
                // support classic txt files
                item.StrAllSkills = "+{1} " + item.StrAllSkills;
            }

            strValue = doc.GetCellString(strSkillTab1ColumnIdx, i);
            LocalizationHelpers::GetStringTxtValue(strValue, item.StrSkillTab1);
            strFind = "%+d";
            strPos = item.StrSkillTab1.find(strFind);
            if (strPos != item.StrSkillTab1.npos)
            {
                item.StrSkillTab1.replace(strPos, strFind.size(), "{2}");
            }
            else
            {
                // support classic txt files
                strFind = "%d";
                if (strPos != item.StrSkillTab1.npos)
                {
                    item.StrSkillTab1.replace(strPos, strFind.size(), "{2}");
                }
            }

            strValue = doc.GetCellString(strSkillTab2ColumnIdx, i);
            LocalizationHelpers::GetStringTxtValue(strValue, item.StrSkillTab2);
            strFind = "%+d";
            strPos = item.StrSkillTab2.find(strFind);
            if (strPos != item.StrSkillTab2.npos)
            {
                item.StrSkillTab2.replace(strPos, strFind.size(), "{2}");
            }
            else
            {
                // support classic txt files
                strFind = "%d";
                if (strPos != item.StrSkillTab2.npos)
                {
                    item.StrSkillTab2.replace(strPos, strFind.size(), "{2}");
                }
            }

            strValue = doc.GetCellString(strSkillTab3ColumnIdx, i);
            LocalizationHelpers::GetStringTxtValue(strValue, item.StrSkillTab3);
            strFind = "%+d";
            strPos = item.StrSkillTab3.find(strFind);
            if (strPos != item.StrSkillTab3.npos)
            {
                item.StrSkillTab3.replace(strPos, strFind.size(), "{2}");
            }
            else
            {
                // support classic txt files
                strFind = "%d";
                if (strPos != item.StrSkillTab2.npos)
                {
                    item.StrSkillTab2.replace(strPos, strFind.size(), "{2}");
                }
            }

            strValue = doc.GetCellString(strClassOnlyColumnIdx, i);
            if (!LocalizationHelpers::GetStringTxtValue(strValue, item.StrClassOnly))
            {
                std::stringstream ss;
                ss << "(";
                ss << item.ClassIndex;
                ss << " Only)";
                item.StrClassOnly = ss.str();
            }

            switch (item.ClassEnum)
            {
            case EnumCharClass::Amazon:
                LocalizationHelpers::GetStringTxtValue("SkillCategoryAm1", item.StrSklTreeTab1, "Javelin and Spear");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryAm2", item.StrSklTreeTab2, "Passive and Magic");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryAm3", item.StrSklTreeTab3, "Bow and Crossbow");
                break;

            case EnumCharClass::Sorceress:
                LocalizationHelpers::GetStringTxtValue("SkillCategorySo1", item.StrSklTreeTab1, "Cold Spells");
                LocalizationHelpers::GetStringTxtValue("SkillCategorySo2", item.StrSklTreeTab2, "Lightning Spells");
                LocalizationHelpers::GetStringTxtValue("SkillCategorySo3", item.StrSklTreeTab3, "Fire Spells");
                break;

            case EnumCharClass::Necromancer:
                LocalizationHelpers::GetStringTxtValue("SkillCategoryNe1", item.StrSklTreeTab1, "Summoning");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryNe2", item.StrSklTreeTab2, "Poison and Bone");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryNe3", item.StrSklTreeTab3, "Curses");
                break;

            case EnumCharClass::Paladin:
                LocalizationHelpers::GetStringTxtValue("SkillCategoryPa1", item.StrSklTreeTab1, "Defensive Auras");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryPa2", item.StrSklTreeTab2, "Offensive Auras");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryPa3", item.StrSklTreeTab3, "Combat Skills");
                break;

            case EnumCharClass::Barbarian:
                LocalizationHelpers::GetStringTxtValue("SkillCategoryBa1", item.StrSklTreeTab1, "Warcries");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryBa2", item.StrSklTreeTab2, "Combat Masteries");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryBa3", item.StrSklTreeTab3, "Combat Skills");
                break;

            case EnumCharClass::Druid:
                LocalizationHelpers::GetStringTxtValue("SkillCategoryDr1", item.StrSklTreeTab1, "Elemental");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryDr2", item.StrSklTreeTab2, "Shape Shifting");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryDr3", item.StrSklTreeTab3, "Summoning");
                break;

            case EnumCharClass::Assassin:
                LocalizationHelpers::GetStringTxtValue("SkillCategoryAs1", item.StrSklTreeTab1, "Martial Arts");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryAs2", item.StrSklTreeTab2, "Shadow Disciplines");
                LocalizationHelpers::GetStringTxtValue("SkillCategoryAs3", item.StrSklTreeTab3, "Traps");
                break;
            }
        }

        s_CharClassNameMap.swap(charClassNameMap);
        s_CharClassEnumMap.swap(charClassEnumMap);
        s_CharClassInfo.swap(charClassInfo);
    }

    std::map<std::string, d2ce::EnumCharClass> s_CharClassCodeMap;
    void InitPlayerClassData(const ITxtReader& txtReader)
    {
        static const ITxtReader* pCurTextReader = nullptr;
        if (!s_CharClassInfo.empty())
        {
            if (pCurTextReader == &txtReader)
            {
                // already initialized
                return;
            }

            s_CharClassNameMap.clear();
            s_CharClassEnumMap.clear();
            s_CharClassInfo.clear();
            s_CharClassCodeMap.clear();
        }

        InitCharStatsData(txtReader);
        pCurTextReader = &txtReader;
        auto pDoc(txtReader.GetPlayerClassTxt());
        auto& doc = *pDoc;
        std::map<std::string, d2ce::EnumCharClass> charClassCodeMap;
        size_t numRows = doc.GetRowCount();
        const SSIZE_T classColumnIdx = doc.GetColumnIdx("Player Class");
        if (classColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T codeColumnIdx = doc.GetColumnIdx("Code");
        if (codeColumnIdx < 0)
        {
            return;
        }

        std::string strValue;
        std::string code;
        for (size_t i = 0; i < numRows; ++i)
        {
            code = doc.GetCellString(codeColumnIdx, i);
            if (code.empty())
            {
                // skip
                continue;
            }

            strValue = doc.GetCellString(classColumnIdx, i);
            if (strValue.empty())
            {
                // skip
                continue;
            }

            if (strValue == "Expansion")
            {
                // skip
                continue;
            }

            auto iterEnum = s_CharClassEnumNameMap.find(strValue);
            if (iterEnum == s_CharClassEnumNameMap.end())
            {
                // skip
                continue;
            }

            auto iterIdx = s_CharClassEnumMap.find(iterEnum->second);
            if (iterIdx == s_CharClassEnumMap.end())
            {
                // skip
                continue;
            }

            auto iter = s_CharClassInfo.find(iterIdx->second);
            if (iter == s_CharClassInfo.end())
            {
                // skip
                continue;
            }

            iter->second.Code = code;
            charClassCodeMap[code] = iter->second.ClassEnum;
        }

        s_CharClassCodeMap.swap(charClassCodeMap);
    }

    const CharacterInfoType& GetCharClassInfo(std::int16_t idx)
    {
        auto iter = s_CharClassInfo.find(idx);
        if (iter == s_CharClassInfo.end())
        {
            static CharacterInfoType badValue;
            return badValue;
        }

        return iter->second;
    }

    EnumCharClass GetEnumCharClassByIndex(std::int16_t idx)
    {
        auto classInfo = GetCharClassInfo(idx);
        return classInfo.ClassEnum;
    }

    const CharacterInfoType& GetCharClassInfo(EnumCharClass charClass)
    {
        auto iter = s_CharClassEnumMap.find(charClass);
        if (iter == s_CharClassEnumMap.end())
        {
            static CharacterInfoType badValue;
            return badValue;
        }

        return GetCharClassInfo(iter->second);
    }

    const CharacterInfoType& GetCharClassInfo(const std::string& charCode)
    {
        auto iter = s_CharClassCodeMap.find(charCode);
        if (iter == s_CharClassCodeMap.end())
        {
            static CharacterInfoType badValue;
            return badValue;
        }

        return GetCharClassInfo(iter->second);
    }

    struct SkillsInfoType
    {
        SkillType Skill;

        std::string Desc;
        std::uint16_t Tab = 0;
        std::uint16_t Row = 0;
        std::uint16_t Col = 0;
    };

    std::map<std::string, std::vector<std::uint16_t>> s_SkillDescMap;
    std::map<std::string, std::uint16_t> s_SkillIndexMap;
    std::map<std::uint16_t, SkillsInfoType> s_SkillInfoMap;
    void InitSkillDescData(const ITxtReader& txtReader)
    {
        auto pDoc(txtReader.GetSkillDescTxt());
        auto& doc = *pDoc;
        size_t numRows = doc.GetRowCount();
        const SSIZE_T skilldescColumnIdx = doc.GetColumnIdx("skilldesc");
        if (skilldescColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T skillPageColumnIdx = doc.GetColumnIdx("SkillPage");
        if (skillPageColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T skillRowColumnIdx = doc.GetColumnIdx("SkillRow");
        if (skillRowColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T skillColumnColumnIdx = doc.GetColumnIdx("SkillColumn");
        if (skillColumnColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strnameColumnColumnIdx = doc.GetColumnIdx("str name");
        if (strnameColumnColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T strlongColumnColumnIdx = doc.GetColumnIdx("str long");
        if (strlongColumnColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T iconCelColumnIdx = doc.GetColumnIdx("IconCel");
        if (iconCelColumnIdx < 0)
        {
            return;
        }

        std::string strValue;
        auto iter = s_SkillDescMap.begin();
        for (size_t i = 0; i < numRows; ++i)
        {
            strValue = doc.GetCellString(skilldescColumnIdx, i);
            if (strValue.empty())
            {
                // skip
                continue;
            }

            iter = s_SkillDescMap.find(strValue);
            if (iter == s_SkillDescMap.end())
            {
                // skip
                continue;
            }

            for (const auto& id : iter->second)
            {
                auto& skillInfo = s_SkillInfoMap[id];

                strValue = doc.GetCellString(strnameColumnColumnIdx, i);
                if (!strValue.empty())
                {
                    LocalizationHelpers::GetStringTxtValue(strValue, skillInfo.Skill.name, skillInfo.Skill.index.c_str());
                }

                strValue = doc.GetCellString(strlongColumnColumnIdx, i);
                if (!strValue.empty())
                {
                    LocalizationHelpers::GetStringTxtValue(strValue, skillInfo.Skill.longName);
                }

                if (skillInfo.Skill.classInfo.has_value())
                {
                    strValue = doc.GetCellString(iconCelColumnIdx, i);
                    if (!strValue.empty())
                    {
                        skillInfo.Skill.classInfo.value().iconIndex = doc.GetCellUInt16(iconCelColumnIdx, i);
                    }

                    strValue = doc.GetCellString(skillPageColumnIdx, i);
                    if (!strValue.empty())
                    {
                        skillInfo.Tab = doc.GetCellUInt16(skillPageColumnIdx, i);
                    }

                    strValue = doc.GetCellString(skillRowColumnIdx, i);
                    if (!strValue.empty())
                    {
                        skillInfo.Row = doc.GetCellUInt16(skillRowColumnIdx, i);
                    }

                    strValue = doc.GetCellString(skillColumnColumnIdx, i);
                    if (!strValue.empty())
                    {
                        skillInfo.Col = doc.GetCellUInt16(skillColumnColumnIdx, i);
                    }

                    if ((skillInfo.Tab > 0) && (skillInfo.Tab <= 3) && (skillInfo.Row > 0) && (skillInfo.Col > 0))
                    {
                        auto iterIdx = s_CharClassEnumMap.find(skillInfo.Skill.classInfo.value().charClass);
                        if (iterIdx != s_CharClassEnumMap.end())
                        {
                            auto iterClass = s_CharClassInfo.find(iterIdx->second);
                            if (iterClass != s_CharClassInfo.end())
                            {
                                switch (skillInfo.Tab)
                                {
                                case 1:
                                    skillInfo.Tab = 3;
                                    iterClass->second.SklTreeTab3[skillInfo.Row][skillInfo.Col] = skillInfo.Skill.id;
                                    break;

                                case 2:
                                    iterClass->second.SklTreeTab2[skillInfo.Row][skillInfo.Col] = skillInfo.Skill.id;
                                    break;

                                case 3:
                                    skillInfo.Tab = 1;
                                    iterClass->second.SklTreeTab1[skillInfo.Row][skillInfo.Col] = skillInfo.Skill.id;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void InitSkillInfoData(const ITxtReader& txtReader)
    {
        static const ITxtReader* pCurTextReader = nullptr;
        if (!s_SkillInfoMap.empty())
        {
            if (pCurTextReader == &txtReader)
            {
                // already initialized
                return;
            }

            s_SkillDescMap.clear();
            s_SkillIndexMap.clear();
            s_SkillInfoMap.clear();
        }

        InitPlayerClassData(txtReader);
        pCurTextReader = &txtReader;
        auto pDoc(txtReader.GetSkillsTxt());
        auto& doc = *pDoc;
        std::map<std::string, std::vector<std::uint16_t>> skillDescMap;
        std::map<std::string, std::uint16_t> skillIndexMap;
        std::map<std::uint16_t, SkillsInfoType> skillInfoMap;
        size_t numRows = doc.GetRowCount();
        const SSIZE_T skillColumnIdx = doc.GetColumnIdx("skill");
        if (skillColumnIdx < 0)
        {
            return;
        }

        SSIZE_T idColumnIdx = doc.GetColumnIdx("*Id");
        if (idColumnIdx < 0)
        {
            idColumnIdx = doc.GetColumnIdx("Id"); // support the alternate version of this file format
            if (idColumnIdx < 0)
            {
                return;
            }
        }

        const SSIZE_T classColumnIdx = doc.GetColumnIdx("charclass");
        if (classColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T skilldescColumnIdx = doc.GetColumnIdx("skilldesc");
        if (skilldescColumnIdx < 0)
        {
            return;
        }

        const SSIZE_T reqlevelColumnColumnIdx = doc.GetColumnIdx("reqlevel");
        if (reqlevelColumnColumnIdx < 0)
        {
            return;
        }

        size_t reqSkillsParamSize = 3;
        std::vector<SSIZE_T> reqSkills(reqSkillsParamSize, -1);
        for (size_t idx = 1; idx <= reqSkillsParamSize; ++idx)
        {
            reqSkills[idx - 1] = doc.GetColumnIdx("reqskill" + std::to_string(idx));
            if (reqSkills[idx - 1] < 0)
            {
                reqSkillsParamSize = idx - 1;
                reqSkills.resize(reqSkillsParamSize);
                break;
            }
        }

        std::string strValue;
        std::string classCode;
        std::uint16_t id = MAXUINT16;
        for (size_t i = 0; i < numRows; ++i)
        {
            strValue = doc.GetCellString(idColumnIdx, i);
            if (strValue.empty())
            {
                // skip
                continue;
            }
            id = doc.GetCellUInt16(idColumnIdx, i);
            classCode = doc.GetCellString(classColumnIdx, i);
            strValue = doc.GetCellString(skillColumnIdx, i);
            if (strValue.empty())
            {
                // skip
                continue;
            }

            auto& skill = skillInfoMap[id];
            skill.Skill.id = id;
            skill.Skill.index = strValue;
            skillIndexMap[skill.Skill.index] = skill.Skill.id;
            skill.Skill.name = skill.Skill.index;
            skill.Desc = doc.GetCellString(skilldescColumnIdx, i);
            if (!skill.Desc.empty())
            {
                skillDescMap[skill.Desc].push_back(id);
            }

            if (!classCode.empty())
            {
                EnumCharClass value;
                if (d2ce::CharClassHelper::getEnumCharClassByCode(classCode, value))
                {
                    if (!skill.Skill.classInfo.has_value())
                    {
                        skill.Skill.classInfo = std::make_optional<ClassSkillType>();
                    }
                    skill.Skill.classInfo.value().charClass = value;

                    auto iterIdx = s_CharClassEnumMap.find(value);
                    if (iterIdx != s_CharClassEnumMap.end())
                    {
                        auto iter = s_CharClassInfo.find(iterIdx->second);
                        if (iter != s_CharClassInfo.end())
                        {
                            skill.Skill.classInfo.value().index = std::uint16_t(iter->second.Skills.size());
                            iter->second.Skills.push_back(id);
                        }
                    }
                }
            }

            strValue = doc.GetCellString(reqlevelColumnColumnIdx, i);
            if (!strValue.empty())
            {
                skill.Skill.reqLevel = doc.GetCellUInt16(reqlevelColumnColumnIdx, i);
            }

            for (const auto& reqSkill : reqSkills)
            {
                strValue = doc.GetCellString(reqSkill, i);
                if (strValue.empty())
                {
                    continue;
                }

                skill.Skill.reqSkills.push_back(strValue);
            }
        }

        s_SkillDescMap.swap(skillDescMap);
        s_SkillIndexMap.swap(skillIndexMap);
        s_SkillInfoMap.swap(skillInfoMap);

        InitSkillDescData(txtReader);
        ItemHelpers::initRunewordData();
    }

    const std::vector<std::uint32_t>& GetExperienceLevels(EnumCharClass charClass)
    {
        auto iter = s_MinExpRequired.find(charClass);
        if (iter == s_MinExpRequired.end())
        {
            static std::vector<std::uint32_t> badValue;
            return badValue;
        }

        return iter->second;
    }

    std::uint32_t GetMinExperienceLevel(std::uint32_t level, std::uint32_t maxLevel, EnumCharClass charClass)
    {
        std::uint32_t curLevel = std::min(level, maxLevel);
        if (curLevel == 0ui32)
        {
            return 0ui32;
        }

        const auto& minExpRequired = GetExperienceLevels(charClass);
        if (minExpRequired.empty())
        {
            return 0ui32;
        }

        if (minExpRequired.size() < curLevel)
        {
            return minExpRequired.back();
        }

        return minExpRequired[curLevel - 1];
    }

    std::uint32_t GetNextExperienceLevel(std::uint32_t level, std::uint32_t maxLevel, EnumCharClass charClass)
    {
        if (level == 0ui32)
        {
            return 0ui32;
        }

        if (level >= maxLevel)
        {
            return MAXUINT32;
        }

        const auto& minExpRequired = GetExperienceLevels(charClass);
        return minExpRequired[level];
    }

    std::uint32_t GetMaxGoldInBelt(std::uint32_t curLevel)
    {
        return curLevel * 10000;
    }

    std::uint32_t GetMaxGoldInStash(std::uint32_t curLevel, EnumCharVersion version = APP_CHAR_VERSION)
    {
        if (version >= EnumCharVersion::v110) // 1.10+ character
        {
            return d2ce::GOLD_IN_STASH_LIMIT;
        }

        if (curLevel < 31) // 1.00 - 1.09 characters
        {

            return (curLevel / 10 + 1) * 50000;
        }

        if (version < EnumCharVersion::v107) // 1.00 - 1.06 character
        {
            return (std::min(curLevel, 90ui32) / 10 + 1) * 50000;
        }

        return (curLevel / 2 + 1) * 50000; // 1.07 - 1.09 characters
    }
}
//---------------------------------------------------------------------------
d2ce::CharacterStats::CharacterStats(Character& charInfo) : CharInfo(charInfo)
{
    data.reserve(54); // reserve the maximum byte length to reduce allocations
}
//---------------------------------------------------------------------------
d2ce::CharacterStats::~CharacterStats()
{
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateMinStats()
{
    const auto& charClassInfo = GetCharClassInfo(CharInfo.getClass());
    std::uint32_t curLevel = std::min(std::max(Cs.Level, std::uint32_t(1)), max_levels);
    std::uint32_t curVitality = std::max(std::min(Cs.Vitality, MAX_BASICSTATS), charClassInfo.Vitality);
    std::uint32_t curEnergy = std::max(std::min(Cs.Energy, MAX_BASICSTATS), charClassInfo.Energy);
    std::uint32_t diffVitality = curVitality - charClassInfo.Vitality;
    std::uint32_t diffEnergy = curEnergy - charClassInfo.Energy;
    min_hit_points = ((charClassInfo.Vitality + charClassInfo.HpAdd) << 8) + (charClassInfo.LifePerLevel << 6) * (curLevel - 1) + (charClassInfo.LifePerVitality << 6) * diffVitality;
    min_stamina = (charClassInfo.Stamina << 8) + (charClassInfo.StaminaPerLevel << 6) * (curLevel - 1) + (charClassInfo.StaminaPerVitality << 6) * diffVitality;
    min_mana = (charClassInfo.Energy << 8) + (charClassInfo.ManaPerLevel << 6) * (curLevel - 1) + (charClassInfo.ManaPerMagic << 6) * diffEnergy;

    Cs.MinExperienceLevel = GetMinExperienceLevel(curLevel, max_levels, CharInfo.getClass());
    Cs.NextExperienceLevel = GetNextExperienceLevel(curLevel, max_levels, CharInfo.getClass());
    Cs.MaxGoldInBelt = GetMaxGoldInBelt(curLevel);
    Cs.MaxGoldInStash = GetMaxGoldInStash(curLevel, CharInfo.getVersion());
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateStartStats()
{
    const auto& minExpRequired = GetExperienceLevels(CharInfo.getClass());
    max_levels = minExpRequired.empty() ? MAX_NUM_LEVELS : std::uint32_t(minExpRequired.size() - 1);
    max_experience = minExpRequired.empty() ? MAX_EXPERIENCE : minExpRequired.back();
    Cs.MaxLevel = max_levels;
    Cs.MinExperienceLevel = 0ui32;
    Cs.NextExperienceLevel = 500ui32;
    Cs.MaxExperience = max_experience;
    Cs.MaxGoldInBelt = 10000ui32;
    Cs.MaxGoldInStash = (CharInfo.getVersion() >= EnumCharVersion::v110) ? d2ce::GOLD_IN_STASH_LIMIT : 50000ui32;

    const auto& charClassInfo = GetCharClassInfo(CharInfo.getClass());
    min_vitality = charClassInfo.Vitality;
    min_energy = charClassInfo.Energy;
    min_dexterity = charClassInfo.Dexterity;
    min_strength = charClassInfo.Strength;
    min_stat_points_used = charClassInfo.TotalStats;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateSkillChoices(std::uint16_t skillPointsEarned)
{
    if ((Cs.SkillChoices > 0) && areSkillsMaxed())
    {
        Cs.SkillChoices = 0;
    }
    else
    {
        std::uint32_t curLevel = std::min(std::max(Cs.Level, std::uint32_t(1)), max_levels);
        std::uint32_t earnedPoints = (curLevel - 1) + skillPointsEarned;
        std::uint32_t curPoints = getSkillPointsUsed();
        if ((curPoints + Cs.SkillChoices) < earnedPoints)
        {
            Cs.SkillChoices = std::min(earnedPoints - curPoints, MAX_SKILL_CHOICES);
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateLifePointsEarned(std::uint16_t lifePointsEarned)
{
    updateMinStats();

    // you get +20 Life for completing Act 3 Quest 1 (The Golden Bird) for each difficulty level
    min_hit_points += lifePointsEarned << 8;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updatePointsEarned(std::uint16_t lifePointsEarned, std::uint16_t statPointEarned, std::uint16_t skillPointsEarned)
{
    auto old_min_hit_points = min_hit_points;
    auto old_min_stamina = min_stamina;
    updateLifePointsEarned(lifePointsEarned);
    if (min_hit_points != old_min_hit_points && Cs.MaxLife < min_hit_points)
    {
        bool updateCur = Cs.CurLife == Cs.MaxLife ? true : false;
        Cs.MaxLife = min_hit_points;
        if (updateCur)
        {
            Cs.CurLife = Cs.MaxLife;
        }
    }

    if (min_stamina != old_min_stamina && Cs.MaxStamina < min_stamina)
    {
        bool updateCur = Cs.CurStamina == Cs.MaxStamina ? true : false;
        Cs.MaxStamina = min_stamina;
        if (updateCur)
        {
            Cs.CurStamina = Cs.MaxStamina;
        }
    }

    std::uint32_t curStatPointsUsed = getStatPointsUsed() - min_stat_points_used;
    std::uint32_t curLevel = std::min(std::max(Cs.Level, std::uint32_t(1)), max_levels);
    std::uint32_t earnedStatPoints = (curLevel - 1) * getStatPointsPerLevel() + statPointEarned;
    if ((curStatPointsUsed + Cs.StatsLeft) < earnedStatPoints)
    {
        Cs.StatsLeft = std::min(earnedStatPoints - curStatPointsUsed, MAX_BASICSTATS);
    }

    updateSkillChoices(skillPointsEarned);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::checkStatInfo() const
{
    // CurMana and CurStamina can be zero but seems to be always present
    StatInfo = EnumCharStatInfo::All;
    if (Cs.StatsLeft == 0)
    {
        StatInfo &= ~EnumCharStatInfo::StatPoints;
    }

    if (Cs.SkillChoices == 0)
    {
        StatInfo &= ~EnumCharStatInfo::SkillChoices;
    }

    if (Cs.CurLife == 0) // if you are dead
    {
        StatInfo &= ~EnumCharStatInfo::CurLife;
    }

    if (Cs.Experience == 0) // if you just started
    {
        StatInfo &= ~EnumCharStatInfo::Experience;
    }

    if (Cs.GoldInBelt == 0)
    {
        StatInfo &= ~EnumCharStatInfo::GoldInBelt;
    }

    if (Cs.GoldInStash == 0)
    {
        StatInfo &= ~EnumCharStatInfo::GoldInStash;
    }
}
//---------------------------------------------------------------------------
d2ce::EnumCharStatInfo d2ce::CharacterStats::GetStatInfoMask(std::uint16_t stat) const
{
    switch (stat)
    {
    case 0:
        return EnumCharStatInfo::Strength;
    case 1:
        return EnumCharStatInfo::Energy;
    case 2:
        return EnumCharStatInfo::Dexterity;
    case 3:
        return EnumCharStatInfo::Vitality;
    case 4:
        return EnumCharStatInfo::StatPoints;
    case 5:
        return EnumCharStatInfo::SkillChoices;
    case 6:
        return EnumCharStatInfo::CurLife;
    case 7:
        return EnumCharStatInfo::MaxLife;
    case 8:
        return EnumCharStatInfo::CurMana;
    case 9:
        return EnumCharStatInfo::MaxMana;
    case 10:
        return EnumCharStatInfo::CurStamina;
    case 11:
        return EnumCharStatInfo::MaxStamina;
    case 12:
        return EnumCharStatInfo::Level;
    case 13:
        return EnumCharStatInfo::Experience;
    case 14:
        return EnumCharStatInfo::GoldInBelt;
    case 15:
        return EnumCharStatInfo::GoldInStash;
    default:
        return EnumCharStatInfo::All;
    }
}
//---------------------------------------------------------------------------
std::uint32_t* d2ce::CharacterStats::GetStatBuffer(std::uint16_t stat) const
{
    switch (stat)
    {
    case 0:
        return &Cs.Strength;
    case 1:
        return &Cs.Energy;
    case 2:
        return &Cs.Dexterity;
    case 3:
        return &Cs.Vitality;
    case 4:
        return &Cs.StatsLeft;
    case 5:
        return &Cs.SkillChoices;
    case 6:
        return &Cs.CurLife;
    case 7:
        return &Cs.MaxLife;
    case 8:
        return &Cs.CurMana;
    case 9:
        return &Cs.MaxMana;
    case 10:
        return &Cs.CurStamina;
    case 11:
        return &Cs.MaxStamina;
    case 12:
        return &Cs.Level;
    case 13:
        return &Cs.Experience;
    case 14:
        return &Cs.GoldInBelt;
    case 15:
        return &Cs.GoldInStash;
    default:
        return nullptr;
    }
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::CharacterStats::readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
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

    return (*((std::uint64_t*)&data[readOffset / 8]) >> (readOffset & 7)) & ((std::uint64_t(1) << bits) - 1);
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
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
size_t d2ce::CharacterStats::readNextStat(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t& stat)
{
    size_t totalBitsRead = STAT_BITS;
    stat = (std::uint16_t)readBits(charfile, current_bit_offset, STAT_BITS);
    if (stat >= STAT_MAX)
    {
        return totalBitsRead;
    }

    totalBitsRead += readStatBits(charfile, current_bit_offset, stat);
    return totalBitsRead;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::readStatBits(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t stat)
{
    std::uint32_t* pStatValue = GetStatBuffer(stat);
    if (pStatValue == nullptr)
    {
        return 0;
    }

    size_t bits = V110_BITS_PER_STAT[stat];
    *pStatValue = (std::uint32_t)readBits(charfile, current_bit_offset, bits);
    return bits;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readAllStats(std::FILE* charfile)
{
    if (update_locations)
    {
        // find stats location
        stats_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (CharInfo.getVersion() >= EnumCharVersion::v109)
        {
            if (cur_pos < (long)MIN_START_STATS_POS)
            {
                cur_pos = MIN_START_STATS_POS;
                std::fseek(charfile, cur_pos, SEEK_SET);
            }
        }
        else
        {
            if (cur_pos < MIN_START_STATS_POS_v100)
            {
                cur_pos = MIN_START_STATS_POS_v100;
                std::fseek(charfile, cur_pos, SEEK_SET);
            }
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != STATS_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != STATS_MARKER[1])
            {
                continue;
            }

            // found stats marker (0x6766). 
            stats_location = std::ftell(charfile);
            break;
        }
    }

    if (stats_location == 0)
    {
        return false;
    }

    updateStartStats();
    std::fseek(charfile, stats_location, SEEK_SET);

    // zero out all stats as zero value stats do not have to exists in the file
    Cs.Strength = 0;
    Cs.Energy = 0;
    Cs.Dexterity = 0;
    Cs.Vitality = 0;
    Cs.StatsLeft = 0;
    Cs.SkillChoices = 0;
    Cs.CurLife = 0;
    Cs.MaxLife = 0;
    Cs.CurMana = 0;
    Cs.MaxMana = 0;
    Cs.CurStamina = 0;
    Cs.MaxStamina = 0;
    Cs.Level = 1;
    Cs.Experience = 0;
    Cs.GoldInBelt = 0;
    Cs.GoldInStash = 0;
    Cs.MaxLevel = MAX_NUM_LEVELS;
    Cs.MinExperienceLevel = 0ui32;
    Cs.NextExperienceLevel = 500ui32;
    Cs.MaxExperience = MAX_EXPERIENCE;
    Cs.MaxGoldInBelt = 10000ui32;
    Cs.MaxGoldInStash = (CharInfo.getVersion() >= EnumCharVersion::v110) ? d2ce::GOLD_IN_STASH_LIMIT : 50000ui32;

    if (CharInfo.getVersion() < EnumCharVersion::v110)
    {
        return readAllStats_109(charfile);
    }

    size_t current_bit_offset = 0;
    data.clear();

    std::uint16_t stat = 0;
    size_t totalBitsRead = 0;
    size_t bitsRead = readNextStat(charfile, current_bit_offset, stat);
    while (bitsRead > 0)
    {
        totalBitsRead += bitsRead;
        if (stat >= STAT_MAX)
        {
            break;
        }

        bitsRead = readNextStat(charfile, current_bit_offset, stat);
    }

    updateMinStats();
    return totalBitsRead > 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readAllStats_109(std::FILE* charfile)
{
    std::uint16_t value;
    std::fread(&value, sizeof(value), 1, charfile);
    StatInfo = static_cast<EnumCharStatInfo>(value);

    // skip null byte if character version is less than 1.09
    if (CharInfo.getVersion() < EnumCharVersion::v109)
    {
        std::fread(&nullByte, sizeof(nullByte), 1, charfile);
    }

    // read basic stats
    std::fread(&Cs.Strength, sizeof(Cs.Strength), 1, charfile);   // always present so no need to check StatInfo bits
    std::fread(&Cs.Energy, sizeof(Cs.Energy), 1, charfile);       // always present so no need to check StatInfo bits
    std::fread(&Cs.Dexterity, sizeof(Cs.Dexterity), 1, charfile); // always present so no need to check StatInfo bits
    std::fread(&Cs.Vitality, sizeof(Cs.Vitality), 1, charfile);   // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::StatPoints) == EnumCharStatInfo::StatPoints)
    {
        std::fread(&Cs.StatsLeft, sizeof(Cs.StatsLeft), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::SkillChoices) == EnumCharStatInfo::SkillChoices)
    {
        std::fread(&Cs.SkillChoices, sizeof(Cs.SkillChoices), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::CurLife) == EnumCharStatInfo::CurLife)
    {
        std::fread(&Cs.CurLife, sizeof(Cs.CurLife), 1, charfile);
    }

    std::fread(&Cs.MaxLife, sizeof(Cs.MaxLife), 1, charfile); // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::CurMana) == EnumCharStatInfo::CurMana) // can be zero but should be always present
    {
        std::fread(&Cs.CurMana, sizeof(Cs.CurMana), 1, charfile);
    }

    std::fread(&Cs.MaxMana, sizeof(Cs.MaxMana), 1, charfile); // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::CurStamina) == EnumCharStatInfo::CurStamina) // can be zero but should be always present
    {
        std::fread(&Cs.CurStamina, sizeof(Cs.CurStamina), 1, charfile);
    }

    std::fread(&Cs.MaxStamina, sizeof(Cs.MaxStamina), 1, charfile); // always present so no need to check StatInfo bits
    std::fread(&Cs.Level, sizeof(Cs.Level), 1, charfile);           // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::Experience) == EnumCharStatInfo::Experience)
    {
        std::fread(&Cs.Experience, sizeof(Cs.Experience), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::GoldInBelt) == EnumCharStatInfo::GoldInBelt)
    {
        std::fread(&Cs.GoldInBelt, sizeof(Cs.GoldInBelt), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::GoldInStash) == EnumCharStatInfo::GoldInStash)
    {
        std::fread(&Cs.GoldInStash, sizeof(Cs.GoldInStash), 1, charfile);
    }

    updateMinStats();
    return true;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::applyJsonStats(const Json::Value& statsRoot, bool bSerializedFormat)
{
    if (!statsRoot.isNull())
    {
        std::string key;
        std::string name;
        static std::initializer_list<std::uint16_t> all_fullstats = { 6, 7, 8, 9, 10, 11 };
        std::uint32_t* pStatValue = nullptr;
        std::uint32_t statValue = 0;
        auto iter_end = statsRoot.end();
        for (auto iter = statsRoot.begin(); iter != iter_end; ++iter)
        {
            if (iter->isNull())
            {
                continue;
            }

            key = iter.name();
            for (std::uint16_t stat = 0; stat < STAT_MAX; ++stat)
            {
                name = getAttributeJsonName(stat, bSerializedFormat);
                if (_stricmp(key.c_str(), name.c_str()) == 0)
                {
                    statValue = std::uint32_t(iter->asInt64());
                    if (std::find(all_fullstats.begin(), all_fullstats.end(), stat) != all_fullstats.end())
                    {
                        statValue <<= 8;
                    }

                    pStatValue = GetStatBuffer(stat);
                    if (pStatValue != nullptr)
                    {
                        *pStatValue = statValue;
                    }
                    break;
                }
            }
        }

        for (auto iter = statsRoot.begin(); iter != iter_end; ++iter)
        {
            if (iter->isNull())
            {
                continue;
            }

            key = iter.name();
            for (const auto& stat : all_fullstats)
            {
                name = getAttributeJsonName(stat, bSerializedFormat, true);
                if (_stricmp(key.c_str(), name.c_str()) == 0)
                {
                    statValue = (std::uint32_t(iter->asInt64()) & 0xFF);
                    pStatValue = GetStatBuffer(stat);
                    if (pStatValue != nullptr)
                    {
                        *pStatValue |= statValue;
                    }
                    break;
                }
            }
        }
    }

    Cs.Vitality = std::min(std::max(Cs.Vitality, min_vitality), MAX_BASICSTATS);
    Cs.Energy = std::min(std::max(Cs.Energy, min_energy), MAX_BASICSTATS);
    Cs.Dexterity = std::min(std::max(Cs.Dexterity, min_dexterity), MAX_BASICSTATS);
    Cs.Strength = std::min(std::max(Cs.Strength, min_strength), MAX_BASICSTATS);

    updateMinStats();
    bool updateCur = Cs.CurLife == Cs.MaxLife ? true : false;
    std::uint32_t maxValue = std::max(Cs.MaxLife, min_hit_points);
    if ((maxValue > Cs.MaxLife) && (((maxValue - Cs.MaxLife) >> 8) > 1))
    {
        Cs.MaxLife = maxValue;
        if (updateCur)
        {
            Cs.CurLife = Cs.MaxLife;
        }
    }

    updateCur = Cs.CurStamina == Cs.MaxStamina ? true : false;
    maxValue = std::max(Cs.MaxStamina, min_stamina);
    if ((maxValue > Cs.MaxStamina) && (((maxValue - Cs.MaxStamina) >> 8) > 1))
    {
        Cs.MaxStamina = maxValue;
        if (updateCur)
        {
            Cs.CurStamina = Cs.MaxStamina;
        }
    }

    updateCur = Cs.CurMana == Cs.MaxMana ? true : false;
    maxValue = std::max(Cs.MaxMana, min_mana);
    if ((maxValue > Cs.MaxMana) && (((maxValue - Cs.MaxMana) >> 8) > 1))
    {
        Cs.MaxMana = maxValue;
        if (updateCur)
        {
            Cs.CurMana = Cs.MaxMana;
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readAllStats(const Json::Value& statsRoot, bool bSerializedFormat, std::FILE* charfile)
{
    updateStartStats();

    std::fwrite(STATS_MARKER.data(), STATS_MARKER.size(), 1, charfile);
    stats_location = std::ftell(charfile);
    applyJsonStats(statsRoot, bSerializedFormat);

    checkStatInfo();
    if (CharInfo.getVersion() < EnumCharVersion::v110)
    {
        return writeStats_109(charfile);
    }

    size_t current_bit_offset = 0;
    data.clear();

    size_t totalBitsWritten = 0;
    for (std::uint16_t stat = 0; stat < STAT_MAX; ++stat)
    {
        totalBitsWritten += updateStat(charfile, current_bit_offset, stat);
    }
    totalBitsWritten += updateStat(charfile, current_bit_offset, STAT_END_MARKER);

    std::fflush(charfile);
    if (totalBitsWritten == 0)
    {
        return false;
    }

    return true;
}
bool d2ce::CharacterStats::readSkills(std::FILE* charfile)
{
    if (update_locations)
    {
        // find stats location
        skills_location = 0;
        pd2_skills_location = 0;
        has_pd2_skills = false;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)stats_location)
        {
            cur_pos = stats_location;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != SKILLS_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != SKILLS_MARKER[1])
            {
                continue;
            }

            // found skills marker (0x6669). 
            skills_location = std::ftell(charfile);
            break;
        }

        if (skills_location == 0)
        {
            return false;
        }
    }
    else
    {
        if (skills_location == 0)
        {
            return false;
        }

        std::fseek(charfile, skills_location, SEEK_SET);
    }

    std::fread(Skills.data(), Skills.size(), 1, charfile);

    if (update_locations && (CharInfo.getVersion() == EnumCharVersion::v110))
    {
        // Check for PD2 version
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        auto temp_pos = cur_pos;
        for (size_t num = 0; (num < 3) && !feof(charfile); ++num)
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[0])
            {
                continue;
            }

            temp_pos = std::ftell(charfile);
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[1])
            {
                std::fseek(charfile, temp_pos, SEEK_SET);
                continue;
            }

            // not a PD2 file
            std::fseek(charfile, cur_pos, SEEK_SET);
            return true;
        }

        if (feof(charfile))
        {
            // not a PD2 file
            std::fseek(charfile, cur_pos, SEEK_SET);
            return true;
        }

        std::fread(&value, sizeof(value), 1, charfile);
        if (value != ITEM_MARKER[0])
        {
            std::fseek(charfile, temp_pos, SEEK_SET);
            return true;
        }

        if (feof(charfile))
        {
            // not a PD2 file
            std::fseek(charfile, cur_pos, SEEK_SET);
            return true;
        }

        std::fread(&value, sizeof(value), 1, charfile);
        if (value != ITEM_MARKER[1])
        {
            std::fseek(charfile, temp_pos, SEEK_SET);
            return true;
        }

        // we detected a PD2 file with 3 extra bytes for skills
        pd2_skills_location = cur_pos;
        has_pd2_skills = true;
        std::fseek(charfile, pd2_skills_location, SEEK_SET);
        std::fread(PD2Skills.data(), PD2Skills.size(), 1, charfile);
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::applyJsonSkills(const Json::Value& /*root*/, const Json::Value& skillsRoot, bool bSerializedFormat)
{
    if (skillsRoot.isNull())
    {
        return;
    }

    Json::Value value;
    std::uint16_t id;
    auto iter_end = skillsRoot.end();
    for (auto iter = skillsRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        value = iter->operator[](bSerializedFormat ? "Id" : "id");
        if (value.isNull())
        {
            continue;
        }

        id = std::uint16_t(value.asInt64());
        for (std::uint16_t skill = 0; skill < NUM_OF_SKILLS; ++skill)
        {
            if (id == getSkillId(skill))
            {
                value = iter->operator[](bSerializedFormat ? "Points" : "points");
                if (!value.isNull())
                {
                    Skills[skill] = std::uint8_t(value.asInt64());
                }
                break;
            }
        }
    }

    pd2_skills_location = 0;
    has_pd2_skills = false;
    /*TODO:
    if (CharInfo.getVersion() == EnumCharVersion::v110)
    {
        Json::Value pd2SkillsRoot = root[bSerializedFormat ? "ClassSkills" : "pd2_skills"];
        if (!pd2SkillsRoot.isNull() && bSerializedFormat)
        {
            pd2SkillsRoot = pd2SkillsRoot["PD2Skills"];
        }

        if (!pd2SkillsRoot.isNull())
        {
            has_pd2_skills = true;
            iter_end = pd2SkillsRoot.end();
            for (auto iter = pd2SkillsRoot.begin(); iter != iter_end; ++iter)
            {
                if (iter->isNull())
                {
                    continue;
                }

                value = iter->operator[](bSerializedFormat ? "Id" : "id");
                if (value.isNull())
                {
                    continue;
                }

                id = std::uint16_t(value.asInt64());
                for (std::uint32_t skill = 0; skill < NUM_OF_PD2_SKILLS; ++skill)
                {
                    if (id == getSkillId(skill + NUM_OF_SKILLS))
                    {
                        value = iter->operator[](bSerializedFormat ? "Points" : "points");
                        if (!value.isNull())
                        {
                            PD2Skills[skill] = std::uint8_t(value.asInt64());
                        }
                        break;
                    }
                }
            }
        }
    }
    */
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readSkills(const Json::Value& root, const Json::Value& skillsRoot, bool bSerializedFormat, std::FILE* charfile)
{
    std::fwrite(SKILLS_MARKER.data(), SKILLS_MARKER.size(), 1, charfile);
    skills_location = std::ftell(charfile);
    applyJsonSkills(root, skillsRoot, bSerializedFormat);
    std::fwrite(Skills.data(), Skills.size(), 1, charfile);
    std::fflush(charfile);

    if (isPD2Format())
    {
        pd2_skills_location = std::ftell(charfile);
        std::fwrite(PD2Skills.data(), PD2Skills.size(), 1, charfile);
        std::fflush(charfile);
    }

    return true;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::updateBits(size_t& current_bit_offset, size_t size, std::uint32_t value) const
{
    size_t startIdx = current_bit_offset / 8;
    size_t endIdx = (current_bit_offset + size) / 8;
    if (endIdx >= data.size())
    {
        // not enough space
        data.resize(endIdx + 1, 0);
    }

    size_t startBit = current_bit_offset % 8;
    size_t numBytes = (endIdx - startIdx) + 1;
    if (numBytes > 5)
    {
        // 32 bit value can't consume more then this many bytes
        return 0;
    }

    std::uint64_t mask = (((std::uint64_t)1 << size) - 1) << (startBit);
    std::uint64_t dest = 0;
    std::memcpy((std::uint8_t*)&dest, &data[startIdx], numBytes);
    dest = (dest & ~mask) | (((std::uint64_t)value << (startBit)) & mask);
    std::memcpy(&data[startIdx], (std::uint8_t*)&dest, numBytes);
    current_bit_offset += size;
    return size;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::updateStat(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t stat) const
{
    if (stat >= STAT_MAX && (stat != STAT_END_MARKER))
    {
        return 0;
    }

    EnumCharStatInfo mask = GetStatInfoMask(stat);
    if ((stat != STAT_END_MARKER) && (StatInfo & mask) != mask)
    {
        // skip any stat that who's mask indicates we should not write it
        return 0;
    }

    size_t totalBitsWritten = updateBits(current_bit_offset, STAT_BITS, std::uint16_t(stat));
    if (totalBitsWritten == 0)
    {
        return 0;
    }

    if (stat == STAT_END_MARKER)
    {
        writeBufferBits(charfile);
        return totalBitsWritten + ((data.size() * 8) - current_bit_offset); // we wrote any bits we did not use as well
    }

    totalBitsWritten += updateStatBits(current_bit_offset, stat);
    return totalBitsWritten;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::updateStatBits(size_t& current_bit_offset, std::uint16_t stat) const
{
    std::uint32_t* pStatValue = GetStatBuffer(stat);
    if (pStatValue == nullptr)
    {
        return 0;
    }

    // write value
    size_t numBits = V110_BITS_PER_STAT[stat];
    return updateBits(current_bit_offset, numBits, *pStatValue);
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::writeBufferBits(std::FILE* charfile) const
{
    std::fseek(charfile, stats_location, SEEK_SET);
    std::fwrite(&data[0], data.size(), 1, charfile);
    return data.size() * 8;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::writeStats_109(std::FILE* charfile) const
{
    std::uint16_t value = StatInfo.bits();
    std::fwrite(&value, sizeof(value), 1, charfile);

    // skip null byte if character version is less than 1.09
    if (CharInfo.getVersion() < EnumCharVersion::v109)
    {
        std::fwrite(&nullByte, sizeof(nullByte), 1, charfile);
    }

    // write basic stats
    std::fwrite(&Cs.Strength, sizeof(Cs.Strength), 1, charfile);   // always present so no need to check StatInfo bits
    std::fwrite(&Cs.Energy, sizeof(Cs.Energy), 1, charfile);       // always present so no need to check StatInfo bits
    std::fwrite(&Cs.Dexterity, sizeof(Cs.Dexterity), 1, charfile); // always present so no need to check StatInfo bits
    std::fwrite(&Cs.Vitality, sizeof(Cs.Vitality), 1, charfile);   // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::StatPoints) == EnumCharStatInfo::StatPoints)
    {
        std::fwrite(&Cs.StatsLeft, sizeof(Cs.StatsLeft), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::SkillChoices) == EnumCharStatInfo::SkillChoices)
    {
        std::fwrite(&Cs.SkillChoices, sizeof(Cs.SkillChoices), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::CurLife) == EnumCharStatInfo::CurLife)
    {
        std::fwrite(&Cs.CurLife, sizeof(Cs.CurLife), 1, charfile);
    }

    std::fwrite(&Cs.MaxLife, sizeof(Cs.MaxLife), 1, charfile);       // always present so no need to check StatInfo bits
    std::fwrite(&Cs.CurMana, sizeof(Cs.CurMana), 1, charfile);       // always present so no need to check StatInfo bits
    std::fwrite(&Cs.MaxMana, sizeof(Cs.MaxMana), 1, charfile);       // always present so no need to check StatInfo bits
    std::fwrite(&Cs.CurStamina, sizeof(Cs.CurStamina), 1, charfile); // always present so no need to check StatInfo bits
    std::fwrite(&Cs.MaxStamina, sizeof(Cs.MaxStamina), 1, charfile); // always present so no need to check StatInfo bits
    std::fwrite(&Cs.Level, sizeof(Cs.Level), 1, charfile);           // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::Experience) == EnumCharStatInfo::Experience)
    {
        std::fwrite(&Cs.Experience, sizeof(Cs.Experience), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::GoldInBelt) == EnumCharStatInfo::GoldInBelt)
    {
        std::fwrite(&Cs.GoldInBelt, sizeof(Cs.GoldInBelt), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::GoldInStash) == EnumCharStatInfo::GoldInStash)
    {
        std::fwrite(&Cs.GoldInStash, sizeof(Cs.GoldInStash), 1, charfile);
    }
    std::fflush(charfile);

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::writeSkills(std::FILE* charfile) const
{
    std::fwrite(SKILLS_MARKER.data(), SKILLS_MARKER.size(), 1, charfile);
    skills_location = std::ftell(charfile);
    std::fwrite(Skills.data(), Skills.size(), 1, charfile);
    std::fflush(charfile);

    if (isPD2Format())
    {
        pd2_skills_location = std::ftell(charfile);
        std::fwrite(PD2Skills.data(), PD2Skills.size(), 1, charfile);
        std::fflush(charfile);
    }

    return true;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getStatPointsPerLevel() const
{
    const auto& charClassInfo = GetCharClassInfo(CharInfo.getClass());
    return charClassInfo.StatPerLevel == 0 ? 5 : charClassInfo.StatPerLevel;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::setTxtReader()
{
    auto& txtReader = ItemHelpers::getTxtReader();
    InitSkillInfoData(txtReader);
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readStats(std::FILE* charfile)
{
    update_locations = stats_location == 0 ? true : false;
    if (!readAllStats(charfile) || stats_location == 0)
    {
        return false;
    }

    if (!readSkills(charfile) || skills_location == 0)
    {
        return false;
    }

    update_locations = false;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readStats(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile)
{
    clear();
    Json::Value childRoot;
    if (!root.isNull())
    {
        childRoot = root[bSerializedFormat ? "Attributes" : "attributes"];
        if (!childRoot.isNull() && bSerializedFormat)
        {
            childRoot = childRoot["Stats"];
        }
    }

    update_locations = true;
    if (!readAllStats(childRoot, bSerializedFormat, charfile) || stats_location == 0)
    {
        return false;
    }

    if (!root.isNull())
    {
        childRoot = root[bSerializedFormat ? "ClassSkills" : "skills"];
        if (!childRoot.isNull() && bSerializedFormat)
        {
            childRoot = childRoot["Skills"];
        }
    }

    if (!readSkills(root, childRoot, bSerializedFormat, charfile) || skills_location == 0)
    {
        return false;
    }

    update_locations = false;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::writeStats(std::FILE* charfile) const
{
    if (stats_location == 0)
    {
        return false;
    }

    checkStatInfo();

    std::fseek(charfile, stats_location - std::uint32_t(STATS_MARKER.size()), SEEK_SET);
    std::fwrite(STATS_MARKER.data(), STATS_MARKER.size(), 1, charfile);
    if (CharInfo.getVersion() < EnumCharVersion::v110)
    {
        if (!writeStats_109(charfile))
        {
            return false;
        }
    }
    else
    {
        size_t current_bit_offset = 0;
        data.clear();

        size_t totalBitsWritten = 0;
        for (std::uint16_t stat = 0; stat < STAT_MAX; ++stat)
        {
            totalBitsWritten += updateStat(charfile, current_bit_offset, stat);
        }
        totalBitsWritten += updateStat(charfile, current_bit_offset, STAT_END_MARKER);

        std::fflush(charfile);
        if (totalBitsWritten == 0)
        {
            return false;
        }
    }

    return writeSkills(charfile);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getHeaderLocation()
{
    return stats_location >= 2 ? stats_location - 2 : 0;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::resetStats(std::uint16_t lifePointsEarned, std::uint16_t statPointEarned, std::uint16_t skillPointsEarned)
{
    Cs.Vitality = min_vitality;
    Cs.Energy = min_energy;
    Cs.Dexterity = min_dexterity;
    Cs.Strength = min_strength;
    Cs.StatsLeft = 0;

    updateLifePointsEarned(lifePointsEarned);
    Cs.MaxLife = min_hit_points;
    Cs.CurLife = Cs.MaxLife;
    Cs.MaxStamina = min_stamina;
    Cs.CurStamina = Cs.MaxStamina;
    Cs.MaxMana = min_mana;
    Cs.CurMana = Cs.MaxMana;

    std::uint32_t curLevel = std::min(std::max(Cs.Level, std::uint32_t(1)), max_levels);
    Cs.StatsLeft = (curLevel - 1) * 5 + statPointEarned;

    resetSkills(skillPointsEarned);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateSkills(const std::array<std::uint8_t, NUM_OF_SKILLS>& updated_skills, std::uint16_t skillPointsEarned, std::uint32_t skillChoices)
{
    for (size_t i = 0; i < NUM_OF_SKILLS; ++i)
    {
        Skills[i] = std::min(updated_skills[i], MAX_SKILL_VALUE);
    }
    Cs.SkillChoices = skillChoices;
    updateSkillChoices(skillPointsEarned);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::resetSkills(std::uint16_t skillPointsEarned)
{
    Skills.fill(0);
    Cs.SkillChoices = 0;
    updateSkillChoices(skillPointsEarned);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateClass()
{
    auto oldVitality = Cs.Vitality;
    auto oldEnergy = Cs.Energy;
    auto oldMaxLife = Cs.MaxLife;
    auto oldMaxStamina = Cs.MaxStamina;
    auto oldMaxMana = Cs.MaxMana;

    updateStartStats();
    Cs.Vitality = std::min(std::max(Cs.Vitality, min_vitality), MAX_BASICSTATS);
    Cs.Energy = std::min(std::max(Cs.Energy, min_energy), MAX_BASICSTATS);
    Cs.Dexterity = std::min(std::max(Cs.Dexterity, min_dexterity), MAX_BASICSTATS);
    Cs.Strength = std::min(std::max(Cs.Strength, min_strength), MAX_BASICSTATS);

    updateMinStats();
    if (oldVitality != Cs.Vitality || oldMaxLife > Cs.MaxLife)
    {
        bool updateCur = Cs.CurLife == Cs.MaxLife ? true : false;
        Cs.MaxLife = std::max(Cs.MaxLife, min_hit_points);
        if (updateCur)
        {
            Cs.CurLife = Cs.MaxLife;
        }
    }

    if (oldVitality != Cs.Vitality || oldMaxStamina > Cs.MaxStamina)
    {
        bool updateCur = Cs.CurStamina == Cs.MaxStamina ? true : false;
        Cs.MaxStamina = std::max(Cs.MaxStamina, min_stamina);
        if (updateCur)
        {
            Cs.CurStamina = Cs.MaxStamina;
        }
    }

    if (oldEnergy != Cs.Energy || oldMaxMana > Cs.MaxMana)
    {
        bool updateCur = Cs.CurMana == Cs.MaxMana ? true : false;
        Cs.MaxMana = std::max(Cs.MaxMana, min_mana);
        if (updateCur)
        {
            Cs.CurMana = Cs.MaxMana;
        }
    }
}
//---------------------------------------------------------------------------
std::string d2ce::CharacterStats::getAttributeJsonName(std::uint16_t stat, bool bSerializedFormat, bool bRemainder) const
{
    if (bRemainder)
    {
        switch (stat)
        {
        case 6:
            return bSerializedFormat ? "hitpoints_remainder" : "current_hp_remainder";
        case 7:
            return bSerializedFormat ? "maxhp_remainder" : "max_hp_remainder";
        case 8:
            return bSerializedFormat ? "mana_remainder" : "current_mana_remainder";
        case 9:
            return bSerializedFormat ? "maxmana_remainder" : "max_mana_remainder";
        case 10:
            return bSerializedFormat ? "stamina_remainder" : "current_stamina_remainder";
        case 11:
            return bSerializedFormat ? "maxstamina_remainder" : "max_stamina_remainder";
        }
    }
    switch (stat)
    {
    case 0:
        return "strength";
    case 1:
        return "energy";
    case 2:
        return "dexterity";
    case 3:
        return "vitality";
    case 4:
        return bSerializedFormat ? "statpts" : "unused_stats";
    case 5:
        return bSerializedFormat ? "newskills" : "unused_skill_points";
    case 6:
        return bSerializedFormat ? "hitpoints" : "current_hp";
    case 7:
        return bSerializedFormat ? "maxhp" : "max_hp";
    case 8:
        return bSerializedFormat ? "mana" : "current_mana";
    case 9:
        return bSerializedFormat ? "maxmana" : "max_mana";
    case 10:
        return bSerializedFormat ? "stamina" : "current_stamina";
    case 11:
        return bSerializedFormat ? "maxstamina" : "max_stamina";
    case 12:
        return "level";
    case 13:
        return "experience";
    case 14:
        return "gold";
    case 15:
        return bSerializedFormat ? "goldbank" : "stashed_gold";
    }

    return "";
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::attributesAsJson(Json::Value& parent, bool bSerializedFormat) const
{
    Json::Value attributes;
    std::uint32_t* pStatValue = nullptr;
    std::uint32_t statValue = 0;
    static std::initializer_list<std::uint16_t> all_fullstats = { 6, 7, 8, 9, 10, 11 };

    (const_cast<CharacterStats*>(this))->checkStatInfo();
    if (bSerializedFormat)
    {
        attributes["Header"] = *((std::uint16_t*)STATS_MARKER.data());

        Json::Value stats;
        for (std::uint16_t stat = 0; stat < STAT_MAX; ++stat)
        {
            pStatValue = (const_cast<CharacterStats*>(this))->GetStatBuffer(stat);
            if (pStatValue == nullptr)
            {
                continue;
            }

            statValue = *pStatValue;
            if (std::find(all_fullstats.begin(), all_fullstats.end(), stat) != all_fullstats.end())
            {
                if ((statValue & 0xFF) != 0)
                {
                    stats[getAttributeJsonName(stat, bSerializedFormat, true)] = (statValue & 0xFF);
                }
                statValue >>= 8;
            }

            stats[getAttributeJsonName(stat, bSerializedFormat)] = statValue;
        }
        attributes["Stats"] = stats;
        parent["Attributes"] = attributes;
    }
    else
    {
        EnumCharStatInfo mask = EnumCharStatInfo::All;
        for (std::uint16_t stat = 0; stat < STAT_MAX; ++stat)
        {
            mask = GetStatInfoMask(stat);
            pStatValue = (const_cast<CharacterStats*>(this))->GetStatBuffer(stat);
            if (pStatValue == nullptr || ((StatInfo & mask) != mask))
            {
                continue;
            }

            statValue = *pStatValue;
            if (std::find(all_fullstats.begin(), all_fullstats.end(), stat) != all_fullstats.end())
            {
                if ((statValue & 0xFF) != 0)
                {
                    attributes[getAttributeJsonName(stat, bSerializedFormat, true)] = (statValue & 0xFF);
                }
                statValue >>= 8;
            }
            attributes[getAttributeJsonName(stat, bSerializedFormat)] = statValue;
        }
        parent["attributes"] = attributes;
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::skillsAsJson(Json::Value& parent, bool bSerializedFormat) const
{
    if (bSerializedFormat)
    {
        Json::Value classSkills;
        classSkills["Header"] = *((std::uint16_t*)SKILLS_MARKER.data());

        Json::Value skills(Json::arrayValue);
        for (std::uint16_t skill = 0; skill < NUM_OF_SKILLS; ++skill)
        {
            Json::Value skillElement;
            skillElement["Id"] = getSkillId(skill);
            skillElement["Points"] = getSkillPoints(skill);
            skills.append(skillElement);
        }
        classSkills["Skills"] = skills;

        if (isPD2Format())
        {
            Json::Value pd2Skills(Json::arrayValue);
            for (std::uint16_t skill = NUM_OF_SKILLS; skill < NUM_OF_SKILLS + NUM_OF_PD2_SKILLS; ++skill)
            {
                Json::Value skillElement;
                skillElement["Id"] = getSkillId(skill);
                skillElement["Points"] = getSkillPoints(skill);
                pd2Skills.append(skillElement);
            }
            classSkills["PD2Skills"] = pd2Skills;
        }

        parent["ClassSkills"] = classSkills;
    }
    else
    {
        Json::Value skills(Json::arrayValue);
        for (std::uint16_t skill = 0; skill < NUM_OF_SKILLS; ++skill)
        {
            const auto& skillInfo = getSkill(skill);
            Json::Value skillElement;
            skillElement["id"] = skillInfo.id;
            skillElement["points"] = getSkillPoints(skill);
            skillElement["name"] = skillInfo.name;
            skills.append(skillElement);
        }
        parent["skills"] = skills;

        if (isPD2Format())
        {
            Json::Value pd2Skills(Json::arrayValue);
            for (std::uint16_t skill = NUM_OF_SKILLS; skill < NUM_OF_SKILLS + NUM_OF_PD2_SKILLS; ++skill)
            {
                const auto& skillInfo = getSkill(skill);
                Json::Value skillElement;
                skillElement["id"] = skillInfo.id;
                skillElement["points"] = getSkillPoints(skill);
                skillElement["name"] = skillInfo.name;
                pd2Skills.append(skillElement);
            }
            parent["pd2_skills"] = pd2Skills;
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::asJson(Json::Value& parent, bool bSerializedFormat) const
{
    attributesAsJson(parent, bSerializedFormat);
    skillsAsJson(parent, bSerializedFormat);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::clear()
{
    StatInfo = EnumCharStatInfo::All;
    Cs.Strength = 0;
    Cs.Energy = 0;
    Cs.Dexterity = 0;
    Cs.Vitality = 0;
    Cs.StatsLeft = 0;
    Cs.SkillChoices = 0;
    Cs.CurLife = 0;
    Cs.MaxLife = 0;
    Cs.CurMana = 0;
    Cs.MaxMana = 0;
    Cs.CurStamina = 0;
    Cs.MaxStamina = 0;
    Cs.Level = 1;
    Cs.Experience = 0;
    Cs.GoldInBelt = 0;
    Cs.GoldInStash = 0;
    Cs.MaxLevel = MAX_NUM_LEVELS;
    Cs.MinExperienceLevel = 0ui32;
    Cs.NextExperienceLevel = 500ui32;
    Cs.MaxExperience = MAX_EXPERIENCE;
    Cs.MaxGoldInBelt = 10000ui32;
    Cs.MaxGoldInStash = (CharInfo.getVersion() >= EnumCharVersion::v110) ? d2ce::GOLD_IN_STASH_LIMIT : 50000ui32;
    data.clear();
    Skills.fill(0);
    stats_location = 0;
    update_locations = true;

    min_hit_points = 0x100;
    min_stamina = 0x100;
    min_mana = 0x100;
    min_vitality = 1;
    min_energy = 1;
    min_dexterity = 1;
    min_strength = 1;
    min_stat_points_used = 80;
    max_levels = MAX_NUM_LEVELS;
    max_experience = MAX_EXPERIENCE;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::clearSkillChoices()
{
    Cs.SkillChoices = 0;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::getSkillBonusPoints(std::vector<std::uint16_t>& points) const
{
    points.clear();
    points.resize(d2ce::NUM_OF_SKILLS, 0);
    auto iterIdx = s_CharClassEnumMap.find(CharInfo.getClass());
    if (iterIdx == s_CharClassEnumMap.end())
    {
        return false;
    }

    auto iter = s_CharClassInfo.find(iterIdx->second);
    if (iter == s_CharClassInfo.end())
    {
        return false;
    }

    const auto& skills = iter->second.Skills;
    if (skills.empty())
    {
        return false;
    }
    if (skills.size() > points.size())
    {
        points.resize(skills.size(), 0);
    }

    // find character items with skill bonuses
    auto iterSkills = skills.begin();
    std::uint16_t classIdx = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    if (!CharInfo.getItemBonuses(magicalAttributes) || magicalAttributes.empty())
    {
        // no bonuses
        return true;
    }

    for (auto& attrib : magicalAttributes)
    {
        const auto& stat = ItemHelpers::getItemStat(attrib);
        if (stat.name == "item_allskills")
        {
            auto addSkill = std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, 1, 0, stat));
            for (auto& point : points)
            {
                point += addSkill;
            }
        }
        else
        {
            switch (stat.descFunc)
            {
            case 13: // +[value] to [class] Skill Levels
                // first value is the class
                classIdx = std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, 1, 0, stat));
                if (GetEnumCharClassByIndex(classIdx) == CharInfo.getClass())
                {
                    auto addSkill = std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, 1, 1, stat));
                    for (auto& point : points)
                    {
                        point += addSkill;
                    }
                }
                break;

            case 14: // +[value] to [skilltab] Skill Levels ([class] Only)
                // second value is the class
                classIdx = std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, 1, 1, stat));
                if (GetEnumCharClassByIndex(classIdx) == CharInfo.getClass())
                {
                    auto addSkill = std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, 1, 0, stat));
                    const auto& tabSkills = CharClassHelper::getSklTreeTab(std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, 1, 0, stat)), classIdx);
                    for (const auto& skillRows : tabSkills)
                    {
                        if ((skillRows.first == 0) || (skillRows.first > 6))
                        {
                            // should not happend
                            continue;
                        }

                        for (const auto& skillCols : skillRows.second)
                        {
                            if ((skillCols.first == 0) || (skillCols.first > 3))
                            {
                                // should not happend
                                continue;
                            }

                            const auto& skillInfo = d2ce::CharClassHelper::getSkillById(skillCols.second);
                            if (!skillInfo.classInfo.has_value())
                            {
                                // should not happend
                                continue;
                            }

                            auto charSkillIdx = skillInfo.classInfo.value().index;
                            if (points.size() > charSkillIdx)
                            {
                                auto& point = points[charSkillIdx];
                                point += addSkill;
                            }
                        }
                    }
                }
                break;

            case 27: // +[value] to [skill] ([class] Only)
            case 28: // +[value] to [skill]
                iterSkills = std::find(skills.begin(), skills.end(), std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, 1, 0, stat)));
                if (iterSkills != skills.end())
                {
                    const auto& skillInfo = getSkill(*iterSkills);
                    if (skillInfo.classInfo.has_value())
                    {
                        auto charSkillIdx = skillInfo.classInfo.value().index;
                        if (points.size() > charSkillIdx)
                        {
                            auto& point = points[charSkillIdx];
                            auto addSkill = std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, 1, 1, stat));
                            point += addSkill;
                        }
                    }
                }
                break;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::isPD2Format() const
{
    return has_pd2_skills;
}
//---------------------------------------------------------------------------
std::array<std::uint8_t, d2ce::NUM_OF_PD2_SKILLS>& d2ce::CharacterStats::getPD2Skills()
{
    return PD2Skills;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::fillCharacterStats(CharStats& cs) const
{
    std::memcpy(&cs, &Cs, sizeof(cs));
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::fillDisplayedCharacterStats(CharStats& cs) const
{
    fillCharacterStats(cs);

    // Apply item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (CharInfo.getItemBonuses(magicalAttributes))
    {
        ItemHelpers::applyNonMaxMagicalAttributes(cs, magicalAttributes);

        // update max values
        const auto& charClassInfo = GetCharClassInfo(CharInfo.getClass());
        std::uint32_t curLevel = std::min(std::max(cs.Level, std::uint32_t(1)), max_levels);
        std::uint32_t curVitality = std::max(std::min(cs.Vitality, MAX_BASICSTATS), charClassInfo.Vitality);
        std::uint32_t curEnergy = std::max(std::min(cs.Energy, MAX_BASICSTATS), charClassInfo.Energy);
        std::uint32_t diffVitality = curVitality - charClassInfo.Vitality;
        std::uint32_t diffEnergy = curEnergy - charClassInfo.Energy;
        std::uint32_t displayed_min_hit_points = ((charClassInfo.Vitality + charClassInfo.HpAdd) << 8) + (charClassInfo.LifePerLevel << 6) * (curLevel - 1) + (charClassInfo.LifePerVitality << 6) * diffVitality;
        std::uint32_t displayed_min_stamina = (charClassInfo.Stamina << 8) + (charClassInfo.StaminaPerLevel << 6) * (curLevel - 1) + (charClassInfo.StaminaPerVitality << 6) * diffVitality;
        std::uint32_t displayed_min_mana = (charClassInfo.Energy << 8) + (charClassInfo.ManaPerLevel << 6) * (curLevel - 1) + (charClassInfo.ManaPerMagic << 6) * diffEnergy;
        cs.MaxLife = std::max(cs.MaxLife, displayed_min_hit_points);
        cs.MaxStamina = std::max(cs.MaxStamina, displayed_min_stamina);
        cs.MaxMana = std::max(cs.MaxMana, displayed_min_mana);
        ItemHelpers::applyMaxMagicalAttributes(cs, magicalAttributes);
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateCharacterStats(const CharStats& cs)
{
    auto oldLevel = Cs.Level;
    auto oldVitality = Cs.Vitality;
    auto oldEnergy = Cs.Energy;
    auto oldMaxLife = Cs.MaxLife;
    auto oldMaxStamina = Cs.MaxStamina;
    auto oldMaxMana = Cs.MaxMana;
    std::memcpy(&Cs, &cs, sizeof(CharStats));

    // make sure level is correct
    Cs.Level = std::min(std::max(Cs.Level, std::uint32_t(1)), max_levels);
    Cs.MaxLevel = max_levels;
    Cs.MinExperienceLevel = GetMinExperienceLevel(Cs.Level, max_levels, CharInfo.getClass());
    Cs.NextExperienceLevel = GetNextExperienceLevel(Cs.Level, max_levels, CharInfo.getClass());
    Cs.MaxExperience = max_experience;
    Cs.MaxGoldInBelt = GetMaxGoldInBelt(Cs.Level);
    Cs.MaxGoldInStash = GetMaxGoldInStash(Cs.Level, CharInfo.getVersion());

    // make sure gold is correct
    Cs.GoldInBelt = std::min(Cs.GoldInBelt, Cs.MaxGoldInBelt);
    Cs.GoldInStash = std::min(Cs.GoldInStash, Cs.MaxGoldInStash);

    // Check min stats for chartacter
    updateStartStats();
    Cs.Vitality = std::min(std::max(Cs.Vitality, min_vitality), MAX_BASICSTATS);
    Cs.Energy = std::min(std::max(Cs.Energy, min_energy), MAX_BASICSTATS);
    Cs.Dexterity = std::min(std::max(Cs.Dexterity, min_dexterity), MAX_BASICSTATS);
    Cs.Strength = std::min(std::max(Cs.Strength, min_strength), MAX_BASICSTATS);

    updateMinStats();
    if (oldLevel != Cs.Level || oldVitality != Cs.Vitality || oldMaxLife > Cs.MaxLife)
    {
        bool updateCur = Cs.CurLife == Cs.MaxLife ? true : false;
        Cs.MaxLife = std::max(Cs.MaxLife, min_hit_points);
        if (updateCur)
        {
            Cs.CurLife = Cs.MaxLife;
        }
    }

    if (oldLevel != Cs.Level || oldVitality != Cs.Vitality || oldMaxStamina > Cs.MaxStamina)
    {
        bool updateCur = Cs.CurStamina == Cs.MaxStamina ? true : false;
        Cs.MaxStamina = std::max(Cs.MaxStamina, min_stamina);
        if (updateCur)
        {
            Cs.CurStamina = Cs.MaxStamina;
        }
    }

    if (oldLevel != Cs.Level || oldEnergy != Cs.Energy || oldMaxMana > Cs.MaxMana)
    {
        bool updateCur = Cs.CurMana == Cs.MaxMana ? true : false;
        Cs.MaxMana = std::max(Cs.MaxMana, min_mana);
        if (updateCur)
        {
            Cs.CurMana = Cs.MaxMana;
        }
    }

    // Check experiene level
    Cs.Experience = std::min(Cs.Experience, max_experience);
    if (oldLevel < Cs.Level)
    {
        Cs.Experience = std::max(Cs.Experience, Cs.MinExperienceLevel);
    }
    else if (oldLevel > Cs.Level)
    {
        Cs.Experience = std::min(Cs.Experience, Cs.NextExperienceLevel - 1);
    }

    // Check Stat Points remaining makes sense
    if (Cs.StatsLeft > 0)
    {
        std::uint32_t curTotalStatPoints = Cs.Strength + Cs.Dexterity + Cs.Vitality + Cs.Energy;
        if (curTotalStatPoints >= 4 * MAX_BASICSTATS)
        {
            Cs.StatsLeft = 0;
        }
        else if (curTotalStatPoints + Cs.StatsLeft > 4 * MAX_BASICSTATS)
        {
            Cs.StatsLeft = 4 * MAX_BASICSTATS - curTotalStatPoints;
        }
    }

    // Check remaining skill points
    if (Cs.SkillChoices > 0)
    {
        std::uint32_t maxSkills = MAX_SKILL_VALUE * NUM_OF_SKILLS;
        std::uint32_t curSkills = getSkillPointsUsed();
        if (curSkills >= maxSkills)
        {
            Cs.SkillChoices = 0;
        }
        else if (curSkills + Cs.SkillChoices > maxSkills)
        {
            Cs.SkillChoices = maxSkills - curSkills;
        }
    }
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getLevel() const
{
    return Cs.Level;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getLevelFromExperience() const
{
    return getLevelFromExperience(Cs.Experience);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getLevelFromExperience(std::uint32_t experience) const
{
    auto& minExpRequired = GetExperienceLevels(CharInfo.getClass());
    std::uint32_t expLevel = Cs.MaxLevel;
    while ((expLevel > 1) && (experience < minExpRequired[expLevel - 1]))
    {
        --expLevel;
    }

    return expLevel;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxLevel() const
{
    return Cs.MaxLevel;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getExperience() const
{
    return Cs.Experience;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxExperience() const
{
    return Cs.MaxExperience;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinExperience(std::uint32_t level) const
{
    return GetMinExperienceLevel(level, max_levels, CharInfo.getClass());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinExperienceLevel() const
{
    return Cs.MinExperienceLevel;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getNextExperience(std::uint32_t level) const
{
    return GetNextExperienceLevel(level, max_levels, CharInfo.getClass());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getNextExperienceLevel() const
{
    return Cs.NextExperienceLevel;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxGoldInBelt() const
{
    return Cs.MaxGoldInBelt;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxGoldInStash() const
{
    return Cs.MaxGoldInStash;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinStrength() const
{
    return min_strength;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getStrength() const
{
    return Cs.Strength;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinEnergy() const
{
    return min_energy;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getDexterity() const
{
    return Cs.Dexterity;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinDexterity() const
{
    return min_dexterity;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinVitality() const
{
    return min_vitality;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxHitPoints() const
{
    return min_hit_points;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxStamina() const
{
    return min_stamina;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxMana() const
{
    return min_mana;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getTotalStartStatPoints() const
{
    return min_stat_points_used;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getTotalStatPoints() const
{
    return getStatPointsUsed() + getStatLeft();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getStatPointsUsed() const
{
    return std::max(Cs.Vitality + Cs.Energy + Cs.Dexterity + Cs.Strength, min_stat_points_used);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getStatLeft() const
{
    return Cs.StatsLeft;
}
//---------------------------------------------------------------------------
const d2ce::SkillType& d2ce::CharacterStats::getSkill(std::uint16_t skill) const
{
    return CharClassHelper::getSkillByClass(CharInfo.getClass(), skill);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::CharacterStats::getSkillId(std::uint16_t skill) const
{
    const auto& skillInfo = getSkill(skill);
    return skillInfo.id;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::CharacterStats::getSkillPoints(std::uint16_t skill) const
{
    if (skill >= NUM_OF_SKILLS)
    {
        if (isPD2Format())
        {
            auto pd2Skill = skill - NUM_OF_SKILLS;
            if (pd2Skill >= NUM_OF_PD2_SKILLS)
            {
                return 0;
            }

            return PD2Skills[pd2Skill];
        }

        return 0;
    }

    return Skills[skill];
}
//---------------------------------------------------------------------------
std::array<std::uint8_t, d2ce::NUM_OF_SKILLS>& d2ce::CharacterStats::getSkills()
{
    return Skills;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getTotalSkillPoints() const
{
    return getSkillPointsUsed() + getSkillChoices();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getSkillPointsUsed() const
{
    std::uint32_t skillUsed = 0;
    for (std::uint32_t i = 0; i < NUM_OF_SKILLS; ++i)
    {
        skillUsed += Skills[i];
    }

    if (isPD2Format())
    {
        for (std::uint32_t i = 0; i < NUM_OF_PD2_SKILLS; ++i)
        {
            skillUsed += PD2Skills[i];
        }
    }

    return skillUsed;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getSkillChoices() const
{
    return Cs.SkillChoices;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::areSkillsMaxed() const
{
    for (std::uint32_t i = 0; i < NUM_OF_SKILLS; ++i)
    {
        if (Skills[i] < MAX_SKILL_VALUE)
        {
            return false;
        }
    }

    if (isPD2Format())
    {
        for (std::uint32_t i = 0; i < NUM_OF_PD2_SKILLS; ++i)
        {
            if (PD2Skills[i] < MAX_SKILL_VALUE)
            {
                return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::maxSkills()
{
    Skills.fill(MAX_SKILL_VALUE);

    if (isPD2Format())
    {
        PD2Skills.fill(MAX_SKILL_VALUE);
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getClassName(std::int16_t idx) // localized name
{
    const auto& charClassInfo = GetCharClassInfo(idx);
    return charClassInfo.ClassName;
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getClassName(EnumCharClass charClass) // localized name
{
    const auto& charClassInfo = GetCharClassInfo(charClass);
    return charClassInfo.ClassName;
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getClassName(const std::string& classCode) // localized name
{
    const auto& charClassInfo = GetCharClassInfo(classCode);
    return charClassInfo.ClassName;
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getClassCode(std::int16_t idx)
{
    const auto& charClassInfo = GetCharClassInfo(idx);
    return charClassInfo.Code;
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getClassCode(EnumCharClass charClass)
{
    const auto& charClassInfo = GetCharClassInfo(charClass);
    return charClassInfo.Code;
}
//---------------------------------------------------------------------------
bool d2ce::CharClassHelper::getEnumCharClassByName(const std::string& name, d2ce::EnumCharClass& value) // localized name
{
    auto iter = s_CharClassNameMap.find(name);
    if (iter == s_CharClassNameMap.end())
    {
        return false;
    }

    auto iter2 = s_CharClassInfo.find(iter->second);
    if (iter2 == s_CharClassInfo.end())
    {
        return false;
    }

    return getEnumCharClassByCode(iter2->second.Code, value);
}
//---------------------------------------------------------------------------
bool d2ce::CharClassHelper::getEnumCharClassByIndex(const std::string& name, EnumCharClass& value) // non-localized name
{
    auto iter = s_CharClassEnumNameMap.find(name);
    if (iter == s_CharClassEnumNameMap.end())
    {
        return false;
    }

    value = iter->second;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::CharClassHelper::getEnumCharClassByCode(const std::string& classCode, EnumCharClass& value)
{
    auto iter = s_CharClassCodeMap.find(classCode);
    if (iter == s_CharClassCodeMap.end())
    {
        return false;
    }

    value = iter->second;
    return true;
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getStrAllSkills(std::int16_t idx)
{
    const auto& charClassInfo = GetCharClassInfo(idx);
    return charClassInfo.StrAllSkills;
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getStrAllSkills(EnumCharClass charClass)
{
    const auto& charClassInfo = GetCharClassInfo(charClass);
    return charClassInfo.StrAllSkills;
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getStrSkillTab(std::int16_t tab, std::int16_t idx)
{
    static std::string badValue;
    const auto& charClassInfo = GetCharClassInfo(idx);
    switch (tab)
    {
    case 0:
        return charClassInfo.StrSkillTab1;

    case 1:
        return charClassInfo.StrSkillTab2;

    case 2:
        return charClassInfo.StrSkillTab3;

    default:
        return badValue;
    }
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getStrSkillTab(std::int16_t tab, EnumCharClass charClass)
{
    auto iter = s_CharClassEnumMap.find(charClass);
    if (iter == s_CharClassEnumMap.end())
    {
        static std::string badValue;
        return badValue;
    }

    return getStrSkillTab(tab, iter->second);
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getStrClassOnly(std::int16_t idx)
{
    const auto& charClassInfo = GetCharClassInfo(idx);
    return charClassInfo.StrClassOnly;
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getStrClassOnly(EnumCharClass charClass)
{
    const auto& charClassInfo = GetCharClassInfo(charClass);
    return charClassInfo.StrClassOnly;
}
//---------------------------------------------------------------------------
const std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>>& d2ce::CharClassHelper::getSklTreeTab(std::int16_t tab, std::int16_t idx)
{
    static std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>> badValue;
    const auto& charClassInfo = GetCharClassInfo(idx);
    switch (tab)
    {
    case 0:
        return charClassInfo.SklTreeTab1;

    case 1:
        return charClassInfo.SklTreeTab2;

    case 2:
        return charClassInfo.SklTreeTab3;

    default:
        return badValue;
    }
}
//---------------------------------------------------------------------------
const std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>>& d2ce::CharClassHelper::getSklTreeTab(std::int16_t tab, EnumCharClass charClass)
{
    auto iter = s_CharClassEnumMap.find(charClass);
    if (iter == s_CharClassEnumMap.end())
    {
        static std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>> badValue;
        return badValue;
    }

    return getSklTreeTab(tab, iter->second);
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getSkillTabName(std::int16_t tab, std::int16_t idx)
{
    static std::string badValue;
    const auto& charClassInfo = GetCharClassInfo(idx);
    switch (tab)
    {
    case 0:
        return charClassInfo.StrSklTreeTab1;

    case 1:
        return charClassInfo.StrSklTreeTab2;

    case 2:
        return charClassInfo.StrSklTreeTab3;

    default:
        return badValue;
    }
}
//---------------------------------------------------------------------------
const std::string& d2ce::CharClassHelper::getSkillTabName(std::int16_t tab, EnumCharClass charClass)
{
    auto iter = s_CharClassEnumMap.find(charClass);
    if (iter == s_CharClassEnumMap.end())
    {
        static std::string badValue;
        return badValue;
    }

    return getSkillTabName(tab, iter->second);
}
//---------------------------------------------------------------------------
const d2ce::SkillType& d2ce::CharClassHelper::getSkillById(std::uint16_t skill)
{
    auto iter = s_SkillInfoMap.find(skill);
    if (iter != s_SkillInfoMap.end())
    {
        return iter->second.Skill;
    }

    static d2ce::SkillType badValue;
    return badValue;
}
//---------------------------------------------------------------------------
const d2ce::SkillType& d2ce::CharClassHelper::getSkillByClass(EnumCharClass charClass, std::uint16_t skill)
{
    static d2ce::SkillType badValue;
    auto iterIdx = s_CharClassEnumMap.find(charClass);
    if (iterIdx == s_CharClassEnumMap.end())
    {
        return badValue;
    }

    auto iter = s_CharClassInfo.find(iterIdx->second);
    if (iter == s_CharClassInfo.end())
    {
        return badValue;
    }

    const auto& skills = iter->second.Skills;
    if (skills.size() <= skill)
    {
        return badValue;
    }

    return getSkillById(std::uint16_t(skills[skill]));
}
//---------------------------------------------------------------------------
std::string d2ce::CharClassHelper::getSkillIndexById(std::uint16_t id)
{
    const auto& skill = getSkillById(id);
    return skill.index;
}
//---------------------------------------------------------------------------
std::string d2ce::CharClassHelper::getSkillNameById(std::uint16_t id)
{
    const auto& skill = getSkillById(id);
    return skill.name;
}
//---------------------------------------------------------------------------
std::string d2ce::CharClassHelper::getSkillNameByClass(EnumCharClass charClass, std::uint16_t skill)
{
    const auto& skillInfo = getSkillByClass(charClass, skill);
    return skillInfo.name;
}
//---------------------------------------------------------------------------
const d2ce::SkillType& d2ce::CharClassHelper::getSkillByIndex(const std::string& index)
{
    auto iter = s_SkillIndexMap.find(index);
    if (iter != s_SkillIndexMap.end())
    {
        return getSkillById(iter->second);
    }

    // There is a bug in some case for the "Enchant" skill that shows up as "enchant"
    // in the txt properties file.  To cover this case, if the skill index lookup fails
    // search for the description (it's the lower case version of index) and get the skill
    // via that method
    auto iterDesc = s_SkillDescMap.find(index);
    if (iterDesc != s_SkillDescMap.end())
    {
        if (!iterDesc->second.empty())
        {
            if (iterDesc->second.size() > 1)
            {
                // Currenlty only FireStorm and DiabWall share a description.
                // do a case insensitive compare of the skill index to determine the right one.
                for (const auto& skill : iterDesc->second)
                {
                    auto iterSkill = s_SkillInfoMap.find(skill);
                    if (iterSkill != s_SkillInfoMap.end())
                    {
                        if (_stricmp(index.c_str(), iterSkill->second.Skill.index.c_str()) == 0)
                        {
                            return iterSkill->second.Skill;
                        }
                    }
                }
            }
            else
            {
                // must be the right skill
                return getSkillById(iterDesc->second.front());
            }
        }
    }

    static d2ce::SkillType badValue;
    return badValue;
}
//---------------------------------------------------------------------------