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
        const std::string& GetProperties()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"code\t*Enabled\tfunc1\tstat1\tset1\tval1\tfunc2\tstat2\tset2\tval2\tfunc3\tstat3\tset3\tval3\tfunc4\tstat4\tset4\tval4\tfunc5\tstat5\tset5\tval5\tfunc6\tstat6\tset6\tval6\tfunc7\tstat7\tset7\tval7\t*Tooltip\t*Parameter\t*Min\t*Max\t*Notes\t*eol\n";
                ss << u8"ac\t1\t1\tarmorclass\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Defense\t\tMin #\tMax #\t\t0\n";
                ss << u8"ac-miss\t1\t1\tarmorclass_vs_missile\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Defense vs. Missile\t\tMin #\tMax #\t\t0\n";
                ss << u8"ac-hth\t1\t1\tarmorclass_vs_hth\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Defense vs. Melee\t\tMin #\tMax #\t\t0\n";
                ss << u8"red-dmg\t1\t1\tnormal_damage_reduction\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tDamage Reduced by #\t\tMin #\tMax #\t\t0\n";
                ss << u8"red-dmg%\t1\t1\tdamageresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tDamage Reduced by #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"ac%\t1\t2\titem_armor_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Enhanced Defense\t\tMin %\tMax %\t\t0\n";
                ss << u8"red-mag\t1\t1\tmagic_damage_reduction\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMagic Damage Reduced by #\t\tMin #\tMax #\t\t0\n";
                ss << u8"str\t1\t1\tstrength\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Strength\t\tMin #\tMax #\t\t0\n";
                ss << u8"dex\t1\t1\tdexterity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Dexterity\t\tMin #\tMax #\t\t0\n";
                ss << u8"vit\t1\t1\tvitality\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Vitality\t\tMin #\tMax #\t\t0\n";
                ss << u8"enr\t1\t1\tenergy\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Energy\t\tMin #\tMax #\t\t0\n";
                ss << u8"mana\t1\t1\tmaxmana\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Mana\t\tMin #\tMax #\t\t0\n";
                ss << u8"mana%\t1\t1\titem_maxmana_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tIncrease Maximum Mana #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"hp\t1\t1\tmaxhp\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Life\t\tMin #\tMax #\t\t0\n";
                ss << u8"hp%\t1\t1\titem_maxhp_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tIncrease Maximum Life #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"att\t1\t1\ttohit\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating\t\tMin #\tMax #\t\t0\n";
                ss << u8"block\t1\t1\ttoblock\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Increased Chance of Blocking\t\tMin %\tMax %\t\t0\n";
                ss << u8"cold-min\t1\t1\tcoldmindam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Minimum Cold Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"cold-max\t1\t1\tcoldmaxdam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Cold Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"cold-len\t1\t1\tcoldlength\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin #\tMax #\t# of Frames (25 Frames = 1 Second)\t0\n";
                ss << u8"fire-min\t1\t1\tfiremindam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Minimum Fire Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"fire-max\t1\t1\tfiremaxdam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Fire Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"ltng-min\t1\t1\tlightmindam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Minimum Lightning Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"ltng-max\t1\t1\tlightmaxdam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Lightning Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"pois-min\t1\t1\tpoisonmindam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Minimum Poison Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"pois-max\t1\t1\tpoisonmaxdam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Poison Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"pois-len\t1\t1\tpoisonlength\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin #\tMax #\t# of Frames (25 Frames = 1 Second)\t0\n";
                ss << u8"dmg-min\t1\t5\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Minimum Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"dmg-max\t1\t6\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"dmg%\t1\t7\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Enhanced Damage\t\tMin %\tMax %\t\t0\n";
                ss << u8"dmg-to-mana\t1\t1\titem_damagetomana\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Damage Taken Goes To Mana\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-fire\t1\t1\tfireresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tFire Resist +#%\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-fire-max\t1\t1\tmaxfireresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Maximum Fire Resist\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-ltng\t1\t1\tlightresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tLightning Resist +#%\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-ltng-max\t1\t1\tmaxlightresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Maximum Lightning Resist\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-cold\t1\t1\tcoldresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tCold Resist +#%\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-cold-max\t1\t1\tmaxcoldresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Maximum Cold Resist\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-mag\t1\t1\tmagicresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMagic Resist +#%\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-mag-max\t1\t1\tmaxmagicresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Maximum Magic Resist\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-pois\t1\t1\tpoisonresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tPoison Resist +#%\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-pois-max\t1\t1\tmaxpoisonresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Maximum Poison Resist\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-all\t1\t1\tfireresist\t\t\t3\tlightresist\t\t\t3\tcoldresist\t\t\t3\tpoisonresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAll Resistances +#\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-all-max\t1\t1\tmaxfireresist\t\t\t3\tmaxlightresist\t\t\t3\tmaxcoldresist\t\t\t3\tmaxpoisonresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin %\tMax %\tCurrently Not Used\t0\n";
                ss << u8"abs-fire%\t1\t1\titem_absorbfire_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Fire Absorb\t\tMin #\tMax #\t\t0\n";
                ss << u8"abs-fire\t1\t1\titem_absorbfire\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tFire Absorb #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"abs-ltng%\t1\t1\titem_absorblight_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Lightning Absorb\t\tMin #\tMax #\t\t0\n";
                ss << u8"abs-ltng\t1\t1\titem_absorblight\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tLightning Absorb #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"abs-mag%\t1\t1\titem_absorbmagic_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Magic Absorb\t\tMin #\tMax #\t\t0\n";
                ss << u8"abs-mag\t1\t1\titem_absorbmagic\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMagic Absorb #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"abs-cold%\t1\t1\titem_absorbcold_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Cold Absorb\t\tMin #\tMax #\t\t0\n";
                ss << u8"abs-cold\t1\t1\titem_absorbcold\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tCold Absorb #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"dur\t1\t1\tmaxdurability\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tDurability: # of #\t\tMin #\tMax #\t\t0\n";
                ss << u8"dur%\t1\t13\titem_maxdurability_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tIncrease Maximum Durability #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"regen\t1\t1\thpregen\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tReplenish Life +#\t\tMin #\tMax #\t\t0\n";
                ss << u8"thorns\t1\t1\titem_attackertakesdamage\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAttacker Takes Damage of #\t\tMin #\tMax #\t\t0\n";
                ss << u8"swing1\t1\t8\titem_fasterattackrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Increased Attack Speed\t\tMin %\tMax %\t\t0\n";
                ss << u8"swing2\t1\t8\titem_fasterattackrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Increased Attack Speed\t\tMin %\tMax %\t\t0\n";
                ss << u8"swing3\t1\t8\titem_fasterattackrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Increased Attack Speed\t\tMin %\tMax %\t\t0\n";
                ss << u8"gold%\t1\t1\titem_goldbonus\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Extra Gold from Monsters\t\tMin %\tMax %\t\t0\n";
                ss << u8"mag%\t1\t1\titem_magicbonus\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Better Chance of Getting Magic Items\t\tMin %\tMax %\t\t0\n";
                ss << u8"knock\t1\t1\titem_knockback\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tKnockback\t\t1\t1\t\t0\n";
                ss << u8"regen-stam\t1\t1\tstaminarecoverybonus\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tHeal Stamina Plus #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"regen-mana\t1\t1\tmanarecoverybonus\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tRegenerate Mana #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"stam\t1\t1\tmaxstamina\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Maximum Stamina\t\tMin #\tMax #\t\t0\n";
                ss << u8"time\t1\t1\titem_timeduration\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin #\tMax #\tCurrently Not Used\t0\n";
                ss << u8"manasteal\t1\t1\tmanadrainmindam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Mana stolen per hit\t\tMin #\tMax #\t\t0\n";
                ss << u8"lifesteal\t1\t1\tlifedrainmindam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Life stolen per hit\t\tMin #\tMax #\t\t0\n";
                ss << u8"ama\t1\t21\titem_addclassskills\t\t0\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Amazon Skill Levels\t\tMin #\tMax #\t\t0\n";
                ss << u8"pal\t1\t21\titem_addclassskills\t\t3\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Paladin Skill Levels\t\tMin #\tMax #\t\t0\n";
                ss << u8"nec\t1\t21\titem_addclassskills\t\t2\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Necromancer Skill Levels\t\tMin #\tMax #\t\t0\n";
                ss << u8"sor\t1\t21\titem_addclassskills\t\t1\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Sorceress Skill Levels\t\tMin #\tMax #\t\t0\n";
                ss << u8"bar\t1\t21\titem_addclassskills\t\t4\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#  to Barbarian Skill Levels\t\tMin #\tMax #\t\t0\n";
                ss << u8"herb\t1\t1\titem_doubleherbduration\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin #\tMax #\tCurrently Not Used\t0\n";
                ss << u8"light\t1\t1\titem_lightradius\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Light Radius\t\tMin #\tMax #\t\t0\n";
                ss << u8"color\t1\t1\titem_lightcolor\t1\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin #\tMax #\tCurrently Not Used\t0\n";
                ss << u8"ease\t1\t1\titem_req_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tRequirements -#%\t\tMin %\tMax %\t\t0\n";
                ss << u8"move1\t1\t8\titem_fastermovevelocity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Run/Walk\t\tMin %\tMax %\t\t0\n";
                ss << u8"move2\t1\t8\titem_fastermovevelocity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Run/Walk\t\tMin %\tMax %\t\t0\n";
                ss << u8"move3\t1\t8\titem_fastermovevelocity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Run/Walk\t\tMin %\tMax %\t\t0\n";
                ss << u8"balance1\t1\t8\titem_fastergethitrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Hit Recovery\t\tMin %\tMax %\t\t0\n";
                ss << u8"balance2\t1\t8\titem_fastergethitrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Hit Recovery\t\tMin %\tMax %\t\t0\n";
                ss << u8"balance3\t1\t8\titem_fastergethitrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Hit Recovery\t\tMin %\tMax %\t\t0\n";
                ss << u8"block1\t1\t8\titem_fasterblockrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Block Rate\t\tMin %\tMax %\t\t0\n";
                ss << u8"block2\t1\t8\titem_fasterblockrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Block Rate\t\tMin %\tMax %\t\t0\n";
                ss << u8"block3\t1\t8\titem_fasterblockrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Block Rate\t\tMin %\tMax %\t\t0\n";
                ss << u8"cast1\t1\t8\titem_fastercastrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Cast Rate\t\tMin %\tMax %\t\t0\n";
                ss << u8"cast2\t1\t8\titem_fastercastrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Cast Rate\t\tMin %\tMax %\t\t0\n";
                ss << u8"cast3\t1\t8\titem_fastercastrate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Faster Cast Rate\t\tMin %\tMax %\t\t0\n";
                ss << u8"res-pois-len\t1\t1\titem_poisonlengthresist\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tPoison Length Reduced by #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"dmg\t1\t1\titem_normaldamage\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tDamage +#\t\tMin #\tMax #\t\t0\n";
                ss << u8"howl\t1\t1\titem_howl\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tHit Causes Monster to Flee #%\t\tMin %/128\tMax %/128\t\t0\n";
                ss << u8"stupidity\t1\t1\titem_stupidity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tHit Blinds Target +#\t\tMin #\tMax #\t\t0\n";
                ss << u8"ignore-ac\t1\t1\titem_ignoretargetac\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tIgnore Target's Defense\t\t1\t1\t\t0\n";
                ss << u8"reduce-ac\t1\t1\titem_fractionaltargetac\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t-#% Target Defense\t\tMin #\tMax #\t\t0\n";
                ss << u8"noheal\t1\t1\titem_preventheal\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tPrevent Monster Heal\t\t1\t1\t\t0\n";
                ss << u8"half-freeze\t1\t1\titem_halffreezeduration\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tHalf Freeze Duration\t\t1\t1\t\t0\n";
                ss << u8"att%\t1\t1\titem_tohit_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Bonus to Attack Rating\t\tMin %\tMax %\t\t0\n";
                ss << u8"dmg-ac\t1\t1\titem_damagetargetac\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t-# to Monster Defense Per Hit\t\tMin #\tMax #\t\t0\n";
                ss << u8"dmg-demon\t1\t1\titem_demondamage_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Damage to Demons\t\tMin %\tMax %\t\t0\n";
                ss << u8"dmg-undead\t1\t1\titem_undeaddamage_percent\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Damage to Undead\t\tMin %\tMax %\t\t0\n";
                ss << u8"att-demon\t1\t1\titem_demon_tohit\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating against Demons\t\tMin #\tMax #\t\t0\n";
                ss << u8"att-undead\t1\t1\titem_undead_tohit\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating against Undead\t\tMin #\tMax #\t\t0\n";
                ss << u8"throw\t1\t1\titem_throwable\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t1\t1\tCurrently Not Used\t0\n";
                ss << u8"fireskill\t1\t21\titem_elemskill\t\t1\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Fire Skills\t\tMin #\tMax #\t\t0\n";
                ss << u8"allskills\t1\t1\titem_allskills\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to All Skills\t\tMin #\tMax #\t\t0\n";
                ss << u8"light-thorns\t1\t1\titem_attackertakeslightdamage\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAttacker Takes Lightning Damage of #\t\tMin #\tMax #\t\t0\n";
                ss << u8"freeze\t1\t1\titem_freeze\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tFreezes Target +#\t\tMin #\tMax #\t\t0\n";
                ss << u8"openwounds\t1\t1\titem_openwounds\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance of Open Wounds\t\tMin %\tMax %\t\t0\n";
                ss << u8"crush\t1\t1\titem_crushingblow\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance of Crushing Blow\t\tMin %\tMax %\t\t0\n";
                ss << u8"kick\t1\t1\titem_kickdamage\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Kick Damage\t\tMin %\tMax %\t\t0\n";
                ss << u8"mana-kill\t1\t1\titem_manaafterkill\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Mana after each Kill\t\tMin #\tMax #\t\t0\n";
                ss << u8"demon-heal\t1\t1\titem_healafterdemonkill\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Life after each Demon Kill\t\tMin #\tMax #\t\t0\n";
                ss << u8"bloody\t1\t1\titem_extrablood\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin #\tMax #\tVisuals Only\t0\n";
                ss << u8"deadly\t1\t1\titem_deadlystrike\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Deadly Strike\t\tMin #\tMax #\t\t0\n";
                ss << u8"slow\t1\t1\titem_slow\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tSlows Target by #%\t\tMin #\tMax #\t\t0\n";
                ss << u8"nofreeze\t1\t1\titem_cannotbefrozen\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tCannot Be Frozen\t\t1\t1\t\t0\n";
                ss << u8"stamdrain\t1\t1\titem_staminadrainpct\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Slower Stamina Drain\t\tMin #\tMax #\t\t0\n";
                ss << u8"reanimate\t1\t24\titem_reanimate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tReanimate As: [Returned]\tmonstats.txt hcIdx\tMin %\tMax %\t\t0\n";
                ss << u8"pierce\t1\t1\titem_pierce\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tPiercing Attack\t\tMin %\tMax %\t\t0\n";
                ss << u8"magicarrow\t1\t1\titem_magicarrow\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tFires Magic Arrows\t\tMin #\tMax #\t\t0\n";
                ss << u8"explosivearrow\t1\t1\titem_explosivearrow\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tFires Explosive Arrows or Bolts\t\tMin #\tMax #\t\t0\n";
                ss << u8"Expansion\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"dru\t1\t21\titem_addclassskills\t\t5\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Druid Skill Levels\t\tMin #\tMax #\t\t0\n";
                ss << u8"ass\t1\t21\titem_addclassskills\t\t6\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Assassin Skill Levels\t\tMin #\tMax #\t\t0\n";
                ss << u8"skill\t1\t22\titem_singleskill\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to [Skill] ([Class] only)\tSkill\tMin #\tMax #\t\t0\n";
                ss << u8"skilltab\t1\t10\titem_addskill_tab\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to [Class Skill Tab] Skills\tClass Skill Tab ID\tMin #\tMax #\t\"[Class Skill Tab ID] = (Amazon = 0-2, Sorceress = 3-5, Necromancer = 6-8, Paladin = 9-11, Barbarian = 12-14, Druid = 15-17,  Assassin = 18-20)\"\t0\n";
                ss << u8"aura\t1\t22\titem_aura\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tLevel # [Skill] Aura When Equipped\tSkill\tMin #\tMax #\t\t0\n";
                ss << u8"att-skill\t1\t11\titem_skillonattack\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance to cast level # [Skill] on attack\tSkill\t\"% Chance (If 0, then default to 5)\"\tSkill Level\t\t0\n";
                ss << u8"hit-skill\t1\t11\titem_skillonhit\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance to cast level # [Skill] on striking\tSkill\t\"% Chance (If 0, then default to 5)\"\tSkill Level\t\t0\n";
                ss << u8"gethit-skill\t1\t11\titem_skillongethit\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance to cast level # [Skill] when struck\tSkill\t\"% Chance (If 0, then default to 5)\"\tSkill Level\t\t0\n";
                ss << u8"gembonus\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"regen-dur\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"fire-fx\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"ltng-fx\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"sock\t1\t14\titem_numsockets\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tSocketed (#)\t\tMin #\tMax #\t\t0\n";
                ss << u8"dmg-fire\t1\t15\tfiremindam\t\t\t16\tfiremaxdam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAdds #-# Fire Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"dmg-ltng\t1\t15\tlightmindam\t\t\t16\tlightmaxdam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAdds #-# Lightning Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"dmg-mag\t1\t15\tmagicmindam\t\t\t16\tmagicmaxdam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAdds #-# Magic Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"dmg-cold\t1\t15\tcoldmindam\t\t\t16\tcoldmaxdam\t\t\t17\tcoldlength\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAdds #-# Cold Damage\tLength (Frames)\tMin #\tMax #\t# of Frames (25 Frames = 1 Second)\t0\n";
                ss << u8"dmg-pois\t1\t15\tpoisonmindam\t\t\t16\tpoisonmaxdam\t\t\t17\tpoisonlength\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAdds #-# Poison Damage Over # Seconds\tLength (Frames)\tMin #\tMax #\t# of Frames (25 Frames = 1 Second)\t0\n";
                ss << u8"dmg-throw\t1\t15\titem_throw_mindamage\t\t\t16\titem_throw_maxdamage\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin #\tMax #\tCurrently Not Used\t0\n";
                ss << u8"dmg-norm\t1\t15\tmindamage\t\t\t16\tmaxdamage\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAdds #-# Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"ac/lvl\t1\t17\titem_armor_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Defense (Based on Character Level)\tac/lvl (8ths)\t\t\t\t0\n";
                ss << u8"ac%/lvl\t1\t17\titem_armorpercent_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Enhanced Defense (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"hp/lvl\t1\t17\titem_hp_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Life (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"mana/lvl\t1\t17\titem_mana_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Mana (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"dmg/lvl\t1\t17\titem_maxdamage_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Damage (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"dmg%/lvl\t1\t17\titem_maxdamage_percent_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Enhanced Maximum Damage (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"str/lvl\t1\t17\titem_strength_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Strength (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"dex/lvl\t1\t17\titem_dexterity_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Dexterity (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"enr/lvl\t1\t17\titem_energy_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Energy (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"vit/lvl\t1\t17\titem_vitality_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Vitality (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"att/lvl\t1\t17\titem_tohit_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating (Based on Character Level)\t#/2 per Level\t\t\t\t0\n";
                ss << u8"att%/lvl\t1\t17\titem_tohitpercent_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Bonus to Attack Rating (Based on Character Level)\t#/2 per Level\t\t\t\t0\n";
                ss << u8"dmg-cold/lvl\t1\t17\titem_cold_damagemax_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Cold Damage (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"dmg-fire/lvl\t1\t17\titem_fire_damagemax_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Fire Damage (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"dmg-ltng/lvl\t1\t17\titem_ltng_damagemax_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Lightning Damage (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"dmg-pois/lvl\t1\t17\titem_pois_damagemax_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Poison Damage (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"res-cold/lvl\t1\t17\titem_resist_cold_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tCold Resist +#% (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"res-fire/lvl\t1\t17\titem_resist_fire_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tFire Resist +#% (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"res-ltng/lvl\t1\t17\titem_resist_ltng_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tLightning Resist +#% (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"res-pois/lvl\t1\t17\titem_resist_pois_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tPoison Resist +#% (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"abs-cold/lvl\t1\t17\titem_absorb_cold_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAbsorbs Cold Damage (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"abs-fire/lvl\t1\t17\titem_absorb_fire_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAbsorbs Fire Damage (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"abs-ltng/lvl\t1\t17\titem_absorb_ltng_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAbsorbs Lightning Damage (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"abs-pois/lvl\t1\t17\titem_absorb_pois_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAbsorbs Poison Damage (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"thorns/lvl\t1\t17\titem_thorns_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAttacker Takes Damage of # (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"gold%/lvl\t1\t17\titem_find_gold_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Extra Gold from Monsters (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"mag%/lvl\t1\t17\titem_find_magic_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Better Chance of Getting Magic Items (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"regen-stam/lvl\t1\t17\titem_regenstamina_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tHeal Stamina Plus #% (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"stam/lvl\t1\t17\titem_stamina_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Maximum Stamina (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"dmg-dem/lvl\t1\t17\titem_damage_demon_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Damage to Demons (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"dmg-und/lvl\t1\t17\titem_damage_undead_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Damage to Undead (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"att-dem/lvl\t1\t17\titem_tohit_demon_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating against Demons (Based on Character Level)\t#/2 per Level\t\t\t\t0\n";
                ss << u8"att-und/lvl\t1\t17\titem_tohit_undead_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating against Undead (Based on Character Level)\t#/2 per Level\t\t\t\t0\n";
                ss << u8"crush/lvl\t1\t17\titem_crushingblow_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance of Crushing Blow (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"wounds/lvl\t1\t17\titem_openwounds_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance of Open Wounds (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"kick/lvl\t1\t17\titem_kick_damage_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Kick Damage (Based on Character Level)\t#/8 per Level\t\t\tCurrently Not Used\t0\n";
                ss << u8"deadly/lvl\t1\t17\titem_deadlystrike_perlevel\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Deadly Strike (Based on Character Level)\t#/8 per Level\t\t\t\t0\n";
                ss << u8"gems%/lvl\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"rep-dur\t1\t17\titem_replenish_durability\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tRepairs 1 durability in # seconds\tRepair Speed\t\t\tRepair 1 Durability every 100/[Repair Speed] seconds\t0\n";
                ss << u8"rep-quant\t1\t17\titem_replenish_quantity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tReplenishes quantity\tRepair Speed\t\t\tReplenish 1 Quantity every 100/[Repair Speed] seconds\t0\n";
                ss << u8"stack\t1\t1\titem_extra_stack\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tIncreased Stack Size\t\tMin #\tMax #\t\t0\n";
                ss << u8"item%\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"dmg-slash\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"dmg-slash%\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"dmg-crush\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"dmg-crush%\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"dmg-thrust\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"dmg-thrust%\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"abs-slash\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"abs-crush\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"abs-thrust\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"abs-slash%\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"abs-crush%\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"abs-thrust%\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"ac/time\t1\t18\titem_armor_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Defense (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"ac%/time\t1\t18\titem_armorpercent_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Enhanced Defense (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"hp/time\t1\t18\titem_hp_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Life (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"mana/time\t1\t18\titem_mana_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Mana (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dmg/time\t1\t18\titem_maxdamage_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dmg%/time\t1\t18\titem_maxdamage_percent_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Enhanced Maximum Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"str/time\t1\t18\titem_strength_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Strength (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dex/time\t1\t18\titem_dexterity_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Dexterity (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"enr/time\t1\t18\titem_energy_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Energy (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"vit/time\t1\t18\titem_vitality_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Vitality (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"att/time\t1\t18\titem_tohit_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"att%/time\t1\t18\titem_tohitpercent_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Bonus to Attack Rating (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dmg-cold/time\t1\t18\titem_cold_damagemax_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Cold Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dmg-fire/time\t1\t18\titem_fire_damagemax_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Fire Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dmg-ltng/time\t1\t18\titem_ltng_damagemax_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Lightning Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dmg-pois/time\t1\t18\titem_pois_damagemax_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Poison Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"res-cold/time\t1\t18\titem_resist_cold_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tCold Resist +#% (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"res-fire/time\t1\t18\titem_resist_fire_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tFire Resist +#% (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"res-ltng/time\t1\t18\titem_resist_ltng_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tLightning Resist +#% (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"res-pois/time\t1\t18\titem_resist_pois_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tPoison Resist +#% (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"abs-cold/time\t1\t18\titem_absorb_cold_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAbsorbs Cold Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"abs-fire/time\t1\t18\titem_absorb_fire_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAbsorbs Fire Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"abs-ltng/time\t1\t18\titem_absorb_ltng_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAbsorbs Lightning Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"abs-pois/time\t1\t18\titem_absorb_pois_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tAbsorbs Poison Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"gold%/time\t1\t18\titem_find_gold_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Extra Gold from Monsters (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"mag%/time\t1\t18\titem_find_magic_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Better Chance of Getting Magic Items (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"regen-stam/time\t1\t18\titem_regenstamina_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tHeal Stamina Plus #% (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"stam/time\t1\t18\titem_stamina_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Maximum Stamina (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dmg-dem/time\t1\t18\titem_damage_demon_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Damage to Demons (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"dmg-und/time\t1\t18\titem_damage_undead_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% Damage to Undead (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"att-dem/time\t1\t18\titem_tohit_demon_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating against Demons (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"att-und/time\t1\t18\titem_tohit_undead_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Attack Rating against Undead (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"crush/time\t1\t18\titem_crushingblow_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance of Crushing Blow (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"wounds/time\t1\t18\titem_openwounds_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance of Open Wounds (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"kick/time\t1\t18\titem_kick_damage_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Kick Damage (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin #\tMax #\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"deadly/time\t1\t18\titem_deadlystrike_bytime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Deadly Strike (Increases near [Day/Dusk/Night/Dawn])\tTime Period for Max value\tMin %\tMax %\t\"(0=Day, 1=Dusk, 2=Night, 3=Dawn) Min value starts at opposite Time Period (Linear Increase) Currently Not Being Used\"\t0\n";
                ss << u8"gems%/time\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"pierce-fire\t1\t1\tpassive_fire_pierce\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t-#% to Enemy Fire Resistance\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-ltng\t1\t1\tpassive_ltng_pierce\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t-#% to Enemy Lightning Resistance\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-cold\t1\t1\tpassive_cold_pierce\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t-#% to Enemy Cold Resistance\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-pois\t1\t1\tpassive_pois_pierce\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t-#% to Enemy Poison Resistance\t\tMin %\tMax %\t\t0\n";
                ss << u8"dmg-mon\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"dmg%-mon\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"att-mon\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"att%-mon\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"ac-mon\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"ac%-mon\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"indestruct\t1\t20\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tIndestructible\t\t1\t1\t\t0\n";
                ss << u8"charged\t1\t19\titem_charged_skill\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tLevel # [Skill] (#/# Charges)\tSkill\t# of Max Charges\tSkill Level\t\t0\n";
                ss << u8"extra-fire\t1\t1\tpassive_fire_mastery\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Fire Skill Damage\t\tMin %\tMax %\t\t0\n";
                ss << u8"extra-ltng\t1\t1\tpassive_ltng_mastery\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Lightning Skill Damage\t\tMin %\tMax %\t\t0\n";
                ss << u8"extra-cold\t1\t1\tpassive_cold_mastery\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Cold Skill Damage\t\tMin %\tMax %\t\t0\n";
                ss << u8"extra-pois\t1\t1\tpassive_pois_mastery\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Poison Skill Damage\t\tMin %\tMax %\t\t0\n";
                ss << u8"dmg-elem\t1\t15\tfiremindam\t\t\t16\tfiremaxdam\t\t\t15\tlightmindam\t\t\t16\tlightmaxdam\t\t\t15\tcoldmindam\t\t\t16\tcoldmaxdam\t\t\t17\tcoldlength\t\t\tAdds #-# Fire/Lightning/Cold Damage\t\tMin #\tMax #\t\t0\n";
                ss << u8"dmg-elem-min\t1\t1\tfiremindam\t\t\t3\tlightmindam\t\t\t3\tcoldmindam\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Minimum Fire/Lightning/Cold Damage\t\tMin #\tMax #\tCurrently Not Used\t0\n";
                ss << u8"dmg-elem-max\t1\t1\tfiremaxdam\t\t\t3\tlightmaxdam\t\t\t3\tcoldmaxdam\t\t\t17\tcoldlength\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to Maximum Fire/Lightning/Cold Damage\t\tMin #\tMax #\tCurrently Not Used\t0\n";
                ss << u8"all-stats\t1\t1\tstrength\t\t\t3\tenergy\t\t\t3\tdexterity\t\t\t3\tvitality\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to all Attributes\t\tMin #\tMax #\t\t0\n";
                ss << u8"addxp\t1\t1\titem_addexperience\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+#% to Experience Gained\t\tMin %\tMax %\t\t0\n";
                ss << u8"heal-kill\t1\t1\titem_healafterkill\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# Life after each Kill\t\tMin #\tMax #\t\t0\n";
                ss << u8"cheap\t1\t1\titem_reducedprices\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tReduces all Vendor Prices #%\t\tMin %\tMax %\t\t0\n";
                ss << u8"rip\t1\t1\titem_restinpeace\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tSlain Monsters Rest in Peace\t\t1\t1\t\t0\n";
                ss << u8"att-mon%\t1\t24\tattack_vs_montype\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% to Attack Rating versus [Monster Type]\tmonstats.txt MonType\tMin #\tMax #\tCurrently Not Used\t0\n";
                ss << u8"dmg-mon%\t1\t24\tdamage_vs_montype\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% to Damage versus [Monster Type]\tmonstats.txt MonType\tMin #\tMax #\tCurrently Not Used\t0\n";
                ss << u8"kill-skill\t1\t11\titem_skillonkill\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance to cast level # [Skill] when you Kill an Enemy\tSkill\t\"% Chance (If 0, then default to 5)\"\tSkill Level\t\t0\n";
                ss << u8"death-skill\t1\t11\titem_skillondeath\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance to cast level # [Skill] when you Die\tSkill\t\"% Chance (If 0, then default to 5)\"\tSkill Level\t\t0\n";
                ss << u8"levelup-skill\t1\t11\titem_skillonlevelup\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t#% Chance to cast level # [Skill] when you Level-Up\tSkill\t\"% Chance (If 0, then default to 5)\"\tSkill Level\t\t0\n";
                ss << u8"skill-rand\t1\t12\titem_singleskill\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to [Skill] ([Class] only)\tSkill Level\tMin Skill ID\tMax Skill ID\t\t0\n";
                ss << u8"fade\t1\t17\tfade\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMin #\tMax #\tVisuals Only\t0\n";
                ss << u8"levelreq\t1\t1\titem_levelreq\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tRequired Level: #\t\tMin #\tMax #\t\t0\n";
                ss << u8"ethereal\t1\t23\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tEthereal\t\t1\t1\t\t0\n";
                ss << u8"oskill\t1\t22\titem_nonclassskill\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to [Skill]\tSkill\tMin #\tMax #\t\t0\n";
                ss << u8"state\t1\t24\tstate\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tState\t1\t1\tApplies a State on the unit\t0\n";
                ss << u8"randclassskill\t1\t36\titem_addclassskills\t\t3\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to [Class] Skill Levels\t\tMin Class ID\tMax Class ID\tval1 = # of Skill levels\t0\n";
                ss << u8"noconsume\t\t1\titem_noconsume\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t+# to not Consume Quantity\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-immunity-cold\t1\t1\titem_pierce_cold_immunity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMonster Cold Immunity is Sundered\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-immunity-fire\t1\t1\titem_pierce_fire_immunity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMonster Fire Immunity is Sundered\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-immunity-light\t1\t1\titem_pierce_light_immunity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMonster Lightning Immunity is Sundered\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-immunity-poison\t1\t1\titem_pierce_poison_immunity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMonster Poison Immunity is Sundered\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-immunity-damage\t1\t1\titem_pierce_damage_immunity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMonster Physical Immunity is Sundered\t\tMin %\tMax %\t\t0\n";
                ss << u8"pierce-immunity-magic\t1\t1\titem_pierce_magic_immunity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tMonster Magic Immunity is Sundered\t\tMin %\tMax %\t\t0\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}