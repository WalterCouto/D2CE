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

#include "pch.h"
#include "SharedStash.h"
#include "Character.h"

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::array<std::uint8_t, 4> HEADER = { 0x55, 0xAA, 0x55, 0xAA };
    constexpr size_t PAGE_HEADER_SIZE = 0x44;

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
}

//---------------------------------------------------------------------------
d2ce::SharedStash::SharedStash()
{
}
//---------------------------------------------------------------------------
d2ce::SharedStash::SharedStash(const Character& charInfo)
{
    CharVersion = charInfo.getVersion();
    if (CharVersion >= EnumCharVersion::v115 && charInfo.isExpansionCharacter())
    {
        m_d2ifilename = charInfo.getPath();
        m_d2ifilename.replace_filename("SharedStashSoftCoreV2").replace_extension("d2i");
        if (!std::filesystem::exists(m_d2ifilename))
        {
            m_d2ifilename.clear();
        }
    }
}
//---------------------------------------------------------------------------
d2ce::SharedStash::SharedStash(const SharedStash& other)
{
    *this = other;
}
//---------------------------------------------------------------------------
d2ce::SharedStash::~SharedStash()
{
}
//---------------------------------------------------------------------------
d2ce::SharedStash& d2ce::SharedStash::operator=(const SharedStash& other)
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    BufferItems = other.BufferItems;
    Pages = other.Pages;
    m_d2ifilename = other.m_d2ifilename;
    return *this;
}
//---------------------------------------------------------------------------
d2ce::SharedStash& d2ce::SharedStash::operator=(SharedStash&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    BufferItems.swap(other.BufferItems);
    other.BufferItems.clear();
    Pages.swap(other.Pages);
    other.Pages.clear();
    m_d2ifilename.swap(other.m_d2ifilename);
    other.m_d2ifilename.clear();
    return *this;
}
//---------------------------------------------------------------------------
void d2ce::SharedStash::clear(bool bItemsOnly)
{
    if (!bItemsOnly)
    {
        m_d2ifilename.clear();
    }
    Pages.clear();
}
//---------------------------------------------------------------------------
void d2ce::SharedStash::reset(const Character& charInfo)
{
    clear();
    CharVersion = charInfo.getVersion();
    if (CharVersion >= EnumCharVersion::v115 && charInfo.isExpansionCharacter())
    {
        m_d2ifilename = charInfo.getPath();
        m_d2ifilename.replace_filename("SharedStashSoftCoreV2").replace_extension("d2i");
        if (!std::filesystem::exists(m_d2ifilename))
        {
            m_d2ifilename.clear();
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::SharedStash::swap(SharedStash& other)
{
    std::swap(*this, other);
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::load()
{
    if (isLoaded())
    {
        return true;
    }

    return refresh();
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::refresh()
{
    Pages.clear();
    if (m_d2ifilename.empty() || !std::filesystem::exists(m_d2ifilename))
    {
        m_d2ifilename.clear();
        return false;
    }

    std::FILE* charfile = nullptr;
#ifdef _MSC_VER
    charfile = _wfsopen(m_d2ifilename.wstring().c_str(), L"rb+", _SH_DENYNO);
#else
    errno_t err = wfopen_s(&charfile, m_d2ifilename.wstring().c_str(), L"rb+");
    if (err != 0)
    {
        return false;
    }
#endif

    if (!refresh(charfile))
    {
        std::fclose(charfile);
        charfile = nullptr;
        return false;
    }

    std::fclose(charfile);
    charfile = nullptr;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::save(bool saveBackup)
{
    if(m_d2ifilename.empty())
    {
        return false;
    }

    if (saveBackup && std::filesystem::exists(m_d2ifilename))
    {
        auto now = std::chrono::system_clock::now();
        auto UTC = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        auto ext = "." + std::to_string(UTC) + ".bak";
        auto backupd2ifilename = m_d2ifilename;
        backupd2ifilename.replace_extension(ext);
        std::filesystem::copy_file(m_d2ifilename, backupd2ifilename, std::filesystem::copy_options::overwrite_existing);
    }

    std::FILE* charfile = nullptr;
    errno_t err = _wfopen_s(&charfile, m_d2ifilename.wstring().c_str(), L"wb");
    if (err != 0)
    {
        return false;
    }

    for (auto& page : Pages)
    {
        auto& items = page.StashItems;
        auto& pageHeader = page.Header;
        pageHeader.PageLength = std::uint32_t(items.getByteSize() + PAGE_HEADER_SIZE);
        std::fwrite(&pageHeader, sizeof(pageHeader), 1, charfile); 
        if (!items.writeSharedStashPage(charfile))
        {
            // corrupt file
            std::fclose(charfile);
            charfile = nullptr;
            return false;
        }
    }

    std::fclose(charfile);
    charfile = nullptr;
    return true;
}
//---------------------------------------------------------------------------
const std::filesystem::path& d2ce::SharedStash::getPath() const
{
    return m_d2ifilename;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::hasSharedStash() const
{
    return m_d2ifilename.empty() ? false : true;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::isLoaded() const
{
    if (Pages.empty())
    {
        return false;
    }

    if (m_d2ifilename.empty() || !std::filesystem::exists(m_d2ifilename))
    {
        // should not happen
        const_cast<d2ce::SharedStash*>(this)->clear();
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::size() const
{
    return Pages.size();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::SharedStash::getGoldInStash(size_t page) const
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].Header.Gold;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::setGoldInStash(std::uint32_t goldValue, size_t page)
{
    if (page >= Pages.size())
    {
        return false;
    }

    return Pages[page].Header.Gold = std::min(goldValue, d2ce::GOLD_IN_STASH_LIMIT);
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::getDimensions(ItemDimensions& dimensions) const
{
    dimensions.Width = dimensions.InvWidth = 10;
    dimensions.Height = dimensions.InvHeight = 10;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::anyUpgradableGems() const
{
    for (const auto& page : Pages)
    {
        if (page.StashItems.anyUpgradableGems())
        {
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::anyUpgradableGems(size_t page) const
{
    if (page >= Pages.size())
    {
        return false;
    }

    return Pages[page].StashItems.anyUpgradableGems();
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::anyUpgradablePotions() const
{
    for (const auto& page : Pages)
    {
        if (page.StashItems.anyUpgradablePotions())
        {
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::anyUpgradablePotions(size_t page) const
{
    if (page >= Pages.size())
    {
        return false;
    }

    return Pages[page].StashItems.anyUpgradablePotions();
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::anyUpgradableRejuvenations() const
{
    for (const auto& page : Pages)
    {
        if (page.StashItems.anyUpgradableRejuvenations())
        {
            return true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::anyUpgradableRejuvenations(size_t page) const
{
    if (page >= Pages.size())
    {
        return false;
    }

    return Pages[page].StashItems.anyUpgradableRejuvenations();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfItems() const
{
    size_t numItems = 0;
    for (const auto& page : Pages)
    {
        numItems += page.StashItems.getNumberOfItems();
    }

    return numItems;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfItems(size_t page) const
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.getNumberOfItems();
}
//---------------------------------------------------------------------------
const std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> d2ce::SharedStash::getItems() const
{
    std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> items;
    for (const auto& page : Pages)
    {
        items.push_back(page.StashItems.getItemsInStash());
    }

    return items;
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::SharedStash::getItems(size_t page) const
{
    if (page >= Pages.size())
    {
        static std::vector<std::reference_wrapper<d2ce::Item>> s_dummy;
        return s_dummy;
    }

    return Pages[page].StashItems.getItemsInStash();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getByteSize() const
{
    size_t totalSize = 0;
    for (const auto& page : Pages)
    {
        totalSize += page.StashItems.getByteSize() + PAGE_HEADER_SIZE;
    }
    return totalSize;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getByteSize(size_t page) const
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.getByteSize() + PAGE_HEADER_SIZE;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfGPSs() const
{
    size_t total = 0;
    for (const auto& page : Pages)
    {
        total += page.StashItems.getNumberOfGPSs();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfGPSs(size_t page) const
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.getNumberOfGPSs();
}
//---------------------------------------------------------------------------
const std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> d2ce::SharedStash::getGPSs() const
{
    std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> items;
    for (const auto& page : Pages)
    {
        items.push_back(page.StashItems.getGPSs());
    }

    return items;
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::SharedStash::getGPSs(size_t page) const
{
    if (page >= Pages.size())
    {
        static std::vector<std::reference_wrapper<d2ce::Item>> s_dummy;
        return s_dummy;
    }

    return Pages[page].StashItems.getGPSs();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfStackables() const
{
    size_t total = 0;
    for (const auto& page : Pages)
    {
        total += page.StashItems.getNumberOfStackables();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfStackables(size_t page) const
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.getNumberOfStackables();
}
//---------------------------------------------------------------------------
const std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> d2ce::SharedStash::getStackables() const
{
    std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> items;
    for (const auto& page : Pages)
    {
        items.push_back(page.StashItems.getStackables());
    }

    return items;
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::SharedStash::getStackables(size_t page) const
{
    if (page >= Pages.size())
    {
        static std::vector<std::reference_wrapper<d2ce::Item>> s_dummy;
        return s_dummy;
    }

    return Pages[page].StashItems.getStackables();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfArmor() const
{
    size_t total = 0;
    for (const auto& page : Pages)
    {
        total += page.StashItems.getNumberOfArmor();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfArmor(size_t page) const
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.getNumberOfArmor();
}
//---------------------------------------------------------------------------
const std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> d2ce::SharedStash::getArmor() const
{
    std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> items;
    for (const auto& page : Pages)
    {
        items.push_back(page.StashItems.getArmor());
    }

    return items;
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::SharedStash::getArmor(size_t page) const
{
    if (page >= Pages.size())
    {
        static std::vector<std::reference_wrapper<d2ce::Item>> s_dummy;
        return s_dummy;
    }

    return Pages[page].StashItems.getArmor();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfWeapons() const
{
    size_t total = 0;
    for (const auto& page : Pages)
    {
        total += page.StashItems.getNumberOfWeapons();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::getNumberOfWeapons(size_t page) const
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.getNumberOfWeapons();
}
//---------------------------------------------------------------------------
const std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> d2ce::SharedStash::getWeapons() const
{
    std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> items;
    for (const auto& page : Pages)
    {
        items.push_back(page.StashItems.getWeapons());
    }

    return items;
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::SharedStash::getWeapons(size_t page) const
{
    if (page >= Pages.size())
    {
        static std::vector<std::reference_wrapper<d2ce::Item>> s_dummy;
        return s_dummy;
    }

    return Pages[page].StashItems.getWeapons();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::upgradeGems()
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.upgradeGems();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::upgradeGems(size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.upgradeGems();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::upgradePotions()
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.upgradePotions();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::upgradePotions(size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.upgradePotions();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::upgradeRejuvenationPotions()
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.upgradeRejuvenationPotions();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::upgradeRejuvenationPotions(size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.upgradeRejuvenationPotions();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem)
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.convertGPSs(existingGem, desiredGem);
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.convertGPSs(existingGem, desiredGem);
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::fillAllStackables()
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.fillAllStackables();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::fillAllStackables(size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.fillAllStackables();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::repairAllItems()
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.repairAllItems();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::repairAllItems(size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.repairAllItems();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::maxDurabilityAllItems()
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.maxDurabilityAllItems();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::maxDurabilityAllItems(size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.maxDurabilityAllItems();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::maxSocketCountAllItems()
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.maxSocketCountAllItems();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::maxSocketCountAllItems(size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.maxSocketCountAllItems();
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::setIndestructibleAllItems()
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.setIndestructibleAllItems();
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::setIndestructibleAllItems(size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.setIndestructibleAllItems();
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::addItem(std::array<std::uint8_t, 4>& strcode, size_t page)
{
    if (page >= Pages.size())
    {
        return false;
    }

    return Pages[page].StashItems.addItem(EnumAltItemLocation::STASH, strcode);
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::importItem(const std::filesystem::path& path, const d2ce::Item*& pImportedItem, bool bRandomizeId)
{
    if (Pages.empty())
    {
        return false;
    }

    // doesn't matter which page does the import
    return Pages.front().StashItems.importItem(path, pImportedItem, bRandomizeId);
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::fillEmptySlots(std::array<std::uint8_t, 4>& strcode)
{
    size_t total = 0;
    for (auto& page : Pages)
    {
        total += page.StashItems.fillEmptySlots(EnumAltItemLocation::STASH, strcode);
    }
    return total;
}
//---------------------------------------------------------------------------
size_t d2ce::SharedStash::fillEmptySlots(std::array<std::uint8_t, 4>& strcode, size_t page)
{
    if (page >= Pages.size())
    {
        return 0;
    }

    return Pages[page].StashItems.fillEmptySlots(EnumAltItemLocation::STASH, strcode);
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::setItemLocation(d2ce::Item& item, size_t itemPage, std::uint16_t positionX, std::uint16_t positionY, size_t page, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    if ((page >= Pages.size()) || (itemPage >= Pages.size()))
    {
        return false;
    }

    // find entry
    auto& pageStash = Pages[page].StashItems;
    auto& itemPageStash = Pages[itemPage].StashItems;
    auto& inventory = itemPageStash.Inventory;
    auto iter = std::find_if(inventory.begin(), inventory.end(), ItemPredicate(item));
    if (iter != inventory.end())
    {
        if (page != itemPage)
        {
            // move item to Buffer
            if (!Pages[itemPage].StashItems.setItemLocation(item, EnumAltItemLocation::STASH, item.getPositionX(), item.getPositionY(), EnumItemInventory::BUFFER, pRemovedItem))
            {
                return false;
            }
        }
    }

    auto prevPositionX = item.getPositionX();
    auto prevPositionY = item.getPositionY();
    if (!pageStash.setItemLocation(item, EnumAltItemLocation::STASH, positionX, positionY, EnumItemInventory::SHARED_STASH, pRemovedItem))
    {
        // if item was moved to the buffer, move it back to the inventory
        if (std::find_if(BufferItems.begin(), BufferItems.end(), ItemPredicate(item)) != BufferItems.end())
        {
            if (!itemPageStash.setItemLocation(item, EnumAltItemLocation::STASH, prevPositionX, prevPositionY, EnumItemInventory::SHARED_STASH, pRemovedItem))
            {
                pRemovedItem = &item;
            }
        }
        return false;
    }
    
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::removeSocketedItems(d2ce::Item& item)
{
    for (auto& page : Pages)
    {
        auto& pageStash = page.StashItems;
        auto& inventory = pageStash.Inventory;
        auto iter = std::find_if(inventory.begin(), inventory.end(), ItemPredicate(item));
        if (iter != inventory.end())
        {
            return pageStash.removeSocketedItems(item);
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::removeItem(d2ce::Item& item)
{
    for (auto& page : Pages)
    {
        auto& pageStash = page.StashItems;
        auto& inventory = pageStash.Inventory;
        auto iter = std::find_if(inventory.begin(), inventory.end(), ItemPredicate(item));
        if (iter != inventory.end())
        {
            const d2ce::Item* pRemovedItem = nullptr;
            return pageStash.setItemLocation(item, EnumItemLocation::BUFFER, 0, 0, EnumItemInventory::BUFFER, pRemovedItem);
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::setItemRuneword(d2ce::Item& item, std::uint16_t id)
{
    for (auto& page : Pages)
    {
        auto& pageStash = page.StashItems;
        auto& inventory = pageStash.Inventory;
        auto iter = std::find_if(inventory.begin(), inventory.end(), ItemPredicate(item));
        if (iter != inventory.end())
        {
            return pageStash.setItemRuneword(item, id);
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::SharedStash::refresh(std::FILE* charfile)
{
    std::uint32_t fileSize = 0;
    std::uint32_t pos = 0;
    std::uint32_t expectedPos = 0;
    std::fseek(charfile, 0, SEEK_END);
    fileSize = std::ftell(charfile);
    std::fseek(charfile, 0, SEEK_SET);
    while (!feof(charfile) && (pos < fileSize))
    {
        Pages.emplace_back(SharedStashPage(BufferItems));
        auto& page = Pages.back();
        auto& pageHeader = page.Header;
        std::fread(&pageHeader, sizeof(pageHeader), 1, charfile);
        if (pageHeader.Header != HEADER)
        { 
            // corrupt file
            Pages.pop_back();
            return false;
        }

        if (pageHeader.Version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v115))
        {
            // corrupt file
            Pages.pop_back();
            return false;
        }

        auto& items = page.StashItems;
        if (!items.readSharedStashPage(CharVersion, charfile))
        {
            // corrupt file
            Pages.pop_back();
            return false;
        }

        expectedPos = pos + pageHeader.PageLength;
        pos = std::ftell(charfile);
        if (pos != expectedPos)
        {
            // corrupt file
            Pages.pop_back();
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
