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
    namespace JsonReaderHelper
    {
        const std::string& GetBankExpansionLayout()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"{\n";
                ss << u8"    \"basedOn\": \"BankOriginalLayout.json\",\n";
                ss << u8"    \"type\": \"BankPanel\", \"name\": \"BankExpansionLayout\",\n";
                ss << u8"    \"children\": [\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ClickCatcherWidget\", \"name\": \"click_catcher\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"GridImageWidget\", \"name\": \"background\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"filename\": \"PANEL\\expandedstash\",\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"TextBoxWidget\", \"name\": \"gold_max\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 78, \"y\": 35 },\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"TextBoxWidget\", \"name\": \"gold_amount\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 40, \"y\": 358 },\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ButtonWidget\", \"name\": \"gold_withdraw\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 15, \"y\": 357 },\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ButtonWidget\", \"name\": \"close\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 272, \"y\": 384 },\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventoryGridWidget\", \"name\": \"grid\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 15, \"y\": 63 },\n";
                ss << u8"                \"cellCount\": { \"x\": 10, \"y\": 10 },\n";
                ss << u8"                \"cellSize\": \"$ItemCellSize\",\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"TabBarWidget\", \"name\": \"BankTabs\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"tabCount\": 4,\n";
                ss << u8"                \"filename\": \"PANEL\\buyselltabs\",\n";
                ss << u8"                \"inactiveFrames\": [ 4, 5, 6, 7],\n";
                ss << u8"                \"activeFrames\": [ 0, 1, 2, 3],\n";
                ss << u8"                \"disabledFrames\": [-1, 5, ],\n";
                ss << u8"                \"textStrings\": [ \"@personal\", \"@shared\", \"@shared\", \"@shared\"],\n";
                ss << u8"                \"tabSize\": { \"x\": 79, \"y\": 24 },\n";
                ss << u8"                \"tabPadding\": { \"x\": 1, \"y\": 0 },\n";
                ss << u8"                \"onSwitchTabMessage\": \"BankPanelMessage:SelectTab\",\n";
                ss << u8"            }\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"Widget\", \"name\": \"PreviousSeasonToggleDisplay\",\n";
                ss << u8"            \"children\": [\n";
                ss << u8"                {\n";
                ss << u8"                    \"type\": \"TextBoxWidget\", \"name\": \"ShowPreviousLadderText\",\n";
                ss << u8"                    \"fields\": {\n";
                ss << u8"                        \"text\": \"@oldStash\",\n";
                ss << u8"                        \"rect\": { \"x\": 45, \"y\": 404 },\n";
                ss << u8"                        \"style\": {\n";
                ss << u8"                            \"fontColor\": \"$PanelTitleColor\",\n";
                ss << u8"                            \"pointSize\": \"$MediumFontSize\",\n";
                ss << u8"                            \"alignment\": { \"v\": \"center\", \"h\": \"left\" },\n";
                ss << u8"                            \"spacing\" : \"$MinimumSpacing\",\n";
                ss << u8"                            \"dropShadow\": \"$DefaultDropShadow\"\n";
                ss << u8"                        }\n";
                ss << u8"                    }\n";
                ss << u8"                },\n";
                ss << u8"                {\n";
                ss << u8"                    \"type\": \"ToggleButtonWidget\", \"name\": \"ShowPreviousLadderSeasonToggle\",\n";
                ss << u8"                    \"fields\": {\n";
                ss << u8"                        \"rect\": { \"x\": 10, \"y\": 388 },\n";
                ss << u8"                        \"filename\": \"PANEL\\buysellbtn\",\n";
                ss << u8"                        \"untoggledFrame\": 0,\n";
                ss << u8"                        \"untoggledPressedFrame\": 1,\n";
                ss << u8"                        \"toggledFrame\": 16,\n";
                ss << u8"                        \"toggledPressedFrame\": 17,\n";
                ss << u8"                        \"onClickMessage\": \"BankPanelMessage:TogglePreviousSeasonTabs\",\n";
                ss << u8"                        \"tooltipString\": \"@SharedStashPreviousSeasonToggleTooltip\",\n";
                ss << u8"                    }\n";
                ss << u8"                }\n";
                ss << u8"            ]\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"TabBarWidget\", \"name\": \"PreviousLadderSeasonBankTabs\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"tabCount\": 3,\n";
                ss << u8"                \"filename\": \"PANEL\\buyselltabs\",\n";
                ss << u8"                \"inactiveFrames\": [ 12, 13, 14 ],\n";
                ss << u8"                \"activeFrames\": [ 8, 9, 10 ],\n";
                ss << u8"                \"textStrings\": [ \"@shared\", \"@shared\", \"@shared\"],\n";
                ss << u8"                \"tabSize\": { \"x\": 79, \"y\": 24 },\n";
                ss << u8"                \"tabPadding\": { \"x\": 1, \"y\": 0 },\n";
                ss << u8"                \"onSwitchTabMessage\": \"BankPanelMessage:SelectTab\",\n";
                ss << u8"            }\n";
                ss << u8"        },\n";
                ss << u8"    ]\n";
                ss << u8"}\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }

        const std::string& GetHoradricCubeLayout()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"{\n";
                ss << u8"    \"type\": \"HoradricCubePanel\", \"name\": \"HoradricCubeLayout\",\n";
                ss << u8"    \"fields\": {\n";
                ss << u8"        \"priority\": 10,\n";
                ss << u8"        \"rect\": { \"x\": -320, \"y\": -240, \"width\": 320, \"height\": 432 },\n";
                ss << u8"        \"anchor\": \"$LeftPanelAnchor\",\n";
                ss << u8"    },\n";
                ss << u8"    \"children\": [\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ClickCatcherWidget\", \"name\": \"click_catcher\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"width\": 320, \"height\": 432 }\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"GridImageWidget\", \"name\": \"background\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"filename\": \"PANEL\\supertransmogrifier\",\n";
                ss << u8"                \"frames\": 4,\n";
                ss << u8"                \"rows\": 2,\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ButtonWidget\", \"name\": \"convert\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 144, \"y\": 260 },\n";
                ss << u8"                \"filename\": \"PANEL\\miniconvert\",\n";
                ss << u8"                \"tooltipString\": \"@strUiMenu2\",\n";
                ss << u8"                \"tooltipOffset\": { \"x\": 0, \"y\": -3 },\n";
                ss << u8"                \"onClickMessage\": \"HoradricCubePanelMessage:Convert\",\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ButtonWidget\", \"name\": \"close\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 275, \"y\": 383 },\n";
                ss << u8"                \"filename\": \"PANEL\\buysellbtn\",\n";
                ss << u8"                \"normalFrame\": 10,\n";
                ss << u8"                \"pressedFrame\": 11,\n";
                ss << u8"                \"tooltipString\": \"@strClose\",\n";
                ss << u8"                \"tooltipOffset\": { \"x\": 0, \"y\": -3 },\n";
                ss << u8"                \"onClickMessage\": \"HoradricCubePanelMessage:Close\",\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventoryGridWidget\", \"name\": \"grid\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 118, \"y\": 139 },\n";
                ss << u8"                \"cellCount\": { \"x\": 3, \"y\": 4 },\n";
                ss << u8"                \"cellSize\": \"$ItemCellSize\",\n";
                ss << u8"                \"gemSocketFilename\": \"PANEL\\gemsocket\",\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"AnimatedImageWidget\", \"name\": \"flourish\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 320, \"y\": 237 },\n";
                ss << u8"                \"filename\": \"MENU\\horadric\",\n";
                ss << u8"                \"blendMode\": \"black\",\n";
                ss << u8"                \"fps\": 14,\n";
                ss << u8"                \"playing\": false,\n";
                ss << u8"                \"loop\": false,\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"    ]\n";
                ss << u8"}\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }

        const std::string& GetPlayerInventoryExpansionLayout()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"{\n";
                ss << u8"    \"basedOn\": \"PlayerInventoryOriginalLayout.json\",\n";
                ss << u8"    \"type\": \"PlayerInventoryPanel\", \"name\": \"PlayerInventoryExpansionLayout\",\n";
                ss << u8"    \"children\": [\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ClickCatcherWidget\", \"name\": \"click_catcher\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"GridImageWidget\", \"name\": \"background\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"filename\": \"PANEL\\invchar6\",\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ImageWidget\", \"name\": \"background_right_arm\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 15, \"y\": 22 },\n";
                ss << u8"                \"filename\": \"PANEL\\invchar6Tab\",\n";
                ss << u8"                \"frame\": 1,\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ImageWidget\", \"name\": \"background_left_arm\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 246, \"y\": 22 },\n";
                ss << u8"                \"filename\": \"PANEL\\invchar6Tab\",\n";
                ss << u8"                \"frame\": 0,\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"FocusableWidget\", \"name\": \"weaponswap_right_arm\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 20, \"y\": 23, \"width\": 56, \"height\": 24 },\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"FocusableWidget\", \"name\": \"weaponswap_left_arm\",\n";
                ss << u8"            \"fields\": {\n";
                ss << u8"                \"rect\": { \"x\": 251, \"y\": 23, \"width\": 56, \"height\": 24 },\n";
                ss << u8"            },\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"TextBoxWidget\", \"name\": \"gold_amount\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ButtonWidget\", \"name\": \"gold_button\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"ButtonWidget\", \"name\": \"close\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventoryGridWidget\", \"name\": \"grid\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_head\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_neck\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_torso\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_right_arm\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_left_arm\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_right_hand\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_left_hand\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_belt\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_feet\",\n";
                ss << u8"        },\n";
                ss << u8"        {\n";
                ss << u8"            \"type\": \"InventorySlotWidget\", \"name\": \"slot_gloves\",\n";
                ss << u8"        },\n";
                ss << u8"    ]\n";
                ss << u8"}\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}