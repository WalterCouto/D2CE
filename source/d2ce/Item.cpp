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

#include "pch.h"
#include <map>
#include <bitset>
#include <sstream>
#include "Item.h"
#include "helpers/ItemHelpers.h"
#include "SkillConstants.h"

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint16_t MAX_TOME_QUANTITY    =   20; // max # of scrolls in tome
    constexpr std::uint16_t MAX_KEY_QUANTITY_100 =    6; // max # of keys stacked (verion 1.06 and lower)
    constexpr std::uint16_t MAX_KEY_QUANTITY     =   12; // max # of keys stacked
    constexpr std::uint16_t MAX_GLD_QUANTITY     = 4095; // max gld amount
    constexpr std::uint16_t MAX_STACKED_QUANTITY =  511; // max # of items in a stack
    constexpr std::uint16_t MAX_DURABILITY       = 0xFF; // max durability of an item (0 is Indestructible)

    constexpr std::array<std::uint8_t, 2> MERC_ITEM_MARKER  = { 0x6A, 0x66 };  // alternatively "jf"
    constexpr std::array<std::uint8_t, 2> GOLEM_ITEM_MARKER = { 0x6B, 0x66 };  // alternatively "jk"

    constexpr std::uint32_t MIN_START_STATS_POS = 641;

    constexpr std::uint32_t IS_IDENTIFIED_FLAG_OFFSET   =  4;
    constexpr std::uint32_t IS_DISABLED_FLAG_OFFSET     =  8;
    constexpr std::uint32_t IS_SOCKETED_FLAG_OFFSET     = 11;
    constexpr std::uint32_t IS_NEW_FLAG_OFFSET          = 13;
    constexpr std::uint32_t IS_BAD_EQUIPPED_FLAG_OFFSET = 14;
    constexpr std::uint32_t IS_EAR_FLAG_OFFSET          = 16;
    constexpr std::uint32_t IS_STARTER_FLAG_OFFSET      = 17;
    constexpr std::uint32_t IS_SIMPLE_FLAG_OFFSET       = 21;
    constexpr std::uint32_t IS_ETHEREAL_FLAG_OFFSET     = 22;
    constexpr std::uint32_t IS_PERSONALIZED_FLAG_OFFSET = 24;
    constexpr std::uint32_t IS_RUNEWORD_FLAG_OFFSET     = 26;

    constexpr std::uint32_t ITEM_TYPE_BIT_OFFSET = 19; // for v1.00 to 1.06, these two bits tell us the item version (non-zero means 1.04 - 1.06 items)

    constexpr std::uint32_t QUALITY_BIT_OFFSET     = 134;
    constexpr std::uint32_t QUALITY_BIT_OFFSET_100 =  65;
    constexpr std::uint32_t QUALITY_BIT_OFFSET_104 =  97;

    constexpr std::uint32_t QUANTITY_BIT_OFFSET_100 = 69;
    constexpr std::uint32_t QUANTITY_BIT_OFFSET_104 = 101;

    constexpr std::uint16_t SOCKET_COUNT_NUM_BITS            =  4;
    constexpr std::uint16_t REAL_DATA_NUM_BITS               = 96;
    constexpr std::uint16_t DEFENSE_RATING_NUM_BITS          = 11;
    constexpr std::uint16_t DEFENSE_RATING_NUM_BITS_108      = 10; // 1.09 or older
    constexpr std::uint16_t DURABILITY_MAX_NUM_BITS          =  8;
    constexpr std::uint16_t DURABILITY_CURRENT_READ_NUM_BITS =  8;
    constexpr std::uint16_t DURABILITY_CURRENT_NUM_BITS      =  9;
    constexpr std::uint16_t DURABILITY_CURRENT_NUM_BITS_108  =  8; // 1.09 or older
    constexpr std::uint16_t STACKABLE_NUM_BITS               =  9;
    constexpr std::uint16_t GLD_STACKABLE_NUM_BITS           = 12;
    constexpr std::uint16_t RUNEWORD_ID_NUM_BITS             = 12;
    constexpr std::uint16_t RUNEWORD_PADDING_NUM_BITS        =  4;
    constexpr std::uint16_t MAGICAL_AFFIX_NUM_BITS           = 11;
    constexpr std::uint16_t SET_UNIQUE_ID_NUM_BITS           = 12;
    constexpr std::uint16_t INFERIOR_SUPERIOR_ID_NUM_BITS    =  3;
    constexpr std::uint16_t RARE_CRAFTED_ID_NUM_BITS         =  8;
    constexpr std::uint16_t PROPERTY_ID_NUM_BITS             =  9;

    constexpr std::uint16_t ITEM_V100_NUM_BITS           = 216;
    constexpr std::uint16_t ITEN_V100_UNIQUE_ID_NUM_BITS =   8;
    constexpr std::uint16_t ITEM_V104_EAR_NUM_BITS       = 208;
    constexpr std::uint16_t ITEM_V104_SM_NUM_BITS        = 120;
    constexpr std::uint16_t ITEM_V104_EX_NUM_BITS        = 248;

    constexpr std::uint32_t ITEM_V100_COORDINATES_BIT_OFFSET     = 120;
    constexpr std::uint32_t ITEM_V100_SPECIALITEMCODE_BIT_OFFSET = 129;
    constexpr std::uint32_t ITEM_V100_EAR_COORDINATES_BIT_OFFSET =  64;
    constexpr std::uint32_t ITEM_V104_SM_COORDINATES_BIT_OFFSET  =  64;
    constexpr std::uint32_t ITEM_V104_EX_COORDINATES_BIT_OFFSET  = 152;
    constexpr std::uint32_t ITEM_V104_EX_UNIQUECODE_BIT_OFFSET   = 161;
    constexpr std::uint32_t ITEM_V104_EAR_COORDINATES_BIT_OFFSET =  64;

    constexpr std::uint32_t ITEM_V100_CONTAINER_BIT_OFFSET     = 200;
    constexpr std::uint32_t ITEM_V100_EAR_CONTAINER_BIT_OFFSET =  80;
    constexpr std::uint32_t ITEM_V104_EX_CONTAINER_BIT_OFFSET  = 232;

    constexpr std::uint32_t ITEM_V100_ITEMCODE_BIT_OFFSET      =  64;
    constexpr std::uint32_t ITEM_V100_EAR_TYPECODE_BIT_OFFSET  =  74;
    constexpr std::uint32_t ITEM_V100_TYPECODE_BIT_OFFSET      =  68;
    constexpr std::uint32_t ITEM_V100_DURABILITY_BIT_OFFSET    = 105;
    constexpr std::uint32_t ITEM_V100_DWA_BIT_OFFSET           = 139;
    constexpr std::uint32_t ITEM_V100_DWB_BIT_OFFSET           = 171;
    constexpr std::uint32_t ITEM_V104_EX_TYPECODE_BIT_OFFSET   =  58;
    constexpr std::uint32_t ITEM_V104_SM_ITEMCODE_BIT_OFFSET   =  80;
    constexpr std::uint32_t ITEM_V104_SM_TYPECODE_BIT_OFFSET   =  82;
    constexpr std::uint32_t ITEM_V104_EAR_ATTRIBE_BIT_OFFSET   =  82;
    constexpr std::uint32_t ITEM_V104_EX_DURABILITY_BIT_OFFSET = 145;
    constexpr std::uint32_t ITEM_V104_EX_DWA_BIT_OFFSET        = 171;
    constexpr std::uint32_t ITEM_V104_EX_DWB_BIT_OFFSET        = 203;
    
    constexpr std::uint32_t ITEM_V100_BITFIELD3_BIT_OFFSET       = 48;
    constexpr std::uint32_t ITEM_V100_NUM_SOCKETED_BIT_OFFSET    = 53;
    constexpr std::uint32_t ITEM_V100_LEVEL_BIT_OFFSET           = 56;
    constexpr std::uint32_t ITEM_V104_EX_EQUIP_ID_BIT_OFFSET     = 90;
    constexpr std::uint32_t ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET = 95;
    constexpr std::uint32_t ITEM_V104_EX_LEVEL_BIT_OFFSET        = 98;

    constexpr std::uint32_t ITEM_V107_EXTENDED_DATA_BIT_OFFSET   = 108;
    constexpr std::uint32_t ITEM_V107_START_BIT_OFFSET           =  23; // 16 bit for item header + 7 bits for the item version
    constexpr std::uint32_t ITEM_V115_LOCATION_BIT_OFFSET        =  35;
    constexpr std::uint32_t ITEM_V115_EQUIPPED_ID_BIT_OFFSET     =  38;
    constexpr std::uint32_t ITEM_V115_POSITION_BIT_OFFSET        =  42;
    constexpr std::uint32_t ITEM_V115_ALT_POSITION_ID_BIT_OFFSET =  50;
    constexpr std::uint32_t ITEM_V115_TYPE_CODE_BIT_OFFSET       =  53;

    constexpr std::uint16_t ATTRIBUTE_ID_INDESTRUCTIBLE = 152;

#define readtemp_bits(data,start,size) \
    ((*((std::uint64_t*) &(data)[(start) / 8]) >> ((start) & 7))& (((std::uint64_t)1 << (size)) - 1))

#define read_uint32_bits(start,size) \
    ((*((std::uint32_t *) &data[(start) / 8]) >> ((start) & 7))& (((std::uint32_t)1 << (size)) - 1))

#define read_uint64_bits(start,size) \
    ((*((std::uint64_t*) &data[(start) / 8]) >> ((start) & 7))& (((std::uint64_t)1 << (size)) - 1))

#define set_bit(start) \
    ((data[(start) / 8]) |= (std::uint8_t)(1ul << ((start) & 7)))

#define clear_bit(start) \
    ((data[(start) / 8]) &= ~(std::uint8_t)(1ul << ((start) & 7)))

    void SetFlagBit(const Json::Value& node, size_t bitNum, std::bitset<32>& flags)
    {
        if (bitNum >= flags.size())
        {
            return;
        }

        if (!node.isNull())
        {
            if (node.isBool())
            {
                flags[bitNum] = (node.asBool() ? 1 : 0);
            }
            else
            {
                flags[bitNum] = (std::uint16_t(node.asInt64()) != 0 ? 1 : 0);
            }
        }
    }
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
d2ce::Item::Item()
{
}
//---------------------------------------------------------------------------
d2ce::Item::Item(size_t itemsize) : data(itemsize)
{
}
d2ce::Item::Item(const Item& other)
{
    *this = other;
}
d2ce::Item::Item(EnumCharVersion version, EnumItemVersion itemVersion, std::array<std::uint8_t, 4>& strcode, bool isExpansion)
{
    static Item invalidItem;

    start_bit_offset = 0;
    FileVersion = version;

    // We only support simple items
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (!itemType.isSimpleItem())
    {
        *this = invalidItem;
        return;
    }

    // reserve enough space to reduce chance of reallocation (haven't seen an item size bigger then 80
    data.reserve(80);

    ItemVersion = itemVersion;
    std::uint16_t rawVersion = 5;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
    case EnumItemVersion::v107:
        rawVersion = 0;
        break;

    case EnumItemVersion::v108:
        rawVersion = isExpansion ? 100 : 1;
        break;

    case EnumItemVersion::v110:
        rawVersion = isExpansion ? 101 : 2;
        break;

    case EnumItemVersion::v115:
        rawVersion = isExpansion ? 5 : 4;
        break;
    }

    std::uint32_t value = 0;
    std::uint64_t value64 = 0;
    start_bit_offset = 0;
    size_t current_bit_offset = start_bit_offset;
    size_t max_bit_offset = current_bit_offset;

    if (version < EnumCharVersion::v115)
    {
        value = *((std::uint16_t*)ITEM_MARKER.data());
        setBits(current_bit_offset, ITEM_MARKER.size() * 8, value);
        start_bit_offset = current_bit_offset;
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // flags
    std::bitset<32> flags = 0;
    flags[IS_IDENTIFIED_FLAG_OFFSET] = 1;

    if (ItemVersion > EnumItemVersion::v100)
    {
        flags[IS_SIMPLE_FLAG_OFFSET] = 1;
    }

    if (FileVersion >= EnumCharVersion::v107)
    {
        flags[23] = 1; // unknown but always one as far as I have noticed
    }
    else if (ItemVersion == EnumItemVersion::v104)
    {
        // unknown but both 1 for v1.04 to v1.06 items as far as I have noticed
        flags[19] = 1;
        flags[20] = 1;
    }

    value = flags.to_ulong();
    if (!setBits(current_bit_offset, flags.size(), value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    std::uint32_t itemCode = 0;
    size_t bitSize = 0;
    if (ItemVersion < EnumItemVersion::v107)
    {
        switch (ItemVersion)
        {
        case EnumItemVersion::v100:
            // 27 bytes total
            data.resize((ITEM_V100_NUM_BITS + 7) / 8, 0);

            location_bit_offset = 58;
            alt_position_id_offset = 0;
            extended_data_offset = ITEM_V100_NUM_BITS;
            equipped_id_offset = ITEM_V100_BITFIELD3_BIT_OFFSET;
            nr_of_items_in_sockets_offset = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 2;
            type_code_offset = ITEM_V100_TYPECODE_BIT_OFFSET;
            durability_bit_offset = ITEM_V100_DURABILITY_BIT_OFFSET;
            position_offset = ITEM_V100_COORDINATES_BIT_OFFSET + 1;
            quality_bit_offset = start_bit_offset + QUALITY_BIT_OFFSET_100;
            quality_attrib_bit_offset = ITEM_V100_SPECIALITEMCODE_BIT_OFFSET;
            item_level_bit_offset = ITEM_V100_LEVEL_BIT_OFFSET;
            dwa_bit_offset = ITEM_V100_DWA_BIT_OFFSET;
            dwb_bit_offset = ITEM_V100_DWB_BIT_OFFSET;
            
            itemCode = ItemHelpers::getTypeCodev100(strcode);
            if (itemCode >= UINT16_MAX)
            {
                *this = invalidItem;
                return;
            }

            bitSize = 10;
            current_bit_offset = type_code_offset;
            if (!setBits(current_bit_offset, bitSize, itemCode))
            {
                *this = invalidItem;
                return;
            }

            value = static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::NORMAL);
            current_bit_offset = quality_bit_offset;
            bitSize = 3;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            value = 1;
            current_bit_offset = item_level_bit_offset;
            if (!setBits(current_bit_offset, 8, value))
            {
                *this = invalidItem;
                return;
            }

            value = ItemHelpers::generarateRandomDW();
            current_bit_offset = dwa_bit_offset;
            if (!setBits(current_bit_offset, 32, value))
            {
                *this = invalidItem;
                return;
            }

            value = ItemHelpers::generarateRandomDW();
            current_bit_offset = dwb_bit_offset;
            if (!setBits(current_bit_offset, 32, value))
            {
                *this = invalidItem;
                return;
            }

            item_end_bit_offset = ITEM_V100_NUM_BITS;
            max_bit_offset = item_end_bit_offset;
            break;

        case EnumItemVersion::v104:
            // 15 bytes total
            data.resize((ITEM_V104_SM_NUM_BITS + 7) / 8, 0);

            position_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET + 2;
            type_code_offset = ITEM_V104_SM_TYPECODE_BIT_OFFSET;
            extended_data_offset = ITEM_V104_SM_NUM_BITS;
            location_bit_offset = 0;
            equipped_id_offset = 0;
            alt_position_id_offset = 0;
            type_code_offset = 0;
            durability_bit_offset = 0;
            nr_of_items_in_sockets_offset = 0;
            quality_attrib_bit_offset = 0;
            item_level_bit_offset = 0;
            
            itemCode = *((std::uint32_t*)strcode.data());
            bitSize = 30;
            current_bit_offset = type_code_offset;
            if (!setBits(current_bit_offset, bitSize, itemCode))
            {
                *this = invalidItem;
                return;
            }

            item_end_bit_offset = ITEM_V104_SM_NUM_BITS;
            max_bit_offset = item_end_bit_offset;
            break;

        default: // should not happen
            *this = invalidItem;
            max_bit_offset = 0;
            break;
        }

        return;
    }

    value = rawVersion;
    bitSize = (FileVersion < EnumCharVersion::v115 ? 10 : 3);
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    location_bit_offset = current_bit_offset;
    value = static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::BUFFER);
    bitSize = 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    equipped_id_offset = current_bit_offset;
    value = 0;
    bitSize = 4;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    
    // position x/y
    position_offset = current_bit_offset;
    value = 0;
    bitSize = 4;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    value = 0;
    bitSize = 4;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    
    // alt position x/y
    alt_position_id_offset = current_bit_offset;
    value = static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::UNKNOWN);
    bitSize = 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // type code
    type_code_offset = current_bit_offset;
    bitSize = 32; 
    
    std::uint8_t numBitsSet = 0;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.0.6
        *this = invalidItem; // should not get here
        return; 

    case EnumItemVersion::v107: // v1.07
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10
        value = *((std::uint32_t*)strcode.data());
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        break;

    default: // v1.15+
        ItemHelpers::encodeItemCodev115(strcode, value64, numBitsSet);
        bitSize = numBitsSet;
        if (!setBits64(current_bit_offset, bitSize, value64))
        {
            *this = invalidItem;
            return;
        }
        break;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    extended_data_offset = max_bit_offset;

    current_bit_offset = extended_data_offset;
    value = 0;
    bitSize = 1;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    nr_of_items_in_sockets_offset = extended_data_offset;
    nr_of_items_in_sockets_bits = 1;
    item_end_bit_offset = max_bit_offset;
}
//---------------------------------------------------------------------------
bool d2ce::Item::writeItem(std::FILE* charfile)
{
    if (data.empty())
    {
        return true;
    }

    if (std::fwrite(&data[0], data.size(), 1, charfile) != 1)
    {
        std::fflush(charfile);
        return false;
    }

    // now write child items
    for (auto& item : SocketedItems)
    {
        if (!item.writeItem(charfile))
        {
            std::fflush(charfile);
            return false;
        }
    }

    std::fflush(charfile);
    return true;
}
//---------------------------------------------------------------------------
d2ce::Item::~Item()
{
}
//---------------------------------------------------------------------------
d2ce::Item& d2ce::Item::operator=(const Item& other)
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    data = other.data;
    SocketedItems = other.SocketedItems;

    FileVersion = other.FileVersion;
    ItemVersion = other.ItemVersion;
    start_bit_offset = other.start_bit_offset;
    location_bit_offset = other.location_bit_offset;
    equipped_id_offset = other.equipped_id_offset;
    position_offset = other.position_offset;
    alt_position_id_offset = other.alt_position_id_offset;
    type_code_offset = other.type_code_offset;
    extended_data_offset = other.extended_data_offset;
    quest_difficulty_offset = other.quest_difficulty_offset;
    nr_of_items_in_sockets_offset = other.nr_of_items_in_sockets_offset;
    nr_of_items_in_sockets_bits = other.nr_of_items_in_sockets_bits;
    item_id_bit_offset = other.item_id_bit_offset;
    item_level_bit_offset = other.item_level_bit_offset;
    quality_bit_offset = other.quality_bit_offset;
    multi_graphic_bit_offset = other.multi_graphic_bit_offset;
    autoAffix_bit_offset = other.autoAffix_bit_offset;
    quality_attrib_bit_offset = other.quality_attrib_bit_offset;
    runeword_id_bit_offset = other.runeword_id_bit_offset;
    personalized_bit_offset_marker = other.personalized_bit_offset_marker;
    personalized_bit_offset = other.personalized_bit_offset;
    tome_bit_offset = other.tome_bit_offset;
    realm_bit_offset = other.realm_bit_offset;
    defense_rating_bit_offset = other.defense_rating_bit_offset;
    durability_bit_offset = other.durability_bit_offset;
    stackable_bit_offset = other.stackable_bit_offset;
    gld_stackable_bit_offset = other.gld_stackable_bit_offset;
    socket_count_bit_offset_marker = other.socket_count_bit_offset_marker;
    socket_count_bit_offset = other.socket_count_bit_offset;
    bonus_bits_bit_offset = other.bonus_bits_bit_offset;
    magical_props_bit_offset = other.magical_props_bit_offset;
    set_bonus_props_bit_offset = other.set_bonus_props_bit_offset;
    runeword_props_bit_offset = other.runeword_props_bit_offset;
    item_end_bit_offset = other.item_end_bit_offset;
    item_current_socket_idx = other.item_current_socket_idx;
    dwa_bit_offset = other.dwa_bit_offset;
    dwb_bit_offset = other.dwb_bit_offset;
    magic_affixes_v100 = other.magic_affixes_v100;
    rare_affixes_v100 = other.rare_affixes_v100;
    return *this;
}
d2ce::Item& d2ce::Item::operator=(Item&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    data.swap(other.data);
    other.data.clear();
    SocketedItems.swap(other.SocketedItems);
    other.SocketedItems.clear();
    FileVersion = std::exchange(other.FileVersion, APP_CHAR_VERSION);
    ItemVersion = std::exchange(other.ItemVersion, APP_ITEM_VERSION);
    start_bit_offset = std::exchange(other.start_bit_offset, 16);
    location_bit_offset = std::exchange(other.location_bit_offset, ITEM_V115_LOCATION_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET);
    equipped_id_offset = std::exchange(other.equipped_id_offset, ITEM_V115_EQUIPPED_ID_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET);
    position_offset = std::exchange(other.position_offset, ITEM_V115_POSITION_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET);
    alt_position_id_offset = std::exchange(other.alt_position_id_offset, ITEM_V115_ALT_POSITION_ID_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET);
    type_code_offset = std::exchange(other.type_code_offset, ITEM_V115_TYPE_CODE_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET);
    extended_data_offset = std::exchange(other.extended_data_offset, ITEM_V107_EXTENDED_DATA_BIT_OFFSET);
    quest_difficulty_offset = std::exchange(other.quest_difficulty_offset, 0);
    nr_of_items_in_sockets_offset = std::exchange(other.nr_of_items_in_sockets_offset, 0);
    nr_of_items_in_sockets_bits = std::exchange(other.nr_of_items_in_sockets_bits, 3);
    item_id_bit_offset = std::exchange(other.item_id_bit_offset, 0);
    item_level_bit_offset = std::exchange(other.item_level_bit_offset, 0);
    quality_bit_offset = std::exchange(other.quality_bit_offset, 0);
    multi_graphic_bit_offset = std::exchange(other.multi_graphic_bit_offset, 0);
    autoAffix_bit_offset = std::exchange(other.autoAffix_bit_offset, 0);
    quality_attrib_bit_offset = std::exchange(other.quality_attrib_bit_offset, 0);
    runeword_id_bit_offset = std::exchange(other.runeword_id_bit_offset, 0);
    personalized_bit_offset_marker = std::exchange(other.personalized_bit_offset_marker, 0);
    personalized_bit_offset = std::exchange(other.personalized_bit_offset, 0);
    tome_bit_offset = std::exchange(other.tome_bit_offset, 0);
    realm_bit_offset = std::exchange(other.realm_bit_offset, 0);
    defense_rating_bit_offset = std::exchange(other.defense_rating_bit_offset, 0);
    durability_bit_offset = std::exchange(other.durability_bit_offset, 0);
    stackable_bit_offset = std::exchange(other.stackable_bit_offset, 0);
    gld_stackable_bit_offset = std::exchange(other.gld_stackable_bit_offset, 0);
    socket_count_bit_offset_marker = std::exchange(other.socket_count_bit_offset_marker, 0);
    socket_count_bit_offset = std::exchange(other.socket_count_bit_offset, 0);
    bonus_bits_bit_offset = std::exchange(other.bonus_bits_bit_offset, 0);
    magical_props_bit_offset = std::exchange(other.magical_props_bit_offset, 0);
    set_bonus_props_bit_offset = std::exchange(other.set_bonus_props_bit_offset, 0);
    runeword_props_bit_offset = std::exchange(other.runeword_props_bit_offset, 0);
    item_end_bit_offset = std::exchange(other.item_end_bit_offset, 0);
    item_current_socket_idx = std::exchange(other.item_current_socket_idx, 0);
    dwa_bit_offset = std::exchange(other.dwa_bit_offset, 0);
    dwb_bit_offset = std::exchange(other.dwb_bit_offset, 0);
    magic_affixes_v100 = std::exchange(other.magic_affixes_v100, MagicalCachev100());
    rare_affixes_v100 = std::exchange(other.rare_affixes_v100, RareOrCraftedCachev100());
    return *this;
}
//---------------------------------------------------------------------------
void d2ce::Item::swap(Item& other)
{
    std::swap(*this, other);
}
//---------------------------------------------------------------------------
std::uint8_t& d2ce::Item::operator [](size_t position) const
{
    return data[position];
}
//---------------------------------------------------------------------------
// returns the number of bytes not including socketed items
size_t d2ce::Item::size() const
{
    return data.size();
}
//---------------------------------------------------------------------------
// returns the number of bytes to store the item, including socketed items
size_t d2ce::Item::getFullSize() const
{
    size_t byteSize = data.size();
    for (auto& item : SocketedItems)
    {
        byteSize += item.getFullSize();
    }

    return byteSize;
}
//---------------------------------------------------------------------------
// itemsize is the number of bytes not including socketed items
void d2ce::Item::resize(size_t itemsize)
{
    data.resize(itemsize);
}
//---------------------------------------------------------------------------
// itemsize is the number of bytes not including socketed items
void d2ce::Item::reserve(size_t itemsize)
{
    data.reserve(itemsize);
}
//---------------------------------------------------------------------------
void d2ce::Item::clear()
{
    *this = Item();
}
//---------------------------------------------------------------------------
void d2ce::Item::push_back(const std::uint8_t& value)
{
    data.push_back(value);
}
//---------------------------------------------------------------------------
d2ce::EnumItemVersion d2ce::Item::Version() const
{
    return ItemVersion;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getRawVersion() const
{
    if (FileVersion < EnumCharVersion::v107) // pre-1.07 character file
    {
        return 0; // pre-1.08
    }

    return (std::uint16_t)read_uint32_bits(start_bit_offset + 32, (FileVersion < EnumCharVersion::v115 ? 8 : 3));
}
//---------------------------------------------------------------------------
bool d2ce::Item::setItemLocation(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY)
{
    if ((locationId == EnumItemLocation::STORED) && (altPositionId == EnumAltItemLocation::UNKNOWN))
    {
        // invalid state
        return false;
    }

    if (isEar())
    {
        // don't support placing ears
        return false;
    }

    size_t current_bit_offset = 0;
    size_t bitSize = 10;
    std::uint32_t value = 0;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100:
        switch (locationId)
        {
        case EnumItemLocation::STORED:
            switch (altPositionId)
            {
            case EnumAltItemLocation::INVENTORY:
                current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xF807);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = equipped_id_offset;
                bitSize = 4;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x0020);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = equipped_id_offset;
                bitSize = 4;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x0018);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = equipped_id_offset;
                bitSize = 4;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;
            }
            break;

        case EnumItemLocation::BELT:
            if (!isPotion())
            {
                return false;
            }

            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            positionY = positionX / 4;
            positionX = positionX % 4;
            current_bit_offset = position_offset;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = position_offset + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = equipped_id_offset;
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, 0))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF) | 0x8000);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            return true;

        case EnumItemLocation::EQUIPPED:
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = position_offset;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x01);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = equipped_id_offset;
            bitSize = 4;
            value = positionX;
            if (value > static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_HAND_LEFT))
            {
                return false;
            }
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF) | 0x4000);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            return true;
        }
        break;

    case EnumItemVersion::v104:
        if (isSimpleItem())
        {
            switch (locationId)
            {
            case EnumItemLocation::STORED:
                switch (altPositionId)
                {
                case EnumAltItemLocation::INVENTORY:
                    current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x03FE);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset;
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                    bitSize = 32;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    break;

                case EnumAltItemLocation::STASH:
                    current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x03FE) | 0x1000);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset;
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                    bitSize = 32;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    return true;

                case EnumAltItemLocation::HORADRIC_CUBE:
                    current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x03FE) | 0x0C00);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset;
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                    bitSize = 32;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    break;
                }
                return true;

            case EnumItemLocation::BELT:
                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x03FE) | 0xFC01);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                positionY = positionX / 4;
                positionX = positionX % 4;
                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) | 0x00000003);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;

            case EnumItemLocation::EQUIPPED:
                return false; // Simple items can't be equipped
            }
        }
        else
        {
            switch (locationId)
            {
            case EnumItemLocation::STORED:
                switch (altPositionId)
                {
                case EnumAltItemLocation::INVENTORY:
                    current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xF807);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset;
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                    bitSize = 8;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xC3);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                    bitSize = 32;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                    bitSize = 16;
                    value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xBFFF);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    return true;

                case EnumAltItemLocation::STASH:
                    current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x0020);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset;
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                    bitSize = 8;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xC3);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                    bitSize = 32;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                    bitSize = 16;
                    value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xBFFF);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    break;

                case EnumAltItemLocation::HORADRIC_CUBE:
                    current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x0018);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset;
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = position_offset + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                    bitSize = 8;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xC3);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                    bitSize = 32;
                    value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                    bitSize = 16;
                    value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xBFFF);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    break;
                }
                break;

            case EnumItemLocation::BELT:
            case EnumItemLocation::SOCKET:
                return false; // Only simple items can be socketted or put into the belt

            case EnumItemLocation::EQUIPPED:
                current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 16;
                value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x01);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                bitSize = 8;
                value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xC3);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = equipped_id_offset;
                bitSize = 4;
                value = positionX;
                if (value > static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_HAND_LEFT))
                {
                    return false;
                }
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                bitSize = 32;
                value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                bitSize = 16;
                value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) | 0x4000);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;
            }
            break;
        }
        break;

    default:
        current_bit_offset = location_bit_offset;
        value = static_cast<std::underlying_type_t<EnumItemLocation>>(locationId);
        bitSize = 3;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        if (locationId == EnumItemLocation::BELT)
        {
            positionX = positionY * 4 + positionX;
            positionY = 0;
        }
        else if (locationId == EnumItemLocation::EQUIPPED)
        {
            // if not equipped, equipped_id could be missing
            bitSize = 4;
            current_bit_offset = equipped_id_offset;
            value = positionX;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            positionX = 0;
            positionY = 0;
        }

        // position x/y
        current_bit_offset = position_offset;
        bitSize = 4;
        value = positionX;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        bitSize = 4;
        value = positionY;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        
        current_bit_offset = alt_position_id_offset;
        bitSize = 3;
        value = static_cast<std::underlying_type_t<EnumAltItemLocation>>(altPositionId);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isIdentified() const
{
    return read_uint32_bits(start_bit_offset + IS_IDENTIFIED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isDisabled() const
{
    return read_uint32_bits(start_bit_offset + IS_DISABLED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isSocketed() const
{
    if (isSimpleItem() && (Version() != EnumItemVersion::v100))
    {
        return false;
    }

    return read_uint32_bits(start_bit_offset + IS_SOCKETED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isNew() const
{
    return read_uint32_bits(start_bit_offset + IS_NEW_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isBadEquipped() const
{
    return read_uint32_bits(start_bit_offset + IS_BAD_EQUIPPED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isEar() const
{
    return read_uint32_bits(start_bit_offset + IS_EAR_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isStarterItem() const
{
    return read_uint32_bits(start_bit_offset + IS_STARTER_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
/*
   Returns true if this item is a simple item (i.e. 14 byte item)
*/
bool d2ce::Item::isSimpleItem() const
{
    if (Version() == EnumItemVersion::v100)
    {
        // v1.00 - v1.03
        return isEar() ? true : false;
    }

    return read_uint32_bits(start_bit_offset + IS_SIMPLE_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isEthereal() const
{
    return read_uint32_bits(start_bit_offset + IS_ETHEREAL_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPersonalized() const
{
    if(FileVersion < EnumCharVersion::v107)
    {
        // v1.00 - v1.06
        return false;
    }

    return read_uint32_bits(start_bit_offset + IS_PERSONALIZED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRuneword() const
{
    if (FileVersion < EnumCharVersion::v107)
    {
        // v1.00 - v1.06
        return false;
    }

    return read_uint32_bits(start_bit_offset + IS_RUNEWORD_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
d2ce::EnumItemLocation d2ce::Item::getLocation() const
{
    std::uint8_t loc = 0;
    std::uint16_t loc16 = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        if (isEar())
        {
            return EnumItemLocation::STORED;
        }

        loc16 = (std::uint16_t)read_uint32_bits(ITEM_V100_CONTAINER_BIT_OFFSET, 16); 
        switch (loc16 & 0x07F8)
        {
        case 0x0000:
        case 0x0020:
        case 0x0018:
            return EnumItemLocation::STORED;

        case 0x07F8:
            loc16 = (std::uint16_t)read_uint32_bits(ITEM_V100_ITEMCODE_BIT_OFFSET, 16);
            if (loc16 & 0x8000)
            {
                return isPotion() ? EnumItemLocation::BELT : EnumItemLocation::SOCKET;
            }

            if (loc16 & 0x4000)
            {
                return EnumItemLocation::EQUIPPED;
            }

            return EnumItemLocation::BUFFER;
        }

        return EnumItemLocation::BUFFER;

    case EnumItemVersion::v104: // v1.04 - v1.06
        if (isEar())
        {
            return EnumItemLocation::STORED;
        }

        if (isSimpleItem())
        {
            loc16 = (std::uint16_t)read_uint32_bits(ITEM_V104_SM_COORDINATES_BIT_OFFSET, 16);
            switch (loc16 & 0xFC01)
            {
            case 0x0000:
            case 0x1000:
            case 0x0C00:
                loc = (std::uint8_t)read_uint32_bits(ITEM_V104_SM_ITEMCODE_BIT_OFFSET, 8);
                if ((loc & 0x03) != 0)
                {
                    return EnumItemLocation::BUFFER;
                }

                return EnumItemLocation::STORED;

            case 0xFC01:
                loc = (std::uint8_t)read_uint32_bits(ITEM_V104_SM_ITEMCODE_BIT_OFFSET, 8);
                if ((loc & 0x03) != 0x03)
                {
                    return EnumItemLocation::BUFFER;
                }

                if ((loc16 & 0xFC1B) == 0xFC1B)
                {
                    return EnumItemLocation::SOCKET;
                }

                return EnumItemLocation::BELT;
            }
        }

        loc16 = (std::uint16_t)read_uint32_bits(ITEM_V104_EX_CONTAINER_BIT_OFFSET, 16);
        switch (loc16 & 0x07F8)
        {
        case 0x0000:
        case 0x0020:
        case 0x0018:
            return EnumItemLocation::STORED;

        case 0x07F8:
            return EnumItemLocation::EQUIPPED;
        }

        return EnumItemLocation::BUFFER;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II Resurrected
        loc = (std::uint8_t)read_uint32_bits(location_bit_offset, 3);

        if (loc > static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::SOCKET))
        {
            return EnumItemLocation::STORED;
        }

        return static_cast<EnumItemLocation>(loc);
    }

    return EnumItemLocation::BUFFER;
}
//---------------------------------------------------------------------------
d2ce::EnumEquippedId d2ce::Item::getEquippedId() const
{
    if (equipped_id_offset == 0)
    {
        return EnumEquippedId::NONE;
    }

    auto value = (std::uint8_t)read_uint32_bits(equipped_id_offset, 4);
    if (value > static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_HAND_LEFT))
    {
        return EnumEquippedId::NONE;
    }

    return static_cast<EnumEquippedId>(value);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPositionX() const
{
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        switch (getLocation())
        {
        case EnumItemLocation::BELT:
           return (std::uint8_t)(read_uint32_bits(position_offset, 2) + read_uint32_bits(position_offset + 2, 2) * 4);

        case EnumItemLocation::STORED:
            return (std::uint8_t)read_uint32_bits(position_offset, 5);

        default:
            return 0;
        }
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        return (std::uint8_t)read_uint32_bits(position_offset, 4);
    }

    return 0;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPositionY() const
{
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        switch (getLocation())
        {
        case EnumItemLocation::BELT:
            return 0;

        case EnumItemLocation::STORED:
            return (std::uint8_t)read_uint32_bits(position_offset + 5, 2);

        default:
            return 0;
        }
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        return (std::uint8_t)read_uint32_bits(position_offset + 4, 4);
    }

    return 0;
}
//---------------------------------------------------------------------------
d2ce::EnumAltItemLocation d2ce::Item::getAltPositionId() const
{
    std::uint8_t loc = 0;
    std::uint16_t loc16 = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        if (getLocation() != EnumItemLocation::STORED)
        {
            return EnumAltItemLocation::UNKNOWN;
        }

        if (isEar())
        {
            loc = (std::uint16_t)read_uint32_bits(ITEM_V100_EAR_CONTAINER_BIT_OFFSET, 8);
            switch (loc & 0x60)
            {
            case 0x00:
                return EnumAltItemLocation::INVENTORY;

            case 0x40:
                return EnumAltItemLocation::STASH;

            case 0x60:
                return EnumAltItemLocation::HORADRIC_CUBE;

            default:
                return EnumAltItemLocation::UNKNOWN;
            }
        }
        else
        {
            loc16 = (std::uint16_t)read_uint32_bits(ITEM_V100_CONTAINER_BIT_OFFSET, 16);
            switch (loc16 & 0x07F8)
            {
            case 0x0000:
                return EnumAltItemLocation::INVENTORY;

            case 0x0020:
                return EnumAltItemLocation::STASH;

            case 0x0018:
                return EnumAltItemLocation::HORADRIC_CUBE;

            default:
                return EnumAltItemLocation::UNKNOWN;
            }
        }
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06
        if (getLocation() != EnumItemLocation::STORED)
        {
            return EnumAltItemLocation::UNKNOWN;
        }

        if (isEar())
        {
            loc16 = (std::uint16_t)read_uint32_bits(ITEM_V104_EAR_COORDINATES_BIT_OFFSET, 16);
            switch (loc16 & 0x1C00)
            {
            case 0x0000:
                return EnumAltItemLocation::INVENTORY;

            case 0x1000:
                return EnumAltItemLocation::STASH;

            case 0x0C00:
                return EnumAltItemLocation::HORADRIC_CUBE;

            default:
                return EnumAltItemLocation::UNKNOWN;
            }
        }
        else if (isSimpleItem())
        {
            loc16 = (std::uint16_t)read_uint32_bits(ITEM_V104_SM_COORDINATES_BIT_OFFSET, 16);
            switch (loc16 & 0xFC01)
            {
            case 0x0000:
                return EnumAltItemLocation::INVENTORY;

            case 0x1000:
                return EnumAltItemLocation::STASH;

            case 0x0C00:
                return EnumAltItemLocation::HORADRIC_CUBE;

            default:
                return EnumAltItemLocation::UNKNOWN;
            }
        }
        else
        {
            loc16 = (std::uint16_t)read_uint32_bits(ITEM_V104_EX_CONTAINER_BIT_OFFSET, 16);
            switch (loc16 & 0x07F8)
            {
            case 0x0000:
                return EnumAltItemLocation::INVENTORY;

            case 0x0020:
                return EnumAltItemLocation::STASH;

            case 0x0018:
                return EnumAltItemLocation::HORADRIC_CUBE;

            default:
                return EnumAltItemLocation::UNKNOWN;
            }
        }
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        loc = (std::uint8_t)read_uint32_bits(alt_position_id_offset, 3); 
        if (loc > static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::STASH))
        {
            return EnumAltItemLocation::UNKNOWN;
        }

        return static_cast<EnumAltItemLocation>(loc);
    }

    return EnumAltItemLocation::UNKNOWN;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getItemCode(std::array<std::uint8_t, 4>& strcode) const
{
    strcode.fill(0);
    if (isEar() || (type_code_offset == 0))
    {
        return false;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        return ItemHelpers::getItemCodev100(std::uint16_t(read_uint32_bits(type_code_offset, 10)), strcode);

    case EnumItemVersion::v104: // v1.04 - v1.06
    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
        strcode[0] = std::uint8_t(read_uint32_bits(type_code_offset, 8));
        strcode[1] = std::uint8_t(read_uint32_bits(type_code_offset + 8, 8));
        strcode[2] = std::uint8_t(read_uint32_bits(type_code_offset + 16, 8));
        strcode[3] = std::uint8_t(read_uint32_bits(type_code_offset + 24, 6));
        return true;

    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        ItemHelpers::getItemCodev115(data, type_code_offset, strcode);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
d2ce::EnumItemType d2ce::Item::getItemType() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        return ItemHelpers::getEnumItemTypeFromCode(strcode);
    }

    return EnumItemType::Other;

}
//---------------------------------------------------------------------------
std::string d2ce::Item::getItemTypeName() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.name;
    }

    return "";
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateGem(const std::array<std::uint8_t, 4> &newgem)
{
    if (isEar())
    {
        return false;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        // should not happen
        return false;
    }

    const auto& result = ItemHelpers::getItemTypeHelper(strcode);
    if (!result.isGem() && !result.isPotion() && !result.isRune())
    {
        // we can only upgrade GPS
        return false;
    }

    if (result.isGem() && (getLocation() == EnumItemLocation::SOCKET))
    {
        // can't update gem in socket
        return false;
    }

    const auto& newResult = ItemHelpers::getItemTypeHelper(newgem);
    if (!newResult.isGem() && !newResult.isPotion() && !newResult.isRune())
    {
        // we can only upgrade GPS
        return false;
    }

    if (newResult.isGem() && (getLocation() == EnumItemLocation::BELT))
    {
        // can't put gem in belt
        return false;
    }

    std::uint64_t code = 0;
    std::uint8_t numBitsSet = 0;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        code = ItemHelpers::getTypeCodev100(newgem);
        if (code >= UINT16_MAX)
        {
            return false;
        }
        updateBits64(type_code_offset, 10, code);
        return true;

    case EnumItemVersion::v104: // v1.04 - v1.06
        code = *((std::uint32_t*)newgem.data());
        updateBits64(type_code_offset, 30, code);
        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
        code = *((std::uint32_t*)newgem.data());
        updateBits64(type_code_offset, 32, code);
        return true;

    case EnumItemVersion::v115: // v1.15 normal or expansion
        ItemHelpers::encodeItemCodev115(newgem, code, numBitsSet);
        updateItemCodev115(code, numBitsSet);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
/*
   Converts the gem to its perfect state
   Returns true if the gem was converted
*/
bool d2ce::Item::upgradeGem()
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    getItemCode(strcode);
    switch (gem)
    {
    case 'g':
        switch (gemcondition)
        {
        case 'c': // chipped
        case 'f': // flawed
        case 's': // regular
        case 'l': // flawless
            switch (gemcolour)
            {
            case 'v': // amethyst
                if (gemcondition != 'l')
                {
                    gemcondition = 'p'; // perfect
                    return updateGem(strcode);
                }
                break;

            case 'w': // diamond
            case 'g': // emerald
            case 'r': // ruby
            case 'b': // sapphire
            case 'y': // topaz
                gemcondition = 'p'; // perfect
                return updateGem(strcode);
            }
            break;

        case 'z': // flawless amethyst
            if (gemcolour == 'v')
            {
                gemcondition = 'p'; // perfect
                return updateGem(strcode);
            }
            break;
        }
        break;

    case 's': // skulls
        if (gemcondition == 'k')
        {
            switch (gemcolour)
            {
            case 'c': // chipped
            case 'f': // flawed
            case 'u': // regular
            case 'l': // flawless
                gemcolour = 'z'; // perfect
                return updateGem(strcode);
            }
        }
        break;
    }  // end switch

    return false;
}
//---------------------------------------------------------------------------
/*
   Converts the potion to its highest quality.
   Returns true if the the potion was converted.
*/
bool d2ce::Item::upgradePotion()
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    getItemCode(strcode);
    switch (gem)
    {
    case 'r': // rejuvenation potions
        switch (gemcondition)
        {
        case 'v':
            switch (gemcolour)
            {
            case 's':
                gemcolour = 'l';
                return updateGem(strcode);
            }
            break;

        case 'p': // not a valid potion
            switch (gemcolour)
            {
            case 's':
            case 'l':
                gem = 'h';
                gemcolour = '5';
                return updateGem(strcode);
            }
            break;
        }
        break;

    case 'b': // not a valid potion
        switch (gemcondition)
        {
        case 'p': // not a valid potion
            switch (gemcolour)
            {
            case 's':
            case 'l':
                gem = 'm';
                gemcolour = '5';
                return updateGem(strcode);
            }
            break;
        }
        break;

    case 'h': // healing potions
    case 'm': // mana potions
        switch (gemcondition)
        {
        case 'p':
            switch (gemcolour)
            {
            case '1':
            case '2':
            case '3':
            case '4':
            case 'f': // not a valid potion
            case 'o': // not a valid potion
                gemcolour = '5';
                return updateGem(strcode);
            }
            break;
        }
        break;
    }  // end switch

    return false;
}
//---------------------------------------------------------------------------
/*
   Convert the potion to  a Full Rejuvenation potion.
   Returns true if the the potion was converted.
*/
bool d2ce::Item::upgradeToFullRejuvenationPotion()
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    strcode[3] = 0x20;
    getItemCode(strcode);
    switch (gem)
    {
    case 'r': // rejuvenation potions
        switch (gemcondition)
        {
        case 'v':
            switch (gemcolour)
            {
            case 's':
                gemcolour = 'l';
                return updateGem(strcode);
            }
            break;

        case 'p': // not a valid potion
            switch (gemcolour)
            {
            case 's':
            case 'l':
                gemcondition = 'v';
                gemcolour = 'l';
                return updateGem(strcode);
            }
            break;
        }
        break;

    case 'b': // not a valid potion
        switch (gemcondition)
        {
        case 'p': // not a valid potion
            switch (gemcolour)
            {
            case 's':
            case 'l':
                gem = 'r';
                gemcondition = 'v';
                gemcolour = 'l';
                return updateGem(strcode);
            }
            break;
        }
        break;

    case 'h': // healing potions
    case 'm': // mana potions
        switch (gemcondition)
        {
        case 'p':
            switch (gemcolour)
            {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case 'f': // not a valid potion
            case 'o': // not a valid potion
                gem = 'r';
                gemcondition = 'v';
                gemcolour = 'l';
                return updateGem(strcode);
            }
        }
        break;
    }  // end switch

    return false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getQuestDifficulty() const
{
    if (quest_difficulty_offset == 0)
    {
        return 0;
    }

    return (std::uint8_t)read_uint32_bits(quest_difficulty_offset, 2);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::socketedItemCount() const
{
    if (nr_of_items_in_sockets_offset == 0)
    {
        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
            if (isEar())
            {
                return 0;
            }
            
            nr_of_items_in_sockets_offset = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 2;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06
            if (isSimpleItem() || isEar())
            {
                return 0;
            }

            nr_of_items_in_sockets_offset = ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 3;
            break;

        case EnumItemVersion::v107: // v1.07 item
        case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
        case EnumItemVersion::v110: // v1.10 normal or expansion
        case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
            nr_of_items_in_sockets_offset = extended_data_offset;
            nr_of_items_in_sockets_bits = isSimpleItem() ? 1 : 3;
            break;
        }
    }

    return (std::uint8_t)read_uint32_bits(nr_of_items_in_sockets_offset, nr_of_items_in_sockets_bits);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getEarAttributes(d2ce::EarAttributes& attrib) const
{
    attrib.clear();
    if (!isEar())
    {
        return false;
    }

    size_t currentOffset = type_code_offset;
    attrib.Class = static_cast<EnumCharClass>(read_uint32_bits(currentOffset, 3));
    currentOffset += 3;

    size_t levelBits = 7;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        levelBits = 8;
        break;
    }

    attrib.Level = std::uint32_t(read_uint32_bits(currentOffset, levelBits));
    currentOffset += levelBits;

    // up to 15 7 bit characters
    attrib.Name.fill(0);
    char c = 0;
    for (std::uint8_t idx = 0; idx < 15; ++idx)
    {
        c = (char)read_uint32_bits(currentOffset, 7);
        if (c == 0)
        {
            break;
        }
        attrib.Name[idx] = c;
        currentOffset += 7;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRequirements(ItemRequirements& req) const
{
    req.clear();
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        req = result.req;

        d2ce::SetAttributes setAttrib;
        d2ce::UniqueAttributes uniqueAttrib;
        switch (getQuality())
        {
        case EnumItemQuality::SET:
            getSetAttributes(setAttrib);
            if (setAttrib.ReqLevel != 0)
            {
                req.Level = setAttrib.ReqLevel;
            }
            break;

        case EnumItemQuality::UNIQUE:
            getUniqueAttributes(uniqueAttrib);
            if (uniqueAttrib.ReqLevel != 0)
            {
                req.Level = uniqueAttrib.ReqLevel;
            }
            break;
        }
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getCategories(std::vector<std::string>& categories) const
{
    categories.clear();
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        categories = result.categories;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDimensions(ItemDimensions& dimensions) const
{
    dimensions.clear();
    if (isEar())
    {
        dimensions = { 1, 1 };
        return true;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        dimensions = result.dimensions;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::getTotalItemSlots() const
{
    ItemDimensions dimensions;
    if (!getDimensions(dimensions))
    {
        return 0;
    }

    return dimensions.InvHeight * dimensions.InvWidth;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getInvFile() const
{
    if (isEar())
    {
        return "invear";
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.inv_file;
    }

    return "";
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getTransformColor() const
{
    std::string result;
    std::string tc;
    d2ce::MagicalAffixes magicalAffixes;
    if (getMagicalAffixes(magicalAffixes))
    {
        bool hasPrefixOrSuffix = false;
        if (magicalAffixes.PrefixId != 0)
        {
            hasPrefixOrSuffix = true;
            tc = ItemHelpers::getMagicalPrefixTCFromId(magicalAffixes.PrefixId);
            if (!tc.empty())
            {
                result = tc;
            }
        }

        if (magicalAffixes.SuffixId != 0)
        {
            hasPrefixOrSuffix = true;
            tc = ItemHelpers::getMagicalSuffixTCFromId(magicalAffixes.SuffixId);
            if (!tc.empty())
            {
                result = tc;
            }
        }

        if (hasPrefixOrSuffix)
        {
            return result;
        }
    }

    d2ce::UniqueAttributes uniqueAttrib;
    if (getUniqueAttributes(uniqueAttrib))
    {
        return ItemHelpers::getUniqueTCFromId(uniqueAttrib.Id);
    }
    
    d2ce::RareAttributes rareAttrib;
    if (getRareOrCraftedAttributes(rareAttrib))
    {
        for (auto affix : rareAttrib.Affixes)
        {
            if (magicalAffixes.PrefixId != 0)
            {
                tc = ItemHelpers::getMagicalPrefixTCFromId(magicalAffixes.PrefixId);
                if (!tc.empty())
                {
                    result = tc;
                }
            }

            if (magicalAffixes.SuffixId != 0)
            {
                tc = ItemHelpers::getMagicalSuffixTCFromId(magicalAffixes.SuffixId);
                if (!tc.empty())
                {
                    result = tc;
                }
            }
        }

        return result;
    }

    d2ce::SetAttributes setAttrib;
    if (getSetAttributes(setAttrib))
    {
        return ItemHelpers::getSetTCFromId(setAttrib.Id);
    }

    return result;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::getId() const
{
    if (isSimpleItem())
    {
        return 0;
    }

    if (item_id_bit_offset == 0)
    {
        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
        case EnumItemVersion::v104: // v1.04 - v1.06
            return 0;

        case EnumItemVersion::v107: // v1.07 item
        case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
        case EnumItemVersion::v110: // v1.10 normal or expansion
        case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
            item_id_bit_offset = start_bit_offset + 95;
            break;
        }
    }

    return (std::uint32_t)read_uint64_bits(item_id_bit_offset, 32);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getLevel() const
{
    if (isSimpleItem())
    {
        return 0;
    }

    if (item_level_bit_offset == 0)
    {
        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
            item_level_bit_offset = ITEM_V100_LEVEL_BIT_OFFSET;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06
            item_level_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET;
            break;

        case EnumItemVersion::v107: // v1.07 item
        case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
        case EnumItemVersion::v110: // v1.10 normal or expansion
        case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
            item_level_bit_offset = start_bit_offset + 127;
            break;
        }
    }

    size_t numBits = 7;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        numBits = 8;
        break;
    }

    return (std::uint8_t)read_uint32_bits(item_level_bit_offset, numBits);
}
//---------------------------------------------------------------------------
d2ce::EnumItemQuality d2ce::Item::getQuality() const
{
    size_t numBits = 4;
    std::uint32_t value;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        if (isEar())
        {
            return EnumItemQuality::UNKNOWN;
        }

        if (quality_bit_offset == 0)
        {
            quality_bit_offset = start_bit_offset + QUALITY_BIT_OFFSET_100;
        }

        numBits = 3;
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06
        if (isSimpleItem())
        {
            return EnumItemQuality::UNKNOWN;
        }

        if (quality_bit_offset == 0)
        {
            quality_bit_offset = start_bit_offset + QUALITY_BIT_OFFSET_104;
        }

        numBits = 4;
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        if (isSimpleItem())
        {
            return EnumItemQuality::UNKNOWN;
        }

        if (quality_bit_offset == 0)
        {
            quality_bit_offset = start_bit_offset + QUALITY_BIT_OFFSET;
        }

        numBits = 4;
        break;

    default:
        return EnumItemQuality::UNKNOWN;
    }

    value = read_uint32_bits(quality_bit_offset, numBits);
    if (value > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
    {
        return EnumItemQuality::UNKNOWN;
    }

    return static_cast<EnumItemQuality>(value);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAffixes(MagicalAffixes& affixes) const
{
    affixes.clear();
    switch (getQuality())
    {
    case EnumItemQuality::MAGIC:
        break;

    default:
        return false;
    }

    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        return getMagicalAffixesv100(affixes);
    }

    size_t current_bit_offset = quality_attrib_bit_offset; // must copy value as readPropertyList will modify value
    affixes.PrefixId = (std::uint16_t)read_uint32_bits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS);
    current_bit_offset += MAGICAL_AFFIX_NUM_BITS;
    affixes.PrefixName = ItemHelpers::getMagicalPrefixFromId(affixes.PrefixId);

    affixes.SuffixId = (std::uint16_t)read_uint32_bits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS);
    current_bit_offset += MAGICAL_AFFIX_NUM_BITS;
    affixes.SuffixName = ItemHelpers::getMagicalSuffixFromId(affixes.SuffixId);

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRunewordAttributes(d2ce::RunewordAttributes& attrib) const
{
    attrib.clear();
    if (!isRuneword() || (runeword_id_bit_offset == 0))
    {
        return false;
    }

    attrib.Id = (std::uint16_t)read_uint32_bits(runeword_id_bit_offset, RUNEWORD_ID_NUM_BITS);
    attrib.Name = ItemHelpers::getRunewordNameFromId(attrib.Id);
    size_t current_bit_offset = runeword_props_bit_offset; // must copy value as readPropertyList will modify value
    if (!readPropertyList(current_bit_offset, attrib.MagicalAttributes))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getPersonalizedName() const
{
    if (!isPersonalized() || (personalized_bit_offset == 0))
    {
        return std::string();
    }

    // up to 15 7 bit characters
    size_t currentOffset = personalized_bit_offset;
    std::array<char, NAME_LENGTH> name;
    name.fill(0);
    char c = 0;
    for (std::uint8_t idx = 0; idx < 15; ++idx)
    {
        c = (char)read_uint32_bits(currentOffset, 7);
        if (c == 0)
        {
            break;
        }
        name[idx] = c;
        currentOffset += 7;
    }

    return name.data();
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getTomeValue() const
{
    if (!isTome() || (tome_bit_offset == 0))
    {
        return 0;
    }

    return std::uint8_t(read_uint32_bits(tome_bit_offset, 5));
}
//---------------------------------------------------------------------------
bool d2ce::Item::getSetAttributes(SetAttributes& attrib) const
{
    attrib.clear();
    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::SET:
        break;

    default:
        return false;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        return getSetAttributesv100(attrib);
    }

    attrib.Id = (std::uint16_t)read_uint32_bits(quality_attrib_bit_offset, SET_UNIQUE_ID_NUM_BITS);
    attrib.Name = ItemHelpers::getSetNameFromId(attrib.Id);
    attrib.ReqLevel = ItemHelpers::getSetLevelReqFromId(attrib.Id);
    if (bonus_bits_bit_offset == 0)
    {
        return false;
    }

    std::vector<MagicalAttribute> setAttribs;
    std::uint16_t setBonusBits = (std::uint16_t)read_uint32_bits(bonus_bits_bit_offset, 5);
    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        size_t current_bit_offset = set_bonus_props_bit_offset; // must copy value as readPropertyList will modify value
        for (size_t i = 0; i < 5 && setBonusBits > 0; ++i, setBonusBits >>= 1)
        {
            if ((setBonusBits & 0x01) != 0)
            {
                if (!readPropertyList(current_bit_offset, setAttribs))
                {
                    return false;
                }

                attrib.SetAttributes.emplace_back(setAttribs);
                setAttribs.clear();
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRareOrCraftedAttributes(RareAttributes& attrib) const
{
    attrib.clear();
    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }


    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        break;

    default:
        return false;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        return getRareOrCraftedAttributesv100(attrib);
    }

    size_t current_bit_offset = quality_attrib_bit_offset;
    attrib.Id = (std::uint16_t)read_uint32_bits(current_bit_offset, RARE_CRAFTED_ID_NUM_BITS);
    current_bit_offset += RARE_CRAFTED_ID_NUM_BITS;
    attrib.Name = ItemHelpers::getRareNameFromId(attrib.Id);

    attrib.Id2 = (std::uint16_t)read_uint32_bits(current_bit_offset, RARE_CRAFTED_ID_NUM_BITS);
    current_bit_offset += RARE_CRAFTED_ID_NUM_BITS;
    attrib.Name2 = ItemHelpers::getRareNameFromId(attrib.Id2);

    // Following the name IDs, we got 6 possible magical affixes, the pattern
    // is 1 bit id, 11 bit value... But the value will only exist if the prefix
    // is 1. So we'll read the id first and check it against 1.
    std::uint8_t prefix = 0;
    for (size_t i = 3; i > 0; --i)
    {
        MagicalAffixes affixes;
        prefix = (std::uint8_t)read_uint32_bits(current_bit_offset, 1);
        current_bit_offset += 1;
        if (prefix != 0)
        {
            affixes.PrefixId = (std::uint16_t)read_uint32_bits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS);
            current_bit_offset += MAGICAL_AFFIX_NUM_BITS;
            affixes.PrefixName = ItemHelpers::getMagicalPrefixFromId(affixes.PrefixId);
        }

        prefix = (std::uint8_t)read_uint32_bits(current_bit_offset, 1);
        current_bit_offset += 1;
        if (prefix != 0)
        {
            affixes.SuffixId = (std::uint16_t)read_uint32_bits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS);
            current_bit_offset += MAGICAL_AFFIX_NUM_BITS;
            affixes.SuffixName = ItemHelpers::getMagicalSuffixFromId(affixes.SuffixId);
        }

        attrib.Affixes.emplace_back(affixes);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getUniqueAttributes(UniqueAttributes& attrib) const
{
    attrib.clear();
    if (quality_attrib_bit_offset == 0)
    {
        if (isSimpleItem())
        {
            return false;
        }

        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
            quality_attrib_bit_offset = ITEM_V100_SPECIALITEMCODE_BIT_OFFSET;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06
            quality_attrib_bit_offset = ITEM_V104_EX_UNIQUECODE_BIT_OFFSET;
            break;

        default:
            return false;
        }
    }

    switch (getQuality())
    {
    case EnumItemQuality::UNIQUE:
        break;

    default:
        return false;
    }

    size_t numBits = SET_UNIQUE_ID_NUM_BITS;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        numBits = ITEN_V100_UNIQUE_ID_NUM_BITS;
        break;
    }

    attrib.Id = (std::uint16_t)read_uint32_bits(quality_attrib_bit_offset, numBits);
    attrib.Name = ItemHelpers::getUniqueNameFromId(attrib.Id);
    attrib.ReqLevel = ItemHelpers::getUniqueLevelReqFromId(attrib.Id);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAttributes(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();
    d2ce::UniqueAttributes uniqueAttrib;
    d2ce::SetAttributes setAttrib;
    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        return getMagicalAttributesv100(attribs);
    }

    if (isSimpleItem() || (magical_props_bit_offset == 0))
    {
        if (!socketedMagicalAttributes.empty())
        {
            attribs.insert(attribs.end(), socketedMagicalAttributes.begin(), socketedMagicalAttributes.end());
            return true;
        }

        return false;
    }

    size_t current_bit_offset = magical_props_bit_offset;
    if (!readPropertyList(current_bit_offset, attribs))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();
    if (hasUndeadBonus())
    {
        // Push back special class attribute
        static const auto& stat = ItemHelpers::getItemStat(122);
        MagicalAttribute attrib;
        attrib.Id = stat.id;
        attrib.Desc = stat.desc;
        attrib.Name = stat.name;
        attrib.Values = { 50 };
        attribs.push_back(attrib);
    }

    std::vector<MagicalAttribute> tempAttribs;
    if (!getMagicalAttributes(tempAttribs))
    {
        return !attribs.empty();
    }

    std::multimap<size_t, size_t> itemIndexMap;
    ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);

    RunewordAttributes runeAttrib;
    if (getRunewordAttributes(runeAttrib))
    {
        ItemHelpers::combineMagicalAttribute(itemIndexMap, runeAttrib.MagicalAttributes, attribs);
    }

    for (const auto& item : SocketedItems)
    {
        if (item.getMagicalAttributes(tempAttribs))
        {
            ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::hasMultipleGraphics() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (multi_graphic_bit_offset == 0)
    {
        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
        case EnumItemVersion::v104: // v1.04 - v1.06
            return hasMultipleGraphicsv100();

        case EnumItemVersion::v107: // v1.07 item
        case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
        case EnumItemVersion::v110: // v1.10 normal or expansion
        case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
            multi_graphic_bit_offset = start_bit_offset + 138;
            break;
        }
    }

    return read_uint32_bits(multi_graphic_bit_offset, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPictureId() const
{
    if (!hasMultipleGraphics())
    {
        return 0;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        return getPictureIdv100();
    }

    return (std::uint8_t)read_uint32_bits(multi_graphic_bit_offset + 1, 3);
}
//---------------------------------------------------------------------------
bool d2ce::Item::isAutoAffix() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (autoAffix_bit_offset == 0)
    {
        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
        case EnumItemVersion::v104: // v1.04 - v1.06
            return false;

        case EnumItemVersion::v107: // v1.07 item
        case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
        case EnumItemVersion::v110: // v1.10 normal or expansion
        case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
            autoAffix_bit_offset = start_bit_offset + 139;
            if (hasMultipleGraphics())
            {
                autoAffix_bit_offset += 3;
            }
            break;
        }
    }

    return read_uint32_bits(autoAffix_bit_offset, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getAutoAffixId() const
{
    if (!isAutoAffix())
    {
        return 0;
    }

    return (std::uint16_t)read_uint32_bits(autoAffix_bit_offset + 1, 11);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getInferiorQualityId() const
{
    switch (getQuality())
    {
    case EnumItemQuality::INFERIOR:
        break;

    default:
        return 0;
    }

    if (quality_attrib_bit_offset == 0)
    {
        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
        case EnumItemVersion::v104: // v1.04 - v1.06
            return getInferiorQualityIdv100();
        }
        return false;
    }

    return (std::uint8_t)read_uint32_bits(quality_attrib_bit_offset, INFERIOR_SUPERIOR_ID_NUM_BITS);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getFileIndex() const
{
    if (quality_attrib_bit_offset == 0)
    {
        EarAttributes earAttrib;
        if (getEarAttributes(earAttrib))
        {
            return (std::uint16_t)earAttrib.getClass();
        }

        return 0;
    }

    switch (getQuality())
    {
    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::SUPERIOR:
        return (std::uint16_t)read_uint32_bits(quality_attrib_bit_offset, INFERIOR_SUPERIOR_ID_NUM_BITS);

    case EnumItemQuality::SET:
    case EnumItemQuality::UNIQUE:
        return (std::uint16_t)read_uint32_bits(quality_attrib_bit_offset, SET_UNIQUE_ID_NUM_BITS);
    }

    return 0;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getSetItemMask() const
{
    if (bonus_bits_bit_offset == 0)
    {
        return 0;
    }

    switch (getQuality())
    {
    case EnumItemQuality::SET:
        break;

    default:
        return 0;
    }

    return (std::uint16_t)read_uint32_bits(bonus_bits_bit_offset, 5);
}
//---------------------------------------------------------------------------
bool d2ce::Item::isArmor() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        base = ItemHelpers::getItemBase(strcode);
    }

    return (base & 4) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        base = ItemHelpers::getItemBase(strcode);
    }

    return (base & 2) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isMissileWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isMissileWeapon();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isTome() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        base = ItemHelpers::getItemBase(strcode);
    }

    return (base & 8) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isStackable() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::uint8_t base = 0;
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        base = ItemHelpers::getItemBase(strcode);
    }

    return (base & 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPotion() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isPotion();
    }
    
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isGem() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isGem();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isUpgradableGem() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isUpgradableGem();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isUpgradablePotion() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isUpgradablePotion();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isUpgradableToFullRejuvenationPotion() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isUpgradableRejuvenationPotion() || (result.isPotion() && !result.isRejuvenationPotion());
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRune() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isRune();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isJewel() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isJewel();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isCharm() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isCharm();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isBelt() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isBelt();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isQuestItem() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.isQuestItem();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isHoradricCube() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        if (strcode[0] == 'b' && strcode[1] == 'o' && strcode[2] == 'x')
        {
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isIndestructible() const
{
    d2ce::ItemDurability durability;
    if (getDurability(durability) && durability.Max == 0)
    {
        // Indestructible without the need for the magical attribute of indestructibility
        return true;
    }

    // Check for magical attribute of indestructibility
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);
        for (auto& attrib : magicalAttributes)
        {
            switch (attrib.Id)
            {
            case ATTRIBUTE_ID_INDESTRUCTIBLE:
                // Indestructible
                return true;
            }
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::hasUndeadBonus() const
{
    if (isSimpleItem())
    {
        return false;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.hasUndeadBonus();
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canHaveSockets() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (isSocketed())
    {
        return true;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        return result.canHaveSockets();
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::totalNumberOfSockets() const
{
    if (isSimpleItem() || !isSocketed())
    {
        return 0;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        return getMaxSocketCount();

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        if (socket_count_bit_offset == 0)
        {
            return 0;
        }

        return (std::uint8_t)read_uint32_bits(socket_count_bit_offset, SOCKET_COUNT_NUM_BITS);
    }

    return 0;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getQuantity() const
{
    if (!isStackable() || (stackable_bit_offset == 0 && gld_stackable_bit_offset == 0))
    {
        return 0;
    }

    if (gld_stackable_bit_offset != 0)
    {
        return (std::uint8_t)read_uint32_bits(gld_stackable_bit_offset, GLD_STACKABLE_NUM_BITS);
    }

    return (std::uint8_t)read_uint32_bits(stackable_bit_offset, STACKABLE_NUM_BITS);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setQuantity(std::uint16_t quantity)
{
    if (!isStackable() || (stackable_bit_offset == 0 && gld_stackable_bit_offset == 0))
    {
        return false;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    std::uint8_t& gem = strcode[0];
    std::uint8_t& gemcondition = strcode[1];
    std::uint8_t& gemcolour = strcode[2];
    getItemCode(strcode);
    switch (gem)
    {
    case 't': // Town portal book
    case 'i': // Identify scroll book
        if (gemcondition == 'b' && gemcolour == 'k')
        {
            // can hold up to 20 scrolls
            quantity = std::min(quantity, MAX_TOME_QUANTITY);
        }
        break;
    case 'k':
        if (gemcondition == 'e' && gemcolour == 'y')
        {
            // only 12 keys can be stacked ajd need at least 1
            auto maxKeys = FileVersion < EnumCharVersion::v107 ? MAX_KEY_QUANTITY_100 : MAX_KEY_QUANTITY;
            quantity = std::max(std::uint16_t(1), std::min(quantity, maxKeys));
        }
        break;
    case 'g':
        if (gemcondition == 'l' && gemcolour == 'd')
        {
            // can hold up to 4095 gold pieces
            quantity = std::min(quantity, MAX_GLD_QUANTITY);
        }
        break;

    default:
        quantity = std::min(quantity, MAX_STACKED_QUANTITY);
        break;

    }  // end switch

    if (getQuantity() == quantity)
    {
        // nothing to do
        return false;
    }

    if (gld_stackable_bit_offset != 0)
    {
        return updateBits(gld_stackable_bit_offset, GLD_STACKABLE_NUM_BITS, quantity);
    }

    return updateBits(stackable_bit_offset, STACKABLE_NUM_BITS, quantity);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMaxQuantity()
{
    if (!isStackable() || (stackable_bit_offset == 0 && gld_stackable_bit_offset == 0))
    {
        return false;
    }

    std::uint16_t quantity = gld_stackable_bit_offset != 0 ? MAX_GLD_QUANTITY : MAX_STACKED_QUANTITY;
    return setQuantity(quantity);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getDefenseRating() const
{
    if (defense_rating_bit_offset == 0)
    {
        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
        case EnumItemVersion::v104: // v1.04 - v1.06
            return getDefenseRatingv100();
        }

        return 0;
    }

    static const auto& stat = ItemHelpers::getItemStat(31);
    return (std::uint16_t)(read_uint32_bits(defense_rating_bit_offset, ((FileVersion >= EnumCharVersion::v110) ? DEFENSE_RATING_NUM_BITS : DEFENSE_RATING_NUM_BITS_108)) - stat.saveAdd);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDurability(ItemDurability& durability) const
{
    durability.clear();
    if (durability_bit_offset == 0)
    {
        return false;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        durability.Current = (std::uint16_t)read_uint32_bits(durability_bit_offset, DURABILITY_CURRENT_NUM_BITS_108);
        durability.Max = (std::uint16_t)read_uint32_bits(durability_bit_offset + DURABILITY_CURRENT_NUM_BITS_108, DURABILITY_MAX_NUM_BITS);
        if (durability.Max == 0)
        {
            durability.Current = 0;

            std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
            if (!getItemCode(strcode))
            {
                return false;
            }

            const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
            std::uint8_t base = itemType.getBaseType();
            bool isArmor = (base & 4) != 0 ? true : false;
            bool isWeapon = (base & 2) != 0 ? true : false;
            if (!(isArmor || isWeapon) || itemType.isMissileWeapon())
            {
                return false;
            }

            // Indestructible without the need for the magical attribute of indestructibility
        }

        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        durability.Max = (std::uint16_t)read_uint32_bits(durability_bit_offset, DURABILITY_MAX_NUM_BITS);
        if (durability.Max == 0)
        {
            std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
            if (!getItemCode(strcode))
            {
                return false;
            }

            const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
            if (itemType.isMissileWeapon())
            {
                return false;
            }

            // Indestructible without the need for the magical attribute of indestructibility
            return true;
        }

        durability.Current = (std::uint16_t)read_uint32_bits(durability_bit_offset + DURABILITY_MAX_NUM_BITS, DURABILITY_CURRENT_READ_NUM_BITS);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setDurability(const ItemDurability& attrib)
{
    if (attrib.Max == 0)
    {
        return setIndestructible();
    }

    // Stackable weapon  have secret durablity that we don't fix
    ItemDurability oldAttrib;
    if (isStackable() || !getDurability(oldAttrib) || oldAttrib.Max == 0)
    {
        return false;
    }

    if (attrib.Current == oldAttrib.Current &&
        attrib.Max == oldAttrib.Max)
    {
        // nothing to do
        return false;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        if (!updateBits(durability_bit_offset, DURABILITY_CURRENT_NUM_BITS_108, attrib.Current))
        {
            return false;
        }

        if (!updateBits(durability_bit_offset + DURABILITY_CURRENT_NUM_BITS_108, DURABILITY_MAX_NUM_BITS, attrib.Current))
        {
            return false;
        }

        // Make sure item is not marked broken
        updateBits(start_bit_offset + IS_DISABLED_FLAG_OFFSET, 1, 0);
        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        if (!updateBits(durability_bit_offset, DURABILITY_MAX_NUM_BITS, attrib.Max))
        {
            return false;
        }

        if (!updateBits(durability_bit_offset + DURABILITY_MAX_NUM_BITS, DURABILITY_MAX_NUM_BITS, attrib.Current))
        {
            return false;
        }

        // Make sure item is not marked broken
        updateBits(start_bit_offset + IS_DISABLED_FLAG_OFFSET, 1, 0);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::fixDurability()
{
    // Stackable weapon  have secret durablity that we don't fix
    ItemDurability attrib;
    if (isStackable() || !getDurability(attrib) || attrib.Max == 0)
    {
        return false;
    }

    if (attrib.Current == attrib.Max)
    {
        // nothing to do
        return false;
    }

    attrib.Current = attrib.Max;
    return setDurability(attrib);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMaxDurability()
{
    static ItemDurability maxDurability = { MAX_DURABILITY, MAX_DURABILITY };
    return setDurability(maxDurability);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDamage(ItemDamage& damage) const
{
    damage.clear();

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getItemCode(strcode))
    {
        const auto& result = ItemHelpers::getItemTypeHelper(strcode);
        if (!result.isWeapon())
        {
            return false;
        }

        damage = result.dam;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRealmDataFlag() const
{
    if (isSimpleItem() || (realm_bit_offset == 0))
    {
        return false;
    }

    return read_uint32_bits(realm_bit_offset, 1) == 0 ? false : true;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getMaxSocketCount() const
{
    std::uint8_t maxSockets = 0;
    if (isSimpleItem())
    {
        return maxSockets;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        return maxSockets;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        return ItemHelpers::getMaxSocketsv100(strcode);
    }

    const auto& result = ItemHelpers::getItemTypeHelper(strcode);
    if (!result.canHaveSockets())
    {
        return maxSockets;
    }
    return result.getMaxSockets(getLevel());
}
//---------------------------------------------------------------------------
bool d2ce::Item::addMaxSocketCount()
{
    auto maxSockets = getMaxSocketCount();
    if (maxSockets <= totalNumberOfSockets())
    {
        // no more room
        return false;
    }

    return setSocketCount(maxSockets);
}
//---------------------------------------------------------------------------
bool d2ce::Item::addSocket()
{
    auto maxSockets = getMaxSocketCount();
    auto curSockets = totalNumberOfSockets();
    if (maxSockets <= curSockets)
    {
        // no more room
        return false;
    }

    return setSocketCount(uint8_t(curSockets + 1));
}
//---------------------------------------------------------------------------
bool d2ce::Item::removeEmptySockets()
{
    if (!isSocketed())
    {
        return false;
    }
    
    auto numSocketed = socketedItemCount();
    auto numSockets = totalNumberOfSockets();
    if (numSocketed >= numSockets)
    {
        // no empty sockets
        return false;
    }

    return setSocketCount(numSocketed);
}
//---------------------------------------------------------------------------
bool d2ce::Item::removeSocket()
{
    if (!isSocketed())
    {
        return false;
    }

    auto numSocketed = socketedItemCount();
    auto numSockets = totalNumberOfSockets();
    if (numSocketed >= numSockets)
    {
        // no empty sockets
        return false;
    }

    return setSocketCount(uint8_t(numSockets - 1));
}
//---------------------------------------------------------------------------
bool d2ce::Item::setSocketCount(std::uint8_t numSockets)
{
    if (isSimpleItem())
    {
        return false;
    }

    switch (Version())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        // number of sockets is always the max number possible
        if (isSocketed())
        {
            if ((numSockets > 0) || (socketedItemCount() > 0))
            {
                // nothing to do here
                return false;
            }

            // Set the item as not socketed
            updateBits(start_bit_offset + IS_SOCKETED_FLAG_OFFSET, 1, 0);
            return true;
        }

        if ((numSockets == 0) || !canHaveSockets())
        {
            // nothing to do here
            return false;
        }

        // Set the item as not socketed
        updateBits(start_bit_offset + IS_SOCKETED_FLAG_OFFSET, 1, 1);
        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10 normal or expansion
    case EnumItemVersion::v115: // v1.15 Diable II: Resurrected
        if (socket_count_bit_offset_marker == 0)
        {
            return false;
        }
        break;
    }

    numSockets = std::min(numSockets, getMaxSocketCount());
    if (isSocketed())
    {
        // Already has sockets, so just update count
        auto numSocketed = socketedItemCount();
        numSockets = std::max(numSockets, numSocketed); // can only remove empty sockets
        if (numSockets == totalNumberOfSockets())
        {
            // nothing to do
            return false;
        }

        if (socket_count_bit_offset == 0)
        {
            return false;
        }

        size_t current_bit_offset = socket_count_bit_offset;
        if (numSockets > 0)
        {
            return setBits(current_bit_offset, SOCKET_COUNT_NUM_BITS, numSockets);
        }

        // Complex change, we are removing all sockets
        size_t old_current_bit_offset = current_bit_offset + SOCKET_COUNT_NUM_BITS;
        auto old_item_end_bit_offset = item_end_bit_offset;
        size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;

        // move bits up
        std::uint32_t value = 0;
        std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
        while (bitsToCopy > 0)
        {
            bitsToCopy -= bits;
            value = read_uint32_bits(old_current_bit_offset, bits);
            old_current_bit_offset += bits;
            updateBits(current_bit_offset, bits, value);
            current_bit_offset += bits;
            bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
        }

        // clear any bits not written to
        if ((current_bit_offset % 8) > 0)
        {
            value = 0;
            bits = (std::uint8_t)(8 - (current_bit_offset % 8));
            updateBits(current_bit_offset, bits, 0);
        }
        
        // Set item as not socketed
        socket_count_bit_offset = 0;
        updateBits(start_bit_offset + IS_SOCKETED_FLAG_OFFSET, 1, 0);

        // truncate data
        item_end_bit_offset -= SOCKET_COUNT_NUM_BITS;
        size_t newSize = (item_end_bit_offset + 7) / 8;
        data.resize(newSize, 0);

        if (bonus_bits_bit_offset != 0)
        {
            bonus_bits_bit_offset -= SOCKET_COUNT_NUM_BITS;
        }

        if (magical_props_bit_offset != 0)
        {
            magical_props_bit_offset -= SOCKET_COUNT_NUM_BITS;
        }

        if (set_bonus_props_bit_offset != 0)
        {
            set_bonus_props_bit_offset -= SOCKET_COUNT_NUM_BITS;
        }

        if (runeword_props_bit_offset != 0)
        {
            runeword_props_bit_offset -= SOCKET_COUNT_NUM_BITS;
        }

        return true;
    }

    if (numSockets == 0)
    {
        // must have at least one socket
        return false;
    }

    // not socketed already, complex change
    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // resize to fit socket count
    auto old_item_end_bit_offset = item_end_bit_offset;
    item_end_bit_offset += SOCKET_COUNT_NUM_BITS;
    size_t newSize = (item_end_bit_offset + 7) / 8;
    data.resize(newSize, 0);

    // Locate position for socket count
    size_t current_bit_offset = socket_count_bit_offset_marker;
    size_t old_current_bit_offset = current_bit_offset;
    size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;
   
    // Set item as socketed
    updateBits(start_bit_offset + IS_SOCKETED_FLAG_OFFSET, 1, 1);
    socket_count_bit_offset = current_bit_offset;
    updateBits(socket_count_bit_offset, SOCKET_COUNT_NUM_BITS, numSockets);
    current_bit_offset += SOCKET_COUNT_NUM_BITS;

    // now copy the remaining bits
    std::uint32_t value = 0;
    std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBits(current_bit_offset, bits, value);
        current_bit_offset += bits;
        bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        value = 0;
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBits(current_bit_offset, bits, 0);
    }

    if (bonus_bits_bit_offset != 0)
    {
        bonus_bits_bit_offset += SOCKET_COUNT_NUM_BITS;
    }

    if (magical_props_bit_offset != 0)
    {
        magical_props_bit_offset += SOCKET_COUNT_NUM_BITS;
    }

    if (set_bonus_props_bit_offset != 0)
    {
        set_bonus_props_bit_offset += SOCKET_COUNT_NUM_BITS;
    }

    if (runeword_props_bit_offset != 0)
    {
        runeword_props_bit_offset += SOCKET_COUNT_NUM_BITS;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::addPersonalization(const std::string& name)
{
    if (isPersonalized() || isSimpleItem() || name.empty() || (personalized_bit_offset_marker == 0))
    {
        return false;
    }

    // Remove any invalid characters from the name
    std::string strNewText;
    for (size_t iPos = 0, numberOfUnderscores = 0, nLen = name.size(); iPos < nLen && strNewText.size() < NAME_LENGTH; ++iPos)
    {
        char c = name[iPos];
        if (std::isalpha(c))
        {
            strNewText += c;
        }
        else if ((c == '_' || c == '-') && !strNewText.empty() && numberOfUnderscores < 1)
        {
            strNewText += c;
            ++numberOfUnderscores;
        }
    } 
    
    // trim bad characters
    strNewText.erase(strNewText.find_last_not_of("_-") + 1);
    if (strNewText.size() < 2)
    {
        return false;
    }

    // up to 15 7 bit characters
    size_t numberOfBitsToAdd = (strNewText.size() + 1) * 7;

    std::array<char, NAME_LENGTH> playerName;
    playerName.fill(0);
    strcpy_s(playerName.data(), strNewText.length() + 1, strNewText.c_str());
    playerName[15] = 0; // must be zero

    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // resize to fit personalization
    auto old_item_end_bit_offset = item_end_bit_offset;
    item_end_bit_offset += numberOfBitsToAdd;
    size_t newSize = (item_end_bit_offset + 7) / 8;
    data.resize(newSize, 0);

    // Locate position for personalized name
    size_t current_bit_offset = personalized_bit_offset_marker;
    size_t old_current_bit_offset = current_bit_offset;
    size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;

    // Set item as personalized
    updateBits(start_bit_offset + IS_PERSONALIZED_FLAG_OFFSET, 1, 1);
    personalized_bit_offset = current_bit_offset;

    // up to 15 7 bit characters
    for (size_t idx = 0; idx < 15; ++idx)
    {
        if (!updateBits(current_bit_offset, 7, std::uint32_t(playerName[idx])))
        {
            return false;
        }

        current_bit_offset += 7;
        if (playerName[idx] == 0)
        {
            break;
        }
    }

    // now copy the remaining bits
    std::uint32_t value = 0;
    std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBits(current_bit_offset, bits, value);
        current_bit_offset += bits;
        bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        value = 0;
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBits(current_bit_offset, bits, 0);
    }

    if (tome_bit_offset != 0)
    {
        tome_bit_offset += numberOfBitsToAdd;
    }

    if (realm_bit_offset != 0)
    {
        realm_bit_offset += numberOfBitsToAdd;
    }

    if (defense_rating_bit_offset != 0)
    {
        defense_rating_bit_offset += numberOfBitsToAdd;
    }

    if (durability_bit_offset != 0)
    {
        durability_bit_offset += numberOfBitsToAdd;
    }

    if (stackable_bit_offset != 0)
    {
        stackable_bit_offset += numberOfBitsToAdd;
    }

    if (gld_stackable_bit_offset != 0)
    {
        gld_stackable_bit_offset += numberOfBitsToAdd;
    }

    if (socket_count_bit_offset_marker != 0)
    {
        socket_count_bit_offset_marker += numberOfBitsToAdd;
    }

    if (socket_count_bit_offset != 0)
    {
        socket_count_bit_offset += numberOfBitsToAdd;
    }

    if (bonus_bits_bit_offset != 0)
    {
        bonus_bits_bit_offset += numberOfBitsToAdd;
    }

    if (magical_props_bit_offset != 0)
    {
        magical_props_bit_offset += numberOfBitsToAdd;
    }

    if (set_bonus_props_bit_offset != 0)
    {
        set_bonus_props_bit_offset += numberOfBitsToAdd;
    }

    if (runeword_props_bit_offset != 0)
    {
        runeword_props_bit_offset += numberOfBitsToAdd;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::removePersonalization()
{
    if (!isPersonalized() || (personalized_bit_offset_marker == 0))
    {
        return false;
    }

    // up to 15 7 bit characters
    size_t current_bit_offset = personalized_bit_offset_marker;
    size_t old_current_bit_offset = current_bit_offset;
    std::array<char, NAME_LENGTH> name;
    name.fill(0);
    char c = 0;
    for (std::uint8_t idx = 0; idx < 15; ++idx)
    {
        c = (char)read_uint32_bits(old_current_bit_offset, 7);
        old_current_bit_offset += 7;
        if (c == 0)
        {
            break;
        }
    }
    size_t numberOfBitsToRemove = old_current_bit_offset - current_bit_offset;

    auto old_item_end_bit_offset = item_end_bit_offset;
    size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;

    // move bits up
    std::uint32_t value = 0;
    std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = read_uint32_bits(old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBits(current_bit_offset, bits, value);
        current_bit_offset += bits;
        bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        value = 0;
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBits(current_bit_offset, bits, 0);
    }

    // Set item as not socketed
    personalized_bit_offset = 0;
    updateBits(start_bit_offset + IS_PERSONALIZED_FLAG_OFFSET, 1, 0);

    // truncate data
    item_end_bit_offset -= numberOfBitsToRemove;
    size_t newSize = (item_end_bit_offset + 7) / 8;
    data.resize(newSize, 0);

    if (tome_bit_offset != 0)
    {
        tome_bit_offset -= numberOfBitsToRemove;
    }

    if (realm_bit_offset != 0)
    {
        realm_bit_offset -= numberOfBitsToRemove;
    }

    if (defense_rating_bit_offset != 0)
    {
        defense_rating_bit_offset -= numberOfBitsToRemove;
    }

    if (durability_bit_offset != 0)
    {
        durability_bit_offset -= numberOfBitsToRemove;
    }

    if (stackable_bit_offset != 0)
    {
        stackable_bit_offset -= numberOfBitsToRemove;
    }

    if (gld_stackable_bit_offset != 0)
    {
        gld_stackable_bit_offset -= numberOfBitsToRemove;
    }

    if (socket_count_bit_offset_marker != 0)
    {
        socket_count_bit_offset_marker -= numberOfBitsToRemove;
    }

    if (socket_count_bit_offset != 0)
    {
        socket_count_bit_offset -= numberOfBitsToRemove;
    }

    if (bonus_bits_bit_offset != 0)
    {
        bonus_bits_bit_offset -= numberOfBitsToRemove;
    }

    if (magical_props_bit_offset != 0)
    {
        magical_props_bit_offset -= numberOfBitsToRemove;
    }

    if (set_bonus_props_bit_offset != 0)
    {
        set_bonus_props_bit_offset -= numberOfBitsToRemove;
    }

    if (runeword_props_bit_offset != 0)
    {
        runeword_props_bit_offset -= numberOfBitsToRemove;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setIndestructible()
{
    // Stackable weapon  have secret durablity that we don't fix
    if (isSimpleItem() || isStackable() || isIndestructible() || (durability_bit_offset == 0))
    {
        return false;
    }

    size_t current_bit_offset = magical_props_bit_offset;
    std::vector<MagicalAttribute> attribs;
    if (!readPropertyList(current_bit_offset, attribs))
    {
        // coruption
        return false;
    }

    if (attribs.empty())
    {
        // Indestructible without the need for the magical attribute of indestructibility
        if (read_uint32_bits(magical_props_bit_offset, DURABILITY_MAX_NUM_BITS) == 0)
        {
            // should not happen, already indestructable
            return false;
        }

        current_bit_offset = durability_bit_offset;
        updateBits(current_bit_offset, DURABILITY_MAX_NUM_BITS, 0);
        current_bit_offset += DURABILITY_MAX_NUM_BITS;

        size_t old_current_bit_offset = current_bit_offset + ((FileVersion >= EnumCharVersion::v110) ? DURABILITY_CURRENT_NUM_BITS : DURABILITY_CURRENT_NUM_BITS_108);
        auto old_item_end_bit_offset = item_end_bit_offset;
        size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;
        size_t numberOfBitsToRemove = old_current_bit_offset - current_bit_offset;

        // move bits up
        std::uint32_t value = 0;
        std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
        while (bitsToCopy > 0)
        {
            bitsToCopy -= bits;
            value = read_uint32_bits(old_current_bit_offset, bits);
            old_current_bit_offset += bits;
            updateBits(current_bit_offset, bits, value);
            current_bit_offset += bits;
            bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
        }

        // clear any bits not written to
        if ((current_bit_offset % 8) > 0)
        {
            value = 0;
            bits = (std::uint8_t)(8 - (current_bit_offset % 8));
            updateBits(current_bit_offset, bits, 0);
        }

        // truncate data
        item_end_bit_offset -= numberOfBitsToRemove;
        size_t newSize = (item_end_bit_offset + 7) / 8;
        data.resize(newSize, 0);

        if (stackable_bit_offset != 0)
        {
            stackable_bit_offset -= numberOfBitsToRemove;
        }

        if (gld_stackable_bit_offset != 0)
        {
            gld_stackable_bit_offset -= numberOfBitsToRemove;
        }

        if (socket_count_bit_offset_marker != 0)
        {
            socket_count_bit_offset_marker -= numberOfBitsToRemove;
        }

        if (socket_count_bit_offset != 0)
        {
            socket_count_bit_offset -= numberOfBitsToRemove;
        }

        if (bonus_bits_bit_offset != 0)
        {
            bonus_bits_bit_offset -= numberOfBitsToRemove;
        }

        if (magical_props_bit_offset != 0)
        {
            magical_props_bit_offset -= numberOfBitsToRemove;
        }

        if (set_bonus_props_bit_offset != 0)
        {
            set_bonus_props_bit_offset -= numberOfBitsToRemove;
        }

        if (runeword_props_bit_offset != 0)
        {
            runeword_props_bit_offset -= numberOfBitsToRemove;
        }

        return true;
    }

    // go back before end of list
    current_bit_offset -= PROPERTY_ID_NUM_BITS;
    size_t old_current_bit_offset = current_bit_offset;
    size_t numberOfBitsToAdd = PROPERTY_ID_NUM_BITS + 1;

    std::uint16_t id = ATTRIBUTE_ID_INDESTRUCTIBLE;
    const ItemStat* stat = &ItemHelpers::getItemStat(id);
    if (stat->saveBits != 1 || stat->saveParamBits != 0 || stat->encode != 0)
    {
        // corrupt file
        return false;
    }

    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // resize to fit new magical attribute
    auto old_item_end_bit_offset = item_end_bit_offset;
    item_end_bit_offset += numberOfBitsToAdd;
    size_t newSize = (item_end_bit_offset + 7) / 8;
    data.resize(newSize, 0);

    // add indestructible attribute
    updateBits(current_bit_offset, PROPERTY_ID_NUM_BITS, id);
    current_bit_offset += PROPERTY_ID_NUM_BITS;
    updateBits(current_bit_offset, 1, 1);
    current_bit_offset += 1;

    // now copy the remaining bits
    size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;
    std::uint32_t value = 0;
    std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBits(current_bit_offset, bits, value);
        current_bit_offset += bits;
        bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        value = 0;
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBits(current_bit_offset, bits, 0);
    }

    if (set_bonus_props_bit_offset != 0)
    {
        set_bonus_props_bit_offset += numberOfBitsToAdd;
    }

    if (runeword_props_bit_offset != 0)
    {
        runeword_props_bit_offset += numberOfBitsToAdd;
    }

    return true;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedItemName() const
{
    std::stringstream ss;
    EarAttributes earAttrib;
    if (getEarAttributes(earAttrib))
    {
        ss << earAttrib.getName().data() << "'s Ear\n" << earAttrib.getClassName() << "\nLevel " << std::dec << earAttrib.getLevel();
        return ss.str();
    }

    if (!isIdentified())
    {
        ss << getItemTypeName() << "\nUnidentified";
        return ss.str();
    }

    d2ce::RunewordAttributes runewordAttrib;
    if (getRunewordAttributes(runewordAttrib) && !runewordAttrib.Name.empty())
    {
        ss << runewordAttrib.Name << "\n";
    }

    std::string personalizedName = getPersonalizedName();
    if (!personalizedName.empty())
    {
        
        ss << personalizedName << "'";
        if (personalizedName.back() != 's')
        {
            ss << "s";
        }
        ss << " ";
    }

    MagicalAffixes affixes;
    d2ce::RareAttributes rareAttrib;
    d2ce::UniqueAttributes uniqueAttrib;
    d2ce::SetAttributes setAttrib;
    auto quality = getQuality();
    switch (quality)
    {
    case EnumItemQuality::INFERIOR:
        switch (getInferiorQualityId())
        {
        case 0:
            ss << "Crude ";
            break;

        case 1:
            ss << "Cracked ";
            break;

        case 2:
            ss << "Damaged ";
            break;

        case 3:
            ss << "Low Quality ";
            break;
        }
        ss << getItemTypeName();
        break;

    case EnumItemQuality::NORMAL:
        if (socketedItemCount() > 0)
        {
            ss << "Gemmed ";
        }
        ss << getItemTypeName();
        break;

    case EnumItemQuality::SUPERIOR:
        ss << "Superior " << getItemTypeName();
        break;

    case EnumItemQuality::MAGIC:
        if (getMagicalAffixes(affixes))
        {
            if (!affixes.PrefixName.empty())
            {
                ss << affixes.PrefixName << " ";
            }

            ss << getItemTypeName();

            if (!affixes.SuffixName.empty())
            {
                ss << " " << affixes.SuffixName;
            }
        }
        else
        {
            // should not happen
            ss << getItemTypeName();
        }
        break;

    case EnumItemQuality::SET:
        if (getSetAttributes(setAttrib) && !setAttrib.Name.empty())
        {
            ss << setAttrib.Name << '\n' << getItemTypeName();
        }
        else
        {
            // should not happen
            ss << getItemTypeName();
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        if (getRareOrCraftedAttributes(rareAttrib))
        {
            if (!rareAttrib.Name.empty())
            {
                ss << rareAttrib.Name << " ";
            }

            if (!rareAttrib.Name2.empty())
            {
                ss << rareAttrib.Name2;
            }

            ss << '\n' << getItemTypeName();
        }
        else
        {
            // should not happen
            ss << getItemTypeName();
        }
        break;

    case EnumItemQuality::UNIQUE:
        if (getUniqueAttributes(uniqueAttrib) && !uniqueAttrib.Name.empty())
        {
            ss << uniqueAttrib.Name;
            auto typeName = getItemTypeName();
            if (typeName != uniqueAttrib.Name)
            {
                ss << '\n' << typeName;
            }
        }
        else
        {
            // should not happen
            ss << getItemTypeName();
        }
        break;

    default:
        // should not happen
        ss << getItemTypeName();
        break;
    }

    return ss.str();
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedSocketedRunes() const
{
    std::string name;
    std::stringstream ss;
    bool bFirstItem = true;
    for (const auto& item : SocketedItems)
    {
        if (item.isRune())
        {
            if (bFirstItem)
            {
                ss << "'";
                bFirstItem = false;
            }

            name = item.getItemTypeName();
            ss << name.substr(0, name.find(" "));
        }
    }

    if (!bFirstItem)
    {
        ss << "'";
    }

    return ss.str();
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getDisplayedDefenseRating(std::uint32_t charLevel) const
{
    auto defenseRating = getDefenseRating();
    if (defenseRating == 0)
    {
        return 0;
    }

    // Calculate item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);

        std::uint64_t eDef = 0;
        std::uint64_t def = 0;
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib.Id);
            switch (attrib.Id)
            {
            case 16:
            case 215:
                eDef += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 31:
            case 214:
                def += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;
            }
        }

        defenseRating += std::uint16_t((defenseRating * eDef) / 100 + def);
    }

    return defenseRating;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedDurability(ItemDurability& durability, std::uint32_t charLevel) const
{
    // Stackable weapon have secret durablity that we don't fix
    if (!getDurability(durability) || isStackable())
    {
        return false;
    }

    if (durability.Max == 0)
    {
        // Indestructible without the need for the magical attribute of indestructibility
        return true;
    }

    // Calculate item bonus
    std::uint64_t eDur = 0;
    std::uint64_t dur = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib.Id);
            switch (attrib.Id)
            {
            case 73:
                dur += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 75:
                eDur += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case ATTRIBUTE_ID_INDESTRUCTIBLE:
                // Indestructible
                durability.Max = 0;
                durability.Current = 0;
                return true;
            }
        }

        durability.Max += std::uint16_t((durability.Max * eDur) / 100 + dur);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedDamage(ItemDamage& damage, std::uint32_t charLevel) const
{
    if (!getDamage(damage))
    {
        return false;
    }

    // Calculate item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);

        // Calculate item bonus
        std::uint64_t eDmg = 0;
        std::uint64_t eDmgMax = 0;
        std::uint64_t dmgMin = 0;
        std::uint64_t dmgMax = 0;
        std::uint64_t dmgMin2 = 0;
        std::uint64_t dmgMax2 = 0;
        std::uint64_t dmgMinThrow = 0;
        std::uint64_t dmgMaxThrow = 0;
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib.Id);
            switch (attrib.Id)
            {
            case 17:
                eDmg += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 21:
                dmgMin += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 22:
                dmgMax += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 23:
                dmgMin2 += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMin
                break;

            case 24:
                dmgMax2 += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMax
                break;

            case 219:
                eDmgMax += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                break;

            case 159:
                dmgMinThrow += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMin
                break;

            case 160:
                dmgMaxThrow += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMax
                break;
            }
        }

        if (dmgMin == 0)
        {
            dmgMin = std::max(dmgMinThrow, dmgMin2);
        }

        if (dmgMax == 0)
        {
            dmgMax = std::max(dmgMaxThrow, dmgMax2);
        }

        if (isEthereal())
        {
            if (damage.OneHanded.Max != 0)
            {
                damage.OneHanded.Min += std::uint16_t((std::uint64_t(damage.OneHanded.Min) * 50) / 100);
                damage.OneHanded.Max += std::uint16_t((std::uint64_t(damage.OneHanded.Max) * 50) / 100);
            }

            if (damage.bTwoHanded)
            {
                damage.TwoHanded.Min += std::uint16_t((std::uint64_t(damage.TwoHanded.Min) * 50) / 100);
                damage.TwoHanded.Max += std::uint16_t((std::uint64_t(damage.TwoHanded.Max) * 50) / 100);
            }

            if (damage.Missile.Max != 0)
            {
                damage.Missile.Min += std::uint16_t((std::uint64_t(damage.Missile.Min) * 50) / 100);
                damage.Missile.Max += std::uint16_t((std::uint64_t(damage.Missile.Max) * 50) / 100);
            }
        }

        if (damage.OneHanded.Max != 0)
        {
            damage.OneHanded.Min += std::uint16_t((damage.OneHanded.Min * eDmg) / 100 + dmgMin);
            damage.OneHanded.Max += std::uint16_t((damage.OneHanded.Max * (eDmg + eDmgMax)) / 100 + dmgMax);
            if (damage.OneHanded.Min > damage.OneHanded.Max)
            {
                damage.OneHanded.Max = damage.OneHanded.Min + 1;
            }
        }

        if (damage.bTwoHanded)
        {
            damage.TwoHanded.Min += std::uint16_t((damage.TwoHanded.Min * eDmg) / 100 + dmgMin);
            damage.TwoHanded.Max += std::uint16_t((damage.TwoHanded.Max * (eDmg + eDmgMax)) / 100 + dmgMax);
            if (damage.TwoHanded.Min > damage.TwoHanded.Max)
            {
                damage.TwoHanded.Max = damage.TwoHanded.Min + 1;
            }
        }

        if (damage.Missile.Max != 0)
        {
            damage.Missile.Min += std::uint16_t((damage.Missile.Min * eDmg) / 100 + dmgMin);
            damage.Missile.Max += std::uint16_t((damage.Missile.Max * (eDmg + eDmgMax)) / 100 + dmgMax);
            if (damage.Missile.Min > damage.Missile.Max)
            {
                damage.Missile.Max = damage.Missile.Min + 1;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedItemAttributes(EnumCharClass charClass, std::uint32_t charLevel) const
{
    if (isEar())
    {
        return "";
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    getItemCode(strcode);
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (itemType.isPotion())
    {
        auto points = itemType.getPotionPoints(charClass);
        if (points == 0)
        {
            return "";
        }

        std::stringstream ss;
        if (itemType.isRejuvenationPotion())
        {
            ss << "Heals " << std::dec << points << "% Life and Mana";
        }
        else
        {
            ss << "Points: " << std::dec << points;
        }

        return ss.str();
    }

    bool bFirst = true;
    std::stringstream ss;
    if (itemType.isSocketFiller())
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Can be Inserted into Socketed Items";

        if (!itemType.isJewel())
        {
            std::string attribSep = (itemType.isRune() ? ",\n" : ", ");
            ss << "\n";
            std::vector<MagicalAttribute> attribs;
            if (itemType.getSocketedMagicalAttributes(*this, attribs, d2ce::EnumItemType::Weapon))
            {
                // check for the "all" cases
                ItemHelpers::checkForRelatedMagicalAttributes(attribs);

                bool bFirstAttrib = true;
                ss << "\nWeapons: ";
                for (auto& attrib : attribs)
                {
                    if (!attrib.Visible)
                    {
                        continue;
                    }

                    if (!ItemHelpers::formatDisplayedMagicalAttribute(attrib, d2ce::NUM_OF_LEVELS))
                    {
                        continue;
                    }

                    if (bFirstAttrib)
                    {
                        bFirstAttrib = false;
                    }
                    else
                    {
                        ss << attribSep;
                    }

                    ss << attrib.Desc;
                }
            }

            if (itemType.getSocketedMagicalAttributes(*this, attribs, d2ce::EnumItemType::Armor))
            {
                // check for the "all" cases
                ItemHelpers::checkForRelatedMagicalAttributes(attribs);

                bool bFirstAttrib = true;
                ss << "\nArmor: ";
                std::stringstream ss2;
                for (auto& attrib : attribs)
                {
                    if (!attrib.Visible)
                    {
                        continue;
                    }

                    if (!ItemHelpers::formatDisplayedMagicalAttribute(attrib, d2ce::NUM_OF_LEVELS))
                    {
                        continue;
                    }

                    if (bFirstAttrib)
                    {
                        bFirstAttrib = false;
                    }
                    else
                    {
                        ss2 << attribSep;
                    }

                    ss2 << attrib.Desc;
                }
                auto commonText = ss2.str();
                ss << commonText;
                ss << "\nHelms: ";
                ss << commonText;
            }

            if (itemType.getSocketedMagicalAttributes(*this, attribs, d2ce::EnumItemType::Shield))
            {
                // check for the "all" cases
                ItemHelpers::checkForRelatedMagicalAttributes(attribs);

                bool bFirstAttrib = true;
                ss << "\nShields: ";

                for (auto& attrib : attribs)
                {
                    if (!attrib.Visible)
                    {
                        continue;
                    }

                    if (!ItemHelpers::formatDisplayedMagicalAttribute(attrib, d2ce::NUM_OF_LEVELS))
                    {
                        continue;
                    }

                    if (bFirstAttrib)
                    {
                        bFirstAttrib = false;
                    }
                    else
                    {
                        ss << attribSep;
                    }

                    ss << attrib.Desc;
                }
            }
            ss << "\n";
        }
    }

    auto defenseRating = getDisplayedDefenseRating(charLevel);
    if (defenseRating > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }

        ss << "Defense: " << std::dec << defenseRating;
    }

    if (itemType.isBelt())
    {
        ItemDimensions dimensions;
        if (getDimensions(dimensions))
        {
            std::uint16_t numSlots = dimensions.InvWidth * dimensions.InvHeight - 4;
            if (numSlots > 0)
            {
                if (bFirst)
                {
                    bFirst = false;
                }
                else
                {
                    ss << "\n";
                }

                ss << "Belt Size: +" << std::dec << numSlots << " Slots";
            }
        }
    }

    if (itemType.isWeapon())
    {
        ItemDamage dam;
        getDisplayedDamage(dam, charLevel);
        if (dam.Missile.Max != 0 && dam.Missile.Min)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            ss << "Throw Damage: ";
            if (dam.Missile.Min != dam.Missile.Max)
            {
                ss << std::dec << dam.Missile.Min << " to ";
            }

            ss << std::dec << dam.Missile.Max;
        }

        if (dam.OneHanded.Max != 0 && dam.OneHanded.Min != 0)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            ss << "One-Hand Damage: ";
            if (dam.OneHanded.Min != dam.OneHanded.Max)
            {
                ss << std::dec << dam.OneHanded.Min << " to ";
            }

            ss << std::dec << dam.OneHanded.Max;
        }

        if (dam.bTwoHanded && dam.TwoHanded.Max != 0 && dam.TwoHanded.Min != 0)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            ss << "Two-Hand Damage: ";
            if (dam.TwoHanded.Min != dam.TwoHanded.Max)
            {
                ss << std::dec << dam.TwoHanded.Min << " to ";
            }

            ss << std::dec << dam.TwoHanded.Max;
        }
    }

    ItemDurability durability;
    if (getDisplayedDurability(durability, charLevel) && durability.Max > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }

        ss << "Durability: " << std::dec << durability.Current << " of " << std::dec << durability.Max;
    }

    if (itemType.isStackable())
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Quantity: " << std::dec << getQuantity();
    }

    ItemRequirements req;
    if (!getRequirements(req))
    {
        req = itemType.req;
    }

    if (req.Dexterity > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Required Dexterity: " << std::dec << req.Dexterity;
    }

    if (req.Strength > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Required Strength: " << std::dec << req.Strength;
    }

    if (req.Level > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }
        ss << "Required Level: " << std::dec << req.Level;
    }

    return ss.str();
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getMagicalAttributes(attribs))
    {
        if (!hasUndeadBonus())
        {
            return false;
        }
    }

    if (hasUndeadBonus())
    {
        // Push back special class attribute
        static const auto& stat = ItemHelpers::getItemStat(122);
        MagicalAttribute attrib;
        attrib.Id = stat.id;
        attrib.Desc = stat.desc;
        attrib.Name = stat.name;
        attrib.Values = { 50 };
        attribs.push_back(attrib);
    }

    // check for the "all" cases
    ItemHelpers::checkForRelatedMagicalAttributes(attribs);

    bool bFormatted = false;
    for (auto& attrib : attribs)
    {
        bFormatted |= ItemHelpers::formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.begin(), attribs.end(), ItemHelpers::magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedRunewordAttributes(RunewordAttributes& attribs, std::uint32_t charLevel) const
{
    if (!getRunewordAttributes(attribs))
    {
        return false;
    }

    // check for the "all" cases
    ItemHelpers::checkForRelatedMagicalAttributes(attribs.MagicalAttributes);

    bool bFormatted = false;
    for (auto& attrib : attribs.MagicalAttributes)
    {
        bFormatted |= ItemHelpers::formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.MagicalAttributes.begin(), attribs.MagicalAttributes.end(), ItemHelpers::magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getCombinedMagicalAttributes(attribs))
    {
        return false;
    }

    // check for the "all" cases
    ItemHelpers::checkForRelatedMagicalAttributes(attribs);

    bool bFormatted = false;
    for (auto& attrib : attribs)
    {
        bFormatted |= ItemHelpers::formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.begin(), attribs.end(), ItemHelpers::magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::Item::readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
{
    size_t readOffset = current_bit_offset;

    // Ensure we read enough
    if (!skipBits(charfile, current_bit_offset, bits))
    {
        return 0;
    }

    if (bits > 64)
    {
        return 0;
    }

    return read_uint64_bits(readOffset, bits);
}
//---------------------------------------------------------------------------
bool d2ce::Item::skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
{
    if (feof(charfile))
    {
        return false;
    }

    size_t bytesRequired = (current_bit_offset + bits + 7) / 8;
    std::uint8_t value = 0;
    while (data.size() < bytesRequired)
    {
        if (feof(charfile))
        {
            return false;
        }

        std::fread(&value, sizeof(value), 1, charfile);
        data.push_back(value);
    }

    current_bit_offset += bits;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setBits(size_t& current_bit_offset, size_t bits, std::uint32_t value)
{
    size_t readOffset = current_bit_offset;
    if (bits > 32)
    {
        return false;
    }

    size_t bytesRequired = (current_bit_offset + bits + 7) / 8;
    std::uint8_t byte = 0;
    while (data.size() < bytesRequired)
    {
        data.push_back(byte);
    }

    current_bit_offset += bits;
    return updateBits(readOffset, bits, value);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setBits64(size_t& current_bit_offset, size_t bits, std::uint64_t value)
{
    size_t readOffset = current_bit_offset;
    if (bits > 64)
    {
        return false;
    }

    size_t bytesRequired = (current_bit_offset + bits + 7) / 8;
    std::uint8_t byte = 0;
    while (data.size() < bytesRequired)
    {
        data.push_back(byte);
    }

    current_bit_offset += bits;
    return updateBits64(readOffset, bits, value);
}
//---------------------------------------------------------------------------
bool d2ce::Item::readItem(EnumCharVersion version, std::FILE* charfile)
{
    FileVersion = version;
    data.clear();
    SocketedItems.clear();

    item_current_socket_idx = 0;
    dwa_bit_offset = 0;
    dwb_bit_offset = 0;
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();

    // reserve enough space to reduce chance of reallocation (haven't seen an item size bigger then 80
    data.reserve(80);

    start_bit_offset = 0;
    size_t current_bit_offset = start_bit_offset;
    if (FileVersion < EnumCharVersion::v115) // pre-1.15 character file
    {
        std::uint8_t value = (std::uint8_t)readBits(charfile, current_bit_offset, 8);
        if (value != ITEM_MARKER[0])
        {
            // corrupt file
            return false;
        }

        value = (std::uint8_t)readBits(charfile, current_bit_offset, 8);
        if (value != ITEM_MARKER[1])
        {
            // corrupt file
            return false;
        }

        start_bit_offset = current_bit_offset;

        // At least the Simple/Compact size is required
        if (!skipBits(charfile, current_bit_offset, 95))
        {
            return false;
        }

        ItemVersion = EnumItemVersion::v110;
        switch (getRawVersion())
        {
        case 0: // pre-1.08
            if (FileVersion < EnumCharVersion::v107) // pre-1.07 character file
            {
                if (read_uint32_bits(start_bit_offset + ITEM_TYPE_BIT_OFFSET, 2) != 0)
                {
                    ItemVersion = EnumItemVersion::v104; // v1.04 - v1.06 item
                }
                else
                {
                    ItemVersion = EnumItemVersion::v100; // v1.00 - v1.03
                }
            }
            else
            {
                ItemVersion = EnumItemVersion::v107;     // v1.07 item
            }
            break;

        case 1:
        case 100:
            ItemVersion = EnumItemVersion::v108;         // v1.08/1.09 normal or expansion
            break;

        case 2:
        case 101:
            ItemVersion = EnumItemVersion::v110;         // v1.10 normal or expansion
            break;
        }

        switch (Version())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
            location_bit_offset = 0;
            equipped_id_offset = 0;
            alt_position_id_offset = 0;
            durability_bit_offset = 0;
            nr_of_items_in_sockets_offset = 0;
            quality_attrib_bit_offset = 0;
            item_level_bit_offset = 0;

            // 27 bytes total
            if (!skipBits(charfile, current_bit_offset, (ITEM_V100_NUM_BITS - current_bit_offset)))
            {
                return false;
            }

            if (isEar())
            {
                position_offset = ITEM_V100_EAR_COORDINATES_BIT_OFFSET + 2;
                type_code_offset = ITEM_V100_EAR_TYPECODE_BIT_OFFSET;
            }
            else
            {
                equipped_id_offset = ITEM_V100_BITFIELD3_BIT_OFFSET;
                nr_of_items_in_sockets_offset = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
                nr_of_items_in_sockets_bits = 2;
                type_code_offset = ITEM_V100_TYPECODE_BIT_OFFSET;
                durability_bit_offset = ITEM_V100_DURABILITY_BIT_OFFSET;
                position_offset = ITEM_V100_COORDINATES_BIT_OFFSET + 1;
                quality_bit_offset = start_bit_offset + QUALITY_BIT_OFFSET_100;
                quality_attrib_bit_offset = ITEM_V100_SPECIALITEMCODE_BIT_OFFSET;
                item_level_bit_offset = ITEM_V100_LEVEL_BIT_OFFSET;
                dwa_bit_offset = ITEM_V100_DWA_BIT_OFFSET;
                dwb_bit_offset = ITEM_V100_DWB_BIT_OFFSET;

                stackable_bit_offset = 0;
                gld_stackable_bit_offset = 0;
                std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
                getItemCode(strcode);
                const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
                if (itemType.isGoldItem())
                {
                    gld_stackable_bit_offset = start_bit_offset + QUANTITY_BIT_OFFSET_100;
                }
                else if (isStackable())
                {
                    stackable_bit_offset = start_bit_offset + QUANTITY_BIT_OFFSET_100;
                }
            }

            extended_data_offset = current_bit_offset;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06
            location_bit_offset = 0;
            equipped_id_offset = 0;
            alt_position_id_offset = 0;
            type_code_offset = 0;
            durability_bit_offset = 0;
            nr_of_items_in_sockets_offset = 0;
            quality_attrib_bit_offset = 0;
            item_level_bit_offset = 0;

            if (isEar())
            {
                position_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET + 2;
                type_code_offset = ITEM_V104_EAR_ATTRIBE_BIT_OFFSET;

                // 26 bytes total
                if (!skipBits(charfile, current_bit_offset, (ITEM_V104_EAR_NUM_BITS - current_bit_offset)))
                {
                    return false;
                }
            }
            else if (isSimpleItem())
            {
                position_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET + 2;
                type_code_offset = ITEM_V104_SM_TYPECODE_BIT_OFFSET;

                // 15 bytes total
                if (!skipBits(charfile, current_bit_offset, (ITEM_V104_SM_NUM_BITS - current_bit_offset)))
                {
                    return false;
                }
            }
            else
            {
                type_code_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET;
                equipped_id_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET;
                nr_of_items_in_sockets_offset = ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET;
                nr_of_items_in_sockets_bits = 3;
                durability_bit_offset = ITEM_V104_EX_DURABILITY_BIT_OFFSET;
                position_offset = ITEM_V104_EX_COORDINATES_BIT_OFFSET + 1;
                quality_bit_offset = start_bit_offset + QUALITY_BIT_OFFSET_104;
                quality_attrib_bit_offset = ITEM_V104_EX_UNIQUECODE_BIT_OFFSET;
                item_level_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET;
                dwa_bit_offset = ITEM_V100_DWA_BIT_OFFSET;
                dwb_bit_offset = ITEM_V100_DWB_BIT_OFFSET;

                // 31 bytes total
                if (!skipBits(charfile, current_bit_offset, (ITEM_V104_EX_NUM_BITS - current_bit_offset)))
                {
                    return false;
                }

                stackable_bit_offset = 0;
                gld_stackable_bit_offset = 0;
                std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
                getItemCode(strcode);
                const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
                if (itemType.isGoldItem())
                {
                    gld_stackable_bit_offset = start_bit_offset + QUANTITY_BIT_OFFSET_104;
                }
                else if (isStackable())
                {
                    stackable_bit_offset = start_bit_offset + QUANTITY_BIT_OFFSET_104;
                }
            }

            extended_data_offset = current_bit_offset;
            break;

        default: // v1.07 - v1.14d
            location_bit_offset = ITEM_V115_LOCATION_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            equipped_id_offset = ITEM_V115_EQUIPPED_ID_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            position_offset = ITEM_V115_POSITION_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            alt_position_id_offset = ITEM_V115_ALT_POSITION_ID_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            type_code_offset = ITEM_V115_TYPE_CODE_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            extended_data_offset = ITEM_V107_EXTENDED_DATA_BIT_OFFSET;
            break;
        }
    }
    else
    {
        location_bit_offset = ITEM_V115_LOCATION_BIT_OFFSET;
        equipped_id_offset = ITEM_V115_EQUIPPED_ID_BIT_OFFSET;
        position_offset = ITEM_V115_POSITION_BIT_OFFSET;
        alt_position_id_offset = ITEM_V115_ALT_POSITION_ID_BIT_OFFSET;
        type_code_offset = ITEM_V115_TYPE_CODE_BIT_OFFSET;

        ItemVersion = EnumItemVersion::v115; // v1.15 Diable II: Resurrected

        // skip up to type code
        if (!skipBits(charfile, current_bit_offset, type_code_offset))
        {
            return false;
        }

        if (isEar())
        {
            if (!skipBits(charfile, current_bit_offset, 10))
            {
                return false;
            }

            // up to 15 7 bit characters
            for (std::uint8_t idx = 0; !feof(charfile) && idx < 15 && readBits(charfile, current_bit_offset, 7) != 0; ++idx);
            extended_data_offset = current_bit_offset;
            item_end_bit_offset = current_bit_offset;
            return true;
        }

        for (size_t i = 0; i < 4; ++i)
        {
            if (getEncodedChar(charfile, current_bit_offset) == 0xFF)
            {
                return false;
            }
        }

        extended_data_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, isSimpleItem() ? 1 : 3))
        {
            return false;
        }
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (Version() >= EnumItemVersion::v107) // pre-1.15 character file
    {
        quest_difficulty_offset = 0;
        gld_stackable_bit_offset = 0;
        getItemCode(strcode);
        const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
        if (isSimpleItem())
        {
            if (itemType.isGoldItem())
            {
                // Is this correct for gld items? It's not currently used, so is it even needed?
                gld_stackable_bit_offset = extended_data_offset + 1;
                if (!skipBits(charfile, extended_data_offset, GLD_STACKABLE_NUM_BITS))
                {
                    return false;
                }

                nr_of_items_in_sockets_offset = current_bit_offset;
                nr_of_items_in_sockets_bits = 1;
            }
            else
            {
                nr_of_items_in_sockets_offset = extended_data_offset;
                nr_of_items_in_sockets_bits = 1;
                if (itemType.isQuestItem())
                {
                    quest_difficulty_offset = extended_data_offset;
                    nr_of_items_in_sockets_offset = quest_difficulty_offset + 2;
                    nr_of_items_in_sockets_bits = 1;
                    if (!skipBits(charfile, current_bit_offset, 2))
                    {
                        return false;
                    }
                }
            }

            item_end_bit_offset = current_bit_offset;
            return true;
        }
        else if (itemType.isQuestItem())
        {
            quest_difficulty_offset = extended_data_offset;
            nr_of_items_in_sockets_offset = quest_difficulty_offset + 2;
            nr_of_items_in_sockets_bits = 1;
        }
        else
        {
            nr_of_items_in_sockets_offset = extended_data_offset;
            nr_of_items_in_sockets_bits = 3;
        }

        item_id_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 32))
        {
            return false;
        }

        item_level_bit_offset = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 7))
        {
            return false;
        }

        quality_bit_offset = current_bit_offset;
        auto qualityval = readBits(charfile, current_bit_offset, 4);
        if (qualityval > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
        {
            return false;
        }

        auto quality = static_cast<EnumItemQuality>(qualityval);

        // If this is TRUE, it means the item has more than one picture associated with it.
        multi_graphic_bit_offset = current_bit_offset;
        if (readBits(charfile, current_bit_offset, 1) != 0)
        {
            // The next 3 bits contain the picture ID
            if (!skipBits(charfile, current_bit_offset, 3))
            {
                return false;
            }
        }

        // If this is TRUE, it means the item is class specific.
        autoAffix_bit_offset = current_bit_offset;
        if (readBits(charfile, current_bit_offset, 1) != 0)
        {
            // If the item is class specific, the next 11 bits will
            // contain the class specific data.
            if (!skipBits(charfile, current_bit_offset, 11))
            {
                return false;
            }
        }

        quality_attrib_bit_offset = 0;
        switch (quality)
        {
        case EnumItemQuality::INFERIOR:
        case EnumItemQuality::SUPERIOR:
            quality_attrib_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, INFERIOR_SUPERIOR_ID_NUM_BITS))
            {
                return false;
            }
            break;

        case EnumItemQuality::MAGIC:
            quality_attrib_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, 2 * MAGICAL_AFFIX_NUM_BITS))
            {
                return false;
            }
            break;

        case EnumItemQuality::RARE:
        case EnumItemQuality::CRAFT:
        case EnumItemQuality::TEMPERED:
            quality_attrib_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, 2 * RARE_CRAFTED_ID_NUM_BITS))
            {
                return false;
            }

            // Following the name IDs, we got 6 possible magical affixes, the pattern
            // is 1 bit id, 11 bit value... But the value will only exist if the prefix
            // is 1. So we'll read the id first and check it against 1.
            for (size_t i = 3; i > 0; --i)
            {
                if (readBits(charfile, current_bit_offset, 1) != 0)
                {
                    if (!skipBits(charfile, current_bit_offset, MAGICAL_AFFIX_NUM_BITS))
                    {
                        return false;
                    }
                }

                if (feof(charfile))
                {
                    return false;
                }

                if (readBits(charfile, current_bit_offset, 1) != 0)
                {
                    if (!skipBits(charfile, current_bit_offset, MAGICAL_AFFIX_NUM_BITS))
                    {
                        return false;
                    }
                }

                if (feof(charfile))
                {
                    return false;
                }
            }
            break;

        case EnumItemQuality::SET:
        case EnumItemQuality::UNIQUE:
            quality_attrib_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, SET_UNIQUE_ID_NUM_BITS))
            {
                return false;
            }
            break;
        }

        if (isRuneword())
        {
            runeword_id_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, RUNEWORD_ID_NUM_BITS + RUNEWORD_PADDING_NUM_BITS))
            {
                return false;
            }
        }

        personalized_bit_offset_marker = current_bit_offset;
        if (isPersonalized())
        {
            personalized_bit_offset = current_bit_offset;

            // up to 15 7 bit characters
            for (std::uint8_t idx = 0; !feof(charfile) && idx < 15 && readBits(charfile, current_bit_offset, 7) != 0; ++idx);
        }

        std::uint8_t base = itemType.getBaseType();
        bool isArmor = (base & 4) != 0 ? true : false;
        bool isWeapon = (base & 2) != 0 ? true : false;
        bool isTome = (base & 8) != 0 ? true : false;
        bool isStackable = (base & 1) != 0 ? true : false;

        if (isTome)
        {
            // If the item is a tome, it will contain 5 extra bits, we're not
            // interested in these bits, the value is usually 1, but not sure
            // what it is.
            tome_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, 5))
            {
                return false;
            }
        }

        // Realm Data Flag
        realm_bit_offset = current_bit_offset;
        if (readBits(charfile, current_bit_offset, 1) != 0)
        {
            if (!skipBits(charfile, current_bit_offset, REAL_DATA_NUM_BITS))
            {
                return false;
            }
        }

        if (feof(charfile))
        {
            return false;
        }

        if (isArmor || isWeapon)
        {
            if (isArmor)
            {
                // Defense rating
                defense_rating_bit_offset = current_bit_offset;
                if (!skipBits(charfile, current_bit_offset, ((FileVersion >= EnumCharVersion::v110) ? DEFENSE_RATING_NUM_BITS : DEFENSE_RATING_NUM_BITS_108)))
                {
                    return false;
                }
            }

            // Some armor/weapons like phase blades don't have durability
            durability_bit_offset = current_bit_offset;
            if (readBits(charfile, current_bit_offset, DURABILITY_MAX_NUM_BITS) > 0)
            {
                // current durability value (8 bits + unknown single bit)
                if (!skipBits(charfile, current_bit_offset, ((FileVersion >= EnumCharVersion::v110) ? DURABILITY_CURRENT_NUM_BITS : DURABILITY_CURRENT_NUM_BITS_108)))
                {
                    return false;
                }
            }

            if (feof(charfile))
            {
                return false;
            }
        }

        if (isStackable)
        {
            // If the item is a stacked item, e.g. a javelin or something, these 9
            // bits will contain the quantity.
            stackable_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, STACKABLE_NUM_BITS))
            {
                return false;
            }
        }

        socket_count_bit_offset_marker = current_bit_offset;
        if (isSocketed())
        {
            // If the item is socketed, it will contain 4 bits of data which are the
            // number of total sockets the item have, regardless of how many are occupied
            // by an item.
            socket_count_bit_offset = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, SOCKET_COUNT_NUM_BITS))
            {
                return false;
            }
        }

        // If the item is part of a set, these bit will tell us how many lists
        // of magical properties follow the one regular magical property list.
        std::uint8_t setBonusBits = 0;
        if (quality == EnumItemQuality::SET)
        {
            bonus_bits_bit_offset = current_bit_offset;
            setBonusBits = (std::uint8_t)readBits(charfile, current_bit_offset, 5);
            if (feof(charfile))
            {
                return false;
            }
        }

        // magical properties
        magical_props_bit_offset = current_bit_offset;
        if (!parsePropertyList(charfile, current_bit_offset))
        {
            return false;
        }

        if (setBonusBits > 0)
        {
            // Item has more magical property lists due to being a set item
            set_bonus_props_bit_offset = current_bit_offset;
            for (size_t i = 0; i < 5 && setBonusBits > 0; ++i, setBonusBits >>= 1)
            {
                if ((setBonusBits & 0x01) != 0)
                {
                    if (!parsePropertyList(charfile, current_bit_offset))
                    {
                        return false;
                    }
                }
            }
        }

        if (isRuneword())
        {
            // runewords have their own list of magical properties
            runeword_props_bit_offset = current_bit_offset;
            if (!parsePropertyList(charfile, current_bit_offset))
            {
                return false;
            }
        }
    }

    auto numSocketed = socketedItemCount();
    if (numSocketed > 0)
    {
        for (std::uint8_t i = 0; !feof(charfile) && i < numSocketed; ++i)
        {
            SocketedItems.resize(SocketedItems.size() + 1);
            auto& childItem = SocketedItems.back();
            if (!childItem.readItem(FileVersion, charfile))
            {
                return false;
            }

            if (childItem.getLocation() != EnumItemLocation::SOCKET)
            {
                // corrupt file
                return false;
            }

            // resolve magical properties of socketed gem
            childItem.getItemCode(strcode);
            const auto& childItemType = ItemHelpers::getItemTypeHelper(strcode);
            std::vector<MagicalAttribute> attribs;
            childItemType.getSocketedMagicalAttributes(childItem, childItem.socketedMagicalAttributes, getItemType());
        }
    }

    item_end_bit_offset = current_bit_offset;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readItemv100(const Json::Value& itemRoot, bool bSerializedFormat)
{
    if (itemRoot.isNull())
    {
        return false;
    }

    Json::Value node;
    std::uint32_t value = 0;
    size_t current_bit_offset = start_bit_offset;

    location_bit_offset = 0;
    equipped_id_offset = 0;
    alt_position_id_offset = 0;
    durability_bit_offset = 0;
    nr_of_items_in_sockets_offset = 0;
    quality_attrib_bit_offset = 0;
    item_level_bit_offset = 0;

    // 27 bytes total
    data.resize((ITEM_V100_NUM_BITS + 7) / 8, 0);

    extended_data_offset = ITEM_V100_NUM_BITS;
    if (isEar())
    {
        position_offset = ITEM_V100_EAR_COORDINATES_BIT_OFFSET + 2;
        type_code_offset = ITEM_V100_EAR_TYPECODE_BIT_OFFSET;

        size_t bitSize = 10;
        current_bit_offset = type_code_offset;
        if (!setBits(current_bit_offset, bitSize, 0x13B))
        {
            return false;
        }

        Json::Value earRoot = bSerializedFormat ? itemRoot : itemRoot["ear_attributes"];
        if (earRoot.isNull())
        {
            return false;
        }

        EarAttributes earAttrib;
        node = earRoot[bSerializedFormat ? "FileIndex" : "class"];
        if (node.isNull())
        {
            return false;
        }

        if (bSerializedFormat)
        {
            value = std::uint32_t(node.asInt64());
            if (value > std::uint8_t(NUM_OF_CLASSES_NO_EXPANSION))
            {
                return false;
            }
            earAttrib.Class = static_cast<EnumCharClass>(value);
        }
        else
        {
            bool bFound = false;
            std::string className = node.asString();
            for (std::uint8_t idx = 0; idx < std::uint8_t(NUM_OF_CLASSES_NO_EXPANSION); ++idx)
            {
                if (ClassNames[idx].compare(className) == 0)
                {
                    bFound = true;
                    value = idx;
                    break;
                }
            }

            if (!bFound)
            {
                return false;
            }

            earAttrib.Class = static_cast<EnumCharClass>(value);
        }

        node = earRoot[bSerializedFormat ? "EarLevel" : "level"];
        if (node.isNull())
        {
            return false;
        }

        earAttrib.Level = std::uint32_t(node.asInt64());

        node = earRoot[bSerializedFormat ? "PlayerName" : "name"];
        if (node.isNull())
        {
            return false;
        }

        // Check Name
        // Remove any invalid characters from the number
        if (!ItemHelpers::ProcessNameNode(node, earAttrib.Name))
        {
            return false;
        }

        current_bit_offset = type_code_offset;
        bitSize = 3;
        if (!setBits(current_bit_offset, bitSize, std::uint32_t(earAttrib.Class)))
        {
            return false;
        }

        bitSize = 8;
        if (!setBits(current_bit_offset, bitSize, earAttrib.Level))
        {
            return false;
        }

        // up to 15 7 bit characters
        bitSize = 7;
        for (size_t idx = 0; idx < 15; ++idx)
        {
            if (!setBits(current_bit_offset, bitSize, std::uint32_t(earAttrib.Name[idx])))
            {
                return false;
            }

            if (earAttrib.Name[idx] == 0)
            {
                break;
            }
        }

        node = itemRoot[bSerializedFormat ? "Mode" : "location_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());
        if (value > static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::SOCKET))
        {
            return false;
        }

        EnumItemLocation locationId = static_cast<EnumItemLocation>(value);

        node = itemRoot[bSerializedFormat ? "Page" : "alt_position_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());
        if (value > static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::STASH))
        {
            return false;
        }
        EnumAltItemLocation altPositionId = static_cast<EnumAltItemLocation>(value);

        node = itemRoot[bSerializedFormat ? "X" : "position_x"];
        if (node.isNull())
        {
            return false;
        }
        auto positionX = std::uint16_t(node.asInt64());

        node = itemRoot[bSerializedFormat ? "Y" : "position_y"];
        if (node.isNull())
        {
            return false;
        }
        auto positionY = std::uint16_t(node.asInt64());

        switch (locationId)
        {
        case EnumItemLocation::STORED:
            switch (altPositionId)
            {
            case EnumAltItemLocation::INVENTORY:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint8_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x9F);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint8_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x9F) | 0x40);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint8_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x9F) | 0x60);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;
            }
            break;

        case EnumItemLocation::BELT:
        case EnumItemLocation::EQUIPPED:
        case EnumItemLocation::SOCKET:
            return false; // Ears can't be equipped, socketted or put into the belt
        }

        item_end_bit_offset = ITEM_V100_NUM_BITS;
        return true;
    }

    equipped_id_offset = ITEM_V100_BITFIELD3_BIT_OFFSET;
    nr_of_items_in_sockets_offset = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
    nr_of_items_in_sockets_bits = 2;
    type_code_offset = ITEM_V100_TYPECODE_BIT_OFFSET;
    durability_bit_offset = ITEM_V100_DURABILITY_BIT_OFFSET;
    position_offset = ITEM_V100_COORDINATES_BIT_OFFSET + 1;
    quality_bit_offset = start_bit_offset + QUALITY_BIT_OFFSET_100;
    quality_attrib_bit_offset = ITEM_V100_SPECIALITEMCODE_BIT_OFFSET;
    item_level_bit_offset = ITEM_V100_LEVEL_BIT_OFFSET;
    dwa_bit_offset = ITEM_V100_DWA_BIT_OFFSET;
    dwb_bit_offset = ITEM_V100_DWB_BIT_OFFSET;

    node = itemRoot[bSerializedFormat ? "Code" : "type"];
    if (node.isNull())
    {
        return false;
    }

    std::string sValue = node.asString();
    if (sValue.size() < 3)
    {
        return false;
    }

    std::array<std::uint8_t, 4> strcode = { 0x20, 0x20, 0x20, 0x20 };
    std::memcpy(strcode.data(), sValue.c_str(), 3);
    std::uint16_t itemCode = ItemHelpers::getTypeCodev100(strcode);
    if (itemCode >= UINT16_MAX)
    {
        return false;
    }

    size_t bitSize = 10;
    current_bit_offset = type_code_offset;
    if (!setBits(current_bit_offset, bitSize, itemCode))
    {
        return false;
    }

    node = itemRoot[bSerializedFormat ? "Quality" : "quality"];
    if (node.isNull())
    {
        return false;
    }

    value = std::uint32_t(node.asInt64());
    if (value > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
    {
        return false;
    }

    current_bit_offset = quality_bit_offset;
    bitSize = 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        break;

    case EnumItemQuality::SET:
        node = itemRoot[bSerializedFormat ? "FileIndex" : "set_id"];
        if (node.isNull())
        {
            return false;
        }
        value = ItemHelpers::getSetCodev100(std::uint16_t(node.asInt64()));

        current_bit_offset = quality_attrib_bit_offset;
        bitSize = ITEN_V100_UNIQUE_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;

    case EnumItemQuality::UNIQUE:
        node = itemRoot[bSerializedFormat ? "FileIndex" : "unique_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());

        current_bit_offset = quality_attrib_bit_offset;
        bitSize = ITEN_V100_UNIQUE_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;
    }

    value = 0;
    bitSize = DURABILITY_MAX_NUM_BITS;
    node = itemRoot[bSerializedFormat ? "MaxDurability" : "max_durability"];
    if (!node.isNull())
    {
        value = std::uint16_t(node.asInt64());
    }

    current_bit_offset = durability_bit_offset + DURABILITY_CURRENT_NUM_BITS_108;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }

    if (value > 0)
    {
        current_bit_offset = durability_bit_offset;
        bitSize = DURABILITY_CURRENT_NUM_BITS_108;
        value = 0;
        node = itemRoot[bSerializedFormat ? "Durability" : "current_durability"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }

    stackable_bit_offset = 0;
    gld_stackable_bit_offset = 0;
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (itemType.isGoldItem())
    {
        gld_stackable_bit_offset = start_bit_offset + QUANTITY_BIT_OFFSET_100;

        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (node.isNull())
        {
            return false;
        }

        // can hold up to 4095 gold pieces
        value = std::min(std::uint16_t(node.asInt64()), MAX_GLD_QUANTITY);
        bitSize = GLD_STACKABLE_NUM_BITS;
        current_bit_offset = gld_stackable_bit_offset;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }
    else if (itemType.isStackable())
    {
        stackable_bit_offset = start_bit_offset + QUANTITY_BIT_OFFSET_100;

        value = 0;
        bitSize = STACKABLE_NUM_BITS;
        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (!node.isNull())
        {
            value = std::min(std::uint16_t(node.asInt64()), MAX_STACKED_QUANTITY);
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }

    node = itemRoot[bSerializedFormat ? "ItemLevel" : "level"];
    if (node.isNull())
    {
        // generate new one
        value = ItemHelpers::generarateRandomMagicLevel();
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = item_level_bit_offset;
    if (!setBits(current_bit_offset, 8, value))
    {
        return false;
    }

    node = itemRoot[bSerializedFormat ? "Mode" : "location_id"];
    if (node.isNull())
    {
        return false;
    }
    value = std::uint16_t(node.asInt64());
    if (value > static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::SOCKET))
    {
        return false;
    }

    EnumItemLocation locationId = static_cast<EnumItemLocation>(value);

    node = itemRoot[bSerializedFormat ? "Page" : "alt_position_id"];
    if (node.isNull())
    {
        return false;
    }
    value = std::uint16_t(node.asInt64());
    if (value > static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::STASH))
    {
        return false;
    }
    EnumAltItemLocation altPositionId = static_cast<EnumAltItemLocation>(value);

    EnumEquippedId equippedId = EnumEquippedId::NONE;
    node = itemRoot[bSerializedFormat ? "Location" : "equipped_id"];
    if (!node.isNull())
    {
        value = std::uint16_t(node.asInt64());
        if (value < static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_HAND_LEFT))
        {
            equippedId = static_cast<EnumEquippedId>(value);
        }
    }

    node = itemRoot[bSerializedFormat ? "X" : "position_x"];
    if (node.isNull())
    {
        return false;
    }
    auto positionX = std::uint16_t(node.asInt64());

    node = itemRoot[bSerializedFormat ? "Y" : "position_y"];
    if (node.isNull())
    {
        return false;
    }
    auto positionY = std::uint16_t(node.asInt64());

    switch (locationId)
    {
    case EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case EnumAltItemLocation::INVENTORY:
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xF807);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = position_offset;
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = position_offset + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = equipped_id_offset;
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, 0))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumAltItemLocation::STASH:
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x0020);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = position_offset;
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = position_offset + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = equipped_id_offset;
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, 0))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumAltItemLocation::HORADRIC_CUBE:
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x0018);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = position_offset;
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = position_offset + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = equipped_id_offset;
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, 0))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;
        }
        break;

    case EnumItemLocation::BELT:
        current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        positionY = positionX / 4;
        positionX = positionX % 4;
        current_bit_offset = position_offset;
        bitSize = 2;
        if (!setBits(current_bit_offset, bitSize, positionX))
        {
            return false;
        }

        current_bit_offset = position_offset + bitSize;
        bitSize = 2;
        if (!setBits(current_bit_offset, bitSize, positionY))
        {
            return false;
        }

        current_bit_offset = equipped_id_offset;
        bitSize = 4;
        if (!setBits(current_bit_offset, bitSize, 0))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF) | 0x8000);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;

    case EnumItemLocation::EQUIPPED:
        current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = position_offset;
        bitSize = 16;
        value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x01);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = equipped_id_offset;
        bitSize = 4;
        value = static_cast<std::underlying_type_t<EnumEquippedId>>(equippedId);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF) | 0x4000);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;

    case EnumItemLocation::SOCKET:
        current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_COORDINATES_BIT_OFFSET;
        bitSize = 8;
        value = (std::uint8_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xE1);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_BITFIELD3_BIT_OFFSET;
        bitSize = 8;
        value = (std::uint8_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xF0);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x3FFF) | 0x8000);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;
    }

    value = 0;
    node = itemRoot[bSerializedFormat ? "Dwa" : "dwa"];
    if (node.isNull())
    {
        // generate new one
        value = ItemHelpers::generarateRandomDW();
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = dwa_bit_offset;
    if (!setBits(current_bit_offset, 32, value))
    {
        return false;
    }

    value = 0;
    node = itemRoot[bSerializedFormat ? "Dwb" : "dwb"];
    if (node.isNull())
    {
        // generate new one
        value = ItemHelpers::generarateRandomDW();
        if (getQuality() == EnumItemQuality::SET)
        {
            // Find correct DWB value for the SET
            value = ItemHelpers::generateSetDWBCodev100((std::uint16_t)read_uint32_bits(quality_attrib_bit_offset, ITEN_V100_UNIQUE_ID_NUM_BITS), strcode, value, getLevel());
        }
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = dwb_bit_offset;
    if (!setBits(current_bit_offset, 32, value))
    {
        return false;
    }

    item_end_bit_offset = ITEM_V100_NUM_BITS;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readItemv104(const Json::Value& itemRoot, bool bSerializedFormat)
{
    if (itemRoot.isNull())
    {
        return false;
    }

    Json::Value node;
    std::uint32_t value = 0;
    size_t current_bit_offset = start_bit_offset;

    location_bit_offset = 0;
    equipped_id_offset = 0;
    alt_position_id_offset = 0;
    type_code_offset = 0;
    durability_bit_offset = 0;
    nr_of_items_in_sockets_offset = 0;
    quality_attrib_bit_offset = 0;
    item_level_bit_offset = 0;

    if (isEar())
    {
        position_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET + 2;
        type_code_offset = ITEM_V104_EAR_ATTRIBE_BIT_OFFSET;

        // 26 bytes total
        data.resize((ITEM_V104_EAR_NUM_BITS + 7) / 8, 0);
        extended_data_offset = ITEM_V104_EAR_NUM_BITS;

        Json::Value earRoot = bSerializedFormat ? itemRoot : itemRoot["ear_attributes"];
        if (earRoot.isNull())
        {
            return false;
        }

        EarAttributes earAttrib;
        node = earRoot[bSerializedFormat ? "FileIndex" : "class"];
        if (node.isNull())
        {
            return false;
        }

        if (bSerializedFormat)
        {
            value = std::uint32_t(node.asInt64());
            if (value > std::uint8_t(NUM_OF_CLASSES_NO_EXPANSION))
            {
                return false;
            }
            earAttrib.Class = static_cast<EnumCharClass>(value);
        }
        else
        {
            bool bFound = false;
            std::string className = node.asString();
            for (std::uint8_t idx = 0; idx < std::uint8_t(NUM_OF_CLASSES_NO_EXPANSION); ++idx)
            {
                if (ClassNames[idx].compare(className) == 0)
                {
                    bFound = true;
                    value = idx;
                    break;
                }
            }

            if (!bFound)
            {
                return false;
            }

            earAttrib.Class = static_cast<EnumCharClass>(value);
        }

        node = earRoot[bSerializedFormat ? "EarLevel" : "level"];
        if (node.isNull())
        {
            return false;
        }

        earAttrib.Level = std::uint32_t(node.asInt64());

        node = earRoot[bSerializedFormat ? "PlayerName" : "name"];
        if (node.isNull())
        {
            return false;
        }

        // Check Name
        // Remove any invalid characters from the number
        if (!ItemHelpers::ProcessNameNode(node, earAttrib.Name))
        {
            return false;
        }

        current_bit_offset = type_code_offset;
        size_t bitSize = 3;
        if (!setBits(current_bit_offset, bitSize, std::uint32_t(earAttrib.Class)))
        {
            return false;
        }

        bitSize = 8;
        if (!setBits(current_bit_offset, bitSize, earAttrib.Level))
        {
            return false;
        }

        // up to 15 7 bit characters
        bitSize = 7;
        for (size_t idx = 0; idx < 15; ++idx)
        {
            if (!setBits(current_bit_offset, bitSize, std::uint32_t(earAttrib.Name[idx])))
            {
                return false;
            }

            if (earAttrib.Name[idx] == 0)
            {
                break;
            }
        }

        node = itemRoot[bSerializedFormat ? "Mode" : "location_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());
        if (value > static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::SOCKET))
        {
            return false;
        }

        EnumItemLocation locationId = static_cast<EnumItemLocation>(value);

        node = itemRoot[bSerializedFormat ? "Page" : "alt_position_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());
        if (value > static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::STASH))
        {
            return false;
        }
        EnumAltItemLocation altPositionId = static_cast<EnumAltItemLocation>(value);

        node = itemRoot[bSerializedFormat ? "X" : "position_x"];
        if (node.isNull())
        {
            return false;
        }
        auto positionX = std::uint16_t(node.asInt64());

        node = itemRoot[bSerializedFormat ? "Y" : "position_y"];
        if (node.isNull())
        {
            return false;
        }
        auto positionY = std::uint16_t(node.asInt64());

        switch (locationId)
        {
        case EnumItemLocation::STORED:
            switch (altPositionId)
            {
            case EnumAltItemLocation::INVENTORY:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xE3FF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xE3FF) | 0x1000);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xE3FF) | 0x0C00);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;
            }
            break;

        case EnumItemLocation::BELT:
        case EnumItemLocation::EQUIPPED:
        case EnumItemLocation::SOCKET:
            return false; // Ears can't be equipped, socketted or put into the belt
        }

        item_end_bit_offset = ITEM_V104_EAR_NUM_BITS;
        return true;
    }

    if (isSimpleItem())
    {
        position_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET + 2;
        type_code_offset = ITEM_V104_SM_TYPECODE_BIT_OFFSET;

        // 15 bytes total
        data.resize((ITEM_V104_SM_NUM_BITS + 7) / 8, 0);
        extended_data_offset = ITEM_V104_SM_NUM_BITS;

        node = itemRoot[bSerializedFormat ? "Code" : "type"];
        if (node.isNull())
        {
            return false;
        }

        std::string sValue = node.asString();
        if (sValue.size() < 3)
        {
            return false;
        }

        std::array<std::uint8_t, 4> strcode = { 0x20, 0x20, 0x20, 0x20 };
        std::memcpy(strcode.data(), sValue.c_str(), 3);
        std::uint32_t itemCode = *((std::uint32_t*)strcode.data());
        size_t bitSize = 30;
        current_bit_offset = type_code_offset;
        if (!setBits(current_bit_offset, bitSize, itemCode))
        {
            return false;
        }

        node = itemRoot[bSerializedFormat ? "Mode" : "location_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());
        if (value > static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::SOCKET))
        {
            return false;
        }

        EnumItemLocation locationId = static_cast<EnumItemLocation>(value);

        node = itemRoot[bSerializedFormat ? "Page" : "alt_position_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());
        if (value > static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::STASH))
        {
            return false;
        }
        EnumAltItemLocation altPositionId = static_cast<EnumAltItemLocation>(value);

        node = itemRoot[bSerializedFormat ? "X" : "position_x"];
        if (node.isNull())
        {
            return false;
        }
        auto positionX = std::uint16_t(node.asInt64());

        node = itemRoot[bSerializedFormat ? "Y" : "position_y"];
        if (node.isNull())
        {
            return false;
        }
        auto positionY = std::uint16_t(node.asInt64());

        switch (locationId)
        {
        case EnumItemLocation::STORED:
            switch (altPositionId)
            {
            case EnumAltItemLocation::INVENTORY:
                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x03FE);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x03FE) | 0x1000);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x03FE) | 0x0C00);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = position_offset;
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = position_offset + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                break;
            }
            break;

        case EnumItemLocation::BELT:
            current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x03FE) | 0xFC01);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            positionY = positionX / 4;
            positionX = positionX % 4;
            current_bit_offset = position_offset;
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = position_offset + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
            bitSize = 32;
            value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) | 0x00000003);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumItemLocation::EQUIPPED:
            return false; // Simple items can't be equipped

        case EnumItemLocation::SOCKET:
            current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0x03E0) | 0xFC1B);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            if (item_current_socket_idx >= 7)
            {
                return false;
            }

            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xFC7F) | (item_current_socket_idx << 7));
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
            bitSize = 32;
            value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) | 0x00000003);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;
        }

        current_bit_offset = ITEM_V104_SM_NUM_BITS;
        return true;
    }

    type_code_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET;
    equipped_id_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET;
    nr_of_items_in_sockets_offset = ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET;
    nr_of_items_in_sockets_bits = 3;
    durability_bit_offset = ITEM_V104_EX_DURABILITY_BIT_OFFSET;
    position_offset = ITEM_V104_EX_COORDINATES_BIT_OFFSET + 1;
    quality_bit_offset = start_bit_offset + QUALITY_BIT_OFFSET_104;
    quality_attrib_bit_offset = ITEM_V104_EX_UNIQUECODE_BIT_OFFSET;
    item_level_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET;
    dwa_bit_offset = ITEM_V104_EX_DWA_BIT_OFFSET;
    dwb_bit_offset = ITEM_V104_EX_DWB_BIT_OFFSET;

    // 31 bytes total
    data.resize((ITEM_V104_EX_NUM_BITS + 7) / 8, 0);
    extended_data_offset = ITEM_V104_EX_NUM_BITS;

    node = itemRoot[bSerializedFormat ? "Code" : "type"];
    if (node.isNull())
    {
        return false;
    }

    std::string sValue = node.asString();
    if (sValue.size() < 3)
    {
        return false;
    }

    std::array<std::uint8_t, 4> strcode = { 0x20, 0x20, 0x20, 0x20 };
    std::memcpy(strcode.data(), sValue.c_str(), 3);
    std::uint32_t itemCode = *((std::uint32_t*)strcode.data());
    size_t bitSize = 30;
    current_bit_offset = type_code_offset;
    if (!setBits(current_bit_offset, bitSize, itemCode))
    {
        return false;
    }

    node = itemRoot[bSerializedFormat ? "Quality" : "quality"];
    if (node.isNull())
    {
        return false;
    }

    value = std::uint32_t(node.asInt64());
    if (value > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
    {
        return false;
    }

    current_bit_offset = quality_bit_offset;
    bitSize = 4;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        break;

    case EnumItemQuality::SET:
        node = itemRoot[bSerializedFormat ? "FileIndex" : "set_id"];
        if (node.isNull())
        {
            return false;
        }
        value = ItemHelpers::getSetCodev100(std::uint16_t(node.asInt64()));

        current_bit_offset = quality_attrib_bit_offset;
        bitSize = ITEN_V100_UNIQUE_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;

    case EnumItemQuality::UNIQUE:
        node = itemRoot[bSerializedFormat ? "FileIndex" : "unique_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());

        current_bit_offset = quality_attrib_bit_offset;
        bitSize = ITEN_V100_UNIQUE_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;
    }

    value = 0;
    bitSize = DURABILITY_MAX_NUM_BITS;
    node = itemRoot[bSerializedFormat ? "MaxDurability" : "max_durability"];
    if (!node.isNull())
    {
        value = std::uint16_t(node.asInt64());
    }

    current_bit_offset = durability_bit_offset + DURABILITY_CURRENT_NUM_BITS_108;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }

    if (value > 0)
    {
        current_bit_offset = durability_bit_offset;
        bitSize = DURABILITY_CURRENT_NUM_BITS_108;
        value = 0;
        node = itemRoot[bSerializedFormat ? "Durability" : "current_durability"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }

    stackable_bit_offset = 0;
    gld_stackable_bit_offset = 0;
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (itemType.isGoldItem())
    {
        gld_stackable_bit_offset = start_bit_offset + QUANTITY_BIT_OFFSET_104;

        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (node.isNull())
        {
            return false;
        }

        // can hold up to 4095 gold pieces
        value = std::min(std::uint16_t(node.asInt64()), MAX_GLD_QUANTITY);
        bitSize = GLD_STACKABLE_NUM_BITS;
        current_bit_offset = gld_stackable_bit_offset;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }
    else if (itemType.isStackable())
    {
        stackable_bit_offset = start_bit_offset + QUANTITY_BIT_OFFSET_104;

        value = 0;
        bitSize = STACKABLE_NUM_BITS;
        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (!node.isNull())
        {
            value = std::min(std::uint16_t(node.asInt64()), MAX_STACKED_QUANTITY);
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }

    node = itemRoot[bSerializedFormat ? "ItemLevel" : "level"];
    if (node.isNull())
    {
        // generate new one
        value = ItemHelpers::generarateRandomMagicLevel();
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = item_level_bit_offset;
    if (!setBits(current_bit_offset, 8, value))
    {
        return false;
    }

    node = itemRoot[bSerializedFormat ? "Mode" : "location_id"];
    if (node.isNull())
    {
        return false;
    }
    value = std::uint16_t(node.asInt64());
    if (value > static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::SOCKET))
    {
        return false;
    }

    EnumItemLocation locationId = static_cast<EnumItemLocation>(value);

    node = itemRoot[bSerializedFormat ? "Page" : "alt_position_id"];
    if (node.isNull())
    {
        return false;
    }
    value = std::uint16_t(node.asInt64());
    if (value > static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::STASH))
    {
        return false;
    }
    EnumAltItemLocation altPositionId = static_cast<EnumAltItemLocation>(value);

    EnumEquippedId equippedId = EnumEquippedId::NONE;
    node = itemRoot[bSerializedFormat ? "Location" : "equipped_id"];
    if (!node.isNull())
    {
        value = std::uint16_t(node.asInt64());
        if (value < static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_HAND_LEFT))
        {
            equippedId = static_cast<EnumEquippedId>(value);
        }
    }

    node = itemRoot[bSerializedFormat ? "X" : "position_x"];
    if (node.isNull())
    {
        return false;
    }
    auto positionX = std::uint16_t(node.asInt64());

    node = itemRoot[bSerializedFormat ? "Y" : "position_y"];
    if (node.isNull())
    {
        return false;
    }
    auto positionY = std::uint16_t(node.asInt64());

    switch (locationId)
    {
    case EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case EnumAltItemLocation::INVENTORY:
            current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xF807);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = position_offset;
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = position_offset + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
            bitSize = 8;
            value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xC3);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
            bitSize = 32;
            value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xBFFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumAltItemLocation::STASH:
            current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x0020);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = position_offset;
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = position_offset + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
            bitSize = 8;
            value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xC3);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
            bitSize = 32;
            value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xBFFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumAltItemLocation::HORADRIC_CUBE:
            current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x0018);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = position_offset;
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = position_offset + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
            bitSize = 8;
            value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xC3);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
            bitSize = 32;
            value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
            bitSize = 16;
            value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xBFFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;
        }
        break;

    case EnumItemLocation::BELT:
    case EnumItemLocation::SOCKET:
        return false; // Only simple items can be socketted or put into the belt

    case EnumItemLocation::EQUIPPED:
        current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((read_uint32_bits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = position_offset;
        bitSize = 16;
        value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) & 0x01);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
        bitSize = 8;
        value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xC3);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = equipped_id_offset;
        bitSize = 4;
        value = static_cast<std::underlying_type_t<EnumEquippedId>>(equippedId);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
        bitSize = 32;
        value = (std::uint32_t)(read_uint32_bits(current_bit_offset, bitSize) & 0xFFFFFFFC);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
        bitSize = 16;
        value = (std::uint16_t)(read_uint32_bits(current_bit_offset, bitSize) | 0x4000);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;
    }

    value = 0;
    node = itemRoot[bSerializedFormat ? "Dwa" : "dwa"];
    if (node.isNull())
    {
        // generate new one
        value = ItemHelpers::generarateRandomDW();
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = dwa_bit_offset;
    if (!setBits(current_bit_offset, 32, value))
    {
        return false;
    }

    value = 0;
    node = itemRoot[bSerializedFormat ? "Dwb" : "dwb"];
    if (node.isNull())
    {
        // generate new one
        value = ItemHelpers::generarateRandomDW();
        if (getQuality() == EnumItemQuality::SET)
        {
            // Find correct DWB value for the SET
            value = ItemHelpers::generateSetDWBCodev100((std::uint16_t)read_uint32_bits(quality_attrib_bit_offset, ITEN_V100_UNIQUE_ID_NUM_BITS), strcode, value, getLevel());
        }
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = dwb_bit_offset;
    if (!setBits(current_bit_offset, 32, value))
    {
        return false;
    }

    item_end_bit_offset = ITEM_V104_EX_NUM_BITS;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readItem(const Json::Value& itemRoot, bool bSerializedFormat, EnumCharVersion version)
{
    if (itemRoot.isNull())
    {
        return false;
    }

    FileVersion = version;
    data.clear();
    SocketedItems.clear();

    item_current_socket_idx = 0;
    dwa_bit_offset = 0;
    dwb_bit_offset = 0;
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();

    // reserve enough space to reduce chance of reallocation (haven't seen an item size bigger then 80
    data.reserve(80);

    Json::Value node;
    std::uint16_t rawVersion = 0;
    std::uint32_t value = 0;
    std::uint64_t value64 = 0;
    start_bit_offset = 0;
    size_t current_bit_offset = start_bit_offset;
    size_t max_bit_offset = current_bit_offset;

    bool bIsCompact = false;
    node = itemRoot[bSerializedFormat ? "IsCompact" : "simple_item"];
    if (!node.isNull())
    {
        if (node.isBool())
        {
            bIsCompact = node.asBool();
        }
        else
        {
            bIsCompact = (std::uint16_t(node.asInt64()) != 0 ? true : false);
        }
    }

    bool bIsSocketed = false;
    node = itemRoot[bSerializedFormat ? "IsSocketed" : "socketed"];
    if (!node.isNull())
    {
        if (node.isBool())
        {
            bIsSocketed = node.asBool();
        }
        else
        {
            bIsSocketed = (std::uint16_t(node.asInt64()) != 0 ? true : false);
        }
    }

    if (FileVersion < EnumCharVersion::v115)
    {
        value = *((std::uint16_t*)ITEM_MARKER.data());
        setBits(current_bit_offset, ITEM_MARKER.size()*8, value);
        start_bit_offset = current_bit_offset;
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        ItemVersion = EnumItemVersion::v110;
        node = itemRoot[bSerializedFormat ? "Version" : "version"];
        if (node.isNull())
        {
            if (FileVersion < EnumCharVersion::v107) // pre-1.07 character file
            {
                rawVersion = 0;
                if (bIsCompact)
                {
                    ItemVersion = EnumItemVersion::v100; // v1.00 - v1.03
                }
                else
                {
                    ItemVersion = EnumItemVersion::v104;     // v1.04 - v1.06 item
                }
            }
            else if (FileVersion == EnumCharVersion::v107) // pre-1.07 character file
            {
                rawVersion = 0;
                ItemVersion = EnumItemVersion::v107;         // v1.07 item
            }
            else if (FileVersion < EnumCharVersion::v110) // pre-1.10 character file
            {
                rawVersion = 100;
                ItemVersion = EnumItemVersion::v108; // v1.08/1.09 normal or expansion
            }
            else
            {
                rawVersion = 2;
                ItemVersion = EnumItemVersion::v110; // v1.10 normal or expansion
            }
        }
        else
        {
            rawVersion = bSerializedFormat ? std::uint16_t(std::stoul(node.asString(), nullptr, 10)) : std::uint16_t(node.asInt64());
            switch (rawVersion)
            {
            case 0:
                if (FileVersion < EnumCharVersion::v107) // pre-1.07 character file
                {
                    if (bIsCompact)
                    {
                        ItemVersion = EnumItemVersion::v104; // v1.04 - v1.06 item
                    }
                    else
                    {
                        ItemVersion = EnumItemVersion::v100; // v1.00 - v1.03
                    }
                }
                else
                {
                    ItemVersion = EnumItemVersion::v107;     // v1.07 item
                }
                break;

            case 1:
            case 100:
                ItemVersion = EnumItemVersion::v108;         // v1.08/1.09 normal or expansion
                break;

            case 2:
            case 101:
                ItemVersion = EnumItemVersion::v110;         // v1.10 normal or expansion
                break;

            default:
                return false;
            }
        }
    }
    else
    {
        rawVersion = 5;
        node = itemRoot[bSerializedFormat ? "Version" : "version"];
        if (!node.isNull())
        {
            rawVersion = bSerializedFormat ? std::uint16_t(std::stoul(node.asString(), nullptr, 2)) : std::uint16_t(node.asInt64());
        }
        ItemVersion = EnumItemVersion::v115;  // v1.15 Diable II: Resurrected
    }

    // flags
    std::bitset<32> flags = 0;
    node = itemRoot[bSerializedFormat ? "IsIdentified" : "identified"];
    SetFlagBit(node, IS_IDENTIFIED_FLAG_OFFSET, flags);

    node = itemRoot[bSerializedFormat ? "IsSocketed" : "socketed"];
    SetFlagBit(node, IS_SOCKETED_FLAG_OFFSET, flags);

    node = itemRoot[bSerializedFormat ? "IsNew" : "new"];
    SetFlagBit(node, IS_NEW_FLAG_OFFSET, flags);

    node = itemRoot[bSerializedFormat ? "IsEar" : "is_ear"];
    SetFlagBit(node, IS_EAR_FLAG_OFFSET, flags);

    node = itemRoot[bSerializedFormat ? "IsStarterItem" : "starter_item"];
    SetFlagBit(node, IS_STARTER_FLAG_OFFSET, flags);

    node = itemRoot[bSerializedFormat ? "IsCompact" : "simple_item"];
    SetFlagBit(node, IS_SIMPLE_FLAG_OFFSET, flags);

    node = itemRoot[bSerializedFormat ? "IsEthereal" : "ethereal"];
    SetFlagBit(node, IS_ETHEREAL_FLAG_OFFSET, flags);

    if (FileVersion >= EnumCharVersion::v107)
    {
        flags[23] = 1; // unknown but always one as far as I have noticed

        node = itemRoot[bSerializedFormat ? "IsPersonalized" : "personalized"];
        SetFlagBit(node, IS_PERSONALIZED_FLAG_OFFSET, flags);

        node = itemRoot[bSerializedFormat ? "IsRuneword" : "given_runeword"];
        SetFlagBit(node, IS_RUNEWORD_FLAG_OFFSET, flags);
    }
    else if (ItemVersion == EnumItemVersion::v104)
    {
        // unknown but both 1 for v1.04 to v1.06 items as far as I have noticed
        flags[19] = 1;
        flags[20] = 1;
    }

    if (!bSerializedFormat)
    {
        Json::Value unknowns = itemRoot["_unknown_data"];
        if (!unknowns.isNull())
        {
            struct byteRange
            {
                size_t startIdx = 0;
                size_t endIdx = 0;
            };
            static std::vector<byteRange> unknowns_range = { {0,3},{5,10},{12,12},{14,15},{18,20},{23,23},{25,25},{27,31} };

            size_t bitNum = 0;
            Json::Value unknownData;
            for (const auto& byteInfo : unknowns_range)
            {
                std::stringstream ss;
                ss << "b" << std::dec << byteInfo.startIdx;
                if (byteInfo.endIdx > byteInfo.startIdx)
                {
                    ss << "_" << std::dec << byteInfo.endIdx;
                }

                node = unknowns[ss.str()];
                if (node.isNull())
                {
                    continue;
                }

                auto iter_end = node.end();
                for (auto iter = node.begin(); iter != iter_end; ++iter)
                {
                    if (iter->isNull())
                    {
                        continue;
                    }

                    bitNum = byteInfo.startIdx + size_t(std::stoi(iter.name()));
                    SetFlagBit(*iter, bitNum, flags);
                }
            }
        }
    }

    value = flags.to_ulong();
    if (!setBits(current_bit_offset, flags.size(), value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    if (ItemVersion < EnumItemVersion::v107)
    {
        if (ItemVersion == EnumItemVersion::v100)
        {
            // Check unknown bits that indicate it's an 1.04 - 1.06 not 1.03 or older
            if (read_uint32_bits(start_bit_offset + ITEM_TYPE_BIT_OFFSET, 2) != 0)
            {
                ItemVersion = EnumItemVersion::v104; // v1.04 - v1.06 item
            }
            else if(bSerializedFormat)
            {
                value = 0;
                node = itemRoot["IsV104"];
                if (!node.isNull() && (node.asInt64() != 0))
                {
                    ItemVersion = EnumItemVersion::v104; // v1.04 - v1.06 item
                    auto tempOffset = start_bit_offset + ITEM_TYPE_BIT_OFFSET;
                    if (!setBits(tempOffset, 2, 0x3))
                    {
                        return false;
                    }
                }
            }
        }

        value = 0;
        node = itemRoot[bSerializedFormat ? "NumberOfSocketedItems" : "nr_of_items_in_sockets"];
        if (!node.isNull())
        {
            value = std::uint32_t(node.asInt64());
        }
        std::uint8_t numSocketed = std::uint8_t(value);

        switch (ItemVersion)
        {
        case EnumItemVersion::v100:
            if (!readItemv100(itemRoot, bSerializedFormat))
            {
                return 0;
            }

            if (isEar())
            {
                numSocketed = 0;
            }
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06
            if (!readItemv104(itemRoot, bSerializedFormat))
            {
                return 0;
            }

            if (isEar() || isSimpleItem())
            {
                numSocketed = 0;
            }
            break;
        }

        if (numSocketed > 0)
        {
            std::array<std::uint8_t, 4> strcode = { 0x20, 0x20, 0x20, 0x20 };
            node = itemRoot[bSerializedFormat ? "SocketedItems" : "socketed_items"];
            if (node.isNull() || !node.isArray() || node.size() != numSocketed)
            {
                return false;
            }

            item_current_socket_idx = 0;
            auto iter_end = node.end();
            for (auto iter = node.begin(); iter != iter_end; ++iter, ++item_current_socket_idx)
            {
                SocketedItems.resize(SocketedItems.size() + 1);
                auto& childItem = SocketedItems.back();
                if (!childItem.readItem(*iter, bSerializedFormat, FileVersion))
                {
                    return false;
                }

                if (childItem.getLocation() != EnumItemLocation::SOCKET)
                {
                    // corrupt file
                    return false;
                }

                // resolve magical properties of socketed gem
                childItem.getItemCode(strcode);
                const auto& childItemType = ItemHelpers::getItemTypeHelper(strcode);
                std::vector<MagicalAttribute> attribs;
                childItemType.getSocketedMagicalAttributes(childItem, childItem.socketedMagicalAttributes, getItemType());
            }
        }

        return true;
    }

    value = rawVersion;
    size_t bitSize = (FileVersion < EnumCharVersion::v115 ? 10 : 3);
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    location_bit_offset = current_bit_offset;
    node = itemRoot[bSerializedFormat ? "Mode" : "location_id"];
    if (node.isNull())
    {
        return false;
    }

    current_bit_offset = location_bit_offset;
    value = std::uint16_t(node.asInt64());
    bitSize = 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // if not equipped, equipped_id could be missing
    bitSize = 4; 
    equipped_id_offset = current_bit_offset;
    value = 0;
    node = itemRoot[bSerializedFormat ? "Location" : "equipped_id"];
    if (!node.isNull())
    {
        value = std::uint16_t(node.asInt64());
    }

    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // position x/y
    position_offset = current_bit_offset;
    bitSize = 4;

    node = itemRoot[bSerializedFormat ? "X" : "position_x"];
    if (node.isNull())
    {
        return false;
    }

    current_bit_offset = position_offset;
    value = std::uint16_t(node.asInt64());
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    bitSize = 4;
    node = itemRoot[bSerializedFormat ? "Y" : "position_y"];
    if (node.isNull())
    {
        return false;
    }

    value = std::uint16_t(node.asInt64());
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // alt position x/y
    alt_position_id_offset = current_bit_offset;
    bitSize = 3;

    node = itemRoot[bSerializedFormat ? "Page" : "alt_position_id"];
    if (node.isNull())
    {
        return false;
    }

    current_bit_offset = alt_position_id_offset;
    value = std::uint16_t(node.asInt64());
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // type code
    type_code_offset = current_bit_offset;
    bitSize = 32;

    if (isEar())
    {
        Json::Value earRoot = bSerializedFormat ? itemRoot : itemRoot["ear_attributes"];
        if (earRoot.isNull())
        {
            return false;
        }

        EarAttributes earAttrib;
        node = earRoot[bSerializedFormat ? "FileIndex" : "class"];
        if (node.isNull())
        {
            return false;
        }

        if (bSerializedFormat)
        {
            value = std::uint32_t(node.asInt64());
            if (value > std::uint8_t(NUM_OF_CLASSES))
            {
                return false;
            }
            earAttrib.Class = static_cast<EnumCharClass>(value);
        }
        else
        {
            bool bFound = false;
            std::string className = node.asString();
            for (std::uint8_t idx = 0; idx < std::uint8_t(NUM_OF_CLASSES); ++idx)
            {
                if (ClassNames[idx].compare(className) == 0)
                {
                    bFound = true;
                    value = idx;
                    break;
                }
            }

            if (!bFound)
            {
                return false;
            }

            earAttrib.Class = static_cast<EnumCharClass>(value);
        }

        node = earRoot[bSerializedFormat ? "EarLevel" : "level"];
        if (node.isNull())
        {
            return false;
        }

        earAttrib.Level = std::uint32_t(node.asInt64());

        node = earRoot[bSerializedFormat ? "PlayerName" : "name"];
        if (node.isNull())
        {
            return false;
        }

        // Check Name
        // Remove any invalid characters from the number
        if (!ItemHelpers::ProcessNameNode(node, earAttrib.Name))
        {
            return false;
        }

        current_bit_offset = type_code_offset;
        if (!setBits(current_bit_offset, 3, std::uint32_t(earAttrib.Class)))
        {
            return false;
        }

        if (!setBits(current_bit_offset, 7, earAttrib.Level))
        {
            return false;
        }

        // up to 15 7 bit characters
        for (size_t idx = 0; idx < 15; ++idx)
        {
            if (!setBits(current_bit_offset, 7, std::uint32_t(earAttrib.Name[idx])))
            {
                return false;
            }

            if (earAttrib.Name[idx] == 0)
            {
                break;
            }
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        item_end_bit_offset = max_bit_offset;
        return true;
    }

    node = itemRoot[bSerializedFormat ? "Code" : "type"];
    if (node.isNull())
    {
        return false;
    }

    std::string sValue = node.asString();
    if (sValue.size() < 3)
    {
        return false;
    }

    std::array<std::uint8_t, 4> strcode = { 0x20, 0x20, 0x20, 0x20 };
    std::memcpy(strcode.data(), sValue.c_str(), 3);
    std::uint8_t numBitsSet = 0;

    current_bit_offset = type_code_offset;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.0.6
        return false; // should not get here

    case EnumItemVersion::v107: // v1.07
    case EnumItemVersion::v108: // v1.08/1.09 normal or expansion
    case EnumItemVersion::v110: // v1.10
        value = *((std::uint32_t*)strcode.data());
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;

    default: // v1.15+
        ItemHelpers::encodeItemCodev115(strcode, value64, numBitsSet);
        bitSize = numBitsSet;
        if (!setBits64(current_bit_offset, bitSize, value64))
        {
            return false;
        }
        break;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    extended_data_offset = max_bit_offset;

    current_bit_offset = extended_data_offset;
    value = 0;
    bitSize = bIsCompact ? 1 : 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    quest_difficulty_offset = 0;
    gld_stackable_bit_offset = 0;
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (bIsCompact)
    {
        if (itemType.isGoldItem())
        {
            // Is this correct for gld items? It's not currently used, so is it even needed?
            gld_stackable_bit_offset = extended_data_offset + 1;

            node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
            if (node.isNull())
            {
                return false;
            }
            // can hold up to 4095 gold pieces
            value = std::min(std::uint16_t(node.asInt64()), MAX_GLD_QUANTITY);
            bitSize = GLD_STACKABLE_NUM_BITS;
            current_bit_offset = gld_stackable_bit_offset;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            nr_of_items_in_sockets_offset = current_bit_offset;
            nr_of_items_in_sockets_bits = 1;
        }
        else
        {
            nr_of_items_in_sockets_offset = extended_data_offset;
            nr_of_items_in_sockets_bits = 1;
            if (itemType.isQuestItem())
            {
                quest_difficulty_offset = extended_data_offset;
                nr_of_items_in_sockets_offset = quest_difficulty_offset + 2;
                nr_of_items_in_sockets_bits = 1;

                value = 0;
                bitSize = 2;
                node = itemRoot[bSerializedFormat ? "QuestDifficulty" : "quest_difficulty"];
                if (!node.isNull())
                {
                    // can hold up to 4095 gold pieces
                    value = std::uint32_t(node.asInt64());
                }

                current_bit_offset = quest_difficulty_offset;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            }
        }

        item_end_bit_offset = current_bit_offset;
        return true;
    }
    else if (itemType.isQuestItem())
    {
        quest_difficulty_offset = extended_data_offset;
        nr_of_items_in_sockets_offset = quest_difficulty_offset + 2;
        nr_of_items_in_sockets_bits = 1;

        value = 0;
        bitSize = 2;
        node = itemRoot[bSerializedFormat ? "QuestDifficulty" : "quest_difficulty"];
        if (!node.isNull())
        {
            value = std::uint32_t(node.asInt64());
        }

        current_bit_offset = quest_difficulty_offset;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }
    else
    {
        nr_of_items_in_sockets_offset = extended_data_offset;
        nr_of_items_in_sockets_bits = 3;
    }

    value = 0;
    bitSize = nr_of_items_in_sockets_bits;
    node = itemRoot[bSerializedFormat ? "NumberOfSocketedItems" : "nr_of_items_in_sockets"];
    if (!node.isNull())
    {
        value = std::uint32_t(node.asInt64());
    }
    std::uint8_t numSocketed = std::uint8_t(value);

    current_bit_offset = nr_of_items_in_sockets_offset;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    item_id_bit_offset = current_bit_offset;
    node = itemRoot[bSerializedFormat ? "Id" : "id"];
    if (node.isNull())
    {
        return false;
    }

    value = std::uint32_t(node.asInt64());
    bitSize = 32;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    item_level_bit_offset = current_bit_offset;
    node = itemRoot[bSerializedFormat ? "ItemLevel" : "level"];
    if (node.isNull())
    {
        return false;
    }

    value = std::uint32_t(node.asInt64());
    bitSize = 7;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    quality_bit_offset = current_bit_offset;
    node = itemRoot[bSerializedFormat ? "Quality" : "quality"];
    if (node.isNull())
    {
        return false;
    }

    value = std::uint32_t(node.asInt64());
    if (value > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
    {
        return false;
    }

    auto quality = static_cast<EnumItemQuality>(value);
    bitSize = 4;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // If this is TRUE, it means the item has more than one picture associated with it.
    multi_graphic_bit_offset = current_bit_offset;
    value = 0;
    bitSize = 1;
    node = itemRoot[bSerializedFormat ? "HasMultipleGraphics" : "multiple_pictures"];
    if (!node.isNull())
    {
        if (node.isBool())
        {
            value = (node.asBool() ? 1 : 0);
        }
        else
        {
            value = (std::uint16_t(node.asInt64()) != 0 ? 1 : 0);
        }
    }
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    if (value != 0)
    {
        value = 0;
        node = itemRoot[bSerializedFormat ? "GraphicId" : "picture_id"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }

        bitSize = 3;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // If this is TRUE, it means the item is class specific.
    autoAffix_bit_offset = current_bit_offset;
    value = 0;
    bitSize = 1;
    node = itemRoot[bSerializedFormat ? "IsAutoAffix" : "class_specific"];
    if (!node.isNull())
    {
        if (node.isBool())
        {
            value = (node.asBool() ? 1 : 0);
        }
        else
        {
            value = (std::uint16_t(node.asInt64()) != 0 ? 1 : 0);
        }
    }
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    if (value != 0)
    {
        value = 0;
        node = itemRoot[bSerializedFormat ? "AutoAffixId" : "auto_affix_id"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }

        bitSize = 11;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    quality_attrib_bit_offset = 0;
    switch (quality)
    {
    case EnumItemQuality::INFERIOR:
        quality_attrib_bit_offset = current_bit_offset;
        value = 0;
        node = itemRoot[bSerializedFormat ? "FileIndex" : "low_quality_id"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }

        bitSize = INFERIOR_SUPERIOR_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        break;

    case EnumItemQuality::SUPERIOR:
        quality_attrib_bit_offset = current_bit_offset;
        value = 0;
        node = itemRoot[bSerializedFormat ? "FileIndex" : "file_index"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }

        bitSize = INFERIOR_SUPERIOR_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        break;

    case EnumItemQuality::MAGIC:
        quality_attrib_bit_offset = current_bit_offset;
        if (bSerializedFormat)
        {
            value = 0;
            node = itemRoot["MagicPrefixIds"];
            if (node.isArray() && !node.empty())
            {
                value = std::uint16_t(node[0].asInt64());
            }

            bitSize = MAGICAL_AFFIX_NUM_BITS;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            
            value = 0;
            node = itemRoot["MagicSuffixIds"];
            if (node.isArray() && !node.empty())
            {
                value = std::uint16_t(node[0].asInt64());
            }

            bitSize = MAGICAL_AFFIX_NUM_BITS;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        }
        else
        {
            value = 0;
            node = itemRoot["magic_prefix"];
            if (!node.isNull())
            {
                value = std::uint16_t(node.asInt64());
            }

            bitSize = MAGICAL_AFFIX_NUM_BITS;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);

            value = 0;
            node = itemRoot["magic_suffix"];
            if (!node.isNull())
            {
                value = std::uint16_t(node.asInt64());
            }

            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        quality_attrib_bit_offset = current_bit_offset;
        node = itemRoot[bSerializedFormat ? "RarePrefixId" : "rare_name_id"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }
        else
        {
            if (bSerializedFormat)
            {
                return false;
            }
            else
            {
                node = itemRoot["rare_name"];
                if (node.isNull())
                {
                    return false;
                }

                value = ItemHelpers::getIdFromRareName(node.asString());
                if (value == 0)
                {
                    return false;
                }
            }
        }

        bitSize = RARE_CRAFTED_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        node = itemRoot[bSerializedFormat ? "RareSuffixId" : "rare_name_id2"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }
        else
        {
            if (bSerializedFormat)
            {
                return false;
            }
            else
            {
                node = itemRoot["rare_name2"];
                if (node.isNull())
                {
                    return false;
                }

                value = ItemHelpers::getIdFromRareName(node.asString());
                if (value == 0)
                {
                    return false;
                }
            }
        }

        bitSize = RARE_CRAFTED_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        // Following the name IDs, we got 6 possible magical affixes, the pattern
        // is 1 bit id, 11 bit value... But the value will only exist if the prefix
        // is 1. 
        if (bSerializedFormat)
        {
            Json::Value prefixIdsNode = itemRoot["MagicPrefixIds"];
            if (prefixIdsNode.isNull() || !prefixIdsNode.isArray())
            {
                return false;
            }

            Json::Value suffixIdsNode = itemRoot["MagicSuffixIds"];
            if (suffixIdsNode.isNull() || !suffixIdsNode.isArray())
            {
                return false;
            }

            for (unsigned int i = 0; i < 3; ++i)
            {
                value = 0;
                if (prefixIdsNode.size() > i)
                {
                    value = std::uint16_t(prefixIdsNode[i].asInt64());
                }

                if (value > 0)
                {
                    if (!setBits(current_bit_offset, 1, 1))
                    {
                        return false;
                    }

                    if (!setBits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
                    {
                        return false;
                    }
                }
                else if (!setBits(current_bit_offset, 1, 0))
                {
                    return false;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);

                value = 0;
                if (suffixIdsNode.size() > i)
                {
                    value = std::uint16_t(suffixIdsNode[i].asInt64());
                }

                if (value > 0)
                {
                    if (!setBits(current_bit_offset, 1, 1))
                    {
                        return false;
                    }

                    if (!setBits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
                    {
                        return false;
                    }
                }
                else if (!setBits(current_bit_offset, 1, 0))
                {
                    return false;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            }
        }
        else
        {
            value = 0;
            node = itemRoot["magical_name_ids"];
            if (node.isNull() || !node.isArray())
            {
                return false;
            }

            for (unsigned int i = 0; i < 3; ++i)
            {
                value = 0;
                if (node.size() > (i * 2))
                {
                    value = std::uint16_t(node[i * 2].asInt64());
                }

                if (value > 0)
                {
                    if (!setBits(current_bit_offset, 1, 1))
                    {
                        return false;
                    }

                    if (!setBits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
                    {
                        return false;
                    }
                }
                else if (!setBits(current_bit_offset, 1, 0))
                {
                    return false;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);

                value = 0;
                if (node.size() > (i * 2 + 1))
                {
                    value = std::uint16_t(node[i * 2 + 1].asInt64());
                }

                if (value > 0)
                {
                    if (!setBits(current_bit_offset, 1, 1))
                    {
                        return false;
                    }

                    if (!setBits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
                    {
                        return false;
                    }
                }
                else if (!setBits(current_bit_offset, 1, 0))
                {
                    return false;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            }
        }
        break;

    case EnumItemQuality::SET:
        quality_attrib_bit_offset = current_bit_offset;
        node = itemRoot[bSerializedFormat ? "FileIndex" : "set_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());

        bitSize = SET_UNIQUE_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        break;

    case EnumItemQuality::UNIQUE:
        quality_attrib_bit_offset = current_bit_offset;
        node = itemRoot[bSerializedFormat ? "FileIndex" : "unique_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());

        bitSize = SET_UNIQUE_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        break;
    }

    if (isRuneword())
    {
        runeword_id_bit_offset = current_bit_offset;
        node = itemRoot[bSerializedFormat ? "RunewordId" : "runeword_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());

        bitSize = RUNEWORD_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        value = 5;
        bitSize = RUNEWORD_PADDING_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    personalized_bit_offset_marker = current_bit_offset;
    if (isPersonalized())
    {
        personalized_bit_offset = current_bit_offset;
        node = itemRoot[bSerializedFormat ? "PlayerName" : "personalized_name"];
        if (node.isNull())
        {
            return false;
        }

        // Check Name
        // Remove any invalid characters from the number
        std::array<char, NAME_LENGTH> playerName;
        if (!ItemHelpers::ProcessNameNode(node, playerName))
        {
            return false;
        }

        // up to 15 7 bit characters
        for (size_t idx = 0; idx < 15; ++idx)
        {
            if (!setBits(current_bit_offset, 7, std::uint32_t(playerName[idx])))
            {
                return false;
            }

            if (playerName[idx] == 0)
            {
                break;
            }
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    std::uint8_t base = itemType.getBaseType();
    bool isArmor = (base & 4) != 0 ? true : false;
    bool isWeapon = (base & 2) != 0 ? true : false;
    bool isTome = (base & 8) != 0 ? true : false;
    bool isStackable = (base & 1) != 0 ? true : false;

    if (isTome)
    {
        // If the item is a tome, it will contain 5 extra bits, we're not
        // interested in these bits, the value is usually 1, but not sure
        // what it is.
        tome_bit_offset = current_bit_offset;
        value = strcode[0] == 'i' ? 1 : 0;
        if (bSerializedFormat)
        {
            node = itemRoot["MagicSuffixIds"];
            if (node.isArray() && !node.empty())
            {
                value = std::uint16_t(node[0].asInt64());
            }
        }
        else
        {
            node = itemRoot["magic_suffix"];
            if (!node.isNull())
            {
                value = std::uint16_t(node.asInt64());
            }
        }

        bitSize = 5;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // Realm Data Flag
    // TODO: should we add information to the json export (always 0 it seems)
    realm_bit_offset = current_bit_offset;
    value = 0;
    bitSize = 1;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    if (isArmor || isWeapon)
    {
        if (isArmor)
        {
            // Defense rating
            static const auto& stat = ItemHelpers::getItemStat(31);
            defense_rating_bit_offset = current_bit_offset;
            value = stat.saveAdd;
            bitSize = (FileVersion >= EnumCharVersion::v110) ? DEFENSE_RATING_NUM_BITS : DEFENSE_RATING_NUM_BITS_108;
            node = itemRoot[bSerializedFormat ? "Armor" : "defense_rating"];
            if (!node.isNull())
            {
                value += std::uint16_t(node.asInt64());
            }

            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        }

        // Some armor/weapons like phase blades don't have durability
        durability_bit_offset = current_bit_offset;
        value = 0;
        bitSize = DURABILITY_MAX_NUM_BITS;
        node = itemRoot[bSerializedFormat ? "MaxDurability" : "max_durability"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        if(value > 0)
        {
            // current durability value (8 bits + unknown single bit)
            bitSize = (FileVersion >= EnumCharVersion::v110) ? DURABILITY_CURRENT_NUM_BITS : DURABILITY_CURRENT_NUM_BITS_108;
            value = 0;
            node = itemRoot[bSerializedFormat ? "Durability" : "current_durability"];
            if (!node.isNull())
            {
                value = std::uint16_t(node.asInt64());
            }

            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        }
    }

    if (isStackable)
    {
        // If the item is a stacked item, e.g. a javelin or something, these 9
        // bits will contain the quantity.
        stackable_bit_offset = current_bit_offset;
        value = 0;
        bitSize = STACKABLE_NUM_BITS;
        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (!node.isNull())
        {
            value = std::min(std::uint16_t(node.asInt64()), MAX_STACKED_QUANTITY);
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    socket_count_bit_offset_marker = current_bit_offset;
    if (bIsSocketed)
    {
        // If the item is socketed, it will contain 4 bits of data which are the
        // number of total sockets the item have, regardless of how many are occupied
        // by an item.
        socket_count_bit_offset = current_bit_offset;
        value = 0;
        bitSize = SOCKET_COUNT_NUM_BITS;
        node = itemRoot[bSerializedFormat ? "TotalNumberOfSockets" : "total_nr_of_sockets"];
        if (node.isNull())
        {
            return false;
        }

        value = std::uint16_t(node.asInt64());
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // If the item is part of a set, these bit will tell us how many lists
    // of magical properties follow the one regular magical property list.
    std::uint8_t setBonusBits = 0;
    if (quality == EnumItemQuality::SET)
    {
        bonus_bits_bit_offset = current_bit_offset;
        if (bSerializedFormat)
        {
            node = itemRoot["StatLists"];
            if (node.isNull() || !node.isArray())
            {
                return false;
            }

            size_t numStats = node.size();
            if (numStats == 0) // should not happen
            {
                return false;
            }

            --numStats; // normal magical properties
            if (isRuneword())
            {
                if (numStats == 0) // should not happen
                {
                    return false;
                }

                --numStats; // rune word magical properties
            }

            if (numStats > 0)
            {
                std::bitset<5> bonusBits;
                numStats = std::min(numStats, bonusBits.size());
                for (size_t idx = 0; idx < numStats; ++idx)
                {
                    bonusBits[idx] = 1;
                }

                setBonusBits = std::uint8_t(bonusBits.to_ulong());
            }
        }
        else
        {
            node = itemRoot["set_attributes"];
            if (node.isNull() || !node.isArray())
            {
                return false;
            }

            if (!node.empty())
            {
                std::bitset<5> bonusBits;
                size_t numStats = std::min(size_t(node.size()), bonusBits.size());
                for (size_t idx = 0; idx < numStats; ++idx)
                {
                    bonusBits[idx] = 1;
                }

                setBonusBits = std::uint8_t(bonusBits.to_ulong());
            }
        }

        bitSize = 5;
        value = setBonusBits;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // magical properties
    magical_props_bit_offset = current_bit_offset;
    if (bSerializedFormat)
    {
        node = itemRoot["StatLists"];
        if (node.isNull() || !node.isArray())
        {
            return false;
        }

        if (node.empty())
        {
            node.clear();
        }
        else
        {
            node = node[0];
            if (!node.isNull())
            {
                node = node["Stats"];
            }
        }
    }
    else
    {
        node = itemRoot["magic_attributes"];
    }

    if (!parsePropertyList(node, bSerializedFormat, current_bit_offset))
    {
        return false;
    }

    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        set_bonus_props_bit_offset = current_bit_offset;
        Json::Value setAttribs = itemRoot[bSerializedFormat ? "StatLists" : "set_attributes"];
        if (setAttribs.isNull() || !setAttribs.isArray())
        {
            return false;
        }

        auto iter_end = setAttribs.end();
        auto iter = setAttribs.begin();
        if (bSerializedFormat)
        {
            ++iter;
            if(iter == iter_end)
            {
                return false;
            }
            
            if (isRuneword())
            {
                ++iter;
                if (iter == iter_end)
                {
                    return false;
                }
            }
        }

        size_t i = 0;
        for (; i < 5 && iter != iter_end; ++iter, ++i)
        {
            node.clear();
            if (!iter->isNull())
            {
                node = bSerializedFormat ? iter->operator[]("Stats") : *iter;
            }

            if (!parsePropertyList(node, bSerializedFormat, current_bit_offset))
            {
                return false;
            }
        }
    }

    if (isRuneword())
    {
        // runewords have their own list of magical properties
        runeword_props_bit_offset = current_bit_offset;
        if (bSerializedFormat)
        {
            node = itemRoot["StatLists"];
            if (node.isNull() || !node.isArray())
            {
                return false;
            }

            if (node.size() < 2)
            {
                node.clear();
            }
            else
            {
                node = node[1];
                if (!node.isNull())
                {
                    node = node["Stats"];
                }
            }
        }
        else
        {
            node = itemRoot["runeword_attributes"];
        }

        if (node.isNull() || !node.isArray())
        {
            return false;
        }

        if (!parsePropertyList(node, bSerializedFormat, current_bit_offset))
        {
            return false;
        }
    }

    if (numSocketed > 0)
    {
        node = itemRoot[bSerializedFormat ? "SocketedItems" : "socketed_items"];
        if (node.isNull() || !node.isArray() || node.size() != numSocketed)
        {
            return false;
        }

        auto iter_end = node.end();
        for (auto iter = node.begin(); iter != iter_end; ++iter)
        {
            SocketedItems.resize(SocketedItems.size() + 1);
            auto& childItem = SocketedItems.back();
            if (!childItem.readItem(*iter, bSerializedFormat, FileVersion))
            {
                return false;
            }

            if (childItem.getLocation() != EnumItemLocation::SOCKET)
            {
                // corrupt file
                return false;
            }

            // resolve magical properties of socketed gem
            childItem.getItemCode(strcode);
            const auto& childItemType = ItemHelpers::getItemTypeHelper(strcode);
            std::vector<MagicalAttribute> attribs;
            childItemType.getSocketedMagicalAttributes(childItem, childItem.socketedMagicalAttributes, getItemType());
        }
    }

    item_end_bit_offset = max_bit_offset;
    return true;
}
//---------------------------------------------------------------------------
void d2ce::Item::asJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat) const
{
    std::vector<MagicalAttribute> magicalAttributes;
    d2ce::RunewordAttributes runewordAttrib;
    Json::Value item;
    if (bSerializedFormat)
    {
        if (FileVersion < EnumCharVersion::v115)
        {
            item["Header"] = *((std::uint16_t*)ITEM_MARKER.data());

            std::stringstream ss;
            ss << std::dec << getRawVersion();
            item["Version"] = ss.str();
            if (ItemVersion == EnumItemVersion::v104)
            {
                item["IsV104"] = true;
            }
        }
        else
        {
            switch (getRawVersion())
            {
            case 0:
                item["Version"] = "0";
                break;

            case 1:
                item["Version"] = "1";
                break;

            case 2:
                item["Version"] = "10";
                break;

            case 3:
                item["Version"] = "11";
                break;

            case 4:
                item["Version"] = "100";
                break;

            case 5:
                item["Version"] = "101";
                break;

            case 6:
                item["Version"] = "110";
                break;

            case 7:
                item["Version"] = "111";
                break;

            default:
                item["Version"] = "101";
                break;

            }
        }

        item["Mode"] = std::uint16_t(getLocation());
        item["Location"] = std::uint16_t(getEquippedId());
        item["X"] = std::uint16_t(getPositionX());
        item["Y"] = std::uint16_t(getPositionY());
        item["Page"] = std::uint16_t(getAltPositionId());

        std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
        getItemCode(strcode);

        bool bIsEar = isEar();
        EarAttributes earAttrib;
        getEarAttributes(earAttrib);
        if (bIsEar)
        {
            item["EarLevel"] = earAttrib.getLevel();
            item["PlayerName"] = earAttrib.getName().data();
            item["Code"] = "";
        }
        else
        {
            item["EarLevel"] = 0;
            if (isPersonalized())
            {
                item["PlayerName"] = getPersonalizedName();
            }

            std::string sCode((char*)strcode.data(), 4);
            item["Code"] = sCode;
        }

        const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
        if (itemType.isQuestItem())
        {
            item["QuestDifficulty"] = std::uint16_t(getQuestDifficulty());
        }

        item["NumberOfSocketedItems"] = std::uint16_t(socketedItemCount());
        item["TotalNumberOfSockets"] = std::uint16_t(totalNumberOfSockets());

        // Socketed items
        Json::Value socketedItems(Json::arrayValue);
        for (auto& socketedItem : SocketedItems)
        {
            socketedItem.asJson(socketedItems, charLevel, bSerializedFormat);
        }
        item["SocketedItems"] = socketedItems;

        item["Id"] = getId();
        item["ItemLevel"] = std::uint16_t(getLevel());

        if (dwa_bit_offset != 0)
        {
            if (dwb_bit_offset == 0)
            {
                dwb_bit_offset = dwa_bit_offset + 32;
            }

            item["Dwa"] = (std::uint32_t)read_uint64_bits(dwa_bit_offset, 32);
            item["Dwb"] = (std::uint32_t)read_uint64_bits(dwb_bit_offset, 32);
        }
        
        auto quality = getQuality();
        item["Quality"] = std::uint16_t(quality);
        item["HasMultipleGraphics"] = hasMultipleGraphics();
        item["GraphicId"] = std::uint16_t(getPictureId());
        item["IsAutoAffix"] = isAutoAffix();
        item["AutoAffixId"] = getAutoAffixId();
        item["FileIndex"] = getFileIndex();

        d2ce::RareAttributes rareAttrib;
        std::vector<MagicalAffixes> affixes;
        affixes.resize(3);
        switch (quality)
        {
        case EnumItemQuality::MAGIC:
            getMagicalAffixes(affixes[0]);
            break;

        case EnumItemQuality::RARE:
        case EnumItemQuality::CRAFT:
        case EnumItemQuality::TEMPERED:
            if (getRareOrCraftedAttributes(rareAttrib))
            {
                affixes = rareAttrib.Affixes;
            }
            break;
        }

        if (isTome())
        {
            affixes[0].SuffixId = getTomeValue();
        }

        Json::Value magicPrefixIds(Json::arrayValue);
        for (auto& affix : affixes)
        {
            magicPrefixIds.append(affix.PrefixId);
        }
        item["MagicPrefixIds"] = magicPrefixIds;

        Json::Value magicSuffixIds(Json::arrayValue);
        for (auto& affix : affixes)
        {
            magicSuffixIds.append(affix.SuffixId);
        }
        item["MagicSuffixIds"] = magicSuffixIds;

        item["RarePrefixId"] = rareAttrib.Id;
        item["RareSuffixId"] = rareAttrib.Id2;

        getRunewordAttributes(runewordAttrib);
        item["RunewordId"] = runewordAttrib.Id;
        item["Armor"] = getDefenseRating();

        ItemDurability durability;
        getDurability(durability);
        item["MaxDurability"] = durability.Max;
        if (durability.Max > 0)
        {
            item["Durability"] = durability.Current;
        }
        item["Quantity"] = getQuantity();
        item["SetItemMask"] = getSetItemMask();

        Json::Value statLists(Json::arrayValue);
        if (!isSimpleItem())
        {
            getMagicalAttributes(magicalAttributes);
            MagicalAttribute::attributesAsJsonArray(statLists, magicalAttributes, bSerializedFormat);

            if (isRuneword())
            {
                MagicalAttribute::attributesAsJsonArray(statLists, runewordAttrib.MagicalAttributes, bSerializedFormat);
            }

            d2ce::SetAttributes setAttrib;
            switch (quality)
            {
            case EnumItemQuality::SET:
                getSetAttributes(setAttrib);
                if (!setAttrib.SetAttributes.empty())
                {
                    setAttrib.setAttributesAsJsonArray(statLists, bSerializedFormat);
                }
                break;
            }
        }
        item["StatLists"] = statLists;

        item["IsIdentified"] = isIdentified();
        item["IsSocketed"] = isSocketed();
        item["IsNew"] = isNew();
        item["IsEar"] = isEar();
        item["IsStarterItem"] = isStarterItem();
        item["IsCompact"] = isSimpleItem();
        item["IsEthereal"] = isEthereal();
        item["IsPersonalized"] = isPersonalized();
        item["IsRuneword"] = isRuneword();

        if (parent.isArray())
        {
            parent.append(item);
        }
        else
        {
            parent.swap(item);
        }
    }
    else
    {
        unknownAsJson(item);
        item["identified"] = (isIdentified() ? 1 : 0);
        item["socketed"] = (isSocketed() ? 1 : 0);
        item["new"] = (isNew() ? 1 : 0);
        item["is_ear"] = (isEar() ? 1 : 0);
        item["starter_item"] = (isStarterItem() ? 1 : 0);
        item["simple_item"] = (isSimpleItem() ? 1 : 0);
        item["ethereal"] = (isEthereal() ? 1 : 0);
        item["personalized"] = (isPersonalized() ? 1 : 0);
        item["given_runeword"] = (isRuneword() ? 1 : 0);
        item["version"] = getRawVersion();
        item["location_id"] = std::uint16_t(getLocation());
        item["equipped_id"] = std::uint16_t(getEquippedId());
        item["position_x"] = std::uint16_t(getPositionX());
        item["position_y"] = std::uint16_t(getPositionY());
        item["alt_position_id"] = std::uint16_t(getAltPositionId());

        if (isEar())
        {
            EarAttributes earAttrib;
            getEarAttributes(earAttrib);
            earAttrib.asJson(item);
            return;
        }

        std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
        getItemCode(strcode);
        const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
        std::string sCode((char*)strcode.data(), 3);
        item["type"] = sCode;

        Json::Value categories(Json::arrayValue);
        for (auto& category : itemType.categories)
        {
            categories.append(category);
        }
        item["categories"] = categories;

        item["type_id"] = std::uint16_t(itemType.getEnumItemType());
        if (itemType.isQuestItem())
        {
            item["quest_difficulty"] = std::uint16_t(getQuestDifficulty());
        }
        item["nr_of_items_in_sockets"] = std::uint16_t(socketedItemCount());

        if (isSimpleItem())
        {
            getMagicalAttributes(magicalAttributes);
            if (!magicalAttributes.empty())
            {
                Json::Value magicalAttribs(Json::arrayValue);
                for (auto& attrib : magicalAttributes)
                {
                    attrib.asJson(magicalAttribs, bSerializedFormat);
                }
                item["magic_attributes"] = magicalAttribs;
            }
        }
        else
        {
            item["id"] = getId();
            item["level"] = std::uint16_t(getLevel());

            if (dwa_bit_offset != 0)
            {
                if (dwb_bit_offset == 0)
                {
                    dwb_bit_offset = dwa_bit_offset + 32;
                }

                item["dwa"] = (std::uint32_t)read_uint64_bits(dwa_bit_offset, 32);
                item["dwb"] = (std::uint32_t)read_uint64_bits(dwb_bit_offset, 32);
            }

            auto quality = getQuality();
            item["quality"] = std::uint16_t(quality);
            item["multiple_pictures"] = (hasMultipleGraphics() ? 1 : 0);
            if (hasMultipleGraphics())
            {
                item["picture_id"] = std::uint16_t(getPictureId());
            }
            item["class_specific"] = (isAutoAffix() ? 1 : 0);
            if (isAutoAffix())
            {
                item["auto_affix_id"] = getAutoAffixId();
            }
            if (quality == EnumItemQuality::INFERIOR)
            {
                item["low_quality_id"] = std::uint16_t(getInferiorQualityId());
            }
            else if (quality == EnumItemQuality::SUPERIOR)
            {
                item["file_index"] = std::uint16_t(getFileIndex());
            }
            else if (quality == EnumItemQuality::MAGIC)
            {
                d2ce::MagicalAffixes magicalAffixes;
                getMagicalAffixes(magicalAffixes);
                magicalAffixes.asJson(item);
            }

            if (isRuneword())
            {
                getRunewordAttributes(runewordAttrib);
                runewordAttrib.asJson(item);
            }

            d2ce::SetAttributes setAttrib;
            d2ce::RareAttributes rareAttrib;
            d2ce::UniqueAttributes uniqueAttrib;
            switch (quality)
            {
            case EnumItemQuality::SET:
                getSetAttributes(setAttrib);
                setAttrib.asJson(item);
                break;

            case EnumItemQuality::RARE:
            case EnumItemQuality::CRAFT:
            case EnumItemQuality::TEMPERED:
                getRareOrCraftedAttributes(rareAttrib);
                rareAttrib.asJson(item);
                break;

            case EnumItemQuality::UNIQUE:
                getUniqueAttributes(uniqueAttrib);
                uniqueAttrib.asJson(item);
                break;
            }

            if (isPersonalized())
            {
                item["personalized_name"] = getPersonalizedName();
            }

            if (isTome())
            {
                item["magic_suffix"] = getTomeValue();
            }

            item["timestamp"] = (getRealmDataFlag() ? 1 : 0);
            if (isStackable())
            {
                item["quantity"] = getQuantity();
            }

            auto defenseRating = getDefenseRating();
            if (defenseRating > 0)
            {
                item["defense_rating"] = defenseRating;
            }

            ItemDurability durability;
            if (getDurability(durability))
            {
                item["max_durability"] = durability.Max;
                if (durability.Max > 0)
                {
                    item["current_durability"] = durability.Current;
                }
            }

            if (isSocketed())
            {
                item["total_nr_of_sockets"] = std::uint16_t(totalNumberOfSockets());
            }

            Json::Value magicalAttribs(Json::arrayValue);
            getMagicalAttributes(magicalAttributes);
            for (auto& attrib : magicalAttributes)
            {
                attrib.asJson(magicalAttribs, bSerializedFormat);
            }
            item["magic_attributes"] = magicalAttribs;

            if (itemType.isWeapon())
            {
                itemType.dam.asJson(item);
            }

            if (isSocketed())
            {
                // Socketed items
                Json::Value socketedItems(Json::arrayValue);
                for (auto& socketedItem : SocketedItems)
                {
                    socketedItem.asJson(socketedItems, charLevel, bSerializedFormat);
                }
                item["socketed_items"] = socketedItems;
            }
        }

        item["type_name"] = itemType.name;

        ItemRequirements req;
        if (!getRequirements(req))
        {
            req = itemType.req;
        }

        if (req.Strength != 0)
        {
            item["reqstr"] = req.Strength;
        }

        if (req.Dexterity != 0)
        {
            item["reqdex"] = req.Dexterity;
        }

        if (req.Level != 0)
        {
            item["levelreq"] = req.Level;
        }

        item["inv_file"] = itemType.inv_file;
        item["inv_height"] = itemType.dimensions.Height;
        item["inv_width"] = itemType.dimensions.Width;

        if (itemType.inv_transform != 0)
        {
            item["inv_transform"] = itemType.inv_transform;
        }

        auto tc = getTransformColor();
        if (!tc.empty())
        {
            item["transform_color"] = tc;
        }

        if (!isSimpleItem() || !magicalAttributes.empty())
        {
            // For efficiency reasons we do the formatting using the existing list 
            // instead of building it again
            ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);
            for (auto& attrib : magicalAttributes)
            {
                ItemHelpers::formatDisplayedMagicalAttribute(attrib, charLevel);
            }

            // Sort display items in proper order
            std::sort(magicalAttributes.begin(), magicalAttributes.end(), ItemHelpers::magicalAttributeSorter);

            Json::Value displayedMagicAttributes(Json::arrayValue);
            for (auto& attrib : magicalAttributes)
            {
                attrib.asJson(displayedMagicAttributes, bSerializedFormat);
            }
            item["displayed_magic_attributes"] = displayedMagicAttributes;

            Json::Value displayedRunewordAttributes(Json::arrayValue);
            if (!runewordAttrib.MagicalAttributes.empty())
            {
                ItemHelpers::checkForRelatedMagicalAttributes(runewordAttrib.MagicalAttributes);
                for (auto& attrib : runewordAttrib.MagicalAttributes)
                {
                    ItemHelpers::formatDisplayedMagicalAttribute(attrib, charLevel);
                }

                // Sort display items in proper order
                std::sort(runewordAttrib.MagicalAttributes.begin(), runewordAttrib.MagicalAttributes.end(), ItemHelpers::magicalAttributeSorter);

                for (auto& attrib : runewordAttrib.MagicalAttributes)
                {
                    attrib.asJson(displayedRunewordAttributes, bSerializedFormat);
                }
            }
            item["displayed_runeword_attributes"] = displayedRunewordAttributes;

            Json::Value combinedMagicAttributes(Json::arrayValue);
            getCombinedMagicalAttributes(magicalAttributes);
            for (auto& attrib : magicalAttributes)
            {
                attrib.asJson(combinedMagicAttributes, bSerializedFormat);
            }
            item["combined_magic_attributes"] = combinedMagicAttributes;

            // For efficiency reasons we do the formatting using the existing list 
            // instead of building it again
            ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);
            for (auto& attrib : magicalAttributes)
            {
                ItemHelpers::formatDisplayedMagicalAttribute(attrib, charLevel);
            }

            // Sort display items in proper order
            std::sort(magicalAttributes.begin(), magicalAttributes.end(), ItemHelpers::magicalAttributeSorter);

            Json::Value displayedCombinedMagicAttributes(Json::arrayValue);
            for (auto& attrib : magicalAttributes)
            {
                attrib.asJson(displayedCombinedMagicAttributes, bSerializedFormat);
            }
            item["displayed_combined_magic_attributes"] = displayedCombinedMagicAttributes;
        }

        if (parent.isArray())
        {
            parent.append(item);
        }
        else
        {
            parent.swap(item);
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::Item::unknownAsJson(Json::Value& parent, bool /*bSerializedFormat*/) const
{
    struct byteRange
    {
        size_t startIdx = 0;
        size_t endIdx = 0;
    };
    static std::vector<byteRange> unknowns = { {0,3},{5,10},{12,12},{14,15},{18,20},{23,23},{25,25},{27,31} };

    Json::Value unknownData;
    for (const auto& byteInfo : unknowns)
    {
        byteRangeAsJson(unknownData, byteInfo.startIdx, byteInfo.endIdx);
    }
    parent["_unknown_data"] = unknownData;
}
//---------------------------------------------------------------------------
void d2ce::Item::byteRangeAsJson(Json::Value& parent, size_t startByte, size_t endByte) const
{
    if (endByte < startByte)
    {
        return;
    }

    std::string propName;
    {
        std::stringstream ss;
        ss << "b" << std::dec << startByte;
        if (endByte > startByte)
        {
            ss << "_" << std::dec << endByte;
        }

        propName = ss.str();
    }

    Json::Value unknownData;
    startByte += start_bit_offset;
    endByte += start_bit_offset;
    for (size_t idx = startByte; idx <= endByte; ++idx)
    {
        std::stringstream ss;
        ss << std::dec << (idx - startByte);
        unknownData[ss.str()] = read_uint32_bits(idx, 1);
    }
    parent[propName] = unknownData;
}
//---------------------------------------------------------------------------
bool d2ce::Item::parsePropertyList(std::FILE* charfile, size_t& current_bit_offset)
{
    std::uint16_t id = (std::uint16_t)readBits(charfile, current_bit_offset, PROPERTY_ID_NUM_BITS);
    if (feof(charfile))
    {
        return false;
    }

    while (id != 0x1FF)
    {
        if (id >= ItemHelpers::getItemStatSize())
        {
            // corrupt file
            return false;
        }

        const ItemStat* stat = &ItemHelpers::getItemStat(id);

        // saveBits being zero or >= 64 is unrecoverably bad, and
        if (stat->saveBits == 0 || stat->saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        size_t numParms = 0;
        if (stat->encode == 2)
        {
            if (stat->saveBits != 7 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 3;
        }
        else if (stat->encode == 3)
        {
            if (stat->saveBits != 16 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 4;
        }
        else if (stat->encode == 4)
        {
            numParms = 3;
            if (stat->saveBits != 3 || stat->saveParamBits != 16)
            {
                // time-based stats were never implemented, so it's a corrupt file
                if (stat->saveBits != 22 || stat->saveParamBits != 0)
                {
                    // corrupt file
                    return false;
                }

                numParms = 1;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }
        }
        else if (stat->saveParamBits > 0)
        {
            if (!skipBits(charfile, current_bit_offset, size_t(stat->saveBits) + size_t(stat->saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 2;
        }
        else
        {
            if (!skipBits(charfile, current_bit_offset, stat->saveBits))
            {
                // corrupt file
                return false;
            }

            numParms = 1;
        }

        while (stat->nextInChain && numParms < 4)
        {
            if (stat->nextInChain >= ItemHelpers::getItemStatSize())
            {
                // corrupt file
                return false;
            }

            stat = &ItemHelpers::getItemStat(stat->nextInChain);
            if (stat->saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, stat->saveBits))
            {
                // corrupt file
                return false;
            }
            ++numParms;
        }

        id = (std::uint16_t)readBits(charfile, current_bit_offset, PROPERTY_ID_NUM_BITS);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::parsePropertyList(const Json::Value& propListRoot, bool bSerializedFormat, size_t& current_bit_offset)
{
    std::uint16_t id = 0x1FF;
    std::uint16_t nextId = 0x1FF;
    std::int64_t value = 0;
    std::vector<std::int64_t> values;
    size_t valueIdx = 0;
    if (propListRoot.isNull() || !propListRoot.isArray() || propListRoot.empty())
    {
        id = 0x1FF;
        return setBits(current_bit_offset, PROPERTY_ID_NUM_BITS, id);
    }

    Json::Value node;
    auto iter_end = propListRoot.end();
    for (auto iter = propListRoot.begin(); iter != iter_end; ++iter)
    {
        values.clear();
        valueIdx = 0;
        if (iter->isNull())
        {
            continue;
        }

        node = iter->operator[](bSerializedFormat ? "Id" : "id");
        if (node.isNull())
        {
            return false;
        }

        id = std::uint16_t(node.asInt64());
        if (id >= ItemHelpers::getItemStatSize())
        {
            return false;
        }

        if (!setBits(current_bit_offset, PROPERTY_ID_NUM_BITS, id))
        {
            return false;
        }

        const ItemStat* stat = &ItemHelpers::getItemStat(id);

        // saveBits being zero or >= 64 is unrecoverably bad, and
        // encode type 4 is only used by stats that were never implemented (time-based stats)
        if (stat->saveBits == 0 || stat->saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        if (bSerializedFormat)
        {
            switch (id)
            {
            case 17:
            case 48:
            case 50:
            case 52:
                node = iter->operator[]("Value");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                ++iter;
                if (iter == iter_end)
                {
                    return false;
                }

                node = iter->operator[](bSerializedFormat ? "Id" : "id");
                if (node.isNull())
                {
                    return false;
                }

                nextId = id + 1;
                id = std::uint16_t(node.asInt64());
                if (id >= ItemHelpers::getItemStatSize() || id != nextId)
                {
                    return false;
                }

                node = iter->operator[]("Value");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());
                break;

            case 54:
            case 57:
                node = iter->operator[]("Value");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                ++iter;
                if (iter == iter_end)
                {
                    return false;
                }

                node = iter->operator[]("Id");
                if (node.isNull())
                {
                    return false;
                }

                nextId = id + 1;
                id = std::uint16_t(node.asInt64());
                if (id >= ItemHelpers::getItemStatSize() || id != nextId)
                {
                    return false;
                }

                node = iter->operator[]("Value");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                ++iter;
                if (iter == iter_end)
                {
                    return false;
                }

                node = iter->operator[]("Id");
                if (node.isNull())
                {
                    return false;
                }
                
                nextId = id + 1;
                id = std::uint16_t(node.asInt64());
                if (id >= ItemHelpers::getItemStatSize() || id != nextId)
                {
                    return false;
                }

                node = iter->operator[]("Values");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());
                break;

            case 188:
                node = iter->operator[]("SkillTab");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                node = iter->operator[]("SkillLevel");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                node = iter->operator[]("Value");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());
                break;

            case 195:
            case 196:
            case 197:
            case 198:
            case 199:
            case 201:
                node = iter->operator[]("SkillLevel");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                node = iter->operator[]("SkillId");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                node = iter->operator[]("Value");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());
                break;

            case 204:
                node = iter->operator[]("SkillLevel");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                node = iter->operator[]("SkillId");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                node = iter->operator[]("MaxCharges");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());

                node = iter->operator[]("Value");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());
                break;

            default:
                if (stat->saveParamBits > 0)
                {
                    node = iter->operator[]("Param");
                    if (node.isNull())
                    {
                        values.push_back(0);
                    }

                    values.push_back(node.asInt64());
                }

                node = iter->operator[]("Value");
                if (node.isNull())
                {
                    values.push_back(0);
                }

                values.push_back(node.asInt64());
                break;
            }
        }
        else
        {
            node = iter->operator[]("values");
            if (node.isNull() || !node.isArray())
            {
                return false;
            }

            auto iter2_end = node.end();
            for (auto iter2 = node.begin(); iter2 != iter2_end; ++iter2)
            {
                if (iter2->isNull())
                {
                    values.push_back(0);
                }

                values.push_back(iter2->asInt64());
            }
        }

        if (stat->encode == 2)
        {
            if (stat->saveBits != 7 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 6, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 10, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, stat->saveBits, value))
            {
                return false;
            }
        }
        else if (stat->encode == 3)
        {
            if (stat->saveBits != 16 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 6, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 10, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 8, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 8, value))
            {
                return false;
            }
        }
        else if (stat->encode == 4)
        {
            if (stat->saveBits != 3 || stat->saveParamBits != 16)
            {
                // time-based stats were never implemented, so it's a corrupt file
                if (stat->saveBits != 22 || stat->saveParamBits != 0)
                {
                    // corrupt file
                    return false;
                }

                if (valueIdx >= values.size())
                {
                    return false;
                }

                value = values[valueIdx] + stat->saveAdd;
            }
            else
            {
                if (valueIdx >= values.size())
                {
                    return false;
                }

                value = values[valueIdx] + stat->saveAdd;
                ++valueIdx;
                if (!setBits64(current_bit_offset, 3, value))
                {
                    return false;
                }

                if (valueIdx >= values.size())
                {
                    return false;
                }
                value = values[valueIdx] + stat->saveAdd;
                ++valueIdx;
                if (!setBits64(current_bit_offset, 13, value))
                {
                    return false;
                }

                if (valueIdx >= values.size())
                {
                    return false;
                }
                value = values[valueIdx] + stat->saveAdd;
                ++valueIdx;
                if (!setBits64(current_bit_offset, stat->saveBits, value))
                {
                    return false;
                }
            }
        }
        else if (stat->saveParamBits > 0)
        {
            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, stat->saveParamBits, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, stat->saveBits, value))
            {
                return false;
            }
        }
        else
        {
            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, stat->saveBits, value))
            {
                return false;
            }
        }

        while (stat->nextInChain && valueIdx < 4)
        {
            if (stat->nextInChain >= ItemHelpers::getItemStatSize())
            {
                // corrupt file
                return false;
            }

            stat = &ItemHelpers::getItemStat(stat->nextInChain);
            if (stat->saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat->saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, stat->saveBits, value))
            {
                return false;
            }
        }
    }

    id = 0x1FF;
    if (!setBits(current_bit_offset, PROPERTY_ID_NUM_BITS, id))
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readPropertyList(size_t& current_bit_offset, std::vector<MagicalAttribute>& attrib) const
{
    attrib.clear();
    if (current_bit_offset == 0)
    {
        return false;
    }

    std::uint16_t id = (std::uint16_t)read_uint32_bits(current_bit_offset, PROPERTY_ID_NUM_BITS);
    current_bit_offset += PROPERTY_ID_NUM_BITS;

    while (id != 0x1FF)
    {
        MagicalAttribute magicalAttrib;
        if (id >= ItemHelpers::getItemStatSize())
        {
            // corrupt file
            return false;
        }

        magicalAttrib.Id = id;

        const ItemStat* stat = &ItemHelpers::getItemStat(id);
        magicalAttrib.Name = stat->name;
        magicalAttrib.Desc = stat->desc;

        // saveBits being zero or >= 64 is unrecoverably bad, and
        // encode type 4 is only used by stats that were never implemented (time-based stats)
        if (stat->saveBits == 0 || stat->saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        if (stat->encode == 2)
        {
            if (stat->saveBits != 7 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 6) - stat->saveAdd);
            current_bit_offset += 6;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 10) - stat->saveAdd);
            current_bit_offset += 10;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }
        else if (stat->encode == 3)
        {
            if (stat->saveBits != 16 || stat->saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 6) - stat->saveAdd);
            current_bit_offset += 6;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 10) - stat->saveAdd);
            current_bit_offset += 10;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 8) - stat->saveAdd);
            current_bit_offset += 8;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 8) - stat->saveAdd);
            current_bit_offset += 8;
        }
        else if (stat->encode == 4)
        {
            if (stat->saveBits != 3 || stat->saveParamBits != 16)
            {
                // time-based stats were never implemented, so it's a corrupt file
                if (stat->saveBits != 22 || stat->saveParamBits != 0)
                {
                    // corrupt file
                    return false;
                }

                magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            }
            else
            {
                magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 3) - stat->saveAdd);
                current_bit_offset += 3;

                magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, 13) - stat->saveAdd);
                current_bit_offset += 13;

                magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
                current_bit_offset += stat->saveBits;
            }
        }
        else if (stat->saveParamBits > 0)
        {
            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveParamBits) - stat->saveAdd);
            current_bit_offset += stat->saveParamBits;

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }
        else
        {
            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }

        while (stat->nextInChain && magicalAttrib.Values.size() < 4)
        {
            if (stat->nextInChain >= ItemHelpers::getItemStatSize())
            {
                // corrupt file
                return false;
            }

            stat = &ItemHelpers::getItemStat(stat->nextInChain);
            if (stat->saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(read_uint64_bits(current_bit_offset, stat->saveBits) - stat->saveAdd);
            current_bit_offset += stat->saveBits;
        }

        attrib.push_back(magicalAttrib);
        id = (std::uint16_t)read_uint32_bits(current_bit_offset, PROPERTY_ID_NUM_BITS);
        current_bit_offset += PROPERTY_ID_NUM_BITS;
    }

    return true;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getEncodedChar(std::FILE* charfile, size_t& current_bit_offset)
{
    std::string bitStr;
    size_t startRead = current_bit_offset;
    size_t readOffset = startRead;
    std::uint8_t result = UINT8_MAX;
    while (bitStr.size() < 9)
    {
        readOffset = startRead;
        std::stringstream ss2;
        ss2 << std::bitset<9>(readBits(charfile, readOffset, bitStr.size() + 1));
        current_bit_offset = readOffset;
        bitStr = ss2.str().substr(8 - bitStr.size());
        result = ItemHelpers::HuffmanDecodeBitString(bitStr);
        if (result != UINT8_MAX)
        {
            return result;
        }
    }

    // something went wrong
    return UINT8_MAX;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateBits(size_t start, size_t size, std::uint32_t value)
{
    size_t startIdx = start / 8;
    size_t endIdx = (start + size - 1) / 8;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    size_t startBit = start % 8;
    size_t numBytes = (endIdx - startIdx) + 1;
    if (numBytes > 5)
    {
        // 32 bit value can't consume more then this many bytes
        return false;
    }

    std::uint64_t mask = (((std::uint64_t)1 << size) - 1) << (startBit);
    std::uint64_t dest = 0;
    std::memcpy((std::uint8_t*)&dest, &data[startIdx], numBytes);
    dest = (dest & ~mask) | (((std::uint64_t)value << (startBit)) & mask);
    std::memcpy(&data[startIdx], (std::uint8_t*)&dest, numBytes);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateBits64(size_t start, size_t size, std::uint64_t value)
{
    size_t startIdx = start / 8;
    size_t endIdx = (start + size - 1) / 8;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    size_t startBit = start % 8;
    size_t numBytes = (endIdx - startIdx) + 1;
    if (numBytes > 8)
    {
        // value is too big for us to handle
        return false;
    }

    std::uint64_t mask = (((std::uint64_t)1 << size) - 1) << (startBit);
    std::uint64_t dest = 0;
    std::memcpy((std::uint8_t*)&dest, &data[startIdx], numBytes);
    dest = (dest & ~mask) | (((std::uint64_t)value << (startBit)) & mask);
    std::memcpy(&data[startIdx], (std::uint8_t*)&dest, numBytes);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateItemCodev115(std::uint64_t code, size_t numBitsSet)
{
    auto oldNumBitsSet = std::uint8_t(std::int64_t(extended_data_offset) - std::int64_t(type_code_offset));
    if (oldNumBitsSet == numBitsSet)
    {
        // easy change
        return updateBits64(type_code_offset, numBitsSet, code);
    }

    // complex change
    size_t old_current_bit_offset = extended_data_offset;
    size_t bitsToCopy = item_end_bit_offset - extended_data_offset;

    std::int64_t diff = std::int64_t(numBitsSet) - std::int64_t(oldNumBitsSet);
    extended_data_offset += diff;
    item_end_bit_offset += diff;
    size_t current_bit_offset = extended_data_offset;

    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // truncate the original
    size_t newSize = (item_end_bit_offset + 7) / 8;
    data.resize(newSize, 0);
    updateBits64(type_code_offset, numBitsSet, code); // update the bits

    // now copy the remaining bits
    std::uint32_t value = 0;
    std::uint8_t bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBits(current_bit_offset, bits, value);
        current_bit_offset += bits;
        bits = (std::uint8_t)std::min(sizeof(value), bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        value = 0;
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBits(current_bit_offset, bits, 0);
    }

    if (quest_difficulty_offset != 0)
    {
        quest_difficulty_offset += diff;
    }

    if (nr_of_items_in_sockets_offset != 0)
    {
        nr_of_items_in_sockets_offset += diff;
    }

    // update offsets after the extended_data_offset
    if (!isSimpleItem())
    {
        if (item_id_bit_offset != 0)
        {
            item_id_bit_offset += diff;
        }

        if (item_level_bit_offset != 0)
        {
            item_level_bit_offset += diff;
        }

        if (quality_bit_offset != 0)
        {
            quality_bit_offset += diff;
        }

        if (multi_graphic_bit_offset != 0)
        {
            multi_graphic_bit_offset += diff;
        }

        if (autoAffix_bit_offset != 0)
        {
            autoAffix_bit_offset += diff;
        }

        if (quality_attrib_bit_offset != 0)
        {
            quality_attrib_bit_offset += diff;
        }

        if (runeword_id_bit_offset != 0)
        {
            runeword_id_bit_offset += diff;
        }

        if (personalized_bit_offset_marker != 0)
        {
            personalized_bit_offset_marker += diff;
        }

        if (personalized_bit_offset != 0)
        {
            personalized_bit_offset += diff;
        }

        if (tome_bit_offset != 0)
        {
            tome_bit_offset += diff;
        }

        if (realm_bit_offset != 0)
        {
            realm_bit_offset += diff;
        }

        if (defense_rating_bit_offset != 0)
        {
            defense_rating_bit_offset += diff;
        }

        if (durability_bit_offset != 0)
        {
            durability_bit_offset += diff;
        }

        if (stackable_bit_offset != 0)
        {
            stackable_bit_offset += diff;
        }

        if (gld_stackable_bit_offset != 0)
        {
            gld_stackable_bit_offset += diff;
        }

        if (socket_count_bit_offset_marker != 0)
        {
            socket_count_bit_offset_marker += diff;
        }

        if (socket_count_bit_offset != 0)
        {
            socket_count_bit_offset += diff;
        }

        if (bonus_bits_bit_offset != 0)
        {
            bonus_bits_bit_offset += diff;
        }

        if (magical_props_bit_offset != 0)
        {
            magical_props_bit_offset += diff;
        }

        if (set_bonus_props_bit_offset != 0)
        {
            set_bonus_props_bit_offset += diff;
        }

        if (runeword_props_bit_offset != 0)
        {
            runeword_props_bit_offset += diff;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getInferiorQualityIdv100() const
{
    if (dwa_bit_offset == 0)
    {
        return 0;
    }

    switch (getQuality())
    {
    case EnumItemQuality::INFERIOR:
        break;

    default:
        return 0;
    }

    if (dwb_bit_offset == 0)
    {
        dwb_bit_offset = dwa_bit_offset + 32;
    }

    return ItemHelpers::generateInferiorQualityIdv100((std::uint32_t)read_uint64_bits(dwb_bit_offset, 32), getLevel());
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAffixesv100(MagicalAffixes& affixes) const
{
    affixes.clear();
    if ((dwa_bit_offset == 0) || (quality_attrib_bit_offset == 0) || (type_code_offset == 0))
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::MAGIC:
        break;

    default:
        return false;
    }

    if (magic_affixes_v100.Affixes.PrefixId != MAXUINT16)
    {
        affixes = magic_affixes_v100.Affixes;
        return true;
    }

    if (dwb_bit_offset == 0)
    {
        dwb_bit_offset = dwa_bit_offset + 32;
    }

    if (!ItemHelpers::generateMagicalAffixesv100(std::uint16_t(read_uint32_bits(type_code_offset, 10)), (std::uint32_t)read_uint64_bits(dwb_bit_offset, 32), getLevel(), magic_affixes_v100) || (magic_affixes_v100.Affixes.PrefixId == MAXUINT16))
    {
        return false;
    }

    affixes = magic_affixes_v100.Affixes;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getSetAttributesv100(SetAttributes& attrib) const
{
    attrib.clear();
    if (quality_attrib_bit_offset == 0)
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::SET:
        break;

    default:
        return false;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        // should not happen
        return false;
    }

    attrib.Id = ItemHelpers::getSetIDv100((std::uint16_t)read_uint32_bits(quality_attrib_bit_offset, ITEN_V100_UNIQUE_ID_NUM_BITS), strcode);
    if (attrib.Id >= UINT16_MAX)
    {
        attrib.Id = 0;
        return false;
    }

    attrib.Name = ItemHelpers::getSetNameFromId(attrib.Id);
    attrib.ReqLevel = ItemHelpers::getSetLevelReqFromId(attrib.Id);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAttributesv100(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();
    d2ce::UniqueAttributes uniqueAttrib;
    d2ce::SetAttributes setAttrib;
    MagicalAffixes affixes;
    switch (getQuality())
    {
    case EnumItemQuality::SET:
        if (getSetAttributes(setAttrib))
        {
            return ItemHelpers::getSetMagicAttribsV100(setAttrib.Id, attribs);
        }
        break;
       
    case EnumItemQuality::UNIQUE:
        if (getUniqueAttributes(uniqueAttrib) && !uniqueAttrib.Name.empty())
        {
            return ItemHelpers::getUniqueMagicAttribsV100(uniqueAttrib.Id, attribs);
        }
        break;

    case EnumItemQuality::MAGIC:
        if (getMagicalAffixesv100(affixes) && (magic_affixes_v100.Affixes.PrefixId != MAXUINT16))
        {
            attribs = magic_affixes_v100.MagicalAttributes;
            return true;
        }
        break;

    default:
        if (isQuestItem())
        {
            std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
            if (getItemCode(strcode))
            {
                return ItemHelpers::getUniqueQuestMagicAttribsV100(strcode, attribs);
            }

        }
        break;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRareOrCraftedAttributesv100(RareAttributes& attrib) const
{
    attrib.clear();
    if ((dwa_bit_offset == 0) || (quality_attrib_bit_offset == 0) || (type_code_offset == 0))
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFT:
    case EnumItemQuality::TEMPERED:
        break;

    default:
        return false;
    }

    if (rare_affixes_v100.Id != MAXUINT16)
    {
        attrib.Id = rare_affixes_v100.Id;
        attrib.Name = rare_affixes_v100.Name;
        attrib.Id2 = rare_affixes_v100.Id2;
        attrib.Name2 = rare_affixes_v100.Name2;
        for (auto& item : rare_affixes_v100.affixes)
        {
            attrib.Affixes.push_back(item.Affixes);
        }
        return true;
    }

    if (dwb_bit_offset == 0)
    {
        dwb_bit_offset = dwa_bit_offset + 32;
    }

    if (!ItemHelpers::generateRareOrCraftedAffixesv100(std::uint16_t(read_uint32_bits(type_code_offset, 10)), (std::uint32_t)read_uint64_bits(dwb_bit_offset, 32), getLevel(), rare_affixes_v100) || (rare_affixes_v100.Id == MAXUINT16))
    {
        return false;
    }

    attrib.Id = rare_affixes_v100.Id;
    attrib.Name = rare_affixes_v100.Name;
    attrib.Id2 = rare_affixes_v100.Id2;
    attrib.Name2 = rare_affixes_v100.Name2;
    for (auto& item : rare_affixes_v100.affixes)
    {
        attrib.Affixes.push_back(item.Affixes);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::hasMultipleGraphicsv100() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        return false;
    }

    auto code = ItemHelpers::getTypeCodev100(strcode);
    switch (code)
    {
    case 0x117: // amulet
    case 0x119: // rings
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPictureIdv100() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        return false;
    }

    std::uint32_t modulo = 3;
    auto code = ItemHelpers::getTypeCodev100(strcode);
    switch (code)
    {
    case 0x117: // amulet
        modulo = 3;
        break;

    case 0x119: // rings
        modulo = 5;
        break;

    default:
        return 0;
    }

    return std::uint8_t(ItemHelpers::generateDWARandomOffsetv100((std::uint32_t)read_uint64_bits(dwa_bit_offset, 32), 1) % modulo);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getDefenseRatingv100() const
{
    if (dwa_bit_offset == 0)
    {
        return 0;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        return false;
    }

    return ItemHelpers::generateDefenseRatingv100(strcode, (std::uint32_t)read_uint64_bits(dwa_bit_offset, 32));
}
//---------------------------------------------------------------------------
void d2ce::Items::findItems()
{
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    ItemLocationReference.clear();
    ItemAltLocationReference.clear();
    ItemLocationEmptySpots.clear();
    ItemAltLocationEmptySpots.clear();
    HasBeltEquipped = false;
    HasHoradricCube = false;
    EquippedBeltSlots = 0;

    std::map<d2ce::EnumAltItemLocation, ItemDimensions> altItemDimensions;
    {
        auto& emptySlots = ItemAltLocationEmptySpots[d2ce::EnumAltItemLocation::INVENTORY];
        auto& dimensions = altItemDimensions[d2ce::EnumAltItemLocation::INVENTORY];
        if (getItemLocationDimensions(d2ce::EnumAltItemLocation::INVENTORY, dimensions))
        {
            std::uint16_t totalPos = dimensions.InvHeight * dimensions.InvWidth;
            for (std::uint16_t pos = 0; pos < totalPos; ++pos)
            {
                emptySlots.insert(pos);
            }
        }
    }

    ItemDimensions stashDimensions;
    {
        auto& emptySlots = ItemAltLocationEmptySpots[d2ce::EnumAltItemLocation::STASH];
        auto& dimensions = altItemDimensions[d2ce::EnumAltItemLocation::STASH];
        if (getItemLocationDimensions(d2ce::EnumAltItemLocation::STASH, dimensions))
        {
            std::uint16_t totalPos = dimensions.InvHeight * dimensions.InvWidth;
            for (std::uint16_t pos = 0; pos < totalPos; ++pos)
            {
                emptySlots.insert(pos);
            }
        }
    }

    ItemDimensions dimensions;
    auto itemLocation = d2ce::EnumItemLocation::BUFFER;
    auto itemAltLocation = d2ce::EnumAltItemLocation::UNKNOWN;
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    for (auto& item : Inventory)
    {
        item.getItemCode(strcode);
        const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
        if (itemType.isStackable())
        {
            Stackables.push_back(item);

            if (itemType.isWeapon())
            {
                Weapons.push_back(item);
            }
        }
        else if (itemType.isWeapon())
        {
            Weapons.push_back(item);
        }
        else if (itemType.isArmor())
        {
            Armor.push_back(item);
        }
        else if (itemType.isPotion() || itemType.isGem() || itemType.isRune())
        {
            GPSs.push_back(item);
        }

        itemLocation = item.getLocation();
        switch (itemLocation)
        {
        case d2ce::EnumItemLocation::BELT:
            HasBeltEquipped = true;
            ItemLocationReference[itemLocation].push_back(const_cast<d2ce::Item&>(item));
            break;

        case d2ce::EnumItemLocation::EQUIPPED:
            ItemLocationReference[itemLocation].push_back(const_cast<d2ce::Item&>(item));
            if ((!HasBeltEquipped || (EquippedBeltSlots == 0)) && item.isBelt())
            {
                HasBeltEquipped = true;
                EquippedBeltSlots = item.getTotalItemSlots();
            }
            break;

        case d2ce::EnumItemLocation::STORED:
            itemAltLocation = item.getAltPositionId();
            switch (itemAltLocation)
            {
            case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
                HasHoradricCube = true;
                ItemAltLocationReference[itemAltLocation].push_back(const_cast<d2ce::Item&>(item));
                break;

            case d2ce::EnumAltItemLocation::INVENTORY:
            case d2ce::EnumAltItemLocation::STASH:
                ItemAltLocationReference[itemAltLocation].push_back(const_cast<d2ce::Item&>(item));
                if (!HasHoradricCube)
                {
                    HasHoradricCube = item.isHoradricCube();
                }

                if (item.getDimensions(dimensions))
                {
                    // remove slots from empty list
                    auto& emptySlots = ItemAltLocationEmptySpots[itemAltLocation];
                    auto& invDimenstion =altItemDimensions[itemAltLocation];
                    auto posX = item.getPositionX();
                    auto posY = item.getPositionY();
                    for (std::uint16_t y = posY; y < posY + dimensions.Height; ++y)
                    {
                        for (std::uint16_t x = posX; x < posX + dimensions.Width; ++x)
                        {
                            emptySlots.erase(y * invDimenstion.Width + x);
                        }
                    }
                }
                break;
            }
        }
    }

    {
        auto& emptySlots = ItemLocationEmptySpots[d2ce::EnumItemLocation::BELT];
        if (getItemLocationDimensions(d2ce::EnumItemLocation::BELT, dimensions))
        {
            std::uint16_t totalPos = dimensions.InvHeight * dimensions.InvWidth;
            for (std::uint16_t pos = 0; pos < totalPos; ++pos)
            {
                emptySlots.insert(pos);
            }

            // remove slots from empty list
            for (const auto& itemRef : ItemLocationReference[d2ce::EnumItemLocation::BELT])
            {
                emptySlots.erase(itemRef.get().getPositionX());
            }
        }
    } // end for

    if (HasHoradricCube)
    {
        itemAltLocation = EnumAltItemLocation::HORADRIC_CUBE;
        auto& emptySlots = ItemAltLocationEmptySpots[itemAltLocation];
        auto& cubeDimensions = altItemDimensions[itemAltLocation];
        if (getItemLocationDimensions(itemAltLocation, cubeDimensions))
        {
            std::uint16_t totalPos = dimensions.InvHeight * dimensions.InvWidth;
            for (std::uint16_t pos = 0; pos < totalPos; ++pos)
            {
                emptySlots.insert(pos);
            }

            // remove slots from empty list
            for (const auto& itemRef : ItemAltLocationReference[itemAltLocation])
            {
                // remove slots from empty list
                auto posX = itemRef.get().getPositionX();
                auto posY = itemRef.get().getPositionY();
                if (itemRef.get().getDimensions(dimensions))
                {
                    for (std::uint16_t y = posY; y < posY + dimensions.Height; ++y)
                    {
                        for (std::uint16_t x = posX; x < posX + dimensions.Width; ++x)
                        {
                            emptySlots.erase(y * cubeDimensions.Width + x);
                        }
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::list<d2ce::Item>& items)
{
    numItems = 0;
    items.clear();
    if (update_locations)
    {
        // find items location
        location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)MIN_START_STATS_POS)
        {
            cur_pos = MIN_START_STATS_POS;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[1])
            {
                continue;
            }

            // found item marker
            std::fread(&numItems, sizeof(numItems), 1, charfile);
            location = std::ftell(charfile);
            break;
        }

        if (location == 0)
        {
            return false;
        }
    }
    else
    {
        if (location == 0)
        {
            return false;
        }

        std::fseek(charfile, location - sizeof(numItems), SEEK_SET);
        std::fread(&numItems, sizeof(numItems), 1, charfile);
    }

    if (numItems > 0)
    {
        if (!fillItemsArray(charfile, location, numItems, items))
        {
            // Corrupt file
            if (update_locations)
            {
                location = 0;
            }
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::fillItemsArray(std::FILE* charfile, std::uint32_t location, std::uint16_t numItems, std::list<d2ce::Item>& items)
{
    std::fseek(charfile, location, SEEK_SET);
    while (items.size() < numItems)
    {
        if (feof(charfile))
        {
            return false;
        }

        items.resize(items.size() + 1);
        auto& item = items.back();
        if (!item.readItem(Version, charfile))
        {
            // corrupt file
            items.pop_back();
            return false;
        }
    }  // end while

    return numItems == items.size() ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItemsList(const Json::Value& itemListroot, bool bSerializedFormat, std::list<Item>& items)
{
    if (!itemListroot.isNull())
    {
        if (!fillItemsArray(itemListroot, bSerializedFormat, items))
        {
            // Corrupt file
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::list<Item>& items)
{
    numItems = 0;
    bool checkItemCount = false;
    std::uint16_t expectedNumOfItems = 0;
    Json::Value playerItemsRoot;
    if (!root.isNull())
    {
        playerItemsRoot = root[bSerializedFormat ? "PlayerItemList" : "items"];
        if (!playerItemsRoot.isNull() && bSerializedFormat)
        {
            // If "Header" value is present, it needs to be valid
            Json::Value value = playerItemsRoot["Header"];
            if (!value.isNull())
            {
                if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)ITEM_MARKER.data()))
                {
                    // bad header
                    location = 0;
                    return false;
                }
            }

            // If "Count" value is present, it needs to be validated to be true
            value = playerItemsRoot["Count"];
            if (!value.isNull())
            {
                checkItemCount = true;
                expectedNumOfItems = std::uint16_t(value.asInt64());
            }

            playerItemsRoot = playerItemsRoot["Items"];
        }
    }

    if (!readItemsList(playerItemsRoot, bSerializedFormat, items))
    {
        // Corrupt file
        location = 0;
        return false;
    }

    numItems = std::uint16_t(items.size());
    if (checkItemCount && (numItems != expectedNumOfItems))
    {
        // Corrupt file
        location = 0;
        return false;
    }

    std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
    std::fwrite(&numItems, sizeof(numItems), 1, charfile);
    location = std::ftell(charfile);
    for (auto& item : items)
    {
        if (!item.writeItem(charfile))
        {
            // Corrupt file
            location = 0;
            return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::fillItemsArray(const Json::Value& itemsRoot, bool bSerializedFormat, std::list<Item>& items)
{
    if (itemsRoot.isNull())
    {
        return true;
    }

    auto iter_end = itemsRoot.end();
    for (auto iter = itemsRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        items.resize(items.size() + 1);
        auto& item = items.back();
        if (!item.readItem(*iter, bSerializedFormat, Version))
        {
            // corrupt file
            items.pop_back();
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readCorpseItems(std::FILE* charfile)
{
    CorpseInfo.clear();
    if (update_locations)
    {
        // find items location
        corpse_location = 0;
        corpse_item_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)MIN_START_STATS_POS)
        {
            cur_pos = MIN_START_STATS_POS;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[1])
            {
                continue;
            }

            // found item marker
            std::fread(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
            if (CorpseInfo.IsDead > 1)
            {
                return false;
            }

            corpse_location = std::ftell(charfile);
            break;
        }

        if (corpse_location == 0)
        {
            return false;
        }
    }
    else
    {
        if (corpse_location == 0)
        {
            return false;
        }

        std::fseek(charfile, corpse_location - sizeof(CorpseInfo.IsDead), SEEK_SET);
        std::fread(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
        if (CorpseInfo.IsDead > 1)
        {
            return false;
        }
    }

    if (CorpseInfo.IsDead > 0)
    {
        std::fread(&CorpseInfo.Unknown, sizeof(CorpseInfo.Unknown), 1, charfile);
        std::fread(&CorpseInfo.X, sizeof(CorpseInfo.X), 1, charfile);
        std::fread(&CorpseInfo.Y, sizeof(CorpseInfo.Y), 1, charfile);
        if (!readItems(charfile, corpse_item_location, NumOfCorpseItems, CorpseItems))
        {
            if (update_locations)
            {
                corpse_location = 0;
            }

            return false;
        }
    }
    else
    {
        corpse_item_location = 0;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readCorpseItems(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile)
{
    CorpseInfo.clear();

    bool checkItemCount = false;
    std::uint16_t expectedNumOfItems = 0;
    Json::Value corpseItemsRoot;
    Json::Value corpseLocationRoot;
    if (!root.isNull())
    {
        if (!bSerializedFormat)
        {
            Json::Value value = root["is_dead"];
            if (!value.isNull())
            {
                CorpseInfo.IsDead = std::uint16_t(value.asInt64());
                if (CorpseInfo.IsDead)
                {
                    corpseLocationRoot = root["corpse_location"];
                }
            }
        }

        corpseItemsRoot = root[bSerializedFormat ? "PlayerCorpses" : "corpse_items"];
        if (!corpseItemsRoot.isNull() && bSerializedFormat)
        {
            // If "Header" value is present, it needs to be valid
            Json::Value value = corpseItemsRoot["Header"];
            if (!value.isNull())
            {
                if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)ITEM_MARKER.data()))
                {
                    // bad header
                    return false;
                }

                // If "Count" value is present, it needs to be validated to be true
                value = corpseItemsRoot["Count"];
                if (!value.isNull())
                {
                    CorpseInfo.IsDead = std::uint16_t(value.asInt64());
                }
            }

            corpseLocationRoot = corpseItemsRoot["Corpses"];
            if (!corpseLocationRoot.isNull() && corpseLocationRoot.isArray() && !corpseLocationRoot.empty())
            {
                corpseItemsRoot.clear();
                corpseLocationRoot = corpseLocationRoot[0];
                if(!corpseLocationRoot.isNull())
                {
                    value = corpseLocationRoot["Header"];
                    if (!value.isNull())
                    {
                        if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)ITEM_MARKER.data()))
                        {
                            // bad header
                            corpse_location = 0;
                            return false;
                        }
                    }

                    // If "Count" value is present, it needs to be validated to be true
                    value = corpseLocationRoot["Count"];
                    if (!value.isNull())
                    {
                        checkItemCount = true;
                        expectedNumOfItems = std::uint16_t(value.asInt64());
                    }

                    corpseItemsRoot = corpseLocationRoot["ItemList"];
                }
            }
        }
    }

    std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
    if (CorpseInfo.IsDead != 1)
    {
        CorpseInfo.clear();
        NumOfCorpseItems = 0;
        CorpseItems.clear();
        corpse_item_location = 0;
    }
    else if (!corpseLocationRoot.isNull())
    {
        Json::Value value = corpseLocationRoot[bSerializedFormat ? "Unk0x0" : "unknown"];
        if (!value.isNull())
        {
            CorpseInfo.Unknown = std::uint32_t(value.asInt64());
        }

        value = corpseLocationRoot[bSerializedFormat ? "X" : "position_x"];
        if (!value.isNull())
        {
            CorpseInfo.X = std::uint32_t(value.asInt64());
        }

        value = corpseLocationRoot[bSerializedFormat ? "Y" : "position_y"];
        if (!value.isNull())
        {
            CorpseInfo.Y = std::uint32_t(value.asInt64());
        }
    }

    std::fwrite(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
    std::fflush(charfile);
    corpse_location = std::ftell(charfile);

    if (CorpseInfo.IsDead > 0)
    {
        std::fwrite(&CorpseInfo.Unknown, sizeof(CorpseInfo.Unknown), 1, charfile);
        std::fwrite(&CorpseInfo.X, sizeof(CorpseInfo.X), 1, charfile);
        std::fwrite(&CorpseInfo.Y, sizeof(CorpseInfo.Y), 1, charfile);

        if (!corpseItemsRoot.isNull())
        {
            if (!readItemsList(corpseItemsRoot, bSerializedFormat, CorpseItems))
            {
                // Corrupt file
                corpse_location = 0;
                return false;
            }

            NumOfCorpseItems = std::uint16_t(CorpseItems.size());
            std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
            std::fwrite(&NumOfCorpseItems, sizeof(NumOfCorpseItems), 1, charfile);
            corpse_item_location = std::ftell(charfile);
            for (auto& item : CorpseItems)
            {
                if (!item.writeItem(charfile))
                {
                    // Corrupt file
                    corpse_item_location = 0;
                    return false;
                }
            }
        }
    }

    if (checkItemCount && (NumOfCorpseItems != expectedNumOfItems))
    {
        // Corrupt file
        corpse_location = 0;
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::readMercItems(std::FILE* charfile)
{
    if (!isFileExpansionCharacter)
    {
        if (Version < EnumCharVersion::v109)
        {
            MercId_v100 = 0;
            if (feof(charfile))
            {
                return;
            }

            std::uint8_t value = 0;
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[0])
            {
                return;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != ITEM_MARKER[1])
            {
                return;
            }

            std::fread(&MercId_v100, 6, 1, charfile);
        }
        return;
    }

    if (update_locations)
    {
        bool bFoundMercMarker = false;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)MIN_START_STATS_POS)
        {
            cur_pos = MIN_START_STATS_POS;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != MERC_ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != MERC_ITEM_MARKER[1])
            {
                continue;
            }

            // found merc Item marker
            bFoundMercMarker = true;
            break;
        }

        if (!bFoundMercMarker)
        {
            return;
        }
    }

    // lock ahead for no merc case
    bool bHasMercId = true;
    if (!feof(charfile))
    {
        auto startLoc = std::ftell(charfile);
        std::uint8_t value = 0;
        std::fread(&value, sizeof(value), 1, charfile);
        if (value != ITEM_MARKER[0])
        {
            bHasMercId = false;
        }

        std::fseek(charfile, startLoc, SEEK_SET);
    }

    if (bHasMercId)
    {
        readItems(charfile, merc_location, NumOfMercItems, MercItems);
        if (merc_location == 0)
        {
            return;
        }
    }

    readGolemItem(charfile);
}
//---------------------------------------------------------------------------
void d2ce::Items::readMercItems(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile)
{
    if (!isFileExpansionCharacter)
    {
        if (Version < EnumCharVersion::v109)
        {
            MercId_v100 = 0;
            Json::Value jsonValue = root[bSerializedFormat ? "MercId" : "merc_id"];
            if (!jsonValue.isNull())
            {
                MercId_v100 = std::uint64_t(std::stoull(jsonValue.asString(), nullptr, 16));
            }

            std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
            std::fwrite(&MercId_v100, 6, 1, charfile);
        }
        return;
    }

    bool checkItemCount = false;
    std::uint16_t expectedNumOfItems = 0;
    Json::Value mercItemsRoot;
    std::uint32_t mercId = 0;
    if (!root.isNull())
    {
        Json::Value mercInfoRoot = root[bSerializedFormat ? "Mercenary" : "header"];
        if (!mercInfoRoot.isNull())
        {
            Json::Value value = mercInfoRoot[bSerializedFormat ? "Id" : "merc_id"];
            if (!value.isNull())
            {
                mercId = bSerializedFormat ? std::uint32_t(value.asInt64()) : std::uint32_t(std::stoul(value.asString(), nullptr, 16));
            }
        }

        mercItemsRoot = root[bSerializedFormat ? "MercenaryItemList" : "merc_items"];
        if (!mercItemsRoot.isNull() && bSerializedFormat)
        {
            // If "Header" value is present, it needs to be valid
            Json::Value value = mercItemsRoot["Header"];
            if (!value.isNull())
            {
                if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)MERC_ITEM_MARKER.data()))
                {
                    mercItemsRoot.clear();
                }
            }

            if (!mercItemsRoot.isNull())
            {
                mercItemsRoot = mercItemsRoot["ItemList"];
                if (!mercItemsRoot.isNull())
                {
                    value = mercItemsRoot["Header"];
                    if (!value.isNull())
                    {
                        if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)ITEM_MARKER.data()))
                        {
                            mercItemsRoot.clear();
                        }
                    }

                    if (!mercItemsRoot.isNull())
                    {
                        // If "Count" value is present, it needs to be validated to be true
                        value = mercItemsRoot["Count"];
                        if (!value.isNull())
                        {
                            checkItemCount = true;
                            expectedNumOfItems = std::uint16_t(value.asInt64());
                        }

                        mercItemsRoot = mercItemsRoot["Items"];
                    }
                }
            }
        }
    }

    std::fwrite(MERC_ITEM_MARKER.data(), MERC_ITEM_MARKER.size(), 1, charfile);
    if (!readItemsList(mercItemsRoot, bSerializedFormat, MercItems))
    {
        // Corrupt file
        merc_location = 0;
        return;
    }

    NumOfMercItems = std::uint16_t(MercItems.size());
    if (mercId != 0 || (NumOfMercItems > 0))
    {
        std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
        std::fwrite(&NumOfMercItems, sizeof(NumOfMercItems), 1, charfile);
        merc_location = std::ftell(charfile);
        for (auto& item : MercItems)
        {
            if (!item.writeItem(charfile))
            {
                // Corrupt file
                merc_location = 0;
                return;
            }
        }
    }

    readGolemItem(root, bSerializedFormat, charfile);
}
//---------------------------------------------------------------------------
void d2ce::Items::readGolemItem(std::FILE* charfile)
{
    HasGolem = 0;
    GolemItem.clear();
    if (update_locations)
    {
        // find items location
        golem_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)items_location)
        {
            cur_pos = items_location;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != GOLEM_ITEM_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != GOLEM_ITEM_MARKER[1])
            {
                continue;
            }

            // found golem item marker (0x464B). 
            std::fread(&HasGolem, sizeof(HasGolem), 1, charfile);
            golem_location = std::ftell(charfile);
            break;
        }

        if (golem_location == 0)
        {
            return;
        }
    }
    else
    {
        if (golem_location == 0)
        {
            return;
        }

        std::fseek(charfile, golem_location - sizeof(HasGolem), SEEK_SET);
        std::fread(&HasGolem, sizeof(HasGolem), 1, charfile);
    }

    if (HasGolem != 0)
    {
        if (!GolemItem.readItem(Version, charfile))
        {
            // Corrupt file
            if (update_locations)
            {
                golem_location = 0;
            }

            GolemItem.clear();
            HasGolem = false;
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::Items::readGolemItem(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile)
{
    golem_location = 0;
    HasGolem = 0;
    GolemItem.clear();

    Json::Value golemItemRoot;
    if (root.isNull())
    {
        golemItemRoot = root[bSerializedFormat ? "Golem" : "golem_item"];
        if (!golemItemRoot.isNull() && bSerializedFormat)
        {
            // If "Header" value is present, it needs to be valid
            Json::Value value = golemItemRoot["Header"];
            if (!value.isNull())
            {
                if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)GOLEM_ITEM_MARKER.data()))
                {
                    golemItemRoot.clear();
                }
            }

            if (!golemItemRoot.isNull())
            {
                value = golemItemRoot["Exists"];
                if (!value.isNull())
                {
                    HasGolem = value.asBool() ? 1 : 0;
                    if (HasGolem)
                    {
                        golemItemRoot = golemItemRoot["Item"];
                    }
                    else
                    {
                        golemItemRoot.clear();
                    }
                }
                else
                {
                    golemItemRoot = golemItemRoot["Item"];
                }
            }
        }
    }

    if (!GolemItem.readItem(golemItemRoot, bSerializedFormat, Version))
    {
        // Corrupt file
        golem_location = 0;
        GolemItem.clear();
        HasGolem = false;
    }

    std::fwrite(GOLEM_ITEM_MARKER.data(), GOLEM_ITEM_MARKER.size(), 1, charfile);
    HasGolem = GolemItem.size() > 0 ? 1 : 0;
    std::fwrite(&HasGolem, sizeof(HasGolem), 1, charfile);
    golem_location = std::ftell(charfile);
    if (!GolemItem.writeItem(charfile))
    {
        golem_location = 0;
        return;
    }
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeCorpseItems(std::FILE* charfile)
{
    std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
    if (CorpseInfo.IsDead != 1 || CorpseItems.empty())
    {
        CorpseInfo.clear();
        NumOfCorpseItems = 0;
        CorpseItems.clear();
        corpse_item_location = 0;
    }

    std::fwrite(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
    std::fflush(charfile);
    corpse_location = std::ftell(charfile);

    if (CorpseInfo.IsDead > 0)
    {
        std::fwrite(&CorpseInfo.Unknown, sizeof(CorpseInfo.Unknown), 1, charfile);
        std::fwrite(&CorpseInfo.X, sizeof(CorpseInfo.X), 1, charfile);
        std::fwrite(&CorpseInfo.Y, sizeof(CorpseInfo.Y), 1, charfile);

        std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
        NumOfCorpseItems = (std::uint16_t)CorpseItems.size();
        std::fwrite(&NumOfCorpseItems, sizeof(NumOfCorpseItems), 1, charfile);
        corpse_item_location = std::ftell(charfile);
        for (auto& item : CorpseItems)
        {
            if (!item.writeItem(charfile))
            {
                return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeMercItems(std::FILE* charfile)
{
    if (!isFileExpansionCharacter)
    {
        if (Version < EnumCharVersion::v109)
        {
            std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
            std::fwrite(&MercId_v100, 6, 1, charfile);
        }
        return true;
    }

    std::fwrite(MERC_ITEM_MARKER.data(), MERC_ITEM_MARKER.size(), 1, charfile);
    NumOfMercItems = (std::uint16_t)MercItems.size();
    if (merc_location != 0)
    {
        std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
        std::fwrite(&NumOfMercItems, sizeof(NumOfMercItems), 1, charfile);
        merc_location = std::ftell(charfile);
        for (auto& item : MercItems)
        {
            if (!item.writeItem(charfile))
            {
                return false;
            }
        }
    }

    return writeGolemItem(charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeGolemItem(std::FILE* charfile)
{
    std::fwrite(GOLEM_ITEM_MARKER.data(), GOLEM_ITEM_MARKER.size(), 1, charfile);
    HasGolem = GolemItem.size() > 0 ? 1 : 0;
    std::fwrite(&HasGolem, sizeof(HasGolem), 1, charfile);
    golem_location = std::ftell(charfile);
    if (!GolemItem.writeItem(charfile))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(EnumCharVersion version, std::FILE* charfile, bool isExpansion)
{
    Version = version;
    update_locations = items_location == 0 ? true : false;
    isFileExpansionCharacter = isExpansion;
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    if (!readItems(charfile, items_location, NumOfItems, Inventory) || items_location == 0)
    {
        return false;
    }

    if (!readCorpseItems(charfile) || corpse_location == 0)
    {
        return false;
    }

    readMercItems(charfile);

    update_locations = false;
    findItems();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(const Json::Value& root, bool bSerializedFormat, EnumCharVersion version, std::FILE* charfile, bool isExpansion)
{
    clear();
    Version = version;
    update_locations = true;
    isFileExpansionCharacter = isExpansion;
    if (!readItems(root, bSerializedFormat, charfile, items_location, NumOfItems, Inventory) || items_location == 0)
    {
        return false;
    }

    if (!readCorpseItems(root, bSerializedFormat, charfile) || corpse_location == 0)
    {
        return false;
    }

    readMercItems(root, bSerializedFormat, charfile);

    update_locations = false;
    findItems();
    return true;
}
//---------------------------------------------------------------------------
// write items in place at offset saved from reasding
bool d2ce::Items::writeItems(std::FILE* charfile, bool isExpansion)
{
    isFileExpansionCharacter = isExpansion;

    // Write Items
    std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
    NumOfItems = (std::uint16_t)Inventory.size();
    std::fwrite(&NumOfItems, sizeof(NumOfItems), 1, charfile);
    for (auto& item : Inventory)
    {
        if (!item.writeItem(charfile))
        {
            return false;
        }
    }

    // Write Corpse Items
    if (!writeCorpseItems(charfile))
    {
        return false;
    }

    if (!writeMercItems(charfile))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::itemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat) const
{
    if (bSerializedFormat)
    {
        Json::Value playerItemList;
        playerItemList["Header"] = *((std::uint16_t*)ITEM_MARKER.data());
        playerItemList["Count"] = Inventory.size();

        Json::Value items(Json::arrayValue);
        for (auto& item : Inventory)
        {
            item.asJson(items, charLevel, bSerializedFormat);
        }
        playerItemList["Items"] = items;
        parent["PlayerItemList"] = playerItemList;
    }
    else
    {
        Json::Value items(Json::arrayValue);
        for (auto& item : Inventory)
        {
            item.asJson(items, charLevel, bSerializedFormat);
        }
        parent["items"] = items;
    }
}
//---------------------------------------------------------------------------
void d2ce::Items::corpseItemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat) const
{
    if (bSerializedFormat)
    {
        Json::Value playerCorpses;
        playerCorpses["Header"] = *((std::uint16_t*)ITEM_MARKER.data());
        playerCorpses["Count"] = (CorpseInfo.IsDead ? 1 : 0);

        Json::Value corpses(Json::arrayValue);
        if (!CorpseItems.empty())
        {
            Json::Value corpse;
            corpse["Unk0x0"] = CorpseInfo.Unknown;
            corpse["X"] = CorpseInfo.X;
            corpse["Y"] = CorpseInfo.Y;
            corpse["Header"] = *((std::uint16_t*)ITEM_MARKER.data());
            corpse["Count"] = CorpseItems.size();

            Json::Value items(Json::arrayValue);
            for (auto& item : CorpseItems)
            {
                item.asJson(items, charLevel, bSerializedFormat);
            }
            corpse["ItemList"] = items;
            corpses.append(corpse);
        }
        playerCorpses["Corpses"] = corpses;
        parent["PlayerCorpses"] = playerCorpses;
    }
    else
    {
        Json::Value corpses(Json::arrayValue);
        for (auto& item : CorpseItems)
        {
            item.asJson(corpses, charLevel, bSerializedFormat);
        }
        parent["corpse_items"] = corpses;

        CorpseInfo.asJson(parent);
    }
}
//---------------------------------------------------------------------------
bool d2ce::Items::mercItemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat) const
{
    if (!isFileExpansionCharacter)
    {
        if ((Version < EnumCharVersion::v109) && MercId_v100 > 0)
        {
            std::stringstream ss;
            ss << std::hex << MercId_v100;
            parent[bSerializedFormat ? "MercId" : "merc_id"] = ss.str();
        }

        return true;
    }

    if (bSerializedFormat)
    {
        Json::Value mercenaryItemList;
        mercenaryItemList["Header"] = *((std::uint16_t*)MERC_ITEM_MARKER.data());
        if (!MercItems.empty())
        {
            Json::Value itemList;
            itemList["Header"] = *((std::uint16_t*)ITEM_MARKER.data());
            itemList["Count"] = MercItems.size();

            Json::Value items(Json::arrayValue);
            for (auto& item : MercItems)
            {
                item.asJson(items, charLevel, bSerializedFormat);
            }
            itemList["Items"] = items;
            mercenaryItemList["ItemList"] = itemList;
        }
        parent["MercenaryItemList"] = mercenaryItemList;
    }
    else
    {
        Json::Value items(Json::arrayValue);
        for (auto& item : MercItems)
        {
            item.asJson(items, charLevel, bSerializedFormat);
        }
        parent["merc_items"] = items;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::golemItemAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat) const
{
    if (!isFileExpansionCharacter)
    {
        return false;
    }

    if (bSerializedFormat)
    {
        Json::Value golem;
        golem["Header"] = *((std::uint16_t*)GOLEM_ITEM_MARKER.data());
        golem["Exists"] = (HasGolem ? true : false);
        if (HasGolem != 0)
        {
            Json::Value item;
            GolemItem.asJson(item, charLevel, bSerializedFormat);
            golem["Item"] = item;
        }
        parent["Golem"] = golem;
    }
    else
    {
        if (!HasGolem)
        {
            return false;
        }

        Json::Value item;
        GolemItem.asJson(item, charLevel, bSerializedFormat);
        parent["golem_item"] = item;
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::itemBonusesAsJson(Json::Value& parent, bool bSerializedFormat) const
{
    if (bSerializedFormat)
    {
        return false;
    }

    std::vector<MagicalAttribute> attribs;
    if (!getItemBonuses(attribs))
    {
        return false;
    }

    Json::Value itemBonuses(Json::arrayValue);
    for (auto& attrib : attribs)
    {
        attrib.asJson(itemBonuses, bSerializedFormat);
    }
    parent["item_bonuses"] = itemBonuses;
    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::asJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat) const
{
    itemsAsJson(parent, charLevel, bSerializedFormat);
    corpseItemsAsJson(parent, charLevel, bSerializedFormat);
    mercItemsAsJson(parent, charLevel, bSerializedFormat);
    golemItemAsJson(parent, charLevel, bSerializedFormat);
    itemBonusesAsJson(parent, bSerializedFormat);
}
//---------------------------------------------------------------------------
d2ce::Items::Items()
{
}
//---------------------------------------------------------------------------
d2ce::Items::Items(const Items& other)
{
    *this = other;
}
//---------------------------------------------------------------------------
d2ce::Items::~Items()
{
}
//---------------------------------------------------------------------------
d2ce::Items& d2ce::Items::operator=(const Items& other)
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    Version = other.Version;

    items_location = other.items_location;
    corpse_location = other.corpse_location;
    corpse_item_location = other.corpse_item_location;
    merc_location = other.merc_location;
    golem_location = other.golem_location;

    NumOfItems = other.NumOfItems;
    Inventory = other.Inventory;

    CorpseInfo = other.CorpseInfo;
    NumOfCorpseItems = other.NumOfCorpseItems;
    CorpseItems = other.CorpseItems;

    NumOfMercItems = other.NumOfMercItems;
    MercItems = other.MercItems;

    HasGolem = other.HasGolem;
    GolemItem = other.GolemItem;

    update_locations = other.update_locations;
    isFileExpansionCharacter = other.isFileExpansionCharacter;

    // refetch references to items
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    findItems();
    return *this;
}
d2ce::Items& d2ce::Items::operator=(Items&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    Version = std::exchange(other.Version, APP_CHAR_VERSION);

    items_location = std::exchange(other.items_location, 0);
    corpse_location = std::exchange(other.corpse_location, 0);
    corpse_item_location = std::exchange(other.corpse_item_location, 0);
    merc_location = std::exchange(other.merc_location, 0);
    golem_location = std::exchange(other.golem_location, 0);

    // copy reference to items
    GPSs.swap(other.GPSs);
    other.Inventory.clear();
    Stackables.swap(other.Stackables);
    other.Stackables.clear();
    Armor.swap(other.Armor);
    other.Armor.clear();
    Weapons.swap(other.Weapons);
    other.Weapons.clear();

    NumOfItems = std::exchange(other.NumOfItems, std::uint16_t(0));
    Inventory.swap(other.Inventory);
    other.Inventory.clear();

    CorpseInfo.swap(other.CorpseInfo);
    NumOfCorpseItems = std::exchange(other.NumOfCorpseItems, std::uint16_t(0));
    CorpseItems.swap(other.CorpseItems);
    other.CorpseItems.clear();

    NumOfMercItems = std::exchange(other.NumOfMercItems, std::uint16_t(0));
    MercItems.swap(other.MercItems);
    other.MercItems.clear();

    HasGolem = std::exchange(other.HasGolem, std::uint8_t(0));
    GolemItem.swap(other.GolemItem);
    other.GolemItem.clear();

    update_locations = std::exchange(other.update_locations, true);
    isFileExpansionCharacter = std::exchange(other.isFileExpansionCharacter, false);
    return *this;
}
//---------------------------------------------------------------------------
void d2ce::Items::swap(Items& other)
{
    std::swap(*this, other);
}
//---------------------------------------------------------------------------
void d2ce::Items::clear()
{
    *this = Items();
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-perfect gems.
   Returns false if no gems or all gems are perfect.
*/
bool d2ce::Items::anyUpgradableGems() const
{
    if (GPSs.empty())
    {
        return 0;
    }

    for (auto& item : GPSs)
    {
        if (item.get().isUpgradableGem())
        {
            return true;
        }
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-Super Health/Mana potions or
                non-Full Rejuvenation potions.
   Returns false if no potions or all potions are the top kind.
*/
bool d2ce::Items::anyUpgradablePotions() const
{
    if (GPSs.empty())
    {
        return false;
    }

    for (auto& item : GPSs)
    {
        if (item.get().isUpgradablePotion())
        {
            return true;
        }
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns true if there are any non-Full Rejuvenation potions.
   Returns false if no potions or all potions Full Rejuvenation potions.
*/
bool d2ce::Items::anyUpgradableRejuvenations() const
{
    if (GPSs.empty())
    {
        return false;
    }

    for (auto& item : GPSs)
    {
        if (item.get().isUpgradableToFullRejuvenationPotion())
        {
            return true;
        }
    }  // end for

    return false;
}
//---------------------------------------------------------------------------
/*
   Returns the number of items in character's inventory.
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Items::getNumberOfItems() const
{
    return Inventory.size();
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfEquippedItems() const
{
    return getEquippedItems().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getEquippedItems() const
{
    return ItemLocationReference[d2ce::EnumItemLocation::EQUIPPED];
}
//---------------------------------------------------------------------------
d2ce::EnumItemVersion d2ce::Items::getDefaultItemVersion()
{
    switch (Version)
    {
    case EnumCharVersion::v100:
        if (!Inventory.empty())
        {
            return Inventory.front().Version(); // v1.00 - v1.06 item
        }

        return EnumItemVersion::v104; // v1.04 - v1.06 item

    case EnumCharVersion::v107:
        return EnumItemVersion::v107; // v1.07 item

    case EnumCharVersion::v108:
        return EnumItemVersion::v108; // v1.08/1.09 normal or expansion

    case EnumCharVersion::v110:
        return EnumItemVersion::v110; // v1.10 normal or expansion

    case EnumCharVersion::v115:
        return EnumItemVersion::v115; // v1.15 Diable II Resurrected
    }

    // should not happen
    return EnumItemVersion::v115;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getItemLocationDimensions(EnumItemLocation locationId, EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const
{
    dimensions.clear();
    switch (locationId)
    {
    case EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case EnumAltItemLocation::INVENTORY:
            // Inventory is a 10 x 4 grid
            dimensions.Width = dimensions.InvWidth = 10;
            dimensions.Height = dimensions.InvHeight = 4;
            return true;

        case EnumAltItemLocation::STASH:
            if ((Version < d2ce::EnumCharVersion::v115) || !isFileExpansionCharacter)
            {
                // STASH is a 6 x 4/8 grid
                dimensions.Width = dimensions.InvWidth = 6;
                dimensions.Height = dimensions.InvHeight = !isFileExpansionCharacter ? 4 : 8;
            }
            else
            {
                // STASH is at most a 10 x 10 grid
                dimensions.Width = dimensions.InvWidth = 10;
                dimensions.Height = dimensions.InvHeight = 10;
            }
            return true;

        case EnumAltItemLocation::HORADRIC_CUBE:
            // HORADRIC CUBE is a 3 x 4 grid
            dimensions.Width = dimensions.InvWidth = 3;
            dimensions.Height = dimensions.InvHeight = 4;
            return true;
        }
        break;

    case EnumItemLocation::BELT:
        dimensions.Width = dimensions.InvWidth = 4;
        dimensions.Height = dimensions.InvHeight = std::max(std::uint16_t(1), std::uint16_t(getMaxNumberOfItemsInBelt() / 4));
        return true;

    case EnumItemLocation::EQUIPPED:
        dimensions.Height = dimensions.InvHeight = 1;
        dimensions.Width = dimensions.InvWidth = static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_HAND_LEFT);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getItemLocationDimensions(EnumItemLocation locationId, ItemDimensions& dimensions) const
{
    dimensions.clear();
    if (locationId == EnumItemLocation::STORED)
    {
        return false;
    }

    return getItemLocationDimensions(locationId, EnumAltItemLocation::UNKNOWN, dimensions);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getItemLocationDimensions(EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const
{
    dimensions.clear();
    return getItemLocationDimensions(EnumItemLocation::STORED, altPositionId, dimensions);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getHasBeltEquipped() const
{
    return HasBeltEquipped;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getMaxNumberOfItemsInBelt() const
{
    if (!getHasBeltEquipped())
    {
        return 4;
    }

    return EquippedBeltSlots;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInBelt() const
{
    return getItemsInBelt().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInBelt() const
{
    return ItemLocationReference[d2ce::EnumItemLocation::BELT];
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInInventory() const
{
    return getItemsInInventory().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInInventory() const
{
    return ItemAltLocationReference[d2ce::EnumAltItemLocation::INVENTORY];
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInStash() const
{
    return getItemsInStash().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInStash() const
{
    return ItemAltLocationReference[d2ce::EnumAltItemLocation::STASH];
}
//---------------------------------------------------------------------------
bool d2ce::Items::getHasHoradricCube() const
{
    return HasHoradricCube;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInHoradricCube() const
{
    if (!getHasHoradricCube())
    {
        return 0;
    }

    return getItemsInHoradricCube().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInHoradricCube() const
{
    return ItemAltLocationReference[d2ce::EnumAltItemLocation::HORADRIC_CUBE];
}
//---------------------------------------------------------------------------
// number of bytes to store all item sections
size_t d2ce::Items::getByteSize() const
{
    size_t byteSize = GolemItem.getFullSize();
    for (auto& item : Inventory)
    {
        byteSize += item.getFullSize();
    }

    for (auto& item : CorpseItems)
    {
        byteSize += item.getFullSize();
    }

    for (auto& item : MercItems)
    {
        byteSize += item.getFullSize();
    }

    return byteSize;
}
//---------------------------------------------------------------------------
/*
   Returns the number of Gems, Potions or Skulls in character's inventory.
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Items::getNumberOfGPSs() const
{
    return GPSs.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getGPSs() const
{
    return GPSs;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfStackables() const
{
    return Stackables.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getStackables() const
{
    return Stackables;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfArmor() const
{
    return Armor.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getArmor() const
{
    return Armor;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfWeapons() const
{
    return Weapons.size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getWeapons() const
{
    return Weapons;
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& d2ce::Items::getMercItems() const
{
    return MercItems;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getMercItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();

    // Combine all equipped magical properties
    std::vector<MagicalAttribute> tempAttribs;
    std::multimap<size_t, size_t> itemIndexMap;
    for (auto& item : MercItems)
    {
        if (item.getCombinedMagicalAttributes(tempAttribs))
        {
            ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getDisplayedMercItemBonuses(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getMercItemBonuses(attribs))
    {
        return false;
    }

    // check for the "all" cases
    ItemHelpers::checkForRelatedMagicalAttributes(attribs);

    bool bFormatted = false;
    for (auto& attrib : attribs)
    {
        bFormatted |= ItemHelpers::formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.begin(), attribs.end(), ItemHelpers::magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Items::getCombinedMercDefenseRating(std::uint32_t charLevel) const
{
    // Combine all equipped defense ratings
    EnumItemLocation location_id = EnumItemLocation::STORED;
    std::uint64_t eDef = 0;
    std::uint64_t defenseRating = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : MercItems)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            defenseRating += item.getDefenseRating();
            if (item.getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
            {
                for (auto& attrib : magicalAttributes)
                {
                    switch (attrib.Id)
                    {
                    case 16:
                    case 215:
                        eDef += Items::getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 31:
                    case 214:
                        defenseRating += Items::getMagicalAttributeValue(attrib, charLevel, 0);
                        break;
                    }
                }
            }
        }
    }

    defenseRating += (defenseRating * eDef) / 100;
    return std::uint16_t(defenseRating);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getCombinedMercDamage(BaseDamage& damage, std::uint32_t charLevel) const
{
    ItemDamage itemDamage;
    damage.clear();

    EnumItemLocation location_id = EnumItemLocation::STORED;
    std::uint64_t eDmg = 0;
    std::uint64_t eDmgMax = 0;
    std::uint64_t dmgMin = 0;
    std::uint64_t dmgMax = 0;
    std::uint64_t dmgMin2 = 0;
    std::uint64_t dmgMax2 = 0;
    std::uint64_t dmgMinThrow = 0;
    std::uint64_t dmgMaxThrow = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : MercItems)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            if (item.getDamage(itemDamage))
            {
                if (item.isEthereal())
                {
                    itemDamage.OneHanded.Min += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Min) * 50) / 100);
                    itemDamage.OneHanded.Max += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Max) * 50) / 100);
                }
                damage.add(itemDamage.OneHanded);
            }

            if (item.getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
            {
                for (auto& attrib : magicalAttributes)
                {
                    switch (attrib.Id)
                    {
                    case 17:
                        eDmg += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 21:
                        dmgMin += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 22:
                        dmgMax += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 23:
                        dmgMin2 += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMin
                        break;

                    case 24:
                        dmgMax2 += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMax
                        break;

                    case 219:
                        eDmgMax += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 159:
                        dmgMinThrow += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMin
                        break;

                    case 160:
                        dmgMaxThrow += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMax
                        break;
                    }
                }

                if (dmgMin == 0)
                {
                    dmgMin = std::max(dmgMinThrow, dmgMin2);
                }

                if (dmgMax == 0)
                {
                    dmgMax = std::max(dmgMaxThrow, dmgMax2);
                }

                if (dmgMin != 0)
                {
                    damage.Min += std::uint16_t(((dmgMin * eDmg) / 100) + dmgMin);
                }

                if (dmgMax != 0)
                {
                    damage.Max += std::uint16_t(((dmgMax * (eDmg + eDmgMax)) / 100) + dmgMax);
                }

                if (damage.Min > damage.Max)
                {
                    damage.Max = damage.Min + 1;
                }
            }
        }
    }

    return damage.Max != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Items::hasCorpse() const
{
    return CorpseInfo.IsDead == 1 ? true : false;
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& d2ce::Items::getCorpseItems() const
{
    return CorpseItems;
}
//---------------------------------------------------------------------------
bool d2ce::Items::hasGolem() const
{
    return HasGolem != 0 ? true : false;
}
//---------------------------------------------------------------------------
const d2ce::Item& d2ce::Items::getGolemItem() const
{
    return GolemItem;
}
//---------------------------------------------------------------------------
/*
   Converts the all gems to their perfect state
   Returns the number of gems converted.
*/
size_t d2ce::Items::upgradeGems()
{
    if (GPSs.empty())
    {
        return 0;
    }

    size_t gemsconverted = 0;

    for (auto& item : GPSs)
    {
        if (item.get().upgradeGem())
        {
            ++gemsconverted;
        }
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to their highest quality.
   Returns the number of potions converted.
*/
size_t d2ce::Items::upgradePotions()
{
    if (GPSs.empty())
    {
        return 0;
    }

    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        if (item.get().upgradePotion())
        {
            ++gemsconverted;
        }
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to Full Rejuvenation potions.
   Returns the number of potions converted.
*/
size_t d2ce::Items::upgradeRejuvenationPotions()
{
    if (GPSs.empty())
    {
        return 0;
    }

    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        if (item.get().upgradeToFullRejuvenationPotion())
        {
            ++gemsconverted;
        }
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems, potions or skulls to the specified
   final gem, potion or skull.
   Returns the number of gems converted.
*/
size_t d2ce::Items::convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem)
{
    if (GPSs.empty())
    {
        return 0;
    }

    const std::uint8_t& oldgem = existingGem[0];
    const std::uint8_t& oldgemcondition = existingGem[1];
    const std::uint8_t& oldgemcolour = existingGem[2];

    std::array<std::uint8_t, 4> temp = { 0, 0, 0, 0 };
    std::uint8_t& currentgem = temp[0];
    std::uint8_t& currentgemcondition = temp[1];
    std::uint8_t& currentgemcolour = temp[2];

    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        item.get().getItemCode(temp);
        if (oldgem == currentgem && oldgemcondition == currentgemcondition && oldgemcolour == currentgemcolour)
        {
            // found a match, try to update it
            if (item.get().updateGem(desiredGem))
            {
                ++gemsconverted;
            }
        }
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::fillAllStackables()
{
    size_t itemsFilled = 0;
    for (auto& item : Stackables)
    {
        if (item.get().setMaxQuantity())
        {
            ++itemsFilled;
        }
    }  // end for

    return itemsFilled;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::fixAllItems()
{
    size_t itemsFixed = 0;
    for (auto& item : Armor)
    {
        if (item.get().fixDurability())
        {
            ++itemsFixed;
        }
    }

    for (auto& item : Weapons)
    {
        if (item.get().fixDurability())
        {
            ++itemsFixed;
        }
    }

    for (auto& item : MercItems)
    {
        if (item.fixDurability())
        {
            ++itemsFixed;
        }
    }

    if (HasGolem)
    {
        if (GolemItem.fixDurability())
        {
            ++itemsFixed;
        }
    }

    return itemsFixed;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::maxDurabilityAllItems()
{
    size_t itemsFixed = 0;
    for (auto& item : Armor)
    {
        if (item.get().setMaxDurability())
        {
            ++itemsFixed;
        }
    }

    for (auto& item : Weapons)
    {
        if (item.get().setMaxDurability())
        {
            ++itemsFixed;
        }
    }

    for (auto& item : MercItems)
    {
        if (item.setMaxDurability())
        {
            ++itemsFixed;
        }
    }

    if (HasGolem)
    {
        if (GolemItem.setMaxDurability())
        {
            ++itemsFixed;
        }
    }

    return itemsFixed;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::maxSocketCountAllItems()
{
    size_t itemsChanged = 0;
    for (auto& item : Armor)
    {
        if (item.get().addMaxSocketCount())
        {
            ++itemsChanged;
        }
    }

    for (auto& item : Weapons)
    {
        if (item.get().addMaxSocketCount())
        {
            ++itemsChanged;
        }
    }

    for (auto& item : MercItems)
    {
        if (item.addMaxSocketCount())
        {
            ++itemsChanged;
        }
    }

    return itemsChanged;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::setIndestructibleAllItems()
{
    size_t itemsChanged = 0;
    for (auto& item : Armor)
    {
        if (item.get().setIndestructible())
        {
            ++itemsChanged;
        }
    }

    for (auto& item : Weapons)
    {
        if (item.get().setIndestructible())
        {
            ++itemsChanged;
        }
    }

    for (auto& item : MercItems)
    {
        if (item.setIndestructible())
        {
            ++itemsChanged;
        }
    }

    return itemsChanged;
}
//---------------------------------------------------------------------------
bool d2ce::Items::addItem(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    Item newItem(Version, getDefaultItemVersion(), strcode, isFileExpansionCharacter);
    if (newItem.data.empty())
    {
        // invalid item
        return false;
    }

    ItemDimensions invDimensions;
    if (!getItemLocationDimensions(locationId, altPositionId, invDimensions))
    {
        return false;
    }

    ItemDimensions dimensions;
    if (!newItem.getDimensions(dimensions))
    {
        return false;
    }

    std::set<std::uint16_t>* pEmptySlots = nullptr;
    std::vector<std::reference_wrapper<Item>>* pLocationReference = nullptr;
    switch (locationId)
    {
    case EnumItemLocation::STORED:
        pEmptySlots = &ItemAltLocationEmptySpots[altPositionId];
        pLocationReference = &ItemAltLocationReference[altPositionId];
        break;

    case EnumItemLocation::BELT:
        pEmptySlots = &ItemLocationEmptySpots[locationId];
        pLocationReference = &ItemLocationReference[locationId];
        break;

    case EnumItemLocation::EQUIPPED:
        pEmptySlots = &ItemLocationEmptySpots[locationId];
        pLocationReference = &ItemLocationReference[locationId];
        break;

    default:
        return false;
    }

    if (pEmptySlots == nullptr || pEmptySlots->empty() || pLocationReference == nullptr)
    {
        // now free space
        return false;
    }

    // make sure we have free slots
    std::uint16_t positionY = 0;
    std::uint16_t positionX = 0;
    std::uint16_t freeSlot = MAXUINT16;
    for (const auto& slot : *pEmptySlots)
    {
        positionY = slot / invDimensions.InvWidth;
        positionX = slot % invDimensions.InvWidth;
        bool isGoodLocation = true;
        for (std::uint16_t y = positionY; y < positionY + dimensions.Height && isGoodLocation; ++y)
        {
            for (std::uint16_t x = positionX; x < positionX + dimensions.Width && isGoodLocation; ++x)
            {
                if (pEmptySlots->find(y * invDimensions.InvWidth + x) == pEmptySlots->end())
                {
                    isGoodLocation = false;
                }
            }
        }

        if (isGoodLocation)
        {
            freeSlot = slot;
            break;
        }
    }

    if (freeSlot == MAXUINT16)
    {
        // no room
        return false;
    }

    if (!newItem.setItemLocation(locationId, altPositionId, positionX, positionY))
    {
        // failed to place item
        return false;
    }

    Inventory.push_back(newItem);
    auto& item = const_cast<d2ce::Item&>(Inventory.back());
    pLocationReference->push_back(item);

    // remove slots now occupied
    for (std::uint16_t y = positionY; y < positionY + dimensions.Height; ++y)
    {
        for (std::uint16_t x = positionX; x < positionX + dimensions.Width; ++x)
        {
            pEmptySlots->erase(y * invDimensions.InvWidth + x);
        }
    }

    // add item to appropriate containers
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (itemType.isStackable())
    {
        Stackables.push_back(item);

        if (itemType.isWeapon())
        {
            Weapons.push_back(item);
        }
    }
    else if (itemType.isWeapon())
    {
        Weapons.push_back(item);
    }
    else if (itemType.isArmor())
    {
        Armor.push_back(item);
    }
    else if (itemType.isPotion() || itemType.isGem() || itemType.isRune())
    {
        GPSs.push_back(item);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::addItem(EnumItemLocation locationId, std::array<std::uint8_t, 4>& strcode)
{
    if (locationId == EnumItemLocation::STORED)
    {
        return false;
    }

    return addItem(locationId, EnumAltItemLocation::UNKNOWN, strcode);
}
//---------------------------------------------------------------------------
bool d2ce::Items::addItem(EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    return addItem(EnumItemLocation::STORED, altPositionId, strcode);
}
//---------------------------------------------------------------------------
size_t d2ce::Items::fillEmptySlots(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    size_t numAdded = 0;
    while (addItem(locationId, altPositionId, strcode))
    {
        ++numAdded;
    }
    
    return numAdded;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::fillEmptySlots(EnumItemLocation locationId, std::array<std::uint8_t, 4>& strcode)
{
    if (locationId == EnumItemLocation::STORED)
    {
        return 0;
    }

    return fillEmptySlots(locationId, EnumAltItemLocation::UNKNOWN, strcode);
}
//---------------------------------------------------------------------------
size_t d2ce::Items::fillEmptySlots(EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    return fillEmptySlots(EnumItemLocation::STORED, altPositionId, strcode);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();

    // Combine all equipped magical properties
    std::vector<MagicalAttribute> tempAttribs;
    std::multimap<size_t, size_t> itemIndexMap;
    EnumItemLocation location_id = EnumItemLocation::STORED;
    for (auto& item : Inventory)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            if (item.getCombinedMagicalAttributes(tempAttribs))
            {
                ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getCharmBonuses(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();

    // Combine all charm magical properties
    std::vector<MagicalAttribute> tempAttribs;
    std::multimap<size_t, size_t> itemIndexMap;
    EnumItemLocation location_id = EnumItemLocation::STORED;
    EnumAltItemLocation altLocation_id = EnumAltItemLocation::UNKNOWN;
    for (auto& item : Inventory)
    {
        if (!item.isCharm())
        {
            continue;
        }

        location_id = item.getLocation();
        if (location_id != EnumItemLocation::STORED)
        {
            continue;
        }

        altLocation_id = item.getAltPositionId();
        if (altLocation_id == EnumAltItemLocation::INVENTORY)
        {
            if (item.getCombinedMagicalAttributes(tempAttribs))
            {
                ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getItemBonuses(attribs))
    {
        return false;
    }

    // check for the "all" cases
    ItemHelpers::checkForRelatedMagicalAttributes(attribs);

    bool bFormatted = false;
    for (auto& attrib : attribs)
    {
        bFormatted |= ItemHelpers::formatDisplayedMagicalAttribute(attrib, charLevel);
    }

    // Sort display items in proper order
    std::sort(attribs.begin(), attribs.end(), ItemHelpers::magicalAttributeSorter);
    return bFormatted;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Items::getCombinedDefenseRating(std::uint32_t charLevel) const
{
    // Combine all equipped defense ratings
    EnumItemLocation location_id = EnumItemLocation::STORED;
    std::uint64_t eDef = 0;
    std::uint64_t defenseRating = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : Inventory)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            defenseRating += item.getDefenseRating();
            if (item.getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
            {
                for (auto& attrib : magicalAttributes)
                {
                    switch (attrib.Id)
                    {
                    case 16:
                    case 215:
                        eDef += Items::getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 31:
                    case 214:
                        defenseRating += Items::getMagicalAttributeValue(attrib, charLevel, 0);
                        break;
                    }
                }
            }
        }
    }

    defenseRating += (defenseRating * eDef) / 100;
    return std::uint16_t(defenseRating);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getCombinedDamage(BaseDamage& damage, std::uint32_t charLevel) const
{
    ItemDamage itemDamage;
    damage.clear();

    EnumItemLocation location_id = EnumItemLocation::STORED;
    std::uint64_t eDmg = 0;
    std::uint64_t eDmgMax = 0;
    std::uint64_t dmgMin = 0;
    std::uint64_t dmgMax = 0;
    std::uint64_t dmgMin2 = 0;
    std::uint64_t dmgMax2 = 0;
    std::uint64_t dmgMinThrow = 0;
    std::uint64_t dmgMaxThrow = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : Inventory)
    {
        location_id = item.getLocation();
        if (location_id == EnumItemLocation::EQUIPPED)
        {
            if (item.getDamage(itemDamage))
            {
                if (item.isEthereal())
                {
                    itemDamage.OneHanded.Min += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Min) * 50) / 100);
                    itemDamage.OneHanded.Max += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Max) * 50) / 100);
                }
                damage.add(itemDamage.OneHanded);
            }

            if (item.getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
            {
                for (auto& attrib : magicalAttributes)
                {
                    switch (attrib.Id)
                    {
                    case 17:
                        eDmg += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 21:
                        dmgMin += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 22:
                        dmgMax += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 23:
                        dmgMin2 += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMin
                        break;

                    case 24:
                        dmgMax2 += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMax
                        break;

                    case 219:
                        eDmgMax += getMagicalAttributeValue(attrib, charLevel, 0);
                        break;

                    case 159:
                        dmgMinThrow += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMin
                        break;

                    case 160:
                        dmgMaxThrow += getMagicalAttributeValue(attrib, charLevel, 0); // should be the same as dmgMax
                        break;
                    }
                }

                if (dmgMin == 0)
                {
                    dmgMin = std::max(dmgMinThrow, dmgMin2);
                }

                if (dmgMax == 0)
                {
                    dmgMax = std::max(dmgMaxThrow, dmgMax2);
                }

                if (dmgMin != 0)
                {
                    damage.Min += std::uint16_t(((dmgMin * eDmg) / 100) + dmgMin);
                }

                if (dmgMax != 0)
                {
                    damage.Max += std::uint16_t(((dmgMax * (eDmg + eDmgMax)) / 100) + dmgMax);
                }

                if (damage.Min > damage.Max)
                {
                    damage.Max = damage.Min + 1;
                }
            }
        }
    }

    return damage.Max != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::int64_t d2ce::Items::getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx)
{
    const auto& stat = ItemHelpers::getItemStat(attrib.Id);
    return ItemHelpers::getMagicalAttributeValue(attrib, charLevel, idx, stat);
}
//---------------------------------------------------------------------------