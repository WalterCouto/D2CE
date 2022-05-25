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

//---------------------------------------------------------------------------
#include "Constants.h"
#include "SkillConstants.h"
#include "DataTypes.h"
#include "ActsInfo.h"
#include "CharacterStats.h"
#include "Mercenary.h"
#include "Item.h"
#include "SharedStash.h"
#include <json/json.h>
#include <filesystem>

namespace d2ce
{
    //---------------------------------------------------------------------------
    class Character
    {
    public:
        enum class EnumCharSaveOp : std::uint8_t { NoSave, SaveWithBackup, SaveOnly, BackupOnly };

    protected:
        // the following variables are what would be found in the character file format
        // variables that are commented out means that no functions have been
        // declared that use them
        std::array<std::uint8_t, HEADER_LENGTH> Header = { 0x55, 0xAA, 0x55, 0xAA };
        std::uint32_t Version = 0;                                      // pos 4 in file, character file version
        std::uint32_t FileSize = 0;                                     // pos 8 (1.09+ only), file's size
        long Checksum = 0;                                              // pos 12 (1.09+ only), stores (possible) checksum
        std::uint32_t WeaponSet = 0;                                    // pos 16 (1.09+, otherwise pos 26 uint16_t)
        mutable BasicStats Bs;                                          // Name:   pos 267 (D2R 1.2+, pos 20 for 1.09 - 1.14d, otherwise pos 8),
                                                                        //         name includes terminating NULL
                                                                        // Status: pos 36 (1.09+, otherwise, pos 24), determines character status
                                                                        // Title:  pos 37 (1.09+, otherwise pos 25), character's title
                                                                        // Class:  pos 40 (1.09+, otherwise pos 34),
        std::uint8_t DisplayLevel = 1;                                  // pos 43 (1.09+, otherwise pos 36),
                                                                        // level displayed at character selection screen
        std::uint32_t Created = 0;                                      // pos 44 (1.09+ only), file date and time
                                                                        // using the Standard C time() function
        std::uint32_t LastPlayed = 0;                                   // pos 48 (1.09+ only), file date and time
                                                                        // using the Standard C time() function
        std::array<std::uint32_t, NUM_OF_SKILL_HOTKEYS> AssignedSkills; // pos 56 (1.09+, otherwise pos 70 and size 8)
        std::uint32_t LeftSkill = 0;                                    // pos 120 (1.09+, otherwise pos 86)
        std::uint32_t RightSkill = 0;                                   // pos 124 (1.09+, otherwise pos 87)
        std::uint32_t LeftSwapSkill = 0;                                // pos 128 (1.09+ only)
        std::uint32_t RightSwapSkill = 0;                               // pos 132 (1.09+ only)
        std::array<std::uint8_t, APPEARANCES_LENGTH> Appearances;       // pos 136 (1.09+, otherwise pos 38) Character menu appearance
        std::array<std::uint8_t, NUM_OF_DIFFICULTY> StartingAct;        // pos 168 (normal, nightmare, hell; used in 1.09+ only)
                                                                        // four MSBs value always 8 (hex, i.e. 0x80)
                                                                        // four least significant bits = which act is character saved at
        std::uint32_t MapID = 0;                                        // pos 171 (1.09+, otherwise 126)
                                                                       
        mutable Mercenary Merc; // Dead:       pos 177 (1.09+ only)
                                // Id:         pos 179 (1.09+ only)
                                // NameId:     pos 183 (1.09+ only)
                                // Type:       pos 185 (1.09+ only)
                                // Experience: pos 187 (1.09+ only), hireling's experience

        std::array<std::uint8_t, D2R_APPEARANCES_LENGTH> D2RAppearances; // pos 219 - 266 (1.15+ D2R only) Character menu appearance

        // normal act info starts at pos 345 (1.09+ only)
        // nightmare act info starts at pos 441 (1.09+ only)
        // hell act info starts at pos 537 (1.09+ only)
        // pos 643 (1.09+, otherwise pos 438), waypoints for normal level
        // pos 667 (1.09+, otherwise pos 462), waypoints for nightmare level
        // pos 691 (1.09+, otherwise pos 486), waypoints for hell level
        ActsInfo Acts;

        CharacterStats Cs; // pos 765 (1.09+, otherwise pos 562) 
        
        mutable Items  m_items;

        // the following variables are not part of the character file format
        std::FILE* m_charfile = nullptr;
        mutable std::filesystem::path m_d2sfilename, m_jsonfilename, m_tempfilename;
        std::error_code m_error_code;
        std::uint32_t m_filesize_location,
            m_checksum_location,
            m_name_location = 0,
            m_status_location = 0,
            m_class_location = 0,
            m_level_location = 0,
            m_starting_location = 0,
            m_assigned_skilled_location = 0,
            m_appearances_location = 0,
            m_difficulty_location = 0,
            m_mapid_location = 0,
            m_stats_header_location = 0;
        bool m_update_locations = true;
        bool m_bJsonSerializedFormat = false;

        SharedStash m_shared_stash;

        void calculateChecksum();

        void initialize();
        bool openD2S(const std::filesystem::path& path, bool validateChecksum = true);
        bool openJson(const std::filesystem::path& path);
        void readHeader();
        void readHeader(const Json::Value& root);
        bool isValidHeader() const;
        bool refresh(const Json::Value& root);
        void readBasicInfo();
        bool readBasicInfo(const Json::Value& root);
        bool readActs();
        bool readActs(const Json::Value& root);
        bool readStats();
        bool readStats(const Json::Value& root);
        bool readItems();
        bool readItems(const Json::Value& root);

        void writeBasicInfo() const;
        bool writeActs() const;
        bool writeStats() const;
        bool writeItems() const;
        void writeTempFile() const;

        void headerAsJson(Json::Value& parent, EnumCharVersion version, bool bSerializedFormat = false) const;
        void headerAsJson(Json::Value& parent, bool bSerializedFormat = false) const;
        void validateActs();

        std::string asJson(EnumCharVersion version, bool bSerializedFormat = false, EnumCharSaveOp saveOp = EnumCharSaveOp::SaveWithBackup); // utf-8

    public:
        Character();
        ~Character();

        // File operations
        bool open(const std::filesystem::path& path, bool validateChecksum = true);
        bool refresh();
        bool save(bool backup = true);
        bool saveAsVersion(EnumCharVersion version, EnumCharSaveOp saveOp = EnumCharSaveOp::SaveWithBackup);
        bool saveAsVersion(const std::filesystem::path& path, EnumCharVersion version, EnumCharSaveOp saveOp = EnumCharSaveOp::SaveWithBackup);
        bool saveAsD2s(EnumCharSaveOp saveOp = EnumCharSaveOp::SaveWithBackup);
        bool saveAsD2s(const std::filesystem::path& path, EnumCharSaveOp saveOp = EnumCharSaveOp::SaveWithBackup);
        void close();
        const std::filesystem::path& getPath() const;
        std::string asJson(bool bSerializedFormat = false, EnumCharSaveOp backup = EnumCharSaveOp::SaveWithBackup); // utf-8

        void setDefaultTxtReader();
        void setTxtReader(const ITxtReader& txtReader);
        const ITxtReader& getTxtReader() const;

        const std::string& getLanguage() const;
        const std::string& setLanguage(const std::string& lang) const;

        bool is_open() const;
        bool is_json() const;
        std::error_code getLastError() const;
        std::uint32_t getFileSize() const;

        // Mercenary Info
        bool hasMercenary() const;
        Mercenary& getMercenaryInfo() const;
        const std::list<d2ce::Item>& getMercItems() const;

        bool getMercItemBonuses(std::vector<MagicalAttribute>& attribs) const;
        bool getDisplayedMercItemBonuses(std::vector<MagicalAttribute>& attribs) const;
        std::uint16_t getCombinedMercDefenseRating() const;
        bool getCombinedMercDamage(BaseDamage& damage) const;

        // Corpse Items
        const std::list<d2ce::Item>& getCorpseItems() const;

        // Golem Info
        bool hasGolem() const;
        const std::list<d2ce::Item>& getGolemItem() const;

        // Character Stats
        void fillBasicStats(BasicStats& bs) const;
        void fillCharacterStats(CharStats& cs) const;
        void fillDisplayedCharacterStats(CharStats& cs) const;

        void updateBasicStats(BasicStats& bs);
        void updateCharacterStats(CharStats& cs);

        void resetStats(); // both skills and stats

        EnumCharVersion getVersion() const;
        const std::array<char, NAME_LENGTH>& getName() const;
        bitmask::bitmask<EnumCharStatus> getStatus() const;
        std::uint8_t getTitle() const;
        EnumCharTitle getTitleEnum() const;
        EnumDifficulty getTitleDifficulty() const;
        EnumAct getTitleAct() const;
        void ensureTitleAct(EnumAct act);
        EnumCharClass getClass() const;
        const std::string& getClassName() const;
        const std::string& getClassCode() const;
        EnumDifficulty getDifficultyLastPlayed() const;
        EnumAct getStartingAct() const;
        EnumAct getLastAct() const;
        std::uint8_t getNumActs() const;
        std::uint32_t getWeaponSet() const;

        bool isExpansionCharacter() const;
        void setIsExpansionCharacter(bool flag);
        bool isLadderCharacter() const;
        void setIsLadderCharacter(bool flag);
        bool isHardcoreCharacter() const;
        void setIsHardcoreCharacter(bool flag);
        bool isResurrectedCharacter() const;
        void setIsResurrectedCharacter(bool flag);
        bool isDeadCharacter() const;
        void setIsDeadCharacter(bool flag);
        bool isFemaleCharacter() const;
        bool isGameComplete() const;
        void setGameComplete();
        bool isDifficultyComplete(d2ce::EnumDifficulty diff) const;
        void setDifficultyComplete(d2ce::EnumDifficulty diff);
        void setNoDifficultyComplete();

        std::uint32_t getLevel() const;
        std::uint32_t getMaxLevel() const;
        std::uint32_t getExperience() const;
        std::uint32_t getMaxExperience() const;
        std::uint32_t getMinExperience(std::uint32_t level) const;
        std::uint32_t getMinExperienceLevel() const;
        std::uint32_t getNextExperience(std::uint32_t level) const;
        std::uint32_t getNextExperienceLevel() const;
        std::uint32_t getMaxGoldInBelt() const;
        std::uint32_t getMaxGoldInStash() const;
        std::uint32_t getStrength() const;
        std::uint32_t getMinStrength() const;
        std::uint32_t getMinEnergy() const;
        std::uint32_t getDexterity() const;
        std::uint32_t getMinDexterity() const;
        std::uint32_t getMinVitality() const;
        std::uint32_t getMaxHitPoints() const;
        std::uint32_t getMaxStamina() const;
        std::uint32_t getMaxMana() const;

        std::uint32_t getSkillPointsEarned() const;
        std::uint32_t getSkillPointsEarned(std::uint32_t level) const;
        std::uint32_t getLevelFromTotalSkillPoints() const;
        std::uint32_t getLevelFromSkillPointsEarned(std::uint32_t earned) const;
        std::uint32_t getTotalStartStatPoints() const;
        std::uint32_t getTotalStatPoints() const;
        std::uint32_t getStatPointsUsed() const;
        std::uint32_t getStatPointsEarned() const;
        std::uint32_t getStatPointsEarned(std::uint32_t level) const;
        std::uint32_t getLevelFromTotalStatPoints() const;
        std::uint32_t getLevelFromStatPointsEarned(std::uint32_t earned) const;
        std::uint32_t getLevelFromExperience() const;
        std::uint32_t getLevelFromExperience(std::uint32_t experience) const;
        // Quests
        const ActsInfo& getQuests();
        void updateQuests(const ActsInfo& qi);

        // Waypoints
        std::uint64_t getWaypoints(d2ce::EnumDifficulty difficulty) const;
        void setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue);

        // Skills
        std::array<std::uint8_t, NUM_OF_SKILLS>& getSkills();
        void updateSkills(const std::array<std::uint8_t, NUM_OF_SKILLS>& updated_skills, std::uint32_t skillChoices);

        std::uint32_t getTotalSkillPoints() const;
        std::uint32_t getSkillPointsUsed() const;
        std::uint32_t getSkillChoices() const;

        bool areSkillsMaxed() const;
        void maxSkills();
        void resetSkills();
        void clearSkillChoices();

        bool getSkillBonusPoints(std::vector<std::uint16_t>& points) const;

        // Items
        size_t getNumberOfItems() const;

        size_t getNumberOfEquippedItems() const;
        const std::vector<std::reference_wrapper<Item>>& getEquippedItems() const;

        bool getHasBeltEquipped() const;
        size_t getMaxNumberOfItemsInBelt() const;
        size_t getNumberOfItemsInBelt() const;
        const std::vector<std::reference_wrapper<Item>>& getItemsInBelt() const;

        size_t getNumberOfItemsInInventory() const;
        const std::vector<std::reference_wrapper<Item>>& getItemsInInventory() const;

        size_t getNumberOfItemsInStash() const;
        const std::vector<std::reference_wrapper<Item>>& getItemsInStash() const;

        bool getHasHoradricCube() const;
        size_t getNumberOfItemsInHoradricCube() const;
        const std::vector<std::reference_wrapper<Item>>& getItemsInHoradricCube() const; 
        
        bool getItemLocationDimensions(EnumItemLocation locationId, EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const;
        bool getItemLocationDimensions(EnumItemLocation locationId, ItemDimensions& dimensions) const;
        bool getItemLocationDimensions(EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const;

        size_t getNumberOfArmor() const;
        size_t getNumberOfWeapons() const;
        size_t repairAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());
        size_t maxDurabilityAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());
        size_t setIndestructibleAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());
        size_t maxSocketCountAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());

        bool setItemLocation(d2ce::Item& item, EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem);
        bool setItemLocation(d2ce::Item& item, EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem);
        bool setItemLocation(d2ce::Item& item, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem);
        bool setItemLocation(d2ce::Item& item, EnumEquippedId equippedId, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem);

        bool setItemRuneword(d2ce::Item& item, std::uint16_t id);

        size_t getNumberOfStackables() const;
        size_t fillAllStackables(d2ce::ItemFilter filter = d2ce::ItemFilter());

        size_t getNumberOfGPSs() const;
        const std::vector<std::reference_wrapper<Item>>& getGPSs();
        size_t convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, ItemFilter filter = ItemFilter());
        bool anyUpgradableGems() const;
        size_t upgradeGems(ItemFilter filter = ItemFilter());
        bool anyUpgradablePotions() const;
        size_t upgradePotions(ItemFilter filter = ItemFilter());
        bool anyUpgradableRejuvenations() const;
        size_t upgradeRejuvenationPotions(ItemFilter filter = ItemFilter());

        bool addItem(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
        bool addItem(EnumItemLocation locationId, std::array<std::uint8_t, 4>& strcode);
        bool addItem(EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
        bool importItem(const std::filesystem::path& path, const d2ce::Item*& pImportedItem, bool bRandomizeId = true);
        size_t fillEmptySlots(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
        size_t fillEmptySlots(EnumItemLocation locationId, std::array<std::uint8_t, 4>& strcode);
        size_t fillEmptySlots(EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);

        bool removeSocketedItems(d2ce::Item& item);

        bool getItemBonuses(std::vector<MagicalAttribute>& attribs) const;
        bool getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs) const;

        // Shared Stash Info
        SharedStash& getSharedStash();
        bool hasSharedStash() const;
    };
}
//---------------------------------------------------------------------------