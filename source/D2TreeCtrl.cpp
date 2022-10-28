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
#include "D2TreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------------------------------------------------------
// CD2TreeCtrl
IMPLEMENT_DYNAMIC(CD2TreeCtrl, CTreeCtrl)

CD2TreeCtrl::CD2TreeCtrl()
{
}
//---------------------------------------------------------------------------
CD2TreeCtrl::~CD2TreeCtrl()
{
}
//---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CD2TreeCtrl, CTreeCtrl)
    ON_WM_CREATE()
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
HTREEITEM CD2TreeCtrl::FindByName(HTREEITEM hParent, LPCTSTR nodeName)
{
    // Retrieve a child for item <parent> by comparing names. If not found return TVI_ROOT
    for (HTREEITEM hItem = GetChildItem(hParent); hItem != NULL; hItem = GetNextItem(hItem, TVGN_NEXT))
    {
        if (GetItemText(hItem).Compare(nodeName) == 0)
        {
            return hItem;
        }
    }

    return TVI_ROOT;
}
//---------------------------------------------------------------------------
void CD2TreeCtrl::BuildSelectionPath(D2TreeCtrlPath& selection)
{
    BuildPath(GetSelectedItem(), selection);
}
//---------------------------------------------------------------------------
void CD2TreeCtrl::BuildPath(HTREEITEM hItem, D2TreeCtrlPath& selection)
{
    for (; hItem != 0 && hItem != TVI_ROOT; hItem = GetParentItem(hItem))
    {
        D2TreeCtrlPathElement pathElement;
        pathElement.text = GetItemText(hItem);

        UINT state = GetItemState(hItem, TVIS_SELECTED | TVIS_EXPANDED );
        if ((state & TVIS_EXPANDED) != 0)
        {
            pathElement.isExpanded = true;
        }

        if ((state & TVIS_SELECTED) != 0)
        {
            pathElement.isSelected = true;
        }

        selection.push_front(pathElement);
    }
}
//---------------------------------------------------------------------------
void CD2TreeCtrl::OpenPath(const D2TreeCtrlPath& selection)
{
    HTREEITEM hParentItem = TVI_ROOT;
    HTREEITEM hItem = TVI_ROOT;
    for (const auto& item : selection)
    {
        hParentItem = hItem;
        hItem = FindByName(hParentItem, item.text);
        if (TVI_ROOT == hItem || 0 == hItem)
        {
            return;
        }

        EnsureVisible(hItem);
        if (item.isSelected)
        {
            SelectItem(hItem);
        }

        if (item.isExpanded)
        {
            Expand(hItem, TVE_EXPAND);
        }
    }
}
//---------------------------------------------------------------------------
void CD2TreeCtrl::PreSubclassWindow()
{
    __super::PreSubclassWindow();

    ::SetWindowTheme(this->GetSafeHwnd(), L"Explorer", nullptr);
}
//---------------------------------------------------------------------------
// CD2AddGemsForm message handlers
//---------------------------------------------------------------------------
int CD2TreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (__super::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    ::SetWindowTheme(this->GetSafeHwnd(), L"Explorer", nullptr);
    return 0;
}
//---------------------------------------------------------------------------
