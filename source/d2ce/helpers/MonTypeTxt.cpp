/*
    Diablo II Character Editor
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
#include <sstream>

namespace d2ce
{
    namespace TxtReaderHelper
    {
        const std::string& GetMonType()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"type\tequiv1\tequiv2\tequiv3\tstrplur\telement\t*eol\n";
                ss << u8"\t\t\t\t\t\t0\n";
                ss << u8"undead\t\t\t\t\t\t0\n";
                ss << u8"demon\t\t\t\t\t\t0\n";
                ss << u8"insect\t\t\t\t\t\t0\n";
                ss << u8"human\t\t\t\t\t\t0\n";
                ss << u8"construct\t\t\t\t\t\t0\n";
                ss << u8"lowundead\tundead\t\t\t\t\t0\n";
                ss << u8"highundead\tundead\t\t\t\t\t0\n";
                ss << u8"skeleton\tlowundead\t\t\t\t\t0\n";
                ss << u8"zombie\tlowundead\t\t\t\t\t0\n";
                ss << u8"bighead\tdemon\t\t\tAfflicted\t\t0\n";
                ss << u8"foulcrow\t\t\t\t\t\t0\n";
                ss << u8"fallen\tdemon\t\t\tFallen\t\t0\n";
                ss << u8"brute\t\t\t\tYeti\t\t0\n";
                ss << u8"sandraider\t\t\t\t\t\t0\n";
                ss << u8"wraith\tundead\t\t\t\t\t0\n";
                ss << u8"corruptrogue\tdemon\t\t\t\t\t0\n";
                ss << u8"baboon\t\t\t\t\t\t0\n";
                ss << u8"goatman\tdemon\t\t\t\t\t0\n";
                ss << u8"quillrat\t\t\t\t\t\t0\n";
                ss << u8"sandmaggot\t\t\t\t\t\t0\n";
                ss << u8"clawviper\tdemon\t\t\t\t\t0\n";
                ss << u8"sandleaper\t\t\t\t\t\t0\n";
                ss << u8"pantherwoman\t\t\t\t\t\t0\n";
                ss << u8"swarm\tinsect\t\t\tHellSwarm\t\t0\n";
                ss << u8"scarab\tinsect\t\t\t\t\t0\n";
                ss << u8"mummy\tlowundead\t\t\t\t\t0\n";
                ss << u8"unraveler\thighundead\t\t\t\t\t0\n";
                ss << u8"vulture\t\t\t\t\t\t0\n";
                ss << u8"mosquito\tinsect\t\t\t\t\t0\n";
                ss << u8"willowisp\t\t\t\t\t\t0\n";
                ss << u8"arach\tinsect\t\t\t\t\t0\n";
                ss << u8"thornhulk\t\t\t\t\t\t0\n";
                ss << u8"vampire\thighundead\t\t\t\t\t0\n";
                ss << u8"batdemon\t\t\t\t\t\t0\n";
                ss << u8"fetish\tdemon\t\t\tFetish\t\t0\n";
                ss << u8"blunderbore\tdemon\t\t\t\t\t0\n";
                ss << u8"undeadfetish\tlowundead\tdemon\t\tUndead Fetish\t\t0\n";
                ss << u8"zakarum\t\t\t\t\t\t0\n";
                ss << u8"frogdemon\t\t\t\t\t\t0\n";
                ss << u8"tentacle\t\t\t\t\t\t0\n";
                ss << u8"fingermage\tlowundead\t\t\t\t\t0\n";
                ss << u8"golem\tconstruct\t\t\t\t\t0\n";
                ss << u8"vilekind\tdemon\t\t\t\t\t0\n";
                ss << u8"regurgitator\tdemon\t\t\t\t\t0\n";
                ss << u8"doomknight\tlowundead\t\t\t\t\t0\n";
                ss << u8"councilmember\tdemon\t\t\t\t\t0\n";
                ss << u8"megademon\tdemon\t\t\t\t\t0\n";
                ss << u8"bovine\t\t\t\t\t\t0\n";
                ss << u8"siegebeast\t\t\t\t\t\t0\n";
                ss << u8"snowyeti\t\t\t\tYeti\t\t0\n";
                ss << u8"minion\t\t\t\t\t\t0\n";
                ss << u8"succubus\tdemon\t\t\t\t\t0\n";
                ss << u8"overseer\tdemon\t\t\t\t\t0\n";
                ss << u8"imp\tdemon\t\t\t\t\t0\n";
                ss << u8"frozenhorror\t\t\t\t\t\t0\n";
                ss << u8"bloodlord\tdemon\t\t\t\t\t0\n";
                ss << u8"deathmauler\t\t\t\t\t\t0\n";
                ss << u8"putriddefiler\tdemon\t\t\t\t\t0\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}