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
#include "D2Editor.h"
#include "D2MagicalPropsRandomizer.h"
#include "d2ce/helpers/ItemHelpers.h"
#include "d2ce/item.h"
#include <utf8/utf8.h>
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------------------------------------------------------
namespace d2ce
{
    namespace ItemHelpers
    {
        std::uint32_t generarateRandomDW(std::uint32_t itemDwbCode = 0, std::uint16_t level = 0);
    }
}

//---------------------------------------------------------------------------
// CD2MagicalPropsRandomizer dialog

IMPLEMENT_DYNAMIC(CD2MagicalPropsRandomizer, CDialogEx)

//---------------------------------------------------------------------------
CD2MagicalPropsRandomizer::CD2MagicalPropsRandomizer(CD2ItemsForm& form)
    : CDialogEx(CD2MagicalPropsRandomizer::IDD, (CWnd*)&form), MainForm(form.MainForm), ItemsFormPtr(&form), ItemPtr(form.CurrItem)
{
    if (ItemPtr != nullptr)
    {
        CurrentItem = *ItemPtr;
        CurrentDWBCode = CurrentItem.getDWBCode();
        GeneratedDWBCode.push_back(CurrentDWBCode);
    }
}
//---------------------------------------------------------------------------
CD2MagicalPropsRandomizer::CD2MagicalPropsRandomizer(CD2SharedStashForm& form)
    : CDialogEx(CD2MagicalPropsRandomizer::IDD, (CWnd*)&form), MainForm(form.MainForm), SharedStashFormPtr(&form), ItemPtr(form.CurrItem)
{
    if (ItemPtr != nullptr)
    {
        CurrentItem = *ItemPtr;
        CurrentDWBCode = CurrentItem.getDWBCode();
        GeneratedDWBCode.push_back(CurrentDWBCode);
    }
}
//---------------------------------------------------------------------------
CD2MagicalPropsRandomizer::~CD2MagicalPropsRandomizer()
{
}
//---------------------------------------------------------------------------
void CD2MagicalPropsRandomizer::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TOOLTIP_RECT, ItemTooltipBox);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2MagicalPropsRandomizer, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2MagicalPropsRandomizer::OnBnClickedOk)
    ON_BN_CLICKED(IDC_GAMBLE_BUTTON, &CD2MagicalPropsRandomizer::OnBnClickedGambleButton)
    ON_BN_CLICKED(IDC_PREV_BUTTON, &CD2MagicalPropsRandomizer::OnBnClickedPrevButton)
    ON_BN_CLICKED(IDC_NEXT_BUTTON, &CD2MagicalPropsRandomizer::OnBnClickedNextButton)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2MagicalPropsRandomizer message handlers

//---------------------------------------------------------------------------
BOOL CD2MagicalPropsRandomizer::OnInitDialog()
{
    __super::OnInitDialog();

    if (ItemPtr == nullptr)
    {
        // should not happen
        EndDialog(IDCANCEL);
        return FALSE;
    }

    switch (ItemPtr->getQuality())
    {
    case d2ce::EnumItemQuality::INFERIOR:
    case d2ce::EnumItemQuality::NORMAL:
        // should not happen
        EndDialog(IDCANCEL);
        return FALSE;

    default:
        break;
    }

    {
        std::string strValue;
        std::u16string uText;
        CString text;
        CStringA textA;
        CWnd* pWnd = nullptr;
        if (d2ce::LocalizationHelpers::GetStringTxtValue("UICreate", strValue))
        {
            pWnd = GetDlgItem(IDOK);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("cancel", strValue))
        {
            pWnd = GetDlgItem(IDCANCEL);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("gamble", strValue))
        {
            pWnd = GetDlgItem(IDC_GAMBLE_BUTTON);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }
    }

    auto pWnd = GetDlgItem(IDOK);
    if (pWnd != nullptr)
    {
        // don't enable for the first index
        pWnd->EnableWindow(FALSE);
    }

    ItemTooltipBox.RedrawWindow();
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2MagicalPropsRandomizer::OnBnClickedOk()
{
    UpdateData(TRUE); // save results

    if (ItemPtr == nullptr)
    {
        // should not happen
        __super::OnOK();
        return;
    }

    MainForm.swapItems(*ItemPtr, CurrentItem);
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2MagicalPropsRandomizer::OnBnClickedGambleButton()
{
    CurrentDWBCode = d2ce::ItemHelpers::generarateRandomDW();
    CurrentDWBCodeIndex = GeneratedDWBCode.size();
    GeneratedDWBCode.push_back(CurrentDWBCode);
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2MagicalPropsRandomizer::OnBnClickedPrevButton()
{
    if (CurrentDWBCodeIndex > 0)
    {
        --CurrentDWBCodeIndex;
        if (CurrentDWBCodeIndex >= GeneratedDWBCode.size())
        {
            CurrentDWBCodeIndex = GeneratedDWBCode.size() - 1;
        }
        CurrentDWBCode = GeneratedDWBCode[CurrentDWBCodeIndex];
        UpdateCurrentAttribs();
    }
}
//---------------------------------------------------------------------------
void CD2MagicalPropsRandomizer::OnBnClickedNextButton()
{
    if (CurrentDWBCodeIndex < (GeneratedDWBCode.size() - 1))
    {
        ++CurrentDWBCodeIndex;
        if (CurrentDWBCodeIndex >= GeneratedDWBCode.size())
        {
            CurrentDWBCodeIndex = GeneratedDWBCode.size() - 1;
        }
        CurrentDWBCode = GeneratedDWBCode[CurrentDWBCodeIndex];
        UpdateCurrentAttribs();
    }
}
//---------------------------------------------------------------------------
void CD2MagicalPropsRandomizer::UpdateCurrentAttribs()
{
    // Rest attributes for item
    CurrentItem.setDWBCode(CurrentDWBCode);
    CurrentItem.fixDurability();

    auto pWnd = GetDlgItem(IDOK);
    if (pWnd != nullptr)
    {
        // don't enable for the first index
        pWnd->EnableWindow((CurrentDWBCodeIndex > 0) ? TRUE : FALSE);
    }

    pWnd = GetDlgItem(IDC_PREV_BUTTON);
    if (pWnd != nullptr)
    {
        pWnd->EnableWindow((CurrentDWBCodeIndex > 0) ? TRUE : FALSE);
    }

    pWnd = GetDlgItem(IDC_NEXT_BUTTON);
    if (pWnd != nullptr)
    {
        pWnd->EnableWindow((CurrentDWBCodeIndex < (GeneratedDWBCode.size() - 1)) ? TRUE : FALSE);
    }

    ItemTooltipBox.RedrawWindow();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2MagicalPropsRandomizer::GetSelectedItem() const
{
    return &CurrentItem;
}
//---------------------------------------------------------------------------
const d2ce::Character* CD2MagicalPropsRandomizer::GetCharacterInfo() const
{
    return &MainForm.getCharacterInfo();
}
//---------------------------------------------------------------------------
