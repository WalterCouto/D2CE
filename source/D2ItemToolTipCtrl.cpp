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
#include <utf8/utf8.h>
#include "helpers/ItemHelpers.h"

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

    // color codes as described in the text files
    static const COLORREF colors[] = { RGB(255,255,255), RGB(255, 0, 0), RGB(0,255,0), RGB(94,94,255), RGB(148,128,100), RGB(117, 117, 117), RGB(255,255,255), RGB(255,255,255), RGB(255,128,0), RGB(255,255,0) };
    enum { WHITE = 0, RED = 1, GREEN = 2, BLUE = 3, UNIQUE = 4, GRAY = 5, CRAFT = 8, RARE = 9 };

    // Get color of top text
    COLORREF color = colors[WHITE];
    if (CurrItem->isIdentified())
    {
        if (CurrItem->isEthereal())
        {
            color = colors[GRAY];
        }
        else if (CurrItem->isQuestItem())
        {
            color = colors[UNIQUE];
        }
        else if (CurrItem->isRune())
        {
            color = colors[CRAFT];
        }

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
    }
    else
    {
        color = colors[RED];
    }
    pDC->SetTextColor(color);

    std::u16string uText = utf8::utf8to16(CurrItem->getDisplayedItemName());
    CString strText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    CSize sizeText(CalcTextSize(pDC, strText, rect, bCalcOnly));

    // draw possible rune name
    uText = utf8::utf8to16(CurrItem->getDisplayedSocketedRunes());
    strText = reinterpret_cast<LPCWSTR>(uText.c_str());
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
    uText = utf8::utf8to16(CurrItem->getDisplayedItemAttributes(CharInfo.getClass(), charLevel));
    strText = reinterpret_cast<LPCWSTR>(uText.c_str());
    if (!strText.IsEmpty())
    {
        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[WHITE]);
        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        sizeText.cy += prevSizeText.cy;
        sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
    }

    d2ce::ItemDurability durability;
    if (CurrItem->getDurability(durability) && durability.Max == 0)
    {
        // Indestructible without the need for the magical attribute of indestructibility
        std::string u8Text;
        uText = utf8::utf8to16(d2ce::LocalizationHelpers::GetIndestructibleStringTxtValue(u8Text));
        strText = reinterpret_cast<LPCWSTR>(uText.c_str());

        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[BLUE]);

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

            uText = utf8::utf8to16(attrib.Desc);
            strText = reinterpret_cast<LPCWSTR>(uText.c_str());
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

        std::string u8Text;
        uText = utf8::utf8to16(d2ce::LocalizationHelpers::GetEtherealStringTxtValue(u8Text));
        strText = reinterpret_cast<LPCWSTR>(uText.c_str());
        if (CurrItem->isSocketed())
        {
            // Socketed text
            strText += _T(", ");
            uText = utf8::utf8to16(d2ce::LocalizationHelpers::GetSocketedStringTxtValue(u8Text));
            CString socketedText(reinterpret_cast<LPCWSTR>(uText.c_str()));
            CString temp;
            temp.Format(socketedText, (int)CurrItem->getDisplayedSocketCount());
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

        std::string u8Text;
        uText = utf8::utf8to16(d2ce::LocalizationHelpers::GetSocketedStringTxtValue(u8Text));
        CString socketedText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        CString temp;
        temp.Format(socketedText, (int)CurrItem->getDisplayedSocketCount());
        strText = temp;

        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        sizeText.cy += prevSizeText.cy;
        sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
    }

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
    bool isInvItem = false;
    auto currID = (UINT)pNMHDR->idFrom;
    switch (currID)
    {
    case IDC_INV_MERC_HEAD:
    case IDC_INV_MERC_RIGHT_ARM:
    case IDC_INV_MERC_TORSO:
    case IDC_INV_MERC_LEFT_ARM:
        IsMerc = true;
        isInvItem = true;
        break;

    case IDC_INV_HEAD:
    case IDC_INV_CORPSE_HEAD:
    case IDC_INV_NECK:
    case IDC_INV_CORPSE_NECK:
    case IDC_INV_RIGHT_ARM:
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_TORSO:
    case IDC_INV_CORPSE_TORSO:
    case IDC_INV_LEFT_ARM:
    case IDC_INV_CORPSE_LEFT_ARM:
    case IDC_INV_GLOVES:
    case IDC_INV_CORPSE_GLOVES:
    case IDC_INV_RIGHT_RING:
    case IDC_INV_CORPSE_RIGHT_RING:
    case IDC_INV_BELT:
    case IDC_INV_CORPSE_BELT:
    case IDC_INV_LEFT_RING:
    case IDC_INV_CORPSE_LEFT_RING:
    case IDC_INV_FEET:
    case IDC_INV_CORPSE_FEET:
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