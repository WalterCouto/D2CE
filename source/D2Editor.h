/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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

#ifndef __AFXWIN_H__
#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CD2EditorApp:
// See D2Editor.cpp for the implementation of this class
//

class CD2EditorApp : public CWinAppEx
{
public:
    CD2EditorApp();

    // Overrides
public:
    virtual BOOL InitInstance();

    // Implementation

    DECLARE_MESSAGE_MAP()

protected:
    HACCEL m_haccel = NULL;
public:
    virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
    virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
};
//---------------------------------------------------------------------------
extern CD2EditorApp theApp;
//---------------------------------------------------------------------------
