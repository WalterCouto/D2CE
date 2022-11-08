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
#include "D2RunewordForm.h"
#include "afxdialogex.h"
#include "d2ce/helpers/ItemHelpers.h"
#include <utf8/utf8.h>

//---------------------------------------------------------------------------
// CD2RunewordForm dialog

IMPLEMENT_DYNAMIC(CD2RunewordForm, CDialogEx)

//---------------------------------------------------------------------------
CD2RunewordForm::CD2RunewordForm(CD2ItemsForm& form)
    : CDialogEx(CD2RunewordForm::IDD, (CWnd*)&form), MainForm(form.MainForm), ItemsFormPtr(&form), ItemPtr(form.CurrItem)
{
    if (ItemPtr != nullptr)
    {
        CurrentItem = *ItemPtr;
    }
}
//---------------------------------------------------------------------------
CD2RunewordForm::CD2RunewordForm(CD2MercenaryForm& form)
    : CDialogEx(CD2RunewordForm::IDD, (CWnd*)&form), MainForm(form.MainForm), MercenaryFormPtr(&form), ItemPtr(form.CurrItem)
{
    if (ItemPtr != nullptr)
    {
        CurrentItem = *ItemPtr;
    }
}
//---------------------------------------------------------------------------
CD2RunewordForm::CD2RunewordForm(CD2SharedStashForm& form)
    : CDialogEx(CD2RunewordForm::IDD, (CWnd*)&form), MainForm(form.MainForm), SharedStashFormPtr(&form), ItemPtr(form.CurrItem)
{
    if (ItemPtr != nullptr)
    {
        CurrentItem = *ItemPtr;
    }
}
//---------------------------------------------------------------------------
CD2RunewordForm::CD2RunewordForm(CD2NewItemForm& form)
    : CDialogEx(CD2RunewordForm::IDD, (CWnd*)&form), MainForm(form.MainForm), NewItemFormPtr(&form), ItemPtr(const_cast<d2ce::Item*>(form.CreatedItem))
{
    if (ItemPtr != nullptr)
    {
        CurrentItem = *ItemPtr;
    }
}
//---------------------------------------------------------------------------
CD2RunewordForm::~CD2RunewordForm()
{
}
//---------------------------------------------------------------------------
void CD2RunewordForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RUNEWORD_STATIC, RunewordStatic);
    DDX_Control(pDX, IDC_RUNEWORD_LIST, Runeword);
    DDX_Control(pDX, IDC_TOOLTIP_RECT, ItemTooltipBox);
}
//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2RunewordForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2RunewordForm::OnBnClickedOk)
    ON_CBN_SELCHANGE(IDC_PREFIX1_COMBO, &CD2RunewordForm::OnCbnSelchangeRunewordCombo)
END_MESSAGE_MAP()

// CD2RunewordForm message handlers

//---------------------------------------------------------------------------
BOOL CD2RunewordForm::OnInitDialog()
{
    __super::OnInitDialog();

    {
        std::string strValue;
        std::u16string uText;
        CString text;
        CStringA textA;
        CWnd* pWnd = nullptr;
        if (d2ce::LocalizationHelpers::GetStringTxtValue("ok", strValue))
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
    }

    InitRunewords();
    return TRUE;  // return TRUE unless you set the focus to a control
}
//---------------------------------------------------------------------------
void CD2RunewordForm::OnBnClickedOk()
{
    if (ItemPtr != nullptr)
    {
        auto runewordId = std::uint16_t(Runeword.GetItemData(Runeword.GetCurSel()));
        if (SharedStashFormPtr != nullptr)
        {
            SharedStashFormPtr->setItemRuneword(*ItemPtr, runewordId);
        }
        else if (ItemsFormPtr != nullptr)
        {
            ItemsFormPtr->setItemRuneword(*ItemPtr, runewordId);
        }
        else if (MercenaryFormPtr != nullptr)
        {
            MercenaryFormPtr->setItemRuneword(*ItemPtr, runewordId);
        }
        else
        {
            ItemPtr->setRuneword(runewordId);
        }
    }
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2RunewordForm::OnCbnSelchangeRunewordCombo()
{
    UpdateData(TRUE); // save results
    if (ItemPtr != nullptr)
    {
        CurrentItem = *ItemPtr;
    }
    CurrentItem.setRuneword(std::uint16_t(Runeword.GetItemData(Runeword.GetCurSel())));
    ItemTooltipBox.RedrawWindow();
}
//---------------------------------------------------------------------------
void CD2RunewordForm::InitRunewords()
{
    if (ItemPtr == nullptr)
    {
        EndDialog(IDCANCEL);
        return;
    }

    std::u16string uText;
    CString strText;
    int idx = -1;
    int runewordIdx = 0;
    d2ce::RunewordAttributes runeAttrib;
    auto bHasRuneword = CurrentItem.getRunewordAttributes(runeAttrib);
    for (auto& runeword : CurrentItem.getPossibleRunewords())
    {
        uText = utf8::utf8to16(runeword.name);
        strText = reinterpret_cast<LPCWSTR>(uText.c_str());
        idx = Runeword.AddString(strText);
        if (idx >= 0)
        {
            if (bHasRuneword && runeAttrib.Id == runeword.id)
            {
                runewordIdx = idx;
            }
            Runeword.SetItemData(idx, runeword.id);
        }
    }

    if (Runeword.GetCount() == 0)
    {
        EndDialog(IDCANCEL);
        return;
    }

    Runeword.SetCurSel(runewordIdx);
    CurrentItem.setRuneword(std::uint16_t(Runeword.GetItemData(Runeword.GetCurSel())));
    ItemTooltipBox.RedrawWindow();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2RunewordForm::GetSelectedItem() const
{
    return &CurrentItem;
}
//---------------------------------------------------------------------------
const d2ce::Character* CD2RunewordForm::GetCharacterInfo() const
{
    return &MainForm.getCharacterInfo();
}
//---------------------------------------------------------------------------
