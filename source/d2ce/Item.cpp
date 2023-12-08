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

#include "pch.h"
#include <map>
#include <bitset>
#include <sstream>
#include "Item.h"
#include "helpers/ItemHelpers.h"
#include "SkillConstants.h"
#include "Character.h"
#include <utf8/utf8.h>

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint32_t NUM_OF_LEVELS = 99ui32; // max level (game limit)
    constexpr std::uint16_t MAX_DURABILITY = 0xFF;  // max durability of an item (0 is Indestructible)

    constexpr std::array<std::uint8_t, 2> MERC_ITEM_MARKER = { 0x6A, 0x66 };  // alternatively "jf"
    constexpr std::array<std::uint8_t, 2> GOLEM_ITEM_MARKER = { 0x6B, 0x66 }; // alternatively "kf"

    constexpr std::uint32_t MIN_START_STATS_POS = 64ui32;

    constexpr std::uint32_t IS_IDENTIFIED_FLAG_OFFSET = 4ui32;
    constexpr std::uint32_t IS_DISABLED_FLAG_OFFSET = 8ui32;
    constexpr std::uint32_t IS_SOCKETED_FLAG_OFFSET = 11ui32;
    constexpr std::uint32_t IS_NEW_FLAG_OFFSET = 13ui32;
    constexpr std::uint32_t IS_BAD_EQUIPPED_FLAG_OFFSET = 14ui32;
    constexpr std::uint32_t IS_EAR_FLAG_OFFSET = 16ui32;
    constexpr std::uint32_t IS_STARTER_FLAG_OFFSET = 17ui32;
    constexpr std::uint32_t IS_SIMPLE_FLAG_OFFSET = 21ui32;
    constexpr std::uint32_t IS_ETHEREAL_FLAG_OFFSET = 22ui32;
    constexpr std::uint32_t IS_PERSONALIZED_FLAG_OFFSET = 24ui32;
    constexpr std::uint32_t IS_RUNEWORD_FLAG_OFFSET = 26ui32;

    constexpr std::uint32_t ITEM_TYPE_BIT_OFFSET = 19ui32; // for v1.00 to 1.06, these two bits tell us the item version (non-zero means 1.04 - 1.06 items)

    constexpr std::uint32_t QUALITY_BIT_OFFSET = 134ui32;
    constexpr std::uint32_t QUALITY_BIT_OFFSET_100 = 65ui32;
    constexpr std::uint32_t QUALITY_BIT_OFFSET_104 = 97ui32;
    constexpr std::uint32_t QUALITY_NUM_BITS_100 = 3ui32;
    constexpr std::uint16_t QUALITY_NUM_BITS = 4ui16;

    constexpr std::uint32_t QUANTITY_BIT_OFFSET_100 = 69ui32;
    constexpr std::uint32_t QUANTITY_BIT_OFFSET_104 = 101ui32;

    constexpr std::uint16_t SOCKET_COUNT_NUM_BITS = 4ui16;
    constexpr std::uint16_t REAL_DATA_NUM_BITS_110 = 96ui16; // realm data prior to D2R
    constexpr std::uint16_t REAL_DATA_NUM_BITS = 128ui16;
    constexpr std::uint16_t DEFENSE_RATING_NUM_BITS = 11ui16;
    constexpr std::uint16_t DEFENSE_RATING_NUM_BITS_108 = 10ui16; // 1.09 or older
    constexpr std::uint16_t DURABILITY_MAX_NUM_BITS = 8ui16;
    constexpr std::uint16_t DURABILITY_CURRENT_READ_NUM_BITS = 8ui16;
    constexpr std::uint16_t DURABILITY_CURRENT_NUM_BITS = 9ui16;
    constexpr std::uint16_t DURABILITY_CURRENT_NUM_BITS_108 = 8ui16; // 1.09 or older
    constexpr std::uint16_t STACKABLE_NUM_BITS = 9ui16;
    constexpr std::uint16_t GLD_STACKABLE_NUM_BITS = 12ui16;
    constexpr std::uint16_t GLD_STACKABLE_LARGE_NUM_BITS = 32ui16;
    constexpr std::uint16_t RUNEWORD_ID_NUM_BITS = 12ui16;
    constexpr std::uint16_t RUNEWORD_PADDING_NUM_BITS = 4ui16;
    constexpr std::uint16_t MAGICAL_AFFIX_NUM_BITS = 11ui16;
    constexpr std::uint16_t SET_UNIQUE_ID_NUM_BITS = 12ui16;
    constexpr std::uint16_t NORMAL_CHARM_ID_NUM_BITS = 12ui16;
    constexpr std::uint16_t INFERIOR_SUPERIOR_ID_NUM_BITS = 3ui16;
    constexpr std::uint16_t RARE_CRAFTED_ID_NUM_BITS = 8ui16;
    constexpr std::uint16_t PROPERTY_ID_NUM_BITS = 9ui16;
    constexpr std::uint16_t MONSTER_ID_NUM_BITS = 10ui16;

    constexpr std::uint16_t ITEM_V100_NUM_BITS = 216ui16;
    constexpr std::uint16_t ITEM_V100_UNIQUE_ID_NUM_BITS = 8ui16;
    constexpr std::uint16_t ITEM_V104_EAR_NUM_BITS = 208ui16;
    constexpr std::uint16_t ITEM_V104_SM_NUM_BITS = 120ui16;
    constexpr std::uint16_t ITEM_V104_EX_NUM_BITS = 248ui16;

    constexpr std::uint32_t ITEM_V100_COORDINATES_BIT_OFFSET = 120ui32;
    constexpr std::uint32_t ITEM_V100_SPECIALITEMCODE_BIT_OFFSET = 129ui32;
    constexpr std::uint32_t ITEM_V100_EAR_COORDINATES_BIT_OFFSET = 64ui32;
    constexpr std::uint32_t ITEM_V104_SM_COORDINATES_BIT_OFFSET = 64ui32;
    constexpr std::uint32_t ITEM_V104_EX_COORDINATES_BIT_OFFSET = 152ui32;
    constexpr std::uint32_t ITEM_V104_EX_UNIQUECODE_BIT_OFFSET = 161ui32;
    constexpr std::uint32_t ITEM_V104_EAR_COORDINATES_BIT_OFFSET = 64ui32;

    constexpr std::uint32_t ITEM_V100_CONTAINER_BIT_OFFSET = 200ui32;
    constexpr std::uint32_t ITEM_V100_EAR_CONTAINER_BIT_OFFSET = 80ui32;
    constexpr std::uint32_t ITEM_V104_EX_CONTAINER_BIT_OFFSET = 232ui32;

    constexpr std::uint32_t ITEM_V100_ITEMCODE_BIT_OFFSET = 64ui32;
    constexpr std::uint32_t ITEM_V100_EAR_ITEMCODE_BIT_OFFSET = 48ui32;
    constexpr std::uint32_t ITEM_V100_EAR_ATTRIBE_BIT_OFFSET = 74ui32;
    constexpr std::uint32_t ITEM_V100_EAR_LEVEL_BIT_OFFSET = 92ui32;
    constexpr std::uint32_t ITEM_V100_TYPECODE_BIT_OFFSET = 68ui32;
    constexpr std::uint32_t ITEM_V100_DURABILITY_BIT_OFFSET = 105ui32;
    constexpr std::uint32_t ITEM_V100_DWA_BIT_OFFSET = 139ui32;
    constexpr std::uint32_t ITEM_V104_EX_TYPECODE_BIT_OFFSET = 58ui32;
    constexpr std::uint32_t ITEM_V104_SM_ITEMCODE_BIT_OFFSET = 80ui32;
    constexpr std::uint32_t ITEM_V104_SM_TYPECODE_BIT_OFFSET = 82ui32;
    constexpr std::uint32_t ITEM_V104_EAR_ATTRIBE_BIT_OFFSET = 82ui32;
    constexpr std::uint32_t ITEM_V104_EX_DURABILITY_BIT_OFFSET = 137ui32;
    constexpr std::uint32_t ITEM_V104_EX_DWA_BIT_OFFSET = 171ui32;

    constexpr std::uint32_t ITEM_V100_BITFIELD3_BIT_OFFSET = 48ui32;
    constexpr std::uint32_t ITEM_V100_NUM_SOCKETED_BIT_OFFSET = 53ui32;
    constexpr std::uint32_t ITEM_V100_LEVEL_BIT_OFFSET = 56ui32;
    constexpr std::uint32_t ITEM_V104_EX_EQUIP_ID_BIT_OFFSET = 90ui32;
    constexpr std::uint32_t ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET = 95ui32;
    constexpr std::uint32_t ITEM_V104_EX_LEVEL_BIT_OFFSET = 98ui32;

    constexpr std::uint32_t ITEM_V107_EXTENDED_DATA_BIT_OFFSET = 108ui32;
    constexpr std::uint32_t ITEM_V107_START_BIT_OFFSET = 23ui32; // 16 bit for item header + 7 bits for the item version
    constexpr std::uint32_t ITEM_V115_LOCATION_BIT_OFFSET = 35ui32;
    constexpr std::uint32_t ITEM_V115_EQUIPPED_ID_BIT_OFFSET = 38ui32;
    constexpr std::uint32_t ITEM_V115_POSITION_BIT_OFFSET = 42ui32;
    constexpr std::uint32_t ITEM_V115_ALT_POSITION_ID_BIT_OFFSET = 50ui32;
    constexpr std::uint32_t ITEM_V115_TYPE_CODE_BIT_OFFSET = 53ui32;

    constexpr std::uint16_t ATTRIBUTE_ID_INDESTRUCTIBLE = 152ui16;

    enum class ItemOffsets : std::uint32_t
    {
        START_BIT_OFFSET,
        LOCATION_BIT_OFFSET,
        EQUIPPED_ID_OFFSET,
        POSITION_OFFSET,
        ALT_POSITION_ID_OFFSET,
        TYPE_CODE_OFFSET,
        EXTENDED_DATA_OFFSET,
        QUEST_DIFFICULTY_OFFSET,
        NR_OF_ITEMS_IN_SOCKETS_OFFSET,
        ITEM_ID_BIT_OFFSET,
        ITEM_LEVEL_BIT_OFFSET,
        QUALITY_BIT_OFFSET,
        MULTI_GRAPHIC_BIT_OFFSET,
        AUTOAFFIX_BIT_OFFSET,
        QUALITY_ATTRIB_BIT_OFFSET,
        RUNEWORD_ID_BIT_OFFSET,
        PERSONALIZED_BIT_OFFSET,
        TOME_BIT_OFFSET,
        BODY_PART_BIT_OFFSET,
        REALM_BIT_OFFSET,
        DEFENSE_RATING_BIT_OFFSET,
        DURABILITY_BIT_OFFSET,
        STACKABLE_BIT_OFFSET,
        GLD_STACKABLE_BIT_OFFSET,
        SOCKET_COUNT_BIT_OFFSET,
        BONUS_BITS_BIT_OFFSET,
        MAGICAL_PROPS_BIT_OFFSET,
        SET_BONUS_PROPS_BIT_OFFSET,
        RUNEWORD_PROPS_BIT_OFFSET,
        ITEM_END_BIT_OFFSET,
        DWB_BIT_OFFSET,
    };

#define GET_BIT_OFFSET(x) \
    bitOffsets[static_cast<std::underlying_type_t<ItemOffsets>>((x))]

    enum class ItemOffsetMarkers : std::uint32_t
    {
        QUALITY_ATTRIB_BIT_OFFSET_MARKER,
        RUNEWORD_ID_BIT_OFFSET_MARKER, // offset where to put the runeword id
        PERSONALIZED_BIT_OFFSET_MARKER, // offset where to put the personalization
        SOCKET_COUNT_BIT_OFFSET_MARKER,
        BONUS_BITS_BIT_OFFSET_MARKER,
        SET_BONUS_PROPS_BIT_OFFSET_MARKER, // offset where to put the set bonus properties
        RUNEWORD_PROPS_BIT_OFFSET_MARKER, // offset where to put the runeword bonus properties
    };

    std::map<ItemOffsets, ItemOffsetMarkers> g_markerOffsets = {
        {ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET, ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER},
        {ItemOffsets::RUNEWORD_ID_BIT_OFFSET, ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER},
        {ItemOffsets::PERSONALIZED_BIT_OFFSET, ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER},
        {ItemOffsets::SOCKET_COUNT_BIT_OFFSET, ItemOffsetMarkers::SOCKET_COUNT_BIT_OFFSET_MARKER},
        {ItemOffsets::BONUS_BITS_BIT_OFFSET, ItemOffsetMarkers::BONUS_BITS_BIT_OFFSET_MARKER},
        {ItemOffsets::SET_BONUS_PROPS_BIT_OFFSET, ItemOffsetMarkers::SET_BONUS_PROPS_BIT_OFFSET_MARKER},
        {ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET, ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER}
    };

#define GET_BIT_OFFSET_MARKER(x) \
    bitOffsetMarkers[static_cast<std::underlying_type_t<ItemOffsetMarkers>>((x))]

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

    std::uint64_t SafeGetNodeValue(Json::Value& node)
    {
        if (node.isNull())
        {
            return 0;
        }

        return node.asInt64();
    }

    struct ItemPredicate
    {
        ItemPredicate(const d2ce::Item& item) : m_item(item) {}

        bool operator() (const d2ce::Item& item)
        {
            return &item == &m_item ? true : false;
        }

        bool operator() (const std::reference_wrapper<Item>& itemRef)
        {
            return &(itemRef.get()) == &m_item ? true : false;
        }
    private:
        const d2ce::Item& m_item;
    };

    struct ItemEnumEquippedIdPredicate
    {
        ItemEnumEquippedIdPredicate(d2ce::EnumEquippedId equippedId) : m_equippedId(equippedId) {}

        bool operator() (const d2ce::Item& item)
        {
            if (item.getLocation() != EnumItemLocation::EQUIPPED)
            {
                return false;
            }
            return item.getEquippedId() == m_equippedId ? true : false;
        }

        bool operator() (const std::reference_wrapper<Item>& itemRef)
        {
            auto& item = itemRef.get();
            if (item.getLocation() != EnumItemLocation::EQUIPPED)
            {
                return false;
            }

            return item.getEquippedId() == m_equippedId ? true : false;
        }
    private:
        d2ce::EnumEquippedId m_equippedId;
    };

    struct ItemIsBeltablePredicate
    {
        ItemIsBeltablePredicate() {}

        bool operator() (const d2ce::Item& item)
        {
            return item.isBeltable();
        }

        bool operator() (const std::reference_wrapper<Item>& itemRef)
        {
            return itemRef.get().isBeltable();
        }
    };

    struct ItemIsInHoradricCube
    {
        ItemIsInHoradricCube() {}

        bool operator() (const d2ce::Item& item)
        {
            return (item.getLocation() == d2ce::EnumItemLocation::STORED) &&
                (item.getAltPositionId() == d2ce::EnumAltItemLocation::HORADRIC_CUBE);
        }

        bool operator() (const std::reference_wrapper<Item>& itemRef)
        {
            return (itemRef.get().getLocation() == d2ce::EnumItemLocation::STORED) &&
                (itemRef.get().getAltPositionId() == d2ce::EnumAltItemLocation::HORADRIC_CUBE);
        }
    };

    void AddUndeadBonusMagicalAttribute(EnumItemVersion version, std::uint16_t gameVersion, std::vector<MagicalAttribute>& attribs)
    {
        const auto& stat = ItemHelpers::getItemStat(version, "item_undeaddamage_percent");
        bool bHasUndeadAttrib = false;
        for (auto& item : attribs)
        {
            if (item.Id == stat.id)
            {
                bHasUndeadAttrib = true;
                item.Values[0] += 50;
                return;
            }
        }

        MagicalAttribute attrib;
        attrib.Id = stat.id;
        attrib.Desc = stat.desc;
        attrib.Name = stat.name;
        attrib.Version = version;
        attrib.GameVersion = gameVersion;
        attrib.DescPriority = 0;
        attrib.Values = { 50 };
        attribs.push_back(attrib);
    }

    //---------------------------------------------------------------------------
    namespace ItemHelpers
    {
        bool getItemCodev100(std::uint16_t code, std::array<std::uint8_t, 4>& strcode);
        std::uint8_t getResurrectedItemCode(const std::vector<std::uint8_t>& data, size_t startOffset, std::array<std::uint8_t, 4>& strcode);
        void encodeResurrectedItem(const std::array<std::uint8_t, 4>& strcode, std::uint64_t& encodedVal, std::uint8_t& numBitsSet);
        std::uint8_t HuffmanDecodeBitString(const std::string& bitstr);

        const ItemType& getInvalidItemTypeHelper();

        std::string getRunewordNameFromId(std::uint16_t id);
        const d2ce::RunewordType& getRunewordFromId(std::uint16_t id);
        std::vector<d2ce::RunewordType> getPossibleRunewords(const d2ce::Item& item, bool bUseCurrentSocketCount = false, bool bExcludeServerOnly = true);
        std::vector<d2ce::RunewordType> getPossibleRunewords(const d2ce::Item& item, std::uint32_t level, bool bUseCurrentSocketCount = false, bool bExcludeServerOnly = true);
        bool getPossibleMagicalAffixes(const d2ce::Item& item, std::vector<std::uint16_t>& prefixes, std::vector<std::uint16_t>& suffixes);
        bool getPossibleMagicalAffixes(const d2ce::Item& item, std::map<std::uint16_t, std::vector<std::uint16_t>>& prefixes, std::map<std::uint16_t, std::vector<std::uint16_t>>& suffixes);
        bool getPossibleRareAffixes(const d2ce::Item& item, std::vector<std::uint16_t>& prefixes, std::vector<std::uint16_t>& suffixes);
        bool getPossibleSuperiorAttributes(const d2ce::Item& item, std::vector<MagicalAttribute>& attribs);
        bool getPossibleCraftingRecipies(const ItemCreateParams& createParams, std::vector<CraftRecipieType>& attribs);
        bool findDWForMagicalAffixes(const MagicalAffixes& affixes, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t& dwb);
        bool findDWForRareOrCraftedAffixes(const d2ce::RareAttributes& affixes, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t& dwb);

        bool getMagicAttribs(const d2ce::MagicalAffixes& magicalAffixes, std::vector<MagicalAttribute>& attribs, const ItemCreateParams& createParams, bool bMaxAlways = true);
        bool getRareOrCraftedAttribs(const d2ce::RareAttributes& rareAttrib, std::vector<MagicalAttribute>& attribs, const ItemCreateParams& createParams, bool bMaxAlways = true);
        bool getSetMagicAttribs(std::uint16_t id, std::vector<MagicalAttribute>& attribs, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = true);
        bool getSetItemBonusAttribs(std::uint16_t id, std::vector<std::vector<MagicalAttribute>>& attribs, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = true);
        bool getSetBonusAttribs(std::uint16_t id, std::vector<std::vector<MagicalAttribute>>& attribs, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = true);
        bool getFullSetBonusAttribs(std::uint16_t id, std::vector<MagicalAttribute>& attribs, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = true);
        bool getUniqueMagicAttribs(std::uint16_t id, std::vector<MagicalAttribute>& attribs, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = true);
        bool getUniqueQuestMagicAttribs(const std::array<std::uint8_t, 4>& strcode, std::vector<MagicalAttribute>& attribs, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = true);

        std::uint8_t generateInferiorQualityId(std::uint16_t level, std::uint32_t dwb = 0);
        bool generateSuperiorAttributes(std::vector<MagicalAttribute>& attribs, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = false);
        bool generateMagicalAffixes(MagicalCachev100& cache, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = false);
        bool generateRareOrCraftedAffixes(RareOrCraftedCachev100& cache, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = false);
        std::uint16_t generateDefenseRating(const std::array<std::uint8_t, 4>& strcode, std::uint32_t dwa = 0);
        std::uint32_t generateDWARandomOffset(std::uint32_t dwa, std::uint16_t numRndCalls);
        std::uint32_t generarateRandomDW(std::uint32_t itemDwbCode = 0, std::uint16_t level = 0);
        std::string getSetTCFromId(std::uint16_t id);
        const std::string& getRareNameFromId(std::uint16_t id);
        const std::string& getRareIndexFromId(std::uint16_t id);
        const std::string& getMagicalPrefixFromId(std::uint16_t id);
        const std::string& getMagicalSuffixFromId(std::uint16_t id);
        const std::string& getMagicalPrefixTCFromId(std::uint16_t id);
        const std::string& getMagicalSuffixTCFromId(std::uint16_t id);
        bool isAddSocketsMagicalPrefix(std::uint16_t id);
        std::uint16_t getIdFromRareIndex(const std::string& rareIndex);
        std::uint16_t getIdFromRareName(const std::string& rareName);
        std::string getUniqueTCFromId(std::uint16_t id);
        std::uint16_t getMagicalPrefixLevelReqFromId(std::uint16_t id);
        std::uint16_t getMagicalSuffixLevelReqFromId(std::uint16_t id);

        bool magicalAttributeSorter(const MagicalAttribute& left, const MagicalAttribute& right);
        void checkForRelatedMagicalAttributes(std::vector<MagicalAttribute>& attribs);
        std::string formatMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx, const ItemStat& stat);
        bool formatDisplayedMagicalAttribute(MagicalAttribute& attrib, std::uint32_t charLevel);
        void combineMagicalAttribute(std::multimap<size_t, size_t>& itemIndexMap, const std::vector<MagicalAttribute>& newAttribs, std::vector<MagicalAttribute>& attribs);
        void mergeMagicalAttributes(std::vector<MagicalAttribute>& attribs, const std::vector<MagicalAttribute>& newAttribs);
        bool ProcessNameNode(const Json::Value& node, std::array<char, NAME_LENGTH>& name, d2ce::EnumItemVersion version);

        const std::string& getMonsterNameFromId(std::uint16_t id);
    }

    void FilterMagicalAttribsForBonus(const std::vector<MagicalAttribute>& unfilteredAttribs, std::vector<MagicalAttribute>& attribs)
    {
        // filter out the properties that give us a bonuses
        attribs.clear();
        for (auto& attrib : unfilteredAttribs)
        {
            switch (attrib.Id)
            {
            case   0: // strength
            case   1: // energy
            case   2: // dexterity
            case   3: // vitality
            case   7: // maxhp
            case   9: // maxmana
            case  11: // maxstamina
            case  19: // tohit
            case  27: // manarecoverybonus
            case  74: // hpregen
            case 138: // item_manaafterkill
            case  89: // item_lightradius
                attribs.push_back(attrib);
                continue;

            case  78: // item_attackertakesdamage
            case 128: // item_attackertakeslightdamage
                attribs.push_back(attrib);
                continue;

            case  93: // item_fasterattackrate
            case  96: // item_fastermovevelocity
            case  99: // item_fastergethitrate
            case 102: // item_fasterblockrate
            case 105: // item_fastercastrate
                attribs.push_back(attrib);
                continue;

            case  16: // item_armor_percent
            case  31: // armorclass
            case 214: // item_armor_perlevel
            case 215: // item_armorpercent_perlevel
                attribs.push_back(attrib);
                continue;

            case  17: // item_maxdamage_percent
            case  21: // mindamage
            case  22: // maxdamage
            case  23: // secondary_mindamage
            case  24: // secondary_maxdamage
            case  25: // damagepercent
            case  35: // magic_damage_reduction
            case  80: // item_magicbonus
            case 159: // item_throw_mindamage
            case 160: // item_throw_maxdamage
            case 218: // item_maxdamage_perlevel
            case 219: // item_maxdamage_percent_perlevel
                attribs.push_back(attrib);
                continue;

            case  36: // damageresist
            case  37: // magicresist
            case  38: // maxmagicresist
            case  39: // fireresist
            case  40: // maxfireresist
            case  41: // lightresist
            case  42: // maxlightresist
            case  43: // coldresist
            case  44: // maxcoldresist
            case  45: // poisonresist
            case  46: // maxpoisonresist
            case 110: // item_poisonlengthresist
                attribs.push_back(attrib);
                continue;

            case  83: // item_addclassskills
            case  97: // item_nonclassskill
            case 107: // item_singleskill
            case 126: // item_elemskill
            case 127: // item_allskills
                attribs.push_back(attrib);
                continue;

            case 142: // item_absorbfire_percent
            case 143: // item_absorbfire
            case 144: // item_absorblight_percent
            case 145: // item_absorblight
            case 146: // item_absorbmagic_percent
            case 147: // item_absorbmagic
            case 148: // item_absorbcold_percent
            case 149: // item_absorbcold
                attribs.push_back(attrib);
                continue;
            }

            const auto& stat = ItemHelpers::getItemStat(attrib);
            if (stat.opAttribs.op_base == "level")
            {
                for (const auto& opStat : stat.opAttribs.op_stats)
                {
                    if ((opStat == "strength") || (opStat == "energy") || (opStat == "dexterity") || (opStat == "vitality") ||
                        (opStat == "armorclass") || (opStat == "maxdamage") || (opStat == "mindamage") ||
                        (opStat == "item_attackertakeslightdamage") ||
                        (opStat == "maxhp") || (opStat == "maxmana") || (opStat == "maxstamina") ||
                        (opStat == "coldresist") || (opStat == "fireresist") || (opStat == "lightresist") || (opStat == "poisonresist"))
                    {
                        attribs.push_back(attrib);
                        break;
                    }
                }
            }
        }
    }

    std::array<std::uint8_t, 4> ItemCodeStringConverter(const std::string& sValue)
    {
        std::array<std::uint8_t, 4> strcode = { 0x20, 0x20, 0x20, 0x20 };
        if (sValue.size() != 3)
        {
            return strcode;
        }

        std::memcpy(strcode.data(), sValue.c_str(), 3);
        return strcode;
    }

    void CheckOffsetValue(size_t& curBitOffset, size_t& startOffset, bool& bFoundMatch, ptrdiff_t diff)
    {
        if ((curBitOffset == 0) && (&startOffset != &curBitOffset))
        {
            return;
        }

        if (!bFoundMatch && (&startOffset == &curBitOffset))
        {
            bFoundMatch = true;
            return;
        }

        if (curBitOffset == startOffset)
        {
            if (bFoundMatch)
            {
                curBitOffset += diff;
            }
            return;
        }

        if (curBitOffset > startOffset)
        {
            curBitOffset += diff;
            return;
        }
    }

    void CheckMarkerOffsetValue(size_t& curMarkerBitOffset, size_t& curBitOffset, size_t& startOffset, bool& bFoundMatch, ptrdiff_t diff)
    {
        if (bFoundMatch)
        {
            CheckOffsetValue(curMarkerBitOffset, startOffset, bFoundMatch, diff);
            if (curBitOffset > 0)
            {
                curBitOffset = curMarkerBitOffset;
            }
            return;
        }

        if ((&startOffset == &curMarkerBitOffset) || (&startOffset == &curBitOffset))
        {
            bFoundMatch = true;
            return;
        }

        if (curMarkerBitOffset > startOffset)
        {
            curMarkerBitOffset += diff;
            if (curBitOffset > 0)
            {
                curBitOffset = curMarkerBitOffset;
            }
            return;
        }
    }

    EnumCharVersion ConvertItemVersion(EnumItemVersion version)
    {
        auto charVersion = APP_CHAR_VERSION;
        switch (version)
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
        case EnumItemVersion::v104: // v1.04 - v1.06
            charVersion = EnumCharVersion::v100;
            break;

        case EnumItemVersion::v107: // v1.07
            charVersion = EnumCharVersion::v107;
            break;

        case EnumItemVersion::v108: // v1.08
            charVersion = EnumCharVersion::v108;
            break;

        case EnumItemVersion::v109: // v1.09
            charVersion = EnumCharVersion::v109;
            break;

        case EnumItemVersion::v110: // v1.10 - v1.14d
            charVersion = EnumCharVersion::v110;
            break;

        case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected
            charVersion = EnumCharVersion::v100R;
            break;

        case EnumItemVersion::v120: // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
            charVersion = EnumCharVersion::v120;
            break;

        case EnumItemVersion::v140: // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        default:
            charVersion = EnumCharVersion::v140;
            break;
        }

        return charVersion;
    }

    EnumItemVersion ConvertCharVersion(EnumCharVersion version)
    {
        auto itemVersion = APP_ITEM_VERSION;
        switch (version)
        {
        case EnumCharVersion::v100: // v1.00 - v1.06
            itemVersion = EnumItemVersion::v100;
            break;

        case EnumCharVersion::v107: // v1.07
            itemVersion = EnumItemVersion::v107;
            break;

        case EnumCharVersion::v108: // v1.08
            itemVersion = EnumItemVersion::v108;
            break;

        case EnumCharVersion::v109: // v1.09
            itemVersion = EnumItemVersion::v109;
            break;

        case EnumCharVersion::v110: // v1.10 - v1.14d
            itemVersion = EnumItemVersion::v110;
            break;

        case EnumCharVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected
            itemVersion = EnumItemVersion::v100R;
            break;

        case EnumCharVersion::v120: // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
            itemVersion = EnumItemVersion::v120;
            break;

        case EnumCharVersion::v140: // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        default:
            itemVersion = EnumItemVersion::v140;
            break;
        }

        return itemVersion;
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams()
{
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version)
    : itemVersion(version)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::uint16_t gameVer)
    : itemVersion(version), gameVersion(gameVer)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, bool isExp)
    : itemVersion(version), isExpansion(isExp)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type)
    : itemVersion(version), itemType(std::ref(type))
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode)
    : itemVersion(version)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type, std::uint16_t gameVer)
    : itemVersion(version), itemType(std::ref(type)), gameVersion(gameVer)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode, std::uint16_t gameVer)
    : itemVersion(version), gameVersion(gameVer)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type, bool isExp)
    : itemVersion(version), itemType(std::ref(type)), isExpansion(isExp)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode, bool isExp)
    : itemVersion(version), isExpansion(isExp)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type, d2ce::EnumDifficulty diff)
    : itemVersion(version), itemType(std::ref(type)), difficulty(diff)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode, d2ce::EnumDifficulty diff)
    : itemVersion(version), difficulty(diff)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type, d2ce::EnumDifficulty diff, std::uint16_t gameVer)
    : itemVersion(version), itemType(std::ref(type)), difficulty(diff), gameVersion(gameVer)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode, d2ce::EnumDifficulty diff, std::uint16_t gameVer)
    : itemVersion(version), difficulty(diff), gameVersion(gameVer)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type, d2ce::EnumDifficulty diff, bool isExp)
    : itemVersion(version), itemType(std::ref(type)), difficulty(diff), isExpansion(isExp)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode, d2ce::EnumDifficulty diff, bool isExp)
    : itemVersion(version), difficulty(diff), isExpansion(isExp)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type, d2ce::EnumDifficulty diff, d2ce::EnumCharClass clazz)
    : itemVersion(version), itemType(std::ref(type)), difficulty(diff), charClass(clazz)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode, d2ce::EnumDifficulty diff, d2ce::EnumCharClass clazz)
    : itemVersion(version), difficulty(diff), charClass(clazz)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type, d2ce::EnumDifficulty diff, d2ce::EnumCharClass clazz, std::uint16_t gameVer)
    : itemVersion(version), itemType(std::ref(type)), difficulty(diff), charClass(clazz), gameVersion(gameVer)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode, d2ce::EnumDifficulty diff, d2ce::EnumCharClass clazz, std::uint16_t gameVer)
    : itemVersion(version), difficulty(diff), charClass(clazz), gameVersion(gameVer)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        isExpansion = gameVersion >= 100 ? true : false;
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, const ItemType& type, d2ce::EnumDifficulty diff, d2ce::EnumCharClass clazz, bool isExp)
    : itemVersion(version), itemType(std::ref(type)), difficulty(diff), charClass(clazz), isExpansion(isExp)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }
}
//---------------------------------------------------------------------------
d2ce::ItemCreateParams::ItemCreateParams(EnumItemVersion version, std::array<std::uint8_t, 4>& strcode, d2ce::EnumDifficulty diff, d2ce::EnumCharClass clazz, bool isExp)
    : itemVersion(version), difficulty(diff), charClass(clazz), isExpansion(isExp)
{
    switch (itemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        gameVersion = 0;
        break;

    case EnumItemVersion::v107:
        gameVersion = isExpansion ? 100 : 0;
        break;

    default:
        gameVersion = isExpansion ? 100 : 1;
    }

    itemType = std::ref(ItemHelpers::getItemTypeHelper(strcode));
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
//---------------------------------------------------------------------------
d2ce::Item::Item(const Item& other)
{
    *this = other;
}
//---------------------------------------------------------------------------
d2ce::Item::Item(const ItemCreateParams& createParams)
{
    static Item invalidItem;
    GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) = 0;
    if (!createParams.itemType.has_value())
    {
        // invalid params
        *this = invalidItem;
        return;
    }

    const auto& itemType = createParams.itemType.value().get();
    if (!createParams.isExpansion && itemType.isExpansionItem())
    {
        // unsupported item
        *this = invalidItem;
        return;
    }

    if (itemType.isGoldItem())
    {
        // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
        // D2R will crash if we create this unused item
        *this = invalidItem;
        return;
    }

    std::array<std::uint8_t, 4> strcode = { 0x20, 0x20, 0x20, 0x20 };
    std::memcpy(strcode.data(), itemType.code.c_str(), std::min(itemType.code.size(), size_t(3)));

    // reserve enough space to reduce chance of reallocation (haven't seen an item size bigger then 80 bytes)
    data.reserve(80);

    std::uint64_t itemCode = 0;
    std::uint8_t itemCodeBitsSet = 0;
    ItemVersion = createParams.itemVersion;
    GameVersion = createParams.gameVersion;
    std::uint16_t rawVersion = 5;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100:
        itemCodeBitsSet = 10;
        itemCode = itemType.code_v100;
        rawVersion = 0;
        break;

    case EnumItemVersion::v104:
        itemCodeBitsSet = 30;
        itemCode = *((std::uint32_t*)strcode.data());
        rawVersion = 0;
        break;

    case EnumItemVersion::v107:
        itemCodeBitsSet = 32;
        itemCode = *((std::uint32_t*)strcode.data());
        rawVersion = 0;
        break;

    case EnumItemVersion::v108:
    case EnumItemVersion::v109:
        itemCodeBitsSet = 32;
        itemCode = *((std::uint32_t*)strcode.data());
        rawVersion = createParams.isExpansion ? 100 : 1;
        break;

    case EnumItemVersion::v110:
        itemCodeBitsSet = 32;
        itemCode = *((std::uint32_t*)strcode.data());
        rawVersion = createParams.isExpansion ? 101 : 2;
        break;

    case EnumItemVersion::v100R:
    case EnumItemVersion::v120:
    case EnumItemVersion::v140:
    default:
        ItemHelpers::encodeResurrectedItem(strcode, itemCode, itemCodeBitsSet);
        rawVersion = createParams.isExpansion ? 5 : 4;
        break;
    }

    std::uint32_t value = 0;
    std::uint64_t value64 = 0;
    GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) = 0;
    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET);
    size_t max_bit_offset = current_bit_offset;

    if (ItemVersion < EnumItemVersion::v100R)
    {
        value = *((std::uint16_t*)ITEM_MARKER.data());
        setBits(current_bit_offset, ITEM_MARKER.size() * 8, value);
        GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) = current_bit_offset;
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // flags
    std::bitset<32> flags = 0;
    flags[IS_IDENTIFIED_FLAG_OFFSET] = 1;

    if (itemType.isEar())
    {
        flags[IS_EAR_FLAG_OFFSET] = 1;
    }

    if (ItemVersion > EnumItemVersion::v100)
    {
        if (itemType.isSimpleItem())
        {
            flags[IS_SIMPLE_FLAG_OFFSET] = 1;
        }
    }

    if (ItemVersion >= EnumItemVersion::v107)
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

    auto quality = EnumItemQuality::NORMAL;
    if (itemType.isUniqueItem())
    {
        quality = EnumItemQuality::UNIQUE;
        switch (ItemVersion)
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
        case EnumItemVersion::v104: // v1.04 - v1.06 item
        case EnumItemVersion::v107: // v1.07 item
        case EnumItemVersion::v108:  // v1.08 item
        case EnumItemVersion::v109:  // v1.09 item
        case EnumItemVersion::v110:  // v1.10 - v1.14d item
        case EnumItemVersion::v100R:  // v1.0.x - v1.1.x Diablo II: Resurrected item
        case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
            if (itemType.isSunderedCharms())
            {
                // should not happen
                // Saundered Charms are only valid for D2R PTR 2.5 or higher
                *this = invalidItem;
                return;
            }
            break;
        }
    }
    else if (itemType.isSetItem())
    {
        quality = EnumItemQuality::SET;
    }
    else if (itemType.isRing() || itemType.isAmulet())
    {
        switch (createParams.createQualityOption)
        {
        case EnumItemQuality::MAGIC:
        case EnumItemQuality::RARE:
        case EnumItemQuality::CRAFTED:
            quality = createParams.createQualityOption;
            break;

        default:
            quality = (ItemHelpers::generarateRandomDW() % 2 == 0) ? EnumItemQuality::MAGIC : EnumItemQuality::RARE;
            break;

        }
    }
    else if (itemType.isJewel())
    {
        switch (createParams.createQualityOption)
        {
        case EnumItemQuality::MAGIC:
        case EnumItemQuality::RARE:
            quality = createParams.createQualityOption;
            break;

        default:
            quality = (ItemHelpers::generarateRandomDW() % 2 == 0) ? EnumItemQuality::MAGIC : EnumItemQuality::RARE;
            break;

        }
    }
    else if (itemType.isCharm())
    {
        quality = EnumItemQuality::MAGIC;
    }
    else
    {
        switch (createParams.createQualityOption)
        {
        case EnumItemQuality::MAGIC:
        case EnumItemQuality::RARE:
        case EnumItemQuality::CRAFTED:
            quality = createParams.createQualityOption;
            break;
        }
    }

    size_t bitSize = 0;
    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        switch (ItemVersion)
        {
        case EnumItemVersion::v100:
            // 27 bytes total
            data.resize((ITEM_V100_NUM_BITS + 7) / 8, 0);

            GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V100_NUM_BITS;

            if (itemType.isEar())
            {
                GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = 0;
                GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = 0;
                GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V100_EAR_COORDINATES_BIT_OFFSET + 2;
                GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V100_EAR_ATTRIBE_BIT_OFFSET;

                bitSize = 10;
                current_bit_offset = ITEM_V100_EAR_ITEMCODE_BIT_OFFSET + 5;
                if (!setBits(current_bit_offset, bitSize, 0x13B))
                {
                    *this = invalidItem;
                    return;
                }

                auto dw = ItemHelpers::generarateRandomDW();
                EarAttributes earAttrib;
                earAttrib.Class = static_cast<EnumCharClass>(dw % NUM_OF_CLASSES_NO_EXPANSION);
                earAttrib.Level = dw % MAX_NUM_LEVELS + 1;
                strcpy_s(earAttrib.Name.data(), earAttrib.Name.size(), "SOMEONE");

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
                bitSize = 3;
                if (!setBits(current_bit_offset, bitSize, std::uint32_t(earAttrib.Class)))
                {
                    *this = invalidItem;
                    return;
                }

                current_bit_offset = ITEM_V100_EAR_LEVEL_BIT_OFFSET;
                bitSize = 8;
                if (!setBits(current_bit_offset, bitSize, earAttrib.Level))
                {
                    *this = invalidItem;
                    return;
                }

                // up to 15 7 bit characters
                for (size_t idx = 0; idx <= 15; ++idx)
                {
                    if (!setBits(current_bit_offset, 7, std::uint32_t(earAttrib.Name[idx])))
                    {
                        *this = invalidItem;
                        return;
                    }

                    if (earAttrib.Name[idx] == 0)
                    {
                        break;
                    }
                }

                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint8_t)((readBits(current_bit_offset, bitSize) & 0x9F) | 0x08);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    *this = invalidItem;
                    return;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    *this = invalidItem;
                    return;
                }

                return;
            }

            GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = 58;
            GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = ITEM_V100_BITFIELD3_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 2;
            GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V100_TYPECODE_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = ITEM_V100_DURABILITY_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V100_COORDINATES_BIT_OFFSET + 1;
            GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_100;
            GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = ITEM_V100_SPECIALITEMCODE_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
            GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = ITEM_V100_LEVEL_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = ITEM_V100_DWA_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;
            if (itemCode >= MAXUINT16)
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
            if (!setBits(current_bit_offset, itemCodeBitsSet, std::uint32_t(itemCode)))
            {
                *this = invalidItem;
                return;
            }

            value = static_cast<std::underlying_type_t<EnumItemQuality>>(quality);
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET);
            bitSize = 3;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            switch (quality)
            {
            case EnumItemQuality::SET:
                value = itemType.getSetId();

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
                bitSize = ITEM_V100_UNIQUE_ID_NUM_BITS;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
                break;

            case EnumItemQuality::UNIQUE:
                value = itemType.getId();

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
                bitSize = ITEM_V100_UNIQUE_ID_NUM_BITS;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
                break;
            }

            value = ItemHelpers::generarateRandomDW();
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET);
            if (!setBits(current_bit_offset, 32, value))
            {
                *this = invalidItem;
                return;
            }

            if (quality == EnumItemQuality::SET)
            {
                value = itemType.getSetItemDWBCode();
            }
            else
            {
                value = ItemHelpers::generarateRandomDW();
            }
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET);
            if (!setBits(current_bit_offset, 32, value))
            {
                *this = invalidItem;
                return;
            }

            if (itemType.isArmor() || itemType.isWeapon())
            {
                value = itemType.durability.Max;
                bitSize = DURABILITY_MAX_NUM_BITS;
                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_CURRENT_NUM_BITS_108;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }

                if (value > 0)
                {
                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET);
                    bitSize = DURABILITY_CURRENT_NUM_BITS_108;
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        *this = invalidItem;
                        return;
                    }
                }
            }

            if (itemType.isGoldItem())
            {
                // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
                GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_100;
                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET);
                value = MAX_GLD_QUANTITY;
                if (!setBits(current_bit_offset, 1, 0))
                {
                    *this = invalidItem;
                    return;
                }

                bitSize = GLD_STACKABLE_NUM_BITS;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
            }
            else if (itemType.isStackable())
            {
                GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_100;

                bitSize = STACKABLE_NUM_BITS;
                value = std::max(1ui32, itemType.stackable.Max);
                if (itemType.isKey())
                {
                    // we need at lease one key
                    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
                    {
                        // TODO: should be handled by loading of file
                        value = std::min(value, MAX_KEY_QUANTITY_100);
                    }
                    value = std::max(1ui32, value);
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
            }

            value = std::max(1ui16, itemType.level.Quality);
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET);
            if (!setBits(current_bit_offset, 8, value))
            {
                *this = invalidItem;
                return;
            }

            // Place item in the buffer
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0008);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 8;
            value = 0;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
            bitSize = 4;
            value = 0;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x3FFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = ITEM_V100_NUM_BITS;
            max_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
            return;

        case EnumItemVersion::v104:
            if (itemType.isEar())
            {
                GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = 0;
                GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = 0;
                GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = 0;

                GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_EAR_COORDINATES_BIT_OFFSET + 2;
                GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_EAR_ATTRIBE_BIT_OFFSET;

                // 26 bytes total
                data.resize((ITEM_V104_EAR_NUM_BITS + 7) / 8, 0);
                GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V104_EAR_NUM_BITS;

                auto dw = ItemHelpers::generarateRandomDW();
                EarAttributes earAttrib;
                earAttrib.Class = static_cast<EnumCharClass>(dw % NUM_OF_CLASSES_NO_EXPANSION);
                earAttrib.Level = dw % MAX_NUM_LEVELS + 1;
                strcpy_s(earAttrib.Name.data(), earAttrib.Name.size(), "SOMEONE");

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
                bitSize = 3;
                if (!setBits(current_bit_offset, bitSize, std::uint32_t(earAttrib.Class)))
                {
                    *this = invalidItem;
                    return;
                }

                bitSize = 8;
                if (!setBits(current_bit_offset, bitSize, earAttrib.Level))
                {
                    *this = invalidItem;
                    return;
                }

                // up to 15 7 bit characters
                for (size_t idx = 0; idx <= 15; ++idx)
                {
                    if (!setBits(current_bit_offset, 7, std::uint32_t(earAttrib.Name[idx])))
                    {
                        *this = invalidItem;
                        return;
                    }

                    if (earAttrib.Name[idx] == 0)
                    {
                        break;
                    }
                }

                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xE3FF) | 0x0100);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    *this = invalidItem;
                    return;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    *this = invalidItem;
                    return;
                }

                return;
            }

            if (itemType.isSimpleItem())
            {
                // 15 bytes total
                data.resize((ITEM_V104_SM_NUM_BITS + 7) / 8, 0);

                GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_SM_COORDINATES_BIT_OFFSET + 2;
                GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_SM_TYPECODE_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V104_SM_NUM_BITS;
                GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = 0;
                GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = 0;
                GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = 0;

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
                if (!setBits(current_bit_offset, itemCodeBitsSet, std::uint32_t(itemCode)))
                {
                    *this = invalidItem;
                    return;
                }

                // Place item in the buffer
                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03FE) | 0x0001);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    *this = invalidItem;
                    return;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    *this = invalidItem;
                    return;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = ((std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC) | 0x00000003);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }

                GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = ITEM_V104_SM_NUM_BITS;
                max_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
                return;
            }

            GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_EX_TYPECODE_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 3;
            GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = ITEM_V104_EX_DURABILITY_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_EX_COORDINATES_BIT_OFFSET + 1;
            GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_104;
            GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = ITEM_V104_EX_UNIQUECODE_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
            GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = ITEM_V104_EX_LEVEL_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = ITEM_V104_EX_DWA_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;

            // 31 bytes total
            data.resize((ITEM_V104_EX_NUM_BITS + 7) / 8, 0);
            GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V104_EX_NUM_BITS;

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
            if (!setBits(current_bit_offset, itemCodeBitsSet, std::uint32_t(itemCode)))
            {
                *this = invalidItem;
                return;
            }

            value = static_cast<std::underlying_type_t<EnumItemQuality>>(quality);
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET);
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            switch (quality)
            {
            case EnumItemQuality::SET:
                value = itemType.getSetId();

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
                bitSize = ITEM_V100_UNIQUE_ID_NUM_BITS;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
                break;

            case EnumItemQuality::UNIQUE:
                value = itemType.getId();

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
                bitSize = ITEM_V100_UNIQUE_ID_NUM_BITS;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
                break;
            }

            if (itemType.isArmor() || itemType.isWeapon())
            {
                value = itemType.durability.Max;
                bitSize = DURABILITY_MAX_NUM_BITS;
                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_CURRENT_NUM_BITS_108;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }

                if (value > 0)
                {
                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET);
                    bitSize = DURABILITY_CURRENT_NUM_BITS_108;
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        *this = invalidItem;
                        return;
                    }
                }
            }

            if (itemType.isGoldItem())
            {
                // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
                GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_104;
                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET);
                value = MAX_GLD_QUANTITY;
                if (!setBits(current_bit_offset, 1, 0))
                {
                    *this = invalidItem;
                    return;
                }

                bitSize = GLD_STACKABLE_NUM_BITS;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
            }
            else if (itemType.isStackable())
            {
                GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_104;

                bitSize = STACKABLE_NUM_BITS;
                value = std::max(1ui32, itemType.stackable.Max);
                if (itemType.isKey())
                {
                    // we need at lease one key
                    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
                    {
                        // TODO: should be handled by loading of file
                        value = std::min(value, MAX_KEY_QUANTITY_100);
                    }
                    value = std::max(1ui32, value);
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
            }

            value = std::max(1ui16, itemType.level.Quality);
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET);
            if (!setBits(current_bit_offset, 8, value))
            {
                *this = invalidItem;
                return;
            }

            value = ItemHelpers::generarateRandomDW();
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET);
            if (!setBits(current_bit_offset, 32, value))
            {
                *this = invalidItem;
                return;
            }

            if (quality == EnumItemQuality::SET)
            {
                value = itemType.getSetItemDWBCode();
            }
            else
            {
                value = ItemHelpers::generarateRandomDW();
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET);
            if (!setBits(current_bit_offset, 32, value))
            {
                *this = invalidItem;
                return;
            }

            // Place item in the buffer
            current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 8;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x01);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
            bitSize = 8;
            value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
            bitSize = 4;
            value = 0;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
            bitSize = 32;
            value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xBFFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }

            GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = ITEM_V104_EX_NUM_BITS;
            max_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
            return;

        default: // should not happen
            *this = invalidItem;
            max_bit_offset = 0;
            return;
        }
    }

    value = rawVersion;
    bitSize = (ItemVersion < EnumItemVersion::v100R ? 10 : 3);
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = current_bit_offset;
    value = static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::BUFFER);
    bitSize = 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = current_bit_offset;
    value = 0;
    bitSize = 4;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // position x/y
    GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = current_bit_offset;
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
    GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = current_bit_offset;
    value = static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::UNKNOWN);
    bitSize = 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // type code
    GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = current_bit_offset;
    if (itemType.isEar())
    {
        auto dw = ItemHelpers::generarateRandomDW();
        EarAttributes earAttrib;
        earAttrib.Class = static_cast<EnumCharClass>(dw % NUM_OF_CLASSES_NO_EXPANSION);
        earAttrib.Level = dw % MAX_NUM_LEVELS + 1;
        strcpy_s(earAttrib.Name.data(), earAttrib.Name.size(), "SOMEONE");

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
        if (!setBits(current_bit_offset, 3, std::uint32_t(earAttrib.Class)))
        {
            *this = invalidItem;
            return;
        }

        if (!setBits(current_bit_offset, 7, earAttrib.Level))
        {
            *this = invalidItem;
            return;
        }

        // up to 15 7/8 bit characters
        bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
        for (size_t idx = 0; idx <= 15; ++idx)
        {
            if (!setBits(current_bit_offset, bitSize, std::uint32_t(earAttrib.Name[idx])))
            {
                *this = invalidItem;
                return;
            }

            if (earAttrib.Name[idx] == 0)
            {
                break;
            }
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = max_bit_offset;
        return;
    }

    std::uint8_t numBitsSet = 0;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        *this = invalidItem; // should not get here
        return;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        if (!setBits(current_bit_offset, itemCodeBitsSet, std::uint32_t(itemCode)))
        {
            *this = invalidItem;
            return;
        }
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        ItemHelpers::encodeResurrectedItem(strcode, value64, numBitsSet);
        if (!setBits64(current_bit_offset, itemCodeBitsSet, itemCode))
        {
            *this = invalidItem;
            return;
        }
        break;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = max_bit_offset;

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
    value = 0;
    bitSize = itemType.isSimpleItem() ? 1 : 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    if (itemType.isSimpleItem())
    {
        if (itemType.isGoldItem())
        {
            // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
            GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) + 1;
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET);
            value = MAX_GLD_QUANTITY;
            if (!setBits(current_bit_offset, 1, 0))
            {
                *this = invalidItem;
                return;
            }

            bitSize = GLD_STACKABLE_NUM_BITS;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = current_bit_offset;
            nr_of_items_in_sockets_bits = 1;
        }
        else
        {
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
            nr_of_items_in_sockets_bits = 1;
            if (itemType.isQuestItem())
            {
                GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) + 2;
                nr_of_items_in_sockets_bits = 1;

                value = static_cast<std::underlying_type_t<EnumDifficulty>>(createParams.difficulty);
                bitSize = 2;

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    *this = invalidItem;
                    return;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            }
        }

        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
        max_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
        return;
    }

    if (itemType.isQuestItem())
    {
        GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
        GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) + 2;
        nr_of_items_in_sockets_bits = 1;

        value = static_cast<std::underlying_type_t<EnumDifficulty>>(createParams.difficulty);
        bitSize = 2;

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }
    else
    {
        GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
        nr_of_items_in_sockets_bits = 3;
    }

    value = 0;
    bitSize = nr_of_items_in_sockets_bits;
    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET);
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = current_bit_offset;
    auto dwa = ItemHelpers::generarateRandomDW();
    value = dwa;
    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET);
    if (!setBits(current_bit_offset, 32, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = current_bit_offset;
    value = std::max(1ui16, itemType.level.Quality);
    bitSize = 7;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = current_bit_offset;
    value = std::uint32_t(quality);
    bitSize = 4;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // If this is TRUE, it means the item has more than one picture associated with it.
    GET_BIT_OFFSET(ItemOffsets::MULTI_GRAPHIC_BIT_OFFSET) = current_bit_offset;
    value = itemType.hasMultipleGraphics() ? 1 : 0;
    bitSize = 1;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    if (value != 0)
    {
        value = 0;
        std::uint32_t modulo = itemType.getNumPictureIds();
        if (modulo > 0)
        {
            value = ItemHelpers::generateDWARandomOffset(dwa, 1) % modulo;
        }

        bitSize = 3;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // If this is TRUE, it means the item is class specific.
    GET_BIT_OFFSET(ItemOffsets::AUTOAFFIX_BIT_OFFSET) = current_bit_offset;
    value = itemType.isClassSpecific() ? 1 : 0;
    bitSize = 1;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    if (value != 0)
    {
        value = itemType.auto_prefix;
        bitSize = 11;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    auto dwb = ItemHelpers::generarateRandomDW();
    MagicalCachev100 generated_magic_affixes;
    RareOrCraftedCachev100 generated_rare_affixes;
    std::vector<MagicalAttribute> craftingAttributes;
    switch (quality)
    {
    case EnumItemQuality::MAGIC:
        if (!ItemHelpers::generateMagicalAffixes(generated_magic_affixes, createParams, getLevel(), dwb, true))
        {
            *this = invalidItem;
            return;
        }
        break;

    case EnumItemQuality::RARE:
        if (!ItemHelpers::generateRareOrCraftedAffixes(generated_rare_affixes, createParams, getLevel(), dwb, true))
        {
            *this = invalidItem;
            return;
        }

        for (const auto& affix : generated_rare_affixes.Affixes)
        {
            generated_magic_affixes.MagicalAttributes.insert(generated_magic_affixes.MagicalAttributes.end(), affix.MagicalAttributes.begin(), affix.MagicalAttributes.end());
        }
        break;

    case EnumItemQuality::CRAFTED:
        if (!ItemHelpers::generateRareOrCraftedAffixes(generated_rare_affixes, createParams, getLevel(), dwb, true))
        {
            *this = invalidItem;
            return;
        }

        craftingAttributes.swap(generated_rare_affixes.Affixes.back().MagicalAttributes);
        generated_rare_affixes.Affixes.pop_back();
        for (const auto& affix : generated_rare_affixes.Affixes)
        {
            generated_magic_affixes.MagicalAttributes.insert(generated_magic_affixes.MagicalAttributes.end(), affix.MagicalAttributes.begin(), affix.MagicalAttributes.end());
        }
        ItemHelpers::mergeMagicalAttributes(generated_magic_affixes.MagicalAttributes, craftingAttributes);
        break;

    case EnumItemQuality::UNIQUE:
        if (!ItemHelpers::getUniqueMagicAttribs(itemType.getId(), generated_magic_affixes.MagicalAttributes, createParams, getLevel(), dwb, true))
        {
            *this = invalidItem;
            return;
        }
        break;

    case EnumItemQuality::SET:
        if (!ItemHelpers::getSetMagicAttribs(itemType.getId(), generated_magic_affixes.MagicalAttributes, createParams, getLevel(), dwb, true))
        {
            *this = invalidItem;
            return;
        }
        break;

    default:
        if (itemType.isQuestItem())
        {
            // not all quest items are unqique, so allow empty list
            ItemHelpers::getUniqueQuestMagicAttribs(strcode, generated_magic_affixes.MagicalAttributes, createParams, getLevel(), dwb, true);
        }
        break;
    }

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = current_bit_offset;
    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
    switch (quality)
    {
    case EnumItemQuality::MAGIC:
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
        value = generated_magic_affixes.Affixes.PrefixId;
        bitSize = MAGICAL_AFFIX_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        value = generated_magic_affixes.Affixes.SuffixId;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::TEMPERED:
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
        value = generated_rare_affixes.Id;
        bitSize = RARE_CRAFTED_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        value = generated_rare_affixes.Id2;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }

        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        for (auto& affix : generated_rare_affixes.Affixes)
        {
            value = affix.Affixes.PrefixId;
            if (!setBits(current_bit_offset, 1, (value != 0) ? 1 : 0))
            {
                *this = invalidItem;
                return;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);

            if (value != 0)
            {
                if (!setBits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
                {
                    *this = invalidItem;
                    return;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            }

            value = affix.Affixes.SuffixId;
            if (!setBits(current_bit_offset, 1, (value == 0) ? 0 : 1))
            {
                *this = invalidItem;
                return;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);

            if (value != 0)
            {
                if (!setBits(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
                {
                    *this = invalidItem;
                    return;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            }
        }

        // Fill in any missing affix missing flags
        value = 0;
        bitSize = size_t((3ui64 - generated_rare_affixes.Affixes.size()) * 2ui64);
        if (bitSize > 0)
        {
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }
        }
        break;

    case EnumItemQuality::SET:
    case EnumItemQuality::UNIQUE:
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
        value = itemType.getId();

        bitSize = SET_UNIQUE_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        break;
    }
    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER) = current_bit_offset;
    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER) = current_bit_offset;

    bool isArmor = itemType.isArmor();
    bool isWeapon = itemType.isWeapon();
    bool isTome = itemType.isBook();
    bool isStackable = itemType.isStackable();

    if (isTome)
    {
        // If the item is a tome, it will contain 5 extra bits, indicating
        // the spell ID. Town Portal = 0, Indentify = 1
        GET_BIT_OFFSET(ItemOffsets::TOME_BIT_OFFSET) = current_bit_offset;
        value = strcode[0] == 'i' ? 1 : 0;
        bitSize = 5;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }
    else if (itemType.isBodyPart())
    {
        // If the item is a body part, it will contain 10 extra bits indicating
        // the monster Id from monstat.txt
        GET_BIT_OFFSET(ItemOffsets::BODY_PART_BIT_OFFSET) = current_bit_offset;

        value = 0; // Skeleton
        bitSize = MONSTER_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // Realm Data Flag
    GET_BIT_OFFSET(ItemOffsets::REALM_BIT_OFFSET) = current_bit_offset;
    value = 0;
    bitSize = 1;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        *this = invalidItem;
        return;
    }

    if (isArmor || isWeapon)
    {
        if (isArmor)
        {
            // Defense rating
            const auto& stat = ItemHelpers::getItemStat(getVersion(), "armorclass");
            GET_BIT_OFFSET(ItemOffsets::DEFENSE_RATING_BIT_OFFSET) = current_bit_offset;
            value = stat.saveAdd;
            bitSize = (ItemVersion >= EnumItemVersion::v110) ? DEFENSE_RATING_NUM_BITS : DEFENSE_RATING_NUM_BITS_108;
            value += std::max(itemType.ac.Min, itemType.ac.Max);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        }

        // Some armor/weapons like phase blades don't have durability
        GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = current_bit_offset;
        value = itemType.durability.Base;
        bitSize = DURABILITY_MAX_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        if (value > 0)
        {
            // current durability value (8 bits + unknown single bit)
            bitSize = (ItemVersion >= EnumItemVersion::v110) ? DURABILITY_CURRENT_NUM_BITS : DURABILITY_CURRENT_NUM_BITS_108;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        }
    }

    if (isStackable)
    {
        // If the item is a stacked item, e.g. a javelin or something, these 9
        // bits will contain the quantity.
        GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = current_bit_offset;
        value = itemType.stackable.Max;
        bitSize = STACKABLE_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            *this = invalidItem;
            return;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SOCKET_COUNT_BIT_OFFSET_MARKER) = current_bit_offset;

    // If the item is part of a set, these bit will tell us how many lists
    // of magical properties follow the one regular magical property list.
    std::uint8_t setBonusBits = 0;
    std::vector<std::vector<MagicalAttribute>> bonusAttribs;
    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::BONUS_BITS_BIT_OFFSET_MARKER) = current_bit_offset;
    if (quality == EnumItemQuality::SET)
    {
        setBonusBits = std::uint8_t(itemType.getSetBonusBits());
        if (setBonusBits > 0)
        {
            GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET) = current_bit_offset;

            if (!ItemHelpers::getSetItemBonusAttribs(itemType.getId(), bonusAttribs, createParams, getLevel(), dwb, true))
            {
                *this = invalidItem;
                return;
            }

            if (bonusAttribs.empty() || bonusAttribs.size() > 5)
            {
                *this = invalidItem;
                return;
            }

            value = setBonusBits;
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                *this = invalidItem;
                return;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        }
    }

    // magical properties
    GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET) = current_bit_offset;
    if (!updatePropertyList(current_bit_offset, generated_magic_affixes.MagicalAttributes))
    {
        *this = invalidItem;
        return;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SET_BONUS_PROPS_BIT_OFFSET_MARKER) = current_bit_offset;
    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        GET_BIT_OFFSET(ItemOffsets::SET_BONUS_PROPS_BIT_OFFSET) = current_bit_offset;
        for (const auto& bonusAttrib : bonusAttribs)
        {
            for (; setBonusBits != 0 && (setBonusBits & 1) == 0; setBonusBits >>= 1);
            if (setBonusBits == 0)
            {
                *this = invalidItem;
                return;
            }

            if (!updatePropertyList(current_bit_offset, bonusAttrib))
            {
                *this = invalidItem;
                return;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            setBonusBits >>= 1;
        }
    }

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER) = current_bit_offset;
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = max_bit_offset;
}
//---------------------------------------------------------------------------
d2ce::Item::Item(EnumItemVersion itemVersion, bool isExpansion, const std::filesystem::path& path)
{
    std::FILE* charfile = nullptr;
    errno_t err = err = _wfopen_s(&charfile, path.wstring().c_str(), L"rb");
    if (err != 0)
    {
        // something went wrong
        return;
    }

    std::fseek(charfile, 0, SEEK_END);
    auto fileBitSize = std::ftell(charfile) * 8;
    std::rewind(charfile);

    bool bIsResurrected = false;
    std::uint8_t value = 0;
    std::fread(&value, sizeof(value), 1, charfile);
    if (value != ITEM_MARKER[0])
    {
        bIsResurrected = true; // we can only hope it is true
    }
    else
    {
        std::fread(&value, sizeof(value), 1, charfile);
        if (value != ITEM_MARKER[1])
        {
            bIsResurrected = true;  // we can only hope it is true
        }
    }
    std::rewind(charfile);

    // d2i items files are usually based on v1.07 - v1.14d item format, however
    // there is the rare chance you have v1.00 - v1.06 item format and more likely
    // v1.2.x Diablo II: Resurrected Patch 2.4 item with a player name being non-ASCII
    switch (itemVersion)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        isExpansion = false;
        if (!bIsResurrected)
        {
            switch (fileBitSize)
            {
            case ITEM_V100_NUM_BITS:
                if (readItem(itemVersion, false, charfile) && verifyItemConsistency())
                {
                    // imported successfully
                    std::fclose(charfile);
                    return;
                }
                clear();
                break;

            default:
                break;
            }
        }
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        isExpansion = false;
        if (!bIsResurrected)
        {
            switch (fileBitSize)
            {
            case ITEM_V104_EAR_NUM_BITS:
            case ITEM_V104_SM_NUM_BITS:
            case ITEM_V104_EX_NUM_BITS:
                if (readItem(itemVersion, false, charfile) && verifyItemConsistency())
                {
                    // imported successfully
                    std::fclose(charfile);
                    return;
                }
                clear();
                break;

            default:
                break;
            }
        }
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        if (isExpansion && !bIsResurrected)
        {
            if (readItem(itemVersion, isExpansion, charfile))
            {
                // update raw version
                size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + 32;
                switch (itemVersion)
                {
                case EnumItemVersion::v107:

                    switch (getRawVersion())
                    {
                    case 0:
                        break;

                    default:
                        setBits(current_bit_offset, 8, 0);
                        break;
                    }
                    break;

                case EnumItemVersion::v108:
                case EnumItemVersion::v109:
                    switch (getRawVersion())
                    {
                    case   1:
                    case 100:
                        break;

                    default:
                        setBits(current_bit_offset, 8, 100);
                        break;
                    }
                    break;

                case EnumItemVersion::v110:
                    switch (getRawVersion())
                    {
                    case   2:
                    case 101:
                        break;

                    default:
                        setBits(current_bit_offset, 8, 101);
                        break;
                    }
                    break;
                }

                if (verifyItemConsistency())
                {
                    // imported successfully
                    std::fclose(charfile);
                    return;
                }
            }
            clear();
        }
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        if (isExpansion && bIsResurrected)
        {
            if (readItem(itemVersion, isExpansion, charfile) && verifyItemConsistency())
            {
                // imported successfully
                std::fclose(charfile);
                return;
            }
            clear();
        }
        break;
    }

    // this is a more complex import as it requires us to convert
    std::rewind(charfile);
    Item convertFrom;
    auto importFromVersion = EnumItemVersion::v110;
    if (bIsResurrected)
    {
        importFromVersion = EnumItemVersion::v140;
        if (!convertFrom.readItem(importFromVersion, isExpansion, charfile) && !convertFrom.verifyItemConsistency())
        { 
            // try D2R PTR 2.4
            importFromVersion = EnumItemVersion::v120;
            if (!convertFrom.readItem(importFromVersion, isExpansion, charfile) && !convertFrom.verifyItemConsistency())
            {
                // try D2R v1.0.x - v1.1.x
                std::rewind(charfile);
                convertFrom.clear();
                importFromVersion = EnumItemVersion::v100R;
                if (!convertFrom.readItem(importFromVersion, isExpansion, charfile) && !convertFrom.verifyItemConsistency())
                {
                    // failed to import
                    std::fclose(charfile);
                    return;
                }
            }
        }
        std::fclose(charfile);
    }
    else if (!convertFrom.readItem(importFromVersion, isExpansion, charfile) && !convertFrom.verifyItemConsistency())
    {
        // Check for v1.00 - v1.06 item format
        std::rewind(charfile);
        convertFrom.clear();
        switch (fileBitSize)
        {
        case ITEM_V100_NUM_BITS:
            importFromVersion = EnumItemVersion::v100;
            if (!convertFrom.readItem(importFromVersion, false, charfile) && !convertFrom.verifyItemConsistency())
            {
                // failed to import
                std::fclose(charfile);
                return;
            }
            break;

        case ITEM_V104_EAR_NUM_BITS:
        case ITEM_V104_SM_NUM_BITS:
        case ITEM_V104_EX_NUM_BITS:
            importFromVersion = EnumItemVersion::v104;
            if (!convertFrom.readItem(importFromVersion, false, charfile) && !convertFrom.verifyItemConsistency())
            {
                // failed to import
                std::fclose(charfile);
                return;
            }
            break;

        default:
            // failed to import
            std::fclose(charfile);
            return;
        }
    }
    std::fclose(charfile);

    // convert to json first
    Json::Value itemRoot;
    convertFrom.asJson(itemRoot, 1ui32, true);

    // modify it to work for the item version we are using
    switch (itemVersion)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        isExpansion = false;
        itemRoot["Version"] = "0";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        isExpansion = false;
        itemRoot["Version"] = "0";
        itemRoot["IsV104"] = true;
        break;

    case EnumItemVersion::v107: // v1.07 item
        itemRoot["Version"] = "0";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;

    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
        itemRoot["Version"] = "100";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;

    case EnumItemVersion::v110: // v1.10 - v1.14d item 
        itemRoot["Version"] = "101";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        itemRoot["Version"] = "101";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;
    }

    if (!isExpansion)
    {
        if (itemRoot.isMember("IsRuneword"))
        {
            itemRoot["IsRuneword"] = false;
        }
    }

    if ((importFromVersion > EnumItemVersion::v100R) && (itemVersion < EnumItemVersion::v120))
    {
        // check for personaliztion string
        if (convertFrom.isPersonalized())
        {
            // personalization has changed, make sure they are all 7 bit characters
            for (std::uint8_t singleChar : convertFrom.getPersonalizedName())
            {
                if (singleChar > 0x7F)
                {
                    itemRoot["IsPersonalized"] = false;
                    itemRoot["PlayerName"] = "SOMEONE";
                    break;
                }
            }
        }
        else if (convertFrom.isEar())
        {
            // player name has changed, make sure they are all 7 bit characters
            EarAttributes earAttrib;
            if (convertFrom.getEarAttributes(earAttrib))
            {
                for (std::uint8_t singleChar : earAttrib.getName())
                {
                    if (singleChar > 0x7F)
                    {
                        itemRoot["PlayerName"] = "SOMEONE";
                        break;
                    }
                }
            }
        }
    }

    // read item in
    readItem(itemRoot, true, itemVersion, isExpansion);
    if (!verifyItemConsistency())
    {
        // failed to import
        clear();
    }
}
//---------------------------------------------------------------------------
bool d2ce::Item::writeItem(std::FILE* charfile) const
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
    socketedMagicalAttributes = other.socketedMagicalAttributes;
    cachedCombinedMagicalAttributes = other.cachedCombinedMagicalAttributes;

    ItemVersion = other.ItemVersion;
    GameVersion = other.GameVersion;
    bitOffsets = other.bitOffsets;
    bitOffsetMarkers = other.bitOffsetMarkers;
    nr_of_items_in_sockets_bits = other.nr_of_items_in_sockets_bits;
    item_current_socket_idx = other.item_current_socket_idx;
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
    cachedCombinedMagicalAttributes.swap(other.cachedCombinedMagicalAttributes);
    other.cachedCombinedMagicalAttributes.clear();
    ItemVersion = std::exchange(other.ItemVersion, APP_ITEM_VERSION);
    GameVersion = std::exchange(other.GameVersion, APP_ITEM_GAME_VERSION);
    bitOffsets.swap(other.bitOffsets);
    bitOffsetMarkers.swap(other.bitOffsetMarkers);
    nr_of_items_in_sockets_bits = std::exchange(other.nr_of_items_in_sockets_bits, 3);
    item_current_socket_idx = std::exchange(other.item_current_socket_idx, 0);
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
d2ce::EnumItemVersion d2ce::Item::getVersion() const
{
    return ItemVersion;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getGameVersion() const
{
    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        return 0; // pre-1.08 normal
    }

    return GameVersion;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getRawVersion() const
{
    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        return 0; // pre-1.08 normal
    }

    return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + 32, (ItemVersion < EnumItemVersion::v100R ? 8 : 3));
}
//---------------------------------------------------------------------------
bool d2ce::Item::isExpansionGame() const
{
    return getGameVersion() == 100 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setLocation(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY)
{
    switch (locationId)
    {
    case EnumItemLocation::STORED:
        if (altPositionId == EnumAltItemLocation::UNKNOWN)
        {
            // invalid state
            return false;
        }
        break;

    case EnumItemLocation::EQUIPPED:
        // invalid state
        return false;

    case EnumItemLocation::BELT:
    case EnumItemLocation::BUFFER:
    case EnumItemLocation::SOCKET:
        altPositionId = EnumAltItemLocation::UNKNOWN;
        break;

    default:
        return false;
    }

    return setLocation(locationId, altPositionId, EnumEquippedId::NONE, positionX, positionY);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setLocation(EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY)
{
    switch (locationId)
    {
    case EnumItemLocation::BELT:
    case EnumItemLocation::BUFFER:
    case EnumItemLocation::SOCKET:
        break;

    case EnumItemLocation::STORED:
    case EnumItemLocation::EQUIPPED:
    default:
        // invalid state
        return false;
    }

    return setLocation(locationId, EnumAltItemLocation::UNKNOWN, EnumEquippedId::NONE, positionX, positionY);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setLocation(EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY)
{
    switch (altPositionId)
    {
    case EnumAltItemLocation::UNKNOWN:
        // invalid state
        return false;

    case EnumAltItemLocation::HORADRIC_CUBE:
        if (isHoradricCube())
        {
            // invalid state
            return false;
        }
        break;
    }

    return setLocation(EnumItemLocation::STORED, altPositionId, EnumEquippedId::NONE, positionX, positionY);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setLocation(EnumEquippedId equippedId)
{
    if (equippedId == EnumEquippedId::NONE)
    {
        // invalid state
        return false;
    }

    if (!canEquip(equippedId))
    {
        // invalid state
        return false;
    }

    return setLocation(EnumItemLocation::EQUIPPED, EnumAltItemLocation::UNKNOWN, equippedId, static_cast<std::uint16_t>(equippedId), 0);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setLocation(EnumItemLocation locationId, EnumAltItemLocation altPositionId, EnumEquippedId equippedId, std::uint16_t positionX, std::uint16_t positionY)
{
    switch (locationId)
    {
    case EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case EnumAltItemLocation::UNKNOWN:
            // invalid state
            return false;

        case EnumAltItemLocation::HORADRIC_CUBE:
            if (isHoradricCube())
            {
                // invalid state
                return false;
            }
            break;
        }

        equippedId = EnumEquippedId::NONE;
        break;

    case EnumItemLocation::EQUIPPED:
        if (equippedId == EnumEquippedId::NONE)
        {
            // invalid state
            return false;
        }

        if (!canEquip(equippedId))
        {
            // invalid state
            return false;
        }

        altPositionId = EnumAltItemLocation::UNKNOWN;
        positionX = static_cast<std::uint16_t>(equippedId);
        positionY = 0;
        break;

    case EnumItemLocation::BELT:
    case EnumItemLocation::BUFFER:
    case EnumItemLocation::SOCKET:
        equippedId = EnumEquippedId::NONE;
        altPositionId = EnumAltItemLocation::UNKNOWN;
        break;

    default:
        return false;
    }

    if (isEar())
    {
        size_t current_bit_offset = 0;
        size_t bitSize = 10;
        std::uint32_t value = 0;
        switch (locationId)
        {
        case EnumItemLocation::STORED:
            break;

        case EnumItemLocation::BUFFER:
            switch (ItemVersion)
            {
            case EnumItemVersion::v100:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint8_t)((readBits(current_bit_offset, bitSize) & 0x9F) | 0x08);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }
                return true;

            case EnumItemVersion::v104:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xE3FF) | 0x0100);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }
                return true;

            default:
                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET);
                value = static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::BUFFER);
                bitSize = 3;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                // position x/y
                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 4;
                value = 0;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                bitSize = 4;
                value = 0;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET);
                bitSize = 3;
                value = static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::UNKNOWN);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;
            }
            break;

        case EnumItemLocation::BELT:
        case EnumItemLocation::SOCKET:
        case EnumItemLocation::EQUIPPED:
        default:
            // ears can only be stored
            return false;
        }

        switch (ItemVersion)
        {
        case EnumItemVersion::v100:
            switch (altPositionId)
            {
            case EnumAltItemLocation::INVENTORY:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x9F);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                return true;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x9F) | 0x40);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                return true;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x9F) | 0x60);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                return true;
            }
            break;

        case EnumItemVersion::v104:
            switch (altPositionId)
            {
            case EnumAltItemLocation::INVENTORY:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xE3FF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                return true;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xE3FF) | 0x1000);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                return true;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xE3FF) | 0x0C00);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                return true;
            }
            break;

        default:
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET);
            value = static_cast<std::underlying_type_t<EnumItemLocation>>(locationId);
            bitSize = 3;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            // position x/y
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
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

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET);
            bitSize = 3;
            value = static_cast<std::underlying_type_t<EnumAltItemLocation>>(altPositionId);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            return true;
        }
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
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xF807);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
                bitSize = 4;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x3FFF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0020);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
                bitSize = 4;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x3FFF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0018);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
                bitSize = 4;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x3FFF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;
            }
            break;

        case EnumItemLocation::BELT:
            if (!isBeltable())
            {
                return false;
            }

            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            positionY = positionX / 4;
            positionX = positionX % 4;
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, 0))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x3FFF) | 0x8000);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            return true;

        case EnumItemLocation::EQUIPPED:
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 8;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x01);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
            bitSize = 4;
            value = static_cast<std::underlying_type_t<EnumEquippedId>>(equippedId);
            if (value > static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_LEFT_ARM))
            {
                return false;
            }

            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x3FFF) | 0x4000);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            return true;

        case EnumItemLocation::BUFFER:
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0008);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 8;
            value = 0;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
            bitSize = 4;
            value = 0;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x3FFF);
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
                    value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x03FE);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                    bitSize = 32;
                    value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    return true;

                case EnumAltItemLocation::STASH:
                    current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03FE) | 0x1000);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                    bitSize = 32;
                    value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    return true;

                case EnumAltItemLocation::HORADRIC_CUBE:
                    current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03FE) | 0x0C00);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                    bitSize = 32;
                    value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    return true;
                }
                break;

            case EnumItemLocation::BELT:
                if (!isBeltable())
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03FE) | 0xFC01);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                positionY = positionX / 4;
                positionX = positionX % 4;
                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) | 0x00000003);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;

            case EnumItemLocation::SOCKET:
                if (!isSocketFiller())
                {
                    return false;
                }

                if (positionX >= 7)
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03E0) | 0xFC1B);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) | 0x00000003);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;

            case EnumItemLocation::EQUIPPED:
                return false; // Simple items can't be equipped

            case EnumItemLocation::BUFFER:
                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03FE) | 0x0001);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, 0))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = ((std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC) | 0x00000003);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;
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
                    value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xF807);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                    bitSize = 8;
                    value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                    bitSize = 32;
                    value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                    bitSize = 16;
                    value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xBFFF);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    return true;

                case EnumAltItemLocation::STASH:
                    current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0020);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                    bitSize = 8;
                    value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                    bitSize = 32;
                    value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                    bitSize = 16;
                    value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xBFFF);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    return true;

                case EnumAltItemLocation::HORADRIC_CUBE:
                    current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
                    bitSize = 16;
                    value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0018);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                    bitSize = 5;
                    if (!setBits(current_bit_offset, bitSize, positionX))
                    {
                        return false;
                    }

                    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                    bitSize = 2;
                    if (!setBits(current_bit_offset, bitSize, positionY))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                    bitSize = 8;
                    value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                    bitSize = 32;
                    value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }

                    current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                    bitSize = 16;
                    value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xBFFF);
                    if (!setBits(current_bit_offset, bitSize, value))
                    {
                        return false;
                    }
                    return true;
                }
                return true;

            case EnumItemLocation::BELT:
            case EnumItemLocation::SOCKET:
                return false; // Only simple items can be socketted or put into the belt

            case EnumItemLocation::EQUIPPED:
                current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 8;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x01);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                bitSize = 8;
                value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
                bitSize = 4;
                value = static_cast<std::underlying_type_t<EnumEquippedId>>(equippedId);
                if (value > static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_LEFT_ARM))
                {
                    return false;
                }

                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                bitSize = 32;
                value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                bitSize = 16;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) | 0x4000);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                return true;

            case EnumItemLocation::BUFFER:
                current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 8;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x01);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
                bitSize = 8;
                value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
                bitSize = 4;
                value = 0;
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
                bitSize = 32;
                value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
                bitSize = 16;
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xBFFF);
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
        if ((locationId == EnumItemLocation::BELT) && !isBeltable())
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET);
        value = static_cast<std::underlying_type_t<EnumItemLocation>>(locationId);
        bitSize = 3;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        bitSize = 4;
        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
        value = 0;
        if (locationId == EnumItemLocation::EQUIPPED)
        {
            value = static_cast<std::underlying_type_t<EnumEquippedId>>(equippedId);
            positionX = std::uint16_t(value);
            positionY = 0;
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        // position x/y
        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
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

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET);
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
void d2ce::Item::verifyRuneword()
{
    if (!isSocketed() || !isExpansionGame())
    {
        return;
    }

    for (const auto& runeword : getPossibleRunewords(true))
    {
        // possible runeword
        bool foundRuneword = true;
        auto iter = runeword.runeCodes.begin();
        for (const auto& socketedItem : SocketedItems)
        {
            if (iter == runeword.runeCodes.end())
            {
                foundRuneword = false;
                break;
            }

            auto& itemType = socketedItem.getItemTypeHelper();
            if (itemType.code != *iter)
            {
                foundRuneword = false;
                break;
            }
            ++iter;
        }

        if (foundRuneword)
        {
            RunewordAttributes runeAttrib;
            if (getRunewordAttributes(runeAttrib))
            {
                if (runeword.id == runeAttrib.Id)
                {
                    // we are still ok
                    return;
                }
            }

            // change runeword id
            // make copy of original incase of failure
            size_t diff = 0;
            d2ce::Item origItem(*this);
            const auto& origData = origItem.data;

            randomizeId(); // change the item Id as we are making a new runeword
            if (isRuneword() && (GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET) != 0))
            {
                if (!updateBits(GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET), RUNEWORD_ID_NUM_BITS, runeword.id))
                {
                    swap(origItem);
                    return;
                }

                // truncate bonus list
                if (GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER) != 0)
                {
                    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER);
                    if ((GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) % 8) > 0)
                    {
                        auto bits = (std::uint8_t)(8 - (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) % 8));
                        updateBits(GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET), bits, 0);
                    }
                    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
                    data.resize(newSize, 0);
                }
            }
            else
            {
                // Complex change, add runeword id
                // resize to fit socket count
                auto old_item_end_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
                diff = RUNEWORD_ID_NUM_BITS + RUNEWORD_PADDING_NUM_BITS;
                GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += diff;
                size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
                data.resize(newSize, 0);

                // Locate position for runeword id
                size_t current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER);
                size_t old_current_bit_offset = current_bit_offset;
                size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;

                // Set item as having a runeword id
                updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_RUNEWORD_FLAG_OFFSET, 1, 1);
                GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET) = current_bit_offset;
                updateBitsEx(current_bit_offset, diff, runeword.id);

                // now copy the remaining bits
                std::uint32_t value = 0;
                size_t valueBitSize = sizeof(value) * 8;
                std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
                while (bitsToCopy > 0)
                {
                    bitsToCopy -= bits;
                    value = readtemp_bits(origData, old_current_bit_offset, bits);
                    old_current_bit_offset += bits;
                    updateBitsEx(current_bit_offset, bits, value);
                    bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
                }

                // clear any bits not written to
                if ((current_bit_offset % 8) > 0)
                {
                    bits = (std::uint8_t)(8 - (current_bit_offset % 8));
                    updateBitsEx(current_bit_offset, bits, 0);
                }

                updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER), diff);
                GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER) = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
            }

            // add bonus magical attributes from runeword id
            GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER);
            size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET);
            if (!updatePropertyList(current_bit_offset, runeword.attribs))
            {
                swap(origItem);
                return;
            }

            GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
            size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
            data.resize(newSize, 0);

            cachedCombinedMagicalAttributes.clear();
            return;
        }
    }

    cachedCombinedMagicalAttributes.clear();
    if (!isRuneword())
    {
        return;
    }

    // Complex change, remove runeword
    randomizeId(); // change the item Id as we are removing a runeword
    GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET) = 0;

    // truncate data
    size_t current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER);
    size_t old_current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER);
    size_t diff = old_current_bit_offset - current_bit_offset;
    size_t bitsToCopy = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER) - old_current_bit_offset;

    std::uint32_t value = 0;
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readBits(old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    // clear any bits not written to
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
    if ((current_bit_offset % 8) > 0)
    {
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    // truncate data
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // Set item as not having a runeword id
    GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET) = 0;
    updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_RUNEWORD_FLAG_OFFSET, 1, 0);
    GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET) = 0;
    updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER), -std::int64_t(diff));
}
//---------------------------------------------------------------------------
void d2ce::Item::updateSocketedItemCount()
{
    if (!isSocketed())
    {
        return;
    }

    cachedCombinedMagicalAttributes.clear();
    if (GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) == 0)
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            if (isSimpleItem() || isEar())
            {
                return;
            }

            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 2;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            if (isSimpleItem() || isEar())
            {
                return;
            }

            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 3;
            break;

        case EnumItemVersion::v107:  // v1.07 item
        case EnumItemVersion::v108:  // v1.08 item
        case EnumItemVersion::v109:  // v1.09 item
        case EnumItemVersion::v110:  // v1.10 - v1.14d item
        case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
        case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        default:
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
            nr_of_items_in_sockets_bits = isSimpleItem() ? 1 : 3;
            break;
        }
    }

    updateBits(GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET), nr_of_items_in_sockets_bits, std::uint32_t(SocketedItems.size()));
}
//---------------------------------------------------------------------------
void d2ce::Item::updateOffset(size_t& startOffset, ptrdiff_t diff)
{
    bool bFoundMatch = false;
    auto offsetIdx = ItemOffsets::START_BIT_OFFSET;
    auto iter = g_markerOffsets.end();
    for (size_t i = 0; i < bitOffsets.size(); ++i)
    {
        offsetIdx = static_cast<ItemOffsets>(i);
        if (offsetIdx == ItemOffsets::ITEM_END_BIT_OFFSET)
        {
            // this offset is not handled in this update
            continue;
        }

        iter = g_markerOffsets.find(offsetIdx);
        if (iter != g_markerOffsets.end())
        {
            CheckMarkerOffsetValue(GET_BIT_OFFSET_MARKER(iter->second), GET_BIT_OFFSET(iter->first), startOffset, bFoundMatch, diff);
        }
        else
        {
            CheckOffsetValue(GET_BIT_OFFSET(offsetIdx), startOffset, bFoundMatch, diff);
        }
    }
}
//---------------------------------------------------------------------------
/*
   Thanks goes to Stoned2000 for making his checksum calculation source
   available to the public.  The Visual Basic source for his editor can be
   found at http://stoned.d2network.com.
*/
void d2ce::Item::calculateChecksum(long& checksum, std::uint8_t& overflow)
{
    if (ItemVersion < EnumItemVersion::v109)
    {
        // checksum not supported
        return;
    }

    for (auto& byteValue : data)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += byteValue + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    // now calculate checksum for child items
    for (auto& item : SocketedItems)
    {
        item.calculateChecksum(checksum, overflow);
    }
}
//---------------------------------------------------------------------------
bool d2ce::Item::isIdentified() const
{
    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_IDENTIFIED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isDisabled() const
{
    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_DISABLED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isSocketed() const
{
    if (isSimpleItem() && (getVersion() != EnumItemVersion::v100))
    {
        return false;
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_SOCKETED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isNew() const
{
    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_NEW_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isBadEquipped() const
{
    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_BAD_EQUIPPED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isEar() const
{
    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_EAR_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isStarterItem() const
{
    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_STARTER_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
/*
   Returns true if this item is a simple item (i.e. 14 byte item)
*/
bool d2ce::Item::isSimpleItem() const
{
    if (getVersion() == EnumItemVersion::v100)
    {
        // v1.00 - v1.03 item
        if (isEar())
        {
            return true;
        }

        const auto& result = getItemTypeHelper();
        if (&result != &ItemHelpers::getInvalidItemTypeHelper())
        {
            return result.isSimpleItem();
        }

        return false;
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_SIMPLE_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isEthereal() const
{
    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_ETHEREAL_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPersonalized() const
{
    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        // v1.00 - v1.06 item
        return false;
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_PERSONALIZED_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRuneword() const
{
    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        // v1.00 - v1.06 item
        return false;
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_RUNEWORD_FLAG_OFFSET, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
d2ce::EnumItemLocation d2ce::Item::getLocation() const
{
    std::uint8_t loc = 0;
    std::uint16_t loc16 = 0;
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        if (isEar())
        {
            return EnumItemLocation::STORED;
        }

        loc16 = (std::uint16_t)readBits(ITEM_V100_CONTAINER_BIT_OFFSET, 16);
        switch (loc16 & 0x07F8)
        {
        case 0x0000:
        case 0x0020:
        case 0x0018:
            return EnumItemLocation::STORED;

        case 0x07F8:
            loc16 = (std::uint16_t)readBits(ITEM_V100_ITEMCODE_BIT_OFFSET, 16);
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

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        if (isEar())
        {
            return EnumItemLocation::STORED;
        }

        if (isSimpleItem())
        {
            loc16 = (std::uint16_t)readBits(ITEM_V104_SM_COORDINATES_BIT_OFFSET, 16);
            switch (loc16 & 0xFC01)
            {
            case 0x0000:
            case 0x1000:
            case 0x0C00:
                loc = (std::uint8_t)readBits(ITEM_V104_SM_ITEMCODE_BIT_OFFSET, 8);
                if ((loc & 0x03) != 0)
                {
                    return EnumItemLocation::BUFFER;
                }

                return EnumItemLocation::STORED;

            case 0xFC01:
                loc = (std::uint8_t)readBits(ITEM_V104_SM_ITEMCODE_BIT_OFFSET, 8);
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

        loc16 = (std::uint16_t)readBits(ITEM_V104_EX_CONTAINER_BIT_OFFSET, 16);
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

    case EnumItemVersion::v107:  // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        loc = (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET), 3);

        if (loc > static_cast<std::underlying_type_t<EnumItemLocation>>(EnumItemLocation::SOCKET))
        {
            return EnumItemLocation::STORED;
        }

        return static_cast<EnumItemLocation>(loc);
    }
}
//---------------------------------------------------------------------------
d2ce::EnumEquippedId d2ce::Item::getEquippedId() const
{
    if (GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) == 0)
    {
        return EnumEquippedId::NONE;
    }

    auto value = (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET), 4);
    if (value > static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_LEFT_ARM))
    {
        return EnumEquippedId::NONE;
    }

    return static_cast<EnumEquippedId>(value);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPositionX() const
{
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        switch (getLocation())
        {
        case EnumItemLocation::BELT:
            return (std::uint8_t)(readBits(GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET), 2) + readBits(GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + 2, 2) * 4);

        case EnumItemLocation::STORED:
            return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET), 5);

        case EnumItemLocation::SOCKET:
            return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + 5, 3);

        default:
            return 0;
        }
        break;

    case EnumItemVersion::v107:  // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET), 4);
    }
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPositionY() const
{
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        switch (getLocation())
        {
        case EnumItemLocation::BELT:
        case EnumItemLocation::SOCKET:
            return 0;

        case EnumItemLocation::STORED:
            return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + 5, 2);

        default:
            return 0;
        }
        break;

    case EnumItemVersion::v107:  // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + 4, 4);
    }
}
//---------------------------------------------------------------------------
d2ce::EnumAltItemLocation d2ce::Item::getAltPositionId() const
{
    std::uint8_t loc = 0;
    std::uint16_t loc16 = 0;
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
        if (getLocation() != EnumItemLocation::STORED)
        {
            return EnumAltItemLocation::UNKNOWN;
        }

        if (isEar())
        {
            loc = (std::uint8_t)readBits(ITEM_V100_EAR_CONTAINER_BIT_OFFSET, 8);
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
            loc16 = (std::uint16_t)readBits(ITEM_V100_CONTAINER_BIT_OFFSET, 16);
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
            loc16 = (std::uint16_t)readBits(ITEM_V104_EAR_COORDINATES_BIT_OFFSET, 16);
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
            loc16 = (std::uint16_t)readBits(ITEM_V104_SM_COORDINATES_BIT_OFFSET, 16);
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
            loc16 = (std::uint16_t)readBits(ITEM_V104_EX_CONTAINER_BIT_OFFSET, 16);
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

    case EnumItemVersion::v107:  // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        loc = (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET), 3);
        if (loc > static_cast<std::underlying_type_t<EnumAltItemLocation>>(EnumAltItemLocation::STASH))
        {
            return EnumAltItemLocation::UNKNOWN;
        }

        return static_cast<EnumAltItemLocation>(loc);
    }
}
//---------------------------------------------------------------------------
const d2ce::ItemType& d2ce::Item::getItemTypeHelper() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        return ItemHelpers::getInvalidItemTypeHelper();
    }

    const auto& result = ItemHelpers::getItemTypeHelper(strcode);
    if (result.isExpansionItem() && !isExpansionGame())
    {
        return ItemHelpers::getInvalidItemTypeHelper();
    }

    auto quality = getQuality();
    if (quality == EnumItemQuality::UNIQUE)
    {
        d2ce::UniqueAttributes uniqueAttrib;
        if (getUniqueAttributes(uniqueAttrib))
        {
            const auto& uniqueResult = ItemHelpers::getUniqueItemTypeHelper(uniqueAttrib.Id);
            if (&uniqueResult != &ItemHelpers::getInvalidItemTypeHelper())
            {
                // make sure the base type matches (for hacked items they may not)
                if (uniqueResult.code == result.code)
                {
                    return uniqueResult;
                }
            }
        }
    }
    else if (quality == EnumItemQuality::SET)
    {
        d2ce::SetAttributes setAttrib;
        if (getSetAttributes(setAttrib))
        {
            const auto& setResult = ItemHelpers::getSetItemTypeHelper(setAttrib.Id);
            if (&setResult != &ItemHelpers::getInvalidItemTypeHelper())
            {
                // make sure the base type matches (for hacked items they may not)
                if (setResult.code == result.code)
                {
                    return setResult;
                }
            }
        }
    }

    // default to base item
    return result;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getItemCode(std::array<std::uint8_t, 4>& strcode) const
{
    strcode.fill(0);
    if (GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) == 0)
    {
        return false;
    }

    if (isEar())
    {
        strcode = ItemCodeStringConverter("ear");
        return true;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        return ItemHelpers::getItemCodev100(std::uint16_t(readBits(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET), 10)), strcode);

    case EnumItemVersion::v104: // v1.04 - v1.06 item
    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        strcode[0] = std::uint8_t(readBits(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET), 8));
        strcode[1] = std::uint8_t(readBits(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) + 8, 8));
        strcode[2] = std::uint8_t(readBits(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) + 16, 8));
        strcode[3] = std::uint8_t(readBits(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) + 24, 6));
        return true;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        ItemHelpers::getResurrectedItemCode(data, GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET), strcode);
        return true;
    }
}
//---------------------------------------------------------------------------
const std::string& d2ce::Item::getItemTypeName() const
{
    const auto& result = getItemTypeHelper();
    if (&result != &ItemHelpers::getInvalidItemTypeHelper())
    {
        return result.name;
    }

    static std::string badValue;
    return badValue;
}
//---------------------------------------------------------------------------
const std::string& d2ce::Item::getRuneLetter() const
{
    const auto& result = getItemTypeHelper();
    if (&result != &ItemHelpers::getInvalidItemTypeHelper())
    {
        return result.getRuneLetter();
    }

    static std::string badValue;
    return badValue;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getGemApplyType() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return 0;
    }

    return result.gemApplyType;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateGem(const std::array<std::uint8_t, 4>& newgem)
{
    if (isEar())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

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
    if (newResult.isExpansionItem() && !isExpansionGame())
    {
        // should not happen
        return false;
    }

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
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        numBitsSet = 10;
        code = newResult.code_v100;
        if (code >= MAXUINT16)
        {
            return false;
        }
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        numBitsSet = 30;
        code = *((std::uint32_t*)newgem.data());
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        numBitsSet = 32;
        code = *((std::uint32_t*)newgem.data());
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        ItemHelpers::encodeResurrectedItem(newgem, code, numBitsSet);
        break;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        updateBits64(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET), numBitsSet, code);
        return true;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        updateResurrectedItemCode(code, numBitsSet);
        return true;
    }
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
    if (GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) == 0)
    {
        return 0;
    }

    return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET), 2);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getEarAttributes(d2ce::EarAttributes& attrib) const
{
    attrib.clear();
    if (!isEar())
    {
        return false;
    }

    size_t currentOffset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
    attrib.Class = static_cast<EnumCharClass>(readBits(currentOffset, 3));
    currentOffset += 3;

    size_t levelBits = 7;
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        currentOffset = ITEM_V100_EAR_LEVEL_BIT_OFFSET;
        levelBits = 8;
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        levelBits = 8;
        break;
    }

    attrib.Level = std::uint32_t(readBits(currentOffset, levelBits));
    currentOffset += levelBits;

    // up to 15 7/8 bit characters
    size_t bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
    attrib.Name.fill(0);
    char c = 0;
    for (std::uint8_t idx = 0; idx < 15; ++idx)
    {
        c = (char)readBits(currentOffset, bitSize);
        if (c == 0)
        {
            break;
        }
        attrib.Name[idx] = c;
        currentOffset += bitSize;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setEarAttributes(const d2ce::EarAttributes& attrib)
{
    if (!isEar())
    {
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);

    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
    size_t bitSize = 3;
    if (!updateBitsEx(current_bit_offset, bitSize, std::uint32_t(attrib.Class)))
    {
        swap(origItem);
        return false;
    }

    bitSize = 7;
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        current_bit_offset = ITEM_V100_EAR_LEVEL_BIT_OFFSET;
        bitSize = 8;
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        bitSize = 8;
        break;
    }

    if (!updateBitsEx(current_bit_offset, bitSize, std::uint32_t(attrib.Level)))
    {
        swap(origItem);
        return false;
    }

    // up to 15 7/8 bit characters
    bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
    for (size_t idx = 0; idx <= 15; ++idx)
    {
        if (!updateBitsEx(current_bit_offset, bitSize, std::uint32_t(attrib.Name[idx])))
        {
            swap(origItem);
            return false;
        }

        if (attrib.Name[idx] == 0)
        {
            for (; idx <= 15; ++idx)
            {
                updateBitsEx(current_bit_offset, bitSize, 0ui32);
            }
            break;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRequirements(ItemRequirements& req) const
{
    req.clear();
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    req = result.req;

    d2ce::SetAttributes setAttrib;
    d2ce::UniqueAttributes uniqueAttrib;
    d2ce::MagicalAffixes magicalAffixes;
    d2ce::RareAttributes rareAttrib;
    switch (getQuality())
    {
    case EnumItemQuality::MAGIC:
        if (getMagicalAffixes(magicalAffixes))
        {
            std::uint16_t levelReq = 0;
            if (magicalAffixes.PrefixId != 0)
            {
                levelReq = ItemHelpers::getMagicalPrefixLevelReqFromId(magicalAffixes.PrefixId);
                if (levelReq > 1)
                {
                    req.Level = std::max(req.Level, levelReq);
                }
            }

            if (magicalAffixes.SuffixId != 0)
            {
                levelReq = ItemHelpers::getMagicalSuffixLevelReqFromId(magicalAffixes.SuffixId);
                if (levelReq > 1)
                {
                    req.Level = std::max(req.Level, levelReq);
                }
            }
        }
        break;

    case EnumItemQuality::SET:
        if (!result.isSetItem())
        {
            // should not happen except for hacked items
            if (getSetAttributes(setAttrib))
            {
                if (setAttrib.ReqLevel > 1)
                {
                    req.Level = setAttrib.ReqLevel;
                }
            }
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::TEMPERED:
        if (getRareOrCraftedAttributes(rareAttrib))
        {
            std::uint16_t levelReq = 0;
            for (auto affix : rareAttrib.Affixes)
            {
                if (affix.PrefixId != 0)
                {
                    levelReq = ItemHelpers::getMagicalPrefixLevelReqFromId(affix.PrefixId);
                    if (levelReq > 1)
                    {
                        req.Level = std::max(req.Level, levelReq);
                    }
                }

                if (affix.SuffixId != 0)
                {
                    levelReq = ItemHelpers::getMagicalSuffixLevelReqFromId(affix.SuffixId);
                    if (levelReq > 1)
                    {
                        req.Level = std::max(req.Level, levelReq);
                    }
                }
            }
        }
        break;

    case EnumItemQuality::UNIQUE:
        if (!result.isUniqueItem())
        {
            // should not happen except for hacked items
            if (getUniqueAttributes(uniqueAttrib))
            {
                if (uniqueAttrib.ReqLevel > 1)
                {
                    req.Level = uniqueAttrib.ReqLevel;
                }
            }
        }
        break;

    case EnumItemQuality::CRAFTED:
        if (getRareOrCraftedAttributes(rareAttrib))
        {
            std::uint16_t levelReq = 0;
            std::uint16_t affixCount = 0;
            for (auto affix : rareAttrib.Affixes)
            {
                if (affix.PrefixId != 0)
                {
                    ++affixCount;
                    levelReq = ItemHelpers::getMagicalPrefixLevelReqFromId(affix.PrefixId);
                    if (levelReq > 1)
                    {
                        req.Level = std::max(req.Level, levelReq);
                    }
                }

                if (affix.SuffixId != 0)
                {
                    ++affixCount;
                    levelReq = ItemHelpers::getMagicalSuffixLevelReqFromId(affix.SuffixId);
                    if (levelReq > 1)
                    {
                        req.Level = std::max(req.Level, levelReq);
                    }
                }
            }

            req.Level = req.Level + 10 + (3 * affixCount);
        }
        break;
    }

    ItemRequirements socketedReq;
    for (const auto& item : SocketedItems)
    {
        if (item.getRequirements(socketedReq))
        {
            if (socketedReq.Level > 1)
            {
                req.Level = std::max(req.Level, socketedReq.Level);
            }
        }
    }

    if (isEthereal())
    {
        if (req.Strength >= 10)
        {
            req.Strength -= 10;
        }
        else
        {
            req.Strength = 0;
        }

        if (req.Dexterity >= 10)
        {
            req.Dexterity -= 10;
        }
        else
        {
            req.Dexterity = 0;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getCategories(std::vector<std::string>& categories) const
{
    categories.clear();
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    categories = result.categories;
    return true;
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

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    dimensions = result.dimensions;
    return true;
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
const std::string& d2ce::Item::getInvFile() const
{
    if (isEar())
    {
        static std::string ear("invear");
        return ear;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        static std::string badValue;
        return badValue;
    }

    return result.inv_file;
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
            if (affix.PrefixId != 0)
            {
                tc = ItemHelpers::getMagicalPrefixTCFromId(affix.PrefixId);
                if (!tc.empty())
                {
                    result = tc;
                }
            }

            if (affix.SuffixId != 0)
            {
                tc = ItemHelpers::getMagicalSuffixTCFromId(affix.SuffixId);
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

    if (GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0)
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = ITEM_V100_DWA_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = ITEM_V104_EX_DWA_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;
            break;

        case EnumItemVersion::v107:  // v1.07 item
        case EnumItemVersion::v108:  // v1.08 item
        case EnumItemVersion::v109:  // v1.09 item
        case EnumItemVersion::v110:  // v1.10 - v1.14d item
        case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
        case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        default:
            GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + 95;
            break;
        }
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET), 32);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::getDWBCode() const
{
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        break;

    default:
        switch (getQuality())
        {
        case EnumItemQuality::SET:
            return ItemHelpers::getSetItemTypeHelper(getSetItemId()).getSetItemDWBCode();

        default:
            return ItemHelpers::generarateRandomDW();
        }
    }

    if (GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0)
    {
        return 0;
    }

    if (GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) == 0)
    {
        GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET), 32);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setDWBCode(std::uint32_t dwb)
{
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        break;

    default:
        return false;
    }

    if (GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0)
    {
        return false;
    }

    if (GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) == 0)
    {
        GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;
    }

    if (dwb == 0)
    {
        dwb = ItemHelpers::generarateRandomDW();
    }

    cachedCombinedMagicalAttributes.clear();
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();

    auto current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET);
    if (!setBits(current_bit_offset, 32, dwb))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::randomizeId()
{
    if (isSimpleItem() || (getVersion() < EnumItemVersion::v107))
    {
        return false;
    }

    if (GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0)
    {
        // should not happen
        getId();
        if (GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0)
        {
            return false;
        }
    }

    auto current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET);
    if (!setBits(current_bit_offset, 32, ItemHelpers::generarateRandomDW()))
    {
        return false;
    }

    for (auto& item : SocketedItems)
    {
        item.randomizeId();
    }

    return true;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getLevel() const
{
    if (isSimpleItem())
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            break;

        default:
            return 0;
        }
    }

    if (GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) == 0)
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = ITEM_V100_LEVEL_BIT_OFFSET;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = ITEM_V104_EX_LEVEL_BIT_OFFSET;
            break;

        case EnumItemVersion::v107:  // v1.07 item
        case EnumItemVersion::v108:  // v1.08 item
        case EnumItemVersion::v109:  // v1.09 item
        case EnumItemVersion::v110:  // v1.10 - v1.14d item
        case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
        case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        default:
            GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + 127;
            break;
        }
    }

    size_t numBits = 7;
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        numBits = 8;
        break;
    }

    return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET), numBits);
}
//---------------------------------------------------------------------------
d2ce::EnumItemQuality d2ce::Item::getQuality() const
{
    size_t numBits = 4;
    std::uint32_t value;
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        if (isEar())
        {
            return EnumItemQuality::UNKNOWN;
        }

        if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
        {
            GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_100;
        }

        numBits = QUALITY_NUM_BITS_100;
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        if (isSimpleItem())
        {
            return EnumItemQuality::UNKNOWN;
        }

        if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
        {
            GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_104;
        }

        numBits = QUALITY_NUM_BITS;
        break;

    case EnumItemVersion::v107:  // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        if (isSimpleItem())
        {
            return EnumItemQuality::UNKNOWN;
        }

        if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
        {
            GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET;
        }

        numBits = QUALITY_NUM_BITS;
        break;
    }

    value = readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET), numBits);
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

    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0)
    {
        return false;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        return getMagicalAffixesv100(affixes);
    }

    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET); // must copy value as readPropertyList will modify value
    affixes.PrefixId = (std::uint16_t)readBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS);
    affixes.PrefixName = ItemHelpers::getMagicalPrefixFromId(affixes.PrefixId);

    affixes.SuffixId = (std::uint16_t)readBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS);
    affixes.SuffixName = ItemHelpers::getMagicalSuffixFromId(affixes.SuffixId);

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRunewordAttributes(d2ce::RunewordAttributes& attrib) const
{
    attrib.clear();
    if (!isRuneword() || (GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET) == 0))
    {
        return false;
    }

    attrib.Id = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET), RUNEWORD_ID_NUM_BITS);
    attrib.Name = ItemHelpers::getRunewordNameFromId(attrib.Id);
    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET); // must copy value as readPropertyList will modify value
    if (!readPropertyList(current_bit_offset, attrib.MagicalAttributes))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getPersonalizedName() const
{
    if (!isPersonalized() || (GET_BIT_OFFSET(ItemOffsets::PERSONALIZED_BIT_OFFSET) == 0))
    {
        return std::string();
    }

    // up to 15 7/8 bit characters
    size_t bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
    size_t currentOffset = GET_BIT_OFFSET(ItemOffsets::PERSONALIZED_BIT_OFFSET);
    std::array<char, NAME_LENGTH> name;
    name.fill(0);
    char c = 0;
    for (std::uint8_t idx = 0; idx < 15; ++idx)
    {
        c = (char)readBits(currentOffset, bitSize);
        if (c == 0)
        {
            break;
        }
        name[idx] = c;
        currentOffset += bitSize;
    }

    return name.data();
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getTomeValue() const
{
    if (!isTome() || (GET_BIT_OFFSET(ItemOffsets::TOME_BIT_OFFSET) == 0))
    {
        return 0;
    }

    return std::uint8_t(readBits(GET_BIT_OFFSET(ItemOffsets::TOME_BIT_OFFSET), 5));
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getSetItemId() const
{
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0)
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

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        if (!getItemCode(strcode))
        {
            // should not happen
            return 0ui16;
        }

        {
            const auto& itemType = ItemHelpers::getSetItemTypeHelper((std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), ITEM_V100_UNIQUE_ID_NUM_BITS), strcode);
            if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
            {
                // should not happen
                return 0ui16;
            }

            auto id = itemType.getId();
            return (id >= MAXUINT16) ? 0 : id;
        }
    }

    return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), SET_UNIQUE_ID_NUM_BITS);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getSetAttributes(SetAttributes& attrib) const
{
    attrib.clear();
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0)
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

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        return getSetAttributesv100(attrib);
    }

    attrib.Id = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), SET_UNIQUE_ID_NUM_BITS);
    const auto& itemType = ItemHelpers::getSetItemTypeHelper(attrib.Id);
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    attrib.Name = itemType.name;
    attrib.ReqLevel = itemType.req.Level;
    if (GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET) == 0)
    {
        return false;
    }

    std::vector<MagicalAttribute> setAttribs;
    std::uint16_t setBonusBits = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET), 5);
    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::SET_BONUS_PROPS_BIT_OFFSET); // must copy value as readPropertyList will modify value
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
bool d2ce::Item::getCombinedSetAttributes(std::vector<MagicalAttribute>& attribs) const
{
    d2ce::SetAttributes setAttrib;
    if (!getSetAttributes(setAttrib))
    {
        return false;
    }

    std::vector<MagicalAttribute> tempAttribs;
    for (const auto& setAttribs : setAttrib.SetAttributes)
    {
        tempAttribs.insert(tempAttribs.end(), setAttribs.begin(), setAttribs.end());
    }

    if (tempAttribs.empty())
    {
        return false;
    }

    std::multimap<size_t, size_t> itemIndexMap;
    ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, attribs);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getFullSetAttributes(std::vector<MagicalAttribute>& attribs) const
{
    d2ce::SetAttributes setAttrib;
    if (!getSetAttributes(setAttrib))
    {
        return false;
    }

    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    return ItemHelpers::getFullSetBonusAttribs(setAttrib.Id, attribs, createParams, getLevel(), getDWBCode(), true);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRareOrCraftedAttributes(RareAttributes& attrib) const
{
    attrib.clear();
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0)
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::TEMPERED:
        break;

    default:
        return false;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        return getRareOrCraftedAttributesv100(attrib);
    }

    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
    attrib.Id = (std::uint16_t)readBitsEx(current_bit_offset, RARE_CRAFTED_ID_NUM_BITS);
    attrib.Name = ItemHelpers::getRareNameFromId(attrib.Id);
    attrib.Index = ItemHelpers::getRareIndexFromId(attrib.Id);

    attrib.Id2 = (std::uint16_t)readBitsEx(current_bit_offset, RARE_CRAFTED_ID_NUM_BITS);
    attrib.Name2 = ItemHelpers::getRareNameFromId(attrib.Id2);
    attrib.Index2 = ItemHelpers::getRareIndexFromId(attrib.Id2);

    // Following the name IDs, we got 6 possible magical affixes, the pattern
    // is 1 bit id, 11 bit value... But the value will only exist if the prefix
    // is 1. So we'll read the id first and check it against 1.
    std::uint8_t prefix = 0;
    for (size_t i = 3; i > 0; --i)
    {
        MagicalAffixes affixes;
        prefix = (std::uint8_t)readBitsEx(current_bit_offset, 1);
        if (prefix != 0)
        {
            affixes.PrefixId = (std::uint16_t)readBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS);
            affixes.PrefixName = ItemHelpers::getMagicalPrefixFromId(affixes.PrefixId);
        }

        prefix = (std::uint8_t)readBitsEx(current_bit_offset, 1);
        if (prefix != 0)
        {
            affixes.SuffixId = (std::uint16_t)readBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS);
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
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0)
    {
        if (isSimpleItem())
        {
            return false;
        }

        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = ITEM_V100_SPECIALITEMCODE_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = ITEM_V104_EX_UNIQUECODE_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
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
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        numBits = ITEM_V100_UNIQUE_ID_NUM_BITS;
        break;
    }

    attrib.Id = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), numBits);
    const auto& itemType = ItemHelpers::getUniqueItemTypeHelper(attrib.Id);
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R:  // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        if (itemType.isSunderedCharms())
        {
            // should not happen
            // Saundered Charms are only valid for D2R PTR 2.5 or higher
            return false;
        }
        break;
    }

    attrib.Name = itemType.name;
    attrib.ReqLevel = itemType.req.Level;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAttributes(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();
    d2ce::UniqueAttributes uniqueAttrib;
    d2ce::SetAttributes setAttrib;
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        return getMagicalAttributesv100(attribs);
    }

    if (isSimpleItem() || (GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET) == 0))
    {
        if (!socketedMagicalAttributes.empty())
        {
            attribs.insert(attribs.end(), socketedMagicalAttributes.begin(), socketedMagicalAttributes.end());
            return true;
        }

        return false;
    }

    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET);
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
    if (!cachedCombinedMagicalAttributes.empty())
    {
        attribs = cachedCombinedMagicalAttributes;
        return true;
    }

    std::vector<MagicalAttribute> tempAttribs;
    if (!getMagicalAttributes(tempAttribs))
    {
        if (hasUndeadBonus())
        {
            AddUndeadBonusMagicalAttribute(getVersion(), getGameVersion(), attribs);
        }

        cachedCombinedMagicalAttributes = attribs;
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

    if (hasUndeadBonus())
    {
        AddUndeadBonusMagicalAttribute(getVersion(), getGameVersion(), attribs);
    }

    cachedCombinedMagicalAttributes = attribs;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::hasMultipleGraphics() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (GET_BIT_OFFSET(ItemOffsets::MULTI_GRAPHIC_BIT_OFFSET) == 0)
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
        case EnumItemVersion::v104: // v1.04 - v1.06 item
            return hasMultipleGraphicsv100();

        case EnumItemVersion::v107:  // v1.07 item
        case EnumItemVersion::v108:  // v1.08 item
        case EnumItemVersion::v109:  // v1.09 item
        case EnumItemVersion::v110:  // v1.10 - v1.14d item
        case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
        case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        default:
            GET_BIT_OFFSET(ItemOffsets::MULTI_GRAPHIC_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + 138;
            break;
        }
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::MULTI_GRAPHIC_BIT_OFFSET), 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPictureId() const
{
    if (!hasMultipleGraphics())
    {
        return 0;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        return getPictureIdv100();
    }

    return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::MULTI_GRAPHIC_BIT_OFFSET) + 1, 3);
}
//---------------------------------------------------------------------------
bool d2ce::Item::isAutoAffix() const
{
    if (isSimpleItem())
    {
        return false;
    }

    if (GET_BIT_OFFSET(ItemOffsets::AUTOAFFIX_BIT_OFFSET) == 0)
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
        case EnumItemVersion::v104: // v1.04 - v1.06 item
            return false;

        case EnumItemVersion::v107:  // v1.07 item
        case EnumItemVersion::v108:  // v1.08 item
        case EnumItemVersion::v109:  // v1.09 item
        case EnumItemVersion::v110:  // v1.10 - v1.14d item
        case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
        case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        default:
            GET_BIT_OFFSET(ItemOffsets::AUTOAFFIX_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + 139;
            if (hasMultipleGraphics())
            {
                GET_BIT_OFFSET(ItemOffsets::AUTOAFFIX_BIT_OFFSET) += 3;
            }
            break;
        }
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::AUTOAFFIX_BIT_OFFSET), 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getAutoAffixId() const
{
    if (!isAutoAffix())
    {
        return 0;
    }

    return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::AUTOAFFIX_BIT_OFFSET) + 1, 11);
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

    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0)
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
        case EnumItemVersion::v104: // v1.04 - v1.06 item
            return getInferiorQualityIdv100();
        }
        return false;
    }

    return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), INFERIOR_SUPERIOR_ID_NUM_BITS);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getFileIndex() const
{
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0)
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
    case EnumItemQuality::NORMAL:
        if (isCharm())
        {
            // If the item is a NORMAL quality Charm, the next 12 bits will contain the Charm specific data.
            return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), NORMAL_CHARM_ID_NUM_BITS);
        }
        break;

    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::SUPERIOR:
        return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), INFERIOR_SUPERIOR_ID_NUM_BITS);

    case EnumItemQuality::SET:
    case EnumItemQuality::UNIQUE:
        return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), SET_UNIQUE_ID_NUM_BITS);
    }

    if (isBodyPart())
    {
        return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::BODY_PART_BIT_OFFSET), MONSTER_ID_NUM_BITS);
    }

    return 0;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getSetItemMask() const
{
    if (GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET) == 0)
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

    return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET), 5);
}
//---------------------------------------------------------------------------
bool d2ce::Item::isArmor() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isArmor();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isWeapon();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isTwoHandedWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isTwoHandedWeapon();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isOneOrTwoHandedWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isOneOrTwoHandedWeapon();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isShield() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isShield();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isThrownWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isThrownWeapon();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isMissileWeapon() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isMissileWeapon();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isMissile() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isMissile();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isMiscellaneous() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isMiscellaneous();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isTome() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isBook();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isBodyPart() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isBodyPart();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isStackable() const
{
    if (isSimpleItem() && !isGoldItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isStackable();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPotion() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isPotion();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isGem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isGem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isGPSItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isGPSItem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isUpgradableGem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isUpgradableGem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isUpgradablePotion() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isUpgradablePotion();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isUpgradableItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if (!result.isUpgradableItem())
    {
        return false;
    }

    switch (getQuality())
    {
    case d2ce::EnumItemQuality::INFERIOR:
    case d2ce::EnumItemQuality::NORMAL:
    case d2ce::EnumItemQuality::SUPERIOR:
    case d2ce::EnumItemQuality::MAGIC:
    case d2ce::EnumItemQuality::RARE:
    case d2ce::EnumItemQuality::CRAFTED:
    case d2ce::EnumItemQuality::TEMPERED:
        break;

    case d2ce::EnumItemQuality::SET:
    case d2ce::EnumItemQuality::UNIQUE:
    case d2ce::EnumItemQuality::UNKNOWN:
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::isUpgradableToFullRejuvenationPotion() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isUpgradableRejuvenationPotion() || (result.isPotion() && !result.isRejuvenationPotion());
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRune() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isRune();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isJewel() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isJewel();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isCharm() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isCharm();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isSunderedCharms() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isSunderedCharms();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isBelt() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isBelt();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isBeltable() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isBeltable();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isQuestItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isQuestItem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isGoldItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isGoldItem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isHoradricCube() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isHoradricCube();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isPhaseBlade() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isPhaseBlade();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isBow() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isBow();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isCrossbow() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isCrossbow();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRing() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isRing();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isAmulet() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isAmulet();
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
bool d2ce::Item::isUnusedItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isUnusedItem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isSecondHand() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isSecondHand();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isSocketFiller() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isSocketFiller();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isClassSpecific() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isClassSpecific();
}
//---------------------------------------------------------------------------
std::optional<d2ce::EnumCharClass> d2ce::Item::getClass() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return std::optional<d2ce::EnumCharClass>();
    }

    return result.getClass();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isUniqueItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isUniqueItem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isSetItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isSetItem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRestrictedItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isRestrictedItem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::isRestrictedSharedStashItem() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.isRestrictedSharedStashItem();
}
//---------------------------------------------------------------------------
bool d2ce::Item::hasUndeadBonus() const
{
    if (isSimpleItem())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.hasUndeadBonus();
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

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.canHaveSockets();
}
//---------------------------------------------------------------------------
bool d2ce::Item::canPersonalize() const
{
    if (isSimpleItem() || isRuneword() || (GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER) == 0) || !isExpansionGame())
    {
        return false;
    }

    if (isPersonalized())
    {
        return true;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.canPersonalize();
}
//---------------------------------------------------------------------------
bool d2ce::Item::canMakeSuperior() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if (!result.isArmor() && !result.isWeapon())
    {
        return false;
    }

    switch (getQuality())
    {
    case d2ce::EnumItemQuality::INFERIOR:
    case d2ce::EnumItemQuality::NORMAL:
        break;

    case d2ce::EnumItemQuality::SUPERIOR:
    case d2ce::EnumItemQuality::MAGIC:
    case d2ce::EnumItemQuality::RARE:
    case d2ce::EnumItemQuality::CRAFTED:
    case d2ce::EnumItemQuality::TEMPERED:
    case d2ce::EnumItemQuality::SET:
    case d2ce::EnumItemQuality::UNIQUE:
    case d2ce::EnumItemQuality::UNKNOWN:
    default:
        return false;
    }

    return isRuneword() ? false : true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canFixDurability() const
{
    // Stackable weapon have secret durablity that we don't fix
    ItemDurability attrib;
    if (isStackable() || isIndestructible() || !getDurability(attrib) || attrib.Max == 0)
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canMakeIndestructible() const
{
    // Stackable weapon have secret durablity that we don't fix
    if (isSimpleItem() || isStackable() || isIndestructible() || isEthereal() || (GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) == 0))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canMakeEthereal() const
{
    if (!isExpansionGame() || isIndestructible() || (getQuality() == EnumItemQuality::CRAFTED))
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if ((!result.isArmor() && !result.isWeapon()) || 
        result.isPhaseBlade() || result.isSetItem() ||
        result.isBow() || result.isCrossbow())
    {
        return false;
    }

    return isEthereal() ? false : true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canAddMagicalAffixes() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if (!result.isJewel() && (result.isGem() || result.isRune() || result.isSimpleItem()))
    {
        return false;
    }

    switch (getQuality())
    {
    case d2ce::EnumItemQuality::INFERIOR:
    case d2ce::EnumItemQuality::SUPERIOR:
        if (result.isJewel())
        {
            // Jewels should be Magical or Rare
            return false;
        }
        break;

    case d2ce::EnumItemQuality::MAGIC:
        break;

    case d2ce::EnumItemQuality::NORMAL:
        if (isRuneword() || result.isJewel())
        {
            // Jewels should be Magical or Rare
            // and runeword items can't be magical
            return false;
        }
        break;

    case d2ce::EnumItemQuality::RARE:
    case d2ce::EnumItemQuality::CRAFTED:
    case d2ce::EnumItemQuality::TEMPERED:
    case d2ce::EnumItemQuality::SET:
    case d2ce::EnumItemQuality::UNIQUE:
    case d2ce::EnumItemQuality::UNKNOWN:
    default:
        return false;
    }

    std::vector<std::uint16_t> prefixes;
    std::vector<std::uint16_t> suffixes;
    if (!getPossibleMagicalAffixes(prefixes, suffixes))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canAddRareAffixes() const
{
    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if(!result.isArmor() && !result.isWeapon() && !result.isJewel())
    {
        return false;
    }

    switch (getQuality())
    {
    case d2ce::EnumItemQuality::INFERIOR:
        if (result.isJewel())
        {
            // Jewels should be Magical or Rare
            return false;
        }
        break;

    case d2ce::EnumItemQuality::NORMAL:
        if (isRuneword() || result.isJewel())
        {
            // Jewels should be Magical or Rare
            // and runeword items can't be magical
            return false;
        }
        break;

    case d2ce::EnumItemQuality::SUPERIOR:
        if (result.isJewel())
        {
            // Jewels should be Magical or Rare
            return false;
        }
        break;

    case d2ce::EnumItemQuality::RARE:
    case d2ce::EnumItemQuality::CRAFTED:
    case d2ce::EnumItemQuality::TEMPERED:
        break;

    case d2ce::EnumItemQuality::MAGIC:
    case d2ce::EnumItemQuality::SET:
    case d2ce::EnumItemQuality::UNIQUE:
    case d2ce::EnumItemQuality::UNKNOWN:
    default:
        return false;
    }

    std::vector<std::uint16_t> prefixes;
    std::vector<std::uint16_t> suffixes;
    if (!getPossibleRareAffixes(prefixes, suffixes))
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canEquip(EnumEquippedId equipId) const
{
    if (isSimpleItem())
    {
        return false;
    }

    switch (equipId)
    {
    case EnumEquippedId::ALT_RIGHT_ARM:
    case EnumEquippedId::ALT_LEFT_ARM:
        if (!isExpansionGame())
        {
            return false;
        }
        break;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.canEquip(equipId);
}
//---------------------------------------------------------------------------
bool d2ce::Item::canEquip(EnumEquippedId equipId, EnumCharClass charClass) const
{
    if (isSimpleItem())
    {
        return false;
    }

    switch (equipId)
    {
    case EnumEquippedId::ALT_RIGHT_ARM:
    case EnumEquippedId::ALT_LEFT_ARM:
        if (!isExpansionGame())
        {
            return false;
        }
        break;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    return result.canEquip(equipId, charClass);
}
//---------------------------------------------------------------------------
bool d2ce::Item::canEquip(EnumEquippedId equipId, EnumCharClass charClass, const CharStats& cs) const
{
    if (!canEquip(equipId, charClass))
    {
        return false;
    }

    ItemRequirements itemReq;
    if (getRequirements(itemReq))
    {
        if (itemReq.Level > cs.Level)
        {
            return false;
        }

        if (itemReq.Strength > cs.Strength)
        {
            return false;
        }

        if (itemReq.Dexterity > cs.Dexterity)
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::getQuantity() const
{
    if (!isStackable() || (GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) == 0 && GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) == 0))
    {
        return 0;
    }

    if (GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) != 0)
    {
        if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
        {
            return readBits(GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET), GLD_STACKABLE_NUM_BITS);
        }

        if ((std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET), 1) > 0)
        {
            return readBits(GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) + 1, GLD_STACKABLE_LARGE_NUM_BITS);
        }

        return readBits(GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) + 1, GLD_STACKABLE_NUM_BITS);
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET), STACKABLE_NUM_BITS);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::getMaxQuantity() const
{
    const auto& itemType = getItemTypeHelper();
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return 0ui32;
    }

    if (!itemType.isStackable())
    {
        return 0ui32;
    }

    if (itemType.isKey())
    {
        // we need at lease one key
        if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
        {
            return MAX_KEY_QUANTITY_100;
        }
    }

    return itemType.stackable.Max;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setQuantity(std::uint32_t quantity)
{
    if (GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) == 0 && GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) == 0)
    {
        return false;
    }

    const auto& itemType = getItemTypeHelper();
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if (!itemType.isStackable())
    {
        return false;
    }

    quantity = std::min(quantity, itemType.stackable.Max);
    if (itemType.isKey())
    {
        // we need at lease one key
        if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
        {
            // TODO: should be handled by loading of file
            quantity = std::min(quantity, MAX_KEY_QUANTITY_100);
        }
        quantity = std::max(1ui32, quantity);
    }

    if (itemType.isGoldItem())
    {
        // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
        if (GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) != 0)
        {
            if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
            {
                return updateBits(GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET), GLD_STACKABLE_NUM_BITS, quantity);
            }

            if ((std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET), 1) > 0)
            {
                return updateBits64(GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) + 1, GLD_STACKABLE_LARGE_NUM_BITS, quantity);
            }

            quantity = std::min(quantity, MAX_GLD_QUANTITY);
            return updateBits(GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) + 1, GLD_STACKABLE_NUM_BITS, quantity);
        }

        return false;
    }

    if (getQuantity() == quantity)
    {
        // nothing to do
        return false; // so we don't count it as a change
    }

    return updateBits(GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET), STACKABLE_NUM_BITS, quantity);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMaxQuantity()
{
    if (!isStackable() || (GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) == 0 && GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) == 0))
    {
        return false;
    }

    std::uint32_t quantity = GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) != 0 ? MAXUINT32 : MAX_STACKED_QUANTITY;
    return setQuantity(quantity);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getDefenseRating() const
{
    if (GET_BIT_OFFSET(ItemOffsets::DEFENSE_RATING_BIT_OFFSET) == 0)
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
        case EnumItemVersion::v104: // v1.04 - v1.06 item
            return getDefenseRatingv100();
        }

        return 0;
    }

    const auto& stat = ItemHelpers::getItemStat(getVersion(), "armorclass");
    return (std::uint16_t)(readBits(GET_BIT_OFFSET(ItemOffsets::DEFENSE_RATING_BIT_OFFSET), ((ItemVersion >= EnumItemVersion::v110) ? DEFENSE_RATING_NUM_BITS : DEFENSE_RATING_NUM_BITS_108)) - stat.saveAdd);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setDefenseRating(std::uint16_t ac)
{
    if (GET_BIT_OFFSET(ItemOffsets::DEFENSE_RATING_BIT_OFFSET) == 0 || ac == 0)
    {
        return false;
    }

    if (getDefenseRating() == ac)
    {
        // nothing to do
        return false; // so we don't count it as a change
    }

    const auto& stat = ItemHelpers::getItemStat(getVersion(), "armorclass");
    std::uint32_t value = ac + stat.saveAdd;
    return updateBits(GET_BIT_OFFSET(ItemOffsets::DEFENSE_RATING_BIT_OFFSET), ((ItemVersion >= EnumItemVersion::v110) ? DEFENSE_RATING_NUM_BITS : DEFENSE_RATING_NUM_BITS_108), value);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMaxDefenseRating()
{
    if (GET_BIT_OFFSET(ItemOffsets::DEFENSE_RATING_BIT_OFFSET) == 0)
    {
        return false;
    }

    const auto& itemType = getItemTypeHelper();
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    auto ac = (itemType.ac.Max > itemType.ac.Min) ? itemType.ac.Max : itemType.ac.Min;
    return setDefenseRating(ac);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDurability(ItemDurability& durability) const
{
    durability.clear();
    if (GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) == 0)
    {
        return false;
    }

    const auto& itemType = getItemTypeHelper();
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        if (!(itemType.isArmor() || itemType.isWeapon()) || itemType.isMissileWeapon() || itemType.isThrownWeapon() || itemType.isMiscellaneous())
        {
            return false;
        }

        durability.Current = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET), DURABILITY_CURRENT_NUM_BITS_108);
        durability.Max = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_CURRENT_NUM_BITS_108, DURABILITY_MAX_NUM_BITS);
        if (durability.Max == 0)
        {
            durability.Current = 0;

            // Indestructible without the need for the magical attribute of indestructibility
        }

        return true;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
        durability.Max = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET), DURABILITY_MAX_NUM_BITS);
        if (durability.Max == 0)
        {
            if (itemType.isMissileWeapon() || itemType.isMiscellaneous())
            {
                return false;
            }

            if (!isExpansionGame() && itemType.isThrownWeapon())
            {
                // Durability on throwing weapons is only in the Expansion
                return false;
            }

            // Indestructible without the need for the magical attribute of indestructibility
            return true;
        }

        durability.Current = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_MAX_NUM_BITS, DURABILITY_CURRENT_READ_NUM_BITS);
        return true;

    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        durability.Max = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET), DURABILITY_MAX_NUM_BITS);
        if (durability.Max == 0)
        {
            if (itemType.isMissileWeapon() || itemType.isMiscellaneous())
            {
                return false;
            }

            if (!isExpansionGame() && itemType.isThrownWeapon())
            {
                // Durability on throwing weapons is only in the Expansion
                return false;
            }

            // Indestructible without the need for the magical attribute of indestructibility
            return true;
        }

        durability.Current = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_MAX_NUM_BITS, DURABILITY_CURRENT_READ_NUM_BITS);
        durability.CurrentBit9 = (readBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_MAX_NUM_BITS + DURABILITY_CURRENT_READ_NUM_BITS, 1) != 0) ? true : false;
        return true;
    }
}
//---------------------------------------------------------------------------
bool d2ce::Item::setDurability(const ItemDurability& attrib)
{
    if (attrib.Max == 0)
    {
        return setIndestructible();
    }

    // Stackable weapon have secret durablity that we don't fix
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

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        if (!updateBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET), DURABILITY_CURRENT_NUM_BITS_108, attrib.Current))
        {
            return false;
        }

        if (!updateBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_CURRENT_NUM_BITS_108, DURABILITY_MAX_NUM_BITS, attrib.Max))
        {
            return false;
        }

        // Make sure item is not marked broken
        updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_DISABLED_FLAG_OFFSET, 1, 0);
        return true;

    case EnumItemVersion::v107:  // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        if (!updateBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET), DURABILITY_MAX_NUM_BITS, attrib.Max))
        {
            return false;
        }

        if (!updateBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_MAX_NUM_BITS, DURABILITY_MAX_NUM_BITS, attrib.Current))
        {
            return false;
        }

        // Make sure item is not marked broken
        updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_DISABLED_FLAG_OFFSET, 1, 0);
        return true;
    }
}
//---------------------------------------------------------------------------
bool d2ce::Item::fixDurability()
{
    // Stackable weapon have secret durablity that we don't fix
    ItemDurability attrib;
    if (isStackable() || isIndestructible() || !getDurability(attrib) || attrib.Max == 0)
    {
        return false;
    }

    auto maxDurability = attrib.Max;
    const auto& result = getItemTypeHelper();
    if (&result != &ItemHelpers::getInvalidItemTypeHelper())
    {
        maxDurability = std::max(result.durability.Max, maxDurability);
    }

    auto maxCurDurability = maxDurability; // Current Durability may be higher then Max if it boosted
    ItemDurability displayedAttrib;
    if (getDisplayedDurability(displayedAttrib, 1) && (displayedAttrib.Max > maxCurDurability))
    {
        maxCurDurability = displayedAttrib.Max;
    }

    if ((maxCurDurability == attrib.Current) && (maxDurability == attrib.Max))
    {
        // nothing to do
        return false;
    }

    attrib.Max = maxDurability;
    attrib.Current = maxCurDurability;
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

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if (!result.isWeapon())
    {
        return false;
    }

    damage = result.dam;
    if (getQuality() == EnumItemQuality::INFERIOR)
    {
        if (damage.OneHanded.Min != 0)
        {
            damage.OneHanded.Min = std::min(1ui16, std::uint16_t(damage.OneHanded.Min * 0.75));
            damage.OneHanded.Max = std::min(1ui16, std::uint16_t(damage.OneHanded.Max * 0.75));
        }

        if (damage.TwoHanded.Min != 0)
        {
            damage.TwoHanded.Min = std::min(1ui16, std::uint16_t(damage.TwoHanded.Min * 0.75));
            damage.TwoHanded.Max = std::min(1ui16, std::uint16_t(damage.TwoHanded.Max * 0.75));
        }

        if (damage.Missile.Min != 0)
        {
            damage.Missile.Min = std::min(1ui16, std::uint16_t(damage.Missile.Min * 0.75));
            damage.Missile.Max = std::min(1ui16, std::uint16_t(damage.Missile.Max * 0.75));
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getRealmDataFlag() const
{
    if (isSimpleItem() || (GET_BIT_OFFSET(ItemOffsets::REALM_BIT_OFFSET) == 0))
    {
        return false;
    }

    return readBits(GET_BIT_OFFSET(ItemOffsets::REALM_BIT_OFFSET), 1) == 0 ? false : true;
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

    const auto& result = ItemHelpers::getItemTypeHelper(strcode);
    if (result.isExpansionItem() && !isExpansionGame())
    {
        // should not happen
        return false;
    }

    if (!result.canHaveSockets())
    {
        return maxSockets;
    }

    auto level = getLevel();
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        level = 1;
        break;
    }

    return std::max(result.getMaxSockets(level), getSocketCount());
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getMaxSocketedCount() const
{
    auto numSockets = getDisplayedSocketCount();
    if (numSockets == 0)
    {
        return numSockets;
    }

    return std::min(numSockets, std::uint8_t((1 << nr_of_items_in_sockets_bits) - 1));
}
//---------------------------------------------------------------------------
bool d2ce::Item::addMaxSocketCount()
{
    auto maxSockets = getMaxSocketCount();
    if (maxSockets <= getSocketCount())
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
    auto curSockets = getSocketCount();
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

    auto numSocketed = getSocketedItemCount();
    auto numSockets = getSocketCount();
    if (numSocketed >= numSockets)
    {
        // no empty sockets
        return false;
    }

    return setSocketCount(numSocketed);
}
//---------------------------------------------------------------------------
bool d2ce::Item::removeSockets()
{
    if (!isSocketed())
    {
        return false;
    }

    auto numSocketed = getSocketedItemCount();
    auto numSockets = getSocketCount();
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

    if (numSockets > 0)
    {
        auto minSockets = getSocketCountBonus();
        if (minSockets > 0)
        {
            if (numSockets <= minSockets)
            {
                numSockets = 1;
            }
            else
            {
                numSockets -= minSockets;
            }
        }

        numSockets = std::min(numSockets, getMaxSocketCount());
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        // number of sockets is always the max number possible
        if (isSocketed())
        {
            if ((numSockets > 0) || (getSocketedItemCount() > 0))
            {
                // nothing to do here
                return false;
            }

            // Set the item as not socketed
            if (numSockets == 0)
            {
                updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_SOCKETED_FLAG_OFFSET, 1, 0);
            }

            return true;
        }

        if (numSockets == 0)
        {
            // nothing to do here
            return true;
        }

        if (!canHaveSockets())
        {
            // can't add sockets
            return false;
        }

        // Set the item as not sockets
        updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_SOCKETED_FLAG_OFFSET, 1, 1);
        return true;

    case EnumItemVersion::v107:  // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        if (GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SOCKET_COUNT_BIT_OFFSET_MARKER) == 0)
        {
            return false;
        }
        break;
    }

    numSockets = std::min(numSockets, getMaxSocketCount());
    if (isSocketed())
    {
        // Already has sockets, so just update count
        auto numSocketed = getSocketedItemCount();
        numSockets = std::max(numSockets, numSocketed); // can only remove empty sockets
        if (numSockets == getSocketCount())
        {
            // nothing to do
            return false;
        }

        if (GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET) == 0)
        {
            return false;
        }

        size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET);
        if (numSockets > 0)
        {
            return setBits(current_bit_offset, SOCKET_COUNT_NUM_BITS, numSockets);
        }

        // Complex change, we are removing all sockets
        size_t old_current_bit_offset = current_bit_offset + SOCKET_COUNT_NUM_BITS;
        auto old_item_end_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
        size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;

        // move bits up
        std::uint32_t value = 0;
        size_t valueBitSize = sizeof(value) * 8;
        std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
        while (bitsToCopy > 0)
        {
            bitsToCopy -= bits;
            value = readBits(old_current_bit_offset, bits);
            old_current_bit_offset += bits;
            updateBitsEx(current_bit_offset, bits, value);
            bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
        }

        // clear any bits not written to
        if ((current_bit_offset % 8) > 0)
        {
            bits = (std::uint8_t)(8 - (current_bit_offset % 8));
            updateBitsEx(current_bit_offset, bits, 0);
        }

        // Set item as not socketed
        GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET) = 0;
        updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_SOCKETED_FLAG_OFFSET, 1, 0);

        // truncate data
        auto bitDiff = -std::int64_t(SOCKET_COUNT_NUM_BITS);
        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += bitDiff;
        size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
        data.resize(newSize, 0);

        updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SOCKET_COUNT_BIT_OFFSET_MARKER), bitDiff);
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
    auto old_item_end_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += SOCKET_COUNT_NUM_BITS;
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // Locate position for socket count
    size_t current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SOCKET_COUNT_BIT_OFFSET_MARKER);
    size_t old_current_bit_offset = current_bit_offset;
    size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;

    // Set item as socketed
    updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_SOCKETED_FLAG_OFFSET, 1, 1);
    GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET) = current_bit_offset;
    updateBitsEx(current_bit_offset, SOCKET_COUNT_NUM_BITS, numSockets);

    // now copy the remaining bits
    std::uint32_t value = 0;
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SOCKET_COUNT_BIT_OFFSET_MARKER), std::int64_t(SOCKET_COUNT_NUM_BITS));
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::addPersonalization(const std::string& name)
{
    if (!canPersonalize() || name.empty())
    {
        return false;
    }

    // Remove any invalid characters from the name
    std::string curName = name;
    LocalizationHelpers::CheckCharName(curName, ItemVersion);

    // up to 15 7/8 bit characters
    size_t bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
    size_t numChars = std::min(curName.size(), size_t(NAME_LENGTH - 1));
    size_t numberOfBitsToAdd = (numChars + 1) * bitSize;

    std::array<char, NAME_LENGTH> playerName;
    playerName.fill(0);
    strncpy_s(playerName.data(), NAME_LENGTH, curName.c_str(), numChars);
    playerName[NAME_LENGTH - 1] = 0;

    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // resize to fit personalization
    auto old_item_end_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += numberOfBitsToAdd;
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // Locate position for personalized name
    size_t current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER);
    size_t old_current_bit_offset = current_bit_offset;
    size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;

    // Set item as personalized
    updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_PERSONALIZED_FLAG_OFFSET, 1, 1);
    GET_BIT_OFFSET(ItemOffsets::PERSONALIZED_BIT_OFFSET) = current_bit_offset;

    // up to 15 7/8 bit characters
    for (size_t idx = 0; idx <= 15; ++idx)
    {
        if (!updateBitsEx(current_bit_offset, bitSize, std::uint32_t(playerName[idx])))
        {
            return false;
        }

        if (playerName[idx] == 0)
        {
            for (; idx <= 15; ++idx)
            {
                updateBitsEx(current_bit_offset, bitSize, 0ui32);
            }
            break;
        }
    }

    // now copy the remaining bits
    std::uint32_t value = 0;
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER), std::int64_t(numberOfBitsToAdd));
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::removePersonalization()
{
    if (!isPersonalized() || (GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER) == 0))
    {
        return false;
    }

    // up to 15 7/8 bit characters
    size_t bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
    size_t current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER);
    size_t old_current_bit_offset = current_bit_offset;
    std::array<char, NAME_LENGTH> name;
    name.fill(0);
    char c = 0;
    for (std::uint8_t idx = 0; idx < 15; ++idx)
    {
        c = (char)readBits(old_current_bit_offset, bitSize);
        old_current_bit_offset += bitSize;
        if (c == 0)
        {
            break;
        }
    }
    size_t numberOfBitsToRemove = old_current_bit_offset - current_bit_offset;

    auto old_item_end_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
    size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;

    // move bits up
    std::uint32_t value = 0;
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readBits(old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    // Set item as not personlized
    GET_BIT_OFFSET(ItemOffsets::PERSONALIZED_BIT_OFFSET) = 0;
    updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_PERSONALIZED_FLAG_OFFSET, 1, 0);

    // truncate data
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) -= numberOfBitsToRemove;
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER), -std::int64_t(numberOfBitsToRemove));
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setIndestructible()
{
    // Stackable weapon have secret durablity that we don't fix
    if (isSimpleItem() || isStackable() || isIndestructible() || isEthereal() || (GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) == 0))
    {
        return false;
    }

    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET);
    std::vector<MagicalAttribute> attribs;
    if (isExpansionGame())
    {
        // Indestructible magical property only exists for Expansion
        if (!readPropertyList(current_bit_offset, attribs))
        {
            // coruption
            return false;
        }
    }

    if (attribs.empty())
    {
        // Indestructible without the need for the magical attribute of indestructibility
        if (readBits(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET), DURABILITY_MAX_NUM_BITS) == 0)
        {
            // should not happen, already indestructable
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET);
        updateBitsEx(current_bit_offset, DURABILITY_MAX_NUM_BITS, 0);

        size_t old_current_bit_offset = current_bit_offset + ((ItemVersion >= EnumItemVersion::v110) ? DURABILITY_CURRENT_NUM_BITS : DURABILITY_CURRENT_NUM_BITS_108);
        auto old_item_end_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
        size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;
        size_t numberOfBitsToRemove = old_current_bit_offset - current_bit_offset;

        // move bits up
        std::uint32_t value = 0;
        size_t valueBitSize = sizeof(value) * 8;
        std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
        while (bitsToCopy > 0)
        {
            bitsToCopy -= bits;
            value = readBits(old_current_bit_offset, bits);
            old_current_bit_offset += bits;
            updateBitsEx(current_bit_offset, bits, value);
            bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
        }

        // clear any bits not written to
        if ((current_bit_offset % 8) > 0)
        {
            bits = (std::uint8_t)(8 - (current_bit_offset % 8));
            updateBitsEx(current_bit_offset, bits, 0);
        }

        // truncate data
        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) -= numberOfBitsToRemove;
        size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
        data.resize(newSize, 0);

        updateOffset(GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET), -std::int64_t(numberOfBitsToRemove));
        return true;
    }

    // go back before end of list
    cachedCombinedMagicalAttributes.clear();
    current_bit_offset -= PROPERTY_ID_NUM_BITS;
    size_t old_current_bit_offset = current_bit_offset;
    size_t numberOfBitsToAdd = PROPERTY_ID_NUM_BITS + 1;

    std::uint16_t id = ATTRIBUTE_ID_INDESTRUCTIBLE;
    const auto& stat = ItemHelpers::getItemStat(getVersion(), id);
    if (id != stat.id)
    {
        // corrupt file
        return false;
    }

    if (stat.saveBits != 1 || stat.saveParamBits != 0 || stat.encode != 0)
    {
        // corrupt file
        return false;
    }

    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // resize to fit new magical attribute
    auto old_item_end_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET);
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += numberOfBitsToAdd;
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // add indestructible attribute
    updateBitsEx(current_bit_offset, PROPERTY_ID_NUM_BITS, id);
    updateBitsEx(current_bit_offset, 1, 1);

    // now copy the remaining bits
    size_t bitsToCopy = old_item_end_bit_offset - old_current_bit_offset;
    std::uint32_t value = 0;
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    updateOffset(old_current_bit_offset, std::int64_t(numberOfBitsToAdd));
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::upgradeTier(const CharStats& cs)
{
    const auto& itemType = getItemTypeHelper();
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happend
        return false;
    }

    if (itemType.codes.size() <= 1)
    {
        // nothing to upgrade
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::UNIQUE:
    case EnumItemQuality::SET:
        return false;
    }

    // Restrict upgrade by character level only unless item is equipped
    bool bIsEquipped = (getLocation() == EnumItemLocation::EQUIPPED) ? true : false;
    ItemRequirements req = itemType.req;
    if ((req.Level > cs.Level) || (bIsEquipped && 
         ((req.Strength > cs.Strength) || (req.Dexterity > cs.Dexterity))))
    {
        // requirement are too high for character level
        return false;
    }

    std::string sCode = itemType.codes.back();
    if (sCode == itemType.code)
    {
        // already at highest level
        return false;
    }

    {
        const auto& newItemType = ItemHelpers::getItemTypeHelper(sCode);
        if (&newItemType == &ItemHelpers::getInvalidItemTypeHelper())
        {
            // should not happend
            return false;
        }

        req = newItemType.req;
    }

    if ((req.Level > cs.Level) || (bIsEquipped &&
        ((req.Strength > cs.Strength) || (req.Dexterity > cs.Dexterity))))
    {
        // requirement are too high for character level, try a lower code
        if (itemType.codes.size() <= 2)
        {
            // nothing to upgrade
            return false;
        }

        sCode = itemType.codes[1];
        if (sCode == itemType.code)
        {
            // already at highest level
            return false;
        }

        {
            const auto& newItemType = ItemHelpers::getItemTypeHelper(sCode);
            if (&newItemType == &ItemHelpers::getInvalidItemTypeHelper())
            {
                // should not happend
                return false;
            }

            req = newItemType.req;
        }

        if ((req.Level > cs.Level) || (bIsEquipped &&
            ((req.Strength > cs.Strength) || (req.Dexterity > cs.Dexterity))))
        {
            // requirement are too high for character level
            return false;
        }
    }

    // we have the upgraded code
    const auto& upgradedItemType = ItemHelpers::getItemTypeHelper(sCode);
    if (&upgradedItemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happend
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);

    auto strcode = ItemCodeStringConverter(sCode);
    std::uint64_t code = 0;
    std::uint8_t numBitsSet = 0;
    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        numBitsSet = 10;
        code = upgradedItemType.code_v100;
        if (code >= MAXUINT16)
        {
            return false;
        }
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        numBitsSet = 30;
        code = *((std::uint32_t*)strcode.data());
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        numBitsSet = 32;
        code = *((std::uint32_t*)strcode.data());
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        ItemHelpers::encodeResurrectedItem(strcode, code, numBitsSet);
        break;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        if (!updateBits64(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET), numBitsSet, code))
        {
            swap(origItem);
            return false;
        }
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        if(!updateResurrectedItemCode(code, numBitsSet))
        {
            swap(origItem);
            return false;
        }
        break;
    }

    if (!isIndestructible() && (itemType.durability.Max != 0))
    {
        auto durability = itemType.durability;
        durability.Current = durability.Max;
        if (!setDurability(durability))
        {
            // should not happen
            swap(origItem);
            return false;
        }
    }

    if (isArmor())
    {
        if (!setMaxDefenseRating())
        {
            // should not happen
            swap(origItem);
            return false;
        }
    }

    cachedCombinedMagicalAttributes.clear();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canEquipWith(const d2ce::Item& item, d2ce::EnumCharClass charClass) const
{
    if (&item == this)
    {
        return true;
    }

    // assume items are going to be placed in the hands
    if (canEquip(d2ce::EnumEquippedId::RIGHT_ARM, charClass))
    {
        if (!item.canEquip(d2ce::EnumEquippedId::LEFT_ARM, charClass))
        {
            return false;
        }
    }
    else if (canEquip(d2ce::EnumEquippedId::LEFT_ARM, charClass))
    {
        if (!item.canEquip(d2ce::EnumEquippedId::RIGHT_ARM, charClass))
        {
            return false;
        }
    }

    const auto& itemType = getItemTypeHelper();
    const auto& otherItemType = item.getItemTypeHelper();
    if (itemType.isSecondHand())
    {
        if (otherItemType.isSecondHand() || otherItemType.isTwoHandedWeapon())
        {
            // no room for two second hand items
            return false;
        }

        // is it a valid second hand item?
        if (itemType.isMissile())
        {
            if (!otherItemType.quiverCode.has_value())
            {
                // no room for quivers
                return false;
            }

            if (!itemType.hasCategoryCode(otherItemType.quiverCode.value()))
            {
                return false;
            }
        }
    }
    else if (otherItemType.isSecondHand())
    {
        if (itemType.isTwoHandedWeapon())
        {
            // no room for a second hand item
            return false;
        }

        // is it a valid second hand item?
        if (otherItemType.isMissile())
        {
            if (!itemType.quiverCode.has_value())
            {
                // no room for quivers
                return false;
            }

            if (!otherItemType.hasCategoryCode(itemType.quiverCode.value()))
            {
                return false;
            }
        }
    }
    else if (itemType.isMissileWeapon() || otherItemType.isMissileWeapon())
    {
        // No second hand item, so we have no room for the missile weapon
        return false;
    }
    else if (itemType.isTwoHandedWeapon())
    {
        switch (charClass)
        {
        case d2ce::EnumCharClass::Barbarian:
            if (!itemType.isOneOrTwoHandedWeapon() || !otherItemType.isOneOrTwoHandedWeapon())
            {
                // no room for two-handed weapon
                return false;
            }
            break;

        default:
            // no room for two-handed weapon
            return false;
        }
    }
    else if (otherItemType.isTwoHandedWeapon())
    {
        switch (charClass)
        {
        case d2ce::EnumCharClass::Barbarian:
            if (!itemType.isOneOrTwoHandedWeapon() || !otherItemType.isOneOrTwoHandedWeapon())
            {
                // no room for two-handed weapon
                return false;
            }
            break;

        default:
            // no room for two-handed weapon
            return false;
        }
    }
    else if (itemType.isWeapon())
    {
        switch (charClass)
        {
        case d2ce::EnumCharClass::Barbarian:
            break;

        case d2ce::EnumCharClass::Assassin:
            if (!itemType.isClassSpecific() || !otherItemType.isClassSpecific())
            {
                return false;
            }
            break;

        default:
            // no room for two weapons
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canEquipWith(const d2ce::Item& item, d2ce::EnumCharClass charClass, const CharStats& cs) const
{
    if (&item == this)
    {
        return true;
    }

    // assume items are going to be placed in the hands
    if (!item.canEquip(d2ce::EnumEquippedId::LEFT_ARM, charClass, cs) &&
        !item.canEquip(d2ce::EnumEquippedId::RIGHT_ARM, charClass, cs))
    {
        return false;
    }

    return canEquipWith(item, charClass);
}
//---------------------------------------------------------------------------
bool d2ce::Item::canSocketItem(const d2ce::Item& socketFiller) const
{
    if (!isSocketed() || !socketFiller.isSocketFiller())
    {
        return false;
    }

    // any free slots?
    if (getSocketedItemCount() >= getMaxSocketedCount())
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::canSocketItem(const d2ce::Item& socketFiller, std::uint32_t level) const
{
    if (!canSocketItem(socketFiller))
    {
        return false;
    }

    if (getLocation() != EnumItemLocation::EQUIPPED)
    {
        // no need to check level
        return true;
    }

    // Do we meet requirements?
    d2ce::ItemRequirements existingItemReq;
    if (getRequirements(existingItemReq))
    {
        d2ce::ItemRequirements itemReq;
        if (socketFiller.getRequirements(itemReq))
        {
            if ((itemReq.Level > existingItemReq.Level) &&
                (itemReq.Level > level))
            {
                // the required level can't be met
                return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::removeSocketedItems()
{
    if (SocketedItems.empty())
    {
        return true;
    }

    SocketedItems.clear();
    updateSocketedItemCount();
    verifyRuneword();
    return SocketedItems.empty() ? true : false;
}
//---------------------------------------------------------------------------
std::vector<d2ce::RunewordType> d2ce::Item::getPossibleRunewords(bool bUseCurrentSocketCount, bool bExcludeServerOnly) const
{
    return ItemHelpers::getPossibleRunewords(*this, bUseCurrentSocketCount, bExcludeServerOnly);
}
//---------------------------------------------------------------------------
std::vector<d2ce::RunewordType> d2ce::Item::getPossibleRunewords(std::uint32_t level, bool bUseCurrentSocketCount, bool bExcludeServerOnly) const
{
    return ItemHelpers::getPossibleRunewords(*this, level, bUseCurrentSocketCount, bExcludeServerOnly);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setRuneword(std::uint16_t id)
{
    std::vector<d2ce::RunewordType> result;
    switch (getQuality())
    {
    case EnumItemQuality::MAGIC:
    case EnumItemQuality::SET:
    case EnumItemQuality::RARE:
    case EnumItemQuality::TEMPERED:
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::UNIQUE:
        // runewords do not work with these
        return false;
    }

    RunewordAttributes runeAttrib;
    if (isRuneword())
    {
        if (getRunewordAttributes(runeAttrib))
        {
            if (id == runeAttrib.Id)
            {
                // we are still ok
                return true;
            }
        }
    }

    auto& runeword = ItemHelpers::getRunewordFromId(id);
    if (runeword.id != id)
    {
        return false;
    }

    auto runeSocketCount = std::uint8_t(runeword.runeCodes.size());
    auto numSockets = std::max(getMaxSocketCount(), getDisplayedSocketCount());
    if (numSockets < runeSocketCount)
    {
        // not enough sockets
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);

    // remove any socketed items and change the item ID
    if (isSocketed() && !SocketedItems.empty())
    {
        removeSocketedItems();
    }
    else
    {
        randomizeId();
    }

    setSocketCount(runeSocketCount);

    // remove personalization
    removePersonalization();

    // might as well make it as good as possible by making it superior
    std::vector<MagicalAttribute> superiorAttribs;
    if (getPossibleSuperiorAttributes(superiorAttribs))
    {
        // remove conflicting attribs
        for (const auto& attrib : runeword.attribs)
        {
            for (auto iter = superiorAttribs.begin(); iter != superiorAttribs.end(); ++iter)
            {
                if (iter->Id == attrib.Id)
                {
                    // found a conflict
                    superiorAttribs.erase(iter);
                    break;
                }
            }
        }

        if (superiorAttribs.size() > 2)
        {
            const auto& itemType = getItemTypeHelper();
            if (itemType.isWeapon())
            {
                bool usesDurablility = !itemType.isMissileWeapon() && !itemType.isThrownWeapon() && !itemType.isMiscellaneous() && !isIndestructible();
                if (usesDurablility)
                {
                    ItemDurability durability;
                    if (!getDurability(durability) || durability.Max == 0)
                    {
                        usesDurablility = false;
                    }
                }

                if (usesDurablility)
                {
                    // remove the ac option
                    for (auto iter = superiorAttribs.begin(); iter != superiorAttribs.end(); ++iter)
                    {
                        if (iter->Name == "tohit")
                        {
                            superiorAttribs.erase(iter);
                            break;
                        }
                    }
                }

                if (superiorAttribs.size() > 2)
                {
                    for (auto iter = superiorAttribs.begin(); iter != superiorAttribs.end(); ++iter)
                    {
                        // remove the max damage option
                        if (iter->Name == "maxdamage")
                        {
                            superiorAttribs.erase(iter);
                            break;
                        }
                    }
                }
            }

            if (superiorAttribs.size() > 2)
            {
                superiorAttribs.resize(2);
            }
        }

        if (!superiorAttribs.empty())
        {
            if (makeNormal())
            {
                setMaxDefenseRating();
                makeSuperior(superiorAttribs);
            }
        }
    }

    auto version = getVersion();
    auto isExpansion = isExpansionGame();
    auto gemApplyType = getGemApplyType();
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    for (const auto& runeCode : runeword.runeCodes)
    {
        strcode = ItemCodeStringConverter(runeCode);
        std::list<Item> runeItems;
        ItemCreateParams createParams(version, strcode, isExpansion);
        runeItems.push_back(Item(createParams));
        auto iter = runeItems.begin();
        auto& runeItem = *iter;
        if (runeItem.data.empty() || !runeItem.isRune() || !canSocketItem(runeItem))
        {
            // invalid item
            swap(origItem);
            return false;
        }

        if (!runeItem.setLocation(d2ce::EnumItemLocation::SOCKET, std::uint16_t(SocketedItems.size()), 0ui16))
        {
            swap(origItem);
            return false;
        }

        const auto& runeItemType = runeItem.getItemTypeHelper();
        runeItemType.getSocketedMagicalAttributes(runeItem, runeItem.socketedMagicalAttributes, gemApplyType);
        SocketedItems.splice(SocketedItems.end(), runeItems, iter);
        updateSocketedItemCount();
    }

    cachedCombinedMagicalAttributes.clear();
    verifyRuneword();

    if (!isRuneword())
    {
        swap(origItem);
        return false;
    }

    if (!getRunewordAttributes(runeAttrib))
    {
        swap(origItem);
        return false;
    }

    if (id != runeAttrib.Id)
    {
        swap(origItem);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setRunewordPropertyList(const std::vector<MagicalAttribute>& attribs)
{
    if (attribs.empty() || !isRuneword() || (GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET) == 0))
    {
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);

    // truncate bonus list
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER);
    if ((GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) % 8) > 0)
    {
        auto bits = (std::uint8_t)(8 - (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) % 8));
        updateBits(GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET), bits, 0);
    }
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // add bonus magical attributes from runeword id
    GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER);
    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET);
    if (!updatePropertyList(current_bit_offset, attribs))
    {
        swap(origItem);
        return false;
    }

    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
    newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    cachedCombinedMagicalAttributes.clear();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getPossibleMagicalAffixes(std::vector<std::uint16_t>& prefixes, std::vector<std::uint16_t>& suffixes) const
{
    return ItemHelpers::getPossibleMagicalAffixes(*this, prefixes, suffixes);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getPossibleMagicalAffixes(std::map<std::uint16_t, std::vector<std::uint16_t>>& prefixes, std::map<std::uint16_t, std::vector<std::uint16_t>>& suffixes) const
{
    return ItemHelpers::getPossibleMagicalAffixes(*this, prefixes, suffixes);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getPossibleRareAffixes(std::vector<std::uint16_t>& prefixes, std::vector<std::uint16_t>& suffixes) const
{
    return ItemHelpers::getPossibleRareAffixes(*this, prefixes, suffixes);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getPossibleCraftingRecipies(std::vector<CraftRecipieType>& attribs) const
{
    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    createParams.createQualityOption = EnumItemQuality::CRAFTED;
    return ItemHelpers::getPossibleCraftingRecipies(createParams, attribs);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMagicalAffixes(const d2ce::MagicalAffixes& affixes)
{
    if (!affixes.isValid() || isSimpleItem() || (isSocketFiller() && !isJewel()) || (affixes.PrefixId == 0 && affixes.SuffixId == 0))
    {
        return false;
    }

    if (ItemVersion < EnumItemVersion::v107)
    {
        return setMagicalAffixesSimple(affixes);
    }

    d2ce::MagicalAffixes curAffixes;
    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
        if (isRuneword())
        {
            // magical affixes do not work with these
            return false;
        }

        return setMagicalAffixesSimple(affixes);

    case EnumItemQuality::RARE:     // must convert to NORMAL quality
    case EnumItemQuality::TEMPERED:
    case EnumItemQuality::CRAFTED:
        break;

    case EnumItemQuality::MAGIC:
        return setMagicalAffixesSimple(affixes);

    default:
        // magical affixes do not work with these
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);
    if (!isRing() && !isAmulet() && !isJewel()) // can't remove magical attributes
    {
        if (!makeNormal())
        {
            swap(origItem);
            return false;
        }
    }

    if (!setMagicalAffixesSimple(affixes))
    {
        swap(origItem);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMagicalAffixesSimple(const d2ce::MagicalAffixes& affixes)
{
    if (!affixes.isValid() || isSimpleItem() || (isSocketFiller() && !isJewel()) || (affixes.PrefixId == 0 && affixes.SuffixId == 0))
    {
        return false;
    }

    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        // you can't pick your affixes, they are generated by using the value of DWB
        switch (getQuality())
        {
        case EnumItemQuality::NORMAL:
            break;

        case EnumItemQuality::MAGIC:
            break;

        case EnumItemQuality::RARE:      // converting to MAGIC
        case EnumItemQuality::CRAFTED:
        case EnumItemQuality::TEMPERED:
            break;

        default:
            // magical affixes do not work with these
            return false;
        }

        size_t numBits = QUALITY_NUM_BITS;
        switch (ItemVersion)
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            if (isEar())
            {
                return false;
            }

            if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
            {
                GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_100;
            }

            numBits = QUALITY_NUM_BITS_100;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            if (isSimpleItem())
            {
                return false;
            }

            if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
            {
                GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_104;
            }

            numBits = QUALITY_NUM_BITS;
            break;

        default:
            return false;
        }

        // make a copy first
        d2ce::Item origItem(*this);

        std::uint32_t value = static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::MAGIC);
        if (!updateBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET), numBits, value))
        {
            swap(origItem);
            return false;
        }

        // We need to find a DWB code that is able to produce the prefix/suffix values selected
        auto dwb = getDWBCode();
        ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
        bool bRet = d2ce::ItemHelpers::findDWForMagicalAffixes(affixes, createParams, getLevel(), dwb);
        if (!setDWBCode(dwb))
        {
            swap(origItem);
            return false;
        }

        return bRet;
    }

    bool bMagical = false;
    d2ce::MagicalAffixes curAffixes;
    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
        if (isRuneword())
        {
            // magical affixes do not work with these
            return false;
        }
        break;

    case EnumItemQuality::RARE:     // converting to MAGIC
    case EnumItemQuality::TEMPERED:
    case EnumItemQuality::CRAFTED:
        if (!isRing() && !isAmulet() && !isJewel())
        {
            // must be converted to NORMAL quality for method to work
            return false;
        }
        break;

    case EnumItemQuality::MAGIC:
        if (!getMagicalAffixes(curAffixes))
        {
            return false;
        }

        if ((curAffixes.PrefixId == affixes.PrefixId) &&
            (curAffixes.SuffixId == affixes.SuffixId))
        {
            // already set to the correct value;
            return true;
        }

        bMagical = true;
        break;

    default:
        // magical affixes do not work with these
        return false;
    }

    // Check if the prefix and/or suffix is allowed
    std::vector<std::uint16_t> prefixes;
    std::vector<std::uint16_t> suffixes;
    if (!getPossibleMagicalAffixes(prefixes, suffixes))
    {
        return false;
    }

    bool bHasAffix = false;
    if (affixes.PrefixId != 0)
    {
        if (std::find(prefixes.begin(), prefixes.end(), affixes.PrefixId) == prefixes.end())
        {
            // invalid prefix
            return false;
        }
        bHasAffix = true;
    }

    if (affixes.SuffixId != 0)
    {
        if (std::find(suffixes.begin(), suffixes.end(), affixes.SuffixId) == suffixes.end())
        {
            // invalid suffix
            return false;
        }
        bHasAffix = true;
    }

    if (!bHasAffix)
    {
        // no affix
        return false;
    }

    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    createParams.createQualityOption = EnumItemQuality::MAGIC;
    createParams.magicalAffixes = affixes;

    std::vector<MagicalAttribute> attribs;
    if (!d2ce::ItemHelpers::getMagicAttribs(affixes, attribs, createParams, getGameVersion()))
    {
        return false;
    }

    // make a copy first
    size_t diff = 0;
    d2ce::Item origItem(*this);
    const auto& origData = origItem.data;

    auto numSockets = getSocketCount();
    auto origNumSockets = numSockets;
    if (d2ce::ItemHelpers::isAddSocketsMagicalPrefix(affixes.PrefixId))
    {
        // item_numsockets makes the item socketable
        for (auto iter = attribs.begin(); iter != attribs.end(); ++iter)
        {
            auto& attrib = *iter;
            if (attrib.Name == "item_numsockets")
            {
                if (!attrib.Values.empty())
                {
                    numSockets = std::uint8_t(numSockets + attrib.Values.front());
                }

                iter = attribs.erase(iter);
                break;
            }
        }
    }

    size_t current_bit_offset = 0;
    if (bMagical)
    {
        // already magical so just change affix values and list of attributes
        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
        std::uint32_t value = affixes.PrefixId;
        if (!updateBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
        {
            swap(origItem);
            return false;
        }

        value = affixes.SuffixId;
        if (!updateBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
        {
            swap(origItem);
            return false;
        }

        // you will only have magical list at the end, so truncate it to make room for new list
        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET);
        size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
        data.resize(newSize, 0);
    }
    else
    {
        if (isRing() || isAmulet() || isJewel())
        {
            d2ce::Item newItem(createParams);
            if (newItem.data.empty())
            {
                return false;
            }

            swap(newItem);
            return true;
        }

        // complex change: make item have magical quality
        if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
        {
            GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET);
        std::uint32_t value = static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::MAGIC);
        if (!updateBitsEx(current_bit_offset, QUALITY_NUM_BITS, value))
        {
            swap(origItem);
            return false;
        }

        // make room for affixes
        diff = MAGICAL_AFFIX_NUM_BITS * 2;
        current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
        size_t old_current_bit_offset = current_bit_offset;
        size_t bitsToCopy = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) - old_current_bit_offset;
        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += diff; // add new attribs

        size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
        data.resize(newSize, 0);

        // update affixes
        value = affixes.PrefixId;
        if (!updateBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
        {
            swap(origItem);
            return false;
        }

        value = affixes.SuffixId;
        if (!updateBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
        {
            swap(origItem);
            return false;
        }

        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);

        // now copy the remaining bits
        size_t valueBitSize = sizeof(value) * 8;
        std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
        while (bitsToCopy > 0)
        {
            bitsToCopy -= bits;
            value = readtemp_bits(origData, old_current_bit_offset, bits);
            old_current_bit_offset += bits;
            updateBitsEx(current_bit_offset, bits, value);
            bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
        }

        updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER), diff);
    }

    // write out new data
    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET);
    if (!updatePropertyList(current_bit_offset, attribs))
    {
        swap(origItem);
        return false;
    }

    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
    size_t diff2 = current_bit_offset - origItem.bitOffsets[static_cast<std::underlying_type_t<ItemOffsets>>(ItemOffsets::ITEM_END_BIT_OFFSET)] - diff;
    updateOffset(GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET), diff2);

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        std::uint8_t bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    if (numSockets != origNumSockets)
    {
        setSocketCount(numSockets);
    }

    fixDurability();
    cachedCombinedMagicalAttributes.clear();
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMagicalPropertyList(const std::vector<MagicalAttribute>& attribs)
{
    if (attribs.empty() || isSimpleItem() || (isSocketFiller() && !isJewel()) || (ItemVersion < EnumItemVersion::v107))
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::UNKNOWN:
        return false; // no posibility of magical properties

    default:
        break;
    }

    // make a copy first
    d2ce::Item origItem(*this);
    const auto& origData = origItem.data;

    size_t old_current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SET_BONUS_PROPS_BIT_OFFSET_MARKER);
    size_t bitsToCopy = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) - GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SET_BONUS_PROPS_BIT_OFFSET_MARKER);

    // write out new data
    size_t old_magic_bit_offset = GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET);
    size_t current_bit_offset = old_magic_bit_offset;
    if (!updatePropertyList(current_bit_offset, attribs))
    {
        swap(origItem);
        return false;
    }

    ptrdiff_t diff = ptrdiff_t(current_bit_offset) - ptrdiff_t(old_magic_bit_offset);
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset + bitsToCopy;
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // now copy the remaining bits
    std::uint32_t value = 0;
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(origData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    updateOffset(GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET), diff);

    cachedCombinedMagicalAttributes.clear();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setSetBonusPropertyLists(const std::vector<std::vector<MagicalAttribute>>& attribs)
{
    if (attribs.empty() || (GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET) == 0) || 
        (GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SET_BONUS_PROPS_BIT_OFFSET_MARKER) == 0) ||
        (ItemVersion < EnumItemVersion::v107))
    {
        return false;
    }

    for (const auto& attrib : attribs)
    {
        if (attrib.empty())
        {
            return false;
        }
    }

    size_t numBonusLists = 0;
    std::uint16_t setBonusBits = (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET), 5);
    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        for (size_t i = 0; i < 5 && setBonusBits > 0; ++i, setBonusBits >>= 1)
        {
            if ((setBonusBits & 0x01) != 0)
            {
                ++numBonusLists;
            }
        }
    }

    if (numBonusLists > attribs.size())
    {
        // missing some attributes
        return false;
    }


    // make a copy first
    d2ce::Item origItem(*this);
    const auto& origData = origItem.data;

    size_t old_current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER);
    size_t bitsToCopy = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) - GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER);

    // write out new data
    size_t old_set_bit_offset = GET_BIT_OFFSET(ItemOffsets::SET_BONUS_PROPS_BIT_OFFSET);
    size_t current_bit_offset = old_set_bit_offset;

    auto attribIter = attribs.begin();
    for (size_t i = 0; i < numBonusLists; ++i, ++attribIter)
    {
        if (!updatePropertyList(current_bit_offset, *attribIter))
        {
            swap(origItem);
            return false;
        }
    }

    ptrdiff_t diff = ptrdiff_t(current_bit_offset) - ptrdiff_t(old_set_bit_offset);
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset + bitsToCopy;
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // now copy the remaining bits
    std::uint32_t value = 0;
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(origData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SET_BONUS_PROPS_BIT_OFFSET_MARKER), diff);

    cachedCombinedMagicalAttributes.clear();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getPossibleSuperiorAttributes(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();
    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        // you can't pick your affixes, they are generated by using the value of DWB
        return false;
    }

    bool bIsWeapon = isWeapon();
    bool bIsArmor = isArmor();
    if (isSimpleItem() || isSocketFiller() || (!bIsWeapon && !bIsArmor))
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::NORMAL:
    case EnumItemQuality::SUPERIOR:
        break;

    default:
        // not able to make it superior
        return false;
    }

    return ItemHelpers::getPossibleSuperiorAttributes(*this, attribs);
}
//---------------------------------------------------------------------------
bool d2ce::Item::makeSuperior(const std::vector<MagicalAttribute>& attribs)
{
    // Check for valid attributes
    if (attribs.empty() || attribs.size() > 2)
    {
        // need one or two attributes
        return false;
    }

    if (attribs.size() == 2)
    {
        if (attribs[0].Id == attribs[1].Id)
        {
            return false;
        }

        if (attribs[0].Name == "item_maxdamage_percent")
        {
            if (attribs[1].Name == "maxdamage")
            {
                return false;
            }
        }
        else if (attribs[0].Name == "maxdamage")
        {
            if (attribs[1].Name == "item_maxdamage_percent")
            {
                return false;
            }
        }
    }

    std::vector<MagicalAttribute> possibleAttribs;
    if (!getPossibleSuperiorAttributes(possibleAttribs))
    {
        return false;
    }

    for (const auto& attrib : attribs)
    {
        bool bFound = false;
        for (const auto& possibleAttrib : possibleAttribs)
        {
            if (possibleAttrib.Id == attrib.Id)
            {
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            return false;
        }
    }

    // first make the item normal
    if (!makeNormal())
    {
        return false;
    }

    if (isArmor())
    {
        setMaxDefenseRating();
    }

    // make a copy first
    d2ce::Item origItem(*this);
    const auto& origData = origItem.data;

    // complex change: make item have magical quality
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
    {
        GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET;
    }

    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET);
    std::uint32_t value = static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::SUPERIOR);
    if (!updateBitsEx(current_bit_offset, QUALITY_NUM_BITS, value))
    {
        swap(origItem);
        return false;
    }

    // make room for superior bits
    size_t diff = INFERIOR_SUPERIOR_ID_NUM_BITS;
    current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
    size_t old_current_bit_offset = current_bit_offset;
    size_t bitsToCopy = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) - old_current_bit_offset;
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += diff; // add new superior bits

    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // update Superior quality value
    current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
    value = 1;
    if (!updateBitsEx(current_bit_offset, INFERIOR_SUPERIOR_ID_NUM_BITS, value))
    {
        swap(origItem);
        return false;
    }

    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);

    // now copy the remaining bits
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(origData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER), diff);

    // write out new data
    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET);
    if (!updatePropertyList(current_bit_offset, attribs))
    {
        swap(origItem);
        return false;
    }

    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
    size_t diff2 = current_bit_offset - origItem.bitOffsets[static_cast<std::underlying_type_t<ItemOffsets>>(ItemOffsets::ITEM_END_BIT_OFFSET)] - diff;
    updateOffset(GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET), diff2);

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    fixDurability();
    cachedCombinedMagicalAttributes.clear();
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::makeSuperior()
{
    if (getQuality() == EnumItemQuality::SUPERIOR)
    {
        // nothing to do
        return false; // so we don't count it as a change
    }

    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        // make a copy first
        d2ce::Item origItem(*this);

        size_t numBits = QUALITY_NUM_BITS;
        switch (ItemVersion)
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            if (isEar())
            {
                return false;
            }

            if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
            {
                GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_100;
            }

            numBits = QUALITY_NUM_BITS_100;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            if (isSimpleItem())
            {
                return false;
            }

            if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
            {
                GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_104;
            }

            numBits = QUALITY_NUM_BITS;
            break;

        default:
            return false;
        }

        std::uint32_t value = static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::SUPERIOR);
        if (!updateBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET), numBits, value))
        {
            swap(origItem);
            return false;
        }

        cachedCombinedMagicalAttributes.clear();
        fixDurability();
        return true;
    }

    std::vector<MagicalAttribute> attribs;
    if (!getPossibleSuperiorAttributes(attribs))
    {
        return false;
    }

    if (attribs.size() > 2)
    {
        const auto& itemType = getItemTypeHelper();
        if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
        {
            // should not happend
            return false;
        }

        if (itemType.isWeapon())
        {
            auto iter = attribs.begin();
            auto iter_end = attribs.end();
            for (; iter != iter_end; ++iter)
            {
                // remove the max damage option
                if (iter->Name == "maxdamage")
                {
                    attribs.erase(iter);
                    break;
                }
            }

            if (attribs.size() > 2)
            {
                bool usesDurablility = !itemType.isMissileWeapon() && !itemType.isThrownWeapon() && !itemType.isMiscellaneous() && !isIndestructible();
                if (usesDurablility)
                {
                    ItemDurability durability;
                    if (!getDurability(durability) || durability.Max == 0)
                    {
                        usesDurablility = false;
                    }
                }

                if (usesDurablility)
                {
                    // remove the ac option
                    iter = attribs.begin();
                    iter_end = attribs.end();
                    for (; iter != iter_end; ++iter)
                    {
                        if (iter->Name == "tohit")
                        {
                            attribs.erase(iter);
                            break;
                        }
                    }
                }
            }
        }

        if (attribs.size() > 2)
        {
            attribs.resize(2);
        }
    }

    return makeSuperior(attribs);
}
//---------------------------------------------------------------------------
bool d2ce::Item::makeNormal()
{
    if (isRuneword())
    {
        return false;
    }

    bool bIsInferior = false;
    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
        return true;

    case EnumItemQuality::INFERIOR:
        bIsInferior = true;
        break;

    case EnumItemQuality::SUPERIOR:
        break;

    case EnumItemQuality::MAGIC:
    case EnumItemQuality::RARE:
    case EnumItemQuality::TEMPERED:
    case EnumItemQuality::CRAFTED:
        if (isSimpleItem() || isSocketFiller())
        {
            return true; // doest't have magical affixes
        }

        if (isRing() || isAmulet() || isJewel())
        {
            // can't remove magical attributes
            return false;
        }
        break;

    default:
        // not able to make it normal
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);
    const auto& origData = origItem.data;

    // complex change: make item have normal quality
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
    {
        GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET;
    }

    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET);
    std::uint32_t value = static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::NORMAL);
    if (!updateBitsEx(current_bit_offset, QUALITY_NUM_BITS, value))
    {
        swap(origItem);
        return false;
    }

    if (bIsInferior)
    {
        // put back the normal attributes
        const auto& itemType = getItemTypeHelper();
        if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
        {
            // should not happen
            swap(origItem);
            return false;
        }

        if (!isIndestructible() && (itemType.durability.Max != 0))
        {
            auto durability = itemType.durability;
            durability.Current = durability.Max;
            if (!setDurability(durability))
            {
                // should not happen
                swap(origItem);
                return false;
            }
        }

        if (isArmor())
        {
            if (!setMaxDefenseRating())
            {
                // should not happen
                swap(origItem);
                return false;
            }
        }

        cachedCombinedMagicalAttributes.clear();
        magic_affixes_v100.clear();
        rare_affixes_v100.clear();
        return true;
    }

    if (isArmor())
    {
        setMaxDefenseRating();
    }

    // remove quality and magical attributes
    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
    current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
    size_t old_current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER);
    size_t bitsToCopy = GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET) - old_current_bit_offset;
    size_t diff = old_current_bit_offset - current_bit_offset;
    size_t diff2 = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) - (GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET) + PROPERTY_ID_NUM_BITS);
    if (diff == 0 && diff2 == 0)
    {
        // we have have nothing to remove, should not happen
        return true;
    }

    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) -= (diff + diff2);
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    if (diff > 0) // should always be the case
    {
        // now copy the remaining bits
        size_t valueBitSize = sizeof(value) * 8;
        std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
        while (bitsToCopy > 0)
        {
            bitsToCopy -= bits;
            value = readtemp_bits(origData, old_current_bit_offset, bits);
            old_current_bit_offset += bits;
            updateBitsEx(current_bit_offset, bits, value);
            bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
        }
    }

    // clear property list
    setBits(current_bit_offset, PROPERTY_ID_NUM_BITS, 0x1FF);

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        std::uint8_t bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    if (diff > 0)
    {
        updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER), -std::int64_t(diff));
    }

    if (diff2 > 0)
    {
        updateOffset(GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET), -std::int64_t(diff2));
    }

    fixDurability();
    cachedCombinedMagicalAttributes.clear();
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::makeMagical()
{
    if (isRuneword())
    {
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);
    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
        break;

    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::SUPERIOR:
        if (!makeNormal())
        {
            swap(origItem);
            return false;
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::TEMPERED:
    case EnumItemQuality::CRAFTED:
        if (!isRing() && !isAmulet() && !isJewel())
        {
            if (!makeNormal())
            {
                swap(origItem);
                return false;
            }
        }
        break;

    case EnumItemQuality::MAGIC:
        return true;

    default:
        // not able to make it normal
        return false;
    }

    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    MagicalCachev100 generated_magic_affixes;
    if (!ItemHelpers::generateMagicalAffixes(generated_magic_affixes, createParams, getLevel(), getDWBCode(), true))
    {
        swap(origItem);
        return false;
    }

    if (isRing() || isAmulet() || isJewel())
    {
        createParams.createQualityOption = EnumItemQuality::MAGIC;
        createParams.magicalAffixes = generated_magic_affixes.Affixes;
        d2ce::Item newItem(createParams);
        if (newItem.data.empty())
        {
            swap(origItem);
            return false;
        }

        swap(newItem);
        return true;
    }

    if (!setMagicalAffixes(generated_magic_affixes.Affixes))
    {
        swap(origItem);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::makeRare()
{
    if (isRuneword())
    {
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);
    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
        break;

    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::SUPERIOR:
        if (!makeNormal())
        {
            swap(origItem);
            return false;
        }
        break;

    case EnumItemQuality::MAGIC:
    case EnumItemQuality::CRAFTED:
        if (!isRing() && !isAmulet() && !isJewel())
        {
            if (!makeNormal())
            {
                swap(origItem);
                return false;
            }
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::TEMPERED:
        return true;

    default:
        // not able to make it normal
        return false;
    }

    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    createParams.createQualityOption = EnumItemQuality::RARE;
    RareOrCraftedCachev100 generated_rare_affixes;
    if (!ItemHelpers::generateRareOrCraftedAffixes(generated_rare_affixes, createParams, getLevel(), getDWBCode(), true))
    {
        swap(origItem);
        return false;
    }

    createParams.rareAttribs.Id = generated_rare_affixes.Id;
    createParams.rareAttribs.Name = generated_rare_affixes.Name;
    createParams.rareAttribs.Index = generated_rare_affixes.Index;
    createParams.rareAttribs.Id2 = generated_rare_affixes.Id2;
    createParams.rareAttribs.Name2 = generated_rare_affixes.Name2;
    createParams.rareAttribs.Index2 = generated_rare_affixes.Index2;
    for (auto& item : generated_rare_affixes.Affixes)
    {
        createParams.rareAttribs.Affixes.push_back(item.Affixes);
    }

    if (isRing() || isAmulet() || isJewel())
    {
        d2ce::Item newItem(createParams);
        if (newItem.data.empty())
        {
            swap(origItem);
            return false;
        }

        swap(newItem);
        return true;
    }

    if (!setRareOrCraftedAttributes(createParams.rareAttribs))
    {
        swap(origItem);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::makeCrafted(std::uint16_t& id)
{
    if (isRuneword())
    {
        return false;
    }

    std::vector<CraftRecipieType> recipies;
    if (!d2ce::Item::getPossibleCraftingRecipies(recipies) || recipies.empty())
    {
        return false;
    }

    auto recipieId = recipies.front().id;
    if (recipies.size() > 1)
    {
        if (id == MAXUINT16)
        {
            // pick a random id
            id = recipies[ItemHelpers::generarateRandomDW() % recipies.size()].id;
        }

        for (const auto& recipie : recipies)
        {
            if (recipie.id == id)
            {
                recipieId = id;
                break;
            }
        }
    }

    // make a copy first
    d2ce::Item origItem(*this);
    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
        break;

    case EnumItemQuality::INFERIOR:
    case EnumItemQuality::SUPERIOR:
        if (!makeNormal())
        {
            swap(origItem);
            return false;
        }
        break;

    case EnumItemQuality::MAGIC:
    case EnumItemQuality::RARE:
    case EnumItemQuality::TEMPERED:
        if (!isRing() && !isAmulet() && !isJewel())
        {
            if (!makeNormal())
            {
                swap(origItem);
                return false;
            }
        }
        break;

    default:
        // not able to make it normal
        return false;
    }

    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    createParams.createQualityOption = EnumItemQuality::CRAFTED;
    createParams.rareAttribs.CraftingRecipieId = recipieId;
    if (isRing() || isAmulet())
    {
        d2ce::Item newItem(createParams);
        if (newItem.data.empty())
        {
            swap(origItem);
            return false;
        }

        swap(newItem);
        return true;
    }
    else if (isJewel())
    {
        swap(origItem);
        return false;
    }

    RareOrCraftedCachev100 generated_rare_affixes;
    if (!ItemHelpers::generateRareOrCraftedAffixes(generated_rare_affixes, createParams, getLevel(), getDWBCode(), true))
    {
        swap(origItem);
        return false;
    }

    RareAttributes affixes;
    affixes.Id = generated_rare_affixes.Id;
    affixes.Name = generated_rare_affixes.Name;
    affixes.Index = generated_rare_affixes.Index;
    affixes.Id2 = generated_rare_affixes.Id2;
    affixes.Name2 = generated_rare_affixes.Name2;
    affixes.Index2 = generated_rare_affixes.Index2;
    affixes.CraftingRecipieId = createParams.rareAttribs.CraftingRecipieId;
    generated_rare_affixes.Affixes.pop_back(); // remove crafted attributes list
    for (auto& item : generated_rare_affixes.Affixes)
    {
        affixes.Affixes.push_back(item.Affixes);
    }

    if (!setRareOrCraftedAttributes(affixes))
    {
        swap(origItem);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::makeCrafted()
{
    std::uint16_t id = MAXUINT16;
    return makeCrafted(id);
}
//---------------------------------------------------------------------------
bool d2ce::Item::makeEthereal()
{
    if (isEthereal())
    {
        return true;
    }

    if (!isExpansionGame())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if (!result.isArmor() && !result.isWeapon())
    {
        return false;
    }

    return updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_ETHEREAL_FLAG_OFFSET, 1, 1);
}
//---------------------------------------------------------------------------
bool d2ce::Item::removeEthereal()
{
    if (!isEthereal())
    {
        return true;
    }

    if (!isExpansionGame())
    {
        return false;
    }

    const auto& result = getItemTypeHelper();
    if (&result == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    if (!result.isArmor() && !result.isWeapon())
    {
        return false;
    }

    return updateBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + IS_ETHEREAL_FLAG_OFFSET, 1, 0);
}

//---------------------------------------------------------------------------
bool d2ce::Item::setRareOrCraftedAttributes(const d2ce::RareAttributes& affixes)
{
    if (isSimpleItem() || (isSocketFiller() && !isJewel()))
    {
        return false;
    }

    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        return setRareOrCraftedAttributesSimple(affixes);
    }

    d2ce::RareAttributes curAffixes;
    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
        if (isRuneword())
        {
            // magical affixes do not work with these
            return false;
        }

        return setRareOrCraftedAttributesSimple(affixes);

    case EnumItemQuality::MAGIC:  // must convert to NORMAL quality
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::TEMPERED:
        break;

    default:
        // rare affixes do not work with these
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);
    if (!isRing() && !isAmulet() && !isJewel()) // can't remove magical attributes
    {
        if (!makeNormal())
        {
            swap(origItem);
            return false;
        }
    }

    if (!setRareOrCraftedAttributesSimple(affixes))
    {
        swap(origItem);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::setRareOrCraftedAttributesSimple(const d2ce::RareAttributes& affixes)
{
    if (!affixes.isValid() || isSimpleItem() || (isSocketFiller() && !isJewel()))
    {
        return false;
    }

    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        // you can't pick your affixes, they are generated by using the value of DWB
        switch (getQuality())
        {
        case EnumItemQuality::NORMAL:
            break;

        case EnumItemQuality::MAGIC: // converting to RARE
        case EnumItemQuality::RARE:
        case EnumItemQuality::CRAFTED:
        case EnumItemQuality::TEMPERED:
            break;

        default:
            // magical affixes do not work with these
            return false;
        }

        size_t numBits = QUALITY_NUM_BITS;
        switch (ItemVersion)
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            if (isEar())
            {
                return false;
            }

            if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
            {
                GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_100;
            }

            numBits = QUALITY_NUM_BITS_100;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            if (isSimpleItem())
            {
                return false;
            }

            if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
            {
                GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_104;
            }

            numBits = QUALITY_NUM_BITS;
            break;

        default:
            return false;
        }

        // make a copy first
        d2ce::Item origItem(*this);
        std::uint32_t value = static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::RARE);
        if (!updateBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET), numBits, value))
        {
            swap(origItem);
            return false;
        }

        // We need to find a DWB code that is able to produce the prefix/suffix values selected
        auto dwb = getDWBCode();
        ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
        bool bRet = d2ce::ItemHelpers::findDWForRareOrCraftedAffixes(affixes, createParams, getLevel(), dwb);
        if (!setDWBCode(dwb))
        {
            swap(origItem);
            return false;
        }

        return bRet;
    }

    bool bIsCraft = false;
    if (isJewel())
    {
        // Before 1.09, rare jewels could have up to three prefixes and three suffixes
        if (ItemVersion >= EnumItemVersion::v109)
        {
            if (affixes.getAffixCount() > 4)
            {
                // Post-1.09, Rare jewels can have up to 4 total affixes
                return false;
            }
        }
    }
    else
    {
        bIsCraft = (isExpansionGame() && affixes.CraftingRecipieId != MAXUINT16) ? true : false;
        if (bIsCraft && affixes.getAffixCount() > 4)
        {
            return false;
        }
    }

    d2ce::RareAttributes curAffixes;
    switch (getQuality())
    {
    case EnumItemQuality::NORMAL:
        if (isRuneword())
        {
            // magical affixes do not work with these
            return false;
        }
        break;

    case EnumItemQuality::MAGIC:
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::TEMPERED:
        if (!isRing() && !isAmulet() && !isJewel())
        {
            // must be converted to NORMAL quality for method to work
            return false;
        }
        break;

    default:
        // rare affixes do not work with these
        return false;
    }

    // Check if the prefix and/or suffix is allowed
    std::vector<std::uint16_t> prefixes;
    std::vector<std::uint16_t> suffixes;
    if (!getPossibleRareAffixes(prefixes, suffixes))
    {
        return false;
    }

    bool bHasAffix = false;
    if (affixes.Id != 0)
    {
        if (std::find(prefixes.begin(), prefixes.end(), affixes.Id) == prefixes.end())
        {
            // invalid rare prefix
            return false;
        }
        bHasAffix = true;
    }

    if (affixes.Id2 != 0)
    {
        if (std::find(suffixes.begin(), suffixes.end(), affixes.Id2) == suffixes.end())
        {
            // invalid rare suffix
            return false;
        }
        bHasAffix = true;
    }

    if (!bHasAffix)
    {
        // no affix
        return false;
    }

    if (!getPossibleMagicalAffixes(prefixes, suffixes))
    {
        return false;
    }

    size_t diff = RARE_CRAFTED_ID_NUM_BITS * 2 + size_t((3ui64 - affixes.Affixes.size()) * 2ui64);
    for (auto& affix : affixes.Affixes)
    {
        bHasAffix = false;
        diff += 1;
        if (affix.PrefixId != 0)
        {
            diff += MAGICAL_AFFIX_NUM_BITS;
            if (std::find(prefixes.begin(), prefixes.end(), affix.PrefixId) == prefixes.end())
            {
                // invalid prefix
                return false;
            }
            bHasAffix = true;
        }

        diff += 1;
        if (affix.SuffixId != 0)
        {
            diff += MAGICAL_AFFIX_NUM_BITS;
            if (std::find(suffixes.begin(), suffixes.end(), affix.SuffixId) == suffixes.end())
            {
                // invalid suffix
                return false;
            }
            bHasAffix = true;
        }

        if (!bHasAffix)
        {
            // no affix
            return false;
        }
    }

    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    createParams.rareAttribs = affixes;
    if (bIsCraft)
    {
        createParams.createQualityOption = EnumItemQuality::CRAFTED;
    }
    else
    {
        createParams.createQualityOption = EnumItemQuality::RARE;
    }

    if (isRing() || isAmulet() || isJewel())
    {
        d2ce::Item newItem(createParams);
        if (newItem.data.empty())
        {
            return false;
        }

        swap(newItem);
        return true;
    }

    std::vector<MagicalAttribute> attribs;
    if (!d2ce::ItemHelpers::getRareOrCraftedAttribs(affixes, attribs, createParams, getGameVersion()))
    {
        return false;
    }

    // make a copy first
    d2ce::Item origItem(*this);
    const auto& origData = origItem.data;

    if (bIsCraft)
    {
        if (!removeEthereal())
        {
            swap(origItem);
            return false;
        }
    }

    // complex change: make item have RARE ore CRAFTED quality
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) == 0)
    {
        GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET;
    }

    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET);
    std::uint32_t value = static_cast<std::underlying_type_t<EnumItemQuality>>(bIsCraft ? EnumItemQuality::CRAFTED : EnumItemQuality::RARE);
    if (!updateBitsEx(current_bit_offset, QUALITY_NUM_BITS, value))
    {
        swap(origItem);
        return false;
    }

    // make room for affixes
    current_bit_offset = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
    size_t old_current_bit_offset = current_bit_offset;
    size_t bitsToCopy = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) - old_current_bit_offset;
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += diff; // add new attribs

    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);

    // update affixes
    value = affixes.Id;
    if (!updateBitsEx(current_bit_offset, RARE_CRAFTED_ID_NUM_BITS, value))
    {
        swap(origItem);
        return false;
    }

    value = affixes.Id2;
    if (!updateBitsEx(current_bit_offset, RARE_CRAFTED_ID_NUM_BITS, value))
    {
        swap(origItem);
        return false;
    }

    for (auto& affix : affixes.Affixes)
    {
        value = affix.PrefixId;
        if (!updateBitsEx(current_bit_offset, 1, (value == 0) ? 0 : 1))
        {
            swap(origItem);
            return false;
        }

        if (value != 0)
        {
            if (!updateBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
            {
                swap(origItem);
                return false;
            }
        }

        value = affix.SuffixId;
        if (!updateBitsEx(current_bit_offset, 1, (value == 0) ? 0 : 1))
        {
            swap(origItem);
            return false;
        }

        if (value != 0)
        {
            if (!updateBitsEx(current_bit_offset, MAGICAL_AFFIX_NUM_BITS, value))
            {
                swap(origItem);
                return false;
            }
        }
    }

    // Fill in any missing affix missing flags
    value = 0;
    size_t valueBitSize = size_t((3ui64 - affixes.Affixes.size()) * 2ui64);
    if (valueBitSize > 0)
    {
        if (!updateBitsEx(current_bit_offset, valueBitSize, value))
        {
            swap(origItem);
            return false;
        }
    }

    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);

    // now copy the remaining bits
    valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(origData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    updateOffset(GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER), diff);

    // write out new data
    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET);
    if (!updatePropertyList(current_bit_offset, attribs))
    {
        swap(origItem);
        return false;
    }

    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
    size_t diff2 = current_bit_offset - origItem.bitOffsets[static_cast<std::underlying_type_t<ItemOffsets>>(ItemOffsets::ITEM_END_BIT_OFFSET)] - diff;
    updateOffset(GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET), diff2);

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    fixDurability();
    cachedCombinedMagicalAttributes.clear();
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::exportItem(const std::filesystem::path& path) const
{
    if (data.empty())
    {
        return false;
    }

    bool directExport = false;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        directExport = true;
        break;

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        if (isExpansionGame())
        {
            directExport = true;
        }
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
        break;

    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        if (isExpansionGame())
        {
            if (isPersonalized())
            {
                // personalization has change, make sure they are all 7 bit characters
                for (std::uint8_t singleChar : getPersonalizedName())
                {
                    if (singleChar > 0x7F)
                    {
                        directExport = true;
                        break;
                    }
                }
            }
            else if (isEar())
            {
                // player name has change, make sure they are all 7 bit characters
                EarAttributes earAttrib;
                if (getEarAttributes(earAttrib))
                {
                    for (std::uint8_t singleChar : earAttrib.getName())
                    {
                        if (singleChar > 0x7F)
                        {
                            directExport = true;
                            break;
                        }
                    }
                }
            }
        }
        break;
    }

    if (directExport)
    {
        // we can directly write out the data
        std::FILE* charfile = NULL;
        _wfopen_s(&charfile, path.wstring().c_str(), L"wb");
        std::rewind(charfile);
        bool ret = writeItem(charfile);
        std::fclose(charfile);
        return ret;
    }

    // convert to json first
    Json::Value itemRoot;
    asJson(itemRoot, 1ui32, true);

    // modify it to work for the item version we are using
    auto convertToVersion = EnumItemVersion::v110;
    if (ItemVersion >= EnumItemVersion::v120)
    {
        if (isPersonalized())
        {
            // personalization has change, make sure they are all 7 bit characters
            for (std::uint8_t singleChar : getPersonalizedName())
            {
                if (singleChar > 0x7F)
                {
                    convertToVersion = EnumItemVersion::v120;
                    break;
                }
            }
        }
        else if (isEar())
        {
            // player name has change, make sure they are all 7 bit characters
            EarAttributes earAttrib;
            if (getEarAttributes(earAttrib))
            {
                for (std::uint8_t singleChar : earAttrib.getName())
                {
                    if (singleChar > 0x7F)
                    {
                        convertToVersion = EnumItemVersion::v120;
                        break;
                    }
                }
            }
        }
    }

    bool isExpansion = true;
    switch (convertToVersion)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
        isExpansion = false;
        itemRoot["Version"] = "0";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;

    case EnumItemVersion::v104: // v1.04 - v1.06 item
        isExpansion = false;
        itemRoot["Version"] = "0";
        itemRoot["IsV104"] = true;
        break;

    case EnumItemVersion::v107: // v1.07 item
        itemRoot["Version"] = "0";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;

    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
        itemRoot["Version"] = "100";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;

    case EnumItemVersion::v110: // v1.10 - v1.14d item 
        itemRoot["Version"] = "101"; 
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        itemRoot["Version"] = "101";
        if (itemRoot.isMember("IsV104"))
        {
            itemRoot["IsV104"] = false;
        }
        break;
    }

    if (!isExpansion)
    {
        if (itemRoot.isMember("IsRuneword"))
        {
            itemRoot["IsRuneword"] = false;
        }
    }

    // read item into correct version for export
    Item exportItem;
    exportItem.readItem(itemRoot, true, convertToVersion, isExpansion);
    return exportItem.exportItem(path);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getMonsterId() const
{
    if (!isBodyPart() || GET_BIT_OFFSET(ItemOffsets::BODY_PART_BIT_OFFSET) == 0)
    {
        return 0;
    }

    return (std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::BODY_PART_BIT_OFFSET), MONSTER_ID_NUM_BITS);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setMonsterId(std::uint16_t id)
{
    if (!isBodyPart() || GET_BIT_OFFSET(ItemOffsets::BODY_PART_BIT_OFFSET) == 0)
    {
        return false;
    }

    if (ItemHelpers::getMonsterNameFromId(id).empty())
    {
        // invalid monster name
        return false;
    }

    return updateBits(GET_BIT_OFFSET(ItemOffsets::BODY_PART_BIT_OFFSET), MONSTER_ID_NUM_BITS, id);
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedItemName() const
{
    std::string strValue;
    std::string strFormat;
    std::stringstream ss;
    EarAttributes earAttrib;
    if (getEarAttributes(earAttrib))
    {
        strValue = earAttrib.getNameAsString();
        if (strValue.back() != 's')
        {
            LocalizationHelpers::GetStringTxtValue("PlayerNameOnItemstring", strFormat, "%s's");
        }
        else
        {
            LocalizationHelpers::GetStringTxtValue("PlayerNameOnItemstringX", strFormat, "%s'");
        }
        strValue = d2ce::LocalizationHelpers::string_format(strFormat, strValue.c_str());
        LocalizationHelpers::GetStringTxtValue("PlayerBodyPartFormat", strFormat, "%0 %1");
        std::string earStr;
        LocalizationHelpers::GetStringTxtValue("ear", earStr, "Ear");
        ss << d2ce::LocalizationHelpers::string_formatDiablo(strFormat, strValue, earStr);
        ss << "\n" << earAttrib.getClassName() << "\n";
        LocalizationHelpers::GetStringTxtValue("strChatLevel", strFormat, "Level %d");
        ss << d2ce::LocalizationHelpers::string_format(strFormat, earAttrib.getLevel());
        return ss.str();
    }

    const auto& itemType = getItemTypeHelper();
    if (!isIdentified())
    {
        ss << itemType.name << "\n";
        LocalizationHelpers::GetStringTxtValue("ItemStats1b", strValue, "Unidentified");
        ss << strValue;
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
        if (personalizedName.back() != 's')
        {
            LocalizationHelpers::GetStringTxtValue("PlayerNameOnItemstring", strFormat, "%s's");
        }
        else
        {
            LocalizationHelpers::GetStringTxtValue("PlayerNameOnItemstringX", strFormat, "%s'");
        }
        personalizedName = d2ce::LocalizationHelpers::string_format(strFormat, personalizedName.c_str());
    }

    MagicalAffixes affixes;
    d2ce::RareAttributes rareAttrib;
    d2ce::UniqueAttributes uniqueAttrib;
    d2ce::SetAttributes setAttrib;
    strValue.clear();
    auto quality = getQuality();
    switch (quality)
    {
    case EnumItemQuality::INFERIOR:
        switch (getInferiorQualityId())
        {
        case 0:
            LocalizationHelpers::GetStringTxtValue("Crude", strValue);
            break;

        case 1:
            LocalizationHelpers::GetStringTxtValue("Cracked", strValue);
            break;

        case 2:
            LocalizationHelpers::GetStringTxtValue("Damaged", strValue);
            break;

        case 3:
            LocalizationHelpers::GetStringTxtValue("Low Quality", strValue);
            break;
        }

        if (!strValue.empty())
        {
            LocalizationHelpers::GetStringTxtValue("LowqualityFormat", strFormat, "%0 %1");
            strValue = LocalizationHelpers::string_formatDiablo(strFormat, strValue, itemType.name);
        }
        else
        {
            strValue = itemType.name;
        }

        if (personalizedName.empty())
        {
            ss << strValue;
        }
        else
        {
            LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
            ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, strValue);
        }
        break;

    case EnumItemQuality::NORMAL:
        if (getSocketedItemCount() > 0)
        {
            if (isRuneword())
            {
                strValue = itemType.name;
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("GemmedNormalName", strFormat, "%0 %1");
                LocalizationHelpers::GetStringTxtValue("Gemmed", strValue);
                strValue = LocalizationHelpers::string_formatDiablo(strFormat, strValue, itemType.name);
            }
        }
        else if (isBodyPart())
        {
            const auto& monsterName = ItemHelpers::getMonsterNameFromId(getMonsterId());
            if (monsterName.empty())
            {
                strValue = itemType.name;
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("BodyPartsFormat", strFormat, "%0 %1");
                strValue = LocalizationHelpers::string_formatDiablo(strFormat, monsterName, itemType.name);
            }
        }
        else
        {
            strValue = itemType.name;
        }

        if (personalizedName.empty())
        {
            ss << strValue;
        }
        else
        {
            LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
            ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, strValue);
        }
        break;

    case EnumItemQuality::SUPERIOR:
        if (runewordAttrib.Name.empty())
        {
            LocalizationHelpers::GetStringTxtValue("HiqualityFormat", strFormat, "%0 %1");
            LocalizationHelpers::GetStringTxtValue("Hiquality", strValue, "Superior");
            strValue = LocalizationHelpers::string_formatDiablo(strFormat, strValue, itemType.name);
        }
        else
        {
            strValue = itemType.name;
        }

        if (personalizedName.empty())
        {
            ss << strValue;
        }
        else
        {
            LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
            ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, strValue);
        }
        break;

    case EnumItemQuality::MAGIC:
        if (getMagicalAffixes(affixes))
        {
            if (affixes.PrefixName.empty())
            {
                if (affixes.SuffixName.empty())
                {
                    strValue = itemType.name;
                }
                else
                {
                    LocalizationHelpers::GetStringTxtValue("ItemNameMagicFormatSuffixOnly", strFormat, "%0 %1");
                    strValue = LocalizationHelpers::string_formatDiablo(strFormat, itemType.name, affixes.SuffixName);
                }
            }
            else if (affixes.SuffixName.empty())
            {
                LocalizationHelpers::GetStringTxtValue("ItemNameMagicFormatPrefixOnly", strFormat, "%0 %1");
                strValue = LocalizationHelpers::string_formatDiablo(strFormat, affixes.PrefixName, itemType.name);
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("MagicFormat", strFormat, "%0 %1 %2");
                strValue = LocalizationHelpers::string_formatDiablo(strFormat, affixes.PrefixName, itemType.name, affixes.SuffixName);
            }
        }
        else
        {
            // should not happen
            strValue = itemType.name;
        }

        if (personalizedName.empty())
        {
            ss << strValue;
        }
        else
        {
            LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
            ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, strValue);
        }
        break;

    case EnumItemQuality::SET:
        if (getSetAttributes(setAttrib) && !setAttrib.Name.empty())
        {
            if (personalizedName.empty())
            {
                ss << setAttrib.Name;
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, setAttrib.Name);
            }

            ss << '\n' << itemType.getTypeName();
        }
        else
        {
            if (personalizedName.empty())
            {
                ss << itemType.getTypeName();
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, itemType.getTypeName());
            }
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::TEMPERED:
        if (getRareOrCraftedAttributes(rareAttrib))
        {
            if (rareAttrib.Name.empty())
            {
                if (rareAttrib.Name2.empty())
                {
                    if (personalizedName.empty())
                    {
                        ss << itemType.name;
                    }
                    else
                    {
                        LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                        ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, itemType.name);
                    }
                }
                else
                {
                    if (personalizedName.empty())
                    {
                        ss << rareAttrib.Name2;
                    }
                    else
                    {
                        LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                        ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, rareAttrib.Name2);
                    }
                    ss << '\n' << itemType.name;
                }
            }
            else if (rareAttrib.Name2.empty())
            {
                if (personalizedName.empty())
                {
                    ss << rareAttrib.Name;
                }
                else
                {
                    LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                    ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, rareAttrib.Name);
                }
                ss << '\n' << itemType.name;
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("RareFormat", strFormat, "%0 %1");
                strValue = LocalizationHelpers::string_formatDiablo(strFormat, rareAttrib.Name, rareAttrib.Name2);
                if (personalizedName.empty())
                {
                    ss << strValue;
                }
                else
                {
                    LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                    ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, strValue);
                }
                ss << '\n' << itemType.name;
            }
        }
        else
        {
            // should not happen
            if (personalizedName.empty())
            {
                ss << itemType.name;
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, itemType.name);
            }
        }
        break;

    case EnumItemQuality::UNIQUE:
        if (getUniqueAttributes(uniqueAttrib) && !uniqueAttrib.Name.empty())
        {
            if (personalizedName.empty())
            {
                ss << uniqueAttrib.Name;
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, uniqueAttrib.Name);
            }

            if (!itemType.skipName)
            {
                ss << '\n' << itemType.getTypeName();
            }
        }
        else
        {
            // should not happen
            ss << itemType.name;
            if (personalizedName.empty())
            {
                ss << itemType.getTypeName();
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
                ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, itemType.getTypeName());
            }
        }
        break;

    default:
        // should not happen
        if (personalizedName.empty())
        {
            ss << itemType.name;
        }
        else
        {
            LocalizationHelpers::GetStringTxtValue("ChampionFormat", strFormat, "%0 %1");
            ss << LocalizationHelpers::string_formatDiablo(strFormat, personalizedName, itemType.name);
        }
        break;
    }

    return ss.str();
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getDisplayedSocketCount() const
{
    auto count = getSocketCount();
    if (count == 0)
    {
        return 0;
    }

    // Calculate item bonus
    count += getSocketCountBonus();
    return count;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getSocketCountBonus() const
{
    // Calculate item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        std::uint64_t eCount = 0;
        ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib);
            if (stat.name == "item_numsockets")
            {
                eCount += ItemHelpers::getMagicalAttributeValue(attrib, 1, 0, stat);
            }
        }

        return (std::uint8_t)eCount;
    }

    return 0;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getSocketCount() const
{
    if (isSimpleItem() || !isSocketed())
    {
        return 0;
    }

    switch (getVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        return getMaxSocketCount();

    case EnumItemVersion::v107:  // v1.07 item
    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        if (GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET) == 0)
        {
            return 0;
        }

        return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET), SOCKET_COUNT_NUM_BITS);
    }
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getSocketedItemCount() const
{
    if (GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) == 0)
    {
        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            if (isSimpleItem() || isEar())
            {
                return 0;
            }

            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 2;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            if (isSimpleItem() || isEar())
            {
                return 0;
            }

            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET;
            nr_of_items_in_sockets_bits = 3;
            break;

        case EnumItemVersion::v107:  // v1.07 item
        case EnumItemVersion::v108:  // v1.08 item
        case EnumItemVersion::v109:  // v1.09 item
        case EnumItemVersion::v110:  // v1.10 - v1.14d item
        case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
        case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
        default:
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
            nr_of_items_in_sockets_bits = isSimpleItem() ? 1 : 3;
            break;
        }
    }

    return (std::uint8_t)readBits(GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET), nr_of_items_in_sockets_bits);
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedSocketedRunes() const
{
    std::string name;
    std::stringstream ss;
    bool bFirstItem = true;

    std::string quoteStr;
    LocalizationHelpers::GetStringTxtValue("RuneQuote", quoteStr, "'");
    for (const auto& item : SocketedItems)
    {
        if (item.isRune())
        {
            if (bFirstItem)
            {
                ss << quoteStr;
                bFirstItem = false;
            }

            ss << item.getRuneLetter();
        }
    }

    if (!bFirstItem)
    {
        ss << quoteStr;
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

    std::vector<MagicalAttribute> magicalAttributes;
    // Calculate item bonus
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);

        std::uint64_t eDef = 0;
        std::uint64_t def = 0;
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib);
            if (stat.name == "armorclass")
            {
                def += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
            }
            else
            {
                for (const auto& opStat : stat.opAttribs.op_stats)
                {
                    if (opStat == "armorclass")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 13:
                        case 5:
                            eDef += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                            break;

                        case 4:
                            def += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                            break;
                        }
                    }
                }
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
            const auto& stat = ItemHelpers::getItemStat(attrib);
            if (stat.name == "maxdurability")
            {
                dur += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
            }
            else if (stat.name == "item_indesctructible")
            {
                // Indestructible
                durability.Max = 0;
                durability.Current = 0;
                return true;
            }
            else
            {
                for (const auto& opStat : stat.opAttribs.op_stats)
                {
                    if (opStat == "maxdurability")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 13:
                            eDur += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                            break;
                        }
                    }
                }
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
        std::uint64_t eDmg2 = 0;
        std::uint64_t eDmgMax2 = 0;
        std::uint64_t eDmgThrow = 0;
        std::uint64_t eDmgMaxThrow = 0;
        std::uint64_t dmgMin = 0;
        std::uint64_t dmgMax = 0;
        std::uint64_t dmgMin2 = 0;
        std::uint64_t dmgMax2 = 0;
        std::uint64_t dmgMinThrow = 0;
        std::uint64_t dmgMaxThrow = 0;
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib);
            if (stat.name == "mindamage")
            {
                dmgMin += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
            }
            else if (stat.name == "maxdamage")
            {
                dmgMax += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
            }
            else if (stat.name == "secondary_mindamage")
            {
                dmgMin2 += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMin
            }
            else if (stat.name == "secondary_maxdamage")
            {
                dmgMax2 += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMax
            }
            else if (stat.name == "item_throw_mindamage")
            {
                dmgMinThrow += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMin
            }
            else if (stat.name == "item_throw_maxdamage")
            {
                dmgMaxThrow += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMax
            }
            else
            {
                for (const auto& opStat : stat.opAttribs.op_stats)
                {
                    if (opStat == "mindamage")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 13:
                        case 5:
                            eDmg += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                            break;

                        case 4:
                            dmgMin += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                            break;
                        }
                    }
                    else if (opStat == "maxdamage")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 13:
                        case 5:
                            if (stat.name == "item_maxdamage_percent")
                            {
                                auto maxDmg = ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                                auto minDmg = ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 1, stat);
                                if (minDmg > maxDmg)
                                {
                                    maxDmg = minDmg;
                                }

                                // this stat is chained
                                eDmg += minDmg;
                                if (maxDmg > minDmg)
                                {
                                    eDmgMax += (maxDmg - minDmg);
                                }
                            }
                            else
                            {
                                eDmgMax += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                            }
                            break;

                        case 4:
                            dmgMax += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                            break;
                        }

                        
                    }
                    else if (opStat == "secondary_mindamage")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 13:
                        case 5:
                            eDmg2 += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as eDmg
                            break;

                        case 4:
                            dmgMin2 += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMin
                            break;
                        }
                    }
                    else if (opStat == "secondary_maxdamage")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 13:
                        case 5:
                            if (stat.name == "item_maxdamage_percent")
                            {
                                auto maxDmg = ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                                auto minDmg = ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 1, stat);
                                if (minDmg > maxDmg)
                                {
                                    maxDmg = minDmg;
                                }

                                // this stat is chained
                                eDmg2 += minDmg; // should be the same as eDmg
                                if (maxDmg > minDmg)
                                {
                                    eDmgMax2 += (maxDmg - minDmg); // // should be the same as eDmgMax
                                }
                            }
                            else
                            {
                                eDmgMax2 += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as eDmgMax
                            }
                            break;

                        case 4:
                            dmgMax2 += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMax
                            break;
                        }
                    }
                    else if (opStat == "item_throw_mindamage")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 13:
                        case 5:
                            eDmgThrow += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as eDmg
                            break;

                        case 4:
                            dmgMinThrow += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMin
                            break;
                        }
                    }
                    else if (opStat == "item_throw_maxdamage")
                    {
                        switch (stat.opAttribs.op)
                        {
                        case 13:
                        case 5:
                            if (stat.name == "item_maxdamage_percent")
                            {
                                auto maxDmg = ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
                                auto minDmg = ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 1, stat);
                                if (minDmg > maxDmg)
                                {
                                    maxDmg = minDmg;
                                }

                                // this stat is chained
                                eDmgThrow += minDmg; // should be the same as eDmg
                                if (maxDmg > minDmg)
                                {
                                    eDmgMaxThrow += (maxDmg - minDmg); // // should be the same as eDmgMax
                                }
                            }
                            else
                            {
                                eDmgMaxThrow += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as eDmgMax
                            }
                            break;

                        case 4:
                            dmgMaxThrow += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat); // should be the same as dmgMax
                            break;
                        }
                    }
                }
            }
        }

        if (dmgMin == 0)
        {
            dmgMin = std::max(dmgMinThrow, dmgMin2);
        }

        if (eDmg == 0)
        {
            eDmg = std::max(eDmgThrow, eDmg2);
        }

        if (dmgMax == 0)
        {
            dmgMax = std::max(dmgMaxThrow, dmgMax2);
        }

        if (eDmgMax == 0)
        {
            eDmgMax = std::max(eDmgMaxThrow, eDmgMax2);
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
bool d2ce::Item::getDisplayedRequirements(ItemRequirements& req, std::uint32_t charLevel) const
{
    req.clear();
    if (!getRequirements(req))
    {
        return false;
    }

    // Calculate item bonus
    std::vector<MagicalAttribute> magicalAttributes;
    if (getCombinedMagicalAttributes(magicalAttributes))
    {
        ItemHelpers::checkForRelatedMagicalAttributes(magicalAttributes);

        std::int64_t eReq = 0;
        for (auto& attrib : magicalAttributes)
        {
            const auto& stat = ItemHelpers::getItemStat(attrib);
            if (stat.name == "item_req_percent")
            {
                eReq += ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat);
            }
            else if (stat.name == "item_levelreq")
            {
                req.Level += std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat));
            }
            else if ((stat.name == "item_nonclassskill") || (stat.name == "item_singleskill"))
            {
                auto skillId = std::uint16_t(ItemHelpers::getMagicalAttributeValue(attrib, charLevel, 0, stat));
                const auto& skillInfo = d2ce::CharClassHelper::getSkillById(skillId);
                if (skillInfo.reqLevel > 1)
                {
                    req.Level = std::max(req.Level, skillInfo.reqLevel);
                }
            }
        }

        if (eReq > 0)
        {
            req.Dexterity += std::uint16_t((req.Dexterity * eReq) / 100);
            req.Strength += std::uint16_t((req.Strength * eReq) / 100);
        }
        else if (eReq < 0)
        {
            eReq *= -1;
            req.Dexterity -= std::uint16_t((req.Dexterity * eReq) / 100);
            req.Strength -= std::uint16_t((req.Strength * eReq) / 100);
        }
    }
    return true;
}
//---------------------------------------------------------------------------
std::string d2ce::Item::getDisplayedItemAttributes(EnumCharClass charClass, const CharStats& cs, bool bAddColorChar) const
{
    if (isEar())
    {
        return "";
    }

    std::string temp;
    const auto& itemType = getItemTypeHelper();
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return "";
    }

    if (itemType.isPotion())
    {
        return itemType.getPotionDesc(charClass);
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

        LocalizationHelpers::GetStringTxtValue("ExInsertSockets", temp, "Can be Inserted into Socketed Items");
        ss << temp;

        if (!itemType.isJewel())
        {
            std::string attribSep = (itemType.isRune() ? ",\n" : ", ");
            ss << "\n";
            std::vector<MagicalAttribute> attribs;
            if (itemType.getSocketedMagicalAttributes(*this, attribs, 0))
            {
                // check for the "all" cases
                ItemHelpers::checkForRelatedMagicalAttributes(attribs);

                bool bFirstAttrib = true;
                LocalizationHelpers::GetStringTxtValue("GemXp3", temp, "Weapons:");
                ss << "\n";
                ss << temp;
                ss << " ";
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

            if (itemType.getSocketedMagicalAttributes(*this, attribs, 1))
            {
                // check for the "all" cases
                ItemHelpers::checkForRelatedMagicalAttributes(attribs);

                bool bFirstAttrib = true;
                LocalizationHelpers::GetStringTxtValue("GemXp4", temp, "Armor:");
                ss << "\n";
                ss << temp;
                ss << " ";
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

                LocalizationHelpers::GetStringTxtValue("GemXp1", temp, "Helms:");
                ss << "\n";
                ss << temp;
                ss << " ";
                ss << commonText;
            }

            if (itemType.getSocketedMagicalAttributes(*this, attribs, 2))
            {
                // check for the "all" cases
                ItemHelpers::checkForRelatedMagicalAttributes(attribs);

                bool bFirstAttrib = true;

                LocalizationHelpers::GetStringTxtValue("GemXp2", temp, "Shields:");
                ss << "\n";
                ss << temp;
                ss << " ";

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

    auto defenseRating = getDisplayedDefenseRating(cs.Level);
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

        LocalizationHelpers::GetStringTxtValue("ItemStats1h", temp, "Defense: %d");
        auto baseDefenseRating = getDefenseRating();
        if (bAddColorChar && baseDefenseRating != defenseRating)
        {
            auto pos = temp.find("%d");
            if (pos != temp.npos)
            {
                temp.insert(pos + 2, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE));
                temp.insert(pos, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::BLUE));
            }
        }

        ss << d2ce::LocalizationHelpers::string_format(temp, defenseRating);
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

                LocalizationHelpers::GetStringTxtValue("BeltStorageModifierInfo", temp, "Belt Size: %+d Slots");
                ss << d2ce::LocalizationHelpers::string_format(temp, numSlots);
            }
        }
    }

    if (itemType.isWeapon())
    {
        ItemDamage dam;
        getDisplayedDamage(dam, cs.Level);

        ItemDamage base;
        bool bCheckBase = bAddColorChar && getDamage(base);

        bool bHasDamage = false;
        if (dam.Missile.Max != 0 && dam.Missile.Min)
        {
            bHasDamage = true;
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            if (dam.Missile.Min != dam.Missile.Max)
            {
                LocalizationHelpers::GetStringTxtValue("strItemStatThrowDamageRange", temp, "Throw Damage: %d to %d");
                if (bCheckBase)
                {
                    if (base.Missile.Min != dam.Missile.Min)
                    {
                        auto pos = temp.find("%d");
                        if (pos != temp.npos)
                        {
                            temp.insert(pos + 2, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE));
                            temp.insert(pos, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::BLUE));
                        }
                    }

                    if (base.Missile.Max != dam.Missile.Max)
                    {
                        auto pos = temp.rfind("%d");
                        if (pos != temp.npos)
                        {
                            temp.insert(pos + 2, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE));
                            temp.insert(pos, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::BLUE));
                        }
                    }
                }

                ss << d2ce::LocalizationHelpers::string_format(temp, dam.Missile.Min, dam.Missile.Max);
            }
            else
            {
                LocalizationHelpers::GetStringTxtValue("ItemStats1n", temp, "Throw Damage: %d");
                if (bCheckBase)
                {
                    if (base.Missile.Max != dam.Missile.Max)
                    {
                        auto pos = temp.rfind("%d");
                        if (pos != temp.npos)
                        {
                            temp.insert(pos + 2, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE));
                            temp.insert(pos, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::BLUE));
                        }
                    }
                }

                ss << d2ce::LocalizationHelpers::string_format(temp, dam.Missile.Max);
            }
        }

        if (dam.OneHanded.Max != 0 && dam.OneHanded.Min != 0)
        {
            bHasDamage = true;
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            LocalizationHelpers::GetStringTxtValue("ItemStats1l", temp, "One-Hand Damage: %d to %d");
            if (bCheckBase)
            {
                if (base.OneHanded.Min != dam.OneHanded.Min)
                {
                    auto pos = temp.find("%d");
                    if (pos != temp.npos)
                    {
                        temp.insert(pos + 2, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE));
                        temp.insert(pos, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::BLUE));
                    }
                }

                if (base.OneHanded.Max != dam.OneHanded.Max)
                {
                    auto pos = temp.rfind("%d");
                    if (pos != temp.npos)
                    {
                        temp.insert(pos + 2, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE));
                        temp.insert(pos, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::BLUE));
                    }
                }
            }

            ss << d2ce::LocalizationHelpers::string_format(temp, dam.OneHanded.Min, dam.OneHanded.Max);
        }

        if (dam.bTwoHanded && dam.TwoHanded.Max != 0 && dam.TwoHanded.Min != 0)
        {
            bHasDamage = true;
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            LocalizationHelpers::GetStringTxtValue("ItemStats1m", temp, "Two-Hand Damage: %d to %d");
            if (bCheckBase)
            {
                if (base.TwoHanded.Min != dam.TwoHanded.Min)
                {
                    auto pos = temp.find("%d");
                    if (pos != temp.npos)
                    {
                        temp.insert(pos + 2, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE));
                        temp.insert(pos, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::BLUE));
                    }
                }

                if (base.TwoHanded.Max != dam.TwoHanded.Max)
                {
                    auto pos = temp.rfind("%d");
                    if (pos != temp.npos)
                    {
                        temp.insert(pos + 2, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE));
                        temp.insert(pos, ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::BLUE));
                    }
                }
            }

            ss << d2ce::LocalizationHelpers::string_format(temp, dam.TwoHanded.Min, dam.TwoHanded.Max);
        }

        if (!bHasDamage)
        {
            // Edge case where Min and Max are zero for a weapon, then Max is 1
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            LocalizationHelpers::GetStringTxtValue("ItemStats1l", temp, "One-Hand Damage: %d to %d");
            ss << d2ce::LocalizationHelpers::string_format(temp, 0, 1);
        }
    }

    ItemDurability durability;
    if (getDisplayedDurability(durability, cs.Level) && durability.Max > 0)
    {
        if (bFirst)
        {
            bFirst = false;
        }
        else
        {
            ss << "\n";
        }

        LocalizationHelpers::GetStringTxtValue("ItemStats1d", temp, "Durability: %i of %i");
        ss << d2ce::LocalizationHelpers::string_format(temp, durability.Current, durability.Max);
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

        LocalizationHelpers::GetStringTxtValue("ItemStats1i", temp, "Quantity: %d of %d");
        ss << d2ce::LocalizationHelpers::string_format(temp, getQuantity(), getMaxQuantity());
    }

    auto itemClass = getClass();
    if (itemClass.has_value())
    {
        // class specific item
        temp.clear();
        switch (*itemClass)
        {
        case EnumCharClass::Amazon:
            LocalizationHelpers::GetStringTxtValue("AmaOnly", temp, "(Amazon Only)");
            break;

        case EnumCharClass::Sorceress:
            LocalizationHelpers::GetStringTxtValue("SorOnly", temp, "(Sorceress Only)");
            break;

        case EnumCharClass::Necromancer:
            LocalizationHelpers::GetStringTxtValue("NecOnly", temp, "(Necromancer Only)");
            break;

        case EnumCharClass::Paladin:
            LocalizationHelpers::GetStringTxtValue("PalOnly", temp, "(Paladin Only)");
            break;

        case EnumCharClass::Barbarian:
            LocalizationHelpers::GetStringTxtValue("BarOnly", temp, "(Barbarian Only)");
            break;

        case EnumCharClass::Druid:
            LocalizationHelpers::GetStringTxtValue("DruOnl", temp, "(Druid Only)");
            break;

        case EnumCharClass::Assassin:
            LocalizationHelpers::GetStringTxtValue("AssOnly", temp, "(Assassin Only)");
            break;
        }

        if (!temp.empty())
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                ss << "\n";
            }

            if (bAddColorChar && charClass != *itemClass)
            {
                ss << ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::RED);
                ss << temp;
                ss << ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE);
            }
            else
            {
                ss << temp;
            }
        }
    }

    ItemRequirements req;
    if (!getDisplayedRequirements(req, cs.Level))
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

        LocalizationHelpers::GetStringTxtValue("ItemStats1f", temp, "Required Dexterity: %d");
        temp = d2ce::LocalizationHelpers::string_format(temp, req.Dexterity);
        if (bAddColorChar && req.Dexterity > cs.Dexterity)
        {
            ss << ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::RED);
            ss << temp;
            ss << ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE);
        }
        else
        {
            ss << temp;
        }
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

        LocalizationHelpers::GetStringTxtValue("ItemStats1e", temp, "Required Strength: %d");
        temp = d2ce::LocalizationHelpers::string_format(temp, req.Strength);
        if (bAddColorChar && req.Strength > cs.Strength)
        {
            ss << ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::RED);
            ss << temp;
            ss << ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE);
        }
        else
        {
            ss << temp;
        }
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

        LocalizationHelpers::GetStringTxtValue("ItemStats1p", temp, "Required Level: %d");
        temp = d2ce::LocalizationHelpers::string_format(temp, req.Level);
        if (bAddColorChar && req.Level > cs.Level)
        {
            ss << ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::RED);
            ss << temp;
            ss << ColorHelpers::GetColorEscapeString(ColorHelpers::D2Colors::WHITE);
        }
        else
        {
            ss << temp;
        }
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
        AddUndeadBonusMagicalAttribute(getVersion(), getGameVersion(), attribs);
    }

    return d2ce::ItemHelpers::formatMagicalAttributes(attribs, charLevel);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedRunewordAttributes(RunewordAttributes& attribs, std::uint32_t charLevel) const
{
    if (!getRunewordAttributes(attribs))
    {
        return false;
    }

    return d2ce::ItemHelpers::formatMagicalAttributes(attribs.MagicalAttributes, charLevel);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedSetItemAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    SetAttributes setAttribs;
    if (!getSetAttributes(setAttribs))
    {
        return false;
    }

    bool bFormatted = false;
    for (auto& setItemAttribs : setAttribs.SetAttributes)
    {
        bFormatted |= d2ce::ItemHelpers::formatMagicalAttributes(setItemAttribs, charLevel);
        attribs.insert(attribs.end(), setItemAttribs.begin(), setItemAttribs.end());
    }

    return !attribs.empty();
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedFullSetAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getFullSetAttributes(attribs))
    {
        return false;
    }

    return d2ce::ItemHelpers::formatMagicalAttributes(attribs, charLevel);
}
//---------------------------------------------------------------------------
bool d2ce::Item::getDisplayedCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const
{
    if (!getCombinedMagicalAttributes(attribs))
    {
        return false;
    }

    return d2ce::ItemHelpers::formatMagicalAttributes(attribs, charLevel);
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

    return readBits64(readOffset, bits);
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

    return updateBitsEx(current_bit_offset, bits, value);
}
//---------------------------------------------------------------------------
bool d2ce::Item::setBits64(size_t& current_bit_offset, size_t bits, std::uint64_t value)
{
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

    return updateBits64Ex(current_bit_offset, bits, value);
}
//---------------------------------------------------------------------------
bool d2ce::Item::readItem(EnumItemVersion version, bool isExpansion, std::FILE* charfile)
{
    data.clear();
    SocketedItems.clear();
    cachedCombinedMagicalAttributes.clear();
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();

    GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = 0;
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();

    // reserve enough space to reduce chance of reallocation (haven't seen an item size bigger then 80
    data.reserve(80);

    GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) = 0;
    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET);
    ItemVersion = version;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
        isExpansion = false;
        GameVersion = 0;
        break;

    case EnumItemVersion::v107:
        GameVersion = isExpansion ? 100 : 0;
        break;

    default:
        GameVersion = isExpansion ? 100 : 1;
    }

    if (ItemVersion < EnumItemVersion::v100R) // pre-D2R 1.0.x character file
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

        GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) = current_bit_offset;

        // At least the Simple/Compact size is required
        if (!skipBits(charfile, current_bit_offset, 95))
        {
            return false;
        }

        if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
        {
            if (readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + ITEM_TYPE_BIT_OFFSET, 2) != 0)
            {
                ItemVersion = EnumItemVersion::v104; // v1.04 - v1.06 item
            }
            else
            {
                ItemVersion = EnumItemVersion::v100; // v1.00 - v1.03 item
            }
        }

        switch (getVersion())
        {
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = 0;

            // 27 bytes total
            if (!skipBits(charfile, current_bit_offset, (ITEM_V100_NUM_BITS - current_bit_offset)))
            {
                return false;
            }

            if (isEar())
            {
                GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V100_EAR_COORDINATES_BIT_OFFSET + 2;
                GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V100_EAR_ATTRIBE_BIT_OFFSET;
            }
            else
            {
                GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = ITEM_V100_BITFIELD3_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
                nr_of_items_in_sockets_bits = 2;
                GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V100_TYPECODE_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = ITEM_V100_DURABILITY_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V100_COORDINATES_BIT_OFFSET + 1;
                GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_100;
                GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = ITEM_V100_SPECIALITEMCODE_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = ITEM_V100_LEVEL_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = ITEM_V100_DWA_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;

                GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = 0;
                std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
                getItemCode(strcode);
                const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
                if (itemType.version != 0)
                {
                    // should not happen
                    return false;
                }

                if (itemType.isGoldItem())
                {
                    // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
                    GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_100;
                }
                else if (isStackable())
                {
                    GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_100;
                }
            }

            GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = current_bit_offset;
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
            GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
            GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = 0;

            if (isEar())
            {
                GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_EAR_COORDINATES_BIT_OFFSET + 2;
                GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_EAR_ATTRIBE_BIT_OFFSET;

                // 26 bytes total
                if (!skipBits(charfile, current_bit_offset, (ITEM_V104_EAR_NUM_BITS - current_bit_offset)))
                {
                    return false;
                }
            }
            else if (isSimpleItem())
            {
                GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_SM_COORDINATES_BIT_OFFSET + 2;
                GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_SM_TYPECODE_BIT_OFFSET;

                // 15 bytes total
                if (!skipBits(charfile, current_bit_offset, (ITEM_V104_SM_NUM_BITS - current_bit_offset)))
                {
                    return false;
                }
            }
            else
            {
                GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_EX_TYPECODE_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET;
                nr_of_items_in_sockets_bits = 3;
                GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = ITEM_V104_EX_DURABILITY_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_EX_COORDINATES_BIT_OFFSET + 1;
                GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_104;
                GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = ITEM_V104_EX_UNIQUECODE_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = ITEM_V104_EX_LEVEL_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = ITEM_V104_EX_DWA_BIT_OFFSET;
                GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;

                // 31 bytes total
                if (!skipBits(charfile, current_bit_offset, (ITEM_V104_EX_NUM_BITS - current_bit_offset)))
                {
                    return false;
                }

                GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = 0;
                GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = 0;
                std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
                getItemCode(strcode);
                const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
                if (itemType.version != 0)
                {
                    // should not happen
                    return false;
                }

                if (itemType.isGoldItem())
                {
                    // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
                    GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_104;
                }
                else if (isStackable())
                {
                    GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_104;
                }
            }

            GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = current_bit_offset;
            break;

        default: // v1.07 - v1.14d item
            GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = ITEM_V115_LOCATION_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = ITEM_V115_EQUIPPED_ID_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V115_POSITION_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = ITEM_V115_ALT_POSITION_ID_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V115_TYPE_CODE_BIT_OFFSET + ITEM_V107_START_BIT_OFFSET;
            GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V107_EXTENDED_DATA_BIT_OFFSET;
            break;
        }
    }
    else
    {
        GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = ITEM_V115_LOCATION_BIT_OFFSET;
        GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = ITEM_V115_EQUIPPED_ID_BIT_OFFSET;
        GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V115_POSITION_BIT_OFFSET;
        GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = ITEM_V115_ALT_POSITION_ID_BIT_OFFSET;
        GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V115_TYPE_CODE_BIT_OFFSET;

        // skip up to type code
        if (!skipBits(charfile, current_bit_offset, GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET)))
        {
            return false;
        }

        if (isEar())
        {
            if (!skipBits(charfile, current_bit_offset, 10))
            {
                return false;
            }

            // up to 15 7/8 bit characters
            size_t bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
            for (std::uint8_t idx = 0; !feof(charfile) && idx <= 15 && readBits(charfile, current_bit_offset, bitSize) != 0; ++idx);
            GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = current_bit_offset;
            GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
            return true;
        }

        for (size_t i = 0; i < 4; ++i)
        {
            if (getEncodedChar(charfile, current_bit_offset) == 0xFF)
            {
                return false;
            }
        }

        GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, isSimpleItem() ? 1 : 3))
        {
            return false;
        }
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (getVersion() >= EnumItemVersion::v107) // pre-D2R 1.0.x character file
    {
        GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) = 0;
        GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = 0;
        getItemCode(strcode);
        const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
        if (isSimpleItem())
        {
            if (itemType.isGoldItem())
            {
                // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
                // Is this correct for gld items? is it even needed?
                GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) + 1;
                if (readBits(charfile, current_bit_offset, 1) != 0)
                {
                    if (!skipBits(charfile, current_bit_offset, GLD_STACKABLE_LARGE_NUM_BITS))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!skipBits(charfile, current_bit_offset, GLD_STACKABLE_NUM_BITS))
                    {
                        return false;
                    }
                }

                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = current_bit_offset;
                nr_of_items_in_sockets_bits = 1;
            }
            else
            {
                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
                nr_of_items_in_sockets_bits = 1;
                if (itemType.isQuestItem())
                {
                    GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
                    GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) + 2;
                    nr_of_items_in_sockets_bits = 1;
                    if (!skipBits(charfile, current_bit_offset, 2))
                    {
                        return false;
                    }
                }
            }

            GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
            return true;
        }
        else if (itemType.isQuestItem())
        {
            GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) + 2;
            nr_of_items_in_sockets_bits = 1;
        }
        else
        {
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
            nr_of_items_in_sockets_bits = 3;
        }

        GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 32))
        {
            return false;
        }

        GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = current_bit_offset;
        if (!skipBits(charfile, current_bit_offset, 7))
        {
            return false;
        }

        GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = current_bit_offset;
        auto qualityval = readBits(charfile, current_bit_offset, 4);
        if (qualityval > static_cast<std::underlying_type_t<EnumItemQuality>>(EnumItemQuality::TEMPERED))
        {
            return false;
        }

        auto quality = static_cast<EnumItemQuality>(qualityval);

        // If this is TRUE, it means the item has more than one picture associated with it.
        GET_BIT_OFFSET(ItemOffsets::MULTI_GRAPHIC_BIT_OFFSET) = current_bit_offset;
        if (readBits(charfile, current_bit_offset, 1) != 0)
        {
            // The next 3 bits contain the picture ID
            if (!skipBits(charfile, current_bit_offset, 3))
            {
                return false;
            }
        }

        // If this is TRUE, it means the item is class specific.
        GET_BIT_OFFSET(ItemOffsets::AUTOAFFIX_BIT_OFFSET) = current_bit_offset;
        if (readBits(charfile, current_bit_offset, 1) != 0)
        {
            // If the item is class specific, the next 11 bits will
            // contain the class specific data.
            if (!skipBits(charfile, current_bit_offset, 11))
            {
                return false;
            }
        }

        GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = current_bit_offset;
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
        switch (quality)
        {
        case EnumItemQuality::NORMAL:
            if (itemType.isCharm())
            {
                // If the item is a NORMAL quality Charm, the next 12 bits will contain the Charm specific data.
                GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
                if (!skipBits(charfile, current_bit_offset, NORMAL_CHARM_ID_NUM_BITS))
                {
                    return false;
                }
            }
            break;

        case EnumItemQuality::INFERIOR:
        case EnumItemQuality::SUPERIOR:
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, INFERIOR_SUPERIOR_ID_NUM_BITS))
            {
                return false;
            }
            break;

        case EnumItemQuality::MAGIC:
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, 2 * MAGICAL_AFFIX_NUM_BITS))
            {
                return false;
            }
            break;

        case EnumItemQuality::RARE:
        case EnumItemQuality::CRAFTED:
        case EnumItemQuality::TEMPERED:
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
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
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, SET_UNIQUE_ID_NUM_BITS))
            {
                return false;
            }
            break;
        }

        GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER) = current_bit_offset;
        if (isRuneword())
        {
            GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET) = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, RUNEWORD_ID_NUM_BITS + RUNEWORD_PADDING_NUM_BITS))
            {
                return false;
            }
        }

        GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER) = current_bit_offset;
        if (isPersonalized())
        {
            GET_BIT_OFFSET(ItemOffsets::PERSONALIZED_BIT_OFFSET) = current_bit_offset;

            // up to 15 7/8 bit characters
            size_t bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
            for (std::uint8_t idx = 0; !feof(charfile) && idx <= 15 && readBits(charfile, current_bit_offset, bitSize) != 0; ++idx);
        }

        bool isArmor = itemType.isArmor();
        bool isWeapon = itemType.isWeapon();
        bool isTome = itemType.isBook();
        bool isStackable = itemType.isStackable();

        if (isTome)
        {
            // If the item is a tome, it will contain 5 extra bits, we're not
            // interested in these bits, the value is usually 1, but not sure
            // what it is.
            GET_BIT_OFFSET(ItemOffsets::TOME_BIT_OFFSET) = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, 5))
            {
                return false;
            }
        }
        else if (itemType.isBodyPart())
        {
            // If the item is a body part, it will contain 10 extra bits indicating
            // the monster Id from monstat.txt
            GET_BIT_OFFSET(ItemOffsets::BODY_PART_BIT_OFFSET) = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, MONSTER_ID_NUM_BITS))
            {
                return false;
            }
        }

        GET_BIT_OFFSET(ItemOffsets::REALM_BIT_OFFSET) = current_bit_offset;
        auto realmBits = (ItemVersion >= EnumItemVersion::v100R) ? REAL_DATA_NUM_BITS : REAL_DATA_NUM_BITS_110;
        if (readBits(charfile, current_bit_offset, 1) != 0)
        {
            if (!itemType.isMiscellaneous() || itemType.isGem() || itemType.isRing() ||
                itemType.isAmulet() || itemType.isCharm() || itemType.isRune())
            {
                if (!skipBits(charfile, current_bit_offset, realmBits))
                {
                    return false;
                }
            }
            else
            {
                if (!skipBits(charfile, current_bit_offset, 3))
                {
                    return false;
                }
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
                GET_BIT_OFFSET(ItemOffsets::DEFENSE_RATING_BIT_OFFSET) = current_bit_offset;
                if (!skipBits(charfile, current_bit_offset, ((ItemVersion >= EnumItemVersion::v110) ? DEFENSE_RATING_NUM_BITS : DEFENSE_RATING_NUM_BITS_108)))
                {
                    return false;
                }
            }

            // Some armor/weapons like phase blades don't have durability
            GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = current_bit_offset;
            if (readBits(charfile, current_bit_offset, DURABILITY_MAX_NUM_BITS) > 0)
            {
                // current durability value (8 bits + unknown single bit)
                if (!skipBits(charfile, current_bit_offset, ((ItemVersion >= EnumItemVersion::v110) ? DURABILITY_CURRENT_NUM_BITS : DURABILITY_CURRENT_NUM_BITS_108)))
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
            GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, STACKABLE_NUM_BITS))
            {
                return false;
            }
        }

        GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SOCKET_COUNT_BIT_OFFSET_MARKER) = current_bit_offset;
        if (isSocketed())
        {
            // If the item is socketed, it will contain 4 bits of data which are the
            // number of total sockets the item have, regardless of how many are occupied
            // by an item.
            GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET) = current_bit_offset;
            if (!skipBits(charfile, current_bit_offset, SOCKET_COUNT_NUM_BITS))
            {
                return false;
            }
        }

        // If the item is part of a set, these bit will tell us how many lists
        // of magical properties follow the one regular magical property list.
        std::uint8_t setBonusBits = 0;
        GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::BONUS_BITS_BIT_OFFSET_MARKER) = current_bit_offset;
        if (quality == EnumItemQuality::SET)
        {
            GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET) = current_bit_offset;
            setBonusBits = (std::uint8_t)readBits(charfile, current_bit_offset, 5);
            if (feof(charfile))
            {
                return false;
            }
        }

        // magical properties
        GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET) = current_bit_offset;
        if (!parsePropertyList(charfile, current_bit_offset))
        {
            return false;
        }

        GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SET_BONUS_PROPS_BIT_OFFSET_MARKER) = current_bit_offset;
        if (setBonusBits > 0)
        {
            // Item has more magical property lists due to being a set item
            GET_BIT_OFFSET(ItemOffsets::SET_BONUS_PROPS_BIT_OFFSET) = current_bit_offset;
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

        GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER) = current_bit_offset;
        if (isRuneword())
        {
            // runewords have their own list of magical properties
            GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET) = current_bit_offset;
            if (!parsePropertyList(charfile, current_bit_offset))
            {
                return false;
            }
        }
    }
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;

    auto numSocketed = getSocketedItemCount();
    if (numSocketed > 0)
    {
        auto gemApplyType = getGemApplyType();
        for (std::uint8_t i = 0; !feof(charfile) && i < numSocketed; ++i)
        {
            SocketedItems.resize(SocketedItems.size() + 1);
            auto& childItem = SocketedItems.back();
            childItem.item_current_socket_idx = SocketedItems.size() - 1;
            if (!childItem.readItem(version, isExpansion, charfile))
            {
                return false;
            }

            if (childItem.getLocation() != EnumItemLocation::SOCKET)
            {
                // corrupt file
                return false;
            }

            // resolve magical properties of socketed gem
            childItem.item_current_socket_idx = 0;
            const auto& childItemType = childItem.getItemTypeHelper();
            childItemType.getSocketedMagicalAttributes(childItem, childItem.socketedMagicalAttributes, gemApplyType);
        }
    }

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
    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET);

    GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = 0;
    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = 0;
    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = 0;

    // 27 bytes total
    data.resize((ITEM_V100_NUM_BITS + 7) / 8, 0);

    GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V100_NUM_BITS;
    if (isEar())
    {
        GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V100_EAR_COORDINATES_BIT_OFFSET + 2;
        GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V100_EAR_ATTRIBE_BIT_OFFSET;

        size_t bitSize = 10;
        current_bit_offset = ITEM_V100_EAR_ITEMCODE_BIT_OFFSET + 5;
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
        node = earRoot[bSerializedFormat ? "FileIndex" : "class_id"];
        if (node.isNull())
        {
            if (bSerializedFormat)
            {
                return false;
            }

            node = earRoot["class"];
            if (node.isNull())
            {
                return false;
            }

            std::string className = node.asString();
            if (!CharClassHelper::getEnumCharClassByName(className, earAttrib.Class))
            {
                if (!CharClassHelper::getEnumCharClassByIndex(className, earAttrib.Class))
                {
                    return false;
                }
            }
        }
        else
        {
            value = std::uint32_t(node.asInt64());
            if (value > std::uint8_t(NUM_OF_CLASSES_NO_EXPANSION))
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
        if (!ItemHelpers::ProcessNameNode(node, earAttrib.Name, d2ce::EnumItemVersion::v100))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
        bitSize = 3;
        if (!setBits(current_bit_offset, bitSize, std::uint32_t(earAttrib.Class)))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_EAR_LEVEL_BIT_OFFSET;
        bitSize = 8;
        if (!setBits(current_bit_offset, bitSize, earAttrib.Level))
        {
            return false;
        }

        // up to 15 7/8 bit characters
        bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
        for (size_t idx = 0; idx <= 15; ++idx)
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
        switch (locationId)
        {
        case EnumItemLocation::STORED:
            if (node.isNull())
            {
                return false;
            }

            value = std::uint16_t(node.asInt64());
            break;

        default:
            value = 0;
            break;
        }
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
        switch (locationId)
        {
        case EnumItemLocation::SOCKET:
            positionX = std::uint16_t(item_current_socket_idx);
            break;
        }

        node = itemRoot[bSerializedFormat ? "Y" : "position_y"];
        std::uint16_t positionY = 0;
        switch (locationId)
        {
        case EnumItemLocation::SOCKET:
            positionY = 0;
            break;

        case EnumItemLocation::STORED:
            if (node.isNull())
            {
                return false;
            }

            positionY = std::uint16_t(node.asInt64());
            break;

        default:
            if (!node.isNull())
            {
                positionY = std::uint16_t(node.asInt64());
            }
            break;
        }

        switch (locationId)
        {
        case EnumItemLocation::STORED:
            switch (altPositionId)
            {
            case EnumAltItemLocation::INVENTORY:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint8_t)(readBits(current_bit_offset, bitSize) & 0x9F);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint8_t)((readBits(current_bit_offset, bitSize) & 0x9F) | 0x40);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V100_EAR_CONTAINER_BIT_OFFSET;
                bitSize = 8;
                value = (std::uint8_t)((readBits(current_bit_offset, bitSize) & 0x9F) | 0x60);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
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

        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = ITEM_V100_NUM_BITS;
        return true;
    }

    GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = ITEM_V100_BITFIELD3_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V100_NUM_SOCKETED_BIT_OFFSET;
    nr_of_items_in_sockets_bits = 2;
    GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V100_TYPECODE_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = ITEM_V100_DURABILITY_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V100_COORDINATES_BIT_OFFSET + 1;
    GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_100;
    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = ITEM_V100_SPECIALITEMCODE_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
    GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = ITEM_V100_LEVEL_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = ITEM_V100_DWA_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;

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
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (itemType.code_v100 == UINT16_MAX)
    {
        return false;
    }

    size_t bitSize = 10;
    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
    if (!setBits(current_bit_offset, bitSize, itemType.code_v100))
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

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET);
    bitSize = 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::TEMPERED:
        break;

    case EnumItemQuality::SET:
        node = itemRoot[bSerializedFormat ? "FileIndex" : "set_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint16_t(node.asInt64());

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
        bitSize = ITEM_V100_UNIQUE_ID_NUM_BITS;
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

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
        bitSize = ITEM_V100_UNIQUE_ID_NUM_BITS;
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

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_CURRENT_NUM_BITS_108;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }

    if (value > 0)
    {
        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET);
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

    GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = 0;
    if (itemType.isGoldItem())
    {
        // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
        GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_100;

        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (node.isNull())
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET);
        value = std::uint32_t(node.asInt64());
        if (value > MAX_GLD_QUANTITY)
        {
            if (!setBits(current_bit_offset, 1, 1))
            {
                return false;
            }

            bitSize = GLD_STACKABLE_LARGE_NUM_BITS;
        }
        else
        {
            if (!setBits(current_bit_offset, 1, 0))
            {
                return false;
            }

            // can hold up to 4095 gold pieces
            value = std::min(value, MAX_GLD_QUANTITY);
            bitSize = GLD_STACKABLE_NUM_BITS;
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }
    else if (itemType.isStackable())
    {
        GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_100;

        value = 0;
        bitSize = STACKABLE_NUM_BITS;
        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (!node.isNull())
        {
            value = std::min(std::uint32_t(node.asInt64()), MAX_STACKED_QUANTITY);
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }

    node = itemRoot[bSerializedFormat ? "ItemLevel" : "level"];
    if (node.isNull())
    {
        value = std::max(1ui16, itemType.level.Quality);
    }
    else
    {
        value = std::max(1ui32, std::uint32_t(node.asInt64()));
    }

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET);
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
        if (value < static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_LEFT_ARM))
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
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xF807);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, 0))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x3FFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumAltItemLocation::STASH:
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0020);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, 0))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x3FFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumAltItemLocation::HORADRIC_CUBE:
            current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0018);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
            bitSize = 4;
            if (!setBits(current_bit_offset, bitSize, 0))
            {
                return false;
            }

            current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x3FFF);
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
        value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        positionY = positionX / 4;
        positionX = positionX % 4;
        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
        bitSize = 2;
        if (!setBits(current_bit_offset, bitSize, positionX))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
        bitSize = 2;
        if (!setBits(current_bit_offset, bitSize, positionY))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
        bitSize = 4;
        if (!setBits(current_bit_offset, bitSize, 0))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x3FFF) | 0x8000);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;

    case EnumItemLocation::EQUIPPED:
        current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
        bitSize = 8;
        value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x01);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
        bitSize = 4;
        value = static_cast<std::underlying_type_t<EnumEquippedId>>(equippedId);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x3FFF) | 0x4000);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;

    case EnumItemLocation::SOCKET:
        current_bit_offset = ITEM_V100_CONTAINER_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_COORDINATES_BIT_OFFSET;
        bitSize = 8;
        value = (std::uint8_t)(readBits(current_bit_offset, bitSize) & 0xE1);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_BITFIELD3_BIT_OFFSET;
        bitSize = 8;
        value = (std::uint8_t)(readBits(current_bit_offset, bitSize) & 0xF0);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V100_ITEMCODE_BIT_OFFSET;
        bitSize = 16;
        value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x3FFF) | 0x8000);
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
        node = itemRoot[bSerializedFormat ? "Id" : "id"];
        if (node.isNull())
        {
            value = ItemHelpers::generarateRandomDW();
        }
        else
        {
            value = std::uint32_t(node.asInt64());
            if (value == 0)
            {
                value = ItemHelpers::generarateRandomDW();
            }
        }
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET);
    if (!setBits(current_bit_offset, 32, value))
    {
        return false;
    }

    value = 0;
    node = itemRoot[bSerializedFormat ? "Dwb" : "dwb"];
    if (node.isNull())
    {
        // generate new one
        if (getQuality() == EnumItemQuality::SET)
        {
            // Find correct DWB value for the SET
            const auto& setItemType = ItemHelpers::getSetItemTypeHelper((std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), ITEM_V100_UNIQUE_ID_NUM_BITS), strcode);
            value = setItemType.getSetItemDWBCode();
        }
        else
        {
            value = ItemHelpers::generarateRandomDW();
        }
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET);
    if (!setBits(current_bit_offset, 32, value))
    {
        return false;
    }

    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = ITEM_V100_NUM_BITS;
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
    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET);

    GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = 0;
    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = 0;
    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = 0;

    if (isEar())
    {
        GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_EAR_COORDINATES_BIT_OFFSET + 2;
        GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_EAR_ATTRIBE_BIT_OFFSET;

        // 26 bytes total
        data.resize((ITEM_V104_EAR_NUM_BITS + 7) / 8, 0);
        GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V104_EAR_NUM_BITS;

        Json::Value earRoot = bSerializedFormat ? itemRoot : itemRoot["ear_attributes"];
        if (earRoot.isNull())
        {
            return false;
        }

        EarAttributes earAttrib;
        node = earRoot[bSerializedFormat ? "FileIndex" : "class_id"];
        if (node.isNull())
        {
            if (bSerializedFormat)
            {
                return false;
            }

            node = earRoot["class"];
            if (node.isNull())
            {
                return false;
            }

            std::string className = node.asString();
            if (!CharClassHelper::getEnumCharClassByName(className, earAttrib.Class))
            {
                if (!CharClassHelper::getEnumCharClassByIndex(className, earAttrib.Class))
                {
                    return false;
                }
            }
        }
        else
        {
            value = std::uint32_t(node.asInt64());
            if (value > std::uint8_t(NUM_OF_CLASSES_NO_EXPANSION))
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
        if (!ItemHelpers::ProcessNameNode(node, earAttrib.Name, d2ce::EnumItemVersion::v104))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
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

        // up to 15 7/8 bit characters
        bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
        for (size_t idx = 0; idx <= 15; ++idx)
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
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xE3FF);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xE3FF) | 0x1000);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V104_EAR_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xE3FF) | 0x0C00);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
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

        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = ITEM_V104_EAR_NUM_BITS;
        return true;
    }

    if (isSimpleItem())
    {
        GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_SM_COORDINATES_BIT_OFFSET + 2;
        GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_SM_TYPECODE_BIT_OFFSET;

        // 15 bytes total
        data.resize((ITEM_V104_SM_NUM_BITS + 7) / 8, 0);
        GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V104_SM_NUM_BITS;

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
        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
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
                value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x03FE);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::STASH:
                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03FE) | 0x1000);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                break;

            case EnumAltItemLocation::HORADRIC_CUBE:
                current_bit_offset = ITEM_V104_SM_COORDINATES_BIT_OFFSET;
                bitSize = 16;
                value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03FE) | 0x0C00);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
                bitSize = 5;
                if (!setBits(current_bit_offset, bitSize, positionX))
                {
                    return false;
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
                bitSize = 2;
                if (!setBits(current_bit_offset, bitSize, positionY))
                {
                    return false;
                }

                current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
                bitSize = 32;
                value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
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
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03FE) | 0xFC01);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            positionY = positionX / 4;
            positionX = positionX % 4;
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
            bitSize = 32;
            value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) | 0x00000003);
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
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0x03E0) | 0xFC1B);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            if (item_current_socket_idx >= 7)
            {
                return false;
            }

            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xFC7F) | (item_current_socket_idx << 7));
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_SM_ITEMCODE_BIT_OFFSET;
            bitSize = 32;
            value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) | 0x00000003);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;
        }

        current_bit_offset = ITEM_V104_SM_NUM_BITS;
        return true;
    }

    GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = ITEM_V104_EX_TYPECODE_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = ITEM_V104_EX_NUM_SOCKETED_BIT_OFFSET;
    nr_of_items_in_sockets_bits = 3;
    GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = ITEM_V104_EX_DURABILITY_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = ITEM_V104_EX_COORDINATES_BIT_OFFSET + 1;
    GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUALITY_BIT_OFFSET_104;
    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = ITEM_V104_EX_UNIQUECODE_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER);
    GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = ITEM_V104_EX_LEVEL_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = ITEM_V104_EX_DWA_BIT_OFFSET;
    GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;

    // 31 bytes total
    data.resize((ITEM_V104_EX_NUM_BITS + 7) / 8, 0);
    GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = ITEM_V104_EX_NUM_BITS;

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
    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
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

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET);
    bitSize = 4;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }

    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::TEMPERED:
        break;

    case EnumItemQuality::SET:
        node = itemRoot[bSerializedFormat ? "FileIndex" : "set_id"];
        if (node.isNull())
        {
            return false;
        }
        value = std::uint32_t(node.asInt64());

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
        bitSize = ITEM_V100_UNIQUE_ID_NUM_BITS;
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

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET);
        bitSize = ITEM_V100_UNIQUE_ID_NUM_BITS;
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

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) + DURABILITY_CURRENT_NUM_BITS_108;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }

    if (value > 0)
    {
        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET);
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

    GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = 0;
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (itemType.isGoldItem())
    {
        // Gold Items should not exist in the inventory as it is a place holder for the gold dropped in the game
        GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_104;

        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (node.isNull())
        {
            return false;
        }

        // can hold up to 4095 gold pieces
        value = std::min(std::uint32_t(node.asInt64()), MAX_GLD_QUANTITY);
        bitSize = GLD_STACKABLE_NUM_BITS;
        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }
    else if (itemType.isStackable())
    {
        GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + QUANTITY_BIT_OFFSET_104;

        value = 0;
        bitSize = STACKABLE_NUM_BITS;
        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (!node.isNull())
        {
            value = std::min(std::uint32_t(node.asInt64()), MAX_STACKED_QUANTITY);
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
    }

    node = itemRoot[bSerializedFormat ? "ItemLevel" : "level"];
    if (node.isNull())
    {
        value = std::max(1ui16, itemType.level.Quality);
    }
    else
    {
        value = std::max(1ui32, std::uint32_t(node.asInt64()));
    }

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET);
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
        if (value < static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_LEFT_ARM))
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
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xF807);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
            bitSize = 8;
            value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
            bitSize = 32;
            value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xBFFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumAltItemLocation::STASH:
            current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0020);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
            bitSize = 8;
            value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
            bitSize = 32;
            value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xBFFF);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            break;

        case EnumAltItemLocation::HORADRIC_CUBE:
            current_bit_offset = ITEM_V104_EX_CONTAINER_BIT_OFFSET;
            bitSize = 16;
            value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x0018);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
            bitSize = 5;
            if (!setBits(current_bit_offset, bitSize, positionX))
            {
                return false;
            }

            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) + bitSize;
            bitSize = 2;
            if (!setBits(current_bit_offset, bitSize, positionY))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
            bitSize = 8;
            value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
            bitSize = 32;
            value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }

            current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
            bitSize = 16;
            value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0xBFFF);
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
        value = (std::uint16_t)((readBits(current_bit_offset, bitSize) & 0xF807) | 0x07F8);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
        bitSize = 8;
        value = (std::uint16_t)(readBits(current_bit_offset, bitSize) & 0x01);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V104_EX_EQUIP_ID_BIT_OFFSET - 2;
        bitSize = 8;
        value = (std::uint32_t)(readBits(current_bit_offset, bitSize) & 0xC3);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET);
        bitSize = 4;
        value = static_cast<std::underlying_type_t<EnumEquippedId>>(equippedId);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V104_EX_TYPECODE_BIT_OFFSET - 2;
        bitSize = 32;
        value = (std::uint32_t)(readBits64(current_bit_offset, bitSize) & 0xFFFFFFFC);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        current_bit_offset = ITEM_V104_EX_LEVEL_BIT_OFFSET - 2;
        bitSize = 16;
        value = (std::uint16_t)(readBits(current_bit_offset, bitSize) | 0x4000);
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
        node = itemRoot[bSerializedFormat ? "Id" : "id"];
        if (node.isNull())
        {
            value = ItemHelpers::generarateRandomDW();
        }
        else
        {
            value = std::uint32_t(node.asInt64());
            if (value == 0)
            {
                value = ItemHelpers::generarateRandomDW();
            }
        }
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET);
    if (!setBits(current_bit_offset, 32, value))
    {
        return false;
    }

    value = 0;
    node = itemRoot[bSerializedFormat ? "Dwb" : "dwb"];
    if (node.isNull())
    {
        // generate new one
        if (getQuality() == EnumItemQuality::SET)
        {
            // Find correct DWB value for the SET
            const auto& setItemType = ItemHelpers::getSetItemTypeHelper(getSetItemId());
            value = setItemType.getSetItemDWBCode();
        }
        else
        {
            value = ItemHelpers::generarateRandomDW();
        }
    }
    else
    {
        value = std::uint32_t(node.asInt64());
    }

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET);
    if (!setBits(current_bit_offset, 32, value))
    {
        return false;
    }

    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = ITEM_V104_EX_NUM_BITS;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::verifyItemConsistency() const
{
    if (data.empty())
    {
        return false;
    }

    // make sure it was read poperly.
    auto& helper = getItemTypeHelper();
    if (&helper == &ItemHelpers::getInvalidItemTypeHelper())
    {
        return false;
    }

    switch (ItemVersion)
    {
    case EnumItemVersion::v100:
    case EnumItemVersion::v104:
    case EnumItemVersion::v107:
        switch (getRawVersion())
        {
        case 0:
            break;

        default:
            return false;
        }
        break;

    case EnumItemVersion::v108:
    case EnumItemVersion::v109:
        switch (getRawVersion())
        {
        case   1:
        case 100:
            break;

        default:
            return false;
        }
        break;

    case EnumItemVersion::v110:
        switch (getRawVersion())
        {
        case   2:
        case 101:
            break;

        default:
            return false;
        }
        break;

    case EnumItemVersion::v100R:
    case EnumItemVersion::v120:
    case EnumItemVersion::v140:
        switch (getRawVersion())
        {
        case 4:
        case 5:
            break;

        default:
            return false;
        }
        break;
    }

    if (isSimpleItem() != helper.isSimpleItem())
    {
        return false;
    }

    // looks mostly proper
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::readItem(const Json::Value& itemRoot, bool bSerializedFormat, EnumItemVersion version, bool isExpansion)
{
    if (itemRoot.isNull())
    {
        return false;
    }

    data.clear();
    SocketedItems.clear();
    cachedCombinedMagicalAttributes.clear();
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();

    GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = 0;
    magic_affixes_v100.clear();
    rare_affixes_v100.clear();

    // reserve enough space to reduce chance of reallocation (haven't seen an item size bigger then 80
    data.reserve(80);

    Json::Value node;
    std::uint16_t rawVersion = 0;
    std::uint32_t value = 0;
    std::uint64_t value64 = 0;
    GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) = 0;
    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET);
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

    ItemVersion = version;
    switch (ItemVersion)
    {
    case EnumItemVersion::v100:
        isExpansion = false;
        GameVersion = 0;
        ItemVersion = EnumItemVersion::v100; // v1.00 - v1.03 item
        node = itemRoot["IsV104"];
        if (!node.isNull() && (node.asInt64() != 0))
        {
            ItemVersion = EnumItemVersion::v104; // v1.04 - v1.06 item
        }
        break;

    case EnumItemVersion::v104:
        isExpansion = false;
        GameVersion = 0;
        break;

    case EnumItemVersion::v107:
        GameVersion = isExpansion ? 100 : 0;
        break;

    default:
        GameVersion = isExpansion ? 100 : 1;
    }

    if (ItemVersion < EnumItemVersion::v100R)
    {
        value = *((std::uint16_t*)ITEM_MARKER.data());
        setBits(current_bit_offset, ITEM_MARKER.size() * 8, value);
        GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) = current_bit_offset;
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);

        if (ItemVersion <= EnumItemVersion::v107) // pre-1.07 character file
        {
            rawVersion = 0;
        }
        else
        {
            node = itemRoot[bSerializedFormat ? "Version" : "version"];
            if (node.isNull())
            {
                if (version < EnumItemVersion::v110) // pre-1.10 character file
                {
                    rawVersion = isExpansion ? 100 : 1;
                }
                else
                {
                    rawVersion = isExpansion ? 101 : 2;
                }
            }
            else
            {
                rawVersion = bSerializedFormat ? std::uint16_t(std::stoul(node.asString(), nullptr, 10)) : std::uint16_t(node.asInt64());
                switch (rawVersion)
                {
                case 100:
                case 101:
                    if (version < EnumItemVersion::v110) // pre-1.10 character file
                    {
                        rawVersion = 100;
                    }
                    break;

                case 5:
                    if (version < EnumItemVersion::v110) // pre-1.10 character file
                    {
                        rawVersion = 100;
                    }
                    else
                    {
                        rawVersion = 101;
                    }
                    break;

                case 0:
                case 1:
                case 2:
                    if (version < EnumItemVersion::v110) // pre-1.10 character file
                    {
                        rawVersion = 1;
                    }
                    break;

                case 4:
                    if (version < EnumItemVersion::v110) // pre-1.10 character file
                    {
                        rawVersion = 1;
                    }
                    else
                    {
                        rawVersion = 2;
                    }
                    break;

                default:
                    if (version < EnumItemVersion::v110) // pre-1.10 character file
                    {
                        rawVersion = isExpansion ? 100 : 1;
                    }
                    else
                    {
                        rawVersion = isExpansion ? 101 : 2;
                    }
                    break;
                }
            }
        }
    }
    else
    {
        rawVersion = isExpansion ? 5 : 4;
        node = itemRoot[bSerializedFormat ? "Version" : "version"];
        if (!node.isNull())
        {
            rawVersion = bSerializedFormat ? std::uint16_t(std::stoul(node.asString(), nullptr, 2)) : std::uint16_t(node.asInt64());
            switch (rawVersion)
            {
            case 100:
            case 101:
            case 5:
                rawVersion = 5;
                break;

            case 0:
            case 1:
            case 2:
            case 4:
                rawVersion = 4;
                break;

            default:
                rawVersion = isExpansion ? 5 : 4;
                break;
            }
        }
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

    if (ItemVersion >= EnumItemVersion::v107) // pre-1.07 character file
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

    Json::Value unknowns = itemRoot["_unknown_data"];
    if (!unknowns.isNull())
    {
        struct bitRange
        {
            size_t startIdx = 0;
            size_t endIdx = 0;
        };
        static std::vector<bitRange> unknowns_range = { {0,3},{5,10},{12,12},{14,15},{18,20},{23,23},{25,25},{27,31} };

        size_t bitNum = 0;
        Json::Value unknownData;
        for (const auto& bitInfo : unknowns_range)
        {
            std::stringstream ss;
            ss << "b" << std::dec << bitInfo.startIdx;
            if (bitInfo.endIdx > bitInfo.startIdx)
            {
                ss << "_" << std::dec << bitInfo.endIdx;
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

                bitNum = bitInfo.startIdx + size_t(std::stoi(iter.name()));
                SetFlagBit(*iter, bitNum, flags);
            }
        }
    }

    value = flags.to_ulong();
    if (!setBits(current_bit_offset, flags.size(), value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    if (ItemVersion < EnumItemVersion::v107) // pre-1.07 character file
    {
        if (ItemVersion == EnumItemVersion::v100)
        {
            // Check unknown bits that indicate it's an 1.04 - 1.06 not 1.03 or less
            if (readBits(GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + ITEM_TYPE_BIT_OFFSET, 2) != 0)
            {
                ItemVersion = EnumItemVersion::v104; // v1.04 - v1.06 item
            }
            else if (bSerializedFormat)
            {
                value = 0;
                node = itemRoot["IsV104"];
                if (!node.isNull() && (node.asInt64() != 0))
                {
                    ItemVersion = EnumItemVersion::v104; // v1.04 - v1.06 item
                    auto tempOffset = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + ITEM_TYPE_BIT_OFFSET;
                    if (!setBits(tempOffset, 2, 0x3))
                    {
                        return false;
                    }
                }
            }
        }
        else if (ItemVersion == EnumItemVersion::v104)
        {
            auto tempOffset = GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET) + ITEM_TYPE_BIT_OFFSET;
            if (!setBits(tempOffset, 2, 0x3))
            {
                return false;
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
        case EnumItemVersion::v100: // v1.00 - v1.03 item
            if (!readItemv100(itemRoot, bSerializedFormat))
            {
                return false;
            }

            if (isEar() || isSimpleItem())
            {
                numSocketed = 0;
            }
            break;

        case EnumItemVersion::v104: // v1.04 - v1.06 item
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

            auto gemApplyType = getGemApplyType();
            auto iter_end = node.end();
            for (auto iter = node.begin(); iter != iter_end; ++iter)
            {
                SocketedItems.resize(SocketedItems.size() + 1);
                auto& childItem = SocketedItems.back();
                childItem.item_current_socket_idx = SocketedItems.size() - 1;
                if (!childItem.readItem(*iter, bSerializedFormat, version, isExpansion))
                {
                    return false;
                }

                if (childItem.getLocation() != EnumItemLocation::SOCKET)
                {
                    // corrupt file
                    return false;
                }

                // resolve magical properties of socketed gem
                childItem.item_current_socket_idx = 0;
                const auto& childItemType = childItem.getItemTypeHelper();
                childItemType.getSocketedMagicalAttributes(childItem, childItem.socketedMagicalAttributes, gemApplyType);
            }
        }

        return true;
    }

    value = rawVersion;
    size_t bitSize = (ItemVersion < EnumItemVersion::v100R ? 10 : 3);
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET) = current_bit_offset;
    node = itemRoot[bSerializedFormat ? "Mode" : "location_id"];
    if (node.isNull())
    {
        return false;
    }

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::LOCATION_BIT_OFFSET);
    value = std::uint16_t(node.asInt64());
    bitSize = 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // if not equipped, equipped_id could be missing
    auto itemLocation = getLocation();
    bitSize = 4;
    GET_BIT_OFFSET(ItemOffsets::EQUIPPED_ID_OFFSET) = current_bit_offset;
    value = 0;
    node = itemRoot[bSerializedFormat ? "Location" : "equipped_id"];
    if (!node.isNull())
    {
        switch (itemLocation)
        {
        case EnumItemLocation::EQUIPPED:
            value = std::uint16_t(node.asInt64());
            break;

        default:
            value = 0;
            break;
        }
    }

    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // position x/y
    GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET) = current_bit_offset;
    bitSize = 4;

    node = itemRoot[bSerializedFormat ? "X" : "position_x"];
    if (node.isNull())
    {
        return false;
    }

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::POSITION_OFFSET);
    value = std::uint16_t(node.asInt64());
    switch (itemLocation)
    {
    case EnumItemLocation::SOCKET:
        value = std::uint16_t(item_current_socket_idx);
        break;
    }

    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    bitSize = 4;
    node = itemRoot[bSerializedFormat ? "Y" : "position_y"];
    switch (itemLocation)
    {
    case EnumItemLocation::SOCKET:
        value = 0;
        break;

    case EnumItemLocation::STORED:
        if (node.isNull())
        {
            return false;
        }

        value = std::uint16_t(node.asInt64());
        break;

    default:
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }
        break;
    }

    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // alt position x/y
    GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET) = current_bit_offset;
    bitSize = 3;

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::ALT_POSITION_ID_OFFSET);
    node = itemRoot[bSerializedFormat ? "Page" : "alt_position_id"];
    switch (itemLocation)
    {
    case EnumItemLocation::STORED:
        if (node.isNull())
        {
            return false;
        }

        value = std::uint16_t(node.asInt64());
        break;

    default:
        value = 0;
        break;
    }

    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    // type code
    GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) = current_bit_offset;
    bitSize = 32;

    if (isEar())
    {
        Json::Value earRoot = bSerializedFormat ? itemRoot : itemRoot["ear_attributes"];
        if (earRoot.isNull())
        {
            return false;
        }

        EarAttributes earAttrib;
        node = earRoot[bSerializedFormat ? "FileIndex" : "class_id"];
        if (node.isNull())
        {
            if (bSerializedFormat)
            {
                return false;
            }

            node = earRoot["class"];
            if (node.isNull())
            {
                return false;
            }

            std::string className = node.asString();
            if (!CharClassHelper::getEnumCharClassByName(className, earAttrib.Class))
            {
                if (!CharClassHelper::getEnumCharClassByIndex(className, earAttrib.Class))
                {
                    return false;
                }
            }
        }
        else
        {
            value = std::uint32_t(node.asInt64());
            if (value > std::uint8_t(NUM_OF_CLASSES_NO_EXPANSION))
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
        // Remove any invalid characters from the name
        if (!ItemHelpers::ProcessNameNode(node, earAttrib.Name, ItemVersion))
        {
            return false;
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
        if (!setBits(current_bit_offset, 3, std::uint32_t(earAttrib.Class)))
        {
            return false;
        }

        if (!setBits(current_bit_offset, 7, earAttrib.Level))
        {
            return false;
        }

        // up to 15 7/8 bit characters
        bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
        for (size_t idx = 0; idx <= 15; ++idx)
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
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = max_bit_offset;
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

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET);
    switch (ItemVersion)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        return false; // should not get here

    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
    case EnumItemVersion::v109: // v1.09 item
    case EnumItemVersion::v110: // v1.10 - v1.14d item
        value = *((std::uint32_t*)strcode.data());
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        ItemHelpers::encodeResurrectedItem(strcode, value64, numBitsSet);
        bitSize = numBitsSet;
        if (!setBits64(current_bit_offset, bitSize, value64))
        {
            return false;
        }
        break;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) = max_bit_offset;

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
    value = 0;
    bitSize = bIsCompact ? 1 : 3;
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) = 0;
    GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = 0;
    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    if (bIsCompact)
    {
        if (itemType.isGoldItem())
        {
            // Is this correct for gld items? It's not currently used, so is it even needed?
            GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) + 1;

            node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
            if (node.isNull())
            {
                return false;
            }
            // can hold up to 4095 gold pieces
            value = std::min(std::uint32_t(node.asInt64()), MAX_GLD_QUANTITY);
            bitSize = GLD_STACKABLE_NUM_BITS;
            current_bit_offset = GET_BIT_OFFSET(ItemOffsets::GLD_STACKABLE_BIT_OFFSET);
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = current_bit_offset;
            nr_of_items_in_sockets_bits = 1;
        }
        else
        {
            GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
            nr_of_items_in_sockets_bits = 1;
            if (itemType.isQuestItem())
            {
                GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
                GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) + 2;
                nr_of_items_in_sockets_bits = 1;

                value = 0;
                bitSize = 2;
                node = itemRoot[bSerializedFormat ? "QuestDifficulty" : "quest_difficulty"];
                if (!node.isNull())
                {
                    value = std::uint32_t(node.asInt64());
                }

                current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET);
                if (!setBits(current_bit_offset, bitSize, value))
                {
                    return false;
                }
                max_bit_offset = std::max(max_bit_offset, current_bit_offset);
            }
        }

        GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = current_bit_offset;
        return true;
    }
    else if (itemType.isQuestItem())
    {
        GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
        GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET) + 2;
        nr_of_items_in_sockets_bits = 1;

        value = 0;
        bitSize = 2;
        node = itemRoot[bSerializedFormat ? "QuestDifficulty" : "quest_difficulty"];
        if (!node.isNull())
        {
            value = std::uint32_t(node.asInt64());
        }

        current_bit_offset = GET_BIT_OFFSET(ItemOffsets::QUEST_DIFFICULTY_OFFSET);
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }
    else
    {
        GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET) = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
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

    current_bit_offset = GET_BIT_OFFSET(ItemOffsets::NR_OF_ITEMS_IN_SOCKETS_OFFSET);
    if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) = current_bit_offset;
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

    GET_BIT_OFFSET(ItemOffsets::ITEM_LEVEL_BIT_OFFSET) = current_bit_offset;
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

    GET_BIT_OFFSET(ItemOffsets::QUALITY_BIT_OFFSET) = current_bit_offset;
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
    GET_BIT_OFFSET(ItemOffsets::MULTI_GRAPHIC_BIT_OFFSET) = current_bit_offset;
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
    GET_BIT_OFFSET(ItemOffsets::AUTOAFFIX_BIT_OFFSET) = current_bit_offset;
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

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::QUALITY_ATTRIB_BIT_OFFSET_MARKER) = current_bit_offset;
    GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = 0;
    switch (quality)
    {
    case EnumItemQuality::INFERIOR:
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
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

    case EnumItemQuality::NORMAL:
        if (itemType.isCharm())
        {
            // If the item is a NORMAL quality Charm, the next 12 bits will contain the Charm specific data.
            GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
            value = 0;
            node = itemRoot[bSerializedFormat ? "FileIndex" : "file_index"];
            if (!node.isNull())
            {
                value = std::uint16_t(node.asInt64());
            }

            bitSize = NORMAL_CHARM_ID_NUM_BITS;
            if (!setBits(current_bit_offset, bitSize, value))
            {
                return false;
            }
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        }
        break;

    case EnumItemQuality::SUPERIOR:
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
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
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
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
    case EnumItemQuality::CRAFTED:
    case EnumItemQuality::TEMPERED:
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
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
                node = itemRoot["rare_index"];
                if (node.isNull())
                {
                    node = itemRoot["rare_name"];
                    if (node.isNull())
                    {
                        return false;
                    }

                    value = ItemHelpers::getIdFromRareName(node.asString());
                }
                else
                {
                    value = ItemHelpers::getIdFromRareIndex(node.asString());
                }

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
                node = itemRoot["rare_index2"];
                if (node.isNull())
                {
                    node = itemRoot["rare_name2"];
                    if (node.isNull())
                    {
                        return false;
                    }

                    value = ItemHelpers::getIdFromRareName(node.asString());
                }
                else
                {
                    value = ItemHelpers::getIdFromRareIndex(node.asString());
                }

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

        // Following the name IDs, we got 6 possible magical affixes, the pattern is
        // 1 bit id, 11 bit value... But the value will only exist if the prefix is 1. 
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
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
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
        GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) = current_bit_offset;
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

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_ID_BIT_OFFSET_MARKER) = current_bit_offset;
    if (isRuneword())
    {
        GET_BIT_OFFSET(ItemOffsets::RUNEWORD_ID_BIT_OFFSET) = current_bit_offset;
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

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::PERSONALIZED_BIT_OFFSET_MARKER) = current_bit_offset;
    if (isPersonalized())
    {
        GET_BIT_OFFSET(ItemOffsets::PERSONALIZED_BIT_OFFSET) = current_bit_offset;
        node = itemRoot[bSerializedFormat ? "PlayerName" : "personalized_name"];
        if (node.isNull())
        {
            return false;
        }

        // Check Name
        // Remove any invalid characters from the name
        std::array<char, NAME_LENGTH> playerName;
        if (!ItemHelpers::ProcessNameNode(node, playerName, ItemVersion))
        {
            return false;
        }

        // up to 15 7/8 bit characters
        bitSize = (ItemVersion >= EnumItemVersion::v120) ? 8 : 7;
        for (size_t idx = 0; idx <= 15; ++idx)
        {
            if (!setBits(current_bit_offset, bitSize, std::uint32_t(playerName[idx])))
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

    bool isArmor = itemType.isArmor();
    bool isWeapon = itemType.isWeapon();
    bool isTome = itemType.isBook();
    bool isStackable = itemType.isStackable();

    if (isTome)
    {
        // If the item is a tome, it will contain 5 extra bits, we're not
        // interested in these bits, the value is usually 1, but not sure
        // what it is.
        GET_BIT_OFFSET(ItemOffsets::TOME_BIT_OFFSET) = current_bit_offset;
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
    else if (itemType.isBodyPart())
    {
        // If the item is a body part, it will contain 10 extra bits indicating
        // the monster Id from monstat.txt
        GET_BIT_OFFSET(ItemOffsets::BODY_PART_BIT_OFFSET) = current_bit_offset;
        value = 0;
        node = itemRoot[bSerializedFormat ? "FileIndex" : "file_index"];
        if (!node.isNull())
        {
            value = std::uint16_t(node.asInt64());
        }

        bitSize = MONSTER_ID_NUM_BITS;
        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    // Realm Data Flag
    GET_BIT_OFFSET(ItemOffsets::REALM_BIT_OFFSET) = current_bit_offset;
    value = 0;
    bitSize = 1;
    size_t realmBits;
    std::vector<std::uint32_t> realmData;
    if (!unknowns.isNull())
    {
        node = unknowns["realm_data"];
        if (!node.isNull())
        {
            if (!itemType.isMiscellaneous() || itemType.isGem() || itemType.isRing() ||
                itemType.isAmulet() || itemType.isCharm() || itemType.isRune())
            {
                realmBits = (ItemVersion >= EnumItemVersion::v100R) ? REAL_DATA_NUM_BITS : REAL_DATA_NUM_BITS_110;
                auto numItems = ((realmBits + 31) / 32);
                if (node.size() == numItems)
                {
                    value = 1;
                    auto iter_end = node.end();
                    for (auto iter = node.begin(); iter != iter_end; ++iter)
                    {
                        realmData.push_back(std::uint32_t(iter->asInt64()));
                    }
                }
            }
            else if (node.size() == 1)
            {
                value = 1;
                realmBits = 3;
                realmData.push_back(std::uint32_t(node[0].asInt64()));
            }
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }

        if (value == 1)
        {
            for (const auto& realmValue : realmData)
            {
                auto writeSize = std::min(realmBits, size_t(32));
                if (writeSize == 0)
                {
                    break;
                }

                realmBits -= writeSize;
                if (!setBits(current_bit_offset, writeSize, realmValue))
                {
                    return false;
                }
            }
        }
    }
    else if (!setBits(current_bit_offset, bitSize, value))
    {
        return false;
    }
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    if (isArmor || isWeapon)
    {
        if (isArmor)
        {
            // Defense rating
            const auto& stat = ItemHelpers::getItemStat(getVersion(), "armorclass");
            GET_BIT_OFFSET(ItemOffsets::DEFENSE_RATING_BIT_OFFSET) = current_bit_offset;
            value = stat.saveAdd;
            bitSize = (ItemVersion >= EnumItemVersion::v110) ? DEFENSE_RATING_NUM_BITS : DEFENSE_RATING_NUM_BITS_108;
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
        GET_BIT_OFFSET(ItemOffsets::DURABILITY_BIT_OFFSET) = current_bit_offset;
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

        if (value > 0)
        {
            // current durability value (8 bits + unknown single bit)
            bitSize = (ItemVersion >= EnumItemVersion::v110) ? DURABILITY_CURRENT_NUM_BITS : DURABILITY_CURRENT_NUM_BITS_108;
            value = 0;
            node = itemRoot[bSerializedFormat ? "Durability" : "current_durability"];
            if (!node.isNull())
            {
                auto mask = (1ui16 << bitSize) - 1;
                value = std::uint16_t(node.asInt64() & mask);
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
        GET_BIT_OFFSET(ItemOffsets::STACKABLE_BIT_OFFSET) = current_bit_offset;
        value = 0;
        bitSize = STACKABLE_NUM_BITS;
        node = itemRoot[bSerializedFormat ? "Quantity" : "quantity"];
        if (!node.isNull())
        {
            value = std::min(std::uint32_t(node.asInt64()), MAX_STACKED_QUANTITY);
        }

        if (!setBits(current_bit_offset, bitSize, value))
        {
            return false;
        }
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SOCKET_COUNT_BIT_OFFSET_MARKER) = current_bit_offset;
    if (bIsSocketed)
    {
        // If the item is socketed, it will contain 4 bits of data which are the
        // number of total sockets the item have, regardless of how many are occupied
        // by an item.
        GET_BIT_OFFSET(ItemOffsets::SOCKET_COUNT_BIT_OFFSET) = current_bit_offset;
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
    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::BONUS_BITS_BIT_OFFSET_MARKER) = current_bit_offset;
    if (quality == EnumItemQuality::SET)
    {
        GET_BIT_OFFSET(ItemOffsets::BONUS_BITS_BIT_OFFSET) = current_bit_offset;
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
    GET_BIT_OFFSET(ItemOffsets::MAGICAL_PROPS_BIT_OFFSET) = current_bit_offset;
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
    max_bit_offset = std::max(max_bit_offset, current_bit_offset);

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::SET_BONUS_PROPS_BIT_OFFSET_MARKER) = current_bit_offset;
    if (setBonusBits > 0)
    {
        // Item has more magical property lists due to being a set item
        GET_BIT_OFFSET(ItemOffsets::SET_BONUS_PROPS_BIT_OFFSET) = current_bit_offset;
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
            if (iter == iter_end)
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
            max_bit_offset = std::max(max_bit_offset, current_bit_offset);
        }
    }

    GET_BIT_OFFSET_MARKER(ItemOffsetMarkers::RUNEWORD_PROPS_BIT_OFFSET_MARKER) = current_bit_offset;
    if (isRuneword())
    {
        // runewords have their own list of magical properties
        GET_BIT_OFFSET(ItemOffsets::RUNEWORD_PROPS_BIT_OFFSET) = current_bit_offset;
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
        max_bit_offset = std::max(max_bit_offset, current_bit_offset);
    }
    
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) = max_bit_offset;
    if (numSocketed > 0)
    {
        node = itemRoot[bSerializedFormat ? "SocketedItems" : "socketed_items"];
        if (node.isNull() || !node.isArray() || node.size() != numSocketed)
        {
            return false;
        }

        auto gemApplyType = getGemApplyType();
        auto iter_end = node.end();
        for (auto iter = node.begin(); iter != iter_end; ++iter)
        {
            SocketedItems.resize(SocketedItems.size() + 1);
            auto& childItem = SocketedItems.back();
            childItem.item_current_socket_idx = SocketedItems.size() - 1;
            if (!childItem.readItem(*iter, bSerializedFormat, version, isExpansion))
            {
                return false;
            }

            if (childItem.getLocation() != EnumItemLocation::SOCKET)
            {
                // corrupt file
                return false;
            }

            // resolve magical properties of socketed gem
            childItem.item_current_socket_idx = 0;
            const auto& childItemType = childItem.getItemTypeHelper();
            childItemType.getSocketedMagicalAttributes(childItem, childItem.socketedMagicalAttributes, gemApplyType);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Item::asJson(Json::Value& parent, std::uint32_t charLevel, EnumItemVersion version, bool bSerializedFormat) const
{
    if (version == ItemVersion)
    {
        asJson(parent, charLevel, bSerializedFormat);
        return;
    }

    bool isExpansion = isExpansionGame();
    bool bIsPersonalized = isPersonalized();

    // We are converting versions, so strict compliance on game version and raw version values
    auto gameVersion = getGameVersion();
    auto rawVersion = getRawVersion();
    switch (version)
    {
    case EnumItemVersion::v100: // v1.00 - v1.03
    case EnumItemVersion::v104: // v1.04 - v1.06
        isExpansion = false;
        bIsPersonalized = false;
        gameVersion = 0;
        rawVersion = 0;
        break;

    case EnumItemVersion::v107: // v1.07
        gameVersion = isExpansion ? 100 : 0;
        rawVersion = 0;
        break;

    case EnumItemVersion::v108: // v1.08
    case EnumItemVersion::v109: // v1.09
        switch (rawVersion)
        {
        case 100:
        case 101:
        case 5:
            gameVersion = 100;
            rawVersion = 100;
            break;

        case 0:
        case 1:
        case 2:
        case 4:
            gameVersion = 1;
            rawVersion = 1;
            break;

        default:
            gameVersion = isExpansion ? 100 : 1;
            rawVersion = isExpansion ? 100 : 1;
            break;
        }
        break;

    case EnumItemVersion::v110: // v1.10 - v1.14d
        switch (rawVersion)
        {
        case 100:
        case 101:
        case 5:
            gameVersion = 100;
            rawVersion = 101;
            break;

        case 0:
        case 1:
        case 2:
        case 4:
            gameVersion = 1;
            rawVersion = 2;
            break;

        default:
            gameVersion = isExpansion ? 100 : 1;
            rawVersion = isExpansion ? 101 : 1;
            break;
        }
        break;

    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        switch (rawVersion)
        {
        case 100:
        case 101:
        case 5:
            gameVersion = 100;
            rawVersion = 5;
            break;

        case 0:
        case 1:
        case 2:
        case 4:
            gameVersion = 1;
            rawVersion = 4;
            break;

        default:
            gameVersion = isExpansion ? 100 : 1;
            rawVersion = isExpansion ? 5 : 4;
            break;
        }
        break;
    }

    bool bIsEar = isEar();
    EarAttributes earAttrib;
    if (bIsEar)
    {
        getEarAttributes(earAttrib);
    }

    if (version < ItemVersion)
    {
        // check stash location and item to make sure it is valid
        const auto& itemType = getItemTypeHelper();
        if ((getLocation() == EnumItemLocation::STORED) && (getAltPositionId() == EnumAltItemLocation::STASH))
        {
            if ((ItemVersion >= d2ce::EnumItemVersion::v100R && version < d2ce::EnumItemVersion::v100R) ||
                (isExpansionGame() && !isExpansion))
            {

                // STASH is a 6 x 4/8 grid
                ItemDimensions stashDimensions;
                stashDimensions.Width = stashDimensions.InvWidth = 6;
                stashDimensions.Height = stashDimensions.InvHeight = !isExpansion ? 4 : 8;
                if (((getPositionX() + itemType.dimensions.Width - 1) > stashDimensions.Width) ||
                    ((getPositionY() + itemType.dimensions.Height - 1) > stashDimensions.Height))
                {
                    return;
                }
            }
        }

        if (itemType.isExpansionItem() && !isExpansion)
        {
            // invalid item
            return;
        }

        // check for personaliztion string
        if ((ItemVersion >= d2ce::EnumItemVersion::v100R && version < d2ce::EnumItemVersion::v100R) ||
            (isExpansionGame() && !isExpansion))
        {
            if ((ItemVersion >= d2ce::EnumItemVersion::v120) && (version <= d2ce::EnumItemVersion::v100R))
            {
                if (bIsPersonalized)
                {
                    // personalization has changed, make sure they are all 7 bit characters
                    for (std::uint8_t singleChar : getPersonalizedName())
                    {
                        if (singleChar > 0x7F)
                        {
                            bIsPersonalized = false;
                            break;
                        }
                    }
                }
                else if (bIsEar)
                {
                    // player name has changed, make sure they are all 7 bit characters
                    for (std::uint8_t singleChar : earAttrib.Name)
                    {
                        if (singleChar > 0x7F)
                        {
                            earAttrib.Name.fill(0);
                            strcpy_s(earAttrib.Name.data(), earAttrib.Name.size(), "SOMEONE");
                            break;
                        }
                    }
                }
            }
        }
    }

    std::vector<MagicalAttribute> magicalAttributes;
    d2ce::RunewordAttributes runewordAttrib;
    Json::Value item;
    if (bSerializedFormat)
    {
        if (version < EnumItemVersion::v100R)
        {
            item["Header"] = *((std::uint16_t*)ITEM_MARKER.data());

            std::stringstream ss;
            ss << std::dec << rawVersion;
            item["Version"] = ss.str();
            if (version == EnumItemVersion::v104)
            {
                item["IsV104"] = true;
            }
        }
        else
        {
            switch (rawVersion)
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

        if (bIsEar)
        {
            item["EarLevel"] = earAttrib.getLevel();
            item["PlayerName"] = earAttrib.Name.data();
            item["Code"] = "";
        }
        else
        {
            item["EarLevel"] = 0;
            if (bIsPersonalized)
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

        item["TotalNumberOfSockets"] = std::uint16_t(getSocketCount());

        // Socketed items
        Json::Value socketedItems(Json::arrayValue);
        for (auto& socketedItem : SocketedItems)
        {
            socketedItem.asJson(socketedItems, charLevel, version, bSerializedFormat);
        }
        item["NumberOfSocketedItems"] = std::uint16_t(socketedItems.size());
        item["SocketedItems"] = socketedItems;

        auto id = getId();
        auto quality = getQuality();
        item["Id"] = id;
        item["ItemLevel"] = std::uint16_t(getLevel());
        item["Dwa"] = id == 0 ? ItemHelpers::generarateRandomDW() : id;
        item["Dwb"] = getDWBCode();

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
        case EnumItemQuality::CRAFTED:
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
            auto durValue = durability.Current & 0xFF;
            if (durability.CurrentBit9)
            {
                durValue |= 0x100;
            }
            item["Durability"] = durValue;
        }
        else if (!isExpansion && itemType.isThrownWeapon())
        {
            // Durability on throwing weapons is only in the Expansion
            item["MaxDurability"] = itemType.durability.Max;
            item["Durability"] = itemType.durability.Max;
        }
        item["Quantity"] = getQuantity();
        item["SetItemMask"] = getSetItemMask();

        Json::Value statLists(Json::arrayValue);
        if (!isSimpleItem())
        {
            getMagicalAttributes(magicalAttributes);
            MagicalAttribute::attributesAsJsonArray(statLists, magicalAttributes, bSerializedFormat);

            if (isExpansion && isRuneword())
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
        item["IsEar"] = bIsEar;
        item["IsStarterItem"] = isStarterItem();
        item["IsCompact"] = isSimpleItem();
        item["IsEthereal"] = isEthereal();
        item["IsPersonalized"] = bIsPersonalized;
        item["IsRuneword"] = isExpansion && isRuneword();
        unknownAsJson(item, bSerializedFormat);

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
        unknownAsJson(item, bSerializedFormat);
        item["identified"] = (isIdentified() ? 1 : 0);
        item["socketed"] = (isSocketed() ? 1 : 0);
        item["new"] = (isNew() ? 1 : 0);
        item["is_ear"] = (bIsEar ? 1 : 0);
        item["starter_item"] = (isStarterItem() ? 1 : 0);
        item["simple_item"] = (isSimpleItem() ? 1 : 0);
        item["ethereal"] = (isEthereal() ? 1 : 0);
        item["personalized"] = (bIsPersonalized ? 1 : 0);
        item["given_runeword"] = (isExpansion && isRuneword() ? 1 : 0);
        item["version"] = rawVersion;
        item["location_id"] = std::uint16_t(getLocation());
        item["equipped_id"] = std::uint16_t(getEquippedId());
        item["position_x"] = std::uint16_t(getPositionX());
        item["position_y"] = std::uint16_t(getPositionY());
        item["alt_position_id"] = std::uint16_t(getAltPositionId());

        if (bIsEar)
        {
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

        // For compatibility with other Json exports we add type_id
        if (itemType.isWeapon())
        {
            item["type_id"] = 3;
        }
        else if (itemType.isShield())
        {
            item["type_id"] = 2;
        }
        else if (itemType.isArmor())
        {
            item["type_id"] = 1;
        }
        else
        {
            item["type_id"] = 4;
        }

        if (itemType.isQuestItem())
        {
            item["quest_difficulty"] = std::uint16_t(getQuestDifficulty());
        }

        if (isSimpleItem() && (version != EnumItemVersion::v100))
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
            auto id = getId();
            item["id"] = id;
            item["level"] = std::uint16_t(getLevel());
            item["dwa"] = id == 0 ? ItemHelpers::generarateRandomDW() : id;
            item["dwb"] = getDWBCode();

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

            d2ce::MagicalAffixes magicalAffixes;
            d2ce::SetAttributes setAttrib;
            d2ce::RareAttributes rareAttrib;
            d2ce::UniqueAttributes uniqueAttrib;
            switch (quality)
            {
            case EnumItemQuality::INFERIOR:
                item["low_quality_id"] = std::uint16_t(getInferiorQualityId());
                break;

            case EnumItemQuality::NORMAL:
                if (itemType.isCharm())
                {
                    item["file_index"] = std::uint16_t(getFileIndex());
                }
                break;

            case EnumItemQuality::SUPERIOR:
                if (itemType.isCharm())
                {
                    item["file_index"] = std::uint16_t(getFileIndex());
                }
                break;

            case EnumItemQuality::MAGIC:
                getMagicalAffixes(magicalAffixes);
                magicalAffixes.asJson(item);
                break;

            case EnumItemQuality::SET:
                getSetAttributes(setAttrib);
                setAttrib.asJson(item);
                break;

            case EnumItemQuality::RARE:
            case EnumItemQuality::CRAFTED:
            case EnumItemQuality::TEMPERED:
                getRareOrCraftedAttributes(rareAttrib);
                rareAttrib.asJson(item);
                break;

            case EnumItemQuality::UNIQUE:
                getUniqueAttributes(uniqueAttrib);
                uniqueAttrib.asJson(item);
                break;
            }

            if (isExpansion && isRuneword())
            {
                getRunewordAttributes(runewordAttrib);
                runewordAttrib.asJson(item);
            }

            if (bIsPersonalized)
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
                    auto durValue = durability.Current & 0xFF;
                    if (durability.CurrentBit9)
                    {
                        durValue |= 0x100;
                    }
                    item["current_durability"] = durValue;
                }
                else if (!isExpansion && itemType.isThrownWeapon())
                {
                    // Durability on throwing weapons is only in the Expansion
                    item["max_durability"] = itemType.durability.Max;
                    item["current_durability"] = itemType.durability.Max;
                }
            }

            if (isSocketed())
            {
                item["total_nr_of_sockets"] = std::uint16_t(getSocketCount());
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
                    socketedItem.asJson(socketedItems, charLevel, version, bSerializedFormat);
                }
                item["nr_of_items_in_sockets"] = std::uint16_t(socketedItems.size());
                item["socketed_items"] = socketedItems;
            }
        }

        item["type_name"] = itemType.name;

        ItemRequirements req;
        if (!getDisplayedRequirements(req, charLevel))
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
void d2ce::Item::asJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat) const
{
    std::vector<MagicalAttribute> magicalAttributes;
    d2ce::RunewordAttributes runewordAttrib;
    Json::Value item;
    if (bSerializedFormat)
    {
        if (ItemVersion < EnumItemVersion::v100R)
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
            item["PlayerName"] = earAttrib.getNameAsString();
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

        item["NumberOfSocketedItems"] = std::uint16_t(getSocketedItemCount());
        item["TotalNumberOfSockets"] = std::uint16_t(getSocketCount());

        // Socketed items
        Json::Value socketedItems(Json::arrayValue);
        for (auto& socketedItem : SocketedItems)
        {
            socketedItem.asJson(socketedItems, charLevel, bSerializedFormat);
        }
        item["SocketedItems"] = socketedItems;

        item["Id"] = getId();
        item["ItemLevel"] = std::uint16_t(getLevel());

        if (GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) != 0)
        {
            item["Dwa"] = readBits(GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET), 32);
            item["Dwb"] = readBits(GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET), 32);
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
        case EnumItemQuality::CRAFTED:
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
            auto durValue = durability.Current & 0xFF;
            if (durability.CurrentBit9)
            {
                durValue |= 0x100;
            }
            item["Durability"] = durValue;
        }
        else if (!isExpansionGame() && itemType.isThrownWeapon())
        {
            // Durability on throwing weapons is only in the Expansion
            item["MaxDurability"] = itemType.durability.Max;
            item["Durability"] = itemType.durability.Max;
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
        unknownAsJson(item, bSerializedFormat);

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
        unknownAsJson(item, bSerializedFormat);
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

        // For compatibility with other Json exports we add type_id
        if (itemType.isWeapon())
        {
            item["type_id"] = 3;
        }
        else if (itemType.isShield())
        {
            item["type_id"] = 2;
        }
        else if (itemType.isArmor())
        {
            item["type_id"] = 1;
        }
        else
        {
            item["type_id"] = 4;
        }

        if (itemType.isQuestItem())
        {
            item["quest_difficulty"] = std::uint16_t(getQuestDifficulty());
        }
        item["nr_of_items_in_sockets"] = std::uint16_t(getSocketedItemCount());

        if (isSimpleItem() && (ItemVersion != EnumItemVersion::v100))
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

            if (GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) != 0)
            {
                item["dwa"] = readBits(GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET), 32);
                item["dwb"] = readBits(GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET), 32);
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

            d2ce::MagicalAffixes magicalAffixes;
            d2ce::SetAttributes setAttrib;
            d2ce::RareAttributes rareAttrib;
            d2ce::UniqueAttributes uniqueAttrib;
            switch (quality)
            {
            case EnumItemQuality::INFERIOR:
                item["low_quality_id"] = std::uint16_t(getInferiorQualityId());
                break;

            case EnumItemQuality::NORMAL:
                if (itemType.isCharm())
                {
                    item["file_index"] = std::uint16_t(getFileIndex());
                }
                break;

            case EnumItemQuality::SUPERIOR:
                item["file_index"] = std::uint16_t(getFileIndex());
                break;

            case EnumItemQuality::MAGIC:
                getMagicalAffixes(magicalAffixes);
                magicalAffixes.asJson(item);
                break;

            case EnumItemQuality::SET:
                getSetAttributes(setAttrib);
                setAttrib.asJson(item);
                break;

            case EnumItemQuality::RARE:
            case EnumItemQuality::CRAFTED:
            case EnumItemQuality::TEMPERED:
                getRareOrCraftedAttributes(rareAttrib);
                rareAttrib.asJson(item);
                break;

            case EnumItemQuality::UNIQUE:
                getUniqueAttributes(uniqueAttrib);
                uniqueAttrib.asJson(item);
                break;
            }

            if (isRuneword())
            {
                getRunewordAttributes(runewordAttrib);
                runewordAttrib.asJson(item);
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
                    auto durValue = durability.Current & 0xFF;
                    if (durability.CurrentBit9)
                    {
                        durValue |= 0x100;
                    }
                    item["current_durability"] = durValue;
                }
                else if (!isExpansionGame() && itemType.isThrownWeapon())
                {
                    // Durability on throwing weapons is only in the Expansion
                    item["max_durability"] = itemType.durability.Max;
                    item["current_durability"] = itemType.durability.Max;
                }
            }

            if (isSocketed())
            {
                item["total_nr_of_sockets"] = std::uint16_t(getSocketCount());
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
        if (!getDisplayedRequirements(req, charLevel))
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
        bitRangeAsJson(unknownData, byteInfo.startIdx, byteInfo.endIdx);
    }

    if (getRealmDataFlag())
    {
        size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::REALM_BIT_OFFSET) + 1;

        Json::Value unknownBytes;
        auto itemType = getItemTypeHelper();
        if (!itemType.isMiscellaneous() || itemType.isGem() || itemType.isRing() ||
            itemType.isAmulet() || itemType.isCharm() || itemType.isRune())
        {
            auto realmBits = (ItemVersion >= EnumItemVersion::v100R) ? REAL_DATA_NUM_BITS : REAL_DATA_NUM_BITS_110;
            std::uint16_t numBits = 32;
            while (realmBits > 0)
            {
                auto readSize = std::min(realmBits, numBits);
                std::stringstream ss;
                ss << "b" << std::dec << current_bit_offset;
                ss << "_" << std::dec << (current_bit_offset + readSize);
                unknownBytes[ss.str()] = readBitsEx(current_bit_offset, readSize);
                realmBits -= readSize;
            }
        }
        else
        {
            std::uint16_t numBits = 3;
            std::stringstream ss;
            ss << "b" << std::dec << current_bit_offset;
            ss << "_" << std::dec << (current_bit_offset + numBits);
            unknownBytes[ss.str()] = readBitsEx(current_bit_offset, numBits);
        }
        unknownData["realm_data"] = unknownBytes;
    }
    parent["_unknown_data"] = unknownData;
}
//---------------------------------------------------------------------------
void d2ce::Item::bitRangeAsJson(Json::Value& parent, size_t startBit, size_t endBit) const
{
    if (endBit < startBit)
    {
        return;
    }

    std::string propName;
    {
        std::stringstream ss;
        ss << "b" << std::dec << startBit;
        if (endBit > startBit)
        {
            ss << "_" << std::dec << endBit;
        }

        propName = ss.str();
    }

    Json::Value unknownData;
    startBit += GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET);
    endBit += GET_BIT_OFFSET(ItemOffsets::START_BIT_OFFSET);
    for (size_t idx = startBit; idx <= endBit; ++idx)
    {
        std::stringstream ss;
        ss << std::dec << (idx - startBit);
        unknownData[ss.str()] = readBits(idx, 1);
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

    std::uint16_t nextInChain = 0;
    while (id != 0x1FF)
    {
        const auto& stat = ItemHelpers::getItemStat(getVersion(), id);
        if (id != stat.id)
        {
            // corrupt file
            return false;
        }

        // saveBits being zero or >= 64 is unrecoverably bad, and
        if (stat.saveBits == 0 || stat.saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        size_t numParms = 0;
        if (stat.encode == 2)
        {
            if (stat.saveBits != 7 || stat.saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat.saveBits) + size_t(stat.saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 3;
        }
        else if (stat.encode == 3)
        {
            if (stat.saveBits != 16 || stat.saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat.saveBits) + size_t(stat.saveParamBits)))
            {
                // corrupt file
                return false;
            }

            numParms = 4;
        }
        else if (stat.encode == 4)
        {
            // time-based stats were never implemented, but we handle them
            numParms = 3;
            if (stat.saveBits != 22 || stat.saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, size_t(stat.saveBits) + size_t(stat.saveParamBits)))
            {
                // corrupt file
                return false;
            }
        }
        else if (stat.saveParamBits > 0)
        {
            if (!skipBits(charfile, current_bit_offset, size_t(stat.saveBits) + size_t(stat.saveParamBits)))
            {
                // corrupt file
                return false;
            }

            if (stat.name.compare("item_addskill_tab") == 0)
            {
                numParms = 3;
            }
            else
            {
                numParms = 2;
            }
        }
        else
        {
            if (!skipBits(charfile, current_bit_offset, stat.saveBits))
            {
                // corrupt file
                return false;
            }

            numParms = 1;
        }

        auto nextInChainIter = stat.nextInChain.begin();
        auto nextInChainIterEnd = stat.nextInChain.end();
        for (; nextInChainIter != nextInChainIterEnd && numParms < 4; ++nextInChainIter)
        {
            nextInChain = *nextInChainIter;
            const auto& statNext = ItemHelpers::getItemStat(getVersion(), *nextInChainIter);
            if (statNext.id != nextInChain)
            {
                // corrupt file
                return false;
            }

            if (statNext.saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (!skipBits(charfile, current_bit_offset, statNext.saveBits))
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
    std::int64_t value = 0;
    std::vector<std::int64_t> values;
    size_t valueIdx = 0;
    if (propListRoot.isNull() || !propListRoot.isArray() || propListRoot.empty())
    {
        id = 0x1FF;
        return setBits(current_bit_offset, PROPERTY_ID_NUM_BITS, id);
    }

    std::uint16_t nextInChain = 0;
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
        const auto& stat = ItemHelpers::getItemStat(getVersion(), id);
        if (id != stat.id)
        {
            return false;
        }

        if (!setBits(current_bit_offset, PROPERTY_ID_NUM_BITS, id))
        {
            return false;
        }

        // saveBits being zero or >= 64 is unrecoverably bad, and
        // encode type 4 is only used by stats that were never implemented (time-based stats)
        if (stat.saveBits == 0 || stat.saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        if (bSerializedFormat)
        {
            if (stat.encode == 2)
            {
                node = iter->operator[]("SkillLevel");
                values.push_back(SafeGetNodeValue(node));

                node = iter->operator[]("SkillId");
                values.push_back(SafeGetNodeValue(node));

                node = iter->operator[]("Value");
                values.push_back(SafeGetNodeValue(node));

                values.push_back(node.asInt64());
            }
            else if (stat.encode == 3)
            {
                node = iter->operator[]("SkillLevel");
                values.push_back(SafeGetNodeValue(node));

                node = iter->operator[]("SkillId");
                values.push_back(SafeGetNodeValue(node));

                node = iter->operator[]("MaxCharges");
                values.push_back(SafeGetNodeValue(node));

                node = iter->operator[]("Value");
                values.push_back(SafeGetNodeValue(node));
            }
            else if (id == 188)
            {
                node = iter->operator[]("SkillTab");
                values.push_back(SafeGetNodeValue(node));

                node = iter->operator[]("SkillLevel");
                values.push_back(SafeGetNodeValue(node));

                node = iter->operator[]("Value");
                values.push_back(SafeGetNodeValue(node));
            }
            else
            {
                if (stat.saveParamBits > 0)
                {
                    node = iter->operator[]("Param");
                    values.push_back(SafeGetNodeValue(node));
                }

                node = iter->operator[]("Value");
                values.push_back(SafeGetNodeValue(node));

                auto nextInChainIter = stat.nextInChain.begin();
                auto nextInChainIterEnd = stat.nextInChain.end();
                for (; nextInChainIter != nextInChainIterEnd && values.size() < 4; ++nextInChainIter)
                {
                    nextInChain = *nextInChainIter;

                    ++iter;
                    if (iter == iter_end)
                    {
                        // corrupt file
                        return false;
                    }

                    node = iter->operator[]("Id");
                    if (node.isNull())
                    {
                        // corrupt file
                        return false;
                    }

                    id = std::uint16_t(node.asInt64());
                    if (id != nextInChain)
                    {
                        // corrupt file
                        return false;
                    }

                    const auto& statNext = ItemHelpers::getItemStat(getVersion(), nextInChain);
                    if (statNext.id != nextInChain)
                    {
                        // corrupt file
                        return false;
                    }

                    if (statNext.saveParamBits != 0)
                    {
                        // corrupt file
                        return false;
                    }

                    node = iter->operator[]("Value");
                    values.push_back(SafeGetNodeValue(node));
                }
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
                values.push_back(SafeGetNodeValue(*iter2));
            }
        }

        if (stat.encode == 2)
        {
            if (stat.saveBits != 7 || stat.saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 6, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 10, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, stat.saveBits, value))
            {
                return false;
            }
        }
        else if (stat.encode == 3)
        {
            if (stat.saveBits != 16 || stat.saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 6, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 10, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 8, value))
            {
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }
            value = values[valueIdx] + stat.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, 8, value))
            {
                return false;
            }
        }
        else if (stat.encode == 4)
        {
            // time-based stats were never implemented, but we handle them
            if (stat.saveBits != 22 || stat.saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }

            if (values.size() == 1)
            {
                // all 22 bits stored in one value (old format that we should still support)
                value = values[valueIdx];
                ++valueIdx;
                if (!setBits64(current_bit_offset, stat.saveBits, value))
                {
                    return false;
                }
            }
            else
            {
                auto saveBits = stat.saveBits;
                value = values[valueIdx];
                ++valueIdx;
                saveBits -= 2;
                if (!setBits64(current_bit_offset, 2, value))
                {
                    return false;
                }

                if (valueIdx >= values.size())
                {
                    return false;
                }
                value = values[valueIdx];
                ++valueIdx;
                saveBits -= 10;
                if (!setBits64(current_bit_offset, 10, value))
                {
                    return false;
                }

                if (valueIdx >= values.size())
                {
                    return false;
                }
                value = values[valueIdx];
                ++valueIdx;
                if (!setBits64(current_bit_offset, saveBits, value))
                {
                    return false;
                }
            }
        }
        else if (stat.saveParamBits > 0)
        {
            if (valueIdx >= values.size())
            {
                return false;
            }

            if (stat.name.compare("item_addskill_tab") == 0)
            {
                value = values[valueIdx] + stat.saveAdd;
                ++valueIdx;
                if (!setBits64(current_bit_offset, 3, value))
                {
                    return false;
                }

                if (valueIdx >= values.size())
                {
                    return false;
                }
                value = values[valueIdx] + stat.saveAdd;
                ++valueIdx;
                if (!setBits64(current_bit_offset, 13, value))
                {
                    return false;
                }

                if (valueIdx >= values.size())
                {
                    return false;
                }
                value = values[valueIdx] + stat.saveAdd;
                ++valueIdx;
                if (!setBits64(current_bit_offset, stat.saveBits, value))
                {
                    return false;
                }
            }
            else
            {
                value = values[valueIdx] + stat.saveAdd;
                ++valueIdx;
                if (!setBits64(current_bit_offset, stat.saveParamBits, value))
                {
                    return false;
                }

                if (valueIdx >= values.size())
                {
                    return false;
                }

                value = values[valueIdx] + stat.saveAdd;
                ++valueIdx;
                if (!setBits64(current_bit_offset, stat.saveBits, value))
                {
                    return false;
                }
            }
        }
        else
        {
            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + stat.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, stat.saveBits, value))
            {
                return false;
            }
        }

        auto nextInChainIter = stat.nextInChain.begin();
        auto nextInChainIterEnd = stat.nextInChain.end();
        for (; nextInChainIter != nextInChainIterEnd && valueIdx < 4; ++nextInChainIter)
        {
            nextInChain = *nextInChainIter;

            const auto& statNext = ItemHelpers::getItemStat(getVersion(), nextInChain);
            if (statNext.id != nextInChain)
            {
                // corrupt file
                return false;
            }

            if (statNext.saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (valueIdx >= values.size())
            {
                return false;
            }

            value = values[valueIdx] + statNext.saveAdd;
            ++valueIdx;
            if (!setBits64(current_bit_offset, statNext.saveBits, value))
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

    size_t dataBitSize = data.size() * 8;
    if (current_bit_offset >= dataBitSize)
    {
        return false;
    }

    std::uint16_t id = (std::uint16_t)readBitsEx(current_bit_offset, PROPERTY_ID_NUM_BITS);
    std::uint16_t nextInChain = 0;
    auto itemVersion = getVersion();
    auto gameVersion = getGameVersion();
    while (id != 0x1FF)
    {
        if (current_bit_offset >= dataBitSize)
        {
            return false;
        }

        MagicalAttribute magicalAttrib;
        const auto& stat = ItemHelpers::getItemStat(itemVersion, id);
        if (id != stat.id)
        {
            // corrupt file
            return false;
        }

        magicalAttrib.Id = stat.id;
        magicalAttrib.Name = stat.name;
        magicalAttrib.Desc = stat.nextInChain.empty() ? stat.desc : stat.descRange;
        magicalAttrib.Version = itemVersion;
        magicalAttrib.GameVersion = gameVersion;
        magicalAttrib.DescPriority = stat.descPriority;
        magicalAttrib.encode = stat.encode;

        // saveBits being zero or >= 64 is unrecoverably bad, and
        // encode type 4 is only used by stats that were never implemented (time-based stats)
        if (stat.saveBits == 0 || stat.saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        if (stat.encode == 2)
        {
            if (stat.saveBits != 7 || stat.saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 6) - stat.saveAdd);
            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 10) - stat.saveAdd);
            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, stat.saveBits) - stat.saveAdd);
        }
        else if (stat.encode == 3)
        {
            if (stat.saveBits != 16 || stat.saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 6) - stat.saveAdd);
            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 10) - stat.saveAdd);
            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 8) - stat.saveAdd);
            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 8) - stat.saveAdd);
        }
        else if (stat.encode == 4)
        {
            // time-based stats were never implemented, but we handle them
            if (stat.saveBits != 22 || stat.saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 2));
            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 10));
            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 10));
        }
        else if (stat.saveParamBits > 0)
        {
            if (stat.name.compare("item_addskill_tab") == 0)
            {
                magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 3) - stat.saveAdd);
                magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, 13) - stat.saveAdd);
                magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, stat.saveBits) - stat.saveAdd);
            }
            else
            {
                magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, stat.saveParamBits) - stat.saveAdd);
                magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, stat.saveBits) - stat.saveAdd);
            }
        }
        else
        {
            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, stat.saveBits) - stat.saveAdd);
        }

        auto nextInChainIter = stat.nextInChain.begin();
        auto nextInChainIterEnd = stat.nextInChain.end();
        for (; nextInChainIter != nextInChainIterEnd && magicalAttrib.Values.size() < 4; ++nextInChainIter)
        {
            nextInChain = *nextInChainIter;
            const auto& statNext = ItemHelpers::getItemStat(itemVersion, nextInChain);
            if (statNext.id != nextInChain)
            {
                // corrupt file
                return false;
            }

            if (statNext.saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            magicalAttrib.Values.push_back(readBits64Ex(current_bit_offset, statNext.saveBits) - statNext.saveAdd);
        }

        attrib.push_back(magicalAttrib);
        id = (std::uint16_t)readBitsEx(current_bit_offset, PROPERTY_ID_NUM_BITS);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updatePropertyList(size_t& current_bit_offset, const std::vector<MagicalAttribute>& attribs)
{
    auto itemVersion = getVersion();
    std::uint16_t id = 0x1FF;
    std::int64_t value = 0;
    std::uint16_t nextInChain = 0;
    std::vector<std::int64_t> values;
    for (const auto& attrib : attribs)
    {
        auto iterValue = attrib.Values.begin();
        auto iterValueEnd = attrib.Values.end();
        id = attrib.Id;
        if (!setBits(current_bit_offset, PROPERTY_ID_NUM_BITS, id))
        {
            return false;
        }

        const auto& stat = ItemHelpers::getItemStat(itemVersion, id);
        if (id != stat.id)
        {
            // corrupt file
            return false;
        }

        // saveBits being zero or >= 64 is unrecoverably bad, and
        // encode type 4 is only used by stats that were never implemented (time-based stats)
        if (stat.saveBits == 0 || stat.saveBits >= 64)
        {
            // corrupt file
            return false;
        }

        if (stat.encode == 2)
        {
            if (stat.saveBits != 7 || stat.saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (iterValue == iterValueEnd)
            {
                return false;
            }

            value = *iterValue + stat.saveAdd;
            if (!setBits64(current_bit_offset, 6, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            value = *iterValue + stat.saveAdd;
            if (!setBits64(current_bit_offset, 10, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            value = *iterValue + stat.saveAdd;
            if (!setBits64(current_bit_offset, stat.saveBits, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue != iterValueEnd)
            {
                return false;
            }
        }
        else if (stat.encode == 3)
        {
            if (stat.saveBits != 16 || stat.saveParamBits != 16)
            {
                // corrupt file
                return false;
            }

            if (iterValue == iterValueEnd)
            {
                return false;
            }

            value = *iterValue + stat.saveAdd;
            if (!setBits64(current_bit_offset, 6, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            value = *iterValue + stat.saveAdd;
            if (!setBits64(current_bit_offset, 10, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            value = *iterValue + stat.saveAdd;
            if (!setBits64(current_bit_offset, 8, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            value = *iterValue + stat.saveAdd;
            if (!setBits64(current_bit_offset, 8, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue != iterValueEnd)
            {
                return false;
            }
        }
        else if (stat.encode == 4)
        {
            // time-based stats were never implemented, but we handle them
            if (stat.saveBits != 22 || stat.saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            if (iterValue == iterValueEnd)
            {
                return false;
            }

            if (!setBits64(current_bit_offset, 2, *iterValue))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            if (!setBits64(current_bit_offset, 10, *iterValue))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            if (!setBits64(current_bit_offset, 10, *iterValue))
            {
                return false;
            }

            std::advance(iterValue, 1);
            if (iterValue != iterValueEnd)
            {
                return false;
            }
        }
        else if (stat.saveParamBits > 0)
        {
            if (stat.name.compare("item_addskill_tab") == 0)
            {
                if (iterValue == iterValueEnd)
                {
                    return false;
                }

                value = *iterValue + stat.saveAdd;
                if (!setBits64(current_bit_offset, 3, value))
                {
                    return false;
                }

                std::advance(iterValue, 1);
                if (iterValue == iterValueEnd)
                {
                    return false;
                }

                value = *iterValue + stat.saveAdd;
                if (!setBits64(current_bit_offset, 13, value))
                {
                    return false;
                }

                std::advance(iterValue, 1);
                if (iterValue == iterValueEnd)
                {
                    return false;
                }

                value = *iterValue + stat.saveAdd;
                if (!setBits64(current_bit_offset, stat.saveBits, value))
                {
                    return false;
                }

                std::advance(iterValue, 1);
                if (iterValue != iterValueEnd)
                {
                    return false;
                }
            }
            else
            {
                if (iterValue == iterValueEnd)
                {
                    return false;
                }

                value = *iterValue + stat.saveAdd;
                if (!setBits64(current_bit_offset, stat.saveParamBits, value))
                {
                    return false;
                }

                std::advance(iterValue, 1);
                if (iterValue == iterValueEnd)
                {
                    return false;
                }

                value = *iterValue + stat.saveAdd;
                if (!setBits64(current_bit_offset, stat.saveBits, value))
                {
                    return false;
                }

                std::advance(iterValue, 1);
                if (iterValue != iterValueEnd)
                {
                    return false;
                }
            }
        }
        else
        {
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            value = *iterValue + stat.saveAdd;
            if (!setBits64(current_bit_offset, stat.saveBits, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
        }

        auto nextInChainIter = stat.nextInChain.begin();
        auto nextInChainIterEnd = stat.nextInChain.end();
        for (; nextInChainIter != nextInChainIterEnd; ++nextInChainIter)
        {
            nextInChain = *nextInChainIter;
            if (iterValue == iterValueEnd)
            {
                return false;
            }

            const auto& statNext = ItemHelpers::getItemStat(itemVersion, nextInChain);
            if (statNext.id != nextInChain)
            {
                // corrupt file
                return false;
            }

            if (statNext.saveParamBits != 0)
            {
                // corrupt file
                return false;
            }

            value = *iterValue + statNext.saveAdd;
            if (!setBits64(current_bit_offset, statNext.saveBits, value))
            {
                return false;
            }

            std::advance(iterValue, 1);
        }

        if (iterValue != iterValueEnd)
        {
            return false;
        }
    }

    id = 0x1FF;
    return setBits(current_bit_offset, PROPERTY_ID_NUM_BITS, id);
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
std::uint32_t d2ce::Item::readBits(size_t start, size_t size) const
{
    if ((size > 32) || (start >= (data.size() * 8)))
    {
        return 0;
    }

    if (size >= 24)
    {
        return (std::uint32_t)read_uint64_bits(start, size);
    }
    return read_uint32_bits(start, size);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Item::readBitsEx(size_t& start, size_t size) const
{
    auto value = readBits(start, size);
    start += size;
    return value;
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::Item::readBits64(size_t start, size_t size) const
{
    if ((size > 56) || (start >= (data.size() * 8)))
    {
        return 0;
    }

    if (size >= 24)
    {
        return read_uint64_bits(start, size);
    }

    return (std::uint64_t)read_uint32_bits(start, size);
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::Item::readBits64Ex(size_t& start, size_t size) const
{
    auto value = readBits64(start, size);
    start += size;
    return value;
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
bool d2ce::Item::updateBitsEx(size_t& start, size_t size, std::uint32_t value)
{
    if (!updateBits(start, size, value))
    {
        return false;
    }

    start += size;
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
bool d2ce::Item::updateBits64Ex(size_t& start, size_t size, std::uint64_t value)
{
    if (!updateBits64(start, size, value))
    {
        return false;
    }

    start += size;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::updateResurrectedItemCode(std::uint64_t code, size_t numBitsSet)
{
    auto oldNumBitsSet = std::uint8_t(std::int64_t(GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET)) - std::int64_t(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET)));
    if (oldNumBitsSet == numBitsSet)
    {
        // easy change
        return updateBits64(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET), numBitsSet, code);
    }

    // complex change
    size_t old_current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);
    size_t bitsToCopy = GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) - GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);

    std::int64_t diff = std::int64_t(numBitsSet) - std::int64_t(oldNumBitsSet);
    GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET) += diff;
    GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) += diff;
    size_t current_bit_offset = GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET);

    // make a copy first
    std::vector<std::uint8_t> oldData(data);

    // truncate the original
    size_t newSize = (GET_BIT_OFFSET(ItemOffsets::ITEM_END_BIT_OFFSET) + 7) / 8;
    data.resize(newSize, 0);
    updateBits64(GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET), numBitsSet, code); // update the bits

    // now copy the remaining bits
    std::uint32_t value = 0;
    size_t valueBitSize = sizeof(value) * 8;
    std::uint8_t bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    while (bitsToCopy > 0)
    {
        bitsToCopy -= bits;
        value = readtemp_bits(oldData, old_current_bit_offset, bits);
        old_current_bit_offset += bits;
        updateBitsEx(current_bit_offset, bits, value);
        bits = (std::uint8_t)std::min(valueBitSize, bitsToCopy);
    }

    // clear any bits not written to
    if ((current_bit_offset % 8) > 0)
    {
        bits = (std::uint8_t)(8 - (current_bit_offset % 8));
        updateBitsEx(current_bit_offset, bits, 0);
    }

    updateOffset(GET_BIT_OFFSET(ItemOffsets::EXTENDED_DATA_OFFSET), diff);
    return true;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getInferiorQualityIdv100() const
{
    if (GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0)
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

    return ItemHelpers::generateInferiorQualityId(getLevel(), getDWBCode());
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAffixesv100(MagicalAffixes& affixes) const
{
    affixes.clear();
    if ((GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0) || (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0) || (GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) == 0))
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

    if (GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) == 0)
    {
        GET_BIT_OFFSET(ItemOffsets::DWB_BIT_OFFSET) = GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) + 32;
    }

    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    if (!ItemHelpers::generateMagicalAffixes(magic_affixes_v100, createParams, getLevel(), getDWBCode()))
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
    if (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0)
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

    const auto& itemType = ItemHelpers::getSetItemTypeHelper((std::uint16_t)readBits(GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET), ITEM_V100_UNIQUE_ID_NUM_BITS), strcode);
    if (&itemType == &ItemHelpers::getInvalidItemTypeHelper())
    {
        // should not happen
        return false;
    }

    attrib.Id = itemType.getId();
    if (attrib.Id >= MAXUINT16)
    {
        attrib.Id = 0;
        return false;
    }

    attrib.Name = itemType.name;
    attrib.ReqLevel = itemType.req.Level;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Item::getMagicalAttributesv100(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();
    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    d2ce::UniqueAttributes uniqueAttrib;
    d2ce::SetAttributes setAttrib;
    d2ce::MagicalCachev100 magicalAffixes;
    d2ce::RareOrCraftedCachev100 rareAffixes;
    switch (getQuality())
    {
    case EnumItemQuality::SUPERIOR:
        return ItemHelpers::generateSuperiorAttributes(attribs, createParams, getLevel(), getDWBCode(), true);
        break;

    case EnumItemQuality::SET:
        if (getSetAttributes(setAttrib))
        {
            return ItemHelpers::getSetMagicAttribs(setAttrib.Id, attribs, createParams, getGameVersion(), getLevel(), getDWBCode());
        }
        break;

    case EnumItemQuality::UNIQUE:
        if (getUniqueAttributes(uniqueAttrib) && !uniqueAttrib.Name.empty())
        {
            return ItemHelpers::getUniqueMagicAttribs(uniqueAttrib.Id, attribs, createParams, getGameVersion(), getLevel(), getDWBCode());
        }
        break;

    case EnumItemQuality::MAGIC:
        if (ItemHelpers::generateMagicalAffixes(magicalAffixes, createParams, getLevel(), getDWBCode(), true))
        {
            attribs = magicalAffixes.MagicalAttributes;
            return true;
        }
        break;

    case EnumItemQuality::RARE:
    case EnumItemQuality::TEMPERED:
        createParams.createQualityOption = getQuality();
        if (ItemHelpers::generateRareOrCraftedAffixes(rareAffixes, createParams, getLevel(), getDWBCode(), true) && !rareAffixes.Affixes.empty())
        {
            for (const auto& affix : rareAffixes.Affixes)
            {
                attribs.insert(attribs.end(), affix.MagicalAttributes.begin(), affix.MagicalAttributes.end());
            }
            return true;
        }
        break;

    case EnumItemQuality::CRAFTED:
        createParams.createQualityOption = getQuality();
        if (ItemHelpers::generateRareOrCraftedAffixes(rareAffixes, createParams, getLevel(), getDWBCode(), true) && !rareAffixes.Affixes.empty())
        {
            std::vector<MagicalAttribute> craftingAttributes;
            craftingAttributes.swap(rareAffixes.Affixes.back().MagicalAttributes);
            rareAffixes.Affixes.pop_back();
            for (const auto& affix : rareAffixes.Affixes)
            {
                attribs.insert(attribs.end(), affix.MagicalAttributes.begin(), affix.MagicalAttributes.end());
            }

            ItemHelpers::mergeMagicalAttributes(attribs, craftingAttributes);
            return true;
        }
        break;

    default:
        if (isQuestItem())
        {
            std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
            if (getItemCode(strcode))
            {
                return ItemHelpers::getUniqueQuestMagicAttribs(strcode, attribs, createParams, getGameVersion(), getLevel(), getDWBCode());
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
    if ((GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0) || (GET_BIT_OFFSET(ItemOffsets::QUALITY_ATTRIB_BIT_OFFSET) == 0) || (GET_BIT_OFFSET(ItemOffsets::TYPE_CODE_OFFSET) == 0))
    {
        return false;
    }

    bool bCrafted = false;
    switch (getQuality())
    {
    case EnumItemQuality::RARE:
    case EnumItemQuality::TEMPERED:
        break;

    case EnumItemQuality::CRAFTED:
        bCrafted = true;
        break;

    default:
        return false;
    }

    if (rare_affixes_v100.Id != MAXUINT16)
    {
        attrib.Id = rare_affixes_v100.Id;
        attrib.Name = rare_affixes_v100.Name;
        attrib.Index = rare_affixes_v100.Index;
        attrib.Id2 = rare_affixes_v100.Id2;
        attrib.Name2 = rare_affixes_v100.Name2;
        attrib.Index2 = rare_affixes_v100.Index2;
        for (auto& item : rare_affixes_v100.Affixes)
        {
            attrib.Affixes.push_back(item.Affixes);
        }
        return true;
    }

    ItemCreateParams createParams(getVersion(), getItemTypeHelper(), getGameVersion());
    createParams.createQualityOption = getQuality();
    if (!ItemHelpers::generateRareOrCraftedAffixes(rare_affixes_v100, createParams, getLevel(), getDWBCode()) || (rare_affixes_v100.Id == MAXUINT16))
    {
        return false;
    }

    attrib.Id = rare_affixes_v100.Id;
    attrib.Name = rare_affixes_v100.Name;
    attrib.Index = rare_affixes_v100.Name;
    attrib.Id2 = rare_affixes_v100.Id2;
    attrib.Name2 = rare_affixes_v100.Name2;
    attrib.Index2 = rare_affixes_v100.Name;
    if (bCrafted)
    {
        rare_affixes_v100.Affixes.pop_back(); // remove crafted attributes
    }

    for (auto& item : rare_affixes_v100.Affixes)
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

    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    return itemType.hasMultipleGraphics();
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Item::getPictureIdv100() const
{
    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        return false;
    }

    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    std::uint32_t modulo = itemType.getNumPictureIds();
    if (modulo == 0)
    {
        return 0;
    }

    return std::uint8_t(ItemHelpers::generateDWARandomOffset(getId(), 1) % modulo);
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Item::getDefenseRatingv100() const
{
    if (GET_BIT_OFFSET(ItemOffsets::ITEM_ID_BIT_OFFSET) == 0)
    {
        return 0;
    }

    std::array<std::uint8_t, 4> strcode = { 0, 0, 0, 0 };
    if (!getItemCode(strcode))
    {
        return false;
    }

    auto ac = ItemHelpers::generateDefenseRating(strcode, getId());
    if ((ac > 0) && (getQuality() == EnumItemQuality::INFERIOR))
    {
        ac = std::min(1ui16, std::uint16_t(ac * 0.75));
    }
    return ac;
}
//---------------------------------------------------------------------------
void d2ce::Items::findItems()
{
    if (IsSharedStash)
    {
        findSharedStashItems();
        return;
    }

    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    ItemLocationReference.clear();
    ItemLocationEmptySpots.clear();
    HasBeltEquipped = false;
    HasRestrictedItem.clear();
    EquippedBeltSlots = 0;

    bool hasHoradricCube = false;

    std::map<d2ce::EnumAltItemLocation, ItemDimensions> altItemDimensions;
    {
        auto& emptySlots = ItemLocationEmptySpots[EnumItemLocation::STORED][d2ce::EnumAltItemLocation::INVENTORY];
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
        auto& emptySlots = ItemLocationEmptySpots[EnumItemLocation::STORED][d2ce::EnumAltItemLocation::STASH];
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
        itemAltLocation = item.getAltPositionId();
        switch (itemLocation)
        {
        case d2ce::EnumItemLocation::BELT:
            HasBeltEquipped = true;
            itemAltLocation = EnumAltItemLocation::UNKNOWN;
            ItemLocationReference[itemLocation][itemAltLocation].push_back(const_cast<d2ce::Item&>(item));
            break;

        case d2ce::EnumItemLocation::EQUIPPED:
            itemAltLocation = EnumAltItemLocation::UNKNOWN;
            ItemLocationReference[itemLocation][itemAltLocation].push_back(const_cast<d2ce::Item&>(item));
            if ((!HasBeltEquipped || (EquippedBeltSlots == 0)) && item.isBelt())
            {
                HasBeltEquipped = true;
                EquippedBeltSlots = item.getTotalItemSlots();
            }
            break;

        case d2ce::EnumItemLocation::STORED:
            ItemLocationReference[itemLocation][itemAltLocation].push_back(const_cast<d2ce::Item&>(item));
            switch (itemAltLocation)
            {
            case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
                hasHoradricCube = true;
                HasRestrictedItem.push_back(const_cast<d2ce::ItemType&>(item.getItemTypeHelper()));
                break;

            case d2ce::EnumAltItemLocation::INVENTORY:
            case d2ce::EnumAltItemLocation::STASH:
                if (item.isRestrictedItem())
                {
                    HasRestrictedItem.push_back(const_cast<d2ce::ItemType&>(item.getItemTypeHelper()));
                    if (!hasHoradricCube)
                    {
                        hasHoradricCube = item.isHoradricCube();
                    }
                }

                if (item.getDimensions(dimensions))
                {
                    // remove slots from empty list
                    auto& emptySlots = ItemLocationEmptySpots[itemLocation][itemAltLocation];
                    auto& invDimenstion = altItemDimensions[itemAltLocation];
                    auto posX = std::uint16_t(item.getPositionX());
                    auto posY = std::uint16_t(item.getPositionY());
                    for (std::uint16_t y = posY; y < std::uint16_t(posY + dimensions.Height); ++y)
                    {
                        for (std::uint16_t x = posX; x < std::uint16_t(posX + dimensions.Width); ++x)
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
        auto& emptySlots = ItemLocationEmptySpots[d2ce::EnumItemLocation::BELT][d2ce::EnumAltItemLocation::UNKNOWN];
        if (getItemLocationDimensions(d2ce::EnumItemLocation::BELT, dimensions))
        {
            std::uint16_t totalPos = dimensions.InvHeight * dimensions.InvWidth;
            for (std::uint16_t pos = 0; pos < totalPos; ++pos)
            {
                emptySlots.insert(pos);
            }

            // remove slots from empty list
            for (const auto& itemRef : ItemLocationReference[d2ce::EnumItemLocation::BELT][d2ce::EnumAltItemLocation::UNKNOWN])
            {
                emptySlots.erase(itemRef.get().getPositionX());
            }
        }
    } // end for

    if (hasHoradricCube)
    {
        itemAltLocation = EnumAltItemLocation::HORADRIC_CUBE;
        auto& emptySlots = ItemLocationEmptySpots[d2ce::EnumItemLocation::STORED][itemAltLocation];
        auto& cubeDimensions = altItemDimensions[itemAltLocation];
        if (getItemLocationDimensions(itemAltLocation, cubeDimensions))
        {
            std::uint16_t totalPos = cubeDimensions.InvHeight * cubeDimensions.InvWidth;
            for (std::uint16_t pos = 0; pos < totalPos; ++pos)
            {
                emptySlots.insert(pos);
            }

            // remove slots from empty list
            for (const auto& itemRef : ItemLocationReference[d2ce::EnumItemLocation::STORED][itemAltLocation])
            {
                // remove slots from empty list
                auto posX = std::uint16_t(itemRef.get().getPositionX());
                auto posY = std::uint16_t(itemRef.get().getPositionY());
                if (itemRef.get().getDimensions(dimensions))
                {
                    for (std::uint16_t y = posY; y < std::uint16_t(posY + dimensions.Height); ++y)
                    {
                        for (std::uint16_t x = posX; x < std::uint16_t(posX + dimensions.Width); ++x)
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
void d2ce::Items::findSharedStashItems()
{
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    ItemLocationReference.clear();
    ItemLocationEmptySpots.clear();
    HasBeltEquipped = false;
    HasRestrictedItem.clear();
    EquippedBeltSlots = 0;

    ItemDimensions stashDimensions;
    {
        auto& emptySlots = ItemLocationEmptySpots[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::STASH];
        if (getItemLocationDimensions(d2ce::EnumAltItemLocation::STASH, stashDimensions))
        {
            std::uint16_t totalPos = stashDimensions.InvHeight * stashDimensions.InvWidth;
            for (std::uint16_t pos = 0; pos < totalPos; ++pos)
            {
                emptySlots.insert(pos);
            }
        }
    }

    ItemDimensions dimensions;
    for (auto& item : Inventory)
    {
        const auto& itemType = item.getItemTypeHelper();
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

        ItemLocationReference[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::STASH].push_back(const_cast<d2ce::Item&>(item));
        if (item.getDimensions(dimensions))
        {
            // remove slots from empty list
            auto& emptySlots = ItemLocationEmptySpots[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::STASH];
            auto posX = std::uint16_t(item.getPositionX());
            auto posY = std::uint16_t(item.getPositionY());
            for (std::uint16_t y = posY; y < std::uint16_t(posY + dimensions.Height); ++y)
            {
                for (std::uint16_t x = posX; x < std::uint16_t(posX + dimensions.Width); ++x)
                {
                    emptySlots.erase(y * stashDimensions.Width + x);
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(std::FILE* charfile, std::list<d2ce::Item>& items)
{
    items.clear();
    std::uint8_t value = 0;
    std::fread(&value, sizeof(value), 1, charfile);
    if (value != ITEM_MARKER[0])
    {
        return false;
    }

    std::fread(&value, sizeof(value), 1, charfile);
    if (value != ITEM_MARKER[1])
    {
        return false;
    }

    // found item marker
    std::uint16_t numItems = 0;
    std::fread(&numItems, sizeof(numItems), 1, charfile);
    if (numItems > 0)
    {
        if (!fillItemsArray(charfile, numItems, items))
        {
            // Corrupt file
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readSharedStashPage(std::FILE* charfile, std::list<d2ce::Item>& items)
{
    items.clear();
    std::uint8_t value = 0;
    std::fread(&value, sizeof(value), 1, charfile);
    if (value != ITEM_MARKER[0])
    {
        return false;
    }

    std::fread(&value, sizeof(value), 1, charfile);
    if (value != ITEM_MARKER[1])
    {
        return false;
    }

    // found item marker
    std::uint16_t numItems = 0;
    std::fread(&numItems, sizeof(numItems), 1, charfile);
    if (numItems > 0)
    {
        if (!fillItemsArray(charfile, numItems, items))
        {
            // Corrupt file
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::fillItemsArray(std::FILE* charfile, std::uint16_t numItems, std::list<d2ce::Item>& items)
{
    while (items.size() < numItems)
    {
        if (feof(charfile))
        {
            return false;
        }

        items.resize(items.size() + 1);
        auto& item = items.back();
        if (!item.readItem(Version, isExpansionItems(), charfile))
        {
            // corrupt file
            items.pop_back();
            return false;
        }

        if (Version == EnumItemVersion::v100)
        {
            // Update version to v1.04 if any items are v1.04
            Version = item.getVersion();
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
bool d2ce::Items::readItems(const Json::Value& root, bool bSerializedFormat, std::list<Item>& items)
{
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
        return false;
    }

    std::uint16_t numItems = std::uint16_t(items.size());
    if (checkItemCount && (numItems != expectedNumOfItems))
    {
        // Corrupt file
        return false;
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
        if (!item.readItem(*iter, bSerializedFormat, Version, isExpansionItems()))
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
    std::uint8_t value = 0;
    std::fread(&value, sizeof(value), 1, charfile);
    if (value != ITEM_MARKER[0])
    {
        return false;
    }

    std::fread(&value, sizeof(value), 1, charfile);
    if (value != ITEM_MARKER[1])
    {
        return false;
    }

    // found item marker
    std::fread(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
    if (CorpseInfo.IsDead > 1)
    {
        return false;
    }

    if (CorpseInfo.IsDead > 0)
    {
        std::fread(&CorpseInfo.Unknown, sizeof(CorpseInfo.Unknown), 1, charfile);
        std::fread(&CorpseInfo.X, sizeof(CorpseInfo.X), 1, charfile);
        std::fread(&CorpseInfo.Y, sizeof(CorpseInfo.Y), 1, charfile);
        if (!readItems(charfile, CorpseItems))
        {
            return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readCorpseItems(const Json::Value& root, bool bSerializedFormat)
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
                if (!corpseLocationRoot.isNull())
                {
                    value = corpseLocationRoot["Header"];
                    if (!value.isNull())
                    {
                        if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)ITEM_MARKER.data()))
                        {
                            // bad header
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

    if (CorpseInfo.IsDead != 1)
    {
        CorpseInfo.clear();
        CorpseItems.clear();
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

    std::uint16_t numItems = 0;
    if (CorpseInfo.IsDead > 0)
    {
        if (!corpseItemsRoot.isNull())
        {
            if (!readItemsList(corpseItemsRoot, bSerializedFormat, CorpseItems))
            {
                // Corrupt file
                return false;
            }
        }

        numItems = (std::uint16_t)CorpseItems.size();
    }

    if (CorpseInfo.IsDead != 1 || CorpseItems.empty())
    {
        CorpseInfo.clear();
        numItems = 0;
        CorpseItems.clear();
    }

    if (checkItemCount && (numItems != expectedNumOfItems))
    {
        // Corrupt file
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::readMercItems(std::FILE* charfile)
{
    if (!isExpansionItems())
    {
        if (Version < EnumItemVersion::v109)
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

    bool bHasMercId = true;
    MercItems.clear();
    std::uint8_t value = 0;
    std::fread(&value, sizeof(value), 1, charfile);
    if (value != MERC_ITEM_MARKER[0])
    {
        return;
    }

    std::fread(&value, sizeof(value), 1, charfile);
    if (value != MERC_ITEM_MARKER[1])
    {
        return;
    }

    // look ahead for no merc case
    if (!feof(charfile))
    {
        auto startLoc = std::ftell(charfile);
        std::fread(&value, sizeof(value), 1, charfile);
        if (value != ITEM_MARKER[0])
        {
            bHasMercId = false;
        }

        std::fseek(charfile, startLoc, SEEK_SET);
    }

    if (bHasMercId)
    {
        if(!readItems(charfile, MercItems))
        {
            return;
        }
        isMercHired = true;
    }

    readGolemItem(charfile);
}
//---------------------------------------------------------------------------
void d2ce::Items::readMercItems(const Json::Value& root, bool bSerializedFormat)
{
    isMercHired = false;

    if (!isExpansionItems())
    {
        if (Version < EnumItemVersion::v109)
        {
            MercId_v100 = 0;
            Json::Value jsonValue = root[bSerializedFormat ? "MercId" : "merc_id"];
            if (!jsonValue.isNull())
            {
                MercId_v100 = std::uint64_t(std::stoull(jsonValue.asString(), nullptr, 16));
            }
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
                isMercHired = mercId != 0;
            }
            else
            {
                isMercHired = false;
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

    if (!readItemsList(mercItemsRoot, bSerializedFormat, MercItems))
    {
        // Corrupt file
        isMercHired = false;
        return;
    }

    readGolemItem(root, bSerializedFormat);
}
//---------------------------------------------------------------------------
void d2ce::Items::readGolemItem(std::FILE* charfile)
{
    GolemItem.clear();
    std::uint8_t value = 0;
    std::fread(&value, sizeof(value), 1, charfile);
    if (value != GOLEM_ITEM_MARKER[0])
    {
        return;
    }

    std::fread(&value, sizeof(value), 1, charfile);
    if (value != GOLEM_ITEM_MARKER[1])
    {
        return;
    }

    // found golem item marker (0x464B). 
    std::uint8_t hasGolem = 0;
    std::fread(&hasGolem, sizeof(hasGolem), 1, charfile);
    if (hasGolem != 0)
    {
        GolemItem.resize(GolemItem.size() + 1);
        auto& golemItem = GolemItem.back();
        if (!golemItem.readItem(Version, isExpansionItems(), charfile))
        {
            // Corrupt file
            GolemItem.clear();
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::Items::readGolemItem(const Json::Value& root, bool bSerializedFormat)
{
    GolemItem.clear();

    Json::Value golemItemRoot;
    if (!root.isNull())
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
                    if (value.asBool())
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

    if (!golemItemRoot.isNull())
    {
        GolemItem.resize(GolemItem.size() + 1);
        auto& golemItem = GolemItem.back();
        if (!golemItem.readItem(golemItemRoot, bSerializedFormat, Version, isExpansionItems()))
        {
            GolemItem.clear();
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::Items::writeCorpseItems(std::FILE* charfile) const
{
    std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
    if (CorpseInfo.IsDead != 1 || CorpseItems.empty())
    {
        CorpseInfo.clear();
        CorpseItems.clear();
    }

    std::fwrite(&CorpseInfo.IsDead, sizeof(CorpseInfo.IsDead), 1, charfile);
    std::fflush(charfile);

    if (CorpseInfo.IsDead > 0)
    {
        std::fwrite(&CorpseInfo.Unknown, sizeof(CorpseInfo.Unknown), 1, charfile);
        std::fwrite(&CorpseInfo.X, sizeof(CorpseInfo.X), 1, charfile);
        std::fwrite(&CorpseInfo.Y, sizeof(CorpseInfo.Y), 1, charfile);

        std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
        std::uint16_t numItems = (std::uint16_t)CorpseItems.size();
        std::fwrite(&numItems, sizeof(numItems), 1, charfile);
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
bool d2ce::Items::writeMercItems(std::FILE* charfile) const
{
    if (!isExpansionItems())
    {
        if (Version < EnumItemVersion::v109)
        {
            std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
            std::fwrite(&MercId_v100, 6, 1, charfile);
        }
        return true;
    }

    std::fwrite(MERC_ITEM_MARKER.data(), MERC_ITEM_MARKER.size(), 1, charfile);
    std::uint16_t numItems = (std::uint16_t)MercItems.size();
    if (numItems > 0 || isMercHired)
    {
        isMercHired = true;
        std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
        std::fwrite(&numItems, sizeof(numItems), 1, charfile);
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
bool d2ce::Items::writeGolemItem(std::FILE* charfile) const
{
    std::fwrite(GOLEM_ITEM_MARKER.data(), GOLEM_ITEM_MARKER.size(), 1, charfile);
    std::uint8_t hasGolem = GolemItem.empty() ? 0 : 1;
    std::fwrite(&hasGolem, sizeof(hasGolem), 1, charfile);
    if (!GolemItem.empty())
    {
        if (!GolemItem.back().writeItem(charfile))
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(const Character& charInfo, std::FILE* charfile)
{
    bool isExpansion = charInfo.isExpansionCharacter();
    Version = ConvertCharVersion(charInfo.getVersion());
    switch (charInfo.getVersion())
    {
    case EnumCharVersion::v100: // v1.00 - v1.06
        GameVersion = 0;
        break;

    case EnumCharVersion::v107: // v1.07
        GameVersion = isExpansion ? 100 : 0;
        break;

    case EnumCharVersion::v108: // v1.08
    case EnumCharVersion::v109: // v1.09
    case EnumCharVersion::v110: // v1.10 - v1.14d
    case EnumCharVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected
    case EnumCharVersion::v120: // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumCharVersion::v140: // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        GameVersion = isExpansion ? 100 : 1;
        break;
    }

    isMercHired = false;
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    if (!readItems(charfile, Inventory))
    {
        return false;
    }

    if (!readCorpseItems(charfile))
    {
        return false;
    }

    readMercItems(charfile);

    findItems();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readSharedStashPage(EnumCharVersion version, std::FILE* charfile)
{
    clear();
    IsSharedStash = true;
    Version = ConvertCharVersion(version);
    switch (version)
    {
    case EnumCharVersion::v100: // v1.00 - v1.06
        GameVersion = 0;
        break;

    case EnumCharVersion::v107: // v1.07
    case EnumCharVersion::v108: // v1.08
    case EnumCharVersion::v109: // v1.09
    case EnumCharVersion::v110: // v1.10 - v1.14d
    case EnumCharVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected
    case EnumCharVersion::v120: // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumCharVersion::v140: // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        GameVersion = 100;
        break;
    }

    isMercHired = false;
    GPSs.clear();
    Stackables.clear();
    Armor.clear();
    Weapons.clear();
    if (!readSharedStashPage(charfile, Inventory))
    {
        return false;
    }

    findSharedStashItems();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::readItems(const Json::Value& root, bool bSerializedFormat, const Character& charInfo)
{
    clear();
    bool isExpansion = charInfo.isExpansionCharacter();
    Version = ConvertCharVersion(charInfo.getVersion());
    switch (charInfo.getVersion())
    {
    case EnumCharVersion::v100: // v1.00 - v1.06
        GameVersion = 0;
        break;

    case EnumCharVersion::v107: // v1.07
        GameVersion = isExpansion ? 100 : 0;
        break;

    case EnumCharVersion::v108:  // v1.08
    case EnumCharVersion::v109:  // v1.09
    case EnumCharVersion::v110:  // v1.10 - v1.14d
    case EnumCharVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected
    case EnumCharVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumCharVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        GameVersion = isExpansion ? 100 : 1;
        break;
    }

    isMercHired = false;
    if (!readItems(root, bSerializedFormat, Inventory))
    {
        return false;
    }

    if (!readCorpseItems(root, bSerializedFormat))
    {
        return false;
    }

    readMercItems(root, bSerializedFormat);

    findItems();
    return true;
}
//---------------------------------------------------------------------------
// write items in place at offset saved from reasding
bool d2ce::Items::writeItems(std::FILE* charfile, bool isExpansion, bool hasMercID) const
{
    switch (getDefaultItemVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
        GameVersion = 0;
        break;

    case EnumItemVersion::v107: // v1.07 item
        GameVersion = isExpansion ? 100 : 0;
        break;

    case EnumItemVersion::v108:  // v1.08 item
    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        GameVersion = isExpansion ? 100 : 1;
        break;
    }

    isMercHired = !MercItems.empty() || hasMercID;

    // Write Items
    std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
    std::uint16_t numItems = (std::uint16_t)Inventory.size();
    std::fwrite(&numItems, sizeof(numItems), 1, charfile);
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
bool d2ce::Items::writeSharedStashPage(std::FILE* charfile) const
{
    // Write Items
    std::fwrite(ITEM_MARKER.data(), ITEM_MARKER.size(), 1, charfile);
    std::uint16_t numItems = (std::uint16_t)Inventory.size();
    std::fwrite(&numItems, sizeof(numItems), 1, charfile);
    for (auto& item : Inventory)
    {
        if (!item.writeItem(charfile))
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Items::itemsAsJson(Json::Value& parent, std::uint32_t charLevel, EnumItemVersion version, bool bSerializedFormat) const
{
    if (version == Version)
    {
        itemsAsJson(parent, charLevel, bSerializedFormat);
        return;
    }

    if (bSerializedFormat)
    {
        Json::Value playerItemList;
        playerItemList["Header"] = *((std::uint16_t*)ITEM_MARKER.data());

        Json::Value items(Json::arrayValue);
        for (auto& item : Inventory)
        {
            item.asJson(items, charLevel, version, bSerializedFormat);
        }
        playerItemList["Count"] = items.size();
        playerItemList["Items"] = items;
        parent["PlayerItemList"] = playerItemList;
    }
    else
    {
        Json::Value items(Json::arrayValue);
        for (auto& item : Inventory)
        {
            item.asJson(items, charLevel, version, bSerializedFormat);
        }
        parent["items"] = items;
    }
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
    if (!isExpansionItems())
    {
        if ((Version < EnumItemVersion::v109) && MercId_v100 > 0)
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
    if (!isExpansionItems())
    {
        return false;
    }

    if (bSerializedFormat)
    {
        Json::Value golem;
        golem["Header"] = *((std::uint16_t*)GOLEM_ITEM_MARKER.data());
        golem["Exists"] = (GolemItem.empty() ? false : true);
        if (!GolemItem.empty())
        {
            Json::Value item;
            GolemItem.back().asJson(item, charLevel, bSerializedFormat);
            golem["Item"] = item;
        }
        parent["Golem"] = golem;
    }
    else
    {
        if (GolemItem.empty())
        {
            return false;
        }

        Json::Value item;
        GolemItem.back().asJson(item, charLevel, bSerializedFormat);
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
void d2ce::Items::asJson(Json::Value& parent, std::uint32_t charLevel, EnumCharVersion version, bool bSerializedFormat) const
{
    EnumItemVersion itemVersion = APP_ITEM_VERSION;
    switch (version)
    {
    case EnumCharVersion::v100: // v1.00 - v1.06
        itemVersion = EnumItemVersion::v104;
        switch (Version)
        {
        case EnumItemVersion::v100: // v1.00 - v1.03
        case EnumItemVersion::v104: // v1.04 - v1.06
            asJson(parent, charLevel, bSerializedFormat);
            return;
        }
        break;

    case EnumCharVersion::v107: // v1.07
        itemVersion = EnumItemVersion::v107;
        break;

    case EnumCharVersion::v108: // v1.08
        itemVersion = EnumItemVersion::v108;
        break;

    case EnumCharVersion::v109: // v1.09
        itemVersion = EnumItemVersion::v109;
        break;

    case EnumCharVersion::v110: // v1.10 - v1.14d
        itemVersion = EnumItemVersion::v110;
        break;

    case EnumCharVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected
        itemVersion = EnumItemVersion::v100R;
        break;

    case EnumCharVersion::v120: // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
        itemVersion = EnumItemVersion::v120;
        break;

    case EnumCharVersion::v140: // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        itemVersion = EnumItemVersion::v140;
        break;
    }

    if (itemVersion == Version)
    {
        asJson(parent, charLevel, bSerializedFormat);
        return;
    }

    itemsAsJson(parent, charLevel, itemVersion, bSerializedFormat);
    corpseItemsAsJson(parent, charLevel, bSerializedFormat);
    mercItemsAsJson(parent, charLevel, bSerializedFormat);
    golemItemAsJson(parent, charLevel, bSerializedFormat);
    itemBonusesAsJson(parent, bSerializedFormat);
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
bool d2ce::Items::isExpansionItems() const
{
    return getDefaultGameVersion() == 100 ? true : false;
}
//---------------------------------------------------------------------------
void d2ce::Items::verifyBeltSlots()
{
    auto& equippedLocationReference = ItemLocationReference[d2ce::EnumItemLocation::EQUIPPED][d2ce::EnumAltItemLocation::UNKNOWN];
    if (!equippedLocationReference.empty())
    {
        auto iter = std::find_if(equippedLocationReference.begin(), equippedLocationReference.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::BELT));
        if (iter == equippedLocationReference.end())
        {
            HasBeltEquipped = false;
            EquippedBeltSlots = 0;
        }
        else
        {
            HasBeltEquipped = true;
            EquippedBeltSlots = iter->get().getTotalItemSlots();
        }
    }

    auto locationId = d2ce::EnumItemLocation::BELT;
    auto altPositionId = d2ce::EnumAltItemLocation::UNKNOWN;
    auto& invEmptySlots = ItemLocationEmptySpots[locationId][altPositionId];
    auto& invLocationReference = ItemLocationReference[locationId][altPositionId];

    ItemDimensions invDimensions;
    getItemLocationDimensions(d2ce::EnumItemLocation::BELT, invDimensions);
    invEmptySlots.clear();
    std::uint16_t totalPos = invDimensions.InvHeight * invDimensions.InvWidth;
    for (std::uint16_t pos = 0; pos < totalPos; ++pos)
    {
        invEmptySlots.insert(pos);
    }

    // remove slots from empty list that fit into the belt
    std::list<Item> itemToMove; // non-empty if given belt items need moving
    std::uint16_t posX = 0;
    for (const auto& itemRef : invLocationReference)
    {
        posX = itemRef.get().getPositionX();
        if (posX < totalPos)
        {
            invEmptySlots.erase(posX);
        }
    }

    if (invEmptySlots.empty())
    {
        // try to find spots for the items out of bounds still
        auto iter = Inventory.begin();
        bool bRemoveItem = false;
        for (const auto& itemRef : invLocationReference)
        {
            posX = itemRef.get().getPositionX();
            if (posX < totalPos)
            {
                continue;
            }

            bRemoveItem = true;
            if (!invEmptySlots.empty())
            {
                posX = *invEmptySlots.begin();
                if (iter->setLocation(d2ce::EnumItemLocation::BELT, posX, 0))
                {
                    bRemoveItem = false;
                    invEmptySlots.erase(posX);
                }
            }

            if (bRemoveItem)
            {
                iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(itemRef.get()));
                if (iter != Inventory.end())
                {
                    const auto& itemType = iter->getItemTypeHelper();
                    itemToMove.splice(itemToMove.end(), Inventory, iter);
                    if (itemType.isPotion() || itemType.isGem() || itemType.isRune())
                    {
                        d2ce::removeItem_if(GPSs, ItemPredicate(itemRef.get()));
                    }
                }
            }
        }
    }
    else
    {
        // move any beltable items in the buffer over to the belt
        auto iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemIsBeltablePredicate());
        while (iter != BufferItems.end() && !invEmptySlots.empty())
        {
            posX = *(invEmptySlots.begin());
            auto& item = *iter;
            if (!item.setLocation(d2ce::EnumItemLocation::BELT, posX, 0))
            {
                iter = BufferItems.end();
                continue;
            }

            invEmptySlots.erase(posX);

            Inventory.splice(Inventory.end(), BufferItems, iter);
            if (std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemPredicate(item)) == invLocationReference.end())
            {
                invLocationReference.push_back(item);
            }

            const auto& itemType = item.getItemTypeHelper();
            if (itemType.isPotion() || itemType.isGem() || itemType.isRune())
            {
                GPSs.push_back(item);
            }

            iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemIsBeltablePredicate());
        }
    }

    if (itemToMove.empty())
    {
        return;
    }

    for (const auto& item : itemToMove)
    {
        d2ce::removeItem_if(invLocationReference, ItemPredicate(item));
    }

    locationId = d2ce::EnumItemLocation::STORED;
    altPositionId = d2ce::EnumAltItemLocation::INVENTORY;
    auto* pInvEmptySlots = &ItemLocationEmptySpots[locationId][altPositionId];
    auto* pInvLocationReference = &ItemLocationReference[locationId][altPositionId];
    getItemLocationDimensions(locationId, altPositionId, invDimensions);
    while (!itemToMove.empty() && (pInvEmptySlots != nullptr))
    {
        if (pInvEmptySlots->empty())
        {
            switch (altPositionId)
            {
            case d2ce::EnumAltItemLocation::INVENTORY:
                altPositionId = d2ce::EnumAltItemLocation::STASH;
                pInvEmptySlots = &ItemLocationEmptySpots[locationId][altPositionId];
                pInvLocationReference = &ItemLocationReference[locationId][altPositionId];
                getItemLocationDimensions(locationId, altPositionId, invDimensions);
                break;

            case d2ce::EnumAltItemLocation::STASH:
                altPositionId = d2ce::EnumAltItemLocation::HORADRIC_CUBE;
                pInvEmptySlots = &ItemLocationEmptySpots[locationId][altPositionId];
                pInvLocationReference = &ItemLocationReference[locationId][altPositionId];
                getItemLocationDimensions(locationId, altPositionId, invDimensions);
                break;

            case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
            default:
                pInvEmptySlots = nullptr;
                break;
            }
        }
        else
        {
            auto iterMove = itemToMove.begin();
            auto& item = *iterMove;
            Inventory.splice(Inventory.end(), itemToMove, iterMove);
            if (std::find_if(pInvLocationReference->begin(), pInvLocationReference->end(), ItemPredicate(item)) == pInvLocationReference->end())
            {
                pInvLocationReference->push_back(item);
            }

            const auto& itemType = item.getItemTypeHelper();
            if (itemType.isPotion() || itemType.isGem() || itemType.isRune())
            {
                GPSs.push_back(item);
            }

            posX = *(pInvEmptySlots->begin());
            if (item.setLocation(locationId, altPositionId, posX % invDimensions.Width, posX / invDimensions.Width))
            {
                pInvEmptySlots->erase(posX);
            }
        }
    }

    if (!itemToMove.empty())
    {
        BufferItems.splice(BufferItems.end(), itemToMove, itemToMove.begin(), itemToMove.end());
    }
}
//---------------------------------------------------------------------------
void d2ce::Items::verifyRestrictedItems()
{
    std::vector<std::reference_wrapper<ItemType>> foundRestrictedItem;
    bool bFoundCube = false;
    bool bHadCube = getHasHoradricCube();
    for (const auto& item : Inventory)
    {
        if (item.isRestrictedItem())
        {
            foundRestrictedItem.push_back(const_cast<d2ce::ItemType&>(item.getItemTypeHelper()));
            if (!bFoundCube && item.isHoradricCube())
            {
                bFoundCube = true;
            }
        }
    }

    HasRestrictedItem.swap(foundRestrictedItem);
    if (bHadCube == bFoundCube)
    {
        // nothing changed
        return;
    }

    ItemDimensions dimensions;
    auto itemLoaction = d2ce::EnumItemLocation::STORED;
    auto itemAltLocation = EnumAltItemLocation::HORADRIC_CUBE;
    auto& emptySlots = ItemLocationEmptySpots[itemLoaction][itemAltLocation];
    auto& invLocationReference = ItemLocationReference[itemLoaction][itemAltLocation];
    if (bFoundCube)
    {
        ItemDimensions cubeDimensions;
        ItemDimensions invItemDimensions;
        if (getItemLocationDimensions(itemAltLocation, cubeDimensions))
        {
            std::uint16_t totalPos = cubeDimensions.InvHeight * cubeDimensions.InvWidth;
            for (std::uint16_t pos = 0; pos < totalPos; ++pos)
            {
                emptySlots.insert(pos);
            }

            // move any Horadric Cube items in the buffer back into the inventory
            auto iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemIsInHoradricCube());
            while (iter != BufferItems.end() && !emptySlots.empty())
            {
                auto& item = *iter;
                if (!item.getDimensions(dimensions))
                {
                    item.setLocation(EnumItemLocation::BUFFER, 0, 0);
                    iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemIsInHoradricCube());
                    continue;
                }

                bool bFits = true;
                auto positionX = std::uint16_t(item.getPositionX());
                auto positionY = std::uint16_t(item.getPositionY());
                auto positionX2 = std::uint16_t(positionX + dimensions.Width - 1);
                auto positionY2 = std::uint16_t(positionY + dimensions.Height - 1);
                for (auto& itemRef : invLocationReference)
                {
                    auto& invItem = itemRef.get();
                    if (&invItem == &item)
                    {
                        // don't bother checking yourself
                        continue;
                    }

                    if (!invItem.getDimensions(invItemDimensions))
                    {
                        continue;
                    }

                    auto invItemPosX = std::uint16_t(invItem.getPositionX());
                    auto invItemPosY = std::uint16_t(invItem.getPositionY());
                    auto invItemPosX2 = std::uint16_t(invItemPosX + dimensions.Width - 1);
                    auto invItemPosY2 = std::uint16_t(invItemPosY + dimensions.Height - 1);
                    if (positionX > invItemPosX2 || positionX2 < invItemPosX)
                    {
                        // does not intersect
                        continue;
                    }

                    if (positionY > invItemPosY2 || positionY2 < invItemPosY)
                    {
                        // does not intersect
                        continue;
                    }

                    // intersects with existing item
                    bFits = false;
                    item.setLocation(EnumItemLocation::BUFFER, 0, 0);
                    break;
                }

                if (!bFits)
                {
                    iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemIsInHoradricCube());
                }

                Inventory.splice(Inventory.end(), BufferItems, iter);
                if (std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemPredicate(item)) == invLocationReference.end())
                {
                    invLocationReference.push_back(item);
                }

                auto& itemType = item.getItemTypeHelper();
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
                else if (itemType.isGPSItem())
                {
                    GPSs.push_back(item);
                }

                // remove slots from empty list
                if (item.getDimensions(dimensions))
                {
                    for (std::uint16_t y = positionY; y <= positionY2; ++y)
                    {
                        for (std::uint16_t x = positionX; x <= positionX2; ++x)
                        {
                            emptySlots.erase(y * cubeDimensions.Width + x);
                        }
                    }
                }

                iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemIsInHoradricCube());
            }
        }

        return;
    }

    // Horadric Cube was removed, make sure any items located in there are removed as well
    std::list<Item> itemToMove; // non-empty if given Horadric Cube items need moving

    auto iter = Inventory.begin();
    for (const auto& itemRef : invLocationReference)
    {
        iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(itemRef.get()));
        if (iter != Inventory.end())
        {
            const auto& itemType = iter->getItemTypeHelper();
            itemToMove.splice(itemToMove.end(), Inventory, iter);

            if (itemType.isStackable())
            {
                d2ce::removeItem_if(Stackables, ItemPredicate(itemRef.get()));

                if (itemType.isWeapon())
                {
                    d2ce::removeItem_if(Weapons, ItemPredicate(itemRef.get()));
                }
            }
            else if (itemType.isWeapon())
            {
                d2ce::removeItem_if(Weapons, ItemPredicate(itemRef.get()));
            }
            else if (itemType.isArmor())
            {
                d2ce::removeItem_if(Armor, ItemPredicate(itemRef.get()));
            }
            else if (itemType.isGPSItem())
            {
                d2ce::removeItem_if(GPSs, ItemPredicate(itemRef.get()));
            }
        }
    }

    if (itemToMove.empty())
    {
        return;
    }

    for (const auto& item : itemToMove)
    {
        d2ce::removeItem_if(invLocationReference, ItemPredicate(item));
    }

    BufferItems.splice(BufferItems.end(), itemToMove, itemToMove.begin(), itemToMove.end());
}
//---------------------------------------------------------------------------
/*
   Thanks goes to Stoned2000 for making his checksum calculation source
   available to the public.  The Visual Basic source for his editor can be
   found at http://stoned.d2network.com.
*/
void d2ce::Items::calculateChecksum(long& checksum, std::uint8_t& overflow, bool isExpansion, bool hasMercID)
{
    switch (getDefaultItemVersion())
    {
    case EnumItemVersion::v100: // v1.00 - v1.03 item
    case EnumItemVersion::v104: // v1.04 - v1.06 item
    case EnumItemVersion::v107: // v1.07 item
    case EnumItemVersion::v108: // v1.08 item
        return; // checksum not supported

    case EnumItemVersion::v109:  // v1.09 item
    case EnumItemVersion::v110:  // v1.10 - v1.14d item
    case EnumItemVersion::v100R: // v1.0.x - v1.1.x Diablo II: Resurrected item
    case EnumItemVersion::v120:  // v1.2.x - v1.3.x Diablo II: Resurrected Patch 2.4 item
    case EnumItemVersion::v140:  // v1.4.x+ Diablo II: Resurrected Patch 2.5 item
    default:
        GameVersion = isExpansion ? 100 : 1;
        break;
    }

    isMercHired = !MercItems.empty() || hasMercID;

    // Calulate checksum for inventory
    for (auto& byteValue : ITEM_MARKER)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += byteValue + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    std::uint16_t numItems = (std::uint16_t)Inventory.size();
    auto* pBytes = (std::uint8_t*)&numItems;
    for (size_t i = 0; i < sizeof(numItems); ++i)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += pBytes[i] + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    for (auto& item : Inventory)
    {
        item.calculateChecksum(checksum, overflow);
    }

    // Calulate checksum for corpse
    for (auto& byteValue : ITEM_MARKER)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += byteValue + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    if (CorpseInfo.IsDead != 1 || CorpseItems.empty())
    {
        CorpseInfo.clear();
        CorpseItems.clear();
    }

    pBytes = (std::uint8_t*)&CorpseInfo.IsDead;
    for (size_t i = 0; i < sizeof(CorpseInfo.IsDead); ++i)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += pBytes[i] + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    if (CorpseInfo.IsDead > 0)
    {
        pBytes = (std::uint8_t*)&CorpseInfo.Unknown;
        for (size_t i = 0; i < sizeof(CorpseInfo.Unknown); ++i)
        {
            checksum <<= 1; // doubles the checksum result by left shifting once
            checksum += pBytes[i] + overflow;
            if (checksum < 0)
            {
                overflow = 1;
            }
            else
            {
                overflow = 0;
            }
        }

        pBytes = (std::uint8_t*)&CorpseInfo.X;
        for (size_t i = 0; i < sizeof(CorpseInfo.X); ++i)
        {
            checksum <<= 1; // doubles the checksum result by left shifting once
            checksum += pBytes[i] + overflow;
            if (checksum < 0)
            {
                overflow = 1;
            }
            else
            {
                overflow = 0;
            }
        }

        pBytes = (std::uint8_t*)&CorpseInfo.Y;
        for (size_t i = 0; i < sizeof(CorpseInfo.Y); ++i)
        {
            checksum <<= 1; // doubles the checksum result by left shifting once
            checksum += pBytes[i] + overflow;
            if (checksum < 0)
            {
                overflow = 1;
            }
            else
            {
                overflow = 0;
            }
        }

        for (auto& byteValue : ITEM_MARKER)
        {
            checksum <<= 1; // doubles the checksum result by left shifting once
            checksum += byteValue + overflow;
            if (checksum < 0)
            {
                overflow = 1;
            }
            else
            {
                overflow = 0;
            }
        }

        numItems = (std::uint16_t)CorpseItems.size();
        pBytes = (std::uint8_t*)&numItems;
        for (size_t i = 0; i < sizeof(numItems); ++i)
        {
            checksum <<= 1; // doubles the checksum result by left shifting once
            checksum += pBytes[i] + overflow;
            if (checksum < 0)
            {
                overflow = 1;
            }
            else
            {
                overflow = 0;
            }
        }

        for (auto& item : CorpseItems)
        {
            item.calculateChecksum(checksum, overflow);
        }
    }

    if (!isExpansionItems())
    {
        return;
    }

    // Calulate checksum for mercenary
    for (auto& byteValue : MERC_ITEM_MARKER)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += byteValue + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    numItems = (std::uint16_t)MercItems.size();
    if (numItems > 0 || isMercHired)
    {
        isMercHired = true;
        for (auto& byteValue : ITEM_MARKER)
        {
            checksum <<= 1; // doubles the checksum result by left shifting once
            checksum += byteValue + overflow;
            if (checksum < 0)
            {
                overflow = 1;
            }
            else
            {
                overflow = 0;
            }
        }

        pBytes = (std::uint8_t*)&numItems;
        for (size_t i = 0; i < sizeof(numItems); ++i)
        {
            checksum <<= 1; // doubles the checksum result by left shifting once
            checksum += pBytes[i] + overflow;
            if (checksum < 0)
            {
                overflow = 1;
            }
            else
            {
                overflow = 0;
            }
        }

        for (auto& item : MercItems)
        {
            item.calculateChecksum(checksum, overflow);
        }
    }

    // Calulate checksum for Golem
    for (auto& byteValue : GOLEM_ITEM_MARKER)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += byteValue + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    std::uint8_t hasGolem = GolemItem.empty() ? 0 : 1;
    pBytes = &hasGolem;
    for (size_t i = 0; i < sizeof(hasGolem); ++i)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += pBytes[i] + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    if (!GolemItem.empty())
    {
        GolemItem.back().calculateChecksum(checksum, overflow);
    }
}
//---------------------------------------------------------------------------
d2ce::Items::Items() : BufferItems(Buffer)
{
}
//---------------------------------------------------------------------------
d2ce::Items::Items(std::list<Item>& bufferItems) : BufferItems(bufferItems)
{
}
//---------------------------------------------------------------------------
d2ce::Items::Items(const Items& other) : BufferItems(other.BufferItems)
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
    GameVersion = other.GameVersion;

    Inventory = other.Inventory;

    CorpseInfo = other.CorpseInfo;
    CorpseItems = other.CorpseItems;

    MercItems = other.MercItems;

    GolemItem = other.GolemItem;

    isMercHired = other.isMercHired;

    Buffer = other.Buffer;
    LastItemMoved = other.LastItemMoved;
    LastItemIdx = other.LastItemIdx;

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

    Version = std::exchange(other.Version, APP_ITEM_VERSION);
    GameVersion = std::exchange(other.GameVersion, APP_ITEM_GAME_VERSION);

    // copy reference to items
    GPSs.swap(other.GPSs);
    other.Inventory.clear();
    Stackables.swap(other.Stackables);
    other.Stackables.clear();
    Armor.swap(other.Armor);
    other.Armor.clear();
    Weapons.swap(other.Weapons);
    other.Weapons.clear();

    Inventory.swap(other.Inventory);
    other.Inventory.clear();

    CorpseInfo.swap(other.CorpseInfo);
    CorpseItems.swap(other.CorpseItems);
    other.CorpseItems.clear();

    MercItems.swap(other.MercItems);
    other.MercItems.clear();

    GolemItem.swap(other.GolemItem);
    other.GolemItem.clear();

    isMercHired = std::exchange(other.isMercHired, false);

    Buffer.swap(other.Buffer);
    LastItemMoved = std::exchange(other.LastItemMoved, nullptr);
    LastItemIdx = std::exchange(other.LastItemIdx, MAXSIZE_T);
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
    return ItemLocationReference[d2ce::EnumItemLocation::EQUIPPED][d2ce::EnumAltItemLocation::UNKNOWN];
}
//---------------------------------------------------------------------------
d2ce::EnumItemVersion d2ce::Items::getDefaultItemVersion() const
{
    return Version;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Items::getDefaultGameVersion() const
{
    return GameVersion;
}
//---------------------------------------------------------------------------
bool d2ce::Items::getItemLocationDimensions(EnumItemLocation locationId, EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const
{
    dimensions.clear();
    switch (locationId)
    {
    case EnumItemLocation::STORED:
        return InventoryGridHelpers::GetSize(altPositionId, dimensions, ConvertItemVersion(Version), isExpansionItems());

    case EnumItemLocation::BELT:
        dimensions.Width = dimensions.InvWidth = 4;
        dimensions.Height = dimensions.InvHeight = std::max(std::uint16_t(1), std::uint16_t(getMaxNumberOfItemsInBelt() / 4));
        return true;

    case EnumItemLocation::EQUIPPED:
        dimensions.Height = dimensions.InvHeight = 1;
        dimensions.Width = dimensions.InvWidth = static_cast<std::underlying_type_t<EnumEquippedId>>(EnumEquippedId::ALT_LEFT_ARM);
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
    return ItemLocationReference[d2ce::EnumItemLocation::BELT][d2ce::EnumAltItemLocation::UNKNOWN];
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInInventory() const
{
    return getItemsInInventory().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInInventory() const
{
    return ItemLocationReference[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::INVENTORY];
}
//---------------------------------------------------------------------------
size_t d2ce::Items::getNumberOfItemsInStash() const
{
    return getItemsInStash().size();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Items::getItemsInStash() const
{
    return ItemLocationReference[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::STASH];
}
//---------------------------------------------------------------------------
bool d2ce::Items::getHasHoradricCube() const
{
    for (const auto& itemType : HasRestrictedItem)
    {
        if (itemType.get().isHoradricCube())
        {
            return true;
        }
    }

    return false;
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
    return ItemLocationReference[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::HORADRIC_CUBE];
}
//---------------------------------------------------------------------------
bool d2ce::Items::getHasRestrictedItem(const d2ce::ItemType& itemType) const
{
    if (!itemType.isRestrictedItem())
    {
        return false;
    }

    for (const auto& invItemType : HasRestrictedItem)
    {
        if (&itemType == &(invItemType.get()))
        {
            return true;
        }

        if (itemType.isQuestItem())
        {
            if (invItemType.get().isQuestItem())
            {
                if (itemType.code == invItemType.get().code)
                {
                    return true;
                }

                if ((invItemType.get().isHoradricStaff() && itemType.isHoradricStaffPart()) ||
                    (invItemType.get().isHoradricStaffPart() && itemType.isHoradricStaff()))
                {
                    // once we have the staff and the other parts are not allowed to co-exist
                    return true;
                }
            }
        }
        else if (itemType.isUniqueItem())
        {
            if (invItemType.get().isUniqueItem())
            {
                if (itemType.getId() == invItemType.get().getId())
                {
                    return true;
                }
            }
        }
    }

    return false;
}
//---------------------------------------------------------------------------
// number of bytes to store all item sections
size_t d2ce::Items::getByteSize() const
{
    size_t byteSize = ITEM_MARKER.size() + sizeof(std::uint16_t);
    for (auto& item : Inventory)
    {
        byteSize += item.getFullSize();
    }

    byteSize += ITEM_MARKER.size() + sizeof(CorpseInfo.IsDead);
    if (CorpseInfo.IsDead > 0)
    {
        byteSize += sizeof(CorpseInfo.Unknown) + sizeof(CorpseInfo.X) + sizeof(CorpseInfo.Y) + ITEM_MARKER.size() + sizeof(std::uint16_t);
        for (auto& item : CorpseItems)
        {
            byteSize += item.getFullSize();
        }
    }

    if (!isExpansionItems())
    {
        if (Version < EnumItemVersion::v109)
        {
            byteSize += ITEM_MARKER.size() + sizeof(MercId_v100);
        }
        return byteSize;
    }

    byteSize += MERC_ITEM_MARKER.size();
    if (!MercItems.empty() || isMercHired)
    {
        byteSize += ITEM_MARKER.size() + sizeof(std::uint16_t);
        for (auto& item : MercItems)
        {
            byteSize += item.getFullSize();
        }
    }

    byteSize += GOLEM_ITEM_MARKER.size() + sizeof(std::uint8_t);
    if (!GolemItem.empty())
    {
        byteSize += GolemItem.back().getFullSize();
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
    std::vector<MagicalAttribute> unfilteredAttribs;
    std::vector<MagicalAttribute> tempAttribs;
    std::multimap<size_t, size_t> itemIndexMap;
    for (auto& item : MercItems)
    {
        if (item.getCombinedMagicalAttributes(tempAttribs))
        {
            ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, unfilteredAttribs);
        }
    }

    // filter out the properties that give us a bonuses
    FilterMagicalAttribsForBonus(unfilteredAttribs, attribs);
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
    std::uint64_t eDef = 0;
    std::uint64_t defenseRating = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : MercItems)
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

    defenseRating += (defenseRating * eDef) / 100;
    return std::uint16_t(defenseRating);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getCombinedMercDamage(BaseDamage& damage, std::uint32_t charLevel) const
{
    ItemDamage itemDamage;
    damage.clear();

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
    return GolemItem.empty() ? false : true;
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& d2ce::Items::getGolemItem() const
{
    return GolemItem;
}
//---------------------------------------------------------------------------
d2ce::Item d2ce::Items::removeItem(const d2ce::Item& item)
{
    auto iter = Inventory.begin();
    auto iter_end = Inventory.end();
    for (; iter != iter_end; ++iter)
    {
        auto& invItem = *iter;
        if (&invItem == &item)
        {
            // found the item
            d2ce::Item copy;
            copy.swap(invItem);
            copy.setLocation(EnumItemLocation::BUFFER, EnumAltItemLocation::UNKNOWN, 0, 0);
            Inventory.erase(iter);
            findItems();
            return copy;
        }
    }

    iter = CorpseItems.begin();
    iter_end = CorpseItems.end();
    for (; iter != iter_end; ++iter)
    {
        auto& invItem = *iter;
        if (&invItem == &item)
        {
            // found the item
            d2ce::Item copy;
            copy.swap(invItem);
            copy.setLocation(EnumItemLocation::BUFFER, EnumAltItemLocation::UNKNOWN, 0, 0);
            CorpseItems.erase(iter);
            findItems();
            return copy;
        }
    }

    iter = MercItems.begin();
    iter_end = MercItems.end();
    for (; iter != iter_end; ++iter)
    {
        auto& invItem = *iter;
        if (&invItem == &item)
        {
            // found the item
            d2ce::Item copy;
            copy.swap(invItem);
            copy.setLocation(EnumItemLocation::BUFFER, EnumAltItemLocation::UNKNOWN, 0, 0);
            MercItems.erase(iter);
            findItems();
            return copy;
        }
    }

    iter = GolemItem.begin();
    iter_end = GolemItem.end();
    for (; iter != iter_end; ++iter)
    {
        auto& invItem = *iter;
        if (&invItem == &item)
        {
            // found the item
            d2ce::Item copy;
            copy.swap(invItem);
            copy.setLocation(EnumItemLocation::BUFFER, EnumAltItemLocation::UNKNOWN, 0, 0);
            GolemItem.erase(iter);
            findItems();
            return copy;
        }
    }

    // not found
    return d2ce::Item();
}
//---------------------------------------------------------------------------
bool d2ce::Items::canEquipItem(const d2ce::Item& item, EnumEquippedId equipId) const
{
    return item.canEquip(equipId);
}
//---------------------------------------------------------------------------
bool d2ce::Items::canEquipItem(const d2ce::Item& item, EnumEquippedId equipId, EnumCharClass charClass, const CharStats& cs) const
{
    return item.canEquip(equipId, charClass, cs);
}
//---------------------------------------------------------------------------
/*
   Converts the all gems to their perfect state
   Returns the number of gems converted.
*/
size_t d2ce::Items::upgradeGems(ItemFilter filter)
{
    if (GPSs.empty())
    {
        return 0;
    }

    bool bFiltered = false;
    filter.clearEquipped(); // GPS can't be equipped
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;
    }

    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        if (bFiltered)
        {
            if (item.get().getLocation() != filter.LocationId)
            {
                // skip item
                continue;
            }

            if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
            {
                // skip item
                continue;
            }
        }

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
size_t d2ce::Items::upgradePotions(ItemFilter filter)
{
    if (GPSs.empty())
    {
        return 0;
    }

    bool bFiltered = false;
    filter.clearEquipped(); // GPS can't be equipped
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;
    }

    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        if (bFiltered)
        {
            if (item.get().getLocation() != filter.LocationId)
            {
                // skip item
                continue;
            }

            if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
            {
                // skip item
                continue;
            }
        }

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
size_t d2ce::Items::upgradeRejuvenationPotions(ItemFilter filter)
{
    if (GPSs.empty())
    {
        return 0;
    }

    bool bFiltered = false;
    filter.clearEquipped(); // GPS can't be equipped
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;
    }

    size_t gemsconverted = 0;
    for (auto& item : GPSs)
    {
        if (bFiltered)
        {
            if (item.get().getLocation() != filter.LocationId)
            {
                // skip item
                continue;
            }

            if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
            {
                // skip item
                continue;
            }
        }

        if (item.get().upgradeToFullRejuvenationPotion())
        {
            ++gemsconverted;
        }
    }  // end for

    return gemsconverted;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::upgradeTierAllItems(const d2ce::Character& charInfo, ItemFilter filter)
{
    d2ce::CharStats cs;
    charInfo.fillDisplayedCharacterStats(cs);
    bool bFiltered = false;
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;

    case EnumItemLocation::EQUIPPED:
        bFiltered = true;
        break;
    }

    size_t itemsUpgraded = 0;
    if (!bFiltered || filter.IsBody || (filter.LocationId != EnumItemLocation::EQUIPPED))
    {
        for (auto& item : Armor)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().upgradeTier(cs))
            {
                ++itemsUpgraded;
            }
        }

        for (auto& item : Weapons)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().upgradeTier(cs))
            {
                ++itemsUpgraded;
            }
        }
    }

    if (!bFiltered || filter.IsMerc)
    {
        if (charInfo.hasMercenary())
        {
            d2ce::CharStats mercCs;
            charInfo.getMercenaryInfo().fillMercStats(mercCs);
            for (auto& item : MercItems)
            {
                if (item.upgradeTier(mercCs))
                {
                    ++itemsUpgraded;
                }
            }
        }
    }

    if (!bFiltered || filter.IsGolem)
    {
        for (auto& item : GolemItem)
        {
            if (item.upgradeTier(cs))
            {
                ++itemsUpgraded;
            }
        }
    }

    if (itemsUpgraded > 0)
    {
        verifyBeltSlots();
    }

    return itemsUpgraded;
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems, potions or skulls to the specified
   final gem, potion or skull.
   Returns the number of gems converted.
*/
size_t d2ce::Items::convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, ItemFilter filter)
{
    if (GPSs.empty())
    {
        return 0;
    }

    bool bFiltered = false;
    filter.clearEquipped(); // GPS can't be equipped
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;
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
        if (bFiltered)
        {
            if (item.get().getLocation() != filter.LocationId)
            {
                // skip item
                continue;
            }

            if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
            {
                // skip item
                continue;
            }
        }

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
size_t d2ce::Items::fillAllStackables(ItemFilter filter)
{
    if (Stackables.empty())
    {
        return 0;
    }

    bool bFiltered = false;
    filter.clearEquipped(); // only fill stackables for equipped on us
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;

    case EnumItemLocation::EQUIPPED:
        bFiltered = true;
        filter.IsBody = true;
        break;
    }

    size_t itemsFilled = 0;
    for (auto& item : Stackables)
    {
        if (bFiltered)
        {
            if (item.get().getLocation() != filter.LocationId)
            {
                // skip item
                continue;
            }

            if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
            {
                // skip item
                continue;
            }
        }

        if (item.get().setMaxQuantity())
        {
            ++itemsFilled;
        }
    }  // end for

    return itemsFilled;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::repairAllItems(ItemFilter filter)
{
    bool bFiltered = false;
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;

    case EnumItemLocation::EQUIPPED:
        bFiltered = true;
        break;
    }

    size_t itemsFixed = 0;
    if (!bFiltered || filter.IsBody || (filter.LocationId != EnumItemLocation::EQUIPPED))
    {
        for (auto& item : Armor)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().fixDurability())
            {
                ++itemsFixed;
            }
        }

        for (auto& item : Weapons)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().fixDurability())
            {
                ++itemsFixed;
            }
        }
    }

    if (!bFiltered || filter.IsMerc)
    {
        for (auto& item : MercItems)
        {
            if (item.fixDurability())
            {
                ++itemsFixed;
            }
        }
    }

    if (!bFiltered || filter.IsGolem)
    {
        for (auto& item : GolemItem)
        {
            if (item.fixDurability())
            {
                ++itemsFixed;
            }
        }
    }

    return itemsFixed;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::maxDurabilityAllItems(ItemFilter filter)
{
    bool bFiltered = false;
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;

    case EnumItemLocation::EQUIPPED:
        bFiltered = true;
        break;
    }

    size_t itemsFixed = 0;
    if (!bFiltered || filter.IsBody || (filter.LocationId != EnumItemLocation::EQUIPPED))
    {
        for (auto& item : Armor)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().setMaxDurability())
            {
                ++itemsFixed;
            }
        }

        for (auto& item : Weapons)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().setMaxDurability())
            {
                ++itemsFixed;
            }
        }
    }

    if (!bFiltered || filter.IsMerc)
    {
        for (auto& item : MercItems)
        {
            if (item.setMaxDurability())
            {
                ++itemsFixed;
            }
        }
    }

    if (!bFiltered || filter.IsGolem)
    {
        for (auto& item : GolemItem)
        {
            if (item.setMaxDurability())
            {
                ++itemsFixed;
            }
        }
    }

    return itemsFixed;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::maxSocketCountAllItems(ItemFilter filter)
{
    bool bFiltered = false;
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;

    case EnumItemLocation::EQUIPPED:
        bFiltered = true;
        break;
    }

    size_t itemsChanged = 0;
    if (!bFiltered || filter.IsBody || (filter.LocationId != EnumItemLocation::EQUIPPED))
    {
        for (auto& item : Armor)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().addMaxSocketCount())
            {
                ++itemsChanged;
            }
        }

        for (auto& item : Weapons)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().addMaxSocketCount())
            {
                ++itemsChanged;
            }
        }
    }

    if (!bFiltered || filter.IsMerc)
    {
        for (auto& item : MercItems)
        {
            if (item.addMaxSocketCount())
            {
                ++itemsChanged;
            }
        }
    }

    return itemsChanged;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::setMaxDefenseRatingAllItems(ItemFilter filter)
{
    bool bFiltered = false;
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;

    case EnumItemLocation::EQUIPPED:
        bFiltered = true;
        break;
    }

    size_t itemsChanged = 0;
    if (!bFiltered || filter.IsBody || (filter.LocationId != EnumItemLocation::EQUIPPED))
    {
        for (auto& item : Armor)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().setMaxDefenseRating())
            {
                ++itemsChanged;
            }
        }
    }

    if (!bFiltered || filter.IsMerc)
    {
        for (auto& item : MercItems)
        {
            if (item.setMaxDefenseRating())
            {
                ++itemsChanged;
            }
        }
    }

    return itemsChanged;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::setIndestructibleAllItems(ItemFilter filter)
{
    bool bFiltered = false;
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;

    case EnumItemLocation::EQUIPPED:
        bFiltered = true;
        break;
    }

    size_t itemsChanged = 0;
    if (!bFiltered || filter.IsBody || (filter.LocationId != EnumItemLocation::EQUIPPED))
    {
        for (auto& item : Armor)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().setIndestructible())
            {
                ++itemsChanged;
            }
        }

        for (auto& item : Weapons)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().setIndestructible())
            {
                ++itemsChanged;
            }
        }
    }

    if (!bFiltered || filter.IsMerc)
    {
        for (auto& item : MercItems)
        {
            if (item.setIndestructible())
            {
                ++itemsChanged;
            }
        }
    }

    return itemsChanged;
}
//---------------------------------------------------------------------------
size_t d2ce::Items::setSuperiorAllItems(ItemFilter filter)
{
    bool bFiltered = false;
    switch (filter.LocationId)
    {
    case EnumItemLocation::STORED:
        switch (filter.AltPositionId)
        {
        case EnumAltItemLocation::HORADRIC_CUBE:
        case EnumAltItemLocation::INVENTORY:
        case EnumAltItemLocation::STASH:
            bFiltered = true;
            break;
        }
        break;

    case EnumItemLocation::BELT:
        bFiltered = true;
        break;

    case EnumItemLocation::EQUIPPED:
        bFiltered = true;
        break;
    }

    size_t itemsChanged = 0;
    if (!bFiltered || filter.IsBody || (filter.LocationId != EnumItemLocation::EQUIPPED))
    {
        for (auto& item : Armor)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().makeSuperior())
            {
                ++itemsChanged;
            }
        }

        for (auto& item : Weapons)
        {
            if (bFiltered)
            {
                if (item.get().getLocation() != filter.LocationId)
                {
                    // skip item
                    continue;
                }

                if ((filter.LocationId == EnumItemLocation::STORED) && (item.get().getAltPositionId() != filter.AltPositionId))
                {
                    // skip item
                    continue;
                }
            }

            if (item.get().makeSuperior())
            {
                ++itemsChanged;
            }
        }
    }

    if (!bFiltered || filter.IsMerc)
    {
        for (auto& item : MercItems)
        {
            if (item.makeSuperior())
            {
                ++itemsChanged;
            }
        }
    }

    return itemsChanged;
}
//---------------------------------------------------------------------------
bool d2ce::Items::addItem(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    d2ce::ItemCreateParams createParams(getDefaultItemVersion(), strcode, isExpansionItems());
    Item newItem(createParams);
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

    switch (locationId)
    {
    case EnumItemLocation::STORED:
        break;

    case EnumItemLocation::BELT:
        if (!newItem.isBeltable())
        {
            return false;
        }
        altPositionId = EnumAltItemLocation::UNKNOWN;
        break;

    case EnumItemLocation::EQUIPPED:
        altPositionId = EnumAltItemLocation::UNKNOWN;
        break;

    default:
        return false;
    }

    auto& itemLocationEmptySpots = ItemLocationEmptySpots[locationId][altPositionId];
    if (itemLocationEmptySpots.empty())
    {
        // no free space
        return false;
    }

    // make sure we have free slots
    std::uint16_t positionY = 0;
    std::uint16_t positionX = 0;
    std::uint16_t freeSlot = MAXUINT16;
    for (const auto& slot : itemLocationEmptySpots)
    {
        positionY = slot / invDimensions.InvWidth;
        positionX = slot % invDimensions.InvWidth;
        bool isGoodLocation = true;
        for (std::uint16_t y = positionY; y < std::uint16_t(positionY + dimensions.Height) && isGoodLocation; ++y)
        {
            for (std::uint16_t x = positionX; x < std::uint16_t(positionX + dimensions.Width) && isGoodLocation; ++x)
            {
                if (itemLocationEmptySpots.find(y * invDimensions.InvWidth + x) == itemLocationEmptySpots.end())
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
    
    switch (locationId)
    {
    case EnumItemLocation::BELT:
        positionX = freeSlot;
        positionY = 0;
        break;
    }

    BufferItems.resize(BufferItems.size() + 1);
    auto& bufferItem = BufferItems.back();
    bufferItem.swap(newItem);
    const d2ce::Item* pRemovedItem = nullptr;
    if (!setItemLocation(bufferItem, locationId, altPositionId, positionX, positionY, d2ce::EnumItemInventory::PLAYER, pRemovedItem))
    {
        d2ce::removeItem_if(BufferItems, ItemPredicate(bufferItem));
        return false;
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
bool d2ce::Items::importItem(const d2ce::Item*& pImportedItem, bool bRandomizeId)
{
    if (pImportedItem == nullptr)
    {
        return false;
    }

    // import item
    auto& importedItem = *const_cast<d2ce::Item*>(pImportedItem);
    pImportedItem = nullptr;
    if (importedItem.data.empty())
    {
        return false;
    }

    const auto& itemType = importedItem.getItemTypeHelper();
    if (itemType.isRestrictedItem() && getHasRestrictedItem(itemType))
    {
        // can only have one of this item
        return false;
    }

    if (IsSharedStash && itemType.isRestrictedSharedStashItem())
    {
        // can't add quest items to shared stash
        return false;
    }

    // place it into the buffer
    if (!importedItem.setLocation(EnumItemLocation::BUFFER, EnumAltItemLocation::UNKNOWN, 0, 0))
    {
        return false;
    }

    if (importedItem.isPersonalized() && (importedItem.isRuneword() || !itemType.canPersonalize()))
    {
        // Can't personalize this item, so remove personalization
        importedItem.removePersonalization();
    }

    if (bRandomizeId)
    {
        importedItem.randomizeId();
    }

    BufferItems.resize(BufferItems.size() + 1);
    auto& bufferItem = BufferItems.back();
    bufferItem.swap(importedItem);
    pImportedItem = &bufferItem;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::importItem(const std::filesystem::path& path, const d2ce::Item*& pImportedItem, bool bRandomizeId)
{
    // import item
    Item importedItem(Version, isExpansionItems(), path);
    pImportedItem = &importedItem;
    return importItem(pImportedItem, bRandomizeId);
}
//---------------------------------------------------------------------------
bool d2ce::Items::exportItem(d2ce::Item& item, const std::filesystem::path& path) const
{
    return item.exportItem(path);
}
//---------------------------------------------------------------------------
bool d2ce::Items::setItemLocation(d2ce::Item& item, EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const  d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    switch (locationId)
    {
    case EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case EnumAltItemLocation::UNKNOWN:
            // invalid state
            return false;

        case EnumAltItemLocation::HORADRIC_CUBE:
            if (item.isHoradricCube())
            {
                // invalid state
                return false;
            }
            break;
        }
        break;

    case EnumItemLocation::EQUIPPED:
        // invalid state
        return false;

    case EnumItemLocation::BELT:
    case EnumItemLocation::BUFFER:
        altPositionId = EnumAltItemLocation::UNKNOWN;
        break;

    default:
        return false;
    }

    switch (invType)
    {
    case d2ce::EnumItemInventory::PLAYER:
    case d2ce::EnumItemInventory::BUFFER:
        break;

    case d2ce::EnumItemInventory::SHARED_STASH:
        if ((locationId != d2ce::EnumItemLocation::STORED) ||
            (altPositionId != EnumAltItemLocation::STASH))
        {
            // invalid state
            return false;
        }
        break;

    case d2ce::EnumItemInventory::CORPSE:
    case d2ce::EnumItemInventory::MERCENARY:
    case d2ce::EnumItemInventory::GOLEM:
    default:
        return false;
    }

    std::list<Item> removedItems; // items to be removed into the buffer
    std::array<std::uint8_t, 4> strcode;
    if (!item.getItemCode(strcode))
    {
        // should not happen
        return false;
    }

    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    std::map<d2ce::EnumItemLocation, std::map<d2ce::EnumAltItemLocation, ItemDimensions>> invDimensions;
    getItemLocationDimensions(d2ce::EnumAltItemLocation::INVENTORY, invDimensions[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::INVENTORY]);
    getItemLocationDimensions(d2ce::EnumAltItemLocation::HORADRIC_CUBE, invDimensions[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::HORADRIC_CUBE]);
    getItemLocationDimensions(d2ce::EnumAltItemLocation::STASH, invDimensions[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::STASH]);
    getItemLocationDimensions(d2ce::EnumItemLocation::BELT, invDimensions[d2ce::EnumItemLocation::BELT][d2ce::EnumAltItemLocation::UNKNOWN]);

    // Detect if is item being moved between inventory types
    ItemDimensions dimensions;
    if (!item.getDimensions(dimensions))
    {
        // should not happen
        return false;
    }

    bool bCheckEquippedBelt = false;
    d2ce::EnumItemInventory itemInvType = d2ce::EnumItemInventory::UNKNOWN;
    auto itemLocationId = item.getLocation();
    auto itemAltPositionId = (itemLocationId == d2ce::EnumItemLocation::STORED) ? item.getAltPositionId() : d2ce::EnumAltItemLocation::UNKNOWN;
    std::list<Item> itemToMove; // non-empty if given item is moving inventories
    std::list<d2ce::Item>* pMoveInv = nullptr;
    size_t iterMoveIdx = MAXSIZE_T;
    auto& itemInvDimenstion = invDimensions[itemLocationId][itemAltPositionId];
    auto& itemEmptySlots = ItemLocationEmptySpots[itemLocationId][itemAltPositionId];
    std::uint16_t itemPositionX = item.getPositionX();
    std::uint16_t itemPositionY = item.getPositionY();
    std::uint16_t itemPositionX2 = itemPositionX + dimensions.Width - 1;
    std::uint16_t itemPositionY2 = itemPositionY + dimensions.Height - 1;
    if ((itemLocationId != locationId) || (itemAltPositionId != altPositionId) || (invType == EnumItemInventory::BUFFER))
    {
        // item is moving between containers
        std::list<Item>::iterator iter;
        switch (itemLocationId)
        {
        case d2ce::EnumItemLocation::STORED:
        case d2ce::EnumItemLocation::BELT:
            // Find item and remove references and mark occupied slots as empty again
            iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(item));
            if (iter != Inventory.end())
            {
                d2ce::removeItem_if(ItemLocationReference[itemLocationId][itemAltPositionId], ItemPredicate(item));
                if (itemLocationId == EnumItemLocation::BELT)
                {
                    itemInvType = d2ce::EnumItemInventory::PLAYER;
                }
                else
                {
                    switch (invType)
                    {
                    case d2ce::EnumItemInventory::SHARED_STASH:
                    case d2ce::EnumItemInventory::PLAYER:
                        itemInvType = invType;
                        break;

                    default:
                        itemInvType = d2ce::EnumItemInventory::PLAYER;
                        break;
                    }
                }

                // mark item slots as empty again
                for (std::uint16_t y = itemPositionY; y <= itemPositionY2; ++y)
                {
                    for (std::uint16_t x = itemPositionX; x <= itemPositionX2; ++x)
                    {
                        itemEmptySlots.insert(y * itemInvDimenstion.Width + x);
                    }
                }

                // Check if we are moving inventories
                bool bMoving = false;
                switch (locationId)
                {
                case d2ce::EnumItemLocation::STORED:
                case d2ce::EnumItemLocation::BELT:
                    switch (invType)
                    {
                    case d2ce::EnumItemInventory::BUFFER:
                        // we are moving
                        bMoving = true;
                        break;
                    }
                    break;

                case d2ce::EnumItemLocation::BUFFER:
                    // we are moving
                    bMoving = true;
                    break;

                default:
                    return false;
                }

                if (bMoving)
                {
                    pMoveInv = &Inventory;
                    iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                    itemToMove.splice(itemToMove.end(), Inventory, iter);

                    if (itemType.isStackable())
                    {
                        d2ce::removeItem_if(Stackables, ItemPredicate(item));

                        if (itemType.isWeapon())
                        {
                            d2ce::removeItem_if(Weapons, ItemPredicate(item));
                        }
                    }
                    else if (itemType.isWeapon())
                    {
                        d2ce::removeItem_if(Weapons, ItemPredicate(item));
                    }
                    else if (itemType.isArmor())
                    {
                        d2ce::removeItem_if(Armor, ItemPredicate(item));
                    }
                    else if (itemType.isGPSItem())
                    {
                        d2ce::removeItem_if(GPSs, ItemPredicate(item));
                    }
                }
            }
            else
            {
                iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
                if (iter != BufferItems.end())
                {
                    // Buffer item
                    itemInvType = d2ce::EnumItemInventory::BUFFER;

                    // Check if we are moving inventories
                    if (invType == EnumItemInventory::BUFFER)
                    {
                        LastItemMoved = nullptr;
                        LastItemIdx = MAXSIZE_T;
                        return true;
                    }

                    switch (locationId)
                    {
                    case d2ce::EnumItemLocation::STORED:
                    case d2ce::EnumItemLocation::BELT:
                        break;

                    case d2ce::EnumItemLocation::BUFFER:
                        LastItemMoved = nullptr;
                        LastItemIdx = MAXSIZE_T;
                        return true;

                    default:
                        return false;
                    }

                    itemToMove.splice(itemToMove.end(), BufferItems, iter);
                    pMoveInv = &BufferItems;
                    iterMoveIdx = MAXSIZE_T;
                }
                else
                {
                    // can't find item!
                    return false;
                }
            }
            break;

        case d2ce::EnumItemLocation::EQUIPPED:
            // find out where it is equipped (i.e. Player, Player Cropse, Mercenary or Golem)
            iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(item));
            if (iter != Inventory.end())
            {
                // Still in same main inventory but moving in references
                itemInvType = d2ce::EnumItemInventory::PLAYER;
                d2ce::removeItem_if(ItemLocationReference[itemLocationId][itemAltPositionId], ItemPredicate(item));
                if (item.isBelt())
                {
                    // make sure the potions stored in the inventory is correct
                    bCheckEquippedBelt = true;
                }

                // Check if we are moving inventories
                bool bMoving = false;
                switch (locationId)
                {
                case d2ce::EnumItemLocation::STORED:
                case d2ce::EnumItemLocation::BELT:
                    switch (invType)
                    {
                    case d2ce::EnumItemInventory::BUFFER:
                        // we are moving
                        bMoving = true;
                        break;
                    }
                    break;

                case d2ce::EnumItemLocation::BUFFER:
                    // we are moving
                    bMoving = true;
                    break;

                default:
                    return false;
                }

                if (bMoving)
                {
                    // moving to another inventory
                    pMoveInv = &Inventory;
                    iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                    itemToMove.splice(itemToMove.end(), Inventory, iter);

                    if (itemType.isStackable())
                    {
                        d2ce::removeItem_if(Stackables, ItemPredicate(item));

                        if (itemType.isWeapon())
                        {
                            d2ce::removeItem_if(Weapons, ItemPredicate(item));
                        }
                    }
                    else if (itemType.isWeapon())
                    {
                        d2ce::removeItem_if(Weapons, ItemPredicate(item));
                    }
                    else if (itemType.isArmor())
                    {
                        d2ce::removeItem_if(Armor, ItemPredicate(item));
                    }
                    else if (itemType.isGPSItem())
                    {
                        d2ce::removeItem_if(GPSs, ItemPredicate(item));
                    }
                }
            }
            else
            {
                iter = std::find_if(CorpseItems.begin(), CorpseItems.end(), ItemPredicate(item));
                if (iter != CorpseItems.end())
                {
                    // Player Corpse inventory
                    itemInvType = d2ce::EnumItemInventory::CORPSE;

                    // Check if we are moving inventories
                    bool bMoving = false;
                    switch (locationId)
                    {
                    case d2ce::EnumItemLocation::STORED:
                    case d2ce::EnumItemLocation::BELT:
                    case d2ce::EnumItemLocation::BUFFER:
                        // we are moving
                        bMoving = true;
                        break;

                    default:
                        return false;
                    }

                    if (bMoving)
                    {
                        pMoveInv = &CorpseItems;
                        iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                        itemToMove.splice(itemToMove.end(), CorpseItems, iter);
                    }
                }

                if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
                {
                    iter = std::find_if(MercItems.begin(), MercItems.end(), ItemPredicate(item));
                    if (iter != MercItems.end())
                    {
                        // Mercenary inventory
                        itemInvType = d2ce::EnumItemInventory::MERCENARY;

                        // Check if we are moving inventories
                        bool bMoving = false;
                        switch (locationId)
                        {
                        case d2ce::EnumItemLocation::STORED:
                        case d2ce::EnumItemLocation::BELT:
                        case d2ce::EnumItemLocation::BUFFER:
                            // we are moving
                            bMoving = true;
                            break;

                        default:
                            return false;
                        }

                        if (bMoving)
                        {
                            pMoveInv = &MercItems;
                            iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                            itemToMove.splice(itemToMove.end(), MercItems, iter);
                        }
                    }
                }

                if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
                {
                    iter = std::find_if(GolemItem.begin(), GolemItem.end(), ItemPredicate(item));
                    if (iter != GolemItem.end())
                    {
                        // Player Corpse inventory
                        itemInvType = d2ce::EnumItemInventory::GOLEM;

                        // Check if we are moving inventories
                        switch (locationId)
                        {
                        case d2ce::EnumItemLocation::STORED:
                        case d2ce::EnumItemLocation::BELT:
                        case d2ce::EnumItemLocation::BUFFER:
                            break;

                        default:
                            return false;
                        }

                        itemToMove.splice(itemToMove.end(), GolemItem, iter);
                        pMoveInv = &GolemItem;
                        iterMoveIdx = MAXSIZE_T;
                    }
                }

                if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
                {
                    iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
                    if (iter != BufferItems.end())
                    {
                        // Buffer item
                        itemInvType = d2ce::EnumItemInventory::BUFFER;

                        // Check if we are moving inventories
                        if (invType == EnumItemInventory::BUFFER)
                        {
                            LastItemMoved = nullptr;
                            LastItemIdx = MAXSIZE_T;
                            return true;
                        }

                        switch (locationId)
                        {
                        case d2ce::EnumItemLocation::STORED:
                        case d2ce::EnumItemLocation::BELT:
                            break;

                        case d2ce::EnumItemLocation::BUFFER:
                            LastItemMoved = nullptr;
                            LastItemIdx = MAXSIZE_T;
                            return true;

                        default:
                            return false;
                        }

                        itemToMove.splice(itemToMove.end(), BufferItems, iter);
                        pMoveInv = &BufferItems;
                        iterMoveIdx = MAXSIZE_T;
                    }
                }

                if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
                {
                    // failed to find item to move
                    return false;
                }
            }
            break;

        case d2ce::EnumItemLocation::BUFFER:
            iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
            if (iter != BufferItems.end())
            {
                // Buffer item
                itemInvType = d2ce::EnumItemInventory::BUFFER;

                // Check if we are moving inventories
                if (invType == EnumItemInventory::BUFFER)
                {
                    LastItemMoved = nullptr;
                    LastItemIdx = MAXSIZE_T;
                    return true;
                }

                switch (locationId)
                {
                case d2ce::EnumItemLocation::STORED:
                case d2ce::EnumItemLocation::BELT:
                    break;

                case d2ce::EnumItemLocation::BUFFER:
                    LastItemMoved = nullptr;
                    LastItemIdx = MAXSIZE_T;
                    return true;

                default:
                    return false;
                }

                itemToMove.splice(itemToMove.end(), BufferItems, iter);
                pMoveInv = &BufferItems;
                iterMoveIdx = MAXSIZE_T;
            }
            break;
        }
    }
    else
    {
        std::list<Item>::iterator iter;
        switch (itemLocationId)
        {
        case d2ce::EnumItemLocation::STORED:
        case d2ce::EnumItemLocation::BELT:
            // Find item and remove references and mark occupied slots as empty again
            iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(item));
            if (iter != Inventory.end())
            {
                switch (invType)
                {
                case d2ce::EnumItemInventory::UNKNOWN:
                    return false; // invalid

                case d2ce::EnumItemInventory::PLAYER:
                case d2ce::EnumItemInventory::SHARED_STASH:
                    itemInvType = invType;
                    break;

                default:
                    itemInvType = d2ce::EnumItemInventory::PLAYER;
                    break;
                }
            }
            else
            {
                iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
                if (iter == BufferItems.end())
                {
                    // could not find it
                    return false;
                }

                switch (invType)
                {
                case d2ce::EnumItemInventory::UNKNOWN:
                    return false; // invalid

                default:
                    itemInvType = d2ce::EnumItemInventory::BUFFER;
                    break;
                }

                // Check if we are moving inventories
                switch (locationId)
                {
                case d2ce::EnumItemLocation::STORED:
                case d2ce::EnumItemLocation::BELT:
                    break;

                case d2ce::EnumItemLocation::BUFFER:
                    LastItemMoved = nullptr;
                    LastItemIdx = MAXSIZE_T;
                    return true;

                default:
                    return false;
                }

                itemToMove.splice(itemToMove.end(), BufferItems, iter);
                pMoveInv = &BufferItems;
                iterMoveIdx = MAXSIZE_T;
            }
            break;

        case d2ce::EnumItemLocation::EQUIPPED:
            // find out where it is equipped (i.e. Player, Player Cropse, Mercenary or Golem)
            iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(item));
            if (iter != Inventory.end())
            {
                itemInvType = d2ce::EnumItemInventory::PLAYER;

                // Check if we are moving inventories
                bool bMoving = false;
                switch (invType)
                {
                case d2ce::EnumItemInventory::UNKNOWN:
                case d2ce::EnumItemInventory::SHARED_STASH:
                    return false; // invalid

                case d2ce::EnumItemInventory::PLAYER:
                    break;

                default:
                    // we are moving
                    bMoving = true;
                    break;
                }

                if (bMoving)
                {
                    // moving to another inventory
                    d2ce::removeItem_if(ItemLocationReference[itemLocationId][itemAltPositionId], ItemPredicate(item));
                    pMoveInv = &Inventory;
                    iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                    itemToMove.splice(itemToMove.end(), Inventory, iter);
                }
            }
            else
            {
                iter = std::find_if(CorpseItems.begin(), CorpseItems.end(), ItemPredicate(item));
                if (iter != CorpseItems.end())
                {
                    // Player Corpse inventory
                    itemInvType = d2ce::EnumItemInventory::CORPSE;

                    // Check if we are moving inventories
                    bool bMoving = false;
                    switch (locationId)
                    {
                    case d2ce::EnumItemLocation::STORED:
                    case d2ce::EnumItemLocation::BELT:
                    case d2ce::EnumItemLocation::BUFFER:
                        // we are moving
                        bMoving = true;
                        break;

                    default:
                        return false;
                    }

                    if (bMoving)
                    {
                        pMoveInv = &CorpseItems;
                        iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                        itemToMove.splice(itemToMove.end(), CorpseItems, iter);
                    }
                }

                if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
                {
                    iter = std::find_if(MercItems.begin(), MercItems.end(), ItemPredicate(item));
                    if (iter != MercItems.end())
                    {
                        // Mercenary inventory
                        itemInvType = d2ce::EnumItemInventory::MERCENARY;

                        // Check if we are moving inventories
                        bool bMoving = false;
                        switch (locationId)
                        {
                        case d2ce::EnumItemLocation::STORED:
                        case d2ce::EnumItemLocation::BELT:
                        case d2ce::EnumItemLocation::BUFFER:
                            // we are moving
                            bMoving = true;
                            break;

                        default:
                            return false;
                        }

                        if (bMoving)
                        {
                            pMoveInv = &MercItems;
                            iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                            itemToMove.splice(itemToMove.end(), MercItems, iter);
                        }
                    }
                }

                if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
                {
                    iter = std::find_if(GolemItem.begin(), GolemItem.end(), ItemPredicate(item));
                    if (iter != GolemItem.end())
                    {
                        // Player Corpse inventory
                        itemInvType = d2ce::EnumItemInventory::GOLEM;

                        // Check if we are moving inventories
                        switch (locationId)
                        {
                        case d2ce::EnumItemLocation::STORED:
                        case d2ce::EnumItemLocation::BELT:
                        case d2ce::EnumItemLocation::BUFFER:
                            break;

                        default:
                            return false;
                        }

                        itemToMove.splice(itemToMove.end(), GolemItem, iter);
                        pMoveInv = &GolemItem;
                        iterMoveIdx = MAXSIZE_T;
                    }
                }

                if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
                {
                    iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
                    if (iter != BufferItems.end())
                    {
                        // Buffer item
                        itemInvType = d2ce::EnumItemInventory::BUFFER;

                        // Check if we are moving inventories
                        switch (locationId)
                        {
                        case d2ce::EnumItemLocation::STORED:
                        case d2ce::EnumItemLocation::BELT:
                            break;

                        case d2ce::EnumItemLocation::BUFFER:
                            LastItemMoved = nullptr;
                            LastItemIdx = MAXSIZE_T;
                            return true;

                        default:
                            return false;
                        }

                        itemToMove.splice(itemToMove.end(), BufferItems, iter);
                        pMoveInv = &BufferItems;
                        iterMoveIdx = MAXSIZE_T;
                    }
                }

                if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
                {
                    // failed to find item to move
                    return false;
                }
            }
            break;

        case d2ce::EnumItemLocation::BUFFER:
            iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
            if (iter != BufferItems.end())
            {
                // Buffer item
                itemInvType = d2ce::EnumItemInventory::BUFFER;

                // Check if we are moving inventories
                switch (locationId)
                {
                case d2ce::EnumItemLocation::STORED:
                case d2ce::EnumItemLocation::BELT:
                    break;

                case d2ce::EnumItemLocation::BUFFER:
                    LastItemMoved = nullptr;
                    LastItemIdx = MAXSIZE_T;
                    return true;

                default:
                    return false;
                }

                itemToMove.splice(itemToMove.end(), BufferItems, iter);
                pMoveInv = &BufferItems;
                iterMoveIdx = MAXSIZE_T;
            }
            else
            {
                // failed to find item to move
                return false;
            }
            break;

        default:
            // failed to find item to move
            return false;
        }
    }

    if (itemInvType == EnumItemInventory::UNKNOWN)
    {
        // failed to find item to move
        return false;
    }

    auto iter = itemToMove.begin();
    switch (invType)
    {
    case EnumItemInventory::BUFFER:
        // just place item in the BUFFER as is
        if (!itemToMove.empty())
        {
            iter = itemToMove.begin();
            iter->setLocation(locationId, altPositionId, positionX, positionY);
            BufferItems.splice(BufferItems.end(), itemToMove, iter);
        }
        LastItemMoved = nullptr;
        LastItemIdx = MAXSIZE_T;
        if (bCheckEquippedBelt)
        {
            verifyBeltSlots();
        }

        if (BufferItems.back().isRestrictedItem())
        {
            // we just place a restricted item in our inventory
            verifyRestrictedItems();
        }
        return true;
    }

    // Item is not equipped, but in an inventory grid
    auto& emptySlots = ItemLocationEmptySpots[locationId][altPositionId];
    auto& invLocationReference = ItemLocationReference[locationId][altPositionId];
    auto& invDimenstion = invDimensions[locationId][altPositionId];
    std::uint16_t positionX2 = positionX + dimensions.Width - 1;
    std::uint16_t positionY2 = positionY + dimensions.Height - 1;
    if (locationId == EnumItemLocation::BELT)
    {
        if (positionX >= invDimenstion.Width * invDimenstion.Height)
        {
            // out of bounds
            if (!itemToMove.empty())
            {
                if (pMoveInv != nullptr)
                {
                    auto iterMoveInv = pMoveInv->end();
                    if (iterMoveIdx != MAXSIZE_T)
                    {
                        iterMoveInv = pMoveInv->begin();
                        std::advance(iterMoveInv, iterMoveIdx);
                    }
                    pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                    pMoveInv = nullptr;
                }
            }

            // mark item slots as occupied again
            for (std::uint16_t y = itemPositionY; y <= itemPositionY2; ++y)
            {
                for (std::uint16_t x = itemPositionX; x <= itemPositionX2; ++x)
                {
                    itemEmptySlots.erase(y * itemInvDimenstion.Width + x);
                }
            }

            return false;
        }
    }
    else if ((positionX2 >= invDimenstion.Width) || (positionY2 >= invDimenstion.Height))
    {
        // out of bounds
        if (!itemToMove.empty())
        {
            if (pMoveInv != nullptr)
            {
                auto iterMoveInv = pMoveInv->end();
                if (iterMoveIdx != MAXSIZE_T)
                {
                    iterMoveInv = pMoveInv->begin();
                    std::advance(iterMoveInv, iterMoveIdx);
                }
                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                pMoveInv = nullptr;
            }
        }

        // mark item slots as occupied again
        for (std::uint16_t y = itemPositionY; y <= itemPositionY2; ++y)
        {
            for (std::uint16_t x = itemPositionX; x <= itemPositionX2; ++x)
            {
                itemEmptySlots.erase(y * itemInvDimenstion.Width + x);
            }
        }

        return false;
    }

    const d2ce::Item* pItem2 = nullptr;
    size_t item2Idx = MAXSIZE_T;
    std::uint16_t invItemPosX = 0;
    std::uint16_t invItemPosX2 = 0;
    std::uint16_t invItemPosY = 0;
    std::uint16_t invItemPosY2 = 0;
    ItemDimensions invItemDimensions;
    for (auto& itemRef : invLocationReference)
    {
        auto& invItem = itemRef.get();
        if (&invItem == &item)
        {
            // don't bother checking yourself
            continue;
        }

        if (!invItem.getDimensions(invItemDimensions))
        {
            continue;
        }

        invItemPosX = invItem.getPositionX();
        if (locationId == EnumItemLocation::BELT)
        {
            if (invItemPosX != positionX)
            {
                // does not intersect
                continue;
            }
        }
        else
        {
            invItemPosY = invItem.getPositionY();
            invItemPosX2 = invItemPosX + invItemDimensions.Width - 1;
            invItemPosY2 = invItemPosY + invItemDimensions.Height - 1;
            invItemPosY = invItem.getPositionY();
            if (positionX > invItemPosX2 || positionX2 < invItemPosX)
            {
                // does not intersect
                continue;
            }

            if (positionY > invItemPosY2 || positionY2 < invItemPosY)
            {
                // does not intersect
                continue;
            }
        }

        if (pItem2 != nullptr)
        {
            // to many items overlap
            if (!itemToMove.empty())
            {
                if (pMoveInv != nullptr)
                {
                    auto iterMoveInv = pMoveInv->end();
                    if (iterMoveIdx != MAXSIZE_T)
                    {
                        iterMoveInv = pMoveInv->begin();
                        std::advance(iterMoveInv, iterMoveIdx);
                    }
                    pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                    pMoveInv = nullptr;
                }
            }

            return false;
        }

        pItem2 = &invItem;
    }

    if (pItem2 != nullptr)
    {
        // can we socket the item?
        if (item.isSocketFiller() && pItem2->canSocketItem(item))
        {
            if (itemToMove.empty())
            {
                // we need to move the socketed item
                iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(item));
                if (iter != Inventory.end())
                {
                    pMoveInv = &Inventory;
                    iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                    itemToMove.splice(itemToMove.end(), Inventory, iter);
                    d2ce::removeItem_if(ItemLocationReference[item.getLocation()][item.getAltPositionId()], ItemPredicate(item));
                    d2ce::removeItem_if(GPSs, ItemPredicate(item));
                }
            }

            if (!itemToMove.empty())
            {
                // try to socket item
                auto& itemToSocket = *const_cast<d2ce::Item*>(pItem2);
                auto& socketedItems = itemToSocket.SocketedItems;
                if (item.setLocation(d2ce::EnumItemLocation::SOCKET, std::uint16_t(socketedItems.size()), 0ui16))
                {
                    socketedItems.splice(socketedItems.end(), itemToMove, itemToMove.begin());
                    itemToSocket.updateSocketedItemCount();
                    itemToSocket.cachedCombinedMagicalAttributes.clear();

                    // resolve magical properties of socketed gem
                    const auto& childItemType = item.getItemTypeHelper();
                    std::vector<MagicalAttribute> attribs;
                    childItemType.getSocketedMagicalAttributes(item, item.socketedMagicalAttributes, itemToSocket.getGemApplyType());

                    if (item.isRune())
                    {
                        itemToSocket.verifyRuneword();
                    }
                    LastItemMoved = nullptr;
                    LastItemIdx = MAXSIZE_T;
                    return true;
                }

                // fall through to just move the item
            }
        }

        // we have found an item to remove
        iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(*pItem2));
        if (iter == Inventory.end())
        {
            // failed to find item, should not happen!
            if (!itemToMove.empty())
            {
                if (pMoveInv != nullptr)
                {
                    auto iterMoveInv = pMoveInv->end();
                    if (iterMoveIdx != MAXSIZE_T)
                    {
                        iterMoveInv = pMoveInv->begin();
                        std::advance(iterMoveInv, iterMoveIdx);
                    }
                    pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                    pMoveInv = nullptr;
                }
            }

            return false;
        }

        item2Idx = std::distance(Inventory.begin(), iter);
        if (itemToMove.empty())
        {
            // mark item slots as empty again
            for (std::uint16_t y = itemPositionY; y <= itemPositionY2; ++y)
            {
                for (std::uint16_t x = itemPositionX; x <= itemPositionX2; ++x)
                {
                    itemEmptySlots.insert(y * itemInvDimenstion.Width + x);
                }
            }
        }

        if (!item.setLocation(locationId, altPositionId, positionX, positionY))
        {
            // failed move item
            if (itemToMove.empty())
            {
                // mark item slots as occupied again
                for (std::uint16_t y = itemPositionY; y <= itemPositionY2; ++y)
                {
                    for (std::uint16_t x = itemPositionX; x <= itemPositionX2; ++x)
                    {
                        itemEmptySlots.erase(y * itemInvDimenstion.Width + x);
                    }
                }
            }
            else if (pMoveInv != nullptr)
            {
                auto iterMoveInv = pMoveInv->end();
                if (iterMoveIdx != MAXSIZE_T)
                {
                    iterMoveInv = pMoveInv->begin();
                    std::advance(iterMoveInv, iterMoveIdx);
                }
                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                pMoveInv = nullptr;
            }

            return false;
        }

        if (item.isRestrictedItem())
        {
            // we just place a restricted item in our inventory
            verifyRestrictedItems();
        }

        d2ce::removeItem_if(invLocationReference, ItemPredicate(*pItem2));

        // mark item slots as empty again
        pItem2->getDimensions(invItemDimensions);
        invItemPosX = pItem2->getPositionX();
        invItemPosX2 = invItemPosX + invItemDimensions.Width - 1;
        invItemPosY = pItem2->getPositionY();
        invItemPosY2 = invItemPosY + invItemDimensions.Height - 1;
        for (std::uint16_t y = invItemPosY; y <= invItemPosY2; ++y)
        {
            for (std::uint16_t x = invItemPosX; x <= invItemPosX2; ++x)
            {
                itemEmptySlots.insert(y * invDimenstion.Width + x);
            }
        }

        BufferItems.splice(BufferItems.end(), Inventory, iter);
        pRemovedItem = pItem2;
        if (bCheckEquippedBelt)
        {
            verifyBeltSlots();
        }
    }
    else
    {
        if (itemToMove.empty())
        {
            // We are not moving ourself to a different repo, nor do we overlap with anyone.
            // Did we move at all?
            if ((locationId == itemLocationId) && (altPositionId == itemAltPositionId) &&
                (positionX == itemPositionX) && (positionY == itemPositionY))
            {
                // nothing to do, we did not move
                return true;
            }

            // mark item slots as empty again
            for (std::uint16_t y = itemPositionY; y <= itemPositionY2; ++y)
            {
                for (std::uint16_t x = itemPositionX; x <= itemPositionX2; ++x)
                {
                    itemEmptySlots.insert(y * itemInvDimenstion.Width + x);
                }
            }
        }

        if (!item.setLocation(locationId, altPositionId, positionX, positionY))
        {
            // failed move item
            if (itemToMove.empty())
            {
                // mark item slots as occupied again
                for (std::uint16_t y = itemPositionY; y <= itemPositionY2; ++y)
                {
                    for (std::uint16_t x = itemPositionX; x <= itemPositionX2; ++x)
                    {
                        itemEmptySlots.erase(y * itemInvDimenstion.Width + x);
                    }
                }
            }
            else if (pMoveInv != nullptr)
            {
                auto iterMoveInv = pMoveInv->end();
                if (iterMoveIdx != MAXSIZE_T)
                {
                    iterMoveInv = pMoveInv->begin();
                    std::advance(iterMoveInv, iterMoveIdx);
                }
                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                pMoveInv = nullptr;
            }

            return false;
        }
    }

    if (itemToMove.empty())
    {
        // make sure we have consistency in the reference cache
        if (std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemPredicate(item)) == invLocationReference.end())
        {
            invLocationReference.push_back(item);
        }
    }
    else
    {
        auto insertIter = Inventory.end();
        if ((LastItemMoved == &item) && (LastItemIdx != MAXSIZE_T))
        {
            insertIter = Inventory.begin();
            std::advance(insertIter, LastItemIdx);
            LastItemMoved = nullptr;
            LastItemIdx = MAXSIZE_T;
        }

        iter = itemToMove.begin();
        Inventory.splice(insertIter, itemToMove, iter);
        if (std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemPredicate(*iter)) == invLocationReference.end())
        {
            invLocationReference.push_back(*iter);
        }

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
        else if (itemType.isGPSItem())
        {
            GPSs.push_back(item);
        }
    }

    // remove slots from empty list
    for (std::uint16_t y = positionY; y <= positionY2; ++y)
    {
        for (std::uint16_t x = positionX; x <= positionX2; ++x)
        {
            emptySlots.erase(y * invDimenstion.Width + x);
        }
    }

    LastItemMoved = pRemovedItem;
    LastItemIdx = item2Idx;
    if (bCheckEquippedBelt)
    {
        verifyBeltSlots();
    }

    if (item.isRestrictedItem())
    {
        // we just place a restricted item in our inventory
        verifyRestrictedItems();
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::setItemLocation(d2ce::Item& item, EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    switch (locationId)
    {
    case EnumItemLocation::BELT:
    case EnumItemLocation::BUFFER:
        break;

    case EnumItemLocation::STORED:
    case EnumItemLocation::EQUIPPED:
    default:
        // invalid state
        return false;
    }

    switch (invType)
    {
    case d2ce::EnumItemInventory::PLAYER:
    case d2ce::EnumItemInventory::BUFFER:
        break;

    default:
        return false;
    }

    return setItemLocation(item, locationId, EnumAltItemLocation::UNKNOWN, positionX, positionY, invType, pRemovedItem);
}
//---------------------------------------------------------------------------
bool d2ce::Items::setItemLocation(d2ce::Item& item, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    switch (altPositionId)
    {
    case EnumAltItemLocation::UNKNOWN:
        // invalid state
        return false;

    case EnumAltItemLocation::HORADRIC_CUBE:
        if (item.isHoradricCube())
        {
            // invalid state
            return false;
        }
        break;
    }

    switch (invType)
    {
    case d2ce::EnumItemInventory::PLAYER:
    case d2ce::EnumItemInventory::BUFFER:
        break;

    case d2ce::EnumItemInventory::SHARED_STASH:
        if (altPositionId != EnumAltItemLocation::STASH)
        {
            // invalid state
            return false;
        }
        break;

    default:
        return false;
    }

    return setItemLocation(item, EnumItemLocation::STORED, altPositionId, positionX, positionY, invType, pRemovedItem);
}
//---------------------------------------------------------------------------
bool d2ce::Items::setItemLocation(d2ce::Item& item, const d2ce::Character& charInfo, EnumEquippedId equippedId, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    if (equippedId == EnumEquippedId::NONE)
    {
        // invalid state
        return false;
    }

    bool bSocketItem = false;
    bool bCheckEquippedBelt = false;
    auto& invLocationReference = ItemLocationReference[d2ce::EnumItemLocation::EQUIPPED][d2ce::EnumAltItemLocation::UNKNOWN];
    auto iterEquipped = invLocationReference.end();
    auto iterMercEquipped = MercItems.end();
    switch (invType)
    {
    case d2ce::EnumItemInventory::PLAYER:
        if (!item.canEquip(equippedId, charInfo.getClass()))
        {
            // can we socket the item?
            if (!item.isSocketFiller())
            {
                return false;
            }

            iterEquipped = std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemEnumEquippedIdPredicate(equippedId));
            if (iterEquipped == invLocationReference.end())
            {
                // nothing to socket
                return false;
            }

            if (!iterEquipped->get().canSocketItem(item, charInfo.getLevel()))
            {
                // can't socket
                return false;
            }

            bSocketItem = true;
        }

        if (!bSocketItem)
        {
            switch (equippedId)
            {
            case EnumEquippedId::LEFT_ARM:
                // find right arm
                iterEquipped = std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::RIGHT_ARM));
                if (iterEquipped != invLocationReference.end())
                {
                    if (!iterEquipped->get().canEquipWith(item, charInfo.getClass()))
                    {
                        return false;
                    }
                }
                break;

            case EnumEquippedId::ALT_LEFT_ARM:
                // find right arm
                iterEquipped = std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::ALT_RIGHT_ARM));
                if (iterEquipped != invLocationReference.end())
                {
                    if (!iterEquipped->get().canEquipWith(item, charInfo.getClass()))
                    {
                        return false;
                    }
                }
                break;

            case EnumEquippedId::RIGHT_ARM:
                // find left arm
                iterEquipped = std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::LEFT_ARM));
                if (iterEquipped != invLocationReference.end())
                {
                    if (!iterEquipped->get().canEquipWith(item, charInfo.getClass()))
                    {
                        return false;
                    }
                }
                break;

            case EnumEquippedId::ALT_RIGHT_ARM:
                // find left arm
                iterEquipped = std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::ALT_LEFT_ARM));
                if (iterEquipped != invLocationReference.end())
                {
                    if (!iterEquipped->get().canEquipWith(item, charInfo.getClass()))
                    {
                        return false;
                    }
                }
                break;

            case EnumEquippedId::BELT:
                bCheckEquippedBelt = true;
                break;
            }
        }
        break;

    case d2ce::EnumItemInventory::CORPSE: // can only drag out of corpse and Golem
    case d2ce::EnumItemInventory::GOLEM:
        return false;

    case d2ce::EnumItemInventory::BUFFER:
        break;

    case d2ce::EnumItemInventory::MERCENARY:
        if (!isExpansionItems() || (Version < EnumItemVersion::v109))
        {
            // invalid state
            return false;
        }

        if (!charInfo.hasMercenary())
        {
            return false;
        }

        if (!charInfo.getMercenaryInfo().canEquipItem(item, equippedId))
        {
            // can we socket the item?
            if (!item.isSocketFiller())
            {
                return false;
            }

            iterMercEquipped = std::find_if(MercItems.begin(), MercItems.end(), ItemEnumEquippedIdPredicate(equippedId));
            if (iterMercEquipped == MercItems.end())
            {
                // nothing to socket
                return false;
            }

            if (!iterMercEquipped->canSocketItem(item, charInfo.getMercenaryInfo().getLevel()))
            {
                // can't socket
                return false;
            }

            bSocketItem = true;
        }
        else
        {
            equippedId = charInfo.getMercenaryInfo().verifyEquippedId(item, equippedId);
        }
        break;

    case d2ce::EnumItemInventory::SHARED_STASH:
    default:
        // invalid state
        return false;
    }

    std::list<Item> removedItems; // items to be removed into the buffer
    std::array<std::uint8_t, 4> strcode;
    if (!item.getItemCode(strcode))
    {
        // should not happen
        return false;
    }

    const auto& itemType = ItemHelpers::getItemTypeHelper(strcode);
    std::map<d2ce::EnumItemLocation, std::map<d2ce::EnumAltItemLocation, ItemDimensions>> invDimensions;
    getItemLocationDimensions(d2ce::EnumAltItemLocation::INVENTORY, invDimensions[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::INVENTORY]);
    getItemLocationDimensions(d2ce::EnumAltItemLocation::HORADRIC_CUBE, invDimensions[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::HORADRIC_CUBE]);
    getItemLocationDimensions(d2ce::EnumAltItemLocation::STASH, invDimensions[d2ce::EnumItemLocation::STORED][d2ce::EnumAltItemLocation::STASH]);
    getItemLocationDimensions(d2ce::EnumItemLocation::BELT, invDimensions[d2ce::EnumItemLocation::BELT][d2ce::EnumAltItemLocation::UNKNOWN]);

    // Detect if is item being moved between inventory types
    ItemDimensions dimensions;
    if (!item.getDimensions(dimensions))
    {
        // should not happen
        return false;
    }

    d2ce::EnumItemInventory itemInvType = d2ce::EnumItemInventory::UNKNOWN;
    auto itemLocationId = item.getLocation();
    auto itemAltPositionId = (itemLocationId == d2ce::EnumItemLocation::STORED) ? item.getAltPositionId() : d2ce::EnumAltItemLocation::UNKNOWN;
    std::list<Item> itemToMove; // non-empty if given item is moving inventories
    std::list<d2ce::Item>* pMoveInv = nullptr;
    size_t iterMoveIdx = MAXSIZE_T;
    auto& itemInvDimenstion = invDimensions[itemLocationId][itemAltPositionId];
    auto& itemEmptySlots = ItemLocationEmptySpots[itemLocationId][itemAltPositionId];
    std::uint16_t itemPositionX = item.getPositionX();
    std::uint16_t itemPositionY = item.getPositionY();
    std::uint16_t itemPositionX2 = itemPositionX + dimensions.Width - 1;
    std::uint16_t itemPositionY2 = itemPositionY + dimensions.Height - 1;
    // item is moving between containers
    std::list<Item>::iterator iter;
    switch (itemLocationId)
    {
    case d2ce::EnumItemLocation::STORED:
    case d2ce::EnumItemLocation::BELT:
        // Find item and remove references and mark occupied slots as empty again
        iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(item));
        if (iter != Inventory.end())
        {
            d2ce::removeItem_if(ItemLocationReference[itemLocationId][itemAltPositionId], ItemPredicate(item));
            if (itemLocationId == EnumItemLocation::BELT)
            {
                itemInvType = d2ce::EnumItemInventory::PLAYER;
            }
            else
            {
                switch (invType)
                {
                case d2ce::EnumItemInventory::SHARED_STASH:
                case d2ce::EnumItemInventory::PLAYER:
                    itemInvType = invType;
                    break;

                default:
                    itemInvType = d2ce::EnumItemInventory::PLAYER;
                    break;
                }
            }

            // mark item slots as empty again
            for (std::uint16_t y = itemPositionY; y <= itemPositionY2; ++y)
            {
                for (std::uint16_t x = itemPositionX; x <= itemPositionX2; ++x)
                {
                    itemEmptySlots.insert(y * itemInvDimenstion.Width + x);
                }
            }

            // Check if we are moving inventories
            bool bMoving = false;
            if (bSocketItem)
            {
                bMoving = true;
            }
            else
            {
                switch (invType)
                {
                case d2ce::EnumItemInventory::UNKNOWN:
                    return false; // invalid

                case d2ce::EnumItemInventory::PLAYER:
                case d2ce::EnumItemInventory::SHARED_STASH:
                    break;

                default:
                    // we are moving
                    bMoving = true;
                    break;
                }
            }

            if (bMoving)
            {
                pMoveInv = &Inventory;
                iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                itemToMove.splice(itemToMove.end(), Inventory, iter);

                if (itemType.isStackable())
                {
                    d2ce::removeItem_if(Stackables, ItemPredicate(item));

                    if (itemType.isWeapon())
                    {
                        d2ce::removeItem_if(Weapons, ItemPredicate(item));
                    }
                }
                else if (itemType.isWeapon())
                {
                    d2ce::removeItem_if(Weapons, ItemPredicate(item));
                }
                else if (itemType.isArmor())
                {
                    d2ce::removeItem_if(Armor, ItemPredicate(item));
                }
                else if (itemType.isGPSItem())
                {
                    d2ce::removeItem_if(GPSs, ItemPredicate(item));
                }
            }
        }
        else
        {
            iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
            if (iter != BufferItems.end())
            {
                // Buffer item
                itemInvType = d2ce::EnumItemInventory::BUFFER;

                // Check if we are moving inventories
                if (invType == EnumItemInventory::BUFFER)
                {
                    LastItemMoved = nullptr;
                    LastItemIdx = MAXSIZE_T;
                    return true;
                }

                switch (invType)
                {
                case d2ce::EnumItemInventory::UNKNOWN:
                    return false; // invalid
                }

                itemToMove.splice(itemToMove.end(), BufferItems, iter);
                pMoveInv = &BufferItems;
                iterMoveIdx = MAXSIZE_T;
            }
            else
            {
                // can't find item!
                return false;
            }
        }
        break;

    case d2ce::EnumItemLocation::EQUIPPED:
        // find out where it is equipped (i.e. Player, Player Cropse, Mercenary or Golem)
        iter = std::find_if(Inventory.begin(), Inventory.end(), ItemPredicate(item));
        if (iter != Inventory.end())
        {
            // Still in same main inventory but moving in references
            itemInvType = d2ce::EnumItemInventory::PLAYER;
            d2ce::removeItem_if(ItemLocationReference[itemLocationId][itemAltPositionId], ItemPredicate(item));

            // Check if we are moving inventories
            bool bMoving = false;
            if (bSocketItem)
            {
                bMoving = true;
            }
            else
            {
                switch (invType)
                {
                case d2ce::EnumItemInventory::UNKNOWN:
                    return false; // invalid

                case d2ce::EnumItemInventory::PLAYER:
                case d2ce::EnumItemInventory::SHARED_STASH:
                    break;

                default:
                    // we are moving
                    bMoving = true;
                    break;
                }
            }

            if (bMoving)
            {
                // moving to another inventory
                pMoveInv = &Inventory;
                iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                itemToMove.splice(itemToMove.end(), Inventory, iter);

                if (itemType.isStackable())
                {
                    d2ce::removeItem_if(Stackables, ItemPredicate(item));

                    if (itemType.isWeapon())
                    {
                        d2ce::removeItem_if(Weapons, ItemPredicate(item));
                    }
                }
                else if (itemType.isWeapon())
                {
                    d2ce::removeItem_if(Weapons, ItemPredicate(item));
                }
                else if (itemType.isArmor())
                {
                    d2ce::removeItem_if(Armor, ItemPredicate(item));
                }
                else if (itemType.isGPSItem())
                {
                    d2ce::removeItem_if(GPSs, ItemPredicate(item));
                }
            }
        }
        else
        {
            iter = std::find_if(CorpseItems.begin(), CorpseItems.end(), ItemPredicate(item));
            if (iter != CorpseItems.end())
            {
                // Player Corpse inventory
                itemInvType = d2ce::EnumItemInventory::CORPSE;

                // Check if we are moving inventories
                bool bMoving = false;
                switch (invType)
                {
                case d2ce::EnumItemInventory::UNKNOWN:
                    return false; // invalid

                case d2ce::EnumItemInventory::CORPSE:
                    break;

                default:
                    // we are moving
                    bMoving = true;
                    break;
                }

                if (bMoving)
                {
                    pMoveInv = &CorpseItems;
                    iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                    itemToMove.splice(itemToMove.end(), CorpseItems, iter);
                }
            }

            if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
            {
                iter = std::find_if(MercItems.begin(), MercItems.end(), ItemPredicate(item));
                if (iter != MercItems.end())
                {
                    // Mercenary inventory
                    itemInvType = d2ce::EnumItemInventory::MERCENARY;

                    // Check if we are moving inventories
                    bool bMoving = false;
                    switch (invType)
                    {
                    case d2ce::EnumItemInventory::UNKNOWN:
                        return false; // invalid

                    case d2ce::EnumItemInventory::MERCENARY:
                        break;

                    default:
                        // we are moving
                        bMoving = true;
                        break;
                    }

                    if (bMoving)
                    {
                        pMoveInv = &MercItems;
                        iterMoveIdx = std::distance(pMoveInv->begin(), iter);
                        itemToMove.splice(itemToMove.end(), MercItems, iter);
                    }
                }
            }

            if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
            {
                iter = std::find_if(GolemItem.begin(), GolemItem.end(), ItemPredicate(item));
                if (iter != GolemItem.end())
                {
                    // Player Corpse inventory
                    itemInvType = d2ce::EnumItemInventory::GOLEM;

                    // Check if we are moving inventories
                    switch (invType)
                    {
                    case d2ce::EnumItemInventory::UNKNOWN:
                        return false; // invalid

                    case d2ce::EnumItemInventory::GOLEM:
                        LastItemMoved = nullptr;
                        LastItemIdx = MAXSIZE_T;
                        return true; // there is only one GOLEM item
                    }

                    itemToMove.splice(itemToMove.end(), GolemItem, iter);
                    pMoveInv = &GolemItem;
                    iterMoveIdx = MAXSIZE_T;
                }
            }

            if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
            {
                iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
                if (iter != BufferItems.end())
                {
                    // Buffer item
                    itemInvType = d2ce::EnumItemInventory::BUFFER;

                    // Check if we are moving inventories
                    if (invType == EnumItemInventory::BUFFER)
                    {
                        LastItemMoved = nullptr;
                        LastItemIdx = MAXSIZE_T;
                        return true;
                    }
                    switch (invType)
                    {
                    case d2ce::EnumItemInventory::UNKNOWN:
                        return false; // invalid
                    }

                    itemToMove.splice(itemToMove.end(), BufferItems, iter);
                    pMoveInv = &BufferItems;
                    iterMoveIdx = MAXSIZE_T;
                }
            }

            if (itemInvType == d2ce::EnumItemInventory::UNKNOWN)
            {
                // failed to find item to move
                return false;
            }
        }
        break;

    case d2ce::EnumItemLocation::BUFFER:
        iter = std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item));
        if (iter != BufferItems.end())
        {
            // Buffer item
            itemInvType = d2ce::EnumItemInventory::BUFFER;

            // Check if we are moving inventories
            if (invType == EnumItemInventory::BUFFER)
            {
                LastItemMoved = nullptr;
                LastItemIdx = MAXSIZE_T;
                return true;
            }

            switch (invType)
            {
            case d2ce::EnumItemInventory::UNKNOWN:
                return false; // invalid
            }

            itemToMove.splice(itemToMove.end(), BufferItems, iter);
            pMoveInv = &BufferItems;
            iterMoveIdx = MAXSIZE_T;
        }
        break;
    }

    if (itemInvType == EnumItemInventory::UNKNOWN)
    {
        // failed to find item to move
        return false;
    }

    iter = itemToMove.begin();
    switch (invType)
    {
    case EnumItemInventory::BUFFER:
        // just place item in the BUFFER as is
        if (!itemToMove.empty())
        {
            iter = itemToMove.begin();
            BufferItems.splice(BufferItems.end(), itemToMove, iter);
        }
        LastItemMoved = nullptr;
        LastItemIdx = MAXSIZE_T;
        return true;

    case EnumItemInventory::CORPSE: // only allowed to remove items from corpse
        iter = std::find_if(CorpseItems.begin(), CorpseItems.end(), ItemEnumEquippedIdPredicate(equippedId));
        if (iter != CorpseItems.end())
        {
            if (&(*iter) == &item)
            {
                return true;
            }
        }

        if (!itemToMove.empty())
        {
            if (pMoveInv != nullptr)
            {
                auto iterMoveInv = pMoveInv->end();
                if (iterMoveIdx != MAXSIZE_T)
                {
                    iterMoveInv = pMoveInv->begin();
                    std::advance(iterMoveInv, iterMoveIdx);
                }
                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                pMoveInv = nullptr;
            }
        }
        return false;

    case EnumItemInventory::MERCENARY:
        iter = std::find_if(MercItems.begin(), MercItems.end(), ItemEnumEquippedIdPredicate(equippedId));
        if (iter != MercItems.end())
        {
            if (&(*iter) == &item)
            {
                // we didn't move
                return true;
            }

            // found existing item at location
            if (bSocketItem)
            {
                if (itemToMove.empty())
                {
                    return false;
                }

                auto& itemToSocket = *iter;
                auto& socketedItems = itemToSocket.SocketedItems;
                if (!item.setLocation(d2ce::EnumItemLocation::SOCKET, std::uint16_t(socketedItems.size()), 0ui16))
                {
                    if (pMoveInv != nullptr)
                    {
                        auto iterMoveInv = pMoveInv->end();
                        if (iterMoveIdx != MAXSIZE_T)
                        {
                            iterMoveInv = pMoveInv->begin();
                            std::advance(iterMoveInv, iterMoveIdx);
                        }
                        pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                        pMoveInv = nullptr;
                    }
                    return false;
                }

                socketedItems.splice(socketedItems.end(), itemToMove, itemToMove.begin());
                itemToSocket.updateSocketedItemCount();

                // resolve magical properties of socketed gem
                const auto& childItemType = item.getItemTypeHelper();
                std::vector<MagicalAttribute> attribs;
                childItemType.getSocketedMagicalAttributes(item, item.socketedMagicalAttributes, itemToSocket.getGemApplyType());

                if (item.isRune())
                {
                    itemToSocket.verifyRuneword();
                }
                LastItemMoved = nullptr;
                LastItemIdx = MAXSIZE_T;
                return true;
            }
            else if (iter->isTwoHandedWeapon())
            {
                BufferItems.splice(BufferItems.end(), MercItems, iter);
                pRemovedItem = &BufferItems.back();
            }
            else if (item.isTwoHandedWeapon())
            {
                switch (equippedId)
                {
                case EnumEquippedId::LEFT_ARM:
                case EnumEquippedId::ALT_LEFT_ARM:
                    if (std::find_if(MercItems.begin(), MercItems.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::RIGHT_ARM)) == MercItems.end())
                    {
                        BufferItems.splice(BufferItems.end(), MercItems, iter);
                        pRemovedItem = &BufferItems.back();
                    }
                    else
                    {
                        if (!itemToMove.empty())
                        {
                            if (pMoveInv != nullptr)
                            {
                                auto iterMoveInv = pMoveInv->end();
                                if (iterMoveIdx != MAXSIZE_T)
                                {
                                    iterMoveInv = pMoveInv->begin();
                                    std::advance(iterMoveInv, iterMoveIdx);
                                }
                                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                                pMoveInv = nullptr;
                            }
                        }
                        return false;
                    }
                    break;

                case EnumEquippedId::RIGHT_ARM:
                case EnumEquippedId::ALT_RIGHT_ARM:
                    if (std::find_if(MercItems.begin(), MercItems.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::LEFT_ARM)) == MercItems.end())
                    {
                        BufferItems.splice(BufferItems.end(), MercItems, iter);
                        pRemovedItem = &BufferItems.back();
                    }
                    else
                    {
                        if (!itemToMove.empty())
                        {
                            if (pMoveInv != nullptr)
                            {
                                auto iterMoveInv = pMoveInv->end();
                                if (iterMoveIdx != MAXSIZE_T)
                                {
                                    iterMoveInv = pMoveInv->begin();
                                    std::advance(iterMoveInv, iterMoveIdx);
                                }
                                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                                pMoveInv = nullptr;
                            }
                        }
                        return false;
                    }
                    break;
                }
            }
            else
            {
                BufferItems.splice(BufferItems.end(), MercItems, iter);
                pRemovedItem = &BufferItems.back();
            }
        }
        else if (item.isTwoHandedWeapon())
        {
            auto iterOtherHand = MercItems.end();
            switch (equippedId)
            {
            case EnumEquippedId::LEFT_ARM:
            case EnumEquippedId::ALT_LEFT_ARM:
                iterOtherHand = std::find_if(MercItems.begin(), MercItems.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::RIGHT_ARM));
                if (iterOtherHand != MercItems.end())
                {
                    BufferItems.splice(BufferItems.end(), MercItems, iterOtherHand);
                    pRemovedItem = &BufferItems.back();
                }
                break;

            case EnumEquippedId::RIGHT_ARM:
            case EnumEquippedId::ALT_RIGHT_ARM:
                iterOtherHand = std::find_if(MercItems.begin(), MercItems.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::LEFT_ARM));
                if (iterOtherHand != MercItems.end())
                {
                    BufferItems.splice(BufferItems.end(), MercItems, iterOtherHand);
                    pRemovedItem = &BufferItems.back();
                }
                break;
            }
        }

        item.setLocation(equippedId);
        if (!itemToMove.empty())
        {
            iter = itemToMove.begin();
            MercItems.splice(MercItems.end(), itemToMove, iter);
        }
        LastItemMoved = nullptr;
        LastItemIdx = MAXSIZE_T;
        return true;

    case EnumItemInventory::GOLEM:
        if (!itemToMove.empty() || GolemItem.empty())
        {
            // should not happend
            return false;
        }

        if (&(GolemItem.back()) == &item)
        {
            return true;
        }
        return false;

    case EnumItemInventory::PLAYER:
        iter = std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(equippedId));
        if (iter != Inventory.end())
        {
            if (&(*iter) == &item)
            {
                // we didn't move
                return true;
            }

            // found existing item at location
            if (bSocketItem)
            {
                if (itemToMove.empty())
                {
                    return false;
                }

                auto& itemToSocket = *iter;
                auto& socketedItems = itemToSocket.SocketedItems;
                if (!item.setLocation(d2ce::EnumItemLocation::SOCKET, std::uint16_t(socketedItems.size()), 0ui16))
                {
                    if (pMoveInv != nullptr)
                    {
                        auto iterMoveInv = pMoveInv->end();
                        if (iterMoveIdx != MAXSIZE_T)
                        {
                            iterMoveInv = pMoveInv->begin();
                            std::advance(iterMoveInv, iterMoveIdx);
                        }
                        pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                        pMoveInv = nullptr;
                    }
                    return false;
                }

                socketedItems.splice(socketedItems.end(), itemToMove, itemToMove.begin());
                itemToSocket.updateSocketedItemCount();

                // resolve magical properties of socketed gem
                const auto& childItemType = item.getItemTypeHelper();
                std::vector<MagicalAttribute> attribs;
                childItemType.getSocketedMagicalAttributes(item, item.socketedMagicalAttributes, itemToSocket.getGemApplyType());

                if (item.isRune())
                {
                    itemToSocket.verifyRuneword();
                }
                LastItemMoved = nullptr;
                LastItemIdx = MAXSIZE_T;
                return true;
            }
            if (iter->isTwoHandedWeapon())
            {
                BufferItems.splice(BufferItems.end(), Inventory, iter);
                d2ce::removeItem_if(invLocationReference, ItemPredicate(*iter));
                pRemovedItem = &BufferItems.back();
            }
            else if (item.isTwoHandedWeapon())
            {
                switch (equippedId)
                {
                case EnumEquippedId::LEFT_ARM:
                    if (std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::RIGHT_ARM)) == Inventory.end())
                    {
                        BufferItems.splice(BufferItems.end(), Inventory, iter);
                        d2ce::removeItem_if(invLocationReference, ItemPredicate(*iter));
                        pRemovedItem = &BufferItems.back();
                    }
                    else
                    {
                        if (!itemToMove.empty())
                        {
                            if (pMoveInv != nullptr)
                            {
                                auto iterMoveInv = pMoveInv->end();
                                if (iterMoveIdx != MAXSIZE_T)
                                {
                                    iterMoveInv = pMoveInv->begin();
                                    std::advance(iterMoveInv, iterMoveIdx);
                                }
                                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                                pMoveInv = nullptr;
                            }
                        }
                        return false;
                    }
                    break;

                case EnumEquippedId::ALT_LEFT_ARM:
                    if (std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::ALT_RIGHT_ARM)) == Inventory.end())
                    {
                        BufferItems.splice(BufferItems.end(), Inventory, iter);
                        d2ce::removeItem_if(invLocationReference, ItemPredicate(*iter));
                        pRemovedItem = &BufferItems.back();
                    }
                    else
                    {
                        if (!itemToMove.empty())
                        {
                            if (pMoveInv != nullptr)
                            {
                                auto iterMoveInv = pMoveInv->end();
                                if (iterMoveIdx != MAXSIZE_T)
                                {
                                    iterMoveInv = pMoveInv->begin();
                                    std::advance(iterMoveInv, iterMoveIdx);
                                }
                                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                                pMoveInv = nullptr;
                            }
                        }
                        return false;
                    }
                    break;

                case EnumEquippedId::RIGHT_ARM:
                    if (std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::LEFT_ARM)) == Inventory.end())
                    {
                        BufferItems.splice(BufferItems.end(), Inventory, iter);
                        d2ce::removeItem_if(invLocationReference, ItemPredicate(*iter));
                        pRemovedItem = &BufferItems.back();
                    }
                    else
                    {
                        if (!itemToMove.empty())
                        {
                            if (pMoveInv != nullptr)
                            {
                                auto iterMoveInv = pMoveInv->end();
                                if (iterMoveIdx != MAXSIZE_T)
                                {
                                    iterMoveInv = pMoveInv->begin();
                                    std::advance(iterMoveInv, iterMoveIdx);
                                }
                                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                                pMoveInv = nullptr;
                            }
                        }
                        return false;
                    }
                    break;

                case EnumEquippedId::ALT_RIGHT_ARM:
                    if (std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::ALT_LEFT_ARM)) == Inventory.end())
                    {
                        BufferItems.splice(BufferItems.end(), Inventory, iter);
                        d2ce::removeItem_if(invLocationReference, ItemPredicate(*iter));
                        pRemovedItem = &BufferItems.back();
                    }
                    else
                    {
                        if (!itemToMove.empty())
                        {
                            if (pMoveInv != nullptr)
                            {
                                auto iterMoveInv = pMoveInv->end();
                                if (iterMoveIdx != MAXSIZE_T)
                                {
                                    iterMoveInv = pMoveInv->begin();
                                    std::advance(iterMoveInv, iterMoveIdx);
                                }
                                pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
                                pMoveInv = nullptr;
                            }
                        }
                        return false;
                    }
                    break;
                }
            }
            else
            {
                BufferItems.splice(BufferItems.end(), Inventory, iter);
                d2ce::removeItem_if(invLocationReference, ItemPredicate(*iter));
                pRemovedItem = &BufferItems.back();
            }
        }
        else if (item.isTwoHandedWeapon())
        {
            auto iterOtherHand = Inventory.end();
            switch (equippedId)
            {
            case EnumEquippedId::LEFT_ARM:
                iterOtherHand = std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::RIGHT_ARM));
                if (iterOtherHand != Inventory.end())
                {
                    BufferItems.splice(BufferItems.end(), Inventory, iterOtherHand);
                    d2ce::removeItem_if(invLocationReference, ItemPredicate(*iterOtherHand));
                    pRemovedItem = &BufferItems.back();
                }
                break;

            case EnumEquippedId::ALT_LEFT_ARM:
                iterOtherHand = std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::ALT_RIGHT_ARM));
                if (iterOtherHand != Inventory.end())
                {
                    BufferItems.splice(BufferItems.end(), Inventory, iterOtherHand);
                    d2ce::removeItem_if(invLocationReference, ItemPredicate(*iterOtherHand));
                    pRemovedItem = &BufferItems.back();
                }
                break;

            case EnumEquippedId::RIGHT_ARM:
                iterOtherHand = std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::LEFT_ARM));
                if (iterOtherHand != Inventory.end())
                {
                    BufferItems.splice(BufferItems.end(), Inventory, iterOtherHand);
                    d2ce::removeItem_if(invLocationReference, ItemPredicate(*iterOtherHand));
                    pRemovedItem = &BufferItems.back();
                }
                break;

            case EnumEquippedId::ALT_RIGHT_ARM:
                iterOtherHand = std::find_if(Inventory.begin(), Inventory.end(), ItemEnumEquippedIdPredicate(EnumEquippedId::ALT_LEFT_ARM));
                if (iterOtherHand != Inventory.end())
                {
                    BufferItems.splice(BufferItems.end(), Inventory, iterOtherHand);
                    d2ce::removeItem_if(invLocationReference, ItemPredicate(*iterOtherHand));
                    pRemovedItem = &BufferItems.back();
                }
                break;
            }
        }

        item.setLocation(equippedId);
        if (itemToMove.empty())
        {
            if (std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemPredicate(item)) == invLocationReference.end())
            {
                invLocationReference.push_back(item);
            }
        }
        else
        {
            iter = itemToMove.begin();
            Inventory.splice(Inventory.end(), itemToMove, iter);
            if (std::find_if(invLocationReference.begin(), invLocationReference.end(), ItemPredicate(*iter)) == invLocationReference.end())
            {
                invLocationReference.push_back(*iter);
            }

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
            else if (itemType.isGPSItem())
            {
                GPSs.push_back(item);
            }
        }

        LastItemMoved = nullptr;
        LastItemIdx = MAXSIZE_T;

        if (bCheckEquippedBelt)
        {
            verifyBeltSlots();
        }
        return true;
    }

    if (!itemToMove.empty())
    {
        if (pMoveInv != nullptr)
        {
            auto iterMoveInv = pMoveInv->end();
            if (iterMoveIdx != MAXSIZE_T)
            {
                iterMoveInv = pMoveInv->begin();
                std::advance(iterMoveInv, iterMoveIdx);
            }
            pMoveInv->splice(iterMoveInv, itemToMove, itemToMove.begin());
            pMoveInv = nullptr;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Items::removeSocketedItems(d2ce::Item& item)
{
    if (item.SocketedItems.empty())
    {
        return true;
    }

    // radomize the Ids as item has changed
    item.randomizeId();

    // try to find spots for the items out of bounds still
    auto locationId = d2ce::EnumItemLocation::STORED;
    auto altPositionId = d2ce::EnumAltItemLocation::INVENTORY;
    auto* pInvEmptySlots = &ItemLocationEmptySpots[locationId][altPositionId];
    auto* pInvLocationReference = &ItemLocationReference[locationId][altPositionId];

    std::uint16_t posX = 0;
    ItemDimensions invDimensions;
    getItemLocationDimensions(locationId, altPositionId, invDimensions);
    while (!item.SocketedItems.empty() && (pInvEmptySlots != nullptr))
    {
        if (pInvEmptySlots->empty())
        {
            switch (altPositionId)
            {
            case d2ce::EnumAltItemLocation::INVENTORY:
                altPositionId = d2ce::EnumAltItemLocation::STASH;
                pInvEmptySlots = &ItemLocationEmptySpots[locationId][altPositionId];
                pInvLocationReference = &ItemLocationReference[locationId][altPositionId];
                getItemLocationDimensions(locationId, altPositionId, invDimensions);
                break;

            case d2ce::EnumAltItemLocation::STASH:
                altPositionId = d2ce::EnumAltItemLocation::HORADRIC_CUBE;
                pInvEmptySlots = &ItemLocationEmptySpots[locationId][altPositionId];
                pInvLocationReference = &ItemLocationReference[locationId][altPositionId];
                getItemLocationDimensions(locationId, altPositionId, invDimensions);
                break;

            case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
            default:
                pInvEmptySlots = nullptr;
                break;
            }
        }
        else
        {
            auto iterMove = item.SocketedItems.begin();
            auto& itemToMove = *iterMove;
            Inventory.splice(Inventory.end(), item.SocketedItems, iterMove);
            if (std::find_if(pInvLocationReference->begin(), pInvLocationReference->end(), ItemPredicate(itemToMove)) == pInvLocationReference->end())
            {
                pInvLocationReference->push_back(itemToMove);
            }

            const auto& itemType = itemToMove.getItemTypeHelper();
            if (itemType.isGPSItem())
            {
                GPSs.push_back(itemToMove);
            }

            posX = *(pInvEmptySlots->begin());
            if (itemToMove.setLocation(locationId, altPositionId, posX % invDimensions.Width, posX / invDimensions.Width))
            {
                pInvEmptySlots->erase(posX);
            }
        }
    }

    if (!item.SocketedItems.empty())
    {
        BufferItems.splice(BufferItems.end(), item.SocketedItems, item.SocketedItems.begin(), item.SocketedItems.end());
    }

    item.updateSocketedItemCount();
    item.verifyRuneword();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::setItemRuneword(d2ce::Item& item, std::uint16_t id)
{
    if (item.getSocketedItemCount() == 0)
    {
        return item.setRuneword(id);
    }

    RunewordAttributes runeAttrib;
    if (item.isRuneword())
    {
        if (item.getRunewordAttributes(runeAttrib))
        {
            if (id == runeAttrib.Id)
            {
                // we are still ok
                return true;
            }
        }
    }

    auto& runeword = ItemHelpers::getRunewordFromId(id);
    if (runeword.id != id)
    {
        return false;
    }

    auto runeSocketCount = std::uint8_t(runeword.runeCodes.size());
    auto numSockets = std::max(item.getMaxSocketCount(), item.getDisplayedSocketCount());
    if (numSockets < runeSocketCount)
    {
        // not enough sockets
        return false;
    }

    // remove any socketed items
    removeSocketedItems(item);
    return item.setRuneword(id);
}
//---------------------------------------------------------------------------
bool d2ce::Items::upgradeItemTier(d2ce::Item& item, const CharStats& cs)
{
    if (!item.upgradeTier(cs))
    {
        return false;
    }

    if (item.isBelt() && (item.getLocation() == EnumItemLocation::EQUIPPED))
    {
        verifyBeltSlots();
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Items::changeItemEthereal(d2ce::Item& item)
{
    if (item.isEthereal())
    {
        return item.removeEthereal();
    }

    return item.makeEthereal();
}
//---------------------------------------------------------------------------
bool d2ce::Items::getItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    attribs.clear();

    // Combine all equipped magical properties
    std::vector<MagicalAttribute> unfilteredAttribs;
    std::vector<MagicalAttribute> tempAttribs;
    std::multimap<size_t, size_t> itemIndexMap;
    for (auto& item : ItemLocationReference[EnumItemLocation::EQUIPPED][EnumAltItemLocation::UNKNOWN])
    {
        if (item.get().getCombinedMagicalAttributes(tempAttribs))
        {
            ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, unfilteredAttribs);
        }
    }

    // Combine all charm magical properties
    for (auto& item : ItemLocationReference[EnumItemLocation::STORED][EnumAltItemLocation::INVENTORY])
    {
        if (!item.get().isCharm())
        {
            continue;
        }

        if (item.get().getCombinedMagicalAttributes(tempAttribs))
        {
            ItemHelpers::combineMagicalAttribute(itemIndexMap, tempAttribs, unfilteredAttribs);
        }
    }

    // filter out the properties that give us a bonuses
    FilterMagicalAttribsForBonus(unfilteredAttribs, attribs);
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
    std::uint64_t eDef = 0;
    std::uint64_t defenseRating = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : ItemLocationReference[EnumItemLocation::EQUIPPED][EnumAltItemLocation::UNKNOWN])
    {
        defenseRating += item.get().getDefenseRating();
        if (item.get().getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
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

    for (auto& item : ItemLocationReference[EnumItemLocation::STORED][EnumAltItemLocation::INVENTORY])
    {
        if (!item.get().isCharm())
        {
            continue;
        }

        if (item.get().getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
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

    defenseRating += (defenseRating * eDef) / 100;
    return std::uint16_t(defenseRating);
}
//---------------------------------------------------------------------------
bool d2ce::Items::getCombinedDamage(BaseDamage& damage, std::uint32_t charLevel) const
{
    ItemDamage itemDamage;
    damage.clear();

    std::uint64_t eDmg = 0;
    std::uint64_t eDmgMax = 0;
    std::uint64_t dmgMin = 0;
    std::uint64_t dmgMax = 0;
    std::uint64_t dmgMin2 = 0;
    std::uint64_t dmgMax2 = 0;
    std::uint64_t dmgMinThrow = 0;
    std::uint64_t dmgMaxThrow = 0;
    std::vector<MagicalAttribute> magicalAttributes;
    for (auto& item : ItemLocationReference[EnumItemLocation::EQUIPPED][EnumAltItemLocation::UNKNOWN])
    {
        if (item.get().getDamage(itemDamage))
        {
            if (item.get().isEthereal())
            {
                itemDamage.OneHanded.Min += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Min) * 50) / 100);
                itemDamage.OneHanded.Max += std::uint16_t((std::uint64_t(itemDamage.OneHanded.Max) * 50) / 100);
            }
            damage.add(itemDamage.OneHanded);
        }

        if (item.get().getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
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

    for (auto& item : ItemLocationReference[EnumItemLocation::STORED][EnumAltItemLocation::INVENTORY])
    {
        if (!item.get().isCharm())
        {
            continue;
        }

        if (item.get().getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
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

    return damage.Max != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::int64_t d2ce::Items::getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx)
{
    const auto& stat = ItemHelpers::getItemStat(attrib);
    if (stat.id != attrib.Id)
    {
        return 0;
    }

    return ItemHelpers::getMagicalAttributeValue(attrib, charLevel, idx, stat);
}
//---------------------------------------------------------------------------