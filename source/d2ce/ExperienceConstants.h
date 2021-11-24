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

#include <cstdint>
#include <array>

namespace d2ce
{
    constexpr std::uint32_t MAX_BASICSTATS = 0x03FF;     // strength, dexterity, vitality, energy max value (10 bits)
    constexpr std::uint32_t MAX_EXPERIENCE = 3600000000; // experience max value (game limit)
    constexpr std::uint32_t MAX_LMS = 0x1FFFFF;          // life, mana, stamina max value (21 bits)
    constexpr std::uint32_t LMS_MASK = 0x1FFF00;         // mask of significant bits of LMS values
    constexpr std::uint32_t MAX_SKILL_CHOICES = 0xFF;    // max # of skill choices left to use

    constexpr std::uint32_t NUM_OF_LEVELS = 99; // max level (game limit)

    // mininum experience for each character level
    constexpr std::array<std::uint32_t, NUM_OF_LEVELS> MinExpRequired =
    { 0, 500, 1500, 3750, 7875, 14175,
     22680, 32886, 44396, 57715, 72144,
     90180, 112725, 140906, 176132, 220165,
     275207, 344008, 430010, 537513, 671891,
     839864, 1049830, 1312287, 1640359, 2050449,
     2563061, 3203826, 3902260, 4663553, 5493363,
     6397855, 7383752, 8458379, 9629723, 10906488,
     12298162, 13815086, 15468534, 17270791, 19235252,
     21376515, 23710491, 26254525, 29027522, 32050088,
     35344686, 38935798, 42850109, 47116709, 51767302,
     56836449, 62361819, 68384473, 74949165, 82104680,
     89904191, 98405658, 107672256, 117772849, 128782495,
     140783010, 153863570, 168121381, 183662396, 200602101,
     219066380, 239192444, 261129853, 285041630, 311105466,
     339515048, 370481492, 404234916, 441026148, 481128591,
     524840254, 572485967, 624419793, 681027665, 742730244,
     809986056, 883294891, 963201521, 1050299747, 1145236814,
     1248718217, 1361512946, 1484459201, 1618470619, 1764543065,
     1923762030, 2097310703, 2286478756, 2492671933, 2717422497,
     2962400612, 3229426756, 3520485254 };
}
//---------------------------------------------------------------------------
