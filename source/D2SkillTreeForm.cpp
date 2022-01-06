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
#include "D2SkillTreeForm.h"
#include "d2ce\SkillConstants.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------------------------------------------------------
// CD2SkillTreeForm dialog

//---------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CD2SkillTreeForm, CDialogEx)

//---------------------------------------------------------------------------
CD2SkillTreeForm::CD2SkillTreeForm(CD2MainForm& form)
    : CDialogEx(CD2SkillTreeForm::IDD, (CWnd*)&form), MainForm(form)
{
    Class = MainForm.getCharacterClass();
    SkillsUsed = MainForm.getSkillPointsUsed();
    SkillChoices = MainForm.getSkillChoices();
    Skills = MainForm.getSkills();

    // Fix up Skill Choices
    EarnedSkillPoints = MainForm.getSkillPointsEarned();
    if (SkillsUsed >= EarnedSkillPoints)
    {
        SkillChoices = 0;
    }
    else
    {
        SkillChoices = EarnedSkillPoints - SkillsUsed;
    }
}
//---------------------------------------------------------------------------
CD2SkillTreeForm::~CD2SkillTreeForm()
{
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
}
//---------------------------------------------------------------------------
BOOL CD2SkillTreeForm::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if (pMsg->wParam == VK_RETURN)
        {
            TCHAR szClass[10];
            CWnd* pWndFocus = GetFocus();
            if (((pWndFocus = GetFocus()) != NULL) &&
                IsChild(pWndFocus) &&
                GetClassName(pWndFocus->m_hWnd, szClass, 10) &&
                (lstrcmpi(szClass, _T("EDIT")) == 0))
            {
                // pressing the ENTER key will take the focus to the next control
                pMsg->wParam = VK_TAB;
            }
        }
    }
    return __super::PreTranslateMessage(pMsg);
}
//---------------------------------------------------------------------------
CString CD2SkillTreeForm::ToText(CWnd* Sender)
{
    CString strClassName;
    if (::GetClassName(Sender->GetSafeHwnd(), strClassName.GetBuffer(_MAX_PATH), _MAX_PATH))
    {
        if (strClassName.CompareNoCase(_T("Edit")) == 0 || strClassName.CompareNoCase(_T("STATIC")) == 0)
        {
            CString sWindowText;
            Sender->GetWindowText(sWindowText);
            return sWindowText;
        }

        if (strClassName.CompareNoCase(_T("ComboBox")) == 0)
        {
            CString sWindowText;
            ((CComboBox*)Sender)->GetLBText(((CComboBox*)Sender)->GetCurSel(), sWindowText);
            return sWindowText;
        }
    }

    return CString();
}
//---------------------------------------------------------------------------
std::string CD2SkillTreeForm::ToStdString(CWnd* Sender)
{
    return (LPCSTR)CStringA(ToText(Sender));
}
//---------------------------------------------------------------------------
std::uint32_t CD2SkillTreeForm::ToInt(UINT nID)
{
    CWnd* Sender = GetDlgItem(nID);
    if (Sender == nullptr)
    {
        return 0;
    }

    CString strClassName;
    if (::GetClassName(Sender->GetSafeHwnd(), strClassName.GetBuffer(_MAX_PATH), _MAX_PATH))
    {
        if (strClassName.CompareNoCase(_T("Edit")) == 0 || strClassName.CompareNoCase(_T("STATIC")) == 0)
        {
            std::string sWindowText(ToStdString(Sender));
            char* ptr = nullptr;
            return std::strtoul(sWindowText.c_str(), &ptr, 10);
        }

        if (strClassName.CompareNoCase(_T("ComboBox")) == 0)
        {
            return ((CComboBox*)Sender)->GetCurSel();
        }
    }

    return 0;
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::SetInt(UINT nID, std::uint32_t newValue)
{
    CWnd* Sender = GetDlgItem(nID);
    if (Sender == nullptr)
    {
        return;
    }

    CString strClassName;
    if (::GetClassName(Sender->GetSafeHwnd(), strClassName.GetBuffer(_MAX_PATH), _MAX_PATH))
    {
        if (strClassName.CompareNoCase(_T("Edit")) == 0 || strClassName.CompareNoCase(_T("STATIC")) == 0)
        {
            CString sWindowText;
            sWindowText.Format(_T("%lu"), newValue);
            Sender->SetWindowText(sWindowText);
            return;
        }

        if (strClassName.CompareNoCase(_T("ComboBox")) == 0)
        {
            ((CComboBox*)Sender)->SetCurSel(newValue);
            return;
        }
    }
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2SkillTreeForm, CDialogEx)
    ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_TREE_1_SKILL_1, IDC_EDIT_TREE_3_SKILL_10, OnSkillKillFocus)
    ON_BN_CLICKED(IDOK, &CD2SkillTreeForm::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CD2SkillTreeForm::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_SET_ALL_SKILLS, &CD2SkillTreeForm::OnBnClickedSetAll)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2SkillTreeForm message handlers

//---------------------------------------------------------------------------
BOOL CD2SkillTreeForm::OnInitDialog()
{
    __super::OnInitDialog();
    GetWindowText(OrigCaption);
    UpdateCaption();

    // Fill out tab names.
    auto classNumber = static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(Class);
    const auto& tabNames = d2ce::TabNames[classNumber];
    for (UINT i = 0, nIDC = IDC_STATIC_TREE_1; i < 3; ++i, ++nIDC)
    {
        GetDlgItem(nIDC)->SetWindowText(CString(tabNames[i].c_str()));
    }

    // Fill out skill names and values;
    const auto& tabSkillPos = d2ce::TabSkillPos[classNumber];
    const auto& skillsNames = d2ce::SkillsNames[classNumber];
    std::uint8_t Pos = 0;
    for (UINT i = 0, nIDC = IDC_STATIC_TREE_1_SKILL_1, nEditIDC = IDC_EDIT_TREE_1_SKILL_1; i < d2ce::NUM_OF_SKILLS; ++i, ++nIDC, ++nEditIDC)
    {
        Pos = tabSkillPos[i];
        GetDlgItem(nIDC)->SetWindowText(CString(skillsNames[Pos].c_str()));
        ((CEdit*)GetDlgItem(nEditIDC))->LimitText(2);
        SetInt(nEditIDC, Skills[Pos]);
    }
    ((CEdit*)GetDlgItem(IDC_EDIT_SET_ALL_SKILLS))->LimitText(2);
    SetInt(IDC_EDIT_SET_ALL_SKILLS, 0);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnSkillKillFocus(UINT nID)
{
    // Fill out skill names
    const auto& tabSkillPos = d2ce::TabSkillPos[static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(Class)];
    auto Pos = tabSkillPos[nID - IDC_EDIT_TREE_1_SKILL_1];
    std::uint8_t skillValue = (std::uint8_t)ToInt(nID);
    if (skillValue > d2ce::MAX_SKILL_VALUE)
    {
        skillValue = d2ce::MAX_SKILL_VALUE;
        SetInt(nID, skillValue);
    }

    // value actually changed?
    if (skillValue != Skills[Pos])
    {
        auto oldValue = Skills[Pos];
        Skills[Pos] = skillValue;
        if (oldValue > skillValue)
        {
            SkillsUsed -= (oldValue - skillValue);
            if (SkillsUsed + SkillChoices < EarnedSkillPoints)
            {
                SkillChoices = std::min(EarnedSkillPoints - SkillsUsed, d2ce::MAX_SKILL_CHOICES);
                UpdateCaption();
            }
        }
        else
        {
            std::uint8_t diff = (std::uint8_t)(skillValue - oldValue);
            SkillsUsed += diff;
            if (SkillChoices > 0)
            {
                if (SkillChoices > diff)
                {
                    SkillChoices -= diff;
                }
                else
                {
                    SkillChoices = 0;
                }

                if (SkillsUsed + SkillChoices < EarnedSkillPoints)
                {
                    SkillChoices = std::min(EarnedSkillPoints - SkillsUsed, d2ce::MAX_SKILL_CHOICES);
                }
                UpdateCaption();
            }
        }

        SkillsChanged = true;
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::SaveSkills()
{
    if (SkillsChanged)
    {
        MainForm.updateSkills(Skills);
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::UpdateCaption()
{
    if (!OrigCaption.IsEmpty())
    {
        std::wstringstream ss;
        ss << OrigCaption.GetString();
        if (SkillChoices > 0)
        {
            ss << _T(" (Skill Choices Remaining: ");
            ss << SkillChoices;
            ss << _T(")");
        }

        SetWindowText(ss.str().c_str());
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnBnClickedOk()
{
    SaveSkills();
    __super::OnOK();

}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnBnClickedCancel()
{
    SkillsChanged = false;
    __super::OnCancel();
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnBnClickedSetAll()
{
    std::uint8_t skillValue = (std::uint8_t)ToInt(IDC_EDIT_SET_ALL_SKILLS);
    if (skillValue > d2ce::MAX_SKILL_VALUE)
    {
        skillValue = d2ce::MAX_SKILL_VALUE;
        SetInt(IDC_EDIT_SET_ALL_SKILLS, skillValue);
    }

    // Save Values
    SkillsChanged = true;
    Skills.fill(skillValue);
    for (std::uint32_t nEditIDC = IDC_EDIT_TREE_1_SKILL_1; nEditIDC <= IDC_EDIT_TREE_3_SKILL_10; ++nEditIDC)
    {
        SetInt(nEditIDC, skillValue);
    }

    SkillsUsed = d2ce::NUM_OF_SKILLS * skillValue;
    if (SkillsUsed >= EarnedSkillPoints)
    {
        SkillChoices = 0;
    }
    else
    {
        SkillChoices = EarnedSkillPoints - SkillsUsed;
    }
    UpdateCaption();
}
//---------------------------------------------------------------------------
bool CD2SkillTreeForm::isSkillChoicesChanged() const
{
    return SkillsChanged;
}
//---------------------------------------------------------------------------

