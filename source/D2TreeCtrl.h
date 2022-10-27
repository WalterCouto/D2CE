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

#pragma once

//---------------------------------------------------------------------------
#include <afxcmn.h>
#include <deque>

struct D2TreeCtrlPathElement
{
    D2TreeCtrlPathElement() : isSelected(false), isExpanded(false) { }
    CString text;
    bool isSelected;
    bool isExpanded;
};

typedef std::deque<D2TreeCtrlPathElement> D2TreeCtrlPath;

//---------------------------------------------------------------------------
class CD2TreeCtrl :  public CTreeCtrl
{
public:
    CD2TreeCtrl();
    ~CD2TreeCtrl();

protected:
    DECLARE_MESSAGE_MAP()

public:
    HTREEITEM FindByName(HTREEITEM hParent, LPCTSTR nodeName);
    void  BuildSelectionPath(D2TreeCtrlPath& selection);
    void  BuildPath(HTREEITEM hItem, D2TreeCtrlPath& selection);
    void  OpenPath(const D2TreeCtrlPath& selection);

};
//---------------------------------------------------------------------------
