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

#include "pch.h"
#include "D2MultiLineListCtrl.h"

// CD2MultiLineListCtrl

IMPLEMENT_DYNAMIC(CD2MultiLineListCtrl, CListCtrl)

//---------------------------------------------------------------------------
CD2MultiLineListCtrl::CD2MultiLineListCtrl()
{
}
//---------------------------------------------------------------------------
CD2MultiLineListCtrl::~CD2MultiLineListCtrl()
{
}

BEGIN_MESSAGE_MAP(CD2MultiLineListCtrl, CListCtrl)
    ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CD2MultiLineListCtrl::OnLvnItemchanged)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CD2MultiLineListCtrl::OnCustomDraw)
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
DWORD CD2MultiLineListCtrl::SetExtendedStyle(DWORD dwNewStyle)
{
    if (dwNewStyle & LVS_EX_GRIDLINES)
    {
        m_bDrawGrid = TRUE;
        dwNewStyle &= ~LVS_EX_GRIDLINES;
    }
    else
    {
        m_bDrawGrid = FALSE;
    }

    return __super::SetExtendedStyle(dwNewStyle);;
}
//---------------------------------------------------------------------------
DWORD CD2MultiLineListCtrl::GetExtendedStyle() const
{
    auto flags = __super::GetExtendedStyle();
    if (m_bDrawGrid)
    {
        flags |= LVS_EX_GRIDLINES;
    }
    else
    {
        flags &= ~LVS_EX_GRIDLINES;
    }

    return flags;
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::GetItem(LVITEM* pItem) const
{
    if (pItem == nullptr)
    {
        return FALSE;
    }

    auto nItem = pItem->iItem;
    if (nItem >= 0 && nItem < (int)m_rowData.size())
    {
        LVITEM lvItem = *pItem;
        const auto& rowData = m_rowData[nItem];
        lvItem.iItem = (int)rowData.startRow;
        auto numRows = rowData.numRows;
        if (!__super::GetItem(&lvItem))
        {
            return FALSE;
        }

        *pItem = lvItem;
        pItem->iItem = nItem;
        if (((pItem->mask & LVIF_TEXT) != 0) && (pItem->pszText != LPSTR_TEXTCALLBACK))
        {
            if (numRows > 1)
            {
                const auto& colData = rowData.colData[pItem->iSubItem];
                _tcsncpy_s(pItem->pszText, pItem->cchTextMax, colData.text.GetString(), colData.text.GetLength());
            }
        }

        if ((pItem->mask & LVIF_PARAM) != 0)
        {
            pItem->lParam = LPARAM(rowData.dwData);
        }
        return TRUE;
    }

    return __super::GetItem(pItem);
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::SetItem(const LVITEM* pItem)
{
    if (pItem == nullptr)
    {
        return FALSE;
    }

    auto nItem = pItem->iItem;
    if (nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    if ((pItem->mask & LVIF_PARAM) != 0)
    {
        m_rowData[nItem].dwData = pItem->lParam;
    }

    if (((pItem->mask & LVIF_TEXT) != 0) && (pItem->pszText != LPSTR_TEXTCALLBACK))
    {
        SetItemText(nItem, pItem->iSubItem, pItem->pszText);
    }
    return TRUE;
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int /*nImage*/, UINT /*nState*/, UINT /*nStateMask*/, LPARAM lParam)
{
    if (nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    if ((nMask & LVIF_PARAM) != 0)
    {
        m_rowData[nItem].dwData = lParam;
    }

    if (((nMask & LVIF_TEXT) != 0) && (lpszItem != LPSTR_TEXTCALLBACK))
    {
        SetItemText(nItem, nSubItem, lpszItem);
    }

    return TRUE;
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int /*nImage*/, UINT /*nState*/, UINT /*nStateMask*/, LPARAM lParam, int /*nIndent*/)
{
    if (nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    if ((nMask & LVIF_PARAM) != 0)
    {
        m_rowData[nItem].dwData = lParam;
    }

    if (((nMask & LVIF_TEXT) != 0) && (lpszItem != LPSTR_TEXTCALLBACK))
    {
        SetItemText(nItem, nSubItem, lpszItem);
    }

    return TRUE;
}
//---------------------------------------------------------------------------
void CD2MultiLineListCtrl::ShowGridLines(BOOL bShow)
{
    DWORD flags = GetExtendedStyle();
    if (bShow)
    {
        flags |= LVS_EX_GRIDLINES;
    }
    else
    {
        flags &= ~LVS_EX_GRIDLINES;
    }
    SetExtendedStyle(flags);
}
//---------------------------------------------------------------------------
int CD2MultiLineListCtrl::GetItemCount() const
{
    return (int)m_rowData.size();
}
//---------------------------------------------------------------------------
POSITION CD2MultiLineListCtrl::GetFirstSelectedItemPosition() const
{
    return __super::GetFirstSelectedItemPosition();
}
//---------------------------------------------------------------------------
int CD2MultiLineListCtrl::GetNextSelectedItem(POSITION& pos) const
{
    int nItem = __super::GetNextSelectedItem(pos);
    auto rowPos = int(__super::GetItemData(nItem));
    if (rowPos < 0)
    {
        // only the top row is allowed to be selected
        rowPos = (int)__super::GetItemData(nItem + rowPos);
    }

    return rowPos;
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::GetItemRect(int nItem, LPRECT lpRect, UINT nCode) const
{
    if (nItem < 0 || nItem >= (int)m_rowData.size() || lpRect == nullptr)
    {
        return FALSE;
    }

    auto& rowData = m_rowData[nItem];
    auto numRows = int(rowData.numRows);
    nItem = int(rowData.startRow);

    CRect bounds;
    __super::GetItemRect(nItem, lpRect, nCode);
    if (numRows > 1)
    {
        __super::GetItemRect(nItem + (numRows - 1), &bounds, nCode);
        lpRect->bottom = bounds.bottom;
    }

    return TRUE;
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::InvalidateItemRect(int nItem, BOOL bErase)
{
    CRect bounds;
    if (!GetItemRect(nItem, &bounds, LVIR_BOUNDS))
    {
        return FALSE;
    }

    InvalidateRect(&bounds, bErase);
    return true;
}
//---------------------------------------------------------------------------
void CD2MultiLineListCtrl::OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);

    *pResult = 0;

    // forget messages that don't change the state
    if (pNMListView->uOldState == pNMListView->uNewState)
    {
        return;
    }

    if ((pNMListView->uNewState ^ pNMListView->uOldState) & LVIS_SELECTED)
    {
        // selection state changing
        auto rowPos = int(__super::GetItemData(pNMListView->iItem));
        if (rowPos < 0)
        {
            // should not happen as only top guy should be selected
            rowPos = int(__super::GetItemData(pNMListView->iItem + rowPos));
        }

        InvalidateItemRect(rowPos, FALSE);
        return;
    }
}
//---------------------------------------------------------------------------
void CD2MultiLineListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = CDRF_DODEFAULT;
    LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

    int nItem = int(lplvcd->nmcd.dwItemSpec);
    int nSubItem = lplvcd->iSubItem;
    int nRowPos = (int)__super::GetItemData(nItem);
    int nFirstItemRow = nRowPos;
    int nLastItemRow = nRowPos;
    size_t nNumLines = 1;
    int nCurrentSelection = -1;
    BOOL isSelected = FALSE;
    POSITION posSelection;
    HDC hdc = lplvcd->nmcd.hdc;
    CString str;
    CRect boxRect, bounds, rect;
    CDC* pDC = CDC::FromHandle(hdc);
    //BOOL wndFocused = (GetFocus() == this);

    switch (lplvcd->nmcd.dwDrawStage) 
    {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        return;

    case CDDS_ITEMPREPAINT:
        if (lplvcd->nmcd.uItemState & CDIS_FOCUS)
        {
            lplvcd->nmcd.uItemState &= ~CDIS_FOCUS;
        }
        *pResult = CDRF_NOTIFYSUBITEMDRAW | CDRF_NOTIFYPOSTPAINT;
        return;

    case CDDS_ITEMPREERASE:
        *pResult = CDRF_NOTIFYSUBITEMDRAW;
        return;

    case CDDS_ITEMPOSTPAINT:
        *pResult = CDRF_SKIPDEFAULT;
        return;

    case CDDS_SUBITEM | CDDS_PREPAINT | CDDS_ITEM:
        if (nRowPos < 0)
        {
            const auto& rowData = m_rowData[__super::GetItemData(nItem + nRowPos)];
            nFirstItemRow = int(rowData.startRow);
            nNumLines = rowData.numRows;
            nLastItemRow = int(nFirstItemRow + nNumLines - 1);
        }
        else
        {
            const auto& rowData = m_rowData[nRowPos];
            nFirstItemRow = int(rowData.startRow);
            nNumLines = rowData.numRows;
            nLastItemRow = int(nFirstItemRow + nNumLines - 1);
        }

        // Get background box
        boxRect = lplvcd->nmcd.rc;
        __super::GetItemRect(int(lplvcd->nmcd.dwItemSpec), &bounds, LVIR_BOUNDS);
        boxRect.top = bounds.top;
        boxRect.bottom = bounds.bottom;
        if (nSubItem == 0)
        {
            CRect lrect;
            __super::GetItemRect(int(lplvcd->nmcd.dwItemSpec), &lrect, LVIR_LABEL);
            boxRect.left = lrect.left;
            boxRect.right = lrect.right;
        }
        else
        {
            boxRect.right += bounds.left;
            boxRect.left += bounds.left;
        }

        // Get selection
        posSelection = __super::GetFirstSelectedItemPosition();
        if (posSelection)
        {
            nCurrentSelection = __super::GetNextSelectedItem(posSelection);
            if ((nCurrentSelection >= nFirstItemRow) && (nCurrentSelection <= nLastItemRow))
            {
                isSelected = true;
            }
        }
        else
        {
            nCurrentSelection = -1;
        }

        if (isSelected)
        {
            lplvcd->clrTextBk = ::GetSysColor(COLOR_HIGHLIGHT);
            lplvcd->clrText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
        }
        else
        {
            lplvcd->clrTextBk = ::GetSysColor(COLOR_WINDOW);
            lplvcd->clrText = ::GetSysColor(COLOR_WINDOWTEXT);
        }

        // Get text string
        str = __super::GetItemText(nItem, nSubItem);

        // Get text box
        rect = boxRect;
        rect.left += nSubItem ? 6 : 2;

        // Fill background box
        {
            CBrush brush(lplvcd->clrTextBk);
            pDC->FillRect(boxRect, &brush);
        }

        // Draw text
        {
            auto oldTextColor = pDC->SetTextColor(lplvcd->clrText);
            pDC->DrawText(str, rect, DT_SINGLELINE | DT_NOPREFIX | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
            pDC->SetTextColor(oldTextColor);
        }

        if (m_bDrawGrid)
        {
            // Draw grid
            CPen gridline(PS_SOLID, 1, ::GetSysColor(COLOR_BTNFACE));
            pDC->SelectObject(gridline);
            if (nSubItem > 0)
            {
                pDC->MoveTo(boxRect.left, boxRect.top);
                pDC->LineTo(boxRect.left, boxRect.bottom);
            }

            if (nItem == nLastItemRow) 
            {
                pDC->MoveTo(boxRect.left, boxRect.bottom - 1);
                pDC->LineTo(boxRect.right, boxRect.bottom - 1);
            }
        }
        *pResult = CDRF_SKIPDEFAULT;
        return;
    }
}
//---------------------------------------------------------------------------
int CD2MultiLineListCtrl::InsertItem(const LVITEM* pItem)
{
    if (pItem == nullptr)
    {
        return -1;
    }

    size_t colCount = 1;
    CHeaderCtrl* pHeader = GetHeaderCtrl();
    if (pHeader != nullptr)
    {
        auto headerCount = pHeader->GetItemCount();
        if (headerCount > 0)
        {
            colCount = size_t(headerCount);
        }
    }

    int result = -1;
    if (pItem->iItem >= 0 && pItem->iItem < (int)m_rowData.size())
    {
        auto row = int(m_rowData[pItem->iItem].startRow);
        LVITEM newItem = *pItem;
        newItem.iItem = row;
        result = __super::InsertItem(&newItem);
        if (result == -1)
        {
            return result;
        }
        __super::SetItemData(result, pItem->iItem);

        auto iter = m_rowData.begin();
        std::advance(iter, pItem->iItem);
        RowData rowData;
        rowData.startRow = result;
        rowData.colData.resize(colCount);
        rowData.colData[0].text = pItem->pszText;
        m_rowData.insert(iter, rowData);

        // adjust row offsets
        for (size_t i = size_t(pItem->iItem + 1); i < m_rowData.size(); ++i)
        {
            ++m_rowData[i].startRow;
            __super::SetItemData(int(m_rowData[i].startRow), i);
        }
    }
    else
    {
        LVITEM lvItem = *pItem;
        lvItem.iItem = __super::GetItemCount();
        result = __super::InsertItem(&lvItem);
        if (result == -1)
        {
            return result;
        }
        __super::SetItemData(result, m_rowData.size());

        RowData rowData;
        rowData.startRow = result;
        rowData.colData.resize(colCount);
        rowData.colData[0].text = pItem->pszText;
        m_rowData.push_back(rowData);
    }

    SetMultilineText(pItem->iItem, pItem->pszText);
    return result;
}
//---------------------------------------------------------------------------
int CD2MultiLineListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
    size_t colCount = 1;
    CHeaderCtrl* pHeader = GetHeaderCtrl();
    if (pHeader != nullptr)
    {
        auto headerCount = pHeader->GetItemCount();
        if (headerCount > 0)
        {
            colCount = size_t(headerCount);
        }
    }

    int result = -1;
    if (nItem >= 0 && nItem < (int)m_rowData.size())
    {
        auto row = int(m_rowData[nItem].startRow);
        result = __super::InsertItem(row, lpszItem);
        if (result == -1)
        {
            return result;
        }
        __super::SetItemData(result, nItem);

        auto iter = m_rowData.begin();
        std::advance(iter, nItem);
        RowData rowData;
        rowData.startRow = result;
        rowData.colData.resize(colCount);
        rowData.colData[0].text = lpszItem;
        m_rowData.insert(iter, rowData);

        // adjust row offsets
        for (size_t i = size_t(nItem + 1); i < m_rowData.size(); ++i)
        {
            ++m_rowData[i].startRow;
            __super::SetItemData(int(m_rowData[i].startRow), i);
        }
    }
    else
    {
        result = __super::InsertItem(nItem, lpszItem);
        if (result == -1)
        {
            return result;
        }
        __super::SetItemData(result, m_rowData.size());

        RowData rowData;
        rowData.startRow = result;
        rowData.colData.resize(colCount);
        rowData.colData[0].text = lpszItem;
        m_rowData.push_back(rowData);
    }

    SetMultilineText(nItem, lpszItem);
    return result;
}
//---------------------------------------------------------------------------
int CD2MultiLineListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
{
    size_t colCount = 1;
    CHeaderCtrl* pHeader = GetHeaderCtrl();
    if (pHeader != nullptr)
    {
        auto headerCount = pHeader->GetItemCount();
        if (headerCount > 0)
        {
            colCount = size_t(headerCount);
        }
    }

    int result = -1;
    if (nItem >= 0 && nItem < (int)m_rowData.size())
    {
        auto row = int(m_rowData[nItem].startRow);
        result = __super::InsertItem(row, lpszItem, nImage);
        if (result == -1)
        {
            return result;
        }
        __super::SetItemData(result, nItem);

        auto iter = m_rowData.begin();
        std::advance(iter, nItem);
        RowData rowData;
        rowData.startRow = result;
        rowData.colData.resize(colCount);
        rowData.colData[0].text = lpszItem;
        m_rowData.insert(iter, rowData);

        // adjust row offsets
        for (size_t i = size_t(nItem + 1); i < m_rowData.size(); ++i)
        {
            ++m_rowData[i].startRow;
            __super::SetItemData(int(m_rowData[i].startRow), i);
        }
    }
    else
    {
        result = __super::InsertItem(nItem, lpszItem, nImage);
        if (result == -1)
        {
            return result;
        }
        __super::SetItemData(result, m_rowData.size());

        RowData rowData;
        rowData.startRow = result;
        rowData.colData.resize(colCount);
        rowData.colData[0].text = lpszItem;
        m_rowData.push_back(rowData);
    }

    SetMultilineText(nItem, lpszItem);
    return result;
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::DeleteItem(int nItem)
{
    if (nItem >= 0 && nItem < (int)m_rowData.size())
    {
        auto rowStart = int(m_rowData[nItem].startRow);
        auto numRows = m_rowData[nItem].numRows;
        auto iter = m_rowData.begin();
        std::advance(iter, nItem);
        m_rowData.erase(iter);

        // adjust row offsets
        for (size_t i = size_t(nItem); i < m_rowData.size(); ++i)
        {
            --m_rowData[i].startRow;
            __super::SetItemData(int(m_rowData[i].startRow), i);
        }

        for (size_t i = 0; i < numRows; ++i)
        {
            if (!__super::DeleteItem(rowStart))
            {
                return FALSE;
            }
        }

        return TRUE;
    }

    return __super::DeleteItem(nItem);
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::DeleteAllItems()
{
    m_rowData.clear();
    return __super::DeleteAllItems();
}
//---------------------------------------------------------------------------
int CD2MultiLineListCtrl::GetTopIndex() const
{
    auto idx = __super::GetTopIndex();
    if (idx < 0)
    {
        return idx;
    }


    return 0;
}
//---------------------------------------------------------------------------
CString CD2MultiLineListCtrl::GetItemText(int nItem, int nSubItem) const
{
    if (nSubItem < 0 || nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    auto& rowData = m_rowData[nItem];
    if (rowData.colData.size() <= size_t(nSubItem))
    {
        return FALSE;
    }

    return rowData.colData[nSubItem].text;
}
//---------------------------------------------------------------------------
int CD2MultiLineListCtrl::GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const
{
    if (nSubItem < 0 || nItem < 0 || nItem >= (int)m_rowData.size() || nLen <= 0)
    {
        return 0;
    }

    auto& rowData = m_rowData[nItem];
    if (rowData.colData.size() <= size_t(nSubItem))
    {
        return 0;
    }

    auto maxSize = min(rowData.colData[nSubItem].text.GetLength(), nLen);
    _tcsncpy_s(lpszText, nLen, rowData.colData[nSubItem].text.GetString(), rowData.colData[nSubItem].text.GetLength());
    return maxSize;
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
    if (nSubItem < 0 || nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    return SetMultilineText(nItem, lpszText, nSubItem);
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::SetItemData(int nItem, DWORD_PTR dwData)
{
    if (nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    m_rowData[nItem].dwData = dwData;
    return TRUE;
}
//---------------------------------------------------------------------------
DWORD_PTR CD2MultiLineListCtrl::GetItemData(int nItem) const
{
    if (nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return 0;
    }

    return m_rowData[nItem].dwData;
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::EnsureVisible(int nItem, BOOL bPartialOK)
{
    if (nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    auto& rowData = m_rowData[nItem];
    if (bPartialOK || rowData.numRows <= 1)
    {
        return __super::EnsureVisible(int(rowData.startRow), bPartialOK);
    }

    __super::EnsureVisible(int(rowData.startRow + rowData.numRows - 1), bPartialOK);
    return __super::EnsureVisible(int(rowData.startRow), bPartialOK);
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::RedrawItems(int nFirst, int nLast)
{
    if (m_rowData.empty() || (nFirst > nLast))
    {
        return FALSE;
    }

    if (nFirst < 0)
    {
        nFirst = 0;
    }

    if (nLast >= (int)m_rowData.size())
    {
        nLast = int(m_rowData.size() - 1);
    }

    auto& rowDataFirst = m_rowData[nFirst];
    auto& rowDataLast = m_rowData[nLast];
    nFirst = int(rowDataFirst.startRow);
    nLast = int(rowDataLast.startRow + rowDataLast.numRows - 1);
    return __super::RedrawItems(nFirst, nLast);
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::Update(int nItem)
{
    if (nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    auto& rowData = m_rowData[nItem];
    nItem = int(rowData.startRow);
    int nLastItem = int(nItem + (rowData.numRows - 1));
    BOOL bRet = TRUE;
    for (auto i = nItem; i <= nLastItem; ++i)
    {
        if (!__super::Update(nItem))
        {
            bRet = false;
        }
    }

    return bRet;
}
//---------------------------------------------------------------------------
void CD2MultiLineListCtrl::AdjustColumnWidths()
{
    SetRedraw(FALSE);
    CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
    int nColumnCount = pHeaderCtrl->GetItemCount();
    for (int i = 0; i < nColumnCount; i++)
    {
        SetColumnWidth(i, LVSCW_AUTOSIZE);
        int nColumnWidth = GetColumnWidth(i);
        SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
        int nHeaderWidth = GetColumnWidth(i);
        SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
    }
    SetRedraw(TRUE);
}
//---------------------------------------------------------------------------
BOOL CD2MultiLineListCtrl::SetMultilineText(int nItem, LPCTSTR lpszText, int nSubItem)
{
    if (nItem < 0 || nItem >= (int)m_rowData.size())
    {
        return FALSE;
    }

    BOOL bInsert = (nSubItem < 0) ? true : false;

    size_t colCount = 1;
    CHeaderCtrl* pHeader = GetHeaderCtrl();
    if (pHeader != nullptr)
    {
        auto headerCount = pHeader->GetItemCount();
        if (headerCount > 0)
        {
            colCount = size_t(headerCount);
        }
    }

    if (nSubItem >= (int)colCount)
    {
        return FALSE;
    }

    auto& rowData = m_rowData[nItem];
    auto& colData = rowData.colData[bInsert ? 0 : nSubItem];
    colData.text = lpszText;

    CString str(lpszText);
    str.Replace(_T("\r\n"), _T("\n"));
    str.Replace(_T("\r"), _T("\n"));

    CString s;
    int pos = -1;
    std::vector<CString> strings;
    while (!str.IsEmpty())
    {
        pos = str.Find('\n');
        if (pos == -1)
        {
            s = str;
            str.Empty();
        }
        else
        {
            s = str.Left(pos);
            str = str.Right(str.GetLength() - pos - 1);
        }
        strings.push_back(s);
    }

    auto row = int(rowData.startRow);
    auto existingNumLines = rowData.numRows;
    auto existingNumColLines = colData.numRows;
    if (strings.empty())
    {
        colData.numRows = 1;
        size_t numRows = 0;
        for (const auto& col : rowData.colData)
        {
            numRows = max(numRows, col.numRows);
        }
        rowData.numRows = numRows;

        if (!__super::SetItemText(row, nSubItem, _T("")))
        {
            return FALSE;
        }

        // make sure the other lines are blank
        for (size_t i = 1; i < existingNumColLines; ++i)
        {
            __super::SetItemText(int(row + i), nSubItem, _T(""));
        }

        for (auto i = rowData.numRows; i < existingNumLines; ++i)
        {
            __super::DeleteItem(int(row + rowData.numRows));

            // adjust row offsets
            for (size_t j = row + 1; j < m_rowData.size(); ++j)
            {
                --m_rowData[j].startRow;
            }
        }
        return TRUE;
    }

    size_t numLines = strings.size();
    colData.numRows = numLines;
    size_t line = 1;
    int newRowNum = row;
    for (const auto& lineStr : strings)
    {
        if (line > existingNumLines)
        {
            if (bInsert || (nSubItem == 0))
            {
                auto idx = __super::InsertItem(newRowNum, lineStr);
                if (idx == -1)
                {
                    return FALSE;
                }
            }
            else
            {
                auto idx = __super::InsertItem(newRowNum, _T(""));
                if (idx == -1)
                {
                    return FALSE;
                }

                if (!__super::SetItemText(newRowNum, nSubItem, lineStr))
                {
                    return FALSE;
                }
            }
            __super::SetItemData(newRowNum, -std::int64_t(line - 1));

            // adjust row offsets
            for (size_t i = row + 1; i < m_rowData.size(); ++i)
            {
                ++m_rowData[i].startRow;
            }
            ++rowData.numRows;
        }
        else if (bInsert)
        {
            if (__super::SetItemText(newRowNum, 0, lineStr) == -1)
            {
                return FALSE;
            }
        }
        else
        {
            if (__super::SetItemText(newRowNum, nSubItem, lineStr) == -1)
            {
                return FALSE;
            }
        }

        ++line;
        ++newRowNum;
    }

    // make sure the other lines are blank
    for (auto i = line; i < existingNumColLines; ++i)
    {
        __super::SetItemText(int(row + i), nSubItem, _T(""));
    }

    size_t numRows = 0;
    for (const auto& col : rowData.colData)
    {
        numRows = max(numRows, col.numRows);
    }
    rowData.numRows = numRows;

    for (auto i = rowData.numRows; i < existingNumLines; ++i)
    {
        __super::DeleteItem(int(row + rowData.numRows));

        // adjust row offsets
        for (size_t j = row + 1; j < m_rowData.size(); ++j)
        {
            --m_rowData[j].startRow;
        }
    }
    return TRUE;
}
//---------------------------------------------------------------------------
void CD2MultiLineListCtrl::OnKillFocus(CWnd* pNewWnd)
{
    __super::OnKillFocus(pNewWnd);

    auto posSelection = GetFirstSelectedItemPosition();
    if (posSelection)
    {
        CRect clean;
        CRect bounds;
        auto rowPos = GetNextSelectedItem(posSelection);
        if (rowPos >= 0)
        {
            // make sure to invalidate all rows
            InvalidateItemRect(rowPos, FALSE);
        }
    }
}
//---------------------------------------------------------------------------
void CD2MultiLineListCtrl::OnSetFocus(CWnd* pOldWnd)
{
    CListCtrl::OnSetFocus(pOldWnd);

    auto posSelection = GetFirstSelectedItemPosition();
    if (posSelection)
    {
        CRect clean;
        CRect bounds;
        auto rowPos = GetNextSelectedItem(posSelection);
        if (rowPos >= 0)
        {
            // make sure to invalidate all rows
            InvalidateItemRect(rowPos, FALSE);
        }
    }
}
//---------------------------------------------------------------------------
