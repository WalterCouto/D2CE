/*
    Diablo II Character Editor
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

#include <cstdint>
#include "bitmask.hpp"

namespace d2ce
{
    constexpr std::uint32_t GOLD_IN_BELT_LIMIT = 990000;
    constexpr std::uint32_t GOLD_IN_STASH_LIMIT = 2500000;

    // This bit field indicates the presence or absence of each particular statistic.
    // If the bit is 0, the corresponding statistic is zero and not stored in the file.
    // If the bit is 1, the statistic has a long value stored in the file.  
    // Note that many of the statistics are never zero, so they will always be present; but there is a bit assigned to them nonetheless
    enum class EnumCharStatInfo : std::uint16_t
    {
        Strength = 0x0001,
        Energy = 0x0002,
        Dexterity = 0x0004,
        Vitality = 0x0008,
        StatPoints = 0x0010,
        SkillChoices = 0x0020,
        CurLife = 0x0040,
        MaxLife = 0x0080,
        CurMana = 0x0100,
        MaxMana = 0x0200,
        CurStamina = 0x0400,
        MaxStamina = 0x0800,
        Level = 0x1000,
        Experience = 0x2000,
        GoldInBelt = 0x4000,
        GoldInStash = 0x8000,
        All = 0xFFFF
    };
    BITMASK_DEFINE_VALUE_MASK(EnumCharStatInfo, 0xFFFF);
}
//---------------------------------------------------------------------------
