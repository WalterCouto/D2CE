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
#include "D2QuestsForm.h"
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
    IsExpansionCharacter = MainForm.isExpansionCharacter();
    ItemIndex = static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(MainForm.getDifficultyLastPlayed());
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

    if (!IsExpansionCharacter)
    {
        // Hide all of Act V and Mo Mo Farm
        CWnd* pWnd = GetDlgItem(IDC_STATIC_ACTV);
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

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2QuestsForm::DDX_CheckQuests(CDataExchange* pDX)
{
    d2ce::EnumDifficulty diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);

    int value = 0;
    std::uint32_t numQuests = IDC_CHECK_ACTIV_QUEST_3 - IDC_CHECK_ACTI_QUEST_1 + 1;

    // Save checkbox values before difficulty level
    if (pDX->m_bSaveAndValidate)
    {
        std::uint8_t actNumber = 0;
        std::uint8_t questNumber = 0;
        d2ce::EnumAct act = d2ce::EnumAct::I;

        // update non-Expansion Quests
        for (std::uint32_t i = 0, nIDC = IDC_CHECK_ACTI_QUEST_1; i < numQuests; ++i, ++nIDC)
        {
            actNumber = std::uint8_t(i / d2ce::NUM_OF_QUESTS);
            act = static_cast<d2ce::EnumAct>(actNumber);
            questNumber = std::uint8_t(i % d2ce::NUM_OF_QUESTS);

            DDX_Check(pDX, nIDC, value);
            switch (value)
            {
            case 0:
                Acts.resetQuest(diff, act, questNumber);
                break;
            case 1:
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

        if (IsExpansionCharacter)
        {
            // update Expansion Quests
            actNumber = 4;
            act = d2ce::EnumAct::V;
            questNumber = 0;
            for (std::uint32_t nIDC = IDC_CHECK_ACTV_QUEST_1; questNumber < d2ce::NUM_OF_QUESTS; ++questNumber, ++nIDC)
            {
                DDX_Check(pDX, nIDC, value);
                switch (value)
                {
                case 0:
                    Acts.resetQuest(diff, act, questNumber);
                    break;
                case 1:
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
        DDX_Check(pDX, IDC_CHECK_QUEST_FARM, value);
        if (value == 1)
        {
            Acts.setMooMooFarmComplete(diff);
        }
        else
        {
            Acts.clearMooMooFarmComplete(diff);
        }

        DDX_Check(pDX, IDC_CHECK_ACTI_RESET_STATS, value);
        if (value == 1)
        {
            Acts.setStatsReset(diff);
        }
        else
        {
            Acts.clearStatsReset(diff);
        }
    }

    // Update radio after checkboxes
    auto oldItemIndex = ItemIndex;
    DDX_Radio(pDX, IDC_RADIO_DIFFICULTY_NORMAL, ItemIndex);

    // load checkbox values after difficulty level
    if (!pDX->m_bSaveAndValidate || (oldItemIndex != ItemIndex))
    {
        CDataExchange dx(this, FALSE);
        std::uint8_t actNumber = 0;
        std::uint8_t questNumber = 0;
        d2ce::EnumAct act = d2ce::EnumAct::I;
        diff = static_cast<d2ce::EnumDifficulty>(ItemIndex);

        // update non-Expansion Quests
        for (std::uint32_t i = 0, nIDC = IDC_CHECK_ACTI_QUEST_1; i < numQuests; ++i, ++nIDC)
        {
            actNumber = std::uint8_t(i / d2ce::NUM_OF_QUESTS);
            act = static_cast<d2ce::EnumAct>(actNumber);
            questNumber = std::uint8_t(i % d2ce::NUM_OF_QUESTS);
            if (Acts.getQuestYetToStart(diff, act, questNumber))
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
        }

        if (IsExpansionCharacter)
        {
            // update Expansion Quests
            actNumber = 4;
            act = d2ce::EnumAct::V;
            questNumber = 0;
            for (std::uint32_t nIDC = IDC_CHECK_ACTV_QUEST_1; questNumber < d2ce::NUM_OF_QUESTS; ++questNumber, ++nIDC)
            {
                if (Acts.getQuestYetToStart(diff, act, questNumber))
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
    std::uint32_t lastQuestIDC = IsExpansionCharacter ? IDC_CHECK_ACTV_QUEST_6 : IDC_CHECK_ACTIV_QUEST_3;
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

