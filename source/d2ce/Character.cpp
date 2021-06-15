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

#include "pch.h"
#include "Character.h"
#include "CharacterConstants.h"
#include "ItemConstants.h"
#include "SkillConstants.h"
//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint8_t HEADER[] = { 0x55, 0xAA, 0x55, 0xAA };

    constexpr std::uint8_t SKILLS_MARKER[] = { 0x69, 0x66 };             // alternatively "if"
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
d2ce::Character::Character()
{
    initialize();
}
//---------------------------------------------------------------------------
d2ce::Character::~Character()
{
    if (charfile != nullptr)
    {
        std::fclose(charfile);
    }
}
//---------------------------------------------------------------------------
void d2ce::Character::initialize()
{
    std::memset(Header, 0, sizeof(Header));
    Version = 0;
    FileSize = 0;
    Checksum = 0;
    WeaponSet = 0;
    std::memset(Bs.Name, 0, sizeof(Bs.Name));

    Bs.Status = EnumCharStatus::NoDeaths;
    Bs.Title = EnumCharTitle::None;
    Bs.Class = EnumCharClass::Amazon;
    DisplayLevel = 1;

    DifficultyAndAct = 0;
    Bs.DifficultyLastPlayed = EnumDifficulty::Normal;
    std::memset(StartingAct, 0, sizeof(StartingAct));
    Bs.StartingAct = EnumAct::I;
    Acts.clear();

    Cs.clear();
    std::memset(Skills, 0, sizeof(Skills));

    error_code.clear();

    items.clear();

    filesize_location = 0;
    checksum_location = 0;
    name_location = 0;
    class_location = 0;
    level_location = 0;
    starting_location = 0;
    stats_header_location = 0;
    skills_location = 0;
    update_locations = true;

    if (charfile != nullptr)
    {
        std::rewind(charfile);
    }
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

    error_code.clear();
#ifdef _MSC_VER
    charfile = _fsopen(szfilename, "rb+", _SH_DENYNO);
#else
    errno_t err = fopen_s(&charfile, szfilename, "rb+");
    if (err != 0)
    {
        error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }
#endif

    filename = szfilename;
    if (charfile == nullptr)
    {
        error_code = std::make_error_code(CharacterErrc::CannotOpenFile);
        return false;
    }

    readHeader();
    if (!isValidHeader())
    {
        error_code = std::make_error_code(CharacterErrc::InvalidHeader);
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
        error_code = std::make_error_code(CharacterErrc::InvalidChecksum);
        if (validateChecksum)
        {
            close();
            return false;
        }
    }

    update_locations = false;
    return true;
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
    std::rewind(charfile);

    Checksum = 0;

    if ((checksum_location == 0) || (Bs.Version < EnumCharVersion::v109))
    {
        // checksum not supported
        return;
    }

    std::uint32_t i = 0;
    std::uint8_t data, overflow = 0;
    for (; i < checksum_location; ++i)
    {
        // doubles the checksum result by left shifting once
        Checksum <<= 1;

        std::fread(&data, sizeof(data), 1, charfile);

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
    std::fseek(charfile, sizeof(Checksum), SEEK_CUR);
    std::uint32_t nextStop = std::ftell(charfile);
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

        std::fread(&data, sizeof(data), 1, charfile);

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
    std::rewind(charfile);
}
//---------------------------------------------------------------------------
void d2ce::Character::readHeader()
{
    std::rewind(charfile);
    std::fread(Header, sizeof(Header), 1, charfile);
}
//---------------------------------------------------------------------------
/*
   Returns true if the file has a valid header
*/
bool d2ce::Character::isValidHeader() const
{
    return std::memcmp(Header, HEADER, HEADER_LENGTH) == 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::Character::refresh()
{
    readBasicInfo();
    if (!readActs())
    {
        // bad file
        close();
        error_code = std::make_error_code(CharacterErrc::InvalidActsInfo);
        return false;
    }

    if (!readStats())
    {
        // bad file
        close();
        error_code = std::make_error_code(CharacterErrc::InvalidCharStats);
        return false;
    }

    if(!readSkills())
    {
        // bad file
        close();
        error_code = std::make_error_code(CharacterErrc::InvalidCharSkills);
        return false;
    }

    // Read Character, Corpse, Mercenary and Golem items
    if (!readItems())
    {
        // bad file
        close();
        error_code = std::make_error_code(CharacterErrc::InvalidItemInventory);
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Character::readBasicInfo()
{
    std::fseek(charfile, HEADER_LENGTH, SEEK_SET);
    std::fread(&Version, sizeof(Version), 1, charfile);

    Bs.Version = getVersion();
    Cs.Cs.Version = Bs.Version;

    filesize_location = 0;
    checksum_location = 0;
    if (Bs.Version >= EnumCharVersion::v109)
    {
        filesize_location = std::ftell(charfile);
        std::fread(&FileSize, sizeof(FileSize), 1, charfile);
        checksum_location = std::ftell(charfile);
        std::fread(&Checksum, sizeof(Checksum), 1, charfile);
        std::fread(&WeaponSet, sizeof(WeaponSet), 1, charfile);
    }

    name_location = std::ftell(charfile);
    std::fread(Bs.Name, sizeof(Bs.Name), 1, charfile);
    std::uint8_t value = 0;
    std::fread(&value, sizeof(value), 1, charfile);
    Bs.Status = static_cast<EnumCharStatus>(value);

    // ladder is for 1.10 or higher
    if (Bs.Version < EnumCharVersion::v110)
    {
        Bs.Status &= ~EnumCharStatus::Ladder;
    }

    value = 0;
    std::fread(&value, sizeof(value), 1, charfile);
    Bs.Title = static_cast<EnumCharTitle>(value);

    if (Bs.Version < EnumCharVersion::v109)
    {
        class_location = 34;
    }
    else
    {
        class_location = 40;
    }

    std::fseek(charfile, class_location, SEEK_SET);
    std::fread(&value, sizeof(value), 1, charfile);
    Bs.Class = static_cast<EnumCharClass>(value);

    if (Bs.Version < EnumCharVersion::v109)
    {
        level_location = 36;
    }
    else
    {
        level_location = 43;
    }

    std::fseek(charfile, level_location, SEEK_SET);
    std::fread(&DisplayLevel, sizeof(DisplayLevel), 1, charfile);

    if (Bs.Version < EnumCharVersion::v109)
    {
        starting_location = 88;
        std::fseek(charfile, starting_location, SEEK_SET);
        std::fread(&DifficultyAndAct, sizeof(DifficultyAndAct), 1, charfile);
        Bs.DifficultyLastPlayed = static_cast<EnumDifficulty>(DifficultyAndAct & 0x0F);
        Bs.StartingAct = static_cast<EnumAct>(DifficultyAndAct >> 4);
    }
    else
    {
        starting_location = 168;
        std::fseek(charfile, starting_location, SEEK_SET);
        std::fread(StartingAct, sizeof(StartingAct), 1, charfile);
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
    }
}
//---------------------------------------------------------------------------
bool d2ce::Character::readActs()
{
    return Acts.readActs(Bs.Version, charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::readStats()
{
    if (Cs.readStats(Bs.Version, charfile))
    {
        stats_header_location = Cs.getHeaderLocation();
        DisplayLevel = (std::uint8_t)Cs.Cs.Level; // updates character's display level
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool d2ce::Character::readSkills()
{
    if (update_locations)
    {
        // find stats location
        skills_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)stats_header_location)
        {
            cur_pos = (long)stats_header_location;
            std::fseek(charfile, cur_pos, SEEK_SET);
        }

        while (!feof(charfile))
        {
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != SKILLS_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != SKILLS_MARKER[1])
            {
                continue;
            }

            // found skills marker (0x6669). 
            skills_location = std::ftell(charfile);
            break;
        }

        if (skills_location == 0)
        {
            return false;
        }
    }
    else
    {
        if (skills_location == 0)
        {
            return false;
        }

        std::fseek(charfile, skills_location, SEEK_SET);
    }

    std::fread(Skills, sizeof(Skills), 1, charfile);
    return true;
}
bool d2ce::Character::readItems()
{
    return items.readItems(Bs.Version, charfile, isExpansionCharacter());
}
//---------------------------------------------------------------------------
bool d2ce::Character::save()
{
    if (charfile == nullptr)
    {
        return false;
    }

    error_code.clear();
    writeBasicStats();
    writeActs();

    // From this point on, the location is variable
    writeStats();
    writeSkills();

    // Write Character, Corpse, Mercenary and Golem items
    writeItems();
    if (FileSize > (std::uint32_t)std::ftell(charfile))
    {
        // truncation occured
        writeTempFile();

        // prepare to update the character file
        std::fclose(charfile);
        charfile = nullptr;
        std::remove(filename.c_str());

        // check to see if the filename needs to be changed
        // to match the character's name
        std::filesystem::path p = filename;
        p.replace_extension();
        std::string tempname = p.filename().string();

        // compare filename (w/o extension) to character's name
        if (tempname.compare(0, tempname.length(), Bs.Name) != 0)
        {
            filename = p.replace_filename(Bs.Name).string();
        }

        // rename temp file to character file
        if (std::rename(tempfilename.c_str(), filename.c_str()))
        {
            error_code = std::make_error_code(CharacterErrc::FileRenameError);
            return false;
        }

        if (!open(filename.c_str(), false)) // checksum is calulated and written below
        {
            return false;
        }
    }
    else
    {
        // check to see if the filename needs to be changed
        // to match the character's name
        tempfilename = filename;
        std::filesystem::path p = filename;
        p.replace_extension();
        std::string tempname = p.filename().string();

        // compare filename (w/o extension) to character's name
        if (tempname.compare(0, tempname.length(), Bs.Name) != 0)
        {
            filename = p.replace_filename(Bs.Name).string();
            std::fclose(charfile);
            charfile = nullptr;
            if (std::rename(tempfilename.c_str(), filename.c_str()))
            {
                error_code = std::make_error_code(CharacterErrc::FileRenameError);
                return false;
            }

            if (!open(filename.c_str(), false)) // checksum is calulated and written below
            {
                return false;
            }
        }
    }

    // determine if a checksum needs to be calculated and stored
    if (Bs.Version >= EnumCharVersion::v109)
    {
        // store the file's size
        std::fflush(charfile);
        std::fseek(charfile, 0, SEEK_END);
        FileSize = std::ftell(charfile);
        std::fseek(charfile, filesize_location, SEEK_SET);
        std::fwrite(&FileSize, sizeof(FileSize), 1, charfile);

        // make sure the checksum is zero in the file
        Checksum = 0;
        checksum_location = std::ftell(charfile);
        std::fwrite(&Checksum, sizeof(Checksum), 1, charfile);
        std::fwrite(&WeaponSet, sizeof(WeaponSet), 1, charfile);

        std::fflush(charfile);
        calculateChecksum();

        // write the checksum into the file
        std::fseek(charfile, checksum_location, SEEK_SET);
        std::fwrite(&Checksum, sizeof(Checksum), 1, charfile);
    }

    std::fflush(charfile);
    return true;
}
//---------------------------------------------------------------------------
void d2ce::Character::writeName()
{
    // make sure name is saved correctly
    std::string temp(Bs.Name);
    std::memset(Bs.Name, 0, sizeof(Bs.Name));
    strcpy_s(Bs.Name, temp.length() + 1, temp.c_str());

    std::fseek(charfile, name_location, SEEK_SET);
    std::fwrite(Bs.Name, sizeof(Bs.Name), 1, charfile);
    std::fflush(charfile);
}
//---------------------------------------------------------------------------
void d2ce::Character::writeBasicStats()
{
    writeName();

    // ladder is for 1.10 or higher
    if (Bs.Version < EnumCharVersion::v110)
    {
        Bs.Status &= ~EnumCharStatus::Ladder;
    }

    if ((Bs.Status & EnumCharStatus::Hardcore) != 0)
    {
        Bs.Status &= ~EnumCharStatus::Resurrected; // can't be resurrected
    }

    std::uint8_t value = Bs.Status.bits();
    std::fwrite(&value, sizeof(value), 1, charfile);

    value = Bs.Title.bits();
    std::fwrite(&value, sizeof(value), 1, charfile);

    std::fseek(charfile, class_location, SEEK_SET);
    value = static_cast<std::underlying_type_t<EnumCharClass>>(Bs.Class);
    std::fwrite(&value, sizeof(value), 1, charfile);

    std::fseek(charfile, level_location, SEEK_SET);
    std::fwrite(&DisplayLevel, sizeof(DisplayLevel), 1, charfile);

    std::fseek(charfile, starting_location, SEEK_SET);
    if (Bs.Version < EnumCharVersion::v109)
    {
        std::fwrite(&DifficultyAndAct, sizeof(DifficultyAndAct), 1, charfile);
    }
    else
    {
        std::fwrite(StartingAct, sizeof(StartingAct), 1, charfile);
    }
    std::fflush(charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeActs()
{
    return Acts.writeActs(charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeStats()
{
    return Cs.writeStats(charfile);
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeSkills()
{
    std::fwrite(SKILLS_MARKER, sizeof(SKILLS_MARKER), 1, charfile);
    skills_location = std::ftell(charfile);
    std::fwrite(Skills, sizeof(Skills), 1, charfile);
    std::fflush(charfile);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::Character::writeItems()
{
    return items.writeItems(charfile, isExpansionCharacter());
}
//---------------------------------------------------------------------------
/*
   This function makes sure that any changes to the character's
   experience and gold values are correctly stored in the file.
*/
void d2ce::Character::writeTempFile()
{
    tempfilename.clear();
    char name1[L_tmpnam_s];
    errno_t err = tmpnam_s(name1, L_tmpnam_s);
    if (err == 0)
    {
        tempfilename = name1;
    }

    std::FILE* tempfile = NULL;
    fopen_s(&tempfile, tempfilename.c_str(), "wb");

    std::rewind(charfile);

    // Write the beginning using one buffer
    std::vector<std::uint8_t> buffer((size_t)stats_header_location + 1, 0);
    std::fread(&buffer[0], stats_header_location, 1, charfile);
    std::fwrite(&buffer[0], stats_header_location, 1, tempfile);

    // From this point on, the location is variable
    Cs.writeStats(tempfile);

    std::fwrite(SKILLS_MARKER, sizeof(SKILLS_MARKER), 1, tempfile);
    skills_location = std::ftell(charfile);
    std::fwrite(Skills, sizeof(Skills), 1, tempfile);
    std::fflush(tempfile);

    // Write Character, Corpse, Mercenary and Golem items
    items.writeItems(tempfile, isExpansionCharacter());
    std::fclose(tempfile);
}
//---------------------------------------------------------------------------
void d2ce::Character::close()
{
    if (charfile != nullptr)
    {
        std::fclose(charfile);
    }
    charfile = nullptr;
    initialize();
}
//---------------------------------------------------------------------------
bool d2ce::Character::is_open() const
{
    return charfile == nullptr ? false : true;
}
//---------------------------------------------------------------------------
std::error_code d2ce::Character::getLastError() const
{
    return error_code;
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
void d2ce::Character::updateBasicStats(const BasicStats& bs)
{
    std::memcpy(&Bs, &bs, sizeof(BasicStats));
    Bs.Version = getVersion();

    // ladder is for 1.10 or higher
    if (Bs.Version < EnumCharVersion::v110)
    {
        Bs.Status &= ~EnumCharStatus::Ladder;
    }

    if ((Bs.Status & EnumCharStatus::Hardcore) != 0)
    {
        Bs.Status &= ~EnumCharStatus::Resurrected; // can't be resurrected
    }

    if (Bs.Version < EnumCharVersion::v109)
    {
        DifficultyAndAct = static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
        DifficultyAndAct <<= 4;
        DifficultyAndAct |= (static_cast<std::underlying_type_t<EnumDifficulty>>(Bs.DifficultyLastPlayed) & 0x0F);
    }
    else
    {
        std::memset(StartingAct, 0, sizeof(StartingAct));
        StartingAct[static_cast<std::underlying_type_t<EnumDifficulty>>(bs.DifficultyLastPlayed)] = 0x80 | static_cast<std::underlying_type_t<EnumAct>>(Bs.StartingAct);
    }
}
//---------------------------------------------------------------------------
void d2ce::Character::updateCharacterStats(const CharStats& cs)
{
    Cs.updateCharacterStats(cs);
    DisplayLevel = (std::uint8_t)Cs.Cs.Level; // updates character's display level
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
const char (&d2ce::Character::getName())[NAME_LENGTH]
{
    return Bs.Name;
}
//---------------------------------------------------------------------------
bitmask::bitmask<d2ce::EnumCharStatus> d2ce::Character::getStatus() const
{
    return Bs.Status;
}
//---------------------------------------------------------------------------
bitmask::bitmask<d2ce::EnumCharTitle> d2ce::Character::getTitle() const
{
    return Bs.Title;
}
//---------------------------------------------------------------------------
d2ce::EnumCharClass d2ce::Character::getClass() const
{
    return Bs.Class;
}
//---------------------------------------------------------------------------
/*
   Returns a value indicating the difficulty level the character last played at
*/
d2ce::EnumDifficulty d2ce::Character::getDifficultyLastPlayed()
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
std::uint16_t d2ce::Character::getLifePointsEarned() const
{
    return Acts.getLifePointsEarned();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getSkillPointsEarned(std::uint32_t level) const
{
    return (std::min(d2ce::NUM_OF_LEVELS, level) - 1) + Acts.getSkillPointsEarned();
}
std::uint32_t d2ce::Character::getSkillPointsEarned() const
{
    return getSkillPointsEarned(Cs.getLevel());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromSkillPointsEarned(std::uint32_t earned) const
{
    return std::min(d2ce::NUM_OF_LEVELS, earned - Acts.getSkillPointsEarned() + 1);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getStatPointsEarned(std::uint32_t level) const
{
    return std::uint16_t(std::min(d2ce::NUM_OF_LEVELS, level) - 1) * 5 + Acts.getStatPointsEarned();
}
std::uint32_t d2ce::Character::getStatPointsEarned() const
{
    return getStatPointsEarned(Cs.getLevel());
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getLevelFromStatPointsEarned(std::uint32_t earned) const
{
    return std::min(d2ce::NUM_OF_LEVELS, (earned - Acts.getStatPointsEarned()) / 5 + 1);
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
std::uint8_t(&d2ce::Character::getSkills())[NUM_OF_SKILLS]
{
    return Skills;
}
//---------------------------------------------------------------------------
void d2ce::Character::updateSkills(const std::uint8_t(&updated_skills)[NUM_OF_SKILLS])
{
    std::memcpy(Skills, updated_skills, sizeof(Skills));
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getSkillPointUsed() const
{
    std::uint32_t skillUsed = 0;
    for (std::uint32_t i = 0; i < NUM_OF_SKILLS; ++i)
    {
        skillUsed += Skills[i];
    }

    return skillUsed;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::Character::getSkillChoices() const
{
    return Cs.getSkillChoices();
}
//---------------------------------------------------------------------------
bool d2ce::Character::areSkillsMaxed() const
{
    for (std::uint32_t i = 0; i < NUM_OF_SKILLS; ++i)
    {
        if (Skills[i] != MAX_SKILL_VALUE)
        {
            return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
void d2ce::Character::maxSkills()
{
    std::memset(Skills, MAX_SKILL_VALUE, sizeof(Skills));
}
//---------------------------------------------------------------------------
void d2ce::Character::resetSkills()
{
    std::memset(Skills, 0, sizeof(Skills));
}
//---------------------------------------------------------------------------
void d2ce::Character::clearSkillChoices()
{
    Cs.clearSkillChoices();
}
//---------------------------------------------------------------------------
/*
   Returns the number of items in character's inventory.
   Value returned excludes socketed gems/jewels/runes.
*/
size_t d2ce::Character::getNumberOfItems() const
{
    return items.getNumberOfItems();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfArmor() const
{
    return items.getNumberOfArmor();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfWeapons() const
{
    return items.getNumberOfWeapons();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::maxDurabilityAllItems()
{
    return items.maxDurabilityAllItems();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfStackables() const
{
    return items.getNumberOfStackables();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::fillAllStackables()
{
    return items.fillAllStackables();
}
//---------------------------------------------------------------------------
size_t d2ce::Character::getNumberOfGPSs() const
{
    return items.getNumberOfGPSs();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& d2ce::Character::getGPSs()
{
    return items.getGPSs();
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems, potions or skulls to the specified
   final gem, potion or skull.
   Returns the number of gems converted.
*/
size_t d2ce::Character::convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4])
{
    return items.convertGPSs(existingGem, desiredGem);
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-perfect gems.
   Returns false if no gems or all gems are perfect.
*/
bool d2ce::Character::anyUpgradableGems() const
{
    return items.anyUpgradableGems();
}
//---------------------------------------------------------------------------
/*
   Converts the all gems to their perfect state
   Returns the number of gems converted.
*/
size_t d2ce::Character::upgradeGems()
{
    return items.upgradeGems();
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-Super Health/Mana potions or
                non-Full Rejuvenation potions.
   Returns false if no potions or all potions are the top kind.
*/
bool d2ce::Character::anyUpgradablePotions() const
{
    return items.anyUpgradablePotions();
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to their highest quiality.
   Returns the number of potions converted.
*/
size_t d2ce::Character::upgradePotions()
{
    return items.upgradePotions();
}
//---------------------------------------------------------------------------
/*
   Returns true if there are any non-Full Rejuvenation potions.
   Returns false if no potions or all potions Full Rejuvenation potions.
*/
bool d2ce::Character::anyUpgradableRejuvenations() const
{
    return items.anyUpgradableRejuvenations();
}
//---------------------------------------------------------------------------
/*
   Converts the all potions to Full Rejuvenation potions.
   Returns the number of potions converted.
*/
size_t d2ce::Character::upgradeRejuvenationPotions()
{
    return items.upgradeRejuvenationPotions();
}
//---------------------------------------------------------------------------
