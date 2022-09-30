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
#include "ExperienceConstants.h"
#include "bitmask.hpp"
#include <system_error>
#include <array>
#include <optional>

namespace d2ce
{
    const static std::string jsonIndentStr("  ");

    constexpr std::uint32_t MAX_FILE_SIZE = 8192;

    constexpr std::uint32_t MIN_START_POS = 48;

    constexpr std::uint32_t HEADER_LENGTH = 4;
    constexpr std::uint32_t NAME_LENGTH = 16;        // character name including terminating NULL
    constexpr std::uint32_t APPEARANCES_LENGTH = 32;
    constexpr std::uint32_t D2R_APPEARANCES_LENGTH = 48;

    // character version
    enum class EnumCharVersion : std::uint32_t { v100 = 0x47, v107 = 0x57, v108 = 0x59, v109 = 0x5C, v110 = 0x60, v100R = 0x61, v120 = 0x62, v140 = 0x63};

    constexpr EnumCharVersion APP_CHAR_VERSION = EnumCharVersion::v140; // default version used by application

    // character class
    enum class EnumCharClass : std::uint8_t { Amazon, Sorceress, Necromancer, Paladin, Barbarian, Druid, Assassin };

    // character status
    enum class EnumCharStatus : std::uint8_t { NoDeaths, Hardcore = 0x04, Died = 0x08, Expansion = 0x20, Ladder = 0x40, Dead = 0x0C };
    BITMASK_DEFINE_VALUE_MASK(EnumCharStatus, 0xFF);

    // character title an Expansion character
    enum class EnumCharTitle : std::uint8_t
    {
        None = 0, SirDame, LordLady, BaronBaroness
    };

    //---------------------------------------------------------------------------
    // error codes
    enum class CharacterErrc
    {
        // no 0
        InvalidHeader = 1,
        CannotOpenFile,
        InvalidChecksum,
        InvalidActsInfo,
        InvalidCharStats,
        InvalidCharSkills,
        InvalidItemInventory,
        FileRenameError,
        AuxFileRenameError,
    };

    struct CharacterErrCategory : std::error_category
    {
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    class ITxtDocument
    {
    public:
        virtual ~ITxtDocument() = default;
        virtual std::string GetRowName(SSIZE_T pRowIdx) const = 0;
        virtual std::vector<std::string> GetColumnNames() const = 0;
        virtual std::string GetColumnName(SSIZE_T pColumnIdx) const = 0;
        virtual size_t GetRowCount() const = 0;
        virtual size_t GetColumnCount() const = 0;
        virtual SSIZE_T GetColumnIdx(const std::string& pColumnName) const = 0;
        virtual std::string GetCellString(size_t pColumnIdx, size_t pRowIdx) const = 0;
        virtual std::uint64_t GetCellUInt64(size_t pColumnIdx, size_t pRowIdx) const = 0;
        virtual std::uint32_t GetCellUInt32(size_t pColumnIdx, size_t pRowIdx) const = 0;
        virtual std::uint16_t GetCellUInt16(size_t pColumnIdx, size_t pRowIdx) const = 0;

        // used for string localization loads
        virtual size_t GetRowValues(size_t rowIdx, std::string& index, std::map<std::string, std::string>& stringCols) const = 0;
        
    };

    //---------------------------------------------------------------------------
    class ITxtReader
    {
    public:
        virtual ~ITxtReader() {};
        virtual std::unique_ptr<ITxtDocument> GetStringTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetExpansionStringTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetPatchStringTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetCharStatsTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetPlayerClassTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetExperienceTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetHirelingTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetItemStatCostTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetItemTypesTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetPropertiesTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetGemsTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetBeltsTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetArmorTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetWeaponsTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetMiscTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetMagicPrefixTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetMagicSuffixTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetRarePrefixTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetRareSuffixTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetUniqueItemsTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetSetsTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetSetItemsTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetRunesTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetSkillsTxt() const = 0;
        virtual std::unique_ptr<ITxtDocument> GetSkillDescTxt() const = 0;
    };

    const ITxtReader& getDefaultTxtReader();

    struct ClassSkillType
    {
        EnumCharClass charClass = EnumCharClass::Amazon; // To what character class is this skill assigned (null for skill not specific to a character)
        std::uint16_t index = 0;                         // What index in class skill array does this skill refer to
        std::uint16_t iconIndex = MAXUINT16;             // the character icon index used by this skill
    };

    struct SkillType
    {
        std::string index;            // The ID pointer used to reference this skill
        std::uint16_t id = MAXUINT16; // The actual ID number of the skill, this is what the pointer actually points at, this must be a unique number
        std::string name;             // What string will be displayed in-game for this skill
        std::string longName;

        std::optional<ClassSkillType> classInfo;  // additional information about class-specific skills

        std::uint16_t reqLevel = 1ui16;     // The minimum character level required in order to put a point in this skill
        std::vector<std::string> reqSkills; // The ID pointers (index) of the other skills you need in order to put your first point in this skill
    };

    //---------------------------------------------------------------------------
    class CharClassHelper
    {
    public:
        static const std::string& getClassName(std::int16_t idx); // localized name
        static const std::string& getClassName(EnumCharClass charClass); // localized name
        static const std::string& getClassName(const std::string& classCode); // localized name
        static const std::string& getClassCode(std::int16_t idx);
        static const std::string& getClassCode(EnumCharClass charClass);

        static bool getEnumCharClassByName(const std::string& name, EnumCharClass& value); // localized name
        static bool getEnumCharClassByIndex(const std::string& name, EnumCharClass& value); // non-localized name
        static bool getEnumCharClassByCode(const std::string& classCode, EnumCharClass& value);

        static const std::string& getStrAllSkills(std::int16_t idx);
        static const std::string& getStrAllSkills(EnumCharClass charClass);
        static const std::string& getStrSkillTab(std::int16_t tab, std::int16_t idx);
        static const std::string& getStrSkillTab(std::int16_t tab, EnumCharClass charClass);
        static const std::string& getStrClassOnly(std::int16_t idx);
        static const std::string& getStrClassOnly(EnumCharClass charClass);

        // skill tab names sorted by ascending tab number
        static const std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>>& getSklTreeTab(std::int16_t tab, std::int16_t idx);
        static const std::map<std::uint16_t, std::map<std::uint16_t, std::uint16_t>>& getSklTreeTab(std::int16_t tab, EnumCharClass charClass); // The skill ids for the skill tab, organized by row and column
        static const std::string& getSkillTabName(std::int16_t tab, std::int16_t idx);
        static const std::string& getSkillTabName(std::int16_t tab, EnumCharClass charClass);

        static const SkillType& getSkillById(std::uint16_t skill);
        static const SkillType& getSkillByClass(EnumCharClass charClass, std::uint16_t skill);
        static std::string getSkillIndexById(std::uint16_t id);
        static std::string getSkillNameById(std::uint16_t id);
        static std::string getSkillNameByClass(EnumCharClass charClass, std::uint16_t skill);
        static const SkillType& getSkillByIndex(const std::string& index);
    };
}
//---------------------------------------------------------------------------