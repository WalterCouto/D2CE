/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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
    BOOL LoadBackgroundImage(BOOL isAltImage = FALSE);
    BOOL LoadItemImage(const d2ce::Item& item, CBitmap& bitmap, BOOL isAltImage = FALSE);

    bool GetItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const;

protected:
    CBitmap InvImage;
    CBitmap InvAltImage;
    BOOL UseAltImage = FALSE;
    const d2ce::Item* InvItemPtr = nullptr;
    const d2ce::Item* InvAltItemPtr = nullptr;
};

//---------------------------------------------------------------------------
class CD2ItemsGridCallback
{
public:
    virtual ~CD2ItemsGridCallback() = default;
    virtual CSize getInvGridSize(UINT id) const = 0;
    virtual const std::vector<std::reference_wrapper<d2ce::Item>>& getInvGridItems(UINT id) const = 0;
    virtual bool getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const = 0;
};

//---------------------------------------------------------------------------
class CD2ItemsGridStatic : public CStatic
{
    friend class CD2ItemsForm;
    DECLARE_DYNAMIC(CD2ItemsGridStatic)


    // Construction
public:
    CD2ItemsGridStatic();

public:
    virtual ~CD2ItemsGridStatic();

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
    CRect GetInvRect(const d2ce::Item& item) const;

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
    friend class CD2GemsForm;
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

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnItemContextFix();
    afx_msg void OnItemContextLoad();
    afx_msg void OnItemContextMaxdurability();
    afx_msg void OnItemContextAddsocket();
    afx_msg void OnItemContextMaxsockets();
    afx_msg void OnItemContextUpgradeGem();
    afx_msg void OnItemContextUpgradePotion();
    afx_msg void OnItemContextUpgradeRejuvenation();
    afx_msg void OnItemContextGpsConvertor();
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
    d2ce::Item* CurrItem = nullptr;
    bool HasHoradricCube = false;
    bool HasBeltEquipped = false;
    BOOL IsWeaponII = FALSE;

    void LoadEquippedItemImages();
    void LoadCorpseItemImages();
    void LoadMercItemImages();
    void LoadGolemItemImages();
    void LoadGridItemImages();

    void CheckToolTipCtrl();

    bool updateGem(d2ce::Item& item, const std::array<std::uint8_t, 4>& newgem);
    bool upgradeGem(d2ce::Item& item);
    bool upgradePotion(d2ce::Item& item);
    bool upgradeToFullRejuvenationPotion(d2ce::Item& item);
    void refreshGrid(const d2ce::Item& item) const;

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

    // Inherited via CD2ItemToolTipCtrlCallback
    const d2ce::Item* GetInvItem(UINT id, UINT offset) const override;
    const d2ce::Item* InvHitTest(UINT id, CPoint point, TOOLINFO* pTI = nullptr) const override;

    // Inherited via CD2ItemsGridCallback
    CSize getInvGridSize(UINT id) const override;
    const std::vector<std::reference_wrapper<d2ce::Item>>& getInvGridItems(UINT id) const override;
    bool getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const override;
};
//---------------------------------------------------------------------------
