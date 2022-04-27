/*
    Diablo II Character Editor
    Copyright (C) 2022 Walter Couto

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

#include "Item.h"
#include <vector>
#include <filesystem>

namespace d2ce
{
    class Character;

    //---------------------------------------------------------------------------
    class SharedStash
    {
    private:
        struct SharedStashHeader
        {
            std::array<std::uint8_t, HEADER_LENGTH> Header = { 0x55, 0xAA, 0x55, 0xAA }; // page start header (4 bytes)
            std::uint32_t Unk_0x04 = 0;                 // pos 0x04 relative to page start (4 bytes) unknown, all 0x00
            std::uint32_t Version = 97;                 // pos 0x08 relative to page start (4 bytes) 97 or higher valid
            std::uint32_t Gold = 0;                     // pos 0x0C relative to page start (4 bytes)
            std::uint32_t PageLength = 0;               // pos 0x0F relative to page start (4 bytes) is the lengh of the page including the header
            std::array<std::uint8_t, 44> Unk_0x13 = {}; // pos 0x13 relative to page start(44 bytes) unknown.all 0x00
        };

        struct SharedStashPage
        {
            SharedStashPage(std::list<d2ce::Item>& bufferItems) : StashItems(bufferItems)
            {
            }

            SharedStashHeader Header; // page start (64 bytes)
            Items StashItems;         // pos 0x40 relative to page start
        };

        std::vector<SharedStashPage> Pages;
        std::list<d2ce::Item> BufferItems;       // Buffer for items not in any page's inventory yet
        std::filesystem::path m_d2ifilename;
        EnumCharVersion CharVersion = APP_CHAR_VERSION;

    public:
        SharedStash();
        SharedStash(const Character& charInfo);
        SharedStash(const SharedStash& other);
        ~SharedStash();

        SharedStash& operator=(const SharedStash& other);
        SharedStash& operator=(SharedStash&& other) noexcept;

        void clear(bool bItemsOnly = false);
        void reset(const Character& charInfo);
        void swap(SharedStash& other);
        bool load();
        bool refresh();
        bool save(bool saveBackup = true);
        const std::filesystem::path& getPath() const;

        bool hasSharedStash() const;
        bool isLoaded() const;

        size_t size() const; // number of pages
        std::uint32_t getGoldInStash(size_t page) const;
        bool setGoldInStash(std::uint32_t goldValue, size_t page);

        bool getDimensions(ItemDimensions& dimensions) const;

        // Items
        bool anyUpgradableGems() const;
        bool anyUpgradableGems(size_t page) const;
        bool anyUpgradablePotions() const;
        bool anyUpgradablePotions(size_t page) const;
        bool anyUpgradableRejuvenations() const;
        bool anyUpgradableRejuvenations(size_t page) const;

        // returns the number of items belonging to the shared stash
        // Value returned excludes socketed gems/jewels/runes.
        size_t getNumberOfItems() const;
        size_t getNumberOfItems(size_t page) const;

        const std::vector<std::vector<std::reference_wrapper<Item>>> getItems() const; // items for all pages
        const std::vector<std::reference_wrapper<Item>>& getItems(size_t page) const;

        size_t getByteSize() const; // number of bytes to store all pages
        size_t getByteSize(size_t page) const; // number of bytes to store page

        size_t getNumberOfGPSs() const;
        size_t getNumberOfGPSs(size_t page) const;
        const std::vector<std::vector<std::reference_wrapper<Item>>> getGPSs() const;
        const std::vector<std::reference_wrapper<Item>>& getGPSs(size_t page) const;

        size_t getNumberOfStackables() const;
        size_t getNumberOfStackables(size_t page) const;
        const std::vector<std::vector<std::reference_wrapper<Item>>> getStackables() const;
        const std::vector<std::reference_wrapper<Item>>& getStackables(size_t page) const;

        size_t getNumberOfArmor() const;
        size_t getNumberOfArmor(size_t page) const;
        const std::vector<std::vector<std::reference_wrapper<Item>>> getArmor() const;
        const std::vector<std::reference_wrapper<Item>>& getArmor(size_t page) const;

        size_t getNumberOfWeapons() const;
        size_t getNumberOfWeapons(size_t page) const;
        const std::vector<std::vector<std::reference_wrapper<Item>>> getWeapons() const;
        const std::vector<std::reference_wrapper<Item>>& getWeapons(size_t page) const; 
        
        size_t upgradeGems();
        size_t upgradeGems(size_t page);
        size_t upgradePotions();
        size_t upgradePotions(size_t page);
        size_t upgradeRejuvenationPotions();
        size_t upgradeRejuvenationPotions(size_t page);
        size_t convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem);
        size_t convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, size_t page);
        size_t fillAllStackables();
        size_t fillAllStackables(size_t page);
        size_t repairAllItems();
        size_t repairAllItems(size_t page);
        size_t maxDurabilityAllItems();
        size_t maxDurabilityAllItems(size_t page);
        size_t maxSocketCountAllItems();
        size_t maxSocketCountAllItems(size_t page);
        size_t setIndestructibleAllItems();
        size_t setIndestructibleAllItems(size_t page);
        bool addItem(std::array<std::uint8_t, 4>& strcode, size_t page);
        bool importItem(const std::filesystem::path& path, const d2ce::Item*& pImportedItem, bool bRandomizeId = true);
        size_t fillEmptySlots(std::array<std::uint8_t, 4>& strcode);
        size_t fillEmptySlots(std::array<std::uint8_t, 4>& strcode, size_t page);
        bool setItemLocation(d2ce::Item& item, size_t itemPage, std::uint16_t positionX, std::uint16_t positionY, size_t page, const d2ce::Item*& pRemovedItem);
        bool removeSocketedItems(d2ce::Item& item);

    protected:
        bool refresh(std::FILE* charfile);
    };
    //---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------

