/*
    Diablo II Character Editor
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

#include "Constants.h"
#include "DataTypes.h"

namespace d2ce
{
    //---------------------------------------------------------------------------
    class ActsInfo
    {
        friend class Character;

    protected:
        // works for Act I - III 
        // works for Act IV but only indexes 0, 1 and 2 are valid for quest data and the "Completed" flag is at index 3, the rest is padding
        struct ActInfo
        {
            uint16_t Intro = 0;                     // has player read/heard intro
            uint16_t Quests[NUM_OF_QUESTS] = { 0 }; // actual quests
            uint16_t Completed = 0;
        };

        struct ActVInfo // works for Act V
        {
            uint16_t Intro = 0;             // has player read/heard intro
            uint16_t Padding1[2];
            uint16_t Quests[NUM_OF_QUESTS] = { 0 }; // actual quests

            // Not part of Act V quests
            std::uint8_t ResetStats = 0;
            std::uint8_t Padding2 = 0;
            uint16_t Padding3[6] = { 0 };      // 32 byte boundary (2 16 byte runs)
        };

        struct ActsInfoData // Each difficulty level contain this structure
        {
            ActInfo Act[NUM_OF_ACTS - 1]; // Act I-IV are 16 bytes each
            ActVInfo ActV;                // Act V is 32 bytes in size
        };


        // normal act info starts at pos 345 (1.09+ only)
        // nightmare act info starts at pos 441 (1.09+ only)
        // hell act info starts at pos 537 (1.09+ only)
        mutable ActsInfoData Acts[NUM_OF_DIFFICULTY];

        // pos 438 (pre-1.09, otherwise pos 643), waypoints for normal level
        // pos 462 (pre-1.09, otherwise pos 667), waypoints for nightmare level
        // pos 486 (pre-1.09, otherwise pos 691), waypoints for hell level
        std::uint16_t Waypoints_unknown[NUM_OF_DIFFICULTY] = { 0x0102, 0x0102, 0x0102 };
        std::uint64_t Waypoints[NUM_OF_DIFFICULTY] = { 0 };
        std::uint16_t Waypoints_extraBits[14][NUM_OF_DIFFICULTY] = { 0 };

        EnumCharVersion Version = APP_CHAR_VERSION; // Version for Character file

        std::uint32_t quests_start_location = 0, 
            quests_location = 0,
            waypoints_start_location = 0,
            waypoints_location = 0;
        bool update_locations = true;
        std::uint8_t quests_version[4] = { 0x06, 0x00, 0x00, 0x00 };
        std::uint8_t waypoints_version[4] = { 0x01, 0x00, 0x00, 0x00 };

    private:
        uint16_t& getQuestDataRef(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const;

        bool readQuests(std::FILE* charfile);
        bool readWaypoints(std::FILE* charfile);
        bool writeQuests(std::FILE* charfile);
        bool writeWaypoints(std::FILE* charfile);

    protected:
        bool readActs(EnumCharVersion version, std::FILE* charfile);
        bool writeActs(std::FILE* charfile);

    public:
        ActsInfo();
        ActsInfo(const ActsInfo& other);
        ~ActsInfo();

        ActsInfo& operator=(const ActsInfo& other);
        ActsInfo& operator=(ActsInfo&& other) noexcept;

        void clear();

        // Act info
        bool getActIntroduced(EnumDifficulty diff, EnumAct act) const;
        bool getActCompleted(EnumDifficulty diff, EnumAct act) const;

        // Act Quest info
        uint16_t getQuestData(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const;
        void setQuestData(EnumDifficulty diff, EnumAct act, std::uint8_t quest, uint16_t questValue);
        std::string getQuestNotes(EnumDifficulty diff, EnumAct act, std::uint8_t quest);

        bool getQuestYetToStart(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const;
        bool getQuestStarted(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const;
        void startQuest(EnumDifficulty diff, EnumAct act, std::uint8_t quest);
        bool getQuestCompleted(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const;
        void completeQuest(EnumDifficulty diff, EnumAct act, std::uint8_t quest);
        void resetQuest(EnumDifficulty diff, EnumAct act, std::uint8_t quest);

        bool getStatsReset(EnumDifficulty diff) const;
        void setStatsReset(EnumDifficulty diff);
        void clearStatsReset(EnumDifficulty diff);

        bool getMooMooFarmComplete(EnumDifficulty diff) const;
        void setMooMooFarmComplete(EnumDifficulty diff);
        void clearMooMooFarmComplete(EnumDifficulty diff);

        bool drankPotionOfLife(EnumDifficulty diff) const;

        uint16_t getLifePointsEarned() const;
        uint16_t getSkillPointsEarned() const;
        uint16_t getStatPointsEarned() const;

        void updateQuests(const ActsInfo& qi); // only update quests from acts
        void updateActs(const ActsInfo& qi); // only update both quests and waypoints from acts

        // Waypoints
        std::uint64_t getWaypoints(d2ce::EnumDifficulty difficulty) const;
        void setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue);
    };
}
//---------------------------------------------------------------------------

