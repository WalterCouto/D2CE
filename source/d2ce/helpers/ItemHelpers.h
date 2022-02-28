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

#include "CharacterConstants.h"
#include "ItemConstants.h"
#include "DataTypes.h"
#include <json/json.h>

namespace d2ce
{
    class Item;

    //---------------------------------------------------------------------------
    struct OpAttributes
    {
        std::uint8_t op = 0;
        std::uint8_t op_param = 0;
        std::string op_base;
        std::vector<std::string> op_stats;
    };

    //---------------------------------------------------------------------------
    struct ItemStat
    {
        std::uint16_t id = 0;
        std::uint8_t encode = 0;
        std::uint16_t saveBits = 0;
        std::int16_t saveAdd = 0;
        std::uint16_t saveParamBits = 0;
        std::uint16_t nextInChain = 0;
        std::uint16_t charSaveBits = 0;
        std::string desc;
        std::string name;
        OpAttributes opAttribs;
    };

    struct ItemType
    {
        std::string name;
        ItemDamage dam;
        ItemRequirements req;
        ItemDimensions dimensions;
        bool stackable = 0;
        std::string inv_file;
        std::uint16_t inv_transform = 0;
        std::vector<std::string> categories;

        // Maximumn Sockets for item given its levels:
        // If vector is empty, then the item can't have sockets
        // Index 0 is for levels 1 to 25 (if no other elements in then vector then value applies to all item levels)
        // Index 1 is for levels 26 to 40 (if no other elements in the vector, then value applies to levels 26+)
        // Index 2 is for levels 41+
        std::vector<std::uint8_t> max_sockets;

        bool isStackable() const;
        bool isWeapon() const;
        bool isMissileWeapon() const;
        bool isTwoHandedWeapon() const;
        bool isShield() const;
        bool isArmor() const;
        bool isHelm() const;
        bool isBook() const;
        bool isPotion() const;
        bool isJewel() const;
        bool isGem() const;
        bool isQuestItem() const;
        bool isGoldItem() const;
        bool isRejuvenationPotion() const;
        bool isHealingPotion() const;
        bool isManaPotion() const;
        bool isSocketFiller() const;
        bool isUpgradableGem() const;
        bool isUpgradableRejuvenationPotion() const;
        bool isUpgradablePotion() const;
        bool isRune() const;
        bool isCharm() const;
        bool isBelt() const;
        bool isScroll() const;
        bool isSimpleItem() const;
        bool isUnusedItem() const;

        bool hasUndeadBonus() const;

        EnumItemType getEnumItemType() const;

        bool canHaveSockets() const;
        std::uint8_t getMaxSockets(std::uint8_t level) const;

        std::uint8_t getBaseType() const;

        bool getSocketedMagicalAttributes(const d2ce::Item& item, std::vector<MagicalAttribute>& attribs, EnumItemType parentItemType) const;

        std::uint16_t getPotionPoints(EnumCharClass charClass) const;
    };
    //---------------------------------------------------------------------------
    
    //---------------------------------------------------------------------------
    class ItemHelpers
    {
    public:
        static const d2ce::ItemStat& getItemStat(size_t idx);
        static size_t getItemStatSize();

        static std::uint16_t getTypeCodev100(const std::array<std::uint8_t, 4>& strcode);
        static std::uint8_t getMaxSocketsv100(const std::array<std::uint8_t, 4>& strcode);
        static bool getItemCodev100(std::uint16_t code, std::array<std::uint8_t, 4>& strcode);
        static std::uint8_t getItemCodev115(const std::vector<std::uint8_t>& data, size_t startOffset, std::array<std::uint8_t, 4>& strcode);
        static void d2ce::ItemHelpers::encodeItemCodev115(const std::array<std::uint8_t, 4>& strcode, std::uint64_t& encodedVal, std::uint8_t& numBitsSet);
        static std::uint8_t HuffmanDecodeBitString(const std::string& bitstr);
        static std::uint16_t getGPSSortIndex(const std::array<std::uint8_t, 4>& strcode);
        static void getValidGPSCodes(std::vector <std::string>& gpsCodes, bool isExpansion = true);

        static const ItemType& getItemTypeHelper(const std::array<std::uint8_t, 4>& strcode);
        static std::uint8_t getItemBase(const std::array<std::uint8_t, 4>& strcode);
        static EnumItemType getEnumItemTypeFromCode(std::array<std::uint8_t, 4>& strcode);

        static std::string getRunewordNameFromId(std::uint16_t id);
        static bool getSetMagicAttribsV100(std::uint16_t id, std::vector<MagicalAttribute>& attribs);
        static bool getUniqueMagicAttribsV100(std::uint16_t id, std::vector<MagicalAttribute>& attribs);
        static bool getUniqueQuestMagicAttribsV100(const std::array<std::uint8_t, 4>& strcode, std::vector<MagicalAttribute>& attribs);

        static std::uint16_t getSetIDv100(std::uint16_t id, const std::array<std::uint8_t, 4>& strcode);
        static std::uint16_t getSetCodev100(std::uint16_t id);
        static std::uint32_t generateSetDWBCodev100(std::uint16_t id, const std::array<std::uint8_t, 4>& strcode, std::uint32_t dwb, std::uint16_t magicLevel);
        static std::uint8_t generateInferiorQualityIdv100(std::uint32_t dwb, std::uint16_t magicLevel);
        static bool generateMagicalAffixesv100(std::uint16_t typeCode, std::uint32_t dwb, std::uint16_t magicLevel, MagicalCachev100& cache);
        static bool generateRareOrCraftedAffixesv100(std::uint16_t typeCode, std::uint32_t dwb, std::uint16_t magicLevel, RareOrCraftedCachev100& cache);
        static std::uint8_t generateDefenseRatingv100(const std::array<std::uint8_t, 4>& strcode, std::uint32_t dwa);
        static std::uint32_t generateDWARandomOffsetv100(std::uint32_t dwa, std::uint16_t numRndCalls);
        static std::uint32_t generarateRandomDW();
        static std::uint32_t generarateRandomMagicLevel();
        static std::string getSetNameFromId(std::uint16_t id);
        static std::string getSetTCFromId(std::uint16_t id);
        static std::uint16_t getSetLevelReqFromId(std::uint16_t id);
        static std::string getRareNameFromId(std::uint16_t id);
        static std::string getMagicalPrefixFromId(std::uint16_t id);
        static std::string getMagicalSuffixFromId(std::uint16_t id);
        static std::string getMagicalPrefixTCFromId(std::uint16_t id);
        static std::string getMagicalSuffixTCFromId(std::uint16_t id);
        static std::string getUniqueNameFromId(std::uint16_t id);
        static std::uint16_t getIdFromRareName(const std::string& rareName);
        static std::uint16_t getUniqueLevelReqFromId(std::uint16_t id);
        static std::string getUniqueTCFromId(std::uint16_t id);

        static bool magicalAttributeSorter(const MagicalAttribute& left, const MagicalAttribute& right);
        static void checkForRelatedMagicalAttributes(std::vector<MagicalAttribute>& attribs);
        static std::int64_t getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx, const ItemStat& stat);
        static std::string formatMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx, const ItemStat& stat);
        static bool formatDisplayedMagicalAttribute(MagicalAttribute& attrib, std::uint32_t charLevel);
        static void combineMagicalAttribute(std::multimap<size_t, size_t>& itemIndexMap, const std::vector<MagicalAttribute>& newAttribs, std::vector<MagicalAttribute>& attribs);
        static bool ProcessNameNode(const Json::Value& node, std::array<char, NAME_LENGTH>& name);
    };
    //---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------

