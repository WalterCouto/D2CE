/*
    Diablo II Character Editor
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

#include "pch.h"
#include <cstdint>
#include <locale>
#include "ActsInfo.h"
#include "WaypointConstants.h"

namespace d2ce
{
    constexpr std::array<std::uint8_t, 4> QUESTS_MARKER = { 0x57, 0x6F, 0x6F, 0x21 }; // alternatively "Woo!"
    constexpr std::array<std::uint8_t, 4> QUESTS_VERSION = { 0x06, 0x00, 0x00, 0x00 };
    constexpr std::array<std::uint8_t, 2> QUESTS_SIZE_MARKER = { 0x2A, 0x01 };

    constexpr std::uint32_t MIN_START_QUEST_POS = 335;

    constexpr std::array<std::uint8_t, 2> WAYPOINTS_MARKER = { 0x57, 0x53 };          // alternatively "WS"
    constexpr std::array<std::uint8_t, 4> WAYPOINTS_VERSION = { 0x01, 0x00, 0x00, 0x00 };
    constexpr std::array<std::uint8_t, 2> WAYPOINTS_SIZE_MARKER = { 0x50, 0x00 };

    constexpr std::uint32_t MIN_START_WAYPOINTS_POS = 633;

    constexpr std::array<std::uint8_t, 2> NPC_MARKER = { 0x01, 0x77 };
    constexpr std::array<std::uint8_t, 2> NPC_SIZE_MARKER = { 0x34, 0x00 };

    constexpr std::uint32_t MIN_START_NPC_POS = 713;

    constexpr std::uint16_t questNotStarted = 0x0000;
    constexpr std::uint16_t questStarted = 0x0004;
    constexpr std::uint16_t questCompletedFlag = 0x9001;

    static const std::array<std::array<std::uint16_t, d2ce::NUM_OF_QUESTS>, d2ce::NUM_OF_ACTS> questCompleted = { {
        {0x1001,0x101d,0x900d,0x101d,0x1055,0x101d}, // Act I
        {0x101d,0x1c39,0x100d,0x1181,0x1005,0x1e25}, // Act II
        {0x1001,0x10fd,0x11d9,0x1001,0x100d,0x1871}, // Act III
        {0x1001,0x1301,0x1001,0,0,0},                // Act IV
        {0x9021,0x1001,0x178d,0x901d,0x132d,0x169d}  // Act V
    } };

    constexpr std::uint16_t questMooMooFarm = 0x0400;
}

std::uint16_t& d2ce::ActsInfo::getQuestDataRef(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const
{
    static std::uint16_t dummy = 0;

    if (quest >= (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS))
    {
        // should not happen
        return dummy;
    }

    auto actNum = static_cast<std::underlying_type_t<EnumAct>>(act);
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    if (actNum < 4)
    {
        return Acts[diffNum].Act[actNum].Quests[quest];
    }

    if (actNum == 4)
    {
        return Acts[diffNum].ActV.Quests[quest];
    }

    // should not happen
    return dummy;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readQuests(std::FILE* charfile)
{
    if (update_locations)
    {
        // find stats location
        quests_start_location = 0;
        quests_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (Version >= EnumCharVersion::v109)
        {
            if (cur_pos < (long)MIN_START_QUEST_POS)
            {
                cur_pos = MIN_START_QUEST_POS;
                std::fseek(charfile, cur_pos, SEEK_SET);
            }
        }
        else
        {
            if (cur_pos < MIN_START_POS)
            {
                cur_pos = MIN_START_POS;
                std::fseek(charfile, cur_pos, SEEK_SET);
            }
        }

        while (!feof(charfile))
        {
            quests_start_location = std::ftell(charfile);
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != QUESTS_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != QUESTS_MARKER[1])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != QUESTS_MARKER[2])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != QUESTS_MARKER[3])
            {
                continue;
            }

            // found quests marker (0x216F6F57). 
            std::fread(quests_version.data(), quests_version.size(), 1, charfile);
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != QUESTS_SIZE_MARKER[0])
            {
                return false;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != QUESTS_SIZE_MARKER[1])
            {
                return false;
            }

            // found size marker (0x012A)
            quests_location = std::ftell(charfile);
            break;
        }
    }

    if (quests_location == 0)
    {
        quests_start_location = 0;
        return false;
    }

    std::fseek(charfile, quests_location, SEEK_SET);

    if (feof(charfile))
    {
        return false;
    }

    if (Version >= EnumCharVersion::v107)
    {
        return std::fread(Acts.data(), Acts.size() * sizeof(ActsInfoData), 1, charfile) == 1 ? true : false;
    }

    // Only copy 4 acts of Quest data for each difficulty
    for (std::uint32_t i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        if (feof(charfile))
        {
            return false;
        }

        if (std::fread(Acts[i].Act.data(), Acts[i].Act.size() * sizeof(ActInfo), 1, charfile) != 1)
        {
            return false;
        }

        std::fseek(charfile, 28, SEEK_CUR);
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonQuest(const Json::Value& questRoot, bool bSerializedFormat, bool isExpansion, EnumDifficulty diff, EnumAct act, std::uint8_t quest)
{
    if (questRoot.isNull())
    {
        return;
    }

    if (act == EnumAct::V && !isExpansion)
    {
        return;
    }

    std::string key;
    std::string name;
    std::bitset<16> questValue = 0;
    bool isOptional = false;
    auto iter_end = questRoot.end();
    for (auto iter = questRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
        {
            name = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
            if (_stricmp(key.c_str(), name.c_str()) == 0)
            {
                if (iter->asBool())
                {
                    questValue[bit] = 1;
                }
                break;
            }
        }
    }

    setQuestData(diff, act, quest, std::uint16_t(questValue.to_ulong()));
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonActIntro(const Json::Value& actIntroRoot, bool bSerializedFormat, bool isExpansion, EnumDifficulty diff, EnumAct act)
{
    if (actIntroRoot.isNull())
    {
        return;
    }

    if (act == EnumAct::V && !isExpansion)
    {
        return;
    }

    std::bitset<16> questValue = 0;
    if (bSerializedFormat)
    {
        std::string key;
        std::string name;
        bool isOptional = false;
        auto iter_end = actIntroRoot.end();
        for (auto iter = actIntroRoot.begin(); iter != iter_end; ++iter)
        {
            if (iter->isNull())
            {
                continue;
            }

            key = iter.name();
            for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
            {
                name = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                if (_stricmp(key.c_str(), name.c_str()) == 0)
                {
                    if (iter->asBool())
                    {
                        questValue[bit] = 1;
                    }
                    break;
                }
            }
        }
    }
    else if (actIntroRoot.asBool())
    {
        questValue[0] = 1;
    }

    setActIntroducedData(diff, act, std::uint16_t(questValue.to_ulong()), isExpansion);
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonActComplete(const Json::Value& actCompleteRoot, bool bSerializedFormat, bool isExpansion, EnumDifficulty diff, EnumAct act)
{
    if (actCompleteRoot.isNull())
    {
        return;
    }

    if (act == EnumAct::V && !isExpansion)
    {
        return;
    }

    std::bitset<16> questValue = 0;
    if (bSerializedFormat)
    {
        std::string key;
        std::string name;
        bool isOptional = false;
        auto iter_end = actCompleteRoot.end();
        for (auto iter = actCompleteRoot.begin(); iter != iter_end; ++iter)
        {
            if (iter->isNull())
            {
                continue;
            }

            key = iter.name();
            for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
            {
                name = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                if (_stricmp(key.c_str(), name.c_str()) == 0)
                {
                    if (iter->asBool())
                    {
                        questValue[bit] = 1;
                    }
                    break;
                }
            }
        }
    }
    else if (actCompleteRoot.asBool())
    {
        questValue[0] = 1;
    }

    setActCompletedData(diff, act, std::uint16_t(questValue.to_ulong()), isExpansion);
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonQuestAct(const Json::Value& questActRoot, bool bSerializedFormat, bool isExpansion, EnumDifficulty diff, EnumAct act)
{
    if (questActRoot.isNull())
    {
        return;
    }

    if (act == EnumAct::V && !isExpansion)
    {
        return;
    }

    std::string key;
    std::string name;
    auto iter_end = questActRoot.end();
    for (auto iter = questActRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        if (_stricmp(key.c_str(), (bSerializedFormat ? "Introduction" : "introduced")) == 0)
        {
            applyJsonActIntro(*iter, bSerializedFormat, isExpansion, diff, act);
        }
        else if (_stricmp(key.c_str(), (bSerializedFormat ? "Completion" : "completed")) == 0)
        {
            applyJsonActComplete(*iter, bSerializedFormat, isExpansion, diff, act);
        }
        else
        {
            for (std::uint8_t quest = 0; quest < (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS); ++quest)
            {
                name = getQuestJsonName(act, quest, bSerializedFormat);
                if (_stricmp(key.c_str(), name.c_str()) == 0)
                {
                    applyJsonQuest(*iter, bSerializedFormat, isExpansion, diff, act, quest);
                    break;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonQuestDifficulty(const Json::Value& questDiffRoot, bool bSerializedFormat, bool isExpansion, EnumDifficulty diff)
{
    if (questDiffRoot.isNull())
    {
        return;
    }

    std::string key;
    std::string name;
    auto iter_end = questDiffRoot.end();
    for (auto iter = questDiffRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        static std::initializer_list<EnumAct> all_acts = { EnumAct::I, EnumAct::II, EnumAct::III, EnumAct::IV, EnumAct::V };
        for (auto act : all_acts)
        {
            if (act == EnumAct::V && !isExpansion)
            {
                continue;
            }

            name = getActJsonName(act, bSerializedFormat);
            if (_stricmp(key.c_str(), name.c_str()) == 0)
            {
                applyJsonQuestAct(*iter, bSerializedFormat, isExpansion, diff, act);
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonQuests(const Json::Value& questsRoot, bool bSerializedFormat, bool isExpansion)
{
    if (questsRoot.isNull())
    {
        return;
    }

    std::string key;
    std::string name;
    auto iter_end = questsRoot.end();
    for (auto iter = questsRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };
        for (auto diff : all_diff)
        {
            name = getQuestsJsonName(diff, bSerializedFormat);
            if (_stricmp(key.c_str(), name.c_str()) == 0)
            {
                applyJsonQuestDifficulty(*iter, bSerializedFormat, isExpansion, diff);
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readQuests(const Json::Value& questsRoot, bool bSerializedFormat, bool isExpansion, std::FILE* charfile)
{
    if (bSerializedFormat)
    {
        // If "Header" value is present, it needs to be valid
        Json::Value value = questsRoot["Header"];
        if (!value.isNull())
        {
            if(std::uint32_t(value.asInt64()) != *((std::uint32_t*)QUESTS_MARKER.data()))
            {
                // bad header
                return false;
            }

            // If "Version" value is present, it needs to be valid
            value = questsRoot["Version"];
            if (!value.isNull())
            {
                if (std::uint32_t(value.asInt64()) != *((std::uint32_t*)QUESTS_VERSION.data()))
                {
                    // bad header
                    return false;
                }
            }

            // If "Length" value is present, it needs to be valid
            value = questsRoot["Length"];
            if (!value.isNull())
            {
                if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)QUESTS_SIZE_MARKER.data()))
                {
                    // bad header
                    return false;
                }
            }
        }
    }

    quests_start_location = std::ftell(charfile);
    std::fwrite(QUESTS_MARKER.data(), QUESTS_MARKER.size(), 1, charfile);
    quests_version = QUESTS_VERSION;
    std::fwrite(quests_version.data(), quests_version.size(), 1, charfile);
    std::fwrite(QUESTS_SIZE_MARKER.data(), QUESTS_SIZE_MARKER.size(), 1, charfile);
    quests_location = std::ftell(charfile);
    applyJsonQuests(questsRoot, bSerializedFormat, isExpansion);
    if (Version >= EnumCharVersion::v107)
    {
        bool ret = std::fwrite(Acts.data(), Acts.size() * sizeof(ActsInfoData), 1, charfile) != 1 ? false : true;
        std::fflush(charfile);
        return ret;
    }
    else
    {
        // Only copy 4 acts of Quest data for each difficulty
        for (std::uint32_t i = 0; i < NUM_OF_DIFFICULTY; ++i)
        {
            if (std::fwrite(Acts[i].Act.data(), Acts[i].Act.size() * sizeof(ActInfo), 1, charfile) != 1)
            {
                return false;
            }

            static std::array<std::uint8_t, 28> unknown = {
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
            };
            if (std::fwrite(unknown.data(), unknown.size(), 1, charfile) != 1)
            {
                return false;
            }
        }
        std::fflush(charfile);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readWaypoints(std::FILE* charfile)
{
    if (update_locations)
    {
        // find stats location
        waypoints_start_location = 0;
        waypoints_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (Version >= EnumCharVersion::v109)
        {
            if (cur_pos < (long)MIN_START_WAYPOINTS_POS)
            {
                cur_pos = MIN_START_WAYPOINTS_POS;
                std::fseek(charfile, cur_pos, SEEK_SET);
            }
        }
        else
        {
            if (cur_pos < MIN_START_POS)
            {
                cur_pos = MIN_START_POS;
                std::fseek(charfile, cur_pos, SEEK_SET);
            }
        }

        while (!feof(charfile))
        {
            waypoints_start_location = std::ftell(charfile);
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != WAYPOINTS_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != WAYPOINTS_MARKER[1])
            {
                continue;
            }

            // found waypoints marker (0x5357).
            std::fread(waypoints_version.data(), waypoints_version.size(), 1, charfile);
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != WAYPOINTS_SIZE_MARKER[0])
            {
                return false;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != WAYPOINTS_SIZE_MARKER[1])
            {
                return false;
            }

            // found size marker (0x0050).
            waypoints_location = std::ftell(charfile);
            break;
        }
    }

    if (waypoints_location == 0)
    {
        waypoints_start_location = 0;
        return false;
    }

    std::fseek(charfile, waypoints_location, SEEK_SET);

    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fread(&Waypoints_unknown[i], sizeof(Waypoints_unknown[i]), 1, charfile); // skip 0x0102 marker
        std::fread(&Waypoints[i], sizeof(Waypoints[i]), 1, charfile);
        std::fread(Waypoints_extraBits[i].data(), Waypoints_extraBits[i].size(), 1, charfile); // skip extra bits
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonWaypointAct(const Json::Value& waypointActRoot, bool bSerializedFormat, bool isExpansion, EnumDifficulty diff, EnumAct act)
{
    if (waypointActRoot.isNull())
    {
        return;
    }

    std::bitset<64> waypointValue = d2ce::ActsInfo::getWaypoints(diff);
    std::size_t offset = 0;
    if (act == EnumAct::V)
    {
        if (!isExpansion)
        {
            return;
        }

        offset = MAX_WAYPOINTS_PER_ACT * 3 + NUM_WAYPOINTS_ACT_IV;

        // make sure Act V - Harrogath is active if applicable
        if (getActCompleted(diff, d2ce::EnumAct::IV))
        {
            waypointValue[offset] = 1;
        }
    }
    else if (act == EnumAct::IV)
    {
        offset = MAX_WAYPOINTS_PER_ACT * 3;

        // make sure Act IV - Pandemonium Fortress is active if applicable
        if (getActCompleted(diff, d2ce::EnumAct::III))
        {
            waypointValue[offset] = 1;
        }
    }
    else if (act == EnumAct::III)
    {
        offset = MAX_WAYPOINTS_PER_ACT * 2;

        // make sure Act III - Kurast Docks is active if applicable
        if (getActCompleted(diff, d2ce::EnumAct::II))
        {
            waypointValue[offset] = 1;
        }
    }
    else if (act == EnumAct::II)
    {
        offset = MAX_WAYPOINTS_PER_ACT;

        // make sure Act II - Lut Gholein is active if applicable
        if (getActCompleted(diff, d2ce::EnumAct::I))
        {
            waypointValue[offset] = 1;
        }
    }
    else
    {
        // make sure Act I - Rogue Encampment is always active
        waypointValue[offset] = 1;
    }

    std::string key;
    std::string name;
    auto iter_end = waypointActRoot.end();
    for (auto iter = waypointActRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        for (std::uint8_t waypoint = 0; waypoint < (act == EnumAct::IV ? NUM_WAYPOINTS_ACT_IV : MAX_WAYPOINTS_PER_ACT); ++waypoint)
        {
            name = getWaypointJsonName(act, waypoint, bSerializedFormat);
            if (_stricmp(key.c_str(), name.c_str()) == 0)
            {
                if (iter->asBool())
                {
                    waypointValue[waypoint + offset] = 1;
                }
                break;
            }
        }
    }

    setWaypoints(diff, std::uint64_t(waypointValue.to_ullong()));
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonWaypointDifficulty(const Json::Value& waypointDiffRoot, bool bSerializedFormat, bool isExpansion, EnumDifficulty diff)
{
    if (waypointDiffRoot.isNull())
    {
        return;
    }

    std::string key;
    std::string name;
    auto iter_end = waypointDiffRoot.end();
    for (auto iter = waypointDiffRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        static std::initializer_list<EnumAct> all_acts = { EnumAct::I, EnumAct::II, EnumAct::III, EnumAct::IV, EnumAct::V };
        for (auto act : all_acts)
        {
            if (act == EnumAct::V && !isExpansion)
            {
                continue;
            }

            name = getActJsonName(act, bSerializedFormat);
            if (_stricmp(key.c_str(), name.c_str()) == 0)
            {
                applyJsonWaypointAct(*iter, bSerializedFormat, isExpansion, diff, act);
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonWaypoints(const Json::Value& waypointsRoot, bool isExpansion, bool bSerializedFormat)
{
    if (waypointsRoot.isNull())
    {
        return;
    }

    std::string key;
    std::string name;
    auto iter_end = waypointsRoot.end();
    for (auto iter = waypointsRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };
        for (auto diff : all_diff)
        {
            name = getDifficultyJsonName(diff, bSerializedFormat);
            if (_stricmp(key.c_str(), name.c_str()) == 0)
            {
                applyJsonWaypointDifficulty(*iter, bSerializedFormat, isExpansion, diff);
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readWaypoints(const Json::Value& waypointsRoot, bool bSerializedFormat, bool isExpansion, std::FILE* charfile)
{
    if (bSerializedFormat)
    {
        // If "Header" value is present, it needs to be valid
        Json::Value value = waypointsRoot["Header"];
        if (!value.isNull())
        {
            if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)WAYPOINTS_MARKER.data()))
            {
                // bad header
                return false;
            }

            // If "Version" value is present, it needs to be valid
            value = waypointsRoot["Version"];
            if (!value.isNull())
            {
                if (std::uint32_t(value.asInt64()) != *((std::uint32_t*)WAYPOINTS_VERSION.data()))
                {
                    // bad header
                    return false;
                }
            }

            // If "Length" value is present, it needs to be valid
            value = waypointsRoot["Length"];
            if (!value.isNull())
            {
                if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)WAYPOINTS_SIZE_MARKER.data()))
                {
                    // bad header
                    return false;
                }
            }
        }
    }

    waypoints_start_location = std::ftell(charfile);
    std::fwrite(WAYPOINTS_MARKER.data(), WAYPOINTS_MARKER.size(), 1, charfile);
    waypoints_version = WAYPOINTS_VERSION;
    std::fwrite(waypoints_version.data(), waypoints_version.size(), 1, charfile);
    std::fwrite(WAYPOINTS_SIZE_MARKER.data(), WAYPOINTS_SIZE_MARKER.size(), 1, charfile);
    waypoints_location = std::ftell(charfile);
    applyJsonWaypoints(waypointsRoot, isExpansion, bSerializedFormat);
    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fwrite(&Waypoints_unknown[i], sizeof(Waypoints_unknown[i]), 1, charfile);
        std::fwrite(&Waypoints[i], sizeof(Waypoints[i]), 1, charfile);
        std::fwrite(Waypoints_extraBits[i].data(), Waypoints_extraBits[i].size(), 1, charfile);
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readNPC(std::FILE* charfile)
{
    if (update_locations)
    {
        // find stats location
        npc_start_location = 0;
        npc_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (Version >= EnumCharVersion::v109)
        {
            if (cur_pos < (long)MIN_START_NPC_POS)
            {
                cur_pos = MIN_START_NPC_POS;
                std::fseek(charfile, cur_pos, SEEK_SET);
            }
        }
        else
        {
            if (cur_pos < MIN_START_POS)
            {
                cur_pos = MIN_START_POS;
                std::fseek(charfile, cur_pos, SEEK_SET);
            }
        }

        while (!feof(charfile))
        {
            npc_start_location = std::ftell(charfile);
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != NPC_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != NPC_MARKER[1])
            {
                continue;
            }

            // found NPC marker (0x7701).
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != NPC_SIZE_MARKER[0])
            {
                return false;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != NPC_SIZE_MARKER[1])
            {
                return false;
            }

            // found size marker (0x0034).
            npc_location = std::ftell(charfile);
            break;
        }
    }

    if (npc_location == 0)
    {
        npc_start_location = 0;
        return false;
    }

    std::fseek(charfile, npc_location, SEEK_SET);

    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fread(&NPCIntroductions[i], sizeof(NPCIntroductions[i]), 1, charfile);
    }
    
    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fread(&NPCCongrats[i], sizeof(NPCCongrats[i]), 1, charfile);
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonNPCsDifficulty(const Json::Value& npcsDiffRoot, bool bSerializedFormat, bool isExpansion, EnumDifficulty diff)
{
    if (npcsDiffRoot.isNull())
    {
        return;
    }

    std::bitset<64> intro = getNpcIntroductions(diff);
    std::bitset<64> congrats = getNpcCongrats(diff);

    std::string key;
    std::string name;
    Json::Value npcElement;
    auto iter_end = npcsDiffRoot.end();
    for (auto iter = npcsDiffRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        for (std::uint8_t npc = 0; npc < intro.size(); ++npc)
        {
            name = getNpcJsonName(npc, isExpansion, bSerializedFormat);
            if (name.empty())
            {
                continue;
            }

            if (_stricmp(key.c_str(), name.c_str()) == 0)
            {
                npcElement = iter->operator[](bSerializedFormat ? "Introduction" : "intro");
                if (npcElement.isNull())
                {
                    continue;
                }

                if (npcElement.asBool())
                {
                    intro[npc] = 1;
                }

                npcElement = iter->operator[](bSerializedFormat ? "Congratulations" : "congrats");
                if (npcElement.isNull())
                {
                    continue;
                }

                if (npcElement.asBool())
                {
                    congrats[npc] = 1;
                }
            }
        }
    }

    setNpcIntroductions(diff, std::uint64_t(intro.to_ullong()));
    setNpcCongrats(diff, std::uint64_t(congrats.to_ullong()));
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::applyJsonNPCs(const Json::Value& npcsRoot, bool bSerializedFormat, bool isExpansion)
{
    if (npcsRoot.isNull())
    {
        return;
    }

    std::string key;
    std::string name;
    auto iter_end = npcsRoot.end();
    for (auto iter = npcsRoot.begin(); iter != iter_end; ++iter)
    {
        if (iter->isNull())
        {
            continue;
        }

        key = iter.name();
        static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };
        for (auto diff : all_diff)
        {
            name = getDifficultyJsonName(diff, bSerializedFormat);
            if (_stricmp(key.c_str(), name.c_str()) == 0)
            {
                applyJsonNPCsDifficulty(*iter, bSerializedFormat, isExpansion, diff);
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readNPC(const Json::Value& npcsRoot, bool bSerializedFormat, bool isExpansion, std::FILE* charfile)
{
    if (bSerializedFormat)
    {
        // If "Header" value is present, it needs to be valid
        Json::Value value = npcsRoot["Header"];
        if (!value.isNull())
        {
            if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)NPC_MARKER.data()))
            {
                // bad header
                return false;
            }

            // If "Length" value is present, it needs to be valid
            value = npcsRoot["Length"];
            if (!value.isNull())
            {
                if (std::uint16_t(value.asInt64()) != *((std::uint16_t*)NPC_SIZE_MARKER.data()))
                {
                    // bad header
                    return false;
                }
            }
        }
    }

    npc_start_location = std::ftell(charfile);
    std::fwrite(NPC_MARKER.data(), NPC_MARKER.size(), 1, charfile);
    std::fwrite(NPC_SIZE_MARKER.data(), NPC_SIZE_MARKER.size(), 1, charfile);
    npc_location = std::ftell(charfile);
    applyJsonNPCs(npcsRoot, bSerializedFormat, isExpansion);
    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fwrite(&NPCIntroductions[i], sizeof(NPCIntroductions[i]), 1, charfile);
    }

    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fwrite(&NPCCongrats[i], sizeof(NPCCongrats[i]), 1, charfile);
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::writeQuests(std::FILE* charfile)
{
    if (quests_start_location == 0)
    {
        return false;
    }

    std::fseek(charfile, quests_start_location, SEEK_SET);
    std::fwrite(QUESTS_MARKER.data(), QUESTS_MARKER.size(), 1, charfile);
    std::fwrite(quests_version.data(), quests_version.size(), 1, charfile);
    std::fwrite(QUESTS_SIZE_MARKER.data(), QUESTS_SIZE_MARKER.size(), 1, charfile);
    if (Version >= EnumCharVersion::v107)
    {
        bool ret = std::fwrite(Acts.data(), Acts.size() * sizeof(ActsInfoData), 1, charfile) != 1 ? false : true;
        std::fflush(charfile);
        return ret;
    }
    else
    {
        // Only copy 4 acts of Quest data for each difficulty
        for (std::uint32_t i = 0; i < NUM_OF_DIFFICULTY; ++i)
        {
            if (std::fwrite(Acts[i].Act.data(), Acts[i].Act.size() * sizeof(ActInfo), 1, charfile) != 1)
            {
                return false;
            }
            std::fseek(charfile, 28, SEEK_CUR);
        }
        std::fflush(charfile);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::writeWaypoints(std::FILE* charfile)
{
    if (waypoints_start_location == 0)
    {
        return false;
    }

    std::fseek(charfile, waypoints_start_location, SEEK_SET);
    std::fwrite(WAYPOINTS_MARKER.data(), WAYPOINTS_MARKER.size(), 1, charfile);
    std::fwrite(waypoints_version.data(), waypoints_version.size(), 1, charfile);
    std::fwrite(WAYPOINTS_SIZE_MARKER.data(), WAYPOINTS_SIZE_MARKER.size(), 1, charfile);
    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fwrite(&Waypoints_unknown[i], sizeof(Waypoints_unknown[i]), 1, charfile); // skip 0x0102 marker
        std::fwrite(&Waypoints[i], sizeof(Waypoints[i]), 1, charfile);
        std::fwrite(Waypoints_extraBits[i].data(), Waypoints_extraBits[i].size(), 1, charfile); // skip extra bits
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::writeNPC(std::FILE* charfile)
{
    if (npc_start_location == 0)
    {
        return false;
    }

    std::fseek(charfile, npc_start_location, SEEK_SET);
    std::fwrite(NPC_MARKER.data(), NPC_MARKER.size(), 1, charfile);
    std::fwrite(NPC_SIZE_MARKER.data(), NPC_SIZE_MARKER.size(), 1, charfile);
    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fwrite(&NPCIntroductions[i], sizeof(NPCIntroductions[i]), 1, charfile);
    }

    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fwrite(&NPCCongrats[i], sizeof(NPCCongrats[i]), 1, charfile);
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readActs(EnumCharVersion version, std::FILE* charfile)
{
    Version = version;
    update_locations = quests_location == 0 ? true : false;
    if (!readQuests(charfile) || quests_location == 0)
    {
        return false;
    }

    if (!readWaypoints(charfile) || waypoints_location == 0)
    {
        return false;
    }

    if (!readNPC(charfile) || npc_location == 0)
    {
        return false;
    }

    update_locations = false;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readActs(const Json::Value& root, bool bSerializedFormat, bool isExpansion, EnumCharVersion version, std::FILE* charfile)
{
    Version = version;
    update_locations = true;

    Json::Value childRoot = root[bSerializedFormat ? "Quests" : "header"];
    if (!readQuests(childRoot, bSerializedFormat, isExpansion, charfile) || quests_location == 0)
    {
        return false;
    }

    Json::Value header = root[bSerializedFormat ? "Header" : "header"];
    childRoot = bSerializedFormat ? root["Waypoints"] : header["waypoints"];
    if (!readWaypoints(childRoot, bSerializedFormat, isExpansion, charfile) || waypoints_location == 0)
    {
        return false;
    }

    childRoot = bSerializedFormat ? root["NPCDialog"] : header["npcs"];
    if (!readNPC(childRoot, bSerializedFormat, isExpansion, charfile) || npc_location == 0)
    {
        return false;
    }

    update_locations = false;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::writeActs(std::FILE* charfile)
{
    if (!writeQuests(charfile))
    {
        return false;
    }

    if (!writeWaypoints(charfile))
    {
        return false;
    }

    return writeNPC(charfile);
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::questsAsJson(Json::Value& parent, bool isExpansion, bool bSerializedFormat) const
{
    static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };
    static std::initializer_list<EnumAct> all_acts = { EnumAct::I, EnumAct::II, EnumAct::III, EnumAct::IV, EnumAct::V };
    if (bSerializedFormat)
    {
        Json::Value quests;
        quests["Magic"] = ((Version < EnumCharVersion::v115) ? "1" : "0");
        quests["Header"] = *((std::uint32_t*)QUESTS_MARKER.data());
        quests["Version"] = *((std::uint32_t*)quests_version.data());
        quests["Length"] = *((std::uint16_t*)QUESTS_SIZE_MARKER.data());

        for (auto diff : all_diff)
        {
            Json::Value questDiff;
            for (auto act : all_acts)
            {
                Json::Value questAct;
                if (act == EnumAct::V && !isExpansion)
                {
                    continue;
                }

                Json::Value questIntroduction;
                bool isOptional = false;
                std::string bitName;
                std::bitset<16> questValue = getActIntroducedData(diff, act);
                for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
                {
                    bitName = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                    if (!questValue[bit] && isOptional)
                    {
                        // optional
                        continue;
                    }

                    questIntroduction[bitName] = (questValue[bit] ? true : false);
                }
                questAct["Introduction"] = questIntroduction;

                if (act == EnumAct::V)
                {
                    auto& actInfo = Acts[static_cast<std::underlying_type_t<EnumDifficulty>>(diff)].ActV;
                    for (size_t idx = 1; idx <= 2; ++idx)
                    {
                        Json::Value questExtra;
                        questValue = actInfo.Padding1[idx - 1];
                        for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
                        {
                            bitName = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                            if (!questValue[bit] && isOptional)
                            {
                                // optional
                                continue;
                            }

                            questExtra[bitName] = (questValue[bit] ? true : false);
                        }
                        {
                            std::stringstream ss;
                            ss << "Extra" << std::dec << idx;
                            questAct[ss.str()] = questExtra;
                        }
                    }
                }

                Json::Value questElement;
                for (std::uint8_t quest = 0; quest < (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS); ++quest)
                {
                    isOptional = false;
                    questValue = getQuestData(diff, act, quest);
                    for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
                    {
                        bitName = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                        if (!questValue[bit] && isOptional)
                        {
                            // optional
                            continue;
                        }

                        questElement[bitName] = (questValue[bit] ? true : false);
                    }
                    questAct[getQuestJsonName(act, quest, bSerializedFormat)] = questElement;
                }

                Json::Value questCompletion;
                questValue = act == EnumAct::V ? getActVResetStatCompletedData(diff) : getActCompletedData(diff, act);
                for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
                {
                    bitName = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                    if (!questValue[bit] && isOptional)
                    {
                        // optional
                        continue;
                    }

                    questCompletion[bitName] = (questValue[bit] ? true : false);
                }
                questAct["Completion"] = questCompletion;

                if (act == EnumAct::IV)
                {
                    auto& actInfo = Acts[static_cast<std::underlying_type_t<EnumDifficulty>>(diff)].Act[3];
                    for (size_t idx = 1; idx <= 3; ++idx)
                    {
                        Json::Value questExtra;
                        questValue = idx == 3 ? actInfo.Completed : actInfo.Quests[3 + idx];
                        for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
                        {
                            bitName = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                            if (!questValue[bit] && isOptional)
                            {
                                // optional
                                continue;
                            }

                            questExtra[bitName] = (questValue[bit] ? true : false);
                        }
                        {
                            std::stringstream ss;
                            ss << "Extra" << std::dec << idx;
                            questAct[ss.str()] = questExtra;
                        }
                    }
                }
                else if (act == EnumAct::V)
                {
                    auto& actInfo = Acts[static_cast<std::underlying_type_t<EnumDifficulty>>(diff)].ActV;
                    for (size_t idx = 3; idx <= 8; ++idx)
                    {
                        Json::Value questExtra;
                        questValue = actInfo.Padding2[idx - 3];
                        for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
                        {
                            bitName = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                            if (!questValue[bit] && isOptional)
                            {
                                // optional
                                continue;
                            }

                            questExtra[bitName] = (questValue[bit] ? true : false);
                        }
                        {
                            std::stringstream ss;
                            ss << "Extra" << std::dec << idx;
                            questAct[ss.str()] = questExtra;
                        }
                    }
                }
                questDiff[getActJsonName(act, bSerializedFormat)] = questAct;
            }
            quests[getQuestsJsonName(diff, bSerializedFormat)] = questDiff;
        }
        parent["Quests"] = quests;
    }
    else
    {
        for (auto diff : all_diff)
        {
            Json::Value questDiff;
            for (auto act : all_acts)
            {
                if (act == EnumAct::V && !isExpansion)
                {
                    continue;
                }

                Json::Value questAct;
                std::bitset<16> questValue;
                questAct["introduced"] = getActIntroduced(diff, act);
                for (std::uint8_t quest = 0; quest < (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS); ++quest)
                {
                    Json::Value questElement;
                    bool isOptional = false;
                    std::string bitName;
                    questValue = getQuestData(diff, act, quest);
                    for (std::uint8_t bit = 0; bit < questValue.size(); ++bit)
                    {
                        bitName = getQuestBitJsonName(bit, isOptional, bSerializedFormat);
                        if (!questValue[bit] && isOptional)
                        {
                            // optional
                            continue;
                        }

                        questElement[bitName] = (questValue[bit] ? true : false);
                    }
                    questAct[getQuestJsonName(act, quest, bSerializedFormat)] = questElement;
                }
                questAct["completed"] = getActCompleted(diff, act);
                questDiff[getActJsonName(act, bSerializedFormat)] = questAct;
            }
            parent[getQuestsJsonName(diff, bSerializedFormat)] = questDiff;
        }
    }
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getQuestsJsonName(EnumDifficulty diff, bool bSerializedFormat) const
{
    if (bSerializedFormat)
    {
        return getDifficultyJsonName(diff, bSerializedFormat);
    }

    std::stringstream ss;
    ss << "quests_" << getDifficultyJsonName(diff, bSerializedFormat);
    return ss.str();
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getDifficultyJsonName(EnumDifficulty diff, bool bSerializedFormat) const
{
    std::stringstream ss;
    switch (diff)
    {
    case EnumDifficulty::Normal:
        ss << (bSerializedFormat ? "Normal" : "normal");
        break;

    case EnumDifficulty::Nightmare:
        ss << (bSerializedFormat ? "Nightmare" : "nm");
        break;

    case EnumDifficulty::Hell:
        ss << (bSerializedFormat ? "Hell" : "hell");
        break;
    }

    return ss.str();
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getActJsonName(EnumAct act, bool bSerializedFormat) const
{
    std::stringstream ss;
    if (bSerializedFormat)
    {
        ss << "Act";
        switch (act)
        {
        case EnumAct::I:
            ss << "I";
            break;

        case EnumAct::II:
            ss << "II";
            break;

        case EnumAct::III:
            ss << "III";
            break;

        case EnumAct::IV:
            ss << "IV";
            break;

        case EnumAct::V:
            ss << "V";
            break;
        }
    }
    else
    {
        ss << "act_";
        switch (act)
        {
        case EnumAct::I:
            ss << "i";
            break;

        case EnumAct::II:
            ss << "ii";
            break;

        case EnumAct::III:
            ss << "iii";
            break;

        case EnumAct::IV:
            ss << "iv";
            break;

        case EnumAct::V:
            ss << "v";
            break;
        }
    }

    return ss.str();
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getQuestJsonName(EnumAct act, std::uint8_t quest, bool bSerializedFormat) const
{
    switch (act)
    {
    case EnumAct::I:
        switch (quest)
        {
        case 0:
            return bSerializedFormat ? "DenOfEvil" : "den_of_evil";

        case 1:
            return bSerializedFormat ? "SistersBurialGrounds" : "sisters_burial_grounds";

        case 2:
            return bSerializedFormat ? "ToolsOfTheTrade" : "tools_of_the_trade";

        case 3:
            return bSerializedFormat ? "TheSearchForCain" : "the_search_for_cain";

        case 4:
            return bSerializedFormat ? "TheForgottenTower" : "the_forgotten_tower";

        case 5:
            return bSerializedFormat ? "SistersToTheSlaughter" : "sisters_to_the_slaughter";
        }
        break;

    case EnumAct::II:
        switch (quest)
        {
        case 0:
            return bSerializedFormat ? "RadamentsLair" : "radaments_lair";

        case 1:
            return bSerializedFormat ? "TheHoradricStaff" : "the_horadric_staff";

        case 2:
            return bSerializedFormat ? "TaintedSun" : "tainted_sun";

        case 3:
            return bSerializedFormat ? "ArcaneSanctuary" : "arcane_sanctuary";

        case 4:
            return bSerializedFormat ? "TheSummoner" : "the_summoner";

        case 5:
            return bSerializedFormat ? "TheSevenTombs" : "the_seven_tombs";
        }
        break;

    case EnumAct::III:
        switch (quest)
        {
        case 0:
            return bSerializedFormat ? "LamEsensTome" : "lam_esens_tome";

        case 1:
            return bSerializedFormat ? "KhalimsWill" : "khalims_will";

        case 2:
            return bSerializedFormat ? "BladeOfTheOldReligion" : "blade_of_the_old_religion";

        case 3:
            return bSerializedFormat ? "TheGoldenBird" : "the_golden_bird";

        case 4:
            return bSerializedFormat ? "TheBlackenedTemple" : "the_blackened_temple";

        case 5:
            return bSerializedFormat ? "TheGuardian" : "the_guardian";
        }
        break;

    case EnumAct::IV:
        switch (quest)
        {
        case 0:
            return bSerializedFormat ? "TheFallenAngel" : "the_fallen_angel";

        case 1:
            return bSerializedFormat ? "TerrorsEnd" : "terrors_end";

        case 2:
            return bSerializedFormat ? "Hellforge" : "hellforge";
        }
        break;

    case EnumAct::V:
        switch (quest)
        {
        case 0:
            return bSerializedFormat ? "SiegeOnHarrogath" : "siege_on_harrogath";

        case 1:
            return bSerializedFormat ? "RescueOnMountArreat" : "rescue_on_mount_arreat";

        case 2:
            return bSerializedFormat ? "PrisonOfIce" : "prison_of_ice";

        case 3:
            return bSerializedFormat ? "BetrayalOfHarrogath" : "betrayal_of_harrogath";

        case 4:
            return bSerializedFormat ? "RiteOfPassage" : "rite_of_passage";

        case 5:
            return bSerializedFormat ? "EveOfDestruction" : "eve_of_destruction";
        }
    }

    return "";
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getQuestBitJsonName(std::uint8_t bit, bool& isOptional, bool bSerializedFormat) const
{
    isOptional = false;
    if (bit > 15)
    {
        isOptional = true;
        return "";
    }

    switch (bit)
    {
    case 0:
        return bSerializedFormat ? "RewardGranted" : "is_completed";

    case 1:
        return bSerializedFormat ? "RewardPending" : "is_requirement_completed";

    case 2:
        return bSerializedFormat ? "Started" : "is_received";

    case 3:
        if (bSerializedFormat)
        {
            return "LeftTown";
        }
        break;

    case 4:
        if (bSerializedFormat)
        {
            return "EnterArea";
        }
        break;

    case 5:
        if (bSerializedFormat)
        {
            return "Custom1";
        }
        break;

    case 6:
        if (bSerializedFormat)
        {
            return "Custom2";
        }
        break;

    case 7:
        isOptional = bSerializedFormat ? false : true;
        return bSerializedFormat ? "Custom3" : "consumed_scroll";

    case 8:
        if (bSerializedFormat)
        {
            return "Custom4";
        }
        break;

    case 9:
        if (bSerializedFormat)
        {
            return "Custom5";
        }
        break;

    case 10:
        if (bSerializedFormat)
        {
            return "Custom6";
        }
        break;

    case 11:
        if (bSerializedFormat)
        {
            return "Custom7";
        }
        break;

    case 12:
        return bSerializedFormat ? "QuestLog" : "closed";

    case 13:
        return bSerializedFormat ? "PrimaryGoalAchieved" : "done_recently";

    case 14:
        if (bSerializedFormat)
        {
            return "CompletedNow";
        }
        break;

    case 15:
        if (bSerializedFormat)
        {
            return "CompletedBefore";
        }
        break;
    }

    isOptional = true;
    std::stringstream ss;
    ss << "unk" << std::dec << std::uint16_t(bit);
    return ss.str();
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::waypointsAsJson(Json::Value& parent, bool isExpansion, bool bSerializedFormat) const
{
    static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };
    static std::initializer_list<EnumAct> all_acts = { EnumAct::I, EnumAct::II, EnumAct::III, EnumAct::IV, EnumAct::V };
    if (bSerializedFormat)
    {
        Json::Value waypoints;
        waypoints["Header"] = *((std::uint32_t*)WAYPOINTS_MARKER.data());
        waypoints["Version"] = *((std::uint32_t*)waypoints_version.data());
        waypoints["Length"] = *((std::uint16_t*)WAYPOINTS_SIZE_MARKER.data());

        for (auto diff : all_diff)
        {
            Json::Value waypointDiff;
            waypointDiff["Header"] = Waypoints_unknown[static_cast<std::underlying_type_t<EnumDifficulty>>(diff)];

            for (auto act : all_acts)
            {
                if (act == EnumAct::V && !isExpansion)
                {
                    continue;
                }

                Json::Value waypointAct;
                for (std::uint8_t waypoint = 0; waypoint < (act == EnumAct::IV ? NUM_WAYPOINTS_ACT_IV : MAX_WAYPOINTS_PER_ACT); ++waypoint)
                {
                    waypointAct[getWaypointJsonName(act, waypoint, bSerializedFormat)] = getWaypointActivated(diff, act, waypoint);
                }
                waypointDiff[getActJsonName(act, bSerializedFormat)] = waypointAct;
            }
            waypoints[getDifficultyJsonName(diff, bSerializedFormat)] = waypointDiff;
        }
        parent["Waypoints"] = waypoints;
    }
    else
    {
        Json::Value waypoints;
        for (auto diff : all_diff)
        {
            Json::Value waypointDiff;
            for (auto act : all_acts)
            {
                if (act == EnumAct::V && !isExpansion)
                {
                    continue;
                }

                Json::Value waypointAct;
                for (std::uint8_t waypoint = 0; waypoint < (act == EnumAct::IV ? NUM_WAYPOINTS_ACT_IV : MAX_WAYPOINTS_PER_ACT); ++waypoint)
                {
                    waypointAct[getWaypointJsonName(act, waypoint, bSerializedFormat)] = getWaypointActivated(diff, act, waypoint);
                }
                waypointDiff[getActJsonName(act, bSerializedFormat)] = waypointAct;
            }
            waypoints[getDifficultyJsonName(diff, bSerializedFormat)] = waypointDiff;
        }
        parent["waypoints"] = waypoints;
    }
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getWaypointJsonName(EnumAct act, std::uint8_t waypoint, bool bSerializedFormat) const
{
    switch (act)
    {
    case EnumAct::I:
        switch (waypoint)
        {
        case 0:
            return bSerializedFormat ? "RogueEncampement" : "rogue_encampement";

        case 1:
            return bSerializedFormat ? "ColdPlains" : "cold_plains";

        case 2:
            return bSerializedFormat ? "StonyField" : "stony_field";

        case 3:
            return bSerializedFormat ? "DarkWoods" : "dark_woods";

        case 4:
            return bSerializedFormat ? "BlackMarsh" : "black_marsh";

        case 5:
            return bSerializedFormat ? "OuterCloister" : "outer_cloister";

        case 6:
            return bSerializedFormat ? "JailLvl1" : "jail_lvl_1";

        case 7:
            return bSerializedFormat ? "InnerCloister" : "inner_cloiste";

        case 8:
            return bSerializedFormat ? "CatacombsLvl2" : "catacombs_lvl_2";
        }
        break;

    case EnumAct::II:
        switch (waypoint)
        {
        case 0:
            return bSerializedFormat ? "LutGholein" : "lut_gholein";

        case 1:
            return bSerializedFormat ? "SewersLvl2" : "sewers_lvl_2";

        case 2:
            return bSerializedFormat ? "DryHills" : "dry_hills";

        case 3:
            return bSerializedFormat ? "HallsOfTheDeadLvl2" : "halls_of_the_dead_lvl_2";

        case 4:
            return bSerializedFormat ? "FarOasis" : "far_oasis";

        case 5:
            return bSerializedFormat ? "LostCity" : "lost_city";

        case 6:
            return bSerializedFormat ? "PalaceCellarLvl1" : "palace_cellar_lvl_1";

        case 7:
            return bSerializedFormat ? "ArcaneSanctuary" : "arcane_sanctuary";

        case 8:
            return bSerializedFormat ? "CanyonOfTheMagi" : "canyon_of_the_magi";
        }
        break;

    case EnumAct::III:
        switch (waypoint)
        {
        case 0:
            return bSerializedFormat ? "KurastDocks" : "kurast_docks";

        case 1:
            return bSerializedFormat ? "SpiderForest" : "spider_forest";

        case 2:
            return bSerializedFormat ? "GreatMarsh" : "great_marsh";

        case 3:
            return bSerializedFormat ? "FlayerJungle" : "flayer_jungle";

        case 4:
            return bSerializedFormat ? "LowerKurast" : "lower_kurast";

        case 5:
            return bSerializedFormat ? "KurastBazaar" : "kurast_bazaar";

        case 6:
            return bSerializedFormat ? "UpperKurast" : "upper_kurast";

        case 7:
            return bSerializedFormat ? "Travincal" : "travincal";

        case 8:
            return bSerializedFormat ? "DuranceOfHateLvl2" : "durance_of_hate_lvl_2";
        }
        break;

    case EnumAct::IV:
        switch (waypoint)
        {
        case 0:
            return bSerializedFormat ? "ThePandemoniumFortress" : "the_pandemonium_fortress";

        case 1:
            return bSerializedFormat ? "CityOfTheDamned" : "city_of_the_damned";

        case 2:
            return bSerializedFormat ? "RiverOfFlame" : "river_of_flame";
        }
        break;

    case EnumAct::V:
        switch (waypoint)
        {
        case 0:
            return bSerializedFormat ? "Harrogath" : "harrogath";

        case 1:
            return bSerializedFormat ? "FrigidHighlands" : "frigid_highlands";

        case 2:
            return bSerializedFormat ? "ArreatPlateau" : "arreat_plateau";

        case 3:
            return bSerializedFormat ? "CrystallinePassage" : "crystalline_passage";

        case 4:
            return bSerializedFormat ? "HallsOfPain" : "halls_of_pain";

        case 5:
            return bSerializedFormat ? "GlacialTrail" : "glacial_trail";

        case 6:
            return bSerializedFormat ? "FrozenTundra" : "frozen_tundra";

        case 7:
            return bSerializedFormat ? "TheAncientsWay" : "the_ancients_way";

        case 8:
            return bSerializedFormat ? "WorldstoneKeepLvl2" : "worldstone_keep_lvl_2";
        }
    }

    return "";
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::npcAsJson(Json::Value& parent, bool isExpansion, bool bSerializedFormat) const
{
    static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };

    if (bSerializedFormat)
    {
        Json::Value npcDialog;
        npcDialog["Header"] = *((std::uint16_t*)NPC_MARKER.data());
        npcDialog["Length"] = *((std::uint16_t*)NPC_SIZE_MARKER.data());
        for (auto diff : all_diff)
        {
            Json::Value npcDialogDiff;
            std::bitset<64> intro = getNpcIntroductions(diff);
            std::bitset<64> congrats = getNpcCongrats(diff);

            std::string npcName;
            for (std::uint8_t npc = 0; npc < intro.size(); ++npc)
            {
                Json::Value npcElement;
                npcName = getNpcJsonName(npc, isExpansion, bSerializedFormat);
                if (npcName.empty())
                {
                    continue;
                }

                npcElement["Introduction"] = (intro[npc] == 0 ? false : true);
                npcElement["Congratulations"] = (congrats[npc] == 0 ? false : true);
                npcDialogDiff[npcName] = npcElement;
            }
            npcDialog[getDifficultyJsonName(diff, bSerializedFormat)] = npcDialogDiff;
        }
        parent["NPCDialog"] = npcDialog;
    }
    else
    {
        Json::Value npcDialog;
        for (auto diff : all_diff)
        {
            Json::Value npcDialogDiff;
            std::bitset<64> intro = getNpcIntroductions(diff);
            std::bitset<64> congrats = getNpcCongrats(diff);

            std::string npcName;
            for (std::uint8_t npc = 0; npc < intro.size(); ++npc)
            {
                Json::Value npcElement;
                npcName = getNpcJsonName(npc, isExpansion, bSerializedFormat);
                if (npcName.empty())
                {
                    continue;
                }

                npcElement["intro"] = (intro[npc] == 0 ? false : true);
                npcElement["congrats"] = (congrats[npc] == 0 ? false : true);
                npcDialogDiff[npcName] = npcElement;
            }
            npcDialog[getDifficultyJsonName(diff, bSerializedFormat)] = npcDialogDiff;
        }
        parent["npcs"] = npcDialog;
    }
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getNpcJsonName(std::uint8_t npc, bool isExpansion, bool bSerializedFormat) const
{
    switch (npc)
    {
    case 0:
        return bSerializedFormat ? "WarrivActII" : "warriv_act_ii";

    case 1:
        if (bSerializedFormat)
        {
            return "Unk0x0001";
        }
        break;

    case 2:
        return bSerializedFormat ? "Charsi" : "charsi";

    case 3:
        return bSerializedFormat ? "WarrivActI" : "warriv_act_i";

    case 4:
        return bSerializedFormat ? "Kashya" : "kashya";

    case 5:
        return bSerializedFormat ? "Akara" : "akara";

    case 6:
        return bSerializedFormat ? "Gheed" : "gheed";

    case 7:
        if (bSerializedFormat)
        {
            return "Unk0x0007";
        }
        break;

    case 8:
        return bSerializedFormat ? "Greiz" : "greiz";

    case 9:
        return bSerializedFormat ? "Jerhyn" : "jerhyn";

    case 10:
        return bSerializedFormat ? "MeshifActII" : "meshif_act_ii";

    case 11:
        return bSerializedFormat ? "Geglash" : "geglash";

    case 12:
        return bSerializedFormat ? "Lysander" : "lysnader";

    case 13:
        return bSerializedFormat ? "Fara" : "fara";

    case 14:
        return bSerializedFormat ? "Drogan" : "drogan";

    case 15:
        if (bSerializedFormat)
        {
            return "Unk0x000F";
        }
        break;

    case 16:
        return bSerializedFormat ? "Alkor" : "alkor";

    case 17:
        return bSerializedFormat ? "Hratli" : "hratli";

    case 18:
        return bSerializedFormat ? "Ashera" : "ashera";

    case 19:
        if (bSerializedFormat)
        {
            return "Unk0x0013";
        }
        break;

    case 20:
        if (bSerializedFormat)
        {
            return "Unk0x0014";
        }
        break;

    case 21:
        return bSerializedFormat ? "CainActIII" : "cain_act_iii";

    case 22:
        if (bSerializedFormat)
        {
            return "Unk0x0016";
        }
        break;

    case 23:
        return bSerializedFormat ? "Elzix" : "elzix";

    case 24:
        return isExpansion ? (bSerializedFormat ? "Malah" : "malah") : "";

    case 25:
        return isExpansion ? (bSerializedFormat ? "Anya" : "anya") : "";

    case 26:
        if (bSerializedFormat)
        {
            return "Unk0x001A";
        }
        break;

    case 27:
        return bSerializedFormat ? "Natalya" : "natalya";

    case 28:
        return bSerializedFormat ? "MeshifActIII" : "meshif_act_iii";

    case 29:
        if (bSerializedFormat)
        {
            return "Unk0x001D";
        }
        break;

    case 30:
        if (bSerializedFormat)
        {
            return "Unk0x001F";
        }
        break;

    case 31:
        return bSerializedFormat ? "Ormus" : "ormus";

    case 32:
        if (bSerializedFormat)
        {
            return "Unk0x0021";
        }
        break;

    case 33:
        if (bSerializedFormat)
        {
            return "Unk0x0022";
        }
        break;

    case 34:
        if (bSerializedFormat)
        {
            return "Unk0x0023";
        }
        break;

    case 35:
        if (bSerializedFormat)
        {
            return "Unk0x0024";
        }
        break;

    case 36:
        if (bSerializedFormat)
        {
            return "Unk0x0025";
        }
        break;

    case 37:
        return isExpansion ? (bSerializedFormat ? "CainActV" : "cain_act_v") : "";

    case 38:
        return isExpansion ? (bSerializedFormat ? "Qualkehk" : "qualkehk") : "";

    case 39:
        return isExpansion ? (bSerializedFormat ? "Nihlathak" : "nihlathak") : "";

    case 40:
        if (bSerializedFormat)
        {
            return "Unk0x0029";
        }
        break;
    }

    return "";
}
//---------------------------------------------------------------------------
d2ce::ActsInfo::ActsInfo()
{
}
//---------------------------------------------------------------------------
d2ce::ActsInfo::ActsInfo(const ActsInfo& other)
{
    *this = other;
}
//---------------------------------------------------------------------------
d2ce::ActsInfo::~ActsInfo()
{
}
//---------------------------------------------------------------------------
d2ce::ActsInfo& d2ce::ActsInfo::operator=(const ActsInfo& other)
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    Acts = other.Acts;
    quests_version = other.quests_version;
    waypoints_version = other.waypoints_version;
    Waypoints_unknown = other.Waypoints_unknown;
    Waypoints = other.Waypoints;
    Waypoints_extraBits = other.Waypoints_extraBits;
    quests_start_location = other.quests_start_location;
    quests_location = other.quests_location;
    waypoints_start_location = other.waypoints_start_location;
    waypoints_location = other.waypoints_location;
    npc_location = other.npc_location;
    update_locations = other.update_locations;
    Version = other.Version;
    return *this;
}
d2ce::ActsInfo& d2ce::ActsInfo::operator=(ActsInfo&& other) noexcept
{
    // Guard self assignment
    if (this == &other)
    {
        return *this;
    }

    Acts = other.Acts;
    other.Acts.fill(ActsInfoData());
    quests_version = other.quests_version;
    waypoints_version = other.waypoints_version;
    Waypoints_unknown = other.Waypoints_unknown;
    Waypoints = other.Waypoints;
    other.Waypoints.fill(0);
    Waypoints_extraBits = other.Waypoints_extraBits;
    for (auto& extraBit : other.Waypoints_extraBits)
    {
        extraBit.fill(0);
    }
    quests_start_location = std::exchange(other.quests_start_location, 0);
    quests_location = std::exchange(other.quests_location, 0);
    waypoints_start_location = std::exchange(other.waypoints_start_location, 0);
    waypoints_location = std::exchange(other.waypoints_location, 0);
    npc_location = std::exchange(other.npc_location, 0);
    update_locations = std::exchange(other.update_locations, true);
    Version = std::exchange(other.Version, APP_CHAR_VERSION);
    return *this;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::swap(ActsInfo& other)
{
    std::swap(*this, other);
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::clear()
{
    *this = ActsInfo();
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ActsInfo::getActIntroducedData(EnumDifficulty diff, EnumAct act) const
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    auto actNum = static_cast<std::underlying_type_t<EnumAct>>(act);
    if (actNum < 4)
    {
        return Acts[diffNum].Act[actNum].Intro;
    }

    return Acts[diffNum].ActV.Intro;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setActIntroducedData(EnumDifficulty diff, EnumAct act, std::uint16_t value, bool isExpansion) const
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    auto actNum = static_cast<std::underlying_type_t<EnumAct>>(act);
    if (actNum < 4)
    {
        Acts[diffNum].Act[actNum].Intro = value;
        return;
    }

    if (isExpansion)
    {
        Acts[diffNum].ActV.Intro = value;
    }
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getActIntroduced(EnumDifficulty diff, EnumAct act) const
{
    return getActIntroducedData(diff, act) != 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ActsInfo::getActCompletedData(EnumDifficulty diff, EnumAct act) const
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    auto actNum = static_cast<std::underlying_type_t<EnumAct>>(act);
    if (actNum < 3)
    {
        return Acts[diffNum].Act[actNum].Completed;
    }

    if (actNum == 3)
    {
        // "Completed" flag is at index 3 of the quests
        return Acts[diffNum].Act[actNum].Quests[3];
    }

    return Acts[diffNum].ActV.Completed;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setActCompletedData(EnumDifficulty diff, EnumAct act, std::uint16_t value, bool isExpansion) const
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    auto actNum = static_cast<std::underlying_type_t<EnumAct>>(act);
    if (actNum < 3)
    {
        Acts[diffNum].Act[actNum].Completed = value;
        if ((value != 0) && (Acts[diffNum].Act[actNum].Intro == 0))
        {
            // Make sure act intro is set properly
            Acts[diffNum].Act[actNum].Intro = 1;
        }
        return;
    }

    if (actNum == 3)
    {
        // "Completed" flag is at index 3 of the quests
        Acts[diffNum].Act[actNum].Quests[3] = value;
        if ((value != 0) && (Acts[diffNum].Act[actNum].Intro == 0))
        {
            // Make sure act intro is set properly
            Acts[diffNum].Act[actNum].Intro = 1;
        }
        return;
    }

    if (!isExpansion)
    {
        return;
    }

    Acts[diffNum].ActV.Completed = std::uint8_t(value);
    if ((value != 0) && (Acts[diffNum].ActV.Intro == 0))
    {
        // Make sure act intro is set properly
        Acts[diffNum].ActV.Intro = 1;
    }
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ActsInfo::getActVResetStatCompletedData(EnumDifficulty diff) const
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    std::uint16_t resetStats = Acts[diffNum].ActV.ResetStats;
    return (resetStats << 8) | Acts[diffNum].ActV.Completed;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::validateAct(EnumDifficulty diff, EnumAct act, bool isExpansion)
{
    if (act == EnumAct::V && !isExpansion)
    {
        act = EnumAct::IV;
    }

    if (!getActIntroduced(diff, act))
    {
        if (getActYetToStart(diff, act))
        {
            return;
        }

        setActIntroducedData(diff, act, 1, isExpansion);
    }

    auto actNum = static_cast<std::underlying_type_t<EnumAct>>(act);
    if (actNum > 0)
    {
        auto preAct = static_cast<EnumAct>(actNum - 1);
        if (!getActCompleted(diff, preAct))
        {
            setActCompletedData(diff, preAct, 1, isExpansion);
        }

        validateAct(diff, preAct, isExpansion);
    }
    else
    {
        auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
        if (diffNum > 0)
        {
            auto preDiff = static_cast<EnumDifficulty>(diffNum - 1);
            auto preAct = isExpansion ? EnumAct::V : EnumAct::IV;
            if (!getActCompleted(preDiff, preAct))
            {
                setActCompletedData(preDiff, preAct, 1, isExpansion);
            }

            validateAct(preDiff, preAct, isExpansion);
        }
    }

    // check the minumum requirements
    if (getActCompleted(diff, act))
    {
        // Check all required Quests
        for (std::uint8_t quest = 0; quest < (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS); ++quest)
        {
            if (getQuestIsRequired(diff, act, quest))
            {
                if (!getQuestCompleted(diff, act, quest))
                {
                    completeQuest(diff, act, quest);
                }
            }
        }
    }

    std::bitset<64> waypointValue = d2ce::ActsInfo::getWaypoints(diff);
    switch (act)
    {
    case EnumAct::I:
        waypointValue[0] = 1; // Rogue Encampement
        if (getActCompleted(diff, act))
        {
            waypointValue[MAX_WAYPOINTS_PER_ACT] = 1; // Lut Gholein
        }
        break;

    case EnumAct::II:
        waypointValue[MAX_WAYPOINTS_PER_ACT] = 1; // Lut Gholein
        if (getActCompleted(diff, act))
        {
            waypointValue[MAX_WAYPOINTS_PER_ACT * 2] = 1;// Kurast Docks 
        }
        break;

    case EnumAct::III:
        waypointValue[MAX_WAYPOINTS_PER_ACT * 2] = 1; // Kurast Docks 
        if (getActCompleted(diff, act))
        {
            waypointValue[MAX_WAYPOINTS_PER_ACT * 3] = 1; // The Pandemonium Fortress
        }
        break;

    case EnumAct::IV:
        waypointValue[MAX_WAYPOINTS_PER_ACT * 3] = 1; // The Pandemonium Fortress
        if (isExpansion && getActCompleted(diff, act))
        {
            waypointValue[MAX_WAYPOINTS_PER_ACT * 3 + NUM_WAYPOINTS_ACT_IV] = 1; // Harrogath
        }
        break;

    case EnumAct::V:
        if (isExpansion)
        {
            waypointValue[MAX_WAYPOINTS_PER_ACT * 3 + NUM_WAYPOINTS_ACT_IV] = 1; // Harrogath
        }
        break;
    }

    setWaypoints(diff, std::uint64_t(waypointValue.to_ullong()));
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getActCompleted(EnumDifficulty diff, EnumAct act) const
{
    return (getActCompletedData(diff, act) & 0x01) != 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getActYetToStart(EnumDifficulty diff, EnumAct act)
{
    if (getActIntroduced(diff, act))
    {
        return false;
    }

    for (std::uint8_t quest = 0; quest < (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS); ++quest)
    {
        if (!getQuestYetToStart(diff, act, quest))
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getQuestName(EnumAct act, std::uint8_t quest) const
{
    switch (act)
    {
    case EnumAct::I:
        switch (quest)
        {
        case 0:
            return "Den of Evil";

        case 1:
            return "Sisters' Burial Grounds";

        case 2:
            return "Tools of the Trade";

        case 3:
            return "The Search for Cain";

        case 4:
            return "The Forgotten Tower";

        case 5:
            return "Sisters to the Slaughter";
        }
        break;

    case EnumAct::II:
        switch (quest)
        {
        case 0:
            return "Radament's Lair";

        case 1:
            return "The Horadric Staff";

        case 2:
            return "Tainted Sun";

        case 3:
            return "Arcane Sanctuary";

        case 4:
            return "The Summoner";

        case 5:
            return "The Seven Tombs";
        }
        break;

    case EnumAct::III:
        switch (quest)
        {
        case 0:
            return "Lam Esen's Tome";

        case 1:
            return "Khalim's Will";

        case 2:
            return "Blade of the Old Religion";

        case 3:
            return "The Golden Bird";

        case 4:
            return "The Blackened Temple";

        case 5:
            return "The Guardian";
        }
        break;

    case EnumAct::IV:
        switch (quest)
        {
        case 0:
            return "The Fallen Angel";

        case 1:
            return "Terror's End";

        case 2:
            return "Hell's Forge";
        }
        break;

    case EnumAct::V:
        switch (quest)
        {
        case 0:
            return "Siege on Harrogath";

        case 1:
            return "Rescue on Mount Arreat";

        case 2:
            return "Prison of Ice";

        case 3:
            return "Betrayal of Harrogath";

        case 4:
            return "Rite of Passage";

        case 5:
            return "Eve of Destruction";
        }
    }

    return "";
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ActsInfo::getQuestData(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const
{
    if (quest >= (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS))
    {
        return 0;
    }

    return getQuestDataRef(diff, act, quest);
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setQuestData(EnumDifficulty diff, EnumAct act, std::uint8_t quest, std::uint16_t questValue)
{
    if (quest >= (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS))
    {
        return;
    }

    getQuestDataRef(diff, act, quest) = questValue;
}
//---------------------------------------------------------------------------
std::string d2ce::ActsInfo::getQuestNotes(EnumDifficulty diff, EnumAct act, std::uint8_t quest)
{
    std::string strTipText;
    if (quest >= (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS))
    {
        return strTipText;
    }

    const auto& questValue = getQuestDataRef(diff, act, quest);
    if (questValue == 0)
    {
        strTipText = "Quest Not Started";
        switch (act)
        {
        case EnumAct::I:
            switch (quest)
            {
            case 0:
                strTipText += "\n- Activated by talking to Akara";
                break;

            case 1:
                strTipText += "\n- Available after completing the Den of Evil";
                strTipText += "\n- Activated by talking to Kashya";
                break;

            case 2:
                strTipText += "\n- Available after completing the  The Forgotten Tower,\n  or after entering the Tamoe Highlands";
                strTipText += "\n- Activated by talking to Charsi";
                strTipText += "\n- Character must be at least level 8 to complete this\n  quest";
                break;

            case 3:
                strTipText += "\n- Available after completing the Sisters' Burial Grounds";
                strTipText += "\n- Akara informs the player of a Horadrim sage named\n  Deckard Cain";
                break;

            case 4:
                strTipText += "\n- Activated by reading Moldy Tome in Stony Field";
                break;

            case 5:
                strTipText += "\n- Activated by talking to Deckard Cain";
                break;
            }
            break;

        case EnumAct::II:
            switch (quest)
            {
            case 0:
                strTipText += "\n- Activated by talking to Atma";
                break;

            case 1:
                strTipText += "\n- Activated by bringing the Horadric Scroll that was\n  found in the Sewers, to Deckard Cain";
                break;

            case 2:
                strTipText += "\n- Activated when a player enters the Lost City";
                break;

            case 3:
                strTipText += "\n- Available after completing the Tainted Sun";
                strTipText += "\n- Activated by talking to Drognan";
                break;

            case 4:
                strTipText += "\n- Activated by approaching the platform in the Arcane\n  Sanctuary where The Summoner stands and Horazon's\n  Journal is located.";
                break;

            case 5:
                strTipText += "\n- Available after completing Radament's Lair, The\n  Horadric Staff or Tainted Sun or after reaching\n  level 20";
                strTipText += "\n- Activated by talking to Jerhyn";
                break;
            }
            break;

        case EnumAct::III:
            switch (quest)
            {
            case 0:
                strTipText += "\n- Available after completing the Blade of the Old\n  Religion";
                strTipText += "\n- Activated by talking to Alkor";
                break;

            case 1:
                strTipText += "\n- Available after completing The Golden Bird";
                strTipText += "\n- Activated by talking to Deckard Cain";
                break;

            case 2:
                strTipText += "\n- Available after completing The Golden Bird";
                strTipText += "\n- Activated by talking to Hratli";
                break;

            case 3:
                strTipText += "\n- Activated by killing the a random Unique Monster\n  in the Spider Forest";
                break;

            case 4:
                strTipText += "\n- Available after completing the Lam Esen's Tome";
                strTipText += "\n- Activated by talking to Ormus";
                break;

            case 5:
                strTipText += "\n- Activated by talking to Ormus after nearing the\n  Travincal or after smashing the Compelling Orb";
                break;
            }
            break;

        case EnumAct::IV:
            switch (quest)
            {
            case 0:
                strTipText += "\n- Activated by talking to Tyrael";
                break;

            case 1:
                strTipText += "\n- Available after completing the Hell's Forge";
                strTipText += "\n- Activated by talking to Tyrael";
                break;

            case 2:
                strTipText += "\n- Activated by talking to Deckard Cain";
                strTipText += "\n- Requires Mephisto's Soulstone";
                break;
            }
            break;

        case EnumAct::V:
            switch (quest)
            {
            case 0:
                strTipText += "\n- Activated by talking to Larzuk";
                break;

            case 1:
                strTipText += "\n- Available after completing the Siege on Harrogath";
                strTipText += "\n- Activated by talking to Qual-Kehk";
                break;

            case 2:
                strTipText += "\n- Available after completing the Rescue on Mount Arreat";
                strTipText += "\n- Activated by talking to Malah";
                break;

            case 3:
                strTipText += "\n- Activated by talking to Anya after rescuing her.";
                break;

            case 4:
                strTipText += "\n- Available after completing the Betrayal of Harrogath";
                strTipText += "\n- Activated by talking to Qual-Kehk";
                break;

            case 5:
                strTipText += "\n- Activated after completing the Rite of Passage";
                break;
            }
            break;
        }
        return strTipText;
    }

    if ((questValue & questCompletedFlag) == 0)
    {
        strTipText = "Quest Started";
    }
    else
    {
        strTipText = "Quest Completed";
    }

    switch (act)
    {
    case EnumAct::I:
        switch (quest)
        {
        case 0:
            if ((questValue & 0x10) != 0)
            {
                strTipText += "\n- Entered The Den of Evil";
            }
            break;

        case 1:
            if ((questValue & 0x10) != 0)
            {
                strTipText += "\n- Entered The Burial Grounds";
            }
            break;

        case 2:
            if ((questValue & 0x40) != 0)
            {
                strTipText += "\n- Picked Up Horadric Malus";
            }
            break;

        case 3:
            if ((questValue & 0x10) != 0)
            {
                strTipText += "\n- Entered Tristram";
            }

            if ((questValue & 0x4000) != 0)
            {
                strTipText += "\n- Rogues Resued Cain";
            }

            if ((questValue & questMooMooFarm) != 0)
            {
                strTipText += "\n- Secret Cow Level Completed";
            }
            break;

        case 4:
            if ((questValue & 0x04) != 0)
            {
                strTipText += "\n- Read Moldy Tome";
            }

            if ((questValue & 0x40) != 0)
            {
                strTipText += "\n- Entered The Forgotten Tower";
            }
            break;
        }
        break;

    case EnumAct::II:
        switch (quest)
        {
        case 0:
            if ((questValue & 0x10) != 0)
            {
                strTipText += "\n- Found Radament";
            }
            break;

        case 1:
            if ((questValue & 0x10) != 0)
            {
                strTipText += "\n- Cain Talked About The Viper Amulet";
            }

            if ((questValue & 0x20) != 0)
            {
                strTipText += "\n- Cain Talked About The Staff of Kings";
            }

            if ((questValue & 0x400) != 0)
            {
                strTipText += "\n- Cain Talked About The Horadric Staff";
            }

            if ((questValue & 0x800) != 0)
            {
                strTipText += "\n- You Made The Horadric Staff";
            }
            break;

        case 2:
            if ((questValue & 0x04) != 0)
            {
                strTipText += "\n- Sun Went Out";
            }

            if ((questValue & 0x40) != 0)
            {
                strTipText += "\n- Talked To Drogan About the Sun";
            }
            break;

        case 5:
            if ((questValue & 0x08) != 0)
            {
                strTipText += "\n- Talked To Tyrael";
            }

            if ((questValue & 0x10) != 0)
            {
                strTipText += "\n- Talked To Jerhyn";
            }

            if ((questValue & 0x20) != 0)
            {
                strTipText += "\n- Killed Duriel";
            }

            if ((questValue & 0x40) != 0)
            {
                strTipText += "\n- Atma Congratulated You";
            }

            if ((questValue & 0x80) != 0)
            {
                strTipText += "\n- Warriv Congratulated You";
            }

            if ((questValue & 0x0100) != 0)
            {
                strTipText += "\n- Drognan Congratulated You";
            }

            if ((questValue & 0x0200) != 0)
            {
                strTipText += "\n- Lysander Congratulated You";
            }

            if ((questValue & 0x0400) != 0)
            {
                strTipText += "\n- Cain Congratulated You";
            }

            if ((questValue & 0x0800) != 0)
            {
                strTipText += "\n- Fara Congratulated You";
            }
            break;
        }
        break;

    case EnumAct::III:
        switch (quest)
        {
        case 2:
            if ((questValue & 0x04) != 0)
            {
                strTipText += "\n- Picked Up The Gidbinn";
            }

            if ((questValue & 0x08) != 0)
            {
                strTipText += "\n- Hratli Asked You To Find The Gidbinn";
            }
            break;

        case 3:
            if ((questValue & 0x04) != 0)
            {
                strTipText += "\n- Cain Talked About The Jade Figurine";
            }

            if ((questValue & 0x10) != 0)
            {
                strTipText += "\n- Cain Talked About The Golden Bird";
            }

            if ((questValue & 0x40) != 0)
            {
                strTipText += "\n- You Found The Jade Figurine";
            }

            if ((questValue & 0x20) != 0)
            {
                strTipText += "\n- You Have The Potion of Life";
            }
            break;
        }
        break;

    case EnumAct::V:
        switch (quest)
        {
        case 0:
            if ((questValue & 0x08) != 0)
            {
                strTipText += "\n- You Found Shenk";
            }

            if ((questValue & 0x20) != 0)
            {
                strTipText += "\n- Larzuk Offered To Socket An Item";
            }
            break;

        case 2:
            if ((questValue & 0x80) != 0)
            {
                strTipText += "\n- Your Read The Scroll of Resistance";
            }

            if ((questValue & 0x0100) != 0)
            {
                strTipText += "\n- You Rescued Anya And Talked To Malah";
            }
            break;

        case 5:
            if ((questValue & 0x10) != 0)
            {
                strTipText += "\n- Larzuk Congratulated You";
            }

            if ((questValue & 0x20) != 0)
            {
                strTipText += "\n- Cain Congratulated You";
            }

            if ((questValue & 0x40) != 0)
            {
                strTipText += "\n- Malah Congratulated You";
            }

            if ((questValue & 0x80) != 0)
            {
                strTipText += "\n- Tyrael Congratulated You";
            }

            if ((questValue & 0x0100) != 0)
            {
                strTipText += "\n- Qual-Kehk Congratulated You";
            }

            if ((questValue & 0x0200) != 0)
            {
                strTipText += "\n- Anya Congratulated You";
            }
            break;
        }
        break;
    }

    return strTipText;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getQuestIsRequired(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const
{
    if (!getActCompleted(diff, act))
    {
        return false;
    }

    switch (act)
    {
    case d2ce::EnumAct::I:
        switch (quest)
        {
        case 5: // Sisters to the Slaughter
            return true;
        }
        break;

    case EnumAct::II:
        switch (quest)
        {
        case 2: // Tainted Sun
            return true;

        case 3: // Arcane Sanctuary
            return true;

        case 4: // The Summoner
            return true;

        case 5: // The Seven Tombs
            return true;
        }
        break;

    case EnumAct::III:
        switch (quest)
        {
        case 1: // // Khalims Will
            return true;

        case 4: // The Blackened Temple
            return true;

        case 5: // The Guardian
            return true;
        }
        break;

    case EnumAct::IV:
        switch (quest)
        {
        case 1: // Terrors End
            return true;
        }
        break;

    case EnumAct::V:
        switch (quest)
        {
        case 5: // Eve Of Destruction
            return true;
        }
        break;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getQuestYetToStart(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const
{
    return getQuestData(diff, act, quest) == questNotStarted ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getQuestStarted(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const
{
    auto questValue = getQuestData(diff, act, quest);
    if (questValue == questNotStarted)
    {
        return false; // not started
    }

    if ((questValue & questCompletedFlag) == 0)
    {
        return true;
    }

    return false; // completed
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::startQuest(EnumDifficulty diff, EnumAct act, std::uint8_t quest)
{
    if (quest >= (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS))
    {
        return;
    }

    getQuestDataRef(diff, act, quest) = questStarted;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getQuestCompleted(EnumDifficulty diff, EnumAct act, std::uint8_t quest) const
{
    auto questValue = getQuestData(diff, act, quest);
    if (questValue == 0 || (questValue & questCompletedFlag) == 0)
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::completeQuest(EnumDifficulty diff, EnumAct act, std::uint8_t quest)
{
    if (quest >= (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS))
    {
        return;
    }

    auto actNum = static_cast<std::underlying_type_t<EnumAct>>(act);
    getQuestDataRef(diff, act, quest) = questCompleted[actNum][quest];
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::resetQuest(EnumDifficulty diff, EnumAct act, std::uint8_t quest)
{
    if (quest >= (act == EnumAct::IV ? NUM_OF_QUESTS_ACT_IV : NUM_OF_QUESTS))
    {
        return;
    }

    getQuestDataRef(diff, act, quest) = questNotStarted;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getStatsReset(EnumDifficulty diff) const
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    return Acts[diffNum].ActV.ResetStats != 0 ? true : false;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setStatsReset(EnumDifficulty diff)
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    Acts[diffNum].ActV.ResetStats = 1;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::clearStatsReset(EnumDifficulty diff)
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    Acts[diffNum].ActV.ResetStats = 0;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getMooMooFarmComplete(EnumDifficulty diff) const
{
    const auto& questValue = getQuestDataRef(diff, EnumAct::I, 3);
    return ((questValue & questMooMooFarm) != 0) ? true : false;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setMooMooFarmComplete(EnumDifficulty diff)
{
    auto& questValue = getQuestDataRef(diff, EnumAct::I, 3);
    if (questValue != questNotStarted)
    {
        questValue |= questMooMooFarm;
    }
    else
    {
        questValue &= ~questMooMooFarm;
    }
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::clearMooMooFarmComplete(EnumDifficulty diff)
{
    auto& questValue = getQuestDataRef(diff, EnumAct::I, 3);
    questValue &= ~questMooMooFarm;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::drankPotionOfLife(EnumDifficulty diff) const
{
    if (!getQuestCompleted(diff, EnumAct::III, 3))
    {
        return false;
    }

    // The Golden Bird
    const auto& questValue = getQuestDataRef(diff, EnumAct::III, 3);
    if ((questValue & 0x20) != 0)
    {
        // Character still has the potion of life
        return false;
    }

    return true;

}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::readTheScrollOfResistance(EnumDifficulty diff) const
{
    if (!getQuestCompleted(diff, EnumAct::V, 2))
    {
        return false;
    }

    // Prison of Ice
    const auto& questValue = getQuestDataRef(diff, EnumAct::V, 2);
    if ((questValue & 0x80) != 0)
    {
        // Character has read The Scroll of Resistance
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ActsInfo::getLifePointsEarned() const
{
    std::uint16_t points = 0;
    static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };
    for (auto diff : all_diff)
    {
        if (drankPotionOfLife(diff))
        {
            points += 20;
        }
    }

    return points;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ActsInfo::getSkillPointsEarned() const
{
    std::uint16_t skills = 0;
    static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };
    for (auto diff : all_diff)
    {
        // Den of Evil
        if (getQuestCompleted(diff, EnumAct::I, 0))
        {
            ++skills;
        }

        // Radament's Lair
        if (getQuestCompleted(diff, EnumAct::II, 0))
        {
            ++skills;
        }

        // The Fallen Angel
        if (getQuestCompleted(diff, EnumAct::IV, 0))
        {
            skills += 2;
        }
    }

    return skills;
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::ActsInfo::getStatPointsEarned() const
{
    std::uint16_t stats = 0;
    static std::initializer_list<EnumDifficulty> all_diff = { EnumDifficulty::Normal, EnumDifficulty::Nightmare, EnumDifficulty::Hell };
    for (auto diff : all_diff)
    {
        // Lam Esen's Tome
        if (getQuestCompleted(diff, EnumAct::III, 0))
        {
            stats += 5;
        }
    }

    return stats;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::updateQuests(const ActsInfo& qi)
{
    Acts = qi.Acts;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::updateActs(const ActsInfo& qi)
{
    updateQuests(qi);
    Waypoints = qi.Waypoints;
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::ActsInfo::getWaypoints(d2ce::EnumDifficulty difficulty) const
{
    return Waypoints[static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(difficulty)];
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue)
{
    Waypoints[static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(difficulty)] = newvalue;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getWaypointActivated(EnumDifficulty diff, EnumAct act, std::uint8_t waypoint) const
{
    if (waypoint >= (act == EnumAct::IV ? NUM_WAYPOINTS_ACT_IV : MAX_WAYPOINTS_PER_ACT))
    {
        return false;
    }

    std::uint32_t offset = waypoint;
    if (act == EnumAct::V)
    {
        offset += MAX_WAYPOINTS_PER_ACT * 3 + NUM_WAYPOINTS_ACT_IV;

        // make sure Act V - Harrogath is active if applicable
        if ((waypoint == 0) && getActCompleted(diff, d2ce::EnumAct::IV))
        {
            return true;
        }
    }
    else if (act == EnumAct::IV)
    {
        offset += MAX_WAYPOINTS_PER_ACT * 3;

        // make sure Act IV - Pandemonium Fortress is active if applicable
        if ((waypoint == 0) && getActCompleted(diff, d2ce::EnumAct::III))
        {
            return true;
        }
    }
    else if (act == EnumAct::III)
    {
        offset += MAX_WAYPOINTS_PER_ACT * 2;

        // make sure Act III - Kurast Docks is active if applicable
        if ((waypoint == 0) && getActCompleted(diff, d2ce::EnumAct::II))
        {
            return true;
        }
    }
    else if (act == EnumAct::II)
    {
        offset += MAX_WAYPOINTS_PER_ACT;

        // make sure Act II - Lut Gholein is active if applicable
        if ((waypoint == 0) && getActCompleted(diff, d2ce::EnumAct::I))
        {
            return true;
        }
    }
    else if (waypoint == 0)
    {
        // make sure Act I - Rogue Encampment is always active
        return true;
    }

    std::bitset<d2ce::NUM_OF_WAYPOINTS> wp = getWaypoints(diff);
    return wp[offset] == 0 ? false : true;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::setWaypointActivated(EnumDifficulty diff, EnumAct act, std::uint8_t waypoint, bool flag)
{
    if (waypoint >= (act == EnumAct::IV ? NUM_WAYPOINTS_ACT_IV : MAX_WAYPOINTS_PER_ACT))
    {
        return false;
    }

    std::uint32_t offset = waypoint;
    if (act == EnumAct::V)
    {
        offset += MAX_WAYPOINTS_PER_ACT * 3 + NUM_WAYPOINTS_ACT_IV;

        // make sure Act V - Harrogath is active if applicable
        if ((waypoint == 0) && getActCompleted(diff, d2ce::EnumAct::IV))
        {
            flag = true;
        }
    }
    else if (act == EnumAct::IV)
    {
        offset += MAX_WAYPOINTS_PER_ACT * 3;

        // make sure Act IV - Pandemonium Fortress is active if applicable
        if ((waypoint == 0) && getActCompleted(diff, d2ce::EnumAct::III))
        {
            flag = true;
        }
    }
    else if (act == EnumAct::III)
    {
        offset += MAX_WAYPOINTS_PER_ACT * 2;

        // make sure Act III - Kurast Docks is active if applicable
        if ((waypoint == 0) && getActCompleted(diff, d2ce::EnumAct::II))
        {
            flag = true;
        }
    }
    else if (act == EnumAct::II)
    {
        offset += MAX_WAYPOINTS_PER_ACT;

        // make sure Act II - Lut Gholein is active if applicable
        if ((waypoint == 0) && getActCompleted(diff, d2ce::EnumAct::I))
        {
            flag = true;
        }
    }
    else if (waypoint == 0)
    {
        // make sure Act I - Rogue Encampment is always active
        flag = true;
    }

    std::bitset<d2ce::NUM_OF_WAYPOINTS> wp = getWaypoints(diff);
    wp[offset] = flag;
    setWaypoints(diff, wp.to_ullong());
    return flag;
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::ActsInfo::getNpcIntroductions(d2ce::EnumDifficulty difficulty) const
{
    return NPCIntroductions[static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(difficulty)];
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::ActsInfo::getNpcCongrats(d2ce::EnumDifficulty difficulty) const
{
    return NPCCongrats[static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(difficulty)];
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setNpcIntroductions(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue)
{
    NPCIntroductions[static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(difficulty)] = newvalue;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setNpcCongrats(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue)
{
    NPCCongrats[static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(difficulty)] = newvalue;
}
//---------------------------------------------------------------------------
