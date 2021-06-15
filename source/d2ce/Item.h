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
        size_t type_code_offset = 76;
        size_t extended_data_offset = 108;
        mutable size_t item_id_bit_offset = 0;
        mutable size_t item_level_bit_offset = 0;
        mutable size_t multi_graphic_bit_offset = 0;
        mutable size_t autoAffix_bit_offset = 0;
        mutable size_t quality_bit_offset = 0;
        size_t personalized_bit_offset = 0;
        size_t armor_or_weapon_bit_offset = 0;
        size_t durability_bit_offset = 0;
        size_t stackable_bit_offset = 0;
        size_t socket_count_bit_offset = 0;
        size_t bonus_bits_bit_offset = 0;
        size_t magical_props_bit_offset = 0;
        size_t set_bonus_props_bit_offset = 0;
        size_t runeword_props_bit_offset = 0;
        size_t item_end_bit_offset = 0;

    private:
        std::uint64_t readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits);
        bool skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits);
        bool readPropertyList(std::FILE* charfile, size_t& current_bit_offset);
        std::uint8_t getEncodedChar(std::FILE* charfile, size_t& current_bit_offset);

        bool updateBits(size_t start, std::uint8_t size, std::uint32_t value);
        bool updateBits64(size_t start, std::uint8_t size, std::uint64_t value);
        bool updateItemCodev115(std::uint64_t code, std::uint8_t numBitsSet);

    protected:
        bool readItem(EnumCharVersion version, std::FILE* charfile);
        bool writeItem(std::FILE* charfile);

    public:
        Item();
        Item(size_t itemsize);
        ~Item();

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
        std::uint8_t getLocation() const;
        bool calculateItemCode(std::uint8_t(&strcode)[4]) const;
        bool updateGem(const std::uint8_t(&newgem)[4]);
        std::uint8_t socketedItemCount() const;

        // Extended information
        std::uint32_t Id() const;
        std::uint8_t Level() const;
        EnumItemQuality Quality() const;
        bool hasMultipleGraphics() const;
        bool isAutoAffix() const;
        bool isArmor() const;
        bool isWeapon() const;
        bool isTome() const;
        bool isStackable() const;
        std::uint8_t totalNumberOfSockets() const;
        std::uint16_t getQuantity() const;
        bool setQuantity(std::uint16_t quantity);
        std::uint8_t getMaxDurability() const;
        bool setMaxDurability(std::uint8_t durability);

    public:
        std::vector<Item> Items; // socketed items
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

        uint16_t NumOfItems = 0;           // # of items (according to file) in inventory excluding 
                                           // gems in socketed items
        std::vector<Item> Items;           // items in inventory

        std::vector<std::reference_wrapper<Item>> GPSs;       // inventory of all Gems, Potions or Skulls
        std::vector<std::reference_wrapper<Item>> Stackables; // inventory of all Stackable (includes some weapons)
        std::vector<std::reference_wrapper<Item>> Armor;      // inventory of all Armor
        std::vector<std::reference_wrapper<Item>> Weapons;    // inventory of all Weapons (includes stackable weapons)

        uint16_t NumOfCorpseItems = 0;     // # of items included in the Corpse section (according to file), non zero if you are currently dead
        std::vector<Item> CorpseItems;     // items on our Corpse

        // Expansion Character data
        uint16_t NumOfMercItems = 0;       // # of Mercenary items (according to file)
        std::vector<Item> MercItems;       // items mercenary is currently wearing.

        std::uint8_t HasGolem = 0;         // Necromancer only, non-0 if you have a Golem
        Item GolemItem;                    // Item for the Golem

        bool update_locations = true;
        bool isFileExpansionCharacter = false;

    private:
        void findItems();

        bool readItems(std::FILE* charfile, std::uint32_t& location, uint16_t& numItems, std::vector<Item>& Items);
        bool fillItemsArray(std::FILE* charfile, std::uint32_t location, uint16_t numItems, std::vector<Item>& items);

        bool readCorpseItems(std::FILE* charfile);
        void readMercItems(std::FILE* charfile);
        void readGolemItem(std::FILE* charfile);

        bool writeCorpseItems(std::FILE* charfile);
        bool writeMercItems(std::FILE* charfile);
        bool writeGolemItem(std::FILE* charfile);

    protected:
        bool readItems(EnumCharVersion version, std::FILE* charfile, bool isExpansion = false);
        bool writeItems(std::FILE* charfile, bool isExpansion = false);

    public:
        void clear();

        bool anyUpgradableGems() const;
        bool anyUpgradablePotions() const;
        bool anyUpgradableRejuvenations() const;

        // returns the number of items in character's inventory.
        // Value returned excludes socketed gems/jewels/runes.
        size_t getNumberOfItems() const;

        size_t getByteSize() const; // number of bytes to store all item sections

        size_t getNumberOfGPSs();
        const std::vector<std::reference_wrapper<Item>>& getGPSs();

        size_t getNumberOfStackables();
        const std::vector<std::reference_wrapper<Item>>& getStackables();

        size_t getNumberOfArmor();
        const std::vector<std::reference_wrapper<Item>>& getArmor();

        size_t getNumberOfWeapons();
        const std::vector<std::reference_wrapper<Item>>& getWeapons();

        size_t upgradeGems();
        size_t upgradePotions();
        size_t upgradeRejuvenationPotions();
        size_t convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4]);
        size_t fillAllStackables();
        size_t maxDurabilityAllItems();
    };
    //---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------

