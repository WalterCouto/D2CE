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

#pragma once

#include "d2ce\Constants.h"
#include "d2ce\ExperienceConstants.h"
#include "d2ce\CharacterStatsConstants.h"

constexpr COLORREF EDITED_COLOUR = 0x00FFFF00;
const COLORREF NORMAL_COLOUR = GetSysColor(COLOR_WINDOW);
constexpr std::uint32_t NUM_OF_TITLES = 4;

constexpr std::uint32_t NORMAL = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Normal);
constexpr std::uint32_t NIGHTMARE = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Nightmare);
constexpr std::uint32_t HELL = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Hell);

// titles for regular non-expansion set characters
static const char *FemaleTitle[NUM_OF_TITLES] = {"", "Dame", "Lady", "Baroness"},
                  *MaleTitle[NUM_OF_TITLES]   = {"", "Sir",  "Lord", "Baron"};

// titles for regular expansion set characters
static const char *FemaleExpansionTitle[NUM_OF_TITLES] = {"", "Slayer", "Champion", "Matriarch"},
                  *MaleExpansionTitle[NUM_OF_TITLES]   = {"", "Slayer", "Champion", "Patriarch"};

// titles for hardcore non-expansion set characters
static const char *FemaleHardcoreTitle[NUM_OF_TITLES] = {"", "Countess", "Duchess", "Queen"},
                  *MaleHardcoreTitle[NUM_OF_TITLES]   = {"", "Count",    "Duke",    "King"};

// titles for hardcore expansion set characters
static const char *HardcoreExpansionTitle[NUM_OF_TITLES] = {"", "Destroyer", "Conqueror", "Guardian"};

static const CString SettingsSection("Settings");
static const CString BackupCharacterOption("Backup Character");
//---------------------------------------------------------------------------
