/*
    Diablo II Character Editor
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
#include "D2ItemToolTipCtrl.h"
#include "resource.h"		// main symbols
#include <utf8/utf8.h>
#include "helpers/ItemHelpers.h"
#include "D2NewItemForm.h"
#include "D2MagicalAffixesForm.h"
#include "D2RareAffixesForm.h"
#include "D2RunewordForm.h"
#include "D2AddGemsForm.h"
#include "D2MagicalPropsRandomizer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
#define TOOLTIP_BLUE RGB(94,94,255)

    CSize CalcTextSize(CDC* pDC, CString& strText, CRect& rect, BOOL bCalcOnly)
    {
        CSize sizeText(0, 0);

        strText.Replace(_T("\t"), _T("    "));

        if (bCalcOnly && (strText.Find(_T('\f')) >= 0))
        {
            // don't care about color
            strText.Remove(_T('\f'));
        }

        if (strText.Find(_T('\f')) >= 0) // Multi-color text
        {
            if (strText.Find(_T('\n')) >= 0) // Multi-line text
            {
                CSize lineSizeText(0, 0);
                int curLinePos = 0;
                CString strLineText = strText.Tokenize(_T("\n"), curLinePos);
                while (!strLineText.IsEmpty())
                {
                    if (strLineText.Find(_T('\f')) >= 0) // Multi-color text
                    {
                        CSize colorSizeText(0, 0);
                        CString strNoColorText = strLineText;
                        strNoColorText.Remove(_T('\f'));

                        auto lineRect = rect;
                        colorSizeText = pDC->GetTextExtent(strNoColorText);
                        lineRect.left += (rect.Width() - colorSizeText.cx) / 2;
                        int curPos = 0;
                        auto origColor = pDC->GetTextColor();
                        bool bBlue = false;
                        CString strColorText = strLineText.Tokenize(_T("\f"), curPos);
                        while (!strColorText.IsEmpty())
                        {
                            colorSizeText = pDC->GetTextExtent(strColorText);
                            lineSizeText.cy = std::max(colorSizeText.cy, lineSizeText.cy);
                            if (bBlue)
                            {
                                pDC->SetTextColor(TOOLTIP_BLUE);
                            }
                            bBlue = !bBlue;

                            UINT nFormat = DT_LEFT | DT_NOCLIP | DT_SINGLELINE;
                            colorSizeText.cy = pDC->DrawText(strColorText, lineRect, nFormat);
                            lineRect.left += colorSizeText.cx;
                            lineSizeText.cx = (LONG)rect.Width();
                            pDC->SetTextColor(origColor);
                            strColorText = strLineText.Tokenize(_T("\f"), curPos);
                        }
                    }
                    else
                    {
                        UINT nFormat = DT_CENTER | DT_NOCLIP | DT_SINGLELINE;
                        lineSizeText.cy = pDC->DrawText(strLineText, rect, nFormat);
                        lineSizeText.cx = (LONG)rect.Width();
                    }

                    sizeText.cx = std::max(sizeText.cx, lineSizeText.cx);
                    sizeText.cy += lineSizeText.cy;
                    rect.top += lineSizeText.cy;
                    rect.bottom += lineSizeText.cy;
                    strLineText = strText.Tokenize(_T("\n"), curLinePos);
                }
            }
            else
            {
                CSize lineSizeText(0, 0);
                CSize colorSizeText(0, 0);
                CString strNoColorText = strText;
                strNoColorText.Remove(_T('\f'));

                auto lineRect = rect;
                colorSizeText = pDC->GetTextExtent(strNoColorText);
                lineRect.left += (rect.Width() - colorSizeText.cx) / 2;

                int curPos = 0;
                auto origColor = pDC->GetTextColor();
                bool bBlue = false;
                CString strColorText = strText.Tokenize(_T("\f"), curPos);
                while (!strColorText.IsEmpty())
                {
                    colorSizeText = pDC->GetTextExtent(strColorText);
                    lineSizeText.cy = std::max(colorSizeText.cy, lineSizeText.cy);
                    if (bBlue)
                    {
                        pDC->SetTextColor(TOOLTIP_BLUE);
                    }
                    bBlue = !bBlue;

                    UINT nFormat = DT_LEFT | DT_NOCLIP | DT_SINGLELINE;
                    colorSizeText.cy = pDC->DrawText(strColorText, lineRect, nFormat);
                    lineRect.left += colorSizeText.cx;
                    lineSizeText.cx = (LONG)rect.Width();
                    pDC->SetTextColor(origColor);
                    strColorText = strText.Tokenize(_T("\f"), curPos);
                }

                sizeText.cx = std::max(sizeText.cx, lineSizeText.cx);
                sizeText.cy = lineSizeText.cy;
                rect.top += sizeText.cy;
                rect.bottom += sizeText.cy;
            }
        }
        else if (strText.Find(_T('\n')) >= 0) // Multi-line text
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
    //------------------
    // Create tooltip font:
    //------------------
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = -MulDiv(11, AfxGetMainWnd()->GetDC()->GetDeviceCaps(LOGPIXELSY), 72);
    _tcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Tahoma"));
    ToolTipFont.CreateFontIndirect(&lf);
}
//---------------------------------------------------------------------------
void CD2ItemToolTipCtrl::SetCallback(const CD2ItemToolTipCtrlCallback* callback)
{
    Callback = callback;
}
//---------------------------------------------------------------------------
CSize CD2ItemToolTipCtrl::DoDrawItemInfo(CDC* pDC, CRect rect, BOOL bCalcOnly, const d2ce::Item* currItem, const d2ce::Character* pCharInfo, bool isMerc)
{
    if (currItem == nullptr)
    {
        return CSize(0, 0);
    }

    bool isRuneword = currItem->isRuneword();

    // color codes as described in the text files
    static const COLORREF colors[] = { RGB(255,255,255), RGB(255, 0, 0), RGB(0,255,0), TOOLTIP_BLUE, RGB(148,128,100), RGB(117, 117, 117), RGB(255,255,255), RGB(255,255,255), RGB(255,128,0), RGB(255,255,0) };
    enum { WHITE = 0, RED = 1, GREEN = 2, BLUE = 3, UNIQUE = 4, GRAY = 5, CRAFTED = 8, RARE = 9 };

    // Get color of top text
    COLORREF color = colors[WHITE];
    if (currItem->isIdentified())
    {
        if (currItem->isEthereal())
        {
            color = colors[GRAY];
        }
        else if (currItem->isQuestItem())
        {
            color = colors[UNIQUE];
        }
        else if (currItem->isRune())
        {
            color = colors[CRAFTED];
        }
        else if (isRuneword)
        {
            color = colors[UNIQUE];
        }

        switch (currItem->getQuality())
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

        case d2ce::EnumItemQuality::CRAFTED:
        case d2ce::EnumItemQuality::TEMPERED:
            color = colors[CRAFTED];
            break;
        }
    }
    else
    {
        color = colors[RED];
    }
    pDC->SetTextColor(color);

    std::u16string uText = utf8::utf8to16(currItem->getDisplayedItemName());
    CString strText(reinterpret_cast<LPCWSTR>(uText.c_str()));

    CSize sizeText(0, 0);
    if (bCalcOnly || !isRuneword)
    {
        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
    }
    else
    {
        if (strText.Find(_T('\n')) >= 0) // Multi-line text
        {
            // only name is coloried as unqiue, type name is colored as gray
            CSize prevSizeText = sizeText;
            int curLinePos = 0;
            CString strLineText = strText.Tokenize(_T("\n"), curLinePos);
            while (!strLineText.IsEmpty())
            {
                sizeText = CalcTextSize(pDC, strLineText, rect, bCalcOnly);
                sizeText.cy += prevSizeText.cy;
                sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
                prevSizeText = sizeText;
                pDC->SetTextColor(colors[GRAY]);
                strLineText = strText.Tokenize(_T("\n"), curLinePos);
            }
        }
        else
        {
            sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        }
    }

    // draw possible rune name
    uText = utf8::utf8to16(currItem->getDisplayedSocketedRunes());
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
    d2ce::CharStats displayedCs;
    if (pCharInfo != nullptr)
    {
        if (isMerc)
        {
            pCharInfo->getMercenaryInfo().fillMercStats(displayedCs);
        }
        else
        {
            pCharInfo->fillDisplayedCharacterStats(displayedCs);
        }

    }
    else
    {
        displayedCs.Level = 1;
    }
    auto charClass = pCharInfo == nullptr ? d2ce::EnumCharClass::Amazon : pCharInfo->getClass();
    uText = utf8::utf8to16(currItem->getDisplayedItemAttributes(charClass, displayedCs, true));
    strText = reinterpret_cast<LPCWSTR>(uText.c_str());
    if (!strText.IsEmpty())
    {
        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[WHITE]);

        if (strText.Find(_T('\b')) >= 0) // red text
        {
            if (strText.Find(_T('\n')) >= 0) // Multi-line text
            {
                int curLinePos = 0;
                CString strLineText = strText.Tokenize(_T("\n"), curLinePos);
                while (!strLineText.IsEmpty())
                {
                    if (strLineText.Find(_T('\b')) >= 0) // red text
                    {
                        pDC->SetTextColor(colors[RED]);
                        strLineText.Remove(_T('\b'));
                    }

                    sizeText = CalcTextSize(pDC, strLineText, rect, bCalcOnly);
                    sizeText.cy += prevSizeText.cy;
                    sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
                    prevSizeText = sizeText;
                    strLineText = strText.Tokenize(_T("\n"), curLinePos);
                    pDC->SetTextColor(colors[WHITE]);
                }
            }
            else
            {
                pDC->SetTextColor(colors[RED]);
                strText.Remove(_T('\b'));
                sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
                sizeText.cy += prevSizeText.cy;
                sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
            }
        }
        else
        {
            sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
            sizeText.cy += prevSizeText.cy;
            sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
        }
    }

    d2ce::ItemDurability durability;
    if (currItem->getDurability(durability) && durability.Max == 0)
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
    if (currItem->getDisplayedCombinedMagicalAttributes(magicalAttributes, displayedCs.Level))
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

    if (currItem->isEthereal())
    {
        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[BLUE]);

        std::string u8Text;
        uText = utf8::utf8to16(d2ce::LocalizationHelpers::GetEtherealStringTxtValue(u8Text));
        strText = reinterpret_cast<LPCWSTR>(uText.c_str());
        if (currItem->isSocketed())
        {
            // Socketed text
            strText += _T(", ");
            uText = utf8::utf8to16(d2ce::LocalizationHelpers::GetSocketedStringTxtValue(u8Text));
            CString socketedText(reinterpret_cast<LPCWSTR>(uText.c_str()));
            CString temp;
            temp.Format(socketedText, (int)currItem->getDisplayedSocketCount());
            strText += temp;
        }
        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        sizeText.cy += prevSizeText.cy;
        sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
    }
    else if (currItem->isSocketed())
    {
        // Socketed text
        CSize prevSizeText = sizeText;
        pDC->SetTextColor(colors[BLUE]);

        std::string u8Text;
        uText = utf8::utf8to16(d2ce::LocalizationHelpers::GetSocketedStringTxtValue(u8Text));
        CString socketedText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        CString temp;
        temp.Format(socketedText, (int)currItem->getDisplayedSocketCount());
        strText = temp;

        sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
        sizeText.cy += prevSizeText.cy;
        sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
    }

    // Green Set props
    std::vector<d2ce::MagicalAttribute> setAttributes;
    if (currItem->getDisplayedSetItemAttributes(setAttributes, displayedCs.Level))
    {
        for (const auto& attrib : setAttributes)
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
            pDC->SetTextColor(colors[GREEN]);

            sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
            sizeText.cy += prevSizeText.cy;
            sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
        }
    }

    // Green Set props
    std::vector<d2ce::MagicalAttribute> setBonus;
    if (currItem->getDisplayedFullSetAttributes(setBonus, displayedCs.Level))
    {
        bool bFirst = true;
        for (const auto& attrib : setBonus)
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

            if (bFirst)
            {
                bFirst = false;
                strText.Insert(0, L"\n");
            }

            CSize prevSizeText = sizeText;
            pDC->SetTextColor(colors[UNIQUE]);

            sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
            sizeText.cy += prevSizeText.cy;
            sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
        }
    }

    return sizeText;
}
//---------------------------------------------------------------------------
CSize CD2ItemToolTipCtrl::OnDrawLabel(CDC* pDC, CRect rect, BOOL bCalcOnly)
{
    if (CurrItem == nullptr)
    {
        return __super::OnDrawLabel(pDC, rect, bCalcOnly);
    }

    auto pOldFont = pDC->SelectObject(&ToolTipFont);
    CSize ret = DoDrawItemInfo(pDC, rect, bCalcOnly, CurrItem, &CharInfo, IsMerc);
    pDC->SelectObject(pOldFont);
    return ret;
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

//---------------------------------------------------------------------------
// CD2ItemInfoStatic

IMPLEMENT_DYNAMIC(CD2ItemInfoStatic, CStatic)

CD2ItemInfoStatic::CD2ItemInfoStatic()
{
   //------------------
   // Create tooltip font:
   //------------------
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = -MulDiv(11, AfxGetMainWnd()->GetDC()->GetDeviceCaps(LOGPIXELSY), 72);
    _tcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Tahoma"));
    ToolTipFont.CreateFontIndirect(&lf);
}
//---------------------------------------------------------------------------
CD2ItemInfoStatic::~CD2ItemInfoStatic()
{
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2ItemInfoStatic, CStatic)
    //{{AFX_MSG_MAP(CD2ItemInfoStatic)
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
void CD2ItemInfoStatic::PreSubclassWindow()
{
    __super::PreSubclassWindow();
    ModifyStyle(0, SS_OWNERDRAW);
}
//---------------------------------------------------------------------------
void CD2ItemInfoStatic::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}
//---------------------------------------------------------------------------
BOOL CD2ItemInfoStatic::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);

    auto currItem = GetSelectedItem();
    if (currItem == nullptr)
    {
        CBrush backBrush(::GetSysColor(COLOR_3DFACE)); // (this is meant for dialogs)
        CBrush* pOldBrush = pDC->SelectObject(&backBrush);

        pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
        pDC->SelectObject(pOldBrush);
        return TRUE;
    }

    CBrush backBrush(RGB(0, 0, 0));
    CBrush* pOldBrush = pDC->SelectObject(&backBrush);

    pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOldBrush);

    pDC->SetBkColor(RGB(0, 0, 0));
    pDC->SetBkMode(TRANSPARENT);

    auto pOldFont = pDC->SelectObject(&ToolTipFont);
    CD2ItemToolTipCtrl::DoDrawItemInfo(pDC, rect, false, currItem, GetCharacterInfo(), false);
    pDC->SelectObject(pOldFont);
    return TRUE;
}
//---------------------------------------------------------------------------
const d2ce::Character* CD2ItemInfoStatic::GetCharacterInfo() const
{
    auto pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return nullptr;
    }

    return pCallback->GetCharacterInfo();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemInfoStatic::GetSelectedItem() const
{
    auto pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return nullptr;
    }

    return pCallback->GetSelectedItem();
}
//---------------------------------------------------------------------------
CD2ItemTooltipCallback* CD2ItemInfoStatic::GetCallback() const
{
    auto pCallback = dynamic_cast<CD2ItemTooltipCallback*>(DYNAMIC_DOWNCAST(CD2NewItemForm, GetParent()));
    if (pCallback != nullptr)
    {
        return pCallback;
    }

    pCallback = dynamic_cast<CD2ItemTooltipCallback*>(DYNAMIC_DOWNCAST(CD2MagicalAffixesForm, GetParent()));
    if (pCallback != nullptr)
    {
        return pCallback;
    }

    pCallback = dynamic_cast<CD2ItemTooltipCallback*>(DYNAMIC_DOWNCAST(CD2RareAffixesForm, GetParent()));
    if (pCallback != nullptr)
    {
        return pCallback;
    }

    pCallback = dynamic_cast<CD2ItemTooltipCallback*>(DYNAMIC_DOWNCAST(CD2RunewordForm, GetParent()));
    if (pCallback != nullptr)
    {
        return pCallback;
    }

    pCallback = dynamic_cast<CD2ItemTooltipCallback*>(DYNAMIC_DOWNCAST(CD2AddGemsForm, GetParent()));
    if (pCallback != nullptr)
    {
        return pCallback;
    }

    return dynamic_cast<CD2ItemTooltipCallback*>(DYNAMIC_DOWNCAST(CD2MagicalPropsRandomizer, GetParent()));
}
//---------------------------------------------------------------------------