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
        const std::string& GetCharStats()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"class\tstr\tdex\tint\tvit\tstamina\thpadd\tManaRegen\tToHitFactor\tWalkVelocity\tRunVelocity\tRunDrain\t*Comment\tLifePerLevel\tStaminaPerLevel\tManaPerLevel\tLifePerVitality\tStaminaPerVitality\tManaPerMagic\tStatPerLevel\tSkillsPerLevel\tLightRadius\tBlockFactor\tMinimumCastingDelay\tStartSkill\tSkill 1\tSkill 2\tSkill 3\tSkill 4\tSkill 5\tSkill 6\tSkill 7\tSkill 8\tSkill 9\tSkill 10\tStrAllSkills\tStrSkillTab1\tStrSkillTab2\tStrSkillTab3\tStrClassOnly\tHealthPotionPercent\tManaPotionPercent\tbaseWClass\titem1\titem1loc\titem1count\titem1quality\titem2\titem2loc\titem2count\titem2quality\titem3\titem3loc\titem3count\titem3quality\titem4\titem4loc\titem4count\titem4quality\titem5\titem5loc\titem5count\titem5quality\titem6\titem6loc\titem6count\titem6quality\titem7\titem7loc\titem7count\titem7quality\titem8\titem8loc\titem8count\titem8quality\titem9\titem9loc\titem9count\titem9quality\titem10\titem10loc\titem10count\titem10quality\n";
                ss << u8"Amazon\t20\t25\t15\t20\t84\t30\t120\t5\t6\t9\t20\tThe following are in fourths\t8\t4\t6\t12\t4\t6\t5\t1\t13\t25\t12\t\tThrow\tKick\tScroll of Identify\tBook of Identify\tScroll of Townportal\tBook of Townportal\tUnsummon\t\t\t\tModStr3a\tStrSklTabItem3\tStrSklTabItem2\tStrSklTabItem1\tAmaOnly\t150\t150\thth\tjav\trarm\t1\t2\tbuc\tlarm\t1\t2\thp1\t\t4\t2\ttsc\t\t1\t2\tisc\t\t1\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\n";
                ss << u8"Sorceress\t10\t25\t35\t10\t74\t30\t120\t-15\t6\t9\t20\tThe following are in fourths\t4\t4\t8\t8\t4\t8\t5\t1\t13\t20\t12\tfire bolt\tThrow\tKick\tScroll of Identify\tBook of Identify\tScroll of Townportal\tBook of Townportal\tUnsummon\t\t\t\tModStr3d\tStrSklTabItem15\tStrSklTabItem14\tStrSklTabItem13\tSorOnly\t100\t200\thth\tsst\trarm\t1\t2\thp1\t\t4\t2\ttsc\t\t1\t2\tisc\t\t1\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\n";
                ss << u8"Necromancer\t15\t25\t25\t15\t79\t30\t120\t-10\t6\t9\t20\tThe following are in fourths\t6\t4\t8\t8\t4\t8\t5\t1\t13\t20\t12\traise skeleton\tThrow\tKick\tScroll of Identify\tBook of Identify\tScroll of Townportal\tBook of Townportal\tUnsummon\t\t\t\tModStr3c\tStrSklTabItem8\tStrSklTabItem7\tStrSklTabItem9\tNecOnly\t100\t200\thth\twnd\trarm\t1\t2\thp1\t\t4\t2\ttsc\t\t1\t2\tisc\t\t1\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\n";
                ss << u8"Paladin\t25\t20\t15\t25\t89\t30\t120\t20\t6\t9\t20\tThe following are in fourths\t8\t4\t6\t12\t4\t6\t5\t1\t13\t30\t12\t\tThrow\tKick\tScroll of Identify\tBook of Identify\tScroll of Townportal\tBook of Townportal\tUnsummon\t\t\t\tModStr3b\tStrSklTabItem6\tStrSklTabItem5\tStrSklTabItem4\tPalOnly\t150\t150\thth\tssd\trarm\t1\t2\tbuc\tlarm\t1\t2\thp1\t\t4\t2\ttsc\t\t1\t2\tisc\t\t1\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\n";
                ss << u8"Barbarian\t30\t20\t10\t25\t92\t30\t120\t20\t6\t9\t20\tThe following are in fourths\t8\t4\t4\t16\t4\t4\t5\t1\t13\t25\t12\t\tThrow\tKick\tScroll of Identify\tBook of Identify\tScroll of Townportal\tBook of Townportal\tLeft Hand Throw\tLeft Hand Swing\tUnsummon\t\tModStr3e\tStrSklTabItem11\tStrSklTabItem12\tStrSklTabItem10\tBarOnly\t200\t100\thth\thax\trarm\t1\t2\tbuc\tlarm\t1\t2\thp1\t\t4\t2\ttsc\t\t1\t2\tisc\t\t1\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\n";
                ss << u8"Expansion\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t0\t\n";
                ss << u8"Druid\t15\t20\t20\t25\t84\t30\t120\t5\t6\t9\t20\tThe following are in fourths\t6\t4\t8\t8\t4\t8\t5\t1\t13\t20\t12\t\tThrow\tKick\tScroll of Identify\tBook of Identify\tScroll of Townportal\tBook of Townportal\tUnsummon\t\t\t\tModStre8a\tStrSklTabItem16\tStrSklTabItem17\tStrSklTabItem18\tDruOnly\t100\t200\thth\tclb\trarm\t1\t2\tbuc\tlarm\t1\t2\thp1\t\t4\t2\ttsc\t\t1\t2\tisc\t\t1\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\n";
                ss << u8"Assassin\t20\t20\t25\t20\t95\t30\t120\t15\t6\t9\t15\tThe following are in fourths\t8\t5\t6\t12\t5\t7\t5\t1\t13\t25\t12\t\tThrow\tKick\tScroll of Identify\tBook of Identify\tScroll of Townportal\tBook of Townportal\tLeft Hand Swing\tUnsummon\t\t\tModStre8b\tStrSklTabItem19\tStrSklTabItem20\tStrSklTabItem21\tAssOnly\t150\t150\thth\tktr\trarm\t1\t2\tbuc\tlarm\t1\t2\thp1\t\t4\t2\ttsc\t\t1\t2\tisc\t\t1\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\t0\t\t0\t2\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}