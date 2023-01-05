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
        const std::string& GetCraftModsTxt()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"index\trecipe\tversion\tplvl\tilvl\ttype\tcode\t*ItemName\tprop1\tpar1\tmin1\tmax1\tprop2\tpar2\tmin2\tmax2\tprop3\tpar3\tmin3\tmax3\tprop4\tpar4\tmin4\tmax4\t*eol\n";
                ss << u8"Hit Power Helm\tHit Power\t100\t50\t50\thelm\tfhl\tFull Helm\tgethit-skill\t44\t5\t4\tthorns\t\t3\t7\tac-miss\t\t25\t50\t\t\t\t\t0\n";
                ss << u8"Hit Power Boots\tHit Power\t100\t50\t50\tboot\tmbt\tChain Boots\tgethit-skill\t44\t5\t4\tthorns\t\t3\t7\tac-hth\t\t25\t60\t\t\t\t\t0\n";
                ss << u8"Hit Power Gloves\tHit Power\t100\t50\t50\tglov\tmgl\tBracers\tgethit-skill\t44\t5\t4\tthorns\t\t3\t7\tknock\t\t1\t1\t\t\t\t\t0\n";
                ss << u8"Hit Power Belt\tHit Power\t100\t50\t50\tbelt\ttbl\tHeavy Belt\tgethit-skill\t44\t5\t4\tthorns\t\t3\t7\tdmg-to-mana\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Hit Power Shield\tHit Power\t100\t50\t50\tshie\tgts\tGothic Shield\tgethit-skill\t44\t5\t4\tthorns\t\t3\t10\tblock\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Hit Power Body\tHit Power\t100\t50\t50\ttors\tfld\tField Plate\tgethit-skill\t44\t5\t4\tthorns\t\t3\t10\tbalance1\t\t10\t20\t\t\t\t\t0\n";
                ss << u8"Hit Power Amulet\tHit Power\t100\t50\t50\tamul\tamu\tAmulet\tgethit-skill\t44\t5\t4\tthorns\t\t3\t10\thowl\t\t5\t15\t\t\t\t\t0\n";
                ss << u8"Hit Power Ring\tHit Power\t100\t50\t50\tring\trin\tRing\tgethit-skill\t44\t5\t4\tthorns\t\t3\t6\tdex\t\t1\t5\t\t\t\t\t0\n";
                ss << u8"Hit Power Weapon\tHit Power\t100\t50\t50\tblun\t\tBlunt\tgethit-skill\t44\t5\t4\tthorns\t\t3\t7\tdmg%\t\t35\t60\t\t\t\t\t0\n";
                ss << u8"Blood Helm\tBlood\t100\t50\t50\thelm\thlm\tHelm\tlifesteal\t\t1\t3\thp\t\t10\t20\tdeadly\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Blood Boots\tBlood\t100\t50\t50\tboot\ttbt\tLight Plate Boots\tlifesteal\t\t1\t3\thp\t\t10\t20\tregen\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Blood Gloves\tBlood\t100\t50\t50\tglov\tvgl\tHeavy Gloves\tlifesteal\t\t1\t3\thp\t\t10\t20\tcrush\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Blood Belt\tBlood\t100\t50\t50\tbelt\tmbl\tBelt\tlifesteal\t\t1\t3\thp\t\t10\t20\topenwounds\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Blood Shield\tBlood\t100\t50\t50\tshie\tspk\tSpiked Shield\tlifesteal\t\t1\t3\thp\t\t10\t20\tthorns\t\t4\t7\t\t\t\t\t0\n";
                ss << u8"Blood Body\tBlood\t100\t50\t50\ttors\tplt\tPlate Mail\tlifesteal\t\t1\t3\thp\t\t10\t20\tdemon-heal\t\t1\t3\t\t\t\t\t0\n";
                ss << u8"Blood Amulet\tBlood\t100\t50\t50\tamul\tamu\tAmulet\tlifesteal\t\t1\t4\thp\t\t10\t20\tmove1\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Blood Ring\tBlood\t100\t50\t50\tring\trin\tRing\tlifesteal\t\t1\t3\thp\t\t10\t20\tstr\t\t1\t5\t\t\t\t\t0\n";
                ss << u8"Blood Weapon\tBlood\t100\t50\t50\taxe\t\tAxe\tlifesteal\t\t1\t4\thp\t\t10\t20\tdmg%\t\t35\t60\t\t\t\t\t0\n";
                ss << u8"Caster Helm\tCaster\t100\t50\t50\thelm\tmsk\tMask\tregen-mana\t\t4\t10\tmana\t\t10\t20\tmanasteal\t\t1\t4\t\t\t\t\t0\n";
                ss << u8"Caster Boots\tCaster\t100\t50\t50\tboot\tlbt\tLeather Boots\tregen-mana\t\t4\t10\tmana\t\t10\t20\tmana%\t\t2\t5\t\t\t\t\t0\n";
                ss << u8"Caster Gloves\tCaster\t100\t50\t50\tglov\tlgl\tGloves\tregen-mana\t\t4\t10\tmana\t\t10\t20\tmana-kill\t\t1\t3\t\t\t\t\t0\n";
                ss << u8"Caster Belt\tCaster\t100\t50\t50\tbelt\tvbl\tLight Belt\tregen-mana\t\t4\t10\tmana\t\t10\t20\tcast1\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Caster Shield\tCaster\t100\t50\t50\tshie\tsml\tSmall Shield\tregen-mana\t\t4\t10\tmana\t\t10\t20\tblock\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Caster Body\tCaster\t100\t50\t50\ttors\tltp\tLight Plate\tregen-mana\t\t4\t10\tmana\t\t10\t20\tmana-kill\t\t1\t3\t\t\t\t\t0\n";
                ss << u8"Caster Amulet\tCaster\t100\t50\t50\tamul\tamu\tAmulet\tregen-mana\t\t4\t10\tmana\t\t10\t20\tcast1\t\t5\t10\t\t\t\t\t0\n";
                ss << u8"Caster Ring\tCaster\t100\t50\t50\tring\trin\tRing\tregen-mana\t\t4\t10\tmana\t\t10\t20\tenr\t\t1\t5\t\t\t\t\t0\n";
                ss << u8"Caster Weapon\tCaster\t100\t50\t50\trod\t\tStaves And Rods\tregen-mana\t\t4\t10\tmana\t\t10\t20\tmana%\t\t1\t5\t\t\t\t\t0\n";
                ss << u8"Safety Helm\tSafety\t100\t50\t50\thelm\tcrn\tCrown\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\tres-ltng\t\t5\t10\tac%\t\t10\t30\t0\n";
                ss << u8"Safety Boots\tSafety\t100\t50\t50\tboot\thbt\tPlate Boots\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\tres-fire\t\t5\t10\tac%\t\t10\t30\t0\n";
                ss << u8"Safety Gloves\tSafety\t100\t50\t50\tglov\thgl\tGauntlets\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\tres-cold\t\t5\t10\tac%\t\t10\t30\t0\n";
                ss << u8"Safety Belt\tSafety\t100\t50\t50\tbelt\tlbl\tSash\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\tres-pois\t\t5\t10\tac%\t\t10\t30\t0\n";
                ss << u8"Safety Shield\tSafety\t100\t50\t50\tshie\tkit\tKite Shield\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\tres-mag\t\t5\t10\tac%\t\t10\t30\t0\n";
                ss << u8"Safety Body\tSafety\t100\t50\t50\ttors\tbrs\tBreast Plate\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\thalf-freeze\t\t1\t1\tac%\t\t10\t30\t0\n";
                ss << u8"Safety Amulet\tSafety\t100\t50\t50\tamul\tamu\tAmulet\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\tblock\t\t1\t10\t\t\t\t\t0\n";
                ss << u8"Safety Ring\tSafety\t100\t50\t50\tring\trin\tRing\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\tvit\t\t1\t5\t\t\t\t\t0\n";
                ss << u8"Safety Weapon\tSafety\t100\t50\t50\tspea\t\tSpear\tred-dmg\t\t1\t4\tred-mag\t\t1\t2\tac%\t\t5\t10\t\t\t\t\t0\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}