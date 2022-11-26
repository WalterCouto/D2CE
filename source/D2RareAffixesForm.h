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
class CD2RareAffixesForm : public CDialogEx, public CD2ItemTooltipCallback
{
    DECLARE_DYNAMIC(CD2RareAffixesForm)

    struct AffixControl
    {
        CStatic PrefixStatic;
        CComboBox Prefix;
        CStatic SuffixStatic;
        CComboBox Suffix;
    };

    struct AffixChoice
    {
        std::uint16_t prefix = 0;
        std::uint16_t prefixGroup = 0;
        std::uint16_t suffix = 0;
        std::uint16_t suffixGroup = 0;
    };

public:
    CD2RareAffixesForm(CD2NewItemForm& form);   // standard constructor
    CD2RareAffixesForm(CD2NewItemForm& form, std::uint16_t recipieId);   // standard constructor
    virtual ~CD2RareAffixesForm();

    enum { IDD = IDD_RARE_AFFIXES_DIALOG, IDD_V100 = IDD_RARE_AFFIXES_V100_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedGambleButton();
    afx_msg void OnBnClickedPrevButton();
    afx_msg void OnBnClickedNextButton();
    afx_msg void OnCbnSelchangeNamePrefixCombo();
    afx_msg void OnCbnSelchangeNameSuffixCombo();
    afx_msg void OnCbnSelchangePrefix1Combo();
    afx_msg void OnCbnSelchangeSuffix1Combo();
    afx_msg void OnCbnSelchangePrefix2Combo();
    afx_msg void OnCbnSelchangeSuffix2Combo();
    afx_msg void OnCbnSelchangePrefix3Combo();
    afx_msg void OnCbnSelchangeSuffix3Combo();

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

private:
    void InitAffixes();
    void SyncNameAffixes();
    void SyncAffixesChoices();
    void SyncAffixes();
    void UpdatePrefixChoices();
    void UpdateSuffixChoices();
    void UpdateAffixChoices();
    void UpdateCurrentAffixesValues();
    void UpdateCurrentAttribs();
    void HandleCbnSelchangePrefixCombo(CComboBox& combo, AffixChoice& affix);
    void HandleCbnSelchangeSuffixCombo(CComboBox& combo, AffixChoice& affix);

    const d2ce::Item* GetSelectedItem() const override;

protected:
    const d2ce::Character* GetCharacterInfo() const override;

private:
    CD2ItemInfoStatic ItemTooltipBox;
    CD2NewItemForm& NewItemForm;
    CStatic NamePrefixStatic;
    CComboBox NamePrefix;
    CStatic NameSuffixStatic;
    CComboBox NameSuffix;
    std::array<AffixControl, 3> AffixControls;

    d2ce::ItemCreateParams CreateParams;
    std::uint32_t CurrentDWBCode = 0;
    d2ce::Item CurrentItem;
    d2ce::RareAttributes CurrentAffixes;
    std::map<std::uint16_t, std::vector<std::uint16_t>> PrefixMap;
    std::map<std::uint16_t, std::vector<std::uint16_t>> SuffixMap;
    std::array<AffixChoice, 3> CurrentAffixChoices;
    std::uint32_t NumAllowedAffixes = 0ui32; // zero indicates not to check

    BOOL UseDWBCode = FALSE;
    std::deque<d2ce::RareAttributes> GeneratedAffixes;
    std::deque<std::uint32_t> GeneratedDWBCode;
    size_t CurrentAffixesIndex = 0;
};
//---------------------------------------------------------------------------
