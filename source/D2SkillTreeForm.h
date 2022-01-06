/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
    Copyright (C) 2021-2022 Walter Couto

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; If not, see <http://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------
#include "D2MainForm.h"

//---------------------------------------------------------------------------
class CD2SkillTreeForm : public CDialogEx
{
    DECLARE_DYNAMIC(CD2SkillTreeForm)

public:
    CD2SkillTreeForm(CD2MainForm& form);
    virtual ~CD2SkillTreeForm();

    // Dialog Data
    enum { IDD = IDD_SKILLS_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    void OnSkillKillFocus(UINT nID);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedSetAll();
    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    bool isSkillChoicesChanged() const;

private:
    CString ToText(CWnd* Sender);
    std::string ToStdString(CWnd* Sender);
    std::uint32_t ToInt(UINT nID);
    void SetInt(UINT nID, std::uint32_t newValue);
    void SaveSkills();
    void UpdateCaption();

private:
    std::array<std::uint8_t, d2ce::NUM_OF_SKILLS> Skills;
    bool SkillsChanged = false;
    CD2MainForm& MainForm;
    d2ce::EnumCharClass Class = d2ce::EnumCharClass::Amazon;
    std::uint32_t SkillChoices = 0;
    std::uint32_t SkillsUsed = 0;
    std::uint32_t EarnedSkillPoints = 0;
    CString OrigCaption;
};
//---------------------------------------------------------------------------
