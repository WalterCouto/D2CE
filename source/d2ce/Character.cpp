/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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

#include "pch.h"
#include "Character.h"
#include "CharacterConstants.h"
#include "ExperienceConstants.h"
#include "ItemConstants.h"
#include "SkillConstants.h"
#include "SharedStash.h"
#include "helpers/DefaultTxtReader.h"
#include "helpers/ItemHelpers.h"
#include <fstream>
#include <utf8/utf8.h>
#include <shlobj.h>
#include <objbase.h>

//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::array<std::uint8_t, 4> HEADER = { 0x55, 0xAA, 0x55, 0xAA };

    constexpr std::uint16_t CHAR_V100_BASICINFO_NUM_BYTES = 130ui32;
    constexpr std::uint16_t CHAR_V109_BASICINFO_NUM_BYTES = 335ui32;

    constexpr std::uint32_t CHAR_V100_VERSION_BYTE_OFFSET = 4ui32; // pos 4 in file, character file version
    constexpr std::uint16_t CHAR_V100_VERSION_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_V109_FILESIZE_BYTE_OFFSET = 8ui32; // pos 8 (1.09+ only), file's size
    constexpr std::uint16_t CHAR_V109_FILESIZE_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_V109_CHECKSUM_BYTE_OFFSET = 12ui32; // pos 12 (1.09+ only), stores (possible) checksum
    constexpr std::uint16_t CHAR_V109_CHECKSUM_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_V100_WEAPONSET_BYTE_OFFSET = 26ui32; // pos 16 (1.09+, otherwise pos 26 uint16_t)
    constexpr std::uint32_t CHAR_V109_WEAPONSET_BYTE_OFFSET = 16ui32;
    constexpr std::uint16_t CHAR_V100_WEAPONSET_NUM_BYTES = 1ui32;
    constexpr std::uint16_t CHAR_V109_WEAPONSET_NUM_BYTES = 4ui32;

    constexpr std::array<std::uint8_t, 6> UNKNOWN_01C_v100 = { 0xDD, 0x00, 0x10, 0x00, 0x82, 0x00 };
    constexpr std::array<std::uint8_t, 6> UNKNOWN_01C_v107 = { 0x3F, 0x01, 0x10, 0x00, 0x82, 0x00 };

    constexpr std::uint32_t CHAR_V100_NAME_BYTE_OFFSET = 8ui32;     // pos 267 (D2R 1.2+, pos 20 for 1.09 - 1.14d, otherwise pos 8),
    constexpr std::uint32_t CHAR_V109_NAME_BYTE_OFFSET = 20ui32;
    constexpr std::uint32_t CHAR_V120_NAME_BYTE_OFFSET = 267ui32;
    constexpr std::uint16_t CHAR_V100_NAME_NUM_BYTES = 16ui32;     // name includes terminating NULL

    constexpr std::uint32_t CHAR_V100_STATUS_BYTE_OFFSET = 24ui32; // pos 36 (1.09+, otherwise, pos 24), character's status
    constexpr std::uint32_t CHAR_V109_STAUTS_BYTE_OFFSET = 36ui32;
    constexpr std::uint16_t CHAR_V100_STAUTS_NUM_BYTES = 1ui32;

    constexpr std::uint32_t CHAR_V100_TITLE_BYTE_OFFSET = 25ui32; // pos 37 (1.09+, otherwise pos 25), character's title
    constexpr std::uint32_t CHAR_V109_TITLE_BYTE_OFFSET = 37ui32;
    constexpr std::uint16_t CHAR_V100_TITLE_NUM_BYTES = 1ui32;

    constexpr std::uint32_t CHAR_V100_CLASS_BYTE_OFFSET = 34ui32; // pos 40 (1.09+, otherwise pos 34), character's class
    constexpr std::uint32_t CHAR_V109_CLASS_BYTE_OFFSET = 40ui32;
    constexpr std::uint16_t CHAR_V100_CLASS_NUM_BYTES = 1ui32;

    constexpr std::array<std::uint8_t, 2> UNKNOWN_029 = { 0x10, 0x1E };

    constexpr std::uint32_t CHAR_V109_DISPLAYLEVEL_BYTE_OFFSET = 43ui32; // pos 43 (1.09+, otherwise pos 36)
    constexpr std::uint32_t CHAR_V100_DISPLAYLEVEL_BYTE_OFFSET = 36ui32;
    constexpr std::uint16_t CHAR_V100_DISPLAYLEVEL_NUM_BYTES = 1ui32;

    constexpr std::uint32_t CHAR_V100_STARTINGACT_BYTE_OFFSET = 88i32; // pos 168 (1.09+, otherwise pos 88), last difficulty and starting act
    constexpr std::uint32_t CHAR_V109_STARTINGACT_BYTE_OFFSET = 168ui32;
    constexpr std::uint16_t CHAR_V100_STARTINGACT_NUM_BYTES = 1ui32;
    constexpr std::uint16_t CHAR_V109_STARTINGACT_NUM_BYTES = 3ui32;

    constexpr std::uint32_t CHAR_V100_APPEARANCES_BYTE_OFFSET = 38i32; // pos 136 (1.09+, otherwise pos 38) Character menu appearance
    constexpr std::uint32_t CHAR_V109_APPEARANCES_BYTE_OFFSET = 136ui32;
    constexpr std::uint16_t CHAR_V100_APPEARANCES_NUM_BYTES = 32ui32;

    constexpr std::uint32_t CHAR_V109_CREATED_BYTE_OFFSET = 44ui32; // pos 44 (1.09+ only), file date and time
    constexpr std::uint16_t CHAR_V109_CREATED_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_V109_LASTPLAYED_BYTE_OFFSET = 48ui32; // pos 48 (1.09+ only), file date and time
    constexpr std::uint16_t CHAR_V109_LASTPLAYED_NUM_BYTES = 4ui32;

    constexpr std::array<std::uint8_t, 4> UNKNOWN_034 = { 0xFF, 0xFF, 0xFF, 0xFF };

    constexpr std::uint32_t CHAR_V100_ASSIGNED_SKILLS_BYTE_OFFSET = 70i32; // pos 56 (1.09+, otherwise pos 70 and size 8)
    constexpr std::uint32_t CHAR_V109_ASSIGNED_SKILLS_BYTE_OFFSET = 56ui32;
    constexpr std::uint16_t CHAR_V100_ASSIGNED_SKILLS_NUM_BYTES = NUM_OF_SKILL_HOTKEYS;
    constexpr std::uint16_t CHAR_V109_ASSIGNED_SKILLS_NUM_BYTES = std::uint16_t(NUM_OF_SKILL_HOTKEYS * sizeof(std::uint32_t));

    constexpr std::uint32_t CHAR_V100_LEFTSKILL_BYTE_OFFSET = 86i32; // pos 120 (1.09+, otherwise pos 86)
    constexpr std::uint32_t CHAR_V109_LEFTSKILL_BYTE_OFFSET = 120ui32;
    constexpr std::uint16_t CHAR_V100_LEFTSKILL_NUM_BYTES = 1ui32;
    constexpr std::uint16_t CHAR_V109_LEFTSKILL_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_V100_RIGHTSKILL_BYTE_OFFSET = 87i32; // pos 124 (1.09+, otherwise pos 87)
    constexpr std::uint32_t CHAR_V109_RIGHTSKILL_BYTE_OFFSET = 124ui32;
    constexpr std::uint16_t CHAR_V100_RIGHTSKILL_NUM_BYTES = 1ui32;
    constexpr std::uint16_t CHAR_V109_RIGHTSKILL_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_V100_MAPID_BYTE_OFFSET = 126i32; // pos 171 (1.09+, otherwise 126)
    constexpr std::uint32_t CHAR_V109_MAPID_BYTE_OFFSET = 171ui32;
    constexpr std::uint16_t CHAR_V100_MAPID_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_V109_LEFTSWAPSKILL_BYTE_OFFSET = 128i32; // pos 128 (1.09+ only)
    constexpr std::uint16_t CHAR_V109_LEFTSWAPSKILL_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_V109_RIGHTSWAPSKILL_BYTE_OFFSET = 132i32; // pos 132 (1.09+ only)
    constexpr std::uint16_t CHAR_V109_RIGHTSWAPSKILL_NUM_BYTES = 4ui32;

    constexpr std::uint32_t CHAR_v100R_APPEARANCES_BYTE_OFFSET = 219ui32; // pos 219 (D2R only) Character menu appearance
    constexpr std::uint16_t CHAR_v100R_APPEARANCES_NUM_BYTES = 48ui32;
    
    constexpr std::array<std::uint8_t, 48> DEFAULT_0DB_v100R = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    constexpr std::array<std::uint16_t, 4> UNKNOWN_14B = { 0x01, 0x00, 0x00, 0x00 };

    void ApplyJsonAppearnces(const Json::Value& appearances, std::uint8_t* appearancesValue)
    {
        if (appearancesValue == nullptr)
        {
            return;
        }

        std::memset(appearancesValue, MAXUINT8, APPEARANCES_LENGTH);

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
            else if (_stricmp(key.c_str(), "head") != 0)
            {
                continue;
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
    
    void ApplyJsonD2RAppearnces(const Json::Value& appearances, std::uint8_t* appearancesValue)
    {
        if (appearancesValue == nullptr)
        {
            return;
        }

        std::memcpy(appearancesValue, DEFAULT_0DB_v100R.data(), DEFAULT_0DB_v100R.size());

        size_t startIdx = 0;
        size_t idx = 0;
        std::string key;
        std::string typecode;
        auto iter_end = appearances.end();
        size_t byteSize = 12;
        for (auto iter = appearances.begin(); iter != iter_end; ++iter)
        {
            if (iter->isNull())
            {
                continue;
            }

            static std::initializer_list<std::string> all_d2r_appearance_props = { "RightHand", "LeftHand", "Torso", "Head" };

            key = iter.name();
            idx = 0;
            if ((strcmp(key.c_str(), "left_hand") == 0) || (strcmp(key.c_str(), "LeftHand") == 0))
            {
                startIdx = byteSize;
            }
            else if (_stricmp(key.c_str(), "torso") == 0)
            {
                startIdx = 2 * byteSize;
            }
            else if (_stricmp(key.c_str(), "head") == 0)
            {
                startIdx = 3 * byteSize;
            }
            else if ((strcmp(key.c_str(), "right_hand") != 0) && (strcmp(key.c_str(), "RightHand") != 0))
            {
                continue;
            }

            auto iter2_end = iter->end();
            for (auto iter2 = iter->begin(); iter2 != iter2_end; ++iter2)
            {
                if (iter2->isNull())
                {
                    continue;
                }

                idx = startIdx;
                key = iter2.name();
                if (_stricmp(key.c_str(), "code") == 0)
                {
                    typecode = iter2->asString();
                    for (const auto& ch : typecode)
                    {
                        if ((idx - startIdx) >= 4)
                        {
                            break;
                        }

                        appearancesValue[idx++] = std::uint8_t(ch);
                    }
                }
                else
                {
                    if (_stricmp(key.c_str(), "tint") == 0)
                    {
                        idx += 4;
                    }
                    else if (_stricmp(key.c_str(), "quality") == 0)
                    {
                        idx += 5;
                    }
                    else if (_stricmp(key.c_str(), "id") == 0)
                    {
                        idx += 6;
                    }
                    else if (_stricmp(key.c_str(), "unk1") == 0)
                    {
                        idx += 7;
                    }
                    else if (_stricmp(key.c_str(), "unk2") == 0)
                    {
                        idx += 8;
                    }
                    else if (_stricmp(key.c_str(), "unk3") == 0)
                    {
                        idx += 9;
                    }
                    else if (_stricmp(key.c_str(), "unk4") == 0)
                    {
                        idx += 10;
                    }
                    else if (_stricmp(key.c_str(), "unk5") == 0)
                    {
                        idx += 11;
                    }
                    else
                    {
                        continue;
                    }

                    appearancesValue[idx] = std::uint8_t(iter2->asInt());
                }
            }
        }
    }

    std::uint16_t ApplyJsonSkill(const Json::Value& skill)
    {
        if (skill.isNull())
        {
            return MAXUINT16;
        }

        if (skill.isObject())
        {
            Json::Value value = skill["Id"];
            if (value.isNull())
            {
                return MAXUINT16;
            }

            return std::uint16_t(value.asInt());
        }

        const auto& skillInfo = CharClassHelper::getSkillByIndex(skill.asString());
        return skillInfo.id;
    }
    
    std::uint8_t ApplyJsonSkill8(const Json::Value& skill)
    {
        if (skill.isNull())
        {
            return MAXUINT8;
        }

        if (skill.isObject())
        {
            Json::Value value = skill["Id"];
            if (value.isNull())
            {
                return MAXUINT8;
            }

            return std::uint8_t(value.asInt());
        }

        const auto& skillInfo = CharClassHelper::getSkillByIndex(skill.asString());
        return std::uint8_t(skillInfo.id);
    }

    void ApplyJsonAssignedSkills(const Json::Value& assignedSkills, std::uint32_t *assignedSkillsValue)
    {
        if (assignedSkillsValue == nullptr)
        {
            return;
        }

        for (size_t idx = 0; idx < NUM_OF_SKILL_HOTKEYS; ++idx)
        {
            assignedSkillsValue[idx] = MAXUINT16;
        }

        Json::Value value;
        size_t idx = 0;
        auto iter_end = assignedSkills.end();
        for (auto iter = assignedSkills.begin(); iter != iter_end && idx < NUM_OF_SKILL_HOTKEYS; ++iter, ++idx)
        {
            assignedSkillsValue[idx] = ApplyJsonSkill(*iter);
        }
    }

    void ApplyJsonAssignedSkills(const Json::Value& assignedSkills, std::uint8_t* assignedSkillsValue)
    {
        if (assignedSkillsValue == nullptr)
        {
            return;
        }

        std::memset(assignedSkillsValue, MAXUINT8, NUM_OF_SKILL_HOTKEYS);

        Json::Value value;
        size_t idx = 0;
        auto iter_end = assignedSkills.end();
        for (auto iter = assignedSkills.begin(); iter != iter_end && idx < NUM_OF_SKILL_HOTKEYS; ++iter, ++idx)
        {
            assignedSkillsValue[idx] = ApplyJsonSkill8(*iter);
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

#define read_uint32_bits(start,size) \
    ((*((std::uint32_t *) &data[(start) / 8]) >> ((start) & 7))& (((std::uint32_t)1 << (size)) - 1))

#define read_uint64_bits(start,size) \
    ((*((std::uint64_t*) &data[(start) / 8]) >> ((start) & 7))& (((std::uint64_t)1 << (size)) - 1))


    std::filesystem::path GetD2RGamesFolder()
    {
        PWSTR saveBasePath;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SavedGames, 0, nullptr, &saveBasePath)))
        {
            std::filesystem::path savePath(saveBasePath);
            CoTaskMemFree(saveBasePath);

            // Look for Ressurected path first
            auto d2Path = savePath / "Diablo II Resurrected";
            if (std::filesystem::exists(d2Path))
            {
                return d2Path;
            }
        }

        return std::filesystem::path();
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
d2ce::Character::Character() : Cs(*this), Merc(*this), Acts(*this)
{
    initialize();
    m_error_code.clear();
}
//---------------------------------------------------------------------------
d2ce::Character::~Character()
{
}
//---------------------------------------------------------------------------
void d2ce::Character::initialize()
{
    Bs.Name.fill(0);
    Bs.Status = EnumCharStatus::NoDeaths;
    Bs.Title = 0;
    Bs.Class = EnumCharClass::Amazon;
    Bs.DifficultyLastPlayed = EnumDifficulty::Normal;
    Bs.StartingAct = EnumAct::I;
    Merc.clear();
    Acts.clear();

    Cs.clear();

    m_items.clear();

    m_shared_stash.clear();

    if (!ItemHelpers::isTxtReaderInitialized())
    {
        setDefaultTxtReader();
    }
}
//---------------------------------------------------------------------------
/*
   Returns false if file was not opened or there was an error.
*/
bool d2ce::Character::openD2S(const std::filesystem::path& path, bool validateChecksum)
{
    if (is_open())
    {
        if (&path == &getPath())
        {
            auto isjson = is_json();
            std::filesystem::path pathCopy = getPath();
            close();
            if (isjson)
            {
                m_jsonfilename = pathCopy;
            }
            else
            {
                m_d2sfilename = pathCopy;
            }
        }
        else
        {
            close();
        }
    }

    m_error_code.clear();
    if (path.empty() || !std::filesystem::exists(path))
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    m_ftime = std::filesystem::last_write_time(path);

    m_error_code.clear();
    std::FILE* charfile = nullptr;
    errno_t err = _wfopen_s(&charfile, path.wstring().c_str(), L"rb+");
    if (err != 0)
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    if (charfile == nullptr)
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    m_d2sfilename = path;

    readHeader(charfile);
    if (!isValidHeader())
    {
        m_error_code = std::make_error_code(CharacterErrc::InvalidHeader);
        std::fclose(charfile);
        close();
        return false;
    }

    if (!refresh(charfile))
    {
        std::fclose(charfile);
        close();
        return false;
    }

    std::fclose(charfile);

    // Validate checksum
    long curChecksum = getChecksumBytes();
    calculateChecksum();
    if (curChecksum != getChecksumBytes())
    {
        // If data was correct, don't report invalid checksum.
        // It will update on next save.
        if (!Acts.QuestsDataCorrected)
        {
            m_error_code = std::make_error_code(CharacterErrc::InvalidChecksum);
            if (validateChecksum)
            {
                close();
                return false;
            }
        }

        Acts.QuestsDataCorrected = false; // reset
    }

    m_shared_stash.reset(*this);

    validateActs();
    return true;
}
//---------------------------------------------------------------------------
/*
   Returns false if file was not opened or there was an error.
*/
bool d2ce::Character::openJson(const std::filesystem::path& path)
{
    if (is_open())
    {
        if (&path == &getPath())
        {
            auto isjson = is_json();
            std::filesystem::path pathCopy = getPath();
            close();
            if (isjson)
            {
                m_jsonfilename = pathCopy;
            }
            else
            {
                m_d2sfilename = pathCopy;
            }
        }
        else
        {
            close();
        }
    }

    m_error_code.clear();
    if (path.empty() || !std::filesystem::exists(path))
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    m_ftime = std::filesystem::last_write_time(path);

    std::ifstream ifs;
    ifs.open(path.wstring().c_str());
    if (!ifs.is_open())
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }
    m_jsonfilename = path;

    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
    {
        m_error_code = std::make_error_code(CharacterErrc::InvalidHeader);
        return false;
    }
    ifs.close();

    m_bJsonSerializedFormat = false;
    readHeader(root);
    if (!isValidHeader())
    {
        initialize();
        m_error_code = std::make_error_code(CharacterErrc::InvalidHeader);
        return false;
    }

    if (!refresh(root))
    {
        return false;
    }

    validateActs();
    return true;
}
//---------------------------------------------------------------------------
/*
   Returns false if file was not opened or there was an error.
*/
bool d2ce::Character::open(const std::filesystem::path& path, bool validateChecksum)
{
    if (is_open())
    {
        if (&path == &getPath())
        {
            auto isjson = is_json();
            std::filesystem::path pathCopy = getPath();
            close();
            if (isjson)
            {
                m_jsonfilename = pathCopy;
            }
            else
            {
                m_d2sfilename = pathCopy;
            }
        }
        else
        {
            close();
        }
    }

    m_error_code.clear();
    std::wstring ext = path.extension().wstring();
    if (_wcsicmp(ext.c_str(), L".json") == 0)
    {
        return openJson(path);
    }

    return openD2S(path, validateChecksum);
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
    long checksum = 0;
    if (getVersion() < EnumCharVersion::v109)
    {
        // checksum not supported
        return;
    }

    setFileSizeBytes(std::uint32_t(getByteSize()));

    std::uint32_t i = 0;
    std::uint8_t overflow = 0;
    for (; i < CHAR_V109_CHECKSUM_BYTE_OFFSET; ++i)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += data[i] + overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    // skip checksum location
    std::uint32_t nextStop = CHAR_V109_CHECKSUM_BYTE_OFFSET + CHAR_V109_CHECKSUM_NUM_BYTES;
    for (; i < nextStop; ++i)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += overflow;
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    // continue with the rest of the file
    for (; i < data.size(); ++i)
    {
        checksum <<= 1; // doubles the checksum result by left shifting once
        checksum += data[i] + overflow; 
        if (checksum < 0)
        {
            overflow = 1;
        }
        else
        {
            overflow = 0;
        }
    }

    Acts.calculateChecksum(checksum, overflow);
    Cs.calculateChecksum(checksum, overflow);
    m_items.calculateChecksum(checksum, overflow, isExpansionCharacter(), hasMercenary());
    setChecksumBytes(checksum);
}
//---------------------------------------------------------------------------
void d2ce::Character::readHeader(std::FILE* charFile)
{
    // reserve enough space to reduce chance of reallocation
    data.clear();
    data.reserve(CHAR_V109_BASICINFO_NUM_BYTES);

    std::rewind(charFile);
    size_t current_byte_offset = std::ftell(charFile);
    skipBytes(charFile, current_byte_offset, HEADER_LENGTH);
}
//---------------------------------------------------------------------------
void d2ce::Character::readHeader(const Json::Value& root)
{
    data.clear();
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
    size_t current_byte_offset = 0;
    setBytes(current_byte_offset, HEADER_LENGTH, uint);

    if (getVersion() < EnumCharVersion::v100R)
    {
        setTxtReader(getDefaultTxtReader());
        return;
    }

    value = header[m_bJsonSerializedFormat ? "ModName" : "mod_name"];
    if (value.isNull())
    {
        setTxtReader(getDefaultTxtReader());
        return;
    }

    std::string modName(value.asString());
    setTxtReader(getDefaultTxtReader(modName));
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getHeaderBytes() const
{
    return readBytes(0, HEADER_LENGTH);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getVersionBytes() const
{
    return readBytes(CHAR_V100_VERSION_BYTE_OFFSET, CHAR_V100_VERSION_NUM_BYTES); // pos 4 in file, character file version
}
//---------------------------------------------------------------------------
bool d2ce::Character::setFileSizeBytes(std::uint32_t bytes)
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return false;
    }

    return updateBytes(CHAR_V109_FILESIZE_BYTE_OFFSET, CHAR_V109_FILESIZE_NUM_BYTES, bytes); // pos 8 (1.09+ only), file's size
}
//---------------------------------------------------------------------------
long d2ce::Character::getChecksumBytes() const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return 0ui32;
    }

    return (long)readBytes(CHAR_V109_CHECKSUM_BYTE_OFFSET, CHAR_V109_CHECKSUM_NUM_BYTES); // pos 12 (1.09+ only), stores (possible) checksum
}
//---------------------------------------------------------------------------
bool d2ce::Character::setChecksumBytes(long checksum)
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return false;
    }

    return updateBytes(CHAR_V109_CHECKSUM_BYTE_OFFSET, CHAR_V109_CHECKSUM_NUM_BYTES, std::uint32_t(checksum)); // pos 12 (1.09+ only), stores (possible) checksum
}
//---------------------------------------------------------------------------
bool d2ce::Character::setStatusBytes(std::uint8_t status)
{
    Bs.Status = static_cast<EnumCharStatus>(status);
    size_t current_byte_offset = CHAR_V109_STAUTS_BYTE_OFFSET; // pos 36 (1.09+, otherwise, pos 24), character's status
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_STATUS_BYTE_OFFSET;
    }
    return updateBytes(current_byte_offset, CHAR_V100_NAME_NUM_BYTES, Bs.Status.bits());
}
//---------------------------------------------------------------------------
bool d2ce::Character::setTitleBytes(std::uint8_t Title)
{
    Bs.Title = std::min(Title, Bs.getGameCompleteTitle());;
    size_t current_byte_offset = CHAR_V109_TITLE_BYTE_OFFSET; // pos 37 (1.09+, otherwise pos 25), character's title
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_TITLE_BYTE_OFFSET;
    }
    return updateBytes(current_byte_offset, CHAR_V100_TITLE_NUM_BYTES, Bs.Title);
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::Character::getDisplayLevelBytes() const
{
    size_t current_byte_offset = CHAR_V109_DISPLAYLEVEL_BYTE_OFFSET;
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_DISPLAYLEVEL_BYTE_OFFSET;
    }

    return std::uint8_t(readBytes(current_byte_offset, CHAR_V100_DISPLAYLEVEL_NUM_BYTES)); // pos 43 (1.09+, otherwise pos 36)
}
//---------------------------------------------------------------------------
bool d2ce::Character::setDisplayLevelBytes(std::uint8_t level)
{
    size_t current_byte_offset = CHAR_V109_DISPLAYLEVEL_BYTE_OFFSET;
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_DISPLAYLEVEL_BYTE_OFFSET;
    }

    return updateBytes(current_byte_offset, CHAR_V100_DISPLAYLEVEL_NUM_BYTES, level); // pos 43 (1.09+, otherwise pos 36)
}
//---------------------------------------------------------------------------
bool d2ce::Character::setDifficultyLastPlayedBytes(EnumDifficulty difficultyLastPlayed, EnumAct startingAct)
{
    Bs.DifficultyLastPlayed = difficultyLastPlayed;
    Bs.StartingAct = startingAct;
    if (getVersion() < EnumCharVersion::v109)
    {
        std::uint8_t value = static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
        value <<= 4;
        value |= (static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed) & 0x0F);
        return updateBytes(CHAR_V100_STARTINGACT_BYTE_OFFSET, CHAR_V100_STARTINGACT_NUM_BYTES, value); // pos 88, last difficulty and starting act
    }

    auto startAct = &data[CHAR_V109_STARTINGACT_BYTE_OFFSET]; // pos 168 (normal, nightmare, hell; used in 1.09+ only)
                                                              // four MSBs value always 8 (hex, i.e. 0x80)
                                                              // four least significant bits = which act is character saved at
    std::memset(startAct, 0, CHAR_V109_STARTINGACT_NUM_BYTES);
    startAct[static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed)] = 0x80 | static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
    return true;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getCreatedBytes() const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return 0ui32;
    }

    return readBytes(CHAR_V109_CREATED_BYTE_OFFSET, CHAR_V109_CREATED_NUM_BYTES); // pos 44 (1.09+ only), file date and time
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLastPlayedBytes() const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return 0ui32;
    }

    return readBytes(CHAR_V109_LASTPLAYED_BYTE_OFFSET, CHAR_V109_LASTPLAYED_NUM_BYTES); // pos 48 (1.09+ only), file date and time
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLeftSkillBytes() const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return readBytes(CHAR_V100_LEFTSKILL_BYTE_OFFSET, CHAR_V100_LEFTSKILL_NUM_BYTES); // pos 86
    }

    return readBytes(CHAR_V109_LEFTSKILL_BYTE_OFFSET, CHAR_V109_LEFTSKILL_NUM_BYTES); // pos 120
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getRightSkillBytes() const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return readBytes(CHAR_V100_RIGHTSKILL_BYTE_OFFSET, CHAR_V100_RIGHTSKILL_NUM_BYTES); // pos 87
    }

    return readBytes(CHAR_V109_RIGHTSKILL_BYTE_OFFSET, CHAR_V109_RIGHTSKILL_NUM_BYTES); // pos 124
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLeftSwapSkillBytes() const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return 0ui32;
    }

    return readBytes(CHAR_V109_LEFTSWAPSKILL_BYTE_OFFSET, CHAR_V109_LEFTSWAPSKILL_NUM_BYTES); // pos 128 (1.09+ only)
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getRightSwapSkillBytes() const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return 0ui32;
    }

    return readBytes(CHAR_V109_RIGHTSWAPSKILL_BYTE_OFFSET, CHAR_V109_RIGHTSWAPSKILL_NUM_BYTES); // pos 132 (1.09+ only)
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMapIDBytes() const
{
    if (getVersion() < EnumCharVersion::v109)
    {
        return readBytes(CHAR_V100_MAPID_BYTE_OFFSET, CHAR_V100_MAPID_NUM_BYTES); // pos 87
    }

    return readBytes(CHAR_V109_MAPID_BYTE_OFFSET, CHAR_V100_MAPID_NUM_BYTES); // pos 124
}
//---------------------------------------------------------------------------
/*
   Returns true if the file has a valid header
*/
bool d2ce::Character::isValidHeader() const
{
    std::array<std::uint8_t, HEADER_LENGTH> header = { 0x55, 0xAA, 0x55, 0xAA };
    std::uint32_t uint = getHeaderBytes();
    std::memcpy(header.data(), &uint, header.size());
    return header == HEADER ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Character::refresh(std::FILE* charFile)
{
    readBasicInfo(charFile);
    if (!readActs(charFile))
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidActsInfo);
        return false;
    }

    // From this point on, the location is variable
    if (!readStats(charFile))
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidCharStats);
        return false;
    }

    // Read Character, Corpse, Mercenary and Golem m_items
    if (!readItems(charFile))
    {
        // bad file
        close();
        m_error_code = std::make_error_code(CharacterErrc::InvalidItemInventory);
        return false;
    }

    return true;
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

    // store the file's size and checksum if need be
    setFileSizeBytes(std::uint32_t(getByteSize()));
    calculateChecksum();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::refresh()
{
    if (!m_jsonfilename.empty())
    {
        const std::filesystem::path path = m_jsonfilename;
        return openJson(path);
    }

    if (!m_d2sfilename.empty())
    {
        const std::filesystem::path path = m_d2sfilename;
        return openD2S(path);
    }

    return false;
}
//---------------------------------------------------------------------------
void d2ce::Character::readBasicInfo(std::FILE* charFile)
{
    std::fseek(charFile, (long)CHAR_V100_VERSION_BYTE_OFFSET, SEEK_SET);
    size_t current_byte_offset = CHAR_V100_VERSION_BYTE_OFFSET;
    size_t numBytes = CHAR_V100_VERSION_NUM_BYTES;
    skipBytes(charFile, current_byte_offset, numBytes);

    initVersion();
    checkForD2RMod();

    // read in the remaining fixed number of bytes
    if (getVersion() >= EnumCharVersion::v109)
    {
        numBytes = CHAR_V109_BASICINFO_NUM_BYTES - data.size();
        skipBytes(charFile, current_byte_offset, numBytes);

        fillBasicStats(Bs);

        // ladder is for 1.10 or higher
        if (getVersion() < EnumCharVersion::v110)
        {
            Bs.Status &= ~EnumCharStatus::Ladder;
        }

        if ((Bs.Status & EnumCharStatus::Hardcore) != 0)
        {
            Bs.Status &= ~EnumCharStatus::Died; // can't be resurrected
        }

        Merc.readInfo();
    }
    else
    {
        numBytes = CHAR_V100_BASICINFO_NUM_BYTES - data.size();
        skipBytes(charFile, current_byte_offset, numBytes);

        fillBasicStats(Bs);
    }

    if (Bs.getStartingActTitle() > Bs.Title)
    {
        setTitleBytes(Bs.getStartingActTitle());
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

    Json::Value jsonValue = header[m_bJsonSerializedFormat ? "Version" : "version"];
    if (jsonValue.isNull())
    {
        return false;
    }

    std::uint32_t version = std::uint32_t(jsonValue.asInt64());
    size_t current_byte_offset = CHAR_V100_VERSION_BYTE_OFFSET;
    if (!setBytes(current_byte_offset, CHAR_V100_VERSION_NUM_BYTES, version)) // pos 4 in file, character file version
    {
        return false;
    }
    initVersion();

    // read in the remaining fixed number of bytes
    if (getVersion() >= EnumCharVersion::v109)
    {
        data.resize(CHAR_V109_BASICINFO_NUM_BYTES, 0);
    }
    else
    {
        data.resize(CHAR_V100_BASICINFO_NUM_BYTES, 0);
    }

    std::uint32_t value;
    if (getVersion() >= EnumCharVersion::v109)
    {
        value = 0;
        current_byte_offset = CHAR_V109_FILESIZE_BYTE_OFFSET;
        jsonValue = header[m_bJsonSerializedFormat ? "Filesize" : "filesize"];
        if (!jsonValue.isNull())
        {
            value = std::uint32_t(jsonValue.asInt64());
        }

        if (!updateBytes(current_byte_offset, CHAR_V109_FILESIZE_NUM_BYTES, value)) // pos 8 (1.09+ only), file's size
        {
            return false;
        }

        current_byte_offset = CHAR_V109_CHECKSUM_BYTE_OFFSET;
        jsonValue = header[m_bJsonSerializedFormat ? "Checksum" : "checksum"];
        if (!jsonValue.isNull())
        {
            value = std::uint32_t(m_bJsonSerializedFormat ? long(jsonValue.asInt64()) : long(std::stoul(jsonValue.asString(), nullptr, 16)));
        }

        if (!updateBytes(current_byte_offset, CHAR_V109_CHECKSUM_NUM_BYTES, value)) // pos 12 (1.09+ only), stores (possible) checksum
        {
            return false;
        }

        current_byte_offset = CHAR_V109_WEAPONSET_BYTE_OFFSET;
        jsonValue = m_bJsonSerializedFormat ? root["ActiveWeapon"] : header["active_arms"];
        if (!jsonValue.isNull())
        {
            value = std::uint32_t(jsonValue.asInt64());
        }

        if (!updateBytes(current_byte_offset, CHAR_V109_CHECKSUM_NUM_BYTES, value)) // pos 16 (1.09+, otherwise pos 26 uint16_t)
        {
            return false;
        }
    }

    jsonValue = m_bJsonSerializedFormat ? root["Name"] : header["name"];
    if (jsonValue.isNull())
    {
        return false;
    }

    // Check Name
    // Remove any invalid characters from the name
    std::string curName(jsonValue.asString());
    LocalizationHelpers::CheckCharName(curName, getVersion());
    Bs.Name.fill(0);
    std::memcpy(Bs.Name.data(), curName.c_str(), std::min(Bs.Name.size(), curName.length()));

    current_byte_offset = CHAR_V120_NAME_BYTE_OFFSET; // pos 267 (D2R 1.2+, pos 20 for 1.09 - 1.14d, otherwise pos 8), character's name
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_NAME_BYTE_OFFSET;
    }
    else if (getVersion() < EnumCharVersion::v120)
    {
        current_byte_offset = CHAR_V109_NAME_BYTE_OFFSET;
    }

    if (!updateBytes(current_byte_offset, Bs.Name.size(), (std::uint8_t*)Bs.Name.data()))
    {
        return false;
    }

    jsonValue = m_bJsonSerializedFormat ? root["ClassId"] : header["class_id"];
    if (jsonValue.isNull())
    {
        if (m_bJsonSerializedFormat)
        {
            return false;
        }

        jsonValue = header["class"];
        if (jsonValue.isNull())
        {
            return false;
        }

        std::string className = jsonValue.asString();
        if (!CharClassHelper::getEnumCharClassByName(className, Bs.Class))
        {
            if (!CharClassHelper::getEnumCharClassByIndex(className, Bs.Class))
            {
                return false;
            }
        }
    }
    else
    {
        value = std::uint8_t(jsonValue.asInt());
        if (value > std::uint8_t(NUM_OF_CLASSES))
        {
            return false;
        }
        Bs.Class = static_cast<EnumCharClass>(value);
    }

    Bs.Status = EnumCharStatus::NoDeaths;
    switch (Bs.Class)
    {
    case EnumCharClass::Druid:
    case EnumCharClass::Assassin:
        Bs.Status |= EnumCharStatus::Expansion;
        break;
    }

    std::array<std::uint8_t, NUM_OF_DIFFICULTY> startingAct;
    jsonValue = m_bJsonSerializedFormat ? root["Location"] : header["difficulty"];
    ApplyJsonStartingAct(jsonValue, startingAct);
    if (startingAct[0] != 0)
    {
        Bs.DifficultyLastPlayed = EnumDifficulty::Normal;
        Bs.StartingAct = static_cast<EnumAct>(startingAct[0] & ~0x80);
        if (Bs.StartingAct > EnumAct::IV)
        {
            Bs.Status |= EnumCharStatus::Expansion;
        }
    }
    else if (startingAct[1] != 0)
    {
        Bs.DifficultyLastPlayed = EnumDifficulty::Nightmare;
        Bs.StartingAct = static_cast<EnumAct>(startingAct[1] & ~0x80);
        if (Bs.StartingAct > EnumAct::IV)
        {
            Bs.Status |= EnumCharStatus::Expansion;
        }
    }
    else
    {
        Bs.DifficultyLastPlayed = EnumDifficulty::Hell;
        Bs.StartingAct = static_cast<EnumAct>(startingAct[2] & ~0x80);
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
        if (getVersion() < EnumCharVersion::v110)
        {
            Bs.Status &= ~EnumCharStatus::Ladder;

            if (getVersion() < EnumCharVersion::v107 || getVersion() == EnumCharVersion::v108)
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

    current_byte_offset = CHAR_V109_STAUTS_BYTE_OFFSET; // pos 36 (1.09+, otherwise, pos 24), character's status
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_STATUS_BYTE_OFFSET;
    }

    if (!updateBytes(current_byte_offset, CHAR_V100_STAUTS_NUM_BYTES, Bs.Status.bits()))
    {
        return false;
    }

    // progression can be derived later when looking at the which Difficulty level is complete
    Bs.Title = 0;
    jsonValue = m_bJsonSerializedFormat ? root["Progression"] : header["progression"];
    if (!jsonValue.isNull())
    {
        Bs.Title = std::min(std::uint8_t(jsonValue.asInt()), Bs.getGameCompleteTitle());
    }

    current_byte_offset = CHAR_V109_TITLE_BYTE_OFFSET; // pos 36 (1.09+, otherwise, pos 24), character's status
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_TITLE_BYTE_OFFSET;
    }

    if (!updateBytes(current_byte_offset, CHAR_V100_TITLE_NUM_BYTES, Bs.Title))
    {
        return false;
    }

    if (getVersion() < EnumCharVersion::v109)
    {
        value = 0;
        current_byte_offset = CHAR_V100_WEAPONSET_BYTE_OFFSET; // pos 26
        jsonValue = m_bJsonSerializedFormat ? root["ActiveWeapon"] : header["active_arms"];
        if (!jsonValue.isNull())
        {
            value = std::uint8_t(jsonValue.asInt64());
        }

        if (!updateBytes(current_byte_offset, CHAR_V100_WEAPONSET_NUM_BYTES, value)) // pos 26
        {
            return false;
        }

        current_byte_offset += 2;
        if (getVersion() < EnumCharVersion::v107)
        {
            if (!updateBytes(current_byte_offset, UNKNOWN_01C_v100.size(), (std::uint8_t*)UNKNOWN_01C_v100.data())) // pos 28
            {
                return false;
            }
        }
        else
        {
            if (!updateBytes(current_byte_offset, UNKNOWN_01C_v107.size(), (std::uint8_t*)UNKNOWN_01C_v107.data())) // pos 28
            {
                return false;
            }
        }
    }

    current_byte_offset = CHAR_V109_CLASS_BYTE_OFFSET; // pos 40 (1.09+, otherwise pos 34), character's class
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_CLASS_BYTE_OFFSET;
    }

    value = static_cast<std::underlying_type_t<EnumCharClass>>(Bs.Class);
    if (!updateBytes(current_byte_offset, CHAR_V100_CLASS_NUM_BYTES, value))
    {
        return false;
    }

    if (getVersion() >= EnumCharVersion::v109)
    {
        if (!updateBytes(0x29, UNKNOWN_029.size(), (std::uint8_t*)UNKNOWN_029.data())) // pos 41
        {
            return false;
        }
    }

    // Level can be retrieved from the attributes section
    jsonValue = m_bJsonSerializedFormat ? root["Level"] : header["level"];
    if (!jsonValue.isNull())
    {
        value = std::uint8_t(jsonValue.asInt());
    }

    current_byte_offset = CHAR_V109_DISPLAYLEVEL_BYTE_OFFSET; // pos 43 (1.09+, otherwise pos 36)
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_DISPLAYLEVEL_BYTE_OFFSET;
    }

    if (!updateBytes(current_byte_offset, CHAR_V100_DISPLAYLEVEL_NUM_BYTES, value))
    {
        return false;
    }

    if (!setDifficultyLastPlayedBytes(Bs.DifficultyLastPlayed, Bs.StartingAct))
    {
        return false;
    }

    auto menuAppearances = &data[(getVersion() >= EnumCharVersion::v109) ? CHAR_V109_APPEARANCES_BYTE_OFFSET : CHAR_V100_APPEARANCES_BYTE_OFFSET]; // pos 136 (1.09+, otherwise pos 38) Character menu appearance
    jsonValue = m_bJsonSerializedFormat ? root["Appearances"] : header["menu_appearance"];
    ApplyJsonAppearnces(jsonValue, menuAppearances);

    if (getVersion() >= EnumCharVersion::v100R)
    {
        auto menuD2RAppearances = &data[CHAR_v100R_APPEARANCES_BYTE_OFFSET]; // pos 219 (D2R only) Character menu appearance
        if (!jsonValue.isNull())
        {
            jsonValue = m_bJsonSerializedFormat ? jsonValue["D2R"] : jsonValue["d2r"];
            ApplyJsonD2RAppearnces(jsonValue, menuD2RAppearances);
        }
    }

    jsonValue = m_bJsonSerializedFormat ? root["AssignedSkills"] : header["assigned_skills"];
    if (getVersion() < EnumCharVersion::v109)
    {
        ApplyJsonAssignedSkills(jsonValue, &data[CHAR_V100_ASSIGNED_SKILLS_BYTE_OFFSET]);
    }
    else
    {
        ApplyJsonAssignedSkills(jsonValue, (std::uint32_t*)&data[CHAR_V109_ASSIGNED_SKILLS_BYTE_OFFSET]);
    }

    jsonValue = m_bJsonSerializedFormat ? root["LeftSkill"] : header["left_skill"];
    value = ApplyJsonSkill(jsonValue);
    current_byte_offset = CHAR_V109_LEFTSKILL_BYTE_OFFSET; 
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_LEFTSKILL_BYTE_OFFSET; // pos 36
        if (!updateBytes(current_byte_offset, CHAR_V100_DISPLAYLEVEL_NUM_BYTES, value))
        {
            return false;
        }
    }
    else
    {
        current_byte_offset = CHAR_V109_LEFTSKILL_BYTE_OFFSET; // pos 43
        if (!updateBytes(current_byte_offset, CHAR_V109_LEFTSKILL_NUM_BYTES, value))
        {
            return false;
        }
    }

    jsonValue = m_bJsonSerializedFormat ? root["RightSkill"] : header["right_skill"];
    value = ApplyJsonSkill(jsonValue);
    current_byte_offset = CHAR_V109_LEFTSKILL_BYTE_OFFSET;
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_RIGHTSKILL_BYTE_OFFSET; // pos 36
        if (!updateBytes(current_byte_offset, CHAR_V100_RIGHTSKILL_NUM_BYTES, value))
        {
            return false;
        }
    }
    else
    {
        current_byte_offset = CHAR_V109_RIGHTSKILL_BYTE_OFFSET; // pos 43
        if (!updateBytes(current_byte_offset, CHAR_V109_RIGHTSKILL_NUM_BYTES, value))
        {
            return false;
        }
    }

    value = 0;
    jsonValue = m_bJsonSerializedFormat ? root["MapId"] : header["map_id"];
    if (!jsonValue.isNull())
    {
        value = std::uint32_t(jsonValue.asInt64());
    }

    current_byte_offset = CHAR_V109_MAPID_BYTE_OFFSET; // pos 43 (1.09+, otherwise pos 36)
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_MAPID_BYTE_OFFSET;
    }

    if (!updateBytes(current_byte_offset, CHAR_V100_MAPID_NUM_BYTES, value))
    {
        return false;
    }

    if (getVersion() >= EnumCharVersion::v109)
    {
        value = 0;
        jsonValue = m_bJsonSerializedFormat ? root["Created"] : header["created"];
        if (!jsonValue.isNull())
        {
            value = std::uint32_t(jsonValue.asInt64());
        }

        if (!updateBytes(CHAR_V109_CREATED_BYTE_OFFSET, CHAR_V109_CREATED_NUM_BYTES, value))
        {
            return false;
        }

        value = 0;
        jsonValue = m_bJsonSerializedFormat ? root["LastPlayed"] : header["last_played"];
        if (!jsonValue.isNull())
        {
            value = std::uint32_t(jsonValue.asInt64());
        }

        if (!updateBytes(CHAR_V109_LASTPLAYED_BYTE_OFFSET, CHAR_V109_LASTPLAYED_NUM_BYTES, value))
        {
            return false;
        }

        if (!updateBytes(0x34, UNKNOWN_034.size(), (std::uint8_t*)UNKNOWN_034.data()))
        {
            return false;
        }

        jsonValue = m_bJsonSerializedFormat ? root["LeftSwapSkill"] : header["left_swap_skill"];
        value = ApplyJsonSkill(jsonValue);
        if (!updateBytes(CHAR_V109_LEFTSWAPSKILL_BYTE_OFFSET, CHAR_V109_LEFTSWAPSKILL_NUM_BYTES, value))
        {
            return false;
        }

        jsonValue = m_bJsonSerializedFormat ? root["RightSwapSkill"] : header["right_swap_skill"];
        value = ApplyJsonSkill(jsonValue);
        if (!updateBytes(CHAR_V109_RIGHTSWAPSKILL_BYTE_OFFSET, CHAR_V109_RIGHTSWAPSKILL_NUM_BYTES, value))
        {
            return false;
        }

        if (!Merc.readInfo(root, m_bJsonSerializedFormat))
        {
            return false;
        }

        if (getVersion() < EnumCharVersion::v100R)
        {
            if (!updateBytes(0x14B, UNKNOWN_14B.size(), (std::uint8_t*)UNKNOWN_14B.data()))
            {
                return false;
            }
        }
    }

    if (Bs.getStartingActTitle() > Bs.Title)
    {
        setTitleBytes(Bs.getStartingActTitle());
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readActs(std::FILE* charFile)
{
    return Acts.readActs(charFile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::readActs(const Json::Value& root)
{
    return Acts.readActs(root, m_bJsonSerializedFormat);
}
//---------------------------------------------------------------------------
bool d2ce::Character::readStats(std::FILE* charFile)
{
    if (Cs.readStats(charFile))
    {
        setDisplayLevelBytes((std::uint8_t)Cs.Cs.Level); // updates character's display level
        Cs.updateLifePointsEarned(Acts.getLifePointsEarned());
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readStats(const Json::Value& root)
{
    if (Cs.readStats(root, m_bJsonSerializedFormat))
    {
        setDisplayLevelBytes((std::uint8_t)Cs.Cs.Level); // updates character's display level
        Cs.updateLifePointsEarned(Acts.getLifePointsEarned());
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readItems(std::FILE* charFile)
{
    return m_items.readItems(*this, charFile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::readItems(const Json::Value& root)
{
    return m_items.readItems(root, m_bJsonSerializedFormat, *this);
}
//---------------------------------------------------------------------------
bool d2ce::Character::save(bool backup)
{
    if (backup)
    {
        // backup existing file
        auto now = std::chrono::system_clock::now();
        auto UTC = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::filesystem::path orig;
        std::filesystem::path p;
        if (is_json())
        {
            orig = m_jsonfilename;
            p = orig;
            auto ext = p.extension();
            ext += "." + std::to_string(UTC) + ".bak";
            p.replace_extension(ext);
        }
        else
        {
            orig = m_d2sfilename;
            p = orig;
            auto ext = "." + std::to_string(UTC) + ".bak";
            p.replace_extension(ext);
        }

        try
        {
            std::filesystem::copy_file(orig, p, std::filesystem::copy_options::overwrite_existing);
        }
        catch (std::filesystem::filesystem_error const&)
        {
        }
    }

    calculateChecksum();
    m_error_code.clear();

    if (!m_jsonfilename.empty())
    {
        // export the json again
        auto json = asJson(m_bJsonSerializedFormat, d2ce::Character::EnumCharSaveOp::NoSave);
        if (!json.empty())
        {
            std::FILE* jsonFile = NULL;
            _wfopen_s(&jsonFile, m_jsonfilename.wstring().c_str(), L"wb");
            std::rewind(jsonFile);
            std::fwrite(json.c_str(), json.size(), 1, jsonFile);
            std::fclose(jsonFile);
        }

        m_ftime = std::filesystem::last_write_time(m_jsonfilename);
        return true;
    }

    std::filesystem::path tempfilename;
    wchar_t name1[L_tmpnam_s];
    std::wstring utempfilename;
    errno_t err = _wtmpnam_s(name1, L_tmpnam_s);
    if (err == 0)
    {
        utempfilename = name1;
        tempfilename = utempfilename;
    }

    std::FILE* tempfile = NULL;
    _wfopen_s(&tempfile, utempfilename.c_str(), L"wb");

    std::rewind(tempfile);

    writeBasicInfo(tempfile);
    writeActs(tempfile);

    // From this point on, the location is variable
    writeStats(tempfile);

    // Write Character, Corpse, Mercenary and Golem items
    writeItems(tempfile);

    std::fclose(tempfile);

    // prepare to update the character file
    try
    {
        std::filesystem::remove(m_d2sfilename);
    }
    catch (std::filesystem::filesystem_error const&)
    {
    }

    try
    {
        // rename temp file to character file
        std::filesystem::rename(tempfilename, m_d2sfilename);
    }
    catch (std::filesystem::filesystem_error const&)
    {
        m_error_code = std::make_error_code(CharacterErrc::FileRenameError);
        return false;
    }

    m_ftime = std::filesystem::last_write_time(m_d2sfilename);

    // check to see if the m_d2sfilename needs to be changed
    // to match the character's name
    tempfilename = m_d2sfilename;
    std::filesystem::path p = m_d2sfilename;
    p.replace_extension();
    std::filesystem::path origFileNameBase = p;
    std::string tempname = utf8::utf16to8(p.filename().wstring());
    std::string utf8name = getNameAsString();

    // compare m_d2sfilename (w/o extension) to character's name
    if (_stricmp(tempname.c_str(), utf8name.c_str()) != 0)
    {
        std::filesystem::path fileNameBase = p.replace_filename(std::filesystem::u8path(utf8name));
        m_d2sfilename = fileNameBase;
        m_d2sfilename.replace_extension(".d2s");
        try
        {
            // rename temp file to character file
            std::filesystem::rename(tempfilename, m_d2sfilename);
        }
        catch (std::filesystem::filesystem_error const&)
        {
            m_error_code = std::make_error_code(CharacterErrc::FileRenameError);
            return false;
        }

        std::filesystem::path d2sfilename = m_d2sfilename;
        if (!open(d2sfilename))
        {
            return false;
        }

        // rename other files (don't error out if it fails)
        tempfilename = origFileNameBase;
        tempfilename.replace_extension(".key");
        if (std::filesystem::exists(tempfilename))
        {
            std::filesystem::path tempPath = fileNameBase;
            tempPath.replace_extension(".key");
            try
            {
                std::filesystem::rename(tempfilename, tempname);
            }
            catch (std::filesystem::filesystem_error const&)
            {
                m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                return false;
            }
        }

        tempfilename = origFileNameBase;
        tempfilename.replace_extension(".ma0");
        if (std::filesystem::exists(tempfilename))
        {
            std::filesystem::path tempPath = fileNameBase;
            tempPath.replace_extension(".ma0");
            try
            {
                std::filesystem::rename(tempfilename, tempname);
            }
            catch (std::filesystem::filesystem_error const&)
            {
                m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                return false;
            }
        }

        tempfilename = origFileNameBase;
        tempfilename.replace_extension(".ma1");
        if (std::filesystem::exists(tempfilename))
        {
            std::filesystem::path tempPath = fileNameBase;
            tempPath.replace_extension(".ma1");
            try
            {
                std::filesystem::rename(tempfilename, tempname);
            }
            catch (std::filesystem::filesystem_error const&)
            {
                m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                return false;
            }
        }

        tempfilename = origFileNameBase;
        tempfilename.replace_extension(".ma2");
        if (std::filesystem::exists(tempfilename))
        {
            std::filesystem::path tempPath = fileNameBase;
            tempPath.replace_extension(".ma2");
            try
            {
                std::filesystem::rename(tempfilename, tempname);
            }
            catch (std::filesystem::filesystem_error const&)
            {
                m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                return false;
            }
        }

        tempfilename = origFileNameBase;
        tempfilename.replace_extension(".ma3");
        if (std::filesystem::exists(tempfilename))
        {
            std::filesystem::path tempPath = fileNameBase;
            tempPath.replace_extension(".ma3");
            try
            {
                std::filesystem::rename(tempfilename, tempname);
            }
            catch (std::filesystem::filesystem_error const&)
            {
                m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                return false;
            }
        }

        tempfilename = origFileNameBase;
        tempfilename.replace_extension(".map");
        if (std::filesystem::exists(tempfilename))
        {
            std::filesystem::path tempPath = fileNameBase;
            tempPath.replace_extension(".map");
            try
            {
                std::filesystem::rename(tempfilename, tempname);
            }
            catch (std::filesystem::filesystem_error const&)
            {
                m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                return false;
            }
        }

        tempfilename = origFileNameBase;
        tempfilename.replace_extension(".ctl");
        if (std::filesystem::exists(tempfilename))
        {
            std::filesystem::path tempPath = fileNameBase;
            tempPath.replace_extension(".ctl");
            try
            {
                std::filesystem::rename(tempfilename, tempname);
            }
            catch (std::filesystem::filesystem_error const&)
            {
                m_error_code = std::make_error_code(CharacterErrc::AuxFileRenameError);
                return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::saveAsVersion(EnumCharVersion version, EnumCharSaveOp backup)
{
    if (!is_open())
    {
        return false;
    }

    std::filesystem::path path = is_json() ? m_jsonfilename : m_d2sfilename;
    return saveAsVersion(path, version, backup);
}
//---------------------------------------------------------------------------
bool d2ce::Character::saveAsVersion(const std::filesystem::path& path, EnumCharVersion version, EnumCharSaveOp saveOp)
{
    bool bIsJson = is_json();
    bool bBackup = false;
    bool bSave = true;
    switch (saveOp)
    {
    case EnumCharSaveOp::SaveWithBackup:
        bBackup = true;
        bSave = true;
        break;

    case EnumCharSaveOp::BackupOnly:
        bBackup = true;
        bSave = false;
        break;

    case EnumCharSaveOp::SaveOnly:
        bBackup = true;
        bSave = false;
        break;

    case EnumCharSaveOp::NoSave:
    default:
        bBackup = false;
        bSave = false;
        break;
    }

    if (bSave)
    {
        // first save any outstanding changes
        if (!save(bBackup))
        {
            return false;
        }
    }

    auto jsonStr = asJson(version, true, EnumCharSaveOp::NoSave);
    if (jsonStr.empty())
    {
        return false;
    }

    bool bOpenFile = false;
    auto p = path;
    if (bIsJson)
    {
        if (std::filesystem::exists(p))
        {
            if (p == m_jsonfilename)
            {
                bOpenFile = true;
                close();
            }

            if (bBackup)
            {
                // backup existing file
                auto now = std::chrono::system_clock::now();
                auto UTC = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
                std::filesystem::path orig;
                orig = path;
                auto ext = p.extension();
                ext += "." + std::to_string(UTC) + ".bak";
                p.replace_extension(ext);

                try
                {
                    std::filesystem::rename(orig, p);
                }
                catch (std::filesystem::filesystem_error const&)
                {
                }
            }
            else
            {
                // remove existing file
                try
                {
                    std::filesystem::remove(p);
                }
                catch (std::filesystem::filesystem_error const&)
                {
                }
            }
        }
    }

    std::FILE* jsonFile = NULL;
    if (bIsJson)
    {
        _wfopen_s(&jsonFile, path.wstring().c_str(), L"wb");
        std::rewind(jsonFile);
        std::fwrite(jsonStr.c_str(), jsonStr.size(), 1, jsonFile);
        std::fclose(jsonFile);
        if (bOpenFile)
        {
            return open(path, true);
        }
        return true;
    }

    if (std::filesystem::exists(p))
    {
        if (p == m_d2sfilename)
        {
            bOpenFile = true;
            close();
        }

        if (bBackup)
        {
            // backup existing file
            auto now = std::chrono::system_clock::now();
            auto UTC = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
            auto ext = "." + std::to_string(UTC) + ".bak";
            auto orig = p;
            p.replace_extension(ext);

            try
            {
                std::filesystem::rename(orig, p);
            }
            catch (std::filesystem::filesystem_error const&)
            {
            }
        }
        else
        {
            // remove existing file
            try
            {
                std::filesystem::remove(p);
            }
            catch (std::filesystem::filesystem_error const&)
            {
            }
        }
    }

    m_error_code.clear();
    wchar_t name1[L_tmpnam_s];
    errno_t err = _wtmpnam_s(name1, L_tmpnam_s);
    if (err != 0)
    {
        m_error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    _wfopen_s(&jsonFile, name1, L"wb");
    std::rewind(jsonFile);
    std::fwrite(jsonStr.c_str(), jsonStr.size(), 1, jsonFile);
    std::fflush(jsonFile);
    std::fclose(jsonFile);
    if (bOpenFile)
    {
        if (!openJson(name1))
        {
            return false;
        }

        // remove temp file
        try
        {
            std::filesystem::remove(name1);
        }
        catch (std::filesystem::filesystem_error const&)
        {
        }

        p = path;
        p.remove_filename();
        return saveAsD2s(p);
    }

    Character convert;
    if (!convert.openJson(name1))
    {
        m_error_code = convert.m_error_code;
        return false;
    }

    // remove temp file
    try
    {
        std::filesystem::remove(name1);
    }
    catch (std::filesystem::filesystem_error const&)
    {
    }

    p = path;
    p.remove_filename();
    if (!convert.saveAsD2s(p))
    {
        m_error_code = convert.m_error_code;
        return false;
    }

    convert.close();
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::saveAsD2s(EnumCharSaveOp saveOp)
{
    if (m_jsonfilename.empty())
    {
        return true;
    }

    std::filesystem::path p = m_jsonfilename;
    p.remove_filename();
    return saveAsD2s(p, saveOp);
}

bool d2ce::Character::saveAsD2s(const std::filesystem::path& path, EnumCharSaveOp saveOp)
{
    bool bBackup = false;
    bool bSave = true;
    switch (saveOp)
    {
    case EnumCharSaveOp::SaveWithBackup:
        bBackup = true;
        bSave = true;
        break;

    case EnumCharSaveOp::BackupOnly:
        bBackup = true;
        bSave = false;
        break;

    case EnumCharSaveOp::SaveOnly:
        bBackup = true;
        bSave = false;
        break;

    case EnumCharSaveOp::NoSave:
    default:
        bBackup = false;
        bSave = false;
        break;
    }

    if (bSave)
    {
        // first save any outstanding changes
        if (!save(bBackup))
        {
            return false;
        }
    }

    if (m_jsonfilename.empty())
    {
        return true;
    }

    m_jsonfilename.clear();

    // move d2s file to json file path
    m_d2sfilename = path / std::filesystem::u8path(getNameAsString());
    m_d2sfilename.replace_extension(".d2s");
    if (!save(bBackup))
    {
        return false;
    }

    std::filesystem::path d2sfilename = m_d2sfilename;
    if (!open(d2sfilename, false)) // checksum is calulated and written below
    {
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Character::writeBasicInfo(std::FILE* charFile) const
{
    std::rewind(charFile);
    std::fwrite(&data[0], data.size(), 1, charFile);
    std::fflush(charFile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeActs(std::FILE* charFile) const
{
    return Acts.writeActs(charFile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeStats(std::FILE* charFile) const
{
    return Cs.writeStats(charFile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeItems(std::FILE* charFile) const
{
    return m_items.writeItems(charFile, isExpansionCharacter(), hasMercenary());
}
//---------------------------------------------------------------------------
void d2ce::Character::headerAsJson(Json::Value& parent, EnumCharVersion version, bool bSerializedFormat) const
{
    if (version == getVersion())
    {
        headerAsJson(parent, bSerializedFormat);
        return;
    }

    if (bSerializedFormat)
    {
        Json::Value header;
        if (version >= EnumCharVersion::v100R)
        {
            const auto& txtReader = getTxtReader();
            if (!txtReader.GetModName().empty())
            {
                header["ModName"] = txtReader.GetModName();
            }
        }

        header["Magic"] = getHeaderBytes();
        header["Version"] = static_cast<std::underlying_type_t<EnumCharVersion>>(version);
        if ((version >= EnumCharVersion::v109) && (getVersion() >= EnumCharVersion::v109))
        {
            header["Filesize"] = getFileSize();
            header["Checksum"] = getChecksumBytes();
        }
        parent["Header"] = header;

        parent["ActiveWeapon"] = getWeaponSet();
        parent["Name"] = getNameAsString();

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
        parent["Level"] = std::uint16_t(getDisplayLevelBytes());
        if ((version >= EnumCharVersion::v109) && (getVersion() >= EnumCharVersion::v109))
        {
            parent["Created"] = getCreatedBytes();
            parent["LastPlayed"] = getLastPlayedBytes();
        }

        // assigned_skills
        Json::Value assignedSkills(Json::arrayValue);
        if (getVersion() < EnumCharVersion::v109)
        {
            auto assigned = &data[CHAR_V100_ASSIGNED_SKILLS_BYTE_OFFSET]; // pos 70
            std::uint32_t tempValue = 0;
            for(std::uint16_t i = 0; i < NUM_OF_SKILL_HOTKEYS; ++i)
            {
                tempValue = assigned[i];
                if (tempValue == MAXUINT8)
                {
                    tempValue = MAXUINT16;
                }

                Json::Value skill;
                skill["Id"] = tempValue;
                assignedSkills.append(skill);
            }
        }
        else
        {
            auto assigned = (std::uint32_t*)&data[CHAR_V109_ASSIGNED_SKILLS_BYTE_OFFSET]; // pos 56
            for (std::uint16_t i = 0; i < NUM_OF_SKILL_HOTKEYS; ++i)
            {
                Json::Value skill;
                skill["Id"] = assigned[i];
                assignedSkills.append(skill);
            }
        }
        parent["AssignedSkills"] = assignedSkills;

        {
            Json::Value skill;
            skill["Id"] = getLeftSkillBytes();
            parent["LeftSkill"] = skill;
        }

        {
            Json::Value skill;
            skill["Id"] = getRightSkillBytes();
            parent["RightSkill"] = skill;
        }
        if ((version >= EnumCharVersion::v109) && (getVersion() >= EnumCharVersion::v109))
        {
            {
                Json::Value skill;
                skill["Id"] = getLeftSwapSkillBytes();
                parent["LeftSwapSkill"] = skill;
            }
            {
                Json::Value skill;
                skill["Id"] = getRightSwapSkillBytes();
                parent["RightSwapSkill"] = skill;
            }
        }

        // Appearances
        static std::initializer_list<std::string> all_appearance_props = { "Head", "Torso", "Legs", "RightArm", "LeftArm", "RightHand", "LeftHand", "Shield",
            "Special1", "Special2",  "Special3", "Special4", "Special5", "Special6", "Special7", "Special8" };

        size_t idx = 0;
        Json::Value appearances;
        auto menuAppearances = &data[(getVersion() >= EnumCharVersion::v109) ? CHAR_V109_APPEARANCES_BYTE_OFFSET : CHAR_V100_APPEARANCES_BYTE_OFFSET]; // pos 136 (1.09+, otherwise pos 38) Character menu appearance
        for (const auto& prop : all_appearance_props)
        {
            Json::Value appearance;
            appearance["Graphic"] = std::uint16_t(menuAppearances[idx++]);
            appearance["Tint"] = std::uint16_t(menuAppearances[idx + 15]);
            appearances[prop] = appearance;
        }

        // D2R Appearances
        if ((version >= EnumCharVersion::v100R) && (getVersion() >= EnumCharVersion::v100R))
        {
            static std::initializer_list<std::string> all_d2r_appearance_props = { "RightHand", "LeftHand", "Torso", "Head" };

            auto menuD2RAppearances = &data[CHAR_v100R_APPEARANCES_BYTE_OFFSET]; // pos 219 (D2R only) Character menu appearance
            idx = 0;
            Json::Value d2rAppearances;
            std::uint8_t typecodeValue = 0;
            std::string typecode;
            for (const auto& prop : all_d2r_appearance_props)
            {
                Json::Value appearance;
                {
                    std::stringstream ss;
                    typecodeValue = menuD2RAppearances[idx++];
                    bool bNullFound = false;
                    if (typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }
                    typecode = ss.str();
                }

                appearance["Code"] = typecode;
                appearance["Tint"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Quality"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Id"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk1"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk2"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk3"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk4"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk5"] = std::uint16_t(menuD2RAppearances[idx++]);
                d2rAppearances[prop] = appearance;
            }

            appearances["D2R"] = d2rAppearances;
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
        parent["MapId"] = getMapIDBytes();

        if ((version >= EnumCharVersion::v109) && (getVersion() >= EnumCharVersion::v109))
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
        if (version >= EnumCharVersion::v100R)
        {
            const auto& txtReader = getTxtReader();
            if (!txtReader.GetModName().empty())
            {
                header["mod_name"] = txtReader.GetModName();
            }
        }

        {
            std::stringstream ss;
            ss << std::hex << getHeaderBytes();
            header["identifier"] = ss.str();
        }
        header["version"] = getVersionBytes();
        if ((version >= EnumCharVersion::v109) && (getVersion() >= EnumCharVersion::v109))
        {
            header["filesize"] = getFileSize();
            {
                std::stringstream ss;
                ss << std::hex << getChecksumBytes();
                header["checksum"] = ss.str();
            }
        }
        header["name"] = getNameAsString();

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
        header["active_arms"] = getWeaponSet();
        header["class"] = getClassName();
        header["class_id"] = std::uint16_t(getClass());
        header["level"] = std::uint16_t(getDisplayLevelBytes());
        if ((version >= EnumCharVersion::v109) && (getVersion() >= EnumCharVersion::v109))
        {
            header["created"] = getCreatedBytes();
            header["last_played"] = getLastPlayedBytes();
        }

        // assigned_skills
        size_t nullCount = 0; // no need to include nulls at the end of list
        std::uint32_t skillId = 0;
        Json::Value nullValue;
        Json::Value assignedSkills(Json::arrayValue);
        if (getVersion() < EnumCharVersion::v109)
        {
            auto assigned = &data[CHAR_V100_ASSIGNED_SKILLS_BYTE_OFFSET]; // pos 70
            for (std::uint16_t i = 0; i < NUM_OF_SKILL_HOTKEYS; ++i)
            {
                skillId = assigned[i];
                if (skillId >= MAXUINT8)
                {
                    ++nullCount;
                    continue;
                }

                while (nullCount > 0)
                {
                    assignedSkills.append(nullValue);
                    --nullCount;
                }

                assignedSkills.append(CharClassHelper::getSkillIndexById(std::uint16_t(skillId)));
            }
        }
        else
        {
            auto assigned = (std::uint32_t*)&data[CHAR_V109_ASSIGNED_SKILLS_BYTE_OFFSET]; // pos 56
            for (std::uint16_t i = 0; i < NUM_OF_SKILL_HOTKEYS; ++i)
            {
                skillId = assigned[i];
                if (skillId >= MAXUINT16)
                {
                    ++nullCount;
                    continue;
                }

                while (nullCount > 0)
                {
                    assignedSkills.append(nullValue);
                    --nullCount;
                }

                assignedSkills.append(CharClassHelper::getSkillIndexById(std::uint16_t(skillId)));
            }
        }
        header["assigned_skills"] = assignedSkills;
        header["left_skill"] = CharClassHelper::getSkillIndexById(std::uint16_t(getLeftSkillBytes()));
        header["right_skill"] = CharClassHelper::getSkillIndexById(std::uint16_t(getRightSkillBytes()));
        if ((version >= EnumCharVersion::v109) && (getVersion() >= EnumCharVersion::v109))
        {
            header["left_swap_skill"] = CharClassHelper::getSkillIndexById(std::uint16_t(getLeftSwapSkillBytes()));
            header["right_swap_skill"] = CharClassHelper::getSkillIndexById(std::uint16_t(getRightSwapSkillBytes()));
        }

        // Appearances
        static std::initializer_list<std::string> all_appearance_props = { "head", "torso", "legs", "right_arm", "left_arm", "right_hand", "left_hand", "shield",
            "special1", "special2",  "special3", "special4", "special5", "special6", "special7", "special8" };

        size_t idx = 0;
        Json::Value appearances;
        auto menuAppearances = &data[(getVersion() >= EnumCharVersion::v109) ? CHAR_V109_APPEARANCES_BYTE_OFFSET : CHAR_V100_APPEARANCES_BYTE_OFFSET]; // pos 136 (1.09+, otherwise pos 38) Character menu appearance
        for (const auto& prop : all_appearance_props)
        {
            Json::Value appearance;
            appearance["graphic"] = std::uint16_t(menuAppearances[idx++]);
            appearance["tint"] = std::uint16_t(menuAppearances[idx + 15]);
            appearances[prop] = appearance;
        }

        // D2R Appearances
        if ((version >= EnumCharVersion::v100R) && (getVersion() >= EnumCharVersion::v100R))
        {
            static std::initializer_list<std::string> all_d2r_appearance_props = { "right_hand", "left_hand", "torso", "head" };

            auto menuD2RAppearances = &data[CHAR_v100R_APPEARANCES_BYTE_OFFSET]; // pos 219 (D2R only) Character menu appearance
            idx = 0;
            Json::Value d2rAppearances; std::uint8_t typecodeValue = 0;
            std::string typecode;
            for (const auto& prop : all_d2r_appearance_props)
            {
                Json::Value appearance;
                {
                    std::stringstream ss;
                    typecodeValue = menuD2RAppearances[idx++];
                    bool bNullFound = false;
                    if (typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }
                    typecode = ss.str();
                }

                appearance["code"] = typecode;
                appearance["tint"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["quality"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["id"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk1"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk2"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk3"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk4"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk5"] = std::uint16_t(menuD2RAppearances[idx++]);
                d2rAppearances[prop] = appearance;
            }

            appearances["d2r"] = d2rAppearances;
        }

        header["menu_appearance"] = appearances;

        // StartingAct
        {

            Json::Value difficulty;
            if ((getVersion() < EnumCharVersion::v109))
            {
                std::array<std::uint8_t, NUM_OF_DIFFICULTY> startingAct = { 0 };
                startingAct[static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed)] = 0x80 | static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
                difficulty["Normal"] = std::uint16_t(startingAct[0]);
                difficulty["Nightmare"] = std::uint16_t(startingAct[1]);
                difficulty["Hell"] = std::uint16_t(startingAct[2]);
            }
            else
            {
                auto startingAct = &data[CHAR_V109_STARTINGACT_BYTE_OFFSET]; // pos 168 (normal, nightmare, hell; used in 1.09+ only)
                                                                             // four MSBs value always 8 (hex, i.e. 0x80)
                                                                             // four least significant bits = which act is character saved at
                difficulty["Normal"] = std::uint16_t(startingAct[0]);
                difficulty["Nightmare"] = std::uint16_t(startingAct[1]);
                difficulty["Hell"] = std::uint16_t(startingAct[2]);
            }

            header["difficulty"] = difficulty;
        }
        header["map_id"] = getMapIDBytes();

        if ((version >= EnumCharVersion::v109) && (getVersion() >= EnumCharVersion::v109))
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
void d2ce::Character::headerAsJson(Json::Value& parent, bool bSerializedFormat) const
{
    if (bSerializedFormat)
    {
        Json::Value header;
        if (getVersion() >= EnumCharVersion::v100R)
        {
            const auto& txtReader = getTxtReader();
            if (!txtReader.GetModName().empty())
            {
                header["Mod"] = txtReader.GetModName();
            }
        }

        header["Magic"] = getHeaderBytes();
        header["Version"] = getVersionBytes();
        if (getVersion() >= EnumCharVersion::v109)
        {
            header["Filesize"] = getFileSize();
            header["Checksum"] = getChecksumBytes();
        }
        parent["Header"] = header;

        parent["ActiveWeapon"] = getWeaponSet();
        parent["Name"] = getNameAsString();

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
        parent["Level"] = std::uint16_t(getDisplayLevelBytes());
        if (getVersion() >= EnumCharVersion::v109)
        {
            parent["Created"] = getCreatedBytes();
            parent["LastPlayed"] = getLastPlayedBytes();
        }

        // assigned_skills
        Json::Value assignedSkills(Json::arrayValue);
        if (getVersion() < EnumCharVersion::v109)
        {
            auto assigned = &data[CHAR_V100_ASSIGNED_SKILLS_BYTE_OFFSET]; // pos 70
            std::uint32_t tempValue = 0;

            for (std::uint16_t i = 0; i < NUM_OF_SKILL_HOTKEYS; ++i)
            {
                tempValue = assigned[i];
                if (tempValue >= MAXUINT8)
                {
                    tempValue = MAXUINT16;
                }

                Json::Value skill;
                skill["Id"] = tempValue;
                assignedSkills.append(skill);
            }
        }
        else
        {
            auto assigned = (std::uint32_t*)&data[CHAR_V109_ASSIGNED_SKILLS_BYTE_OFFSET]; // pos 56
            for (std::uint16_t i = 0; i < NUM_OF_SKILL_HOTKEYS; ++i)
            {
                Json::Value skill;
                skill["Id"] = assigned[i];
                assignedSkills.append(skill);
            }
        }
        parent["AssignedSkills"] = assignedSkills;

        {
            Json::Value skill;
            skill["Id"] = getLeftSkillBytes();
            parent["LeftSkill"] = skill;
        }

        {
            Json::Value skill;
            skill["Id"] = getRightSkillBytes();
            parent["RightSkill"] = skill;
        }
        if (getVersion() >= EnumCharVersion::v109)
        {
            {
                Json::Value skill;
                skill["Id"] = getLeftSwapSkillBytes();
                parent["LeftSwapSkill"] = skill;
            }
            {
                Json::Value skill;
                skill["Id"] = getRightSwapSkillBytes();
                parent["RightSwapSkill"] = skill;
            }
        }

        // Appearances
        static std::initializer_list<std::string> all_appearance_props = { "Head", "Torso", "Legs", "RightArm", "LeftArm", "RightHand", "LeftHand", "Shield",
            "Special1", "Special2",  "Special3", "Special4", "Special5", "Special6", "Special7", "Special8" };

        size_t idx = 0;
        Json::Value appearances;
        auto menuAppearances = &data[(getVersion() >= EnumCharVersion::v109) ? CHAR_V109_APPEARANCES_BYTE_OFFSET : CHAR_V100_APPEARANCES_BYTE_OFFSET]; // pos 136 (1.09+, otherwise pos 38) Character menu appearance
        for (const auto& prop : all_appearance_props)
        {
            Json::Value appearance;
            appearance["Graphic"] = std::uint16_t(menuAppearances[idx++]);
            appearance["Tint"] = std::uint16_t(menuAppearances[idx + 15]);
            appearances[prop] = appearance;
        }

        // D2R Appearances
        if (getVersion() >= EnumCharVersion::v100R)
        {
            static std::initializer_list<std::string> all_d2r_appearance_props = { "RightHand", "LeftHand", "Torso", "Head" };


            auto menuD2RAppearances = &data[CHAR_v100R_APPEARANCES_BYTE_OFFSET]; // pos 219 (D2R only) Character menu appearance
            idx = 0;
            Json::Value d2rAppearances;
            std::uint8_t typecodeValue = 0;
            std::string typecode;
            for (const auto& prop : all_d2r_appearance_props)
            {
                Json::Value appearance;
                {
                    std::stringstream ss;
                    typecodeValue = menuD2RAppearances[idx++];
                    bool bNullFound = false;
                    if (typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }
                    typecode = ss.str();
                }
                
                appearance["Code"] = typecode;
                appearance["Tint"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Quality"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Id"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk1"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk2"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk3"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk4"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["Unk5"] = std::uint16_t(menuD2RAppearances[idx++]);
                d2rAppearances[prop] = appearance;
            }

            appearances["D2R"] = d2rAppearances;
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
        parent["MapId"] = getMapIDBytes();

        if (getVersion() >= EnumCharVersion::v109)
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
        if (getVersion() >= EnumCharVersion::v100R)
        {
            const auto& txtReader = getTxtReader();
            if (!txtReader.GetModName().empty())
            {
                header["mod_name"] = txtReader.GetModName();
            }
        }

        {
            std::stringstream ss;
            ss << std::hex << getHeaderBytes();
            header["identifier"] = ss.str();
        }
        header["version"] = getVersionBytes();
        if (getVersion() >= EnumCharVersion::v109)
        {
            header["filesize"] = getFileSize();
            {
                std::stringstream ss;
                ss << std::hex << getChecksumBytes();
                header["checksum"] = ss.str();
            }
        }
        header["name"] = getNameAsString();

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
        header["active_arms"] = getWeaponSet();
        header["class"] = getClassName();
        header["class_id"] = std::uint16_t(getClass());
        header["level"] = std::uint16_t(getDisplayLevelBytes());
        if (getVersion() >= EnumCharVersion::v109)
        {
            header["created"] = getCreatedBytes();
            header["last_played"] = getLastPlayedBytes();
        }

        // assigned_skills
        size_t nullCount = 0; // no need to include nulls at the end of list
        std::uint32_t skillId = 0;
        Json::Value nullValue;
        Json::Value assignedSkills(Json::arrayValue);
        if (getVersion() < EnumCharVersion::v109)
        {
            auto assigned = &data[CHAR_V100_ASSIGNED_SKILLS_BYTE_OFFSET]; // pos 70
            for (std::uint16_t i = 0; i < NUM_OF_SKILL_HOTKEYS; ++i)
            {
                skillId = assigned[i];
                if (skillId >= MAXUINT8)
                {
                    ++nullCount;
                    continue;
                }

                while (nullCount > 0)
                {
                    assignedSkills.append(nullValue);
                    --nullCount;
                }

                assignedSkills.append(CharClassHelper::getSkillIndexById(std::uint16_t(skillId)));
            }
        }
        else
        {
            auto assigned = (std::uint32_t*)&data[CHAR_V109_ASSIGNED_SKILLS_BYTE_OFFSET]; // pos 56
            for (std::uint16_t i = 0; i < NUM_OF_SKILL_HOTKEYS; ++i)
            {
                skillId = assigned[i];
                if (skillId >= MAXUINT16)
                {
                    ++nullCount;
                    continue;
                }

                while (nullCount > 0)
                {
                    assignedSkills.append(nullValue);
                    --nullCount;
                }

                assignedSkills.append(CharClassHelper::getSkillIndexById(std::uint16_t(skillId)));
            }
        }
        header["assigned_skills"] = assignedSkills;
        header["left_skill"] = CharClassHelper::getSkillIndexById(std::uint16_t(getLeftSkillBytes()));
        header["right_skill"] = CharClassHelper::getSkillIndexById(std::uint16_t(getRightSkillBytes()));
        if (getVersion() >= EnumCharVersion::v109)
        {
            header["left_swap_skill"] = CharClassHelper::getSkillIndexById(std::uint16_t(getLeftSwapSkillBytes()));
            header["right_swap_skill"] = CharClassHelper::getSkillIndexById(std::uint16_t(getRightSwapSkillBytes()));
        }

        // Appearances
        static std::initializer_list<std::string> all_appearance_props = { "head", "torso", "legs", "right_arm", "left_arm", "right_hand", "left_hand", "shield",
            "special1", "special2",  "special3", "special4", "special5", "special6", "special7", "special8" };

        size_t idx = 0;
        Json::Value appearances;
        auto menuAppearances = &data[(getVersion() >= EnumCharVersion::v109) ? CHAR_V109_APPEARANCES_BYTE_OFFSET : CHAR_V100_APPEARANCES_BYTE_OFFSET]; // pos 136 (1.09+, otherwise pos 38) Character menu appearance
        for (const auto& prop : all_appearance_props)
        {
            Json::Value appearance;
            appearance["graphic"] = std::uint16_t(menuAppearances[idx++]);
            appearance["tint"] = std::uint16_t(menuAppearances[idx + 15]);
            appearances[prop] = appearance;
        }

        // D2R Appearances
        if (getVersion() >= EnumCharVersion::v100R)
        {
            static std::initializer_list<std::string> all_d2r_appearance_props = { "right_hand", "left_hand", "torso", "head" };

            auto menuD2RAppearances = &data[CHAR_v100R_APPEARANCES_BYTE_OFFSET]; // pos 219 (D2R only) Character menu appearance
            idx = 0;
            Json::Value d2rAppearances; std::uint8_t typecodeValue = 0;
            std::string typecode;
            for (const auto& prop : all_d2r_appearance_props)
            {
                Json::Value appearance;
                {
                    std::stringstream ss;
                    typecodeValue = menuD2RAppearances[idx++];
                    bool bNullFound = false;
                    if (typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }

                    typecodeValue = menuD2RAppearances[idx++];
                    if (!bNullFound && typecodeValue > 0)
                    {
                        ss << (char)typecodeValue;
                    }
                    else
                    {
                        bNullFound = true;
                    }
                    typecode = ss.str();
                }

                appearance["code"] = typecode;
                appearance["tint"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["quality"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["id"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk1"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk2"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk3"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk4"] = std::uint16_t(menuD2RAppearances[idx++]);
                appearance["unk5"] = std::uint16_t(menuD2RAppearances[idx++]);
                d2rAppearances[prop] = appearance;
            }

            appearances["d2r"] = d2rAppearances;
        }

        header["menu_appearance"] = appearances;

        // StartingAct
        {
            Json::Value difficulty;
            if ((getVersion() < EnumCharVersion::v109))
            {
                std::array<std::uint8_t, NUM_OF_DIFFICULTY> startingAct = { 0 };
                startingAct[static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed)] = 0x80 | static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
                difficulty["Normal"] = std::uint16_t(startingAct[0]);
                difficulty["Nightmare"] = std::uint16_t(startingAct[1]);
                difficulty["Hell"] = std::uint16_t(startingAct[2]);
            }
            else
            {
                auto startingAct = &data[CHAR_V109_STARTINGACT_BYTE_OFFSET]; // pos 168 (normal, nightmare, hell; used in 1.09+ only)
                                                                             // four MSBs value always 8 (hex, i.e. 0x80)
                                                                             // four least significant bits = which act is character saved at
                difficulty["Normal"] = std::uint16_t(startingAct[0]);
                difficulty["Nightmare"] = std::uint16_t(startingAct[1]);
                difficulty["Hell"] = std::uint16_t(startingAct[2]);
            }
            header["difficulty"] = difficulty;
        }
        header["map_id"] = getMapIDBytes();

        if (getVersion() >= EnumCharVersion::v109)
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
        setTitleBytes(Bs.getStartingActTitle());
    }

    auto progression = getTitleDifficulty();
    if (progression < EnumDifficulty::Hell)
    {
        if (!Acts.getActYetToStart(EnumDifficulty::Hell, EnumAct::I))
        {
            if (progression < EnumDifficulty::Hell)
            {
                setTitleBytes(std::uint8_t(Bs.getNumActs() * static_cast<std::underlying_type_t<EnumDifficulty>>(EnumDifficulty::Hell) + static_cast<std::underlying_type_t<EnumCharVersion>>(Bs.StartingAct)));
                progression = EnumDifficulty::Hell;
            }
        }
        else if (!Acts.getActYetToStart(EnumDifficulty::Nightmare, EnumAct::I))
        {
            if (progression < EnumDifficulty::Nightmare)
            {
                setTitleBytes(std::uint8_t(Bs.getNumActs() + static_cast<std::underlying_type_t<EnumCharVersion>>(Bs.StartingAct)));
                progression = EnumDifficulty::Nightmare;
            }
        }
    }

    if (Bs.DifficultyLastPlayed > progression)
    {
        // Not able to allow this state
        progression = Bs.DifficultyLastPlayed;
        setTitleBytes(Bs.getStartingActTitle());
    }

    Acts.validateActs();
}
//---------------------------------------------------------------------------
void d2ce::Character::close()
{
    m_d2sfilename.clear();
    m_jsonfilename.clear();
    data.clear();

    initialize();
}
//---------------------------------------------------------------------------
const std::filesystem::path& d2ce::Character::getPath() const
{
    return m_jsonfilename.empty() ? m_d2sfilename : m_jsonfilename;
}
//---------------------------------------------------------------------------
bool d2ce::Character::hasBeenModifiedSinceLoad() const
{
    if (!is_open())
    {
        return false;
    }

    auto ftime = std::filesystem::last_write_time(getPath());
    return (ftime > m_ftime) ? true : false;
}
//---------------------------------------------------------------------------
std::string d2ce::Character::asJson(EnumCharVersion version, bool bSerializedFormat, EnumCharSaveOp saveOp)
{
    if (version == getVersion())
    {
        return asJson(bSerializedFormat, saveOp);
    }

    bool bBackup = false;
    bool bSave = true;
    switch (saveOp)
    {
    case EnumCharSaveOp::SaveWithBackup:
        bBackup = true;
        bSave = true;
        break;

    case EnumCharSaveOp::BackupOnly:
        bBackup = true;
        bSave = false;
        break;

    case EnumCharSaveOp::SaveOnly:
        bBackup = true;
        bSave = false;
        break;

    case EnumCharSaveOp::NoSave:
    default:
        bBackup = false;
        bSave = false;
        break;
    }

    if (bSave)
    {
        // first save any outstanding changes
        save(bBackup);
    }

    Json::Value root;
    headerAsJson(root, version, bSerializedFormat);
    Cs.asJson(root, bSerializedFormat);
    m_items.asJson(root, getLevel(), version, bSerializedFormat);

    Json::StreamWriterBuilder builder;
    builder["indentation"] = jsonIndentStr;
    builder["enableYAMLCompatibility"] = true;
    return Json::writeString(builder, root);
}
//---------------------------------------------------------------------------
void d2ce::Character::initVersion()
{
    std::uint32_t version = readBytes(CHAR_V100_VERSION_BYTE_OFFSET, CHAR_V100_VERSION_NUM_BYTES); // pos 4 in file, character file version
    if (version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v107))
    {
        Bs.Version = EnumCharVersion::v100;
        return;
    }

    if (version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v108))
    {
        Bs.Version = EnumCharVersion::v107;
        return;
    }

    if (version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v109))
    {
        Bs.Version = EnumCharVersion::v108;
        return;
    }

    if (version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v110))
    {
        Bs.Version = EnumCharVersion::v109;
        return;
    }

    if (version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v100R))
    {
        Bs.Version = EnumCharVersion::v110;
        return;
    }

    if (version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v120))
    {
        Bs.Version = EnumCharVersion::v100R;
        return;
    }

    if (version < static_cast<std::underlying_type_t<EnumCharVersion>>(EnumCharVersion::v140))
    {
        Bs.Version = EnumCharVersion::v120;
        return;
    }

    Bs.Version = EnumCharVersion::v140;
}
//---------------------------------------------------------------------------
void d2ce::Character::checkForD2RMod()
{
    // D2R games folder
    static auto d2rGamesFolder = GetD2RGamesFolder();
    if (d2rGamesFolder.empty())
    {
        setTxtReader(getDefaultTxtReader());
        return;
    }

    if (getVersion() < EnumCharVersion::v100R)
    {
        setTxtReader(getDefaultTxtReader());
        return;
    }

    // check if we are a sub path
    if (m_d2sfilename.empty())
    {
        setTxtReader(getDefaultTxtReader());
        return;
    }

    auto rel = std::filesystem::relative(m_d2sfilename.parent_path(), d2rGamesFolder);
    if (rel.empty() || rel.native()[0] == '.')
    {
        setTxtReader(getDefaultTxtReader());
        return;
    }

    // we are a subpath, get Mod name
    auto modName = m_d2sfilename.parent_path().filename().u8string();
    setTxtReader(getDefaultTxtReader(modName));
}
//---------------------------------------------------------------------------
std::string d2ce::Character::asJson(bool bSerializedFormat, EnumCharSaveOp saveOp)
{
    bool bBackup = false;
    bool bSave = true;
    switch (saveOp)
    {
    case EnumCharSaveOp::SaveWithBackup:
        bBackup = true;
        bSave = true;
        break;

    case EnumCharSaveOp::BackupOnly:
        bBackup = true;
        bSave = false;
        break;

    case EnumCharSaveOp::SaveOnly:
        bBackup = true;
        bSave = false;
        break;

    case EnumCharSaveOp::NoSave:
    default:
        bBackup = false;
        bSave = false;
        break;
    }

    if (bSave)
    {
        // first save any outstanding changes
        save(bBackup);
    }

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
void d2ce::Character::setDefaultTxtReader()
{
    setTxtReader(getDefaultTxtReader());
}
//---------------------------------------------------------------------------
void d2ce::Character::setTxtReader(const d2ce::ITxtReader& txtReader)
{
    if (!ItemHelpers::isTxtReaderInitialized() || (&txtReader != &ItemHelpers::getTxtReader()))
    {
        ItemHelpers::setTxtReader(txtReader);
        Cs.setTxtReader();
        Merc.setTxtReader();
    }
}
//---------------------------------------------------------------------------
const d2ce::ITxtReader& d2ce::Character::getTxtReader() const
{
    return ItemHelpers::getTxtReader();
}
//---------------------------------------------------------------------------
const std::string& d2ce::Character::getLanguage() const
{
    return ItemHelpers::getLanguage();
}
//---------------------------------------------------------------------------
const std::string& d2ce::Character::setLanguage(const std::string& lang) const
{
    return ItemHelpers::setLanguage(lang);
}
//---------------------------------------------------------------------------
bool d2ce::Character::is_open() const
{
    return !data.empty() && !getPath().empty();
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
    if (getVersion() < EnumCharVersion::v109)
    {
        return std::uint32_t(getByteSize());
    }

    return readBytes(CHAR_V109_FILESIZE_BYTE_OFFSET, CHAR_V109_FILESIZE_NUM_BYTES); // pos 8 (1.09+ only), file's size
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getByteSize() const
{
    size_t byteSize = getItemsStartLocation();
    if (byteSize == 0)
    {
        return byteSize;
    }

    byteSize += m_items.getByteSize();
    return byteSize;
}
//---------------------------------------------------------------------------
bool d2ce::Character::hasMercenary() const
{
    return Merc.isHired();
}
//---------------------------------------------------------------------------
d2ce::Mercenary& d2ce::Character::getMercenaryInfo() const
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
const std::list<d2ce::Item>& d2ce::Character::getGolemItem() const
{
    return m_items.getGolemItem();
}
//---------------------------------------------------------------------------
void d2ce::Character::fillBasicStats(BasicStats& bs) const
{
    bs.Version = getVersion();
    bs.Name.fill(0);
    bs.Status = EnumCharStatus::NoDeaths;
    bs.Title = 0;
    bs.Class = EnumCharClass::Amazon;
    if (data.empty())
    {
        return;
    }

    size_t current_byte_offset = CHAR_V120_NAME_BYTE_OFFSET; // pos 267 (D2R 1.2+, pos 20 for 1.09 - 1.14d, otherwise pos 8), character's name
    if (bs.Version < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_NAME_BYTE_OFFSET;
    }
    else if (bs.Version < EnumCharVersion::v120)
    {
        current_byte_offset = CHAR_V109_NAME_BYTE_OFFSET;
    }

    // Name is UTF-8 so should all be valid
    std::memcpy(bs.Name.data(), &data[current_byte_offset], bs.Name.size());
    if (bs.Version < EnumCharVersion::v120)
    {
        bs.Name[15] = 0; // must be zero
    }
    else
    {
        // check for valid UTF-8
        try
        {
            auto test = utf8::utf8to16(LocalizationHelpers::ConvertCharNameToString(bs.Name, bs.Version));
        }
        catch (std::exception)
        {
            // invalid name, use filename to replace it
            std::filesystem::path p = m_d2sfilename;
            p.replace_extension();
            std::filesystem::path origFileNameBase = p;
            std::string tempname = utf8::utf16to8(p.filename().wstring());
            LocalizationHelpers::CheckCharName(tempname, bs.Version);
            bs.Name.fill(0);
            std::memcpy(bs.Name.data(), tempname.c_str(), std::min(tempname.size(), bs.Name.size()));
        }
    }

    current_byte_offset = CHAR_V109_STAUTS_BYTE_OFFSET; // pos 36 (1.09+, otherwise, pos 24), character's status
    if (bs.Version < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_STATUS_BYTE_OFFSET;
    }
    std::uint8_t value = std::uint8_t(readBytes(current_byte_offset, CHAR_V100_STAUTS_NUM_BYTES));
    bs.Status = static_cast<EnumCharStatus>(value);

    // ladder is for 1.10 or higher
    auto oldStatus = bs.Status;
    if (bs.Version < EnumCharVersion::v110)
    {
        bs.Status &= ~EnumCharStatus::Ladder;

        if (bs.Version < EnumCharVersion::v107 || bs.Version == EnumCharVersion::v108)
        {
            // expansion not supported
            bs.Status &= ~EnumCharStatus::Expansion;
        }

        if (oldStatus != bs.Status)
        {
            const_cast<Character*>(this)->setStatusBytes(bs.Status.bits());
        }
    }

    current_byte_offset = CHAR_V109_TITLE_BYTE_OFFSET; // pos 37 (1.09+, otherwise pos 25), character's title
    if (bs.Version < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_TITLE_BYTE_OFFSET;
    }
    bs.Title = std::uint8_t(readBytes(current_byte_offset, CHAR_V100_TITLE_NUM_BYTES));
    auto oldTitle = bs.Title;
    bs.Title = std::min(bs.Title, bs.getGameCompleteTitle());
    if (oldTitle != bs.Title)
    {
        const_cast<Character*>(this)->setTitleBytes(bs.Title);
    }

    current_byte_offset = CHAR_V109_CLASS_BYTE_OFFSET; // pos 40 (1.09+, otherwise pos 34), character's class
    if (bs.Version < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_CLASS_BYTE_OFFSET;
    }
    value = std::uint8_t(readBytes(current_byte_offset, CHAR_V100_CLASS_NUM_BYTES));
    bs.Class = static_cast<EnumCharClass>(value);

    if (bs.Version < EnumCharVersion::v109)
    {
        value = std::uint8_t(readBytes(CHAR_V100_STARTINGACT_BYTE_OFFSET, CHAR_V100_STARTINGACT_NUM_BYTES));
        bs.DifficultyLastPlayed = static_cast<EnumDifficulty>(std::min(std::uint32_t(value & 0x0F), NUM_OF_DIFFICULTY - 1));
        bs.StartingAct = static_cast<EnumAct>(value >> 4);
    }
    else
    {
        auto startingAct = &data[CHAR_V109_STARTINGACT_BYTE_OFFSET]; // pos 168 (normal, nightmare, hell; used in 1.09+ only)
                                                                     // four MSBs value always 8 (hex, i.e. 0x80)
                                                                     // four least significant bits = which act is character saved at
        if (startingAct[0] != 0)
        {
            bs.DifficultyLastPlayed = EnumDifficulty::Normal;
            bs.StartingAct = static_cast<EnumAct>(startingAct[0] & ~0x80);
        }
        else if (startingAct[1] != 0)
        {
            bs.DifficultyLastPlayed = EnumDifficulty::Nightmare;
            bs.StartingAct = static_cast<EnumAct>(startingAct[1] & ~0x80);
        }
        else
        {
            bs.DifficultyLastPlayed = EnumDifficulty::Hell;
            bs.StartingAct = static_cast<EnumAct>(startingAct[2] & ~0x80);
        }
    }

    if (bs.getStartingActTitle() > bs.Title)
    {
        bs.Title = bs.getStartingActTitle();
        const_cast<Character*>(this)->setTitleBytes(bs.Title);
    }
}
//---------------------------------------------------------------------------
void d2ce::Character::fillCharacterStats(CharStats& cs) const
{
    Cs.fillCharacterStats(cs);
}
//---------------------------------------------------------------------------
void d2ce::Character::fillDisplayedCharacterStats(CharStats& cs) const
{
    Cs.fillDisplayedCharacterStats(cs);
}
//---------------------------------------------------------------------------
void d2ce::Character::updateBasicStats(BasicStats& bs)
{
    BasicStats oldBs;
    std::memcpy(&oldBs, &Bs, sizeof(BasicStats));

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
            switch (oldBs.Class)
            {
            case EnumCharClass::Druid:
            case EnumCharClass::Assassin:
                bs.Class = EnumCharClass::Amazon;
                break;
            default:
                bs.Class = oldBs.Class;
                break;
            }
            break;
        }
    }

    // Check Name
    // Remove any invalid characters from the name
    std::string curName = LocalizationHelpers::ConvertCharNameToString(bs.Name, bs.Version);
    LocalizationHelpers::CheckCharName(curName, bs.Version);
    bs.Name.fill(0);
    std::memcpy(bs.Name.data(), curName.c_str(), std::min(bs.Name.size(), curName.length()));

    // Check Title
    if (bs.isExpansionCharacter() != oldBs.isExpansionCharacter())
    {
        auto oldNumActs = oldBs.getNumActs();
        auto oldTitleDiff = oldBs.Title / oldNumActs;
        auto oldTitleAct = oldBs.Title % oldNumActs;

        auto newNumActs = bs.getNumActs();
        auto newTitleDiff = bs.Title / newNumActs;
        auto newTitleAct = bs.Title % newNumActs;
        if (bs.Title == oldBs.Title)
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

    if (&bs != &Bs)
    {
        std::memcpy(&Bs, &bs, sizeof(BasicStats));
    }

    size_t current_byte_offset = CHAR_V120_NAME_BYTE_OFFSET; // pos 267 (D2R 1.2+, pos 20 for 1.09 - 1.14d, otherwise pos 8), character's name
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_NAME_BYTE_OFFSET;
    }
    else if (getVersion() < EnumCharVersion::v120)
    {
        current_byte_offset = CHAR_V109_NAME_BYTE_OFFSET;
    }

    updateBytes(current_byte_offset, Bs.Name.size(), (std::uint8_t*)Bs.Name.data());

    current_byte_offset = CHAR_V109_STAUTS_BYTE_OFFSET; // pos 36 (1.09+, otherwise, pos 24), character's status
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_STATUS_BYTE_OFFSET;
    }
    std::uint8_t value = Bs.Status.bits();
    updateBytes(current_byte_offset, CHAR_V100_STAUTS_NUM_BYTES, value);

    current_byte_offset = CHAR_V109_TITLE_BYTE_OFFSET; // pos 37 (1.09+, otherwise pos 25), character's title
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_TITLE_BYTE_OFFSET;
    }
    updateBytes(current_byte_offset, CHAR_V100_TITLE_NUM_BYTES, Bs.Title);

    current_byte_offset = CHAR_V109_CLASS_BYTE_OFFSET; // pos 40 (1.09+, otherwise pos 34), character's class
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_CLASS_BYTE_OFFSET;
    }
    value = static_cast<std::underlying_type_t<EnumCharClass>>(Bs.Class);
    updateBytes(current_byte_offset, CHAR_V100_CLASS_NUM_BYTES, value);

    if (bs.Version < EnumCharVersion::v109)
    {
        value = static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
        value <<= 4;
        value |= (static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed) & 0x0F);
        updateBytes(CHAR_V100_STARTINGACT_BYTE_OFFSET, CHAR_V100_STARTINGACT_NUM_BYTES, value);
    }
    else
    {
        auto startingAct = &data[CHAR_V109_STARTINGACT_BYTE_OFFSET];
        std::memset(startingAct, 0, CHAR_V109_STARTINGACT_NUM_BYTES);
        startingAct[static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed)] = 0x80 | static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
    }

    if (oldBs.Class != Bs.Class)
    {
        // classed changed
        Cs.updateClass();
    }

    Acts.validateActs();

    if ((oldBs.Title < Bs.Title) || (oldBs.Title == Bs.Title) && (oldBs.StartingAct < Bs.StartingAct))
    {
        // check minimum recommended level
        auto minLevel = getRecommendedLevel();
        if (minLevel > getLevel())
        {
            CharStats cs;
            Cs.fillCharacterStats(cs);
            cs.Level = minLevel;
            updateCharacterStats(cs);
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::Character::updateCharacterStats(CharStats& cs)
{
    auto oldLevel = Cs.getLevel();
    Cs.updateCharacterStats(cs);
    setDisplayLevelBytes((std::uint8_t)Cs.getLevel()); // updates character's display level
    Cs.updatePointsEarned(Acts.getLifePointsEarned(), Acts.getStatPointsEarned(), Acts.getSkillPointsEarned());

    // update values sent in to reflect any updates made
    Cs.fillCharacterStats(cs);

    if (oldLevel > Cs.getLevel())
    {
        // Check Stat points and skill points
        bool bChanged = false;
        std::uint32_t totalStatPoints = getTotalStatPoints();
        std::uint32_t earnedStatPoints = getTotalStartStatPoints() + getStatPointsEarned();
        if (totalStatPoints > earnedStatPoints)
        {
            bChanged = true;
            auto diff = totalStatPoints - earnedStatPoints;
            if (cs.StatsLeft <= diff)
            {
                cs.StatsLeft = 0;
            }
            else
            {
                cs.StatsLeft -= diff;
            }
        }

        std::uint32_t totalPoints = getTotalSkillPoints();
        std::uint32_t earnedPoints = getSkillPointsEarned();
        if (totalPoints > earnedPoints)
        {
            bChanged = true;
            auto diff = totalPoints - earnedPoints;
            if (cs.SkillChoices <= diff)
            {
                cs.SkillChoices = 0;
            }
            else
            {
                cs.SkillChoices -= diff;
            }
        }

        if (bChanged)
        {
            Cs.updateCharacterStats(cs);
            Cs.updatePointsEarned(Acts.getLifePointsEarned(), Acts.getStatPointsEarned(), Acts.getSkillPointsEarned());

            // update values sent in to reflect any updates made
            Cs.fillCharacterStats(cs);
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::Character::resetStats()
{
    Cs.resetStats(Acts.getLifePointsEarned(), Acts.getStatPointsEarned(), Acts.getSkillPointsEarned());
}
//---------------------------------------------------------------------------
d2ce::EnumCharVersion d2ce::Character::getVersion() const
{
    return Bs.Version;
}
//---------------------------------------------------------------------------
const std::array<char, d2ce::NAME_LENGTH>& d2ce::Character::getName() const
{
    return Bs.Name;
}
//---------------------------------------------------------------------------
std::string d2ce::Character::getNameAsString() const
{
    return LocalizationHelpers::ConvertCharNameToString(Bs.Name, Bs.Version);
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
            if (progression < EnumDifficulty::Hell && (act == EnumAct::V) && Acts.getActCompleted(progression, act))
            {
                progression = static_cast<EnumDifficulty>(static_cast<std::underlying_type_t<EnumDifficulty>>(progression) + 1);
            }
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
const std::string& d2ce::Character::getClassName() const
{
    return CharClassHelper::getClassName(getClass());
}
//---------------------------------------------------------------------------
const std::string& d2ce::Character::getClassCode() const
{
    return CharClassHelper::getClassCode(getClass());
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
    size_t current_byte_offset = CHAR_V109_WEAPONSET_BYTE_OFFSET;
    size_t numBytes = CHAR_V109_WEAPONSET_NUM_BYTES;
    if (getVersion() < EnumCharVersion::v109)
    {
        current_byte_offset = CHAR_V100_WEAPONSET_BYTE_OFFSET;
        numBytes = CHAR_V100_WEAPONSET_NUM_BYTES;
    }

    return readBytes(current_byte_offset, numBytes); // pos 16 (1.09+, otherwise pos 26 uint16_t)
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
    setStatusBytes(Bs.Status.bits());
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
    setStatusBytes(Bs.Status.bits());
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
    setStatusBytes(Bs.Status.bits());
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
    setStatusBytes(Bs.Status.bits());
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
    setStatusBytes(Bs.Status.bits());
}
//---------------------------------------------------------------------------
bool d2ce::Character::isFemaleCharacter() const
{
    return Bs.isFemaleCharacter();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevel() const
{
    return Cs.getLevel();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getRecommendedLevel() const
{
    if (isExpansionCharacter())
    {
        switch (Bs.getTitleDifficulty())
        {
        case EnumDifficulty::Normal:
            switch (Bs.getTitleAct())
            {
            case EnumAct::I:
                return 1ui32;

            case EnumAct::II:
                return 12ui32;

            case EnumAct::III:
                return 19ui32;

            case EnumAct::IV:
                return 24ui32;

            case EnumAct::V:
            default:
                return 32ui32;
            }

        case EnumDifficulty::Nightmare:
            switch (Bs.getTitleAct())
            {
            case EnumAct::I:
                return 37ui32;

            case EnumAct::II:
                return 44ui32;

            case EnumAct::III:
                return 49ui32;

            case EnumAct::IV:
                return 53ui32;

            case EnumAct::V:
            default:
                return 57ui32;
            }

        case EnumDifficulty::Hell:
        default:
            switch (Bs.getTitleAct())
            {
            case EnumAct::I:
                return 63ui32;

            case EnumAct::II:
                return 74ui32;

            case EnumAct::III:
                return 81ui32;

            case EnumAct::IV:
                return 83ui32;

            case EnumAct::V:
            default:
                return 95ui32;
            }
        }
    }

    switch (Bs.getTitleDifficulty())
    {
    case EnumDifficulty::Normal:
        switch (Bs.getTitleAct())
        {
        case EnumAct::I:
            return 1ui32;

        case EnumAct::II:
            return 12ui32;

        case EnumAct::III:
            return 19ui32;

        case EnumAct::IV:
        default:
            return 24ui32;
        }

    case EnumDifficulty::Nightmare:
        switch (Bs.getTitleAct())
        {
        case EnumAct::I:
            return 32ui32;

        case EnumAct::II:
            return 37ui32;

        case EnumAct::III:
            return 44ui32;

        case EnumAct::IV:
        default:
            return 49ui32;
        }

    case EnumDifficulty::Hell:
    default:
        switch (Bs.getTitleAct())
        {
        case EnumAct::I:
            return 53ui32;

        case EnumAct::II:
            return 57ui32;

        case EnumAct::III:
            return 63ui32;

        case EnumAct::IV:
        default:
            return 74ui32;
        }
    }
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMaxLevel() const
{
    return Cs.getMaxLevel();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getExperience() const
{
    return Cs.getExperience();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMaxExperience() const
{
    return Cs.getMaxExperience();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMinExperience(std::uint32_t level) const
{
    return Cs.getMinExperience(level);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getMinExperienceLevel() const
{
    return Cs.getMinExperienceLevel();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getNextExperience(std::uint32_t level) const
{
    return Cs.getNextExperience(level);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getNextExperienceLevel() const
{
    return Cs.getNextExperienceLevel();
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
std::uint32_t d2ce::Character::getStrength() const
{
    return Cs.getStrength();
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

    setTitleBytes(title);

    // fix up other indicators of progression
    title = Bs.getStartingActTitle();
    if (title > Bs.Title)
    {
        setDifficultyLastPlayedBytes(Bs.getTitleDifficulty(), Bs.getTitleAct());
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

    setTitleBytes(static_cast<std::underlying_type_t<EnumAct>>(EnumAct::V));

    // fix up other indicators of progression
    auto title = Bs.getStartingActTitle();
    if (title > Bs.Title)
    {
        setDifficultyLastPlayedBytes(Bs.getTitleDifficulty(), Bs.getTitleAct());
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
std::uint32_t d2ce::Character::getDexterity() const
{
    return Cs.getDexterity();
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
    return (std::min(Cs.getMaxLevel(), level) - 1) + Acts.getSkillPointsEarned();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromTotalSkillPoints() const
{
    return getLevelFromSkillPointsEarned(getTotalSkillPoints());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromSkillPointsEarned(std::uint32_t earned) const
{
    return std::min(Cs.getMaxLevel() + 1, earned - Acts.getSkillPointsEarned() + 1);
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
    return std::uint16_t(std::min(Cs.getMaxLevel(), level) - 1) * Cs.getStatPointsPerLevel() + Acts.getStatPointsEarned();
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
    return std::min(Cs.getMaxLevel() + 1, (earned - Acts.getStatPointsEarned()) / Cs.getStatPointsPerLevel() + 1);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromExperience() const
{
    return Cs.getLevelFromExperience();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromExperience(std::uint32_t experience) const
{
    return Cs.getLevelFromExperience(experience);
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
bool d2ce::Character::getSkillBonusPoints(std::vector<std::uint16_t>& points) const
{
    return Cs.getSkillBonusPoints(points);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getItemsStartLocation() const
{
    size_t byteSize = data.size();
    if (byteSize == 0)
    {
        return std::uint32_t(byteSize);
    }

    byteSize += Acts.getByteSize();
    byteSize += Cs.getByteSize();
    return std::uint32_t(byteSize);
}
//---------------------------------------------------------------------------
d2ce::EnumItemVersion d2ce::Character::getDefaultItemVersion() const
{
    return m_items.getDefaultItemVersion();
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
bool d2ce::Character::getHasRestrictedItem(const d2ce::ItemType& itemType) const
{
    return m_items.getHasRestrictedItem(itemType);
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
size_t d2ce::Character::upgradeTierAllItems(ItemFilter filter)
{
    return m_items.upgradeTierAllItems(*this, filter);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::maxDurabilityAllItems(d2ce::ItemFilter filter)
{
    return m_items.maxDurabilityAllItems(filter);
}
//---------------------------------------------------------------------------
size_t d2ce::Character::maxDefenseRatingAllItems(d2ce::ItemFilter filter)
{
    return m_items.setMaxDefenseRatingAllItems(filter);
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
size_t d2ce::Character::setSuperiorAllItems(ItemFilter filter)
{
    return m_items.setSuperiorAllItems(filter);
}
//---------------------------------------------------------------------------
bool d2ce::Character::setItemLocation(d2ce::Item& item, EnumItemLocation locationId, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    return m_items.setItemLocation(item, locationId, altPositionId, positionX, positionY, invType, pRemovedItem);
}
//---------------------------------------------------------------------------
bool d2ce::Character::setItemLocation(d2ce::Item& item, EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    return m_items.setItemLocation(item, locationId, positionX, positionY, invType, pRemovedItem);
}
//---------------------------------------------------------------------------
bool d2ce::Character::setItemLocation(d2ce::Item& item, EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    return m_items.setItemLocation(item, altPositionId, positionX, positionY, invType, pRemovedItem);
}
//---------------------------------------------------------------------------
bool d2ce::Character::setItemLocation(d2ce::Item& item, EnumEquippedId equippedId, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    return m_items.setItemLocation(item, *this, equippedId, invType, pRemovedItem);
}
//---------------------------------------------------------------------------
bool d2ce::Character::setItemRuneword(d2ce::Item& item, std::uint16_t id)
{
    return m_items.setItemRuneword(item, id);
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
bool d2ce::Character::importItem(const d2ce::Item*& pImportedItem, bool bRandomizeId)
{
    return m_items.importItem(pImportedItem, bRandomizeId);
}
//---------------------------------------------------------------------------
bool d2ce::Character::importItem(const std::filesystem::path& path, const d2ce::Item*& pImportedItem, bool bRandomizeId)
{
    return m_items.importItem(path, pImportedItem, bRandomizeId);
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
bool d2ce::Character::removeSocketedItems(d2ce::Item& item)
{
    return m_items.removeSocketedItems(item);
}
//---------------------------------------------------------------------------
bool d2ce::Character::upgradeItemTier(d2ce::Item& item)
{
    d2ce::CharStats cs;
    fillDisplayedCharacterStats(cs);
    return m_items.upgradeItemTier(item, cs);
}
//---------------------------------------------------------------------------
bool d2ce::Character::changeItemEthereal(d2ce::Item& item)
{
    return m_items.changeItemEthereal(item);
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
std::uint64_t d2ce::Character::readBytes(std::FILE* charfile, size_t& current_byte_offset, size_t bytes)
{
    size_t readOffset = current_byte_offset;

    // Ensure we read enough
    if (!skipBytes(charfile, current_byte_offset, bytes))
    {
        return 0;
    }

    if (bytes > 8)
    {
        return 0;
    }

    return readBytes64(readOffset, bytes);
}
//---------------------------------------------------------------------------
bool d2ce::Character::skipBytes(std::FILE* charfile, size_t& current_byte_offset, size_t bytes)
{
    if (feof(charfile))
    {
        return false;
    }

    size_t bytesRequired = current_byte_offset + bytes;
    std::uint8_t value = 0;
    while (data.size() < bytesRequired)
    {
        if (feof(charfile))
        {
            return false;
        }

        std::fread(&value, sizeof(value), 1, charfile);
        data.push_back(value);
    }

    current_byte_offset += bytes;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::setBytes(size_t& current_byte_offset, size_t bytes, std::uint8_t* value)
{
    if (value == nullptr)
    {
        return false;
    }

    size_t readOffset = current_byte_offset;
    size_t bytesRequired = current_byte_offset + bytes;
    std::uint8_t byte = 0;
    while (data.size() < bytesRequired)
    {
        data.push_back(byte);
    }

    current_byte_offset += bytes;
    return updateBytes(readOffset, bytes, value);
}
//---------------------------------------------------------------------------
bool d2ce::Character::setBytes(size_t& current_byte_offset, size_t bytes, std::uint32_t value)
{
    size_t readOffset = current_byte_offset;
    if (bytes > 4)
    {
        return false;
    }

    size_t bytesRequired = current_byte_offset + bytes;
    std::uint8_t byte = 0;
    while (data.size() < bytesRequired)
    {
        data.push_back(byte);
    }

    current_byte_offset += bytes;
    return updateBytes(readOffset, bytes, value);
}
//---------------------------------------------------------------------------
bool d2ce::Character::setBytes64(size_t& current_byte_offset, size_t bytes, std::uint64_t value)
{
    size_t readOffset = current_byte_offset;
    if (bytes > 8)
    {
        return false;
    }

    size_t bytesRequired = current_byte_offset + bytes;
    std::uint8_t byte = 0;
    while (data.size() < bytesRequired)
    {
        data.push_back(byte);
    }

    current_byte_offset += bytes;
    return updateBytes64(readOffset, bytes, value);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::readBytes(size_t start, size_t size) const
{
    if ((size > 4) || (start >= data.size()))
    {
        return 0;
    }

    if (size >= 3)
    {
        return (std::uint32_t)read_uint64_bits(start * 8, size * 8);
    }
    return read_uint32_bits(start * 8, size * 8);
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::Character::readBytes64(size_t start, size_t size) const
{
    if ((size > 7) || (start >= data.size()))
    {
        return 0;
    }

    if (size >= 3)
    {
        return read_uint64_bits(start * 8, size * 8);
    }

    return (std::uint64_t)read_uint32_bits(start * 8, size * 8);
}
//---------------------------------------------------------------------------
bool d2ce::Character::updateBytes(size_t start, size_t size, std::uint8_t* value)
{
    if (value == nullptr)
    {
        return false;
    }

    size_t endIdx = start + size - 1;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    std::memcpy(&data[start], value, size);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::updateBytes(size_t start, size_t size, std::uint32_t value)
{
    size_t endIdx = start + size - 1;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    if (size > 4)
    {
        // 32 bit value can't consume more then this many bytes
        return false;
    }

    std::memcpy(&data[start], (std::uint8_t*)&value, size);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::updateBytes64(size_t start, size_t size, std::uint64_t value)
{
    size_t endIdx = start + size - 1;
    if (endIdx >= data.size())
    {
        // not enough space
        return false;
    }

    if (size > 8)
    {
        // value is too big for us to handle
        return false;
    }

    std::memcpy(&data[start], (std::uint8_t*)&value, size);
    return true;
}
//---------------------------------------------------------------------------