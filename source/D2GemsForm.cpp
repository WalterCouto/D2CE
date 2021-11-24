/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
    Copyright (C) 2021 Walter Couto

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
#include "D2Editor.h"
#include "D2GemsForm.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    CString GetGPSNameFromCode(const std::array<std::uint8_t, 4>& gemCode)
    {
        const std::uint8_t& gem = gemCode[0];
        const std::uint8_t& gemCondition = gemCode[1];
        const std::uint8_t& gemColour = gemCode[2];
        switch (gem)
        {
        case 'g': // gem
            switch (gemColour)
            {
            case 'v': // amethyst
                switch (gemCondition)
                {
                case 'c': // chipped
                    return _T("Chipped Amethysts");
                case 'f': // flawed
                    return _T("Flawed Amethysts");
                case 's': // regular
                    return _T("Regular Amethysts");
                case 'z': // flawless
                    return _T("Flawless Amethysts");
                default: // perfect
                    return _T("Perfect Amethysts");
                }
                break;
            case 'y': // topaz
                switch (gemCondition)
                {
                case 'c': // chipped
                    return _T("Chipped Topazes");
                case 'f': // flawed
                    return _T("Flawed Topazes");
                case 's': // regular
                    return _T("Regular Topazes");
                case 'l': // flawless
                    return _T("Flawless Topazes");
                default: // perfect
                    return _T("Perfect Topazes");
                }
                break;
            case 'b': // sapphire
                switch (gemCondition)
                {
                case 'c': // chipped
                    return _T("Chipped Sapphires");
                case 'f': // flawed
                    return _T("Flawed Sapphires");
                case 's': // regular
                    return _T("Regular Sapphires");
                case 'l': // flawless
                    return _T("Flawless Sapphires");
                default: // perfect
                    return _T("Perfect Sapphires");
                }
                break;
            case 'g': // emerald
                switch (gemCondition)
                {
                case 'c': // chipped
                    return _T("Chipped Emeralds");
                case 'f': // flawed
                    return _T("Flawed Emeralds");
                case 's': // regular
                    return _T("Regular Emeralds");
                case 'l': // flawless
                    return _T("Flawless Emeralds");
                default: // perfect
                    return _T("Perfect Emeralds");
                }
                break;
            case 'r': // ruby
                switch (gemCondition)
                {
                case 'c': // chipped
                    return _T("Chipped Rubies");
                case 'f': // flawed
                    return _T("Flawed Rubies");
                case 's': // regular
                    return _T("Regular Rubies");
                case 'l': // flawless
                    return _T("Flawless Rubies");
                default: // perfect
                    return _T("Perfect Rubies");
                }
                break;
            case 'w': // diamond
                switch (gemCondition)
                {
                case 'c': // chipped
                    return _T("Chipped Diamonds");
                case 'f': // flawed
                    return _T("Flawed Diamonds");
                case 's': // regular
                    return _T("Regular Diamonds");
                case 'l': // flawless
                    return _T("Flawless Diamonds");
                default: // perfect
                    return _T("Perfect Diamonds");
                }
                break;
            }
            break;

        case 's': // skull
            if (gemCondition == 'k')
            {
                switch (gemColour)
                {
                case 'c': // chipped
                    return _T("Chipped Skulls");
                case 'f': // flawed
                    return _T("Flawed Skulls");
                case 'u': // regular
                    return _T("Regular Skulls");
                case 'l': // flawless
                    return _T("Flawless Skulls");
                default: // perfect
                    return _T("Perfect Skulls");
                }
            }
            break;

        case 'h': // healing
            if (gemCondition == 'p')
            {
                switch (gemColour)
                {
                case '1': // Minor
                    return _T("Minor Healing Potion");
                case '2': // Light
                    return _T("Light Healing Potion");
                case '3': // Regular
                    return _T("Healing Potion");
                case '4': // Greater
                    return _T("Greater Healing Potion");
                default: // Super
                    return _T("Super Healing Potion");
                }
            }
            break;

        case 'm': // mana
            if (gemCondition == 'p')
            {
                switch (gemColour)
                {
                case '1': // Minor
                    return _T("Minor Mana Potion");
                case '2': // Light
                    return _T("Light Mana Potion");
                case '3': // Regular
                    return _T("Mana Potion");
                case '4': // Greater
                    return _T("Greater Mana Potion");
                default: // Super
                    return _T("Super Mana Potion");
                }
            }
            break;

        case 'r':
            switch (gemCondition)
            {
            case 'v': // rejuvenation potion
                switch (gemColour)
                {
                case 's': // Regular
                    return _T("Rejuvenation Potion");
                default: // Full
                    return _T("Full Rejuvenation Potion");
                }
                break;

            case '0': // El Rune - Ort Rune
                switch (gemColour)
                {
                case '1':
                    return _T("El Rune");

                case '2':
                    return _T("Eld Rune");

                case '3':
                    return _T("Tir Rune");

                case '4':
                    return _T("Nef Rune");

                case '5':
                    return _T("Eth Rune");

                case '6':
                    return _T("Ith Rune");

                case '7':
                    return _T("Tal Rune");

                case '8':
                    return _T("Ral Rune");

                case '9':
                    return _T("Ort Rune");
                }
                break;

            case '1': // Thul Rune - Fal Rune
                switch (gemColour)
                {
                case '0':
                    return _T("Thul Rune");

                case '1':
                    return _T("Amn Rune");

                case '2':
                    return _T("Sol Rune");

                case '3':
                    return _T("Shael Rune");

                case '4':
                    return _T("Dol Rune");

                case '5':
                    return _T("Hel Rune");

                case '6':
                    return _T("Io Rune");

                case '7':
                    return _T("Lum Rune");

                case '8':
                    return _T("Ko Rune");

                case '9':
                    return _T("Fal Rune");
                }
                break;

            case '2': // Lem Rune - Sur Rune
                switch (gemColour)
                {
                case '0':
                    return _T("Lem Rune");

                case '1':
                    return _T("Pul Rune");

                case '2':
                    return _T("Um Rune");

                case '3':
                    return _T("Mal Rune");

                case '4':
                    return _T("Ist Rune");

                case '5':
                    return _T("Gul Rune");

                case '6':
                    return _T("Vex Rune");

                case '7':
                    return _T("Ohm Rune");

                case '8':
                    return _T("Lo Rune");

                case '9':
                    return _T("Sur Rune");
                }
                break;

            case '3': // Ber Rune - Zod Rune
                switch (gemColour)
                {
                case '0':
                    return _T("Ber Rune");

                case '1':
                    return _T("Jah Rune");

                case '2':
                    return _T("Cham Rune");

                case '3':
                    return _T("Zod Rune");
                }
                break;
            }
            break;

        case 'y': // antidote potion
            if (gemCondition == 'p' && gemColour == 's')
            {
                return _T("Antidote Potion");
            }
            break;

        case 'v': //stamina potion
            if (gemCondition == 'p' && gemColour == 's')
            {
                return _T("Stamina Potion");
            }
            break;

        case 'w': // thawing potion
            if (gemCondition == 'm' && gemColour == 's')
            {
                return _T("Thawing Potion");
            }
            break;
        }

        return _T(""); // unknown
    }

    size_t GetGPSSortIndex(const std::array<std::uint8_t, 4>& gemCode)
    {
        size_t idx = 0;
        const std::uint8_t& gem = gemCode[0];
        const std::uint8_t& gemCondition = gemCode[1];
        const std::uint8_t& gemColour = gemCode[2];
        switch (gem)
        {
        case 'g': // gem
            switch (gemColour)
            {
            case 'v': // amethyst
                idx = 0;
                switch (gemCondition)
                {
                case 'c': // chipped
                    return idx;
                case 'f': // flawed
                    return idx + 7;
                case 's': // regular
                    return idx + 14;
                case 'z': // flawless
                    return idx + 21;
                default: // perfect
                    return idx + 28;
                }
                break;
            case 'y': // topaz
                idx = 6;
                switch (gemCondition)
                {
                case 'c': // chipped
                    return idx;
                case 'f': // flawed
                    return idx + 7;
                case 's': // regular
                    return idx + 14;
                case 'l': // flawless
                    return idx + 21;
                default: // perfect
                    return idx + 28;
                }
                break;
            case 'b': // sapphire
                idx = 4;
                switch (gemCondition)
                {
                case 'c': // chipped
                    return idx;
                case 'f': // flawed
                    return idx + 7;
                case 's': // regular
                    return idx + 14;
                case 'l': // flawless
                    return idx + 21;
                default: // perfect
                    return idx + 28;
                }
                break;
            case 'g': // emerald
                idx = 2;
                switch (gemCondition)
                {
                case 'c': // chipped
                    return idx;
                case 'f': // flawed
                    return idx + 7;
                case 's': // regular
                    return idx + 14;
                case 'l': // flawless
                    return idx + 21;
                default: // perfect
                    return idx + 28;
                }
                break;
            case 'r': // ruby
                idx = 3;
                switch (gemCondition)
                {
                case 'c': // chipped
                    return idx;
                case 'f': // flawed
                    return idx + 7;
                case 's': // regular
                    return idx + 14;
                case 'l': // flawless
                    return idx + 21;
                default: // perfect
                    return idx + 28;
                }
                break;
            case 'w': // diamond
                idx = 1;
                switch (gemCondition)
                {
                case 'c': // chipped
                    return idx;
                case 'f': // flawed
                    return idx + 7;
                case 's': // regular
                    return idx + 14;
                case 'l': // flawless
                    return idx + 21;
                default: // perfect
                    return idx + 28;
                }
                break;
            }
            break;

        case 's': // skull
            idx = 5;
            if (gemCondition == 'k')
            {
                switch (gemColour)
                {
                case 'c': // chipped
                    return idx;
                case 'f': // flawed
                    return idx + 7;
                case 'u': // regular
                    return idx + 14;
                case 'l': // flawless
                    return idx + 21;
                default: // perfect
                    return idx + 28;
                }
            }
            break;

        case 'h': // healing
            if (gemCondition == 'p')
            {
                idx = 35;
                switch (gemColour)
                {
                case '1': // Minor
                    return idx;
                case '2': // Light
                    return idx + 1;
                case '3': // Regular
                    return idx + 2;
                case '4': // Greater
                    return idx + 3;
                default: // Super
                    return idx + 4;
                }
            }
            break;

        case 'm': // mana
            if (gemCondition == 'p')
            {
                idx = 40;
                switch (gemColour)
                {
                case '1': // Minor
                    return idx;
                case '2': // Light
                    return idx + 1;
                case '3': // Regular
                    return idx + 2;
                case '4': // Greater
                    return idx + 3;
                default: // Super
                    return idx + 4;
                }
            }
            break;

        case 'r':

            switch (gemCondition)
            {
            case 'v': // rejuvenation potion
                idx = 45;
                switch (gemColour)
                {
                case 's': // Regular
                    return idx;
                default: // Full
                    return idx + 1;
                }
                break;
            case '0': // El Rune - Ort Rune
                idx = 50;
                if (gemColour >= '1' && gemColour <= '9')
                {
                    return idx + gemColour - '1';
                }
                break;

            case '1': // Thul Rune - Fal Rune
                idx = 59;
                if (gemColour >= '0' && gemColour <= '9')
                {
                    return idx + gemColour - '0';
                }
                break;

            case '2': // Lem Rune - Sur Rune
                idx = 69;
                if (gemColour >= '0' && gemColour <= '9')
                {
                    return idx + gemColour - '0';
                }
                break;

            case '3': // Ber Rune - Zod Rune
                idx = 79;
                if (gemColour >= '0' && gemColour <= '3')
                {
                    return idx + gemColour - '0';
                }
                break;
            }

            if (gemCondition == 'v') // rejuvenation potion
            {
                idx = 45;
                switch (gemColour)
                {
                case 's': // Regular
                    return idx;
                default: // Full
                    return idx + 1;
                }
            }
            break;

        case 'y': // antidote potion
            if (gemCondition == 'p' && gemColour == 's')
            {
                return 47;
            }
            break;

        case 'v': //stamina potion
            if (gemCondition == 'p' && gemColour == 's')
            {
                return 48;
            }
            break;

        case 'w': // thawing potion
            if (gemCondition == 'm' && gemColour == 's')
            {
                return 49;
            }
            break;
        }

        return size_t(-1); // unknown
    }
}

//---------------------------------------------------------------------------
// CD2GemsForm dialog
IMPLEMENT_DYNAMIC(CD2GemsForm, CDialogEx)

//---------------------------------------------------------------------------
CD2GemsForm::CD2GemsForm(CD2MainForm& form)
    : CDialogEx(CD2GemsForm::IDD, (CWnd*)&form), MainForm(form)
{

}
//---------------------------------------------------------------------------
CD2GemsForm::CD2GemsForm(CD2ItemsForm& form, d2ce::Item* itemPtr)
    : CDialogEx(CD2GemsForm::IDD, (CWnd*)&form), MainForm(form.MainForm), ItemsFormPtr(&form), ItemPtr(itemPtr)
{
}
//---------------------------------------------------------------------------
CD2GemsForm::~CD2GemsForm()
{
}
//---------------------------------------------------------------------------
void CD2GemsForm::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}
//---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CD2GemsForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2GemsForm::OnBnClickedConvert)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
BOOL CD2GemsForm::OnInitDialog()
{
    __super::OnInitDialog();

    bool potionsOnly = false;

    // Fill in to from combo
    CComboBox* pFromCombo = (CComboBox*)GetDlgItem(IDC_FROM_COMBO);
    auto it = NumGemMap.end();
    if (pFromCombo != nullptr)
    {
        std::uint64_t itemData = 0;
        std::array<std::uint8_t, 4> gemCode;
        *reinterpret_cast<std::uint32_t*>(gemCode.data()) = std::uint32_t(itemData);
        if (ItemPtr == nullptr)
        {
            auto& gems = MainForm.getGPSs();
            for (auto& item : gems)
            {
                gemCode.fill(0);
                if (!item.get().getItemCode(gemCode))
                {
                    continue;
                }

                it = NumGemMap.find(itemData);
                if (it == NumGemMap.end())
                {
                    GemIdxMap.emplace(GetGPSSortIndex(gemCode), itemData);
                    NumGemMap.emplace(itemData, 1);
                }
                else
                {
                    it->second = it->second + 1;
                }
            }
        }
        else
        {
            // Only add selected item
            gemCode.fill(0);
            if (ItemPtr->getItemCode(gemCode))
            {
                GemIdxMap.emplace(GetGPSSortIndex(gemCode), itemData);
                NumGemMap.emplace(itemData, 1);
            }

            switch (ItemPtr->getLocation())
            {
            case d2ce::EnumItemLocation::BELT:
                potionsOnly = true;
                break;
            }
        }

        for (auto& gem : GemIdxMap)
        {
            itemData = gem.second;
            pFromCombo->SetItemData(pFromCombo->AddString(GetGPSNameFromCode(gemCode)), itemData);
        }

        pFromCombo->SetCurSel(0);
    }

    // Fill in to combo
    CComboBox* pToCombo = (CComboBox*)GetDlgItem(IDC_TO_COMBO);
    if (pToCombo != nullptr)
    {
        std::uint64_t itemData = 0;
        std::array< std::uint8_t, 4> gemCode;
        *reinterpret_cast<std::uint32_t*>(gemCode.data()) = std::uint32_t(itemData);
        std::uint8_t& gem = gemCode[0];
        std::uint8_t& gemCondition = gemCode[1];
        std::uint8_t& gemColour = gemCode[2];
        gemCode[3] = 0x20;
        size_t potionIdx = 0;
        size_t runeIdx = 0;
        size_t numGems = MainForm.isExpansionCharacter() ? 83 : 50; // runes are only for expansion
        for (size_t gemIdx = 0; gemIdx < numGems; ++gemIdx)
        {
            itemData = 0;
            gemCode[3] = 0x20;
            if (gemIdx < 35)
            {
                if (potionsOnly)
                {
                    continue;
                }

                gem = 'g'; // gem
                switch (gemIdx % 7)
                {
                case 0:
                    gemColour = 'v'; // amethyst
                    break;
                case 1:
                    gemColour = 'w'; // diamond
                    break;
                case 2:
                    gemColour = 'g'; // emerald
                    break;
                case 3:
                    gemColour = 'r'; // ruby
                    break;
                default:
                    gemColour = 'b'; // sapphire
                    break;
                case 5:
                    gem = 's'; // skull
                    gemCondition = 'k';
                    switch (gemIdx / 7)
                    {
                    case 0:
                        gemColour = 'c'; // chipped
                        break;
                    case 1:
                        gemColour = 'f'; // flawed
                        break;
                    case 2:
                        gemColour = 'u'; // regular
                        break;
                    case 3:
                        gemColour = 'l'; // flawless
                        break;
                    default:
                        gemColour = 'z'; // perfect
                        break;
                    }
                    break;
                case 6:
                    gemColour = 'y'; // topaz
                    break;
                }

                if (gem == 'g')
                {
                    switch (gemIdx / 7)
                    {
                    case 0:
                        gemCondition = 'c'; // chipped
                        break;
                    case 1:
                        gemCondition = 'f'; // flawed
                        break;
                    case 2:
                        gemCondition = 'u'; // regular
                        break;
                    case 3:
                        if (gemColour == 'v')
                        {
                            gemCondition = 'z'; // flawless amethyst
                        }
                        else
                        {
                            gemCondition = 'l'; // flawless
                        }
                        break;
                    default:
                        gemCondition = 'p'; // perfect
                        break;
                    }
                }
            }
            else if (gemIdx < 50)
            {
                potionIdx = gemIdx - 35;
                if (potionIdx / 5 < 2)
                {
                    gem = potionIdx / 5 == 0 ? 'h' : 'm'; // healing or mana potion
                    gemCondition = 'p';
                    switch (potionIdx % 5)
                    {
                    case 0:
                        gemColour = '1'; // Minor
                        break;
                    case 1:
                        gemColour = '2'; // Light
                        break;
                    case 2:
                        gemColour = '3'; // regular
                        break;
                    case 3:
                        gemColour = '4'; // Greater
                        break;
                    default:
                        gemColour = '5'; // Super
                        break;
                    }
                }
                else if (potionIdx < 12)
                {
                    gem = 'r'; // rejuvenation potion
                    gemCondition = 'v';
                    gemColour = potionIdx % 5 == 0 ? 'l' : 's'; // regular or Full
                }
                else if (potionIdx == 12)
                {
                    gem = 'y'; // antidote potion
                    gemCondition = 'p';
                    gemColour = 's';
                }
                else if (potionIdx == 13)
                {
                    gem = 'v'; // stamina potion
                    gemCondition = 'p';
                    gemColour = 's';
                }
                else if (potionIdx == 14)
                {
                    gem = 'w'; // thawing potion
                    gemCondition = 'm';
                    gemColour = 's';
                }
                else
                {
                    continue;
                }
            }
            else // Runes
            {
                if (potionsOnly)
                {
                    continue;
                }

                runeIdx = gemIdx - 49; // first rune starts at 01 not 00
                gem = 'r';
                gemCondition = '0' + uint8_t(runeIdx / 10);
                gemColour = '0' + uint8_t(runeIdx % 10);
            }

            pToCombo->SetItemData(pToCombo->AddString(GetGPSNameFromCode(gemCode)), itemData);
        }

        pToCombo->SetCurSel(0);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
// CD2GemsForm message handlers
//---------------------------------------------------------------------------
void CD2GemsForm::OnBnClickedConvert()
{
    CComboBox* pFromCombo = (CComboBox*)GetDlgItem(IDC_FROM_COMBO);
    CComboBox* pToCombo = (CComboBox*)GetDlgItem(IDC_TO_COMBO);

    size_t numConverted = 0;
    if (pFromCombo != nullptr && pToCombo != nullptr)
    {
        // Make sure we are converting differnet kind of gems
        auto fromIdx = pFromCombo->GetCurSel();
        auto toIdx = pToCombo->GetCurSel();
        if (fromIdx >= 0 && toIdx >= 0 && pFromCombo->GetItemData(fromIdx) != pToCombo->GetItemData(toIdx))
        {
            std::uint64_t itemDataFrom = pFromCombo->GetItemData(fromIdx);
            std::array<std::uint8_t, 4> gemCodeFrom;
            *reinterpret_cast<std::uint32_t*>(gemCodeFrom.data()) = std::uint32_t(itemDataFrom);

            std::uint64_t itemDataTo = pToCombo->GetItemData(toIdx);
            std::array<std::uint8_t, 4> gemCodeTo;
            *reinterpret_cast<std::uint32_t*>(gemCodeTo.data()) = std::uint32_t(itemDataTo);

            if (ItemPtr == nullptr)
            {
                numConverted = MainForm.convertGPSs(gemCodeFrom, gemCodeTo);
            }
            else
            {
                if (ItemsFormPtr == nullptr)
                {
                    numConverted = MainForm.updateGem(*ItemPtr, gemCodeTo) ? 1 : 0;
                }
                else
                {
                    numConverted = ItemsFormPtr->updateGem(*ItemPtr, gemCodeTo) ? 1 : 0;
                }
            }

            if (numConverted > 0)
            {
                // Update entries in from combo box
                auto iter = NumGemMap.find(itemDataFrom);
                if (iter != NumGemMap.end())
                {
                    if (numConverted >= iter->second)
                    {
                        NumGemMap.erase(iter);
                        auto iterIdx = GemIdxMap.find(GetGPSSortIndex(gemCodeFrom));
                        if (iterIdx != GemIdxMap.end())
                        {
                            GemIdxMap.erase(iterIdx);
                        }
                    }
                    else
                    {
                        iter->second = iter->second - numConverted;
                    }
                }

                iter = NumGemMap.find(itemDataTo);
                if (iter == NumGemMap.end())
                {
                    GemIdxMap.emplace(GetGPSSortIndex(gemCodeTo), itemDataTo);
                    NumGemMap.emplace(itemDataTo, numConverted);
                }

                pFromCombo->ResetContent();
                for (auto& gem : GemIdxMap)
                {
                    itemDataTo = gem.second;
                    pFromCombo->SetItemData(pFromCombo->AddString(GetGPSNameFromCode(gemCodeTo)), itemDataTo);
                }

                pFromCombo->SetCurSel(0);
            }
        }
    }

    CString msg;
    if (ItemPtr == nullptr)
    {
        msg.Format(_T("%zd item(s) converted"), numConverted);
        AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
    }
    else if (numConverted == 0)
    {
        msg = _T("Item conversion failed!");
        AfxMessageBox(msg, MB_ICONEXCLAMATION | MB_OK);
    } 
}
//---------------------------------------------------------------------------

