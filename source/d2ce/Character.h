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

//---------------------------------------------------------------------------
#include "Constants.h"
#include "SkillConstants.h"
#include "DataTypes.h"
#include "ActsInfo.h"
#include "CharacterStats.h"
#include "Item.h"

namespace d2ce
{
    //---------------------------------------------------------------------------
    class Character
    {
    protected:
        // the following variables are what would be found in the character file format
        // variables that are commented out means that no functions have been
        // declared that use them
        std::uint8_t Header[HEADER_LENGTH];
        std::uint32_t Version = 0;           // pos 4 in file, character file version
        std::uint32_t FileSize = 0;          // pos 8 (1.09+ only), file's size
        long Checksum = 0;                   // pos 12 (1.09+ only), stores (possible) checksum
        std::uint32_t WeaponSet = 0;         // pos 16 (1.09+ only)
        BasicStats Bs;                       // Name:   pos 8 (pre-1.09, otherwise pos 20),
                                             //         name includes terminating NULL
                                             // Status: pos 24 (pos 36, 1.09+ only), determines character status
                                             // Title:  pos 25 (pos 37, 1.09+ only), character's title
                                             // Class:  pos 34 (pre-1.09, otherwise pos 40),
        std::uint8_t DisplayLevel = 1;       // pos 36 (pre-1.09, otherwise pos 43),
                                             // level displayed at character selection screen
     //   time_t FileDateTime;               // pos 48 (1.09+ only), file date and time
                                             // using the Standard C time() function
        std::uint8_t DifficultyAndAct = 0;   // pos 88 (used in pre-1.09 only)
                                             // four most significant bits = difficulty level last played,
                                             // four least significant bits = which act is character saved at
        std::uint8_t StartingAct[NUM_OF_DIFFICULTY]; // pos 168 (normal, nightmare, hell; used in 1.09+ only)
                                                     // four MSBs value always 8 (hex, i.e. 0x80)
                                                     // four least significant bits = which act is character saved at
    //   std::uint32_t HirelingExperience;   // pos 187 (1.09+ only), hireling's experience


        // normal act info starts at pos 345 (1.09+ only)
        // nightmare act info starts at pos 441 (1.09+ only)
        // hell act info starts at pos 537 (1.09+ only)
        // pos 438 (pre-1.09, otherwise pos 643), waypoints for normal level
        // pos 462 (pre-1.09, otherwise pos 667), waypoints for nightmare level
        // pos 486 (pre-1.09, otherwise pos 691), waypoints for hell level
        ActsInfo Acts;

        CharacterStats Cs; // pos 765 (pos 562 pre-1.09) 

        std::uint8_t Skills[NUM_OF_SKILLS];
        
        mutable Items items;

        // the following variables are not part of the character file format
        std::FILE* charfile = nullptr;
        std::string filename, tempfilename;
        std::error_code error_code;
        std::uint32_t filesize_location,
            checksum_location,
            name_location = 0,
            class_location = 0,
            level_location = 0,
            starting_location = 0,
            stats_header_location = 0,
            skills_location = 0;
        bool update_locations = true;

        void calculateChecksum();

        void initialize();
        void readHeader();
        bool isValidHeader() const;
        void readBasicInfo();
        bool readActs();
        bool readStats();
        bool readSkills();
        bool readItems();

        void writeName();
        void writeBasicStats();
        bool writeActs();
        bool writeStats();
        bool writeSkills();
        bool writeItems();
        void writeTempFile();

    public:
        Character();
        ~Character();

        // File operations
        bool open(const char* filename, bool validateChecksum = true);
        bool refresh();
        bool save();
        void close();

        bool is_open() const;
        std::error_code getLastError() const;

        // Character Stats
        void fillBasicStats(BasicStats& bs);
        void fillCharacterStats(CharStats& cs);

        void updateBasicStats(const BasicStats& bs);
        void updateCharacterStats(const CharStats& cs);

        EnumCharVersion getVersion() const;
        const char (&getName())[NAME_LENGTH];
        bitmask::bitmask<EnumCharStatus> getStatus() const;
        bitmask::bitmask<EnumCharTitle> getTitle() const;
        EnumCharClass getClass() const;
        EnumDifficulty getDifficultyLastPlayed();
        EnumAct getStartingAct() const;

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

        std::uint32_t getLevel() const;
        std::uint32_t getExperience() const;

        std::uint16_t getLifePointsEarned() const;
        std::uint32_t getSkillPointsEarned() const;
        std::uint32_t getSkillPointsEarned(std::uint32_t level) const;
        std::uint32_t getLevelFromSkillPointsEarned(std::uint32_t earned) const;
        std::uint32_t getStatPointsEarned() const;
        std::uint32_t getStatPointsEarned(std::uint32_t level) const;
        std::uint32_t getLevelFromStatPointsEarned(uint32_t earned) const;

        // Quests
        const ActsInfo& getQuests();
        void updateQuests(const ActsInfo& qi);

        // Waypoints
        std::uint64_t getWaypoints(d2ce::EnumDifficulty difficulty) const;
        void setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue);

        // Skills
        std::uint8_t(&getSkills())[NUM_OF_SKILLS];
        void updateSkills(const std::uint8_t (&updated_skills)[NUM_OF_SKILLS]);

        std::uint32_t getSkillPointUsed() const;
        std::uint32_t getSkillChoices() const;

        bool areSkillsMaxed() const;
        void maxSkills();
        void resetSkills();
        void clearSkillChoices();

        // Items
        size_t getNumberOfItems() const;

        size_t getNumberOfArmor() const;
        size_t getNumberOfWeapons() const;
        size_t maxDurabilityAllItems();

        size_t getNumberOfStackables() const;
        size_t fillAllStackables();

        size_t getNumberOfGPSs() const;
        const std::vector<std::reference_wrapper<Item>>& getGPSs();
        size_t convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4]);
        bool anyUpgradableGems() const;
        size_t upgradeGems();
        bool anyUpgradablePotions() const;
        size_t upgradePotions();
        bool anyUpgradableRejuvenations() const;
        size_t upgradeRejuvenationPotions();
    };
}
//---------------------------------------------------------------------------