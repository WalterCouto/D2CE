/*
    Diablo II Character Editor
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
#include "D2ItemToolTipCtrl.h"
#include "resource.h"		// main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    CSize CalcTextSize(CDC* pDC, CString& strText, CRect& rect, BOOL bCalcOnly)
    {
        CSize sizeText(0, 0);

        strText.Replace(_T("\t"), _T("    "));
        if (strText.Find(_T('\n')) >= 0) // Multi-line text
        {
            UINT nFormat = DT_CENTER | DT_NOPREFIX;;
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

IMPLEMENT_DYNAMIC(CD2ItemToolTipCtrl, CMFCToolTipCtrl)

//---------------------------------------------------------------------------
CD2ItemToolTipCtrl::CD2ItemToolTipCtrl(d2ce::Character& charInfo, CMFCToolTipInfo* pParams/* = NULL*/) : CMFCToolTipCtrl(pParams), CharInfo(charInfo)
{
}
//---------------------------------------------------------------------------
void CD2ItemToolTipCtrl::SetCallback(const CD2ItemToolTipCtrlCallback* callback)
{
    Callback = callback;
}
//---------------------------------------------------------------------------
CSize CD2ItemToolTipCtrl::OnDrawLabel(CDC* pDC, CRect rect, BOOL bCalcOnly)
{
    if (CurrItem == nullptr)
    {
        return __super::OnDrawLabel(pDC, rect, bCalcOnly);
    }

    static const COLORREF colors[] = { RGB(255,255,255), RGB(94,94,255), RGB(0,255,0), RGB(255,255,0), RGB(148,128,100), RGB(255,128,0), RGB(255, 0, 0), RGB(117, 117, 117) };
    enum { WHITE = 0, BLUE, GREEN, RARE, UNIQUE, CRAFT, RED, GRAY };

    // Get color of top text
    COLORREF color = CurrItem->isIdentified() ? (CurrItem->isEthereal() ? colors[GRAY] : colors[WHITE]) : colors[RED];
    switch (CurrItem->getQuality())
    {
    case d2ce::EnumItemQuality::MAGIC:
        color = colors[BLUE];
        break;

    case d2ce::EnumItemQuality::SET:
        color = colors[GREEN];
        break;

    case d2ce::EnumItemQuality::RARE:
        color = colors[RARE];
        break;

    case d2ce::EnumItemQuality::UNIQUE:
        color = colors[UNIQUE];
        break;

    case d2ce::EnumItemQuality::CRAFT:
    case d2ce::EnumItemQuality::TEMPERED:
        color = colors[CRAFT];
        break;
    }
    pDC->SetTextColor(color);

    CFont* pOldFont = (CFont*)pDC->SelectObject(&(GetGlobalData()->fontDefaultGUIBold));

    CString strText(CurrItem->getDisplayedItemName().c_str());
    CSize sizeText(CalcTextSize(pDC, strText, rect, bCalcOnly));

    // draw possible rune name
    strText = CurrItem->getDisplayedSocketedRunes().c_str();
    if (!strText.IsEmpty())
    {
        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[UNIQUE]);
        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        sizeText.cy += prevSizeText.cy;
        sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
    }

    // Other non-magical
    auto charLevel = IsMerc ? CharInfo.getMercenaryInfo().getLevel() : CharInfo.getLevel();
    strText = CurrItem->getDisplayedItemAttributes(CharInfo.getClass(), charLevel).c_str();
    if (!strText.IsEmpty())
    {
        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[WHITE]);
        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        sizeText.cy += prevSizeText.cy;
        sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
    }

    // magical props
    std::vector<d2ce::MagicalAttribute> magicalAttributes;
    if (CurrItem->getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
    {
        for (const auto& attrib : magicalAttributes)
        {
            if (!attrib.Visible)
            {
                continue;
            }

            strText = attrib.Desc.c_str();
            if (strText.IsEmpty())
            {
                continue;
            }

            CSize prevSizeText = sizeText;
            pDC->SetTextColor(colors[BLUE]);

            sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
            sizeText.cy += prevSizeText.cy;
            sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
        }
    }

    if (CurrItem->isEthereal())
    {
        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[BLUE]);
        strText = _T("Ethereal (Cannot be Repaired)");
        if (CurrItem->isSocketed())
        {
            // Socketed text
            CString temp;
            temp.Format(_T(", Socketed (%d)"), (int)CurrItem->totalNumberOfSockets());
            strText += temp;
        }
        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        sizeText.cy += prevSizeText.cy;
        sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
    }
    else if (CurrItem->isSocketed())
    {
        // Socketed text
        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[BLUE]);
        strText.Format(_T("Socketed (%d)"), (int)CurrItem->totalNumberOfSockets());
        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        sizeText.cy += prevSizeText.cy;
        sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
    }

    pDC->SelectObject(pOldFont);

    return sizeText;
}
//---------------------------------------------------------------------------
void CD2ItemToolTipCtrl::OnFillBackground(CDC* pDC, CRect rect, COLORREF& clrText, COLORREF& clrLine)
{
    if (CurrItem == nullptr)
    {
        return __super::OnFillBackground(pDC, rect, clrText, clrLine);
    }

    CBrush br(RGB(0, 0, 0));
    pDC->FillRect(rect, &br);
}
//---------------------------------------------------------------------------
CD2ItemToolTipCtrl::~CD2ItemToolTipCtrl()
{
}
//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2ItemToolTipCtrl, CMFCToolTipCtrl)
    ON_NOTIFY_REFLECT(TTN_SHOW, &CD2ItemToolTipCtrl::OnShow)
    ON_MESSAGE(TTM_RELAYEVENT, &CD2ItemToolTipCtrl::OnRelayEvent)
END_MESSAGE_MAP()

// CD2ItemToolTipCtrl message handlers
//---------------------------------------------------------------------------
void CD2ItemToolTipCtrl::OnShow(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (CurrItem != nullptr)
    {
        //AfxGetModuleThreadState()->m_nLastHit = static_cast<INT_PTR>(-1);
    }

    bool isInvItem = false;
    auto currID = (UINT)pNMHDR->idFrom;
    switch (currID)
    {
    case IDC_INV_MERC_HEAD:
    case IDC_INV_MERC_HAND_RIGHT:
    case IDC_INV_MERC_TORSO:
    case IDC_INV_MERC_HAND_LEFT:
        IsMerc = true;
        isInvItem = true;
        break;

    case IDC_INV_HEAD:
    case IDC_INV_CORPSE_HEAD:
    case IDC_INV_NECK:
    case IDC_INV_CORPSE_NECK:
    case IDC_INV_HAND_RIGHT:
    case IDC_INV_CORPSE_HAND_RIGHT:
    case IDC_INV_TORSO:
    case IDC_INV_CORPSE_TORSO:
    case IDC_INV_HAND_LEFT:
    case IDC_INV_CORPSE_HAND_LEFT:
    case IDC_INV_GLOVE:
    case IDC_INV_CORPSE_GLOVE:
    case IDC_INV_RING_RIGHT:
    case IDC_INV_CORPSE_RING_RIGHT:
    case IDC_INV_BELT:
    case IDC_INV_CORPSE_BELT:
    case IDC_INV_RING_LEFT:
    case IDC_INV_CORPSE_RING_LEFT:
    case IDC_INV_BOOTS:
    case IDC_INV_CORPSE_BOOTS:
    case IDC_INV_GOLEM:
    case IDC_INV_GRID:
    case IDC_INV_STASH_GRID:
    case IDC_INV_CUBE_GRID:
    case IDC_INV_BELT_GRID:
        IsMerc = false;
        isInvItem = true;
        break;

    default:
        IsMerc = false;
        isInvItem = false;
        break;
    }

    CurrItem = nullptr;
    if (isInvItem && Callback != nullptr)
    {
        CPoint point;
        ::GetCursorPos(&point);
        CurrItem = Callback->InvHitTest(currID, point);
    }

    __super::OnShow(pNMHDR, pResult);
}
//---------------------------------------------------------------------------
LRESULT CD2ItemToolTipCtrl::OnRelayEvent(WPARAM wParam, LPARAM lParam)
{
    MSG* pMsg = (MSG*)lParam;
    auto currID = UINT(AfxGetModuleThreadState()->m_nLastHit);
    switch (currID)
    {
    case IDC_INV_GRID:
    case IDC_INV_STASH_GRID:
    case IDC_INV_CUBE_GRID:
    case IDC_INV_BELT_GRID:
        if (Callback != nullptr && pMsg != nullptr && pMsg->message == WM_MOUSEMOVE)
        {
            CPoint point = pMsg->pt;
            auto newItem = Callback->InvHitTest(currID, point);
            if (newItem == nullptr && CurrItem != nullptr)
            {
                Pop();
                return 0;
            }

            if (newItem != CurrItem)
            {
                Update();
            }
        }
        break;
    }
    return DefWindowProc(TTM_RELAYEVENT, wParam, lParam);
}
//---------------------------------------------------------------------------