/*
	Diablo II Character Editor
	Copyright (C) 2022 Walter Couto

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

// CD2MultiLineListCtrl
#include <vector>
#include <map>

class CD2MultiLineListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CD2MultiLineListCtrl)

public:
	CD2MultiLineListCtrl();
	virtual ~CD2MultiLineListCtrl();

	void ShowGridLines(BOOL bShow);
	DWORD SetExtendedStyle(DWORD dwNewStyle);
	DWORD GetExtendedStyle() const;

	// Retrieves a description of a particular item in the control.
	BOOL GetItem(LVITEM* pItem) const;

	// Sets information to an existing item in the control.
	BOOL SetItem(const LVITEM* pItem);
	BOOL SetItem(int nItem, int nSubItem, UINT nMask,  LPCTSTR lpszItem,
		int nImage, UINT nState, UINT nStateMask, LPARAM lParam);
	BOOL SetItem(int nItem, int nSubItem, UINT nMask,  LPCTSTR lpszItem,
		int nImage, UINT nState, UINT nStateMask, LPARAM lParam, int nIndent);

	// Retrieves the number of items in the control.
	int GetItemCount() const;

	// Gets first item selected in the control and prepares for
	// finding other selected items (if the control has the multiple
	// selection style).
	POSITION GetFirstSelectedItemPosition() const;

	// Finds the next selected item, after a previous call
	// to GetFirstSelectedItemPosition().
	int GetNextSelectedItem( POSITION& pos) const;

	// Retrieves the bounding rectangle for a particular item.
	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode) const; 
	
	BOOL InvalidateItemRect(int nItem, BOOL bErase);

	// Adds an item to the control.
	int InsertItem(const LVITEM* pItem);
	int InsertItem(int nItem, LPCTSTR lpszItem);
	int InsertItem(int nItem, LPCTSTR lpszItem, int nImage);

	// Removes a single item from the control.
	BOOL DeleteItem(int nItem);

	// Removes all items from the control.
	BOOL DeleteAllItems();

	// Retrieves the index of the topmost visible item in the control.
	int GetTopIndex() const;

	// Retrieves the text associated with a particular item.
	CString GetItemText(int nItem, int nSubItem) const;
	int GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const;

	// Sets the text associated with a particular item.
	BOOL SetItemText(int nItem,int nSubItem,LPCTSTR lpszText);

	// Sets the data (lParam) associated with a particular item.
	BOOL SetItemData(int nItem, DWORD_PTR dwData);

	// Retrieves the data (lParam) associated with a particular item.
	DWORD_PTR GetItemData(int nItem) const;

	// Causes the control to scroll its content so the specified item
	// is completely (or at least partially, depending on the
	// bPartialOK parameter) visible.
	BOOL EnsureVisible(int nItem, BOOL bPartialOK);

	// Forces the control to repaint a specific range of items.
	BOOL RedrawItems(int nFirst, int nLast);

	// Forces the control to repaint a specific item.
	BOOL Update(int nItem);

	void AdjustColumnWidths();

protected:
	DECLARE_MESSAGE_MAP()

	struct ColData
	{
		size_t numRows = 1;
		CString text;
	};

	struct RowData
	{
		size_t startRow = 0;
		size_t numRows = 1; // max of colData
		std::vector<ColData> colData;
		DWORD_PTR dwData = 0;
	};
	std::vector<RowData> m_rowData;
	BOOL m_bDrawGrid = TRUE;
	
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	BOOL SetMultilineText(int nItem, LPCTSTR lpszText, int nSubItem = -1);
};
