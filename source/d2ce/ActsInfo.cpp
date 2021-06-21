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

#include "pch.h"
#include <cstdint>
#include <locale>
#include "ActsInfo.h"

namespace d2ce
{
    constexpr std::uint8_t QUESTS_MARKER[] = { 0x57, 0x6F, 0x6F, 0x21 }; // alternatively "Woo!"
    constexpr std::uint8_t QUESTS_SIZE_MARKER[] = { 0x2A, 0x01 };

    constexpr std::uint32_t MIN_START_QUEST_POS = 335;

    constexpr std::uint8_t WAYPOINTS_MARKER[] = { 0x57, 0x53 };          // alternatively "WS"
    constexpr std::uint8_t WAYPOINTS_SIZE_MARKER[] = { 0x50, 0x00 };

    constexpr std::uint32_t MIN_START_WAYPOINTS_POS = 633;

    constexpr std::uint16_t questNotStarted = 0x0000;
    constexpr std::uint16_t questStarted = 0x0004;
    constexpr std::uint16_t questCompletedFlag = 0x9001;

    static const std::uint16_t questCompleted[][d2ce::NUM_OF_QUESTS] = {
        {0x1001,0x101d,0x900d,0x101d,0x1055,0x101d}, // Act I
        {0x101d,0x1c39,0x100d,0x1181,0x1005,0x1e25}, // Act II
        {0x1001,0x10fd,0x11d9,0x1001,0x100d,0x1871}, // Act III
        {0x1001,0x1301,0x1001,0,0,0},                // Act IV
        {0x9021,0x1001,0x178d,0x901d,0x132d,0x169d}  // Act V
    };

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
            std::fread(quests_version, sizeof(quests_version), 1, charfile);
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
        return std::fread(Acts, sizeof(Acts), 1, charfile) == 1 ? true : false;
    }

    // Only copy 4 acts of Quest data for each difficulty
    for (std::uint32_t i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        if (feof(charfile))
        {
            return false;
        }

        if (std::fread(&Acts[i].Act, sizeof(Acts[i].Act), 1, charfile) != 1)
        {
            return false;
        }

        std::fseek(charfile, 28, SEEK_CUR);
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
            std::fread(waypoints_version, sizeof(waypoints_version), 1, charfile);
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
        std::fread(&Waypoints_extraBits[i], sizeof(Waypoints_extraBits[i]), 1, charfile); // skip extra bits
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
    std::fwrite(QUESTS_MARKER, sizeof(QUESTS_MARKER), 1, charfile);
    std::fwrite(quests_version, sizeof(quests_version), 1, charfile);
    std::fwrite(QUESTS_SIZE_MARKER, sizeof(QUESTS_SIZE_MARKER), 1, charfile);
    if (Version >= EnumCharVersion::v107)
    {
        bool ret = std::fwrite(Acts, sizeof(Acts), 1, charfile) != 1 ? false : true;
        std::fflush(charfile);
        return ret;
    }
    else
    {
        // Only copy 4 acts of Quest data for each difficulty
        for (std::uint32_t i = 0; i < NUM_OF_DIFFICULTY; ++i)
        {
            if (std::fwrite(&Acts[i].Act, sizeof(Acts[i].Act), 1, charfile) != 1)
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
    std::fwrite(WAYPOINTS_MARKER, sizeof(WAYPOINTS_MARKER), 1, charfile);
    std::fwrite(waypoints_version, sizeof(waypoints_version), 1, charfile);
    std::fwrite(WAYPOINTS_SIZE_MARKER, sizeof(WAYPOINTS_SIZE_MARKER), 1, charfile);
    for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
    {
        std::fwrite(&Waypoints_unknown[i], sizeof(Waypoints_unknown[i]), 1, charfile); // skip 0x0102 marker
        std::fwrite(&Waypoints[i], sizeof(Waypoints[i]), 1, charfile);
        std::fwrite(&Waypoints_extraBits[i], sizeof(Waypoints_extraBits[i]), 1, charfile); // skip extra bits
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

    return writeWaypoints(charfile);
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

    std::memcpy(Acts, other.Acts, sizeof(Acts));
    std::memcpy(quests_version, other.quests_version, sizeof(quests_version));
    std::memcpy(waypoints_version, other.waypoints_version, sizeof(waypoints_version));
    std::memcpy(Waypoints_unknown, other.Waypoints_unknown, sizeof(Waypoints_unknown));
    std::memcpy(Waypoints, other.Waypoints, sizeof(Waypoints));
    std::memcpy(Waypoints_extraBits, other.Waypoints_extraBits, sizeof(Waypoints_extraBits));
    quests_start_location = other.quests_start_location;
    quests_location = other.quests_location;
    waypoints_start_location = other.waypoints_start_location;
    waypoints_location = other.waypoints_location;
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

    std::memcpy(Acts, other.Acts, sizeof(Acts));
    std::memset(other.Acts, 0, sizeof(other.Acts));
    std::memcpy(quests_version, other.quests_version, sizeof(quests_version));
    std::memcpy(waypoints_version, other.waypoints_version, sizeof(waypoints_version));
    std::memcpy(Waypoints_unknown, other.Waypoints_unknown, sizeof(Waypoints_unknown));
    std::memcpy(Waypoints, other.Waypoints, sizeof(Waypoints));
    std::memset(other.Waypoints, 0, sizeof(other.Waypoints));
    std::memcpy(Waypoints_extraBits, other.Waypoints_extraBits, sizeof(Waypoints_extraBits));
    std::memset(other.Waypoints_extraBits, 0, sizeof(other.Waypoints_extraBits));
    quests_start_location = std::exchange(other.quests_start_location, 0);
    quests_location = std::exchange(other.quests_location, 0);
    waypoints_start_location = std::exchange(other.waypoints_start_location, 0);
    waypoints_location = std::exchange(other.waypoints_location, 0);
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
bool d2ce::ActsInfo::getActIntroduced(EnumDifficulty diff, EnumAct act) const
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    auto actNum = static_cast<std::underlying_type_t<EnumAct>>(act);
    if (actNum < 4)
    {
        return Acts[diffNum].Act[actNum].Intro;
    }

    if (actNum == 4)
    {
        return Acts[diffNum].ActV.Intro;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::ActsInfo::getActCompleted(EnumDifficulty diff, EnumAct act) const
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
        return Acts[diffNum].Act[actNum].Quests[3] != 0 ? true : false;
    }

    return false;
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
    return Acts[diffNum].ActV.ResetStats;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::setStatsReset(EnumDifficulty diff)
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    Acts[diffNum].ActV.ResetStats = true;
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::clearStatsReset(EnumDifficulty diff)
{
    auto diffNum = static_cast<std::underlying_type_t<EnumDifficulty>>(diff);
    Acts[diffNum].ActV.ResetStats = false;
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
    if (!getQuestCompleted(diff, EnumAct::I, 4))
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
    std::memcpy(Acts, qi.Acts, sizeof(Acts));
}
//---------------------------------------------------------------------------
void d2ce::ActsInfo::updateActs(const ActsInfo& qi)
{
    updateQuests(qi);
    std::memcpy(Waypoints, qi.Waypoints, sizeof(Waypoints));
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
