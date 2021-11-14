/*
    Diablo II Character Editor
    Copyright (C) 2021 Walter Couto

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
#include "D2ItemsForm.h"
#include "afxdialogex.h"
#include "D2MainForm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
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

        // BitBlt from src to dest
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
        case IDC_INV_HEAD:
        case IDC_INV_CORPSE_HEAD:
        case IDC_INV_MERC_HEAD:
        case IDC_INV_GLOVE:
        case IDC_INV_CORPSE_GLOVE:
        case IDC_INV_BOOTS:
        case IDC_INV_CORPSE_BOOTS:
            slots = CSize(2, 2);
            break;

        case IDC_INV_NECK:
        case IDC_INV_CORPSE_NECK:
        case IDC_INV_RING_RIGHT:
        case IDC_INV_CORPSE_RING_RIGHT:
        case IDC_INV_RING_LEFT:
        case IDC_INV_CORPSE_RING_LEFT:
            slots = CSize(1, 1);
            break;

        case IDC_INV_HAND_RIGHT:
        case IDC_INV_CORPSE_HAND_RIGHT:
        case IDC_INV_MERC_HAND_RIGHT:
        case IDC_INV_TORSO:
        case IDC_INV_CORPSE_TORSO:
        case IDC_INV_MERC_TORSO:
        case IDC_INV_HAND_LEFT:
        case IDC_INV_CORPSE_HAND_LEFT:
        case IDC_INV_MERC_HAND_LEFT:
        case IDC_INV_GOLEM:
            slots = CSize(2, 4);
            break;

        case IDC_INV_BELT:
        case IDC_INV_CORPSE_BELT:
            slots = CSize(2, 1);
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

        // determine if this is a rebar:
        CWnd* pWnd = CWnd::FromHandle(hwnd);
        if (!pWnd->IsKindOf(RUNTIME_CLASS(CD2EquippedItemStatic)) && 
            !pWnd->IsKindOf(RUNTIME_CLASS(CD2ItemsGridStatic)))
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
// CD2EquippedItemStatic

IMPLEMENT_DYNAMIC(CD2EquippedItemStatic, CStatic)

CD2EquippedItemStatic::CD2EquippedItemStatic()
{
}
//---------------------------------------------------------------------------
CD2EquippedItemStatic::~CD2EquippedItemStatic()
{
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2EquippedItemStatic::GetInvItem() const
{
    return UseAltImage ? InvAltItemPtr : InvItemPtr;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2EquippedItemStatic::InvHitTest(CPoint point, TOOLINFO* pTI) const
{
    auto& invImage = UseAltImage ? InvAltImage : InvImage;
    if (invImage.GetSafeHandle() == 0)
    {
        return nullptr;
    }

    CRect invBox;
    GetWindowRect(&invBox);
    GetParent()->ScreenToClient(&invBox);
    if (!PtInRect(&invBox, point))
    {
        return nullptr;
    }

    auto pItem = GetInvItem();
    if (pItem != nullptr && pTI != nullptr)
    {
        pTI->hwnd = GetParent()->GetSafeHwnd();
        pTI->uId = (UINT_PTR)GetSafeHwnd();
        pTI->lpszText = LPSTR_TEXTCALLBACK;
        pTI->uFlags |= TTF_IDISHWND;
    }

    return pItem;
}
//---------------------------------------------------------------------------
INT_PTR CD2EquippedItemStatic::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);

    TOOLINFO* pTi = (pTI == nullptr) ? &ti : pTI;
    if (InvHitTest(point, pTi) == nullptr)
    {
        return (INT_PTR)-1;
    }

    UINT_PTR nHit = pTi->uId;
    if (pTi->uFlags & TTF_IDISHWND)
    {
        nHit = (UINT)::GetDlgCtrlID(HWND(pTi->uId));
    }

    return (INT_PTR)nHit;
}
//---------------------------------------------------------------------------
void CD2EquippedItemStatic::SetUseAltImage(BOOL flag)
{
    if (flag && InvAltImage.GetSafeHandle() == 0)
    {
        flag = FALSE;
    }

    UseAltImage = flag;
    Invalidate();
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2EquippedItemStatic, CStatic)
    //{{AFX_MSG_MAP(CD2EquippedItemStatic)
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
void CD2EquippedItemStatic::PreSubclassWindow()
{
    __super::PreSubclassWindow();
    ModifyStyle(0, SS_OWNERDRAW);

    InitBackgroundImage();
}
//---------------------------------------------------------------------------
void CD2EquippedItemStatic::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}
//---------------------------------------------------------------------------
BOOL CD2EquippedItemStatic::InitBackgroundImage()
{
    if (!LoadBackgroundImage())
    {
        return FALSE;
    }

    LoadBackgroundImage(TRUE);
    return TRUE;
}
//---------------------------------------------------------------------------
BOOL CD2EquippedItemStatic::LoadBackgroundImage(BOOL isAltImage)
{
    switch (GetDlgCtrlID())
    {
    case IDC_INV_HAND_RIGHT:
    case IDC_INV_CORPSE_HAND_RIGHT:
    case IDC_INV_HAND_LEFT:
    case IDC_INV_CORPSE_HAND_LEFT:
        break;

    default:
        if (isAltImage)
        {
            // does not support Alternative image
            return FALSE;
        }
        break;
    }

    auto& invImage = isAltImage ? InvAltImage : InvImage;
    if (invImage.GetSafeHandle() != 0)
    {
        invImage.DeleteObject();
    }

    switch (GetDlgCtrlID())
    {
    case IDC_INV_HEAD:
    case IDC_INV_CORPSE_HEAD:
    case IDC_INV_MERC_HEAD:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_HELM_GLOVE_0_1)));
        break;

    case IDC_INV_GLOVE:
    case IDC_INV_CORPSE_GLOVE:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_HELM_GLOVE_0_0)));
        break;

    case IDC_INV_BOOTS:
    case IDC_INV_CORPSE_BOOTS:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_BOOTS)));
        break;

    case IDC_INV_NECK:
    case IDC_INV_CORPSE_NECK:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_RING_AMULET_0_0)));
        break;

    case IDC_INV_RING_RIGHT:
    case IDC_INV_CORPSE_RING_RIGHT:
    case IDC_INV_RING_LEFT:
    case IDC_INV_CORPSE_RING_LEFT:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_RING_AMULET_0_1)));
        break;

    case IDC_INV_HAND_RIGHT:
    case IDC_INV_CORPSE_HAND_RIGHT:
    case IDC_INV_HAND_LEFT:
    case IDC_INV_CORPSE_HAND_LEFT:
    case IDC_INV_MERC_HAND_RIGHT:
    case IDC_INV_MERC_HAND_LEFT:
    case IDC_INV_GOLEM:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_WEAPONS)));
        break;

    case IDC_INV_TORSO:
    case IDC_INV_CORPSE_TORSO:
    case IDC_INV_MERC_TORSO:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_ARMOR)));
        break;

    case IDC_INV_BELT:
    case IDC_INV_CORPSE_BELT:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_BELT)));
        break;
    }

    auto pDC = GetParent()->GetDC();
    CRect rect;
    GetClientRect(rect);
    if (invImage.GetSafeHandle() != 0)
    {
        ScaleImage(pDC, invImage, rect);
    }

    return (invImage.GetSafeHandle() != 0) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
BOOL CD2EquippedItemStatic::LoadItemImage(const d2ce::Item& item, CBitmap& bitmap, BOOL isAltImage)
{
    switch (GetDlgCtrlID())
    {
    case IDC_INV_HAND_RIGHT:
    case IDC_INV_CORPSE_HAND_RIGHT:
    case IDC_INV_HAND_LEFT:
    case IDC_INV_CORPSE_HAND_LEFT:
        break;

    default:
        if (isAltImage)
        {
            // does not support Alternative image
            InvAltItemPtr = nullptr;
        }
        break;
    }

    auto& invItemPtr = isAltImage ? InvAltItemPtr : InvItemPtr;
    auto& invImage = isAltImage ? InvAltImage : InvImage;

    invItemPtr = nullptr;
    if (invImage.GetSafeHandle() == 0)
    {
        if (!LoadBackgroundImage(isAltImage))
        {
            return FALSE;
        }
    }

    if (bitmap.GetSafeHandle() == 0)
    {
        return FALSE;
    }

    CRect rect;
    if (!CalcItemRect(item, *this, rect, GetDlgCtrlID()))
    {
        return FALSE;
    }

    invItemPtr = &item;
    auto pDC = GetParent()->GetDC();
    ScaleImage(pDC, bitmap, rect);
    MergeImage(pDC, invImage, bitmap, rect, true);
    return TRUE;
}
//---------------------------------------------------------------------------
BOOL CD2EquippedItemStatic::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);

    CBitmap& bitmap = UseAltImage ? InvAltImage : InvImage;

    // If no bitmap selected, simply erase the background as per normal and return
    if (!bitmap.GetSafeHandle())
    {
        CBrush backBrush(::GetSysColor(COLOR_3DFACE)); // (this is meant for dialogs)
        CBrush* pOldBrush = pDC->SelectObject(&backBrush);

        pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
        pDC->SelectObject(pOldBrush);

        return TRUE;
    }

    // We have a bitmap - draw it.
    BITMAP bmp;
    bitmap.GetBitmap(&bmp);

    // Create compatible memory DC using the controls DC
    CDC dcMem;
    VERIFY(dcMem.CreateCompatibleDC(pDC));

    // Select bitmap into memory DC.
    CBitmap* pBmpOld = dcMem.SelectObject(&bitmap);

    // BitBlt bitmap onto static's client area
    pDC->BitBlt(rect.left, rect.top, bmp.bmWidth, bmp.bmHeight, &dcMem, 0, 0, SRCCOPY);
    dcMem.SelectObject(pBmpOld);

    return TRUE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// CD2ItemsGridStatic
IMPLEMENT_DYNAMIC(CD2ItemsGridStatic, CStatic)

CD2ItemsGridStatic::CD2ItemsGridStatic()
{
}
//---------------------------------------------------------------------------
CD2ItemsGridStatic::~CD2ItemsGridStatic()
{
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsGridStatic::GetInvItem(UINT offset) const
{
    if ((InvImage.GetSafeHandle() == 0) || (offset >= UINT(GridBoxSize.cx * GridBoxSize.cy)))
    {
        return nullptr;
    }

    return InvGridItemIndex[offset];
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsGridStatic::InvHitTest(CPoint point, TOOLINFO* pTI) const
{
    if (InvImage.GetSafeHandle() == 0)
    {
        return nullptr;
    }

    CRect invBox;
    GetWindowRect(&invBox);
    GetParent()->ScreenToClient(&invBox);
    if (!PtInRect(&invBox, point))
    {
        return nullptr;
    }

    point.x = (point.x - invBox.left) / SlotSize.cx;
    if (point.x < 0 || point.x > GridBoxSize.cx)
    {
        return nullptr;
    }

    point.y = (point.y - invBox.top) / SlotSize.cy;
    if (point.y < 0 || point.y > GridBoxSize.cy)
    {
        return nullptr;
    }

    UINT offset = (UINT)point.y * (UINT)GridBoxSize.cx + (UINT)point.x;
    auto pItem = GetInvItem(offset);
    if (pItem != nullptr && pTI != nullptr)
    {
        pTI->hwnd = GetParent()->GetSafeHwnd();
        pTI->uId = (UINT_PTR)GetSafeHwnd();
        pTI->lpszText = LPSTR_TEXTCALLBACK;
        pTI->uFlags |= TTF_IDISHWND;
    }

    return pItem;
}
//---------------------------------------------------------------------------
INT_PTR CD2ItemsGridStatic::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);

    TOOLINFO* pTi = (pTI == nullptr) ? &ti : pTI;
    if (InvHitTest(point, pTi) == nullptr)
    {
        return (INT_PTR)-1;
    }

    UINT_PTR nHit = pTi->uId;
    if (pTi->uFlags & TTF_IDISHWND)
    {
        nHit = (UINT)::GetDlgCtrlID(HWND(pTi->uId));
    }

    return (INT_PTR)nHit;
}
//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2ItemsGridStatic, CStatic)
    //{{AFX_MSG_MAP(CD2ItemsGridStatic)
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
void CD2ItemsGridStatic::PreSubclassWindow()
{
    __super::PreSubclassWindow();
    ModifyStyle(0, SS_OWNERDRAW);

    GetWindowRect(&GridRect);
    GetParent()->ScreenToClient(&GridRect);
    LoadBackgroundImage();
}
//---------------------------------------------------------------------------
void CD2ItemsGridStatic::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}
//---------------------------------------------------------------------------
BOOL CD2ItemsGridStatic::LoadBackgroundImage()
{
    if (InvGridImage.GetSafeHandle() != 0)
    {
        InvGridImage.DeleteObject();
    }

    SlotSize.cx = 0;
    SlotSize.cy = 0;
    auto id = GetDlgCtrlID();
    switch (id)
    {
    case IDC_INV_GRID:
        // Inventory is a 10 x 4 grid
        SlotSize.cx = GridRect.Width() / 10;
        SlotSize.cy = GridRect.Height() / 4;
        break;

    case IDC_INV_BELT_GRID:
        // Belts is at most a 4 x 4 grid
        SlotSize.cx = GridRect.Width() / 4;
        SlotSize.cy = GridRect.Height() / 4;
        break;

    case IDC_INV_STASH_GRID:
        // STASH is at most a 10 x 10 grid
        SlotSize.cx = GridRect.Width() / 10;
        SlotSize.cy = GridRect.Height() / 10;
        break;

    case IDC_INV_CUBE_GRID:
        // HORADRIC CUBE is a 3 x 4 grid
        SlotSize.cx = GridRect.Width() / 3;
        SlotSize.cy = GridRect.Height() / 4;
        break;

    default:
        return FALSE;
    }

    GridBoxSize = GetInvGridSize();

    CBitmap slotBitmap;
    slotBitmap.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INVEMPTY)));

    auto pDC = GetParent()->GetDC();
    ScaleImage(pDC, slotBitmap, CRect(0, 0, SlotSize.cx, SlotSize.cy));

    CRect rect;
    rect.top = 0;
    rect.left = 0;
    rect.right = (LONG)(SlotSize.cx * GridBoxSize.cx);
    rect.bottom = (LONG)(SlotSize.cy * GridBoxSize.cy);

    CRect invBox;
    invBox.left = GridRect.left;
    invBox.right = invBox.left + rect.Width();
    if (id == IDC_INV_BELT_GRID) // Belt
    {
        invBox.bottom = GridRect.bottom;
        invBox.top = invBox.bottom - rect.Height();
    }
    else
    {
        invBox.top = GridRect.top;
        invBox.bottom = invBox.top + rect.Height();
    }

    if (invBox != GridRect)
    {
        // reposition and resize
        ::SetWindowPos(GetSafeHwnd(), 0, invBox.left, invBox.top, invBox.Width(), invBox.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
    }

    // select the source CBitmap in a memory DC;
    CDC memSrcDc;
    memSrcDc.CreateCompatibleDC(pDC);
    memSrcDc.SelectObject(&slotBitmap); //now bitmap is an instance of CBitmap class

    // Create your new CBitmap with the new desired size and draw the grid
    CDC memDestDC;
    CBitmap image;
    memDestDC.CreateCompatibleDC(pDC);
    image.CreateCompatibleBitmap(&memSrcDc, rect.Width(), rect.Height());
    memDestDC.SelectObject(&image);

    for (auto x = 0; x < GridBoxSize.cx; ++x)
    {
        for (auto y = 0; y < GridBoxSize.cy; ++y)
        {
            memDestDC.BitBlt(x * SlotSize.cx, y * SlotSize.cy, SlotSize.cx, SlotSize.cy, &memSrcDc, 0, 0, SRCCOPY);
        }
    }

    slotBitmap.DeleteObject();
    InvGridImage.Attach(image.Detach());

    return (InvGridImage.GetSafeHandle() != 0) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
BOOL CD2ItemsGridStatic::LoadItemImages()
{
    InvGridItemIndex.clear();
    if (InvGridImage.GetSafeHandle() == 0)
    {
        if (!LoadBackgroundImage())
        {
            return FALSE;
        }
    }

    InvGridItemIndex.resize(size_t(GridBoxSize.cx) * size_t(GridBoxSize.cy));

    auto pDC = GetParent()->GetDC();

    // select the source CBitmap in a memory DC;
    {
        BITMAP bmp;
        InvGridImage.GetBitmap(&bmp);

        CDC memSrcDc;
        memSrcDc.CreateCompatibleDC(pDC);
        memSrcDc.SelectObject(&InvGridImage); //now bitmap is an instance of CBitmap class

        // Draw empty grid
        CDC memDestDC;
        CBitmap image;
        memDestDC.CreateCompatibleDC(pDC);
        image.CreateCompatibleBitmap(&memSrcDc, bmp.bmWidth, bmp.bmHeight);
        memDestDC.SelectObject(&image);
        memDestDC.BitBlt(0, 0, bmp.bmWidth, bmp.bmHeight, &memSrcDc, 0, 0, SRCCOPY);
        InvImage.DeleteObject();
        InvImage.Attach(image.Detach());
    }

    // Add items
    CRect rect;
    for (const auto& item : GetInvGridItems())
    {
        CBitmap bitmap;
        if (!GetItemBitmap(item.get(), bitmap))
        {
            continue;
        }

        rect = GetInvRect(item.get());
        if (rect.IsRectEmpty())
        {
            continue;
        }

        ScaleImage(pDC, bitmap, rect);
        MergeImage(pDC, InvImage, bitmap, rect);

        // Convert to a grid position rect
        rect.top /= SlotSize.cy;
        rect.bottom /= SlotSize.cy;
        rect.left /= SlotSize.cx;
        rect.right /= SlotSize.cx;

        auto pItem = &item.get();
        size_t row = 0;
        for (size_t y = rect.top; y < rect.bottom; ++y)
        {
            row = y * (size_t)GridBoxSize.cx;
            for (size_t x = rect.left; x < rect.right; ++x)
            {
                InvGridItemIndex[row + x] = pItem;
            }
        }
    }

    return TRUE;
}
//---------------------------------------------------------------------------
CSize CD2ItemsGridStatic::GetInvGridSize() const
{
    CD2ItemsGridCallback* pCallback = dynamic_cast<CD2ItemsGridCallback*>(DYNAMIC_DOWNCAST(CD2ItemsForm, GetParent()));
    if (pCallback == nullptr)
    {
        return CSize(0, 0);
    }

    return pCallback->getInvGridSize(GetDlgCtrlID());
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2ItemsGridStatic::GetInvGridItems() const
{
    CD2ItemsGridCallback* pCallback = dynamic_cast<CD2ItemsGridCallback*>(DYNAMIC_DOWNCAST(CD2ItemsForm, GetParent()));
    if (pCallback == nullptr)
    {
        static std::vector<std::reference_wrapper<d2ce::Item>> s_empty;
        return s_empty;
    }

    return pCallback->getInvGridItems(GetDlgCtrlID());
}
//---------------------------------------------------------------------------
bool CD2ItemsGridStatic::GetItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    CD2ItemsGridCallback* pCallback = dynamic_cast<CD2ItemsGridCallback*>(DYNAMIC_DOWNCAST(CD2ItemsForm, GetParent()));
    if (pCallback == nullptr)
    {
        return false;
    }

    return pCallback->getItemBitmap(item, bitmap);
}
//---------------------------------------------------------------------------
CRect CD2ItemsGridStatic::GetInvRect(const d2ce::Item& item) const
{
    CRect rect;
    d2ce::ItemDimensions dimension;
    if (!item.getDimensions(dimension))
    {
        return rect;
    }

    auto cx = item.getPositionX();
    auto cy = item.getPositionY();
    CSize itemPos = CSize(cx, cy);
    if (GetDlgCtrlID() == IDC_INV_BELT_GRID) // Belt
    {
        itemPos = CSize(cx % 4, cx / 4);
    }

    if (((itemPos.cx + dimension.Width) > GridBoxSize.cx) || ((itemPos.cy + dimension.Height) > GridBoxSize.cy))
    {
        // out of bounds
        return rect;
    }

    rect.top = itemPos.cy * SlotSize.cy;
    rect.bottom = rect.top + dimension.Height * SlotSize.cy;
    rect.left = itemPos.cx * SlotSize.cx;
    rect.right = rect.left + dimension.Width * SlotSize.cx;
    return rect;
}
//---------------------------------------------------------------------------
BOOL CD2ItemsGridStatic::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);

    // If no bitmap selected, simply erase the background as per normal and return
    if (!InvImage.GetSafeHandle())
    {
        CBrush backBrush(::GetSysColor(COLOR_3DFACE)); // (this is meant for dialogs)
        CBrush* pOldBrush = pDC->SelectObject(&backBrush);

        pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
        pDC->SelectObject(pOldBrush);

        return TRUE;
    }

    // We have a bitmap - draw it.
    BITMAP bmp;
    InvImage.GetBitmap(&bmp);

    // Create compatible memory DC using the controls DC
    CDC dcMem;
    VERIFY(dcMem.CreateCompatibleDC(pDC));

    // Select bitmap into memory DC.
    CBitmap* pBmpOld = dcMem.SelectObject(&InvImage);

    // BitBlt bitmap onto static's client area
    pDC->BitBlt(rect.left, rect.top, bmp.bmWidth, bmp.bmHeight, &dcMem, 0, 0, SRCCOPY);
    dcMem.SelectObject(pBmpOld);
    return TRUE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// CD2ItemsForm dialog

IMPLEMENT_DYNAMIC(CD2ItemsForm, CDialogEx)

//---------------------------------------------------------------------------
CD2ItemsForm::CD2ItemsForm(CD2MainForm& form)
    : CDialogEx(CD2ItemsForm::IDD, (CWnd*)&form),
    MainForm(form), Merc(form.getMercenaryInfo()), IsWeaponII(form.getWeaponSet() != 0)
{

}
//---------------------------------------------------------------------------
CD2ItemsForm::~CD2ItemsForm()
{
}
//---------------------------------------------------------------------------
void CD2ItemsForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_INV_HEAD, InvHeadBox);
    DDX_Control(pDX, IDC_INV_NECK, InvNeckBox);
    DDX_Control(pDX, IDC_INV_HAND_RIGHT, InvHandRightBox);
    DDX_Control(pDX, IDC_INV_TORSO, InvTorsoBox);
    DDX_Control(pDX, IDC_INV_HAND_LEFT, InvHandLeftBox);
    DDX_Control(pDX, IDC_INV_GLOVE, InvGloveBox);
    DDX_Control(pDX, IDC_INV_RING_RIGHT, InvRingRightBox);
    DDX_Control(pDX, IDC_INV_BELT, InvBeltBox);
    DDX_Control(pDX, IDC_INV_RING_LEFT, InvRingLeftBox);
    DDX_Control(pDX, IDC_INV_BOOTS, InvBootsBox);

    DDX_Control(pDX, IDC_CORPSE_GROUP, CorpseGroupBox);
    DDX_Control(pDX, IDC_INV_CORPSE_HEAD, InvCorpseHeadBox);
    DDX_Control(pDX, IDC_INV_CORPSE_NECK, InvCorpseNeckBox);
    DDX_Control(pDX, IDC_INV_CORPSE_HAND_RIGHT, InvCorpseHandRightBox);
    DDX_Control(pDX, IDC_INV_CORPSE_TORSO, InvCorpseTorsoBox);
    DDX_Control(pDX, IDC_INV_CORPSE_HAND_LEFT, InvCorpseHandLeftBox);
    DDX_Control(pDX, IDC_INV_CORPSE_GLOVE, InvCorpseGloveBox);
    DDX_Control(pDX, IDC_INV_CORPSE_RING_RIGHT, InvCorpseRingRightBox);
    DDX_Control(pDX, IDC_INV_CORPSE_BELT, InvCorpseBeltBox);
    DDX_Control(pDX, IDC_INV_CORPSE_RING_LEFT, InvCorpseRingLeftBox);
    DDX_Control(pDX, IDC_INV_CORPSE_BOOTS, InvCorpseBootsBox);

    DDX_Control(pDX, IDC_MERC_GROUP, MercGroupBox);
    DDX_Control(pDX, IDC_INV_MERC_HEAD, InvMercHeadBox);
    DDX_Control(pDX, IDC_INV_MERC_HAND_RIGHT, InvMercHandRightBox);
    DDX_Control(pDX, IDC_INV_MERC_TORSO, InvMercTorsoBox);
    DDX_Control(pDX, IDC_INV_MERC_HAND_LEFT, InvMercHandLeftBox);

    DDX_Control(pDX, IDC_GOLEM_GROUP, GolemGroupBox);
    DDX_Control(pDX, IDC_INV_GOLEM, InvGolemBox);

    DDX_Radio(pDX, IDC_INV_WEAPON_I, IsWeaponII);
    DDX_Control(pDX, IDC_INV_WEAPON_I, InvWeaponIButton);
    DDX_Control(pDX, IDC_INV_WEAPON_II, InvWeaponIIButton);

    DDX_Control(pDX, IDC_INV_GRID, InvGrid);
    DDX_Control(pDX, IDC_INV_BELT_GROUP, BeltGroupBox);
    DDX_Control(pDX, IDC_INV_BELT_GRID, InvBeltGrid);
    DDX_Control(pDX, IDC_INV_STASH_GRID, InvStashGrid);
    DDX_Control(pDX, IDC_INV_CUBE_GROUP, CubeGroupBox);
    DDX_Control(pDX, IDC_INV_CUBE_GRID, InvCubeGrid);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2ItemsForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2ItemsForm::OnBnClickedOk)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_ITEM_CONTEXT_FIX, &CD2ItemsForm::OnItemContextFix)
    ON_COMMAND(ID_ITEM_CONTEXT_LOAD, &CD2ItemsForm::OnItemContextLoad)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXDURABILITY, &CD2ItemsForm::OnItemContextMaxdurability)
    ON_BN_CLICKED(IDC_INV_WEAPON_I, &CD2ItemsForm::OnClickedInvWeaponRadio)
    ON_BN_CLICKED(IDC_INV_WEAPON_II, &CD2ItemsForm::OnClickedInvWeaponRadio)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// D2ItemsForm message handlers
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadEquippedItemImages()
{
    InvHandRightBox.SetUseAltImage(IsWeaponII);
    InvHandLeftBox.SetUseAltImage(IsWeaponII);

    CSize invSlotSize;
    for (const auto& item : MainForm.getEquippedItems())
    {
        CBitmap bitmap;
        switch (item.get().getEquippedId())
        {
        case d2ce::EnumEquippedId::HEAD:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvHeadBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::NECK:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvNeckBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::HAND_RIGHT:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvHandRightBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::ALT_HAND_RIGHT:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvHandRightBox.LoadItemImage(item.get(), bitmap, TRUE);
            break;

        case d2ce::EnumEquippedId::TORSO:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvTorsoBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::HAND_LEFT:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvHandLeftBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::ALT_HAND_LEFT:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvHandLeftBox.LoadItemImage(item.get(), bitmap, TRUE);
            break;

        case d2ce::EnumEquippedId::HANDS:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvGloveBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::RIGHT_FINGER:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvRingRightBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::WAIST:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvBeltBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::LEFT_FINGER:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvRingLeftBox.LoadItemImage(item.get(), bitmap);
            break;

        case d2ce::EnumEquippedId::FEET:
            if (!MainForm.getItemBitmap(item.get(), bitmap))
            {
                continue;
            }

            InvBootsBox.LoadItemImage(item.get(), bitmap);
            break;
        }
    }
}
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadCorpseItemImages()
{
    InvCorpseHandRightBox.SetUseAltImage(IsWeaponII);
    InvCorpseHandLeftBox.SetUseAltImage(IsWeaponII);

    const auto& corpseItems = MainForm.getCorpseItems();
    if (corpseItems.empty())
    {
        CorpseGroupBox.SetWindowText(_T("No Corpse"));
        InvCorpseHeadBox.ShowWindow(SW_HIDE);
        InvCorpseNeckBox.ShowWindow(SW_HIDE);
        InvCorpseHandRightBox.ShowWindow(SW_HIDE);
        InvCorpseTorsoBox.ShowWindow(SW_HIDE);
        InvCorpseHandLeftBox.ShowWindow(SW_HIDE);
        InvCorpseGloveBox.ShowWindow(SW_HIDE);
        InvCorpseRingRightBox.ShowWindow(SW_HIDE);
        InvCorpseBeltBox.ShowWindow(SW_HIDE);
        InvCorpseRingLeftBox.ShowWindow(SW_HIDE);
        InvCorpseBootsBox.ShowWindow(SW_HIDE);
        return;
    }

    CorpseGroupBox.SetWindowText(_T("Corpse"));
    InvCorpseHeadBox.ShowWindow(SW_SHOW);
    InvCorpseNeckBox.ShowWindow(SW_SHOW);
    InvCorpseHandRightBox.ShowWindow(SW_SHOW);
    InvCorpseTorsoBox.ShowWindow(SW_SHOW);
    InvCorpseHandLeftBox.ShowWindow(SW_SHOW);
    InvCorpseGloveBox.ShowWindow(SW_SHOW);
    InvCorpseRingRightBox.ShowWindow(SW_SHOW);
    InvCorpseBeltBox.ShowWindow(SW_SHOW);
    InvCorpseRingLeftBox.ShowWindow(SW_SHOW);
    InvCorpseBootsBox.ShowWindow(SW_SHOW);

    for (const auto& item : corpseItems)
    {
        CBitmap bitmap;
        switch (item.getEquippedId())
        {
        case d2ce::EnumEquippedId::HEAD:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseHeadBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::NECK:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseNeckBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::HAND_RIGHT:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseHandRightBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::ALT_HAND_RIGHT:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseHandRightBox.LoadItemImage(item, bitmap, TRUE);
            break;

        case d2ce::EnumEquippedId::TORSO:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseTorsoBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::HAND_LEFT:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseHandLeftBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::ALT_HAND_LEFT:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseHandLeftBox.LoadItemImage(item, bitmap, TRUE);
            break;

        case d2ce::EnumEquippedId::HANDS:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseGloveBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::RIGHT_FINGER:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseRingRightBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::WAIST:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseBeltBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::LEFT_FINGER:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseRingLeftBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::FEET:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvCorpseBootsBox.LoadItemImage(item, bitmap);
            break;
        }
    }
}
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadMercItemImages()
{
    if (!MainForm.isExpansionCharacter() || !Merc.isHired())
    {
        MercGroupBox.SetWindowText(_T("Mercenary Not Hired"));
        InvMercHeadBox.ShowWindow(SW_HIDE);
        InvMercHandRightBox.ShowWindow(SW_HIDE);
        InvMercTorsoBox.ShowWindow(SW_HIDE);
        InvMercHandLeftBox.ShowWindow(SW_HIDE);
        return;
    }

    InvMercHeadBox.ShowWindow(SW_SHOW);
    InvMercHandRightBox.ShowWindow(SW_SHOW);
    InvMercTorsoBox.ShowWindow(SW_SHOW);
    InvMercHandLeftBox.ShowWindow(SW_SHOW);

    auto sMercClass = d2ce::MercClassNames[static_cast<std::underlying_type_t<d2ce::EnumMercenaryClass>>(Merc.getClass())];
    CString groupStr(sMercClass.c_str());
    if (groupStr.IsEmpty())
    {
        groupStr = _T("Mercenary Not Hired");
    }
    MercGroupBox.SetWindowText(groupStr);

    for (const auto& item : Merc.getItems())
    {
        CBitmap bitmap;
        switch (item.getEquippedId())
        {
        case d2ce::EnumEquippedId::HEAD:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvMercHeadBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::HAND_RIGHT:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvMercHandRightBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::TORSO:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvMercTorsoBox.LoadItemImage(item, bitmap);
            break;

        case d2ce::EnumEquippedId::HAND_LEFT:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvMercHandLeftBox.LoadItemImage(item, bitmap);
            break;
        }
    }
}
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadGolemItemImages()
{
    if (!MainForm.isExpansionCharacter() || (MainForm.getCharacterClass() != d2ce::EnumCharClass::Necromancer))
    {
        GolemGroupBox.SetWindowText(_T("No Golem"));
        InvGolemBox.ShowWindow(SW_HIDE);
        return;
    }
    InvGolemBox.ShowWindow(SW_SHOW);
    GolemGroupBox.SetWindowText(_T("Iron Golem"));

    if (!MainForm.hasGolem())
    {
        return;
    }

    CBitmap bitmap;
    auto& item = MainForm.getGolemItem();
    if (!MainForm.getItemBitmap(item, bitmap))
    {
        return;
    }

    InvGolemBox.LoadItemImage(item, bitmap);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadGridItemImages()
{
    InvGrid.LoadItemImages();
    InvBeltGrid.LoadItemImages();
    InvStashGrid.LoadItemImages();

    if (getHasHoradricCube())
    {
        CubeGroupBox.SetWindowText(_T("Cube"));
        InvCubeGrid.LoadItemImages();
        InvCubeGrid.ShowWindow(SW_SHOW);
    }
    else
    {
        CubeGroupBox.SetWindowText(_T("No Cube"));
        InvCubeGrid.ShowWindow(SW_HIDE);
    }
}
//---------------------------------------------------------------------------
void CD2ItemsForm::CheckToolTipCtrl()
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
d2ce::EnumCharVersion CD2ItemsForm::getCharacterVersion() const
{
    return MainForm.getCharacterVersion();
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::getHasBeltEquipped() const
{
    return MainForm.getHasBeltEquipped();
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::getMaxNumberOfItemsInBelt() const
{
    return MainForm.getMaxNumberOfItemsInBelt();
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::getNumberOfItemsInBelt() const
{
    return MainForm.getNumberOfItemsInBelt();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2ItemsForm::getItemsInBelt() const
{
    return MainForm.getItemsInBelt();
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::getNumberOfItemsInInventory() const
{
    return MainForm.getNumberOfItemsInInventory();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2ItemsForm::getItemsInInventory() const
{
    return MainForm.getItemsInInventory();
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::getNumberOfItemsInStash() const
{
    return MainForm.getNumberOfItemsInStash();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2ItemsForm::getItemsInStash() const
{
    return MainForm.getItemsInStash();
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::getHasHoradricCube() const
{
    return MainForm.getHasHoradricCube();
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::getNumberOfItemsInHoradricCube() const
{
    return MainForm.getNumberOfItemsInHoradricCube();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2ItemsForm::getItemsInHoradricCube() const
{
    return MainForm.getItemsInHoradricCube();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsForm::GetInvItem(UINT id, UINT offset) const
{
    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_HEAD:
        return InvHeadBox.GetInvItem();

    case IDC_INV_CORPSE_HEAD:
        return InvCorpseHeadBox.GetInvItem();

    case IDC_INV_MERC_HEAD:
        return InvMercHeadBox.GetInvItem();

    case IDC_INV_NECK:
        return InvNeckBox.GetInvItem();

    case IDC_INV_CORPSE_NECK:
        return InvCorpseNeckBox.GetInvItem();

    case IDC_INV_HAND_RIGHT:
        return InvHandRightBox.GetInvItem();

    case IDC_INV_CORPSE_HAND_RIGHT:
        return InvCorpseHandRightBox.GetInvItem();

    case IDC_INV_MERC_HAND_RIGHT:
        return InvMercHandRightBox.GetInvItem();

    case IDC_INV_TORSO:
        return InvTorsoBox.GetInvItem();

    case IDC_INV_CORPSE_TORSO:
        return InvCorpseTorsoBox.GetInvItem();

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.GetInvItem();

    case IDC_INV_HAND_LEFT:
        return InvHandLeftBox.GetInvItem();

    case IDC_INV_CORPSE_HAND_LEFT:
        return InvCorpseHandLeftBox.GetInvItem();

    case IDC_INV_MERC_HAND_LEFT:
        return InvMercHandLeftBox.GetInvItem();

    case IDC_INV_GLOVE:
        return InvGloveBox.GetInvItem();

    case IDC_INV_CORPSE_GLOVE:
        return InvCorpseGloveBox.GetInvItem();

    case IDC_INV_RING_RIGHT:
        return InvRingRightBox.GetInvItem();

    case IDC_INV_CORPSE_RING_RIGHT:
        return InvCorpseRingRightBox.GetInvItem();

    case IDC_INV_BELT:
        return InvBeltBox.GetInvItem();

    case IDC_INV_CORPSE_BELT:
        return InvCorpseBeltBox.GetInvItem();

    case IDC_INV_RING_LEFT:
        return InvRingLeftBox.GetInvItem();

    case IDC_INV_CORPSE_RING_LEFT:
        return InvCorpseRingLeftBox.GetInvItem();

    case IDC_INV_BOOTS:
        return InvBootsBox.GetInvItem();

    case IDC_INV_CORPSE_BOOTS:
        return InvCorpseBootsBox.GetInvItem();

    case IDC_INV_GOLEM:
        return InvGolemBox.GetInvItem();

    case IDC_INV_GRID:
        return InvGrid.GetInvItem(offset);

    case IDC_INV_STASH_GRID:
        return InvStashGrid.GetInvItem(offset);

    case IDC_INV_CUBE_GRID:
        return InvCubeGrid.GetInvItem(offset);

    case IDC_INV_BELT_GRID:
        return InvBeltGrid.GetInvItem(offset);
    }

    return nullptr;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsForm::InvHitTest(UINT id, CPoint point, TOOLINFO* pTI) const
{
    ScreenToClient(&point);

    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_HEAD:
        return InvHeadBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_HEAD:
        return InvCorpseHeadBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_HEAD:
        return InvMercHeadBox.InvHitTest(point, pTI);

    case IDC_INV_NECK:
        return InvNeckBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_NECK:
        return InvCorpseNeckBox.InvHitTest(point, pTI);

    case IDC_INV_HAND_RIGHT:
        return InvHandRightBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_HAND_RIGHT:
        return InvCorpseHandRightBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_HAND_RIGHT:
        return InvMercHandRightBox.InvHitTest(point, pTI);

    case IDC_INV_TORSO:
        return InvTorsoBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_TORSO:
        return InvCorpseTorsoBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.InvHitTest(point, pTI);

    case IDC_INV_HAND_LEFT:
        return InvHandLeftBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_HAND_LEFT:
        return InvCorpseHandLeftBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_HAND_LEFT:
        return InvMercHandLeftBox.InvHitTest(point, pTI);

    case IDC_INV_GLOVE:
        return InvGloveBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_GLOVE:
        return InvCorpseGloveBox.InvHitTest(point, pTI);

    case IDC_INV_RING_RIGHT:
        return InvRingRightBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_RING_RIGHT:
        return InvCorpseRingRightBox.InvHitTest(point, pTI);

    case IDC_INV_BELT:
        return InvBeltBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_BELT:
        return InvCorpseBeltBox.InvHitTest(point, pTI);

    case IDC_INV_RING_LEFT:
        return InvRingLeftBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_RING_LEFT:
        return InvCorpseRingLeftBox.InvHitTest(point, pTI);

    case IDC_INV_BOOTS:
        return InvBootsBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_BOOTS:
        return InvCorpseBootsBox.InvHitTest(point, pTI);

    case IDC_INV_GOLEM:
        return InvGolemBox.InvHitTest(point, pTI);

    case IDC_INV_GRID:
        return InvGrid.InvHitTest(point, pTI);

    case IDC_INV_STASH_GRID:
        return InvStashGrid.InvHitTest(point, pTI);

    case IDC_INV_CUBE_GRID:
        return InvCubeGrid.InvHitTest(point, pTI);

    case IDC_INV_BELT_GRID:
        return InvBeltGrid.InvHitTest(point, pTI);
    }

    return nullptr;
}
//---------------------------------------------------------------------------
CSize CD2ItemsForm::getInvGridSize(UINT id) const
{
    std::uint16_t rectBoxWidth = 0;
    std::uint16_t rectBoxHeight = 0;
    switch (id)
    {
    case IDC_INV_GRID:
        // Inventory is a 10 x 4 grid
        rectBoxWidth = 10;
        rectBoxHeight = 4;
        break;

    case IDC_INV_BELT_GRID:
        // Belts is at most a 4 x 4 grid
        rectBoxWidth = 4;
        rectBoxHeight = std::max(std::uint16_t(1), std::uint16_t(getMaxNumberOfItemsInBelt() / 4));
        break;

    case IDC_INV_STASH_GRID:
        // STASH is at most a 10 x 10 grid
        if (getCharacterVersion() < d2ce::EnumCharVersion::v115)
        {
            // STASH is a 8 x 6 grid
            rectBoxWidth = 8;
            rectBoxHeight = 6;
        }
        else
        {
            // STASH is at most a 10 x 10 grid
            rectBoxWidth = 10;
            rectBoxHeight = 10;
        }
        break;

    case IDC_INV_CUBE_GRID:
        // HORADRIC CUBE is a 3 x 4 grid
        rectBoxWidth = 3;
        rectBoxHeight = 4;
        break;
    }

    return CSize(rectBoxWidth, rectBoxHeight);
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2ItemsForm::getInvGridItems(UINT id) const
{
    static std::vector<std::reference_wrapper<d2ce::Item>> s_empty;
    switch (id)
    {
    case IDC_INV_GRID:
        return getItemsInInventory();

    case IDC_INV_BELT_GRID:
        return getItemsInBelt();

    case IDC_INV_STASH_GRID:
        return getItemsInStash();

    case IDC_INV_CUBE_GRID:
        return getItemsInHoradricCube();
    }

    return s_empty;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    return MainForm.getItemBitmap(item, bitmap);
}
//---------------------------------------------------------------------------
BOOL CD2ItemsForm::OnInitDialog()
{
    __super::OnInitDialog();

    EnableToolTips(TRUE);
    CheckToolTipCtrl();

    if (MainForm.getCharacterVersion() < d2ce::EnumCharVersion::v109)
    {
        InvWeaponIIButton.EnableWindow(FALSE);
        InvWeaponIButton.EnableWindow(FALSE);
        InvWeaponIIButton.ShowWindow(SW_HIDE);
        InvWeaponIButton.ShowWindow(SW_HIDE);
        IsWeaponII = FALSE;
    }

    LoadEquippedItemImages();
    LoadCorpseItemImages();
    LoadMercItemImages();
    LoadGolemItemImages();
    LoadGridItemImages();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
BOOL CD2ItemsForm::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
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
        case IDC_INV_HEAD:
        case IDC_INV_CORPSE_HEAD:
        case IDC_INV_MERC_HEAD:
        case IDC_INV_NECK:
        case IDC_INV_CORPSE_NECK:
        case IDC_INV_HAND_RIGHT:
        case IDC_INV_CORPSE_HAND_RIGHT:
        case IDC_INV_MERC_HAND_RIGHT:
        case IDC_INV_TORSO:
        case IDC_INV_CORPSE_TORSO:
        case IDC_INV_MERC_TORSO:
        case IDC_INV_HAND_LEFT:
        case IDC_INV_CORPSE_HAND_LEFT:
        case IDC_INV_MERC_HAND_LEFT:
        case IDC_INV_GLOVE:
        case IDC_INV_CORPSE_GLOVE:
        case IDC_INV_RING_RIGHT:
        case IDC_INV_CORPSE_RING_RIGHT:
        case IDC_INV_BELT:
        case IDC_INV_CORPSE_BELT:
        case IDC_INV_RING_LEFT:
        case IDC_INV_CORPSE_RING_LEFT:
        case IDC_INV_BOOTS:
        case IDC_INV_CORPSE_BOOTS:
        case IDC_INV_GRID:
        case IDC_INV_STASH_GRID:
        case IDC_INV_CUBE_GRID:
        case IDC_INV_BELT_GRID:
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
    * pResult = 0;

    ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);

    return TRUE;
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnBnClickedOk()
{
    UpdateData(TRUE); // save results
    __super::OnOK();
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsForm::InvHitTest(CPoint point, TOOLINFO* pTI) const
{
    INT_PTR nHit = __super::OnToolHitTest(point, pTI);
    if (nHit == -1)
    {
        return nullptr;
    }

    switch (nHit)
    {
    case IDC_CORPSE_GROUP:
    case IDC_MERC_GROUP:
    case IDC_GOLEM_GROUP:
    case IDC_INV_CUBE_GROUP:
    case IDC_INV_BELT_GROUP:
        return nullptr;
    }

    // Make sure we have hit an item
    ClientToScreen(&point);
    return InvHitTest((UINT)nHit, point, pTI);
}
//---------------------------------------------------------------------------
INT_PTR CD2ItemsForm::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);

    TOOLINFO* pTi = (pTI == nullptr) ? &ti : pTI;
    if (InvHitTest(point, pTi) == nullptr)
    {
        return (INT_PTR)-1;
    }

    UINT_PTR nHit = pTi->uId;
    if (pTi->uFlags & TTF_IDISHWND)
    {
        nHit = (UINT)::GetDlgCtrlID(HWND(pTi->uId));
    }

    return (INT_PTR)nHit;
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CPoint hitTestPoint = point;
    ScreenToClient(&hitTestPoint);
    CurrItem = const_cast<d2ce::Item*>(InvHitTest(hitTestPoint));
    if (CurrItem == nullptr)
    {
        return;
    }

    bool isArmor = CurrItem->isArmor();
    bool isWeapon = CurrItem->isWeapon();
    bool isStackable = CurrItem->isStackable();

    if (isArmor || isWeapon || isStackable)
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_ITEM_MENU));

        CMenu* pPopup = menu.GetSubMenu(0);
        ENSURE(pPopup != NULL);

        if (!isStackable)
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_LOAD, MF_BYCOMMAND);
        }

        if (!isArmor && !isWeapon)
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_FIX, MF_BYCOMMAND);
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_MAXDURABILITY, MF_BYCOMMAND);
        }

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextFix()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.repairItem(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextLoad()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemMaxQuantity(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextMaxdurability()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemMaxDurability(*CurrItem);
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnClickedInvWeaponRadio()
{
    UpdateData(TRUE); // save result

    CRect rect;
    InvHandRightBox.SetUseAltImage(IsWeaponII);
    InvHandRightBox.RedrawWindow();

    InvHandLeftBox.SetUseAltImage(IsWeaponII);
    InvHandLeftBox.RedrawWindow();

    InvCorpseHandRightBox.SetUseAltImage(IsWeaponII);
    InvCorpseHandRightBox.RedrawWindow();

    InvCorpseHandLeftBox.SetUseAltImage(IsWeaponII);
    InvCorpseHandLeftBox.RedrawWindow();
}
//---------------------------------------------------------------------------

