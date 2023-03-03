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

#include "pch.h"
#include "D2Editor.h"
#include "D2AddGemsForm.h"
#include "afxdialogex.h"
#include "d2ce/helpers/ItemHelpers.h"
#include <utf8/utf8.h>
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//---------------------------------------------------------------------------
namespace d2ce
{
    namespace ItemHelpers
    {
        void getAvailableItems(std::map<std::string, d2ce::AvailableItemType>& availItems, EnumItemVersion itemVersion, bool isExpansion);
    }
}

namespace
{
    CString& RemoveColorFromText(CString& coloredText)
    {
        // don't care about color
        if (coloredText.Find(_T("ÿc")) >= 0)
        {
            coloredText = d2ce::ColorHelpers::RemoveColorFromText(coloredText.GetString()).c_str();
        }

        return coloredText;
    }

    std::array<std::uint8_t, 4> ItemCodeStringConverter(const std::string& sValue)
    {
        std::array<std::uint8_t, 4> strcode = { 0x20, 0x20, 0x20, 0x20 };
        if (sValue.size() != 3)
        {
            return strcode;
        }

        std::memcpy(strcode.data(), sValue.c_str(), 3);
        return strcode;
    }

    D2TreeCtrlPath g_LastSelection;

    struct AvailableItemFolder
    {
        CString name;
        HTREEITEM hItem = NULL;
    };

    void InitTreeControl(CTreeCtrl& tree, std::deque<AvailableItemFolder>& parent,
        std::map<std::string, d2ce::AvailableItemType>::const_iterator& iter,
        const std::map<std::string, d2ce::AvailableItemType>::const_iterator& iter_end,
        std::list<d2ce::Item>& bufferItems,
        std::map<HTREEITEM, d2ce::AvailableItemType>& availableItemTypes,
        const d2ce::ItemType* sourceItemTypePtr)
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
                if (availItemType.pItemType != nullptr && availItemType.pItemType->isGPSItem())
                {
                    // we found an item, add it to the tree
                    const auto& itemType = *(availItemType.pItemType);
                    HTREEITEM hParent = TVI_ROOT;
                    // First make sure all the parent folders exist
                    for (auto& folder : parent)
                    {
                        if (folder.hItem == NULL)
                        {
                            folder.hItem = tree.InsertItem(folder.name, hParent, TVI_SORT);
                            if (folder.hItem != NULL)
                            {
                                HTREEITEM hFirstVisible = tree.GetSelectedItem();
                                if (hFirstVisible == NULL)
                                {
                                    hFirstVisible = tree.GetFirstVisibleItem();
                                }

                                tree.Expand(hParent, TVE_EXPAND);
                                if (hFirstVisible != NULL)
                                {
                                    tree.EnsureVisible(hFirstVisible);
                                }

                                hParent = folder.hItem;
                            }
                        }
                        else
                        {
                            hParent = folder.hItem;
                        }
                    }

                    DWORD_PTR itemData = 0;
                    std::u16string uText = utf8::utf8to16(itemType.name);
                    CString strText(reinterpret_cast<LPCWSTR>(uText.c_str()));

                    // Check for Multi-line text
                    auto idx = strText.Find(_T('\n'));
                    if (idx >= 0)
                    {
                        strText = strText.Left(idx);
                    }

                    RemoveColorFromText(strText);

                    HTREEITEM hItem = tree.InsertItem(strText, hParent, TVI_SORT);
                    if (hItem == NULL)
                    {
                        bufferItems.pop_back();
                    }
                    else
                    {
                        HTREEITEM hFirstVisible = tree.GetSelectedItem();
                        if (hFirstVisible == NULL)
                        {
                            hFirstVisible = tree.GetFirstVisibleItem();
                        }

                        tree.Expand(hParent, TVE_EXPAND);
                        if (hFirstVisible != NULL)
                        {
                            tree.EnsureVisible(hFirstVisible);
                        }

                        tree.SetItemData(hItem, itemData);
                        availableItemTypes[hItem] = availItemType;
                        if (sourceItemTypePtr != nullptr && sourceItemTypePtr->code == itemType.code)
                        {
                            tree.SelectItem(hItem);
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
                    uText = utf8::utf8to16(d2ce::ItemHelpers::getCategoryNameFromCode(availItemType.name));
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
                    RemoveColorFromText(strText);
                    parent.push_back({ strText, NULL });
                    auto childIter = availItemType.children.begin();
                    auto childIter_end = availItemType.children.end();
                    InitTreeControl(tree, parent, childIter, childIter_end, bufferItems, availableItemTypes, sourceItemTypePtr);
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
// CD2AddGemsForm dialog
IMPLEMENT_DYNAMIC(CD2AddGemsForm, CDialogEx)

//---------------------------------------------------------------------------
CD2AddGemsForm::CD2AddGemsForm(CD2MainForm& form)
    : CDialogEx(CD2AddGemsForm::IDD, (CWnd*)&form), MainForm(form)
{

}
//---------------------------------------------------------------------------
CD2AddGemsForm::CD2AddGemsForm(CD2ItemsForm& form)
    : CDialogEx(CD2AddGemsForm::IDD, (CWnd*)&form), MainForm(form.MainForm), ItemsFormPtr(&form), ItemPtr(form.CurrItem)
{
}
//---------------------------------------------------------------------------
CD2AddGemsForm::CD2AddGemsForm(CD2SharedStashForm& form)
    : CDialogEx(CD2AddGemsForm::IDD, (CWnd*)&form), MainForm(form.MainForm), SharedStashFormPtr(&form), ItemPtr(form.CurrItem)
{
}
//---------------------------------------------------------------------------
CD2AddGemsForm::~CD2AddGemsForm()
{
}
//---------------------------------------------------------------------------
void CD2AddGemsForm::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ITEM_TREE, ItemTree);
    DDX_Control(pDX, IDC_TOOLTIP_RECT, ItemTooltipBox);
}
//---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CD2AddGemsForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2AddGemsForm::OnBnClickedAdd)
    ON_BN_CLICKED(IDC_FILL_BUTTON, &CD2AddGemsForm::OnBnClickedFill)
    ON_NOTIFY(TVN_SELCHANGED, IDC_ITEM_TREE, &CD2AddGemsForm::OnTvnSelchangedItemtree)
    ON_NOTIFY(NM_DBLCLK, IDC_ITEM_TREE, &CD2AddGemsForm::OnNMDblclkItemtree)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
BOOL CD2AddGemsForm::OnInitDialog()
{
    __super::OnInitDialog();

    bool isBeltable = true;
    bool mustBeBeltable = false;
    if (ItemPtr != nullptr)
    {
        const auto& itemType = ItemPtr->getItemTypeHelper();
        isBeltable = itemType.isBeltable();

        if (ItemsFormPtr != nullptr && !isBeltable)
        {
            auto itemLocation = static_cast<d2ce::EnumItemLocation>(ItemsFormPtr->CurrItemLocation[0]);
            if (itemLocation == d2ce::EnumItemLocation::BELT)
            {
                mustBeBeltable = true;
            }
        }
    }

    // Fill in from tree
    InitTree();
   
    // Fill in to combo
    CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);
    if (pLocationCombo != nullptr)
    {
        if (SharedStashFormPtr != nullptr)
        {
            CString str;
            str.Format(_T("Shared Stash %ld"), (int)SharedStashFormPtr->getCurrentPage() + 1);
            auto insertedIdx = pLocationCombo->AddString(str);
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(SharedStashFormPtr->getCurrentPage()));
            pLocationCombo->SetCurSel(0);
        }
        else
        {
            std::array< std::uint8_t, 2> locationCode = { 0x00, 0x00 };
            std::uint16_t& itemData = *reinterpret_cast<std::uint16_t*>(locationCode.data());
            std::uint8_t& locationID = locationCode[0];
            std::uint8_t& altLocationId = locationCode[1];

            std::uint16_t selectedItemData = 0;
            if (!mustBeBeltable || isBeltable)
            {
                if (ItemsFormPtr != nullptr)
                {
                    selectedItemData = *reinterpret_cast<std::uint16_t*>(ItemsFormPtr->CurrItemLocation.data());
                }
            }
            int selectedIdx = isBeltable ? 0 : 1;

            locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::BELT);
            altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::UNKNOWN);
            auto insertedIdx = pLocationCombo->AddString(_T("Belt"));
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
            if (itemData == selectedItemData)
            {
                selectedIdx = insertedIdx;
            }

            if (MainForm.getHasHoradricCube())
            {
                locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
                altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::HORADRIC_CUBE);
                insertedIdx = pLocationCombo->AddString(_T("Horadric Cube"));
                pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
                if (itemData == selectedItemData)
                {
                    selectedIdx = insertedIdx;
                }
            }

            locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
            altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::INVENTORY);
            insertedIdx = pLocationCombo->AddString(_T("Inventory"));
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
            if (itemData == selectedItemData)
            {
                selectedIdx = insertedIdx;
            }

            locationID = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
            altLocationId = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::STASH);
            insertedIdx = pLocationCombo->AddString(_T("Stash"));
            pLocationCombo->SetItemData(insertedIdx, std::uint64_t(itemData));
            if (itemData == selectedItemData)
            {
                selectedIdx = insertedIdx;
            }

            pLocationCombo->SetCurSel(selectedIdx);
        }
    }
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
// CD2AddGemsForm message handlers
//---------------------------------------------------------------------------
void CD2AddGemsForm::OnBnClickedAdd()
{
    auto item = GetSelectedItem();
    CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);
    if (item == nullptr || pLocationCombo == nullptr)
    {
        return;
    }

    std::array<std::uint8_t, 4> gemCode;
    if (!item->getItemCode(gemCode))
    {
        return;
    }

    auto toIdx = pLocationCombo->GetCurSel();
    if (SharedStashFormPtr != nullptr)
    {
        auto stashPage = size_t(pLocationCombo->GetItemData(toIdx));
        if (!SharedStashFormPtr->addItem(gemCode, stashPage))
        {
            CString msg(_T("A "));
            msg += ItemTree.GetItemText(ItemTree.GetSelectedItem());
            msg += _T(" could not be added to ");
            CString temp;
            pLocationCombo->GetLBText(toIdx, temp);
            msg += temp;
            AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
        }
    }
    else
    {
        std::array< std::uint8_t, 2> locationCode = { 0x00, 0x00 };
        std::uint8_t& locationID = locationCode[0];
        std::uint8_t& altLocationId = locationCode[1];
        std::uint64_t itemDataTo = pLocationCombo->GetItemData(toIdx);
        *reinterpret_cast<std::uint16_t*>(locationCode.data()) = std::uint16_t(itemDataTo);

        if (ItemsFormPtr != nullptr)
        {
            if (!ItemsFormPtr->addItem(static_cast<d2ce::EnumItemLocation>(locationID), static_cast<d2ce::EnumAltItemLocation>(altLocationId), gemCode))
            {
                CString msg(_T("A "));
                msg += ItemTree.GetItemText(ItemTree.GetSelectedItem());
                msg += _T(" could not be added to ");
                CString temp;
                pLocationCombo->GetLBText(toIdx, temp);
                msg += temp;
                AfxMessageBox(msg, MB_ICONEXCLAMATION | MB_OK);
            }
        }
        else if (!MainForm.addItem(static_cast<d2ce::EnumItemLocation>(locationID), static_cast<d2ce::EnumAltItemLocation>(altLocationId), gemCode))
        {
            CString msg(_T("A "));
            msg += ItemTree.GetItemText(ItemTree.GetSelectedItem());
            msg += _T(" could not be added to ");
            CString temp;
            pLocationCombo->GetLBText(toIdx, temp);
            msg += temp;
            AfxMessageBox(msg, MB_ICONEXCLAMATION | MB_OK);
        }
        else
        {
            CString msg(_T("A "));
            msg += ItemTree.GetItemText(ItemTree.GetSelectedItem());
            msg += _T(" was added to ");
            CString temp;
            pLocationCombo->GetLBText(toIdx, temp);
            msg += temp;
            AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
        }
    }

    ItemTree.BuildSelectionPath(g_LastSelection);
}
//---------------------------------------------------------------------------
void CD2AddGemsForm::OnBnClickedFill()
{
    auto item = GetSelectedItem();
    CComboBox* pLocationCombo = (CComboBox*)GetDlgItem(IDC_LOCATION_COMBO);
    if (item == nullptr || pLocationCombo == nullptr)
    {
        return;
    }

    std::array<std::uint8_t, 4> gemCode;
    if (!item->getItemCode(gemCode))
    {
        return;
    }

    auto toIdx = pLocationCombo->GetCurSel();
    std::array< std::uint8_t, 2> locationCode = { 0x00, 0x00 };
    std::uint8_t& locationID = locationCode[0];
    std::uint8_t& altLocationId = locationCode[1];
    std::uint64_t itemDataTo = pLocationCombo->GetItemData(toIdx);
    *reinterpret_cast<std::uint16_t*>(locationCode.data()) = std::uint16_t(itemDataTo);

    size_t numAdded = 0;
    if (ItemsFormPtr != nullptr)
    {
        CWaitCursor wait;
        numAdded = ItemsFormPtr->fillEmptySlots(static_cast<d2ce::EnumItemLocation>(locationID), static_cast<d2ce::EnumAltItemLocation>(altLocationId), gemCode);
    }
    else
    {
        CWaitCursor wait;
        numAdded = MainForm.fillEmptySlots(static_cast<d2ce::EnumItemLocation>(locationID), static_cast<d2ce::EnumAltItemLocation>(altLocationId), gemCode);
    }

    CString msg(_T("%zd item(s) of type "));
    msg += ItemTree.GetItemText(ItemTree.GetSelectedItem());
    msg += _T(" were added to ");

    CString temp;
    pLocationCombo->GetLBText(toIdx, temp);
    msg += temp;
    temp.Format(msg, numAdded);
    AfxMessageBox(temp, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2AddGemsForm::OnTvnSelchangedItemtree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    *pResult = 0;
    ItemTooltipBox.RedrawWindow();

    auto item = GetSelectedItem();
    auto pWnd = GetDlgItem(IDOK);
    if (pWnd != nullptr)
    {
        pWnd->EnableWindow(item == nullptr ? FALSE : TRUE);
    }
}
//---------------------------------------------------------------------------
void CD2AddGemsForm::OnNMDblclkItemtree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    *pResult = 0;
    OnBnClickedAdd();
}
//---------------------------------------------------------------------------
void CD2AddGemsForm::InitTree()
{
    auto& charInfo = MainForm.getCharacterInfo();
    std::map<std::string, d2ce::AvailableItemType> availableItemTypeMap;
    d2ce::ItemHelpers::getAvailableItems(availableItemTypeMap, charInfo.getDefaultItemVersion(), charInfo.isExpansionCharacter());
    auto iter = availableItemTypeMap.begin();
    auto iter_end = availableItemTypeMap.end();

    std::deque<AvailableItemFolder> parent;
    const d2ce::ItemType* pSourceItemType = nullptr;
    if (ItemPtr != nullptr)
    {
        pSourceItemType = &ItemPtr->getItemTypeHelper();
    }
    InitTreeControl(ItemTree, parent, iter, iter_end, AvailableItems, AvailableItemTypes, pSourceItemType);

    auto hTreeItem = ItemTree.GetSelectedItem();
    if (hTreeItem == NULL)
    {
        ItemTree.OpenPath(g_LastSelection);
    }
    else
    {
        ItemTree.EnsureVisible(hTreeItem);
    }
}
//---------------------------------------------------------------------------
const d2ce::Character* CD2AddGemsForm::GetCharacterInfo() const
{
    return &MainForm.getCharacterInfo();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2AddGemsForm::GetSelectedItem() const
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
