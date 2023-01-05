/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
    Copyright (C) 2021-2023 Walter Couto

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

#include <cstdint>

namespace d2ce
{
    // difficulty levels
    enum class EnumDifficulty : std::uint8_t { Normal = 0, Nightmare = 1, Hell = 2 };
    enum class EnumAct : std::uint8_t { I = 0, II = 1, III = 2, IV = 3, V = 4 };

    constexpr std::uint32_t NUM_OF_CLASSES = 7;
    constexpr std::uint32_t NUM_OF_CLASSES_NO_EXPANSION = 5;
    constexpr std::uint32_t NUM_OF_DIFFICULTY = 3;
    constexpr std::uint32_t NUM_OF_SKILL_HOTKEYS = 16;
    constexpr std::uint32_t NUM_OF_TITLES = 4;
}
//---------------------------------------------------------------------------
