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

//---------------------------------------------------------------------------
#include "D2MainForm.h"

//---------------------------------------------------------------------------
class CD2GemsForm : public CDialogEx
{
	DECLARE_DYNAMIC(CD2GemsForm)

public:
	CD2GemsForm(CD2MainForm& form);
	virtual ~CD2GemsForm();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GPS_CONVERTER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    afx_msg void OnBnClickedConvert();
	DECLARE_MESSAGE_MAP()

private:
    CD2MainForm& MainForm;
    std::map<size_t, std::uint64_t> GemIdxMap;
    std::map<std::uint64_t, size_t> NumGemMap;

public:
    virtual BOOL OnInitDialog();
};
//---------------------------------------------------------------------------
