/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
    Copyright (C) 2021-2023 Walter Couto

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
#include "D2MainFormConstants.h"
#include "d2ce/helpers/ItemHelpers.h"
#include <utf8/utf8.h>
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------------------------------------------------------
// CD2WaypointsForm dialog
IMPLEMENT_DYNAMIC(CD2WaypointsForm, CDialogEx)

//---------------------------------------------------------------------------
CD2WaypointsForm::CD2WaypointsForm(CD2MainForm& form)
    : CDialogEx(CD2WaypointsForm::IDD, (CWnd*)&form), MainForm(form), Acts(form.getQuests())
{
    // Initialize Waypoints
    LastAct = MainForm.getLastAct();
    TotalNumWaypoints = (LastAct == d2ce::EnumAct::V) ? d2ce::TOTAL_NUM_OF_WAYPOINTS_EXPANSION : d2ce::TOTAL_NUM_OF_WAYPOINTS;
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

    if (LastAct == d2ce::EnumAct::V)
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
    MaxItemIndex = std::max(ItemIndex, (int)static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(MainForm.getCharacterTitleDifficulty()));
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

    std::string strValue;
    std::u16string uText;
    CWnd* pWnd = nullptr;
    for (auto i = 0; i <= (int)static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(d2ce::EnumDifficulty::Hell); ++i)
    {
        pWnd = GetDlgItem(IDC_RADIO_DIFFICULTY_NORMAL + i);
        if (pWnd != nullptr)
        {
            d2ce::LocalizationHelpers::GetDifficultyStringTxtValue(static_cast<d2ce::EnumDifficulty>(i), strValue);
            uText = utf8::utf8to16(strValue);
            pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
            if (i >= MaxItemIndex + 1)
            {
                pWnd->EnableWindow(FALSE);
            }
        }
    }

    CString text;
    CStringA textA;
    for (UINT i = 0, nIDC = IDC_CHECK_ACTI_1; i < TotalNumWaypoints; ++i, ++nIDC)
    {
        pWnd = GetDlgItem(nIDC);
        if (pWnd == nullptr)
        {
            continue;
        }

        pWnd->GetWindowText(text);
        textA = text;
        if (d2ce::LocalizationHelpers::GetStringTxtValue(textA.GetString(), strValue))
        {
            uText = utf8::utf8to16(strValue);
            pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }
    }

    if (d2ce::LocalizationHelpers::GetStringTxtValue("act1", strValue))
    {
        uText = utf8::utf8to16(strValue);
        GetDlgItem(IDC_STATIC_ACTI)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

    if (d2ce::LocalizationHelpers::GetStringTxtValue("act2", strValue))
    {
        uText = utf8::utf8to16(strValue);
        GetDlgItem(IDC_STATIC_ACTII)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

    if (d2ce::LocalizationHelpers::GetStringTxtValue("act3", strValue))
    {
        uText = utf8::utf8to16(strValue);
        GetDlgItem(IDC_STATIC_ACTIII)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

    if (d2ce::LocalizationHelpers::GetStringTxtValue("act4", strValue))
    {
        uText = utf8::utf8to16(strValue);
        GetDlgItem(IDC_STATIC_ACTIV)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

    if (d2ce::LocalizationHelpers::GetStringTxtValue("act5", strValue))
    {
        uText = utf8::utf8to16(strValue);
        GetDlgItem(IDC_STATIC_ACTV)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

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

    auto& wp = (ItemIndex == 2) ? HellWP : (ItemIndex == 1 ? NightmareWP : NormalWP);
    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);

    auto actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::I);
    wp[UINT(IDC_CHECK_ACTII_1) - IDC_CHECK_ACTI_1] = actComplete;
    GetDlgItem(IDC_CHECK_ACTII_1)->EnableWindow(FALSE);
    for (UINT nIDC = IDC_CHECK_ACTII_2; nIDC <= IDC_CHECK_ACTII_9; ++nIDC)
    {
        GetDlgItem(nIDC)->EnableWindow(actComplete ? TRUE : FALSE);
        if (!actComplete)
        {
            wp[nIDC - IDC_CHECK_ACTI_1] = 0;
        }
    }

    actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::II);
    wp[INT(IDC_CHECK_ACTIII_1) - IDC_CHECK_ACTI_1] = actComplete;
    GetDlgItem(IDC_CHECK_ACTIII_1)->EnableWindow(FALSE);
    for (UINT nIDC = IDC_CHECK_ACTIII_2; nIDC <= IDC_CHECK_ACTIII_9; ++nIDC)
    {
        GetDlgItem(nIDC)->EnableWindow(actComplete ? TRUE : FALSE);
        if (!actComplete)
        {
            wp[nIDC - IDC_CHECK_ACTI_1] = 0;
        }
    }

    actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::III);
    wp[INT(IDC_CHECK_ACTIV_1) - IDC_CHECK_ACTI_1] = actComplete;
    GetDlgItem(IDC_CHECK_ACTIV_1)->EnableWindow(FALSE);
    for (UINT nIDC = IDC_CHECK_ACTIV_2; nIDC <= IDC_CHECK_ACTIV_3; ++nIDC)
    {
        GetDlgItem(nIDC)->EnableWindow(actComplete ? TRUE : FALSE);
        if (!actComplete)
        {
            wp[nIDC - IDC_CHECK_ACTI_1] = 0;
        }
    }

    if (LastAct < d2ce::EnumAct::V)
    {
        // Hide all of Act V
        pWnd = GetDlgItem(IDC_STATIC_ACTV);
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
        actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::IV);
        wp[UINT(IDC_CHECK_ACTV_1) - IDC_CHECK_ACTI_1] = actComplete;
        GetDlgItem(IDC_CHECK_ACTV_1)->EnableWindow(FALSE);
        for (UINT nIDC = IDC_CHECK_ACTV_2; nIDC <= IDC_CHECK_ACTV_9; ++nIDC)
        {
            GetDlgItem(nIDC)->EnableWindow(actComplete ? TRUE : FALSE);
            if (!actComplete)
            {
                wp[nIDC - IDC_CHECK_ACTI_1] = 0;
            }
        }
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2WaypointsForm::DDX_CheckWaypoints(CDataExchange* pDX)
{
    int value = 0;

    // Save checkbox values before difficulty level
    if (pDX->m_bSaveAndValidate)
    {
        auto& wp = (ItemIndex == 2) ? HellWP : (ItemIndex == 1 ? NightmareWP : NormalWP);
        wp[0] = true; // Always checked
        for (UINT i = 1, nIDC = IDC_CHECK_ACTI_2; i < TotalNumWaypoints; ++i, ++nIDC)
        {
            DDX_Check(pDX, nIDC, value);
            wp[i] = value != 0 ? true : false;
        }
    }

    // Update radio after checkboxes
    auto oldItemIndex = ItemIndex;
    DDX_Radio(pDX, IDC_RADIO_DIFFICULTY_NORMAL, ItemIndex);

    auto& wp = (ItemIndex == 2) ? HellWP : (ItemIndex == 1 ? NightmareWP : NormalWP);
    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);

    auto actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::I);
    wp[UINT(IDC_CHECK_ACTII_1) - IDC_CHECK_ACTI_1] = actComplete;
    GetDlgItem(IDC_CHECK_ACTII_1)->EnableWindow(FALSE);
    for (UINT nIDC = IDC_CHECK_ACTII_2; nIDC <= IDC_CHECK_ACTII_9; ++nIDC)
    {
        GetDlgItem(nIDC)->EnableWindow(actComplete ? TRUE : FALSE);
        if (!actComplete)
        {
            wp[nIDC - IDC_CHECK_ACTI_1] = 0;
        }
    }

    actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::II);
    wp[UINT(IDC_CHECK_ACTIII_1) - IDC_CHECK_ACTI_1] = actComplete;
    GetDlgItem(IDC_CHECK_ACTIII_1)->EnableWindow(FALSE);
    for (UINT nIDC = IDC_CHECK_ACTIII_2; nIDC <= IDC_CHECK_ACTIII_9; ++nIDC)
    {
        GetDlgItem(nIDC)->EnableWindow(actComplete ? TRUE : FALSE);
        if (!actComplete)
        {
            wp[nIDC - IDC_CHECK_ACTI_1] = 0;
        }
    }

    actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::III);
    wp[UINT(IDC_CHECK_ACTIV_1) - IDC_CHECK_ACTI_1] = actComplete;
    GetDlgItem(IDC_CHECK_ACTIV_1)->EnableWindow(FALSE);
    for (UINT nIDC = IDC_CHECK_ACTIV_2; nIDC <= IDC_CHECK_ACTIV_3; ++nIDC)
    {
        GetDlgItem(nIDC)->EnableWindow(actComplete ? TRUE : FALSE);
        if (!actComplete)
        {
            wp[nIDC - IDC_CHECK_ACTI_1] = 0;
        }
    }

    if (LastAct == d2ce::EnumAct::V)
    {
        actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::IV);
        wp[UINT(IDC_CHECK_ACTV_1) - IDC_CHECK_ACTI_1] = actComplete;
        GetDlgItem(IDC_CHECK_ACTV_1)->EnableWindow(FALSE);
        for (UINT nIDC = IDC_CHECK_ACTV_2; nIDC <= IDC_CHECK_ACTV_9; ++nIDC)
        {
            GetDlgItem(nIDC)->EnableWindow(actComplete ? TRUE : FALSE);
            if (!actComplete)
            {
                wp[nIDC - IDC_CHECK_ACTI_1] = 0;
            }
        }
    }

    // load checkbox values after difficulty level
    if (!pDX->m_bSaveAndValidate || (oldItemIndex != ItemIndex))
    {
        CDataExchange dx(this, FALSE);
        for (UINT i = 0, nIDC = IDC_CHECK_ACTI_1; i < TotalNumWaypoints; ++i, ++nIDC)
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
    auto& wp = (ItemIndex == 2) ? HellWP : (ItemIndex == 1 ? NightmareWP : NormalWP);

    // Act I
    for (std::uint32_t i = 0; i < d2ce::MAX_WAYPOINTS_PER_ACT; ++i)
    {
        wp[i] = true;
    }


    // Reset of Acts
    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);
    auto actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::I);
    for (std::uint32_t i = d2ce::MAX_WAYPOINTS_PER_ACT; i < TotalNumWaypoints; ++i)
    {
        switch (i)
        {
        case 2 * d2ce::MAX_WAYPOINTS_PER_ACT:
            actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::II);
            break;

        case 3 * d2ce::MAX_WAYPOINTS_PER_ACT:
            actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::III);
            break;

        case 3 * d2ce::MAX_WAYPOINTS_PER_ACT + d2ce::NUM_WAYPOINTS_ACT_IV:
            actComplete = Acts.getActCompleted(diff, d2ce::EnumAct::IV);
            break;
        }

        wp[i] = actComplete;
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
