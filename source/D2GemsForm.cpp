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
#include "d2ce/helpers/ItemHelpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    CString GetGPSNameFromCode(const std::array<std::uint8_t, 4>& gemCode)
    {
        const auto& itemType = d2ce::ItemHelpers::getItemTypeHelper(gemCode);
        if (!itemType.isGPSItem())
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
CD2GemsForm::CD2GemsForm(CD2ItemsForm& form)
    : CDialogEx(CD2GemsForm::IDD, (CWnd*)&form), MainForm(form.MainForm), ItemsFormPtr(&form), ItemPtr(form.CurrItem)
{
}
CD2GemsForm::CD2GemsForm(CD2SharedStashForm& form)
    : CDialogEx(CD2GemsForm::IDD, (CWnd*)&form), MainForm(form.MainForm), SharedStashFormPtr(&form), ItemPtr(form.CurrItem)
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

    if (ItemPtr != nullptr)
    {
        auto pWnd = GetDlgItem(IDC_LOCATION_COMBO);
        if (pWnd != nullptr)
        {
            pWnd->EnableWindow(FALSE);
            pWnd->ShowWindow(SW_HIDE);
        }

        pWnd = GetDlgItem(IDC_LOCATION_STATIC);
        if (pWnd != nullptr)
        {
            pWnd->EnableWindow(FALSE);
            pWnd->ShowWindow(SW_HIDE);
        }
    }
    else if (SharedStashFormPtr != nullptr)
    {
        CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);
        if (pLocationCombo != nullptr)
        {
            CString str;
            str.Format(_T("Shared Stash %ld"), SharedStashFormPtr->getCurrentPage() + 1);
            auto insertedIdx = pLocationCombo->AddString(str);
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(SharedStashFormPtr->getCurrentPage()));
            pLocationCombo->SetCurSel(0);
        }
    }
    else
    {
        CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);
        if (pLocationCombo != nullptr)
        {
            std::uint16_t selectedItemData = 0;
            if (ItemsFormPtr != nullptr)
            {
                selectedItemData = *reinterpret_cast<std::uint16_t*>(ItemsFormPtr->CurrItemLocation.data());
            }

            std::array< std::uint8_t, 2> locationCode = { 0x00, 0x00 };
            std::uint16_t& itemData = *reinterpret_cast<std::uint16_t*>(locationCode.data());
            std::uint8_t& locationID = locationCode[0];
            std::uint8_t& altLocationId = locationCode[1];

            int selectedIdx = 0;
            auto insertedIdx = pLocationCombo->AddString(_T("All Locations"));
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
            if (itemData == selectedItemData)
            {
                selectedIdx = insertedIdx;
            }
            
            locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::BELT);
            altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::UNKNOWN);
            insertedIdx = pLocationCombo->AddString(_T("Belt"));
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
            if (itemData == selectedItemData)
            {
                selectedIdx = insertedIdx;
            }

            if (MainForm.getHasHoradricCube())
            {
                locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
                altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::HORADRIC_CUBE);
                insertedIdx = pLocationCombo->AddString(_T("Horadric Cube"));
                pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
                if (itemData == selectedItemData)
                {
                    selectedIdx = insertedIdx;
                }
            }

            locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
            altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::INVENTORY);
            insertedIdx = pLocationCombo->AddString(_T("Inventory"));
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
            if (itemData == selectedItemData)
            {
                selectedIdx = insertedIdx;
            }

            locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
            altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::STASH);
            insertedIdx = pLocationCombo->AddString(_T("Stash"));
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
            if (itemData == selectedItemData)
            {
                selectedIdx = insertedIdx;
            }

            pLocationCombo->SetCurSel(selectedIdx);
        }
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
    CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);

    d2ce::EnumItemLocation locationId = d2ce::EnumItemLocation::BUFFER;
    d2ce::EnumAltItemLocation altLocationId = d2ce::EnumAltItemLocation::UNKNOWN;
    bool bFiltered = false;
    size_t stashPage = 0;
    if (pLocationCombo != nullptr && pLocationCombo->IsWindowEnabled())
    {
        auto locationIdx = pLocationCombo->GetCurSel();
        if (locationIdx >= 0)
        {
            if (SharedStashFormPtr != nullptr)
            {
                bFiltered = true;
                stashPage = size_t(pLocationCombo->GetItemData(locationIdx));
            }
            else
            {
                std::array< std::uint8_t, 2> locationCode = { 0x00, 0x00 };
                std::uint16_t& itemDataLocation = *reinterpret_cast<std::uint16_t*>(locationCode.data());
                itemDataLocation = std::uint16_t(pLocationCombo->GetItemData(locationIdx));
                locationId = static_cast<d2ce::EnumItemLocation>(locationCode[0]);
                switch (locationId)
                {
                case d2ce::EnumItemLocation::STORED:
                    altLocationId = static_cast<d2ce::EnumAltItemLocation>(locationCode[1]);
                    switch (altLocationId)
                    {
                    case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
                    case d2ce::EnumAltItemLocation::INVENTORY:
                    case d2ce::EnumAltItemLocation::STASH:
                        bFiltered = true;
                        break;
                    }
                    break;

                case d2ce::EnumItemLocation::BELT:
                    bFiltered = true;
                    break;
                }
            }
        }

        if (!bFiltered)
        {
            locationId = d2ce::EnumItemLocation::BUFFER;
            altLocationId = d2ce::EnumAltItemLocation::UNKNOWN;
            stashPage = SharedStashFormPtr->getCurrentPage();
        }
    }

    size_t numConverted = 0;
    if (pFromCombo != nullptr && pToCombo != nullptr)
    {
        // Make sure we are converting differnet kind of gems
        auto fromIdx = pFromCombo->GetCurSel();
        auto toIdx = pToCombo->GetCurSel();
        if (fromIdx >= 0 && toIdx >= 0 && pFromCombo->GetItemData(fromIdx) != pToCombo->GetItemData(toIdx))
        {
            std::array<std::uint8_t, 4> gemCodeFrom;
            std::uint32_t& itemDataFrom = *reinterpret_cast<std::uint32_t*>(gemCodeFrom.data());
            itemDataFrom = std::uint32_t(pFromCombo->GetItemData(fromIdx));

            std::array<std::uint8_t, 4> gemCodeTo;
            std::uint32_t& itemDataTo = *reinterpret_cast<std::uint32_t*>(gemCodeTo.data());
            itemDataTo = std::uint32_t(pToCombo->GetItemData(toIdx));

            if (ItemPtr == nullptr)
            {
                d2ce::ItemFilter filter;
                filter.LocationId = locationId;
                filter.AltPositionId = altLocationId;
                if (SharedStashFormPtr != nullptr)
                {
                    numConverted = SharedStashFormPtr->convertGPSs(gemCodeFrom, gemCodeTo, stashPage);
                }
                else if (ItemsFormPtr != nullptr)
                {
                    numConverted = ItemsFormPtr->convertGPSs(gemCodeFrom, gemCodeTo, filter);
                }
                else
                {
                    numConverted = MainForm.convertGPSs(gemCodeFrom, gemCodeTo, filter);
                }
            }
            else
            {
                if (SharedStashFormPtr != nullptr)
                {
                    numConverted = SharedStashFormPtr->updateGem(*ItemPtr, gemCodeTo) ? 1 : 0;
                }
                else if (ItemsFormPtr != nullptr)
                {
                    numConverted = ItemsFormPtr->updateGem(*ItemPtr, gemCodeTo) ? 1 : 0;
                }
                else
                {
                    numConverted = MainForm.updateGem(*ItemPtr, gemCodeTo) ? 1 : 0;
                }
            }

            if (numConverted > 0)
            {
                // Update entries in from combo box
                bool bFromListModified = false;
                auto iter = NumGemMap.find(itemDataFrom);
                if (iter != NumGemMap.end())
                {
                    if (numConverted >= iter->second)
                    {
                        bFromListModified = true;
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
                    bFromListModified = true;
                    GemIdxMap.emplace(GetGPSSortIndex(gemCodeTo), itemDataTo);
                    NumGemMap.emplace(itemDataTo, numConverted);
                }
                else
                {
                    iter->second = iter->second + numConverted;
                }

                if (bFromListModified)
                {
                    pFromCombo->ResetContent();
                    for (auto& gem : GemIdxMap)
                    {
                        itemDataTo = std::uint32_t(gem.second);
                        pFromCombo->SetItemData(pFromCombo->AddString(GetGPSNameFromCode(gemCodeTo)), itemDataTo);
                    }

                    pFromCombo->SetCurSel(0);
                }
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

