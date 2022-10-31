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
#include "D2NewItemForm.h"

//---------------------------------------------------------------------------
class CD2EarAttributesForm : public CDialogEx
{
	DECLARE_DYNAMIC(CD2EarAttributesForm)

public:
    CD2EarAttributesForm(CD2NewItemForm& form);
	virtual ~CD2EarAttributesForm();

// Dialog Data
	enum { IDD = IDD_EAR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();

	DECLARE_MESSAGE_MAP()

private:
    void UpdateClassDisplay();
    std::string ToStdString(const CWnd* Sender) const; // UTF-8
    CString ToText(const CWnd* Sender) const;          // UTF-16
    CStringA ToTextA(const CWnd* Sender) const;        // ANSI
    void SetText(CWnd* Sender, const std::string& newValue); // UTF-8
    void SetUTF8Text(CWnd* Sender, const char* newValue);    // UTF-8
    void SetText(CWnd* Sender, const char* newValue);        // ANSI
    void SetText(CWnd* Sender, const wchar_t* newValue);     // UTF-16
    std::uint32_t ToInt(const CWnd* Sender) const;
    void SetInt(CWnd* Sender, std::uint32_t newValue);

private:
    CD2MainForm& MainForm;
    CD2NewItemForm* NewItemFormPtr = nullptr;
    d2ce::Item* ItemPtr = nullptr;
    d2ce::EarAttributes EarAttrib;
    CCharNameEdit CharName;
    CComboBox CharClass;
    CEdit CharLevel;
};
