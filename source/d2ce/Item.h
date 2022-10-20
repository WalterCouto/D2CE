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
#include <set>

namespace d2ce
{
    class Character;
    struct ItemType;
    struct ItemFilter
    {
        EnumItemLocation LocationId = EnumItemLocation::BUFFER;
        EnumAltItemLocation AltPositionId = EnumAltItemLocation::UNKNOWN;
        bool IsMerc = false;
        bool IsCorpse = false;
        bool IsGolem = false;
        bool IsBody = false;

        void clearEquipped()
        {
            IsMerc = false;
            IsCorpse = false;
            IsGolem = false;
            IsBody = false;
        }

        void clear()
        {
            LocationId = EnumItemLocation::BUFFER;
            AltPositionId = EnumAltItemLocation::UNKNOWN;
            clearEquipped();
        }
    };
    //---------------------------------------------------------------------------
    class Item
    {
        friend class Items;

    private:
        mutable std::vector<std::uint8_t> data;
        EnumItemVersion ItemVersion = APP_ITEM_VERSION;
        std::uint16_t GameVersion = APP_ITEM_GAME_VERSION;
        mutable std::array<size_t, 31> bitOffsets = { 16, 58, 61, 65, 73, 76, 108 };
        mutable std::array<size_t, 7> bitOffsetMarkers = { 0 };
        mutable size_t nr_of_items_in_sockets_bits = 3;
        size_t item_current_socket_idx = 0; // temp variable for socketed gem ordering
        mutable MagicalCachev100 magic_affixes_v100;
        mutable RareOrCraftedCachev100 rare_affixes_v100;

    private:
        std::uint64_t readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits);
        bool skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits);
        bool setBits(size_t& current_bit_offset, size_t bits, std::uint32_t value);
        bool setBits64(size_t& current_bit_offset, size_t bits, std::uint64_t value);
        bool parsePropertyList(std::FILE* charfile, size_t& current_bit_offset);
        bool parsePropertyList(const Json::Value& propListRoot, bool bSerializedFormat, size_t& current_bit_offset);
        bool readPropertyList(size_t& current_bit_offset, std::vector<MagicalAttribute>& attrib) const;
        bool updatePropertyList(size_t& current_bit_offset, const std::vector<MagicalAttribute>& attribs);
        std::uint8_t getEncodedChar(std::FILE* charfile, size_t& current_bit_offset);

        std::uint32_t readBits(size_t start, size_t size) const;
        std::uint64_t readBits64(size_t start, size_t size) const;
        bool updateBits(size_t start, size_t size, std::uint32_t value);
        bool updateBits64(size_t start, size_t size, std::uint64_t value);
        bool updateResurrectedItemCode(std::uint64_t code, size_t numBitsSet);

        std::uint8_t getInferiorQualityIdv100() const;
        bool getMagicalAffixesv100(MagicalAffixes& affixes) const;
        bool getSetAttributesv100(SetAttributes& attrib) const;
        bool getMagicalAttributesv100(std::vector<MagicalAttribute>& attribs) const;
        bool getRareOrCraftedAttributesv100(RareAttributes& attrib) const;
        bool hasMultipleGraphicsv100() const;
        std::uint8_t getPictureIdv100() const;
        std::uint16_t getDefenseRatingv100() const;
        
        bool readItemv100(const Json::Value& itemRoot, bool bSerializedFormat);
        bool readItemv104(const Json::Value& itemRoot, bool bSerializedFormat);

        bool verifyItemConsistency() const;

    protected:
        bool readItem(EnumItemVersion version, bool isExpansion, std::FILE* charfile);
        bool readItem(const Json::Value& itemRoot, bool bSerializedFormat, EnumItemVersion version, bool isExpansion);
        bool writeItem(std::FILE* charfile) const;

        void asJson(Json::Value& parent, std::uint32_t charLevel, EnumItemVersion version, bool bSerializedFormat = false) const;
        void asJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        void unknownAsJson(Json::Value& parent, bool bSerializedFormat = false) const;
        void bitRangeAsJson(Json::Value& parent, size_t startBit, size_t numBits) const;

        std::uint16_t getRawVersion() const;

        Item(EnumItemVersion itemVersion, bool isExpansion, const std::filesystem::path& path); // read D2I item

        bool setLocation(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY);
        bool setLocation(EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY);
        bool setLocation(EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY);
        bool setLocation(EnumEquippedId equippedId);
        bool setLocation(EnumItemLocation locationId, EnumAltItemLocation altPositionId, EnumEquippedId equippedId, std::uint16_t positionX, std::uint16_t positionY);

        void verifyRuneword();
        void updateSocketedItemCount();
        void updateOffset(size_t& startOffset, ptrdiff_t diff);

    public:
        Item();
        Item(size_t itemsize);
        Item(const ItemCreateParams& createParms);
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
        EnumItemVersion getVersion() const;

        // refers to the "version" column in resource files used internally
        //   0 = pre v1.08
        //   1 - Non-Expansion (post v1.08) (classic and LoD).
        // 100 - Expansion LoD
        std::uint16_t getGameVersion() const;

        bool isExpansionItem() const;

        bool isIdentified() const;
        bool isDisabled() const; // item is broken
        bool isSocketed() const;
        bool isNew() const;
        bool isBadEquipped() const;
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
        const ItemType& getItemTypeHelper() const;
        bool getItemCode(std::array<std::uint8_t, 4>& strcode) const;
        const std::string& getItemTypeName() const;
        const std::string& getRuneLetter() const;
        std::uint8_t getGemApplyType() const;
        bool updateGem(const std::array<std::uint8_t, 4>& newgem);
        bool upgradeGem();
        bool upgradePotion();
        bool upgradeToFullRejuvenationPotion();
        std::uint8_t getQuestDifficulty() const;
        bool getEarAttributes(EarAttributes& attrib) const;
        bool getRequirements(ItemRequirements& req) const;
        bool getCategories(std::vector<std::string>& categories) const;
        bool getDimensions(ItemDimensions& dimensions) const;
        std::uint32_t getTotalItemSlots() const; // non-zero for belts and Horadric Cube only
        const std::string& getInvFile() const;
        std::string getTransformColor() const;

        // Extended information
        std::uint32_t getId() const;
        bool randomizeId();
        std::uint8_t getLevel() const;
        EnumItemQuality getQuality() const;
        bool getMagicalAffixes(MagicalAffixes& affixes) const;
        bool getRunewordAttributes(RunewordAttributes& attrib) const;
        std::string getPersonalizedName() const;
        std::uint8_t getTomeValue() const; // used in serialization
        std::uint16_t getSetItemId() const;
        bool getSetAttributes(SetAttributes& attrib) const;
        bool getCombinedSetAttributes(std::vector<MagicalAttribute>& attribs) const;
        bool getFullSetAttributes(std::vector<MagicalAttribute>& attribs) const;
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
        bool isTwoHandedWeapon() const;
        bool isOneOrTwoHandedWeapon() const; // Can weapon be carried in 1 or 2 hands by Barbarian?
        bool isShield() const;
        bool isThrownWeapon() const;
        bool isMissileWeapon() const;
        bool isMissile() const;
        bool isTome() const;
        bool isBodyPart() const;
        bool isStackable() const;
        bool isPotion() const;
        bool isGem() const;
        bool isUpgradableGem() const;
        bool isUpgradablePotion() const;
        bool isUpgradableItem() const;
        bool isUpgradableToFullRejuvenationPotion() const;
        bool isRune() const;
        bool isJewel() const;
        bool isCharm() const;
        bool isBelt() const;
        bool isBeltable() const;
        bool isQuestItem() const;
        bool isHoradricCube() const;
        bool isIndestructible() const;
        bool isUnusedItem() const;
        bool isSecondHand() const;
        bool isSocketFiller() const;
        bool isClassSpecific() const;
        std::optional<d2ce::EnumCharClass> getClass() const;
        bool hasUndeadBonus() const;
        bool canHaveSockets() const;
        bool canPersonalize() const;
        bool canMakeSuperior() const;
        bool canMakeEthereal() const;
        bool canRemoveEthereal() const;
        bool canAddMagicalAffixes() const;
        bool canAddRareAffixes() const;
        bool canEquip(EnumEquippedId equipId) const;
        bool canEquip(EnumEquippedId equipId, EnumCharClass charClass) const;
        bool canEquip(EnumEquippedId equipId, EnumCharClass charClass, const CharStats& cs) const;
        std::uint8_t getDisplayedSocketCount() const;
        std::uint8_t getSocketCountBonus() const;
        std::uint8_t getSocketCount() const;
        std::uint8_t getSocketedItemCount() const;
        std::uint8_t getMaxSocketCount() const;
        std::uint8_t getMaxSocketedCount() const;
        std::uint32_t getQuantity() const;
        std::uint32_t getMaxQuantity() const;
        bool setQuantity(std::uint32_t quantity);
        bool setMaxQuantity();
        std::uint16_t getDefenseRating() const;
        bool setDefenseRating(std::uint16_t ac);
        bool setMaxDefenseRating();
        bool getDurability(ItemDurability& attrib) const;
        bool setDurability(const ItemDurability& durability);
        bool fixDurability();
        bool setMaxDurability();
        bool getDamage(ItemDamage& damage) const;
        bool getRealmDataFlag() const;
        bool addMaxSocketCount();
        bool addSocket();
        bool removeEmptySockets();
        bool removeSocket(); // can only remove empty sockets
        bool setSocketCount(std::uint8_t numSockets);
        bool addPersonalization(const std::string& name);
        bool removePersonalization();
        bool setIndestructible();
        bool upgradeTier(const CharStats& cs);
        bool canEquipWith(const d2ce::Item& item, d2ce::EnumCharClass charClass) const;
        bool canEquipWith(const d2ce::Item& item, d2ce::EnumCharClass charClass, const CharStats& cs) const;
        bool canSocketItem(const d2ce::Item& socketFiller) const;
        bool canSocketItem(const d2ce::Item& socketFiller, std::uint32_t level) const;
        bool removeSocketedItems();
        std::vector<d2ce::RunewordType> getPossibleRunewords(bool bUseCurrentSocketCount = false, bool bExcludeServerOnly = true) const;
        std::vector<d2ce::RunewordType> getPossibleRunewords(std::uint32_t level, bool bUseCurrentSocketCount = false, bool bExcludeServerOnly = true) const;
        bool setRuneword(std::uint16_t id);
        bool getPossibleMagicalAffixes(std::vector<std::uint16_t>& prefixes, std::vector<std::uint16_t>& suffixes) const;
        bool getPossibleRareAffixes(std::vector<std::uint16_t>& prefixes, std::vector<std::uint16_t>& suffixes) const;
        bool setMagicalAffixes(const d2ce::MagicalAffixes& affixes);
        bool getPossibleSuperiorAttributes(std::vector<MagicalAttribute>& attribs) const;
        bool makeSuperior(const std::vector<MagicalAttribute>& attribs);
        bool makeSuperior();
        bool makeNormal();
        bool makeEthereal();
        bool removeEthereal();
        bool removeMagicalAffixes();
        bool setRareOrCraftedAttributes(RareAttributes& attrib);
        bool removeRareOrCraftedAttributes();
        bool exportItem(const std::filesystem::path& path) const;

        // Helper methods that return the text displayed on tooltips
        std::string getDisplayedItemName() const;
        std::string getDisplayedSocketedRunes() const;
        std::uint16_t getDisplayedDefenseRating(std::uint32_t charLevel) const;
        bool getDisplayedDurability(ItemDurability& durability, std::uint32_t charLevel) const;
        bool getDisplayedDamage(ItemDamage& damage, std::uint32_t charLevel) const;
        bool getDisplayedRequirements(ItemRequirements& req, std::uint32_t charLevel) const;
        std::string getDisplayedItemAttributes(EnumCharClass charClass, std::uint32_t charLevel) const;
        bool getDisplayedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;
        bool getDisplayedRunewordAttributes(RunewordAttributes& attrib, std::uint32_t charLevel) const;
        bool getDisplayedSetItemAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;
        bool getDisplayedFullSetAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;
        bool getDisplayedCombinedMagicalAttributes(std::vector<MagicalAttribute>& attribs, std::uint32_t charLevel) const;

    public:
        std::list<Item> SocketedItems; // socketed items

    protected:
        std::vector<MagicalAttribute> socketedMagicalAttributes;
        mutable std::vector<MagicalAttribute> cachedCombinedMagicalAttributes;
    };

    //---------------------------------------------------------------------------
    class Items
    {
        friend class Character;
        friend class Mercenary;
        friend class SharedStash;

    protected:
        EnumItemVersion Version = APP_ITEM_VERSION;
        mutable std::uint16_t GameVersion = APP_ITEM_GAME_VERSION;

        mutable std::uint32_t items_location = 0,
            corpse_location = 0,
            corpse_item_location = 0,
            merc_location = 0,   // Expansion character only
            golem_location = 0;  // Expansion character only

        mutable std::uint16_t NumOfItems = 0; // # of items (according to file) in inventory excluding 
                                              // gems in socketed items
        std::list<Item> Inventory;            // items in inventory

        std::vector<std::reference_wrapper<Item>> GPSs;       // inventory of all Gems, Potions or Skulls
        std::vector<std::reference_wrapper<Item>> Stackables; // inventory of all Stackable (includes some weapons)
        std::vector<std::reference_wrapper<Item>> Armor;      // inventory of all Armor
        std::vector<std::reference_wrapper<Item>> Weapons;    // inventory of all Weapons (includes stackable weapons)

        mutable std::map<d2ce::EnumItemLocation, std::map<d2ce::EnumAltItemLocation, std::vector<std::reference_wrapper<Item>>>> ItemLocationReference; // Iventory of items equipped or stored in the belt
        mutable std::map<d2ce::EnumItemLocation, std::map<d2ce::EnumAltItemLocation, std::set<std::uint16_t>>> ItemLocationEmptySpots; // a set of empty spots on body or belt

        bool HasHoradricCube = false;
        bool HasBeltEquipped = false;
        size_t EquippedBeltSlots = 0;
        bool IsSharedStash = false;

        mutable CorpseHeader CorpseInfo;
        mutable std::uint16_t NumOfCorpseItems = 0; // # of items included in the Corpse section (according to file), non-zero if you are currently dead
        mutable std::list<Item> CorpseItems;      // items on our Corpse

        // Expansion Character data
        mutable std::uint16_t NumOfMercItems = 0;  // # of Mercenary items (according to file)
        mutable std::list<Item> MercItems;         // items mercenary is currently wearing.
        mutable std::uint64_t MercId_v100 = 0;     // 1.00 - 1.08

        mutable std::uint8_t HasGolem = 0;         // Necromancer only, non-0 if you have a Golem
        mutable std::list<Item> GolemItem;         // Item for the Golem (only one item, but a list to keep memory stable)

        std::list<Item>& BufferItems;       // Buffer for items not in any inventory yet

        bool update_locations = true;
        mutable bool isMercHired = false;

    private:
        std::list<Item> Buffer;
        const Item* LastItemMoved = nullptr;
        size_t LastItemIdx = MAXSIZE_T;

    private:
        void findItems();
        void findSharedStashItems();

        bool readItems(std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::list<Item>& items);
        bool readSharedStashPage(std::FILE* charfile, std::uint32_t& location, std::uint16_t& numItems, std::list<Item>& items);
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

        bool writeCorpseItems(std::FILE* charfile) const;
        bool writeMercItems(std::FILE* charfile) const;
        bool writeGolemItem(std::FILE* charfile) const;

        bool readItems(const Character& charInfo, std::FILE* charfile);
        bool readSharedStashPage(EnumCharVersion version, std::FILE* charfile);
        bool readItems(const Json::Value& root, bool bSerializedFormat, const Character& charInfo, std::FILE* charfile);
        bool writeItems(std::FILE* charfile, bool isExpansion = false, bool hasMercID = false) const;
        bool writeSharedStashPage(std::FILE* charfile) const;

        void itemsAsJson(Json::Value& parent, std::uint32_t charLevel, EnumItemVersion version, bool bSerializedFormat = false) const;
        void itemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        void corpseItemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        bool mercItemsAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        bool golemItemAsJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;
        bool itemBonusesAsJson(Json::Value& parent, bool bSerializedFormat = false) const;
        void asJson(Json::Value& parent, std::uint32_t charLevel, EnumCharVersion version, bool bSerializedFormat = false) const;
        void asJson(Json::Value& parent, std::uint32_t charLevel, bool bSerializedFormat = false) const;

        bool isExpansionItems() const;

        void verifyBeltSlots();
        void verifyHoradricCube();

    public:
        Items();
        Items(std::list<Item>& bufferItems);
        Items(const Items& other);
        ~Items();

        Items& operator=(const Items& other);
        Items& operator=(Items&& other) noexcept;

        void swap(Items& other);

        void clear();

        EnumItemVersion getDefaultItemVersion() const;
        std::uint16_t getDefaultGameVersion() const;
        bool getItemLocationDimensions(EnumItemLocation locationId, EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const;
        bool getItemLocationDimensions(EnumItemLocation locationId, ItemDimensions& dimensions) const;
        bool getItemLocationDimensions(EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const;

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

        size_t upgradeGems(ItemFilter filter = ItemFilter());
        size_t upgradePotions(ItemFilter filter = ItemFilter());
        size_t upgradeRejuvenationPotions(ItemFilter filter = ItemFilter());
        size_t upgradeTierAllItems(const d2ce::Character& charInfo, ItemFilter filter = ItemFilter());
        size_t convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, ItemFilter filter = ItemFilter());
        size_t fillAllStackables(ItemFilter filter = ItemFilter());
        size_t repairAllItems(ItemFilter filter = ItemFilter());
        size_t maxDurabilityAllItems(ItemFilter filter = ItemFilter());
        size_t maxSocketCountAllItems(ItemFilter filter = ItemFilter());
        size_t setMaxDefenseRatingAllItems(ItemFilter filter = ItemFilter());
        size_t setIndestructibleAllItems(ItemFilter filter = ItemFilter());
        size_t setSuperiorAllItems(ItemFilter filter = ItemFilter());
        bool addItem(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
        bool addItem(EnumItemLocation locationId, std::array<std::uint8_t, 4>& strcode);
        bool addItem(EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
        size_t fillEmptySlots(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
        size_t fillEmptySlots(EnumItemLocation locationId, std::array<std::uint8_t, 4>& strcode);
        size_t fillEmptySlots(EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
        bool importItem(const d2ce::Item*& pImportedItem, bool bRandomizeId = false);
        bool importItem(const std::filesystem::path& path, const d2ce::Item*& pImportedItem, bool bRandomizeId = true);
        bool exportItem(d2ce::Item& item, const std::filesystem::path& path) const;

        bool setItemLocation(d2ce::Item& item, EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item* &pRemovedItem);
        bool setItemLocation(d2ce::Item& item, EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item* &pRemovedItem);
        bool setItemLocation(d2ce::Item& item, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item* &pRemovedItem);
        bool setItemLocation(d2ce::Item& item, const d2ce::Character& charInfo, EnumEquippedId equippedId, d2ce::EnumItemInventory invType, const d2ce::Item* &pRemovedItem);

        bool removeSocketedItems(d2ce::Item& item);
        bool setItemRuneword(d2ce::Item& item, std::uint16_t id);
        bool upgradeItemTier(d2ce::Item& item, const CharStats& cs);
        bool changeItemEthereal(d2ce::Item& item);

        bool getItemBonuses(std::vector<MagicalAttribute>& attribs) const;
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
        const std::list<Item>& getGolemItem() const;

        d2ce::Item removeItem(const d2ce::Item& item);

        bool canEquipItem(const d2ce::Item& item, EnumEquippedId equipId) const;
        bool canEquipItem(const d2ce::Item& item, EnumEquippedId equipId, EnumCharClass charClass, const CharStats& cs) const;

        static std::int64_t getMagicalAttributeValue(MagicalAttribute& attrib, std::uint32_t charLevel, size_t idx);
    };
    //---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------

