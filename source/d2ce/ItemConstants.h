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

namespace d2ce
{
    // values that are found in the various item fields
    constexpr std::uint8_t BELT = 0x02;
    constexpr std::uint8_t GLUED_IN_SOCKET = 0x06;

    // item version
    enum class EnumItemVersion : std::uint32_t
    {
        v100, // v1.00 - v1.03 item
        v104, // v1.04 - v1.06 item
        v107, // v1.07 item
        v108, // v1.08/1.09 normal or expansion
        v110, // v1.10 normal or expansion
        v115, // v1.15 Diable II Resurrected
    };

    enum class EnumItemQuality : std::uint8_t
    {
        UKNOWN = 0,
        INFERIOR = 0x01,
        NORMAL,
        SUPERIOR,
        MAGIC,
        SET,
        RARE,
        UNIQUE,
        CRAFT,
        TEMPERED
    };
}
//---------------------------------------------------------------------------
