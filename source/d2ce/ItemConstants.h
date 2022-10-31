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
        v100,  // v1.00 - v1.03 item
        v104,  // v1.04 - v1.06 item
        v107,  // v1.07 item
        v108,  // v1.08 item
        v109,  // v1.09 item
        v110,  // v1.10 - v1.14d item
        v100R, // v1.0.x - v1.1.x Diablo II: Resurrected item
        v120,  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        v140,  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    };

    constexpr EnumItemVersion APP_ITEM_VERSION = EnumItemVersion::v140; // default item version used by application

    // default game version used by application
    // refers to the "version" column in resource files used internally
    //   0 = pre v1.08
    //   1 - Non-Expansion (post v1.08) (classic and LoD).
    // 100 - Expansion LoD
    constexpr std::uint16_t APP_ITEM_GAME_VERSION = 100; 


    constexpr std::uint32_t MAX_KEY_QUANTITY_100 = 6ui32;   // max # of keys stacked (verion 1.06 and lower)
    constexpr std::uint32_t MAX_GLD_QUANTITY = 4095ui32;    // max gld amount
    constexpr std::uint32_t MAX_STACKED_QUANTITY = 511ui32; // max # of items in a stack

    enum class EnumItemQuality : std::uint8_t
    {
        UNKNOWN = 0,
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
        RIGHT_ARM,
        LEFT_ARM,
        RIGHT_RING,
        LEFT_RING,
        BELT,
        FEET,
        GLOVES,
        ALT_RIGHT_ARM,
        ALT_LEFT_ARM,
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
        UNKNOWN = 0x00,
        INVENTORY = 0x01,
        HORADRIC_CUBE = 0x04,
        STASH = 0x05
    };

    enum class EnumItemInventory : std::uint8_t
    {
        UNKNOWN = 0,
        PLAYER,
        CORPSE,
        MERCENARY,
        GOLEM,
        SHARED_STASH,
        BUFFER
    };
}
//---------------------------------------------------------------------------
