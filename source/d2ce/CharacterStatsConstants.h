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

namespace d2ce
{
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
    constexpr std::uint32_t SORCERESS_STRENGTH_MIN = 25;
    constexpr std::uint32_t AMAZON_STRENGTH_MIN = 20;
    constexpr std::uint32_t ASSASSIN_STRENGTH_MIN = 20;
    constexpr std::uint32_t NECROMANCER_STRENGTH_MIN = 15;
    constexpr std::uint32_t PALADIN_STRENGTH_MIN = 15;
    constexpr std::uint32_t DRUID_STRENGTH_MIN = 15;

    constexpr std::uint32_t GOLD_IN_BELT_LIMIT = 990000;
    constexpr std::uint32_t GOLD_IN_STASH_LIMIT = 2500000;

    // 98 skill points for leveling up 1-99. Additional 4 per difficulty from quests
    constexpr std::uint32_t MAX_SKILL_CHOICES_EARNED = 110;

    //  495 stat points for leveling up 1-99. Additional 5 per difficulty from quests
    constexpr std::uint32_t MAX_STAT_POINTS = 510;
}
//---------------------------------------------------------------------------
