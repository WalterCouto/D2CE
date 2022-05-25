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

#pragma once

#include "d2ce/Constants.h"
#include "d2ce/ExperienceConstants.h"
#include "d2ce/CharacterStatsConstants.h"

constexpr COLORREF EDITED_COLOUR = 0x00FFFF00;
const COLORREF NORMAL_COLOUR = GetSysColor(COLOR_WINDOW);

constexpr std::uint32_t NORMAL = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Normal);
constexpr std::uint32_t NIGHTMARE = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Nightmare);
constexpr std::uint32_t HELL = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Hell);

static const CString SettingsSection("Settings");
static const CString BackupCharacterOption("Backup Character");
//---------------------------------------------------------------------------
