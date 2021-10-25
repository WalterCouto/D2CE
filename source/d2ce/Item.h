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

#include "CharacterConstants.h"
#include "ItemConstants.h"
#include "DataTypes.h"
#include "sstream"

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
        bool parsePropertyList(std::FILE* charfile, size_t& current_bit_offset);
        bool readPropertyList(size_t& current_bit_offset, std::vector<MagicalAttribute>& attrib) const;
        std::uint8_t getEncodedChar(std::FILE* charfile, size_t& current_bit_offset);

        bool updateBits(size_t start, std::uint8_t size, std::uint32_t value);
        bool updateBits64(size_t start, std::uint8_t size, std::uint64_t value);
        bool updateItemCodev115(std::uint64_t code, std::uint8_t numBitsSet);

    protected:
        bool readItem(EnumCharVersion version, std::FILE* charfile);
        bool writeItem(std::FILE* charfile);

        void asJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel, bool isListItem = true) const;
        void unknownAsJson(std::stringstream& ss, const std::string& parentIndent) const;
        void byteRangeAsJson(std::stringstream& ss, const std::string& parentIndent, size_t startByte, size_t numBytes) const;

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
        bool isPotion() const;
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
        bool getItemCode(std::uint8_t(&strcode)[4]) const;
        EnumItemType getItemType() const;
        std::string getItemTypeName() const;
        bool updateGem(const std::uint8_t(&newgem)[4]);
        std::uint8_t getQuestDifficulty() const;
        std::uint8_t socketedItemCount() const;
        bool getEarAttributes(EarAttributes& attrib) const;
        bool getRequirements(ItemRequirements& req) const;
        bool getCategories(std::vector<std::string>& categories) const;
        bool getDimensions(ItemDimensions& dimensions) const;
        std::string getInvFile() const;

        // Extended information
        std::uint32_t getId() const;
        std::uint8_t getLevel() const;
        EnumItemQuality getQuality() const;
        bool getMagicalAffixes(MagicalAffixes& affixes) const;
        bool getRunewordAttributes(RunewordAttributes& attrib) const;
        std::string getPersonalizedName() const;
        bool getSetAttributes(SetAttributes& attrib) const;
        bool getRareOrCraftedAttributes(RareAttributes& attrib) const;
        bool getUniqueAttributes(UniqueAttributes& attrib) const;
        bool getMagicalAttributes(std::vector<MagicalAttribute>& attribs) const;
        bool getCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs) const;
        bool hasMultipleGraphics() const;
        std::uint8_t getPictureId() const;
        bool isAutoAffix() const;
        std::uint8_t getInferiorQualityId() const;
        bool isArmor() const;
        bool isWeapon() const;
        bool isTome() const;
        bool isStackable() const;
        bool isRune() const;
        std::uint8_t totalNumberOfSockets() const;
        std::uint16_t getQuantity() const;
        bool setQuantity(std::uint16_t quantity);
        bool setMaxQuantity();
        std::uint16_t getDefenseRating() const;
        bool getDurability(ItemDurability& attrib) const;
        bool setDurability(const ItemDurability& attrib);
        bool setMaxDurability();
        bool getDamage(ItemDamage& damage) const;
        bool getRealmDataFlag() const;

        // Helper methods that return the text displayed on tooltips
        std::string getDisplayedItemName() const;
        std::string getDisplayedSocketedRunes() const;
        std::string getDisplayedItemAttributes(EnumCharClass charClass) const;
        bool getDisplayedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;
        bool getDisplayedRunewordAttributes(RunewordAttributes& attrib, std::uint32_t charLevel) const;
        bool getDisplayedCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;

    public:
        std::vector<Item> SocketedItems; // socketed items

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
            merc_location = 0,   // Expansion character only
            golem_location = 0;  // Expansion character only

        std::uint16_t NumOfItems = 0;      // # of items (according to file) in inventory excluding 
                                           // gems in socketed items
        std::vector<Item> Inventory;       // items in inventory

        std::vector<std::reference_wrapper<Item>> GPSs;       // inventory of all Gems, Potions or Skulls
        std::vector<std::reference_wrapper<Item>> Stackables; // inventory of all Stackable (includes some weapons)
        std::vector<std::reference_wrapper<Item>> Armor;      // inventory of all Armor
        std::vector<std::reference_wrapper<Item>> Weapons;    // inventory of all Weapons (includes stackable weapons)

        std::uint16_t NumOfCorpseItems = 0; // # of items included in the Corpse section (according to file), non zero if you are currently dead
        std::vector<Item> CorpseItems;      // items on our Corpse

        // Expansion Character data
        std::uint16_t NumOfMercItems = 0;  // # of Mercenary items (according to file)
        std::vector<Item> MercItems;       // items mercenary is currently wearing.

        std::uint8_t HasGolem = 0;         // Necromancer only, non-0 if you have a Golem
        Item GolemItem;                    // Item for the Golem

        bool update_locations = true;
        bool isFileExpansionCharacter = false;

    private:
        void findItems();

        bool readItems(std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::vector<Item>& Items);
        bool fillItemsArray(std::FILE* charfile, std::uint32_t location, std::uint16_t numItems, std::vector<Item>& items);

        bool readCorpseItems(std::FILE* charfile);
        void readMercItems(std::FILE* charfile);
        void readGolemItem(std::FILE* charfile);

        bool writeCorpseItems(std::FILE* charfile);
        bool writeMercItems(std::FILE* charfile);
        bool writeGolemItem(std::FILE* charfile);

    protected:
        bool readItems(EnumCharVersion version, std::FILE* charfile, bool isExpansion = false);
        bool writeItems(std::FILE* charfile, bool isExpansion = false);

        void itemsAsJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const;
        void corpseItemsAsJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const;
        bool mercItemsAsJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const;
        bool golemItemAsJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const;
        bool itemBonusesAsJson(std::stringstream& ss, const std::string& parentIndent) const;
        void asJson(std::stringstream& ss, const std::string& parentIndent, std::uint32_t charLevel) const;

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

        // returns the number of items in character's inventory.
        // Value returned excludes socketed gems/jewels/runes.
        size_t getNumberOfItems() const;
        const std::vector<d2ce::Item>& getInventoryItems() const;

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
        size_t convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4]);
        size_t fillAllStackables();
        size_t maxDurabilityAllItems();

        bool getItemBonuses(std::vector<MagicalAttribute>& attribs) const;
        bool getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;

        // Mercenary
        const std::vector<Item>& getMercItems() const;
        bool getMercItemBonuses(std::vector<MagicalAttribute>& attribs) const;
        bool getDisplayedMercItemBonuses(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;
    };
    //---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------

