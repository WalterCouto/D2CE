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
        const std::string& GetRareSuffix()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"name\tversion\titype1\titype2\titype3\titype4\titype5\titype6\titype7\tetype1\tetype2\tetype3\tetype4\n";
                ss << u8"bite\t0\tswor\tknif\tspea\tpole\taxe\th2h\t\t\t\t\t\n";
                ss << u8"scratch\t0\tswor\tknif\tspea\tpole\th2h\t\t\t\t\t\t\n";
                ss << u8"scalpel\t0\tswor\tknif\t\t\t\t\t\t\t\t\t\n";
                ss << u8"fang\t0\tswor\tknif\tspea\tpole\t\t\t\t\t\t\t\n";
                ss << u8"gutter\t0\tswor\tknif\tspea\tpole\t\t\t\t\t\t\t\n";
                ss << u8"thirst\t0\tswor\tknif\tspea\tpole\taxe\th2h\t\t\t\t\t\n";
                ss << u8"razor\t0\tswor\tknif\taxe\th2h\t\t\t\t\t\t\t\n";
                ss << u8"scythe\t0\tswor\taxe\tpole\t\t\t\t\t\t\t\t\n";
                ss << u8"edge\t0\tswor\tknif\taxe\t\t\t\t\t\t\t\t\n";
                ss << u8"saw\t0\tswor\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"splitter\t0\taxe\tmace\tclub\thamm\t\t\t\t\t\t\t\n";
                ss << u8"cleaver\t0\tswor\taxe\t\t\t\t\t\t\t\t\t\n";
                ss << u8"sever\t0\tswor\taxe\t\t\t\t\t\t\t\t\t\n";
                ss << u8"sunder\t0\taxe\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"rend\t0\taxe\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"mangler\t0\taxe\tmace\tclub\thamm\t\t\t\t\t\t\t\n";
                ss << u8"slayer\t0\taxe\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"reaver\t0\taxe\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"spawn\t0\taxe\thamm\t\t\t\t\t\t\t\t\t\n";
                ss << u8"gnash\t0\taxe\tclub\thamm\t\t\t\t\t\t\t\t\n";
                ss << u8"star\t0\tmace\thamm\tscep\twand\t\t\t\t\t\t\t\n";
                ss << u8"blow\t0\tmace\tclub\thamm\tscep\t\t\t\t\t\t\t\n";
                ss << u8"smasher\t0\tmace\tclub\thamm\tscep\t\t\t\t\t\t\t\n";
                ss << u8"Bane\t0\tmace\tscep\twand\t\t\t\t\t\t\t\t\n";
                ss << u8"crusher\t0\tmace\tclub\thamm\tscep\t\t\t\t\t\t\t\n";
                ss << u8"breaker\t0\tmace\tclub\thamm\tscep\t\t\t\t\t\t\t\n";
                ss << u8"grinder\t0\tmace\tclub\thamm\tscep\t\t\t\t\t\t\t\n";
                ss << u8"crack\t0\tmace\tclub\thamm\tscep\twand\t\t\t\t\t\t\n";
                ss << u8"mallet\t0\thamm\tclub\t\t\t\t\t\t\t\t\t\n";
                ss << u8"knell\t0\tmace\tclub\tscep\twand\t\t\t\t\t\t\t\n";
                ss << u8"lance\t0\tspea\tpole\t\t\t\t\t\t\t\t\t\n";
                ss << u8"spike\t0\tswor\tknif\tspea\tpole\t\t\t\t\t\t\t\n";
                ss << u8"impaler\t0\tswor\tknif\tspea\tpole\t\t\t\t\t\t\t\n";
                ss << u8"skewer\t0\tswor\tknif\tspea\t\t\t\t\t\t\t\t\n";
                ss << u8"prod\t0\tspea\tpole\t\t\t\t\t\t\t\t\t\n";
                ss << u8"scourge\t0\tspea\tpole\t\t\t\t\t\t\t\t\t\n";
                ss << u8"wand\t0\twand\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"wrack\t0\tspea\tpole\t\t\t\t\t\t\t\t\t\n";
                ss << u8"barb\t0\tswor\tknif\taxe\tspea\tpole\th2h\t\t\t\t\t\n";
                ss << u8"needle\t0\tswor\tknif\tspea\tmiss\t\t\t\t\t\t\t\n";
                ss << u8"dart\t0\tspea\tmiss\t\t\t\t\t\t\t\t\t\n";
                ss << u8"bolt\t0\tmiss\tjave\t\t\t\t\t\t\t\t\t\n";
                ss << u8"quarrel\t0\tmiss\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"fletch\t0\tmiss\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"flight\t0\tmiss\tjave\t\t\t\t\t\t\t\t\t\n";
                ss << u8"nock\t0\tmiss\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"horn\t0\thelm\tmiss\tknif\t\t\t\t\t\t\t\t\n";
                ss << u8"stinger\t0\tswor\tknif\tspea\tmiss\t\t\t\t\t\t\t\n";
                ss << u8"quill\t0\tknif\tmiss\t\t\t\t\t\t\t\t\t\n";
                ss << u8"goad\t0\tspea\tpole\tstaf\t\t\t\t\t\t\t\t\n";
                ss << u8"branch\t0\tspea\tstaf\tbow\t\t\t\t\t\t\t\t\n";
                ss << u8"spire\t0\tstaf\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"song\t0\tweap\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"call\t0\trod\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"cry\t0\trod\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"spell\t0\trod\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"chant\t0\trod\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"weaver\t0\trod\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"gnarl\t0\tclub\twand\tstaf\t\t\t\t\t\t\t\t\n";
                ss << u8"visage\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"crest\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"circlet\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"veil\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"hood\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"mask\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"brow\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"casque\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"visor\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"cowl\t0\thelm\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"hide\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Pelt\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"carapace\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"coat\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"wrap\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"suit\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"cloak\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"shroud\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"jack\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"mantle\t0\ttors\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"guard\t0\tshld\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"badge\t0\tshld\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"rock\t0\tshld\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"aegis\t0\tshld\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"ward\t0\tshld\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"tower\t0\tshld\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"shield\t0\tshld\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"wing\t0\tshld\tamul\t\t\t\t\t\t\t\t\t\n";
                ss << u8"mark\t0\tshld\tamul\t\t\t\t\t\t\t\t\t\n";
                ss << u8"emblem\t0\tshld\tamul\t\t\t\t\t\t\t\t\t\n";
                ss << u8"hand\t0\tglov\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"fist\t0\tglov\th2h\t\t\t\t\t\t\t\t\t\n";
                ss << u8"claw\t0\tglov\th2h\t\t\t\t\t\t\t\t\t\n";
                ss << u8"clutches\t0\tglov\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"grip\t0\tglov\tring\t\t\t\t\t\t\t\t\t\n";
                ss << u8"grasp\t0\tglov\tring\t\t\t\t\t\t\t\t\t\n";
                ss << u8"hold\t0\tglov\tring\t\t\t\t\t\t\t\t\t\n";
                ss << u8"touch\t0\tglov\tring\t\t\t\t\t\t\t\t\t\n";
                ss << u8"finger\t0\tglov\tring\t\t\t\t\t\t\t\t\t\n";
                ss << u8"knuckle\t0\tglov\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"shank\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"spur\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"tread\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"stalker\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"greave\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"blazer\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"nails\t0\tboot\tspea\tpole\t\t\t\t\t\t\t\t\n";
                ss << u8"trample\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Brogues\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"track\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"slippers\t0\tboot\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"clasp\t0\tbelt\tamul\t\t\t\t\t\t\t\t\t\n";
                ss << u8"buckle\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"harness\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"lock\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"fringe\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"winding\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"chain\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"strap\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"lash\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"cord\t0\tbelt\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"knot\t0\tring\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"circle\t0\tring\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"loop\t0\tring\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"eye\t0\tmisc\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"turn\t0\tring\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"spiral\t0\tring\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"coil\t0\tring\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"gyre\t0\tring\torb\tjewl\t\t\t\t\t\t\t\t\n";
                ss << u8"band\t0\tring\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"whorl\t0\tring\torb\tjewl\t\t\t\t\t\t\t\t\n";
                ss << u8"talisman\t0\tamul\tjewl\t\t\t\t\t\t\t\t\t\n";
                ss << u8"heart\t0\tamul\torb\tjewl\t\t\t\t\t\t\t\t\n";
                ss << u8"noose\t0\tamul\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"necklace\t0\tamul\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"collar\t0\tamul\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"beads\t0\tamul\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"torc\t0\tamul\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"gorget\t0\tamul\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"scarab\t0\tamul\tjewl\t\t\t\t\t\t\t\t\t\n";
                ss << u8"wood\t0\tspea\tpole\twand\tstaf\t\t\t\t\t\t\t\n";
                ss << u8"brand\t0\tblun\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"bludgeon\t0\tblun\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"cudgel\t0\tclub\twand\t\t\t\t\t\t\t\t\t\n";
                ss << u8"loom\t0\tmiss\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"harp\t0\tmiss\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"master\t0\tring\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"barRI\t0\tblun\tspea\tpole\t\t\t\t\t\t\t\t\n";
                ss << u8"hew\t0\tswor\tknif\taxe\t\t\t\t\t\t\t\t\n";
                ss << u8"crook\t0\tstaf\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"mar\t0\tswor\tknif\tmace\tclub\thamm\tspea\t\t\t\t\t\n";
                ss << u8"shell\t0\ttors\thelm\tshld\t\t\t\t\t\t\t\t\n";
                ss << u8"stake\t0\tspea\tpole\t\t\t\t\t\t\t\t\t\n";
                ss << u8"picket\t0\tspea\tpole\t\t\t\t\t\t\t\t\t\n";
                ss << u8"pale\t0\tspea\tpole\t\t\t\t\t\t\t\t\t\n";
                ss << u8"flange\t0\ttors\tmace\thamm\tscep\t\t\t\t\t\t\t\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}