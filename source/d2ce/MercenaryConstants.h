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

#pragma once

#include "Constants.h"

namespace d2ce
{
    constexpr std::uint32_t MERC_MAX_EXPERIENCE = 3600000000ui32; // experience max value (game limit)
    constexpr std::uint32_t MERC_NUM_OF_LEVELS = 98ui32;          // max merc level (game limit)

    enum class EnumMercenaryClass : std::uint8_t { None = 0, RogueScout, DesertMercenary, IronWolf, Barbarian};
}
//---------------------------------------------------------------------------
