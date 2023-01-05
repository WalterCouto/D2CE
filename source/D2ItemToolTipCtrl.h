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

#pragma once

//---------------------------------------------------------------------------
#include "D2MainForm.h"

//---------------------------------------------------------------------------
class CD2ItemToolTipCtrlCallback
{
public:
    virtual ~CD2ItemToolTipCtrlCallback() = default;
    virtual const d2ce::Item* InvHitTest(UINT id, CPoint point, TOOLINFO* pTI = nullptr) const = 0;
    virtual const d2ce::Item* GetInvItem(UINT id, UINT offset) const = 0;
};

//---------------------------------------------------------------------------
class CD2ItemToolTipCtrl : public CMFCToolTipCtrl
{
    DECLARE_DYNCREATE(CD2ItemToolTipCtrl)

// Construction
public:
	CD2ItemToolTipCtrl(d2ce::Character& charInfo, CMFCToolTipInfo * pParams = NULL);

    void SetCallback(const CD2ItemToolTipCtrlCallback* callback = nullptr);

// Overrides
public:
    static CSize DoDrawItemInfo(CDC* pDC, CRect rect, BOOL bCalcOnly, const d2ce::Item* currItem, const d2ce::Character* pCharInfo, bool isMerc);
    CSize OnDrawLabel(CDC* pDC, CRect rect, BOOL bCalcOnly) override;
    void OnFillBackground(CDC* pDC, CRect rect, COLORREF& clrText, COLORREF& clrLine) override;


// Implementation
public:
	virtual ~CD2ItemToolTipCtrl();

protected:
	afx_msg void OnShow(NMHDR* pNMHDR, LRESULT* pResult);
    LRESULT OnRelayEvent(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

protected:
    d2ce::Character& CharInfo;
    bool IsMerc = false;
    const d2ce::Item* CurrItem = nullptr;
    const CD2ItemToolTipCtrlCallback* Callback = nullptr;

    CFont ToolTipFont;
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class CD2ItemTooltipCallback
{
public:
    virtual ~CD2ItemTooltipCallback() = default;
    virtual const d2ce::Character* GetCharacterInfo() const = 0;
    virtual const d2ce::Item* GetSelectedItem() const = 0;
};

//---------------------------------------------------------------------------
class CD2ItemInfoStatic : public CStatic
{
    friend class CD2NewItemForm;
    friend class CD2MagicalAffixesForm;
    DECLARE_DYNAMIC(CD2ItemInfoStatic)

    // Construction
public:
    CD2ItemInfoStatic();

public:
    virtual ~CD2ItemInfoStatic();

    // Generated message map functions
protected:
    //{{AFX_MSG(CD2ItemInfoStatic)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

protected:
    virtual void PreSubclassWindow();
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    const d2ce::Character* GetCharacterInfo() const;
    const d2ce::Item* GetSelectedItem() const;
    CD2ItemTooltipCallback* GetCallback() const;

    CFont ToolTipFont;
};
//---------------------------------------------------------------------------
