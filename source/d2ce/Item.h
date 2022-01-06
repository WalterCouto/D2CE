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
    //---------------------------------------------------------------------------
    class Item
    {
        friend class Items;

    private:
        mutable std::vector<std::uint8_t> data;
        EnumCharVersion FileVersion = APP_CHAR_VERSION;
        size_t start_bit_offset = 16;
        mutable size_t is_potion_bit_offset = 36;
        size_t location_bit_offset = 58;
        size_t equipped_id_offset = 61;
        size_t position_offset = 65;
        size_t alt_position_id_offset = 73;
        size_t type_code_offset = 76;
        size_t extended_data_offset = 108;
        mutable size_t quest_difficulty_offset = 0;
        mutable size_t nr_of_items_in_sockets_offset = 0;
        mutable size_t nr_of_items_in_sockets_bits = 3;
        mutable size_t item_id_bit_offset = 0;
        mutable size_t item_level_bit_offset = 0;
        mutable size_t quality_bit_offset = 0;
        mutable size_t multi_graphic_bit_offset = 0;
        mutable size_t autoAffix_bit_offset = 0;
        size_t quality_attrib_bit_offset = 0;
        size_t runeword_id_bit_offset = 0;
        size_t personalized_bit_offset = 0;
        size_t tome_bit_offset = 0;
        size_t realm_bit_offset = 0;
        size_t defense_rating_bit_offset = 0;
        size_t durability_bit_offset = 0;
        size_t stackable_bit_offset = 0;
        size_t gld_stackable_bit_offset = 0;
        size_t socket_count_bit_offset = 0;
        size_t bonus_bits_bit_offset = 0;
        size_t magical_props_bit_offset = 0;
        size_t set_bonus_props_bit_offset = 0;
        size_t runeword_props_bit_offset = 0;
        size_t item_end_bit_offset = 0;

    private:
        std::uint64_t readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits);
        bool skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits);
        bool setBits(size_t& current_bit_offset, size_t bits, std::uint32_t value);
        bool setBits64(size_t& current_bit_offset, size_t bits, std::uint64_t value);
        bool parsePropertyList(std::FILE* charfile, size_t& current_bit_offset);
        bool parsePropertyList(const Json::Value& propListRoot, bool bSerializedFormat, size_t& current_bit_offset);
        bool readPropertyList(size_t& current_bit_offset, std::vector<MagicalAttribute>& attrib) const;
        std::uint8_t getEncodedChar(std::FILE* charfile, size_t& current_bit_offset);

        bool updateBits(size_t start, size_t size, std::uint32_t value);
        bool updateBits64(size_t start, size_t size, std::uint64_t value);
        bool updateItemCodev115(std::uint64_t code, size_t numBitsSet);

    protected:
        bool readItem(EnumCharVersion version, std::FILE* charfile);
        bool readItem(const Json::Value& itemRoot, bool bSerializedFormat, EnumCharVersion version);
        bool writeItem(std::FILE* charfile);

        void asJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        void unknownAsJson(Json::Value& parent, bool bSerializedFormat = false) const;
        void byteRangeAsJson(Json::Value& parent, size_t startByte, size_t numBytes) const;

        std::uint16_t getRawVersion() const;

    public:
        Item();
        Item(size_t itemsize);
        Item(const Item& other);
        ~Item();

        Item& operator=(const Item& other);
        Item& operator=(Item&& other) noexcept;

        void swap(Item& other);

        std::uint8_t& operator [](size_t position) const;

        size_t size() const; // number of bytes not including socketed items
        size_t getFullSize() const; // number of bytes to store the item, including socketed items
        void resize(size_t itemsize); // itemsize is the number of bytes not including socketed items
        void reserve(size_t itemsize); // itemsize is the number of bytes not including socketed items
        void clear();
        void push_back(const std::uint8_t& value);

        // Simple information
        EnumItemVersion Version() const;
        bool isIdentified() const;
        bool isSocketed() const;
        bool isNew() const;
        bool isEar() const;
        bool isStarterItem() const;
        bool isSimpleItem() const;
        bool isEthereal() const;
        bool isPersonalized() const;
        bool isRuneword() const;
        EnumItemLocation getLocation() const;
        EnumEquippedId getEquippedId() const;
        std::uint8_t getPositionX() const;
        std::uint8_t getPositionY() const;
        EnumAltItemLocation getAltPositionId() const;
        bool getItemCode(std::array<std::uint8_t, 4>& strcode) const;
        EnumItemType getItemType() const;
        std::string getItemTypeName() const;
        bool updateGem(const std::array<std::uint8_t, 4>& newgem);
        bool upgradeGem();
        bool upgradePotion();
        bool upgradeToFullRejuvenationPotion();
        std::uint8_t getQuestDifficulty() const;
        std::uint8_t socketedItemCount() const;
        bool getEarAttributes(EarAttributes& attrib) const;
        bool getRequirements(ItemRequirements& req) const;
        bool getCategories(std::vector<std::string>& categories) const;
        bool getDimensions(ItemDimensions& dimensions) const;
        std::uint32_t getTotalItemSlots() const; // non-zero for belts and Horadric Cube only
        std::string getInvFile() const;
        std::string getTransformColor() const;

        // Extended information
        std::uint32_t getId() const;
        std::uint8_t getLevel() const;
        EnumItemQuality getQuality() const;
        bool getMagicalAffixes(MagicalAffixes& affixes) const;
        bool getRunewordAttributes(RunewordAttributes& attrib) const;
        std::string getPersonalizedName() const;
        std::uint8_t getTomeValue() const; // used in serialization
        bool getSetAttributes(SetAttributes& attrib) const;
        bool getRareOrCraftedAttributes(RareAttributes& attrib) const;
        bool getUniqueAttributes(UniqueAttributes& attrib) const;
        bool getMagicalAttributes(std::vector<MagicalAttribute>& attribs) const;
        bool getCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs) const;
        bool hasMultipleGraphics() const;
        std::uint8_t getPictureId() const;
        bool isAutoAffix() const;
        std::uint16_t getAutoAffixId() const;
        std::uint8_t getInferiorQualityId() const; // used in serialization
        std::uint16_t getFileIndex() const; // used in serialization
        std::uint16_t getSetItemMask() const; // used in serialization
        bool isArmor() const;
        bool isWeapon() const;
        bool isTome() const;
        bool isStackable() const;
        bool isPotion() const;
        bool isGem() const;
        bool isUpgradableGem() const;
        bool isUpgradablePotion() const;
        bool isUpgradableToFullRejuvenationPotion() const;
        bool isRune() const;
        bool isJewel() const;
        bool isCharm() const;
        bool isBelt() const;
        bool isHoradricCube() const;
        std::uint8_t totalNumberOfSockets() const;
        std::uint16_t getQuantity() const;
        bool setQuantity(std::uint16_t quantity);
        bool setMaxQuantity();
        std::uint16_t getDefenseRating() const;
        bool getDurability(ItemDurability& attrib) const;
        bool setDurability(const ItemDurability& durability);
        bool fixDurability();
        bool setMaxDurability();
        bool getDamage(ItemDamage& damage) const;
        bool getRealmDataFlag() const;

        // Helper methods that return the text displayed on tooltips
        std::string getDisplayedItemName() const;
        std::string getDisplayedSocketedRunes() const;
        std::uint16_t getDisplayedDefenseRating(std::uint32_t charLevel) const;
        bool getDisplayedDurability(ItemDurability& durability, std::uint32_t charLevel) const;
        bool getDisplayedDamage(ItemDamage& damage, std::uint32_t charLevel) const;
        std::string getDisplayedItemAttributes(EnumCharClass charClass, std::uint32_t charLevel) const;
        bool getDisplayedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;
        bool getDisplayedRunewordAttributes(RunewordAttributes& attrib, std::uint32_t charLevel) const;
        bool getDisplayedCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;

    public:
        std::list<Item> SocketedItems; // socketed items

    protected:
        std::vector<MagicalAttribute> socketedMagicalAttributes;
    };

    //---------------------------------------------------------------------------
    class Items
    {
        friend class Character;

    protected:
        EnumCharVersion Version = APP_CHAR_VERSION;

        std::uint32_t items_location = 0,
            corpse_location = 0,
            corpse_item_location = 0,
            merc_location = 0,   // Expansion character only
            golem_location = 0;  // Expansion character only

        std::uint16_t NumOfItems = 0;      // # of items (according to file) in inventory excluding 
                                           // gems in socketed items
        std::list<Item> Inventory;         // items in inventory

        std::vector<std::reference_wrapper<Item>> GPSs;       // inventory of all Gems, Potions or Skulls
        std::vector<std::reference_wrapper<Item>> Stackables; // inventory of all Stackable (includes some weapons)
        std::vector<std::reference_wrapper<Item>> Armor;      // inventory of all Armor
        std::vector<std::reference_wrapper<Item>> Weapons;    // inventory of all Weapons (includes stackable weapons)

        mutable std::map<d2ce::EnumItemLocation, std::vector<std::reference_wrapper<Item>>> ItemLocationReference;       // Iventory of items equipped or stored in the belt
        mutable std::map<d2ce::EnumAltItemLocation, std::vector<std::reference_wrapper<Item>>> ItemAltLocationReference; // Iventory of items not equipped or stored in the belt

        bool HasHoradricCube = false;
        bool HasBeltEquipped = false;
        size_t EquippedBeltSlots = 0;

        CorpseHeader CorpseInfo;
        std::uint16_t NumOfCorpseItems = 0; // # of items included in the Corpse section (according to file), non-zero if you are currently dead
        std::list<Item> CorpseItems;      // items on our Corpse

        // Expansion Character data
        std::uint16_t NumOfMercItems = 0;  // # of Mercenary items (according to file)
        std::list<Item> MercItems;         // items mercenary is currently wearing.

        std::uint8_t HasGolem = 0;         // Necromancer only, non-0 if you have a Golem
        Item GolemItem;                    // Item for the Golem

        bool update_locations = true;
        bool isFileExpansionCharacter = false;

    private:
        void findItems();

        bool readItems(std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::list<Item>& items);
        bool fillItemsArray(std::FILE* charfile, std::uint32_t location, std::uint16_t numItems, std::list<Item>& items);
        bool readItemsList(const Json::Value& itemListroot, bool bSerializedFormat, std::list<Item>& items);
        bool readItems(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::list<Item>& items);
        bool fillItemsArray(const Json::Value& itemsRoot, bool bSerializedFormat, std::list<Item>& items);

        bool readCorpseItems(std::FILE* charfile);
        bool readCorpseItems(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile);
        void readMercItems(std::FILE* charfile);
        void readMercItems(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile);
        void readGolemItem(std::FILE* charfile);
        void readGolemItem(const Json::Value& root, bool bSerializedFormat, std::FILE* charfile);

        bool writeCorpseItems(std::FILE* charfile);
        bool writeMercItems(std::FILE* charfile);
        bool writeGolemItem(std::FILE* charfile);

    protected:
        bool readItems(EnumCharVersion version, std::FILE* charfile, bool isExpansion = false);
        bool readItems(const Json::Value& root, bool bSerializedFormat, EnumCharVersion version, std::FILE* charfile, bool isExpansion = false);
        bool writeItems(std::FILE* charfile, bool isExpansion = false);

        void itemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        void corpseItemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        bool mercItemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        bool golemItemAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        bool itemBonusesAsJson(Json::Value& parent, bool bSerializedFormat = false) const;
        void asJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;

    public:
        Items();
        Items(const Items& other);
        ~Items();

        Items& operator=(const Items& other);
        Items& operator=(Items&& other) noexcept;

        void swap(Items& other);

        void clear();

        bool anyUpgradableGems() const;
        bool anyUpgradablePotions() const;
        bool anyUpgradableRejuvenations() const;

        // returns the number of items belonging to the character.
        // Value returned excludes socketed gems/jewels/runes.
        size_t getNumberOfItems() const;

        // returns the number of items equipped on the character.
        // Value returned excludes socketed gems/jewels/runes.
        size_t getNumberOfEquippedItems() const;
        const std::vector<std::reference_wrapper<Item>>& getEquippedItems() const;

        bool getHasBeltEquipped() const;
        size_t getMaxNumberOfItemsInBelt() const;
        size_t getNumberOfItemsInBelt() const;
        const std::vector<std::reference_wrapper<Item>>& getItemsInBelt() const;

        // returns the number of items in character's inventory.
        // Value returned excludes socketed gems/jewels/runes.
        size_t getNumberOfItemsInInventory() const;
        const std::vector<std::reference_wrapper<Item>>& getItemsInInventory() const;

        // returns the number of items in character's private stash.
        // Value returned excludes socketed gems/jewels/runes.
        size_t getNumberOfItemsInStash() const;
        const std::vector<std::reference_wrapper<Item>>& getItemsInStash() const;

        // returns the number of items in character's Horadric Cube
        // Value returned excludes socketed gems/jewels/runes.
        bool getHasHoradricCube() const;
        size_t getNumberOfItemsInHoradricCube() const;
        const std::vector<std::reference_wrapper<Item>>& getItemsInHoradricCube() const;

        size_t getByteSize() const; // number of bytes to store all item sections

        size_t getNumberOfGPSs() const;
        const std::vector<std::reference_wrapper<Item>>& getGPSs() const;

        size_t getNumberOfStackables() const;
        const std::vector<std::reference_wrapper<Item>>& getStackables() const;

        size_t getNumberOfArmor() const;
        const std::vector<std::reference_wrapper<Item>>& getArmor() const;

        size_t getNumberOfWeapons() const;
        const std::vector<std::reference_wrapper<Item>>& getWeapons() const;

        size_t upgradeGems();
        size_t upgradePotions();
        size_t upgradeRejuvenationPotions();
        size_t convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem);
        size_t fillAllStackables();
        size_t fixAllItems();
        size_t maxDurabilityAllItems();

        bool getItemBonuses(std::vector<MagicalAttribute>& attribs) const;
        bool getCharmBonuses(std::vector<MagicalAttribute>& attribs) const;
        bool getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;
        std::uint16_t getCombinedDefenseRating(std::uint32_t charLevel) const;
        bool getCombinedDamage(BaseDamage& damage, std::uint32_t charLevel) const;

        // Mercenary
        const std::list<Item>& getMercItems() const;
        bool getMercItemBonuses(std::vector<MagicalAttribute>& attribs) const;
        bool getDisplayedMercItemBonuses(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;
        std::uint16_t getCombinedMercDefenseRating(std::uint32_t charLevel) const;
        bool getCombinedMercDamage(BaseDamage& damage, std::uint32_t charLevel) const;

        // Corpse
        bool hasCorpse() const;
        const std::list<Item>& getCorpseItems() const;

        // Golem Info
        bool hasGolem() const;
        const d2ce::Item& getGolemItem() const;

    public:
        static std::int64_t getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx);
    };
    //---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------

