/*
	Diablo II Character Editor
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
	if(CurrItem->getDisplayedCombinedMagicalAttributes(magicalAttributes, charLevel))
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
		strText = _T("Ethereal");
		sizeText = CalcTextSize(pDC, strText, rect, bCalcOnly);
		sizeText.cy += prevSizeText.cy;
		sizeText.cx = std::max(prevSizeText.cx, sizeText.cx);
	}

	// Socketed text
	if (CurrItem->isSocketed())
	{
		CSize prevSizeText = sizeText;
		pDC->SetTextColor(colors[BLUE]);
		int socketedCount = CurrItem->socketedItemCount();
		if (socketedCount == 0)
		{
			strText.Format(_T("Sockets(%d)"), (int)CurrItem->totalNumberOfSockets());
		}
		else
		{
			strText.Format(_T("Sockets(%d/%d)"), socketedCount, (int)CurrItem->totalNumberOfSockets());
		}
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
	ON_NOTIFY_REFLECT(TTN_SHOW, OnShow)
END_MESSAGE_MAP()

// CD2ItemToolTipCtrl message handlers

//---------------------------------------------------------------------------
void CD2ItemToolTipCtrl::OnShow(NMHDR* pNMHDR, LRESULT* pResult)
{
	IsMerc = false;
	bool isGolem = false;
	bool isCorpse = false;
	CurrID = (UINT)pNMHDR->idFrom;
	d2ce::EnumEquippedId equippedId = d2ce::EnumEquippedId::NONE;
	switch (CurrID)
	{
	case IDC_INV_HEAD:
		equippedId = d2ce::EnumEquippedId::HEAD;
		break;

	case IDC_INV_CORPSE_HEAD:
		isCorpse = true;
		equippedId = d2ce::EnumEquippedId::HEAD;
		break;

	case IDC_INV_MERC_HEAD:
		IsMerc = true;
		equippedId = d2ce::EnumEquippedId::HEAD;
		break;

	case IDC_INV_HAND_RIGHT:
		equippedId = d2ce::EnumEquippedId::HAND_RIGHT;
		break;

	case IDC_INV_CORPSE_HAND_RIGHT:
		isCorpse = true;
		equippedId = d2ce::EnumEquippedId::HAND_RIGHT;
		break;

	case IDC_INV_MERC_HAND_RIGHT:
		IsMerc = true;
		equippedId = d2ce::EnumEquippedId::HAND_RIGHT;
		break;

	case IDC_INV_TORSO:
		equippedId = d2ce::EnumEquippedId::TORSO;
		break;

	case IDC_INV_CORPSE_TORSO:
		isCorpse = true;
		equippedId = d2ce::EnumEquippedId::TORSO;
		break;

	case IDC_INV_MERC_TORSO:
		IsMerc = true;
		equippedId = d2ce::EnumEquippedId::TORSO;
		break;

	case IDC_INV_HAND_LEFT:
		equippedId = d2ce::EnumEquippedId::HAND_LEFT;
		break;

	case IDC_INV_CORPSE_HAND_LEFT:
		isCorpse = true;
		equippedId = d2ce::EnumEquippedId::HAND_LEFT;
		break;

	case IDC_INV_MERC_HAND_LEFT:
		IsMerc = true;
		equippedId = d2ce::EnumEquippedId::HAND_LEFT;
		break;

	case IDC_INV_ALT_HAND_RIGHT:
		equippedId = d2ce::EnumEquippedId::ALT_HAND_RIGHT;
		break;

	case IDC_INV_CORPSE_ALT_HAND_RIGHT:
		isCorpse = true;
		equippedId = d2ce::EnumEquippedId::ALT_HAND_RIGHT;
		break;

	case IDC_INV_ALT_HAND_LEFT:
		equippedId = d2ce::EnumEquippedId::ALT_HAND_LEFT;
		break;

	case IDC_INV_CORPSE_ALT_HAND_LEFT:
		isCorpse = true;
		equippedId = d2ce::EnumEquippedId::ALT_HAND_LEFT;
		break;

	case IDC_INV_GOLEM:
		isGolem = true;
		equippedId = d2ce::EnumEquippedId::NONE;
		break;
	}

	if (equippedId != d2ce::EnumEquippedId::NONE)
	{
		const auto& charItems = IsMerc ? CharInfo.getMercItems() : (isCorpse ? CharInfo.getCorpseItems() : CharInfo.getInventoryItems());
		for (const auto& item : charItems)
		{
			if ((item.getLocation() != d2ce::EnumItemLocation::EQUIPPED) ||
				(item.getEquippedId() != equippedId))
			{
				continue;
			}

			CurrItem = &item;
			break;
		}
	}
	else if(isGolem && CharInfo.hasGolem())
	{
		CurrItem = &CharInfo.getGolemItem();
	}

	__super::OnShow (pNMHDR, pResult);
}
//---------------------------------------------------------------------------