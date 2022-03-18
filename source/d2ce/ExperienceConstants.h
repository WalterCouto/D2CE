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

#include <cstdint>
#include <array>

namespace d2ce
{
    constexpr std::uint32_t MAX_BASICSTATS = 0x03FF;     // strength, dexterity, vitality, energy max value (10 bits)
    constexpr std::uint32_t MAX_LMS = 0x1FFFFF;          // life, mana, stamina max value (21 bits)
    constexpr std::uint32_t LMS_MASK = 0x1FFF00;         // mask of significant bits of LMS values
    constexpr std::uint32_t MAX_SKILL_CHOICES = 0xFF;    // max # of skill choices left to use
}
//---------------------------------------------------------------------------
