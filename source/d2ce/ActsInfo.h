/*
    Diablo II Character Editor
    Copyright (C) 2021-2023 Walter Couto

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
#include <json/json.h>

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
            std::uint16_t Intro = 0;                                                // has player read/heard intro
            std::array<std::uint16_t, NUM_OF_QUESTS> Quests = { 0, 0, 0, 0, 0, 0 }; // actual quests
            std::uint16_t Completed = 0;
        };

        struct ActVInfo // works for Act V
        {
            std::uint16_t Intro = 0;                                                // has player read/heard intro
            std::array<std::uint16_t, 2> Padding1 = { 0, 0 };
            std::array<std::uint16_t, NUM_OF_QUESTS> Quests = { 0, 0, 0, 0, 0, 0 }; // actual quests

            // Not part of Act V quests
            std::uint8_t ResetStats = 0;
            std::uint8_t Completed = 0;                                   // Difficulty has been completed
            std::array<std::uint16_t, 6> Padding2 = { 0, 0, 0, 0, 0, 0 }; // 32 byte boundary (2 16 byte runs)
        };

        struct ActsInfoData // Each difficulty level contain this structure
        {
            std::array<ActInfo, NUM_OF_ACTS - 1> Act; // Act I-IV are 16 bytes each
            ActVInfo ActV;                            // Act V is 32 bytes in size
        };

        // normal act info starts at pos 345 (1.09+ only)
        // nightmare act info starts at pos 441 (1.09+ only)
        // hell act info starts at pos 537 (1.09+ only)
        mutable std::array <ActsInfoData, NUM_OF_DIFFICULTY> Acts;
        std::array<std::array<std::uint8_t, 28>, NUM_OF_DIFFICULTY> Quests_extraBits = { {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }} };
        std::array<std::uint8_t, 12> Quests_unknown = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::array<std::uint8_t, 4> Quests_version = { 0x06, 0x00, 0x00, 0x00 };

        // pos 438 (pre-1.09, otherwise pos 643), waypoints for normal level
        // pos 462 (pre-1.09, otherwise pos 667), waypoints for nightmare level
        // pos 486 (pre-1.09, otherwise pos 691), waypoints for hell level
        std::array<std::uint16_t, NUM_OF_DIFFICULTY> Waypoints_unknown = { 0x0102, 0x0102, 0x0102 };
        std::array<std::uint64_t, NUM_OF_DIFFICULTY> Waypoints = { 0, 0, 0 };
        std::array<std::array<std::uint8_t, 14>, NUM_OF_DIFFICULTY> Waypoints_extraBits = { {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }} };
        std::array<std::uint8_t, 4> Waypoints_version = { 0x01, 0x00, 0x00, 0x00 };

        // NPC
        std::array<std::uint64_t, NUM_OF_DIFFICULTY> NPCIntroductions = { 0, 0, 0 };
        std::array<std::uint64_t, NUM_OF_DIFFICULTY> NPCCongrats = { 0, 0, 0 };

        mutable bool QuestsDataCorrected = false;

    private:
        Character& CharInfo;

    private:
        std::uint16_t& getQuestDataRef(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const;

        bool readQuests(std::FILE* charfile);
        void applyJsonQuest(const Json::Value& questRoot, bool bSerializedFormat, EnumDifficulty diff, EnumAct act, std::uint8_t quest);
        void applyJsonActIntro(const Json::Value& actIntroRoot, bool bSerializedFormat, EnumDifficulty diff, EnumAct act);
        void applyJsonActComplete(const Json::Value& actCompleteRoot, bool bSerializedFormat, EnumDifficulty diff, EnumAct act);
        void applyJsonQuestAct(const Json::Value& questActRoot, bool bSerializedFormat, EnumDifficulty diff, EnumAct act);
        void applyJsonQuestDifficulty(const Json::Value& questDiffRoot, bool bSerializedFormat, EnumDifficulty diff);
        void applyJsonQuests(const Json::Value& questsRoot, bool bSerializedFormat);
        bool readQuests(const Json::Value& questsRoot, bool bSerializedFormat);
        bool readWaypoints(std::FILE* charfile);
        void applyJsonWaypointAct(const Json::Value& waypointActRoot, bool bSerializedFormat, EnumDifficulty diff, EnumAct act);
        void applyJsonWaypointDifficulty(const Json::Value& waypointDiffRoot, bool bSerializedFormat, EnumDifficulty diff);
        void applyJsonWaypoints(const Json::Value& waypointsRoot, bool bSerializedFormat);
        bool readWaypoints(const Json::Value& waypointsRoot, bool bSerializedFormat);
        bool readNPC(std::FILE* charfile);
        void applyJsonNPCsDifficulty(const Json::Value& npcsDiffRoot, bool bSerializedFormat, EnumDifficulty diff);
        void applyJsonNPCs(const Json::Value& npcsRoot, bool bSerializedFormat);
        bool readNPC(const Json::Value& npcsRoot, bool bSerializedFormat);
        bool writeQuests(std::FILE* charfile) const;
        bool writeWaypoints(std::FILE* charfile) const;
        bool writeNPC(std::FILE* charfile) const;

        void validateAct(EnumDifficulty diff, EnumAct act);

    protected:
        bool readActs(std::FILE* charfile);
        bool readActs(const Json::Value& root, bool bSerializedFormat);
        bool writeActs(std::FILE* charfile) const;

        void questsAsJson(Json::Value& parent, bool bSerializedFormat = false) const;
        std::string getQuestsJsonName(EnumDifficulty diff, bool bSerializedFormat = false) const;
        std::string getDifficultyJsonName(EnumDifficulty diff, bool bSerializedFormat = false) const;
        std::string getActJsonName(EnumAct act, bool bSerializedFormat = false) const;
        std::string getQuestJsonName(EnumAct act, std::uint8_t quest, bool bSerializedFormat = false) const;
        std::string getQuestBitJsonName(std::uint8_t bit, bool& isOptional, bool bSerializedFormat = false) const;

        void waypointsAsJson(Json::Value& parent, bool bSerializedFormat = false) const;
        std::string getWaypointJsonName(EnumAct act, std::uint8_t waypoint, bool bSerializedFormat = false) const;

        void npcAsJson(Json::Value& parent, bool bSerializedFormat = false) const;
        std::string getNpcJsonName(std::uint8_t npc, bool& isOptional, bool bSerializedFormat = false) const;

        std::uint16_t getActIntroducedData(EnumDifficulty diff, EnumAct act) const;
        void setActIntroducedData(EnumDifficulty diff, EnumAct act, std::uint16_t value) const;
        std::uint16_t getActCompletedData(EnumDifficulty diff, EnumAct act) const;
        void setActCompletedData(EnumDifficulty diff, EnumAct act, std::uint16_t value) const;
        std::uint16_t getActVResetStatCompletedData(EnumDifficulty diff) const;

        void validateActs();
        bool getActCompletedStrict(EnumDifficulty diff, EnumAct act) const;
        void resetActs(EnumDifficulty diff);

        void calculateChecksum(long& checksum, std::uint8_t& overflow);

    protected:
        ActsInfo(Character& charInfo);

    public:
        ActsInfo(const ActsInfo& other);
        ~ActsInfo();

        ActsInfo& operator=(const ActsInfo& other);
        ActsInfo& operator=(ActsInfo&& other) noexcept;

        void swap(ActsInfo& other);

        void clear();

        size_t getByteSize() const; // number of bytes to store all act data

        // Act info
        bool getActIntroduced(EnumDifficulty diff, EnumAct act) const;
        bool getActCompleted(EnumDifficulty diff, EnumAct act) const; 
        bool getActYetToStart(EnumDifficulty diff, EnumAct act) const; // has the act been introduced or a quest started?
        bool getActQuestYetToStart(EnumDifficulty diff, EnumAct act) const; // has any quest started yet?
        void resetAct(EnumAct act); // resets the act for the highest difficulty allowed if act after has no quests started
        void updateAct(EnumAct act); // makes sure the act data is correct for the highest difficulty allowed

        // Act Quest info
        std::string getQuestName(EnumAct act, std::uint8_t quest) const;
        std::uint16_t getQuestData(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const;
        void setQuestData(EnumDifficulty diff, EnumAct act, std::uint8_t quest, std::uint16_t questValue);
        std::string getQuestNotes(EnumDifficulty diff, EnumAct act, std::uint8_t quest);

        bool getQuestIsRequired(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const;
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
        bool readTheScrollOfResistance(EnumDifficulty diff) const;

        std::uint16_t getLifePointsEarned() const;
        std::uint16_t getSkillPointsEarned() const;
        std::uint16_t getStatPointsEarned() const;

        void updateQuests(const ActsInfo& qi); // only update quests from acts
        void updateActs(const ActsInfo& qi); // only update both quests and waypoints from acts

        // Waypoints
        std::uint64_t getWaypoints(d2ce::EnumDifficulty difficulty) const;
        void setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue);
        bool getWaypointActivated(EnumDifficulty diff, EnumAct act, std::uint8_t waypoint) const;
        bool setWaypointActivated(EnumDifficulty diff, EnumAct act, std::uint8_t waypoint, bool flag);

        // NPCs
        std::uint64_t getNpcIntroductions(d2ce::EnumDifficulty difficulty) const;
        std::uint64_t getNpcCongrats(d2ce::EnumDifficulty difficulty) const;
        void setNpcIntroductions(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue);
        void setNpcCongrats(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue);
    };
}
//---------------------------------------------------------------------------

