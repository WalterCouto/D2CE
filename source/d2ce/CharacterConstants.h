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

#include "Constants.h"
#include "ExperienceConstants.h"
#include "bitmask.hpp"
#include <system_error>

namespace d2ce
{
    const static std::string jsonIndentStr("    ");

    constexpr std::uint32_t MAX_FILE_SIZE = 8192;

    constexpr std::uint32_t MIN_START_POS = 48;

    constexpr std::uint32_t HEADER_LENGTH = 4;
    constexpr std::uint32_t NAME_LENGTH = 16;       // character name including terminating NULL

    // character version
    enum class EnumCharVersion : std::uint32_t { v100 = 0x47, v107 = 0x57, v108 = 0x59, v109 = 0x5C, v110 = 0x60, v115 = 0x61 };

    constexpr EnumCharVersion APP_CHAR_VERSION = EnumCharVersion::v115; // default version used by application

    // character class
    enum class EnumCharClass : std::uint8_t { Amazon, Sorceress, Necromancer, Paladin, Barbarian, Druid, Assassin };

    const std::string ClassNames[NUM_OF_CLASSES] = { "Amazon", "Sorceress", "Necromancer", "Paladin", "Barbarian", "Druid", "Assassin"};

    // character status
    enum class EnumCharStatus : std::uint8_t { NoDeaths, Hardcore = 0x04, Resurrected = 0x08, Expansion = 0x20, Ladder = 0x40, Dead = 0x0C };
    BITMASK_DEFINE_VALUE_MASK(EnumCharStatus, 0xFF);

    // character title
    // Non-Expansion has upper 2 bits set (00, 01, 10 or 11)
    // Expansion has both the upper and lower 2 bits set to the same 2 bit value (00, 01, 10 or 11)
    // Thus we can use the upper bits as the main enumeration of title and the lower two bits are set if we are an Expansion character
    enum class EnumCharTitle : std::uint8_t
    {
        None,
        Slayer = 0x01, Champion = 0x02, MPatriarch = 0x03, // Expansion Titles
        SirDame = 0x04, LordLady = 0x08, BaronBaroness = 0x0C
    };
    BITMASK_DEFINE_VALUE_MASK(EnumCharTitle, 0x0F);

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