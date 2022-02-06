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

namespace d2ce
{
    constexpr std::array<std::uint8_t, 2> ITEM_MARKER = { 0x4A, 0x4D };        // alternatively "JM"

    // item version
    enum class EnumItemVersion : std::uint32_t
    {
        v100, // v1.00 - v1.03 item
        v104, // v1.04 - v1.06 item
        v107, // v1.07 item
        v108, // v1.08/1.09 normal or expansion
        v110, // v1.10 normal or expansion
        v115, // v1.15 Diable II: Resurrected
    };

    enum class EnumItemType : std::uint8_t
    {
        Armor = 1,
        Shield,
        Weapon,
        Other
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

    enum class EnumEquippedId : std::uint8_t
    {
        NONE = 0,
        HEAD,
        NECK,
        TORSO,
        HAND_RIGHT,
        HAND_LEFT,
        RIGHT_FINGER,
        LEFT_FINGER,
        WAIST,
        FEET,
        HANDS,
        ALT_HAND_RIGHT,
        ALT_HAND_LEFT,
    };

    enum class EnumItemLocation : std::uint8_t
    {
        STORED = 0x00,
        EQUIPPED = 0x01,
        BELT = 0x02,
        BUFFER = 0x04,
        SOCKET = 0x06,
    };

    enum class EnumAltItemLocation : std::uint8_t
    {
        UKNOWN = 0x00,
        INVENTORY = 0x01,
        HORADRIC_CUBE = 0x04,
        STASH = 0x05
    };
}
//---------------------------------------------------------------------------
