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

#include "pch.h"
#include <filesystem>
#include "Character.h"
#include "CharacterConstants.h"
#include "ExperienceConstants.h"
#include "ItemConstants.h"
#include "SkillConstants.h"
#include "SharedStash.h"
#include <fstream>

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::array<std::uint8_t, 4> HEADER = { 0x55, 0xAA, 0x55, 0xAA };

    constexpr std::array<std::uint8_t, 6> UNKNOWN_01C_v100 = { 0xDD, 0x00, 0x10, 0x00, 0x82, 0x00 };
    constexpr std::array<std::uint8_t, 6> UNKNOWN_01C_v107 = { 0x3F, 0x01, 0x10, 0x00, 0x82, 0x00 };
    constexpr std::array<std::uint8_t, 2> UNKNOWN_026 = { 0x00, 0x00 };
    constexpr std::array<std::uint8_t, 2> UNKNOWN_029 = { 0x10, 0x1E };
    constexpr std::array<std::uint8_t, 4> UNKNOWN_034 = { 0xFF, 0xFF, 0xFF, 0xFF };
    constexpr std::array<std::uint8_t, 36> UNKNOWN_05A_v100 = { 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    constexpr std::array<std::uint8_t, 2> UNKNOWN_0AF = { 0x00, 0x00 };
    constexpr std::array<std::uint8_t, 140> UNKNOWN_0BF = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    constexpr std::array<std::uint16_t, 4> UNKOWN_14B_v109 = { 0x01, 0x00, 0x00, 0x00 };
    constexpr std::array<std::uint16_t, 4> UNKOWN_14B_v115 = { 0x00, 0x00, 0x00, 0x00 };

    void ApplyJsonAppearnces(const Json::Value& appearances, std::array<std::uint8_t, APPEARANCES_LENGTH> &appearancesValue)
    {
        if (!appearances.isNull())
        {
            size_t idx = 0;
            std::string key;
            auto iter_end = appearances.end();
            for (auto iter = appearances.begin(); iter != iter_end; ++iter)
            {
                if (iter->isNull())
                {
                    continue;
                }

                key = iter.name();
                idx = 0;
                if (_stricmp(key.c_str(), "torso") == 0)
                {
                    idx = 1;
                }
                else if (_stricmp(key.c_str(), "legs") == 0)
                {
                    idx = 2;
                }
                else if ((strcmp(key.c_str(), "right_arm") == 0) || (strcmp(key.c_str(), "RightArm") == 0))
                {
                    idx = 3;
                }
                else if ((strcmp(key.c_str(), "left_arm") == 0) || (strcmp(key.c_str(), "LeftArm") == 0))
                {
                    idx = 4;
                }
                else if ((strcmp(key.c_str(), "right_hand") == 0) || (strcmp(key.c_str(), "RightHand") == 0))
                {
                    idx = 5;
                }
                else if ((strcmp(key.c_str(), "left_hand") == 0) || (strcmp(key.c_str(), "LeftHand") == 0))
                {
                    idx = 6;
                }
                else if (_stricmp(key.c_str(), "shield") == 0)
                {
                    idx = 7;
                }
                else if (_stricmp(key.c_str(), "special1") == 0)
                {
                    idx = 8;
                }
                else if (_stricmp(key.c_str(), "special2") == 0)
                {
                    idx = 9;
                }
                else if (_stricmp(key.c_str(), "special3") == 0)
                {
                    idx = 10;
                }
                else if (_stricmp(key.c_str(), "special4") == 0)
                {
                    idx = 11;
                }
                else if (_stricmp(key.c_str(), "special5") == 0)
                {
                    idx = 12;
                }
                else if (_stricmp(key.c_str(), "special6") == 0)
                {
                    idx = 13;
                }
                else if (_stricmp(key.c_str(), "special7") == 0)
                {
                    idx = 14;
                }
                else if (_stricmp(key.c_str(), "special8") == 0)
                {
                    idx = 15;
                }

                auto iter2_end = iter->end();
                for (auto iter2 = iter->begin(); iter2 != iter2_end; ++iter2)
                {
                    if (iter2->isNull())
                    {
                        continue;
                    }

                    key = iter2.name();
                    if (_stricmp(key.c_str(), "tint") == 0)
                    {
                        idx += 16;
                    }
                    else if (_stricmp(key.c_str(), "graphic") != 0)
                    {
                        continue;
                    }

                    appearancesValue[idx] = std::uint8_t(iter2->asInt());
                }
            }
        }
    }

    std::uint32_t ApplyJsonSkill(const Json::Value& skill)
    {
        if (skill.isNull())
        {
            return 0xFFFF;
        }

        if (skill.isObject())
        {
            Json::Value value = skill["Id"];
            if (value.isNull())
            {
                return 0xFFFF;
            }

            return std::min(std::uint32_t(value.asInt()), std::uint32_t(0xFFFF));
        }

        return std::min(CharacterStats::getSkillIdByName(skill.asString()), std::uint32_t(0xFFFF));
    }

    void ApplyJsonAssignedSkills(const Json::Value& assignedSkills, std::array<std::uint32_t, NUM_OF_SKILL_HOTKEYS>& assignedSkillsValue)
    {
        Json::Value value;
        if (!assignedSkills.isNull())
        {
            std::uint32_t id = 0xFFFF;
            size_t idx = 0;
            auto iter_end = assignedSkills.end();
            for (auto iter = assignedSkills.begin(); iter != iter_end && idx < NUM_OF_SKILL_HOTKEYS; ++iter, ++idx)
            {
                id = ApplyJsonSkill(*iter);
                if (id < 0xFFFF)
                {
                    assignedSkillsValue[idx] = id;
                }
            }
        }
    }

    std::uint8_t ApplyJsonStartingActDifficulty(const Json::Value& difficulty)
    {
        if (difficulty.isNull())
        {
            return 0;
        }

        if (difficulty.isObject())
        {
            Json::Value value = difficulty["Active"];
            if (value.isNull() || !value.asBool())
            {
                return 0;
            }

            std::uint8_t act = 0;
            value = difficulty["Act"];
            if (!value.isNull())
            {
                act = std::uint8_t(value.asInt());
                if (act > 0)
                {
                    act = std::min(std::uint8_t(act - 1), std::uint8_t(d2ce::NUM_OF_ACTS - 1));
                }
            }

            return std::uint8_t(0x80) | act;
        }

        return std::uint8_t(difficulty.asInt());
    }

    void ApplyJsonStartingAct(const Json::Value& location, std::array<std::uint8_t, NUM_OF_DIFFICULTY>& startingAct)
    {
        Json::Value value;
        if (!location.isNull())
        {
            std::string key;
            size_t idx = 0;
            auto iter_end = location.end();
            for (auto iter = location.begin(); iter != iter_end; ++iter)
            {
                if (iter->isNull())
                {
                    continue;
                }

                key = iter.name();
                if (_stricmp(key.c_str(), "Normal") == 0)
                {
                    idx = 0;
                }
                else if (_stricmp(key.c_str(), "Nightmare") == 0)
                {
                    idx = 1;
                }
                else if (_stricmp(key.c_str(), "Hell") == 0)
                {
                    idx = 2;
                }
                else
                {
                    continue;
                }

                startingAct[idx] = ApplyJsonStartingActDifficulty(*iter);
            }
        }
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
const char* d2ce::CharacterErrCategory::name() const noexcept
{
    return "d2s_character";
}
//---------------------------------------------------------------------------
std::string d2ce::CharacterErrCategory::message(int ev) const
{
    switch (static_cast<CharacterErrc>(ev))
    {
    case CharacterErrc::InvalidHeader:
        return "Not a valid Diablo II character file.";

    case CharacterErrc::CannotOpenFile:
        return "Could not open file. Verify that file is not in use by another application.";

    case CharacterErrc::InvalidChecksum:
        return "Corrupt Diablo II character file detected. Invalid checksum value.";

    case CharacterErrc::InvalidActsInfo:
        return "Corrupt Diablo II character file detected. Unable to find act information.";

    case CharacterErrc::InvalidCharStats:
        return "Corrupt Diablo II character file detected. Unable to find character's Stats.";

    case CharacterErrc::InvalidCharSkills:
        return "Corrupt Diablo II character file detected. Unable to find character's Skills.";

    case CharacterErrc::InvalidItemInventory:
        return "Corrupt Diablo II character file detected. Invalid item inventory.";

    case CharacterErrc::FileRenameError:
        return "Unable to rename Diablo II character file.";

    case CharacterErrc::AuxFileRenameError:
        return "One or more auxiliary Character Files (.key, .ma*) could not be renamed.";

    default:
        return "(unrecognized error)";
    }
}
//---------------------------------------------------------------------------
const d2ce::CharacterErrCategory theCharacterErrCategory{};

namespace std
{
    std::error_code make_error_code(d2ce::CharacterErrc e)
    {
        return { static_cast<int>(e), theCharacterErrCategory };
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
d2ce::Character::Character() : Merc(*this), Acts(*this)
{
    initialize();
}
//---------------------------------------------------------------------------
d2ce::Character::~Character()
{
    if (m_charfile != nullptr)
    {
        std::fclose(m_charfile);
    }
}
//---------------------------------------------------------------------------
void d2ce::Character::initialize()
{
    Header = HEADER;
    Version = static_cast<std::underlying_type_t<EnumCharVersion>>(APP_CHAR_VERSION);
    FileSize = 0;
    Checksum = 0;
    WeaponSet = 0;
    Bs.Name.fill(0);

    Bs.Status = EnumCharStatus::NoDeaths;
    Bs.Title = 0;
    Bs.Class = EnumCharClass::Amazon;
    DisplayLevel = 1;
    LastPlayed = 0;
    AssignedSkills.fill(0xFFFF);
    LeftSkill = 0;
    RightSkill = 0;
    LeftSwapSkill = 0;
    RightSwapSkill = 0;
    Appearances.fill(0xFF);

    Bs.DifficultyLastPlayed = EnumDifficulty::Normal;
    StartingAct.fill(0);
    Bs.StartingAct = EnumAct::I;
    Merc.clear();
    Acts.clear();

    Cs.clear();

    m_error_code.clear();

    m_items.clear();

    m_filesize_location = 0;
    m_checksum_location = 0;
    m_name_location = 0;
    m_class_location = 0;
    m_level_location = 0;
    m_starting_location = 0;
    m_assigned_skilled_location = 0;
    m_appearances_location = 0;
    m_difficulty_location = 0;
    m_mapid_location = 0;
    m_stats_header_location = 0;
    m_update_locations = true;

    m_shared_stash.clear();

    if (m_charfile != nullptr)
    {
        std::rewind(m_charfile);
    }
}
//---------------------------------------------------------------------------
/*
   Returns false if file was not opened or there was an error.
*/
bool d2ce::Character::openD2S(const char* szfilename, bool validateChecksum)
{
    if (is_open())
    {
        close();
    }

    m_error_code.clear();
#ifdef _MSC_VER
    m_charfile = _fsopen(szfilename, "rb+", _SH_DENYNO);
#else
    errno_t err = fopen_s(&m_charfile, szfilename, "rb+");
    if (err != 0)
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }
#endif

    m_d2sfilename = szfilename;
    if (m_charfile == nullptr)
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    readHeader();
    if (!isValidHeader())
    {
        m_error_code = std::make_error_code(CharacterErrc::InvalidHeader);
        close();
        return false;
    }

    if (!refresh())
    {
        return false;
    }

    // Validate checksum
    long curChecksum = Checksum;
    calculateChecksum();
    if (curChecksum != Checksum)
    {
        m_error_code = std::make_error_code(CharacterErrc::InvalidChecksum);
        if (validateChecksum)
        {
            close();
            return false;
        }
    }

    m_update_locations = false;
    m_shared_stash.reset(*this);
    return true;
}
//---------------------------------------------------------------------------
/*
   Returns false if file was not opened or there was an error.
*/
bool d2ce::Character::openJson(const char* szjsonfilename)
{
    if (is_open())
    {
        close();
    }

    std::ifstream ifs;
    ifs.open(szjsonfilename);
    if (!ifs.is_open())
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }
    m_jsonfilename = szjsonfilename;

    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
    {
        m_error_code = std::make_error_code(CharacterErrc::InvalidHeader);
        return false;
    }

    m_bJsonSerializedFormat = false;
    readHeader(root);
    if (!isValidHeader())
    {
        initialize();
        m_error_code = std::make_error_code(CharacterErrc::InvalidHeader);
        return false;
    }

    m_d2sfilename.clear();
    char name1[L_tmpnam_s];
    errno_t err = tmpnam_s(name1, L_tmpnam_s);
    if (err != 0)
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    m_charfile = NULL;
    fopen_s(&m_charfile, name1, "wb+");
    if (m_charfile == nullptr)
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    m_d2sfilename = name1;
    std::fwrite(Header.data(), Header.size(), 1, m_charfile);

    if (!refresh(root))
    {
        return false;
    }

    m_update_locations = false;
    return true;
}
//---------------------------------------------------------------------------
/*
   Returns false if file was not opened or there was an error.
*/
bool d2ce::Character::open(const char* szfilename, bool validateChecksum)
{
    if (is_open())
    {
        close();
    }

    m_error_code.clear();

    if (szfilename != nullptr)
    {
        std::filesystem::path p = szfilename;
        std::string ext = p.extension().string();
        if (_stricmp(ext.c_str(), ".json") == 0)
        {
            return openJson(szfilename);
        }
    }

    return openD2S(szfilename, validateChecksum);
}
//---------------------------------------------------------------------------
/*
   Thanks goes to Stoned2000 for making his checksum calculation source
   available to the public.  The Visual Basic source for his editor can be
   found at http://stoned.d2network.com.
*/
void d2ce::Character::calculateChecksum()
{
    // make sure we start at the beginning of the file
    std::rewind(m_charfile);

    Checksum = 0;

    if ((m_checksum_location == 0) || (Bs.Version < EnumCharVersion::v109))
    {
        // checksum not supported, calculate the file size only for possible later use
        std::fseek(m_charfile, 0, SEEK_END);
        FileSize = std::ftell(m_charfile);
        std::rewind(m_charfile);
        return;
    }

    std::uint32_t i = 0;
    std::uint8_t data, overflow = 0;
    for (; i < m_checksum_location; ++i)
    {
        // doubles the checksum result by left shifting once
        Checksum <<= 1;

        std::fread(&data, sizeof(data), 1, m_charfile);

        Checksum += data + overflow;

        if (Checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    // skip checksum location
    data = 0;
    std::fseek(m_charfile, sizeof(Checksum), SEEK_CUR);
    std::uint32_t nextStop = std::ftell(m_charfile);
    for (; i < nextStop; ++i)
    {
        // doubles the checksum result by left shifting once
        Checksum <<= 1;

        Checksum += data + overflow;

        if (Checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    // continue with the rest of the file
    for (; i < FileSize; ++i)
    {
        // doubles the checksum result by left shifting once
        Checksum <<= 1;

        std::fread(&data, sizeof(data), 1, m_charfile);

        Checksum += data + overflow;

        if (Checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    // rewind the file in case we need to read/write to it again
    std::rewind(m_charfile);
}
//---------------------------------------------------------------------------
void d2ce::Character::readHeader()
{
    std::rewind(m_charfile);
    std::fread(Header.data(), Header.size(), 1, m_charfile);
}
//---------------------------------------------------------------------------
void d2ce::Character::readHeader(const Json::Value& root)
{
    m_bJsonSerializedFormat = false;
    Json::Value header = root["header"];
    if (header.isNull())
    {
        header = root["Header"];
        if (header.isNull())
        {
            return;
        }

        m_bJsonSerializedFormat = true;
    }

    Json::Value value = header[m_bJsonSerializedFormat ? "Magic" : "identifier"];
    std::uint32_t uint = m_bJsonSerializedFormat ? std::uint32_t(value.asInt64()) : std::uint32_t(std::stoul(value.asString(), nullptr, 16));
    std::memcpy(Header.data(), &uint, Header.size());
}
//---------------------------------------------------------------------------
/*
   Returns true if the file has a valid header
*/
bool d2ce::Character::isValidHeader() const
{
    return Header == HEADER ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Character::refresh(const Json::Value& root)
{
    if (!readBasicInfo(root))
    {
        // bad file
        std::error_code error = m_error_code; // capture any existing error
        if (!error)
        {
            error = std::make_error_code(CharacterErrc::InvalidHeader);
        }

        close();
        m_error_code = error;
        return false;
    }

    if (!readActs(root))
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidActsInfo);
        return false;
    }

    // From this point on, the location is variable
    if (!readStats(root))
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidCharStats);
        return false;
    }

    // Read Character, Corpse, Mercenary and Golem items
    if (!readItems(root))
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidItemInventory);
        return false;
    }

    // store the file's size
    std::fflush(m_charfile);
    std::fseek(m_charfile, 0, SEEK_END);
    FileSize = std::ftell(m_charfile);

    // determine if a checksum needs to be calculated and stored
    if (Bs.Version >= EnumCharVersion::v109)
    {
        std::fseek(m_charfile, m_filesize_location, SEEK_SET);
        std::fwrite(&FileSize, sizeof(FileSize), 1, m_charfile);

        // make sure the checksum is zero in the file
        Checksum = 0;
        m_checksum_location = std::ftell(m_charfile);
        std::fwrite(&Checksum, sizeof(Checksum), 1, m_charfile);

        std::fflush(m_charfile);
        calculateChecksum();

        // write the checksum into the file
        std::fseek(m_charfile, m_checksum_location, SEEK_SET);
        std::fwrite(&Checksum, sizeof(Checksum), 1, m_charfile);
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::refresh()
{
    readBasicInfo();
    if (!readActs())
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidActsInfo);
        return false;
    }

    // From this point on, the location is variable
    if (!readStats())
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidCharStats);
        return false;
    }

    // Read Character, Corpse, Mercenary and Golem m_items
    if (!readItems())
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidItemInventory);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Character::readBasicInfo()
{
    std::fseek(m_charfile, HEADER_LENGTH, SEEK_SET);
    std::fread(&Version, sizeof(Version), 1, m_charfile);

    Bs.Version = getVersion();
    Cs.Version = Bs.Version;

    m_filesize_location = 0;
    m_checksum_location = 0;
    if (Bs.Version >= EnumCharVersion::v109)
    {
        m_filesize_location = std::ftell(m_charfile);
        std::fread(&FileSize, sizeof(FileSize), 1, m_charfile);
        m_checksum_location = std::ftell(m_charfile);
        std::fread(&Checksum, sizeof(Checksum), 1, m_charfile);
        std::fread(&WeaponSet, sizeof(WeaponSet), 1, m_charfile);
    }

    m_name_location = std::ftell(m_charfile);
    std::fread(Bs.Name.data(), Bs.Name.size(), 1, m_charfile);
    Bs.Name[15] = 0; // must be zero
    std::uint8_t value = 0;
    std::fread(&value, sizeof(value), 1, m_charfile);
    Bs.Status = static_cast<EnumCharStatus>(value);

    // ladder is for 1.10 or higher
    if (Bs.Version < EnumCharVersion::v110)
    {
        Bs.Status &= ~EnumCharStatus::Ladder;

        if (Bs.Version < EnumCharVersion::v107 || Bs.Version == EnumCharVersion::v108)
        {
            // expansion not supported
            Bs.Status &= ~EnumCharStatus::Expansion;
        }
    }

    std::fread(&Bs.Title, sizeof(Bs.Title), 1, m_charfile);
    Bs.Title = std::min(Bs.Title, Bs.getGameCompleteTitle());

    if (Bs.Version < EnumCharVersion::v109)
    {
        std::fread(&value, sizeof(value), 1, m_charfile);
        WeaponSet = value;
        m_class_location = 34;
    }
    else
    {
        m_class_location = 40;
    }

    std::fseek(m_charfile, m_class_location, SEEK_SET);
    std::fread(&value, sizeof(value), 1, m_charfile);
    Bs.Class = static_cast<EnumCharClass>(value);

    if (Bs.Version < EnumCharVersion::v109)
    {
        m_level_location = 36;
    }
    else
    {
        m_level_location = 43;
    }

    std::fseek(m_charfile, m_level_location, SEEK_SET);
    std::fread(&DisplayLevel, sizeof(DisplayLevel), 1, m_charfile);

    if (Bs.Version < EnumCharVersion::v109)
    {
        if (Bs.Version < EnumCharVersion::v107)
        {
            m_starting_location = 38;
            std::fseek(m_charfile, m_starting_location, SEEK_SET);
        }
        else
        {
            m_starting_location = std::ftell(m_charfile);
        }

        m_appearances_location = m_starting_location;
        std::fread(Appearances.data(), Appearances.size(), 1, m_charfile);

        m_assigned_skilled_location = std::ftell(m_charfile);
        std::fseek(m_charfile, m_assigned_skilled_location, SEEK_SET);

        std::uint8_t tempValue = 0;
        for (size_t idx = 0; idx < NUM_OF_SKILL_HOTKEYS; ++idx)
        {
            std::fread(&tempValue, sizeof(tempValue), 1, m_charfile);
            AssignedSkills[idx] = (tempValue == 0xFF ? 0xFFFF : tempValue);
        }

        std::fread(&tempValue, sizeof(tempValue), 1, m_charfile);
        LeftSkill = tempValue;

        std::fread(&tempValue, sizeof(tempValue), 1, m_charfile);
        RightSkill = tempValue;

        m_difficulty_location = std::ftell(m_charfile);
        std::uint8_t difficultyAndAct = 0;
        std::fread(&difficultyAndAct, sizeof(difficultyAndAct), 1, m_charfile);
        Bs.DifficultyLastPlayed = static_cast<EnumDifficulty>(std::min(std::uint32_t(difficultyAndAct & 0x0F), NUM_OF_DIFFICULTY - 1));
        Bs.StartingAct = static_cast<EnumAct>(difficultyAndAct >> 4);

        StartingAct.fill(0);
        StartingAct[static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed)] = 0x80 | static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);

        m_mapid_location = 126;
        auto diff = m_mapid_location -std::ftell(m_charfile);
        diff;
        std::fseek(m_charfile, m_mapid_location, SEEK_SET);
        std::fread(&MapID, sizeof(MapID), 1, m_charfile);
    }
    else
    {
        m_starting_location = std::ftell(m_charfile);
        std::fread(&Created, sizeof(Created), 1, m_charfile);
        std::fread(&LastPlayed, sizeof(LastPlayed), 1, m_charfile);

        m_assigned_skilled_location = 56;
        std::fseek(m_charfile, m_assigned_skilled_location, SEEK_SET);
        std::fread(AssignedSkills.data(), AssignedSkills.size() * sizeof(std::uint32_t), 1, m_charfile);
        std::fread(&LeftSkill, sizeof(LeftSkill), 1, m_charfile);
        std::fread(&RightSkill, sizeof(RightSkill), 1, m_charfile);
        std::fread(&LeftSwapSkill, sizeof(LeftSkill), 1, m_charfile);
        std::fread(&RightSwapSkill, sizeof(RightSkill), 1, m_charfile);

        m_appearances_location = std::ftell(m_charfile);
        std::fread(Appearances.data(), Appearances.size(), 1, m_charfile);

        m_difficulty_location = std::ftell(m_charfile);
        std::fread(StartingAct.data(), StartingAct.size(), 1, m_charfile);
        if (StartingAct[0] != 0)
        {
            Bs.DifficultyLastPlayed = EnumDifficulty::Normal;
            Bs.StartingAct = static_cast<EnumAct>(StartingAct[0] & ~0x80);
        }
        else if (StartingAct[1] != 0)
        {
            Bs.DifficultyLastPlayed = EnumDifficulty::Nightmare;
            Bs.StartingAct = static_cast<EnumAct>(StartingAct[1] & ~0x80);
        }
        else
        {
            Bs.DifficultyLastPlayed = EnumDifficulty::Hell;
            Bs.StartingAct = static_cast<EnumAct>(StartingAct[2] & ~0x80);
        }

        m_mapid_location = std::ftell(m_charfile);
        std::fread(&MapID, sizeof(MapID), 1, m_charfile);

        Merc.readInfo(Bs.Version, m_charfile);
    }

    if (Bs.getStartingActTitle() > Bs.Title)
    {
        Bs.Title = Bs.getStartingActTitle();
    }
}
//---------------------------------------------------------------------------
bool d2ce::Character::readBasicInfo(const Json::Value& root)
{
    Json::Value header = root[m_bJsonSerializedFormat ? "Header" : "header"];
    if (header.isNull())
    {
        return false;
    }

    std::fseek(m_charfile, HEADER_LENGTH, SEEK_SET);

    Json::Value jsonValue = header[m_bJsonSerializedFormat ? "Version" : "version"];
    if (jsonValue.isNull())
    {
        return false;
    }

    Version = std::uint32_t(jsonValue.asInt64());
    if (std::uint32_t(getVersion()) != Version)
    {
        return false;
    }
    std::fwrite(&Version, sizeof(Version), 1, m_charfile);

    Bs.Version = getVersion();
    Cs.Version = Bs.Version;

    m_filesize_location = 0;
    m_checksum_location = 0;
    if (Bs.Version >= EnumCharVersion::v109)
    {
        m_filesize_location = std::ftell(m_charfile);
        jsonValue = header[m_bJsonSerializedFormat ? "Filesize" : "filesize"];
        if (!jsonValue.isNull())
        {
            FileSize = std::uint32_t(jsonValue.asInt64());
        }
        std::fwrite(&FileSize, sizeof(FileSize), 1, m_charfile);

        m_checksum_location = std::ftell(m_charfile);
        jsonValue = header[m_bJsonSerializedFormat ? "Checksum": "checksum"];
        if (!jsonValue.isNull())
        {
            Checksum = m_bJsonSerializedFormat ? long(jsonValue.asInt64()) : long(std::stoul(jsonValue.asString(), nullptr, 16));
        }
        std::fwrite(&Checksum, sizeof(Checksum), 1, m_charfile);

        jsonValue = m_bJsonSerializedFormat ? root["ActiveWeapon"] : header["active_arms"];
        if (!jsonValue.isNull())
        {
            WeaponSet = std::uint32_t(jsonValue.asInt64());
        }
        std::fwrite(&WeaponSet, sizeof(WeaponSet), 1, m_charfile);
    }

    m_name_location = std::ftell(m_charfile);
    jsonValue = m_bJsonSerializedFormat ? root["Name"] : header["name"];
    if (jsonValue.isNull())
    {
        return false;
    }

    {
        // Check Name
        // Remove any invalid characters from the number
        std::string curName(jsonValue.asString());
        std::string strNewText;
        for (size_t iPos = 0, numberOfUnderscores = 0, nLen = curName.size(); iPos < nLen; ++iPos)
        {
            char c = curName[iPos];
            if (std::isalpha(c))
            {
                strNewText += c;
            }
            else if ((c == '_' || c == '-') && strNewText.size() != 0 && numberOfUnderscores < 1)
            {
                strNewText += c;
                ++numberOfUnderscores;
            }
        }

        // trim bad characters
        if (strNewText.size() > 15)
        {
            strNewText.resize(15);
        }
        strNewText.erase(strNewText.find_last_not_of("_-") + 1);
        if (strNewText.size() < 2)
        {
            return false;
        }

        Bs.Name.fill(0);
        strcpy_s(Bs.Name.data(), strNewText.length() + 1, strNewText.c_str());
        Bs.Name[15] = 0; // must be zero
        std::fwrite(Bs.Name.data(), Bs.Name.size(), 1, m_charfile);
    }

    std::uint8_t value = 0;
    jsonValue = m_bJsonSerializedFormat ? root["ClassId"] : header["class"];
    if (!jsonValue.isNull())
    {
        if (m_bJsonSerializedFormat)
        {
            value = std::uint8_t(jsonValue.asInt());
            if (value > std::uint8_t(NUM_OF_CLASSES))
            {
                return false;
            }
        }
        else
        {
            bool bFound = false;
            std::string className = jsonValue.asString();
            for (std::uint8_t idx = 0; idx < std::uint8_t(NUM_OF_CLASSES); ++idx)
            {
                if (ClassNames[idx].compare(className) == 0)
                {
                    bFound = true;
                    value = idx;
                    break;
                }
            }

            if (!bFound)
            {
                return false;
            }
        }
    }
    Bs.Class = static_cast<EnumCharClass>(value);
    Bs.Status = EnumCharStatus::NoDeaths;
    switch (Bs.Class)
    {
    case EnumCharClass::Druid:
    case EnumCharClass::Assassin:
        Bs.Status |= EnumCharStatus::Expansion;
        return false;
    }

    jsonValue = m_bJsonSerializedFormat ? root["Location"] : header["difficulty"];
    ApplyJsonStartingAct(jsonValue, StartingAct);
    if (StartingAct[0] != 0)
    {
        Bs.DifficultyLastPlayed = EnumDifficulty::Normal;
        Bs.StartingAct = static_cast<EnumAct>(StartingAct[0] & ~0x80);
        if (Bs.StartingAct > EnumAct::IV)
        {
            Bs.Status |= EnumCharStatus::Expansion;
        }
    }
    else if (StartingAct[1] != 0)
    {
        Bs.DifficultyLastPlayed = EnumDifficulty::Nightmare;
        Bs.StartingAct = static_cast<EnumAct>(StartingAct[1] & ~0x80);
        if (Bs.StartingAct > EnumAct::IV)
        {
            Bs.Status |= EnumCharStatus::Expansion;
        }
    }
    else
    {
        Bs.DifficultyLastPlayed = EnumDifficulty::Hell;
        Bs.StartingAct = static_cast<EnumAct>(StartingAct[2] & ~0x80);
        if (Bs.StartingAct > EnumAct::IV)
        {
            Bs.Status |= EnumCharStatus::Expansion;
        }
    }

    Json::Value status = m_bJsonSerializedFormat ? root["Status"] : header["status"];
    if (!status.isNull())
    {
        jsonValue = status[m_bJsonSerializedFormat ? "IsHardcore" : "hardcore"];
        if (jsonValue.asBool())
        {
            Bs.Status |= EnumCharStatus::Hardcore;
        }

        jsonValue = status[m_bJsonSerializedFormat ? "IsDead" : "died"];
        if (jsonValue.asBool())
        {
            Bs.Status |= EnumCharStatus::Died;
        }

        jsonValue = status[m_bJsonSerializedFormat ? "IsExpansion" : "expansion"];
        if (jsonValue.asBool())
        {
            Bs.Status |= EnumCharStatus::Expansion;
        }

        jsonValue = status[m_bJsonSerializedFormat ? "IsLadder" : "ladder"];
        if (jsonValue.asBool())
        {
            Bs.Status |= EnumCharStatus::Ladder;
        }

        // ladder is for 1.10 or higher
        if (Bs.Version < EnumCharVersion::v110)
        {
            Bs.Status &= ~EnumCharStatus::Ladder;

            if (Bs.Version < EnumCharVersion::v107 || Bs.Version == EnumCharVersion::v108)
            {
                // expansion not supported
                switch (Bs.Class)
                {
                case EnumCharClass::Druid:
                case EnumCharClass::Assassin:
                    return false;
                }

                if (Bs.StartingAct > EnumAct::IV)
                {
                    return false;
                }

                Bs.Status &= ~EnumCharStatus::Expansion;
            }
        }
    }

    value = Bs.Status.bits();
    std::fwrite(&value, sizeof(value), 1, m_charfile);

    // progression can be derived later when looking at the which Difficulty level is complete
    Bs.Title = 0;
    jsonValue = m_bJsonSerializedFormat ? root["Progression"] : header["progression"];
    if (!jsonValue.isNull())
    {
        Bs.Title = std::min(std::uint8_t(jsonValue.asInt()), Bs.getGameCompleteTitle());
    }
    std::fwrite(&Bs.Title, sizeof(Bs.Title), 1, m_charfile);

    if (Bs.Version < EnumCharVersion::v109)
    {
        jsonValue = m_bJsonSerializedFormat ? root["ActiveWeapon"] : header["active_arms"];
        if (!jsonValue.isNull())
        {
            WeaponSet = std::uint32_t(jsonValue.asInt64());
        }

        value = std::uint8_t(WeaponSet);
        std::fwrite(&value, sizeof(value), 1, m_charfile);

        value = 0;
        std::fwrite(&value, sizeof(value), 1, m_charfile);

        if (Bs.Version < EnumCharVersion::v107)
        {
            std::fwrite(UNKNOWN_01C_v100.data(), UNKNOWN_01C_v100.size(), 1, m_charfile);
        }
        else
        {
            std::fwrite(UNKNOWN_01C_v107.data(), UNKNOWN_01C_v107.size(), 1, m_charfile);
        }
    }
    else
    {
        std::fwrite(UNKNOWN_026.data(), UNKNOWN_026.size(), 1, m_charfile);
    }

    m_class_location = std::ftell(m_charfile);
    value = static_cast<std::underlying_type_t<EnumCharClass>>(Bs.Class);
    std::fwrite(&value, sizeof(value), 1, m_charfile);
    if (Bs.Version < EnumCharVersion::v109)
    {
        value = 0;
        std::fwrite(&value, sizeof(value), 1, m_charfile);
    }
    else
    {
        std::fwrite(UNKNOWN_029.data(), UNKNOWN_029.size(), 1, m_charfile);
    }

    // Level can be retrieved from the attributes section
    m_level_location = std::ftell(m_charfile);
    jsonValue = m_bJsonSerializedFormat ? root["Level"] : header["level"];
    if (!jsonValue.isNull())
    {
        DisplayLevel = std::uint8_t(jsonValue.asInt());
    }
    std::fwrite(&DisplayLevel, sizeof(DisplayLevel), 1, m_charfile);
    if (Bs.Version < EnumCharVersion::v107)
    {
        value = 0;
        std::fwrite(&value, sizeof(value), 1, m_charfile);
    }

    jsonValue = m_bJsonSerializedFormat ? root["Appearances"] : header["menu_appearance"];
    ApplyJsonAppearnces(jsonValue, Appearances);

    jsonValue = m_bJsonSerializedFormat ? root["AssignedSkills"] : header["assigned_skills"];
    ApplyJsonAssignedSkills(jsonValue, AssignedSkills);

    jsonValue = m_bJsonSerializedFormat ? root["LeftSkill"] : header["left_skill"];
    LeftSkill = ApplyJsonSkill(jsonValue);

    jsonValue = m_bJsonSerializedFormat ? root["RightSkill"] : header["right_skill"];
    RightSkill = ApplyJsonSkill(jsonValue);

    jsonValue = m_bJsonSerializedFormat ? root["RightSkill"] : header["right_skill"];
    RightSkill = ApplyJsonSkill(jsonValue);

    jsonValue = m_bJsonSerializedFormat ? root["MapId"] : header["map_id"];
    if (!jsonValue.isNull())
    {
        MapID = std::uint32_t(jsonValue.asInt64());
    }

    if (Bs.Version < EnumCharVersion::v109)
    {
        m_starting_location = std::ftell(m_charfile);
        m_appearances_location = m_starting_location;
        std::fwrite(&Appearances, sizeof(Appearances), 1, m_charfile);

        m_assigned_skilled_location = std::ftell(m_charfile);
        for (size_t idx = 0; idx < NUM_OF_SKILL_HOTKEYS; ++idx)
        {
            value = (AssignedSkills[idx] >= 0xFFFF ? 0xFF : (std::uint8_t)AssignedSkills[idx]);
            std::fwrite(&value, sizeof(value), 1, m_charfile);
        }

        value = (std::uint8_t)LeftSkill;
        std::fwrite(&value, sizeof(value), 1, m_charfile);

        value = (std::uint8_t)RightSkill;
        std::fwrite(&value, sizeof(value), 1, m_charfile);

        m_difficulty_location = std::ftell(m_charfile);
        std::uint16_t difficultyAndAct = static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
        value <<= 4;
        value |= (static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed) & 0x0F);
        std::fwrite(&value, sizeof(value), 1, m_charfile);
        std::fwrite(&difficultyAndAct, sizeof(difficultyAndAct), 1, m_charfile);

        std::fwrite(UNKNOWN_05A_v100.data(), UNKNOWN_05A_v100.size(), 1, m_charfile);

        m_mapid_location = std::ftell(m_charfile);
        std::fwrite(&MapID, sizeof(MapID), 1, m_charfile);
    }
    else
    {
        m_starting_location = std::ftell(m_charfile);
        jsonValue = m_bJsonSerializedFormat ? root["Created"] : header["created"];
        if (!jsonValue.isNull())
        {
            Created = std::uint32_t(jsonValue.asInt64());
        }
        std::fwrite(&Created, sizeof(Created), 1, m_charfile);

        jsonValue = m_bJsonSerializedFormat ? root["LastPlayed"] : header["last_played"];
        if (!jsonValue.isNull())
        {
            LastPlayed = std::uint32_t(jsonValue.asInt64());
        }
        std::fwrite(&LastPlayed, sizeof(LastPlayed), 1, m_charfile);

        std::fwrite(UNKNOWN_034.data(), UNKNOWN_034.size(), 1, m_charfile);

        m_assigned_skilled_location = std::ftell(m_charfile);
        std::fwrite(AssignedSkills.data(), AssignedSkills.size()*sizeof(std::uint32_t), 1, m_charfile);
        std::fwrite(&LeftSkill, sizeof(LeftSkill), 1, m_charfile);
        std::fwrite(&RightSkill, sizeof(RightSkill), 1, m_charfile);

        jsonValue = m_bJsonSerializedFormat ? root["LeftSwapSkill"] : header["left_swap_skill"];
        LeftSwapSkill = ApplyJsonSkill(jsonValue);
        std::fwrite(&LeftSwapSkill, sizeof(LeftSwapSkill), 1, m_charfile);

        jsonValue = m_bJsonSerializedFormat ? root["RightSwapSkill"] : header["right_swap_skill"];
        RightSwapSkill = ApplyJsonSkill(jsonValue);
        std::fwrite(&RightSwapSkill, sizeof(RightSwapSkill), 1, m_charfile);

        m_appearances_location = std::ftell(m_charfile);
        std::fwrite(Appearances.data(), Appearances.size(), 1, m_charfile);

        m_difficulty_location = std::ftell(m_charfile);
        std::fwrite(StartingAct.data(), StartingAct.size(), 1, m_charfile);

        m_mapid_location = std::ftell(m_charfile);
        std::fwrite(&MapID, sizeof(MapID), 1, m_charfile);

        std::fwrite(UNKNOWN_0AF.data(), UNKNOWN_0AF.size(), 1, m_charfile);

        if (!Merc.readInfo(root, m_bJsonSerializedFormat, Bs.Version, m_charfile))
        {
            return false;
        }

        // Realm data
        std::fwrite(UNKNOWN_0BF.data(), UNKNOWN_0BF.size(), 1, m_charfile);
        if (Bs.Version < EnumCharVersion::v115)
        {
            std::fwrite(UNKOWN_14B_v109.data(), UNKOWN_14B_v109.size(), 1, m_charfile);
        }
        else
        {
            std::fwrite(UNKOWN_14B_v115.data(), UNKOWN_14B_v115.size(), 1, m_charfile);
        }
    }

    if (Bs.getStartingActTitle() > Bs.Title)
    {
        Bs.Title = Bs.getStartingActTitle();
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readActs()
{
    if (!Acts.readActs(m_charfile))
    {
        return false;
    }

    validateActs();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readActs(const Json::Value& root)
{
    if (!Acts.readActs(root, m_bJsonSerializedFormat, m_charfile))
    {
        return false;
    }

    validateActs();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readStats()
{
    if (Cs.readStats(Bs.Version, Bs.Class, m_charfile))
    {
        m_stats_header_location = Cs.getHeaderLocation();
        DisplayLevel = (std::uint8_t)Cs.Cs.Level; // updates character's display level
        Cs.updateLifePointsEarned(Acts.getLifePointsEarned());
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readStats(const Json::Value& root)
{
    if (Cs.readStats(root, m_bJsonSerializedFormat, Bs.Version, Bs.Class, m_charfile))
    {
        m_stats_header_location = Cs.getHeaderLocation();
        DisplayLevel = (std::uint8_t)Cs.Cs.Level; // updates character's display level
        Cs.updateLifePointsEarned(Acts.getLifePointsEarned());
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readItems()
{
    return m_items.readItems(Bs.Version, m_charfile, isExpansionCharacter());
}
//---------------------------------------------------------------------------
bool d2ce::Character::readItems(const Json::Value& root)
{
    return m_items.readItems(root, m_bJsonSerializedFormat, Bs.Version, m_charfile, isExpansionCharacter());
}
//---------------------------------------------------------------------------
bool d2ce::Character::save()
{
    if (m_charfile == nullptr)
    {
        return false;
    }

    m_error_code.clear();
    writeBasicInfo();
    writeActs();

    // From this point on, the location is variable
    writeStats();

    // Write Character, Corpse, Mercenary and Golem items
    writeItems();
    if (FileSize > (std::uint32_t)std::ftell(m_charfile))
    {
        // truncation occured
        writeTempFile();

        // prepare to update the character file
        std::fclose(m_charfile);
        m_charfile = nullptr;
        std::remove(m_d2sfilename.c_str());

        // Don't modify the name of a temporary file
        if (m_jsonfilename.empty())
        {
            // check to see if the m_d2sfilename needs to be changed
            // to match the character's name
            std::filesystem::path p = m_d2sfilename;
            p.replace_extension();
            std::string tempname = p.filename().string();

            // compare m_d2sfilename (w/o extension) to character's name
            if (tempname.compare(0, tempname.length(), Bs.Name.data()) != 0)
            {
                m_d2sfilename = p.replace_filename(Bs.Name.data()).string();
            }
        }

        // rename temp file to character file
        if (std::rename(m_tempfilename.c_str(), m_d2sfilename.c_str()))
        {
            m_error_code = std::make_error_code(CharacterErrc::FileRenameError);
            return false;
        }

        if (!open(m_d2sfilename.c_str(), false)) // checksum is calulated and written below
        {
            return false;
        }
    }
    else if (m_jsonfilename.empty()) // Don't modify the name of a temporary file
    {
        // check to see if the m_d2sfilename needs to be changed
        // to match the character's name
        m_tempfilename = m_d2sfilename;
        std::filesystem::path p = m_d2sfilename;
        p.replace_extension();
        std::string origFileNameBase = p.string();
        std::string tempname = p.filename().string();

        // compare m_d2sfilename (w/o extension) to character's name
        if (_stricmp(tempname.c_str(), Bs.Name.data()) != 0)
        {
            std::string fileNameBase = p.replace_filename(Bs.Name.data()).string();
            m_d2sfilename = fileNameBase + ".d2s";
            std::fclose(m_charfile);
            m_charfile = nullptr;
            if (std::rename(m_tempfilename.c_str(), m_d2sfilename.c_str()))
            {
                m_error_code = std::make_error_code(CharacterErrc::FileRenameError);
                return false;
            }

            if (!open(m_d2sfilename.c_str(), false)) // checksum is calulated and written below
            {
                return false;
            }

            // rename other files (don't error out if it fails)
            m_tempfilename = origFileNameBase + ".key";
            if (std::filesystem::exists(m_tempfilename))
            {
                tempname = fileNameBase + ".key";
                if (std::rename(m_tempfilename.c_str(), tempname.c_str()))
                {
                    m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                }
            }

            m_tempfilename = origFileNameBase + ".ma0";
            if (std::filesystem::exists(m_tempfilename))
            {
                tempname = fileNameBase + ".ma0";
                if (std::rename(m_tempfilename.c_str(), tempname.c_str()))
                {
                    m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                }
            }

            m_tempfilename = origFileNameBase + ".ma1";
            if (std::filesystem::exists(m_tempfilename))
            {
                tempname = fileNameBase + ".ma1";
                if (std::rename(m_tempfilename.c_str(), tempname.c_str()))
                {
                    m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                }
            }

            m_tempfilename = origFileNameBase + ".ma2";
            if (std::filesystem::exists(m_tempfilename))
            {
                tempname = fileNameBase + ".ma2";
                if (std::rename(m_tempfilename.c_str(), tempname.c_str()))
                {
                    m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                }
            }

            m_tempfilename = origFileNameBase + ".ma3";
            if (std::filesystem::exists(m_tempfilename))
            {
                tempname = fileNameBase + ".ma3";
                if (std::rename(m_tempfilename.c_str(), tempname.c_str()))
                {
                    m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                }
            }

            m_tempfilename = origFileNameBase + ".map";
            if (std::filesystem::exists(m_tempfilename))
            {
                tempname = fileNameBase + ".map";
                if (std::rename(m_tempfilename.c_str(), tempname.c_str()))
                {
                    m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                }
            }
        }
    }

    // store the file's size
    std::fflush(m_charfile);
    std::fseek(m_charfile, 0, SEEK_END);
    FileSize = std::ftell(m_charfile);

    // determine if a checksum needs to be calculated and stored
    if (Bs.Version >= EnumCharVersion::v109)
    {
        std::fseek(m_charfile, m_filesize_location, SEEK_SET);
        std::fwrite(&FileSize, sizeof(FileSize), 1, m_charfile);

        // make sure the checksum is zero in the file
        Checksum = 0;
        m_checksum_location = std::ftell(m_charfile);
        std::fwrite(&Checksum, sizeof(Checksum), 1, m_charfile);
        std::fwrite(&WeaponSet, sizeof(WeaponSet), 1, m_charfile);

        std::fflush(m_charfile);
        calculateChecksum();

        // write the checksum into the file
        std::fseek(m_charfile, m_checksum_location, SEEK_SET);
        std::fwrite(&Checksum, sizeof(Checksum), 1, m_charfile);
    }

    std::fflush(m_charfile);

    if (!m_jsonfilename.empty())
    {
        // export the json again
        auto json = asJson(m_bJsonSerializedFormat);
        if (!json.empty())
        {
            std::FILE* jsonFile = NULL;
            fopen_s(&jsonFile, m_jsonfilename.c_str(), "wb");
            std::rewind(jsonFile);
            std::fwrite(json.c_str(), json.size(), 1, jsonFile);
            std::fclose(jsonFile);
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::saveAsD2s()
{
    // first save any outstanding changes
    if (!save())
    {
        return false;
    }

    if (m_jsonfilename.empty())
    {
        return true;
    }

    std::fclose(m_charfile);
    m_charfile = NULL;

    auto oldFileName = m_d2sfilename;

    // move d2s file to json file path
    std::filesystem::path p = m_jsonfilename;
    m_jsonfilename.clear();

    p.replace_extension();
    m_d2sfilename = p.replace_filename(Bs.Name.data()).string() + ".d2s";
    p = m_d2sfilename;
    if (std::filesystem::exists(p))
    {
        // remove exist file, back would be down prior to call this
        std::remove(m_d2sfilename.c_str());
    }

    // rename temp file to character file
    if (std::rename(oldFileName.c_str(), m_d2sfilename.c_str()))
    {
        // this is a temporary d2s file created from a json input file
        std::remove(oldFileName.c_str());

        m_d2sfilename.clear();
        initialize();
        m_error_code = std::make_error_code(CharacterErrc::FileRenameError);
        return false;
    }

    initialize();
    if (!open(m_d2sfilename.c_str(), false)) // checksum is calulated and written below
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Character::writeBasicInfo()
{
    std::fseek(m_charfile, m_name_location, SEEK_SET);
    std::fwrite(Bs.Name.data(), Bs.Name.size(), 1, m_charfile);

    // ladder is for 1.10 or higher
    if (Bs.Version < EnumCharVersion::v110)
    {
        Bs.Status &= ~EnumCharStatus::Ladder;
    }

    if ((Bs.Status & EnumCharStatus::Hardcore) != 0)
    {
        Bs.Status &= ~EnumCharStatus::Died; // can't be resurrected
    }

    std::uint8_t value = Bs.Status.bits();
    std::fwrite(&value, sizeof(value), 1, m_charfile);

    std::fwrite(&Bs.Title, sizeof(Bs.Title), 1, m_charfile);

    if (Bs.Version < EnumCharVersion::v109)
    {
        value = (std::uint8_t)WeaponSet;
        std::fwrite(&value, sizeof(value), 1, m_charfile);
    }

    std::fseek(m_charfile, m_class_location, SEEK_SET);
    value = static_cast<std::underlying_type_t<EnumCharClass>>(Bs.Class);
    std::fwrite(&value, sizeof(value), 1, m_charfile);

    std::fseek(m_charfile, m_level_location, SEEK_SET);
    std::fwrite(&DisplayLevel, sizeof(DisplayLevel), 1, m_charfile);

    std::fseek(m_charfile, m_starting_location, SEEK_SET);
    if (Bs.Version < EnumCharVersion::v109)
    {
        std::fwrite(Appearances.data(), Appearances.size(), 1, m_charfile);
        std::uint8_t tempValue = 0;
        for (size_t idx = 0; idx < NUM_OF_SKILL_HOTKEYS; ++idx)
        {
            tempValue = (AssignedSkills[idx] >= 0xFFFF ? 0xFF : (std::uint8_t)AssignedSkills[idx]);
            std::fwrite(&tempValue, sizeof(tempValue), 1, m_charfile);
        }

        tempValue = (std::uint8_t)LeftSkill;
        std::fwrite(&tempValue, sizeof(tempValue), 1, m_charfile);

        tempValue = (std::uint8_t)RightSkill;
        std::fwrite(&tempValue, sizeof(tempValue), 1, m_charfile);

        std::uint8_t difficultyAndAct = static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
        difficultyAndAct <<= 4;
        difficultyAndAct |= (static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed) & 0x0F);
        std::fwrite(&difficultyAndAct, sizeof(difficultyAndAct), 1, m_charfile);

        std::fseek(m_charfile, m_mapid_location, SEEK_SET);
        std::fwrite(&MapID, sizeof(MapID), 1, m_charfile);
    }
    else
    {
        std::fwrite(&Created, sizeof(Created), 1, m_charfile);
        std::fwrite(&LastPlayed, sizeof(LastPlayed), 1, m_charfile);

        std::fseek(m_charfile, m_assigned_skilled_location, SEEK_SET);
        std::fwrite(AssignedSkills.data(), AssignedSkills.size()*sizeof(std::uint32_t), 1, m_charfile);
        std::fwrite(&LeftSkill, sizeof(LeftSkill), 1, m_charfile);
        std::fwrite(&RightSkill, sizeof(RightSkill), 1, m_charfile);
        std::fwrite(&LeftSwapSkill, sizeof(LeftSkill), 1, m_charfile);
        std::fwrite(&RightSwapSkill, sizeof(RightSkill), 1, m_charfile);
        std::fwrite(Appearances.data(), Appearances.size(), 1, m_charfile);
        std::fwrite(StartingAct.data(), StartingAct.size(), 1, m_charfile);
        std::fwrite(&MapID, sizeof(MapID), 1, m_charfile);

        Merc.writeInfo(m_charfile);
    }
    std::fflush(m_charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeActs()
{
    return Acts.writeActs(m_charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeStats()
{
    return Cs.writeStats(m_charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeItems()
{
    return m_items.writeItems(m_charfile, isExpansionCharacter());
}
//---------------------------------------------------------------------------
/*
   This function makes sure that any changes to the character's
   experience and gold values are correctly stored in the file.
*/
void d2ce::Character::writeTempFile()
{
    m_tempfilename.clear();
    char name1[L_tmpnam_s];
    errno_t err = tmpnam_s(name1, L_tmpnam_s);
    if (err == 0)
    {
        m_tempfilename = name1;
    }

    std::FILE* tempfile = NULL;
    fopen_s(&tempfile, m_tempfilename.c_str(), "wb");

    std::rewind(m_charfile);

    // Write the beginning using one buffer
    std::vector<std::uint8_t> buffer((size_t)m_stats_header_location + 1, 0);
    std::fread(&buffer[0], m_stats_header_location, 1, m_charfile);
    std::fwrite(&buffer[0], m_stats_header_location, 1, tempfile);

    // From this point on, the location is variable
    Cs.writeStats(tempfile);

    // Write Character, Corpse, Mercenary and Golem m_items
    m_items.writeItems(tempfile, isExpansionCharacter());
    std::fclose(tempfile);
}
//---------------------------------------------------------------------------
void d2ce::Character::headerAsJson(Json::Value& parent, bool bSerializedFormat) const
{
    if (bSerializedFormat)
    {
        Json::Value header;
        header["Magic"] = *((std::uint32_t*)Header.data());
        header["Version"] = Version;
        if (Bs.Version >= EnumCharVersion::v109)
        {
            header["Filesize"] = FileSize;
            header["Checksum"] = Checksum;
        }
        parent["Header"] = header;

        parent["ActiveWeapon"] = WeaponSet;
        parent["Name"] = Bs.Name.data();

        // status
        Json::Value status;
        status["IsHardcore"] = isHardcoreCharacter();
        status["IsDead"] = isResurrectedCharacter();
        status["IsExpansion"] = isExpansionCharacter();
        if (isLadderCharacter())
        {
            status["IsLadder"] = true;
        }
        parent["Status"] = status;

        parent["Progression"] = std::uint16_t(getTitle());
        parent["ClassId"] = std::uint16_t(getClass());
        parent["Level"] = std::uint16_t(DisplayLevel);
        if (Bs.Version >= EnumCharVersion::v109)
        {
            parent["Created"] = Created;
            parent["LastPlayed"] = LastPlayed;
        }

        // assigned_skills
        Json::Value assignedSkills(Json::arrayValue);
        for (auto& skillId : AssignedSkills)
        {
            Json::Value skill;
            skill["Id"] = skillId;
            assignedSkills.append(skill);
        }
        parent["AssignedSkills"] = assignedSkills;

        {
            Json::Value skill;
            skill["Id"] = LeftSkill;
            parent["LeftSkill"] = skill;
        }

        {
            Json::Value skill;
            skill["Id"] = RightSkill;
            parent["RightSkill"] = skill;
        }
        if (Bs.Version >= EnumCharVersion::v109)
        {
            {
                Json::Value skill;
                skill["Id"] = LeftSwapSkill;
                parent["LeftSwapSkill"] = skill;
            }
            {
                Json::Value skill;
                skill["Id"] = RightSwapSkill;
                parent["RightSwapSkill"] = skill;
            }
        }

        // Appearances
        static std::initializer_list<std::string> all_appearance_props = { "Head", "Torso", "Legs", "RightArm", "LeftArm", "RightHand", "LeftHand", "Shield",
            "Special1", "Special2",  "Special3", "Special4", "Special5", "Special6", "Special7", "Special8" };

        size_t idx = 0;
        Json::Value appearances;
        for (const auto& prop : all_appearance_props)
        {
            Json::Value appearance;
            appearance["Graphic"] = std::uint16_t(Appearances[idx++]);
            appearance["Tint"] = std::uint16_t(Appearances[idx + 15]);
            appearances[prop] = appearance;
        }
        parent["Appearances"] = appearances;

        // Location
        auto diffLastPlayed = getDifficultyLastPlayed();
        auto startingAct = (std::uint16_t)getStartingAct() + 1;
        Json::Value location;
        {
            Json::Value locationDiff;
            locationDiff["Active"] = ((diffLastPlayed == EnumDifficulty::Normal) ? true : false);
            locationDiff["Act"] = ((diffLastPlayed == EnumDifficulty::Normal) ? startingAct : 1);
            location["Normal"] = locationDiff;
        }
        {
            Json::Value locationDiff;
            locationDiff["Active"] = ((diffLastPlayed == EnumDifficulty::Nightmare) ? true : false);
            locationDiff["Act"] = ((diffLastPlayed == EnumDifficulty::Nightmare) ? startingAct : 1);
            location["Nightmare"] = locationDiff;
        }
        {
            Json::Value locationDiff;
            locationDiff["Active"] = ((diffLastPlayed == EnumDifficulty::Hell) ? true : false);
            locationDiff["Act"] = ((diffLastPlayed == EnumDifficulty::Hell) ? startingAct : 1);
            location["Hell"] = locationDiff;
        }
        parent["Location"] = location;
        parent["MapId"] = MapID;

        if (Bs.Version >= EnumCharVersion::v109)
        {
            Merc.asJson(parent, bSerializedFormat);
        }

        Acts.questsAsJson(parent, bSerializedFormat);
        Acts.waypointsAsJson(parent, bSerializedFormat);
        Acts.npcAsJson(parent, bSerializedFormat);
    }
    else
    {
        Json::Value header;
        {
            std::stringstream ss;
            ss << std::hex << *((std::uint32_t*)Header.data());
            header["identifier"] = ss.str();
        }
        header["version"] = Version;
        if (Bs.Version >= EnumCharVersion::v109)
        {
            header["filesize"] = FileSize;
            {
                std::stringstream ss;
                ss << std::hex << Checksum;
                header["checksum"] = ss.str();
            }
        }
        header["name"] = Bs.Name.data();

        // status
        Json::Value status;
        status["hardcore"] = isHardcoreCharacter();
        status["died"] = isResurrectedCharacter();
        status["expansion"] = isExpansionCharacter();
        if (isLadderCharacter())
        {
            status["ladder"] = true;
        }

        if (isDeadCharacter())
        {
            status["dead"] = true;
        }
        header["status"] = status;

        header["progression"] = std::uint16_t(getTitle());
        header["active_arms"] = WeaponSet;
        header["class"] = getClassName();
        header["level"] = std::uint16_t(DisplayLevel);
        if (Bs.Version >= EnumCharVersion::v109)
        {
            header["created"] = Created;
            header["last_played"] = LastPlayed;
        }

        // assigned_skills
        auto nullCount = 0; // no need to include nulls at the end of list
        Json::Value nullValue;
        Json::Value assignedSkills(Json::arrayValue);
        for (auto& skillId : AssignedSkills)
        {
            if (skillId == 0xFFFF)
            {
                ++nullCount;
                continue;
            }

            while (nullCount > 0)
            {
                assignedSkills.append(nullValue);
                --nullCount;
            }

            assignedSkills.append(Cs.getSkillNameById(skillId));
        }
        header["assigned_skills"] = assignedSkills;
        header["left_skill"] = Cs.getSkillNameById(LeftSkill);
        header["right_skill"] = Cs.getSkillNameById(RightSkill);
        if (Bs.Version >= EnumCharVersion::v109)
        {
            header["left_swap_skill"] = Cs.getSkillNameById(LeftSwapSkill);
            header["right_swap_skill"] = Cs.getSkillNameById(RightSwapSkill);
        }

        // Appearances
        static std::initializer_list<std::string> all_appearance_props = { "head", "torso", "legs", "right_arm", "left_arm", "right_hand", "left_hand", "shield",
            "special1", "special2",  "special3", "special4", "special5", "special6", "special7", "special8" };

        size_t idx = 0;
        Json::Value appearances;
        for (const auto& prop : all_appearance_props)
        {
            Json::Value appearance;
            appearance["graphic"] = std::uint16_t(Appearances[idx++]);
            appearance["tint"] = std::uint16_t(Appearances[idx + 15]);
            appearances[prop] = appearance;
        }
        header["menu_appearance"] = appearances;

        // StartingAct
        {
            Json::Value difficulty;
            difficulty["Normal"] = std::uint16_t(StartingAct[0]);
            difficulty["Nightmare"] = std::uint16_t(StartingAct[1]);
            difficulty["Hell"] = std::uint16_t(StartingAct[2]);
            header["difficulty"] = difficulty;
        }
        header["map_id"] = MapID;

        if (Bs.Version >= EnumCharVersion::v109)
        {
            Merc.asJson(header, bSerializedFormat);
        }

        Acts.questsAsJson(header, bSerializedFormat);
        Acts.waypointsAsJson(header, bSerializedFormat);
        Acts.npcAsJson(header, bSerializedFormat);

        parent["header"] = header;
    }
}
//---------------------------------------------------------------------------
void d2ce::Character::validateActs()
{
    // Check Title to make sure it makes sense what difficulty is allowed to be played
    if (Bs.getStartingActTitle() > Bs.Title)
    {
        Bs.Title = Bs.getStartingActTitle();
    }

    auto progression = getTitleDifficulty();
    if (progression < EnumDifficulty::Hell)
    {
        if (!Acts.getActYetToStart(EnumDifficulty::Hell, EnumAct::I))
        {
            if (progression < EnumDifficulty::Hell)
            {
                Bs.Title = std::uint8_t(Bs.getNumActs() * static_cast<std::underlying_type_t<EnumDifficulty>>(EnumDifficulty::Hell) + static_cast<std::underlying_type_t<EnumCharVersion>>(Bs.StartingAct));
                progression = EnumDifficulty::Hell;
            }
        }
        else if (!Acts.getActYetToStart(EnumDifficulty::Nightmare, EnumAct::I))
        {
            if (progression < EnumDifficulty::Nightmare)
            {
                Bs.Title = std::uint8_t(Bs.getNumActs() + static_cast<std::underlying_type_t<EnumCharVersion>>(Bs.StartingAct));
                progression = EnumDifficulty::Nightmare;
            }
        }
    }

    if (Bs.DifficultyLastPlayed > progression)
    {
        // Not able to allow this state
        progression = Bs.DifficultyLastPlayed;
        Bs.Title = Bs.getStartingActTitle();
    }

    Acts.validateActs();
}
//---------------------------------------------------------------------------
void d2ce::Character::close()
{
    if (m_charfile != nullptr)
    {
        std::fclose(m_charfile);
    }
    m_charfile = nullptr;

    if (!m_jsonfilename.empty())
    {
        // this is a temporary d2s file created from a json input file
        std::remove(m_d2sfilename.c_str());
    }

    m_d2sfilename.clear();
    m_jsonfilename.clear();

    initialize();
}
//---------------------------------------------------------------------------
const char* d2ce::Character::getPathName() const
{
    return m_jsonfilename.empty() ? m_d2sfilename.c_str() : m_jsonfilename.c_str();
}
//---------------------------------------------------------------------------
std::string d2ce::Character::asJson(bool bSerializedFormat) const
{
    Json::Value root;
    headerAsJson(root, bSerializedFormat);
    Cs.asJson(root, bSerializedFormat);
    m_items.asJson(root, getLevel(), bSerializedFormat);

    Json::StreamWriterBuilder builder;
    builder["indentation"] = jsonIndentStr;
    builder["enableYAMLCompatibility"] = true;
    return Json::writeString(builder, root);
}
//---------------------------------------------------------------------------
bool d2ce::Character::is_open() const
{
    return m_charfile == nullptr ? false : true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::is_json() const
{
    if (!is_open())
    {
        return false;
    }

    return m_jsonfilename.empty() ? false : true;
}
//---------------------------------------------------------------------------
std::error_code d2ce::Character::getLastError() const
{
    return m_error_code;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getFileSize() const
{
    return FileSize;
}
//---------------------------------------------------------------------------
d2ce::Mercenary& d2ce::Character::getMercenaryInfo()
{
    return Merc;
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& d2ce::Character::getMercItems() const
{
    return m_items.getMercItems();
}
//---------------------------------------------------------------------------
bool d2ce::Character::getMercItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    return m_items.getMercItemBonuses(attribs);
}
//---------------------------------------------------------------------------
bool d2ce::Character::getDisplayedMercItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    return m_items.getDisplayedMercItemBonuses(attribs, Merc.getLevel());
}
//---------------------------------------------------------------------------
std::uint16_t d2ce::Character::getCombinedMercDefenseRating() const
{
    return m_items.getCombinedMercDefenseRating(Merc.getLevel());
}
//---------------------------------------------------------------------------
bool d2ce::Character::getCombinedMercDamage(BaseDamage& damage) const
{
    return m_items.getCombinedMercDamage(damage, Merc.getLevel());
}
//---------------------------------------------------------------------------
const std::list<d2ce::Item>& d2ce::Character::getCorpseItems() const
{
    return m_items.getCorpseItems();
}
//---------------------------------------------------------------------------
bool d2ce::Character::hasGolem() const
{
    return m_items.hasGolem();
}
//---------------------------------------------------------------------------
const d2ce::Item& d2ce::Character::getGolemItem() const
{
    return m_items.getGolemItem();
}
//---------------------------------------------------------------------------
void d2ce::Character::fillBasicStats(BasicStats& bs)
{
    std::memcpy(&bs, &Bs, sizeof(BasicStats));
}
//---------------------------------------------------------------------------
void d2ce::Character::fillCharacterStats(CharStats& cs)
{
    Cs.fillCharacterStats(cs);
}
//---------------------------------------------------------------------------
void d2ce::Character::updateBasicStats(BasicStats& bs)
{
    // Clean up new stat values
    bs.Version = getVersion();

    // ladder is for 1.10 or higher
    if (bs.Version < EnumCharVersion::v110)
    {
        bs.Status &= ~EnumCharStatus::Ladder;
    }

    if ((bs.Status & EnumCharStatus::Hardcore) != 0)
    {
        bs.Status &= ~EnumCharStatus::Died; // can't be resurrected
    }

    if (bs.Version < EnumCharVersion::v109)
    {
        if (bs.Version < EnumCharVersion::v107 || bs.Version == EnumCharVersion::v108)
        {
            // expansion not supported
            bs.Status &= ~EnumCharStatus::Expansion;
        }
    }

    // Check class
    if (!bs.isExpansionCharacter())
    {
        switch (bs.Class)
        {
        case EnumCharClass::Druid:
        case EnumCharClass::Assassin:
            switch (Bs.Class)
            {
            case EnumCharClass::Druid:
            case EnumCharClass::Assassin:
                bs.Class = EnumCharClass::Amazon;
                break;
            default:
                bs.Class = Bs.Class;
                break;
            }
            break;
        }
    }

    // Check Name
    // Remove any invalid characters from the name
    bs.Name[15] = 0; // must be zero
    std::string curName(bs.Name.data());
    std::string strNewText;
    for (size_t iPos = 0, numberOfUnderscores = 0, nLen = curName.size(); iPos < nLen; ++iPos)
    {
        char c = curName[iPos];
        if (std::isalpha(c))
        {
            strNewText += c;
        }
        else if ((c == '_' || c == '-') && !strNewText.empty() && numberOfUnderscores < 1)
        {
            strNewText += c;
            ++numberOfUnderscores;
        }
    }

    // trim bad characters
    strNewText.erase(strNewText.find_last_not_of("_-") + 1);
    if (strNewText.size() < 2)
    {
        strNewText = std::string(Bs.Name.data());
    }

    bs.Name.fill(0);
    strcpy_s(bs.Name.data(), strNewText.length() + 1, strNewText.c_str());
    bs.Name[15] = 0; // must be zero

    // Check Title
    if (bs.isExpansionCharacter() != Bs.isExpansionCharacter())
    {
        auto oldNumActs = Bs.getNumActs();
        auto oldTitleDiff = Bs.Title / oldNumActs;
        auto oldTitleAct = Bs.Title % oldNumActs;

        auto newNumActs = bs.getNumActs();
        auto newTitleDiff = bs.Title / newNumActs;
        auto newTitleAct = bs.Title % newNumActs;
        if (bs.Title == Bs.Title)
        {
            // make sure title makes sense
            newTitleDiff = oldTitleDiff;
            newTitleAct = std::min(oldTitleAct, newNumActs - 1);
            bs.Title = std::uint8_t(newTitleDiff * newNumActs + newTitleAct);
        }
        else if (newTitleDiff == oldTitleDiff)
        {
            newTitleAct = std::min(newTitleAct, newNumActs - 1);
            bs.Title = std::uint8_t(newTitleDiff * newNumActs + newTitleAct);
        }
    }
    bs.Title = std::min(bs.Title, bs.getGameCompleteTitle());
    bs.StartingAct = std::min(bs.getLastAct(), bs.StartingAct);
    if (bs.getStartingActTitle() > bs.Title)
    {
        bs.Title = bs.getStartingActTitle();
    }

    auto oldClass = Bs.Class;
    std::memcpy(&Bs, &bs, sizeof(BasicStats));
    Bs.Name[15] = 0; // must be zero

    StartingAct.fill(0);
    StartingAct[static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed)] = 0x80 | static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);

    if (oldClass != Bs.Class)
    {
        // classed changed
        Cs.updateClass(Bs.Class);
    }

    Acts.validateActs();
}
//---------------------------------------------------------------------------
void d2ce::Character::updateCharacterStats(CharStats& cs)
{
    Cs.updateCharacterStats(cs);
    DisplayLevel = (std::uint8_t)Cs.getLevel(); // updates character's display level
    Cs.updatePointsEarned(Acts.getLifePointsEarned(), Acts.getStatPointsEarned(), Acts.getSkillPointsEarned());

    // update values sent in to reflect any updates made
    Cs.fillCharacterStats(cs);
}
//---------------------------------------------------------------------------
void d2ce::Character::resetStats()
{
    Cs.resetStats(Acts.getLifePointsEarned(), Acts.getStatPointsEarned(), Acts.getSkillPointsEarned());
}
//---------------------------------------------------------------------------
d2ce::EnumCharVersion d2ce::Character::getVersion() const
{
    if (Version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v107))
    {
        return EnumCharVersion::v100;
    }

    if (Version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v108))
    {
        return EnumCharVersion::v107;
    }

    if (Version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v109))
    {
        return EnumCharVersion::v108;
    }

    if (Version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v110))
    {
        return EnumCharVersion::v109;
    }

    if (Version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v115))
    {
        return EnumCharVersion::v110;
    }

    return EnumCharVersion::v115;
}
//---------------------------------------------------------------------------
const std::array<char, d2ce::NAME_LENGTH>& d2ce::Character::getName() const
{
    return Bs.Name;
}
//---------------------------------------------------------------------------
bitmask::bitmask<d2ce::EnumCharStatus> d2ce::Character::getStatus() const
{
    return Bs.Status;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Character::getTitle() const
{
    return Bs.Title;
}
//---------------------------------------------------------------------------
d2ce::EnumCharTitle d2ce::Character::getTitleEnum() const
{
    return Bs.getTitleEnum();
}
//---------------------------------------------------------------------------
/*
   Returns a value indicating the difficulty level as give by the title
*/
d2ce::EnumDifficulty d2ce::Character::getTitleDifficulty() const
{
    return Bs.getTitleDifficulty();
}
//---------------------------------------------------------------------------
/*
   Returns a value indicating the act currently completing
*/
d2ce::EnumAct d2ce::Character::getTitleAct() const
{
    return Bs.getTitleAct();
}
//---------------------------------------------------------------------------
void d2ce::Character::ensureTitleAct(d2ce::EnumAct act)
{
    if (Bs.isGameComplete())
    {
        return;
    }

    // make sure act makes sense
    auto progression = getTitleDifficulty();
    while (act > EnumAct::I)
    {
        if (!Acts.getActYetToStart(progression, act))
        {
            break;
        }

        auto preAct = static_cast<EnumAct>(static_cast<std::underlying_type_t<EnumAct>>(act) - 1);
        if (Acts.getActCompleted(progression, preAct))
        {
            break;
        }

        act = preAct;
    }

    auto actProgression = getTitleAct();
    if (act > actProgression)
    {
        // Not able to allow this state
        actProgression = act;
        Bs.Title = std::uint8_t(static_cast<std::underlying_type_t<EnumDifficulty>>(progression) * Bs.getNumActs() + static_cast<std::underlying_type_t<EnumCharVersion>>(actProgression));
    }
}
//---------------------------------------------------------------------------
d2ce::EnumCharClass d2ce::Character::getClass() const
{
    return Bs.Class;
}
//---------------------------------------------------------------------------
std::string d2ce::Character::getClassName() const
{
    auto idx = (std::uint8_t)getClass();
    if (idx < NUM_OF_CLASSES)
    {
        return ClassNames[idx];
    }

    return "";
}
//---------------------------------------------------------------------------
/*
   Returns a value indicating the difficulty level the character last played at
*/
d2ce::EnumDifficulty d2ce::Character::getDifficultyLastPlayed() const
{
    return Bs.DifficultyLastPlayed;
}
//---------------------------------------------------------------------------
/*
   Returns a value indicating the act the character was saved at
*/
d2ce::EnumAct d2ce::Character::getStartingAct() const
{
    return Bs.StartingAct;
}
//---------------------------------------------------------------------------
d2ce::EnumAct d2ce::Character::getLastAct() const
{
    return Bs.getLastAct();
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Character::getNumActs() const
{
    return Bs.getNumActs();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getWeaponSet() const
{
    return WeaponSet;
}
//---------------------------------------------------------------------------
bool d2ce::Character::isExpansionCharacter() const
{
    return Bs.isExpansionCharacter();
}
//---------------------------------------------------------------------------
void d2ce::Character::setIsExpansionCharacter(bool flag)
{
    Bs.setIsExpansionCharacter(flag);
}
//---------------------------------------------------------------------------
bool d2ce::Character::isLadderCharacter() const
{
    return Bs.isLadderCharacter();
}
//---------------------------------------------------------------------------
void d2ce::Character::setIsLadderCharacter(bool flag)
{
    Bs.setIsLadderCharacter(flag);
}
//---------------------------------------------------------------------------
bool d2ce::Character::isHardcoreCharacter() const
{
    return Bs.isHardcoreCharacter();
}
//---------------------------------------------------------------------------
void d2ce::Character::setIsHardcoreCharacter(bool flag)
{
    Bs.setIsHardcoreCharacter(flag);
}
//---------------------------------------------------------------------------
bool d2ce::Character::isResurrectedCharacter() const
{
    return Bs.isResurrectedCharacter();
}
//---------------------------------------------------------------------------
void d2ce::Character::setIsResurrectedCharacter(bool flag)
{
    Bs.setIsResurrectedCharacter(flag);
}
//---------------------------------------------------------------------------
bool d2ce::Character::isDeadCharacter() const
{
    return Bs.isDeadCharacter();
}
//---------------------------------------------------------------------------
void d2ce::Character::setIsDeadCharacter(bool flag)
{
    Bs.setIsDeadCharacter(flag);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevel() const
{
    return Cs.getLevel();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getExperience() const
{
    return Cs.getExperience();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMaxGoldInBelt() const
{
    return Cs.getMaxGoldInBelt();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMaxGoldInStash() const
{
    return Cs.getMaxGoldInStash();
}
//---------------------------------------------------------------------------
bool d2ce::Character::isGameComplete() const
{
    return Bs.isGameComplete();
}
//---------------------------------------------------------------------------
void d2ce::Character::setGameComplete()
{
    setDifficultyComplete(EnumDifficulty::Hell);
}
//---------------------------------------------------------------------------
bool d2ce::Character::isDifficultyComplete(d2ce::EnumDifficulty diff) const
{
    return Bs.isDifficultyComplete(diff);
}
//---------------------------------------------------------------------------
void d2ce::Character::setDifficultyComplete(d2ce::EnumDifficulty diff)
{
    auto title = std::uint8_t((static_cast<std::underlying_type_t<EnumDifficulty>>(diff) + 1) * Bs.getNumActs());
    if (Bs.Title == title)
    {
        return;
    }

    Bs.Title = title;

    // fix up other indicators of progression
    title = Bs.getStartingActTitle();
    if (title > Bs.Title)
    {
        Bs.DifficultyLastPlayed = Bs.getTitleDifficulty();
        Bs.StartingAct = Bs.getTitleAct();
    }

    Acts.validateActs();
}
//---------------------------------------------------------------------------
void d2ce::Character::setNoDifficultyComplete()
{
    auto diff = EnumDifficulty::Normal;
    if (!isDifficultyComplete(diff))
    {
        return;
    }

    Bs.Title = static_cast<std::underlying_type_t<EnumAct>>(EnumAct::V);

    // fix up other indicators of progression
    auto title = Bs.getStartingActTitle();
    if (title > Bs.Title)
    {
        Bs.DifficultyLastPlayed = Bs.getTitleDifficulty();
        Bs.StartingAct = Bs.getTitleAct();
    }

    Acts.validateActs();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMinStrength() const
{
    return Cs.getMinStrength();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMinEnergy() const
{
    return Cs.getMinEnergy();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMinDexterity() const
{
    return Cs.getMinDexterity();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMinVitality() const
{
    return Cs.getMinVitality();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMaxHitPoints() const
{
    return Cs.getMaxHitPoints();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMaxStamina() const
{
    return Cs.getMaxStamina();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMaxMana() const
{
    return Cs.getMaxMana();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getSkillPointsEarned() const
{
    return getSkillPointsEarned(Cs.getLevel());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getSkillPointsEarned(std::uint32_t level) const
{
    return (std::min(d2ce::NUM_OF_LEVELS, level) - 1) + Acts.getSkillPointsEarned();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromTotalSkillPoints() const
{
    return getLevelFromSkillPointsEarned(getTotalSkillPoints());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromSkillPointsEarned(std::uint32_t earned) const
{
    return std::min(d2ce::NUM_OF_LEVELS + 1, earned - Acts.getSkillPointsEarned() + 1);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getTotalStartStatPoints() const
{
    return Cs.getTotalStartStatPoints();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getTotalStatPoints() const
{
    return Cs.getTotalStatPoints();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getStatPointsUsed() const
{
    return Cs.getStatPointsUsed();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getStatPointsEarned() const
{
    return getStatPointsEarned(Cs.getLevel());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getStatPointsEarned(std::uint32_t level) const
{
    return std::uint16_t(std::min(d2ce::NUM_OF_LEVELS, level) - 1) * 5 + Acts.getStatPointsEarned();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromTotalStatPoints() const
{
    if (Cs.getTotalStatPoints() <= Cs.getTotalStartStatPoints())
    {
        return 1;
    }

    return getLevelFromStatPointsEarned(Cs.getTotalStatPoints() - Cs.getTotalStartStatPoints());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromStatPointsEarned(std::uint32_t earned) const
{
    return std::min(d2ce::NUM_OF_LEVELS + 1, (earned - Acts.getStatPointsEarned()) / 5 + 1);
}
//---------------------------------------------------------------------------
const d2ce::ActsInfo& d2ce::Character::getQuests()
{
    return Acts;
}
//---------------------------------------------------------------------------
void d2ce::Character::updateQuests(const ActsInfo& qi)
{
    Acts.updateQuests(qi);
    Cs.updatePointsEarned(Acts.getLifePointsEarned(), Acts.getStatPointsEarned(), Acts.getSkillPointsEarned());
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::Character::getWaypoints(d2ce::EnumDifficulty difficulty) const
{
    return Acts.getWaypoints(difficulty);
}
//---------------------------------------------------------------------------
void d2ce::Character::setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue)
{
    Acts.setWaypoints(difficulty, newvalue);
}
//---------------------------------------------------------------------------
std::array<std::uint8_t, d2ce::NUM_OF_SKILLS>& d2ce::Character::getSkills()
{
    return Cs.getSkills();
}
//---------------------------------------------------------------------------
void d2ce::Character::updateSkills(const std::array<std::uint8_t, NUM_OF_SKILLS>& updated_skills, std::uint32_t skillChoices)
{
    Cs.updateSkills(updated_skills, Acts.getSkillPointsEarned(), skillChoices);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getTotalSkillPoints() const
{
    return Cs.getTotalSkillPoints();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getSkillPointsUsed() const
{
    return Cs.getSkillPointsUsed();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getSkillChoices() const
{
    return Cs.getSkillChoices();
}
//---------------------------------------------------------------------------
bool d2ce::Character::areSkillsMaxed() const
{
    return Cs.areSkillsMaxed();
}
//---------------------------------------------------------------------------
void d2ce::Character::maxSkills()
{
    Cs.maxSkills();
}
//---------------------------------------------------------------------------
void d2ce::Character::resetSkills()
{
    Cs.resetSkills(Acts.getSkillPointsEarned());
}
//---------------------------------------------------------------------------
void d2ce::Character::clearSkillChoices()
{
    Cs.clearSkillChoices();
}
//---------------------------------------------------------------------------
/*
   Returns the number of m_items belonging to the character
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Character::getNumberOfItems() const
{
    return m_items.getNumberOfItems();
}
//---------------------------------------------------------------------------
/*
   Returns the number of m_items equipped on the character
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Character::getNumberOfEquippedItems() const
{
    return m_items.getNumberOfEquippedItems();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Character::getEquippedItems() const
{
    return m_items.getEquippedItems();
}
//---------------------------------------------------------------------------
bool d2ce::Character::getHasBeltEquipped() const
{
    return m_items.getHasBeltEquipped();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getMaxNumberOfItemsInBelt() const
{
    return m_items.getMaxNumberOfItemsInBelt();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfItemsInBelt() const
{
    return m_items.getNumberOfItemsInBelt();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Character::getItemsInBelt() const
{
    return m_items.getItemsInBelt();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfItemsInInventory() const
{
    return m_items.getNumberOfItemsInInventory();
}
//---------------------------------------------------------------------------
/*
   Returns the number of m_items in character's inventory
   Value returned excludes socketed gems/jewels/runes.
*/
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Character::getItemsInInventory() const
{
    return m_items.getItemsInInventory();
}
//---------------------------------------------------------------------------
/*
   Returns the number of m_items in character's private stash
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Character::getNumberOfItemsInStash() const
{
    return m_items.getNumberOfItemsInStash();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Character::getItemsInStash() const
{
    return m_items.getItemsInStash();
}
//---------------------------------------------------------------------------
bool d2ce::Character::getHasHoradricCube() const
{
    return m_items.getHasHoradricCube();
}
//---------------------------------------------------------------------------
/*
   Returns the number of m_items in character's Horadric Cube
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Character::getNumberOfItemsInHoradricCube() const
{
    return m_items.getNumberOfItemsInHoradricCube();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Character::getItemsInHoradricCube() const
{
    return m_items.getItemsInHoradricCube();
}
//---------------------------------------------------------------------------
bool d2ce::Character::getItemLocationDimensions(EnumItemLocation locationId, EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const
{
    return m_items.getItemLocationDimensions(locationId, altPositionId, dimensions);
}
//---------------------------------------------------------------------------
bool d2ce::Character::getItemLocationDimensions(EnumItemLocation locationId, ItemDimensions& dimensions) const
{
    return m_items.getItemLocationDimensions(locationId, dimensions);
}
//---------------------------------------------------------------------------
bool d2ce::Character::getItemLocationDimensions(EnumAltItemLocation altPositionId, ItemDimensions& dimensions) const
{
    return m_items.getItemLocationDimensions(altPositionId, dimensions);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfArmor() const
{
    return m_items.getNumberOfArmor();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfWeapons() const
{
    return m_items.getNumberOfWeapons();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::repairAllItems(d2ce::ItemFilter filter)
{
    return m_items.repairAllItems(filter);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::maxDurabilityAllItems(d2ce::ItemFilter filter)
{
    return m_items.maxDurabilityAllItems(filter);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::setIndestructibleAllItems(d2ce::ItemFilter filter)
{
    return m_items.setIndestructibleAllItems(filter);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::maxSocketCountAllItems(d2ce::ItemFilter filter)
{
    return m_items.maxSocketCountAllItems(filter);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfStackables() const
{
    return m_items.getNumberOfStackables();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::fillAllStackables(d2ce::ItemFilter filter)
{
    return m_items.fillAllStackables(filter);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfGPSs() const
{
    return m_items.getNumberOfGPSs();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Character::getGPSs()
{
    return m_items.getGPSs();
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems, potions or skulls to the specified
   final gem, potion or skull.
   Returns the number of gems converted.
*/
size_t d2ce::Character::convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, ItemFilter filter)
{
    return m_items.convertGPSs(existingGem, desiredGem, filter);
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-perfect gems.
   Returns false if no gems or all gems are perfect.
*/
bool d2ce::Character::anyUpgradableGems() const
{
    return m_items.anyUpgradableGems();
}
//---------------------------------------------------------------------------
/*
   Converts the all gems to their perfect state
   Returns the number of gems converted.
*/
size_t d2ce::Character::upgradeGems(ItemFilter filter)
{
    return m_items.upgradeGems(filter);
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-Super Health/Mana potions or
                non-Full Rejuvenation potions.
   Returns false if no potions or all potions are the top kind.
*/
bool d2ce::Character::anyUpgradablePotions() const
{
    return m_items.anyUpgradablePotions();
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to their highest quiality.
   Returns the number of potions converted.
*/
size_t d2ce::Character::upgradePotions(ItemFilter filter)
{
    return m_items.upgradePotions(filter);
}
//---------------------------------------------------------------------------
/*
   Returns true if there are any non-Full Rejuvenation potions.
   Returns false if no potions or all potions Full Rejuvenation potions.
*/
bool d2ce::Character::anyUpgradableRejuvenations() const
{
    return m_items.anyUpgradableRejuvenations();
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to Full Rejuvenation potions.
   Returns the number of potions converted.
*/
size_t d2ce::Character::upgradeRejuvenationPotions(ItemFilter filter)
{
    return m_items.upgradeRejuvenationPotions(filter);
}
//---------------------------------------------------------------------------
bool d2ce::Character::addItem(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    return m_items.addItem(locationId, altPositionId, strcode);
}
//---------------------------------------------------------------------------
bool d2ce::Character::addItem(EnumItemLocation locationId, std::array<std::uint8_t, 4>& strcode)
{
    return m_items.addItem(locationId, strcode);
}
//---------------------------------------------------------------------------
bool d2ce::Character::addItem(EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    return m_items.addItem(altPositionId, strcode);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::fillEmptySlots(EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    return m_items.fillEmptySlots(locationId, altPositionId, strcode);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::fillEmptySlots(EnumItemLocation locationId, std::array<std::uint8_t, 4>& strcode)
{
    return m_items.fillEmptySlots(locationId, strcode);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::fillEmptySlots(EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    return m_items.fillEmptySlots(altPositionId, strcode);
}
//---------------------------------------------------------------------------
bool d2ce::Character::getItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    return m_items.getItemBonuses(attribs);
}
//---------------------------------------------------------------------------
bool d2ce::Character::getDisplayedItemBonuses(std::vector<MagicalAttribute>& attribs) const
{
    return m_items.getDisplayedItemBonuses(attribs, getLevel());
}
//---------------------------------------------------------------------------
d2ce::SharedStash& d2ce::Character::getSharedStash()
{
    return m_shared_stash;
}
//---------------------------------------------------------------------------
bool d2ce::Character::hasSharedStash() const
{
    return m_shared_stash.hasSharedStash();
}
//---------------------------------------------------------------------------
