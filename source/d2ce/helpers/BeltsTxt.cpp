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
        const std::string& GetBelts()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"name\tnumboxes\tbox1left\tbox1right\tbox1top\tbox1bottom\tbox2left\tbox2right\tbox2top\tbox2bottom\tbox3left\tbox3right\tbox3top\tbox3bottom\tbox4left\tbox4right\tbox4top\tbox4bottom\tbox5left\tbox5right\tbox5top\tbox5bottom\tbox6left\tbox6right\tbox6top\tbox6bottom\tbox7left\tbox7right\tbox7top\tbox7bottom\tbox8left\tbox8right\tbox8top\tbox8bottom\tbox9left\tbox9right\tbox9top\tbox9bottom\tbox10left\tbox10right\tbox10top\tbox10bottom\tbox11left\tbox11right\tbox11top\tbox11bottom\tbox12left\tbox12right\tbox12top\tbox12bottom\tbox13left\tbox13right\tbox13top\tbox13bottom\tbox14left\tbox14right\tbox14top\tbox14bottom\tbox15left\tbox15right\tbox15top\tbox15bottom\tbox16left\tbox16right\tbox16top\tbox16bottom\tdefaultItemTypeCol1\tdefaultItemCodeCol1\tdefaultItemTypeCol2\tdefaultItemCodeCol2\tdefaultItemTypeCol3\tdefaultItemCodeCol3\tdefaultItemTypeCol4\tdefaultItemCodeCol4\n";
                ss << u8"belt\t12\t343\t372\t442\t471\t374\t403\t442\t471\t405\t434\t442\t471\t436\t465\t442\t471\t343\t372\t410\t439\t374\t403\t410\t439\t405\t434\t410\t439\t436\t465\t410\t439\t343\t372\t378\t407\t374\t403\t378\t407\t405\t434\t378\t407\t436\t465\t378\t407\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"sash\t8\t343\t372\t442\t471\t374\t403\t442\t471\t405\t434\t442\t471\t436\t465\t442\t471\t343\t372\t410\t439\t374\t403\t410\t439\t405\t434\t410\t439\t436\t465\t410\t439\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"default\t4\t343\t372\t442\t471\t374\t403\t442\t471\t405\t434\t442\t471\t436\t465\t442\t471\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"girdle\t16\t343\t372\t442\t471\t374\t403\t442\t471\t405\t434\t442\t471\t436\t465\t442\t471\t343\t372\t410\t439\t374\t403\t410\t439\t405\t434\t410\t439\t436\t465\t410\t439\t343\t372\t378\t407\t374\t403\t378\t407\t405\t434\t378\t407\t436\t465\t378\t407\t343\t372\t346\t375\t374\t403\t346\t375\t405\t434\t346\t375\t436\t465\t346\t375\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"light belt\t8\t343\t372\t442\t471\t374\t403\t442\t471\t405\t434\t442\t471\t436\t465\t442\t471\t343\t372\t410\t439\t374\t403\t410\t439\t405\t434\t410\t439\t436\t465\t410\t439\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"heavy belt\t12\t343\t372\t442\t471\t374\t403\t442\t471\t405\t434\t442\t471\t436\t465\t442\t471\t343\t372\t410\t439\t374\t403\t410\t439\t405\t434\t410\t439\t436\t465\t410\t439\t343\t372\t378\t407\t374\t403\t378\t407\t405\t434\t378\t407\t436\t465\t378\t407\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"uber belt\t16\t343\t372\t442\t471\t374\t403\t442\t471\t405\t434\t442\t471\t436\t465\t442\t471\t343\t372\t410\t439\t374\t403\t410\t439\t405\t434\t410\t439\t436\t465\t410\t439\t343\t372\t378\t407\t374\t403\t378\t407\t405\t434\t378\t407\t436\t465\t378\t407\t343\t372\t346\t375\t374\t403\t346\t375\t405\t434\t346\t375\t436\t465\t346\t375\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"Expansion\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"belt2\t12\t423\t452\t562\t591\t454\t483\t562\t591\t485\t514\t562\t591\t516\t545\t562\t591\t423\t452\t530\t559\t454\t483\t530\t559\t485\t514\t530\t559\t516\t545\t530\t559\t423\t452\t498\t527\t454\t483\t498\t527\t485\t514\t498\t527\t516\t545\t498\t527\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"sash2\t8\t423\t452\t562\t591\t454\t483\t562\t591\t485\t514\t562\t591\t516\t545\t562\t591\t423\t452\t530\t559\t454\t483\t530\t559\t485\t514\t530\t559\t516\t545\t530\t559\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"default2\t4\t423\t452\t562\t591\t454\t483\t562\t591\t485\t514\t562\t591\t516\t545\t562\t591\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"girdle2\t16\t423\t452\t562\t591\t454\t483\t562\t591\t485\t514\t562\t591\t516\t545\t562\t591\t423\t452\t530\t559\t454\t483\t530\t559\t485\t514\t530\t559\t516\t545\t530\t559\t423\t452\t498\t527\t454\t483\t498\t527\t485\t514\t498\t527\t516\t545\t498\t527\t423\t452\t466\t495\t454\t483\t466\t495\t485\t514\t466\t495\t516\t545\t466\t495\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"light belt2\t8\t423\t452\t562\t591\t454\t483\t562\t591\t485\t514\t562\t591\t516\t545\t562\t591\t423\t452\t530\t559\t454\t483\t530\t559\t485\t514\t530\t559\t516\t545\t530\t559\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"heavy belt2\t12\t423\t452\t562\t591\t454\t483\t562\t591\t485\t514\t562\t591\t516\t545\t562\t591\t423\t452\t530\t559\t454\t483\t530\t559\t485\t514\t530\t559\t516\t545\t530\t559\t423\t452\t498\t527\t454\t483\t498\t527\t485\t514\t498\t527\t516\t545\t498\t527\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                ss << u8"uber belt\t16\t423\t452\t562\t591\t454\t483\t562\t591\t485\t514\t562\t591\t516\t545\t562\t591\t423\t452\t530\t559\t454\t483\t530\t559\t485\t514\t530\t559\t516\t545\t530\t559\t423\t452\t498\t527\t454\t483\t498\t527\t485\t514\t498\t527\t516\t545\t498\t527\t423\t452\t466\t495\t454\t483\t466\t495\t485\t514\t466\t495\t516\t545\t466\t495\thpot\t\tmpot\t\trpot\t\tscro\ttsc\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}