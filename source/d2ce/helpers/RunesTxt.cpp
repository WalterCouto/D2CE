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
        const std::string& GetRunes()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"Name\t*Rune Name\tcomplete\tserver\t*Patch Release\titype1\titype2\titype3\titype4\titype5\titype6\tetype1\tetype2\tetype3\t*RunesUsed\tRune1\tRune2\tRune3\tRune4\tRune5\tRune6\tT1Code1\tT1Param1\tT1Min1\tT1Max1\tT1Code2\tT1Param2\tT1Min2\tT1Max2\tT1Code3\tT1Param3\tT1Min3\tT1Max3\tT1Code4\tT1Param4\tT1Min4\tT1Max4\tT1Code5\tT1Param5\tT1Min5\tT1Max5\tT1Code6\tT1Param6\tT1Min6\tT1Max6\tT1Code7\tT1Param7\tT1Min7\tT1Max7\t*eol\n";
                ss << u8"Runeword1\tAncients' Pledge\t1\t\t109\tshld\t\t\t\t\t\t\t\t\tRalOrtTal\tr08\tr09\tr07\t\t\t\tres-cold\t\t30\t30\tres-all\t\t13\t13\tac%\t\t50\t50\tdmg-to-mana\t\t10\t10\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword2\tArmageddon\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword3\tAuthority\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword4\tBeast\t1\t\t110\taxe\tscep\thamm\t\t\t\t\t\t\tBerTirUmMalLum\tr30\tr03\tr22\tr23\tr17\t\tswing2\t\t40\t40\taura\tFanaticism\t9\t9\tdmg%\t\t240\t270\tstr\t\t25\t40\tcharged\tSummon Grizzly\t5\t13\toskill\tWearbear\t3\t3\toskill\tShape Shifting\t3\t3\t0\n";
                ss << u8"Runeword5\tBeauty\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword6\tBlack\t1\t\t109\tclub\thamm\tmace\t\t\t\t\t\t\tThulIoNef\tr10\tr16\tr04\t\t\t\tcrush\t\t40\t40\tdmg%\t\t120\t120\tswing2\t\t15\t15\tred-mag\t\t2\t2\tatt\t\t200\t200\tcharged\t74\t12\t4\t\t\t\t\t0\n";
                ss << u8"Runeword7\tBlood\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword8\tBone\t1\t\t111\ttors\t\t\t\t\t\t\t\t\tSolUmUm\tr12\tr22\tr22\t\t\t\thit-skill\tBone Spear\t15\t10\tgethit-skill\tBone Armor\t15\t10\tnec\t\t2\t2\tmana\t\t100\t150\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword9\tBramble\t1\t\t110\ttors\t\t\t\t\t\t\t\t\tRalOhmSurEth\tr08\tr27\tr29\tr05\t\t\tbalance2\t\t50\t50\tac\t\t300\t300\taura\tThorns\t15\t21\theal-kill\t\t13\t13\textra-pois\t\t25\t50\tres-pois\t\t100\t100\tcharged\tSpirit of Barbs\t33\t13\t0\n";
                ss << u8"Runeword10\tBrand\t1\t\tPreviously Ladder Only\tmiss\t\t\t\t\t\t\t\t\tJahLoMalGul\tr31\tr28\tr23\tr25\t\t\tdmg%\t\t260\t340\tdmg-demon\t\t280\t330\thit-skill\tBone Spear\t100\t18\tgethit-skill\tAmplify Damage\t35\t14\tknock\t\t1\t1\tnoheal\t\t1\t1\texplosivearrow\t\t15\t15\t0\n";
                ss << u8"Runeword11\tBreath of the Dying\t1\t\t110\tweap\t\t\t\t\t\t\t\t\tVexHelElEldZodEth\tr26\tr15\tr01\tr02\tr33\tr05\tswing2\t\t60\t60\tdmg-undead\t\t125\t125\tlifesteal\t\t12\t15\tnoheal\t\t1\t1\tkill-skill\tPoison Nova\t50\t20\tdmg%\t\t350\t400\tall-stats\t\t30\t30\t0\n";
                ss << u8"Runeword12\tBroken Promise\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword13\tCall to Arms\t1\t\t110\tweap\t\t\t\t\t\t\t\t\tAmnRalMalIstOhm\tr11\tr08\tr23\tr24\tr27\t\tswing2\t\t40\t40\tdmg%\t\t200\t240\tallskills\t\t1\t1\toskill\tBattle Command\t2\t6\toskill\tBattle Orders\t1\t6\toskill\tBattle Cry\t1\t4\tregen\t\t12\t12\t0\n";
                ss << u8"Runeword14\tChains of Honor\t1\t\t110\ttors\t\t\t\t\t\t\t\t\tDolUmBerIst\tr14\tr22\tr30\tr24\t\t\tres-all\t\t50\t50\tac%\t\t70\t70\tdmg-demon\t\t200\t200\tdmg-undead\t\t100\t100\tlifesteal\t\t8\t8\tallskills\t\t2\t2\tstr\t\t20\t20\t0\n";
                ss << u8"Runeword15\tChance\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword16\tChaos\t1\t\t110\th2h\t\t\t\t\t\t\t\t\tFalOhmUm\tr19\tr27\tr22\t\t\t\tdemon-heal\t\t15\t15\tdmg%\t\t240\t290\tdmg-mag\t\t216\t471\toskill\tWhirlwind\t1\t1\tswing2\t\t35\t35\thit-skill\tFrozen Orb\t9\t11\thit-skill\tCharged Bolt\t11\t9\t0\n";
                ss << u8"Runeword17\tCrescent Moon\t1\t\t110\taxe\tswor\tpole\t\t\t\t\t\t\tShaelUmTir\tr13\tr22\tr03\t\t\t\tpierce-ltng\t\t35\t35\tignore-ac\t\t1\t1\tdmg%\t\t180\t220\tabs-mag\t\t9\t11\tcharged\tSummon Spirit Wolf\t30\t18\thit-skill\tStatic Field\t7\t13\thit-skill\tChain Lightning\t10\t17\t0\n";
                ss << u8"Runeword18\tDarkness\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword19\tDaylight\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword20\tDeath\t1\t\tPreviously Ladder Only\tswor\taxe\t\t\t\t\t\t\t\tHelElVexOrtGul\tr15\tr01\tr26\tr09\tr25\t\tdmg%\t\t300\t385\tdeadly/lvl\t4\t\t\tcharged\tBloodGolem\t15\t22\tatt-skill\tGlacial Spike\t25\t18\tdeath-skill\tChain Lightning\t100\t44\tcrush\t\t50\t50\tindestruct\t\t1\t1\t0\n";
                ss << u8"Runeword21\tDeception\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword22\tDelirium\t1\t\t110\thelm\t\t\t\t\t\t\t\t\tLemIstIo\tr20\tr24\tr16\t\t\t\thit-skill\tConfuse\t11\t18\tcharged\tAttract\t60\t17\tgethit-skill\tTerror\t14\t13\tac\t\t261\t261\tgethit-skill\tMind Blast\t6\t14\tgethit-skill\tDelerium Change\t1\t50\tallskills\t\t2\t2\t0\n";
                ss << u8"Runeword23\tDesire\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword24\tDespair\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword25\tDestruction\t1\t\tPreviously Ladder Only\tpole\tswor\t\t\t\t\t\t\t\tVexLoBerJahKo\tr26\tr28\tr30\tr31\tr18\t\tdmg%\t\t350\t350\tdmg-mag\t\t100\t180\thit-skill\tMolten Boulder\t5\t23\tdeath-skill\tMeteor\t100\t45\tatt-skill\tNova\t15\t22\thit-skill\tVolcano\t23\t12\tnoheal\t\t1\t1\t0\n";
                ss << u8"Runeword26\tDoom\t1\t\t110\taxe\tpole\thamm\t\t\t\t\t\t\tHelOhmUmLoCham\tr15\tr27\tr22\tr28\tr32\t\tdmg%\t\t280\t320\taura\tHoly Freeze\t12\t12\tswing2\t\t45\t45\tnoheal\t\t1\t1\tpierce-cold\t\t40\t60\tallskills\t\t2\t2\thit-skill\tVolcano\t5\t18\t0\n";
                ss << u8"Runeword27\tDragon\t1\t\tPreviously Ladder Only\ttors\tshld\t\t\t\t\t\t\t\tSurLoSol\tr29\tr28\tr12\t\t\t\tac\t\t360\t360\tac-miss\t\t230\t230\tstr/lvl\t3\t\t\thit-skill\tHydra\t12\t15\tgethit-skill\tVenom\t20\t18\taura\tHoly Fire\t14\t14\tall-stats\t\t3\t5\t0\n";
                ss << u8"Runeword28\tDread\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword29\tDream\t1\t\tPreviously Ladder Only\thelm\tshld\t\t\t\t\t\t\t\tIoJahPul\tr16\tr31\tr21\t\t\t\tac\t\t150\t220\tgethit-skill\tConfuse\t10\t15\tmana/lvl\t5\t\t\tres-all\t\t5\t20\tbalance3\t\t20\t30\taura\tHoly Shock\t15\t15\tmag%\t\t12\t25\t0\n";
                ss << u8"Runeword30\tDuress\t1\t\t110\ttors\t\t\t\t\t\t\t\t\tShaelUmThul\tr13\tr22\tr10\t\t\t\tdmg-cold\t50\t37\t133\tdmg%\t\t10\t20\tac%\t\t150\t200\tbalance2\t\t20\t20\topenwounds\t\t33\t33\tcrush\t\t15\t15\tstamdrain\t\t-20\t-20\t0\n";
                ss << u8"Runeword31\tEdge\t1\t\tPreviously Ladder Only\tmiss\t\t\t\t\t\t\t\t\tTirTalAmn\tr03\tr07\tr11\t\t\t\tdmg-demon\t\t320\t380\tdmg-undead\t\t280\t280\tswing2\t\t35\t35\tnoheal\t\t1\t1\taura\tThorns\t15\t15\tall-stats\t\t5\t10\tcheap\t\t15\t15\t0\n";
                ss << u8"Runeword32\tElation\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword33\tEnigma\t1\t\t110\ttors\t\t\t\t\t\t\t\t\tJahIthBer\tr31\tr06\tr30\t\t\t\tac\t\t750\t775\theal-kill\t\t14\t14\tmove2\t\t45\t45\tstr/lvl\t6\t\t\tallskills\t\t2\t2\tmag%/lvl\t8\t\t\toskill\tTeleport\t1\t1\t0\n";
                ss << u8"Runeword34\tEnlightenment\t1\t\t111\ttors\t\t\t\t\t\t\t\t\tPulRalSur\tr21\tr08\tr12\t\t\t\thit-skill\tFire Ball\t5\t15\tgethit-skill\tBlaze\t5\t15\tsor\t\t2\t2\toskill\tWarmth\t1\t1\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword35\tEnvy\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword36\tEternity\t1\t\t110\tmele\t\t\t\t\t\t\t\t\tAmnBerIstSolSur\tr11\tr30\tr24\tr12\tr29\t\tdmg%\t\t260\t310\tindestruct\t\t1\t1\tslow\t\t33\t33\tcharged\tRevive\t88\t8\tregen\t\t16\t16\tregen-mana\t\t16\t16\tnofreeze\t\t1\t1\t0\n";
                ss << u8"Runeword37\tExile\t1\t\t110\tpala\t\t\t\t\t\t\t\t\tVexOhmIstDol\tr26\tr27\tr24\tr14\t\t\tblock2\t\t30\t30\tfreeze\t\t1\t1\tac%\t\t220\t260\taura\tDefiance\t13\t16\tskilltab\t10\t2\t2\thit-skill\tLife Tap\t15\t5\trep-dur\t25\t\t\t0\n";
                ss << u8"Runeword38\tFaith\t1\t\t\"110, Previously Ladder Only\"\tmiss\t\t\t\t\t\t\t\t\tOhmJahLemEld\tr27\tr31\tr20\tr02\t\t\tdmg%\t\t280\t280\tatt%\t\t300\t300\tdmg-fire\t\t120\t120\tres-all\t\t15\t15\taura\tFanaticism\t12\t15\treanimate\t1\t10\t10\tallskills\t\t1\t2\t0\n";
                ss << u8"Runeword39\tFamine\t1\t\t\taxe\thamm\t\t\t\t\t\t\t\tFalOhmOrtJah\tr19\tr27\tr09\tr31\t\t\tdmg%\t\t270\t320\tlifesteal\t\t12\t12\tswing2\t\t30\t30\tnoheal\t\t1\t1\tdmg-mag\t\t180\t200\tdmg-elem\t\t50\t200\tethereal\t\t1\t1\t0\n";
                ss << u8"Runeword40\tFlickering Flame\t1\t1\tD2R Ladder 1\thelm\t\t\t\t\t\t\t\t\tNefPulVex\tr04\tr21\tr26\t\t\t\tfireskill\t\t3\t3\taura\tResist Fire\t4\t8\tpierce-fire\t\t10\t15\tmana\t\t50\t75\thalf-freeze\t\t1\t1\tres-pois-len\t\t50\t50\t\t\t\t\t0\n";
                ss << u8"Runeword41\tFortitude\t1\t\tPreviously Ladder Only\tweap\ttors\t\t\t\t\t\t\t\tElSolDolLo\tr01\tr12\tr14\tr28\t\t\tac%\t\t200\t200\tdmg%\t\t300\t300\tcast3\t\t25\t25\tgethit-skill\tChilling Armor\t20\t15\tdmg-to-mana\t\t12\t12\thp/lvl\t\t8\t12\tres-all\t\t25\t30\t0\n";
                ss << u8"Runeword42\tFortune\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword43\tAmity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword44\tFury\t1\t\t109\tmele\t\t\t\t\t\t\t\t\tJahGulEth\tr31\tr25\tr05\t\t\t\tdmg%\t\t209\t209\tswing2\t\t40\t40\tnoheal\t\t1\t1\topenwounds\t\t66\t66\tlifesteal\t\t6\t6\tdeadly\t\t33\t33\tskill\t147\t5\t5\t0\n";
                ss << u8"Runeword45\tGloom\t1\t\t110\ttors\t\t\t\t\t\t\t\t\tFalUmPul\tr19\tr22\tr21\t\t\t\tac%\t\t170\t230\tres-all\t\t30\t30\tgethit-skill\tDim Vision\t15\t3\tbalance2\t\t10\t10\tdmg-to-mana\t\t5\t5\tlight\t\t-3\t-3\thalf-freeze\t\t1\t1\t0\n";
                ss << u8"Runeword46\tGlory\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword47\tGrief\t1\t\tPreviously Ladder Only\tswor\taxe\t\t\t\t\t\t\t\tEthTirLoMalRal\tr05\tr03\tr28\tr23\tr08\t\tdmg-dem/lvl\t15\t\t\tdmg\t\t340\t400\tswing3\t\t30\t40\thit-skill\tVenom\t35\t15\tpierce-pois\t\t20\t25\tignore-ac\t\t1\t1\theal-kill\t\t10\t15\t0\n";
                ss << u8"Runeword48\tHand of Justice\t1\t\t110\tweap\t\t\t\t\t\t\t\t\tSurChamAmnLo\tr29\tr32\tr11\tr28\t\t\tswing2\t\t33\t33\tdmg%\t\t280\t330\taura\tHoly Fire\t16\t16\tlevelup-skill\tBlaze\t100\t36\tdeath-skill\tMeteor\t100\t48\tignore-ac\t\t1\t1\tpierce-fire\t\t20\t20\t0\n";
                ss << u8"Runeword49\tHarmony\t1\t\tPreviously Ladder Only\tmiss\t\t\t\t\t\t\t\t\tTirIthSolKo\tr03\tr06\tr12\tr18\t\t\tdmg%\t\t200\t275\tdmg-elem\t\t55\t160\tcharged\tRevive\t25\t20\taura\tVigor\t10\t10\toskill\tValkyrie\t2\t6\tregen-mana\t\t20\t20\tlight\t\t2\t2\t0\n";
                ss << u8"Runeword50\tHatred\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword51\tHeart of the Oak\t1\t\t110\tstaf\tmace\t\t\t\t\t\t\t\tKoVexPulThul\tr18\tr26\tr21\tr10\t\t\tcast2\t\t40\t40\tcharged\tOak Sage\t25\t4\tmana%\t\t15\t15\tallskills\t\t3\t3\tregen\t\t20\t20\tres-all\t\t30\t40\tcharged\tRaven\t60\t14\t0\n";
                ss << u8"Runeword52\tHeaven's Will\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword53\tHoly Tears\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword54\tHoly Thunder\t1\t\t109\tscep\t\t\t\t\t\t\t\t\tEthRalOrtTal\tr05\tr08\tr09\tr07\t\t\tdmg%\t\t60\t60\tdmg-ltng\t\t20\t60\tdmg-max\t\t10\t10\tres-ltng\t\t60\t60\tres-ltng-max\t\t5\t5\tskill\t118\t3\t3\tcharged\t53\t60\t7\t0\n";
                ss << u8"Runeword55\tHonor\t1\t\t109\tmele\t\t\t\t\t\t\t\t\tAmnElIthTirSol\tr11\tr01\tr06\tr03\tr12\t\tdmg%\t\t160\t160\tregen\t\t10\t10\tallskills\t\t1\t1\tatt\t\t200\t200\tdeadly\t\t25\t25\tstr\t\t10\t10\t\t\t\t\t0\n";
                ss << u8"Runeword56\tRevenge\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword57\tHumility\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword58\tHunger\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword59\tIce\t1\t\tPreviously Ladder Only\tmiss\t\t\t\t\t\t\t\t\tAmnShaelJahLo\tr11\tr13\tr31\tr28\t\t\tdmg%\t\t140\t210\taura\tHoly Freeze\t18\t18\textra-cold\t\t25\t30\thit-skill\tFrost Nova\t25\t22\tlevelup-skill\tBlizzard\t100\t40\tpierce-cold\t\t20\t20\tgold%/lvl\t25\t\t\t0\n";
                ss << u8"Runeword60\tInfinity\t1\t\tPreviously Ladder Only\tpole\tspea\t\t\t\t\t\t\t\tBerMalBerIst\tr30\tr23\tr30\tr24\t\t\tdmg%\t\t255\t325\tmove3\t\t35\t35\tvit/lvl\t4\t\t\taura\tConviction\t12\t12\tkill-skill\tChain Lightning\t50\t20\tpierce-ltng\t\t45\t55\tcharged\tCyclone Armor\t30\t21\t0\n";
                ss << u8"Runeword61\tInnocence\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword62\tInsight\t1\t\tPreviously Ladder Only\tpole\tstaf\tmiss\t\t\t\t\t\t\tRalTirTalSol\tr08\tr03\tr07\tr12\t\t\tdmg%\t\t200\t260\tatt%\t\t180\t250\tmag%\t\t23\t23\toskill\tCritical Strike\t1\t6\tcast2\t\t35\t35\taura\tMeditation\t12\t17\tall-stats\t\t5\t5\t0\n";
                ss << u8"Runeword63\tJealousy\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword64\tJudgment\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword65\tKing's Grace\t1\t\t109\tswor\tscep\t\t\t\t\t\t\t\tAmnRalThul\tr11\tr08\tr10\t\t\t\tdmg%\t\t100\t100\tatt\t\t150\t150\tdmg-demon\t\t100\t100\tdmg-undead\t\t50\t50\tatt-demon\t\t100\t100\tatt-undead\t\t100\t100\t\t\t\t\t0\n";
                ss << u8"Runeword66\tKingslayer\t1\t\t110\tswor\taxe\t\t\t\t\t\t\t\tMalUmGulFal\tr23\tr22\tr25\tr19\t\t\tswing2\t\t30\t30\tdmg%\t\t230\t270\treduce-ac\t\t25\t25\tcrush\t\t33\t33\topenwounds\t\t25\t25\toskill\tVengeance\t1\t1\tgold%\t\t40\t40\t0\n";
                ss << u8"Runeword67\tKnight's Vigil\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword68\tKnowledge\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword69\tLast Wish\t1\t\tPreviously Ladder Only\tswor\thamm\taxe\t\t\t\t\t\t\tJahMalJahSurJahBer\tr31\tr23\tr31\tr29\tr31\tr30\tdmg%\t\t330\t375\tatt-skill\tCharged Bolt\t20\t20\thit-skill\tLife Tap\t10\t18\tgethit-skill\tFade\t6\t11\tcrush\t\t40\t50\tmag%/lvl\t4\t\t\taura\tMight\t17\t17\t0\n";
                ss << u8"Runeword70\tLaw\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword71\tLawbringer\t1\t\tPreviously Ladder Only\tswor\thamm\tscep\t\t\t\t\t\t\tAmnLemKo\tr11\tr20\tr18\t\t\t\tdmg-cold\t\t130\t180\tdmg-fire\t\t150\t210\taura\tSanctuary\t16\t18\thit-skill\tDecrepify\t20\t15\tac-miss\t\t200\t250\trip\t\t1\t1\treduce-ac\t\t50\t50\t0\n";
                ss << u8"Runeword72\tLeaf\t1\t\t109\tstaf\t\t\t\t\t\t\t\t\tTirRal\tr03\tr08\t\t\t\t\tfireskill\t\t3\t3\tac/lvl\t\t16\t16\tres-cold\t\t33\t33\tskill\t41\t3\t3\tskill\t36\t3\t3\tskill\t37\t3\t3\t\t\t\t\t0\n";
                ss << u8"Runeword73\tLightning\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword74\tLionheart\t1\t\t109\ttors\t\t\t\t\t\t\t\t\tHelLumFal\tr15\tr17\tr19\t\t\t\tstr\t\t15\t15\tvit\t\t20\t20\tdex\t\t15\t15\tdmg%\t\t20\t20\thp\t\t50\t50\tres-all\t\t30\t30\t\t\t\t\t0\n";
                ss << u8"Runeword75\tLore\t1\t\t109\thelm\t\t\t\t\t\t\t\t\tOrtSol\tr09\tr12\t\t\t\t\tenr\t\t10\t10\tallskills\t\t1\t1\tlight\t\t2\t2\tmana-kill\t\t2\t2\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword76\tLove\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword77\tLoyalty\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword78\tLust\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword79\tMadness\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword81\tMalice\t1\t\t109\tmele\t\t\t\t\t\t\t\t\tIthElEth\tr06\tr01\tr05\t\t\t\topenwounds\t\t100\t100\tdmg-ac\t\t-100\t-100\tnoheal\t\t1\t1\tdmg%\t\t33\t33\tlight\t\t-1\t-1\tregen\t\t-5\t-5\t\t\t\t\t0\n";
                ss << u8"Runeword82\tMelody\t1\t\t109\tmiss\t\t\t\t\t\t\t\t\tShaelKoNef\tr13\tr18\tr04\t\t\t\tdmg%\t\t50\t50\tskilltab\t0\t3\t3\tskill\t9\t3\t3\tskill\t13\t3\t3\tskill\t17\t3\t3\tdmg-undead\t\t300\t300\t\t\t\t\t0\n";
                ss << u8"Runeword83\tMemory\t1\t\t109\tstaf\t\t\t\t\t\t\t\t\tLumIoSolEth\tr17\tr16\tr12\tr05\t\t\tmana%\t\t20\t20\tred-mag\t\t7\t7\tac%\t\t50\t50\tcast2\t\t33\t33\tsor\t\t3\t3\tskill\t58\t3\t3\tskill\t42\t2\t2\t0\n";
                ss << u8"Runeword84\tMist\t1\t1\tD2R Ladder 1\tmiss\t\t\t\t\t\t\t\t\tChamShaelGulThulIth\tr32\tr13\tr25\tr10\tr06\t\tallskills\t\t3\t3\tdmg%\t\t325\t375\taura\tConcentration\t8\t12\tpierce\t\t100\t100\tvit\t\t24\t24\tres-all\t\t40\t40\t\t\t\t\t0\n";
                ss << u8"Runeword85\tMorning\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword86\tMystery\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword87\tMyth\t1\t\t111\ttors\t\t\t\t\t\t\t\t\tHelAmnNef\tr15\tr11\tr04\t\t\t\thit-skill\tTaunt\t10\t1\tgethit-skill\tHowl\t3\t1\tbar\t\t2\t2\tregen\t\t10\t10\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword88\tNadir\t1\t\t109\thelm\t\t\t\t\t\t\t\t\tNefTir\tr04\tr03\t\t\t\t\tac%\t\t50\t50\tac\t\t10\t10\tlight\t\t-3\t-3\tcharged\t264\t9\t13\tgold%\t\t-33\t-33\tstr\t\t5\t5\t\t\t\t\t0\n";
                ss << u8"Runeword89\tNature's Kingdom\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword90\tNight\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword91\tOath\t1\t\tPreviously Ladder Only\tswor\taxe\tmace\t\t\t\t\t\t\tShaelPulMalLum\tr13\tr21\tr23\tr17\t\t\tdmg%\t\t210\t340\tswing1\t\t30\t30\thit-skill\tBone Spirit\t30\t20\tcharged\tIronGolem\t14\t17\tcharged\tHeart of Wolverine\t20\t16\tabs-mag\t\t10\t15\tindestruct\t\t1\t1\t0\n";
                ss << u8"Runeword92\tObedience\t1\t\tPreviously Ladder Only\tpole\tspea\t\t\t\t\t\t\t\tHelKoThulEthFal\tr15\tr18\tr10\tr05\tr19\t\tdmg%\t\t370\t370\tcrush\t\t40\t40\tkill-skill\tenchant\t30\t21\tpierce-fire\t\t25\t25\tac\t\t200\t300\tbalance3\t\t40\t40\tres-all\t\t20\t30\t0\n";
                ss << u8"Runeword93\tOblivion\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword94\tObsession\t1\t1\tD2R Ladder 1\tstaf\t\t\t\t\t\t\t\t\tZodIstLemLumIoNef\tr33\tr24\tr20\tr17\tr16\tr04\tallskills\t\t4\t4\tgethit-skill\tWeaken\t24\t10\tcast3\t\t65\t65\tbalance3\t\t60\t60\tres-all\t\t60\t70\thp%\t\t15\t25\tregen-mana\t\t15\t30\t0\n";
                ss << u8"Runeword95\tPassion\t1\t\t110\tweap\t\t\t\t\t\t\t\t\tDolOrtEldLem\tr14\tr09\tr02\tr20\t\t\tdmg%\t\t160\t210\toskill\tZeal\t1\t1\tatt%\t\t50\t80\toskill\tBerserk\t1\t1\tswing2\t\t25\t25\tcharged\tHeart of Wolverine\t12\t3\tstupidity\t\t10\t10\t0\n";
                ss << u8"Runeword96\tPatience\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword97\tPattern\t1\t1\tD2R Ladder 1\th2h\t\t\t\t\t\t\t\t\tTalOrtThul\tr07\tr09\tr10\t\t\t\tatt%\t\t10\t10\tdmg%\t\t40\t80\tdmg-fire\t\t12\t32\tres-all\t\t15\t15\tstr\t\t6\t6\tdex\t\t6\t6\tblock2\t\t30\t30\t0\n";
                ss << u8"Runeword98\tPeace\t1\t\t111\ttors\t\t\t\t\t\t\t\t\tShaelThulAmn\tr13\tr10\tr11\t\t\t\thit-skill\tValkyrie\t2\t15\tgethit-skill\tSlow Missiles\t4\t5\tama\t\t2\t2\toskill\tCritical Strike\t2\t2\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword99\tVoice of Reason\t1\t\tPreviously Ladder Only\tswor\tmace\t\t\t\t\t\t\t\tLemKoElEld\tr20\tr18\tr01\tr02\t\t\tdmg-demon\t\t220\t350\tdmg-undead\t\t280\t300\tdmg-cold\t\t100\t220\tpierce-cold\t\t24\t24\thit-skill\tIce Blast\t18\t20\thit-skill\tFrozen Orb\t15\t13\tnofreeze\t\t1\t1\t0\n";
                ss << u8"Runeword100\tPenitence\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword101\tPeril\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword102\tPestilence\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword103\tPhoenix\t1\t\tPreviously Ladder Only\tweap\tshld\t\t\t\t\t\t\t\tVexVexLoJah\tr26\tr26\tr28\tr31\t\t\tdmg%\t\t350\t400\tac-miss\t\t350\t400\thit-skill\tFirestorm\t40\t22\tlevelup-skill\tBlaze\t100\t40\tpierce-fire\t\t28\t28\taura\tRedemption\t10\t15\tabs-fire\t\t15\t21\t0\n";
                ss << u8"Runeword104\tPiety\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword105\tPillar of Faith\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword106\tPlague\t1\t1\tD2R Ladder 1\tswor\tknif\th2h\t\t\t\t\t\t\tChamShaelUm\tr32\tr13\tr22\t\t\t\tdmg%\t\t220\t320\tgethit-skill\tLower Resist\t20\t12\thit-skill\tPoison Nova\t25\t15\tpierce-pois\t\t23\t23\tdeadly/lvl\t3\t\t\taura\tCleansing\t13\t17\tallskills\t\t1\t2\t0\n";
                ss << u8"Runeword107\tPraise\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword108\tPrayer\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword109\tPride\t1\t\tPreviously Ladder Only\tpole\tspea\t\t\t\t\t\t\t\tChamSurIoLo\tr32\tr29\tr16\tr28\t\t\tdmg-dem/lvl\t8\t\t\tdmg-ltng\t\t50\t280\tatt%\t\t260\t300\taura\tConcentration\t16\t20\tgethit-skill\tFire Wall\t25\t17\tregen\t\t8\t8\tgold%/lvl\t15\t\t\t0\n";
                ss << u8"Runeword110\tPrinciple\t1\t\t111\ttors\t\t\t\t\t\t\t\t\tRalGulEld\tr08\tr25\tr02\t\t\t\thit-skill\tHoly Bolt\t100\t5\tpal\t\t2\t2\thp\t\t100\t150\tdmg-undead\t\t50\t50\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword111\tProwess in Battle\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword112\tPrudence\t1\t\t110\ttors\t\t\t\t\t\t\t\t\tMalTir\tr23\tr03\t\t\t\t\tac%\t\t140\t170\tred-mag\t\t10\t10\tred-dmg\t\t3\t3\tres-all\t\t25\t35\tbalance2\t\t25\t25\trep-dur\t25\t\t\tlight\t\t1\t1\t0\n";
                ss << u8"Runeword113\tPunishment\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword114\tPurity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword115\tQuestion\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword116\tRadiance\t1\t\t109\thelm\t\t\t\t\t\t\t\t\tNefSolIth\tr04\tr12\tr06\t\t\t\tlight\t\t5\t5\tenr\t\t10\t10\tvit\t\t10\t10\tred-mag\t\t3\t3\tmana\t\t33\t33\tac%\t\t75\t75\t\t\t\t\t0\n";
                ss << u8"Runeword117\tRain\t1\t\t111\ttors\t\t\t\t\t\t\t\t\tOrtMalIth\tr09\tr23\tr06\t\t\t\thit-skill\tTwister\t5\t15\tgethit-skill\tCyclone Armor\t5\t15\tdru\t\t2\t2\tmana\t\t100\t150\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword118\tReason\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword119\tRed\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword120\tRhyme\t1\t\t109\tshld\t\t\t\t\t\t\t\t\tShaelEth\tr13\tr05\t\t\t\t\tblock2\t\t20\t20\tblock\t\t20\t20\tres-all\t\t25\t25\tnofreeze\t\t1\t1\tgold%\t\t50\t50\tmag%\t\t25\t25\t\t\t\t\t0\n";
                ss << u8"Runeword121\tRift\t1\t\tPreviously Ladder Only\tpole\tscep\t\t\t\t\t\t\t\tHelKoLemGul\tr15\tr18\tr20\tr25\t\t\tdmg-mag\t\t160\t250\tdmg-fire\t\t60\t180\tdmg-to-mana\t\t38\t38\thit-skill\tTornado\t20\t16\tatt-skill\tFrozen Orb\t16\t21\tcharged\tIron Maiden\t40\t15\tall-stats\t\t5\t10\t0\n";
                ss << u8"Runeword122\tSanctuary\t1\t\t110\tshld\t\t\t\t\t\t\t\t\tKoKoMal\tr18\tr18\tr23\t\t\t\tblock\t\t20\t20\tblock2\t\t20\t20\tac%\t\t130\t160\tac-miss\t\t250\t250\tres-all\t\t50\t70\tbalance2\t\t20\t20\tcharged\tSlow Missiles\t60\t12\t0\n";
                ss << u8"Runeword123\tSerendipity\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword124\tShadow\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword125\tShadow of Doubt\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword126\tSilence\t1\t\t109\tweap\t\t\t\t\t\t\t\t\tDolEldHelIstTirVex\tr14\tr02\tr15\tr24\tr03\tr26\tmanasteal\t\t4\t4\tstupidity\t\t33\t33\tdmg%\t\t200\t200\tswing2\t\t20\t20\tres-all\t\t75\t75\tallskills\t\t2\t2\tbalance2\t\t20\t20\t0\n";
                ss << u8"Runeword127\tSiren's Song\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword128\tSmoke\t1\t\t109\ttors\t\t\t\t\t\t\t\t\tNefLum\tr04\tr17\t\t\t\t\tac-miss\t\t250\t250\tac%\t\t75\t75\tres-all\t\t50\t50\tbalance2\t\t20\t20\tlight\t\t-1\t-1\tcharged\t72\t18\t6\t\t\t\t\t0\n";
                ss << u8"Runeword129\tSorrow\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword130\tSpirit\t1\t\tPreviously Ladder Only\tswor\tshld\t\t\t\t\t\t\t\tTalThulOrtAmn\tr07\tr10\tr09\tr11\t\t\tbalance3\t\t55\t55\tmana\t\t89\t112\tac-miss\t\t250\t250\tvit\t\t22\t22\tcast3\t\t25\t35\tabs-mag\t\t3\t8\tallskills\t\t2\t2\t0\n";
                ss << u8"Runeword131\tSplendor\t1\t\t110\tshld\t\t\t\t\t\t\t\t\tEthLum\tr05\tr17\t\t\t\t\tlight\t\t3\t3\tgold%\t\t50\t50\tmag%\t\t20\t20\tac%\t\t60\t100\tblock2\t\t20\t20\tcast2\t\t10\t10\tallskills\t\t1\t1\t0\n";
                ss << u8"Runeword132\tStarlight\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword133\tStealth\t1\t\t109\ttors\t\t\t\t\t\t\t\t\tTalEth\tr07\tr05\t\t\t\t\tred-mag\t\t3\t3\tdex\t\t6\t6\tstam\t\t15\t15\tmove2\t\t25\t25\tcast2\t\t25\t25\tbalance2\t\t25\t25\t\t\t\t\t0\n";
                ss << u8"Runeword134\tSteel\t1\t\t109\tswor\taxe\tmace\t\t\t\t\t\t\tTirEl\tr03\tr01\t\t\t\t\tswing2\t\t25\t25\tdmg-min\t\t3\t3\tdmg-max\t\t3\t3\topenwounds\t\t50\t50\tdmg%\t\t20\t20\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword135\tStill Water\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword136\tSting\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword137\tStone\t1\t\t110\ttors\t\t\t\t\t\t\t\t\tShaelUmPulLum\tr13\tr22\tr21\tr17\t\t\tac%\t\t220\t260\tcharged\tClay Golem\t16\t16\tac-miss\t\t300\t300\tcharged\tMolten Boulder\t80\t16\tstr\t\t16\t16\tvit\t\t16\t16\tbalance2\t\t40\t40\t0\n";
                ss << u8"Runeword138\tStorm\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword139\tStrength\t1\t\t109\tmele\t\t\t\t\t\t\t\t\tAmnTir\tr11\tr03\t\t\t\t\tstr\t\t20\t20\tdmg%\t\t35\t35\tvit\t\t10\t10\tcrush\t\t25\t25\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword140\tTempest\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword141\tTemptation\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword142\tTerror\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword143\tThirst\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword144\tThought\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword145\tThunder\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword146\tTime\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword147\tTradition\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword148\tTreachery\t1\t\t111\ttors\t\t\t\t\t\t\t\t\tShaelThulLem\tr13\tr10\tr20\t\t\t\thit-skill\tVenom\t25\t15\tgethit-skill\tFade\t5\t15\tass\t\t2\t2\tswing2\t\t45\t45\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword149\tTrust\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword150\tTruth\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword151\tUnbending Will\t1\t1\tD2R Ladder 1\tswor\t\t\t\t\t\t\t\t\tFalIoIthEldElHel\tr19\tr16\tr06\tr02\tr01\tr15\tskilltab\t12\t3\t3\thit-skill\tTaunt\t18\t18\tswing2\t\t20\t30\tdmg%\t\t300\t350\tred-dmg\t\t8\t8\tnoheal\t\t1\t1\tlifesteal\t\t8\t10\t0\n";
                ss << u8"Runeword152\tValor\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword153\tVengeance\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword154\tVenom\t1\t\t109\tweap\t\t\t\t\t\t\t\t\tTalDolMal\tr07\tr14\tr23\t\t\t\tdmg-pois\t175\t312\t312\tignore-ac\t\t1\t1\tcharged\t83\t27\t15\tcharged\t92\t11\t13\tmanasteal\t\t7\t7\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword155\tVictory\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword156\tVoice\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword157\tVoid\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword158\tWar\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword159\tWater\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword160\tWealth\t1\t\t109\ttors\t\t\t\t\t\t\t\t\tLmKoTir\tr20\tr18\tr03\t\t\t\tgold%\t\t250\t250\tmag%\t\t100\t100\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword161\tWhisper\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword162\tWhite\t1\t\t109\twand\t\t\t\t\t\t\t\t\tDolIo\tr14\tr16\t\t\t\t\tskilltab\t7\t3\t3\tred-mag\t\t4\t4\tcast2\t\t20\t20\tmana\t\t13\t13\tskill\t68\t3\t3\tskill\t84\t2\t2\tskill\t69\t4\t4\t0\n";
                ss << u8"Runeword163\tWind\t1\t\t110\tmele\t\t\t\t\t\t\t\t\tSurEl\tr29\tr01\t\t\t\t\tdmg%\t\t120\t160\tswing2\t\t40\t40\tmove2\t\t20\t20\treduce-ac\t\t50\t50\thit-skill\tTornado\t10\t9\tcharged\t240\t127\t13\tbalance2\t\t15\t15\t0\n";
                ss << u8"Runeword164\tWings of Hope\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword165\tWisdom\t1\t1\tD2R Ladder 1\thelm\t\t\t\t\t\t\t\t\tPulIthEld\tr21\tr06\tr02\t\t\t\tpierce\t\t33\t33\tmanasteal\t\t4\t8\tatt%\t\t15\t25\tmana-kill\t\t5\t5\tnofreeze\t\t1\t1\tenr\t\t10\t10\t\t\t\t\t0\n";
                ss << u8"Runeword166\tWoe\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword167\tWonder\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword168\tWrath\t1\t\tPreviously Ladder Only\tmiss\t\t\t\t\t\t\t\t\tPulLumBerMal\tr21\tr17\tr30\tr23\t\t\tdmg-demon\t\t300\t300\tdmg-undead\t\t250\t300\tdmg-ltng\t\t41\t240\tdmg-mag\t\t85\t120\thit-skill\tLife Tap\t5\t10\thit-skill\tDecrepify\t30\t1\tnofreeze\t\t1\t1\t0\n";
                ss << u8"Runeword169\tYouth\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\n";
                ss << u8"Runeword170\tZephyr\t1\t\t109\tmiss\t\t\t\t\t\t\t\t\tOrtEth\tr09\tr05\t\t\t\t\tmove2\t\t25\t25\tswing2\t\t25\t25\tdmg%\t\t33\t33\tatt\t\t66\t66\tgethit-skill\t240\t7\t1\tac\t\t25\t25\t\t\t\t\t0\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}