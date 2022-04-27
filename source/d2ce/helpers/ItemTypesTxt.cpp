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
        const std::string& GetItemTypes()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"ItemType\tCode\tEquiv1\tEquiv2\tRepair\tBody\tBodyLoc1\tBodyLoc2\tShoots\tQuiver\tThrowable\tReload\tReEquip\tAutoStack\tMagic\tRare\tNormal\tBeltable\tMaxSockets1\tMaxSocketsLevelThreshold1\tMaxSockets2\tMaxSocketsLevelThreshold2\tMaxSockets3\tTreasureClass\tRarity\tStaffMods\tClass\tVarInvGfx\tInvGfx1\tInvGfx2\tInvGfx3\tInvGfx4\tInvGfx5\tInvGfx6\tStorePage\t*eol\n";
                ss << u8"Any\t\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"None\tnone\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Shield\tshie\tshld\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t3\t40\t4\t0\t3\t\t\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Armor\ttors\tarmo\t\t1\t1\ttors\ttors\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Gold\tgold\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Bow Quiver\tbowq\tmisl\tseco\t0\t1\trarm\tlarm\t\tbow\t0\t1\t0\t1\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Crossbow Quiver\txboq\tmisl\tseco\t0\t1\trarm\tlarm\t\txbow\t0\t1\t0\t1\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Player Body Part\tplay\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Herb\therb\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Potion\tpoti\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Ring\tring\tmisc\t\t0\t1\trrin\tlrin\t\t\t0\t0\t0\t0\t1\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t5\tinvrin1\tinvrin2\tinvrin3\tinvrin4\tinvrin5\t\tmisc\t0\n";
                ss << u8"Elixir\telix\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Amulet\tamul\tmisc\t\t0\t1\tneck\tneck\t\t\t0\t0\t0\t0\t1\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t3\tinvamu1\tinvamu2\tinvamu3\t\t\t\tmisc\t0\n";
                ss << u8"Charm\tchar\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t1\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t3\tinvch1\tinvch4\tinvch7\t\t\t\tmisc\t0\n";
                ss << u8"Not Used\t\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Boots\tboot\tarmo\t\t1\t1\tfeet\tfeet\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Gloves\tglov\tarmo\t\t1\t1\tglov\tglov\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Not Used\t\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Book\tbook\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Belt\tbelt\tarmo\t\t1\t1\tbelt\tbelt\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Gem\tgem\tsock\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Torch\ttorc\tmisc\t\t0\t1\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Scroll\tscro\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Not Used\t\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Scepter\tscep\trod\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t5\t40\t6\t0\t1\tpal\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Wand\twand\trod\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t2\t40\t2\t0\t1\tnec\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Staff\tstaf\trod\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t5\t25\t6\t40\t6\t0\t1\tsor\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Bow\tbow\tmiss\t\t0\t1\trarm\tlarm\tbowq\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t1\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Axe\taxe\tmele\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t4\t25\t5\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Club\tclub\tblun\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Sword\tswor\tblde\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Hammer\thamm\tblun\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Knife\tknif\tblde\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t3\t40\t3\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Spear\tspea\tsppl\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Polearm\tpole\tsppl\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Crossbow\txbow\tmiss\t\t1\t1\trarm\tlarm\txboq\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Mace\tmace\tblun\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t3\t\t\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Helm\thelm\tarmo\t\t1\t1\thead\thead\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t2\t40\t3\t0\t3\t\t\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Missile Potion\ttpot\tthro\t\t0\t1\trarm\tlarm\t\t\t1\t1\t1\t1\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Quest\tques\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Body Part\tbody\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Key\tkey\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t1\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Throwing Knife\ttkni\tcomb\tknif\t1\t1\trarm\tlarm\t\t\t1\t1\t1\t1\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Throwing Axe\ttaxe\tcomb\taxe\t1\t1\trarm\tlarm\t\t\t1\t1\t1\t1\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Javelin\tjave\tcomb\tspea\t1\t1\trarm\tlarm\t\t\t1\t1\t1\t1\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Weapon\tweap\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t1\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Melee Weapon\tmele\tweap\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t1\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Missile Weapon\tmiss\tweap\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Thrown Weapon\tthro\tweap\t\t0\t0\t\t\t\t\t1\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Combo Weapon\tcomb\tmele\tthro\t0\t0\t\t\t\t\t1\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Any Armor\tarmo\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t1\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Any Shield\tshld\tarmo\tseco\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Miscellaneous\tmisc\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Socket Filler\tsock\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Second Hand\tseco\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Staves And Rods\trod\tblun\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Missile\tmisl\tmisc\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Blunt\tblun\tmele\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Expansion\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Jewel\tjewl\tsock\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t1\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t6\tinvjw1\tinvjw2\tinvjw3\tinvjw4\tinvjw5\tinvjw6\tmisc\t0\n";
                ss << u8"Class Specific\tclas\t\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Amazon Item\tamaz\tclas\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t1\t\tama\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Barbarian Item\tbarb\tclas\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t1\t\tbar\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Necromancer Item\tnecr\tclas\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t1\t\tnec\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Paladin Item\tpala\tclas\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t1\t\tpal\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Sorceress Item\tsorc\tclas\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t1\t\tsor\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Assassin Item\tassn\tclas\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t2\t\tass\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Druid Item\tdrui\tclas\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t1\t\tdru\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Hand to Hand\th2h\tmele\tassn\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t3\t40\t3\t0\t2\t\tass\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Orb\torb\tweap\tsorc\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t3\t40\t3\t0\t1\tsor\tsor\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Voodoo Heads\thead\tshld\tnecr\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t3\t40\t3\t0\t1\tnec\tnec\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Auric Shields\tashd\tshld\tpala\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t4\t0\t1\t\tpal\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Primal Helm\tphlm\thelm\tbarb\t1\t1\thead\thead\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t3\t40\t3\t0\t1\tbar\tbar\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Pelt\tpelt\thelm\tdrui\t1\t1\thead\thead\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t3\t40\t3\t0\t1\tdru\tdru\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Cloak\tcloa\ttors\tassn\t1\t1\ttors\ttors\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t1\tass\tass\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Rune\trune\tsock\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Circlet\tcirc\thelm\t\t1\t1\thead\thead\t\t\t0\t0\t0\t0\t\t1\t0\t0\t1\t25\t2\t40\t3\t0\t3\t\t\t0\t\t\t\t\t\t\tarmo\t0\n";
                ss << u8"Healing Potion\thpot\tpoti\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Mana Potion\tmpot\tpoti\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Rejuv Potion\trpot\thpot\tmpot\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Stamina Potion\tspot\tpoti\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Antidote Potion\tapot\tpoti\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Thawing Potion\twpot\tpoti\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t1\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Small Charm\tscha\tchar\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t1\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t3\tinvch1\tinvch4\tinvch7\t\t\t\tmisc\t0\n";
                ss << u8"Medium Charm\tmcha\tchar\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t1\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t3\tinvch2\tinvch5\tinvch8\t\t\t\tmisc\t0\n";
                ss << u8"Large Charm\tlcha\tchar\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t1\t\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t3\tinvch3\tinvch6\tinvch9\t\t\t\tmisc\t0\n";
                ss << u8"Amazon Bow\tabow\tbow\tamaz\t0\t1\trarm\tlarm\tbowq\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t5\t1\t1\t\tama\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Amazon Spear\taspe\tspea\tamaz\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t3\t25\t4\t40\t6\t0\t1\t\tama\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Amazon Javelin\tajav\tjave\tamaz\t1\t1\trarm\tlarm\t\t\t1\t1\t1\t1\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t1\t\tama\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Hand to Hand 2\th2h2\th2h\t\t1\t1\trarm\tlarm\t\t\t0\t0\t0\t0\t\t1\t0\t0\t2\t25\t3\t40\t3\t0\t2\tass\tass\t0\t\t\t\t\t\t\tweap\t0\n";
                ss << u8"Magic Bow Quiv\tmboq\tbowq\t\t0\t1\trarm\tlarm\t\tbow\t0\t1\t0\t0\t1\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Magic Xbow Quiv\tmxbq\txboq\t\t0\t1\trarm\tlarm\t\txbow\t0\t1\t0\t0\t1\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Chipped Gem\tgem0\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Flawed Gem\tgem1\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Standard Gem\tgem2\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Flawless Gem\tgem3\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Perfect Gem\tgem4\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Amethyst\tgema\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Diamond\tgemd\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Emerald\tgeme\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Ruby\tgemr\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Sapphire\tgems\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Topaz\tgemt\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Skull\tgemz\tgem\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t\t1\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\tmisc\t0\n";
                ss << u8"Swords and Knives\tblde\tmele\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                ss << u8"Spears and Polearms\tsppl\tmele\t\t0\t0\t\t\t\t\t0\t0\t0\t0\t\t1\t0\t0\t0\t25\t0\t40\t0\t0\t3\t\t\t0\t\t\t\t\t\t\t\t0\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}