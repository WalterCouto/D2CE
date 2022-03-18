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
        const std::string& GetGems()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"name\tletter\ttransform\tcode\tweaponMod1Code\tweaponMod1Param\tweaponMod1Min\tweaponMod1Max\tweaponMod2Code\tweaponMod2Param\tweaponMod2Min\tweaponMod2Max\tweaponMod3Code\tweaponMod3Param\tweaponMod3Min\tweaponMod3Max\thelmMod1Code\thelmMod1Param\thelmMod1Min\thelmMod1Max\thelmMod2Code\thelmMod2Param\thelmMod2Min\thelmMod2Max\thelmMod3Code\thelmMod3Param\thelmMod3Min\thelmMod3Max\tshieldMod1Code\tshieldMod1Param\tshieldMod1Min\tshieldMod1Max\tshieldMod2Code\tshieldMod2Param\tshieldMod2Min\tshieldMod2Max\tshieldMod3Code\tshieldMod3Param\tshieldMod3Min\tshieldMod3Max\n";
                ss << u8"Chipped Amethyst\t\t18\tgcv\tatt\t0\t40\t40\t\t\t\t\t\t0\t0\t0\tstr\t0\t3\t3\t\t0\t0\t0\t\t0\t0\t0\tac\t0\t8\t8\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawed Amethyst\t\t18\tgfv\tatt\t0\t60\t60\t\t\t\t\t\t0\t0\t0\tstr\t0\t4\t4\t\t0\t0\t0\t\t0\t0\t0\tac\t0\t12\t12\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Amethyst\t\t18\tgsv\tatt\t0\t80\t80\t\t\t\t\t\t0\t0\t0\tstr\t0\t6\t6\t\t0\t0\t0\t\t0\t0\t0\tac\t0\t18\t18\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawless Amethyst\t\t18\tgzv\tatt\t0\t100\t100\t\t\t\t\t\t0\t0\t0\tstr\t0\t8\t8\t\t0\t0\t0\t\t0\t0\t0\tac\t0\t24\t24\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Perfect Amethyst\t\t17\tgpv\tatt\t0\t150\t150\t\t\t\t\t\t0\t0\t0\tstr\t0\t10\t10\t\t0\t0\t0\t\t0\t0\t0\tac\t0\t30\t30\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Chipped Sapphire\t\t5\tgcb\tcold-min\t0\t1\t1\tcold-max\t0\t3\t3\tcold-len\t0\t25\t25\tmana\t0\t10\t10\t\t0\t0\t0\t\t0\t0\t0\tres-cold\t0\t12\t12\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawed Sapphire\t\t5\tgfb\tcold-min\t0\t3\t3\tcold-max\t0\t5\t5\tcold-len\t0\t35\t35\tmana\t0\t17\t17\t\t0\t0\t0\t\t0\t0\t0\tres-cold\t0\t16\t16\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Sapphire\t\t5\tgsb\tcold-min\t0\t4\t4\tcold-max\t0\t7\t7\tcold-len\t0\t50\t50\tmana\t0\t24\t24\t\t0\t0\t0\t\t0\t0\t0\tres-cold\t0\t22\t22\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawless Sapphire\t\t5\tglb\tcold-min\t0\t6\t6\tcold-max\t0\t10\t10\tcold-len\t0\t60\t60\tmana\t0\t31\t31\t\t0\t0\t0\t\t0\t0\t0\tres-cold\t0\t28\t28\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Perfect Sapphire\t\t6\tgpb\tcold-min\t0\t10\t10\tcold-max\t0\t14\t14\tcold-len\t0\t75\t75\tmana\t0\t38\t38\t\t0\t0\t0\t\t0\t0\t0\tres-cold\t0\t40\t40\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Chipped Emerald\t\t11\tgcg\tpois-min\t0\t34\t34\tpois-max\t0\t34\t34\tpois-len\t0\t75\t75\tdex\t0\t3\t3\t\t0\t0\t0\t\t0\t0\t0\tres-pois\t0\t12\t12\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawed Emerald\t\t11\tgfg\tpois-min\t0\t51\t51\tpois-max\t0\t51\t51\tpois-len\t0\t100\t100\tdex\t0\t4\t4\t\t0\t0\t0\t\t0\t0\t0\tres-pois\t0\t16\t16\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Emerald\t\t11\tgsg\tpois-min\t0\t82\t82\tpois-max\t0\t82\t82\tpois-len\t0\t125\t125\tdex\t0\t6\t6\t\t0\t0\t0\t\t0\t0\t0\tres-pois\t0\t22\t22\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawless Emerald\t\t11\tglg\tpois-min\t0\t101\t101\tpois-max\t0\t101\t101\tpois-len\t0\t152\t152\tdex\t0\t8\t8\t\t0\t0\t0\t\t0\t0\t0\tres-pois\t0\t28\t28\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Perfect Emerald\t\t12\tgpg\tpois-min\t0\t143\t143\tpois-max\t0\t143\t143\tpois-len\t0\t179\t179\tdex\t0\t10\t10\t\t0\t0\t0\t\t0\t0\t0\tres-pois\t0\t40\t40\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Chipped Ruby\t\t8\tgcr\tfire-min\t0\t3\t3\tfire-max\t0\t4\t4\t\t0\t0\t0\thp\t0\t10\t10\t\t0\t0\t0\t\t0\t0\t0\tres-fire\t0\t12\t12\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawed Ruby\t\t8\tgfr\tfire-min\t0\t5\t5\tfire-max\t0\t8\t8\t\t0\t0\t0\thp\t0\t17\t17\t\t0\t0\t0\t\t0\t0\t0\tres-fire\t0\t16\t16\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Ruby\t\t8\tgsr\tfire-min\t0\t8\t8\tfire-max\t0\t12\t12\t\t0\t0\t0\thp\t0\t24\t24\t\t0\t0\t0\t\t0\t0\t0\tres-fire\t0\t22\t22\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawless Ruby\t\t8\tglr\tfire-min\t0\t10\t10\tfire-max\t0\t16\t16\t\t0\t0\t0\thp\t0\t31\t31\t\t0\t0\t0\t\t0\t0\t0\tres-fire\t0\t28\t28\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Perfect Ruby\t\t9\tgpr\tfire-min\t0\t15\t15\tfire-max\t0\t20\t20\t\t0\t0\t0\thp\t0\t38\t38\t\t0\t0\t0\t\t0\t0\t0\tres-fire\t0\t40\t40\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Chipped Diamond\t\t1\tgcw\tdmg-undead\t0\t28\t28\t\t\t\t\t\t0\t0\t0\tatt\t0\t20\t20\t\t0\t0\t0\t\t0\t0\t0\tres-all\t0\t6\t6\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawed Diamond\t\t1\tgfw\tdmg-undead\t0\t34\t34\t\t\t\t\t\t0\t0\t0\tatt\t0\t40\t40\t\t0\t0\t0\t\t0\t0\t0\tres-all\t0\t8\t8\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Diamond\t\t1\tgsw\tdmg-undead\t0\t44\t44\t\t\t\t\t\t0\t0\t0\tatt\t0\t60\t60\t\t0\t0\t0\t\t0\t0\t0\tres-all\t0\t11\t11\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawless Diamond\t\t1\tglw\tdmg-undead\t0\t54\t54\t\t\t\t\t\t0\t0\t0\tatt\t0\t80\t80\t\t0\t0\t0\t\t0\t0\t0\tres-all\t0\t14\t14\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Perfect Diamond\t\t0\tgpw\tdmg-undead\t0\t68\t68\t\t\t\t\t\t0\t0\t0\tatt\t0\t100\t100\t\t0\t0\t0\t\t0\t0\t0\tres-all\t0\t19\t19\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Chipped Topaz\t\t14\tgcy\tltng-min\t0\t1\t1\tltng-max\t0\t8\t8\t\t0\t0\t0\tmag%\t0\t9\t9\t\t0\t0\t0\t\t0\t0\t0\tres-ltng\t0\t12\t12\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawed Topaz\t\t14\tgfy\tltng-min\t0\t1\t1\tltng-max\t0\t14\t14\t\t0\t0\t0\tmag%\t0\t13\t13\t\t0\t0\t0\t\t0\t0\t0\tres-ltng\t0\t16\t16\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Topaz\t\t14\tgsy\tltng-min\t0\t1\t1\tltng-max\t0\t22\t22\t\t0\t0\t0\tmag%\t0\t16\t16\t\t0\t0\t0\t\t0\t0\t0\tres-ltng\t0\t22\t22\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawless Topaz\t\t14\tgly\tltng-min\t0\t1\t1\tltng-max\t0\t30\t30\t\t0\t0\t0\tmag%\t0\t20\t20\t\t0\t0\t0\t\t0\t0\t0\tres-ltng\t0\t28\t28\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Perfect Topaz\t\t13\tgpy\tltng-min\t0\t1\t1\tltng-max\t0\t40\t40\t\t0\t0\t0\tmag%\t0\t24\t24\t\t0\t0\t0\t\t0\t0\t0\tres-ltng\t0\t40\t40\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Chipped Skull\t\t2\tskc\tmanasteal\t0\t1\t1\tlifesteal\t0\t2\t2\t\t\t\t\tregen\t0\t2\t2\tregen-mana\t0\t8\t8\t\t0\t0\t0\tthorns\t0\t4\t4\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawed Skull\t\t2\tskf\tmanasteal\t0\t2\t2\tlifesteal\t0\t2\t2\t\t\t\t\tregen\t0\t3\t3\tregen-mana\t0\t8\t8\t\t0\t0\t0\tthorns\t0\t8\t8\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Skull\t\t2\tsku\tmanasteal\t0\t2\t2\tlifesteal\t0\t3\t3\t\t\t\t\tregen\t0\t3\t3\tregen-mana\t0\t12\t12\t\t0\t0\t0\tthorns\t0\t12\t12\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Flawless Skull\t\t2\tskl\tmanasteal\t0\t3\t3\tlifesteal\t0\t3\t3\t\t\t\t\tregen\t0\t4\t4\tregen-mana\t0\t12\t12\t\t0\t0\t0\tthorns\t0\t16\t16\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Perfect Skull\t\t3\tskz\tmanasteal\t0\t3\t3\tlifesteal\t0\t4\t4\t\t\t\t\tregen\t0\t5\t5\tregen-mana\t0\t19\t19\t\t0\t0\t0\tthorns\t0\t20\t20\t\t0\t0\t0\t\t0\t0\t0\n";
                ss << u8"Expansion\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n";
                ss << u8"El Rune\tr01L\t18\tr01\tlight\t\t1\t1\tatt\t\t50\t50\t\t\t\t\tlight\t\t1\t1\tac\t\t15\t15\t\t\t\t\tlight\t\t1\t1\tac\t\t15\t15\t\t\t\t0\n";
                ss << u8"Eld Rune\tr02L\t18\tr02\tatt-undead\t\t50\t50\tdmg-undead\t\t75\t75\t\t\t\t\tstamdrain\t\t15\t15\t\t\t\t\t\t\t\t\tblock\t\t7\t7\t\t\t\t\t\t\t\t0\n";
                ss << u8"Tir Rune\tr03L\t18\tr03\tmana-kill\t\t2\t2\t\t\t\t\t\t\t\t\tmana-kill\t\t2\t2\t\t\t\t\t\t\t\t\tmana-kill\t\t2\t2\t\t\t\t\t\t\t\t0\n";
                ss << u8"Nef Rune\tr04L\t18\tr04\tknock\t\t1\t1\t\t\t\t\t\t\t\t\tac-miss\t\t30\t30\t\t\t\t\t\t\t\t\tac-miss\t\t30\t30\t\t\t\t\t\t\t\t0\n";
                ss << u8"Eth Rune\tr05L\t18\tr05\treduce-ac\t\t25\t25\t\t\t\t\t\t\t\t\tregen-mana\t\t15\t15\t\t\t\t\t\t\t\t\tregen-mana\t\t15\t15\t\t\t\t\t\t\t\t0\n";
                ss << u8"Ith Rune\tr06L\t18\tr06\tdmg-max\t\t9\t9\t\t\t\t\t\t\t\t\tdmg-to-mana\t\t15\t15\t\t\t\t\t\t\t\t\tdmg-to-mana\t\t15\t15\t\t\t\t\t\t\t\t0\n";
                ss << u8"Tal Rune\tr07L\t18\tr07\tdmg-pois\t125\t154\t154\t\t\t\t\t\t\t\t\tres-pois\t\t30\t30\t\t\t\t\t\t\t\t\tres-pois\t\t35\t35\t\t\t\t\t\t\t\t0\n";
                ss << u8"Ral Rune\tr08L\t18\tr08\tdmg-fire\t\t5\t30\t\t\t\t\t\t\t\t\tres-fire\t\t30\t30\t\t\t\t\t\t\t\t\tres-fire\t\t35\t35\t\t\t\t\t\t\t\t0\n";
                ss << u8"Ort Rune\tr09L\t18\tr09\tdmg-ltng\t\t1\t50\t\t\t\t\t\t\t\t\tres-ltng\t\t30\t30\t\t\t\t\t\t\t\t\tres-ltng\t\t35\t35\t\t\t\t\t\t\t\t0\n";
                ss << u8"Thul Rune\tr10L\t18\tr10\tdmg-cold\t75\t3\t14\t\t\t\t\t\t\t\t\tres-cold\t\t30\t30\t\t\t\t\t\t\t\t\tres-cold\t\t35\t35\t\t\t\t\t\t\t\t0\n";
                ss << u8"Amn Rune\tr11L\t18\tr11\tlifesteal\t\t7\t7\t\t\t\t\t\t\t\t\tthorns\t\t14\t14\t\t\t\t\t\t\t\t\tthorns\t\t14\t14\t\t\t\t\t\t\t\t0\n";
                ss << u8"Sol Rune\tr12L\t18\tr12\tdmg-min\t\t9\t9\t\t\t\t\t\t\t\t\tred-dmg\t\t7\t7\t\t\t\t\t\t\t\t\tred-dmg\t\t7\t7\t\t\t\t\t\t\t\t0\n";
                ss << u8"Shael Rune\tr13L\t18\tr13\tswing2\t\t20\t20\t\t\t\t\t\t\t\t\tbalance2\t\t20\t20\t\t\t\t\t\t\t\t\tblock2\t\t20\t20\t\t\t\t\t\t\t\t0\n";
                ss << u8"Dol Rune\tr14L\t18\tr14\thowl\t\t32\t32\t\t\t\t\t\t\t\t\tregen\t\t7\t7\t\t\t\t\t\t\t\t\tregen\t\t7\t7\t\t\t\t\t\t\t\t0\n";
                ss << u8"Hel Rune\tr15L\t18\tr15\tease\t\t-20\t-20\t\t\t\t\t\t\t\t\tease\t\t-15\t-15\t\t\t\t\t\t\t\t\tease\t\t-15\t-15\t\t\t\t\t\t\t\t0\n";
                ss << u8"Io Rune\tr16L\t18\tr16\tvit\t\t10\t10\t\t\t\t\t\t\t\t\tvit\t\t10\t10\t\t\t\t\t\t\t\t\tvit\t\t10\t10\t\t\t\t\t\t\t\t0\n";
                ss << u8"Lum Rune\tr17L\t18\tr17\tenr\t\t10\t10\t\t\t\t\t\t\t\t\tenr\t\t10\t10\t\t\t\t\t\t\t\t\tenr\t\t10\t10\t\t\t\t\t\t\t\t0\n";
                ss << u8"Ko Rune\tr18L\t18\tr18\tdex\t\t10\t10\t\t\t\t\t\t\t\t\tdex\t\t10\t10\t\t\t\t\t\t\t\t\tdex\t\t10\t10\t\t\t\t\t\t\t\t0\n";
                ss << u8"Fal Rune\tr19L\t18\tr19\tstr\t\t10\t10\t\t\t\t\t\t\t\t\tstr\t\t10\t10\t\t\t\t\t\t\t\t\tstr\t\t10\t10\t\t\t\t\t\t\t\t0\n";
                ss << u8"Lem Rune\tr20L\t18\tr20\tgold%\t\t75\t75\t\t\t\t\t\t\t\t\tgold%\t\t50\t50\t\t\t\t\t\t\t\t\tgold%\t\t50\t50\t\t\t\t\t\t\t\t0\n";
                ss << u8"Pul Rune\tr21L\t18\tr21\tatt-demon\t\t100\t100\tdmg-demon\t\t75\t75\t\t\t\t\tac%\t\t30\t30\t\t\t\t\t\t\t\t\tac%\t\t30\t30\t\t\t\t\t\t\t\t0\n";
                ss << u8"Um Rune\tr22L\t18\tr22\topenwounds\t\t25\t25\t\t\t\t\t\t\t\t\tres-all\t\t15\t15\t\t\t\t\t\t\t\t\tres-all\t\t22\t22\t\t\t\t\t\t\t\t0\n";
                ss << u8"Mal Rune\tr23L\t18\tr23\tnoheal\t\t1\t1\t\t\t\t\t\t\t\t\tred-mag\t\t7\t7\t\t\t\t\t\t\t\t\tred-mag\t\t7\t7\t\t\t\t\t\t\t\t0\n";
                ss << u8"Ist Rune\tr24L\t18\tr24\tmag%\t\t30\t30\t\t\t\t\t\t\t\t\tmag%\t\t25\t25\t\t\t\t\t\t\t\t\tmag%\t\t25\t25\t\t\t\t\t\t\t\t0\n";
                ss << u8"Gul Rune\tr25L\t18\tr25\tatt%\t\t20\t20\t\t\t\t\t\t\t\t\tres-pois-max\t\t5\t5\t\t\t\t\t\t\t\t\tres-pois-max\t\t5\t5\t\t\t\t\t\t\t\t0\n";
                ss << u8"Vex Rune\tr26L\t18\tr26\tmanasteal\t\t7\t7\t\t\t\t\t\t\t\t\tres-fire-max\t\t5\t5\t\t\t\t\t\t\t\t\tres-fire-max\t\t5\t5\t\t\t\t\t\t\t\t0\n";
                ss << u8"Ohm Rune\tr27L\t18\tr27\tdmg%\t\t50\t50\t\t\t\t\t\t\t\t\tres-cold-max\t\t5\t5\t\t\t\t\t\t\t\t\tres-cold-max\t\t5\t5\t\t\t\t\t\t\t\t0\n";
                ss << u8"Lo Rune\tr28L\t18\tr28\tdeadly\t\t20\t20\t\t\t\t\t\t\t\t\tres-ltng-max\t\t5\t5\t\t\t\t\t\t\t\t\tres-ltng-max\t\t5\t5\t\t\t\t\t\t\t\t0\n";
                ss << u8"Sur Rune\tr29L\t18\tr29\tstupidity\t\t1\t1\t\t\t\t\t\t\t\t\tmana%\t\t5\t5\t\t\t\t\t\t\t\t\tmana\t\t50\t50\t\t\t\t\t\t\t\t0\n";
                ss << u8"Ber Rune\tr30L\t18\tr30\tcrush\t\t20\t20\t\t\t\t\t\t\t\t\tred-dmg%\t\t8\t8\t\t\t\t\t\t\t\t\tred-dmg%\t\t8\t8\t\t\t\t\t\t\t\t0\n";
                ss << u8"Jah Rune\tr31L\t18\tr31\tignore-ac\t\t1\t1\t\t\t\t\t\t\t\t\thp%\t\t5\t5\t\t\t\t\t\t\t\t\thp\t\t50\t50\t\t\t\t\t\t\t\t0\n";
                ss << u8"Cham Rune\tr32L\t18\tr32\tfreeze\t\t3\t3\t\t\t\t\t\t\t\t\tnofreeze\t\t1\t1\t\t\t\t\t\t\t\t\tnofreeze\t\t1\t1\t\t\t\t\t\t\t\t0\n";
                ss << u8"Zod Rune\tr33L\t18\tr33\tindestruct\t\t1\t1\t\t\t\t\t\t\t\t\tindestruct\t\t1\t1\t\t\t\t\t\t\t\t\tindestruct\t\t1\t1\t\t\t\t\t\t\t\t0\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}