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
#include "D2SharedStashForm.h"
#include "D2TreeCtrl.h"

//---------------------------------------------------------------------------
class CD2AddGemsForm : public CDialogEx, public CD2ItemTooltipCallback
{
	DECLARE_DYNAMIC(CD2AddGemsForm)

public:
	CD2AddGemsForm(CD2MainForm& form);
    CD2AddGemsForm(CD2ItemsForm& form);
    CD2AddGemsForm(CD2SharedStashForm& form);
	virtual ~CD2AddGemsForm();

// Dialog Data
	enum { IDD = IDD_ADD_GPS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnBnClickedAdd();
    afx_msg void OnBnClickedFill();
    afx_msg void OnTvnSelchangedItemtree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkItemtree(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

private:
    void InitTree();

    const d2ce::Item* GetSelectedItem() const override;

protected:
    const d2ce::Character* GetCharacterInfo() const override;

private:
    CD2ItemInfoStatic ItemTooltipBox;
    CD2MainForm& MainForm;
    std::map<size_t, std::uint64_t> GemIdxMap;
    std::map<std::uint64_t, size_t> NumGemMap;
    CD2ItemsForm* ItemsFormPtr = nullptr;
    CD2SharedStashForm* SharedStashFormPtr = nullptr;
    mutable CD2TreeCtrl ItemTree;
    d2ce::Item * ItemPtr = nullptr;

    std::map<HTREEITEM, d2ce::AvailableItemType> AvailableItemTypes;
    mutable std::list<d2ce::Item> AvailableItems;

public:
};
//---------------------------------------------------------------------------
