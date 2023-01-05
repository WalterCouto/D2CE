/*
    Diablo II Character Editor
    Copyright (C) 2022-2023 Walter Couto

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
#include "D2ItemsForm.h"
#include "D2SharedStashForm.h"
#include "D2MercenaryForm.h"
#include "D2NewItemForm.h"
#include "D2ItemToolTipCtrl.h"

//---------------------------------------------------------------------------
class CD2RunewordForm : public CDialogEx, public CD2ItemTooltipCallback
{
    DECLARE_DYNAMIC(CD2RunewordForm)

public:
    CD2RunewordForm(CD2ItemsForm& form);
    CD2RunewordForm(CD2MercenaryForm& form);
    CD2RunewordForm(CD2SharedStashForm& form);
    CD2RunewordForm(CD2NewItemForm& form);
    virtual ~CD2RunewordForm();

    // Dialog Data
    enum { IDD = IDD_RUNEWORD_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnCbnSelchangeRunewordCombo();

    DECLARE_MESSAGE_MAP()

private:
    void InitRunewords();

    const d2ce::Item* GetSelectedItem() const override;

protected:
    const d2ce::Character* GetCharacterInfo() const override;

private:
    CD2ItemInfoStatic ItemTooltipBox;
    CD2MainForm& MainForm;
    CD2ItemsForm* ItemsFormPtr = nullptr;
    CD2MercenaryForm* MercenaryFormPtr = nullptr;
    CD2SharedStashForm* SharedStashFormPtr = nullptr;
    CD2NewItemForm* NewItemFormPtr = nullptr;
    d2ce::Item* ItemPtr = nullptr;
    CStatic RunewordStatic;
    CComboBox  Runeword;
    d2ce::Item CurrentItem;
};
//---------------------------------------------------------------------------
