/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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
#include "D2Editor.h"
#include "D2GemsForm.h"
#include "afxdialogex.h"
#include "d2ce\helpers\ItemHelpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    CString GetGPSNameFromCode(const std::array<std::uint8_t, 4>& gemCode)
    {
        const auto& itemType = d2ce::ItemHelpers::getItemTypeHelper(gemCode);
        if (!itemType.isPotion() && !itemType.isGem() && !itemType.isRune())
        {
            // unknown GPS
            return _T("");
        }

        return CString(itemType.name.c_str());
    }

    size_t GetGPSSortIndex(const std::array<std::uint8_t, 4>& gemCode)
    {
        return d2ce::ItemHelpers::getGPSSortIndex(gemCode);
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

    if (ItemPtr != nullptr)
    {
        SetWindowText(_T("Single Item Converter"));
    }

    bool potionsOnly = false;

    // Fill in to from combo
    CComboBox* pFromCombo = (CComboBox*)GetDlgItem(IDC_FROM_COMBO);
    auto it = NumGemMap.end();
    if (pFromCombo != nullptr)
    {
        std::array<std::uint8_t, 4> gemCode;
        std::uint32_t& itemData = *reinterpret_cast<std::uint32_t*>(gemCode.data());
        itemData = 0;
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

                it = NumGemMap.find(std::uint64_t(itemData));
                if (it == NumGemMap.end())
                {
                    GemIdxMap.emplace(GetGPSSortIndex(gemCode), std::uint64_t(itemData));
                    NumGemMap.emplace(std::uint64_t(itemData), 1);
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
                GemIdxMap.emplace(GetGPSSortIndex(gemCode), std::uint64_t(itemData));
                NumGemMap.emplace(std::uint64_t(itemData), 1);
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
            *reinterpret_cast<std::uint32_t*>(gemCode.data()) = std::uint32_t(gem.second);
            pFromCombo->SetItemData(pFromCombo->AddString(GetGPSNameFromCode(gemCode)), gem.second);
        }

        pFromCombo->SetCurSel(0);
    }

    // Fill in to combo
    CComboBox* pToCombo = (CComboBox*)GetDlgItem(IDC_TO_COMBO);
    if (pToCombo != nullptr)
    {
        std::array< std::uint8_t, 4> gemCode = { 0x20, 0x20, 0x20, 0x20 };
        std::uint32_t& itemData = *reinterpret_cast<std::uint32_t*>(gemCode.data());
        std::vector<std::string> gpsCodes;
        d2ce::ItemHelpers::getValidGPSCodes(gpsCodes, MainForm.isExpansionCharacter());
        for (const auto& code : gpsCodes)
        {
            if (code.length() < 3)
            {
                continue;
            }

            gemCode[0] = code[0];
            gemCode[1] = code[1];
            gemCode[2] = code[2];
            pToCombo->SetItemData(pToCombo->AddString(GetGPSNameFromCode(gemCode)), std::uint64_t(itemData));
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
                if (ItemsFormPtr == nullptr)
                {
                    numConverted = MainForm.convertGPSs(gemCodeFrom, gemCodeTo);
                }
                else
                {
                    numConverted = ItemsFormPtr->convertGPSs(gemCodeFrom, gemCodeTo);
                }
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

