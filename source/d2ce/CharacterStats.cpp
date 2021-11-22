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
#include "SkillConstants.h"
//---------------------------------------------------------------------------
namespace d2ce
{
    constexpr std::uint8_t STATS_MARKER[] = { 0x67, 0x66 };              // alternatively "gf"
    constexpr std::uint16_t STAT_MAX = 16;
    constexpr std::uint16_t STAT_END_MARKER = 0x1FF;
    constexpr size_t STAT_BITS = 9;
    constexpr size_t V110_BITS_PER_STAT[] = { 10,10,10,10,10,8,21,21,21,21,21,21,7,32,25,25 };

    constexpr std::uint32_t MIN_START_STATS_POS = 765;

    constexpr std::uint8_t SKILLS_MARKER[] = { 0x69, 0x66 };             // alternatively "if"

    constexpr std::uint32_t BARBARIAN_VITALITY_MIN = 25;
    constexpr std::uint32_t PALADIN_VITALITY_MIN = 25;
    constexpr std::uint32_t DRUID_VITALITY_MIN = 25;
    constexpr std::uint32_t AMAZON_VITALITY_MIN = 20;
    constexpr std::uint32_t ASSASSIN_VITALITY_MIN = 20;
    constexpr std::uint32_t NECROMANCER_VITALITY_MIN = 15;
    constexpr std::uint32_t SORCERESS_VITALITY_MIN = 10;

    constexpr std::uint32_t SORCERESS_ENERGY_MIN = 35;
    constexpr std::uint32_t NECROMANCER_ENERGY_MIN = 25;
    constexpr std::uint32_t ASSASSIN_ENERGY_MIN = 25;
    constexpr std::uint32_t DRUID_ENERGY_MIN = 20;
    constexpr std::uint32_t AMAZON_ENERGY_MIN = 15;
    constexpr std::uint32_t PALADIN_ENERGY_MIN = 15;
    constexpr std::uint32_t BARBARIAN_ENERGY_MIN = 10;

    constexpr std::uint32_t AMAZON_DEXTERITY_MIN = 25;
    constexpr std::uint32_t NECROMANCER_DEXTERITY_MIN = 25;
    constexpr std::uint32_t SORCERESS_DEXTERITY_MIN = 25;
    constexpr std::uint32_t BARBARIAN_DEXTERITY_MIN = 20;
    constexpr std::uint32_t PALADIN_DEXTERITY_MIN = 20;
    constexpr std::uint32_t ASSASSIN_DEXTERITY_MIN = 20;
    constexpr std::uint32_t DRUID_DEXTERITY_MIN = 20;

    constexpr std::uint32_t BARBARIAN_STRENGTH_MIN = 30;
    constexpr std::uint32_t AMAZON_STRENGTH_MIN = 20;
    constexpr std::uint32_t ASSASSIN_STRENGTH_MIN = 20;
    constexpr std::uint32_t PALADIN_STRENGTH_MIN = 25;
    constexpr std::uint32_t NECROMANCER_STRENGTH_MIN = 15;
    constexpr std::uint32_t DRUID_STRENGTH_MIN = 15;
    constexpr std::uint32_t SORCERESS_STRENGTH_MIN = 10;

    // 98 skill points for leveling up 1-99. Additional 4 per difficulty from quests
    constexpr std::uint32_t MAX_SKILL_CHOICES_EARNED = 110;

    //  495 stat points for leveling up 1-99. Additional 5 per difficulty from quests
    constexpr std::uint32_t MAX_STAT_POINTS = 510;
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
void d2ce::CharacterStats::updateMinStats()
{
    std::uint32_t curLevel = std::min(std::max(Cs.Level, std::uint32_t(1)), NUM_OF_LEVELS);
    std::uint32_t curVitality = std::min(Cs.Vitality, MAX_BASICSTATS);
    std::uint32_t curEnergy = std::min(Cs.Energy, MAX_BASICSTATS);
    switch (Class)
    {
    case d2ce::EnumCharClass::Amazon:
        curVitality = std::max(curVitality, AMAZON_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, AMAZON_ENERGY_MIN); // start stat
        min_hit_points = 0x3200 + ((curLevel - 1) << 8) * 2 + (((curVitality - AMAZON_VITALITY_MIN) << 8) * 3);
        min_stamina = 0x5400 + ((curLevel + curVitality - AMAZON_VITALITY_MIN - 1) << 8);
        min_mana = 0x0F00 + std::uint32_t(((curLevel + curEnergy - AMAZON_ENERGY_MIN - 1) << 8) * 1.5);
        break;

    case d2ce::EnumCharClass::Assassin:
        curVitality = std::max(curVitality, d2ce::ASSASSIN_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::ASSASSIN_ENERGY_MIN); // start stat
        min_hit_points = 0x5400 + ((curLevel - 1) << 8) * 2 + (((curVitality - d2ce::ASSASSIN_VITALITY_MIN) << 8) * 3);
        min_stamina = 0x5F00 + ((curLevel - 1) << 8) + std::uint32_t((((curVitality - d2ce::ASSASSIN_VITALITY_MIN) << 8) * 1.25));
        min_mana = 0x1900 + std::uint32_t(((curLevel - 1) << 8) * 1.5) + std::uint32_t((((curEnergy - d2ce::ASSASSIN_ENERGY_MIN) << 8) * 1.75));
        break;

    case d2ce::EnumCharClass::Sorceress:
        curVitality = std::max(curVitality, d2ce::SORCERESS_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::SORCERESS_ENERGY_MIN); // start stat
        min_hit_points = 0x5400 + ((curLevel - 1) << 8) + (((curVitality - d2ce::SORCERESS_VITALITY_MIN) << 8) * 2);
        min_stamina = 0x4A00 + ((curLevel + curVitality - d2ce::SORCERESS_VITALITY_MIN - 1) << 8);
        min_mana = 0x2300 + ((curLevel + curEnergy - d2ce::SORCERESS_ENERGY_MIN - 1) << 8) * 2;
        break;

    case d2ce::EnumCharClass::Barbarian:
        curVitality = std::max(curVitality, d2ce::BARBARIAN_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::BARBARIAN_ENERGY_MIN); // start stat
        min_hit_points = 0x3700 + ((curLevel - 1) << 8) * 2 + (((curVitality - d2ce::BARBARIAN_VITALITY_MIN) << 8) * 4);
        min_stamina = 0x5C00 + ((curLevel + curVitality - d2ce::BARBARIAN_VITALITY_MIN - 1) << 8);
        min_mana = 0x0A00 + ((curLevel + curEnergy - d2ce::BARBARIAN_ENERGY_MIN - 1) << 8);
        break;

    case d2ce::EnumCharClass::Druid:
        curVitality = std::max(curVitality, d2ce::DRUID_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::DRUID_ENERGY_MIN); // start stat
        min_hit_points = 0x3700 + std::uint32_t((((curLevel - 1) << 8) * 1.5)) + (((curVitality - d2ce::DRUID_VITALITY_MIN) << 8) * 2);
        min_stamina = 0x5400 + ((curLevel + curVitality - d2ce::DRUID_VITALITY_MIN - 1) << 8);
        min_mana = 0x0A00 + ((curLevel - 1) << 8) + (((curEnergy - d2ce::DRUID_ENERGY_MIN) << 8) * 2);
        break;

    case d2ce::EnumCharClass::Necromancer:
        curVitality = std::max(curVitality, d2ce::NECROMANCER_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::NECROMANCER_ENERGY_MIN); // start stat
        min_hit_points = 0x2D00 + std::uint32_t((((curLevel - 1) << 8) * 1.5)) + (((curVitality - d2ce::NECROMANCER_VITALITY_MIN) << 8) * 2);
        min_stamina = 0x4F00 + ((curLevel + curVitality - d2ce::NECROMANCER_VITALITY_MIN - 1) << 8);
        min_mana = 0x1900 + ((curLevel + curEnergy - d2ce::NECROMANCER_ENERGY_MIN - 1) << 8) * 2;
        break;

    case d2ce::EnumCharClass::Paladin:
        curVitality = std::max(curVitality, d2ce::PALADIN_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::PALADIN_ENERGY_MIN); // start stat
        min_hit_points = 0x3700 + ((curLevel - 1) << 8) * 2 + (((curVitality - d2ce::PALADIN_VITALITY_MIN) << 8) * 3);
        min_stamina = 0x5900 + ((curLevel + curVitality - d2ce::PALADIN_VITALITY_MIN - 1) << 8);
        min_mana = 0x0F00 + std::uint32_t(((curLevel + curEnergy - d2ce::PALADIN_ENERGY_MIN - 1) << 8) * 1.5);
        break;
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateStartStats()
{
    switch (Class)
    {
    case d2ce::EnumCharClass::Amazon:
        min_vitality = AMAZON_VITALITY_MIN;
        min_energy = AMAZON_ENERGY_MIN;
        min_dexterity = AMAZON_DEXTERITY_MIN;
        min_strength = AMAZON_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Assassin:
        min_vitality = ASSASSIN_VITALITY_MIN;
        min_energy = ASSASSIN_ENERGY_MIN;
        min_dexterity = ASSASSIN_DEXTERITY_MIN;
        min_strength = ASSASSIN_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Sorceress:
        min_vitality = SORCERESS_VITALITY_MIN;
        min_energy = SORCERESS_ENERGY_MIN;
        min_dexterity = SORCERESS_DEXTERITY_MIN;
        min_strength = SORCERESS_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Barbarian:
        min_vitality = BARBARIAN_VITALITY_MIN;
        min_energy = BARBARIAN_ENERGY_MIN;
        min_dexterity = BARBARIAN_DEXTERITY_MIN;
        min_strength = BARBARIAN_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Druid:
        min_vitality = DRUID_VITALITY_MIN;
        min_energy = DRUID_ENERGY_MIN;
        min_dexterity = DRUID_DEXTERITY_MIN;
        min_strength = DRUID_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Necromancer:
        min_vitality = NECROMANCER_VITALITY_MIN;
        min_energy = NECROMANCER_ENERGY_MIN;
        min_dexterity = NECROMANCER_DEXTERITY_MIN;
        min_strength = NECROMANCER_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Paladin:
        min_vitality = PALADIN_VITALITY_MIN;
        min_energy = PALADIN_ENERGY_MIN;
        min_dexterity = PALADIN_DEXTERITY_MIN;
        min_strength = PALADIN_STRENGTH_MIN;
        break;
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateSkillChoices(std::uint16_t skillPointsEarned)
{
    if ((Cs.SkillChoices > 0) && areSkillsMaxed())
    {
        Cs.SkillChoices = 0;
    }
    else
    {
        std::uint32_t curLevel = std::min(std::max(Cs.Level, std::uint32_t(1)), NUM_OF_LEVELS);
        std::uint32_t earnedPoints = (curLevel - 1) + std::min((curLevel - 1) + skillPointsEarned, MAX_SKILL_CHOICES_EARNED);
        std::uint32_t curPoints = getSkillPointsUsed();
        if ((curPoints + Cs.SkillChoices) < earnedPoints)
        {
            Cs.SkillChoices = std::min(earnedPoints - curPoints, MAX_SKILL_CHOICES);
        }
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateLifePointsEarned(std::uint16_t lifePointsEarned)
{
    updateMinStats();

    // you get +20 Life for completing Act 3 Quest 1 (The Golden Bird) for each difficulty level
    min_hit_points += lifePointsEarned << 8;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updatePointsEarned(std::uint16_t lifePointsEarned, std::uint16_t statPointEarned, std::uint16_t skillPointsEarned)
{
    auto old_min_hit_points = min_hit_points;
    updateLifePointsEarned(lifePointsEarned);
    if (min_hit_points != old_min_hit_points && Cs.MaxLife < min_hit_points)
    {
        bool updateCur = Cs.CurLife == Cs.MaxLife ? true : false;
        Cs.MaxLife = min_hit_points;
        if (updateCur)
        {
            Cs.CurLife = Cs.MaxLife;
        }
    }

    std::uint32_t curStatPointsUsed = getStatPointsUsed();
    std::uint32_t curLevel = std::min(std::max(Cs.Level, std::uint32_t(1)), NUM_OF_LEVELS);
    std::uint32_t earnedStatPoints = std::min((curLevel - 1) * 5 + statPointEarned + getTotalStartStatPoints(), MAX_STAT_POINTS);
    if ((curStatPointsUsed + Cs.StatsLeft) < earnedStatPoints)
    {
        Cs.StatsLeft = std::min(earnedStatPoints - curStatPointsUsed, MAX_BASICSTATS);
    }

    updateSkillChoices(skillPointsEarned);
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
d2ce::EnumCharStatInfo d2ce::CharacterStats::GetStatInfoMask(std::uint16_t stat) const
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
std::uint32_t* d2ce::CharacterStats::GetStatBuffer(std::uint16_t stat)
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
size_t d2ce::CharacterStats::readNextStat(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t& stat)
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
size_t d2ce::CharacterStats::readStatBits(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t stat)
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
        if (Version >= EnumCharVersion::v109)
        {
            if (cur_pos < (long)MIN_START_STATS_POS)
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

    updateStartStats();
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

    if (Version < EnumCharVersion::v110)
    {
        return readStats_109(charfile);
    }

    size_t current_bit_offset = 0;
    data.clear();

    std::uint16_t stat = 0;
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

    updateMinStats();
    return totalBitsRead > 0 ? true : false;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readStats_109(std::FILE* charfile)
{
    std::uint16_t value;
    std::fread(&value, sizeof(value), 1, charfile);
    StatInfo = static_cast<EnumCharStatInfo>(value);

    // skip null byte if character version is less than 1.09
    if (Version < EnumCharVersion::v109)
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

    updateMinStats();
    return true;
}
bool d2ce::CharacterStats::readSkills(std::FILE* charfile)
{
    if (update_locations)
    {
        // find stats location
        skills_location = 0;
        std::uint8_t value = 0;
        auto cur_pos = std::ftell(charfile);
        if (cur_pos < (long)stats_location)
        {
            cur_pos = stats_location;
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
size_t d2ce::CharacterStats::updateStat(std::FILE* charfile, size_t& current_bit_offset, std::uint16_t stat)
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
    if (Version < EnumCharVersion::v109)
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
bool d2ce::CharacterStats::writeSkills(std::FILE* charfile)
{
    std::fwrite(SKILLS_MARKER, sizeof(SKILLS_MARKER), 1, charfile);
    skills_location = std::ftell(charfile);
    std::fwrite(Skills, sizeof(Skills), 1, charfile);
    std::fflush(charfile);
    return true;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::readStats(EnumCharVersion version, EnumCharClass charClass, std::FILE* charfile)
{
    Version = version;
    Class = charClass;
    update_locations = stats_location == 0 ? true : false;
    if (!readStats(charfile) || stats_location == 0)
    {
        return false;
    }


    if (!readSkills(charfile) || skills_location == 0)
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
    if (Version < EnumCharVersion::v110)
    {
        return writeStats_109(charfile);
    }

    size_t current_bit_offset = 0;
    data.clear();

    size_t totalBitsWritten = 0;
    for (std::uint16_t stat = 0; stat < STAT_MAX; ++stat)
    {
        totalBitsWritten += updateStat(charfile, current_bit_offset, stat);
    }
    totalBitsWritten += updateStat(charfile, current_bit_offset, STAT_END_MARKER);

    std::fflush(charfile);
    if (totalBitsWritten == 0)
    {
        return false;
    }

    return writeSkills(charfile);
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getHeaderLocation()
{
    return stats_location >= 2 ? stats_location - 2 : 0;
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::resetStats(std::uint16_t lifePointsEarned, std::uint16_t statPointEarned, std::uint16_t skillPointsEarned)
{
    Cs.Vitality = min_vitality;
    Cs.Energy = min_energy;
    Cs.Dexterity = min_dexterity;
    Cs.Strength = min_strength;
    Cs.StatsLeft = 0;

    updateLifePointsEarned(lifePointsEarned);
    Cs.MaxLife = min_hit_points;
    Cs.CurLife = Cs.MaxLife;
    Cs.MaxStamina = min_stamina;
    Cs.CurStamina = Cs.MaxStamina;
    Cs.MaxMana = min_mana;
    Cs.CurMana = Cs.MaxMana;

    std::uint32_t curLevel = std::min(std::max(Cs.Level, std::uint32_t(1)), NUM_OF_LEVELS);
    Cs.StatsLeft = (curLevel - 1) * 5 + statPointEarned;

    resetSkills(skillPointsEarned);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateSkills(const std::uint8_t(&updated_skills)[NUM_OF_SKILLS], std::uint16_t skillPointsEarned)
{
    for (size_t i = 0; i < NUM_OF_SKILLS; ++i)
    {
        Skills[i] = std::min(updated_skills[i], MAX_SKILL_VALUE);
    }
    updateSkillChoices(skillPointsEarned);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::resetSkills(std::uint16_t skillPointsEarned)
{
    std::memset(Skills, 0, sizeof(Skills));
    updateSkillChoices(skillPointsEarned);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::updateClass(EnumCharClass charClass)
{
    if (charClass == Class)
    {
        return;
    }

    Class = charClass;

    auto oldVitality = Cs.Vitality;
    auto oldEnergy = Cs.Energy;
    auto oldMaxLife = Cs.MaxLife;
    auto oldMaxStamina = Cs.MaxStamina;
    auto oldMaxMana = Cs.MaxMana;

    updateStartStats();
    Cs.Vitality = std::min(std::max(Cs.Vitality, min_vitality), MAX_BASICSTATS);
    Cs.Energy = std::min(std::max(Cs.Energy, min_energy), MAX_BASICSTATS);
    Cs.Dexterity = std::min(std::max(Cs.Dexterity, min_dexterity), MAX_BASICSTATS);
    Cs.Strength = std::min(std::max(Cs.Strength, min_strength), MAX_BASICSTATS);

    updateMinStats();
    if (oldVitality != Cs.Vitality || oldMaxLife > Cs.MaxLife)
    {
        bool updateCur = Cs.CurLife == Cs.MaxLife ? true : false;
        Cs.MaxLife = std::max(Cs.MaxLife, min_hit_points);
        if (updateCur)
        {
            Cs.CurLife = Cs.MaxLife;
        }
    }

    if (oldVitality != Cs.Vitality || oldMaxStamina > Cs.MaxStamina)
    {
        bool updateCur = Cs.CurStamina == Cs.MaxStamina ? true : false;
        Cs.MaxStamina = std::max(Cs.MaxStamina, min_stamina);
        if (updateCur)
        {
            Cs.CurStamina = Cs.MaxStamina;
        }
    }

    if (oldEnergy != Cs.Energy || oldMaxMana > Cs.MaxMana)
    {
        bool updateCur = Cs.CurMana == Cs.MaxMana ? true : false;
        Cs.MaxMana = std::max(Cs.MaxMana, min_mana);
        if (updateCur)
        {
            Cs.CurMana = Cs.MaxMana;
        }
    }
}
//---------------------------------------------------------------------------
std::string d2ce::CharacterStats::getAttributeJsonName(std::uint16_t stat, bool bSerializedFormat) const
{
    switch (stat)
    {
    case 0:
        return "strength";
    case 1:
        return "energy";
    case 2:
        return "dexterity";
    case 3:
        return "vitality";
    case 4:
        return bSerializedFormat ? "statpts" : "unused_stats";
    case 5:
        return bSerializedFormat ? "newskills" : "unused_skill_points";
    case 6:
        return bSerializedFormat ? "hitpoints" : "current_hp";
    case 7:
        return bSerializedFormat ? "maxhp" : "max_hp";
    case 8:
        return bSerializedFormat ? "mana" : "current_mana";
    case 9:
        return bSerializedFormat ? "maxmana" : "max_mana";
    case 10:
        return bSerializedFormat ? "stamina" : "current_stamina";
    case 11:
        return bSerializedFormat ? "maxstamina" : "max_stamina";
    case 12:
        return "level";
    case 13:
        return "experience";
    case 14:
        return "gold";
    case 15:
        return bSerializedFormat ? "goldbank" : "stashed_gold";
    }

    return "";
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::attributesAsJson(std::stringstream& ss, const std::string& parentIndent, bool bSerializedFormat) const
{
    (const_cast<CharacterStats*>(this))->checkStatInfo();
    if (bSerializedFormat)
    {
        std::string attribParentIndent = parentIndent + jsonIndentStr;
        std::string attribStatsParentIndent = attribParentIndent + jsonIndentStr;
        ss << "\n" << parentIndent << "\"Attributes\": {";
        ss << "\n" << attribParentIndent << "\"Header\": " << std::dec << (std::uint16_t) * ((std::uint16_t*)STATS_MARKER);
        ss << ",\n" << attribParentIndent << "\"Stats\": {";

        bool bFirstItem = true;
        std::uint32_t* pStatValue = nullptr;
        std::uint32_t statValue = 0;
        for (std::uint16_t stat = 0; stat < STAT_MAX; ++stat)
        {
            pStatValue = (const_cast<CharacterStats*>(this))->GetStatBuffer(stat);
            if (pStatValue == nullptr)
            {
                continue;
            }

            if (bFirstItem)
            {
                bFirstItem = false;
            }
            else
            {
                ss << ",";
            }

            statValue = *pStatValue;
            switch (stat)
            {
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                statValue >>= 8;
                break;
            }
            ss << "\n" << attribStatsParentIndent << "\"" << getAttributeJsonName(stat, bSerializedFormat) << "\": " << std::dec << statValue;
        }
        ss << "\n" << attribParentIndent << "}";
        ss << "\n" << parentIndent << "}";
    }
    else
    {
        ss << "\n" << parentIndent << "\"attributes\": {";

        bool bFirstItem = true;
        std::uint32_t* pStatValue = nullptr;
        std::uint32_t statValue = 0;
        EnumCharStatInfo mask = EnumCharStatInfo::All;
        for (std::uint16_t stat = 0; stat < STAT_MAX; ++stat)
        {
            mask = GetStatInfoMask(stat);
            pStatValue = (const_cast<CharacterStats*>(this))->GetStatBuffer(stat);
            if (pStatValue == nullptr || ((StatInfo & mask) != mask))
            {
                continue;
            }

            if (bFirstItem)
            {
                bFirstItem = false;
            }
            else
            {
                ss << ",";
            }

            statValue = *pStatValue;
            switch (stat)
            {
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                statValue >>= 8;
                break;
            }
            ss << "\n" << parentIndent << jsonIndentStr << "\"" << getAttributeJsonName(stat, bSerializedFormat) << "\": " << std::dec << statValue;
        }
        ss << "\n" << parentIndent << "}";
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::skillsAsJson(std::stringstream& ss, const std::string& parentIndent, bool bSerializedFormat) const
{
    std::string skillsParentIndent = parentIndent + jsonIndentStr;
    std::string skillListParentIndent = skillsParentIndent + jsonIndentStr;
    if (bSerializedFormat)
    {
        std::string skillListAttribIndent = skillListParentIndent + jsonIndentStr;
        ss << "\n" << parentIndent << "\"ClassSkills\": {";
        ss << "\n" << skillsParentIndent << "\"Header\": " << std::dec << (std::uint16_t) * ((std::uint16_t*)SKILLS_MARKER);
        ss << ",\n" << skillsParentIndent << "\"Skills\": [";
        for (std::uint32_t skill = 0; skill < NUM_OF_SKILLS; ++skill)
        {
            if (skill != 0)
            {
                ss << ",";
            }

            ss << "\n" << skillListParentIndent << "{";
            ss << "\n" << skillListAttribIndent << "\"Id\": " << std::dec << std::uint16_t(getSkillId(skill));
            ss << ",\n" << skillListAttribIndent << "\"Points\": " << std::dec << std::uint16_t(getSkillPoints(skill));
            ss << "\n" << skillsParentIndent << "}";
        }
        ss << "\n" << skillsParentIndent << "]";
        ss << "\n" << parentIndent << "}";
    }
    else
    {
        ss << "\n" << parentIndent << "\"skills\": [";
        for (std::uint32_t skill = 0; skill < NUM_OF_SKILLS; ++skill)
        {
            if (skill != 0)
            {
                ss << ",";
            }

            ss << "\n" << skillsParentIndent << "{";
            ss << "\n" << skillListParentIndent << "\"id\": " << std::dec << std::uint16_t(getSkillId(skill));
            ss << ",\n" << skillListParentIndent << "\"points\": " << std::dec << std::uint16_t(getSkillPoints(skill));
            ss << ",\n" << skillListParentIndent << "\"name\": \"" << getSkillName(skill) << "\"";
            ss << "\n" << skillsParentIndent << "}";
        }
        ss << "\n" << jsonIndentStr << "]";
    }
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::asJson(std::stringstream& ss, const std::string& parentIndent, bool bSerializedFormat) const
{
    attributesAsJson(ss, parentIndent, bSerializedFormat);
    ss << ",";
    skillsAsJson(ss, parentIndent, bSerializedFormat);
}
//---------------------------------------------------------------------------
void d2ce::CharacterStats::clear()
{
    Class = EnumCharClass::Amazon;
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
    std::memset(Skills, 0, sizeof(Skills));
    stats_location = 0;
    update_locations = true;

    min_hit_points = 0x100;
    min_stamina = 0x100;
    min_mana = 0x100;
    min_vitality = 1;
    min_energy = 1;
    min_dexterity = 1;
    min_strength = 1;
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
    auto oldLevel = Cs.Level;
    auto oldVitality = Cs.Vitality;
    auto oldEnergy = Cs.Energy;
    auto oldMaxLife = Cs.MaxLife;
    auto oldMaxStamina = Cs.MaxStamina;
    auto oldMaxMana = Cs.MaxMana;
    std::memcpy(&Cs, &cs, sizeof(CharStats));

    // make sure level is correct
    Cs.Level = std::min(std::max(Cs.Level, std::uint32_t(1)), NUM_OF_LEVELS);

    // make sure gold is correct
    Cs.GoldInBelt = std::min(Cs.GoldInBelt, Cs.getMaxGoldInBelt());
    Cs.GoldInStash = std::min(Cs.GoldInStash, Cs.getMaxGoldInStash());

    // Check min stats for chartacter
    updateStartStats();
    Cs.Vitality = std::min(std::max(Cs.Vitality, min_vitality), MAX_BASICSTATS);
    Cs.Energy = std::min(std::max(Cs.Energy, min_energy), MAX_BASICSTATS);
    Cs.Dexterity = std::min(std::max(Cs.Dexterity, min_dexterity), MAX_BASICSTATS);
    Cs.Strength = std::min(std::max(Cs.Strength, min_strength), MAX_BASICSTATS);

    updateMinStats();
    if (oldLevel != Cs.Level || oldVitality != Cs.Vitality || oldMaxLife > Cs.MaxLife)
    {
        bool updateCur = Cs.CurLife == Cs.MaxLife ? true : false;
        Cs.MaxLife = std::max(Cs.MaxLife, min_hit_points);
        if (updateCur)
        {
            Cs.CurLife = Cs.MaxLife;
        }
    }

    if (oldLevel != Cs.Level || oldVitality != Cs.Vitality || oldMaxStamina > Cs.MaxStamina)
    {
        bool updateCur = Cs.CurStamina == Cs.MaxStamina ? true : false;
        Cs.MaxStamina = std::max(Cs.MaxStamina, min_stamina);
        if (updateCur)
        {
            Cs.CurStamina = Cs.MaxStamina;
        }
    }

    if (oldLevel != Cs.Level || oldEnergy != Cs.Energy || oldMaxMana > Cs.MaxMana)
    {
        bool updateCur = Cs.CurMana == Cs.MaxMana ? true : false;
        Cs.MaxMana = std::max(Cs.MaxMana, min_mana);
        if (updateCur)
        {
            Cs.CurMana = Cs.MaxMana;
        }
    }

    // Check experiene level
    if (oldLevel < Cs.Level)
    {
        Cs.Experience = std::max(Cs.Experience, getMinExperienceRequired());
    }
    else if (oldLevel > Cs.Level)
    {
        Cs.Experience = std::min(Cs.Experience, getNextExperienceLevel() - 1);
    }
    Cs.Experience = std::min(Cs.Experience, MAX_EXPERIENCE);

    // Check Stat Points remaining makes sense
    if (Cs.StatsLeft > 0)
    {
        std::uint32_t curTotalStatPoints = Cs.Strength + Cs.Dexterity + Cs.Vitality + Cs.Energy;
        if (curTotalStatPoints >= 4 * MAX_BASICSTATS)
        {
            Cs.StatsLeft = 0;
        }
        else if (curTotalStatPoints + Cs.StatsLeft > 4 * MAX_BASICSTATS)
        {
            Cs.StatsLeft = 4 * MAX_BASICSTATS - curTotalStatPoints;
        }
    }

    // Check remaining skill points
    if (Cs.SkillChoices > 0)
    {
        std::uint32_t maxSkills = MAX_SKILL_VALUE * NUM_OF_SKILLS;
        std::uint32_t curSkills = getSkillPointsUsed();
        if (curSkills >= maxSkills)
        {
            Cs.SkillChoices = 0;
        }
        else if (curSkills + Cs.SkillChoices > maxSkills)
        {
            Cs.SkillChoices = maxSkills - curSkills;
        }
    }
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
std::uint32_t d2ce::CharacterStats::getMaxGoldInBelt() const
{
    return Cs.getMaxGoldInBelt();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxGoldInStash() const
{
    return Cs.getMaxGoldInStash(Version);
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
std::uint32_t d2ce::CharacterStats::getMinStrength() const
{
    return min_strength;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinEnergy() const
{
    return min_energy;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinDexterity() const
{
    return min_dexterity;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMinVitality() const
{
    return min_vitality;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxHitPoints() const
{
    return min_hit_points;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxStamina() const
{
    return min_stamina;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getMaxMana() const
{
    return min_mana;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getTotalStartStatPoints() const
{
    return min_vitality + min_energy + min_dexterity + min_strength;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getTotalStatPoints() const
{
    return getStatPointsUsed() + getStatLeft();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getStatPointsUsed() const
{
    return Cs.Vitality + Cs.Energy + Cs.Dexterity + Cs.Strength;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getStatLeft() const
{
    return Cs.StatsLeft;
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::CharacterStats::getSkillId(std::uint32_t skill) const
{
    if (skill > NUM_OF_SKILLS)
    {
        return 0;
    }

    size_t idx = START_SKILL_ID;
    size_t class_idx = static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(Class);
    if (class_idx >= 5)
    {
        class_idx -= 5;
        idx = EXPANSION_START_SKILL_ID;
    }
    idx += class_idx * NUM_OF_SKILLS + skill;
    return (std::uint8_t)idx;
}
//---------------------------------------------------------------------------
std::string d2ce::CharacterStats::getSkillNameById(std::uint32_t id) const
{
    if (id >= 0xFF)
    {
        return "";
    }

    auto iter = GenericSkillNames.find(id);
    if (iter != GenericSkillNames.end())
    {
        return iter->second;
    }

    size_t classIdx = NUM_OF_CLASSES;
    size_t skillIdx = NUM_OF_SKILLS;
    if (id <= END_SKILL_ID)
    {
        classIdx = (id - START_SKILL_ID) / NUM_OF_SKILLS;
        skillIdx = (id - START_SKILL_ID) % NUM_OF_SKILLS;
    }
    else if (id >= EXPANSION_START_SKILL_ID && id <= EXPANSION_END_SKILL_ID)
    {
        classIdx = 5 + (id - EXPANSION_START_SKILL_ID) / NUM_OF_SKILLS;
        skillIdx = (id - EXPANSION_START_SKILL_ID) % NUM_OF_SKILLS;
    }

    size_t classNumber = static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(Class);
    if (classIdx != classNumber)
    {
        return "";
    }

    return SkillsNames[classIdx][skillIdx];
}
//---------------------------------------------------------------------------
std::string d2ce::CharacterStats::getSkillName(std::uint32_t skill) const
{
    if (skill >= NUM_OF_SKILLS)
    {
        return "";
    }

    auto classNumber = static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(Class);
    return SkillsNames[classNumber][skill];
}
//---------------------------------------------------------------------------
std::uint8_t d2ce::CharacterStats::getSkillPoints(std::uint32_t skill) const
{
    if (skill >= NUM_OF_SKILLS)
    {
        return 0;
    }

    return Skills[skill];
}
//---------------------------------------------------------------------------
std::uint8_t(&d2ce::CharacterStats::getSkills())[NUM_OF_SKILLS]
{
    return Skills;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getTotalSkillPoints() const
{
    return getSkillPointsUsed() + getSkillChoices();
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getSkillPointsUsed() const
{
    std::uint32_t skillUsed = 0;
    for (std::uint32_t i = 0; i < NUM_OF_SKILLS; ++i)
    {
        skillUsed += Skills[i];
    }

    return skillUsed;
}
//---------------------------------------------------------------------------
std::uint32_t d2ce::CharacterStats::getSkillChoices() const
{
    return Cs.SkillChoices;
}
//---------------------------------------------------------------------------
bool d2ce::CharacterStats::areSkillsMaxed() const
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
void d2ce::CharacterStats::maxSkills()
{
    std::memset(Skills, MAX_SKILL_VALUE, sizeof(Skills));
}
//---------------------------------------------------------------------------