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
#include "D2QuestsForm.h"
#include "d2ce/helpers/ItemHelpers.h"
#include <utf8/utf8.h>
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr std::uint16_t questCompletedFlag = 0x9001;

//---------------------------------------------------------------------------
// CD2QuestsForm dialog

IMPLEMENT_DYNAMIC(CD2QuestsForm, CDialogEx)

//---------------------------------------------------------------------------
CD2QuestsForm::CD2QuestsForm(CD2MainForm& form)
    : CDialogEx(CD2QuestsForm::IDD, (CWnd*)&form), MainForm(form), Acts(form.getQuests()), OrigActs(form.getQuests())
{
    // Initialize Quests
    LastAct = MainForm.getLastAct();
    ItemIndex = (int)static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(MainForm.getDifficultyLastPlayed());
    MaxItemIndex = std::max(ItemIndex, (int)static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(MainForm.getCharacterTitleDifficulty()));
}
//---------------------------------------------------------------------------
CD2QuestsForm::~CD2QuestsForm()
{
}

//---------------------------------------------------------------------------
void CD2QuestsForm::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_CheckQuests(pDX);
}
//---------------------------------------------------------------------------
BOOL CD2QuestsForm::PreTranslateMessage(MSG* pMsg)
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
BEGIN_MESSAGE_MAP(CD2QuestsForm, CDialogEx)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_DIFFICULTY_NORMAL, IDC_RADIO_DIFFICULTY_HELL, ViewOptionsClick)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_ACTI_QUEST_1, IDC_CHECK_ACTV_QUEST_6, QuestsChange)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_QUEST_FARM, IDC_CHECK_QUEST_FARM, QuestsChange)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_ACTI_RESET_STATS, IDC_CHECK_ACTI_RESET_STATS, QuestsChange)
    ON_BN_CLICKED(IDOK, &CD2QuestsForm::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CD2QuestsForm::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_COMPLETE_ALL, &CD2QuestsForm::OnBnClickedCompleteAll)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2QuestsForm message handlers
//---------------------------------------------------------------------------
BOOL CD2QuestsForm::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    static CString strTipText;
    UINT_PTR nID = pNMHDR->idFrom;

    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
        pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
    {
        nID = ((UINT_PTR)(WORD)::GetDlgCtrlID((HWND)nID));
    }

    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);
    if (nID >= IDC_CHECK_ACTI_QUEST_1 && nID <= IDC_CHECK_ACTV_QUEST_6)
    {
        ::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);

        // Get Quest help
        std::uint8_t actNumber = (nID < IDC_CHECK_ACTV_QUEST_1) ? std::uint8_t((nID - IDC_CHECK_ACTI_QUEST_1) / 6) : std::uint8_t(4);
        d2ce::EnumAct act = static_cast<d2ce::EnumAct>(actNumber);
        std::uint8_t quest = (nID < IDC_CHECK_ACTV_QUEST_1) ? std::uint8_t((nID - IDC_CHECK_ACTI_QUEST_1) % 6) : std::uint8_t(nID - IDC_CHECK_ACTV_QUEST_1);
        std::string notes = Acts.getQuestNotes(diff, act, quest);
        strTipText = CString(notes.c_str());
    }
    else if (nID == IDC_CHECK_ACTI_RESET_STATS)
    {
        // Get Quest help
        if (Acts.getStatsReset(diff))
        {
            strTipText = _T("Akara Has Reset Stat/Skill Points");
        }
        else
        {
            strTipText = _T("Akara Has Yet To Reset Stat/Skill Points");
        }
    }
    else if (nID == IDC_CHECK_QUEST_FARM)
    {
        // Get Quest help
        strTipText.Empty();
        if (Acts.getMooMooFarmComplete(diff))
        {
            strTipText = _T("Secret Cow Level Completed");
        }
    }
    else
    {
        strTipText.Empty();
    }

#ifndef _UNICODE
    if (pNMHDR->code == TTN_NEEDTEXTA)
        pTTTA->lpszText = (LPSTR)((LPCSTR)strTipText);
    else
        _mbstowcsz(pTTTW->szText, strTipText, (sizeof(pTTTW->szText) / sizeof(pTTTW->szText[0])));
#else
    if (pNMHDR->code == TTN_NEEDTEXTA)
        _wcstombsz(pTTTA->szText, strTipText, (sizeof(pTTTA->szText) / sizeof(pTTTA->szText[0])));
    else
        pTTTW->lpszText = (LPWSTR)((LPCTSTR)strTipText);
#endif
    * pResult = 0;

    ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

    return TRUE;
}
//---------------------------------------------------------------------------
BOOL CD2QuestsForm::OnInitDialog()
{
    __super::OnInitDialog();

    EnableToolTips(TRUE);

    std::string strValue;
    d2ce::LocalizationHelpers::GetStringTxtValue("strpanel2", strValue, "Quests");
    auto uText = utf8::utf8to16(strValue);
    SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));

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

    if (d2ce::LocalizationHelpers::GetStringTxtValue("respec", strValue))
    {
        uText = utf8::utf8to16(strValue);
        GetDlgItem(IDC_CHECK_ACTI_RESET_STATS)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

    if (d2ce::LocalizationHelpers::GetStringTxtValue("Moo Moo Farm", strValue))
    {
        uText = utf8::utf8to16(strValue);
        GetDlgItem(IDC_CHECK_QUEST_FARM)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
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

    // update non-Expansion Quests
    std::uint32_t numQuests = IDC_CHECK_ACTIV_QUEST_3 - IDC_CHECK_ACTI_QUEST_1 + 1;
    std::uint16_t actNumber = 0;
    std::uint16_t questNumber = 0;
    for (std::uint32_t i = 0, nIDC = IDC_CHECK_ACTI_QUEST_1; i < numQuests; ++i, ++nIDC)
    {
        actNumber = std::uint16_t(i / d2ce::NUM_OF_QUESTS) + 1;
        questNumber = std::uint16_t(i % d2ce::NUM_OF_QUESTS) + 1;
        std::stringstream ss;
        ss << "qstsa";
        ss << actNumber;
        ss << "q";
        ss << questNumber;
        if (d2ce::LocalizationHelpers::GetStringTxtValue(ss.str(), strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(nIDC)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }
    }

    if (LastAct < d2ce::EnumAct::V)
    {
        // Hide all of Act V and Mo Mo Farm
        pWnd = GetDlgItem(IDC_STATIC_ACTV);
        if (pWnd != nullptr)
        {
            pWnd->EnableWindow(FALSE);
            pWnd->ShowWindow(SW_HIDE);
        }

        for (std::uint32_t i = 0; i < d2ce::NUM_OF_QUESTS; i++)
        {
            pWnd = GetDlgItem(IDC_CHECK_ACTV_QUEST_1 + i);
            if (pWnd != nullptr)
            {
                pWnd->EnableWindow(FALSE);
                pWnd->ShowWindow(SW_HIDE);
            }
        }
    }
    else
    {
        // update Expansion Quests
        questNumber = 1;
        for (std::uint32_t i = 0, nIDC = IDC_CHECK_ACTV_QUEST_1; i < d2ce::NUM_OF_QUESTS; ++i, ++nIDC)
        {
            std::stringstream ss;
            ss << "qstsa5q";
            ss << (i + 1);
            if (d2ce::LocalizationHelpers::GetStringTxtValue(ss.str(), strValue))
            {
                uText = utf8::utf8to16(strValue);
                GetDlgItem(nIDC)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
            }
        }
    }

    CString text;
    CStringA textA;
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

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2QuestsForm::DDX_CheckQuests(CDataExchange* pDX)
{
    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);

    int value = 0;
    std::uint32_t numQuests = IDC_CHECK_ACTIV_QUEST_3 - IDC_CHECK_ACTI_QUEST_1 + 1;

    bool bForceUpdate = false;

    // Save checkbox values before difficulty level
    if (pDX->m_bSaveAndValidate)
    {
        std::uint8_t actNumber = 0;
        std::uint8_t questNumber = 0;
        d2ce::EnumAct act = d2ce::EnumAct::I;
        auto lastActStarted = act;

        // update non-Expansion Quests
        for (std::uint32_t i = 0, nIDC = IDC_CHECK_ACTI_QUEST_1; i < numQuests; ++i, ++nIDC)
        {
            actNumber = std::uint8_t(i / d2ce::NUM_OF_QUESTS);
            act = static_cast<d2ce::EnumAct>(actNumber);
            questNumber = std::uint8_t(i % d2ce::NUM_OF_QUESTS);

            DDX_Check(pDX, nIDC, value);
            if (Acts.getQuestIsRequired(diff, act, questNumber))
            {
                lastActStarted = act;
                value = 1;
            }

            switch (value)
            {
            case 0:
                Acts.resetQuest(diff, act, questNumber);
                break;

            case 1:
                lastActStarted = act;

                // Only change "Completed" bits if quest was not already in completed state
                // so that we preserve current state
                if (!Acts.getQuestCompleted(diff, act, questNumber))
                {
                    if (OrigActs.getQuestCompleted(diff, act, questNumber))
                    {
                        // Use the original value
                        Acts.setQuestData(diff, act, questNumber, OrigActs.getQuestData(diff, act, questNumber));
                    }
                    else
                    {
                        Acts.completeQuest(diff, act, questNumber);
                    }
                }
                break;

            case 2:
                lastActStarted = act;

                // Only change "Started" bits if quest was not already in started state
                // so that we preserve current state
                if (!Acts.getQuestStarted(diff, act, questNumber))
                {
                    if (OrigActs.getQuestStarted(diff, act, questNumber))
                    {
                        // Use the original value
                        Acts.setQuestData(diff, act, questNumber, OrigActs.getQuestData(diff, act, questNumber));
                    }
                    else
                    {
                        Acts.startQuest(diff, act, questNumber);
                    }
                }
                break;
            }
        }

        if (LastAct == d2ce::EnumAct::V)
        {
            // update Expansion Quests
            actNumber = 4;
            act = d2ce::EnumAct::V;
            questNumber = 0;
            for (std::uint32_t nIDC = IDC_CHECK_ACTV_QUEST_1; questNumber < d2ce::NUM_OF_QUESTS; ++questNumber, ++nIDC)
            {
                DDX_Check(pDX, nIDC, value);
                if (Acts.getQuestIsRequired(diff, act, questNumber))
                {
                    lastActStarted = act;
                    value = 1;
                }

                switch (value)
                {
                case 0:
                    Acts.resetQuest(diff, act, questNumber);
                    break;

                case 1:
                    lastActStarted = act;

                    // Only change "Completed" bits if quest was not already in completed state
                    // so that we preserve current state
                    if (!Acts.getQuestCompleted(diff, act, questNumber))
                    {
                        if (OrigActs.getQuestCompleted(diff, act, questNumber))
                        {
                            // Use the original value
                            Acts.setQuestData(diff, act, questNumber, OrigActs.getQuestData(diff, act, questNumber));
                        }
                        else
                        {
                            Acts.completeQuest(diff, act, questNumber);
                        }
                    }
                    break;

                case 2:
                    lastActStarted = act;

                    // Only change "Started" bits if quest was not already in started state
                    // so that we preserve current state
                    if (!Acts.getQuestStarted(diff, act, questNumber))
                    {
                        if (OrigActs.getQuestStarted(diff, act, questNumber))
                        {
                            // Use the original value
                            Acts.setQuestData(diff, act, questNumber, OrigActs.getQuestData(diff, act, questNumber));
                        }
                        else
                        {
                            Acts.startQuest(diff, act, questNumber);
                        }
                    }
                    break;
                }
            }
        }

        // Handle other cases
        CDataExchange dx(this, FALSE);
        value = 0;
        if (!Acts.getQuestCompleted(diff, d2ce::EnumAct::I, 0)) // Den of Evil
        {
            DDX_Check(&dx, IDC_CHECK_ACTI_RESET_STATS, value);
            GetDlgItem(IDC_CHECK_ACTI_RESET_STATS)->EnableWindow(FALSE);
            Acts.clearStatsReset(diff);
        }
        else
        {
            GetDlgItem(IDC_CHECK_ACTI_RESET_STATS)->EnableWindow(TRUE);
            DDX_Check(pDX, IDC_CHECK_ACTI_RESET_STATS, value);
            if (value == 0)
            {
                Acts.clearStatsReset(diff);
            }
            else
            {
                Acts.setStatsReset(diff);
            }
        }

        value = 0;
        if (Acts.getQuestYetToStart(diff, d2ce::EnumAct::I, 3)) // The Search for Cain
        {
            DDX_Check(&dx, IDC_CHECK_QUEST_FARM, value);
            GetDlgItem(IDC_CHECK_QUEST_FARM)->EnableWindow(FALSE);
            Acts.clearMooMooFarmComplete(diff);
        }
        else
        {
            GetDlgItem(IDC_CHECK_QUEST_FARM)->EnableWindow(TRUE);
            DDX_Check(pDX, IDC_CHECK_QUEST_FARM, value);
            if (value == 0)
            {
                Acts.clearMooMooFarmComplete(diff);
            }
            else
            {
                Acts.setMooMooFarmComplete(diff);
            }
        }

        if (ItemIndex == MaxItemIndex)
        {
            // Make sure all Acts are in a valid state
            if (lastActStarted < LastAct)
            {
                auto nextAct = static_cast<d2ce::EnumAct>(static_cast<std::underlying_type_t<d2ce::EnumAct>>(lastActStarted) + 1);
                if (Acts.getActCompleted(diff, nextAct) || Acts.getActIntroduced(diff, nextAct))
                {
                    // we have an invalid state, so fix it
                    Acts.resetAct(nextAct);
                    bForceUpdate = true;
                }
                else if (lastActStarted > d2ce::EnumAct::I)
                {
                    auto prevAct = static_cast<d2ce::EnumAct>(static_cast<std::underlying_type_t<d2ce::EnumAct>>(lastActStarted) - 1);
                    if (!Acts.getActCompleted(diff, prevAct))
                    {
                        // we have an invalid state, so fix it
                        Acts.updateAct(lastActStarted);
                        bForceUpdate = true;
                    }
                }
            }
            else if (lastActStarted > d2ce::EnumAct::I)
            {
                auto prevAct = static_cast<d2ce::EnumAct>(static_cast<std::underlying_type_t<d2ce::EnumAct>>(lastActStarted) - 1);
                if (!Acts.getActCompleted(diff, prevAct))
                {
                    // we have an invalid state, so fix it
                    Acts.updateAct(lastActStarted);
                    bForceUpdate = true;
                }
            }
        }
    }

    // Update radio after checkboxes
    auto oldItemIndex = ItemIndex;
    DDX_Radio(pDX, IDC_RADIO_DIFFICULTY_NORMAL, ItemIndex);

    // load checkbox values after difficulty level
    bForceUpdate |= (oldItemIndex != ItemIndex);
    BOOL bEnabled = TRUE;
    if (!pDX->m_bSaveAndValidate || bForceUpdate)
    {
        CDataExchange dx(this, FALSE);
        std::uint8_t actNumber = 0;
        std::uint8_t questNumber = 0;
        d2ce::EnumAct act = d2ce::EnumAct::I;
        diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);

        // update non-Expansion Quests
        for (std::uint32_t i = 0, nIDC = IDC_CHECK_ACTI_QUEST_1; i < numQuests; ++i, ++nIDC)
        {
            bEnabled = TRUE;
            actNumber = std::uint8_t(i / d2ce::NUM_OF_QUESTS);
            act = static_cast<d2ce::EnumAct>(actNumber);
            questNumber = std::uint8_t(i % d2ce::NUM_OF_QUESTS);
            if (Acts.getQuestIsRequired(diff, act, questNumber))
            {
                bEnabled = FALSE;
                value = 1;
                
                // Only change "Completed" bits if quest was not already in completed state
                // so that we preserve current state
                if (!Acts.getQuestCompleted(diff, act, questNumber))
                {
                    if (OrigActs.getQuestCompleted(diff, act, questNumber))
                    {
                        // Use the original value
                        Acts.setQuestData(diff, act, questNumber, OrigActs.getQuestData(diff, act, questNumber));
                    }
                    else
                    {
                        Acts.completeQuest(diff, act, questNumber);
                    }
                }
            }
            else if (Acts.getQuestYetToStart(diff, act, questNumber))
            {
                value = 0;
            }
            else if (Acts.getQuestCompleted(diff, act, questNumber))
            {
                value = 1;
            }
            else
            {
                value = 2;
            }

            DDX_Check(&dx, nIDC, value);
            GetDlgItem(nIDC)->EnableWindow(bEnabled);

            switch (act)
            {
            case d2ce::EnumAct::I:
                switch (questNumber)
                {
                case 0: // Den of Evil
                    if (value == 1)
                    {
                        GetDlgItem(IDC_CHECK_ACTI_RESET_STATS)->EnableWindow(TRUE);
                    }
                    else
                    {
                        int tempValue = 0;
                        DDX_Check(&dx, IDC_CHECK_ACTI_RESET_STATS, tempValue);
                        GetDlgItem(IDC_CHECK_ACTI_RESET_STATS)->EnableWindow(FALSE);

                    }
                    break;

                case 3: // The Search for Cain
                    if (value == 0)
                    {
                        int tempValue = 0;
                        DDX_Check(&dx, IDC_CHECK_QUEST_FARM, tempValue);
                        GetDlgItem(IDC_CHECK_QUEST_FARM)->EnableWindow(value == 0 ? FALSE : true);
                    }
                    else
                    {
                        GetDlgItem(IDC_CHECK_QUEST_FARM)->EnableWindow(TRUE);
                    }
                }
            }
        }

        if (LastAct == d2ce::EnumAct::V)
        {
            // update Expansion Quests
            actNumber = 4;
            act = d2ce::EnumAct::V;
            questNumber = 0;
            for (std::uint32_t nIDC = IDC_CHECK_ACTV_QUEST_1; questNumber < d2ce::NUM_OF_QUESTS; ++questNumber, ++nIDC)
            {
                bEnabled = TRUE;
                if (Acts.getQuestIsRequired(diff, act, questNumber))
                {
                    bEnabled = FALSE;
                    value = 1;

                    // Only change "Completed" bits if quest was not already in completed state
                    // so that we preserve current state
                    if (!Acts.getQuestCompleted(diff, act, questNumber))
                    {
                        if (OrigActs.getQuestCompleted(diff, act, questNumber))
                        {
                            // Use the original value
                            Acts.setQuestData(diff, act, questNumber, OrigActs.getQuestData(diff, act, questNumber));
                        }
                        else
                        {
                            Acts.completeQuest(diff, act, questNumber);
                        }
                    }
                }
                else if (Acts.getQuestYetToStart(diff, act, questNumber))
                {
                    value = 0;
                }
                else if (Acts.getQuestCompleted(diff, act, questNumber))
                {
                    value = 1;
                }
                else
                {
                    value = 2;
                }

                DDX_Check(&dx, nIDC, value);
                GetDlgItem(nIDC)->EnableWindow(bEnabled);
            }
        }

        value = 0;
        if (!Acts.getQuestCompleted(diff, d2ce::EnumAct::I, 0))
        {
            GetDlgItem(IDC_CHECK_ACTI_RESET_STATS)->EnableWindow(FALSE);
            Acts.clearStatsReset(diff);
        }
        else
        {
            GetDlgItem(IDC_CHECK_ACTI_RESET_STATS)->EnableWindow(TRUE);
            if (Acts.getStatsReset(diff))
            {
                value = 1;
            }
        }
        DDX_Check(&dx, IDC_CHECK_ACTI_RESET_STATS, value);

        GetDlgItem(IDC_CHECK_QUEST_FARM)->EnableWindow(Acts.getQuestYetToStart(diff, d2ce::EnumAct::I, 3) ? FALSE : TRUE);
        value = Acts.getMooMooFarmComplete(diff) ? 1 : 0;
        DDX_Check(&dx, IDC_CHECK_QUEST_FARM, value);
    }
}
//---------------------------------------------------------------------------
void CD2QuestsForm::ViewOptionsClick(UINT /*nID*/)
{
    UpdateData(TRUE); // save results
}
//---------------------------------------------------------------------------
void CD2QuestsForm::QuestsChange(UINT /*nID*/)
{
    UpdateData(TRUE); // save results
    Changed = true;
}
//---------------------------------------------------------------------------
void CD2QuestsForm::SaveQuests()
{
    if (Changed)
    {
        MainForm.updateQuests(Acts);
    }
}
//---------------------------------------------------------------------------
bool CD2QuestsForm::getQuestIsRequired(d2ce::EnumAct act, std::uint8_t questNumber)
{
    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);
    return Acts.getQuestIsRequired(diff, act, questNumber);
}
//---------------------------------------------------------------------------
void CD2QuestsForm::OnBnClickedOk()
{
    SaveQuests();
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2QuestsForm::OnBnClickedCancel()
{
    Changed = false;
    __super::OnCancel();
}
//---------------------------------------------------------------------------
void CD2QuestsForm::OnBnClickedCompleteAll()
{
    Changed = true;
    HWND hWndCtrl;
    std::uint32_t lastQuestIDC = (LastAct == d2ce::EnumAct::V) ? IDC_CHECK_ACTV_QUEST_6 : IDC_CHECK_ACTIV_QUEST_3;
    for (std::uint32_t nIDC = IDC_CHECK_ACTI_QUEST_1; nIDC <= lastQuestIDC; ++nIDC)
    {
        GetDlgItem(nIDC, &hWndCtrl);
        ::SendMessage(hWndCtrl, BM_SETCHECK, (WPARAM)1, 0L);
    }
    GetDlgItem(IDC_CHECK_ACTI_RESET_STATS)->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK_QUEST_FARM)->EnableWindow(TRUE);
    UpdateData(TRUE); // save results
}
//---------------------------------------------------------------------------
bool CD2QuestsForm::isQuestsChanged() const
{
    return Changed;
}
//---------------------------------------------------------------------------

