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

#pragma once

//---------------------------------------------------------------------------
#include "d2ce\Constants.h"
#include "d2ce\WaypointConstants.h"
#include "D2MainForm.h"

//---------------------------------------------------------------------------
class CD2WaypointsForm : public CDialogEx
{
    DECLARE_DYNAMIC(CD2WaypointsForm)

public:
    CD2WaypointsForm(CD2MainForm& form);
    virtual ~CD2WaypointsForm();

    // Dialog Data
    enum { IDD = IDD_WAYPOINTS_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    void ViewOptionsClick(UINT nID);
    void WPClickCheck(UINT nID);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedActivateAll();
    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    bool isWaypointsChanged() const;

private:
    void DDX_CheckWaypoints(CDataExchange* pDX);
    void SaveWaypoints();

private:
    d2ce::ActsInfo Acts;
    std::bitset<d2ce::NUM_OF_WAYPOINTS> NormalWP, NightmareWP, HellWP;
    int ItemIndex = 0;
    bool IsExpansionCharacter = false;
    bool Changed[d2ce::NUM_OF_DIFFICULTY] = { false, false, false };
    CD2MainForm& MainForm;
};
//---------------------------------------------------------------------------
