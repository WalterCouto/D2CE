/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
    Copyright (C) 2021-2023 Walter Couto

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

//---------------------------------------------------------------------------
class CD2MercenaryForm : public CDialogEx, CD2ItemToolTipCtrlCallback, public CD2ItemsGridCallback
{
    friend class CD2RunewordForm;

    DECLARE_DYNAMIC(CD2MercenaryForm)

public:
    CD2MercenaryForm(CD2MainForm& form);
    virtual ~CD2MercenaryForm();

    bool MercChanged() const;

    const d2ce::Item* InvHitTest(CPoint point, TOOLINFO* pTI = nullptr) const; // get item at point
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

// Dialog Data
    enum { IDD = IDD_MERC_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // Implementation
protected:
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnEnChangeMercLevel();
    afx_msg void OnEnKillfocusMercLevel();
    afx_msg void OnEnChangeMercExperience();
    afx_msg void OnEnKillfocusMercExperience();
    afx_msg void OnCbnSelchangeDifficulty();
    afx_msg void OnCbnSelchangeAttribute();
    afx_msg void OnCbnSelchangeMercClass();
    afx_msg void OnCbnSelchangeMercName();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnClickedMercHired();
    afx_msg void OnClickedResurrectedCheck();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnItemContextFix();
    afx_msg void OnItemContextLoad();
    afx_msg void OnItemContextMaxdurability();
    afx_msg void OnItemContextIndestructible();
    afx_msg void OnItemContextAddsocket();
    afx_msg void OnItemContextMaxsockets();
    afx_msg void OnItemContextPersonalize();
    afx_msg void OnItemContextRemovePersonalization();
    afx_msg void OnItemContextApplyruneword();
    afx_msg void OnItemContextExportitem();
    afx_msg void OnItemContextRemoveitem();
    afx_msg void OnItemContextMakesuperiorquality();
    afx_msg void OnItemContextUpgradehighertier();
    afx_msg void OnItemContextChangeEthereal();
    DECLARE_MESSAGE_MAP()
    BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

private:
    CButton MercHired;
    CButton MercDead;
    CComboBox MercName;
    d2ce::EnumMercenaryClass CurMercNameClass = d2ce::EnumMercenaryClass::None;
    CButton CharStatusHired;
    CButton CharStatusDead;
    CComboBox MercClass;
    CComboBox Difficulty;
    CComboBox Attribute;
    d2ce::EnumMercenaryClass CurAttributeClass = d2ce::EnumMercenaryClass::None;
    CStatic AttributeDash;
    CEdit MercLevel;
    CEdit MercStrength;
    CEdit MercDexterity;
    CEdit MercLife;
    CEdit Experience;
    CEdit MercDefense;
    CEdit MercDamage;
    CEdit ResistFire;
    CEdit ResistCold;
    CEdit ResistLightning;
    CEdit ResistPoison;
    CD2EquippedItemStatic InvMercHeadBox;
    CD2EquippedItemStatic InvMercNeckBox;
    CD2EquippedItemStatic InvMercHandRightBox;
    CD2EquippedItemStatic InvMercTorsoBox;
    CD2EquippedItemStatic InvMercHandLeftBox;
    CD2EquippedItemStatic InvMercGloveBox;
    CD2EquippedItemStatic InvMercRingRightBox;
    CD2EquippedItemStatic InvMercBeltBox;
    CD2EquippedItemStatic InvMercRingLeftBox;
    CD2EquippedItemStatic InvMercBootsBox;


    std::set<UINT> CtrlEditted;

    CFont BoldFont;

    CD2MainForm& MainForm;
    d2ce::Mercenary& Merc;
    d2ce::MercInfo OrigMerc;
    d2ce::CharStats Cs;
    d2ce::Item* CurrItem = nullptr;

    CString DamageFmt = _T("%d to %d");

    void DisplayMercInfo();
    void EnableMercInfoBox();
    void UpdateMercNames();
    void UpdateAttributes();
    void UpdateModified();
    void LoadMercItemImages();
    void refreshEquipped(const d2ce::Item& item);
    void refreshEquipped(d2ce::EnumEquippedId id);

    std::string ToStdString(const CWnd* Sender) const; // UTF-8
    CString ToText(const CWnd* Sender) const;          // UTF-16
    CStringA ToTextA(const CWnd* Sender) const;        // ANSI
    void SetText(CWnd* Sender, const std::string& newValue); // UTF-8
    void SetUTF8Text(CWnd* Sender, const char* newValue);    // UTF-8
    void SetText(CWnd* Sender, const char* newValue);        // ANSI
    void SetText(CWnd* Sender, const wchar_t* newValue);     // UTF-16
    std::uint32_t ToInt(const CWnd* Sender) const;
    void SetInt(CWnd* Sender, std::uint32_t newValue);

    void CheckToolTipCtrl();

    bool setItemRuneword(d2ce::Item& item, std::uint16_t id);

    // Inherited via CD2ItemToolTipCtrlCallback
    const d2ce::Item* GetInvItem(UINT id, UINT offset) const override;
    virtual const d2ce::Item* InvHitTest(UINT id, CPoint point, TOOLINFO* pTI = nullptr) const override;

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
