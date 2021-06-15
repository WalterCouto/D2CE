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
#include "CharacterStats.h"
//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint8_t STATS_MARKER[] = { 0x67, 0x66 };              // alternatively "gf"
    constexpr uint16_t STAT_MAX = 16;
    constexpr uint16_t STAT_END_MARKER = 0x1FF;
    constexpr size_t STAT_BITS = 9;
    constexpr size_t V110_BITS_PER_STAT[] = { 10,10,10,10,10,8,21,21,21,21,21,21,7,32,25,25 };

    constexpr std::uint32_t MIN_START_STATS_POS = 765;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
d2ce::CharacterStats::CharacterStats()
{
    data.reserve(54); // reserve the maximum byte length to reduce allocations
}
//---------------------------------------------------------------------------
d2ce::CharacterStats::~CharacterStats()
{
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::checkStatInfo()
{
    // CurMana and CurStamina can be zero but seems to be always present
    StatInfo = EnumCharStatInfo::All;
    if (Cs.StatsLeft == 0)
    {
        StatInfo &= ~EnumCharStatInfo::StatPoints;
    }

    if (Cs.SkillChoices == 0)
    {
        StatInfo &= ~EnumCharStatInfo::SkillChoices;
    }

    if (Cs.CurLife == 0) // if you are dead
    {
        StatInfo &= ~EnumCharStatInfo::CurLife;
    }

    if (Cs.Experience == 0) // if you just started
    {
        StatInfo &= ~EnumCharStatInfo::Experience;
    }

    if (Cs.GoldInBelt == 0)
    {
        StatInfo &= ~EnumCharStatInfo::GoldInBelt;
    }

    if (Cs.GoldInStash == 0)
    {
        StatInfo &= ~EnumCharStatInfo::GoldInStash;
    }
}
//---------------------------------------------------------------------------
d2ce::EnumCharStatInfo d2ce::CharacterStats::GetStatInfoMask(uint16_t stat)
{
    switch (stat)
    {
    case 0:
        return EnumCharStatInfo::Strength;
    case 1:
        return EnumCharStatInfo::Energy;
    case 2:
        return EnumCharStatInfo::Dexterity;
    case 3:
        return EnumCharStatInfo::Vitality;
    case 4:
        return EnumCharStatInfo::StatPoints;
    case 5:
        return EnumCharStatInfo::SkillChoices;
    case 6:
        return EnumCharStatInfo::CurLife;
    case 7:
        return EnumCharStatInfo::MaxLife;
    case 8:
        return EnumCharStatInfo::CurMana;
    case 9:
        return EnumCharStatInfo::MaxMana;
    case 10:
        return EnumCharStatInfo::CurStamina;
    case 11:
        return EnumCharStatInfo::MaxStamina;
    case 12:
        return EnumCharStatInfo::Level;
    case 13:
        return EnumCharStatInfo::Experience;
    case 14:
        return EnumCharStatInfo::GoldInBelt;
    case 15:
        return EnumCharStatInfo::GoldInStash;
    default:
        return EnumCharStatInfo::All;
    }
}
//---------------------------------------------------------------------------
std::uint32_t* d2ce::CharacterStats::GetStatBuffer(uint16_t stat)
{
    switch (stat)
    {
    case 0:
        return &Cs.Strength;
    case 1:
        return &Cs.Energy;
    case 2:
        return &Cs.Dexterity;
    case 3:
        return &Cs.Vitality;
    case 4:
        return &Cs.StatsLeft;
    case 5:
        return &Cs.SkillChoices;
    case 6:
        return &Cs.CurLife;
    case 7:
        return &Cs.MaxLife;
    case 8:
        return &Cs.CurMana;
    case 9:
        return &Cs.MaxMana;
    case 10:
        return &Cs.CurStamina;
    case 11:
        return &Cs.MaxStamina;
    case 12:
        return &Cs.Level;
    case 13:
        return &Cs.Experience;
    case 14:
        return &Cs.GoldInBelt;
    case 15:
        return &Cs.GoldInStash;
    default:
        return nullptr;
    }
}
//---------------------------------------------------------------------------
std::uint64_t d2ce::CharacterStats::readBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
{
    size_t readOffset = current_bit_offset;

    // Ensure we read enough
    if (!skipBits(charfile, current_bit_offset, bits))
    {
        return 0;
    }

    if (bits > 64)
    {
        return 0;
    }

    return (*((std::uint64_t*)&data[readOffset / 8]) >> (readOffset & 7)) & ((std::uint64_t(1) << bits) - 1);
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::skipBits(std::FILE* charfile, size_t& current_bit_offset, size_t bits)
{
    if (feof(charfile))
    {
        return false;
    }

    size_t bytesRequired = (current_bit_offset + bits + 7) / 8;
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

    current_bit_offset += bits;
    return true;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::readNextStat(std::FILE* charfile, size_t& current_bit_offset, uint16_t& stat)
{
    size_t totalBitsRead = STAT_BITS;
    stat = (std::uint16_t)readBits(charfile, current_bit_offset, STAT_BITS);
    if (stat >= STAT_MAX)
    {
        return totalBitsRead;
    }

    totalBitsRead += readStatBits(charfile, current_bit_offset, stat);
    return totalBitsRead;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::readStatBits(std::FILE* charfile, size_t& current_bit_offset, uint16_t stat)
{
    std::uint32_t* pStatValue = GetStatBuffer(stat);
    if (pStatValue == nullptr)
    {
        return 0;
    }

    size_t bits = V110_BITS_PER_STAT[stat];
    *pStatValue = (std::uint32_t)readBits(charfile, current_bit_offset, bits);
    return bits;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readStats(std::FILE* charfile)
{
    if (update_locations)
    {
        // find stats location
        stats_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (Cs.Version >= EnumCharVersion::v109)
        {
            if (cur_pos < MIN_START_STATS_POS)
            {
                cur_pos = MIN_START_STATS_POS;
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
            std::fread(&value, sizeof(value), 1, charfile);
            if (value != STATS_MARKER[0])
            {
                continue;
            }

            std::fread(&value, sizeof(value), 1, charfile);
            if (value != STATS_MARKER[1])
            {
                continue;
            }

            // found stats marker (0x6766). 
            stats_location = std::ftell(charfile);
            break;
        }
    }

    if (stats_location == 0)
    {
        return false;
    }

    std::fseek(charfile, stats_location, SEEK_SET);

    // zero out all stats as zero value stats do not have to exists in the file
    Cs.Strength = 0;
    Cs.Energy = 0;
    Cs.Dexterity = 0;
    Cs.Vitality = 0;
    Cs.StatsLeft = 0;
    Cs.SkillChoices = 0;
    Cs.CurLife = 0;
    Cs.MaxLife = 0;
    Cs.CurMana = 0;
    Cs.MaxMana = 0;
    Cs.CurStamina = 0;
    Cs.MaxStamina = 0;
    Cs.Level = 1;
    Cs.Experience = 0;
    Cs.GoldInBelt = 0;
    Cs.GoldInStash = 0;

    if (Cs.Version < EnumCharVersion::v110)
    {
        return readStats_109(charfile);
    }

    size_t current_bit_offset = 0;
    data.clear();

    uint16_t stat = 0;
    size_t totalBitsRead = 0;
    size_t bitsRead = readNextStat(charfile, current_bit_offset, stat);
    while (bitsRead > 0)
    {
        totalBitsRead += bitsRead;
        if (stat >= STAT_MAX)
        {
            break;
        }

        bitsRead = readNextStat(charfile, current_bit_offset, stat);
    }

    return totalBitsRead > 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readStats_109(std::FILE* charfile)
{
    std::uint16_t value;
    std::fread(&value, sizeof(value), 1, charfile);
    StatInfo = static_cast<EnumCharStatInfo>(value);

    // skip null byte if character version is less than 1.09
    if (Cs.Version < EnumCharVersion::v109)
    {
        std::fread(&nullByte, sizeof(nullByte), 1, charfile);
    }

    // read basic stats
    std::fread(&Cs.Strength, sizeof(Cs.Strength), 1, charfile);   // always present so no need to check StatInfo bits
    std::fread(&Cs.Energy, sizeof(Cs.Energy), 1, charfile);       // always present so no need to check StatInfo bits
    std::fread(&Cs.Dexterity, sizeof(Cs.Dexterity), 1, charfile); // always present so no need to check StatInfo bits
    std::fread(&Cs.Vitality, sizeof(Cs.Vitality), 1, charfile);   // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::StatPoints) == EnumCharStatInfo::StatPoints)
    {
        std::fread(&Cs.StatsLeft, sizeof(Cs.StatsLeft), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::SkillChoices) == EnumCharStatInfo::SkillChoices)
    {
        std::fread(&Cs.SkillChoices, sizeof(Cs.SkillChoices), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::CurLife) == EnumCharStatInfo::CurLife)
    {
        std::fread(&Cs.CurLife, sizeof(Cs.CurLife), 1, charfile);
    }

    std::fread(&Cs.MaxLife, sizeof(Cs.MaxLife), 1, charfile); // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::CurMana) == EnumCharStatInfo::CurMana) // can be zero but should be always present
    {
        std::fread(&Cs.CurMana, sizeof(Cs.CurMana), 1, charfile);
    }

    std::fread(&Cs.MaxMana, sizeof(Cs.MaxMana), 1, charfile); // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::CurStamina) == EnumCharStatInfo::CurStamina) // can be zero but should be always present
    {
        std::fread(&Cs.CurStamina, sizeof(Cs.CurStamina), 1, charfile);
    }

    std::fread(&Cs.MaxStamina, sizeof(Cs.MaxStamina), 1, charfile); // always present so no need to check StatInfo bits
    std::fread(&Cs.Level, sizeof(Cs.Level), 1, charfile);           // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::Experience) == EnumCharStatInfo::Experience)
    {
        std::fread(&Cs.Experience, sizeof(Cs.Experience), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::GoldInBelt) == EnumCharStatInfo::GoldInBelt)
    {
        std::fread(&Cs.GoldInBelt, sizeof(Cs.GoldInBelt), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::GoldInStash) == EnumCharStatInfo::GoldInStash)
    {
        std::fread(&Cs.GoldInStash, sizeof(Cs.GoldInStash), 1, charfile);
    }

    return true;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::updateBits(size_t& current_bit_offset, size_t size, std::uint32_t value)
{
    size_t startIdx = current_bit_offset / 8;
    size_t endIdx = (current_bit_offset + size) / 8;
    if (endIdx >= data.size())
    {
        // not enough space
        data.resize(endIdx + 1, 0);
    }

    size_t startBit = current_bit_offset % 8;
    size_t numBytes = (endIdx - startIdx) + 1;
    if (numBytes > 5)
    {
        // 32 bit value can't consume more then this many bytes
        return 0;
    }

    std::uint64_t mask = (((std::uint64_t)1 << size) - 1) << (startBit);
    std::uint64_t dest = 0;
    std::memcpy((std::uint8_t*)&dest, &data[startIdx], numBytes);
    dest = (dest & ~mask) | (((std::uint64_t)value << (startBit)) & mask);
    std::memcpy(&data[startIdx], (std::uint8_t*)&dest, numBytes);
    current_bit_offset += size;
    return size;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::updateStat(std::FILE* charfile, size_t& current_bit_offset, uint16_t stat)
{
    if (stat >= STAT_MAX && (stat != STAT_END_MARKER))
    {
        return 0;
    }

    EnumCharStatInfo mask = GetStatInfoMask(stat);
    if ((stat != STAT_END_MARKER) && (StatInfo & mask) != mask)
    {
        // skip any stat that who's mask indicates we should not write it
        return 0;
    }

    size_t totalBitsWritten = updateBits(current_bit_offset, STAT_BITS, std::uint16_t(stat));
    if (totalBitsWritten == 0)
    {
        return 0;
    }

    if (stat == STAT_END_MARKER)
    {
        writeBufferBits(charfile);
        return totalBitsWritten + ((data.size() * 8) - current_bit_offset); // we wrote any bits we did not use as well
    }

    totalBitsWritten += updateStatBits(current_bit_offset, stat);
    return totalBitsWritten;
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::updateStatBits(size_t& current_bit_offset, std::uint16_t stat)
{
    std::uint32_t* pStatValue = GetStatBuffer(stat);
    if (pStatValue == nullptr)
    {
        return 0;
    }

    // write value
    size_t numBits = V110_BITS_PER_STAT[stat];
    return updateBits(current_bit_offset, numBits, *pStatValue);
}
//---------------------------------------------------------------------------
size_t d2ce::CharacterStats::writeBufferBits(std::FILE* charfile)
{
    std::fseek(charfile, stats_location, SEEK_SET);
    std::fwrite(&data[0], data.size(), 1, charfile);
    return data.size() * 8;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::writeStats_109(std::FILE* charfile)
{
    std::uint16_t value = StatInfo.bits();
    std::fwrite(&value, sizeof(value), 1, charfile);

    // skip null byte if character version is less than 1.09
    if (Cs.Version < EnumCharVersion::v109)
    {
        std::fwrite(&nullByte, sizeof(nullByte), 1, charfile);
    }

    // write basic stats
    std::fwrite(&Cs.Strength, sizeof(Cs.Strength), 1, charfile);   // always present so no need to check StatInfo bits
    std::fwrite(&Cs.Energy, sizeof(Cs.Energy), 1, charfile);       // always present so no need to check StatInfo bits
    std::fwrite(&Cs.Dexterity, sizeof(Cs.Dexterity), 1, charfile); // always present so no need to check StatInfo bits
    std::fwrite(&Cs.Vitality, sizeof(Cs.Vitality), 1, charfile);   // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::StatPoints) == EnumCharStatInfo::StatPoints)
    {
        std::fwrite(&Cs.StatsLeft, sizeof(Cs.StatsLeft), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::SkillChoices) == EnumCharStatInfo::SkillChoices)
    {
        std::fwrite(&Cs.SkillChoices, sizeof(Cs.SkillChoices), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::CurLife) == EnumCharStatInfo::CurLife)
    {
        std::fwrite(&Cs.CurLife, sizeof(Cs.CurLife), 1, charfile);
    }

    std::fwrite(&Cs.MaxLife, sizeof(Cs.MaxLife), 1, charfile);       // always present so no need to check StatInfo bits
    std::fwrite(&Cs.CurMana, sizeof(Cs.CurMana), 1, charfile);       // always present so no need to check StatInfo bits
    std::fwrite(&Cs.MaxMana, sizeof(Cs.MaxMana), 1, charfile);       // always present so no need to check StatInfo bits
    std::fwrite(&Cs.CurStamina, sizeof(Cs.CurStamina), 1, charfile); // always present so no need to check StatInfo bits
    std::fwrite(&Cs.MaxStamina, sizeof(Cs.MaxStamina), 1, charfile); // always present so no need to check StatInfo bits
    std::fwrite(&Cs.Level, sizeof(Cs.Level), 1, charfile);           // always present so no need to check StatInfo bits

    if ((StatInfo & EnumCharStatInfo::Experience) == EnumCharStatInfo::Experience)
    {
        std::fwrite(&Cs.Experience, sizeof(Cs.Experience), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::GoldInBelt) == EnumCharStatInfo::GoldInBelt)
    {
        std::fwrite(&Cs.GoldInBelt, sizeof(Cs.GoldInBelt), 1, charfile);
    }

    if ((StatInfo & EnumCharStatInfo::GoldInStash) == EnumCharStatInfo::GoldInStash)
    {
        std::fwrite(&Cs.GoldInStash, sizeof(Cs.GoldInStash), 1, charfile);
    }
    std::fflush(charfile);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readStats(EnumCharVersion version, std::FILE* charfile)
{
    Cs.Version = version;
    update_locations = stats_location == 0 ? true : false;
    if (!readStats(charfile) || stats_location == 0)
    {
        return false;
    }

    update_locations = false;
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::writeStats(std::FILE* charfile)
{
    if (stats_location == 0)
    {
        return false;
    }

    checkStatInfo();

    std::fseek(charfile, stats_location - sizeof(STATS_MARKER), SEEK_SET);
    std::fwrite(STATS_MARKER, sizeof(STATS_MARKER), 1, charfile);
    if (Cs.Version < EnumCharVersion::v110)
    {
        return writeStats_109(charfile);
    }

    size_t current_bit_offset = 0;
    data.clear();

    size_t totalBitsWritten = 0;
    for (uint16_t stat = 0; stat < STAT_MAX; ++stat)
    {
        totalBitsWritten += updateStat(charfile, current_bit_offset, stat);
    }
    totalBitsWritten += updateStat(charfile, current_bit_offset, STAT_END_MARKER);

    std::fflush(charfile);
    return totalBitsWritten > 0 ? true : false;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getHeaderLocation()
{
    return stats_location >= 2 ? stats_location - 2 : 0;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::clear()
{
    StatInfo = EnumCharStatInfo::All;
    Cs.Strength = 0;
    Cs.Energy = 0;
    Cs.Dexterity = 0;
    Cs.Vitality = 0;
    Cs.StatsLeft = 0;
    Cs.SkillChoices = 0;
    Cs.CurLife = 0;
    Cs.MaxLife = 0;
    Cs.CurMana = 0;
    Cs.MaxMana = 0;
    Cs.CurStamina = 0;
    Cs.MaxStamina = 0;
    Cs.Level = 1;
    Cs.Experience = 0;
    Cs.GoldInBelt = 0;
    Cs.GoldInStash = 0;
    data.clear();
    stats_location = 0;
    update_locations = true;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::clearSkillChoices()
{
    Cs.SkillChoices = 0;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::fillCharacterStats(CharStats& cs)
{
    std::memcpy(&cs, &Cs, sizeof(cs));
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateCharacterStats(const CharStats& cs)
{
    auto version = Cs.Version; // don't allow version to change
    std::memcpy(&Cs, &cs, sizeof(CharStats));
    Cs.Version = version;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getLevel() const
{
    return Cs.Level;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getExperience() const
{
    return Cs.Experience;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getSkillChoices() const
{
    return Cs.SkillChoices;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxGoldInBelt() const
{
    return Cs.getMaxGoldInBelt();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxGoldInStash() const
{
    return Cs.getMaxGoldInStash();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinExperienceRequired() const
{
    return Cs.getMinExperienceRequired();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getNextExperienceLevel() const
{
    return Cs.getNextExperienceLevel();
}
//---------------------------------------------------------------------------
