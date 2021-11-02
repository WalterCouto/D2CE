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

#pragma once

//---------------------------------------------------------------------------
#include "D2MainForm.h"

//---------------------------------------------------------------------------
class CD2ItemToolTipCtrl : public CMFCToolTipCtrl
{
    DECLARE_DYNCREATE(CD2ItemToolTipCtrl)

// Construction
public:
	CD2ItemToolTipCtrl(d2ce::Character& charInfo, CMFCToolTipInfo * pParams = NULL);

// Overrides
public:
    CSize OnDrawLabel(CDC* pDC, CRect rect, BOOL bCalcOnly) override;
    void OnFillBackground(CDC* pDC, CRect rect, COLORREF& clrText, COLORREF& clrLine) override;

// Implementation
public:
	virtual ~CD2ItemToolTipCtrl();

protected:
	afx_msg void OnShow(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

protected:
    d2ce::Character& CharInfo;
    bool IsMerc = false;
    int	CurrID = 0;
    const d2ce::Item* CurrItem = nullptr;
};
//---------------------------------------------------------------------------
