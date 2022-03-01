/*
    Diablo II Character Editor
    Copyright (C) 2022 Walter Couto

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
#include "D2AddGemsForm.h"
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
}

//---------------------------------------------------------------------------
// CD2AddGemsForm dialog
IMPLEMENT_DYNAMIC(CD2AddGemsForm, CDialogEx)

//---------------------------------------------------------------------------
CD2AddGemsForm::CD2AddGemsForm(CD2MainForm& form)
    : CDialogEx(CD2AddGemsForm::IDD, (CWnd*)&form), MainForm(form)
{

}
//---------------------------------------------------------------------------
CD2AddGemsForm::CD2AddGemsForm(CD2ItemsForm& form)
    : CDialogEx(CD2AddGemsForm::IDD, (CWnd*)&form), MainForm(form.MainForm), ItemsFormPtr(&form), ItemPtr(form.CurrItem)
{
}
//---------------------------------------------------------------------------
CD2AddGemsForm::CD2AddGemsForm(CD2SharedStashForm& form)
    : CDialogEx(CD2AddGemsForm::IDD, (CWnd*)&form), MainForm(form.MainForm), SharedStashFormPtr(&form), ItemPtr(form.CurrItem)
{
}
//---------------------------------------------------------------------------
CD2AddGemsForm::~CD2AddGemsForm()
{
}
//---------------------------------------------------------------------------
void CD2AddGemsForm::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}
//---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CD2AddGemsForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2AddGemsForm::OnBnClickedAdd)
    ON_BN_CLICKED(IDC_FILL_BUTTON, &CD2AddGemsForm::OnBnClickedFill)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
BOOL CD2AddGemsForm::OnInitDialog()
{
    __super::OnInitDialog();

    // Fill in to from combo
    bool isPotionSelected = false;
    CComboBox* pFromCombo = (CComboBox*)GetDlgItem(IDC_FROM_COMBO);
    if (pFromCombo != nullptr)
    {
        std::uint32_t selectedItemData = 0;
        if (ItemPtr != nullptr)
        {
            std::array<std::uint8_t, 4> selectedGemCode;
            if (ItemPtr->getItemCode(selectedGemCode))
            {
                selectedItemData = *reinterpret_cast<std::uint32_t*>(selectedGemCode.data());
                isPotionSelected = ItemPtr->isPotion();
            }
            
        }
        int selectedIdx = 0;

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

            auto insertedIdx = pFromCombo->AddString(GetGPSNameFromCode(gemCode));
            pFromCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
            if (itemData == selectedItemData)
            {
                selectedIdx = insertedIdx;
            }
        }

        pFromCombo->SetCurSel(selectedIdx);
    }

    // Fill in to combo
    CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);
    if (pLocationCombo != nullptr)
    {
        if (SharedStashFormPtr != nullptr)
        {
            CString str;
            str.Format(_T("Shared Stash %ld"), SharedStashFormPtr->getCurrentPage() + 1);
            auto insertedIdx = pLocationCombo->AddString(str);
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(SharedStashFormPtr->getCurrentPage()));
            pLocationCombo->SetCurSel(0);
        }
        else
        {
            std::array< std::uint8_t, 2> locationCode = { 0x00, 0x00 };
            std::uint16_t& itemData = *reinterpret_cast<std::uint16_t*>(locationCode.data());
            std::uint8_t& locationID = locationCode[0];
            std::uint8_t& altLocationId = locationCode[1];

            std::uint16_t selectedItemData = 0;
            if (ItemsFormPtr != nullptr)
            {
                selectedItemData = *reinterpret_cast<std::uint16_t*>(ItemsFormPtr->CurrItemLocation.data());
            }
            int selectedIdx = isPotionSelected ? 0 : 1;

            locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::BELT);
            altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::UNKNOWN);
            auto insertedIdx = pLocationCombo->AddString(_T("Belt"));
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
// CD2AddGemsForm message handlers
//---------------------------------------------------------------------------
void CD2AddGemsForm::OnBnClickedAdd()
{
    CComboBox* pFromCombo = (CComboBox*)GetDlgItem(IDC_FROM_COMBO);
    CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);
    if (pFromCombo == nullptr || pLocationCombo == nullptr)
    {
        return;
    }

    auto fromIdx = pFromCombo->GetCurSel();
    std::uint64_t itemDataFrom = pFromCombo->GetItemData(fromIdx);
    std::array<std::uint8_t, 4> gemCode;
    *reinterpret_cast<std::uint32_t*>(gemCode.data()) = std::uint32_t(itemDataFrom);

    auto toIdx = pLocationCombo->GetCurSel();
    if (SharedStashFormPtr != nullptr)
    {
        auto stashPage = size_t(pLocationCombo->GetItemData(toIdx));
        if (!SharedStashFormPtr->addItem(gemCode, stashPage))
        {
            CString msg(_T("A "));
            CString temp;
            pFromCombo->GetLBText(fromIdx, temp);
            msg += temp;
            msg += _T(" was added to ");
            pLocationCombo->GetLBText(toIdx, temp);
            msg += temp;
            AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
        }
    }
    else
    {
        std::array< std::uint8_t, 2> locationCode = { 0x00, 0x00 };
        std::uint8_t& locationID = locationCode[0];
        std::uint8_t& altLocationId = locationCode[1];
        std::uint64_t itemDataTo = pLocationCombo->GetItemData(toIdx);
        *reinterpret_cast<std::uint16_t*>(locationCode.data()) = std::uint16_t(itemDataTo);

        if (ItemsFormPtr != nullptr)
        {
            if (!ItemsFormPtr->addItem(static_cast<d2ce::EnumItemLocation>(locationID), static_cast<d2ce::EnumAltItemLocation>(altLocationId), gemCode))
            {
                CString msg(_T("A "));
                CString temp;
                pFromCombo->GetLBText(fromIdx, temp);
                msg += temp;
                msg += _T(" was added to ");
                pLocationCombo->GetLBText(toIdx, temp);
                msg += temp;
                AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
            }
        }
        else if (!MainForm.addItem(static_cast<d2ce::EnumItemLocation>(locationID), static_cast<d2ce::EnumAltItemLocation>(altLocationId), gemCode))
        {
            CString msg(_T("A "));
            CString temp;
            pFromCombo->GetLBText(fromIdx, temp);
            msg += temp;
            msg += _T(" could not be added to ");
            pLocationCombo->GetLBText(toIdx, temp);
            msg += temp;
            AfxMessageBox(msg, MB_ICONEXCLAMATION | MB_OK);
        }
        else
        {
            CString msg(_T("A "));
            CString temp;
            pFromCombo->GetLBText(fromIdx, temp);
            msg += temp;
            msg += _T(" was added to ");
            pLocationCombo->GetLBText(toIdx, temp);
            msg += temp;
            AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
        }
    }
}
//---------------------------------------------------------------------------
void CD2AddGemsForm::OnBnClickedFill()
{
    CComboBox* pFromCombo = (CComboBox*)GetDlgItem(IDC_FROM_COMBO);
    CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);
    if (pFromCombo == nullptr || pLocationCombo == nullptr)
    {
        return;
    }

    auto fromIdx = pFromCombo->GetCurSel();
    std::uint64_t itemDataFrom = pFromCombo->GetItemData(fromIdx);
    std::array<std::uint8_t, 4> gemCode;
    *reinterpret_cast<std::uint32_t*>(gemCode.data()) = std::uint32_t(itemDataFrom);

    auto toIdx = pLocationCombo->GetCurSel();
    std::array< std::uint8_t, 2> locationCode = { 0x00, 0x00 };
    std::uint8_t& locationID = locationCode[0];
    std::uint8_t& altLocationId = locationCode[1];
    std::uint64_t itemDataTo = pLocationCombo->GetItemData(toIdx);
    *reinterpret_cast<std::uint16_t*>(locationCode.data()) = std::uint16_t(itemDataTo);

    size_t numAdded = 0;
    if (ItemsFormPtr != nullptr)
    {
        numAdded = ItemsFormPtr->fillEmptySlots(static_cast<d2ce::EnumItemLocation>(locationID), static_cast<d2ce::EnumAltItemLocation>(altLocationId), gemCode);
    }
    else
    {
        numAdded = MainForm.fillEmptySlots(static_cast<d2ce::EnumItemLocation>(locationID), static_cast<d2ce::EnumAltItemLocation>(altLocationId), gemCode);
    }

    CString msg(_T("%zd item(s) of type "));
    CString temp;
    pFromCombo->GetLBText(fromIdx, temp);
    msg += temp;
    msg += _T(" were added to ");
    pLocationCombo->GetLBText(toIdx, temp);
    msg += temp;
    temp.Format(msg, numAdded);
    AfxMessageBox(temp, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------

