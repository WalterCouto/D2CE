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

namespace
{
    void AddListColData(CD2MultiLineListCtrl& ctrl, std::uint32_t row, std::uint32_t col, const std::string& u8string)
    {
        std::u16string uText = utf8::utf8to16(u8string);
        CString str(reinterpret_cast<LPCWSTR>(uText.c_str()));
        LVITEM lv;
        lv.iItem = row;
        lv.iSubItem = col;
        lv.pszText = (LPTSTR)(LPCTSTR)str;
        lv.mask = LVIF_TEXT;
        if (col == 0)
        {
            ctrl.InsertItem(&lv);
        }
        else
        {
            ctrl.SetItem(&lv);
        }
    }
}

//---------------------------------------------------------------------------
// CD2RunewordForm dialog

IMPLEMENT_DYNAMIC(CD2RunewordForm, CDialogEx)

//---------------------------------------------------------------------------
CD2RunewordForm::CD2RunewordForm(CD2ItemsForm& form)
    : CDialogEx(CD2RunewordForm::IDD, (CWnd*)&form), MainForm(form.MainForm), ItemsFormPtr(&form), ItemPtr(form.CurrItem)
{
}
//---------------------------------------------------------------------------
CD2RunewordForm::CD2RunewordForm(CD2SharedStashForm& form)
    : CDialogEx(CD2RunewordForm::IDD, (CWnd*)&form), MainForm(form.MainForm), SharedStashFormPtr(&form), ItemPtr(form.CurrItem)
{
}
//---------------------------------------------------------------------------
CD2RunewordForm::~CD2RunewordForm()
{
}
//---------------------------------------------------------------------------
void CD2RunewordForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RUNEWORD_LIST, RunewordGrid);
}
//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2RunewordForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2RunewordForm::OnBnClickedOk)
END_MESSAGE_MAP()

// CD2RunewordForm message handlers

//---------------------------------------------------------------------------
BOOL CD2RunewordForm::OnInitDialog()
{
    __super::OnInitDialog();

    RunewordGrid.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
    RunewordGrid.InsertColumn(0, _T("Runeword"), LVCFMT_RIGHT, LVSCW_AUTOSIZE_USEHEADER);
    RunewordGrid.InsertColumn(1, _T("Runes"), LVCFMT_RIGHT, LVSCW_AUTOSIZE_USEHEADER);
    RunewordGrid.InsertColumn(2, _T("Attributes"), LVCFMT_CENTER, LVSCW_AUTOSIZE_USEHEADER);

    FillCells();
    return 0;
}
//---------------------------------------------------------------------------
void CD2RunewordForm::OnBnClickedOk()
{
    if (ItemPtr != nullptr)
    {
        auto posSelection = RunewordGrid.GetFirstSelectedItemPosition();
        if (posSelection)
        {
            auto nCurrentSelection = RunewordGrid.GetNextSelectedItem(posSelection);
            auto runwordId = std::uint16_t(RunewordGrid.GetItemData(nCurrentSelection));
            if (SharedStashFormPtr != nullptr)
            {
                SharedStashFormPtr->setItemRuneword(*ItemPtr, runwordId);
            }
            else if (ItemsFormPtr != nullptr)
            {
                ItemsFormPtr->setItemRuneword(*ItemPtr, runwordId);
            }
        }
    }
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2RunewordForm::FillCells()
{
    if (ItemPtr == nullptr)
    {
        return;
    }

    auto charLevel = MainForm.getCharacterLevel();

    TCHAR name[255];
    LVCOLUMN col;
    col.mask = LVCF_TEXT;
    col.pszText = name;
    col.cchTextMax = sizeof(name) / sizeof(TCHAR);

    std::u16string uText;
    std::uint32_t row = 0;
    for (auto& runeword : ItemPtr->getPossibleRunewords())
    {
        row = RunewordGrid.GetItemCount();
        AddListColData(RunewordGrid, row, 0, runeword.name);

        {
            std::stringstream ss;
            bool bFirstItem = true;
            std::string quoteStr;
            d2ce::LocalizationHelpers::GetStringTxtValue("RuneQuote", quoteStr, "'");
            for (const auto& runeCode : runeword.runeCodes)
            {
                const auto& item = d2ce::ItemHelpers::getItemTypeHelper(runeCode);
                if (!item.isRune())
                {
                    continue;
                }

                if (bFirstItem)
                {
                    ss << quoteStr;
                    bFirstItem = false;
                }

                ss << item.getRuneLetter();
            }

            if (!bFirstItem)
            {
                ss << quoteStr;
            }

            AddListColData(RunewordGrid, row, 1, ss.str());
        }

        std::vector<d2ce::MagicalAttribute> attribs;
        for (const auto& runeCode : runeword.runeCodes)
        {
            const auto& runeItemType = d2ce::ItemHelpers::getItemTypeHelper(runeCode);
            if (runeItemType.getRuneMagicalAttributes(*ItemPtr, attribs))
            {
                runeword.attribs.reserve(std::max(runeword.attribs.capacity(), runeword.attribs.size() + attribs.size()));
                for (auto& value : attribs)
                {
                    runeword.attribs.emplace_back(std::move(value));
                }
            }
        }

        d2ce::ItemHelpers::formatMagicalAttributes(runeword.attribs, charLevel);
        {
            std::stringstream ss;
            bool bFirstItem = true;
            for (const auto& attrib : runeword.attribs)
            {
                if (!attrib.Visible)
                {
                    continue;
                }

                if (!bFirstItem)
                {
                    ss << "\n";
                }
                bFirstItem = false;

                ss << attrib.Desc;
            }

            AddListColData(RunewordGrid, row, 2, ss.str());
        }

        RunewordGrid.SetItemData(row, runeword.id);
    }

    if (RunewordGrid.GetItemCount() > 0)
    {
        RunewordGrid.AdjustColumnWidths();
        RunewordGrid.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    }
}
