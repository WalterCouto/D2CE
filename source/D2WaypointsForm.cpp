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
#include "D2WaypointsForm.h"
#include "MainFormConstants.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------------------------------------------------------
// CD2WaypointsForm dialog
IMPLEMENT_DYNAMIC(CD2WaypointsForm, CDialogEx)

//---------------------------------------------------------------------------
CD2WaypointsForm::CD2WaypointsForm(CD2MainForm& form)
    : CDialogEx(IDD_WAYPOINTS_DIALOG, (CWnd*)&form), MainForm(form), Acts(form.getQuests())
{
    // Initialize Waypoints
    ItemIndex = 0;
    isExpansionCharacter = MainForm.isExpansionCharacter();
    NormalWP = MainForm.getWaypoints(d2ce::EnumDifficulty::Normal);
    NightmareWP = MainForm.getWaypoints(d2ce::EnumDifficulty::Nightmare);
    HellWP = MainForm.getWaypoints(d2ce::EnumDifficulty::Hell);

    // make sure Act I - Rogue Encampment is always active
    NormalWP[0] = true;
    NightmareWP[0] = true;
    HellWP[0] = true;

    // make sure Act II - Lut Gholein is active if applicable
    if (Acts.getActCompleted(d2ce::EnumDifficulty::Normal, d2ce::EnumAct::I))
    {
        NormalWP[IDC_CHECK_ACTII_1 - IDC_CHECK_ACTI_1] = true;
    }

    if (Acts.getActCompleted(d2ce::EnumDifficulty::Nightmare, d2ce::EnumAct::I))
    {
        NightmareWP[IDC_CHECK_ACTII_1 - IDC_CHECK_ACTI_1] = true;
    }

    if (Acts.getActCompleted(d2ce::EnumDifficulty::Hell, d2ce::EnumAct::I))
    {
        HellWP[IDC_CHECK_ACTII_1 - IDC_CHECK_ACTI_1] = true;
    }

    // make sure Act III - Kurast Docks is active if applicable
    if (Acts.getActCompleted(d2ce::EnumDifficulty::Normal, d2ce::EnumAct::II))
    {
        NormalWP[IDC_CHECK_ACTIII_1 - IDC_CHECK_ACTI_1] = true;
    }

    if (Acts.getActCompleted(d2ce::EnumDifficulty::Nightmare, d2ce::EnumAct::II))
    {
        NightmareWP[IDC_CHECK_ACTIII_1 - IDC_CHECK_ACTI_1] = true;
    }

    if (Acts.getActCompleted(d2ce::EnumDifficulty::Hell, d2ce::EnumAct::II))
    {
        HellWP[IDC_CHECK_ACTIII_1 - IDC_CHECK_ACTI_1] = true;
    }

    // make sure Act IV - Kurast Docks is active if applicable
    if (Acts.getActCompleted(d2ce::EnumDifficulty::Normal, d2ce::EnumAct::III))
    {
        NormalWP[IDC_CHECK_ACTIV_1 - IDC_CHECK_ACTI_1] = true;
    }

    if (Acts.getActCompleted(d2ce::EnumDifficulty::Nightmare, d2ce::EnumAct::III))
    {
        NightmareWP[IDC_CHECK_ACTIV_1 - IDC_CHECK_ACTI_1] = true;
    }

    if (Acts.getActCompleted(d2ce::EnumDifficulty::Hell, d2ce::EnumAct::III))
    {
        HellWP[IDC_CHECK_ACTIV_1 - IDC_CHECK_ACTI_1] = true;
    }

    if (isExpansionCharacter)
    {
        // make sure Act V - Harrogath is active if applicable
        if (Acts.getActCompleted(d2ce::EnumDifficulty::Normal, d2ce::EnumAct::IV))
        {
            NormalWP[IDC_CHECK_ACTV_1 - IDC_CHECK_ACTI_1] = true;
        }

        if (Acts.getActCompleted(d2ce::EnumDifficulty::Nightmare, d2ce::EnumAct::IV))
        {
            NightmareWP[IDC_CHECK_ACTV_1 - IDC_CHECK_ACTI_1] = true;
        }

        if (Acts.getActCompleted(d2ce::EnumDifficulty::Hell, d2ce::EnumAct::IV))
        {
            HellWP[IDC_CHECK_ACTV_1 - IDC_CHECK_ACTI_1] = true;
        }
    }

    ItemIndex = (int)static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(MainForm.getDifficultyLastPlayed());
}
//---------------------------------------------------------------------------
CD2WaypointsForm::~CD2WaypointsForm()
{
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_CheckWaypoints(pDX);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2WaypointsForm, CDialogEx)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_DIFFICULTY_NORMAL, IDC_RADIO_DIFFICULTY_HELL, ViewOptionsClick)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_ACTI_2, IDC_CHECK_ACTV_9, WPClickCheck)
    ON_BN_CLICKED(IDOK, &CD2WaypointsForm::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CD2WaypointsForm::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_ACTIVATE_ALL, &CD2WaypointsForm::OnBnClickedActivateAll)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2WaypointsForm message handlers

//---------------------------------------------------------------------------
BOOL CD2WaypointsForm::OnInitDialog()
{
    __super::OnInitDialog();

    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);
    GetDlgItem(IDC_CHECK_ACTII_1)->EnableWindow(Acts.getActCompleted(diff, d2ce::EnumAct::I) ? FALSE : TRUE);
    GetDlgItem(IDC_CHECK_ACTIII_1)->EnableWindow(Acts.getActCompleted(diff, d2ce::EnumAct::II) ? FALSE : TRUE);
    GetDlgItem(IDC_CHECK_ACTIV_1)->EnableWindow(Acts.getActCompleted(diff, d2ce::EnumAct::III) ? FALSE : TRUE);

    if (!isExpansionCharacter)
    {
        // Hide all of Act V
        CWnd* pWnd = GetDlgItem(IDC_STATIC_ACTV);
        if (pWnd != nullptr)
        {
            pWnd->EnableWindow(FALSE);
            pWnd->ShowWindow(SW_HIDE);
        }

        for (std::uint32_t i = 0; i < d2ce::MAX_WAYPOINTS_PER_ACT; i++)
        {
            pWnd = GetDlgItem(IDC_CHECK_ACTV_1 + i);
            if (pWnd != nullptr)
            {
                pWnd->EnableWindow(FALSE);
                pWnd->ShowWindow(SW_HIDE);
            }
        }
    }
    else
    {
        GetDlgItem(IDC_CHECK_ACTV_1)->EnableWindow(Acts.getActCompleted(diff, d2ce::EnumAct::IV) ? FALSE : TRUE);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::DDX_CheckWaypoints(CDataExchange* pDX)
{
    int value = 0;
    std::uint32_t numWaypoints = isExpansionCharacter ? d2ce::TOTAL_NUM_OF_WAYPOINTS_EXPANSION : d2ce::TOTAL_NUM_OF_WAYPOINTS;

    // Save checkbox values before difficulty level
    if (pDX->m_bSaveAndValidate)
    {
        auto& wp = (ItemIndex == 2) ? HellWP : (ItemIndex == 1 ? NightmareWP : NormalWP);
        wp[0] = true; // Always checked
        for (UINT i = 1, nIDC = IDC_CHECK_ACTI_2; i < numWaypoints; ++i, ++nIDC)
        {
            DDX_Check(pDX, nIDC, value);
            wp[i] = value != 0 ? true : false;
        }
    }

    // Update radio after checkboxes
    auto oldItemIndex = ItemIndex;
    DDX_Radio(pDX, IDC_RADIO_DIFFICULTY_NORMAL, ItemIndex);

    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);
    GetDlgItem(IDC_CHECK_ACTII_1)->EnableWindow(Acts.getActCompleted(diff, d2ce::EnumAct::I) ? FALSE : TRUE);
    GetDlgItem(IDC_CHECK_ACTIII_1)->EnableWindow(Acts.getActCompleted(diff, d2ce::EnumAct::II) ? FALSE : TRUE);
    GetDlgItem(IDC_CHECK_ACTIV_1)->EnableWindow(Acts.getActCompleted(diff, d2ce::EnumAct::III) ? FALSE : TRUE);
    if (isExpansionCharacter)
    {
        GetDlgItem(IDC_CHECK_ACTV_1)->EnableWindow(Acts.getActCompleted(diff, d2ce::EnumAct::IV) ? FALSE : TRUE);
    }

    // load checkbox values after difficulty level
    if (!pDX->m_bSaveAndValidate || (oldItemIndex != ItemIndex))
    {
        CDataExchange dx(this, FALSE);
        auto& wp = (ItemIndex == 2) ? HellWP : (ItemIndex == 1 ? NightmareWP : NormalWP);
        for (UINT i = 0, nIDC = IDC_CHECK_ACTI_1; i < numWaypoints; ++i, ++nIDC)
        {
            value = wp[i];
            DDX_Check(&dx, nIDC, value);
        }
    }
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::ViewOptionsClick(UINT /*nID*/)
{
    UpdateData(TRUE); // save results
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::WPClickCheck(UINT /*nID*/)
{
    Changed[ItemIndex] = true;
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::SaveWaypoints()
{
    UpdateData(TRUE); // save results
    if (Changed[0])
    {
        MainForm.setWaypoints(d2ce::EnumDifficulty::Normal, NormalWP.to_ullong());
    }

    if (Changed[1])
    {
        MainForm.setWaypoints(d2ce::EnumDifficulty::Nightmare, NightmareWP.to_ullong());
    }

    if (Changed[2])
    {
        MainForm.setWaypoints(d2ce::EnumDifficulty::Hell, HellWP.to_ullong());
    }
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::OnBnClickedOk()
{
    SaveWaypoints();
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::OnBnClickedCancel()
{
    for (std::uint32_t i = 0; i < d2ce::NUM_OF_DIFFICULTY; ++i)
    {
        Changed[i] = false;
    }

    __super::OnCancel();
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::OnBnClickedActivateAll()
{
    std::uint32_t numWaypoints = isExpansionCharacter ? d2ce::TOTAL_NUM_OF_WAYPOINTS_EXPANSION : d2ce::TOTAL_NUM_OF_WAYPOINTS;
    auto& wp = (ItemIndex == 2) ? HellWP : (ItemIndex == 1 ? NightmareWP : NormalWP);
    for (std::uint32_t i = 0; i < numWaypoints; ++i)
    {
        wp[i] = true;
    }

    UpdateData(FALSE);
    Changed[ItemIndex] = true;
}
//---------------------------------------------------------------------------
bool CD2WaypointsForm::isWaypointsChanged() const
{
    for (std::uint32_t i = 0; i < d2ce::NUM_OF_DIFFICULTY; ++i)
    {
        if (Changed[i])
        {
            return  true;
        }
    }

    return false;
}
//---------------------------------------------------------------------------
