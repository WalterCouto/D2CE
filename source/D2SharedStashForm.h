/*
    Diablo II Character Editor
    Copyright (C) 2022 Walter Couto

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
#include "D2ItemsForm.h"
#include "SharedStash.h"

//---------------------------------------------------------------------------
class CD2SharedStashForm : public CDialogEx, public CD2ItemToolTipCtrlCallback, public CD2ItemsGridCallback
{
    friend class CD2ItemsGrid;
    friend class CD2GemsForm;
    friend class CD2AddGemsForm; 
    friend class CD2RunewordForm;
    DECLARE_DYNAMIC(CD2SharedStashForm)

public:
    CD2SharedStashForm(CD2MainForm& form);
    CD2SharedStashForm(CD2ItemsForm& form);
    virtual ~CD2SharedStashForm();

    const d2ce::Item* InvHitTest(CPoint point, TOOLINFO* pTI = nullptr) const; // get item at point
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;  // get tool at point

// Dialog Data
    enum { IDD = IDD_SHARED_STASH_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnItemContextFix();
    afx_msg void OnItemContextFixallitems();
    afx_msg void OnItemContextLoad();
    afx_msg void OnItemContextMaxfillstackables();
    afx_msg void OnItemContextMaxdurability();
    afx_msg void OnItemContextMaxdurabilityforallitems();
    afx_msg void OnItemContextIndestructible();
    afx_msg void OnItemContextIndestructibleforallitems();
    afx_msg void OnItemContextAddsocket();
    afx_msg void OnItemContextMaxsockets();
    afx_msg void OnItemContextUnsocket();
    afx_msg void OnItemContextMaxsocketsforallitems();
    afx_msg void OnItemContextPersonalize();
    afx_msg void OnItemContextRemovePersonalization();
    afx_msg void OnItemContextApplyruneword();
    afx_msg void OnItemContextImportitem();
    afx_msg void OnItemContextExportitem();
    afx_msg void OnItemContextRemoveitem();
    afx_msg void OnItemContextUpgradeGem();
    afx_msg void OnItemContextUpgradeGems();
    afx_msg void OnItemContextUpgradePotion();
    afx_msg void OnItemContextUpgradePotions();
    afx_msg void OnItemContextUpgradeRejuvenation();
    afx_msg void OnItemContextUpgradeRejuvenations();
    afx_msg void OnItemContextGpsConvertor();
    afx_msg void OnItemContextGpsCreator();
    afx_msg void OnEnKillfocusGoldInStash();
    afx_msg void OnCbnSelchangeStashPageCmb();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    DECLARE_MESSAGE_MAP()
    BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

protected:
    mutable CD2ItemsGridStatic InvStashGrid;
    
    CComboBox StashPage;
    CEdit GoldInStash;

    CD2MainForm& MainForm;
    d2ce::SharedStash& Stash;
    d2ce::Item* CurrItem = nullptr;
    HCURSOR CurrCursor = NULL;
    HCURSOR ItemCursor = NULL;
    d2ce::Item* CurrDragItem = nullptr;
    size_t CurrDragPage = 0;
    void LoadGridItemImages();

    void CheckToolTipCtrl();

    size_t getCurrentPage() const;

    size_t convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem);
    size_t convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, size_t page);
    bool updateGem(d2ce::Item& item, const std::array<std::uint8_t, 4>& newgem);
    bool upgradeGem(d2ce::Item& item);
    size_t upgradeGems();
    size_t upgradeGems(size_t page);
    bool upgradePotion(d2ce::Item& item);
    size_t upgradePotions();
    size_t upgradePotions(size_t page);
    bool upgradeToFullRejuvenationPotion(d2ce::Item& item);
    size_t upgradeRejuvenationPotions();
    size_t upgradeRejuvenationPotions(size_t page);
    size_t fillAllStackables();
    size_t fillAllStackables(size_t page);
    size_t repairAllItems();
    size_t repairAllItems(size_t page);
    size_t maxDurabilityAllItems();
    size_t maxDurabilityAllItems(size_t page);
    size_t setIndestructibleAllItems();
    size_t setIndestructibleAllItems(size_t page);
    size_t maxSocketCountAllItems();
    size_t maxSocketCountAllItems(size_t page);

    bool repairItem(d2ce::Item& item);
    bool setItemMaxQuantity(d2ce::Item& item);
    bool setItemMaxDurability(d2ce::Item& item);
    bool addItemSocket(d2ce::Item& item);
    bool setItemMaxSocketCount(d2ce::Item& item);
    bool removeSocketedItems(d2ce::Item& item);
    bool personalizeItem(d2ce::Item& item);
    bool removeItemPersonalization(d2ce::Item& item);
    bool setItemIndestructible(d2ce::Item& item);

    bool addItem(std::array<std::uint8_t, 4>& strcode, size_t page);
    size_t fillEmptySlots(std::array<std::uint8_t, 4>& strcode);
    size_t fillEmptySlots(std::array<std::uint8_t, 4>& strcode, size_t page);
    void refreshGrid();

    size_t getNumberOfItems() const;
    size_t getNumberOfItems(size_t page) const;

    const std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> getItems() const; // items for all pages
    const std::vector<std::reference_wrapper<d2ce::Item>>& getItems(size_t page) const;

    bool getDimensions(d2ce::ItemDimensions& dimensions) const;

    void SetCurrItemInfo(CPoint point);
    void ClearCurrItemInfo();

    void ResetCursor();
    bool GetInvBitmap(UINT id, CBitmap& image, CPoint point, TOOLINFO* pTI) const;
    bool CanPlaceItem(UINT id, const d2ce::Item& item, CPoint point);
    const d2ce::Item* PlaceItem(UINT id, d2ce::Item& item, CPoint point, CBitmap& bitmap);

    bool setItemRuneword(d2ce::Item& item, std::uint16_t id);

    // Inherited via CD2ItemToolTipCtrlCallback
    const d2ce::Item* GetInvItem(UINT id, UINT offset) const override;
    const d2ce::Item* InvHitTest(UINT id, CPoint point, TOOLINFO* pTI = nullptr) const override;

    // Inherited via CD2ItemsGridCallback
    std::optional<d2ce::EnumCharClass> getCharClass() const override;
    std::optional<d2ce::CharStats> getDisplayedCharStats() const override;
    std::optional<d2ce::Mercenary*> getMercInfo() const override;
    CSize getInvGridSize(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId) const override;
    const std::vector<std::reference_wrapper<d2ce::Item>>& getInvGridItems(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId) const override;
    const d2ce::Item* getInvEquippedItem(d2ce::EnumEquippedId equippedId, d2ce::EnumItemInventory invType) const override;
    bool getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const override;
    bool setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem) override;
    bool setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem) override;
    bool setItemLocation(d2ce::Item& item, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem) override;
    bool setItemLocation(d2ce::Item& item, d2ce::EnumEquippedId equippedId, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem) override;

private:
    std::string ToStdString(const CWnd* Sender) const;
    CString ToText(const CWnd* Sender) const;
    std::uint32_t ToInt(const CWnd* Sender) const;
    void SetInt(CWnd* Sender, std::uint32_t newValue);
};

//---------------------------------------------------------------------------
