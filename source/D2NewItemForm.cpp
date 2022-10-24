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
#include "D2NewItemForm.h"
#include "d2ce/helpers/ItemHelpers.h"
#include "d2ce/item.h"
#include "D2MagicalAffixesForm.h"
#include "D2RareAffixesForm.h"
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
        const std::map<std::string, d2ce::AvailableItemType>& getAvailableItems();
    }
}

namespace
{
    struct AvailableItemFolder
    {
        CString name;
        HTREEITEM hItem = NULL;
    };
    
    void InitTreeControl(CTreeCtrl& tree, const d2ce::Character& charInfo,
        std::deque<AvailableItemFolder>& parent,
        std::map<std::string, d2ce::AvailableItemType>::const_iterator& iter,
        const std::map<std::string, d2ce::AvailableItemType>::const_iterator& iter_end,
        std::list<d2ce::Item>& bufferItems,
        bool isSharedStash)
    {
        if (iter == iter_end)
        {
            if (!parent.empty())
            {
                parent.pop_back();
            }
            return;
        }

        for (; iter != iter_end; ++iter)
        {
            if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::Item)
            {
                // can't add quest items to shared stash
                if (iter->second.pItemType != nullptr)
                {
                    // we found an item, add it to the tree
                    d2ce::ItemCreateParams createParams(charInfo.getDefaultItemVersion(), *iter->second.pItemType, charInfo.getTitleDifficulty(), charInfo.getClass(), charInfo.isExpansionCharacter());
                    d2ce::Item newItem(createParams);
                    HTREEITEM hParent = TVI_ROOT;
                    if (newItem.size() > 0)
                    {
                        bufferItems.resize(bufferItems.size() + 1);
                        auto& bufferItem = bufferItems.back();
                        bufferItem.swap(newItem);

                        // First make sure all the parent folders exist
                        for (auto& folder : parent)
                        {
                            if (folder.hItem == NULL)
                            {
                                folder.hItem = tree.InsertItem(folder.name, hParent, TVI_SORT);
                                if (folder.hItem != NULL)
                                {
                                    hParent = folder.hItem;
                                }
                            }
                            else
                            {
                                hParent = folder.hItem;
                            }
                        }

                        std::u16string uText = utf8::utf8to16(bufferItem.getItemTypeName());
                        CString strText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                        HTREEITEM hItem = tree.InsertItem(strText, hParent, TVI_SORT);
                        if (hItem == NULL)
                        {
                            bufferItems.pop_back();
                        }
                        else
                        {
                            tree.SetItemData(hItem, (DWORD_PTR)&bufferItem);
                        }
                    }
                }
            }
            else
            {
                // we found an folder, delay the add until an item is found
                std::u16string uText;
                if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::Category)
                {
                    bool bBadItemCategory = false;
                    if (isSharedStash && iter->second.name.compare("ques") == 0)
                    {
                        // can't add quest items to shared stash
                        bBadItemCategory = true;
                    }

                    if (!bBadItemCategory)
                    {
                        uText = utf8::utf8to16(d2ce::ItemHelpers::getCategoryNameFromCode(iter->second.name));
                    }
                }
                else if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::Sets)
                {
                    uText = utf8::utf8to16(iter->second.name); // TODO: translate
                }
                else if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::Set)
                {
                    uText = utf8::utf8to16(iter->second.name); // TODO: translate
                }
                else if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::Unique)
                {
                    uText = utf8::utf8to16(iter->second.name); // TODO: translate
                }
                else if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::Regular)
                {
                    uText = utf8::utf8to16(iter->second.name); // TODO: translate
                }
                else if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::Exceptional)
                {
                    uText = utf8::utf8to16(iter->second.name); // TODO: translate
                }
                else if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::Elite)
                {
                    uText = utf8::utf8to16(iter->second.name); // TODO: translate
                }
                else if (iter->second.folderType == d2ce::AvailableItemType::EnumFolderType::UnusedItems)
                {
                    uText = utf8::utf8to16(iter->second.name); // TODO: translate
                }

                CString strText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                if (!strText.IsEmpty())
                {
                    parent.push_back({ strText, NULL });
                    auto childIter = iter->second.children.begin();
                    auto childIter_end = iter->second.children.end();
                    InitTreeControl(tree, charInfo, parent, childIter, childIter_end, bufferItems, isSharedStash);
                }
            }
        }

        if (!parent.empty())
        {
            parent.pop_back();
        }
    }
}

//---------------------------------------------------------------------------
// CD2NewItemForm dialog

IMPLEMENT_DYNAMIC(CD2NewItemForm, CDialogEx)

//---------------------------------------------------------------------------
CD2NewItemForm::CD2NewItemForm(CD2MainForm& form)
	: CDialogEx(CD2NewItemForm::IDD, (CWnd*)&form), MainForm(form)
{

}
//---------------------------------------------------------------------------
CD2NewItemForm::CD2NewItemForm(CD2ItemsForm& form)
    : CDialogEx(CD2NewItemForm::IDD, (CWnd*)&form), MainForm(form.MainForm), ItemsFormPtr(&form)
{
}
//---------------------------------------------------------------------------
CD2NewItemForm::CD2NewItemForm(CD2SharedStashForm& form)
    : CDialogEx(CD2NewItemForm::IDD, (CWnd*)&form), MainForm(form.MainForm), SharedStashFormPtr(&form)
{
}
//---------------------------------------------------------------------------
CD2NewItemForm::~CD2NewItemForm()
{
}
//---------------------------------------------------------------------------
void CD2NewItemForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ITEM_TREE, ItemTree);
    DDX_Control(pDX, IDC_TOOLTIP_RECT, ItemTooltipBox);
    DDX_Control(pDX, IDC_ETHEREAL_CHECK, Ethereal);
    DDX_Control(pDX, IDC_QUALITY_STATIC, QualityStatic);
    DDX_Control(pDX, IDC_QUALITY_COMBO, Quality);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2NewItemForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2NewItemForm::OnBnClickedOk)
    ON_NOTIFY(TVN_SELCHANGED, IDC_ITEM_TREE, &CD2NewItemForm::OnTvnSelchangedItemtree)
    ON_NOTIFY(NM_DBLCLK, IDC_ITEM_TREE, &CD2NewItemForm::OnNMDblclkItemtree)
    ON_BN_CLICKED(IDC_ETHEREAL_CHECK, &CD2NewItemForm::OnBnClickedEtherealCheck)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2NewItemForm message handlers

//---------------------------------------------------------------------------
BOOL CD2NewItemForm::OnInitDialog()
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

                pWnd->EnableWindow(FALSE);
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
    }

    Ethereal.EnableWindow(FALSE);
    Ethereal.ShowWindow(FALSE);
    QualityStatic.EnableWindow(FALSE);
    QualityStatic.ShowWindow(FALSE);
    Quality.EnableWindow(FALSE);
    Quality.ShowWindow(FALSE);
    InitTree();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2NewItemForm::OnBnClickedOk()
{
    UpdateData(TRUE); // save results
    CreatedItem = GetSelectedItem();

    CD2MagicalAffixesForm magicAffixes(*this);
    CD2RareAffixesForm rareAffixes(*this);
    if (Quality.IsWindowEnabled())
    {
        auto idx = Quality.GetCurSel();
        if (idx >= 0)
        {
            auto quality = static_cast<d2ce::EnumItemQuality>(Quality.GetItemData(idx));
            switch (quality)
            {
            case d2ce::EnumItemQuality::SUPERIOR:
                const_cast<d2ce::Item*>(CreatedItem)->makeSuperior();
                break;

            case d2ce::EnumItemQuality::MAGIC:
                if (magicAffixes.DoModal() != IDOK)
                {
                    return;
                }
                break;

            case d2ce::EnumItemQuality::RARE:
            case d2ce::EnumItemQuality::CRAFT:
            case d2ce::EnumItemQuality::TEMPERED:
                if (rareAffixes.DoModal() != IDOK)
                {
                    return;
                }
                break;
            }
        }
    }

    if (!MainForm.getCharacterInfo().importItem(CreatedItem, false) || CreatedItem == nullptr)
    {
        return;
    }

    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2NewItemForm::OnTvnSelchangedItemtree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    *pResult = 0;
    ItemTooltipBox.RedrawWindow();

    auto item = GetSelectedItem();
    auto pWnd = GetDlgItem(IDOK);
    if (pWnd != nullptr)
    {
        pWnd->EnableWindow(item == nullptr ? FALSE : TRUE);
    }

    if (item != nullptr)
    {
        if (item->isArmor() || item->isWeapon())
        {
            if (item->isEthereal() || item->canMakeEthereal())
            {
                Ethereal.EnableWindow(TRUE);
                Ethereal.ShowWindow(TRUE);
                Ethereal.SetCheck(item->isEthereal() ? 1 : 0);
            }
            else
            {
                Ethereal.EnableWindow(FALSE);
                Ethereal.ShowWindow(FALSE);
            }

            if (!item->isQuestItem())
            {
                switch (item->getQuality())
                {
                case d2ce::EnumItemQuality::NORMAL:
                    Quality.ResetContent();
                    {
                        auto idx = Quality.AddString(_T("Normal"));
                        if (idx >= 0)
                        {
                            Quality.SetItemData(idx, static_cast<std::underlying_type_t<d2ce::EnumItemQuality>>(d2ce::EnumItemQuality::NORMAL));
                            Quality.SetCurSel(idx);
                        }

                        if (item->canMakeSuperior())
                        {
                            idx = Quality.AddString(_T("Superior"));
                            if (idx >= 0)
                            {
                                Quality.SetItemData(idx, static_cast<std::underlying_type_t<d2ce::EnumItemQuality>>(d2ce::EnumItemQuality::SUPERIOR));
                            }
                        }

                        // Check if the prefix and/or suffix is allowed
                        std::vector<std::uint16_t> prefixes;
                        std::vector<std::uint16_t> suffixes;
                        if (item->getPossibleMagicalAffixes(prefixes, suffixes))
                        {
                            idx = Quality.AddString(_T("Magical"));
                            if (idx >= 0)
                            {
                                Quality.SetItemData(idx, static_cast<std::underlying_type_t<d2ce::EnumItemQuality>>(d2ce::EnumItemQuality::MAGIC));
                            }
                        }

                        idx = Quality.AddString(_T("Rare"));
                        if (idx >= 0)
                        {
                            Quality.SetItemData(idx, static_cast<std::underlying_type_t<d2ce::EnumItemQuality>>(d2ce::EnumItemQuality::RARE));
                        }
                    }
                    QualityStatic.EnableWindow(TRUE);
                    QualityStatic.ShowWindow(TRUE);
                    Quality.EnableWindow(TRUE);
                    Quality.ShowWindow(TRUE);
                    break;

                default:
                    QualityStatic.EnableWindow(FALSE);
                    QualityStatic.ShowWindow(FALSE);
                    Quality.EnableWindow(FALSE);
                    Quality.ShowWindow(FALSE);
                    break;
                }
            }
            else
            {
                QualityStatic.EnableWindow(FALSE);
                QualityStatic.ShowWindow(FALSE);
                Quality.EnableWindow(FALSE);
                Quality.ShowWindow(FALSE);
            }
        }
        else
        {
            Ethereal.EnableWindow(FALSE);
            Ethereal.ShowWindow(FALSE);
            QualityStatic.EnableWindow(FALSE);
            QualityStatic.ShowWindow(FALSE);
            Quality.EnableWindow(FALSE);
            Quality.ShowWindow(FALSE);
        }
    }
    else
    {
        Ethereal.EnableWindow(FALSE);
        Ethereal.ShowWindow(FALSE);
        QualityStatic.EnableWindow(FALSE);
        QualityStatic.ShowWindow(FALSE);
        Quality.EnableWindow(FALSE);
        Quality.ShowWindow(FALSE);
    }
}
//---------------------------------------------------------------------------
void CD2NewItemForm::OnNMDblclkItemtree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    *pResult = 0;
    OnBnClickedOk();
}
//---------------------------------------------------------------------------
void CD2NewItemForm::OnBnClickedEtherealCheck()
{
    auto item = const_cast<d2ce::Item*>(GetSelectedItem());
    if (item != nullptr)
    {
        if (item->isEthereal())
        {
            item->removeEthereal();
        }
        else
        {
            item->makeEthereal();
        }
        ItemTooltipBox.RedrawWindow();
    }
}
//---------------------------------------------------------------------------
void CD2NewItemForm::InitTree()
{
    bool isSharedStash = SharedStashFormPtr != nullptr ? true : false;
    std::deque<AvailableItemFolder> parent;
    const auto& allItems = d2ce::ItemHelpers::getAvailableItems();
    auto iter = allItems.begin();
    auto iter_end = allItems.end();
    InitTreeControl(ItemTree, MainForm.getCharacterInfo(), parent, iter, iter_end, AvailableItems, isSharedStash);
}
//---------------------------------------------------------------------------
void CD2NewItemForm::DrawItem(HTREEITEM hTreeItem)
{
    auto itemData = ItemTree.GetItemData(hTreeItem);
    if (itemData == 0)
    {
        return;
    }
}
//---------------------------------------------------------------------------
const d2ce::Character* CD2NewItemForm::GetCharacterInfo() const
{
    return &MainForm.getCharacterInfo();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2NewItemForm::GetSelectedItem() const
{
    auto hTreeItem = ItemTree.GetSelectedItem();
    if (hTreeItem == NULL)
    {
        return nullptr;;
    }

    auto itemData = ItemTree.GetItemData(hTreeItem);
    if (itemData == 0)
    {
        return nullptr;
    }

    return (d2ce::Item*)itemData;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2NewItemForm::GetCreatedItem()
{
    return CreatedItem;
}
//---------------------------------------------------------------------------

