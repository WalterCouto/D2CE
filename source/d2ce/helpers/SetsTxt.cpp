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
        const std::string& GetSets()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"index\tname\tversion\tPCode2a\tPParam2a\tPMin2a\tPMax2a\tPCode2b\tPParam2b\tPMin2b\tPMax2b\tPCode3a\tPParam3a\tPMin3a\tPMax3a\tPCode3b\tPParam3b\tPMin3b\tPMax3b\tPCode4a\tPParam4a\tPMin4a\tPMax4a\tPCode4b\tPParam4b\tPMin4b\tPMax4b\tPCode5a\tPParam5a\tPMin5a\tPMax5a\tPCode5b\tPParam5b\tPMin5b\tPMax5b\tFCode1\tFParam1\tFMin1\tFMax1\tFCode2\tFParam2\tFMin2\tFMax2\tFCode3\tFParam3\tFMin3\tFMax3\tFCode4\tFParam4\tFMin4\tFMax4\tFCode5\tFParam5\tFMin5\tFMax5\tFCode6\tFParam6\tFMin6\tFMax6\tFCode7\tFParam7\tFMin7\tFMax7\tFCode8\tFParam8\tFMin8\tFMax8\t*eol\n";
                ss << u8"Civerb's Vestments\tCiverb's Vestments\t0\tres-fire\t\t25\t25\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tstr\t0\t15\t15\tdmg-undead\t0\t200\t200\tres-ltng\t0\t25\t25\tatt%\t0\t25\t25\tac\t\t50\t50\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Hsarus' Defense\tHsarus' Defense\t0\tthorns\t\t5\t5\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdmg-max\t0\t5\t5\tnofreeze\t0\t1\t1\tres-ltng\t0\t25\t25\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Cleglaw's Brace\tCleglaw's Brace\t0\tac\t\t50\t50\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tac\t0\t50\t50\tmanasteal\t0\t6\t6\tcrush\t0\t35\t35\tswing2\t\t20\t20\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Iratha's Finery\tIratha's Finery\t0\tac\t\t50\t50\t\t\t\t\tmove2\t\t20\t20\tpierce\t\t24\t24\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t0\t20\t20\tres-fire-max\t0\t10\t10\tres-cold-max\t0\t10\t10\tres-ltng-max\t0\t10\t10\tres-pois-max\t0\t10\t10\tdex\t0\t15\t15\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Isenhart's Armory\tIsenhart's Armory\t0\tstr\t\t10\t10\t\t\t\t\tdex\t\t10\t10\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tlifesteal\t0\t5\t5\tres-all\t0\t10\t10\tatt%\t0\t35\t35\tblock\t\t30\t30\tmove2\t\t20\t20\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Vidala's Rig\tVidala's Rig\t0\tatt\t\t75\t75\tmanasteal\t\t7\t7\tdex\t\t15\t15\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdmg-cold/lvl\t12\t\t\tfreeze\t0\t1\t1\tpierce\t0\t50\t50\tstr\t\t10\t10\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Milabrega's Regalia\tMilabrega's Regalia\t0\tatt\t\t75\t75\tdmg-ltng/lvl\t16\t\t\tatt\t\t125\t125\tnofreeze\t\t1\t1\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tlifesteal\t0\t8\t8\tpal\t0\t2\t2\tmanasteal\t0\t10\t10\tres-pois\t0\t15\t15\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Cathan's Traps\tCathan's Traps\t0\tdmg-fire\t\t15\t20\tregen-mana\t\t16\t16\tres-ltng\t\t25\t25\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tatt\t0\t60\t60\tred-mag\t0\t3\t3\tres-all\t0\t25\t25\tcast1\t0\t10\t10\tmana\t0\t20\t20\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Tancred's Battlegear\tTancred's Battlegear\t0\tdmg-ltng\t\t15\t15\t\t\t\t\tlifesteal\t\t5\t5\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tgold%\t0\t75\t75\tres-all\t0\t10\t10\tslow\t0\t35\t35\tmanasteal\t0\t5\t5\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Sigon's Complete Steel\tSigon's Complete Steel\t0\tlifesteal\t\t10\t10\t\t\t\t\tac\t\t100\t100\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tres-fire\t0\t12\t12\tthorns\t0\t12\t12\tred-dmg\t0\t7\t7\tfire-max\t0\t24\t24\tmana\t\t20\t20\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Infernal Tools\tInfernal Tools\t0\tdmg-pois\t80\t25\t25\t\t\t\t\tmana\t\t10\t10\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tatt%\t0\t20\t20\tnec\t0\t1\t1\topenwounds\t0\t20\t20\tmanasteal\t\t6\t6\tmana%\t\t20\t20\tnofreeze\t\t1\t1\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Berserker's Garb\tBerserker's Garb\t0\thp\t\t50\t50\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tres-pois-len\t0\t75\t75\tpois-min\t0\t16\t16\tpois-max\t0\t32\t32\tpois-len\t0\t75\t75\tac\t\t75\t75\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Death's Disguise\tDeath's Disguise\t0\tlifesteal\t\t8\t8\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tatt%\t0\t40\t40\tdmg-min\t0\t10\t10\tres-all\t0\t25\t25\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Angelical Raiment\tAngelical Raiment\t0\tdex\t\t10\t10\t\t\t\t\tmana\t\t50\t50\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t0\t25\t25\thalf-freeze\t0\t1\t1\tmag%\t0\t40\t40\tregen-mana\t\t8\t8\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Arctic Gear\tArctic Gear\t0\tstr\t\t5\t5\t\t\t\t\thp\t\t50\t50\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdmg-cold/lvl\t16\t\t\tcold-len\t0\t100\t100\tnofreeze\t0\t1\t1\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Arcanna's Tricks\tArcanna's Tricks\t0\tmana\t\t50\t50\t\t\t\t\thp\t\t50\t50\tregen-mana\t\t12\t12\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tcast3\t0\t20\t20\tmanasteal\t0\t5\t5\tmana\t0\t25\t25\tallskills\t\t1\t1\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Expansion\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"Natalya's Odium\tNatalya's Odium\t100\tred-mag\t\t15\t15\t\t\t\t\tac\t\t200\t200\t\t\t\t\tres-pois\t\t20\t20\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t\t50\t50\tass\t\t3\t3\tac\t\t150\t150\tlifesteal\t\t14\t14\tmanasteal\t\t14\t14\tred-dmg%\t\t30\t30\tfade\t\t1\t1\t\t\t\t\t0\n";
                ss << u8"Aldur's Watchtower\tAldur's Watchtower\t100\tatt%\t\t150\t150\t\t\t\t\tmag%\t\t50\t50\t\t\t\t\tlifesteal\t\t10\t10\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t\t50\t50\tdru\t\t3\t3\tac\t\t150\t150\tmanasteal\t\t10\t10\tmana\t\t150\t150\tdmg%\t\t350\t350\tstate\tfullsetgeneric\t1\t1\t\t\t\t\t0\n";
                ss << u8"Immortal King\tImmortal King\t100\tatt\t\t50\t50\t\t\t\t\tatt\t\t75\t75\t\t\t\t\tatt\t\t125\t125\t\t\t\t\tatt\t\t200\t200\t\t\t\t\tres-all\t\t50\t50\tbar\t\t3\t3\thp\t\t150\t150\tred-mag\t\t10\t10\tstate\tfullsetgeneric\t1\t1\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Tal Rasha's Wrappings\tTal Rasha's Wrappings\t100\tregen\t\t10\t10\t\t\t\t\tmag%\t\t65\t65\t\t\t\t\tbalance2\t\t25\t25\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t\t50\t50\tsor\t\t3\t3\tac\t\t150\t150\thp\t\t150\t150\tac-miss\t\t50\t50\tstate\tfullsetgeneric\t1\t1\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Griswold's Legacy\tGriswold's Legacy\t100\tstr\t\t20\t20\t\t\t\t\tdex\t\t30\t30\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t\t50\t50\tpal\t\t3\t3\tatt\t\t200\t200\thp\t\t150\t150\tstate\tfullsetgeneric\t1\t1\tbalance2\t\t30\t30\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Trang-Oul's Avatar\tTrang-Oul's Avatar\t100\tregen-mana\t\t15\t15\toskill\tFire Ball\t18\t18\tregen-mana\t\t15\t15\toskill\tFire Wall\t13\t13\tregen-mana\t\t15\t15\toskill\tMeteor\t10\t10\t\t\t\t\t\t\t\t\tres-all\t\t50\t50\tnec\t\t3\t3\tmana\t\t100\t100\tac\t\t200\t200\tstate\tmonsterset\t1\t1\tregen-mana\t\t15\t15\toskill\tFire Mastery\t3\t3\tlifesteal\t\t20\t20\t0\n";
                ss << u8"M'avina's Battle Hymn\tM'avina's Battle Hymn\t100\tstr\t\t20\t20\t\t\t\t\tdex\t\t30\t30\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t\t50\t50\tama\t\t3\t3\tac\t\t100\t100\tatt\t\t100\t100\tmag%\t\t100\t100\tstate\tfullsetgeneric\t1\t1\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"The Disciple\tThe Disciple\t100\tac\t\t150\t150\t\t\t\t\tdmg-pois\t75\t75\t75\t\t\t\t\tstr\t\t10\t10\t\t\t\t\tdex\t\t10\t10\t\t\t\t\tres-all\t\t50\t50\tallskills\t\t2\t2\tmana\t\t100\t100\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Heaven's Brethren\tHeaven's Brethren\t100\tlifesteal\t\t10\t10\t\t\t\t\tregen\t\t30\t30\tdmg-fire/lvl\t24\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t\t50\t50\tallskills\t\t2\t2\tnofreeze\t\t1\t1\tlight\t\t5\t5\tred-dmg%\t\t24\t24\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Orphan's Call\tOrphan's Call\t100\thp\t\t35\t35\t\t\t\t\tthorns\t\t5\t5\t\t\t\t\tac\t\t100\t100\t\t\t\t\t\t\t\t\t\t\t\t\thp\t\t50\t50\tdex\t\t10\t10\tstr\t\t20\t20\tac\t\t100\t100\tres-all\t\t15\t15\tmag%\t0\t80\t80\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Hwanin's Majesty\tHwanin's Majesty\t100\tac\t\t100\t100\t\t\t\t\tac\t\t200\t200\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tallskills\t\t2\t2\tlifesteal\t\t20\t20\tmove3\t\t30\t30\tres-all\t\t30\t30\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Sazabi's Grand Tribute\tSazabi's Grand Tribute\t100\tmove3\t\t40\t40\tres-pois-len\t\t75\t75\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tres-all\t\t30\t30\tlifesteal\t\t15\t15\thp%\t\t27\t27\tallskills\t\t1\t1\tred-dmg%\t\t16\t16\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Bul-Kathos' Children\tBul-Kathos' Children\t100\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdmg-fire\t\t200\t200\tallskills\t\t2\t2\tatt\t\t200\t200\tac\t\t200\t200\tdmg-undead\t\t200\t200\tdmg-demon\t\t200\t200\tlifesteal\t\t10\t10\tdeadly\t\t20\t20\t0\n";
                ss << u8"Cow King's Leathers\tCow King's Leathers\t100\tres-pois\t\t25\t25\tac\t\t100\t100\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tstam\t\t100\t100\tstr\t\t20\t20\tgold%\t\t100\t100\tmag%\t\t100\t100\tgethit-skill\t42\t25\t5\tswing3\t\t30\t30\thp\t\t100\t100\tallskills\t\t1\t1\t0\n";
                ss << u8"Naj's Ancient Set\tNaj's Ancient Set\t100\tac\t\t175\t175\tmag%/lvl\t12\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tallskills\t\t1\t1\tregen\t\t20\t20\tdex\t\t15\t15\tres-all\t\t50\t50\tstr\t\t20\t20\tmana\t\t100\t100\tfireskill\t\t2\t2\thp%\t\t12\t12\t0\n";
                ss << u8"McAuley's Folly\tMcAuley's Folly\t100\tac\t\t50\t50\t\t\t\t\tatt\t\t75\t75\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tallskills\t\t1\t1\tmag%\t\t50\t50\tmana\t\t50\t50\tlifesteal\t\t4\t4\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}