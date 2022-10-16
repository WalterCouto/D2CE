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
#include "D2MainForm.h"
#include "D2ItemToolTipCtrl.h"

//---------------------------------------------------------------------------
class CD2ItemsGridCallback
{
public:
    virtual ~CD2ItemsGridCallback() = default;
    virtual std::optional<d2ce::EnumCharClass> getCharClass() const = 0;
    virtual std::optional<d2ce::CharStats> getDisplayedCharStats() const = 0;
    virtual std::optional<d2ce::Mercenary*> getMercInfo() const = 0;
    virtual CSize getInvGridSize(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId) const = 0;
    virtual const std::vector<std::reference_wrapper<d2ce::Item>>& getInvGridItems(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId) const = 0;
    virtual const d2ce::Item* getInvEquippedItem(d2ce::EnumEquippedId equippedId, d2ce::EnumItemInventory invType) const = 0;
    virtual bool getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const = 0;
    virtual bool setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem) = 0;
    virtual bool setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem) = 0;
    virtual bool setItemLocation(d2ce::Item& item, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem) = 0;
    virtual bool setItemLocation(d2ce::Item& item, d2ce::EnumEquippedId equippedId, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem) = 0;
};

//---------------------------------------------------------------------------
class CD2EquippedItemStatic : public CStatic
{
    friend class CD2ItemsForm;
    friend class CD2MercenaryForm;
    DECLARE_DYNAMIC(CD2EquippedItemStatic)

    // Construction
public:
    CD2EquippedItemStatic();

public:
    virtual ~CD2EquippedItemStatic();

    bool CanPlaceItem(const d2ce::Item& item, CPoint point);
    const d2ce::Item* PlaceItem(d2ce::Item& item, CPoint point, CBitmap& bitmap);
    bool GetInvBitmap(CBitmap& image, CPoint point, TOOLINFO* pTI = nullptr) const; // get item bitmap at point
    const d2ce::Item* GetInvItem() const; // get equipped item
    const d2ce::Item* InvHitTest(CPoint point, TOOLINFO* pTI = nullptr) const; // get item at point
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;  // get tool at point

    void SetUseAltImage(BOOL flag = FALSE);

    // Generated message map functions
protected:
    //{{AFX_MSG(CCharNameEdit)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

protected:
    virtual void PreSubclassWindow();
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    BOOL InitBackgroundImage();
    BOOL LoadBackgroundImage();
    BOOL LoadBackgroundImage(BOOL isAltImage);
    BOOL LoadItemImage();

    const d2ce::Item* GetInvEquippedItem() const;
    const d2ce::Item* GetInvEquippedItem(BOOL isAltImage) const;
    const d2ce::Item* GetInvEquippedItem(d2ce::EnumEquippedId equippedId) const;
    bool GetItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const;
    bool GetScaledItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const;
    bool SetItemLocation(d2ce::Item& item, const d2ce::Item* &pRemovedItem) const;
    CD2ItemsGridCallback* GetCallback() const;

    d2ce::EnumEquippedId GetEquippedId() const;
    d2ce::EnumEquippedId GetEquippedId(BOOL isAltImage) const;
    d2ce::EnumItemInventory GetItemInventory() const;
    std::optional<d2ce::Mercenary*> GetMercInfo() const;
    std::optional<d2ce::EnumCharClass> GetCharClass() const;
    std::optional<d2ce::CharStats> GetDisplayedCharStats() const;

    bool CanPlaceItemWith(const d2ce::Item& item) const;

protected:
    CBitmap InvImage;
    CBitmap InvAltImage;
    BOOL UseAltImage = FALSE;
    const d2ce::Item* InvItemPtr = nullptr;
    const d2ce::Item* InvAltItemPtr = nullptr;
};

//---------------------------------------------------------------------------
class CD2ItemsGridStatic : public CStatic
{
    friend class CD2ItemsForm; 
    friend class CD2SharedStashForm;
    DECLARE_DYNAMIC(CD2ItemsGridStatic)

    // Construction
public:
    CD2ItemsGridStatic();

public:
    virtual ~CD2ItemsGridStatic();

    bool CanPlaceItem(const d2ce::Item& item, CPoint point);
    const d2ce::Item* PlaceItem(d2ce::Item& item, CPoint point, CBitmap& bitmap);
    bool GetInvBitmap(CBitmap& image, CPoint point, TOOLINFO* pTI = nullptr) const; // get item bitmap at point
    const d2ce::Item* GetInvItem(UINT offset) const; // get item at grid offset
    const d2ce::Item* InvHitTest(CPoint point, TOOLINFO* pTI = nullptr) const; // get item at point
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;  // get tool at point

    // Generated message map functions
protected:
    //{{AFX_MSG(CCharNameEdit)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

protected:
    virtual void PreSubclassWindow();
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    BOOL LoadBackgroundImage();
    BOOL LoadItemImages();

    CSize GetInvGridSize() const;
    const std::vector<std::reference_wrapper<d2ce::Item>>& GetInvGridItems() const;
    bool GetItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const;
    bool GetScaledItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const;
    CRect GetInvRect(const d2ce::Item& item) const;
    CRect GetItemRectAtGridPos(const d2ce::Item& item, CPoint position) const;
    bool SetItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item* &pRemovedItem) const;
    CD2ItemsGridCallback* GetCallback() const;

    d2ce::EnumItemLocation GetLocation() const;
    d2ce::EnumAltItemLocation GetAltPositionId() const;

protected:
    CBitmap InvGridImage;
    CBitmap InvImage;
    CRect GridRect;

    CSize SlotSize;
    CSize GridBoxSize;

    // vector is size of size GridBoxSize.cx * GridBoxSize.cy and contains the item at that index
    std::vector<d2ce::Item*> InvGridItemIndex;
};

//---------------------------------------------------------------------------
class CD2ItemsForm : public CDialogEx, public CD2ItemToolTipCtrlCallback, public CD2ItemsGridCallback
{
    friend class CD2ItemsGrid;
    friend class CD2NewItemForm;
    friend class CD2GemsForm;
    friend class CD2AddGemsForm;
    friend class CD2SharedStashForm;
    friend class CD2RunewordForm;
    DECLARE_DYNAMIC(CD2ItemsForm)

public:
    CD2ItemsForm(CD2MainForm& form);
    virtual ~CD2ItemsForm();

    const d2ce::Item* InvHitTest(CPoint point, TOOLINFO* pTI = nullptr) const; // get item at point
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;  // get tool at point

// Dialog Data
    enum { IDD = IDD_ITEMS_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedSharedStashButton();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
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
    afx_msg void OnItemContextMakesuperiorquality();
    afx_msg void OnItemContextUpgradehighertier();
    afx_msg void OnItemContextMaxsocketsforallitems();
    afx_msg void OnItemContextSuperiorforallitems();
    afx_msg void OnItemContextHigherTierforallitems();
    afx_msg void OnItemContextPersonalize();
    afx_msg void OnItemContextRemovePersonalization();
    afx_msg void OnItemContextApplyruneword();
    afx_msg void OnItemContextCreateitem();
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
    afx_msg void OnClickedInvWeaponRadio();
    DECLARE_MESSAGE_MAP()
    BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

protected:
    CD2EquippedItemStatic InvHeadBox;
    CD2EquippedItemStatic InvNeckBox;
    CD2EquippedItemStatic InvHandRightBox;
    CD2EquippedItemStatic InvTorsoBox;
    CD2EquippedItemStatic InvHandLeftBox;
    CD2EquippedItemStatic InvGloveBox;
    CD2EquippedItemStatic InvRingRightBox;
    CD2EquippedItemStatic InvBeltBox;
    CD2EquippedItemStatic InvRingLeftBox;
    CD2EquippedItemStatic InvBootsBox;

    CStatic CorpseGroupBox;
    CD2EquippedItemStatic InvCorpseHeadBox;
    CD2EquippedItemStatic InvCorpseNeckBox;
    CD2EquippedItemStatic InvCorpseHandRightBox;
    CD2EquippedItemStatic InvCorpseTorsoBox;
    CD2EquippedItemStatic InvCorpseHandLeftBox;
    CD2EquippedItemStatic InvCorpseGloveBox;
    CD2EquippedItemStatic InvCorpseRingRightBox;
    CD2EquippedItemStatic InvCorpseBeltBox;
    CD2EquippedItemStatic InvCorpseRingLeftBox;
    CD2EquippedItemStatic InvCorpseBootsBox;

    CStatic MercGroupBox;
    CD2EquippedItemStatic InvMercHeadBox;
    CD2EquippedItemStatic InvMercHandRightBox;
    CD2EquippedItemStatic InvMercTorsoBox;
    CD2EquippedItemStatic InvMercHandLeftBox;

    CStatic GolemGroupBox;
    CD2EquippedItemStatic InvGolemBox;

    CButton InvWeaponIButton;
    CButton InvWeaponIIButton;

    mutable CD2ItemsGridStatic InvGrid;
    CStatic BeltGroupBox;
    mutable CD2ItemsGridStatic InvBeltGrid;
    mutable CD2ItemsGridStatic InvStashGrid;
    CStatic CubeGroupBox;
    mutable CD2ItemsGridStatic InvCubeGrid;

    CD2MainForm& MainForm;
    d2ce::Mercenary& Merc;
    d2ce::CharStats DisplayedCs;
    d2ce::Item* CurrItem = nullptr;
    std::array< std::uint8_t, 2> CurrItemLocation = { 0x00, 0x00 };
    HCURSOR CurrCursor = NULL;
    HCURSOR ItemCursor = NULL;
    HCURSOR TrashCursor = NULL;
    d2ce::Item* CurrDragItem = nullptr;
    d2ce::EnumItemInventory CurrDragItemInv = d2ce::EnumItemInventory::UNKNOWN;
    bool HasHoradricCube = false;
    bool HasBeltEquipped = false;
    BOOL IsWeaponII = FALSE;

    void LoadEquippedItemImages();
    void LoadCorpseItemImages();
    void LoadMercItemImages();
    void LoadGolemItemImages();
    void LoadGridItemImages();
    
    void CheckToolTipCtrl();

    size_t convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, d2ce::ItemFilter filter = d2ce::ItemFilter());
    bool updateGem(d2ce::Item& item, const std::array<std::uint8_t, 4>& newgem);
    bool upgradeGem(d2ce::Item& item);
    size_t upgradeGems(d2ce::ItemFilter filter = d2ce::ItemFilter());
    bool upgradePotion(d2ce::Item& item);
    size_t upgradePotions(d2ce::ItemFilter filter = d2ce::ItemFilter());
    bool upgradeToFullRejuvenationPotion(d2ce::Item& item);
    size_t upgradeRejuvenationPotions(d2ce::ItemFilter filter = d2ce::ItemFilter());
    bool refreshGrid(const d2ce::Item& item);
    bool refreshGrid(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId);
    bool refreshGrid(d2ce::EnumItemLocation locationId);
    bool refreshGrid(d2ce::EnumAltItemLocation altPositionId);
    bool refreshEquipped(const d2ce::Item& item);
    bool refreshEquipped(d2ce::EnumEquippedId id);
    size_t fillAllStackables(d2ce::ItemFilter filter = d2ce::ItemFilter());
    size_t repairAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());
    size_t maxDurabilityAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());
    size_t setIndestructibleAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());
    size_t maxSocketCountAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());
    size_t setSuperiorAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());
    size_t upgradeTierAllItems(d2ce::ItemFilter filter = d2ce::ItemFilter());

    bool addItem(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
    size_t fillEmptySlots(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode);
    void refreshAllGrids();

    // Helpers
    d2ce::EnumCharVersion getCharacterVersion() const;

    bool getHasBeltEquipped() const;
    size_t getMaxNumberOfItemsInBelt() const;
    size_t getNumberOfItemsInBelt() const;
    const std::vector<std::reference_wrapper<d2ce::Item>>& getItemsInBelt() const;

    size_t getNumberOfItemsInInventory() const;
    const std::vector<std::reference_wrapper<d2ce::Item>>& getItemsInInventory() const;

    size_t getNumberOfItemsInStash() const;
    const std::vector<std::reference_wrapper<d2ce::Item>>& getItemsInStash() const;

    bool getHasHoradricCube() const;
    size_t getNumberOfItemsInHoradricCube() const;
    const std::vector<std::reference_wrapper<d2ce::Item>>& getItemsInHoradricCube() const;

    bool getItemLocationDimensions(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, d2ce::ItemDimensions& dimensions) const;
    bool getItemLocationDimensions(d2ce::EnumItemLocation locationId, d2ce::ItemDimensions& dimensions) const;
    bool getItemLocationDimensions(d2ce::EnumAltItemLocation altPositionId, d2ce::ItemDimensions& dimensions) const;

    void SetCurrItemInfo(CPoint point);
    void ClearCurrItemInfo();

    void ResetCursor();
    bool GetInvBitmap(UINT id, CBitmap& image, CPoint point, TOOLINFO* pTI) const;
    bool CanPlaceItem(UINT id, const d2ce::Item& item, CPoint point);
    const d2ce::Item* PlaceItem(UINT id, d2ce::Item& item, CPoint point, CBitmap& bitmap);

    d2ce::ItemFilter GetCurrItemFilter() const;

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
};

//---------------------------------------------------------------------------
