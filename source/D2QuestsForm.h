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

#pragma once

//---------------------------------------------------------------------------
#include "D2MainForm.h"
#include "d2ce/ActsInfo.h"

//---------------------------------------------------------------------------
class CD2QuestsForm : public CDialogEx
{
    DECLARE_DYNAMIC(CD2QuestsForm)

public:
    CD2QuestsForm(CD2MainForm& form);   // standard constructor
    virtual ~CD2QuestsForm();

    // Dialog Data
    enum { IDD = IDD_QUESTS_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    void ViewOptionsClick(UINT nID);
    void QuestsChange(UINT nID);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedCompleteAll();
    DECLARE_MESSAGE_MAP()
    BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

public:
    virtual BOOL OnInitDialog();
    bool isQuestsChanged() const;

private:
    void DDX_CheckQuests(CDataExchange* pDX);
    void SaveQuests();

    bool getQuestIsRequired(d2ce::EnumAct act, std::uint8_t questNumber);

private:
    int ItemIndex = (int)static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Normal);
    int MaxItemIndex = (int)static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Hell);
    d2ce::EnumAct LastAct = d2ce::EnumAct::V;
    bool Changed = false;
    CD2MainForm& MainForm;
    d2ce::ActsInfo Acts;
    d2ce::ActsInfo OrigActs;
};
//---------------------------------------------------------------------------
