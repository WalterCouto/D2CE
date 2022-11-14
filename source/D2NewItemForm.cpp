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
#include "D2RunewordForm.h"
#include "D2EarAttributesForm.h"
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
        void getAvailableItems(std::map<std::string, d2ce::AvailableItemType>& availItems, EnumItemVersion itemVersion, bool isExpansion);
        const std::map<std::uint16_t, std::string>& getMonsterNameMap();
    }
}

namespace
{
    D2TreeCtrlPath g_LastSelection;

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
        std::map<HTREEITEM, d2ce::AvailableItemType>& availableItemTypes,
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
            const auto& availItemType = iter->second;
            if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::Item)
            {
                if (availItemType.pItemType != nullptr)
                {
                    // we found an item, add it to the tree
                    const auto& itemType = *(availItemType.pItemType);
                    if (!itemType.isRestrictedItem() || !charInfo.getHasRestrictedItem(itemType)) // make sure the item is not restricted from being created
                    {
                        HTREEITEM hParent = TVI_ROOT;
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

                        DWORD_PTR itemData = 0;
                        std::u16string uText;
                        if (((itemType.isRing() || itemType.isAmulet() || itemType.isCharm() || itemType.isJewel()) && !itemType.isUniqueItem()) || itemType.isEar())
                        {
                            d2ce::ItemCreateParams createParams(charInfo.getDefaultItemVersion(), itemType, charInfo.getTitleDifficulty(), charInfo.getClass(), charInfo.isExpansionCharacter());
                            d2ce::Item newItem(createParams);
                            if (newItem.size() > 0)
                            {
                                bufferItems.resize(bufferItems.size() + 1);
                                auto& bufferItem = bufferItems.back();
                                bufferItem.swap(newItem);
                                itemData = (DWORD_PTR)&bufferItem;
                                uText = utf8::utf8to16(bufferItem.getDisplayedItemName());
                            }
                            else
                            {
                                uText = utf8::utf8to16(itemType.name);
                            }
                        }
                        else if (itemType.isPotion() && itemType.isUnusedItem())
                        {
                            uText = utf8::utf8to16(itemType.code);
                        }
                        else
                        {
                            uText = utf8::utf8to16(itemType.name);
                        }

                        CString strText(reinterpret_cast<LPCWSTR>(uText.c_str()));

                        // Check for Multi-line text
                        auto idx = strText.Find(_T('\n'));
                        if (idx >= 0)
                        {
                            strText = strText.Left(idx);
                        }

                        HTREEITEM hItem = tree.InsertItem(strText, hParent, TVI_SORT);
                        if (hItem == NULL)
                        {
                            bufferItems.pop_back();
                        }
                        else
                        {
                            tree.SetItemData(hItem, itemData);
                            availableItemTypes[hItem] = availItemType;
                        }
                    }
                }
            }
            else
            {
                // we found an folder, delay the add until an item is found
                std::u16string uText;
                if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::Category)
                {
                    bool bBadItemCategory = false;
                    if (isSharedStash && availItemType.name.compare("ques") == 0)
                    {
                        // can't add quest items to shared stash
                        bBadItemCategory = true;
                    }

                    if (!bBadItemCategory)
                    {
                        uText = utf8::utf8to16(d2ce::ItemHelpers::getCategoryNameFromCode(availItemType.name));
                    }
                }
                else if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::ResourceString)
                {
                    std::string strValue;
                    d2ce::LocalizationHelpers::GetStringTxtValue(availItemType.name, strValue);
                    uText = utf8::utf8to16(strValue);
                }
                else if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::Sets)
                {
                    uText = utf8::utf8to16(availItemType.name); // TODO: translate
                }
                else if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::Set)
                {
                    uText = utf8::utf8to16(availItemType.name); // TODO: translate
                }
                else if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::Unique)
                {
                    uText = utf8::utf8to16(availItemType.name); // TODO: translate
                }
                else if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::Regular)
                {
                    uText = utf8::utf8to16(availItemType.name); // TODO: translate
                }
                else if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::Exceptional)
                {
                    uText = utf8::utf8to16(availItemType.name); // TODO: translate
                }
                else if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::Elite)
                {
                    uText = utf8::utf8to16(availItemType.name); // TODO: translate
                }
                else if (availItemType.folderType == d2ce::AvailableItemType::EnumFolderType::UnusedItems)
                {
                    uText = utf8::utf8to16(availItemType.name); // TODO: translate
                }

                CString strText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                if (!strText.IsEmpty())
                {
                    parent.push_back({ strText, NULL });
                    auto childIter = availItemType.children.begin();
                    auto childIter_end = availItemType.children.end();
                    InitTreeControl(tree, charInfo, parent, childIter, childIter_end, bufferItems, availableItemTypes, isSharedStash);
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
    DDX_Control(pDX, IDC_MONSTERS_STATIC, MonstersStatic);
    DDX_Control(pDX, IDC_MONSTERS_COMBO, Monsters);
    DDX_Control(pDX, IDC_SOCKETS_STATIC, SocketsStatic);
    DDX_Control(pDX, IDC_SOCKETS_EDIT, SocketsEdit);
    DDX_Control(pDX, IDC_SOCKETS_SPIN, SocketsSpinner);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2NewItemForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2NewItemForm::OnBnClickedOk)
    ON_NOTIFY(TVN_SELCHANGED, IDC_ITEM_TREE, &CD2NewItemForm::OnTvnSelchangedItemtree)
    ON_NOTIFY(NM_DBLCLK, IDC_ITEM_TREE, &CD2NewItemForm::OnNMDblclkItemtree)
    ON_BN_CLICKED(IDC_ETHEREAL_CHECK, &CD2NewItemForm::OnBnClickedEtherealCheck)
    ON_CBN_SELCHANGE(IDC_MONSTERS_COMBO, &CD2NewItemForm::OnCbnSelchangeMonstersCombo)
    ON_WM_VSCROLL()
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

        if (d2ce::LocalizationHelpers::GetStringTxtValue("ModStre8c", strValue))
        {
            strValue += " ";
            SocketsStatic.GetWindowText(text);
            textA = text;
            if (textA.CompareNoCase(strValue.c_str()) != 0)
            {
                uText = utf8::utf8to16(strValue);
                SocketsStatic.SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("Monsters", strValue))
        {
            strValue += " ";
            MonstersStatic.GetWindowText(text);
            textA = text;
            if (textA.CompareNoCase(strValue.c_str()) != 0)
            {
                uText = utf8::utf8to16(strValue);
                MonstersStatic.SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
            }
        }
    }

    Ethereal.EnableWindow(FALSE);
    Ethereal.ShowWindow(FALSE);
    QualityStatic.EnableWindow(FALSE);
    QualityStatic.ShowWindow(FALSE);
    Quality.EnableWindow(FALSE);
    Quality.ShowWindow(FALSE);
    MonstersStatic.EnableWindow(FALSE);
    MonstersStatic.ShowWindow(FALSE);
    Monsters.EnableWindow(FALSE);
    Monsters.ShowWindow(FALSE);
    SocketsStatic.ShowWindow(FALSE);
    SocketsEdit.ShowWindow(FALSE);
    SocketsEdit.SetLimitText(1);
    SocketsSpinner.ShowWindow(FALSE);
    InitTree();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2NewItemForm::OnBnClickedOk()
{
    UpdateData(TRUE); // save results
    CreatedItem = GetSelectedItem();
    if (CreatedItem == nullptr)
    {
        return;
    }

    if (Quality.IsWindowEnabled())
    {
        auto idx = Quality.GetCurSel();
        if (idx >= 0)
        {
            auto qualityIntValue = std::int16_t(Quality.GetItemData(idx));
            if (qualityIntValue == -1)
            {
                CD2RunewordForm dlg(*this);
                if (dlg.DoModal() != IDOK)
                {
                    return;
                }
            }
            else
            {
                auto quality = static_cast<d2ce::EnumItemQuality>(qualityIntValue);
                switch (quality)
                {
                case d2ce::EnumItemQuality::SUPERIOR:
                    const_cast<d2ce::Item*>(CreatedItem)->makeSuperior();
                    break;

                case d2ce::EnumItemQuality::MAGIC:
                {
                    CD2MagicalAffixesForm magicAffixes(*this);
                    if (magicAffixes.DoModal() != IDOK)
                    {
                        return;
                    }
                }
                    break;

                case d2ce::EnumItemQuality::RARE:
                case d2ce::EnumItemQuality::CRAFT:
                case d2ce::EnumItemQuality::TEMPERED:
                {
                    CD2RareAffixesForm rareAffixes(*this);
                    if (rareAffixes.DoModal() != IDOK)
                    {
                        return;
                    }
                }
                    break;
                }
            }
        }
    }
    else if (CreatedItem->isCharm() && !CreatedItem->isUniqueItem())
    {
        CD2MagicalAffixesForm magicAffixes(*this);
        if (magicAffixes.DoModal() != IDOK)
        {
            return;
        }
    }
    else if (CreatedItem->isEar())
    {
        CD2EarAttributesForm earAttributes(*this);
        if (earAttributes.DoModal() != IDOK)
        {
            return;
        }
    }

    if (!MainForm.getCharacterInfo().importItem(CreatedItem, false))
    {
        return;
    }

    ItemTree.BuildSelectionPath(g_LastSelection);
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

                        if(item->canHaveSockets() && !item->getPossibleRunewords().empty())
                        {
                            idx = Quality.AddString(_T("Runeword"));
                            if (idx >= 0)
                            {
                                Quality.SetItemData(idx, DWORD_PTR(-1));
                            }
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
        else if (item->isRing() || item->isAmulet() || item->isJewel())
        {
            Ethereal.EnableWindow(FALSE);
            Ethereal.ShowWindow(FALSE);
            switch (item->getQuality())
            {
            case d2ce::EnumItemQuality::MAGIC:
            case d2ce::EnumItemQuality::RARE:
            case d2ce::EnumItemQuality::CRAFT:
            case d2ce::EnumItemQuality::TEMPERED:
                Quality.ResetContent();
                {
                    // Check if the prefix and/or suffix is allowed
                    int idx = -1;
                    std::vector<std::uint16_t> prefixes;
                    std::vector<std::uint16_t> suffixes;
                    if (item->getPossibleMagicalAffixes(prefixes, suffixes))
                    {
                        idx = Quality.AddString(_T("Magical"));
                        if (idx >= 0)
                        {
                            Quality.SetItemData(idx, static_cast<std::underlying_type_t<d2ce::EnumItemQuality>>(d2ce::EnumItemQuality::MAGIC));
                            Quality.SetCurSel(idx);
                        }
                    }

                    idx = Quality.AddString(_T("Rare"));
                    if (idx >= 0)
                    {
                        Quality.SetItemData(idx, static_cast<std::underlying_type_t<d2ce::EnumItemQuality>>(d2ce::EnumItemQuality::RARE));
                        switch (item->getQuality())
                        {
                        case d2ce::EnumItemQuality::RARE:
                        case d2ce::EnumItemQuality::CRAFT:
                        case d2ce::EnumItemQuality::TEMPERED:
                            Quality.SetCurSel(idx);
                            break;
                        }
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
            Ethereal.EnableWindow(FALSE);
            Ethereal.ShowWindow(FALSE);
            QualityStatic.EnableWindow(FALSE);
            QualityStatic.ShowWindow(FALSE);
            Quality.EnableWindow(FALSE);
            Quality.ShowWindow(FALSE);
        }

        if (item->canHaveSockets())
        {
            SocketsSpinner.SetRange32(0, item->getMaxSocketCount());
            SocketsSpinner.SetPos32(item->getSocketCount());
            SocketsStatic.EnableWindow(TRUE);
            SocketsStatic.ShowWindow(TRUE);
            SocketsEdit.EnableWindow(TRUE);
            SocketsEdit.ShowWindow(TRUE);
            SocketsSpinner.EnableWindow(TRUE);
            SocketsSpinner.ShowWindow(TRUE);
        }
        else
        {
            SocketsStatic.EnableWindow(FALSE);
            SocketsStatic.ShowWindow(FALSE);
            SocketsEdit.EnableWindow(FALSE);
            SocketsEdit.ShowWindow(FALSE);
            SocketsSpinner.EnableWindow(FALSE);
            SocketsSpinner.ShowWindow(FALSE);
        }

        if (item->isBodyPart() && (item->getVersion() >= d2ce::EnumItemVersion::v107))
        {
            MonstersStatic.EnableWindow(TRUE);
            MonstersStatic.ShowWindow(TRUE);
            Monsters.EnableWindow(TRUE);
            Monsters.ShowWindow(TRUE);

            auto iter = MonsterIDToIndexMap.find(item->getMonsterId());
            if (iter != MonsterIDToIndexMap.end())
            {
                Monsters.SetCurSel(iter->second);
            }
            else
            {
                Monsters.SetCurSel(0);
            }
        }
        else
        {
            MonstersStatic.EnableWindow(FALSE);
            MonstersStatic.ShowWindow(FALSE);
            Monsters.EnableWindow(FALSE);
            Monsters.ShowWindow(FALSE);
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
        MonstersStatic.EnableWindow(FALSE);
        MonstersStatic.ShowWindow(FALSE);
        Monsters.EnableWindow(FALSE);
        Monsters.ShowWindow(FALSE);
        SocketsStatic.EnableWindow(FALSE);
        SocketsStatic.ShowWindow(FALSE);
        SocketsEdit.EnableWindow(FALSE);
        SocketsEdit.ShowWindow(FALSE);
        SocketsSpinner.EnableWindow(FALSE);
        SocketsSpinner.ShowWindow(FALSE);
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
void CD2NewItemForm::OnCbnSelchangeMonstersCombo()
{
    UpdateData(TRUE); // save results
    auto monsterID = std::uint16_t(Monsters.GetItemData(Monsters.GetCurSel()));
    if (monsterID >= 0)
    {
        auto item = const_cast<d2ce::Item*>(GetSelectedItem());
        if (item != nullptr)
        {
            item->setMonsterId(monsterID);
            ItemTooltipBox.RedrawWindow();
        }
    }
}
//---------------------------------------------------------------------------
void CD2NewItemForm::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    __super::OnVScroll(nSBCode, nPos, pScrollBar);
    if (pScrollBar != NULL && (pScrollBar->GetDlgCtrlID() == IDC_SOCKETS_SPIN))
    {
        auto item = const_cast<d2ce::Item*>(GetSelectedItem());
        if (item != nullptr)
        {
            item->setSocketCount(std::uint8_t(SocketsSpinner.GetPos32()));
            ItemTooltipBox.RedrawWindow();
        }
    }
}
//---------------------------------------------------------------------------
void CD2NewItemForm::InitTree()
{
    auto& charInfo = MainForm.getCharacterInfo();
    std::map<std::string, d2ce::AvailableItemType> availableItemTypeMap;
    d2ce::ItemHelpers::getAvailableItems(availableItemTypeMap, charInfo.getDefaultItemVersion(), charInfo.isExpansionCharacter());
    auto iter = availableItemTypeMap.begin();
    auto iter_end = availableItemTypeMap.end();

    bool isSharedStash = SharedStashFormPtr != nullptr ? true : false;
    std::deque<AvailableItemFolder> parent;
    InitTreeControl(ItemTree, MainForm.getCharacterInfo(), parent, iter, iter_end, AvailableItems, AvailableItemTypes, isSharedStash);
    ItemTree.OpenPath(g_LastSelection);

    // populate Monsters combo
    MonsterIDToIndexMap.clear();
    Monsters.ResetContent();
    DWORD_PTR itemData = 0;
    std::u16string uText;
    CString strText;
    int idx = 0;
    const auto& monsterMap = d2ce::ItemHelpers::getMonsterNameMap();
    for (const auto& monster : monsterMap)
    {
        uText = utf8::utf8to16(monster.second.c_str());
        strText = reinterpret_cast<LPCWSTR>(uText.c_str());
        itemData = (DWORD_PTR)monster.first;
        idx = Monsters.AddString(strText);
        if (idx >= 0)
        {
            Monsters.SetItemData(idx, itemData);
            MonsterIDToIndexMap[monster.first] = idx;
        }
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
        return nullptr;
    }

    auto itemData = ItemTree.GetItemData(hTreeItem);
    if (itemData == 0)
    {
        auto iter = AvailableItemTypes.find(hTreeItem);
        if (iter == AvailableItemTypes.end())
        {
            return nullptr;
        }

        if (iter->second.pItemType == nullptr)
        {
            return nullptr;
        }

        auto& itemType = *iter->second.pItemType;
        auto& charInfo = MainForm.getCharacterInfo();
        d2ce::ItemCreateParams createParams(charInfo.getDefaultItemVersion(), itemType, charInfo.getTitleDifficulty(), charInfo.getClass(), charInfo.isExpansionCharacter());
        d2ce::Item newItem(createParams);
        if (newItem.size() > 0)
        {
            AvailableItems.resize(AvailableItems.size() + 1);
            auto& bufferItem = AvailableItems.back();
            bufferItem.swap(newItem);
            itemData = (DWORD_PTR)&bufferItem;
            ItemTree.SetItemData(hTreeItem, itemData);
        }
        else
        {
            return nullptr;
        }
    }

    return (d2ce::Item*)itemData;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2NewItemForm::GetCreatedItem()
{
    return CreatedItem;
}
//---------------------------------------------------------------------------
