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
#include "D2MercenaryForm.h"
#include "D2RunewordForm.h"
#include "d2ce/helpers/ItemHelpers.h"
#include <utf8/utf8.h>
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    int FindPopupPosition(CMenu& parent, UINT childId)
    {
        auto numItems = parent.GetMenuItemCount();
        if (numItems <= 0)
        {
            return -1;
        }

        for (int i = 0; i < numItems; ++i)
        {
            auto id = parent.GetMenuItemID(i);
            if (id == -1) // popup
            {
                CMenu* pPopup = parent.GetSubMenu(i);
                if (pPopup != NULL)
                {
                    auto numChildItems = pPopup->GetMenuItemCount();
                    for (int j = 0; j < numChildItems; ++j)
                    {
                        if (pPopup->GetMenuItemID(j) == childId)
                        {
                            return i;
                        }
                    }
                }
            }
        }

        return -1;
    }

    CMenu* FindPopupByChild(CMenu& parent, UINT childId)
    {
        auto pos = FindPopupPosition(parent, childId);
        if (pos >= 0)
        {
            return parent.GetSubMenu(pos);
        }

        return nullptr;
    }

    CMenu* FindPopup(CMenu& parent, size_t idx = 0)
    {
        auto numItems = parent.GetMenuItemCount();
        if (numItems <= 0)
        {
            return nullptr;
        }

        size_t curIdx = 0;
        for (int i = 0; i < numItems; ++i)
        {
            auto id = parent.GetMenuItemID(i);
            if (id == -1) // popup
            {
                if (curIdx == idx)
                {
                    return parent.GetSubMenu(i);
                }
                ++curIdx;
            }
        }

        return nullptr;
    }

    template <class charT>
    class num_printer
        : public std::numpunct<charT>
    {
    public:
        num_printer(charT thousands, charT decimals) : thousands(thousands), decimals(decimals) {}
    protected:
        charT do_decimal_point() const { return decimals; }
        charT do_thousands_sep() const { return thousands; }
        std::string do_grouping() const { return "\3"; }
    private:
        charT thousands, decimals;
    };

    void ScaleImage(CDC* pDC, CBitmap& image, const CRect& rectLocation)
    {
        BITMAP bmp;
        image.GetBitmap(&bmp);

        // select the source CBitmap in a memory DC;
        CDC memSrcDc;
        memSrcDc.CreateCompatibleDC(pDC);
        memSrcDc.SelectObject(&image); //now bitmap is an instance of CBitmap class

        // Create your new CBitmap with the new desired size and select it into a destination memory DC
        CDC memDestDC;
        CBitmap image2;
        memDestDC.CreateCompatibleDC(pDC);
        image2.CreateCompatibleBitmap(&memSrcDc, rectLocation.Width(), rectLocation.Height());
        memDestDC.SelectObject(&image2);

        // StretchBlt from src to dest
        memDestDC.StretchBlt(0, 0, rectLocation.Width(), rectLocation.Height(), &memSrcDc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

        HGDIOBJ hbitmap_detach = image.Detach();
        if (hbitmap_detach)
        {
            DeleteObject(hbitmap_detach);
        }

        image.Attach(image2.Detach());
    }

    void MergeImage(CDC* pDC, CBitmap& image, CBitmap& overlay, const CRect& rect, bool center = false)
    {
        BITMAP bmp;
        image.GetBitmap(&bmp);

        BITMAP bmp2;
        overlay.GetBitmap(&bmp2);

        // select the source CBitmaps in a memory DC;
        CDC memSrcDc;
        memSrcDc.CreateCompatibleDC(pDC);
        memSrcDc.SelectObject(&image); //now bitmap is an instance of CBitmap class

        CDC memSrcDc2;
        memSrcDc2.CreateCompatibleDC(pDC);
        memSrcDc2.SelectObject(&overlay); //now bitmap is an instance of CBitmap class

        // Create your new CBitmap with the new desired size and select it into a destination memory DC
        CDC memDestDC;
        CBitmap image2;
        memDestDC.CreateCompatibleDC(pDC);
        image2.CreateCompatibleBitmap(&memSrcDc, bmp.bmWidth, bmp.bmHeight);
        memDestDC.SelectObject(&image2);

        auto cx = 0;
        auto cy = 0;
        if (center)
        {
            cx = (bmp.bmWidth - bmp2.bmWidth) / 2;
            cy = (bmp.bmHeight - bmp2.bmHeight) / 2;
        }

        // StretchBlt from src to dest
        memDestDC.BitBlt(0, 0, bmp.bmWidth, bmp.bmHeight, &memSrcDc, 0, 0, SRCCOPY);

        // combine bmps
        memDestDC.TransparentBlt(rect.left + cx, rect.top - cy, rect.Width(), rect.Height(), &memSrcDc2, 0, 0, bmp2.bmWidth, bmp2.bmHeight, ::GetPixel(memSrcDc2, 0, 0));

        HGDIOBJ hbitmap_detach = image.Detach();
        if (hbitmap_detach)
        {
            DeleteObject(hbitmap_detach);
        }

        image.Attach(image2.Detach());
    }

    bool CalcItemRect(const d2ce::Item& item, CStatic& invBox, CRect& rect, UINT id)
    {
        CSize slots;
        switch (id)
        {
        case IDC_INV_MERC_HEAD:
            slots = CSize(2, 2);
            break;

        case IDC_INV_MERC_RIGHT_ARM:
        case IDC_INV_MERC_TORSO:
        case IDC_INV_MERC_LEFT_ARM:
            slots = CSize(2, 4);
            break;
        }

        d2ce::ItemDimensions dimension;
        CSize invSlotSize;
        if (!item.getDimensions(dimension))
        {
            return false;
        }

        invBox.GetClientRect(&rect);
        invSlotSize.cx = rect.Width() / slots.cx;
        invSlotSize.cy = rect.Height() / slots.cy;
        rect.top = rect.bottom - dimension.Height * invSlotSize.cy;
        rect.right = rect.left + dimension.Width * invSlotSize.cx;
        return true;
    }

    void DrawItem(CPaintDC& dc, CBitmap& bmp, CStatic& box, CWnd& parent)
    {
        CRect rect;
        box.GetWindowRect(&rect);
        parent.ScreenToClient(&rect);

        CDC memDC;
        memDC.CreateCompatibleDC(&dc);
        CBitmap* pOld = memDC.SelectObject(&bmp);
        dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
        memDC.SelectObject(pOld);
    }

    BOOL AddChildToolInfoEnum(HWND hwnd, LPARAM lParam)
    {
        CToolTipCtrl* pToolTip = (CToolTipCtrl*)lParam;
        if (pToolTip == nullptr)
        {
            return TRUE;
        }

        CWnd* pWnd = CWnd::FromHandle(hwnd);
        if (!pWnd->IsKindOf(RUNTIME_CLASS(CD2EquippedItemStatic)) &&
            !pWnd->IsKindOf(RUNTIME_CLASS(CD2ItemsGridStatic)) &&
            !pWnd->IsKindOf(RUNTIME_CLASS(CEdit)) &&
            !pWnd->IsKindOf(RUNTIME_CLASS(CComboBox)))
        {
            return TRUE;
        }

        CRect rect;
        pWnd->GetWindowRect(&rect);
        pWnd->GetParent()->ScreenToClient(&rect);
        pToolTip->AddTool(pWnd->GetParent(), LPSTR_TEXTCALLBACK, rect, pWnd->GetDlgCtrlID());
        return TRUE;
    }
}

//---------------------------------------------------------------------------
// CD2MercenaryForm dialog

IMPLEMENT_DYNAMIC(CD2MercenaryForm, CDialogEx)

//---------------------------------------------------------------------------
CD2MercenaryForm::CD2MercenaryForm(CD2MainForm& form)
    : CDialogEx(CD2MercenaryForm::IDD, (CWnd*)&form), MainForm(form), Merc(form.getMercenaryInfo())
{
    Merc.fillMercInfo(OrigMerc);
}
//---------------------------------------------------------------------------
CD2MercenaryForm::~CD2MercenaryForm()
{
}
//---------------------------------------------------------------------------
bool CD2MercenaryForm::MercChanged() const
{
    return CtrlEditted.empty() ? false : true;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHAR_NAME, MercName);
    DDX_Control(pDX, IDC_MERC_HIRED, MercHired);
    DDX_Control(pDX, IDC_RESURRECTED_CHECK, MercDead);
    DDX_Control(pDX, IDC_CHAR_CLASS_CMB, MercClass);
    DDX_Control(pDX, IDC_DIFFICULTY, Difficulty);
    DDX_Control(pDX, IDC_TYPE_DASH_2, AttributeDash);
    DDX_Control(pDX, IDC_ATTRIBUTE, Attribute);
    DDX_Control(pDX, IDC_CHAR_LEVEL, MercLevel);
    DDX_Control(pDX, IDC_CHAR_STRENGTH, MercStrength);
    DDX_Control(pDX, IDC_CHAR_DEXTERITY, MercDexterity);
    DDX_Control(pDX, IDC_CHAR_DEFENSE, MercDefense);
    DDX_Control(pDX, IDC_CHAR_DAMAGE, MercDamage);
    DDX_Control(pDX, IDC_CUR_LIFE, MercLife);
    DDX_Control(pDX, IDC_CHAR_EXPERIENCE, Experience);
    DDX_Control(pDX, IDC_CHAR_RESIST_FIRE, ResistFire);
    DDX_Control(pDX, IDC_CHAR_RESIST_COLD, ResistCold);
    DDX_Control(pDX, IDC_CHAR_RESIST_LIGHTNING, ResistLightning);
    DDX_Control(pDX, IDC_CHAR_RESIST_POISON, ResistPoison);
    DDX_Control(pDX, IDC_INV_MERC_HEAD, InvMercHeadBox);
    DDX_Control(pDX, IDC_INV_MERC_RIGHT_ARM, InvMercHandRightBox);
    DDX_Control(pDX, IDC_INV_MERC_TORSO, InvMercTorsoBox);
    DDX_Control(pDX, IDC_INV_MERC_LEFT_ARM, InvMercHandLeftBox);
}
//---------------------------------------------------------------------------
BOOL CD2MercenaryForm::PreTranslateMessage(MSG* pMsg)
{
    CWnd* pWndFocus = GetFocus();
    if (pWndFocus != NULL && IsChild(pWndFocus))
    {
        UINT message = pMsg->message;
        if ((message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE ||
            message == WM_LBUTTONUP || message == WM_RBUTTONUP ||
            message == WM_MBUTTONUP) &&
            (GetKeyState(VK_LBUTTON) >= 0 && GetKeyState(VK_RBUTTON) >= 0 &&
                GetKeyState(VK_MBUTTON) >= 0))
        {
            CheckToolTipCtrl();
        }

        if (pMsg->message == WM_KEYDOWN)
        {
            if (pMsg->wParam == VK_RETURN)
            {
                TCHAR szClass[10];
                if (GetClassName(pWndFocus->m_hWnd, szClass, 10) &&
                    (lstrcmpi(szClass, _T("EDIT")) == 0))
                {
                    // pressing the ENTER key will take the focus to the next control
                    pMsg->wParam = VK_TAB;
                }
            }
        }
    }

    return __super::PreTranslateMessage(pMsg);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2MercenaryForm, CDialogEx)
    ON_EN_CHANGE(IDC_CHAR_LEVEL, &CD2MercenaryForm::OnEnChangeMercLevel)
    ON_EN_KILLFOCUS(IDC_CHAR_LEVEL, &CD2MercenaryForm::OnEnKillfocusMercLevel)
    ON_EN_CHANGE(IDC_CHAR_EXPERIENCE, &CD2MercenaryForm::OnEnChangeMercExperience)
    ON_EN_KILLFOCUS(IDC_CHAR_EXPERIENCE, &CD2MercenaryForm::OnEnKillfocusMercExperience)
    ON_CBN_SELCHANGE(IDC_DIFFICULTY, &CD2MercenaryForm::OnCbnSelchangeDifficulty)
    ON_CBN_SELCHANGE(IDC_ATTRIBUTE, &CD2MercenaryForm::OnCbnSelchangeAttribute)
    ON_CBN_SELCHANGE(IDC_CHAR_CLASS_CMB, &CD2MercenaryForm::OnCbnSelchangeMercClass)
    ON_CBN_SELCHANGE(IDC_CHAR_NAME, &CD2MercenaryForm::OnCbnSelchangeMercName)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDOK, &CD2MercenaryForm::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CD2MercenaryForm::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_MERC_HIRED, &CD2MercenaryForm::OnClickedMercHired)
    ON_BN_CLICKED(IDC_RESURRECTED_CHECK, &CD2MercenaryForm::OnClickedResurrectedCheck)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_ITEM_CONTEXT_FIX, &CD2MercenaryForm::OnItemContextFix)
    ON_COMMAND(ID_ITEM_CONTEXT_LOAD, &CD2MercenaryForm::OnItemContextLoad)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXDURABILITY, &CD2MercenaryForm::OnItemContextMaxdurability)
    ON_COMMAND(ID_ITEM_CONTEXT_INDESTRUCTIBLE, &CD2MercenaryForm::OnItemContextIndestructible)
    ON_COMMAND(ID_ITEM_CONTEXT_ADDSOCKET, &CD2MercenaryForm::OnItemContextAddsocket)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXSOCKETS, &CD2MercenaryForm::OnItemContextMaxsockets)
    ON_COMMAND(ID_ITEM_CONTEXT_PERSONALIZE, &CD2MercenaryForm::OnItemContextPersonalize)
    ON_COMMAND(ID_ITEM_CONTEXT_REMOVE_PERSONALIZATION, &CD2MercenaryForm::OnItemContextRemovePersonalization)
    ON_COMMAND(ID_ITEM_CONTEXT_APPLY_RUNEWORD, &CD2MercenaryForm::OnItemContextApplyruneword)
    ON_COMMAND(ID_ITEM_CONTEXT_EXPORT_ITEM, &CD2MercenaryForm::OnItemContextExportitem)
    ON_COMMAND(ID_ITEM_CONTEXT_REMOVE_ITEM, &CD2MercenaryForm::OnItemContextRemoveitem)
    ON_COMMAND(ID_ITEM_CONTEXT_MAKESUPERIORQUALITY, &CD2MercenaryForm::OnItemContextMakesuperiorquality) 
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADEITEMTIER, &CD2MercenaryForm::OnItemContextUpgradehighertier)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// D2MercenaryForm message handlers

//---------------------------------------------------------------------------
void CD2MercenaryForm::DisplayMercInfo()
{
    // set state of Status checkbox
    EnableMercInfoBox();
    auto bEnable = Merc.isHired();
    MercHired.SetCheck(bEnable ? 1 : 0);
    if (!bEnable)
    {
        UpdateModified();
        return;
    }

    MercDead.SetCheck(Merc.isDead() ? 1 : 0);

    UpdateMercNames();
    auto mercClassEnum = Merc.getClass();
    auto mercCass = mercClassEnum == d2ce::EnumMercenaryClass::None ? -1 : static_cast<std::underlying_type_t<d2ce::EnumMercenaryClass>>(mercClassEnum) - 1;
    SetInt(&MercClass, mercCass);
    SetInt(&Difficulty, static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(Merc.getDifficulty()));
    UpdateAttributes();

    d2ce::CharStats cs;
    Merc.fillMercStats(cs);
    SetInt(&MercLevel, cs.Level);
    SetInt(&MercStrength, cs.Strength);
    SetInt(&MercDexterity, cs.Dexterity);
    SetInt(&MercLife, cs.CurLife);
    SetInt(&Experience, cs.Experience);
    SetInt(&MercDefense, Merc.getDefenseRating());

    d2ce::BaseResistance resist;
    Merc.getResistance(resist);
    SetInt(&ResistFire, resist.Fire);
    SetInt(&ResistCold, resist.Cold);
    SetInt(&ResistLightning, resist.Lightning);
    SetInt(&ResistPoison, resist.Poison);

    CString sDamage;
    d2ce::BaseDamage damage;
    Merc.getDamage(damage);
    if (damage.Max != 0)
    {
        sDamage.Format(DamageFmt, damage.Min, damage.Max);
    }
    MercDamage.SetWindowText(sDamage);

    UpdateModified();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::EnableMercInfoBox()
{
    auto bEnable = Merc.isHired();
    auto bHasItem = Merc.getNumberOfItems() > 0 ? true : false;

    MercHired.EnableWindow(!bEnable || !bHasItem);
    MercDead.EnableWindow(bEnable);
    MercName.EnableWindow(bEnable);
    MercClass.EnableWindow(bEnable && !bHasItem);
    MercLevel.EnableWindow(bEnable);
    Difficulty.EnableWindow(bEnable);
    Attribute.EnableWindow(bEnable);
    Attribute.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
    AttributeDash.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
    Experience.EnableWindow(bEnable);
    MercStrength.EnableWindow(bEnable);
    MercDexterity.EnableWindow(bEnable);
    MercDamage.EnableWindow(bEnable);
    MercDefense.EnableWindow(bEnable);
    MercLife.EnableWindow(bEnable);
    ResistFire.EnableWindow(bEnable);
    ResistCold.EnableWindow(bEnable);
    ResistLightning.EnableWindow(bEnable);
    ResistPoison.EnableWindow(bEnable);
    InvMercHeadBox.EnableWindow(bEnable);
    InvMercHeadBox.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
    InvMercHandRightBox.EnableWindow(bEnable);
    InvMercHandRightBox.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
    InvMercTorsoBox.EnableWindow(bEnable);
    InvMercTorsoBox.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
    InvMercHandLeftBox.EnableWindow(bEnable);
    InvMercHandLeftBox.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
    if (!bEnable)
    {
        Attribute.ResetContent();
        CurAttributeClass = d2ce::EnumMercenaryClass::None;
        MercName.ResetContent();
        CurMercNameClass = d2ce::EnumMercenaryClass::None;
        MercDead.SetCheck(0);
        MercName.SetCurSel(-1);
        MercClass.SetCurSel(-1);
        Difficulty.SetCurSel(-1);
        Attribute.SetCurSel(-1);
        MercLevel.SetWindowText(_T(""));
        MercStrength.SetWindowText(_T(""));
        MercDexterity.SetWindowText(_T(""));
        MercDamage.SetWindowText(_T(""));
        MercDefense.SetWindowText(_T(""));
        Experience.SetWindowText(_T(""));
        MercLife.SetWindowText(_T(""));
        ResistFire.SetWindowText(_T(""));
        ResistCold.SetWindowText(_T(""));
        ResistLightning.SetWindowText(_T(""));
        ResistPoison.SetWindowText(_T(""));
    }
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::UpdateMercNames()
{
    if (!Merc.isHired())
    {
        MercName.ResetContent();
        CurMercNameClass = d2ce::EnumMercenaryClass::None;
        return;
    }

    auto curClass = Merc.getClass();
    if (CurMercNameClass == curClass)
    {
        MercName.SetCurSel(Merc.getNameId());
        return;
    }

    std::u16string uText;
    MercName.ResetContent();
    CurMercNameClass = curClass;
    for (const auto& name : d2ce::Mercenary::getMercNames(CurMercNameClass))
    {
        uText = utf8::utf8to16(name.c_str());
        MercName.AddString(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

    MercName.SetCurSel(Merc.getNameId());
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::UpdateAttributes()
{
    if (!Merc.isHired())
    {
        Attribute.ResetContent();
        Attribute.EnableWindow(FALSE);
        CurAttributeClass = d2ce::EnumMercenaryClass::None;
        return;
    }

    auto curClass = Merc.getClass();
    if (CurAttributeClass == curClass)
    {
        if (Attribute.GetCount() > 0)
        {
            Attribute.SetCurSel(Merc.getAttributeId());
        }
        return;
    }

    std::u16string uText;
    Attribute.ResetContent();
    CurAttributeClass = curClass;
    const auto& attribs = d2ce::Mercenary::getMercAttributes(CurAttributeClass, Merc.getVersion());
    for (const auto& name : attribs)
    {
        uText = utf8::utf8to16(name.c_str());
        Attribute.AddString(reinterpret_cast<LPCWSTR>(uText.c_str()));
    }

    if (Attribute.GetCount() <= 0)
    {
        Attribute.EnableWindow(FALSE);
        Attribute.ShowWindow(SW_HIDE);
        AttributeDash.ShowWindow(SW_HIDE);
        return;
    }

    Attribute.EnableWindow(TRUE);
    Attribute.ShowWindow(SW_SHOW);
    AttributeDash.ShowWindow(SW_SHOW);
    Attribute.SetCurSel(Merc.getAttributeId());
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::UpdateModified()
{
    d2ce::MercInfo merc;
    Merc.fillMercInfo(merc);

    if (merc.Id != OrigMerc.Id)
    {
        CtrlEditted.insert(MercHired.GetDlgCtrlID());
    }
    else
    {
        auto iter = CtrlEditted.find(MercHired.GetDlgCtrlID());
        if (iter != CtrlEditted.end())
        {
            CtrlEditted.erase(iter);
        }
    }

    if (merc.Dead != OrigMerc.Dead)
    {
        CtrlEditted.insert(MercDead.GetDlgCtrlID());
    }
    else
    {
        auto iter = CtrlEditted.find(MercDead.GetDlgCtrlID());
        if (iter != CtrlEditted.end())
        {
            CtrlEditted.erase(iter);
        }
    }

    if (merc.NameId != OrigMerc.NameId)
    {
        CtrlEditted.insert(MercName.GetDlgCtrlID());
    }
    else
    {
        auto iter = CtrlEditted.find(MercName.GetDlgCtrlID());
        if (iter != CtrlEditted.end())
        {
            CtrlEditted.erase(iter);
        }
    }

    if (merc.Type != OrigMerc.Type)
    {
        CtrlEditted.insert(MercName.GetDlgCtrlID());
        CtrlEditted.insert(MercClass.GetDlgCtrlID());
        CtrlEditted.insert(Difficulty.GetDlgCtrlID());
        CtrlEditted.insert(Attribute.GetDlgCtrlID());
    }
    else
    {
        auto iter = CtrlEditted.find(MercClass.GetDlgCtrlID());
        if (iter != CtrlEditted.end())
        {
            CtrlEditted.erase(iter);
        }
        
        iter = CtrlEditted.find(Difficulty.GetDlgCtrlID());
        if (iter != CtrlEditted.end())
        {
            CtrlEditted.erase(iter);
        }

        iter = CtrlEditted.find(Attribute.GetDlgCtrlID());
        if (iter != CtrlEditted.end())
        {
            CtrlEditted.erase(iter);
        }
    }

    if (merc.Experience != OrigMerc.Experience)
    {
        CtrlEditted.insert(Experience.GetDlgCtrlID());
        CtrlEditted.insert(MercLevel.GetDlgCtrlID());
    }
    else
    {
        auto iter = CtrlEditted.find(Experience.GetDlgCtrlID());
        if (iter != CtrlEditted.end())
        {
            CtrlEditted.erase(iter);
        }

        iter = CtrlEditted.find(MercLevel.GetDlgCtrlID());
        if (iter != CtrlEditted.end())
        {
            CtrlEditted.erase(iter);
        }
    }
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::LoadMercItemImages()
{
    if (!MainForm.isExpansionCharacter() || !Merc.isHired())
    {
        InvMercHeadBox.ShowWindow(SW_HIDE);
        InvMercHandRightBox.ShowWindow(SW_HIDE);
        InvMercTorsoBox.ShowWindow(SW_HIDE);
        InvMercHandLeftBox.ShowWindow(SW_HIDE);
        return;
    }

    InvMercHeadBox.ShowWindow(SW_SHOW);
    InvMercHeadBox.LoadItemImage();
    InvMercHandRightBox.ShowWindow(SW_SHOW);
    InvMercHandRightBox.LoadItemImage();
    InvMercTorsoBox.ShowWindow(SW_SHOW);
    InvMercTorsoBox.LoadItemImage();
    InvMercHandLeftBox.ShowWindow(SW_SHOW);
    InvMercHandLeftBox.LoadItemImage();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::refreshEquipped(const d2ce::Item& item)
{
    CWaitCursor wait;
    if (item.getLocation() != d2ce::EnumItemLocation::EQUIPPED)
    {
        return;
    }

    refreshEquipped(item.getEquippedId());
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::refreshEquipped(d2ce::EnumEquippedId id)
{
    switch (id)
    {
    case d2ce::EnumEquippedId::HEAD:
        InvMercHeadBox.LoadItemImage();
        break;

    case d2ce::EnumEquippedId::LEFT_ARM:
    case d2ce::EnumEquippedId::RIGHT_ARM:
        InvMercHandRightBox.LoadItemImage();
        InvMercHandLeftBox.LoadItemImage();
        break;

    case d2ce::EnumEquippedId::TORSO:
        InvMercTorsoBox.LoadItemImage();
        break;
    }
}
//---------------------------------------------------------------------------
std::string CD2MercenaryForm::ToStdString(const CWnd* Sender) const
{
    return (LPCSTR)CStringA(ToText(Sender));
}
//---------------------------------------------------------------------------
CString CD2MercenaryForm::ToText(const CWnd* Sender) const
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText;
        Sender->GetWindowText(sWindowText);
        return sWindowText;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        CString sWindowText;
        ((CComboBox*)Sender)->GetLBText(((CComboBox*)Sender)->GetCurSel(), sWindowText);
        return sWindowText;
    }

    return CString();
}
//---------------------------------------------------------------------------
CStringA CD2MercenaryForm::ToTextA(const CWnd* Sender) const
{
    return CStringA(ToText(Sender));
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::SetText(CWnd* Sender, const std::string& newValue)
{
    SetUTF8Text(Sender, newValue.c_str());
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::SetUTF8Text(CWnd* Sender, const char* newValue)
{
    auto uText = utf8::utf8to16(newValue);
    SetText(Sender, reinterpret_cast<LPCWSTR>(uText.c_str()));
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::SetText(CWnd* Sender, const char* newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText(newValue);
        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
    }
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::SetText(CWnd* Sender, const wchar_t* newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText(newValue);
        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
    }
}
//---------------------------------------------------------------------------
std::uint32_t CD2MercenaryForm::ToInt(const CWnd* Sender) const
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        std::string sWindowText(ToStdString(Sender));
        char* ptr = nullptr;
        std::uint32_t value = std::strtoul(sWindowText.c_str(), &ptr, 10);
        return value;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        return ((CComboBox*)Sender)->GetCurSel();
    }

    return 0;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::SetInt(CWnd* Sender, std::uint32_t newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText;
        switch (Sender->GetDlgCtrlID())
        {
        case IDC_CHAR_RESIST_FIRE:
        case IDC_CHAR_RESIST_COLD:
        case IDC_CHAR_RESIST_LIGHTNING:
        case IDC_CHAR_RESIST_POISON:
            sWindowText.Format(_T("%lu%%"), newValue);
            break;
        default:
            sWindowText.Format(_T("%lu"), newValue);
            break;
        }

        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
        return;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        ((CComboBox*)Sender)->SetCurSel(newValue);
        return;
    }
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::CheckToolTipCtrl()
{
    auto pToolTip = AfxGetModuleThreadState()->m_pToolTip;
    if (pToolTip != NULL && (pToolTip->GetOwner() != this || DYNAMIC_DOWNCAST(CD2ItemToolTipCtrl, pToolTip) == NULL))
    {
        pToolTip->DestroyWindow();
        delete pToolTip;
        AfxGetModuleThreadState()->m_pToolTip = NULL;
        pToolTip = NULL;
    }

    if (pToolTip == NULL)
    {
        CMFCToolTipInfo ttParams;
        ttParams.m_bVislManagerTheme = TRUE;
        auto pD2ItemToolTip = new CD2ItemToolTipCtrl(MainForm.getCharacterInfo(), &ttParams);
        pToolTip = pD2ItemToolTip;
        if (pToolTip->Create(this, TTS_ALWAYSTIP))
        {
            pD2ItemToolTip->SetCallback(this);

            EnumChildWindows(GetSafeHwnd(), AddChildToolInfoEnum, (LPARAM)pToolTip);

            pToolTip->SetDelayTime(TTDT_AUTOPOP, 0x7FFF);
            pToolTip->SendMessage(TTM_ACTIVATE, FALSE);
            AfxGetModuleThreadState()->m_pToolTip = pToolTip;
        }
    }
}
//---------------------------------------------------------------------------
bool CD2MercenaryForm::setItemRuneword(d2ce::Item& item, std::uint16_t id)
{
    auto preEquippedId = item.getEquippedId();
    if (MainForm.setItemRuneword(item, id))
    {
        refreshEquipped(preEquippedId);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2MercenaryForm::GetInvItem(UINT id, UINT /*offset*/) const
{
    switch (id)
    {
    case IDC_INV_MERC_HEAD:
        return InvMercHeadBox.GetInvItem();

    case IDC_INV_MERC_RIGHT_ARM:
        return InvMercHandRightBox.GetInvItem();

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.GetInvItem();

    case IDC_INV_MERC_LEFT_ARM:
        return InvMercHandLeftBox.GetInvItem();
        break;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2MercenaryForm::InvHitTest(UINT id, CPoint point, TOOLINFO* pTI) const
{
    ScreenToClient(&point);

    switch (id)
    {
    case IDC_INV_MERC_HEAD:
        return InvMercHeadBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_RIGHT_ARM:
        return InvMercHandRightBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_LEFT_ARM:
        return InvMercHandLeftBox.InvHitTest(point, pTI);
        break;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
std::optional<d2ce::CharStats> CD2MercenaryForm::getDisplayedCharStats() const
{
    return std::optional<d2ce::CharStats>();
}
//---------------------------------------------------------------------------
std::optional<d2ce::EnumCharClass> CD2MercenaryForm::getCharClass() const
{
    return std::optional<d2ce::EnumCharClass>();
}
//---------------------------------------------------------------------------
std::optional<d2ce::Mercenary*> CD2MercenaryForm::getMercInfo() const
{
    return &Merc;
}
//---------------------------------------------------------------------------
CSize CD2MercenaryForm::getInvGridSize(d2ce::EnumItemLocation /*locationId*/, d2ce::EnumAltItemLocation /*altPositionId*/) const
{
    return CSize(0, 0);
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2MercenaryForm::getInvGridItems(d2ce::EnumItemLocation /*locationId*/, d2ce::EnumAltItemLocation /*altPositionId*/) const
{
    static std::vector<std::reference_wrapper<d2ce::Item>> s_empty;
    return s_empty;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2MercenaryForm::getInvEquippedItem(d2ce::EnumEquippedId equippedId, d2ce::EnumItemInventory invType) const
{
    switch (invType)
    {
    case d2ce::EnumItemInventory::MERCENARY:
        for (const auto& item : Merc.getItems())
        {
            if (equippedId == item.getEquippedId())
            {
                return &item;
            }
        }
        break;
    }
    return nullptr;
}
//---------------------------------------------------------------------------
bool CD2MercenaryForm::getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    return MainForm.getItemBitmap(item, bitmap);
}
//---------------------------------------------------------------------------
bool CD2MercenaryForm::setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation /*altPositionId*/, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    switch (locationId)
    {
    case d2ce::EnumItemLocation::BUFFER:
        break;

    default:
        return false;
    }

    auto preEquippedId = item.getEquippedId();
    if (MainForm.setItemLocation(item, locationId, positionX, positionY, invType, pRemovedItem))
    {
        refreshEquipped(preEquippedId);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool CD2MercenaryForm::setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    switch (locationId)
    {
    case d2ce::EnumItemLocation::BUFFER:
        break;

    default:
        return false;
    }

    auto preEquippedId = item.getEquippedId(); 
    if (MainForm.setItemLocation(item, locationId, positionX, positionY, invType, pRemovedItem))
    {
        refreshEquipped(preEquippedId);
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool CD2MercenaryForm::setItemLocation(d2ce::Item& /*item*/, d2ce::EnumAltItemLocation /*altPositionId*/, std::uint16_t /*positionX*/, std::uint16_t /*positionY*/, d2ce::EnumItemInventory /*invType*/, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    return false;
}
//---------------------------------------------------------------------------
bool CD2MercenaryForm::setItemLocation(d2ce::Item& item, d2ce::EnumEquippedId equippedId, d2ce::EnumItemInventory /*invType*/, const d2ce::Item*& pRemovedItem)
{
    auto preEquippedId = item.getEquippedId();
    if (MainForm.setItemLocation(item, equippedId, d2ce::EnumItemInventory::MERCENARY, pRemovedItem))
    {
        refreshEquipped(preEquippedId);
        if ((preEquippedId != item.getEquippedId()))
        {
            refreshEquipped(item);
        }

        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
BOOL CD2MercenaryForm::OnInitDialog()
{
    __super::OnInitDialog();

    EnableToolTips(TRUE);
    CheckToolTipCtrl();

    //------------------
    // Create bold font:
    //------------------
    CFont* pWndFont = MercLevel.GetFont();
    LOGFONT lf;
    pWndFont->GetLogFont(&lf);
    lf.lfWeight = FW_BOLD;
    BoldFont.CreateFontIndirect(&lf);

    // Set text limite on edit boxes
    MercLevel.SetLimitText(2);
    Experience.SetLimitText(10);

    {
        std::string strValue;
        CWaitCursor wait;
        d2ce::LocalizationHelpers::GetStringTxtValue("MiniPanelHire", strValue, "Mercenary");
        auto uText = utf8::utf8to16(strValue);
        SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));

        if(d2ce::LocalizationHelpers::GetStringTxtValue("ItemStats1l", strValue))
        {
            auto pos = strValue.find(":");
            if (pos != strValue.npos)
            {
                strValue = strValue.substr(pos + 1);
                uText = utf8::utf8to16(strValue);
                DamageFmt = reinterpret_cast<LPCWSTR>(uText.c_str());
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("VerifyTransaction6", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_MERC_HIRED)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("sysmsg9", strValue))
        {
            uText = utf8::utf8to16(strValue);
            CStringW tmp(reinterpret_cast<LPCWSTR>(uText.c_str()));
            tmp.Trim();
            tmp.TrimRight(L".。");
            GetDlgItem(IDC_RESURRECTED_CHECK)->SetWindowText(tmp);
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strName", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_NAME)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strClass", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_CLASS)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrlvl", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_LEVEL)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrexp", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_EXPERIENCE)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrlif", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CUR_LIFE)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrstr", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_STRENGTH)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrdex", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_DEXTERITY)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrskm", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_DAMAGE)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrdef", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_DEFENSE)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrfir", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_RESIST_FIRE)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrcol", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_RESIST_COLD)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrlit", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_RESIST_LIGHTNING)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrpos", strValue))
        {
            uText = utf8::utf8to16(strValue);
            GetDlgItem(IDC_STATIC_CHAR_RESIST_POISON)->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        CString text;
        CStringA textA;
        CWnd* pWnd = nullptr;
        if (d2ce::LocalizationHelpers::GetStringTxtValue("ok", strValue))
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

        MercClass.ResetContent();
        for (const auto& name : d2ce::Mercenary::getMercClassNames())
        {
            if (!name.empty()) // skip none
            {
                uText = utf8::utf8to16(name.c_str());
                MercClass.AddString(reinterpret_cast<LPCWSTR>(uText.c_str()));
            }
        }

        static std::initializer_list<d2ce::EnumDifficulty> all_diff = { d2ce::EnumDifficulty::Normal, d2ce::EnumDifficulty::Nightmare, d2ce::EnumDifficulty::Hell };
        Difficulty.ResetContent();
        for (auto diff : all_diff)
        {
            d2ce::LocalizationHelpers::GetDifficultyStringTxtValue(diff, strValue);
            uText = utf8::utf8to16(strValue);
            Difficulty.AddString(reinterpret_cast<LPCWSTR>(uText.c_str()));
        }

        DisplayMercInfo();
        LoadMercItemImages();
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnEnChangeMercLevel()
{
    CtrlEditted.insert(MercLevel.GetDlgCtrlID());
}
void CD2MercenaryForm::OnEnKillfocusMercLevel()
{
    std::uint32_t level = std::min(std::max(ToInt(&MercLevel), std::uint32_t(1)), std::min(MainForm.getCharacterLevel(), d2ce::MERC_NUM_OF_LEVELS));
    if (level != Merc.getLevel())
    {
        Merc.setLevel(level);
    }

    DisplayMercInfo();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnEnChangeMercExperience()
{
    CtrlEditted.insert(Experience.GetDlgCtrlID());
}
void CD2MercenaryForm::OnEnKillfocusMercExperience()
{
    std::uint32_t experience = std::min(std::max(ToInt(&Experience), std::uint32_t(1)), d2ce::MERC_MAX_EXPERIENCE);
    if (experience != Merc.getExperience())
    {
        Merc.setExperience(experience);
    }

    DisplayMercInfo();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnCbnSelchangeDifficulty()
{
    auto difficulty = static_cast<d2ce::EnumDifficulty>(Difficulty.GetCurSel());
    if (difficulty != Merc.getDifficulty())
    {
        Merc.setDifficulty(difficulty);
        if (difficulty != Merc.getDifficulty())
        {
            // tried our best, but this Merc's level is too high!
            AfxMessageBox(_T("A Mercenary cannot be at a higher level than your character."), MB_OK | MB_ICONERROR);
        }
        DisplayMercInfo();
    }
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnCbnSelchangeAttribute()
{
    std::uint8_t attributeId = Attribute.GetCurSel() == -1 ? 0 : std::uint8_t(Attribute.GetCurSel());
    if (attributeId != Merc.getAttributeId())
    {
        Merc.setAttributeId(attributeId);
        if (attributeId != Merc.getAttributeId())
        {
            // tried our best, but this Merc's level is too high!
            AfxMessageBox(_T("A Mercenary cannot be at a higher level than your character."), MB_OK | MB_ICONERROR);

        }

        DisplayMercInfo();
    }
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnCbnSelchangeMercClass()
{
    auto mercCass = MercClass.GetCurSel() == -1 ? d2ce::EnumMercenaryClass::None : static_cast<d2ce::EnumMercenaryClass>(MercClass.GetCurSel() + 1);
    if (mercCass != Merc.getClass())
    {
        Merc.setClass(mercCass);
        if (mercCass != Merc.getClass())
        {
            // tried our best, but this Merc's level is too high!
            AfxMessageBox(_T("A Mercenary cannot be at a higher level than your character."), MB_OK | MB_ICONERROR);

        }

        DisplayMercInfo();
    }
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnCbnSelchangeMercName()
{
    auto nameId =std::uint16_t(MercName.GetCurSel());
    if (nameId != Merc.getNameId())
    {
        Merc.setNameId(nameId);
        if (nameId != Merc.getNameId())
        {
            SetInt(&MercName, Merc.getNameId());
        }

        UpdateModified();
    }
}
//---------------------------------------------------------------------------
BOOL CD2MercenaryForm::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    CString strTipText;
    UINT_PTR nID = pNMHDR->idFrom;

    CRect rect;
    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
        pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
    {
        ::GetWindowRect((HWND)nID, &rect);
        ScreenToClient(&rect);
        nID = ((UINT_PTR)(DWORD)::GetDlgCtrlID((HWND)nID));
    }

    if ((nID != 0) && (nID != ID_VIEW_STATUS_BAR))
    {
        switch (nID)
        {
        case IDC_CHAR_NAME:
            strTipText = _T("Mercenary's Name");
            break;

        case IDC_CHAR_CLASS_CMB:
            strTipText = _T("Mercenary's Class");
            break;

        case IDC_DIFFICULTY:
            strTipText = _T("Difficulty");
            break;

        case IDC_ATTRIBUTE:
            strTipText = _T("Type/Ability/Aura");
            break;

        case IDC_CHAR_LEVEL:
            strTipText.Format(_T("Level (%lu-%ld)"), Merc.getMinLevel(), Merc.getMaxLevel());
            break;

        case IDC_CHAR_EXPERIENCE:
            strTipText = _T("Mercenary's Current experience");
            break;

        case IDC_CUR_LIFE:
            strTipText = _T("Max Hit Points");
            break;

        case IDC_CHAR_STRENGTH:
            strTipText = _T("Strength");
            break;

        case IDC_CHAR_DEXTERITY:
            strTipText = _T("Dexterity");
            break;

        case IDC_INV_MERC_HEAD:
        case IDC_INV_MERC_RIGHT_ARM:
        case IDC_INV_MERC_TORSO:
        case IDC_INV_MERC_LEFT_ARM:
            strTipText = _T("N/A");
            break;
        }
    }
#ifndef _UNICODE
    if (pNMHDR->code == TTN_NEEDTEXTA)
        lstrcpyn(pTTTA->szText, strTipText, (sizeof(pTTTA->szText) / sizeof(pTTTA->szText[0])));
    else
        _mbstowcsz(pTTTW->szText, strTipText, (sizeof(pTTTW->szText) / sizeof(pTTTW->szText[0])));
#else
    if (pNMHDR->code == TTN_NEEDTEXTA)
        _wcstombsz(pTTTA->szText, strTipText, (sizeof(pTTTA->szText) / sizeof(pTTTA->szText[0])));
    else
        lstrcpyn(pTTTW->szText, strTipText, (sizeof(pTTTW->szText) / sizeof(pTTTW->szText[0])));
#endif
    *pResult = 0;

    ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

    return TRUE;
}
//---------------------------------------------------------------------------
HBRUSH CD2MercenaryForm::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    if (CTLCOLOR_EDIT == nCtlColor || CTLCOLOR_LISTBOX == nCtlColor ||
        (pWnd->IsKindOf(RUNTIME_CLASS(CEdit)) || pWnd->IsKindOf(RUNTIME_CLASS(CComboBox))))
    {
        if (CtrlEditted.find(pWnd->GetDlgCtrlID()) != CtrlEditted.end())
        {
            pDC->SelectObject(&BoldFont);
        }
    }

    return hbr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnBnClickedOk()
{
    UpdateData(TRUE); // save results
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnBnClickedCancel()
{
    Merc.updateMercInfo(OrigMerc);
    CtrlEditted.clear();
    __super::OnCancel();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnClickedMercHired()
{
    if (Merc.isHired())
    {
        Merc.setIsHired(false);
        DisplayMercInfo();
        return;
    }

    if (OrigMerc.Id != 0)
    {
        // restore old Mercenary
        Merc.updateMercInfo(OrigMerc);
        if (Merc.isHired())
        {
            DisplayMercInfo();
            return;
        }
    }

    Merc.setIsHired(true);

    // Set initial level to be 1 level lower then the current character level
    Merc.setLevel(std::max(std::uint32_t(2), MainForm.getCharacterLevel()) - 1);
    DisplayMercInfo();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnClickedResurrectedCheck()
{
    Merc.setIsDead(!Merc.isDead());
    UpdateModified();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2MercenaryForm::InvHitTest(CPoint point, TOOLINFO* pTI) const
{
    INT_PTR nHit = __super::OnToolHitTest(point, pTI);
    if (nHit == -1)
    {
        return nullptr;
    }

    // Make sure we have hit an item
    ClientToScreen(&point);
    return InvHitTest((UINT)nHit, point, pTI);
}
//---------------------------------------------------------------------------
INT_PTR CD2MercenaryForm::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);

    TOOLINFO* pTi = (pTI == nullptr) ? &ti : pTI;
    InvHitTest(point, pTi);

    UINT_PTR nHit = pTi->uId;
    if (pTi->uFlags & TTF_IDISHWND)
    {
        nHit = UINT_PTR(::GetDlgCtrlID(HWND(pTi->uId)));
    }

    return (INT_PTR)nHit;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CPoint hitTestPoint = point;
    ScreenToClient(&hitTestPoint);
    CurrItem = const_cast<d2ce::Item*>(InvHitTest(hitTestPoint));
    if (CurrItem == nullptr)
    {
        return;
    }

    const auto& itemType = CurrItem->getItemTypeHelper();
    bool isStackable = itemType.isStackable();
    bool isArmor = itemType.isArmor();
    bool isWeapon = itemType.isWeapon();
    bool canHaveSockets = CurrItem->canHaveSockets();
    bool canPersonalize = CurrItem->canPersonalize();
    bool isSocketed = CurrItem->isSocketed();
    bool canMakeSuperior = CurrItem->canMakeSuperior();
    bool canAddMagicalAffixes = CurrItem->canAddMagicalAffixes();
    bool canAddRareAffixes = CurrItem->canAddRareAffixes();
    bool canUpgradeTier = CurrItem->isUpgradableItem();

    bool removeQualityMenu = !canAddRareAffixes && !canAddMagicalAffixes && !canMakeSuperior && !canUpgradeTier;
    if (isArmor || isWeapon || isStackable)
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_ITEM_MENU));

        CMenu* pPopup = FindPopup(menu, 0);
        ENSURE(pPopup != NULL);

        pPopup->DeleteMenu(ID_ITEM_CONTEXT_IMPORT_ITEM, MF_BYCOMMAND);

        if (!isStackable)
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_LOAD, MF_BYCOMMAND);
        }

        auto pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_ADDSOCKET);
        if (pos >= 0)
        {
            if (!canHaveSockets)
            {
                pPopup->RemoveMenu(pos, MF_BYPOSITION);
            }
            else
            {
                CMenu* pSubPopup = pPopup->GetSubMenu(pos);
                if (pSubPopup != nullptr)
                {
                    if (isSocketed)
                    {
                        if (CurrItem->getMaxSocketCount() <= CurrItem->getSocketCount())
                        {
                            pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_ADDSOCKET, MF_BYCOMMAND);
                            pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_MAXSOCKETS, MF_BYCOMMAND);
                        }

                        if (CurrItem->getSocketedItemCount() == 0)
                        {
                            pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_UNSOCKET, MF_BYCOMMAND);
                        }
                    }
                    else
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_UNSOCKET, MF_BYCOMMAND);
                    }

                    if (CurrItem->getPossibleRunewords().empty())
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_APPLY_RUNEWORD, MF_BYCOMMAND);
                    }

                    if (pSubPopup->GetMenuItemCount() == 0)
                    {
                        pPopup->RemoveMenu(pos, MF_BYPOSITION);
                    }
                }
            }
        }

        pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_PERSONALIZE);
        if (pos >= 0)
        {
            if (!canPersonalize)
            {
                pPopup->RemoveMenu(pos, MF_BYPOSITION);
            }
            else
            {
                CMenu* pSubPopup = pPopup->GetSubMenu(pos);
                if (pSubPopup != nullptr)
                {
                    if (CurrItem->isPersonalized())
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_PERSONALIZE, MF_BYCOMMAND);
                    }
                    else
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_REMOVE_PERSONALIZATION, MF_BYCOMMAND);
                    }
                }
            }
        }

        if ((!isArmor && !isWeapon) || CurrItem->isIndestructible())
        {
            pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_FIX);
            if (pos >= 0)
            {
                pPopup->RemoveMenu(pos, MF_BYPOSITION);
            }
        }

        pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_ADDMAGICALAFFIXES);
        if (pos >= 0)
        {
            if (removeQualityMenu)
            {
                pPopup->RemoveMenu(pos, MF_BYPOSITION);
            }
            else
            {
                CMenu* pSubPopup = pPopup->GetSubMenu(pos);
                if (pSubPopup != nullptr)
                {
                    if (!canMakeSuperior)
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_MAKESUPERIORQUALITY, MF_BYCOMMAND);
                    }

                    if (!canUpgradeTier)
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADEITEMTIER, MF_BYCOMMAND);
                    }

                    if (!canAddMagicalAffixes)
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_ADDMAGICALAFFIXES, MF_BYCOMMAND);
                    }

                    if (!canAddRareAffixes)
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_ADDRAREAFFIXES, MF_BYCOMMAND);
                    }
                }
            }
        }

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
}   
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextFix()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.repairItem(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextLoad()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemMaxQuantity(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextMaxdurability()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemMaxDurability(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextIndestructible()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemIndestructible(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextAddsocket()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.addItemSocket(*CurrItem);
    refreshEquipped(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextMaxsockets()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemMaxSocketCount(*CurrItem);
    refreshEquipped(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextPersonalize()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.personalizeItem(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextRemovePersonalization()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.removeItemPersonalization(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextApplyruneword()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    CD2RunewordForm dlg(*this);
    if (dlg.DoModal() == IDOK)
    {
        refreshEquipped(*CurrItem);
    }
    SetFocus();
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextExportitem()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    auto uName = utf8::utf8to16(CurrItem->getDisplayedItemName());
    CString filename(reinterpret_cast<LPCWSTR>(uName.c_str()));
    filename.Replace(_T("\n"), _T("-"));
    filename += _T(".d2i");

    CFileDialog fileDialog(FALSE, _T("d2i"), filename,
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("Diablo II Item Files (*.d2i)|*.d2i|All Files (*.*)|*.*||"), this);

    if (fileDialog.DoModal() != IDOK)
    {
        return;
    }

    {
        CWaitCursor wait;
        if (!CurrItem->exportItem(fileDialog.GetPathName().GetString()))
        {
            CString msg(_T("Item export failed"));
            AfxMessageBox(msg, MB_OK | MB_ICONERROR);
        }
    }

    CString msg(_T("Item exported successfully"));
    AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextRemoveitem()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    const d2ce::Item* pRemovedItem = nullptr;
    setItemLocation(*CurrItem, d2ce::EnumItemLocation::BUFFER, 0, 0, d2ce::EnumItemInventory::BUFFER, pRemovedItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextMakesuperiorquality()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.makeItemSuperior(*CurrItem))
    {
        refreshEquipped(*CurrItem);
    }
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnItemContextUpgradehighertier()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.upgradeItemTier(*CurrItem))
    {
        refreshEquipped(*CurrItem);
    }
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
