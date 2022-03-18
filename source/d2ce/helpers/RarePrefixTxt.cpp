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

#include "pch.h"
#include <sstream>

namespace d2ce
{
    namespace TxtReaderHelper
    {
        const std::string& GetRarePrefix()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"name\tversion\titype1\titype2\titype3\titype4\titype5\titype6\titype7\tetype1\tetype2\tetype3\tetype4\n";
                ss << u8"Beast\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Eagle\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Raven\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Viper\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"GhoulRI\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Skull\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Blood\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Dread\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Doom\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Grim\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Bone\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Death\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Shadow\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Storm\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Rune\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"PlagueRI\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Stone\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Wraithra\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Spirit\t0\tarmo\trod\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Storm\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Demon\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Cruel\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Empyrion\t0\trod\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Bramble\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Pain\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Loath\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Glyph\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Imp\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Fiendra\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Hailstone\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Gale\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Dire\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Soul\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Brimstone\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Corpse\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Carrion\t0\tarmo\tweap\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Holocaust\t0\ttors\thelm\tshld\tswor\taxe\t\t\t\t\t\t\n";
                ss << u8"Havoc\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Bitter\t0\tarmo\tweap\tmisc\t\t\t\t\t\t\t\t\n";
                ss << u8"Entropy\t0\tring\tamul\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Chaos\t0\tring\tamul\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Order\t0\tring\tamul\tscep\t\t\t\t\t\t\t\t\n";
                ss << u8"Rule\t0\trod\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Warp\t0\t\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Rift\t0\t\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Corruption\t0\tring\tamul\t\t\t\t\t\t\t\t\t\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}