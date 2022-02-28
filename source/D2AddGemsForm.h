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

//---------------------------------------------------------------------------
#include "D2MainForm.h"
#include "D2ItemsForm.h"

//---------------------------------------------------------------------------
class CD2AddGemsForm : public CDialogEx
{
	DECLARE_DYNAMIC(CD2AddGemsForm)

public:
	CD2AddGemsForm(CD2MainForm& form);
    CD2AddGemsForm(CD2ItemsForm& form, d2ce::Item* itemPtr = nullptr);
	virtual ~CD2AddGemsForm();

// Dialog Data
	enum { IDD = IDD_ADD_GPS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    afx_msg void OnBnClickedAdd();
    afx_msg void OnBnClickedFill();
	DECLARE_MESSAGE_MAP()

private:
    CD2MainForm& MainForm;
    std::map<size_t, std::uint64_t> GemIdxMap;
    std::map<std::uint64_t, size_t> NumGemMap;
    CD2ItemsForm* ItemsFormPtr = nullptr;
    d2ce::Item * ItemPtr = nullptr;

public:
    virtual BOOL OnInitDialog();
};
//---------------------------------------------------------------------------
