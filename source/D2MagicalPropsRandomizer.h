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
#include "D2ItemsForm.h"
#include "D2SharedStashForm.h"
#include "D2ItemToolTipCtrl.h"
#include "D2TreeCtrl.h"
#include <list>

//---------------------------------------------------------------------------
class CD2MagicalPropsRandomizer : public CDialogEx, public CD2ItemTooltipCallback
{
	DECLARE_DYNAMIC(CD2MagicalPropsRandomizer)

public:
    CD2MagicalPropsRandomizer(CD2ItemsForm& form); // standard constructor
    CD2MagicalPropsRandomizer(CD2SharedStashForm& form);
	virtual ~CD2MagicalPropsRandomizer();

    // Dialog Data
	enum { IDD = IDD_HACK_PROPERTIES_V100_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedGambleButton();
    afx_msg void OnBnClickedPrevButton();
    afx_msg void OnBnClickedNextButton();

	DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

private:
    void UpdateCurrentAttribs();

    const d2ce::Item* GetSelectedItem() const override;

protected:
    const d2ce::Character* GetCharacterInfo() const override;

protected:
    CD2ItemInfoStatic ItemTooltipBox;
    CD2MainForm& MainForm;
    CD2ItemsForm* ItemsFormPtr = nullptr;
    CD2SharedStashForm* SharedStashFormPtr = nullptr;
    d2ce::Item* ItemPtr = nullptr;
    
    d2ce::ItemCreateParams CreateParams;
    std::uint32_t CurrentDWBCode = 0;
    d2ce::Item CurrentItem;
    std::deque<std::uint32_t> GeneratedDWBCode;
    size_t CurrentDWBCodeIndex = 0;
};
//---------------------------------------------------------------------------
