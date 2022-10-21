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
#include "D2MagicalAffixesForm.h"
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
        bool generateMagicalAffixes(MagicalCachev100& cache, const ItemCreateParams& createParams, std::uint16_t level, std::uint32_t dwb = 0, bool bMaxAlways = false);
        bool getMagicAttribs(const d2ce::MagicalAffixes& magicalAffixes, std::vector<MagicalAttribute>& attribs, const ItemCreateParams& createParams, bool bMaxAlways = true);
    }
}

//---------------------------------------------------------------------------
// CD2MagicalAffixesForm dialog

IMPLEMENT_DYNAMIC(CD2MagicalAffixesForm, CDialogEx)

//---------------------------------------------------------------------------
CD2MagicalAffixesForm::CD2MagicalAffixesForm(CD2NewItemForm& form)
	: CDialogEx(IDD_MAGICAL_AFFIXES_DIALOG, &form), NewItemForm(form)
{
    auto pItem = form.GetCreatedItem();
    if (pItem != nullptr)
    {
        auto charInfo = form.GetCharacterInfo();
        d2ce::EnumDifficulty diff = charInfo == nullptr ? d2ce::EnumDifficulty::Normal : charInfo->getTitleDifficulty();
        d2ce::EnumCharClass clazz = charInfo == nullptr ? d2ce::EnumCharClass::Amazon : charInfo->getClass();
        CreateParams = d2ce::ItemCreateParams(pItem->getVersion(), pItem->getItemTypeHelper(), diff, clazz, pItem->isExpansionItem());
        CurrentItem = *pItem;
        CurrentDWBCode = pItem->getDWBCode();
    }
}
//---------------------------------------------------------------------------
CD2MagicalAffixesForm::~CD2MagicalAffixesForm()
{
}
//---------------------------------------------------------------------------
void CD2MagicalAffixesForm::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PREFIX1_STATIC, PrefixStatic);
    DDX_Control(pDX, IDC_PREFIX1_COMBO, Prefix);
    DDX_Control(pDX, IDC_SUFFIX1_STATIC, SuffixStatic);
    DDX_Control(pDX, IDC_SUFFIX1_COMBO, Suffix);
    DDX_Control(pDX, IDC_TOOLTIP_RECT, ItemTooltipBox);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2MagicalAffixesForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2MagicalAffixesForm::OnBnClickedOk)
    ON_CBN_SELCHANGE(IDC_PREFIX1_COMBO, &CD2MagicalAffixesForm::OnCbnSelchangePrefix1Combo)
    ON_CBN_SELCHANGE(IDC_SUFFIX1_COMBO, &CD2MagicalAffixesForm::OnCbnSelchangeSuffix1Combo)
    ON_BN_CLICKED(IDC_GAMBLE_BUTTON, &CD2MagicalAffixesForm::OnBnClickedGambleButton)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2MagicalAffixesForm message handlers

//---------------------------------------------------------------------------
BOOL CD2MagicalAffixesForm::OnInitDialog()
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
void CD2MagicalAffixesForm::OnBnClickedOk()
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
void CD2MagicalAffixesForm::OnBnClickedGambleButton()
{
    // Generate the starting affixes
    CurrentDWBCode = d2ce::ItemHelpers::generarateRandomDW();
    d2ce::MagicalCachev100 generated_magic_affixes;
    if (d2ce::ItemHelpers::generateMagicalAffixes(generated_magic_affixes, CreateParams, CurrentItem.getLevel(), CurrentDWBCode, true))
    {
        CurrentAffixes = generated_magic_affixes.Affixes;
    }

    bool bFound = false;
    auto idx_end = Prefix.GetCount();
    for (int idx = 0; idx < idx_end; ++idx)
    {
        if (std::uint16_t(Prefix.GetItemData(idx)) == CurrentAffixes.PrefixId)
        {
            bFound = true;
            Prefix.SetCurSel(idx);
            break;
        }
    }

    if (!bFound)
    {
        CurrentAffixes.PrefixId = std::uint16_t(Prefix.GetItemData(Prefix.GetCurSel()));
        CurrentAffixes.PrefixName = d2ce::ItemHelpers::getMagicalPrefixFromId(CurrentAffixes.PrefixId);
    }

    bFound = false;
    idx_end = Suffix.GetCount();
    for (int idx = 0; idx < idx_end; ++idx)
    {
        if (std::uint16_t(Suffix.GetItemData(idx)) == CurrentAffixes.SuffixId)
        {
            bFound = true;
            Suffix.SetCurSel(idx);
            break;
        }
    }

    if (!bFound)
    {
        CurrentAffixes.SuffixId = std::uint16_t(Suffix.GetItemData(Suffix.GetCurSel()));
        CurrentAffixes.SuffixName = d2ce::ItemHelpers::getMagicalSuffixFromId(CurrentAffixes.SuffixId);
    }

    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2MagicalAffixesForm::OnCbnSelchangePrefix1Combo()
{
    UpdateData(TRUE); // save results
    CurrentAffixes.PrefixId = std::uint16_t(Prefix.GetItemData(Prefix.GetCurSel()));
    CurrentAffixes.PrefixName = d2ce::ItemHelpers::getMagicalPrefixFromId(CurrentAffixes.PrefixId);
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2MagicalAffixesForm::OnCbnSelchangeSuffix1Combo()
{
    UpdateData(TRUE); // save results
    CurrentAffixes.SuffixId = std::uint16_t(Suffix.GetItemData(Suffix.GetCurSel()));
    CurrentAffixes.SuffixName = d2ce::ItemHelpers::getMagicalSuffixFromId(CurrentAffixes.SuffixId);
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2MagicalAffixesForm::InitAffixes()
{
    // Generate the starting affixes
    d2ce::MagicalCachev100 generated_magic_affixes;
    if (d2ce::ItemHelpers::generateMagicalAffixes(generated_magic_affixes, CreateParams, CurrentItem.getLevel(), CurrentDWBCode, true))
    {
        CurrentAffixes = generated_magic_affixes.Affixes;
    }

    // Check if the prefix and/or suffix is allowed
    std::vector<std::uint16_t> prefixes;
    std::vector<std::uint16_t> suffixes;
    if (!CurrentItem.getPossibleMagicalAffixes(prefixes, suffixes))
    {
        EndDialog(IDCANCEL);
        return;
    }

    int prefixIdx = -1;
    auto idx = Prefix.AddString(_T(""));
    if (idx >= 0)
    {
        prefixIdx = idx;
        Prefix.SetItemData(idx, 0);
    }

    std::u16string uText;
    CString strText;
    for (const auto& prefix : prefixes)
    {
        uText = utf8::utf8to16(d2ce::ItemHelpers::getMagicalPrefixFromId(prefix));
        strText = reinterpret_cast<LPCWSTR>(uText.c_str());
        idx = Prefix.AddString(strText);
        if (idx >= 0)
        {
            if (generated_magic_affixes.Affixes.PrefixId == prefix)
            {
                prefixIdx = idx;
            }
            Prefix.SetItemData(idx, prefix);
        }
    }

    Prefix.SetCurSel(prefixIdx);
    CurrentAffixes.PrefixId = std::uint16_t(Prefix.GetItemData(prefixIdx));

    int suffixIdx = -1;
    idx = Suffix.AddString(_T(""));
    if (idx >= 0)
    {
        suffixIdx = idx;
        Suffix.SetItemData(idx, 0);
    }

    for (const auto& suffix : suffixes)
    {
        uText = utf8::utf8to16(d2ce::ItemHelpers::getMagicalSuffixFromId(suffix));
        strText = reinterpret_cast<LPCWSTR>(uText.c_str());
        idx = Suffix.AddString(strText);
        if (idx >= 0)
        {
            if (generated_magic_affixes.Affixes.SuffixId == suffix)
            {
                suffixIdx = idx;
            }
            Suffix.SetItemData(idx, suffix);
        }
    }

    Suffix.SetCurSel(suffixIdx);
    CurrentAffixes.SuffixId = std::uint16_t(Suffix.GetItemData(suffixIdx));
    UpdateCurrentAttribs();
}
//---------------------------------------------------------------------------
void CD2MagicalAffixesForm::UpdateCurrentAttribs()
{
    // Rest attributes for item
    auto pItem = NewItemForm.GetCreatedItem();
    if (pItem != nullptr)
    {
        CurrentItem = *pItem;
        CurrentItem.setDWBCode(CurrentDWBCode);
    }

    CurrentAttribs.clear();

    auto charInfo = NewItemForm.GetCharacterInfo();
    std::uint32_t charLevel = charInfo == nullptr ? 1 : charInfo->getLevel();
    if (d2ce::ItemHelpers::getMagicAttribs(CurrentAffixes, CurrentAttribs, CreateParams, CurrentItem.getGameVersion()))
    {
        d2ce::ItemHelpers::formatMagicalAttributes(CurrentAttribs, charLevel);
    }

    auto pWnd = GetDlgItem(IDOK);
    if (CurrentAttribs.empty())
    {
        if (pWnd != nullptr)
        {
            pWnd->EnableWindow(FALSE);
        }
    }
    else
    {
        CurrentItem.setMagicalAffixes(CurrentAffixes);
        if (pWnd != nullptr)
        {
            pWnd->EnableWindow(TRUE);
        }
    }

    ItemTooltipBox.RedrawWindow();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2MagicalAffixesForm::GetSelectedItem() const
{
    return &CurrentItem;
}
//---------------------------------------------------------------------------
const d2ce::Character* CD2MagicalAffixesForm::GetCharacterInfo() const
{
    return NewItemForm.GetCharacterInfo();
}
//---------------------------------------------------------------------------
