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
#include "d2ce/SkillConstants.h"
#include "d2ce/CharacterConstants.h"
#include "d2ce/CharacterStats.h"
#include "d2ce/helpers/ItemHelpers.h"
#include <utf8/utf8.h>
#include "afxdialogex.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    void DrawDependencyLine(Gdiplus::Graphics& graphics, Gdiplus::Pen& pen, CRect& rect, CRect& depRect)
    {
        long y = 0;
        long x = 0;
        long depY = 0;
        long depX = 0;
        if (depRect.left < rect.left)
        {
            // dependency to the left
            x = rect.left;
            depX = depRect.right;

            if (depRect.bottom == rect.bottom)
            {
                y = rect.top + rect.Height() / 3;
                depY = depRect.top + depRect.Height() / 3;
            }
            else if (depRect.bottom < rect.bottom)
            {
                // dependency to the top-left
                y = rect.top + rect.Height() / 2;
                depX = depRect.left + depRect.Width() / 2;
                depY = depRect.bottom;
            }
            else
            {
                // dependency to the bottom-left
                y = rect.top + rect.Height() / 2;
                depY = depRect.top + depRect.Height() / 3;
            }
        }
        else if (depRect.left > rect.right)
        {
            // dependency to the right
            x = rect.right;
            depX = depRect.left;

            if (depRect.bottom == rect.bottom)
            {
                y = rect.top + rect.Height() / 3;
                depY = depRect.top + depRect.Height() / 3;
            }
            else if (depRect.bottom < rect.bottom)
            {
                // dependency to the top-right
                y = rect.top + rect.Height() / 3;
                depX = depRect.left + depRect.Width() / 2;
                depY = depRect.bottom;
            }
            else
            {
                // dependency to the bottom-right
                y = rect.bottom;
                depY = depRect.top + depRect.Height() / 2;
            }
        }
        else if (depRect.top < rect.top)
        {
            // dependency is above
            x = rect.left + rect.Width() / 2;
            depX = depRect.left + depRect.Width() / 2;
            y = rect.top;
            depY = depRect.bottom;
        }
        else
        {
            // dependency is below
            x = rect.left + rect.Width() / 2;
            depX = depRect.left + depRect.Width() / 2;
            y = rect.bottom;
            depY = depRect.top;
        }

        graphics.DrawLine(&pen, depX, depY, x, y);
    }

    void ScaleImage(CDC* pDC, CBitmap& image, const CRect& rect)
    {
        BITMAP bmp;
        image.GetBitmap(&bmp);

        // select the source CBitmap in a memory DC;
        CDC memSrcDc;
        memSrcDc.CreateCompatibleDC(pDC);
        memSrcDc.SelectObject(&image); //now bitmap is an instance of CBitmap class

        // Create your new CBitmap with the new desired size and select it into a destination memory DC
        CDC memDestDC;
        CBitmap image2;
        memDestDC.CreateCompatibleDC(pDC);
        image2.CreateCompatibleBitmap(&memSrcDc, rect.Width(), rect.Height());
        memDestDC.SelectObject(&image2);

        // StretchBlt from src to dest
        memDestDC.StretchBlt(0, 0, rect.Width(), rect.Height(), &memSrcDc, 0, 0, bmp.bmWidth - 1, bmp.bmHeight - 1, SRCCOPY);

        HGDIOBJ hbitmap_detach = image.Detach();
        if (hbitmap_detach)
        {
            DeleteObject(hbitmap_detach);
        }

        image.Attach(image2.Detach());
    }

    void SetScaledButtonImage(CDC* pDC, CButton& button, CBitmap& image)
    {
        CRect buttonRect;
        button.GetClientRect(&buttonRect);
        ScaleImage(pDC, image, buttonRect);
        button.SetBitmap(image);
    }

    CSize CalcTextSize(CDC* pDC, CString& strText, CRect& rect, BOOL bCalcOnly)
    {
        CSize sizeText(0, 0);

        strText.Replace(_T("\t"), _T("    "));
        if (strText.Find(_T('\n')) >= 0) // Multi-line text
        {
            UINT nFormat = DT_CENTER | DT_NOPREFIX;
            if (bCalcOnly)
            {
                nFormat |= DT_CALCRECT;
            }

            int nHeight = pDC->DrawText(strText, rect, nFormat);
            rect.top += nHeight;
            rect.bottom += nHeight;
            sizeText = CSize(rect.Width(), nHeight);
        }
        else
        {
            if (bCalcOnly)
            {
                sizeText = pDC->GetTextExtent(strText);
            }
            else
            {
                UINT nFormat = DT_CENTER | DT_NOCLIP | DT_SINGLELINE;
                sizeText.cy = pDC->DrawText(strText, rect, nFormat);
                rect.top += sizeText.cy;
                rect.bottom += sizeText.cy;
                sizeText.cx = (LONG)rect.Width();
            }
        }

        return sizeText;
    }
}

//---------------------------------------------------------------------------
// CD2ItemToolTipCtrl
class CD2SkillToolTipCtrl : public CMFCToolTipCtrl
{
    DECLARE_DYNCREATE(CD2SkillToolTipCtrl)

    // Construction
public:
    CD2SkillToolTipCtrl(CMFCToolTipInfo* pParams = NULL) : CMFCToolTipCtrl(pParams)
    {
    }

    void SetCallback(const CD2SkillToolTipCtrlCallback* callback = nullptr)
    {
        Callback = callback;
    }
    // Overrides
public:
    CSize OnDrawLabel(CDC* pDC, CRect rect, BOOL bCalcOnly) override
    {
        if (CurrSkillId == nullptr)
        {
            return __super::OnDrawLabel(pDC, rect, bCalcOnly);
        }

        // color codes as described in the statdesc file
        static const COLORREF colors[] = { RGB(255,255,255), RGB(255, 0, 0), RGB(0,255,0), RGB(94,94,255), RGB(148,128,100), RGB(117, 117, 117), RGB(255,255,255), RGB(255,255,255), RGB(255,128,0), RGB(255,255,0) };
        enum { WHITE = 0, RED = 1, GREEN = 2, BLUE = 3, GOLD = 4, GRAY = 5, ORANGE = 8, YELLOW = 9 };

        const auto& skillInfo = d2ce::CharClassHelper::getSkillById(*CurrSkillId);

        // Get color of top text
        COLORREF color = colors[GREEN];
        pDC->SetTextColor(color);

        std::u16string uText = utf8::utf8to16(skillInfo.name);
        CString strText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        CSize sizeText(CalcTextSize(pDC, strText, rect, bCalcOnly));
        auto skipLineHeight = sizeText.cy;

        // Get color of description
        auto colorDesc = (Enabled && (BasePoints > 0)) ? WHITE : RED;
        color = colors[colorDesc];
        pDC->SetTextColor(color);

        // long description
        uText = utf8::utf8to16(skillInfo.longName);
        strText = reinterpret_cast<LPCWSTR>(uText.c_str());
        if (!strText.IsEmpty())
        {
            CSize prevSizeText = sizeText;
            sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
            sizeText.cy += prevSizeText.cy;
            sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
        }

        std::string strValue;
        if (skillInfo.reqLevel > 0)
        {
            d2ce::LocalizationHelpers::GetStringTxtValue("skilldesc3", strValue, "Required Level: %d");
            uText = utf8::utf8to16(d2ce::LocalizationHelpers::string_format(strValue, skillInfo.reqLevel));
            strText = reinterpret_cast<LPCWSTR>(uText.c_str());
            if (!strText.IsEmpty())
            {
                CSize prevSizeText = sizeText;
                sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
                sizeText.cy += prevSizeText.cy;
                sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
            }
        }

        if (BasePoints > 0)
        {
            auto value = std::min(std::uint16_t(BonusPoints + BasePoints), std::uint16_t(MAXUINT8));
            if (value != BasePoints)
            {
                d2ce::LocalizationHelpers::GetStringTxtValue("TooltipSkillLevelBonus", strValue, "Current Skill Level: %d (Base: %d)");
                uText = utf8::utf8to16(d2ce::LocalizationHelpers::string_format(strValue, value, std::uint16_t(BasePoints)));
            }
            else
            {
                d2ce::LocalizationHelpers::GetStringTxtValue("StrSkill2", strValue, "Current Skill Level: %d");
                uText = utf8::utf8to16(d2ce::LocalizationHelpers::string_format(strValue, value));
            }

            strText = reinterpret_cast<LPCWSTR>(uText.c_str());
            if (!strText.IsEmpty())
            {
                // skip line
                rect.top += skipLineHeight;
                rect.bottom += skipLineHeight;
                sizeText.cy += skipLineHeight;

                CSize prevSizeText = sizeText;
                sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
                sizeText.cy += prevSizeText.cy;
                sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
            }

            /*if (BasePoints < d2ce::MAX_SKILL_VALUE)
            {
                d2ce::LocalizationHelpers::GetStringTxtValue("StrSkill1", strValue, "Next Level");
                uText = utf8::utf8to16(strValue);
                strText = reinterpret_cast<LPCWSTR>(uText.c_str());
                if (!strText.IsEmpty())
                {
                    // skip line
                    rect.top += skipLineHeight;
                    rect.bottom += skipLineHeight;
                    sizeText.cy += skipLineHeight;

                    CSize prevSizeText = sizeText;
                    sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
                    sizeText.cy += prevSizeText.cy;
                    sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
                }
            }*/
        }
        else
        {
            /*d2ce::LocalizationHelpers::GetStringTxtValue("StrSkill17", strValue, "First Level");
            uText = utf8::utf8to16(strValue);
            strText = reinterpret_cast<LPCWSTR>(uText.c_str());
            if (!strText.IsEmpty())
            {
                // skip line
                rect.top += skipLineHeight;
                rect.bottom += skipLineHeight;
                sizeText.cy += skipLineHeight;

                CSize prevSizeText = sizeText;
                sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
                sizeText.cy += prevSizeText.cy;
                sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
            }*/
        }

        if (BasePoints < d2ce::MAX_SKILL_VALUE)
        {
            // get next level stats
        }

        return sizeText;
    }

    void OnFillBackground(CDC* pDC, CRect rect, COLORREF& clrText, COLORREF& clrLine) override
    {
        if (CurrSkillId == nullptr)
        {
            return __super::OnFillBackground(pDC, rect, clrText, clrLine);
        }

        CBrush br(RGB(0, 0, 0));
        pDC->FillRect(rect, &br);
    }

    // Implementation
public:
    virtual ~CD2SkillToolTipCtrl()
    {
    }

protected:
    afx_msg void OnShow(NMHDR* pNMHDR, LRESULT* pResult)
    {
        CurrUID = UINT(CWnd::FromHandle((HWND)pNMHDR->idFrom)->GetDlgCtrlID());
        CurrSkillId = nullptr;
        BasePoints = 0;
        BonusPoints = 0;
        Enabled = false;
        if (Callback != nullptr)
        {
            CPoint point;
            ::GetCursorPos(&point);
            CurrSkillId = Callback->InvHitTest(CurrUID, point, BasePoints, BonusPoints, Enabled);
        }

        __super::OnShow(pNMHDR, pResult);
    }
    DECLARE_MESSAGE_MAP()

protected:
    UINT CurrUID = 0;
    const  std::uint16_t* CurrSkillId = nullptr;
    std::uint8_t BasePoints = 0;
    std::uint16_t BonusPoints = 0;
    bool Enabled = false;
    const CD2SkillToolTipCtrlCallback* Callback = nullptr;
};

IMPLEMENT_DYNAMIC(CD2SkillToolTipCtrl, CMFCToolTipCtrl)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2SkillToolTipCtrl, CMFCToolTipCtrl)
    ON_NOTIFY_REFLECT(TTN_SHOW, &CD2SkillToolTipCtrl::OnShow)
END_MESSAGE_MAP()

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
    MainForm.getSkillBonusPoints(BonusSkillPoints);
    Level = std::uint16_t(MainForm.getCharacterLevel());

    {
        const auto& tab1Skills = d2ce::CharClassHelper::getSklTreeTab(0ui16, Class);
        for (const auto& skillRows : tab1Skills)
        {
            if ((skillRows.first == 0) || (skillRows.first > 6))
            {
                // should not happend
                continue;
            }

            UINT id = IDC_BUTTON_TREE_1_ROW_1_COL_1 + 3 * (skillRows.first - 1);
            UINT idEdit = IDC_EDIT_TREE_1_ROW_1_COL_1 + 3 * (skillRows.first - 1);
            for (const auto& skillCols : skillRows.second)
            {
                if ((skillCols.first == 0) || (skillCols.first > 3))
                {
                    // should not happend
                    continue;
                }

                const auto& skillInfo = d2ce::CharClassHelper::getSkillById(skillCols.second);
                if (!skillInfo.classInfo.has_value())
                {
                    // should not happend
                    continue;
                }

                auto& tabSkill = Tab1SkillMap[id + (skillCols.first - 1)];
                tabSkill.skillId = skillCols.second;
                tabSkill.buttonId = id + (skillCols.first - 1);
                tabSkill.editId = idEdit + (skillCols.first - 1);
                tabSkill.skillIdx = skillInfo.classInfo.value().index;
                SkillBnMap[tabSkill.editId] = tabSkill.buttonId;
                SkillIdxBnMap[tabSkill.skillIdx] = tabSkill.buttonId;
                if (skillInfo.reqLevel > Level)
                {
                    tabSkill.levelReqMet = false;
                }

                for (const auto& reqSkill : skillInfo.reqSkills)
                {
                    const auto& reqSkillInfo = d2ce::CharClassHelper::getSkillByIndex(reqSkill);
                    if (!skillInfo.classInfo.has_value())
                    {
                        // should not happend
                        continue;
                    }

                    tabSkill.reqSkills.push_back(reqSkillInfo.classInfo.value().index);
                }
            }
        }
    }
    
    {
        const auto& tab2Skills = d2ce::CharClassHelper::getSklTreeTab(1ui16, Class);
        for (const auto& skillRows : tab2Skills)
        {
            if ((skillRows.first == 0) || (skillRows.first > 6))
            {
                // should not happend
                continue;
            }

            UINT id = IDC_BUTTON_TREE_2_ROW_1_COL_1 + 3 * (skillRows.first - 1);
            UINT idEdit = IDC_EDIT_TREE_2_ROW_1_COL_1 + 3 * (skillRows.first - 1);
            for (const auto& skillCols : skillRows.second)
            {
                if ((skillCols.first == 0) || (skillCols.first > 3))
                {
                    // should not happend
                    continue;
                }

                const auto& skillInfo = d2ce::CharClassHelper::getSkillById(skillCols.second);
                if (!skillInfo.classInfo.has_value())
                {
                    // should not happend
                    continue;
                }

                auto& tabSkill = Tab2SkillMap[id + (skillCols.first - 1)];
                tabSkill.skillId = skillCols.second;
                tabSkill.buttonId = id + (skillCols.first - 1);
                tabSkill.editId = idEdit + (skillCols.first - 1);
                tabSkill.skillIdx = skillInfo.classInfo.value().index;
                SkillBnMap[tabSkill.editId] = tabSkill.buttonId;
                SkillIdxBnMap[tabSkill.skillIdx] = tabSkill.buttonId;
                if (skillInfo.reqLevel > Level)
                {
                    tabSkill.levelReqMet = false;
                }

                for (const auto& reqSkill : skillInfo.reqSkills)
                {
                    const auto& reqSkillInfo = d2ce::CharClassHelper::getSkillByIndex(reqSkill);
                    if (!skillInfo.classInfo.has_value())
                    {
                        // should not happend
                        continue;
                    }

                    tabSkill.reqSkills.push_back(reqSkillInfo.classInfo.value().index);
                }
            }
        }
    }

    {
        const auto& tab3Skills = d2ce::CharClassHelper::getSklTreeTab(2ui16, Class);
        for (const auto& skillRows : tab3Skills)
        {
            if ((skillRows.first == 0) || (skillRows.first > 6))
            {
                // should not happend
                continue;
            }

            UINT id = IDC_BUTTON_TREE_3_ROW_1_COL_1 + 3 * (skillRows.first - 1);
            UINT idEdit = IDC_EDIT_TREE_3_ROW_1_COL_1 + 3 * (skillRows.first - 1);
            for (const auto& skillCols : skillRows.second)
            {
                if ((skillCols.first == 0) || (skillCols.first > 3))
                {
                    // should not happend
                    continue;
                }

                const auto& skillInfo = d2ce::CharClassHelper::getSkillById(skillCols.second);
                if (!skillInfo.classInfo.has_value())
                {
                    // should not happend
                    continue;
                }

                auto& tabSkill = Tab3SkillMap[id + (skillCols.first - 1)];
                tabSkill.skillId = skillCols.second;
                tabSkill.buttonId = id + (skillCols.first - 1);
                tabSkill.editId = idEdit + (skillCols.first - 1);
                tabSkill.skillIdx = skillInfo.classInfo.value().index;
                SkillBnMap[tabSkill.editId] = tabSkill.buttonId;
                SkillIdxBnMap[tabSkill.skillIdx] = tabSkill.buttonId;
                if (skillInfo.reqLevel > Level)
                {
                    tabSkill.levelReqMet = false;
                }

                for (const auto& reqSkill : skillInfo.reqSkills)
                {
                    const auto& reqSkillInfo = d2ce::CharClassHelper::getSkillByIndex(reqSkill);
                    if (!skillInfo.classInfo.has_value())
                    {
                        // should not happend
                        continue;
                    }

                    tabSkill.reqSkills.push_back(reqSkillInfo.classInfo.value().index);
                }
            }
        }
    }

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
const std::uint16_t* CD2SkillTreeForm::InvHitTest(CPoint point, TOOLINFO* pTI) const
{
    INT_PTR nHit = __super::OnToolHitTest(point, pTI);
    if (nHit == -1)
    {
        return nullptr;
    }

    // Make sure we have hit an item
    ClientToScreen(&point);
    std::uint8_t base = 0;
    std::uint16_t bonus = 0;
    bool enabled = false;
    return InvHitTest((UINT)nHit, point, base, bonus, enabled, pTI);
}
//---------------------------------------------------------------------------
INT_PTR CD2SkillTreeForm::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);

    TOOLINFO* pTi = (pTI == nullptr) ? &ti : pTI;
    InvHitTest(point, pTi);

    UINT_PTR nHit = pTi->uId;
    if (pTi->uFlags & TTF_IDISHWND)
    {
        nHit = UINT_PTR(::GetDlgCtrlID(HWND(pTi->uId)));
    }
    else if(nHit != 0)
    {
        nHit = pTi->uId;
    }

    return (INT_PTR)nHit;
}
//---------------------------------------------------------------------------
BOOL CD2SkillTreeForm::PreTranslateMessage(MSG* pMsg)
{
    CWnd* pWndFocus = GetFocus();
    if (pWndFocus != NULL && IsChild(pWndFocus))
    {
        UINT message = pMsg->message;
        if ((message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE ||
            message == WM_LBUTTONUP || message == WM_RBUTTONUP ||
            message == WM_MBUTTONUP) &&
            (GetKeyState(VK_LBUTTON) >= 0 && GetKeyState(VK_RBUTTON) >= 0 &&
                GetKeyState(VK_MBUTTON) >= 0))
        {
            CheckToolTipCtrl();
        }

        if (pMsg->message == WM_KEYDOWN)
        {
            if (pMsg->wParam == VK_RETURN)
            {
                TCHAR szClass[10];
                if (GetClassName(pWndFocus->m_hWnd, szClass, 10) &&
                    (lstrcmpi(szClass, _T("EDIT")) == 0))
                {
                    // pressing the ENTER key will take the focus to the next control
                    pMsg->wParam = VK_TAB;
                }
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
    CStringW wValue(ToText(Sender));
    return utf8::utf16to8(reinterpret_cast<const char16_t*>(wValue.GetString()));
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
    ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_TREE_1_ROW_1_COL_1, IDC_EDIT_TREE_1_ROW_6_COL_3, &CD2SkillTreeForm::OnTab1SkillKillFocus)
    ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_TREE_2_ROW_1_COL_1, IDC_EDIT_TREE_2_ROW_6_COL_3, &CD2SkillTreeForm::OnTab2SkillKillFocus)
    ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_EDIT_TREE_3_ROW_1_COL_1, IDC_EDIT_TREE_3_ROW_6_COL_3, &CD2SkillTreeForm::OnTab3SkillKillFocus)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_TREE_1_ROW_1_COL_1, IDC_BUTTON_TREE_1_ROW_6_COL_3, &CD2SkillTreeForm::OnTab1SkillBnClicked)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_TREE_2_ROW_1_COL_1, IDC_BUTTON_TREE_2_ROW_6_COL_3, &CD2SkillTreeForm::OnTab2SkillBnClicked)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_TREE_3_ROW_1_COL_1, IDC_BUTTON_TREE_3_ROW_6_COL_3, &CD2SkillTreeForm::OnTab3SkillBnClicked)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CD2SkillTreeForm::OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CD2SkillTreeForm::OnToolTipText)
    ON_BN_CLICKED(IDOK, &CD2SkillTreeForm::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CD2SkillTreeForm::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_SET_ALL_SKILLS, &CD2SkillTreeForm::OnBnClickedSetAll)
    ON_WM_PAINT()
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2SkillTreeForm message handlers

//---------------------------------------------------------------------------
BOOL CD2SkillTreeForm::OnInitDialog()
{
    __super::OnInitDialog();

    EnableToolTips(TRUE);
    CheckToolTipCtrl();

    std::string strValue;
    d2ce::LocalizationHelpers::GetStringTxtValue("minipaneltree", strValue, "Skill Tree");
    auto uText = utf8::utf8to16(strValue);
    OrigCaption = reinterpret_cast<LPCWSTR>(uText.c_str());
    if (OrigCaption.IsEmpty())
    {
        GetWindowText(OrigCaption);
    }
    UpdateCaption();

    // Fill out tab names.
    for (UINT i = 0, nIDC = IDC_STATIC_TREE_1; i < d2ce::NUM_OF_SKILL_TABS; ++i, ++nIDC)
    {
        uText = utf8::utf8to16(d2ce::CharClassHelper::getSkillTabName(std::uint16_t(i), Class));
        GetDlgItem(nIDC)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

    CString text;
    CStringA textA;
    CWnd* pWnd = nullptr;
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

    CDC* pDC = GetDC();
    for (auto& skill : Tab1SkillMap)
    {
        BOOL bEnable = TRUE;
        if (!skill.second.reqSkills.empty())
        {
            bEnable = FALSE;
            for (const auto& reqSkillIdx : skill.second.reqSkills)
            {
                if (GetSkillPoints(reqSkillIdx, true) != 0)
                {
                    bEnable = TRUE;
                    break;
                }
            }
        }

        if (bEnable && !skill.second.levelReqMet)
        {
            bEnable = FALSE;
        }

        CButton* pButton = ((CButton*)GetDlgItem(skill.second.buttonId));
        pButton->EnableWindow(bEnable);
        pButton->ShowWindow(SW_SHOW);

        const auto& skillInfo = d2ce::CharClassHelper::getSkillById(skill.second.skillId);
        MainForm.getSkillBitmap(skillInfo, skill.second.bitmap);
        SetScaledButtonImage(pDC, *pButton, skill.second.bitmap);

        CEdit* pEdit = ((CEdit*)GetDlgItem(skill.second.editId));
        pEdit->EnableWindow(bEnable);
        pEdit->ShowWindow(SW_SHOW);
        pEdit->LimitText(2);
        SetInt(skill.second.editId, GetSkillPoints(skill.second.skillIdx));
    }

    for (auto& skill : Tab2SkillMap)
    {
        BOOL bEnable = TRUE;
        if (!skill.second.reqSkills.empty())
        {
            bEnable = FALSE;
            for (const auto& reqSkillIdx : skill.second.reqSkills)
            {
                if (GetSkillPoints(reqSkillIdx, true) != 0)
                {
                    bEnable = TRUE;
                    break;
                }
            }
        }

        if (bEnable && !skill.second.levelReqMet)
        {
            bEnable = FALSE;
        }

        CButton* pButton = ((CButton*)GetDlgItem(skill.second.buttonId));
        pButton->EnableWindow(bEnable);
        pButton->ShowWindow(SW_SHOW);

        const auto& skillInfo = d2ce::CharClassHelper::getSkillById(skill.second.skillId);
        MainForm.getSkillBitmap(skillInfo, skill.second.bitmap);
        SetScaledButtonImage(pDC, *pButton, skill.second.bitmap);

        CEdit* pEdit = ((CEdit*)GetDlgItem(skill.second.editId));
        pEdit->EnableWindow(bEnable);
        pEdit->ShowWindow(SW_SHOW);
        pEdit->LimitText(2);
        SetInt(skill.second.editId, GetSkillPoints(skill.second.skillIdx));
    }

    for (auto& skill : Tab3SkillMap)
    {
        BOOL bEnable = TRUE;
        if (!skill.second.reqSkills.empty())
        {
            bEnable = FALSE;
            for (const auto& reqSkillIdx : skill.second.reqSkills)
            {
                if (GetSkillPoints(reqSkillIdx, true) != 0)
                {
                    bEnable = TRUE;
                    break;
                }
            }
        }

        if (bEnable && !skill.second.levelReqMet)
        {
            bEnable = FALSE;
        }

        CButton* pButton = ((CButton*)GetDlgItem(skill.second.buttonId));
        pButton->EnableWindow(bEnable);
        pButton->ShowWindow(SW_SHOW);

        const auto& skillInfo = d2ce::CharClassHelper::getSkillById(skill.second.skillId);
        MainForm.getSkillBitmap(skillInfo, skill.second.bitmap);
        SetScaledButtonImage(pDC, *pButton, skill.second.bitmap);

        CEdit* pEdit = ((CEdit*)GetDlgItem(skill.second.editId));
        pEdit->EnableWindow(bEnable);
        pEdit->ShowWindow(SW_SHOW);
        pEdit->LimitText(2);
        SetInt(skill.second.editId, GetSkillPoints(skill.second.skillIdx));
    }

    ((CEdit*)GetDlgItem(IDC_EDIT_SET_ALL_SKILLS))->LimitText(2);
    SetInt(IDC_EDIT_SET_ALL_SKILLS, 0);
    //InvalidateRect(NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnTab1SkillKillFocus(UINT nID)
{
    auto iterBn = SkillBnMap.find(nID);
    if (iterBn == SkillBnMap.end())
    {
        return;
    }

    auto iter = Tab1SkillMap.find(iterBn->second);
    if (iter == Tab1SkillMap.end())
    {
        return;
    }

    const auto& skillInfo = iter->second;
    auto pos = skillInfo.skillIdx;
    std::uint8_t skillValue = (std::uint8_t)ToInt(nID);
    if (skillValue > d2ce::MAX_SKILL_VALUE)
    {
        skillValue = d2ce::MAX_SKILL_VALUE;
        SetInt(nID, skillValue);
    }

    // value actually changed?
    auto oldValue = GetSkillPoints(pos);
    if (skillValue != oldValue)
    {
        SetSkillPoints(pos, skillValue);
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
        CheckTab1Skills();
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnTab2SkillKillFocus(UINT nID)
{
    auto iterBn = SkillBnMap.find(nID);
    if (iterBn == SkillBnMap.end())
    {
        return;
    }

    auto iter = Tab2SkillMap.find(iterBn->second);
    if (iter == Tab2SkillMap.end())
    {
        return;
    }

    const auto& skillInfo = iter->second;
    auto pos = skillInfo.skillIdx;
    std::uint8_t skillValue = (std::uint8_t)ToInt(nID);
    if (skillValue > d2ce::MAX_SKILL_VALUE)
    {
        skillValue = d2ce::MAX_SKILL_VALUE;
        SetInt(nID, skillValue);
    }

    // value actually changed?
    auto oldValue = GetSkillPoints(pos);
    if (skillValue != oldValue)
    {
        SetSkillPoints(pos, skillValue);
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
        CheckTab2Skills();
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnTab3SkillKillFocus(UINT nID)
{
    auto iterBn = SkillBnMap.find(nID);
    if (iterBn == SkillBnMap.end())
    {
        return;
    }

    auto iter = Tab3SkillMap.find(iterBn->second);
    if (iter == Tab3SkillMap.end())
    {
        return;
    }

    const auto& skillInfo = iter->second;
    auto pos = skillInfo.skillIdx;
    std::uint8_t skillValue = (std::uint8_t)ToInt(nID);
    if (skillValue > d2ce::MAX_SKILL_VALUE)
    {
        skillValue = d2ce::MAX_SKILL_VALUE;
        SetInt(nID, skillValue);
    }

    // value actually changed?
    auto oldValue = GetSkillPoints(pos);
    if (skillValue != oldValue)
    {
        SetSkillPoints(pos, skillValue);
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
        CheckTab3Skills();
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnTab1SkillBnClicked(UINT nID)
{
    auto iter = Tab1SkillMap.find(nID);
    if (iter == Tab1SkillMap.end())
    {
        return;
    }

    const auto& skillInfo = iter->second;
    std::uint8_t skillValue = (std::uint8_t)ToInt(skillInfo.editId);
    if (skillValue >= d2ce::MAX_SKILL_VALUE)
    {
        return;
    }

    ++skillValue;
    SetInt(skillInfo.editId, skillValue);
    OnTab1SkillKillFocus(skillInfo.editId);
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnTab2SkillBnClicked(UINT nID)
{
    auto iter = Tab2SkillMap.find(nID);
    if (iter == Tab2SkillMap.end())
    {
        return;
    }

    const auto& skillInfo = iter->second;
    std::uint8_t skillValue = (std::uint8_t)ToInt(skillInfo.editId);
    if (skillValue >= d2ce::MAX_SKILL_VALUE)
    {
        return;
    }

    ++skillValue;
    SetInt(skillInfo.editId, skillValue);
    OnTab2SkillKillFocus(skillInfo.editId);
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnTab3SkillBnClicked(UINT nID)
{
    auto iter = Tab3SkillMap.find(nID);
    if (iter == Tab3SkillMap.end())
    {
        return;
    }

    const auto& skillInfo = iter->second;
    std::uint8_t skillValue = (std::uint8_t)ToInt(skillInfo.editId);
    if (skillValue >= d2ce::MAX_SKILL_VALUE)
    {
        return;
    }

    ++skillValue;
    SetInt(skillInfo.editId, skillValue);
    OnTab3SkillKillFocus(skillInfo.editId);
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::SaveSkills()
{
    if (SkillsChanged)
    {
        MainForm.updateSkills(Skills, SkillChoices);
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
std::uint16_t CD2SkillTreeForm::GetBonusSkillPoints(std::uint16_t idx) const
{
    if (idx < BonusSkillPoints.size())
    {
        return BonusSkillPoints[idx];
    }

    return 0;
}
//---------------------------------------------------------------------------
std::uint8_t CD2SkillTreeForm::GetSkillPoints(std::uint16_t idx, bool addBonusPts) const
{
    std::uint8_t points = 0;
    if (idx < d2ce::NUM_OF_SKILLS)
    {
        points += Skills[idx];
    }

    if (addBonusPts)
    {
        points = std::uint8_t(std::min(std::uint16_t(GetBonusSkillPoints(idx) + points), std::uint16_t(MAXUINT8)));
    }

    return points;
}
//---------------------------------------------------------------------------
bool CD2SkillTreeForm::SetSkillPoints(std::uint16_t idx, std::uint8_t value)
{
    if (idx < d2ce::NUM_OF_SKILLS)
    {
        Skills[idx] = value;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
const std::uint16_t* CD2SkillTreeForm::InvHitTest(UINT id, CPoint point, std::uint8_t& base, std::uint16_t& bonus, bool& enabled, TOOLINFO* pTI) const
{
    base = 0;
    bonus = 0;
    auto* pWnd = GetDlgItem(id);
    if ((pWnd == nullptr) || !::IsWindow(pWnd->GetSafeHwnd()))
    {
        enabled = false;
        return nullptr;
    }

    enabled = pWnd->IsWindowEnabled() ? true : false;

    ScreenToClient(&point);

    auto iter = Tab1SkillMap.find(id);
    if (iter != Tab1SkillMap.end())
    {
        CRect buttonRect;
        pWnd->GetWindowRect(buttonRect);
        ScreenToClient(&buttonRect);
        if (!PtInRect(&buttonRect, point))
        {
            return nullptr;
        }

        if (pTI != nullptr)
        {
            pTI->hwnd = GetSafeHwnd();
            pTI->uId = (UINT_PTR)pWnd->GetSafeHwnd();
            pTI->lpszText = LPSTR_TEXTCALLBACK;
            pTI->uFlags |= TTF_IDISHWND;
        }

        base = GetSkillPoints(iter->second.skillIdx);
        bonus = GetBonusSkillPoints(iter->second.skillIdx);
        return &iter->second.skillId;
    }
    
    iter = Tab2SkillMap.find(id);
    if (iter != Tab2SkillMap.end())
    {
        CRect buttonRect;
        pWnd->GetWindowRect(buttonRect);
        ScreenToClient(&buttonRect);
        if (!PtInRect(&buttonRect, point))
        {
            return nullptr;
        }

        if (pTI != nullptr)
        {
            pTI->hwnd = GetSafeHwnd();
            pTI->uId = (UINT_PTR)pWnd->GetSafeHwnd();
            pTI->lpszText = LPSTR_TEXTCALLBACK;
            pTI->uFlags |= TTF_IDISHWND;
        }

        base = GetSkillPoints(iter->second.skillIdx);
        bonus = GetBonusSkillPoints(iter->second.skillIdx);
        return &iter->second.skillId;
    }

    iter = Tab3SkillMap.find(id);
    if (iter != Tab3SkillMap.end())
    {
        CRect buttonRect;
        pWnd->GetWindowRect(buttonRect);
        ScreenToClient(&buttonRect);
        if (!PtInRect(&buttonRect, point))
        {
            return nullptr;
        }

        if (pTI != nullptr)
        {
            pTI->hwnd = GetSafeHwnd();
            pTI->uId = (UINT_PTR)pWnd->GetSafeHwnd();
            pTI->lpszText = LPSTR_TEXTCALLBACK;
            pTI->uFlags |= TTF_IDISHWND;
        }

        base = GetSkillPoints(iter->second.skillIdx);
        bonus = GetBonusSkillPoints(iter->second.skillIdx);
        return &iter->second.skillId;
    }

    return nullptr;
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
    SkillsUsed = 0;
    for (auto& skill : Tab1SkillMap)
    {
        if (!skill.second.levelReqMet)
        {
            // skip
            SkillsUsed += GetSkillPoints(skill.second.skillIdx);
            continue;
        }

        SetSkillPoints(skill.second.skillIdx, skillValue);
        SetInt(skill.second.editId, skillValue);
        SkillsUsed += skillValue;
    }
    CheckTab1Skills();

    for (auto& skill : Tab2SkillMap)
    {
        if (!skill.second.levelReqMet)
        {
            // skip
            SkillsUsed += GetSkillPoints(skill.second.skillIdx);
            continue;
        }

        SetSkillPoints(skill.second.skillIdx, skillValue);
        SetInt(skill.second.editId, skillValue);
        SkillsUsed += skillValue;
    }
    CheckTab2Skills();

    for (auto& skill : Tab3SkillMap)
    {
        if (!skill.second.levelReqMet)
        {
            // skip
            SkillsUsed += GetSkillPoints(skill.second.skillIdx);
            continue;
        }

        SetSkillPoints(skill.second.skillIdx, skillValue);
        SetInt(skill.second.editId, skillValue);
        SkillsUsed += skillValue;
    }
    CheckTab3Skills();

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
BOOL CD2SkillTreeForm::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    CString strTipText;
    UINT_PTR nID = pNMHDR->idFrom;

    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
        pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
    {
        nID = ((UINT_PTR)(DWORD)::GetDlgCtrlID((HWND)nID));
    }

    if ((nID != 0) && (nID != ID_VIEW_STATUS_BAR))
    {
        for (const auto& uid : SkillBnMap)
        {
            if (uid.second == nID)
            {
                strTipText = _T("N/A");
                break;
            }
        }
    }
#ifndef _UNICODE
    if (pNMHDR->code == TTN_NEEDTEXTA)
        lstrcpyn(pTTTA->szText, strTipText, (sizeof(pTTTA->szText) / sizeof(pTTTA->szText[0])));
    else
        _mbstowcsz(pTTTW->szText, strTipText, (sizeof(pTTTW->szText) / sizeof(pTTTW->szText[0])));
#else
    if (pNMHDR->code == TTN_NEEDTEXTA)
        _wcstombsz(pTTTA->szText, strTipText, (sizeof(pTTTA->szText) / sizeof(pTTTA->szText[0])));
    else
        lstrcpyn(pTTTW->szText, strTipText, (sizeof(pTTTW->szText) / sizeof(pTTTW->szText[0])));
#endif
    * pResult = 0;

    ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

    return TRUE;
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::OnPaint()
{
    if (IsIconic())
    {
        __super::OnPaint();
        return;
    }

    CPaintDC dc(this); // device context for painting
    auto color = dc.GetTextColor();
    Gdiplus::Graphics graphics(dc.GetSafeHdc());
    Gdiplus::Pen pen(Gdiplus::Color(GetRValue(color), GetGValue(color), GetBValue(color)), 3.0);
    pen.SetAlignment(Gdiplus::PenAlignmentCenter);
    pen.SetEndCap(Gdiplus::LineCapArrowAnchor);

    CWnd* pWnd = nullptr;
    CRect rect;
    CRect depRect;
    for (auto& skill : Tab1SkillMap)
    {
        pWnd = GetDlgItem(skill.second.buttonId);
        if (pWnd == nullptr || !::IsWindow(pWnd->GetSafeHwnd()))
        {
            continue;
        }
        pWnd->GetWindowRect(rect);
        ScreenToClient(rect);

        for (const auto& reqSkillIdx : skill.second.reqSkills)
        {
            pWnd = GetDlgItem(SkillIdxBnMap[reqSkillIdx]);
            if (pWnd == nullptr || !::IsWindow(pWnd->GetSafeHwnd()))
            {
                continue;
            }
            pWnd->GetWindowRect(depRect);
            ScreenToClient(depRect);
            DrawDependencyLine(graphics, pen, rect, depRect);
        }
    }

    for (auto& skill : Tab2SkillMap)
    {
        pWnd = GetDlgItem(skill.second.buttonId);
        if (pWnd == nullptr || !::IsWindow(pWnd->GetSafeHwnd()))
        {
            continue;
        }
        pWnd->GetWindowRect(rect);
        ScreenToClient(rect);

        for (const auto& reqSkillIdx : skill.second.reqSkills)
        {
            pWnd = GetDlgItem(SkillIdxBnMap[reqSkillIdx]);
            if (pWnd == nullptr || !::IsWindow(pWnd->GetSafeHwnd()))
            {
                continue;
            }
            pWnd->GetWindowRect(depRect);
            ScreenToClient(depRect);
            DrawDependencyLine(graphics, pen, rect, depRect);
        }
    }

    for (auto& skill : Tab3SkillMap)
    {
        pWnd = GetDlgItem(skill.second.buttonId);
        if (pWnd == nullptr || !::IsWindow(pWnd->GetSafeHwnd()))
        {
            continue;
        }
        pWnd->GetWindowRect(rect);
        ScreenToClient(rect);

        for (const auto& reqSkillIdx : skill.second.reqSkills)
        {
            pWnd = GetDlgItem(SkillIdxBnMap[reqSkillIdx]);
            if (pWnd == nullptr || !::IsWindow(pWnd->GetSafeHwnd()))
            {
                continue;
            }
            pWnd->GetWindowRect(depRect);
            ScreenToClient(depRect);
            DrawDependencyLine(graphics, pen, rect, depRect);
        }
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::CheckTab1Skills()
{
    for (auto& skill : Tab1SkillMap)
    {
        if (!skill.second.levelReqMet)
        {
            // skip
            continue;
        }

        if (skill.second.reqSkills.empty())
        {
            // no dependencies
            continue;
        }

        BOOL bEnable = FALSE;
        for (const auto& reqSkillIdx : skill.second.reqSkills)
        {
            if (GetSkillPoints(reqSkillIdx, true) != 0)
            {
                bEnable = TRUE;
                break;
            }
        }

        GetDlgItem(skill.second.buttonId)->EnableWindow(bEnable);
        GetDlgItem(skill.second.editId)->EnableWindow(bEnable);
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::CheckTab2Skills()
{
    for (auto& skill : Tab2SkillMap)
    {
        if (!skill.second.levelReqMet)
        {
            // skip
            continue;
        }

        if (skill.second.reqSkills.empty())
        {
            // no dependencies
            continue;
        }

        BOOL bEnable = FALSE;
        for (const auto& reqSkillIdx : skill.second.reqSkills)
        {
            if (GetSkillPoints(reqSkillIdx, true) != 0)
            {
                bEnable = TRUE;
                break;
            }
        }

        GetDlgItem(skill.second.buttonId)->EnableWindow(bEnable);
        GetDlgItem(skill.second.editId)->EnableWindow(bEnable);
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::CheckTab3Skills()
{
    for (auto& skill : Tab3SkillMap)
    {
        if (!skill.second.levelReqMet)
        {
            // skip
            continue;
        }

        if (skill.second.reqSkills.empty())
        {
            // no dependencies
            continue;
        }

        BOOL bEnable = FALSE;
        for (const auto& reqSkillIdx : skill.second.reqSkills)
        {
            if (GetSkillPoints(reqSkillIdx, true) != 0)
            {
                bEnable = TRUE;
                break;
            }
        }

        GetDlgItem(skill.second.buttonId)->EnableWindow(bEnable);
        GetDlgItem(skill.second.editId)->EnableWindow(bEnable);
    }
}
//---------------------------------------------------------------------------
void CD2SkillTreeForm::CheckToolTipCtrl()
{
    auto pToolTip = AfxGetModuleThreadState()->m_pToolTip;
    if (pToolTip != NULL && (pToolTip->GetOwner() != this || DYNAMIC_DOWNCAST(CD2SkillToolTipCtrl, pToolTip) == NULL))
    {
        pToolTip->DestroyWindow();
        delete pToolTip;
        AfxGetModuleThreadState()->m_pToolTip = NULL;
        pToolTip = NULL;
    }

    if (pToolTip == NULL)
    {
        CMFCToolTipInfo ttParams;
        ttParams.m_bVislManagerTheme = TRUE;
        auto pD2SkillToolTip = new CD2SkillToolTipCtrl(&ttParams);
        pToolTip = pD2SkillToolTip;
        if (pToolTip->Create(this, TTS_ALWAYSTIP))
        {
            pD2SkillToolTip->SetCallback(this);

            CRect rect;
            CWnd* pWnd = nullptr;
            for (const auto& uid : SkillBnMap)
            {
                pWnd = GetDlgItem(uid.second);
                if (pWnd == nullptr)
                {
                   continue;
                }

                pWnd->GetWindowRect(&rect);
                ScreenToClient(&rect);
                pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, rect, uid.second);
            }

            pToolTip->SetDelayTime(TTDT_AUTOPOP, 0x7FFF);
            pToolTip->SendMessage(TTM_ACTIVATE, FALSE);
            AfxGetModuleThreadState()->m_pToolTip = pToolTip;
        }
    }
}
//---------------------------------------------------------------------------
bool CD2SkillTreeForm::isSkillChoicesChanged() const
{
    return SkillsChanged;
}
//---------------------------------------------------------------------------
