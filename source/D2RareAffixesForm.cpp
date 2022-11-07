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

#include "pch.h"
#include "D2Editor.h"
#include "D2RareAffixesForm.h"
#include "D2NewItemForm.h"
#include "d2ce/helpers/ItemHelpers.h"
#include "d2ce/item.h"
#include <utf8/utf8.h>
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------------------------------------------------------
namespace d2ce
{
    namespace ItemHelpers
    {
        std::uint32_t generarateRandomDW(std::uint32_t itemDwbCode = 0, std::uint16_t level = 0);
        const std::string& getMagicalPrefixFromId(std::uint16_t id);
        const std::string& getMagicalSuffixFromId(std::uint16_t id);
        std::uint16_t getMagicalPrefixGroupFromId(std::uint16_t id);
        std::uint16_t getMagicalSuffixGroupFromId(std::uint16_t id);
        bool generateRareOrCraftedAffixes(RareOrCraftedCachev100& cache, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = false);
        const std::string& getRareNameFromId(std::uint16_t id);
        const std::string& getRareIndexFromId(std::uint16_t id);
    }
}

//---------------------------------------------------------------------------
// CD2RareAffixesForm dialog

IMPLEMENT_DYNAMIC(CD2RareAffixesForm, CDialogEx)

//---------------------------------------------------------------------------
CD2RareAffixesForm::CD2RareAffixesForm(CD2NewItemForm& form)
    : CDialogEx(form.GetCharacterInfo()->getVersion() < d2ce::EnumCharVersion::v107 ? CD2RareAffixesForm::IDD_V100 : CD2RareAffixesForm::IDD, &form), NewItemForm(form), SimpleDialog(form.GetCharacterInfo()->getVersion() < d2ce::EnumCharVersion::v107 ? TRUE : FALSE)
{
    auto pItem = form.GetCreatedItem();
    if (pItem != nullptr)
    {
        auto charInfo = form.GetCharacterInfo();
        d2ce::EnumDifficulty diff = charInfo == nullptr ? d2ce::EnumDifficulty::Normal : charInfo->getTitleDifficulty();
        d2ce::EnumCharClass clazz = charInfo == nullptr ? d2ce::EnumCharClass::Amazon : charInfo->getClass();
        CreateParams = d2ce::ItemCreateParams(pItem->getVersion(), pItem->getItemTypeHelper(), diff, clazz, pItem->isExpansionItem());
        CurrentItem = *pItem;
        CurrentItem.makeRare();
        CurrentDWBCode = CurrentItem.getDWBCode();
        if (SimpleDialog)
        {
            CurrentDWBCodeIndex = 0;
            GeneratedDWBCode.push_back(CurrentDWBCode);
        }

        if (CurrentItem.isJewel() && CreateParams.itemVersion >= d2ce::EnumItemVersion::v109)
        {
            // Post-1.09, Rare jewels can have up to 4 total affixes
            NumAllowedAffixes = 4ui32;
        }
    }
}
//---------------------------------------------------------------------------
CD2RareAffixesForm::~CD2RareAffixesForm()
{
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    if (!SimpleDialog)
    {
        DDX_Control(pDX, IDC_RARE_PREFIX_STATIC, NamePrefixStatic);
        DDX_Control(pDX, IDC_RARE_PREFIX_COMBO, NamePrefix);
        DDX_Control(pDX, IDC_RARE_SUFFIX_STATIC, NameSuffixStatic);
        DDX_Control(pDX, IDC_RARE_SUFFIX_COMBO, NameSuffix);
        DDX_Control(pDX, IDC_PREFIX1_STATIC, AffixControls[0].PrefixStatic);
        DDX_Control(pDX, IDC_PREFIX1_COMBO, AffixControls[0].Prefix);
        DDX_Control(pDX, IDC_SUFFIX1_STATIC, AffixControls[0].SuffixStatic);
        DDX_Control(pDX, IDC_SUFFIX1_COMBO, AffixControls[0].Suffix);
        DDX_Control(pDX, IDC_PREFIX2_STATIC, AffixControls[1].PrefixStatic);
        DDX_Control(pDX, IDC_PREFIX2_COMBO, AffixControls[1].Prefix);
        DDX_Control(pDX, IDC_SUFFIX2_STATIC, AffixControls[1].SuffixStatic);
        DDX_Control(pDX, IDC_SUFFIX2_COMBO, AffixControls[1].Suffix);
        DDX_Control(pDX, IDC_PREFIX3_STATIC, AffixControls[2].PrefixStatic);
        DDX_Control(pDX, IDC_PREFIX3_COMBO, AffixControls[2].Prefix);
        DDX_Control(pDX, IDC_SUFFIX3_STATIC, AffixControls[2].SuffixStatic);
        DDX_Control(pDX, IDC_SUFFIX3_COMBO, AffixControls[2].Suffix);
    }
    DDX_Control(pDX, IDC_TOOLTIP_RECT, ItemTooltipBox);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2RareAffixesForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2RareAffixesForm::OnBnClickedOk)
    ON_CBN_SELCHANGE(IDC_RARE_PREFIX_COMBO, &CD2RareAffixesForm::OnCbnSelchangeNamePrefixCombo)
    ON_CBN_SELCHANGE(IDC_RARE_SUFFIX_COMBO, &CD2RareAffixesForm::OnCbnSelchangeNameSuffixCombo)
    ON_CBN_SELCHANGE(IDC_PREFIX1_COMBO, &CD2RareAffixesForm::OnCbnSelchangePrefix1Combo)
    ON_CBN_SELCHANGE(IDC_SUFFIX1_COMBO, &CD2RareAffixesForm::OnCbnSelchangeSuffix1Combo)
    ON_CBN_SELCHANGE(IDC_PREFIX2_COMBO, &CD2RareAffixesForm::OnCbnSelchangePrefix2Combo)
    ON_CBN_SELCHANGE(IDC_SUFFIX2_COMBO, &CD2RareAffixesForm::OnCbnSelchangeSuffix2Combo)
    ON_CBN_SELCHANGE(IDC_PREFIX3_COMBO, &CD2RareAffixesForm::OnCbnSelchangePrefix3Combo)
    ON_CBN_SELCHANGE(IDC_SUFFIX3_COMBO, &CD2RareAffixesForm::OnCbnSelchangeSuffix3Combo)
    ON_BN_CLICKED(IDC_GAMBLE_BUTTON, &CD2RareAffixesForm::OnBnClickedGambleButton)
    ON_BN_CLICKED(IDC_PREV_BUTTON, &CD2RareAffixesForm::OnBnClickedPrevButton)
    ON_BN_CLICKED(IDC_NEXT_BUTTON, &CD2RareAffixesForm::OnBnClickedNextButton)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2RareAffixesForm message handlers

//---------------------------------------------------------------------------
BOOL CD2RareAffixesForm::OnInitDialog()
{
    __super::OnInitDialog();

    {
        std::string strValue;
        std::u16string uText;
        CString text;
        CStringA textA;
        CWnd* pWnd = nullptr;
        if (d2ce::LocalizationHelpers::GetStringTxtValue("UICreate", strValue))
        {
            pWnd = GetDlgItem(IDOK);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("cancel", strValue))
        {
            pWnd = GetDlgItem(IDCANCEL);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("gamble", strValue))
        {
            pWnd = GetDlgItem(IDC_GAMBLE_BUTTON);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }
    }

    InitAffixes();
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnBnClickedOk()
{
    UpdateData(TRUE); // save results

    auto pItem = const_cast<d2ce::Item*>(NewItemForm.GetCreatedItem());
    if (pItem == nullptr)
    {
        // should not happen
        __super::OnOK();
        return;
    }

    pItem->swap(CurrentItem);
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnBnClickedGambleButton()
{
    // Generate the starting affixes
    CurrentDWBCode = d2ce::ItemHelpers::generarateRandomDW();
    if (SimpleDialog)
    {
        CurrentDWBCodeIndex = GeneratedDWBCode.size();
        GeneratedDWBCode.push_back(CurrentDWBCode);
    }
    
    SyncAffixes();
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnBnClickedPrevButton()
{
    if (!SimpleDialog)
    {
        return;
    }

    if (CurrentDWBCodeIndex > 0)
    {
        --CurrentDWBCodeIndex;
        if (CurrentDWBCodeIndex >= GeneratedDWBCode.size())
        {
            CurrentDWBCodeIndex = GeneratedDWBCode.size() - 1;
        }
        CurrentDWBCode = GeneratedDWBCode[CurrentDWBCodeIndex];
        UpdateCurrentAttribs();
    }
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnBnClickedNextButton()
{
    if (!SimpleDialog)
    {
        return;
    }

    if (CurrentDWBCodeIndex < (GeneratedDWBCode.size() - 1))
    {
        ++CurrentDWBCodeIndex;
        if (CurrentDWBCodeIndex >= GeneratedDWBCode.size())
        {
            CurrentDWBCodeIndex = GeneratedDWBCode.size() - 1;
        }
        CurrentDWBCode = GeneratedDWBCode[CurrentDWBCodeIndex];
        UpdateCurrentAttribs();
    }
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnCbnSelchangeNamePrefixCombo()
{
    UpdateData(TRUE); // save results
    CurrentAffixes.Id = std::uint16_t(NamePrefix.GetItemData(NamePrefix.GetCurSel()));
    CurrentAffixes.Name = d2ce::ItemHelpers::getRareNameFromId(CurrentAffixes.Id);
    CurrentAffixes.Index = d2ce::ItemHelpers::getRareIndexFromId(CurrentAffixes.Id);
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnCbnSelchangeNameSuffixCombo()
{
    UpdateData(TRUE); // save results
    CurrentAffixes.Id2 = std::uint16_t(NameSuffix.GetItemData(NameSuffix.GetCurSel()));
    CurrentAffixes.Name2 = d2ce::ItemHelpers::getRareNameFromId(CurrentAffixes.Id2);
    CurrentAffixes.Index2 = d2ce::ItemHelpers::getRareIndexFromId(CurrentAffixes.Id2);
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnCbnSelchangePrefix1Combo()
{
    UpdateData(TRUE); // save results
    HandleCbnSelchangePrefixCombo(AffixControls[0].Prefix, CurrentAffixChoices[0]);
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnCbnSelchangeSuffix1Combo()
{
    UpdateData(TRUE); // save results
    HandleCbnSelchangeSuffixCombo(AffixControls[0].Suffix, CurrentAffixChoices[0]);
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnCbnSelchangePrefix2Combo()
{
    UpdateData(TRUE); // save results
    HandleCbnSelchangePrefixCombo(AffixControls[1].Prefix, CurrentAffixChoices[1]);
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnCbnSelchangeSuffix2Combo()
{
    UpdateData(TRUE); // save results
    HandleCbnSelchangeSuffixCombo(AffixControls[1].Suffix, CurrentAffixChoices[1]);
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnCbnSelchangePrefix3Combo()
{
    UpdateData(TRUE); // save results
    HandleCbnSelchangePrefixCombo(AffixControls[2].Prefix, CurrentAffixChoices[2]);
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::OnCbnSelchangeSuffix3Combo()
{
    UpdateData(TRUE); // save results
    HandleCbnSelchangeSuffixCombo(AffixControls[2].Suffix, CurrentAffixChoices[2]);
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::HandleCbnSelchangePrefixCombo(CComboBox& combo, AffixChoice& affix)
{
    UpdateData(TRUE); // save results
    affix.prefix = std::uint16_t(combo.GetItemData(combo.GetCurSel()));
    auto oldGroup = affix.prefixGroup;
    affix.prefixGroup = d2ce::ItemHelpers::getMagicalPrefixGroupFromId(affix.prefix);
    if (oldGroup != affix.prefixGroup)
    {
        UpdatePrefixChoices();
    }

    UpdateCurrentAffixesValues();
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::HandleCbnSelchangeSuffixCombo(CComboBox& combo, AffixChoice& affix)
{
    UpdateData(TRUE); // save results
    affix.suffix = std::uint16_t(combo.GetItemData(combo.GetCurSel()));
    auto oldGroup = affix.suffixGroup;
    affix.suffixGroup = d2ce::ItemHelpers::getMagicalSuffixGroupFromId(affix.prefix);
    if (oldGroup != affix.suffixGroup)
    {
        UpdateSuffixChoices();
    }

    UpdateCurrentAffixesValues();
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::InitAffixes()
{
    // Generate the starting affixes
    d2ce::RareOrCraftedCachev100 generated_rare_affixes;
    switch (CurrentItem.getQuality())
    {
    case d2ce::EnumItemQuality::RARE:
    case d2ce::EnumItemQuality::CRAFT:
    case d2ce::EnumItemQuality::TEMPERED:
        CurrentItem.getRareOrCraftedAttributes(CurrentAffixes);
        break;

    default:
        if (d2ce::ItemHelpers::generateRareOrCraftedAffixes(generated_rare_affixes, CreateParams, CurrentItem.getLevel(), CurrentDWBCode, true))
        {
            CurrentAffixes.Id = generated_rare_affixes.Id;
            CurrentAffixes.Name = generated_rare_affixes.Name;
            CurrentAffixes.Index = generated_rare_affixes.Index;
            CurrentAffixes.Id2 = generated_rare_affixes.Id2;
            CurrentAffixes.Name2 = generated_rare_affixes.Name2;
            CurrentAffixes.Index2 = generated_rare_affixes.Index2;
            for (auto& item : generated_rare_affixes.affixes)
            {
                CurrentAffixes.Affixes.push_back(item.Affixes);
            }
        }
        break;
    }


    if (!SimpleDialog)
    {
        CurrentAffixChoices.fill(AffixChoice());
        auto iterCurAffix = CurrentAffixChoices.begin();
        for (const auto& affix : CurrentAffixes.Affixes)
        {
            auto& currAffix = *iterCurAffix;
            ++iterCurAffix;
            currAffix.prefix = affix.PrefixId;
            currAffix.prefixGroup = d2ce::ItemHelpers::getMagicalPrefixGroupFromId(affix.PrefixId);
            currAffix.suffix = affix.SuffixId;
            currAffix.suffixGroup = d2ce::ItemHelpers::getMagicalSuffixGroupFromId(affix.SuffixId);
        }

        // Check if the prefix and/or suffix is allowed
        std::vector<std::uint16_t> prefixes;
        std::vector<std::uint16_t> suffixes;
        if (!CurrentItem.getPossibleRareAffixes(prefixes, suffixes))
        {
            EndDialog(IDCANCEL);
            return;
        }

        int idx = -1;
        int prefixIdx = -1;
        std::u16string uText;
        CString strText;
        for (const auto& prefix : prefixes)
        {
            uText = utf8::utf8to16(d2ce::ItemHelpers::getRareNameFromId(prefix));
            strText = reinterpret_cast<LPCWSTR>(uText.c_str());
            idx = NamePrefix.AddString(strText);
            if (idx >= 0)
            {
                if (CurrentAffixes.Id == prefix)
                {
                    prefixIdx = idx;
                }
                NamePrefix.SetItemData(idx, prefix);
            }
        }

        NamePrefix.SetCurSel(prefixIdx);
        CurrentAffixes.Id = std::uint16_t(NamePrefix.GetItemData(prefixIdx));
        CurrentAffixes.Name = d2ce::ItemHelpers::getRareNameFromId(CurrentAffixes.Id);
        CurrentAffixes.Index = d2ce::ItemHelpers::getRareIndexFromId(CurrentAffixes.Id);

        int suffixIdx = -1;
        for (const auto& suffix : suffixes)
        {
            uText = utf8::utf8to16(d2ce::ItemHelpers::getRareNameFromId(suffix));
            strText = reinterpret_cast<LPCWSTR>(uText.c_str());
            idx = NameSuffix.AddString(strText);
            if (idx >= 0)
            {
                if (CurrentAffixes.Id2 == suffix)
                {
                    suffixIdx = idx;
                }
                NameSuffix.SetItemData(idx, suffix);
            }
        }

        NameSuffix.SetCurSel(suffixIdx);
        CurrentAffixes.Id2 = std::uint16_t(NameSuffix.GetItemData(suffixIdx));
        CurrentAffixes.Name2 = d2ce::ItemHelpers::getRareNameFromId(CurrentAffixes.Id2);
        CurrentAffixes.Index2 = d2ce::ItemHelpers::getRareIndexFromId(CurrentAffixes.Id2);

        // Check if the prefix and/or suffix is allowed
        if (!CurrentItem.getPossibleMagicalAffixes(PrefixMap, SuffixMap))
        {
            EndDialog(IDCANCEL);
            return;
        }
        UpdateAffixChoices();
    }
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::SyncNameAffixes()
{
    if (SimpleDialog)
    {
        // thise controls do not exist
        return;
    }

    CWaitCursor wait;
    bool bFound = false;
    auto idx_end = NamePrefix.GetCount();
    for (int idx = 0; idx < idx_end; ++idx)
    {
        if (std::uint16_t(NamePrefix.GetItemData(idx)) == CurrentAffixes.Id)
        {
            bFound = true;
            NamePrefix.SetCurSel(idx);
            break;
        }
    }

    if (!bFound)
    {
        CurrentAffixes.Id = std::uint16_t(NamePrefix.GetItemData(NamePrefix.GetCurSel()));
        CurrentAffixes.Name = d2ce::ItemHelpers::getRareNameFromId(CurrentAffixes.Id);
        CurrentAffixes.Index = d2ce::ItemHelpers::getRareIndexFromId(CurrentAffixes.Id);
    }

    bFound = false;
    idx_end = NameSuffix.GetCount();
    for (int idx = 0; idx < idx_end; ++idx)
    {
        if (std::uint16_t(NameSuffix.GetItemData(idx)) == CurrentAffixes.Id2)
        {
            bFound = true;
            NameSuffix.SetCurSel(idx);
            break;
        }
    }

    if (!bFound)
    {
        CurrentAffixes.Id2 = std::uint16_t(NameSuffix.GetItemData(NameSuffix.GetCurSel()));
        CurrentAffixes.Name2 = d2ce::ItemHelpers::getRareNameFromId(CurrentAffixes.Id2);
        CurrentAffixes.Index2 = d2ce::ItemHelpers::getRareIndexFromId(CurrentAffixes.Id2);
    }
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::SyncAffixes()
{
    {
        CWaitCursor wait;
        d2ce::RareOrCraftedCachev100 generated_rare_affixes;
        if (d2ce::ItemHelpers::generateRareOrCraftedAffixes(generated_rare_affixes, CreateParams, CurrentItem.getLevel(), CurrentDWBCode, true))
        {
            CurrentAffixes.clear();
            CurrentAffixes.Id = generated_rare_affixes.Id;
            CurrentAffixes.Name = generated_rare_affixes.Name;
            CurrentAffixes.Index = generated_rare_affixes.Name;
            CurrentAffixes.Id2 = generated_rare_affixes.Id2;
            CurrentAffixes.Name2 = generated_rare_affixes.Name2;
            CurrentAffixes.Index2 = generated_rare_affixes.Name;
            for (auto& item : generated_rare_affixes.affixes)
            {
                CurrentAffixes.Affixes.push_back(item.Affixes);
            }
        }

        CurrentAffixChoices.fill(AffixChoice());
        auto iterCurAffix = CurrentAffixChoices.begin();
        for (const auto& affix : CurrentAffixes.Affixes)
        {
            auto& currAffix = *iterCurAffix;
            ++iterCurAffix;
            currAffix.prefix = affix.PrefixId;
            currAffix.prefixGroup = d2ce::ItemHelpers::getMagicalPrefixGroupFromId(affix.PrefixId);
            currAffix.suffix = affix.SuffixId;
            currAffix.suffixGroup = d2ce::ItemHelpers::getMagicalSuffixGroupFromId(affix.SuffixId);
        }
    }
    SyncNameAffixes();
    UpdateAffixChoices();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::UpdatePrefixChoices()
{
    if (SimpleDialog)
    {
        // thise controls do not exist
        return;
    }

    CWaitCursor wait;
    std::u16string uText;
    CString strText;
    bool badGroup = false;
    auto iterControls = AffixControls.begin();
    if(CurrentItem.isJewel())
    for (auto& affix : CurrentAffixChoices)
    {
        auto& affixControls = *iterControls;
        ++iterControls;

        auto& prefixCombo = affixControls.Prefix;
        prefixCombo.ResetContent();

        int selectedAffix = -1;
        auto affixIdx = prefixCombo.AddString(_T(""));
        if (affixIdx >= 0)
        {
            selectedAffix = affixIdx;
            prefixCombo.SetItemData(affixIdx, 0);
        }

        for (const auto& groupAffix : PrefixMap)
        {
            badGroup = false;
            for (auto& check : CurrentAffixChoices)
            {
                if ((&affix != &check) && check.prefixGroup == groupAffix.first)
                {
                    badGroup = true;
                    break;
                }
            }

            if (badGroup)
            {
                continue;
            }

            for (const auto& affixId : groupAffix.second)
            {
                uText = utf8::utf8to16(d2ce::ItemHelpers::getMagicalPrefixFromId(affixId));
                strText = reinterpret_cast<LPCWSTR>(uText.c_str());
                affixIdx = prefixCombo.AddString(strText);
                if (affixIdx >= 0)
                {
                    if (affix.prefix == affixId)
                    {
                        selectedAffix = affixIdx;
                    }
                    prefixCombo.SetItemData(affixIdx, affixId);
                }
            }
        }

        prefixCombo.SetCurSel(selectedAffix);
        affix.prefix = std::uint16_t(prefixCombo.GetItemData(prefixCombo.GetCurSel()));
        affix.prefixGroup = d2ce::ItemHelpers::getMagicalPrefixGroupFromId(affix.prefix);
    }
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::UpdateSuffixChoices()
{
    if (SimpleDialog)
    {
        // thise controls do not exist
        return;
    }

    CWaitCursor wait;
    std::u16string uText;
    CString strText;
    bool badGroup = false;
    auto iterControls = AffixControls.begin();
    for (auto& affix : CurrentAffixChoices)
    {
        auto& affixControls = *iterControls;
        ++iterControls;
        auto& suffixCombo = affixControls.Suffix;
        suffixCombo.ResetContent();

        auto selectedAffix = -1;
        auto affixIdx = suffixCombo.AddString(_T(""));
        if (affixIdx >= 0)
        {
            selectedAffix = affixIdx;
            suffixCombo.SetItemData(affixIdx, 0);
        }

        for (const auto& groupAffix : SuffixMap)
        {
            badGroup = false;
            for (auto& check : CurrentAffixChoices)
            {
                if ((&affix != &check) && check.suffixGroup == groupAffix.first)
                {
                    badGroup = true;
                    break;
                }
            }

            if (badGroup)
            {
                continue;
            }

            for (const auto& affixId : groupAffix.second)
            {
                uText = utf8::utf8to16(d2ce::ItemHelpers::getMagicalSuffixFromId(affixId));
                strText = reinterpret_cast<LPCWSTR>(uText.c_str());
                affixIdx = suffixCombo.AddString(strText);
                if (affixIdx >= 0)
                {
                    if (affix.suffix == affixId)
                    {
                        selectedAffix = affixIdx;
                    }
                    suffixCombo.SetItemData(affixIdx, affixId);
                }
            }
        }

        suffixCombo.SetCurSel(selectedAffix);
        affix.suffix = std::uint16_t(suffixCombo.GetItemData(suffixCombo.GetCurSel()));
        affix.suffixGroup = d2ce::ItemHelpers::getMagicalSuffixGroupFromId(affix.suffix);
    }
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::UpdateAffixChoices()
{
    UpdatePrefixChoices();
    UpdateSuffixChoices();
    UpdateCurrentAffixesValues();
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::UpdateCurrentAffixesValues()
{
    if (SimpleDialog)
    {
        // thise controls do not exist
        return;
    }

    std::uint32_t count = 0;
    CurrentAffixes.Affixes.clear();
    for (auto& affix : CurrentAffixChoices)
    {
        if (affix.prefix != 0 || affix.suffix != 0)
        {
            d2ce::MagicalAffixes affixes;
            if (affix.prefix != 0)
            {
                ++count;
                affixes.PrefixId = affix.prefix;
                affixes.PrefixName = d2ce::ItemHelpers::getMagicalPrefixFromId(affixes.PrefixId);
            }

            if (affix.suffix != 0)
            {
                ++count;
                affixes.SuffixId = affix.suffix;
                affixes.SuffixName = d2ce::ItemHelpers::getMagicalSuffixFromId(affixes.SuffixId);
            }

            CurrentAffixes.Affixes.push_back(affixes);
        }
    }

    if (NumAllowedAffixes != 0)
    {
        // disable any comboboxes that do not have a value if we have full count
        BOOL bEnable = count >= NumAllowedAffixes ? FALSE : TRUE;
        for (auto& affixControl : AffixControls)
        {
            affixControl.Prefix.EnableWindow((bEnable || affixControl.Prefix.GetCurSel() != 0) ? TRUE : FALSE);
            affixControl.Suffix.EnableWindow((bEnable || affixControl.Suffix.GetCurSel() != 0) ? TRUE : FALSE);
        }
    }
}
//---------------------------------------------------------------------------
void CD2RareAffixesForm::UpdateCurrentAttribs()
{
    // Rest attributes for item
    auto pItem = NewItemForm.GetCreatedItem();
    if (pItem != nullptr)
    {
        CurrentItem = *pItem;
        CurrentItem.makeRare();
        CurrentItem.setDWBCode(CurrentDWBCode);
    }

    auto pOkWnd = GetDlgItem(IDOK);
    if (CurrentAffixes.Affixes.empty())
    {
        if (pOkWnd != nullptr)
        {
            pOkWnd->EnableWindow(FALSE);
        }

        ItemTooltipBox.RedrawWindow();
        return;
    }

    if (SimpleDialog)
    {
        auto pWnd = GetDlgItem(IDC_PREV_BUTTON); 
        if (pWnd != nullptr)
        {
            pWnd->EnableWindow((CurrentDWBCodeIndex > 0) ? TRUE : FALSE);
        }

        pWnd = GetDlgItem(IDC_NEXT_BUTTON);
        if (pWnd != nullptr)
        {
            pWnd->EnableWindow((CurrentDWBCodeIndex < (GeneratedDWBCode.size() - 1)) ? TRUE : FALSE);
        }
    }
    else
    {
        CWaitCursor wait;
        auto result = CurrentItem.setRareOrCraftedAttributes(CurrentAffixes);
        if (CurrentItem.getVersion() < d2ce::EnumItemVersion::v107) // pre-1.07 character file
        {
            // the DWBCode was changed for the item
            CurrentDWBCode = CurrentItem.getDWBCode();
            if (!result)
            {
                // should not happen, but if it does, we need to make sure we are showing the right affixes
                SyncAffixes();

                if (CurrentAffixes.Affixes.empty())
                {
                    if (pOkWnd != nullptr)
                    {
                        pOkWnd->EnableWindow(FALSE);
                    }

                    ItemTooltipBox.RedrawWindow();
                    return;
                }
            }
            else
            {
                // Name affixes may have changed
                SyncNameAffixes();
            }
        }
    }

    if (pOkWnd != nullptr)
    {
        pOkWnd->EnableWindow(TRUE);
    }

    ItemTooltipBox.RedrawWindow();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2RareAffixesForm::GetSelectedItem() const
{
    return &CurrentItem;
}
//---------------------------------------------------------------------------
const d2ce::Character* CD2RareAffixesForm::GetCharacterInfo() const
{
    return NewItemForm.GetCharacterInfo();
}
//---------------------------------------------------------------------------
