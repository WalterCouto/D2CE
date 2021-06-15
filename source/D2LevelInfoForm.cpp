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
#include "D2LevelInfoForm.h"
#include "afxdialogex.h"
#include "D2MainForm.h"
#include "d2ce\ExperienceConstants.h"

namespace
{
    template <class charT>
    class num_printer
        : public std::numpunct<charT>
    {
    public:
        num_printer(charT thousands, charT decimals) : thousands(thousands), decimals(decimals) {}
    protected:
        charT do_decimal_point() const { return decimals; }
        charT do_thousands_sep() const { return thousands; }
        std::string do_grouping() const { return "\3"; }
    private:
        charT thousands, decimals;
    };

    void AddListColData(CListCtrl& ctrl, std::uint32_t row, std::uint32_t col, std::uint32_t value)
    {
        CString str;
#ifdef _UNICODE
        std::locale local(std::locale(""), new num_printer<wchar_t>(',', '.'));
        std::wstringstream ss;
        ss.imbue(local);
        ss << value;
#else
        std::locale local(std::locale(""), new num_printer<char>(',', '.'));
        std::stringstream ss;
        ss.imbue(local);
        ss << value;
#endif
        str = ss.str().c_str();

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
// CD2LevelInfoForm dialog

IMPLEMENT_DYNAMIC(CD2LevelInfoForm, CDialogEx)

//---------------------------------------------------------------------------
CD2LevelInfoForm::CD2LevelInfoForm(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_D2LEVELINFO_DIALOG, pParent)
{
    Modal = FALSE;
    Version = d2ce::EnumCharVersion::v110;
    if (pParent != nullptr && pParent->IsKindOf(RUNTIME_CLASS(CD2MainForm)))
    {
        auto& CharInfo = ((CD2MainForm*)pParent)->CharInfo;
        if (CharInfo != nullptr)
        {
            Version = CharInfo->getVersion();
        }
    }
}
//---------------------------------------------------------------------------
CD2LevelInfoForm::~CD2LevelInfoForm()
{
}
//---------------------------------------------------------------------------
void CD2LevelInfoForm::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LEVELINFO_GRID, LevelInfoGrid);
}
//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2LevelInfoForm, CDialogEx)
END_MESSAGE_MAP()

// CD2LevelInfoForm message handlers

//---------------------------------------------------------------------------
BOOL CD2LevelInfoForm::OnInitDialog()
{
    __super::OnInitDialog();

    // setup the paths selection list
    LevelInfoGrid.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
    LevelInfoGrid.InsertColumn(0, _T("LEVEL"), LVCFMT_RIGHT, 46);
    LevelInfoGrid.InsertColumn(1, _T("MIN EXP REQ"), LVCFMT_RIGHT, 90);
    LevelInfoGrid.InsertColumn(2, _T("BELT MAX GOLD"), LVCFMT_RIGHT, 110);
    LevelInfoGrid.InsertColumn(3, _T("STASH MAX GOLD"), LVCFMT_RIGHT, 122);
    FillCells();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2LevelInfoForm::FillCells()
{
    LevelInfoGrid.DeleteAllItems();
    std::uint32_t goldValue = 2500000;
    for (std::uint32_t i = 1; i <= d2ce::NUM_OF_LEVELS; ++i)
    {
        AddListColData(LevelInfoGrid, i - 1, 0, i);
        AddListColData(LevelInfoGrid, i - 1, 1, d2ce::MinExpRequired[i - 1]);
        AddListColData(LevelInfoGrid, i - 1, 2, i * 10000);

        if (Version < d2ce::EnumCharVersion::v110) // 1.00 - 1.09 character
        {
            if (i < 31)
            {
                goldValue = (i / 10 + 1) * 50000;
            }
            else if (Version >= d2ce::EnumCharVersion::v107) // 1.07 - 1.09 character
            {
                goldValue = (i / 2 + 1) * 50000;
            }
            else // pre 1.07 character
            {
                if (i < 90)
                {
                    goldValue = (i / 10 + 1) * 50000;
                }
                else
                {
                    goldValue = 2000000;
                }
            }
        }

        AddListColData(LevelInfoGrid, i - 1, 3, goldValue);
    }
}
//---------------------------------------------------------------------------
INT_PTR CD2LevelInfoForm::DoModal()
{
    Modal = true;
    auto ret = __super::DoModal();
    Modal = false;
    return ret;
}
//---------------------------------------------------------------------------
void CD2LevelInfoForm::OnOK()
{
    if (Modal)
    {
        __super::OnOK();
        return;
    }

    DestroyWindow();
}
//---------------------------------------------------------------------------
void CD2LevelInfoForm::OnCancel()
{
    if (Modal)
    {
        __super::OnCancel();
        return;
    }

    DestroyWindow();
}
//---------------------------------------------------------------------------
BOOL CD2LevelInfoForm::Show(CWnd* pParent)
{
    if (!::IsWindow(GetSafeHwnd()))
    {
        Modal = FALSE;
        BOOL bCreated = __super::Create(IDD_D2LEVELINFO_DIALOG, pParent);
        if (!bCreated)
        {
            return FALSE;
        }

        Version = d2ce::EnumCharVersion::v110;
        if (pParent != nullptr && pParent->IsKindOf(RUNTIME_CLASS(CD2MainForm)))
        {
            auto& CharInfo = ((CD2MainForm*)pParent)->CharInfo;
            if (CharInfo != nullptr)
            {
                Version = CharInfo->getVersion();
            }
        }
    }

    ShowWindow(SW_SHOWNORMAL);
    return TRUE;
}
//---------------------------------------------------------------------------
void CD2LevelInfoForm::ResetVersion(d2ce::EnumCharVersion version)
{
    if (!::IsWindow(GetSafeHwnd()))
    {
        return;
    }

    if (version != Version)
    {
        Version = version;
        FillCells();
    }
}
//---------------------------------------------------------------------------