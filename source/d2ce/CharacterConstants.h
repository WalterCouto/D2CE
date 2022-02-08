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

namespace d2ce
{
    const static std::string jsonIndentStr("  ");

    constexpr std::uint32_t MAX_FILE_SIZE = 8192;

    constexpr std::uint32_t MIN_START_POS = 48;

    constexpr std::uint32_t HEADER_LENGTH = 4;
    constexpr std::uint32_t NAME_LENGTH = 16;        // character name including terminating NULL
    constexpr std::uint32_t APPEARANCES_LENGTH = 32; // character name including terminating NULL

    // character version
    enum class EnumCharVersion : std::uint32_t { v100 = 0x47, v107 = 0x57, v108 = 0x59, v109 = 0x5C, v110 = 0x60, v115 = 0x61 };

    constexpr EnumCharVersion APP_CHAR_VERSION = EnumCharVersion::v115; // default version used by application

    // character class
    enum class EnumCharClass : std::uint8_t { Amazon, Sorceress, Necromancer, Paladin, Barbarian, Druid, Assassin };

    const std::array<std::string, NUM_OF_CLASSES> ClassNames = { "Amazon", "Sorceress", "Necromancer", "Paladin", "Barbarian", "Druid", "Assassin"};

    // character status
    enum class EnumCharStatus : std::uint8_t { NoDeaths, Hardcore = 0x04, Died = 0x08, Expansion = 0x20, Ladder = 0x40, Dead = 0x0C };
    BITMASK_DEFINE_VALUE_MASK(EnumCharStatus, 0xFF);

    // character title an Expansion character
    enum class EnumCharTitle : std::uint8_t
    {
        None = 0, SirDame, LordLady, BaronBaroness
    };

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
}
//---------------------------------------------------------------------------