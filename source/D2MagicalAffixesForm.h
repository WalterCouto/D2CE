/*
    Diablo II Character Editor
    Copyright (C) 2021-2022 Walter Couto

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; If not, see <http://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
#include "D2NewItemForm.h"

//---------------------------------------------------------------------------
class CD2MagicalAffixesForm : public CDialogEx, public CD2ItemTooltipCallback
{
	DECLARE_DYNAMIC(CD2MagicalAffixesForm)

public:
	CD2MagicalAffixesForm(CD2NewItemForm& form);   // standard constructor
	virtual ~CD2MagicalAffixesForm();

	enum { IDD = IDD_MAGICAL_AFFIXES_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedGambleButton();
    afx_msg void OnBnClickedPrevButton();
    afx_msg void OnBnClickedNextButton();
    afx_msg void OnCbnSelchangePrefix1Combo();
    afx_msg void OnCbnSelchangeSuffix1Combo();

	DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

private:
    void InitAffixes();
    void SyncAffixesCombo();
    void SyncAffixes();
    void UpdateCurrentAttribs();

    const d2ce::Item* GetSelectedItem() const override;

protected:
    const d2ce::Character* GetCharacterInfo() const override;

private:
    CD2ItemInfoStatic ItemTooltipBox;
    CD2NewItemForm& NewItemForm;
    CStatic PrefixStatic;
    CComboBox  Prefix;
    CStatic SuffixStatic;
    CComboBox  Suffix;
    d2ce::ItemCreateParams CreateParams;
    d2ce::MagicalAffixes CurrentAffixes;
    std::uint32_t CurrentDWBCode = 0;
    d2ce::Item CurrentItem;

    BOOL UseDWBCode = FALSE;
    std::deque<d2ce::MagicalAffixes> GeneratedAffixes;
    std::deque<std::uint32_t> GeneratedDWBCode;
    size_t CurrentAffixesIndex = 0;
};
//---------------------------------------------------------------------------
