/*
    Diablo II Character Editor
    Copyright (C) 2021-2023 Walter Couto

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
#include "D2GemsForm.h"
#include "D2AddGemsForm.h"
#include "D2MercenaryForm.h"
#include "D2SharedStashForm.h"
#include "D2RunewordForm.h"
#include "D2NewItemForm.h"
#include "D2MagicalPropsRandomizer.h"
#include "d2ce/helpers/ItemHelpers.h"
#include <deque>
#include <utf8/utf8.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CELL_SIZE_X 30
#define CELL_SIZE_Y 32

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

    CString BuildItemFilterLocationMessage(d2ce::ItemFilter& filter)
    {
        CString msg;
        switch (filter.LocationId)
        {
        case d2ce::EnumItemLocation::STORED:
            switch (filter.AltPositionId)
            {
            case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
                msg += _T(" located in the Horadric Cube");
                break;

            case d2ce::EnumAltItemLocation::INVENTORY:
                msg += _T(" located in the inventory");
                break;

            case d2ce::EnumAltItemLocation::STASH:
                msg += _T(" located in the Stash");
                break;
            }
            break;

        case d2ce::EnumItemLocation::BELT:
            msg += _T(" located on the belt");
            break;

        case d2ce::EnumItemLocation::EQUIPPED:
            if (filter.IsCorpse)
            {
                msg += _T(" located on the corpse");
            }
            else if (filter.IsMerc)
            {
                msg += _T(" located on the mercenary");
            }
            else
            {
                msg += _T(" equipped");
            }
            break;
        }

        return msg;
    }

    CString BuildNumItemsChangedMessage(size_t numChanged, d2ce::ItemFilter& filter)
    {
        CString msg;
        msg.Format(_T("%zd item(s)"), numChanged);
        msg += BuildItemFilterLocationMessage(filter);
        return msg;
    }

    CString BuildNumStatckedItemsChangedMessage(size_t numChanged, d2ce::ItemFilter& filter)
    {
        CString msg;
        msg.Format(_T("%zd stackable item(s)"), numChanged);
        msg += BuildItemFilterLocationMessage(filter);
        return msg;
    }

    CString BuildNumPotionsChangedMessage(size_t numChanged, d2ce::ItemFilter& filter)
    {
        CString msg;
        msg.Format(_T("%zd potion(s)"), numChanged);
        msg += BuildItemFilterLocationMessage(filter);
        return msg;
    }

    CString BuildNumGemsChangedMessage(size_t numChanged, d2ce::ItemFilter& filter)
    {
        CString msg;
        msg.Format(_T("%zd gem(s)"), numChanged);
        msg += BuildItemFilterLocationMessage(filter);
        return msg;
    }

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

    void AlphaMergeImage(CDC* pDC, CBitmap& image, CBitmap& overlay, CRect rect, bool center = false, std::uint8_t alpha = 64)
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

        if (alpha > 0)
        {
            // blend back the image
            BLENDFUNCTION bf;
            bf.AlphaFormat = 0;
            bf.BlendOp = AC_SRC_OVER;
            bf.SourceConstantAlpha = alpha;
            bf.BlendFlags = 0;
            AlphaBlend(memDestDC, 0, 0, bmp.bmWidth, bmp.bmHeight, memSrcDc, 0, 0, bmp.bmWidth, bmp.bmHeight, bf);
        }

        HGDIOBJ hbitmap_detach = image.Detach();
        if (hbitmap_detach)
        {
            DeleteObject(hbitmap_detach);
        }

        image.Attach(image2.Detach());
    }

    void MergeImage(CDC* pDC, CBitmap& image, CBitmap& overlay, CRect rect, bool center = false)
    {
        AlphaMergeImage(pDC, image, overlay, rect, center, 0);
    }

    HCURSOR CreateItemCursor(CBitmap& xOrMask)
    {
        // Create the "XOr" mask
        BITMAP bmp;
        xOrMask.GetBitmap(&bmp);
        CBitmap andMask;
        andMask.CreateBitmap(bmp.bmWidth, bmp.bmHeight, 1, 1, NULL);

        // Create an icon
        ICONINFO iconInfo;
        iconInfo.fIcon = TRUE;
        iconInfo.xHotspot = 0;
        iconInfo.yHotspot = 0;
        iconInfo.hbmMask = andMask;
        iconInfo.hbmColor = xOrMask;
        return ::CreateIconIndirect(&iconInfo);
    }

    bool CalcSocketRects(const d2ce::Item& item, CBitmap& itemImage, std::deque<CRect>& slotRectMap)
    {
        slotRectMap.clear();
        if (!item.isSocketed())
        {
            return false;
        }

        d2ce::ItemDimensions dimension;
        CSize invSlotSize;
        if (!item.getDimensions(dimension))
        {
            return false;
        }

        BITMAP itemBmp;
        itemImage.GetBitmap(&itemBmp);

        auto socketCount = item.getDisplayedSocketCount();
        CSize slotSize(itemBmp.bmWidth / dimension.Width, itemBmp.bmHeight / dimension.Height);
        auto numSlots = std::min(std::min(std::uint16_t(7), std::uint16_t(dimension.Width * dimension.Height)), std::uint16_t(socketCount));
        if (numSlots == 0)
        {
            return false;
        }

        if (numSlots < item.getDisplayedSocketCount() && (numSlots <= 7))
        {
            // not enough room
            if (dimension.Width < 2)
            {
                if (std::uint16_t(2 * dimension.Height) >= socketCount)
                {
                    dimension.Width = 2;
                    slotSize.cx = itemBmp.bmWidth / dimension.Width;
                    numSlots = std::min(std::min(std::uint16_t(7), std::uint16_t(dimension.Width * dimension.Height)), std::uint16_t(socketCount));
                }
                else if (dimension.Height < 3)
                {
                    if (6 >= socketCount)
                    {
                        dimension.Width = 2;
                        dimension.Height = 3;
                        slotSize.cx = itemBmp.bmWidth / dimension.Width;
                        slotSize.cx = itemBmp.bmHeight / dimension.Height;
                        numSlots = std::min(std::min(std::uint16_t(7), std::uint16_t(dimension.Width * dimension.Height)), std::uint16_t(socketCount));
                    }
                    else
                    {
                        dimension.Width = 2;
                        dimension.Height = 4;
                        slotSize.cx = itemBmp.bmWidth / dimension.Width;
                        slotSize.cx = itemBmp.bmHeight / dimension.Height;
                        numSlots = std::min(std::min(std::uint16_t(7), std::uint16_t(dimension.Width * dimension.Height)), std::uint16_t(socketCount));
                    }
                }
            }
        }

        // add rects in order of insertion
        std::vector<size_t> socketOrder;
        switch (numSlots)
        {
        case 1:
            socketOrder = { 0 };
            break;

        case 2:
            socketOrder = { 0, 1 };
            break;

        case 3:
            socketOrder = { 0, 1, 2 };
            break;

        case 4:
            socketOrder = { 0, 3, 2, 1 };
            break;

        case 5:
            socketOrder = { 2, 0, 4, 3, 1 };
            break;

        case 6:
            socketOrder = { 0, 2, 4, 1, 3, 5 };
            break;
        }

        if (numSlots <= 4)
        {
            if (dimension.Height >= numSlots)
            {
                CSize slotGridSize(dimension.Width * slotSize.cx, (dimension.Height * slotSize.cy) / numSlots);
                CSize slotGridOffset((slotGridSize.cx - slotSize.cx) / 2, (slotGridSize.cy - slotSize.cy) / 2);
                CRect slotRect;
                slotRect.left = slotGridOffset.cx;
                slotRect.right = slotRect.left + slotSize.cx;
                for (size_t i = 0; i < numSlots; ++i)
                {
                    slotRect.top = LONG(slotGridSize.cy * i + slotGridOffset.cy);
                    slotRect.bottom = slotRect.top + slotSize.cy;
                    slotRectMap.push_back(slotRect);
                }
            }
            else // 3 or 4 slots with item being 2 or 3 slots high
            {
                std::uint16_t div = std::uint16_t((numSlots + 1) / 2);
                CSize slotGridSize((dimension.Width * slotSize.cx) / 2, (dimension.Height * slotSize.cy) / div);
                CSize slotGridOffset((slotGridSize.cx - slotSize.cx) / 2, (slotGridSize.cy - slotSize.cy) / 2);
                CRect slotRect;
                size_t idx = 0;
                size_t idy = 0;

                for (auto& i : socketOrder)
                {
                    idx = i % 2;
                    idy = i / 2;
                    if (numSlots == 3 && i >= 2)
                    {
                        idx = (i + 1) % 2;
                        idy = (i + 1) / 2;
                        if (i == 2)
                        {
                            slotRect.top = LONG(slotGridSize.cy * idy + slotGridOffset.cy);
                            slotRect.bottom = slotRect.top + slotSize.cy;
                            slotRect.left = ((dimension.Width - 1) * slotSize.cx) / 2;
                            slotRect.right = slotRect.left + slotSize.cx;
                            slotRectMap.push_back(slotRect);
                            continue;
                        }
                    }

                    slotRect.top = LONG(slotGridSize.cy * idy + slotGridOffset.cy);
                    slotRect.bottom = slotRect.top + slotSize.cy;
                    slotRect.left = LONG(slotGridSize.cx * idx + slotGridOffset.cx);
                    slotRect.right = slotRect.left + slotSize.cx;
                    slotRectMap.push_back(slotRect);
                }
            }
        }
        else if (numSlots <= 6 && dimension.Height >= (numSlots + 1) / 2)
        {
            std::uint16_t div = std::uint16_t((numSlots + 1) / 2);
            CSize slotGridSize((dimension.Width * slotSize.cx) / 2, (dimension.Height * slotSize.cy) / div);
            CSize slotGridOffset((slotGridSize.cx - slotSize.cx) / 2, (slotGridSize.cy - slotSize.cy) / 2);
            CRect slotRect;
            size_t idx = 0;
            size_t idy = 0;

            for (auto& i : socketOrder)
            {
                idx = i % 2;
                idy = i / 2;
                if (numSlots == 5 && i >= 2)
                {
                    idx = (i + 1) % 2;
                    idy = (i + 1) / 2;
                    if (i == 2)
                    {
                        slotRect.top = LONG(slotGridSize.cy * idy + slotGridOffset.cy);
                        slotRect.bottom = slotRect.top + slotSize.cy;
                        slotRect.left = ((dimension.Width - 1) * slotSize.cx) / 2;
                        slotRect.right = slotRect.left + slotSize.cx;
                        slotRectMap.push_back(slotRect);
                        continue;
                    }
                }

                slotRect.top = LONG(slotGridSize.cy * idy + slotGridOffset.cy);
                slotRect.bottom = slotRect.top + slotSize.cy;
                slotRect.left = LONG(slotGridSize.cx * idx + slotGridOffset.cx);
                slotRect.right = slotRect.left + slotSize.cx;
                slotRectMap.push_back(slotRect);
            }
        }
        else
        {
            // An illegal socket count, every slot has a socket
            CRect slotRect;
            for (size_t i = 0; i < numSlots; ++i)
            {
                slotRect.top = LONG(slotSize.cy * (i / dimension.Width));
                slotRect.bottom = slotRect.top + slotSize.cy;
                slotRect.left = LONG(slotSize.cx * (i % dimension.Width));
                slotRect.right = slotRect.left + slotSize.cx;
                slotRectMap.push_back(slotRect);
            }
        }

        return slotRectMap.empty() ? false : true;
    }

    bool AddSocketsToImage(CDC* pDC, const d2ce::Item& item, CBitmap& bitmap, CD2ItemsGridCallback* pCallback)
    {
        if (pDC == nullptr || pCallback == nullptr)
        {
            return false;
        }

        std::deque<CRect> socketRects;
        CalcSocketRects(item, bitmap, socketRects);
        if (socketRects.empty())
        {
            return false;
        }

        // Fill in any use socket slots
        CRect socketRect;
        for (const auto& socketItem : item.SocketedItems)
        {
            if (socketRects.empty())
            {
                break;
            }

            socketRect = socketRects.front();
            socketRects.pop_front();

            CBitmap socketBitmap;
            if (!pCallback->getItemBitmap(socketItem, socketBitmap))
            {
                continue;
            }

            ScaleImage(pDC, socketBitmap, socketRect);
            AlphaMergeImage(pDC, bitmap, socketBitmap, socketRect);
        }

        if (!socketRects.empty())
        {
            // Fill in any empty socket slots
            CBitmap socketBitmap;
            socketBitmap.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_EMPTY_SOCKET)));
            ScaleImage(pDC, socketBitmap, socketRects.front());

            while (!socketRects.empty())
            {
                socketRect = socketRects.front();
                socketRects.pop_front();
                AlphaMergeImage(pDC, bitmap, socketBitmap, socketRect, false, 128);
            }
        }

        return true;
    }
   
    bool CalcItemRect(const d2ce::Item& item, const CStatic& invBox, CRect& rect, CSize& slotSize, UINT id)
    {
        CSize work;
        CSize slots;
        switch (id)
        {
        case IDC_INV_HEAD:
        case IDC_INV_CORPSE_HEAD:
        case IDC_INV_MERC_HEAD:
        case IDC_INV_GLOVES:
        case IDC_INV_CORPSE_GLOVES:
        case IDC_INV_MERC_GLOVES:
        case IDC_INV_FEET:
        case IDC_INV_CORPSE_FEET:
        case IDC_INV_MERC_FEET:
            slots = CSize(2, 2);
            break;

        case IDC_INV_NECK:
        case IDC_INV_CORPSE_NECK:
        case IDC_INV_MERC_NECK:
        case IDC_INV_RIGHT_RING:
        case IDC_INV_CORPSE_RIGHT_RING:
        case IDC_INV_MERC_RIGHT_RING:
        case IDC_INV_LEFT_RING:
        case IDC_INV_CORPSE_LEFT_RING:
        case IDC_INV_MERC_LEFT_RING:
            slots = CSize(1, 1);
            break;

        case IDC_INV_RIGHT_ARM:
        case IDC_INV_CORPSE_RIGHT_ARM:
        case IDC_INV_MERC_RIGHT_ARM:
        case IDC_INV_TORSO:
        case IDC_INV_CORPSE_TORSO:
        case IDC_INV_MERC_TORSO:
        case IDC_INV_LEFT_ARM:
        case IDC_INV_CORPSE_LEFT_ARM:
        case IDC_INV_MERC_LEFT_ARM:
        case IDC_INV_GOLEM:
            slots = CSize(2, 4);
            break;

        case IDC_INV_BELT:
        case IDC_INV_CORPSE_BELT:
        case IDC_INV_MERC_BELT:
            slots = CSize(2, 1);
            break;
        }

        d2ce::ItemDimensions dimension;
        if (!item.getDimensions(dimension))
        {
            return false;
        }

        invBox.GetClientRect(&rect);
        slotSize.cx = rect.Width() / slots.cx;
        slotSize.cy = rect.Height() / slots.cy;

        // keep same ratio
        work.cx = std::min(slotSize.cx, slotSize.cy);
        work.cy = (work.cx * CELL_SIZE_Y) / CELL_SIZE_X;
        if (work.cy > slotSize.cy)
        {
            // too big
            work.cy = slotSize.cy;
            work.cx = (work.cy * CELL_SIZE_X) / CELL_SIZE_Y;

        }
        slotSize.cx = work.cx;
        slotSize.cy = work.cy;

        rect.top = rect.bottom - dimension.Height * slotSize.cy;
        rect.right = rect.left + dimension.Width * slotSize.cx;
        return true;
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
bool CD2EquippedItemStatic::CanPlaceItem(const d2ce::Item& item, CPoint /*point*/)
{
    std::optional<d2ce::Mercenary*> mercInfo;
    switch (GetItemInventory())
    {
    case d2ce::EnumItemInventory::MERCENARY:
        mercInfo = GetMercInfo();
        if (mercInfo.has_value())
        {
            if (!mercInfo.value()->canEquipItem(item, GetEquippedId()))
            {
                auto pExistingItem = GetInvItem();
                if ((pExistingItem == nullptr) || !pExistingItem->canSocketItem(item, mercInfo.value()->getLevel()))
                {
                    return false;
                }
            }

            return true;
        }
        return false; // wrong method called

    case d2ce::EnumItemInventory::CORPSE: // only allowed to drag items out
    case d2ce::EnumItemInventory::GOLEM:
        return (GetInvItem() == &item) ? true : false;
    }

    auto charStats = GetDisplayedCharStats();
    if (charStats.has_value() && !item.canEquip(GetEquippedId()))
    {
        auto pExistingItem = GetInvItem();
        if ((pExistingItem == nullptr) || !pExistingItem->canSocketItem(item, charStats.value().Level))
        {
            return false;
        }

        return true;
    }

    return CanPlaceItemWith(item);
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2EquippedItemStatic::PlaceItem(d2ce::Item& item, CPoint /*point*/, CBitmap& bitmap)
{
    if (GetInvItem() == &item)
    {
        // we haven't moved!
        return nullptr;
    }

    std::optional<d2ce::Mercenary*> mercInfo;
    auto equippId = GetEquippedId();
    switch (GetItemInventory())
    {
    case d2ce::EnumItemInventory::MERCENARY:
        mercInfo = GetMercInfo();
        if (mercInfo.has_value())
        {
            if (!mercInfo.value()->canEquipItem(item, equippId))
            {
                auto pExistingItem = GetInvItem();
                if ((pExistingItem == nullptr) || !pExistingItem->canSocketItem(item, mercInfo.value()->getLevel()))
                {
                    return &item; // failed to place item
                }
            }
            else
            {
                equippId = mercInfo.value()->verifyEquippedId(item, equippId);
                if (equippId == d2ce::EnumEquippedId::NONE)
                {
                    // should not happend
                    return &item; // failed to place item
                }
            }

            const d2ce::Item* pRemovedItem = nullptr;
            if (!SetItemLocation(item, pRemovedItem))
            {
                return &item; // did not place
            }

            if (pRemovedItem != nullptr)
            {
                GetScaledItemBitmap(*pRemovedItem, bitmap);
            }

            return pRemovedItem;
        }
        return nullptr;

    case d2ce::EnumItemInventory::CORPSE: // only allowed to drag items out
    case d2ce::EnumItemInventory::GOLEM:
        return &item; // did not place
    }

    auto charClass = GetCharClass();
    if (charClass.has_value())
    {
        auto charStats = GetDisplayedCharStats();
        if (charStats.has_value())
        {
            if (!item.canEquip(equippId, charClass.value(), charStats.value()))
            {
                auto pExistingItem = GetInvItem();
                if ((pExistingItem == nullptr) || !pExistingItem->canSocketItem(item, charStats.value().Level))
                {
                    return &item; // failed to place item
                }
            }
        }
        else if(!item.canEquip(equippId, charClass.value()))
        {
            auto pExistingItem = GetInvItem();
            if ((pExistingItem == nullptr) || !pExistingItem->canSocketItem(item))
            {
                return &item; // failed to place item
            }
        }
    }
    else if (!item.canEquip(equippId))
    {
        auto pExistingItem = GetInvItem();
        if ((pExistingItem == nullptr) || !pExistingItem->canSocketItem(item))
        {
            return &item; // failed to place item
        }
    }

    const d2ce::Item* pRemovedItem = nullptr;
    if (!SetItemLocation(item, pRemovedItem))
    {
        return &item; // did not place
    }

    if (pRemovedItem != nullptr)
    {
        GetScaledItemBitmap(*pRemovedItem, bitmap);
    }

    return pRemovedItem;
}
//---------------------------------------------------------------------------
bool CD2EquippedItemStatic::GetInvBitmap(CBitmap& image, CPoint point, TOOLINFO* pTI) const
{
    auto pItem = InvHitTest(point, pTI);
    if (pItem == nullptr)
    {
        return false;
    }

    return GetScaledItemBitmap(*pItem, image);
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
        nHit = UINT_PTR(::GetDlgCtrlID(HWND(pTi->uId)));
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
    if (!LoadBackgroundImage(FALSE))
    {
        return FALSE;
    }

    switch (GetDlgCtrlID())
    {
    case IDC_INV_RIGHT_ARM:
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_LEFT_ARM:
    case IDC_INV_CORPSE_LEFT_ARM:
        break;

    default:
        return TRUE;
    }

    return LoadBackgroundImage(TRUE);
}
//---------------------------------------------------------------------------
BOOL CD2EquippedItemStatic::LoadBackgroundImage()
{
    return LoadBackgroundImage(UseAltImage);
}
//---------------------------------------------------------------------------
BOOL CD2EquippedItemStatic::LoadBackgroundImage(BOOL isAltImage)
{
    switch (GetDlgCtrlID())
    {
    case IDC_INV_RIGHT_ARM:
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_LEFT_ARM:
    case IDC_INV_CORPSE_LEFT_ARM:
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
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_HELM_GLOVES_0_1)));
        break;

    case IDC_INV_GLOVES:
    case IDC_INV_CORPSE_GLOVES:
    case IDC_INV_MERC_GLOVES:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_HELM_GLOVES_0_0)));
        break;

    case IDC_INV_FEET:
    case IDC_INV_CORPSE_FEET:
    case IDC_INV_MERC_FEET:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_FEET)));
        break;

    case IDC_INV_NECK:
    case IDC_INV_CORPSE_NECK:
    case IDC_INV_MERC_NECK:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_RING_AMULET_0_0)));
        break;

    case IDC_INV_RIGHT_RING:
    case IDC_INV_CORPSE_RIGHT_RING:
    case IDC_INV_MERC_RIGHT_RING:
    case IDC_INV_LEFT_RING:
    case IDC_INV_CORPSE_LEFT_RING:
    case IDC_INV_MERC_LEFT_RING:
        invImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_RING_AMULET_0_1)));
        break;

    case IDC_INV_RIGHT_ARM:
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_LEFT_ARM:
    case IDC_INV_CORPSE_LEFT_ARM:
    case IDC_INV_MERC_RIGHT_ARM:
    case IDC_INV_MERC_LEFT_ARM:
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
    case IDC_INV_MERC_BELT:
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
BOOL CD2EquippedItemStatic::LoadItemImage()
{
    InvItemPtr = nullptr;
    if (InvImage.GetSafeHandle() != 0)
    {
        InvImage.DeleteObject();
    }

    InvAltItemPtr = nullptr;
    if (InvAltImage.GetSafeHandle() != 0)
    {
        InvAltImage.DeleteObject();
    }

    if (!LoadBackgroundImage(FALSE))
    {
        return FALSE;
    }

    InvItemPtr = GetInvEquippedItem(FALSE);
    if (InvItemPtr != nullptr)
    {
        CBitmap bitmap;
        if (GetItemBitmap(*InvItemPtr, bitmap))
        {
            CRect rect;
            CSize slotSize;
            if (!CalcItemRect(*InvItemPtr, *this, rect, slotSize, GetDlgCtrlID()))
            {
                Invalidate();
                return FALSE;
            }

            auto pDC = GetParent()->GetDC();
            ScaleImage(pDC, bitmap, rect);
            AddSocketsToImage(pDC, *InvItemPtr, bitmap, GetCallback());

            MergeImage(pDC, InvImage, bitmap, rect, true);
        }
    }

    switch (GetDlgCtrlID())
    {
    case IDC_INV_RIGHT_ARM:
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_LEFT_ARM:
    case IDC_INV_CORPSE_LEFT_ARM:
        break;

    default:
        Invalidate();
        return TRUE;
    }

    // load alternate image
    if (!LoadBackgroundImage(TRUE))
    {
        Invalidate();
        return FALSE;
    }

    InvAltItemPtr = GetInvEquippedItem(TRUE);
    if (InvAltItemPtr != nullptr)
    {
        CBitmap bitmap;
        if (GetItemBitmap(*InvAltItemPtr, bitmap))
        {
            CRect rect;
            CSize slotSize;
            if (!CalcItemRect(*InvAltItemPtr, *this, rect, slotSize, GetDlgCtrlID()))
            {
                Invalidate();
                return FALSE;
            }

            auto pDC = GetParent()->GetDC();
            ScaleImage(pDC, bitmap, rect);
            AddSocketsToImage(pDC, *InvAltItemPtr, bitmap, GetCallback());

            MergeImage(pDC, InvAltImage, bitmap, rect, true);
        }
    }

    Invalidate();
    return TRUE;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2EquippedItemStatic::GetInvEquippedItem() const
{
    return GetInvEquippedItem(UseAltImage);
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2EquippedItemStatic::GetInvEquippedItem(BOOL isAltImage) const
{
    return GetInvEquippedItem(GetEquippedId(isAltImage));
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2EquippedItemStatic::GetInvEquippedItem(d2ce::EnumEquippedId equippedId) const
{
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return false;
    }

    return pCallback->getInvEquippedItem(equippedId, GetItemInventory());
}
//---------------------------------------------------------------------------
bool CD2EquippedItemStatic::GetItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return false;
    }

    return pCallback->getItemBitmap(item, bitmap);
}
//---------------------------------------------------------------------------
bool CD2EquippedItemStatic::GetScaledItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    if (!GetItemBitmap(item, bitmap))
    {
        return false;
    }

    CRect rect;
    CSize slotSize;
    if (!CalcItemRect(item, *this, rect, slotSize, GetDlgCtrlID()))
    {
        return false;
    }

    auto pDC = GetParent()->GetDC();
    ScaleImage(pDC, bitmap, rect);
    return true;
}
//---------------------------------------------------------------------------
bool CD2EquippedItemStatic::SetItemLocation(d2ce::Item& item, const d2ce::Item* &pRemovedItem) const
{
    pRemovedItem = nullptr;
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return false;
    }

    return pCallback->setItemLocation(item, GetEquippedId(), GetItemInventory(), pRemovedItem);
}
//---------------------------------------------------------------------------
CD2ItemsGridCallback* CD2EquippedItemStatic::GetCallback() const
{
    CD2ItemsGridCallback* pCallback = dynamic_cast<CD2ItemsGridCallback*>(DYNAMIC_DOWNCAST(CD2ItemsForm, GetParent()));
    if (pCallback == nullptr)
    {
        pCallback = dynamic_cast<CD2ItemsGridCallback*>(DYNAMIC_DOWNCAST(CD2MercenaryForm, GetParent()));
    }

    return pCallback;
}
//---------------------------------------------------------------------------
d2ce::EnumEquippedId CD2EquippedItemStatic::GetEquippedId() const
{
    return GetEquippedId(UseAltImage);
}
//---------------------------------------------------------------------------
d2ce::EnumEquippedId CD2EquippedItemStatic::GetEquippedId(BOOL isAltImage) const
{
    switch (GetDlgCtrlID())
    {
    case IDC_INV_HEAD:
    case IDC_INV_CORPSE_HEAD:
    case IDC_INV_MERC_HEAD:
        return d2ce::EnumEquippedId::HEAD;

    case IDC_INV_NECK:
    case IDC_INV_CORPSE_NECK:
    case IDC_INV_MERC_NECK:
        return d2ce::EnumEquippedId::NECK;

    case IDC_INV_RIGHT_ARM:
        return isAltImage ? d2ce::EnumEquippedId::ALT_RIGHT_ARM : d2ce::EnumEquippedId::RIGHT_ARM;
        
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_MERC_RIGHT_ARM:
        return d2ce::EnumEquippedId::RIGHT_ARM;

    case IDC_INV_TORSO:
    case IDC_INV_CORPSE_TORSO:
    case IDC_INV_MERC_TORSO:
        return d2ce::EnumEquippedId::TORSO;

    case IDC_INV_LEFT_ARM:
        return isAltImage ? d2ce::EnumEquippedId::ALT_LEFT_ARM : d2ce::EnumEquippedId::LEFT_ARM;

    case IDC_INV_CORPSE_LEFT_ARM:
    case IDC_INV_MERC_LEFT_ARM:
        return d2ce::EnumEquippedId::LEFT_ARM;

    case IDC_INV_GLOVES:
    case IDC_INV_CORPSE_GLOVES:
    case IDC_INV_MERC_GLOVES:
        return d2ce::EnumEquippedId::GLOVES;

    case IDC_INV_RIGHT_RING:
    case IDC_INV_CORPSE_RIGHT_RING:
    case IDC_INV_MERC_RIGHT_RING:
        return d2ce::EnumEquippedId::RIGHT_RING;

    case IDC_INV_BELT:
    case IDC_INV_CORPSE_BELT:
    case IDC_INV_MERC_BELT:
        return d2ce::EnumEquippedId::BELT;

    case IDC_INV_LEFT_RING:
    case IDC_INV_CORPSE_LEFT_RING:
    case IDC_INV_MERC_LEFT_RING:
        return d2ce::EnumEquippedId::LEFT_RING;

    case IDC_INV_FEET:
    case IDC_INV_CORPSE_FEET:
    case IDC_INV_MERC_FEET:
        return d2ce::EnumEquippedId::FEET;
    }

    return d2ce::EnumEquippedId::NONE;
}
//---------------------------------------------------------------------------
d2ce::EnumItemInventory CD2EquippedItemStatic::GetItemInventory() const
{
    switch (GetDlgCtrlID())
    {
    case IDC_INV_CORPSE_HEAD:
    case IDC_INV_CORPSE_NECK:
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_CORPSE_TORSO:
    case IDC_INV_CORPSE_LEFT_ARM:
    case IDC_INV_CORPSE_GLOVES:
    case IDC_INV_CORPSE_RIGHT_RING:
    case IDC_INV_CORPSE_BELT:
    case IDC_INV_CORPSE_LEFT_RING:
    case IDC_INV_CORPSE_FEET:
        return d2ce::EnumItemInventory::CORPSE;

    case IDC_INV_MERC_HEAD:
    case IDC_INV_MERC_NECK:
    case IDC_INV_MERC_RIGHT_ARM:
    case IDC_INV_MERC_TORSO:
    case IDC_INV_MERC_LEFT_ARM:
    case IDC_INV_MERC_GLOVES:
    case IDC_INV_MERC_RIGHT_RING:
    case IDC_INV_MERC_BELT:
    case IDC_INV_MERC_LEFT_RING:
    case IDC_INV_MERC_FEET:
        return d2ce::EnumItemInventory::MERCENARY;

    case IDC_INV_GOLEM:
        return d2ce::EnumItemInventory::GOLEM;

    case IDC_INV_HEAD:
    case IDC_INV_NECK:
    case IDC_INV_RIGHT_ARM:
    case IDC_INV_TORSO:
    case IDC_INV_LEFT_ARM:
    case IDC_INV_GLOVES:
    case IDC_INV_RIGHT_RING:
    case IDC_INV_BELT:
    case IDC_INV_LEFT_RING:
    case IDC_INV_FEET:
        return d2ce::EnumItemInventory::PLAYER;
    }

    return d2ce::EnumItemInventory::UNKNOWN;
}
//---------------------------------------------------------------------------
std::optional<d2ce::CharStats> CD2EquippedItemStatic::GetDisplayedCharStats() const
{
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return std::optional<d2ce::CharStats>();
    }

    return pCallback->getDisplayedCharStats();
}
//---------------------------------------------------------------------------
std::optional<d2ce::Mercenary*> CD2EquippedItemStatic::GetMercInfo() const
{
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return std::optional<d2ce::Mercenary*>();
    }

    return pCallback->getMercInfo();
}
//---------------------------------------------------------------------------
std::optional<d2ce::EnumCharClass> CD2EquippedItemStatic::GetCharClass() const
{
    CD2ItemsGridCallback* pCallback = GetCallback();
    std::optional<d2ce::Mercenary*> mercInfo;
    switch (GetItemInventory())
    {
    case d2ce::EnumItemInventory::MERCENARY:
        mercInfo = GetMercInfo();
        if (mercInfo.has_value())
        {
            return mercInfo.value()->getEquivClass();
        }
        break;

    case d2ce::EnumItemInventory::PLAYER:
        if (pCallback != nullptr)
        {
            return pCallback->getCharClass();
        }
        break;
    }

    return std::optional<d2ce::EnumCharClass>();
}
//---------------------------------------------------------------------------
bool CD2EquippedItemStatic::CanPlaceItemWith(const d2ce::Item& item) const
{
    auto equippedId = GetEquippedId();
    auto charClass = GetCharClass();
    if (charClass.has_value())
    {
        auto charStats = GetDisplayedCharStats();
        if (charStats.has_value())
        {
            if (!item.canEquip(equippedId, charClass.value(), charStats.value()))
            {
                return false;
            }
        }
        else if (!item.canEquip(equippedId, charClass.value()))
        {
            return false;
        }
    }
    else if (!item.canEquip(equippedId))
    {
        return false;
    }

    auto otherEquippedId = equippedId;
    switch (equippedId)
    {
    case d2ce::EnumEquippedId::LEFT_ARM:
        otherEquippedId = d2ce::EnumEquippedId::RIGHT_ARM;
        break;

    case d2ce::EnumEquippedId::ALT_LEFT_ARM:
        otherEquippedId = d2ce::EnumEquippedId::ALT_RIGHT_ARM;
        break;

    case d2ce::EnumEquippedId::RIGHT_ARM:
        otherEquippedId = d2ce::EnumEquippedId::LEFT_ARM;
        break;

    case d2ce::EnumEquippedId::ALT_RIGHT_ARM:
        otherEquippedId = d2ce::EnumEquippedId::ALT_LEFT_ARM;
        break;

    default:
        return true;
    }

    if (charClass.has_value())
    {
        return true;
    }

    auto pItem = GetInvEquippedItem(otherEquippedId);
    if ((pItem == nullptr) || (pItem == &item))
    {
        return true;
    }

    auto charStats = GetDisplayedCharStats();
    if (charStats.has_value())
    {
        return item.canEquipWith(*pItem, charClass.value(), charStats.value());
    }

    return item.canEquipWith(*pItem, charClass.value());
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
bool CD2ItemsGridStatic::CanPlaceItem(const d2ce::Item& item, CPoint point)
{
    d2ce::ItemDimensions dimension;
    if (!item.getDimensions(dimension))
    {
        return false;
    }

    auto id = GetDlgCtrlID();
    switch (id)
    {
    case IDC_INV_BELT_GRID:
        if (!item.isBeltable())
        {
            return false;
        }
        break;

    case IDC_INV_CUBE_GRID:
        if (item.isHoradricCube())
        {
            return false;
        }
        break;
    }

    auto pItem = InvHitTest(point);
    if (pItem != nullptr)
    {
        if (pItem == &item)
        {
            return true;
        }

        d2ce::ItemDimensions otherDimension;
        if (!pItem->getDimensions(otherDimension))
        {
            return false;
        }

        if ((otherDimension.Width >= dimension.Width) && (otherDimension.Height >= dimension.Height))
        {
            return true;
        }
    }

    CRect invBox;
    GetWindowRect(&invBox);
    GetParent()->ScreenToClient(&invBox);
    if (!PtInRect(&invBox, point))
    {
        return false;
    }

    // check boundaries
    auto xStart = point.x - invBox.left;
    if ((xStart < 0) || (xStart > invBox.Width()))
    {
        return false;
    }

    auto yStart = point.y - invBox.top;
    if ((yStart < 0) || (yStart > invBox.Height()))
    {
        return false;
    }

    // the point of the cursor is the middle of the item, so move the edge to the closest slot
    xStart -= (SlotSize.cx * (dimension.Width - 1)) / 2;
    if (xStart < 0)
    {
        return false;
    }
    point.x = xStart / SlotSize.cx;

    yStart -= (SlotSize.cy * (dimension.Height - 1)) / 2;
    if (yStart < 0)
    {
        return false;
    }

    point.y = yStart / SlotSize.cy;
    auto rect = GetItemRectAtGridPos(item, point);
    if (rect.IsRectEmpty())
    {
        return false;
    }

    // Convert to a grid position rect
    rect.top /= SlotSize.cy;
    rect.bottom /= SlotSize.cy;
    rect.left /= SlotSize.cx;
    rect.right /= SlotSize.cx;

    size_t row = 0;
    const d2ce::Item* pItem2 = nullptr;
    for (size_t y = rect.top; y < rect.bottom; ++y)
    {
        row = y * (size_t)GridBoxSize.cx;
        for (size_t x = rect.left; x < rect.right; ++x)
        {
            pItem = InvGridItemIndex[row + x];
            if (pItem != nullptr && pItem != &item)
            {
                if (pItem2 != nullptr)
                {
                    if (pItem != pItem2)
                    {
                        // allowed to swap with one item only
                        return false;
                    }
                }
                else
                {
                    pItem2 = pItem;
                }
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsGridStatic::PlaceItem(d2ce::Item& item, CPoint point, CBitmap& bitmap)
{
    auto* existingItem = const_cast<d2ce::Item*>(InvHitTest(point));
    d2ce::ItemDimensions dimension;
    if (!item.getDimensions(dimension))
    {
        return &item; // did not place
    }

    auto id = GetDlgCtrlID();
    switch (id)
    {
    case IDC_INV_BELT_GRID:
        if (!item.isBeltable())
        {
            return &item; // did not place
        }
        break;

    case IDC_INV_CUBE_GRID:
        if (item.isHoradricCube())
        {
            return &item; // did not place
        }
        break;
    }

    CRect invBox;
    GetWindowRect(&invBox);
    GetParent()->ScreenToClient(&invBox);
    if (!PtInRect(&invBox, point))
    {
        return false;
    }

    // check boundaries
    auto xStart = point.x - invBox.left;
    if ((xStart < 0) || (xStart > invBox.Width()))
    {
        return false;
    }

    auto yStart = point.y - invBox.top;
    if ((yStart < 0) || (yStart > invBox.Height()))
    {
        return false;
    }

    // the point of the cursor is the middle of the item, so move the edge to the closest slot
    xStart -= (SlotSize.cx * (dimension.Width - 1)) / 2;
    if (xStart < 0)
    {
        return false;
    }
    point.x = xStart / SlotSize.cx;

    yStart -= (SlotSize.cy * (dimension.Height - 1)) / 2;
    if (yStart < 0)
    {
        return false;
    }

    point.y = yStart / SlotSize.cy;

    d2ce::EnumItemLocation location = d2ce::EnumItemLocation::STORED;
    d2ce::EnumAltItemLocation altLocation = d2ce::EnumAltItemLocation::UNKNOWN;
    switch (id)
    {
    case IDC_INV_GRID:
        altLocation = d2ce::EnumAltItemLocation::INVENTORY;
        break;

    case IDC_INV_BELT_GRID:
        location = d2ce::EnumItemLocation::BELT;
        break;

    case IDC_INV_STASH_GRID:
        altLocation = d2ce::EnumAltItemLocation::STASH;
        break;

    case IDC_INV_CUBE_GRID:
        altLocation = d2ce::EnumAltItemLocation::HORADRIC_CUBE;
        break;

    default:
        return &item; // did not place
    }

    if (existingItem == &item)
    {
        if ((item.getPositionX() == std::uint16_t(point.x)) &&
            (item.getPositionY() == std::uint16_t(point.y)))
        {
            // nothing to do
            return nullptr;
        }
    }

    const d2ce::Item* pRemovedItem = nullptr;
    if (!SetItemLocation(item, location, altLocation, std::uint16_t(point.x), std::uint16_t(point.y), d2ce::EnumItemInventory::PLAYER, pRemovedItem))
    {
        return &item; // did not place
    }

    if (pRemovedItem != nullptr)
    {
        GetScaledItemBitmap(*pRemovedItem, bitmap);
    }

    return pRemovedItem;
}
//---------------------------------------------------------------------------
bool CD2ItemsGridStatic::GetInvBitmap(CBitmap& image, CPoint point, TOOLINFO* pTI) const
{
    auto pItem = InvHitTest(point, pTI);
    if (pItem == nullptr)
    {
        return false;
    }

    return GetScaledItemBitmap(*pItem, image);
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsGridStatic::GetInvItem(UINT offset) const
{
    if ((InvImage.GetSafeHandle() == 0) || (offset >= InvGridItemIndex.size()))
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

    CPoint hitTestPoint = point;
    hitTestPoint.x = (point.x - invBox.left) / SlotSize.cx;
    if (hitTestPoint.x < 0 || hitTestPoint.x > GridBoxSize.cx)
    {
        return nullptr;
    }

    hitTestPoint.y = (point.y - invBox.top) / SlotSize.cy;
    if (hitTestPoint.y < 0 || hitTestPoint.y > GridBoxSize.cy)
    {
        return nullptr;
    }

    UINT offset = (UINT)hitTestPoint.y * (UINT)GridBoxSize.cx + (UINT)hitTestPoint.x;
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
        nHit = UINT_PTR(::GetDlgCtrlID(HWND(pTi->uId)));
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

    GridBoxSize = GetInvGridSize();
    SlotSize.cx = 0;
    SlotSize.cy = 0;
    CSize work;
    LONG diffX = 0;
    LONG diffY = 0;
    auto id = GetDlgCtrlID();
    switch (id)
    {
    case IDC_INV_GRID:
        // Inventory 
        SlotSize.cx = std::min(CELL_SIZE_X, GridRect.Width() / (int)GridBoxSize.cx);
        SlotSize.cy = std::min(CELL_SIZE_Y, GridRect.Height() / (int)GridBoxSize.cy);

        // keep same ratio
        work.cx = std::min(SlotSize.cx, SlotSize.cy);
        work.cy = (work.cx * CELL_SIZE_Y) / CELL_SIZE_X;
        if (work.cy > SlotSize.cy)
        {
            // too big
            work.cy = SlotSize.cy;
            work.cx = (work.cy * CELL_SIZE_X) / CELL_SIZE_Y;

        }
        SlotSize.cx = work.cx;
        SlotSize.cy = work.cy;

        diffX = (GridRect.Width() - SlotSize.cx * GridBoxSize.cx) / 2;
        break;

    case IDC_INV_BELT_GRID:
        // Belts is at most a 4 x 4 grid
        SlotSize.cx = std::min(CELL_SIZE_X, GridRect.Width() / 4);
        SlotSize.cy = std::min(CELL_SIZE_Y, GridRect.Height() / 4);

        // keep same ratio
        work.cx = std::min(SlotSize.cx, SlotSize.cy);
        work.cy = (work.cx * CELL_SIZE_Y) / CELL_SIZE_X;
        if (work.cy > SlotSize.cy)
        {
            // too big
            work.cy = SlotSize.cy;
            work.cx = (work.cy * CELL_SIZE_X) / CELL_SIZE_Y;

        }
        SlotSize.cx = work.cx;
        SlotSize.cy = work.cy;
        break;

    case IDC_INV_STASH_GRID:
        // STASH
        SlotSize.cx = std::min(CELL_SIZE_X, GridRect.Width() / (int)GridBoxSize.cx);
        SlotSize.cy = std::min(CELL_SIZE_Y, GridRect.Height() / (int)GridBoxSize.cy);

        // keep same ratio
        work.cx = std::min(SlotSize.cx, SlotSize.cy);
        work.cy = (work.cx * CELL_SIZE_Y) / CELL_SIZE_X;
        if (work.cy > SlotSize.cy)
        {
            // too big
            work.cy = SlotSize.cy;
            work.cx = (work.cy * CELL_SIZE_X) / CELL_SIZE_Y;

        }
        SlotSize.cx = work.cx;
        SlotSize.cy = work.cy;

        diffX = (GridRect.Width() - SlotSize.cx * GridBoxSize.cx) / 2;
        diffY = (GridRect.Height() - SlotSize.cy * GridBoxSize.cy) / 2;
        break;

    case IDC_INV_CUBE_GRID:
        // HORADRIC CUBE
        SlotSize.cx = std::min(CELL_SIZE_X, GridRect.Width() / (int)GridBoxSize.cx);
        SlotSize.cy = std::min(CELL_SIZE_Y, GridRect.Height() / (int)GridBoxSize.cy);

        // keep same ratio
        work.cx = std::min(SlotSize.cx, SlotSize.cy);
        work.cy = (work.cx * CELL_SIZE_Y) / CELL_SIZE_X;
        if (work.cy > SlotSize.cy)
        {
            // too big
            work.cy = SlotSize.cy;
            work.cx = (work.cy * CELL_SIZE_X) / CELL_SIZE_Y;

        }
        SlotSize.cx = work.cx;
        SlotSize.cy = work.cy;

        diffX = (GridRect.Width() - SlotSize.cx * GridBoxSize.cx) / 2;
        diffY = (GridRect.Height() - SlotSize.cy * GridBoxSize.cy) / 2;
        break;

    default:
        return FALSE;
    }

    if (diffX > 0)
    {
        GridRect.left += diffX;
        GridRect.right += diffX;
    }

    if (diffY > 0)
    {
        GridRect.top += diffY;
        GridRect.bottom += diffY;
    }

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
    if (GetDlgCtrlID() == IDC_INV_BELT_GRID) // Belt
    {
        if (InvGridImage.GetSafeHandle() != 0)
        {
            InvGridImage.DeleteObject();
        }
    }

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
        if (item.get().getLocation() == d2ce::EnumItemLocation::BUFFER)
        {
            // don't draw items in the buffer
            continue;
        }

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
        AddSocketsToImage(pDC, item, bitmap, GetCallback());

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

    Invalidate();
    return TRUE;
}
//---------------------------------------------------------------------------
CSize CD2ItemsGridStatic::GetInvGridSize() const
{
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return CSize(0, 0);
    }

    return pCallback->getInvGridSize(GetLocation(), GetAltPositionId());
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2ItemsGridStatic::GetInvGridItems() const
{
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        static std::vector<std::reference_wrapper<d2ce::Item>> s_empty;
        return s_empty;
    }

    return pCallback->getInvGridItems(GetLocation(), GetAltPositionId());
}
//---------------------------------------------------------------------------
bool CD2ItemsGridStatic::GetItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return false;
    }

    return pCallback->getItemBitmap(item, bitmap);
}
//---------------------------------------------------------------------------
bool CD2ItemsGridStatic::GetScaledItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    if (!GetItemBitmap(item, bitmap))
    {
        return false;
    }

    auto rect = GetInvRect(item);
    if (rect.IsRectEmpty())
    {
        return false;
    }

    auto pDC = GetParent()->GetDC();
    ScaleImage(pDC, bitmap, rect);
    return true;
}
//---------------------------------------------------------------------------
CRect CD2ItemsGridStatic::GetInvRect(const d2ce::Item& item) const
{
    auto cx = item.getPositionX();
    auto cy = item.getPositionY();
    CPoint itemPos(cx, cy);
    if (GetDlgCtrlID() == IDC_INV_BELT_GRID) // Belt
    {
        LONG row = cx / 4;
        if (row >= GridBoxSize.cy)
        {
            // out of bounds
            return CRect();
        }

        itemPos = CPoint(cx % 4, GridBoxSize.cy - (row + 1));
    }

    return GetItemRectAtGridPos(item, itemPos);
}
//---------------------------------------------------------------------------
CRect CD2ItemsGridStatic::GetItemRectAtGridPos(const d2ce::Item& item, CPoint position) const
{
    CRect rect;
    d2ce::ItemDimensions dimension;
    if (!item.getDimensions(dimension))
    {
        return rect;
    }

    CSize itemPos = CSize(position.x, position.y);
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
bool CD2ItemsGridStatic::SetItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item* &pRemovedItem) const
{
    pRemovedItem = nullptr;
    CD2ItemsGridCallback* pCallback = GetCallback();
    if (pCallback == nullptr)
    {
        return false;
    }

    if (locationId == d2ce::EnumItemLocation::BELT)
    {
        if (positionY >= GridBoxSize.cy)
        {
            // out of bounds
            return false;
        }

        auto cy = std::uint16_t(GridBoxSize.cy - (positionY + 1));
        positionX += cy * 4;
        positionY = 0;
    }

    return pCallback->setItemLocation(item, locationId, altPositionId, positionX, positionY, invType, pRemovedItem);
}
//---------------------------------------------------------------------------
CD2ItemsGridCallback* CD2ItemsGridStatic::GetCallback() const
{
    auto pCallback = dynamic_cast<CD2ItemsGridCallback*>(DYNAMIC_DOWNCAST(CD2ItemsForm, GetParent()));
    if (pCallback == nullptr)
    {
        pCallback = dynamic_cast<CD2ItemsGridCallback*>(DYNAMIC_DOWNCAST(CD2SharedStashForm, GetParent()));
    }

    return pCallback;
}
//---------------------------------------------------------------------------
d2ce::EnumItemLocation CD2ItemsGridStatic::GetLocation() const
{
    switch (GetDlgCtrlID())
    {
    case IDC_INV_GRID:
    case IDC_INV_STASH_GRID:
    case IDC_INV_CUBE_GRID:
        return d2ce::EnumItemLocation::STORED;

    case IDC_INV_BELT_GRID:
        return d2ce::EnumItemLocation::BELT;
    }

    return d2ce::EnumItemLocation::BUFFER;
}
//---------------------------------------------------------------------------
d2ce::EnumAltItemLocation CD2ItemsGridStatic::GetAltPositionId() const
{
    switch (GetDlgCtrlID())
    {
    case IDC_INV_GRID:
        return d2ce::EnumAltItemLocation::INVENTORY;

    case IDC_INV_STASH_GRID:
        return d2ce::EnumAltItemLocation::STASH;

    case IDC_INV_CUBE_GRID:
        return d2ce::EnumAltItemLocation::HORADRIC_CUBE;
    }

    return d2ce::EnumAltItemLocation::UNKNOWN;
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
    DDX_Control(pDX, IDC_INV_RIGHT_ARM, InvHandRightBox);
    DDX_Control(pDX, IDC_INV_TORSO, InvTorsoBox);
    DDX_Control(pDX, IDC_INV_LEFT_ARM, InvHandLeftBox);
    DDX_Control(pDX, IDC_INV_GLOVES, InvGloveBox);
    DDX_Control(pDX, IDC_INV_RIGHT_RING, InvRingRightBox);
    DDX_Control(pDX, IDC_INV_BELT, InvBeltBox);
    DDX_Control(pDX, IDC_INV_LEFT_RING, InvRingLeftBox);
    DDX_Control(pDX, IDC_INV_FEET, InvBootsBox);

    DDX_Control(pDX, IDC_CORPSE_GROUP, CorpseGroupBox);
    DDX_Control(pDX, IDC_INV_CORPSE_HEAD, InvCorpseHeadBox);
    DDX_Control(pDX, IDC_INV_CORPSE_NECK, InvCorpseNeckBox);
    DDX_Control(pDX, IDC_INV_CORPSE_RIGHT_ARM, InvCorpseHandRightBox);
    DDX_Control(pDX, IDC_INV_CORPSE_TORSO, InvCorpseTorsoBox);
    DDX_Control(pDX, IDC_INV_CORPSE_LEFT_ARM, InvCorpseHandLeftBox);
    DDX_Control(pDX, IDC_INV_CORPSE_GLOVES, InvCorpseGloveBox);
    DDX_Control(pDX, IDC_INV_CORPSE_RIGHT_RING, InvCorpseRingRightBox);
    DDX_Control(pDX, IDC_INV_CORPSE_BELT, InvCorpseBeltBox);
    DDX_Control(pDX, IDC_INV_CORPSE_LEFT_RING, InvCorpseRingLeftBox);
    DDX_Control(pDX, IDC_INV_CORPSE_FEET, InvCorpseBootsBox);

    DDX_Control(pDX, IDC_MERC_GROUP, MercGroupBox);
    DDX_Control(pDX, IDC_INV_MERC_HEAD, InvMercHeadBox);
    DDX_Control(pDX, IDC_INV_MERC_NECK, InvMercNeckBox);
    DDX_Control(pDX, IDC_INV_MERC_RIGHT_ARM, InvMercHandRightBox);
    DDX_Control(pDX, IDC_INV_MERC_TORSO, InvMercTorsoBox);
    DDX_Control(pDX, IDC_INV_MERC_LEFT_ARM, InvMercHandLeftBox);
    DDX_Control(pDX, IDC_INV_MERC_GLOVES, InvMercGloveBox);
    DDX_Control(pDX, IDC_INV_MERC_RIGHT_RING, InvMercRingRightBox);
    DDX_Control(pDX, IDC_INV_MERC_BELT, InvMercBeltBox);
    DDX_Control(pDX, IDC_INV_MERC_LEFT_RING, InvMercRingLeftBox);
    DDX_Control(pDX, IDC_INV_MERC_FEET, InvMercBootsBox);

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
BOOL CD2ItemsForm::PreTranslateMessage(MSG* pMsg)
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
BEGIN_MESSAGE_MAP(CD2ItemsForm, CDialogEx)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDOK, &CD2ItemsForm::OnBnClickedOk)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
    ON_WM_CONTEXTMENU()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_COMMAND(ID_ITEM_CONTEXT_FIX, &CD2ItemsForm::OnItemContextFix)
    ON_COMMAND(ID_ITEM_CONTEXT_FIXALLITEMS, &CD2ItemsForm::OnItemContextFixallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_LOAD, &CD2ItemsForm::OnItemContextLoad)
    ON_COMMAND(ID_ITEM_CONTEXT_FIXALLITEMS, &CD2ItemsForm::OnItemContextMaxfillstackables)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXDURABILITY, &CD2ItemsForm::OnItemContextMaxdurability)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXDURABILITYFORALLITEMS, &CD2ItemsForm::OnItemContextMaxdurabilityforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_INDESTRUCTIBLE, &CD2ItemsForm::OnItemContextIndestructible)
    ON_COMMAND(ID_ITEM_CONTEXT_INDESTRUCTIBLEFORALLITEMS, &CD2ItemsForm::OnItemContextIndestructibleforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_ADDSOCKET, &CD2ItemsForm::OnItemContextAddsocket)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXSOCKETS, &CD2ItemsForm::OnItemContextMaxsockets)
    ON_COMMAND(ID_ITEM_CONTEXT_UNSOCKET, &CD2ItemsForm::OnItemContextUnsocket)
    ON_COMMAND(ID_ITEM_CONTEXT_MAKESUPERIORQUALITY, &CD2ItemsForm::OnItemContextMakesuperiorquality)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADEITEMTIER, &CD2ItemsForm::OnItemContextUpgradehighertier)
    ON_COMMAND(ID_ITEM_CONTEXT_MAKEETHEREAL, &CD2ItemsForm::OnItemContextChangeEthereal)
    ON_COMMAND(ID_ITEM_CONTEXT_REMOVEETHEREAL, &CD2ItemsForm::OnItemContextChangeEthereal)
    ON_COMMAND(ID_ITEM_CONTEXT_PERSONALIZE, &CD2ItemsForm::OnItemContextPersonalize)
    ON_COMMAND(ID_ITEM_CONTEXT_REMOVE_PERSONALIZATION, &CD2ItemsForm::OnItemContextRemovePersonalization)
    ON_COMMAND(ID_ITEM_CONTEXT_APPLY_RUNEWORD, &CD2ItemsForm::OnItemContextApplyruneword)
    ON_COMMAND(ID_ITEM_CONTEXT_CREATE_ITEM, &CD2ItemsForm::OnItemContextCreateitem)
    ON_COMMAND(ID_ITEM_CONTEXT_MODIFY_ITEM, &CD2ItemsForm::OnItemContextModifyitem)
    ON_COMMAND(ID_ITEM_CONTEXT_IMPORT_ITEM, &CD2ItemsForm::OnItemContextImportitem)
    ON_COMMAND(ID_ITEM_CONTEXT_EXPORT_ITEM, &CD2ItemsForm::OnItemContextExportitem)
    ON_COMMAND(ID_ITEM_CONTEXT_REMOVE_ITEM, &CD2ItemsForm::OnItemContextRemoveitem)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXSOCKETSFORALLITEMS, &CD2ItemsForm::OnItemContextMaxsocketsforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_SUPERIORQUALITYALLITEMS, &CD2ItemsForm::OnItemContextSuperiorforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_ALLITEMSMAXTIER, &CD2ItemsForm::OnItemContextHigherTierforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_GEM, &CD2ItemsForm::OnItemContextUpgradeGem)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_GEMS, &CD2ItemsForm::OnItemContextUpgradeGems)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_POTION, &CD2ItemsForm::OnItemContextUpgradePotion)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_POTIONS, &CD2ItemsForm::OnItemContextUpgradePotions)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_REJUVENATION, &CD2ItemsForm::OnItemContextUpgradeRejuvenation)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_REJUVENATIONS, &CD2ItemsForm::OnItemContextUpgradeRejuvenations)
    ON_COMMAND(ID_ITEM_CONTEXT_GPS_CONVERTOR, &CD2ItemsForm::OnItemContextGpsConvertor)
    ON_COMMAND(ID_ITEM_CONTEXT_GPS_CREATOR, &CD2ItemsForm::OnItemContextGpsCreator)
    ON_BN_CLICKED(IDC_INV_WEAPON_I, &CD2ItemsForm::OnClickedInvWeaponRadio)
    ON_BN_CLICKED(IDC_INV_WEAPON_II, &CD2ItemsForm::OnClickedInvWeaponRadio)
    ON_BN_CLICKED(IDC_SHARED_STASH_BUTTON, &CD2ItemsForm::OnBnClickedSharedStashButton)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// D2ItemsForm message handlers
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadEquippedItemImages()
{
    InvHandRightBox.SetUseAltImage(IsWeaponII);
    InvHandLeftBox.SetUseAltImage(IsWeaponII);
    InvHeadBox.LoadItemImage();
    InvNeckBox.LoadItemImage();
    InvHandRightBox.LoadItemImage();
    InvTorsoBox.LoadItemImage();
    InvHandLeftBox.LoadItemImage();
    InvGloveBox.LoadItemImage();
    InvRingRightBox.LoadItemImage();
    InvBeltBox.LoadItemImage();
    InvRingLeftBox.LoadItemImage();
    InvBootsBox.LoadItemImage();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadCorpseItemImages()
{
    InvCorpseHandRightBox.SetUseAltImage(IsWeaponII);
    InvCorpseHandLeftBox.SetUseAltImage(IsWeaponII);
    std::string strValue;
    std::string gender = MainForm.isFemaleCharacter() ? "[fs]" : "[ms]";
    d2ce::LocalizationHelpers::GetStringTxtValue("Corpse", strValue, gender, "Corpse");
    auto uName = utf8::utf8to16(strValue);
    CString windowText(reinterpret_cast<LPCWSTR>(uName.c_str()));
    CorpseGroupBox.SetWindowText(windowText);

    const auto& corpseItems = MainForm.getCorpseItems();
    if (corpseItems.empty())
    {
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

    InvCorpseHeadBox.ShowWindow(SW_SHOW);
    InvCorpseHeadBox.LoadItemImage();
    InvCorpseNeckBox.ShowWindow(SW_SHOW);
    InvCorpseNeckBox.LoadItemImage();
    InvCorpseHandRightBox.ShowWindow(SW_SHOW);
    InvCorpseHandRightBox.LoadItemImage();
    InvCorpseTorsoBox.ShowWindow(SW_SHOW);
    InvCorpseTorsoBox.LoadItemImage();
    InvCorpseHandLeftBox.ShowWindow(SW_SHOW);
    InvCorpseHandLeftBox.LoadItemImage();
    InvCorpseGloveBox.ShowWindow(SW_SHOW);
    InvCorpseGloveBox.LoadItemImage();
    InvCorpseRingRightBox.ShowWindow(SW_SHOW);
    InvCorpseRingRightBox.LoadItemImage();
    InvCorpseBeltBox.ShowWindow(SW_SHOW);
    InvCorpseBeltBox.LoadItemImage();
    InvCorpseRingLeftBox.ShowWindow(SW_SHOW);
    InvCorpseRingLeftBox.LoadItemImage();
    InvCorpseBootsBox.ShowWindow(SW_SHOW);
    InvCorpseBootsBox.LoadItemImage();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadMercItemImages()
{
    if (!MainForm.isExpansionCharacter())
    {
        MercGroupBox.ShowWindow(SW_HIDE);
        InvMercHeadBox.ShowWindow(SW_HIDE);
        InvMercNeckBox.ShowWindow(SW_HIDE);
        InvMercHandRightBox.ShowWindow(SW_HIDE);
        InvMercTorsoBox.ShowWindow(SW_HIDE);
        InvMercHandLeftBox.ShowWindow(SW_HIDE);
        InvMercGloveBox.ShowWindow(SW_HIDE);
        InvMercRingRightBox.ShowWindow(SW_HIDE);
        InvMercBeltBox.ShowWindow(SW_HIDE);
        InvMercRingLeftBox.ShowWindow(SW_HIDE);
        InvMercBootsBox.ShowWindow(SW_HIDE);
        return;
    }

    if (!Merc.isHired())
    {
        InvMercHeadBox.ShowWindow(SW_HIDE);
        InvMercNeckBox.ShowWindow(SW_HIDE);
        InvMercHandRightBox.ShowWindow(SW_HIDE);
        InvMercTorsoBox.ShowWindow(SW_HIDE);
        InvMercHandLeftBox.ShowWindow(SW_HIDE);
        InvMercGloveBox.ShowWindow(SW_HIDE);
        InvMercRingRightBox.ShowWindow(SW_HIDE);
        InvMercBeltBox.ShowWindow(SW_HIDE);
        InvMercRingLeftBox.ShowWindow(SW_HIDE);
        InvMercBootsBox.ShowWindow(SW_HIDE);
    }
    else
    {
        InvMercHeadBox.ShowWindow(SW_SHOW);
        InvMercHeadBox.LoadItemImage();
        InvMercHandRightBox.ShowWindow(SW_SHOW);
        InvMercHandRightBox.LoadItemImage();
        InvMercTorsoBox.ShowWindow(SW_SHOW);
        InvMercTorsoBox.LoadItemImage();
        InvMercHandLeftBox.ShowWindow(SW_SHOW);
        InvMercHandLeftBox.LoadItemImage();

        if (d2ce::Mercenary::supportsEquippedId(d2ce::EnumEquippedId::NECK))
        {
            InvMercNeckBox.ShowWindow(SW_SHOW);
            InvMercNeckBox.LoadItemImage();
        }
        else
        {
            InvMercNeckBox.ShowWindow(SW_HIDE);
        }

        if (d2ce::Mercenary::supportsEquippedId(d2ce::EnumEquippedId::GLOVES))
        {
            InvMercGloveBox.ShowWindow(SW_SHOW);
            InvMercGloveBox.LoadItemImage();
        }
        else
        {
            InvMercGloveBox.ShowWindow(SW_HIDE);
        }

        if (d2ce::Mercenary::supportsEquippedId(d2ce::EnumEquippedId::RIGHT_RING))
        {
            InvMercRingRightBox.ShowWindow(SW_SHOW);
            InvMercRingRightBox.LoadItemImage();
        }
        else
        {
            InvMercRingRightBox.ShowWindow(SW_HIDE);
        }

        if (d2ce::Mercenary::supportsEquippedId(d2ce::EnumEquippedId::BELT))
        {
            InvMercBeltBox.ShowWindow(SW_SHOW);
            InvMercBeltBox.LoadItemImage();
        }
        else
        {
            InvMercBeltBox.ShowWindow(SW_HIDE);
        }

        if (d2ce::Mercenary::supportsEquippedId(d2ce::EnumEquippedId::LEFT_RING))
        {
            InvMercRingLeftBox.ShowWindow(SW_SHOW);
            InvMercRingLeftBox.LoadItemImage();
        }
        else
        {
            InvMercRingLeftBox.ShowWindow(SW_HIDE);
        }

        if (d2ce::Mercenary::supportsEquippedId(d2ce::EnumEquippedId::FEET))
        {
            InvMercBootsBox.ShowWindow(SW_SHOW);
            InvMercBootsBox.LoadItemImage();
        }
        else
        {
            InvMercBootsBox.ShowWindow(SW_HIDE);
        }
    }

    auto uName = utf8::utf8to16(Merc.getClassName());
    CString windowText(reinterpret_cast<LPCWSTR>(uName.c_str()));
    if (!windowText.IsEmpty())
    {
        uName = utf8::utf8to16(Merc.getAttributeName());
        CString attribName(reinterpret_cast<LPCWSTR>(uName.c_str()));
        if (!attribName.IsEmpty())
        {
            windowText += _T(" - ") + attribName;
        }
    }
    else
    {
        std::string strValue;
        d2ce::LocalizationHelpers::GetStringTxtValue("MiniPanelHire", strValue, "Mercenary");
        uName = utf8::utf8to16(strValue);
        windowText = reinterpret_cast<LPCWSTR>(uName.c_str());
    }

    MercGroupBox.SetWindowText(windowText);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadGolemItemImages()
{
    if (!MainForm.isExpansionCharacter() || (MainForm.getCharacterClass() != d2ce::EnumCharClass::Necromancer))
    {
        GolemGroupBox.ShowWindow(SW_HIDE);
        InvGolemBox.ShowWindow(SW_HIDE);
        return;
    }

    InvGolemBox.ShowWindow(SW_SHOW);
    InvGolemBox.LoadItemImage();
    std::string strValue;
    d2ce::LocalizationHelpers::GetStringTxtValue("IronGolem", strValue, "Iron Golem");
    auto uText = utf8::utf8to16(strValue);
    CString windowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    GolemGroupBox.SetWindowText(windowText);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::LoadGridItemImages()
{
    InvGrid.LoadItemImages();
    InvBeltGrid.LoadItemImages();
    InvStashGrid.LoadItemImages();

    std::string strValue;
    d2ce::LocalizationHelpers::GetStringTxtValue("StrHelp21", strValue, "Belt");
    auto uName = utf8::utf8to16(strValue);
    CString windowText(reinterpret_cast<LPCWSTR>(uName.c_str()));
    BeltGroupBox.SetWindowText(windowText);

    d2ce::LocalizationHelpers::GetStringTxtValue("box", strValue, "Horadric Cube");
    uName = utf8::utf8to16(strValue);
    windowText = reinterpret_cast<LPCWSTR>(uName.c_str());
    CubeGroupBox.SetWindowText(windowText);
    if (getHasHoradricCube())
    {
        InvCubeGrid.LoadItemImages();
        InvCubeGrid.ShowWindow(SW_SHOW);
    }
    else
    {
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
size_t CD2ItemsForm::convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, d2ce::ItemFilter filter)
{
    auto numCoverted = MainForm.convertGPSs(existingGem, desiredGem, filter);
    if (numCoverted > 0)
    {
        bool bFiltered = false;
        switch (filter.LocationId)
        {
        case d2ce::EnumItemLocation::STORED:
            switch (filter.AltPositionId)
            {
            case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
                bFiltered = true;
                InvCubeGrid.LoadItemImages();
                break;

            case d2ce::EnumAltItemLocation::INVENTORY:
                bFiltered = true;
                InvGrid.LoadItemImages();
                break;

            case d2ce::EnumAltItemLocation::STASH:
                bFiltered = true;
                InvStashGrid.LoadItemImages();
                break;
            }
            break;

        case d2ce::EnumItemLocation::BELT:
            InvBeltGrid.LoadItemImages();
            bFiltered = true;
            break;
        }

        if (!bFiltered)
        {
            // refresh all grids
            InvBeltGrid.LoadItemImages();
            InvCubeGrid.LoadItemImages();
            InvGrid.LoadItemImages();
            InvStashGrid.LoadItemImages();
        }
    }

    return numCoverted;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::updateGem(d2ce::Item& item, const std::array<std::uint8_t, 4>& newgem)
{
    if (!MainForm.updateGem(item, newgem))
    {
        return false;
    }

    refreshGrid(item);
    return true;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::upgradeGem(d2ce::Item& item)
{
    if (!MainForm.upgradeGem(item))
    {
        return false;
    }

    refreshGrid(item);
    return true;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::upgradeGems(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.upgradeGems(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::upgradePotion(d2ce::Item& item)
{
    if (!MainForm.upgradePotion(item))
    {
        return false;
    }

    refreshGrid(item);
    return true;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::upgradePotions(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.upgradePotions(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::upgradeToFullRejuvenationPotion(d2ce::Item& item)
{
    if (!MainForm.upgradeToFullRejuvenationPotion(item))
    {
        return false;
    }

    refreshGrid(item);
    return true;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::upgradeRejuvenationPotions(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.upgradeRejuvenationPotions(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::refreshGrid(const d2ce::Item& item)
{
    auto itemLocation = item.getLocation();
    switch (itemLocation)
    {
    case d2ce::EnumItemLocation::STORED:
        return refreshGrid(itemLocation, item.getAltPositionId());

    case d2ce::EnumItemLocation::EQUIPPED:
        return refreshEquipped(item);
    }

    return refreshGrid(itemLocation);
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::refreshGrid(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId)
{
    CWaitCursor wait;
    switch (locationId)
    {
    case d2ce::EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
            InvCubeGrid.LoadItemImages();
            if (getHasHoradricCube())
            {
                InvCubeGrid.ShowWindow(SW_SHOW);
            }
            else
            {
                InvCubeGrid.ShowWindow(SW_HIDE);
            }
            return true;

        case d2ce::EnumAltItemLocation::INVENTORY:
            InvGrid.LoadItemImages();
            return true;

        case d2ce::EnumAltItemLocation::STASH:
            InvStashGrid.LoadItemImages();
            return true;

        default: // refresh all stored
            InvCubeGrid.LoadItemImages();
            if (getHasHoradricCube())
            {
                InvCubeGrid.ShowWindow(SW_SHOW);
            }
            else
            {
                InvCubeGrid.ShowWindow(SW_HIDE);
            }
            InvGrid.LoadItemImages();
            InvStashGrid.LoadItemImages();
            return true;
        }
        break;

    case d2ce::EnumItemLocation::BELT:
        altPositionId = d2ce::EnumAltItemLocation::UNKNOWN;
        InvBeltGrid.LoadItemImages();
        return true;

    case d2ce::EnumItemLocation::BUFFER: // refresh all
        InvBeltGrid.LoadItemImages();
        InvCubeGrid.LoadItemImages();
        if (getHasHoradricCube())
        {
            InvCubeGrid.ShowWindow(SW_SHOW);
        }
        else
        {
            InvCubeGrid.ShowWindow(SW_HIDE);
        }

        InvGrid.LoadItemImages();
        InvStashGrid.LoadItemImages();
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::refreshGrid(d2ce::EnumItemLocation locationId)
{
    switch (locationId)
    {
    case d2ce::EnumItemLocation::BELT:
        InvBeltGrid.LoadItemImages();
        return true;

    case d2ce::EnumItemLocation::STORED: // refresh all stored
        InvCubeGrid.LoadItemImages();
        if (getHasHoradricCube())
        {
            InvCubeGrid.ShowWindow(SW_SHOW);
        }
        else
        {
            InvCubeGrid.ShowWindow(SW_HIDE);
        }

        InvGrid.LoadItemImages();
        InvStashGrid.LoadItemImages();
        return true;

    case d2ce::EnumItemLocation::BUFFER: // refresh all
        InvBeltGrid.LoadItemImages();
        InvCubeGrid.LoadItemImages();
        if (getHasHoradricCube())
        {
            InvCubeGrid.ShowWindow(SW_SHOW);
        }
        else
        {
            InvCubeGrid.ShowWindow(SW_HIDE);
        }

        InvGrid.LoadItemImages();
        InvStashGrid.LoadItemImages();
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::refreshGrid(d2ce::EnumAltItemLocation altPositionId)
{
    return refreshGrid(d2ce::EnumItemLocation::STORED, altPositionId);
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::refreshEquipped(const d2ce::Item& item)
{
    CWaitCursor wait;
    if (item.getLocation() != d2ce::EnumItemLocation::EQUIPPED)
    {
        return false;
    }

    return refreshEquipped(item.getEquippedId());
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::refreshEquipped(d2ce::EnumEquippedId id)
{
    switch (id)
    {
    case d2ce::EnumEquippedId::HEAD:
        InvHeadBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired())
        {
            InvMercHeadBox.LoadItemImage();
        }
        return true;

    case d2ce::EnumEquippedId::NECK:
        InvNeckBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired() && InvMercNeckBox.IsWindowVisible())
        {
            InvMercNeckBox.LoadItemImage();
        }
        return true;

    case d2ce::EnumEquippedId::LEFT_ARM:
    case d2ce::EnumEquippedId::RIGHT_ARM:
        InvHandRightBox.LoadItemImage();
        InvHandLeftBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired())
        {
            InvMercHandRightBox.LoadItemImage();
            InvMercHandLeftBox.LoadItemImage();
        }
        return true;

    case d2ce::EnumEquippedId::ALT_RIGHT_ARM:
    case d2ce::EnumEquippedId::ALT_LEFT_ARM:
        InvHandRightBox.LoadItemImage();
        InvHandLeftBox.LoadItemImage();
        return true;

    case d2ce::EnumEquippedId::TORSO:
        InvTorsoBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired())
        {
            InvMercTorsoBox.LoadItemImage();
        }
        return true;

    case d2ce::EnumEquippedId::GLOVES:
        InvGloveBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired() && InvMercGloveBox.IsWindowVisible())
        {
            InvMercGloveBox.LoadItemImage();
        }
        return true;

    case d2ce::EnumEquippedId::RIGHT_RING:
        InvRingRightBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired() && InvMercRingRightBox.IsWindowVisible())
        {
            InvMercRingRightBox.LoadItemImage();
        }
        return true;

    case d2ce::EnumEquippedId::BELT:
        InvBeltBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired() && InvMercBeltBox.IsWindowVisible())
        {
            InvMercBeltBox.LoadItemImage();
        }
        return true;

    case d2ce::EnumEquippedId::LEFT_RING:
        InvRingLeftBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired() && InvMercRingLeftBox.IsWindowVisible())
        {
            InvMercRingLeftBox.LoadItemImage();
        }
        return true;

    case d2ce::EnumEquippedId::FEET:
        InvBootsBox.LoadItemImage();
        if (MainForm.isExpansionCharacter() && Merc.isHired() && InvMercBootsBox.IsWindowVisible())
        {
            InvMercBootsBox.LoadItemImage();
        }
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::fillAllStackables(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.fillAllStackables(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::repairAllItems(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.repairAllItems(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::maxDurabilityAllItems(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.maxDurabilityAllItems(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::setIndestructibleAllItems(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.setIndestructibleAllItems(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::maxSocketCountAllItems(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.maxSocketCountAllItems(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::setSuperiorAllItems(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.setSuperiorAllItems(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::upgradeTierAllItems(d2ce::ItemFilter filter)
{
    auto numUpgraded = MainForm.upgradeTierAllItems(filter);
    if (numUpgraded > 0)
    {
        refreshGrid(filter.LocationId, filter.AltPositionId);
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::addItem(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    if (!MainForm.addItem(locationId, altPositionId, strcode))
    {
        return false;
    }

    refreshGrid(locationId, altPositionId);
    return true;
}
//---------------------------------------------------------------------------
size_t CD2ItemsForm::fillEmptySlots(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::array<std::uint8_t, 4>& strcode)
{
    auto numAdded = MainForm.fillEmptySlots(locationId, altPositionId, strcode);
    if (numAdded > 0)
    {
        refreshGrid(locationId, altPositionId);
    }

    return numAdded;
}
//---------------------------------------------------------------------------
void CD2ItemsForm::refreshAllGrids()
{
    CWaitCursor wait;
    InvBeltGrid.LoadItemImages();
    InvGrid.LoadItemImages();
    InvStashGrid.LoadItemImages();

    InvCubeGrid.LoadItemImages();
    if (getHasHoradricCube())
    {
        InvCubeGrid.ShowWindow(SW_SHOW);
    }
    else
    {
        InvCubeGrid.ShowWindow(SW_HIDE);
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
bool CD2ItemsForm::getItemLocationDimensions(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, d2ce::ItemDimensions& dimensions) const
{
    return MainForm.getItemLocationDimensions(locationId, altPositionId, dimensions);
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::getItemLocationDimensions(d2ce::EnumItemLocation locationId, d2ce::ItemDimensions& dimensions) const
{
    return MainForm.getItemLocationDimensions(locationId, dimensions);
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::getItemLocationDimensions(d2ce::EnumAltItemLocation altPositionId, d2ce::ItemDimensions& dimensions) const
{
    return MainForm.getItemLocationDimensions(altPositionId, dimensions);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::SetCurrItemInfo(CPoint point)
{
    CurrItemLocation = { 0, 0 };
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);
    CurrItem = const_cast<d2ce::Item*>(InvHitTest(point, &ti));
    if (CurrItem != nullptr)
    {
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(CurrItem->getLocation());
        if (CurrItem->getLocation() == d2ce::EnumItemLocation::EQUIPPED)
        {
            // figure out which stoarge location we are located in
            INT_PTR nHit = (INT_PTR)ti.uId;
            if (ti.uFlags & TTF_IDISHWND)
            {
                nHit = (INT_PTR)::GetDlgCtrlID(HWND(ti.uId));
            }

            switch (nHit)
            {
            case IDC_INV_HEAD:
            case IDC_INV_NECK:
            case IDC_INV_RIGHT_ARM:
            case IDC_INV_TORSO:
            case IDC_INV_LEFT_ARM:
            case IDC_INV_GLOVES:
            case IDC_INV_RIGHT_RING:
            case IDC_INV_BELT:
            case IDC_INV_LEFT_RING:
            case IDC_INV_FEET:
                CurrItemLocation[1] = 1; // character body
                break;

            case IDC_INV_CORPSE_HEAD:
            case IDC_INV_CORPSE_NECK:
            case IDC_INV_CORPSE_RIGHT_ARM:
            case IDC_INV_CORPSE_TORSO:
            case IDC_INV_CORPSE_LEFT_ARM:
            case IDC_INV_CORPSE_GLOVES:
            case IDC_INV_CORPSE_RIGHT_RING:
            case IDC_INV_CORPSE_BELT:
            case IDC_INV_CORPSE_LEFT_RING:
            case IDC_INV_CORPSE_FEET:
                CurrItemLocation[1] = 2; // character corpse
                break;

            case IDC_INV_MERC_HEAD:
            case IDC_INV_MERC_NECK:
            case IDC_INV_MERC_RIGHT_ARM:
            case IDC_INV_MERC_TORSO:
            case IDC_INV_MERC_LEFT_ARM:
            case IDC_INV_MERC_GLOVES:
            case IDC_INV_MERC_RIGHT_RING:
            case IDC_INV_MERC_BELT:
            case IDC_INV_MERC_LEFT_RING:
            case IDC_INV_MERC_FEET:
                CurrItemLocation[1] = 3; // Mercenary body
                break;

            case IDC_INV_GOLEM:
                CurrItemLocation[1] = 4; // Iron Golem
                break;
            }
        }
        else
        {
            CurrItemLocation[1] = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(CurrItem->getAltPositionId());
        }

        return;
    }

    // figure out which stoarge location we are located in
    INT_PTR nHit = (INT_PTR)ti.uId;
    if (ti.uFlags & TTF_IDISHWND)
    {
        nHit = (INT_PTR)::GetDlgCtrlID(HWND(ti.uId));
    }

    switch (nHit)
    {
    case IDC_INV_HEAD:
    case IDC_INV_NECK:
    case IDC_INV_RIGHT_ARM:
    case IDC_INV_TORSO:
    case IDC_INV_LEFT_ARM:
    case IDC_INV_GLOVES:
    case IDC_INV_RIGHT_RING:
    case IDC_INV_BELT:
    case IDC_INV_LEFT_RING:
    case IDC_INV_FEET:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::EQUIPPED);
        CurrItemLocation[1] = 1; // character body
        break;

    case IDC_INV_CORPSE_HEAD:
    case IDC_INV_CORPSE_NECK:
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_CORPSE_TORSO:
    case IDC_INV_CORPSE_LEFT_ARM:
    case IDC_INV_CORPSE_GLOVES:
    case IDC_INV_CORPSE_RIGHT_RING:
    case IDC_INV_CORPSE_BELT:
    case IDC_INV_CORPSE_LEFT_RING:
    case IDC_INV_CORPSE_FEET:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::EQUIPPED);
        CurrItemLocation[1] = 2; // character corpse
        break;

    case IDC_INV_MERC_HEAD:
    case IDC_INV_MERC_NECK:
    case IDC_INV_MERC_RIGHT_ARM:
    case IDC_INV_MERC_TORSO:
    case IDC_INV_MERC_LEFT_ARM:
    case IDC_INV_MERC_GLOVES:
    case IDC_INV_MERC_RIGHT_RING:
    case IDC_INV_MERC_BELT:
    case IDC_INV_MERC_LEFT_RING:
    case IDC_INV_MERC_FEET:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::EQUIPPED);
        CurrItemLocation[1] = 3; // Mercenary body
        break;

    case IDC_INV_GOLEM:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::EQUIPPED);
        CurrItemLocation[1] = 4; // Iron Golem
        break;

    case IDC_INV_GRID:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
        CurrItemLocation[1] = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::INVENTORY);
        break;

    case IDC_INV_STASH_GRID:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
        CurrItemLocation[1] = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::STASH);
        break;

    case IDC_INV_CUBE_GRID:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::STORED);
        CurrItemLocation[1] = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::HORADRIC_CUBE);
        break;

    case IDC_INV_BELT_GRID:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::BELT);
        break;

    default:
        CurrItemLocation[0] = static_cast<std::underlying_type_t<d2ce::EnumItemLocation>>(d2ce::EnumItemLocation::BUFFER);
        CurrItemLocation[1] = static_cast<std::underlying_type_t<d2ce::EnumAltItemLocation>>(d2ce::EnumAltItemLocation::UNKNOWN);
        break;
    }
}
//---------------------------------------------------------------------------
void CD2ItemsForm::ClearCurrItemInfo()
{
    CurrItemLocation = { 0, 0 };
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2ItemsForm::ResetCursor()
{
    if (ItemCursor != NULL)
    {
        ::DestroyIcon(ItemCursor);
        ItemCursor = NULL;
        CurrCursor = NULL;
        ::SetCursor(LoadCursor(NULL, IDC_ARROW));
        CurrDragItem = nullptr;
        CurrDragItemInv = d2ce::EnumItemInventory::UNKNOWN;
    }
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::GetInvBitmap(UINT id, CBitmap& image, CPoint point, TOOLINFO* pTI) const
{
    ScreenToClient(&point);

    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_HEAD:
        return InvHeadBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_HEAD:
        return InvCorpseHeadBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_HEAD:
        return InvMercHeadBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_NECK:
        return InvNeckBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_NECK:
        return InvCorpseNeckBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_NECK:
        return InvMercNeckBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_RIGHT_ARM:
        return InvHandRightBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_RIGHT_ARM:
        return InvCorpseHandRightBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_RIGHT_ARM:
        return InvMercHandRightBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_TORSO:
        return InvTorsoBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_TORSO:
        return InvCorpseTorsoBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_LEFT_ARM:
        return InvHandLeftBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_LEFT_ARM:
        return InvCorpseHandLeftBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_LEFT_ARM:
        return InvMercHandLeftBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_GLOVES:
        return InvGloveBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_GLOVES:
        return InvCorpseGloveBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_GLOVES:
        return InvMercGloveBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_RIGHT_RING:
        return InvRingRightBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_RIGHT_RING:
        return InvCorpseRingRightBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_RIGHT_RING:
        return InvMercRingRightBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_BELT:
        return InvBeltBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_BELT:
        return InvCorpseBeltBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_BELT:
        return InvMercBeltBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_LEFT_RING:
        return InvRingLeftBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_LEFT_RING:
        return InvCorpseRingLeftBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_LEFT_RING:
        return InvMercRingLeftBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_FEET:
        return InvBootsBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_CORPSE_FEET:
        return InvCorpseBootsBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_MERC_FEET:
        return InvMercBootsBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_GOLEM:
        return InvGolemBox.GetInvBitmap(image, point, pTI);

    case IDC_INV_GRID:
        return InvGrid.GetInvBitmap(image, point, pTI);

    case IDC_INV_STASH_GRID:
        return InvStashGrid.GetInvBitmap(image, point, pTI);

    case IDC_INV_CUBE_GRID:
        return InvCubeGrid.GetInvBitmap(image, point, pTI);

    case IDC_INV_BELT_GRID:
        return InvBeltGrid.GetInvBitmap(image, point, pTI);
    }

    return false;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::CanPlaceItem(UINT id, const d2ce::Item& item, CPoint point)
{
    ScreenToClient(&point);

    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_HEAD:
        return InvHeadBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_HEAD:
        return InvCorpseHeadBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_HEAD:
        return InvMercHeadBox.CanPlaceItem(item, point);

    case IDC_INV_NECK:
        return InvNeckBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_NECK:
        return InvCorpseNeckBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_NECK:
        return InvMercNeckBox.CanPlaceItem(item, point);

    case IDC_INV_RIGHT_ARM:
        return InvHandRightBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_RIGHT_ARM:
        return InvCorpseHandRightBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_RIGHT_ARM:
        return InvMercHandRightBox.CanPlaceItem(item, point);

    case IDC_INV_TORSO:
        return InvTorsoBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_TORSO:
        return InvCorpseTorsoBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.CanPlaceItem(item, point);

    case IDC_INV_LEFT_ARM:
        return InvHandLeftBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_LEFT_ARM:
        return InvCorpseHandLeftBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_LEFT_ARM:
        return InvMercHandLeftBox.CanPlaceItem(item, point);

    case IDC_INV_GLOVES:
        return InvGloveBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_GLOVES:
        return InvCorpseGloveBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_GLOVES:
        return InvMercGloveBox.CanPlaceItem(item, point);

    case IDC_INV_RIGHT_RING:
        return InvRingRightBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_RIGHT_RING:
        return InvCorpseRingRightBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_RIGHT_RING:
        return InvMercRingRightBox.CanPlaceItem(item, point);

    case IDC_INV_BELT:
        return InvBeltBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_BELT:
        return InvCorpseBeltBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_BELT:
        return InvMercBeltBox.CanPlaceItem(item, point);

    case IDC_INV_LEFT_RING:
        return InvRingLeftBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_LEFT_RING:
        return InvCorpseRingLeftBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_LEFT_RING:
        return InvMercRingLeftBox.CanPlaceItem(item, point);

    case IDC_INV_FEET:
        return InvBootsBox.CanPlaceItem(item, point);

    case IDC_INV_CORPSE_FEET:
        return InvCorpseBootsBox.CanPlaceItem(item, point);

    case IDC_INV_MERC_FEET:
        return InvMercBootsBox.CanPlaceItem(item, point);

    case IDC_INV_GOLEM:
        return InvGolemBox.CanPlaceItem(item, point);

    case IDC_INV_GRID:
        return InvGrid.CanPlaceItem(item, point);

    case IDC_INV_STASH_GRID:
        return InvStashGrid.CanPlaceItem(item, point);

    case IDC_INV_CUBE_GRID:
        return InvCubeGrid.CanPlaceItem(item, point);

    case IDC_INV_BELT_GRID:
        return InvBeltGrid.CanPlaceItem(item, point);
    }

    return false;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsForm::PlaceItem(UINT id, d2ce::Item& item, CPoint point, CBitmap& bitmap)
{
    ScreenToClient(&point);

    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_HEAD:
        return InvHeadBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_HEAD:
        return InvCorpseHeadBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_HEAD:
        return InvMercHeadBox.PlaceItem(item, point, bitmap);

    case IDC_INV_NECK:
        return InvNeckBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_NECK:
        return InvCorpseNeckBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_NECK:
        return InvMercNeckBox.PlaceItem(item, point, bitmap);

    case IDC_INV_RIGHT_ARM:
        return InvHandRightBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_RIGHT_ARM:
        return InvCorpseHandRightBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_RIGHT_ARM:
        return InvMercHandRightBox.PlaceItem(item, point, bitmap);

    case IDC_INV_TORSO:
        return InvTorsoBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_TORSO:
        return InvCorpseTorsoBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.PlaceItem(item, point, bitmap);

    case IDC_INV_LEFT_ARM:
        return InvHandLeftBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_LEFT_ARM:
        return InvCorpseHandLeftBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_LEFT_ARM:
        return InvMercHandLeftBox.PlaceItem(item, point, bitmap);

    case IDC_INV_GLOVES:
        return InvGloveBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_GLOVES:
        return InvCorpseGloveBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_GLOVES:
        return InvMercGloveBox.PlaceItem(item, point, bitmap);

    case IDC_INV_RIGHT_RING:
        return InvRingRightBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_RIGHT_RING:
        return InvCorpseRingRightBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_RIGHT_RING:
        return InvMercRingRightBox.PlaceItem(item, point, bitmap);

    case IDC_INV_BELT:
        return InvBeltBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_BELT:
        return InvCorpseBeltBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_BELT:
        return InvMercBeltBox.PlaceItem(item, point, bitmap);

    case IDC_INV_LEFT_RING:
        return InvRingLeftBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_LEFT_RING:
        return InvCorpseRingLeftBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_LEFT_RING:
        return InvMercRingLeftBox.PlaceItem(item, point, bitmap);

    case IDC_INV_FEET:
        return InvBootsBox.PlaceItem(item, point, bitmap);

    case IDC_INV_CORPSE_FEET:
        return InvCorpseBootsBox.PlaceItem(item, point, bitmap);

    case IDC_INV_MERC_FEET:
        return InvMercBootsBox.PlaceItem(item, point, bitmap);

    case IDC_INV_GOLEM:
        return InvGolemBox.PlaceItem(item, point, bitmap);

    case IDC_INV_GRID:
        return InvGrid.PlaceItem(item, point, bitmap);

    case IDC_INV_STASH_GRID:
        return InvStashGrid.PlaceItem(item, point, bitmap);

    case IDC_INV_CUBE_GRID:
        return InvCubeGrid.PlaceItem(item, point, bitmap);

    case IDC_INV_BELT_GRID:
        return InvBeltGrid.PlaceItem(item, point, bitmap);
    }

    return false;
}
//---------------------------------------------------------------------------
d2ce::ItemFilter CD2ItemsForm::GetCurrItemFilter() const
{
    d2ce::ItemFilter filter;
    if (CurrItemLocation[0] == 0 && CurrItemLocation[1] == 0)
    {
        return filter; // invalid state
    }

    switch (static_cast<d2ce::EnumItemLocation>(CurrItemLocation[0]))
    {
    case d2ce::EnumItemLocation::STORED:
        filter.LocationId = d2ce::EnumItemLocation::STORED;
        filter.AltPositionId = static_cast<d2ce::EnumAltItemLocation>(CurrItemLocation[1]);
        break;

    case d2ce::EnumItemLocation::BELT:
        filter.LocationId = d2ce::EnumItemLocation::BELT;
        break;

    case d2ce::EnumItemLocation::EQUIPPED:
        filter.LocationId = d2ce::EnumItemLocation::EQUIPPED;
        switch (CurrItemLocation[1])
        {
        case 1: // body
            filter.LocationId = d2ce::EnumItemLocation::EQUIPPED;
            filter.IsBody = true;
            break;

        case 2: // corpse
            filter.LocationId = d2ce::EnumItemLocation::EQUIPPED;
            filter.IsCorpse = true;
            break;

        case 3: // mercenary
            filter.LocationId = d2ce::EnumItemLocation::EQUIPPED;
            filter.IsMerc = true;
            break;

        case 4: // Iron Golem
            filter.LocationId = d2ce::EnumItemLocation::EQUIPPED;
            filter.IsGolem = true;
            break;
        }
        break;
    }

    return filter;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::setItemRuneword(d2ce::Item& item, std::uint16_t id)
{
    auto preLocationId = item.getLocation();
    auto preAltPositionId = item.getAltPositionId();
    auto preEquipId = item.getEquippedId();
    if (MainForm.setItemRuneword(item, id))
    {
        if (preEquipId != d2ce::EnumEquippedId::NONE)
        {
            refreshEquipped(preEquipId);
        }
        else
        {
            refreshGrid(preLocationId, preAltPositionId);
        }

        return true;
    }

    return false;
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

    case IDC_INV_MERC_NECK:
        return InvMercNeckBox.GetInvItem();

    case IDC_INV_RIGHT_ARM:
        return InvHandRightBox.GetInvItem();

    case IDC_INV_CORPSE_RIGHT_ARM:
        return InvCorpseHandRightBox.GetInvItem();

    case IDC_INV_MERC_RIGHT_ARM:
        return InvMercHandRightBox.GetInvItem();

    case IDC_INV_TORSO:
        return InvTorsoBox.GetInvItem();

    case IDC_INV_CORPSE_TORSO:
        return InvCorpseTorsoBox.GetInvItem();

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.GetInvItem();

    case IDC_INV_LEFT_ARM:
        return InvHandLeftBox.GetInvItem();

    case IDC_INV_CORPSE_LEFT_ARM:
        return InvCorpseHandLeftBox.GetInvItem();

    case IDC_INV_MERC_LEFT_ARM:
        return InvMercHandLeftBox.GetInvItem();

    case IDC_INV_GLOVES:
        return InvGloveBox.GetInvItem();

    case IDC_INV_CORPSE_GLOVES:
        return InvCorpseGloveBox.GetInvItem();

    case IDC_INV_MERC_GLOVES:
        return InvMercGloveBox.GetInvItem();

    case IDC_INV_RIGHT_RING:
        return InvRingRightBox.GetInvItem();

    case IDC_INV_CORPSE_RIGHT_RING:
        return InvCorpseRingRightBox.GetInvItem();

    case IDC_INV_MERC_RIGHT_RING:
        return InvMercRingRightBox.GetInvItem();

    case IDC_INV_BELT:
        return InvBeltBox.GetInvItem();

    case IDC_INV_CORPSE_BELT:
        return InvCorpseBeltBox.GetInvItem();

    case IDC_INV_MERC_BELT:
        return InvMercBeltBox.GetInvItem();

    case IDC_INV_LEFT_RING:
        return InvRingLeftBox.GetInvItem();

    case IDC_INV_CORPSE_LEFT_RING:
        return InvCorpseRingLeftBox.GetInvItem();

    case IDC_INV_MERC_LEFT_RING:
        return InvMercRingLeftBox.GetInvItem();

    case IDC_INV_FEET:
        return InvBootsBox.GetInvItem();

    case IDC_INV_CORPSE_FEET:
        return InvCorpseBootsBox.GetInvItem();

    case IDC_INV_MERC_FEET:
        return InvMercBootsBox.GetInvItem();

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

    case IDC_INV_MERC_NECK:
        return InvMercNeckBox.InvHitTest(point, pTI);

    case IDC_INV_RIGHT_ARM:
        return InvHandRightBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_RIGHT_ARM:
        return InvCorpseHandRightBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_RIGHT_ARM:
        return InvMercHandRightBox.InvHitTest(point, pTI);

    case IDC_INV_TORSO:
        return InvTorsoBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_TORSO:
        return InvCorpseTorsoBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_TORSO:
        return InvMercTorsoBox.InvHitTest(point, pTI);

    case IDC_INV_LEFT_ARM:
        return InvHandLeftBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_LEFT_ARM:
        return InvCorpseHandLeftBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_LEFT_ARM:
        return InvMercHandLeftBox.InvHitTest(point, pTI);

    case IDC_INV_GLOVES:
        return InvGloveBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_GLOVES:
        return InvCorpseGloveBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_GLOVES:
        return InvMercGloveBox.InvHitTest(point, pTI);

    case IDC_INV_RIGHT_RING:
        return InvRingRightBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_RIGHT_RING:
        return InvCorpseRingRightBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_RIGHT_RING:
        return InvMercRingRightBox.InvHitTest(point, pTI);

    case IDC_INV_BELT:
        return InvBeltBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_BELT:
        return InvCorpseBeltBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_BELT:
        return InvMercBeltBox.InvHitTest(point, pTI);

    case IDC_INV_LEFT_RING:
        return InvRingLeftBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_LEFT_RING:
        return InvCorpseRingLeftBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_LEFT_RING:
        return InvMercRingLeftBox.InvHitTest(point, pTI);

    case IDC_INV_FEET:
        return InvBootsBox.InvHitTest(point, pTI);

    case IDC_INV_CORPSE_FEET:
        return InvCorpseBootsBox.InvHitTest(point, pTI);

    case IDC_INV_MERC_FEET:
        return InvMercBootsBox.InvHitTest(point, pTI);

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
std::optional<d2ce::EnumCharClass> CD2ItemsForm::getCharClass() const
{
    return MainForm.getCharacterInfo().getClass();
}
//---------------------------------------------------------------------------
std::optional<d2ce::CharStats> CD2ItemsForm::getDisplayedCharStats() const
{
    return MainForm.getDisplayedCharStats();
}
//---------------------------------------------------------------------------
std::optional<d2ce::Mercenary*> CD2ItemsForm::getMercInfo() const
{
    return &Merc;
}
//---------------------------------------------------------------------------
CSize CD2ItemsForm::getInvGridSize(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId) const
{
    d2ce::ItemDimensions dimensions;
    switch (locationId)
    {
    case d2ce::EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case d2ce::EnumAltItemLocation::INVENTORY:
            getItemLocationDimensions(d2ce::EnumAltItemLocation::INVENTORY, dimensions);
            break;

        case d2ce::EnumAltItemLocation::STASH:
            getItemLocationDimensions(d2ce::EnumAltItemLocation::STASH, dimensions);
            break;

        case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
            getItemLocationDimensions(d2ce::EnumAltItemLocation::HORADRIC_CUBE, dimensions);
            break;
        }
        break;

    case d2ce::EnumItemLocation::BELT:
        getItemLocationDimensions(d2ce::EnumItemLocation::BELT, dimensions);
        break;
    }

    return CSize(dimensions.InvWidth, dimensions.InvHeight);
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2ItemsForm::getInvGridItems(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId) const
{
    static std::vector<std::reference_wrapper<d2ce::Item>> s_empty;
    switch (locationId)
    {
    case d2ce::EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case d2ce::EnumAltItemLocation::INVENTORY:
            return getItemsInInventory();

        case d2ce::EnumAltItemLocation::STASH:
            return getItemsInStash();

        case d2ce::EnumAltItemLocation::HORADRIC_CUBE:
            return getItemsInHoradricCube();
        }
        break;

    case d2ce::EnumItemLocation::BELT:
        return getItemsInBelt();
    }

    return s_empty;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2ItemsForm::getInvEquippedItem(d2ce::EnumEquippedId equippedId, d2ce::EnumItemInventory invType) const
{
    switch (invType)
    {
    case d2ce::EnumItemInventory::PLAYER:
        for (const auto& item : MainForm.getEquippedItems())
        {
            if (equippedId == item.get().getEquippedId())
            {
                return &(item.get());
            }
        }
        break;

    case d2ce::EnumItemInventory::CORPSE:
        for (const auto& item : MainForm.getCorpseItems())
        {
            if (equippedId == item.getEquippedId())
            {
                return &item;
            }
        }
        break;

    case d2ce::EnumItemInventory::MERCENARY:
        for (const auto& item : Merc.getItems())
        {
            if (equippedId == item.getEquippedId())
            {
                return &item;
            }
        }
        break;

    case d2ce::EnumItemInventory::GOLEM:
        if (!MainForm.getGolemItem().empty())
        {
            return &MainForm.getGolemItem().front();
        }
        break;
    }
    return nullptr;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    return MainForm.getItemBitmap(item, bitmap);
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    auto bHasBelt = MainForm.getHasBeltEquipped();
    auto bHasCube = MainForm.getHasHoradricCube();
    auto numberOfBeltSlots = MainForm.getMaxNumberOfItemsInBelt();
    auto preLocationId = item.getLocation();
    auto preAltPositionId = item.getAltPositionId();
    auto preEquippedId = item.getEquippedId();
    if ((preLocationId == d2ce::EnumItemLocation::EQUIPPED) && (preEquippedId == d2ce::EnumEquippedId::NONE))
    {
        preLocationId = d2ce::EnumItemLocation::BUFFER;
        preAltPositionId = d2ce::EnumAltItemLocation::UNKNOWN;
    }

    if (MainForm.setItemLocation(item, locationId, altPositionId, positionX, positionY, invType, pRemovedItem))
    {
        switch (preLocationId)
        {
        case d2ce::EnumItemLocation::EQUIPPED:
            refreshEquipped(preEquippedId);
            if ((bHasBelt != MainForm.getHasBeltEquipped()) || (numberOfBeltSlots != MainForm.getMaxNumberOfItemsInBelt()))
            {
                refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN);
            }
            else if (bHasCube != MainForm.getHasHoradricCube())
            {
                refreshGrid(d2ce::EnumItemLocation::STORED);
            }
            else
            {
                if ((preLocationId != locationId) ||
                    (preAltPositionId != altPositionId))
                {
                    refreshGrid(locationId, altPositionId);
                }
            }
            break;

        default:
            if (bHasCube != MainForm.getHasHoradricCube())
            {
                refreshGrid(d2ce::EnumItemLocation::STORED);
            }
            else
            {
                refreshGrid(preLocationId, preAltPositionId);
                if ((preLocationId != locationId) ||
                    (preAltPositionId != altPositionId))
                {
                    refreshGrid(locationId, altPositionId);
                }
            }
            break;
        }

        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    auto bHasBelt = MainForm.getHasBeltEquipped();
    auto bHasCube = MainForm.getHasHoradricCube();
    auto numberOfBeltSlots = MainForm.getMaxNumberOfItemsInBelt();
    auto preLocationId = item.getLocation();
    auto preAltPositionId = item.getAltPositionId();
    auto preEquippedId = item.getEquippedId();
    if ((preLocationId == d2ce::EnumItemLocation::EQUIPPED) && (preEquippedId == d2ce::EnumEquippedId::NONE))
    {
        preLocationId = d2ce::EnumItemLocation::BUFFER;
        preAltPositionId = d2ce::EnumAltItemLocation::UNKNOWN;
    }

    if (MainForm.setItemLocation(item, locationId, positionX, positionY, invType, pRemovedItem))
    {
        switch (preLocationId)
        {
        case d2ce::EnumItemLocation::EQUIPPED:
            refreshEquipped(preEquippedId);
            if ((bHasBelt != MainForm.getHasBeltEquipped()) || (numberOfBeltSlots != MainForm.getMaxNumberOfItemsInBelt()))
            {
                refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN);
            }
            else if (bHasCube != MainForm.getHasHoradricCube())
            {
                refreshGrid(d2ce::EnumItemLocation::STORED);
            }
            else
            {
                if (preLocationId != locationId)
                {
                    refreshGrid(locationId);
                }
            }
            break;

        default:
            if (bHasCube != MainForm.getHasHoradricCube())
            {
                refreshGrid(d2ce::EnumItemLocation::STORED);
            }
            else
            {
                refreshGrid(preLocationId, preAltPositionId);
                if (preLocationId != locationId)
                {
                    refreshGrid(locationId);
                }
            }
            break;
        }

        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::setItemLocation(d2ce::Item& item, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    auto bHasBelt = MainForm.getHasBeltEquipped();
    auto numberOfBeltSlots = MainForm.getMaxNumberOfItemsInBelt();
    auto preLocationId = item.getLocation();
    auto preAltPositionId = item.getAltPositionId();
    auto preEquippedId = item.getEquippedId();
    if ((preLocationId == d2ce::EnumItemLocation::EQUIPPED) && (preEquippedId == d2ce::EnumEquippedId::NONE))
    {
        preLocationId = d2ce::EnumItemLocation::BUFFER;
        preAltPositionId = d2ce::EnumAltItemLocation::UNKNOWN;
    }

    if (MainForm.setItemLocation(item, altPositionId, positionX, positionY, invType, pRemovedItem))
    {
        switch (preLocationId)
        {
        case d2ce::EnumItemLocation::EQUIPPED:
            refreshEquipped(preEquippedId);
            if ((bHasBelt != MainForm.getHasBeltEquipped()) || (numberOfBeltSlots != MainForm.getMaxNumberOfItemsInBelt()))
            {
                refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN);
            }
            else
            {
                if (preAltPositionId != altPositionId)
                {
                    refreshGrid(d2ce::EnumItemLocation::STORED, altPositionId);
                }
            }
            break;

        default:
            refreshGrid(preLocationId, preAltPositionId);
            if (preAltPositionId != altPositionId)
            {
                refreshGrid(d2ce::EnumItemLocation::STORED, altPositionId);
            }
            break;
        }

        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool CD2ItemsForm::setItemLocation(d2ce::Item& item, d2ce::EnumEquippedId equippedId, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    auto bHasBelt = MainForm.getHasBeltEquipped();
    auto numberOfBeltSlots = MainForm.getMaxNumberOfItemsInBelt();
    auto preLocationId = item.getLocation();
    auto preAltPositionId = item.getAltPositionId();
    auto preEquipId = item.getEquippedId();
    if ((preLocationId == d2ce::EnumItemLocation::EQUIPPED) && (preEquipId == d2ce::EnumEquippedId::NONE))
    {
        preLocationId = d2ce::EnumItemLocation::BUFFER;
        preAltPositionId = d2ce::EnumAltItemLocation::UNKNOWN;
    }

    if (MainForm.setItemLocation(item, equippedId, invType, pRemovedItem))
    {
        if ((bHasBelt != MainForm.getHasBeltEquipped()) || (numberOfBeltSlots != MainForm.getMaxNumberOfItemsInBelt()))
        {
            refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN); // refresh all grids
            refreshEquipped(equippedId);
        }
        else
        {
            if (preEquipId != d2ce::EnumEquippedId::NONE)
            {
                refreshEquipped(preEquipId);
            }
            else
            {
                refreshGrid(preLocationId, preAltPositionId);
            }

            if (preEquipId != equippedId)
            {
                refreshEquipped(equippedId);
            }
        }

        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
BOOL CD2ItemsForm::OnInitDialog()
{
    __super::OnInitDialog();

    std::string strValue;
    d2ce::LocalizationHelpers::GetStringTxtValue("strpanel4", strValue, "Inventory");
    auto uText = utf8::utf8to16(strValue);
    CString windowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
    SetWindowText(windowText);

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

    if (!MainForm.hasSharedStash())
    {
        auto* pWnd = GetDlgItem(IDC_SHARED_STASH_BUTTON);
        if (pWnd != nullptr)
        {
            d2ce::LocalizationHelpers::GetStringTxtValue("stash", strValue, "Stash");
            uText = utf8::utf8to16(strValue);
            windowText = reinterpret_cast<LPCWSTR>(uText.c_str());
            d2ce::LocalizationHelpers::GetStringTxtValue("ConcatenatedStringEnding", strValue, "...");
            uText = utf8::utf8to16(strValue);
            windowText += reinterpret_cast<LPCWSTR>(uText.c_str());
            pWnd->SetWindowText(windowText);
            pWnd->EnableWindow(FALSE);
        }
    }

    {
        CWaitCursor wait;
        LoadEquippedItemImages();
        LoadCorpseItemImages();
        LoadMercItemImages();
        LoadGolemItemImages();
        LoadGridItemImages();
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnDestroy()
{
    ResetCursor();
    __super::OnDestroy();
}
//---------------------------------------------------------------------------
BOOL CD2ItemsForm::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    if (ItemCursor != NULL)
    {
        *pResult = 0;
        return FALSE;
    }

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
        case IDC_INV_MERC_NECK:
        case IDC_INV_RIGHT_ARM:
        case IDC_INV_CORPSE_RIGHT_ARM:
        case IDC_INV_MERC_RIGHT_ARM:
        case IDC_INV_TORSO:
        case IDC_INV_CORPSE_TORSO:
        case IDC_INV_MERC_TORSO:
        case IDC_INV_LEFT_ARM:
        case IDC_INV_CORPSE_LEFT_ARM:
        case IDC_INV_MERC_LEFT_ARM:
        case IDC_INV_GLOVES:
        case IDC_INV_CORPSE_GLOVES:
        case IDC_INV_MERC_GLOVES:
        case IDC_INV_RIGHT_RING:
        case IDC_INV_CORPSE_RIGHT_RING:
        case IDC_INV_MERC_RIGHT_RING:
        case IDC_INV_BELT:
        case IDC_INV_CORPSE_BELT:
        case IDC_INV_MERC_BELT:
        case IDC_INV_LEFT_RING:
        case IDC_INV_CORPSE_LEFT_RING:
        case IDC_INV_MERC_LEFT_RING:
        case IDC_INV_FEET:
        case IDC_INV_CORPSE_FEET:
        case IDC_INV_MERC_FEET:
        case IDC_INV_GRID:
        case IDC_INV_STASH_GRID:
        case IDC_INV_CUBE_GRID:
        case IDC_INV_BELT_GRID:
        case IDC_INV_GOLEM:
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
void CD2ItemsForm::OnBnClickedOk()
{
    UpdateData(TRUE); // save results
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnBnClickedSharedStashButton()
{
    CD2SharedStashForm dlg(MainForm);
    dlg.DoModal();
    SetFocus();
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
    SetCurrItemInfo(hitTestPoint);
    auto filter(GetCurrItemFilter());
    if (CurrItem == nullptr)
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_ITEM_MENU));

        CMenu* pPopup = FindPopup(menu, 2);
        ENSURE(pPopup != NULL);

        if (filter.LocationId == d2ce::EnumItemLocation::BELT)
        {
            CMenu* pSubPopup = FindPopupByChild(*pPopup, ID_ITEM_CONTEXT_UPGRADE_GEMS);
            if (pSubPopup != nullptr)
            {
                pSubPopup->RemoveMenu(ID_ITEM_CONTEXT_UPGRADE_GEMS, MF_BYCOMMAND);
                pSubPopup->RemoveMenu(ID_ITEM_CONTEXT_MAXFILLSTACKABLES, MF_BYCOMMAND);
                pSubPopup->RemoveMenu(ID_ITEM_CONTEXT_FIXALLITEMS, MF_BYCOMMAND);
                pSubPopup->RemoveMenu(ID_ITEM_CONTEXT_MAXDURABILITYFORALLITEMS, MF_BYCOMMAND);
                pSubPopup->RemoveMenu(ID_ITEM_CONTEXT_INDESTRUCTIBLEFORALLITEMS, MF_BYCOMMAND);
                pSubPopup->RemoveMenu(ID_ITEM_CONTEXT_MAXSOCKETSFORALLITEMS, MF_BYCOMMAND);
                pSubPopup->RemoveMenu(ID_ITEM_CONTEXT_UNSOCKET, MF_BYCOMMAND);
                pSubPopup->RemoveMenu(ID_ITEM_CONTEXT_APPLY_RUNEWORD, MF_BYCOMMAND);

                auto numItems = pSubPopup->GetMenuItemCount();
                if (numItems > 1)
                {
                    if (pSubPopup->GetMenuItemID(numItems - 1) == 0)
                    {
                        pSubPopup->RemoveMenu(numItems - 1, MF_BYPOSITION);
                    }
                }
            }
        }
        else if (filter.LocationId == d2ce::EnumItemLocation::EQUIPPED)
        {
            auto pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_GPS_CREATOR);
            if (pos >= 0)
            {
                pPopup->RemoveMenu(pos, MF_BYPOSITION);
            }
        }

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
        return;
    }

    const auto& itemType = CurrItem->getItemTypeHelper();
    bool isStackable = itemType.isStackable();
    bool isArmor = itemType.isArmor();
    bool isWeapon = itemType.isWeapon();
    bool isGem = itemType.isGem();
    bool isPotion = itemType.isPotion();
    bool isRune = itemType.isRune();
    bool canHaveSockets = CurrItem->canHaveSockets();
    bool canPersonalize = CurrItem->canPersonalize();
    bool isSocketed = CurrItem->isSocketed();
    bool canMakeSuperior = CurrItem->canMakeSuperior();
    bool canUpgradeTier = CurrItem->isUpgradableItem();
    bool canFixDurability = CurrItem->canFixDurability();
    bool canMakeIndestructible = CurrItem->canMakeIndestructible();
    bool canMakeEthereal = CurrItem->canMakeEthereal();
    bool canRemoveEthereal = CurrItem->isEthereal();

    bool removeDurabilityMenu = !canFixDurability && !canMakeIndestructible && !canMakeEthereal && !canRemoveEthereal;
    bool removeQualityMenu = !canMakeSuperior && !canUpgradeTier;

    bool canModifyItem = false;
    switch (CurrItem->getQuality())
    {
    case d2ce::EnumItemQuality::INFERIOR:
    case d2ce::EnumItemQuality::NORMAL:
        canModifyItem = false;
        break;

    default:
        canModifyItem = (itemType.isUniqueItem() || itemType.isSetItem() || itemType.isQuestItem()) ? false : true;
        break;
    }

    if (isArmor || isWeapon || isStackable)
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_ITEM_MENU));

        CMenu* pPopup = FindPopup(menu, 0);
        ENSURE(pPopup != NULL);

        if (!isStackable)
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_LOAD, MF_BYCOMMAND);
        }

        if (!canModifyItem)
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_MODIFY_ITEM, MF_BYCOMMAND);
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

        pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_FIX);
        if (pos >= 0)
        {
            if (removeDurabilityMenu)
            {
                pPopup->RemoveMenu(pos, MF_BYPOSITION);
            }
            else
            {
                CMenu* pSubPopup = pPopup->GetSubMenu(pos);
                if (pSubPopup != nullptr)
                {
                    if (!canFixDurability)
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_FIX, MF_BYCOMMAND);
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_MAXDURABILITY, MF_BYCOMMAND);
                    }

                    if (!canMakeIndestructible)
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_INDESTRUCTIBLE, MF_BYCOMMAND);
                    }

                    if (!canMakeEthereal)
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_MAKEETHEREAL, MF_BYCOMMAND);
                    }

                    if (!canRemoveEthereal)
                    {
                        pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_REMOVEETHEREAL, MF_BYCOMMAND);
                    }
                }
            }
        }

        pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_MAKESUPERIORQUALITY);
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
                }
            }
        }

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
    else if (isGem | isPotion | isRune)
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_ITEM_MENU));

        CMenu* pPopup = FindPopup(menu, 1);
        ENSURE(pPopup != NULL);

        if (isRune)
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_POTION, MF_BYCOMMAND);
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_REJUVENATION, MF_BYCOMMAND);
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_GEM, MF_BYCOMMAND);
        }
        else if (isGem)
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_POTION, MF_BYCOMMAND);
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_REJUVENATION, MF_BYCOMMAND);
            if (!CurrItem->isUpgradableGem())
            {
                pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_GEM, MF_BYCOMMAND);
            }
        }
        else
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_GEM, MF_BYCOMMAND);
            if (filter.LocationId == d2ce::EnumItemLocation::BELT)
            {
                CMenu* pSubPopup = FindPopupByChild(*pPopup, ID_ITEM_CONTEXT_UPGRADE_GEMS);
                if (pSubPopup != nullptr)
                {
                    pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_GEMS, MF_BYCOMMAND);
                    pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_MAXFILLSTACKABLES, MF_BYCOMMAND);
                    pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_FIXALLITEMS, MF_BYCOMMAND);
                    pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_MAXDURABILITYFORALLITEMS, MF_BYCOMMAND);
                    pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_INDESTRUCTIBLEFORALLITEMS, MF_BYCOMMAND);
                    pSubPopup->DeleteMenu(ID_ITEM_CONTEXT_MAXSOCKETSFORALLITEMS, MF_BYCOMMAND);
                    auto numItems = pSubPopup->GetMenuItemCount();
                    if (numItems > 1)
                    {
                        if (pSubPopup->GetMenuItemID(numItems - 1) == 0)
                        {
                            pSubPopup->RemoveMenu(numItems - 1, MF_BYPOSITION);
                        }
                    }
                }
            }

            if (!CurrItem->isUpgradablePotion())
            {
                pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_POTION, MF_BYCOMMAND);
            }

            if (!CurrItem->isUpgradableToFullRejuvenationPotion())
            {
                pPopup->DeleteMenu(ID_ITEM_CONTEXT_UPGRADE_REJUVENATION, MF_BYCOMMAND);
            }
        }

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
    else
    {
        removeQualityMenu = !canMakeSuperior;

        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_ITEM_MENU));

        CMenu* pPopup = FindPopup(menu, 3);
        ENSURE(pPopup != NULL);

        if (!canModifyItem)
        {
            pPopup->DeleteMenu(ID_ITEM_CONTEXT_MODIFY_ITEM, MF_BYCOMMAND);
        }

        auto pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_MAKESUPERIORQUALITY);
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
                }
            }
        }

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnLButtonDown(UINT nFlags, CPoint point)
{
    TOOLINFO ti = { 0 };
    INT_PTR nHit = __super::OnToolHitTest(point, &ti);
    if (nHit == -1)
    {
        if (ItemCursor != NULL)
        {
            if (CurrDragItem != NULL)
            {
                const d2ce::Item* pRemovedItem = nullptr;
                setItemLocation(*CurrDragItem, d2ce::EnumItemLocation::BUFFER, 0, 0, d2ce::EnumItemInventory::BUFFER, pRemovedItem);
            }

            // placed it
            ResetCursor();
        }

        __super::OnLButtonDown(nFlags, point);
        return;
    }

    switch (nHit)
    {
    case IDC_INV_GRID:
    case IDC_INV_BELT_GRID:
    case IDC_INV_STASH_GRID:
    case IDC_INV_CUBE_GRID:
    case IDC_INV_HEAD:
    case IDC_INV_CORPSE_HEAD:
    case IDC_INV_MERC_HEAD:
    case IDC_INV_GLOVES:
    case IDC_INV_CORPSE_GLOVES:
    case IDC_INV_MERC_GLOVES:
    case IDC_INV_FEET:
    case IDC_INV_CORPSE_FEET:
    case IDC_INV_MERC_FEET:
    case IDC_INV_NECK:
    case IDC_INV_CORPSE_NECK:
    case IDC_INV_MERC_NECK:
    case IDC_INV_RIGHT_RING:
    case IDC_INV_CORPSE_RIGHT_RING:
    case IDC_INV_MERC_RIGHT_RING:
    case IDC_INV_LEFT_RING:
    case IDC_INV_CORPSE_LEFT_RING:
    case IDC_INV_MERC_LEFT_RING:
    case IDC_INV_RIGHT_ARM:
    case IDC_INV_CORPSE_RIGHT_ARM:
    case IDC_INV_MERC_RIGHT_ARM:
    case IDC_INV_TORSO:
    case IDC_INV_CORPSE_TORSO:
    case IDC_INV_MERC_TORSO:
    case IDC_INV_LEFT_ARM:
    case IDC_INV_CORPSE_LEFT_ARM:
    case IDC_INV_MERC_LEFT_ARM:
    case IDC_INV_GOLEM:
    case IDC_INV_BELT:
    case IDC_INV_CORPSE_BELT:
    case IDC_INV_MERC_BELT:
        break;

    default:
        __super::OnLButtonDown(nFlags, point);
        return;
    }

    if (ItemCursor == NULL)
    {
        // Make sure we have hit an item
        auto pt = point;
        ClientToScreen(&pt);
        CurrDragItem = const_cast<d2ce::Item*>(InvHitTest((UINT)nHit, pt, &ti));
        if (CurrDragItem == nullptr)
        {
            __super::OnLButtonDown(nFlags, point);
            return;
        }

        CBitmap image;
        if (!GetInvBitmap((UINT)nHit, image, pt, &ti))
        {
            CurrDragItem = nullptr;
            __super::OnLButtonDown(nFlags, point);
            return;
        }

        ItemCursor = CreateItemCursor(image);
        CurrCursor = ItemCursor;
        ::SetCursor(CurrCursor);

        switch (nHit)
        {
        case IDC_INV_CORPSE_HEAD:
        case IDC_INV_CORPSE_NECK:
        case IDC_INV_CORPSE_RIGHT_ARM:
        case IDC_INV_CORPSE_TORSO:
        case IDC_INV_CORPSE_LEFT_ARM:
        case IDC_INV_CORPSE_GLOVES:
        case IDC_INV_CORPSE_RIGHT_RING:
        case IDC_INV_CORPSE_BELT:
        case IDC_INV_CORPSE_LEFT_RING:
        case IDC_INV_CORPSE_FEET:
            CurrDragItemInv = d2ce::EnumItemInventory::CORPSE;
            break;

        case IDC_MERC_GROUP:
        case IDC_INV_MERC_HEAD:
        case IDC_INV_MERC_NECK:
        case IDC_INV_MERC_RIGHT_ARM:
        case IDC_INV_MERC_TORSO:
        case IDC_INV_MERC_LEFT_ARM:
        case IDC_INV_MERC_GLOVES:
        case IDC_INV_MERC_RIGHT_RING:
        case IDC_INV_MERC_BELT:
        case IDC_INV_MERC_LEFT_RING:
        case IDC_INV_MERC_FEET:
            CurrDragItemInv = d2ce::EnumItemInventory::MERCENARY;
            break;

        case IDC_INV_GOLEM:
            CurrDragItemInv = d2ce::EnumItemInventory::GOLEM;
            break;

        case IDC_INV_HEAD:
        case IDC_INV_NECK:
        case IDC_INV_RIGHT_ARM:
        case IDC_INV_TORSO:
        case IDC_INV_LEFT_ARM:
        case IDC_INV_GLOVES:
        case IDC_INV_RIGHT_RING:
        case IDC_INV_BELT:
        case IDC_INV_LEFT_RING:
        case IDC_INV_FEET:
        case IDC_INV_GRID:
        case IDC_INV_BELT_GRID:
        case IDC_INV_STASH_GRID:
        case IDC_INV_CUBE_GRID:
            CurrDragItemInv = d2ce::EnumItemInventory::PLAYER;
            break;

        default:
            CurrDragItemInv = d2ce::EnumItemInventory::UNKNOWN;
            break;
        }
    }
    else
    {
        // Make sure we have hit an item
        auto pt = point;
        ClientToScreen(&pt);

        CBitmap image;
        auto pItem = PlaceItem((UINT)nHit, *CurrDragItem, pt, image);
        if (pItem == nullptr || (pItem != CurrDragItem))
        {
            // placed it
            ResetCursor();

            if (pItem != nullptr)
            {
                // swap for new drag item
                CurrDragItem = const_cast<d2ce::Item*>(pItem);
                ItemCursor = CreateItemCursor(image);
                CurrCursor = ItemCursor;
                ::SetCursor(CurrCursor);
            }
        }
    }

    __super::OnLButtonDown(nFlags, point);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnMouseMove(UINT nFlags, CPoint point)
{
    if (ItemCursor != NULL)
    {
        if (CurrDragItem == nullptr)
        {
            CurrCursor = NULL;
            ::SetCursor(LoadCursor(NULL, IDC_NO));
            __super::OnMouseMove(nFlags, point);
            return;
        }

        TOOLINFO ti = { 0 };
        INT_PTR nHit = __super::OnToolHitTest(point, &ti);
        if (nHit == -1)
        {
            CurrCursor = NULL;
            ::SetCursor(AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_TRASH_CURSOR)));
            __super::OnMouseMove(nFlags, point);
            return;
        }

        switch (nHit)
        {
        case IDC_INV_GRID:
        case IDC_INV_BELT_GRID:
        case IDC_INV_STASH_GRID:
        case IDC_INV_CUBE_GRID:
        case IDC_INV_HEAD:
        case IDC_INV_CORPSE_HEAD:
        case IDC_INV_MERC_HEAD:
        case IDC_INV_GLOVES:
        case IDC_INV_CORPSE_GLOVES:
        case IDC_INV_MERC_GLOVES:
        case IDC_INV_FEET:
        case IDC_INV_CORPSE_FEET:
        case IDC_INV_MERC_FEET:
        case IDC_INV_NECK:
        case IDC_INV_CORPSE_NECK:
        case IDC_INV_MERC_NECK:
        case IDC_INV_RIGHT_RING:
        case IDC_INV_CORPSE_RIGHT_RING:
        case IDC_INV_MERC_RIGHT_RING:
        case IDC_INV_LEFT_RING:
        case IDC_INV_CORPSE_LEFT_RING:
        case IDC_INV_MERC_LEFT_RING:
        case IDC_INV_RIGHT_ARM:
        case IDC_INV_CORPSE_RIGHT_ARM:
        case IDC_INV_MERC_RIGHT_ARM:
        case IDC_INV_TORSO:
        case IDC_INV_CORPSE_TORSO:
        case IDC_INV_MERC_TORSO:
        case IDC_INV_LEFT_ARM:
        case IDC_INV_CORPSE_LEFT_ARM:
        case IDC_INV_MERC_LEFT_ARM:
        case IDC_INV_GOLEM:
        case IDC_INV_BELT:
        case IDC_INV_CORPSE_BELT:
        case IDC_INV_MERC_BELT:
            break;

        default:
            CurrCursor = NULL;
            ::SetCursor(AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_TRASH_CURSOR)));
            __super::OnMouseMove(nFlags, point);
            return;
        }

        // Make sure we have place the item
        auto pt = point;
        ClientToScreen(&pt);
        if (CanPlaceItem((UINT)nHit, *CurrDragItem, pt))
        {
            if (CurrCursor == NULL)
            {
                CurrCursor = ItemCursor;
                ::SetCursor(CurrCursor);
            }
        }
        else if (CurrCursor != NULL)
        {
            CurrCursor = NULL;
            ::SetCursor(LoadCursor(NULL, IDC_NO));
        }
    }

    __super::OnMouseMove(nFlags, point);
}
//---------------------------------------------------------------------------
BOOL CD2ItemsForm::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if(nHitTest == HTCLIENT)
    {
        if (CurrCursor != NULL)
        {
            ::SetCursor(CurrCursor);
            return TRUE;
        }

        if (ItemCursor != NULL)
        {
            ::SetCursor(LoadCursor(NULL, IDC_NO));
            return TRUE;
        }
    }

    return __super::OnSetCursor(pWnd, nHitTest, message);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextFix()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.repairItem(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextFixallitems()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = repairAllItems(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged, filter));
    msg += _T(" have been fixed");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextLoad()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemMaxQuantity(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextMaxfillstackables()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = fillAllStackables(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumStatckedItemsChangedMessage(numChanged, filter));
    msg += _T(" have been fully filled");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextMaxdurability()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemMaxDurability(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextMaxdurabilityforallitems()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = maxDurabilityAllItems(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged, filter));
    msg += _T(" have been given the highest durability value");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextIndestructible()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.setItemIndestructible(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextIndestructibleforallitems()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = setIndestructibleAllItems(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged, filter));
    msg += _T(" have been change to be indestructible");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextAddsocket()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.addItemSocket(*CurrItem))
    {
        refreshGrid(*CurrItem);
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextMaxsockets()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.setItemMaxSocketCount(*CurrItem))
    {
        refreshGrid(*CurrItem);
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextUnsocket()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.removeSocketedItems(*CurrItem))
    {
        switch (CurrItem->getLocation())
        {
        case d2ce::EnumItemLocation::EQUIPPED:
            refreshEquipped(*CurrItem);
            break;
        }

        // refresh all storage grids
        refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN);
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextMakesuperiorquality()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.makeItemSuperior(*CurrItem))
    {
        switch (CurrItem->getLocation())
        {
        case d2ce::EnumItemLocation::EQUIPPED:
            refreshEquipped(*CurrItem);
            break;
        }

        // refresh all storage grids
        refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN);
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextUpgradehighertier()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.upgradeItemTier(*CurrItem))
    {
        switch (CurrItem->getLocation())
        {
        case d2ce::EnumItemLocation::EQUIPPED:
            refreshEquipped(*CurrItem);
            break;
        }

        // refresh all storage grids
        refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN);
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextChangeEthereal()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.changeItemEthereal(*CurrItem))
    {
        switch (CurrItem->getLocation())
        {
        case d2ce::EnumItemLocation::EQUIPPED:
            refreshEquipped(*CurrItem);
            break;
        }

        // refresh all storage grids
        refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN);
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextMaxsocketsforallitems()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = maxSocketCountAllItems(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged, filter));
    msg += _T(" have been given the highest number of sockets");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextSuperiorforallitems()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = setSuperiorAllItems(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged, filter));
    msg += _T(" have been given Superior quality");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextHigherTierforallitems()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = upgradeTierAllItems(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged, filter));
    msg += _T(" have been upgraded to a higher tier");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextPersonalize()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.personalizeItem(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextRemovePersonalization()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    MainForm.removeItemPersonalization(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextApplyruneword()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    CD2RunewordForm dlg(*this);
    dlg.DoModal();
    SetFocus();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextCreateitem()
{
    CD2NewItemForm dlg(*this);
    if (dlg.DoModal() != IDOK)
    {
        return;
    }

    auto* pCreatedItem = dlg.GetCreatedItem();
    if (pCreatedItem == nullptr)
    {
        return;
    }

    CBitmap image;
    InvStashGrid.GetScaledItemBitmap(*pCreatedItem, image);

    // swap for new drag item
    ResetCursor();
    CurrDragItem = const_cast<d2ce::Item*>(pCreatedItem);
    ItemCursor = CreateItemCursor(image);
    CurrCursor = ItemCursor;
    ::SetCursor(CurrCursor);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextModifyitem()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (MainForm.getCharacterVersion() < d2ce::EnumCharVersion::v107)
    {
        CD2MagicalPropsRandomizer dlg(*this);
        if (dlg.DoModal() == IDOK)
        {
            switch (CurrItem->getLocation())
            {
            case d2ce::EnumItemLocation::EQUIPPED:
                refreshEquipped(*CurrItem);
                break;
            }

            // refresh all storage grids
            refreshGrid(d2ce::EnumItemLocation::BUFFER, d2ce::EnumAltItemLocation::UNKNOWN);
        }
    }

    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextImportitem()
{
    CFileDialog fileDialog(TRUE, _T("d2i"), NULL,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
        _T("Diablo II Item Files (*.d2i)|*.d2i|All Files (*.*)|*.*||"), this, 0, TRUE);
    const int check_id = 101;
    fileDialog.AddCheckButton(check_id, L"Randomize Item Id", TRUE);
    if (fileDialog.DoModal() != IDOK)
    {
        return;
    }

    BOOL check = TRUE;
    fileDialog.GetCheckButtonState(check_id, check);
    bool bRandomizeId = check ? true : false;

    const d2ce::Item* pImportedItem = nullptr;
    {
        CWaitCursor wait;
        if (!MainForm.importItem(fileDialog.GetPathName().GetString(), pImportedItem, bRandomizeId) || (pImportedItem == nullptr))
        {
            return;
        }
    }

    CBitmap image;
    InvStashGrid.GetScaledItemBitmap(*pImportedItem, image);

    // swap for new drag item
    ResetCursor();
    CurrDragItem = const_cast<d2ce::Item*>(pImportedItem);
    ItemCursor = CreateItemCursor(image);
    CurrCursor = ItemCursor;
    ::SetCursor(CurrCursor);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextExportitem()
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
void CD2ItemsForm::OnItemContextRemoveitem()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    const d2ce::Item* pRemovedItem = nullptr;
    setItemLocation(*CurrItem, d2ce::EnumItemLocation::BUFFER, 0, 0, d2ce::EnumItemInventory::BUFFER, pRemovedItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextUpgradeGem()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    upgradeGem(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextUpgradeGems()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = upgradeGems(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumGemsChangedMessage(numChanged, filter));
    msg += _T(" have been upgraded to perfect state");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextUpgradePotion()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    upgradePotion(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextUpgradePotions()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = upgradePotions(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumPotionsChangedMessage(numChanged, filter));
    msg += _T(" have been upgraded to their highest level");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextUpgradeRejuvenation()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    upgradeToFullRejuvenationPotion(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextUpgradeRejuvenations()
{
    auto filter(GetCurrItemFilter());
    auto numChanged = upgradeRejuvenationPotions(filter);
    ClearCurrItemInfo();

    CString msg(BuildNumPotionsChangedMessage(numChanged, filter));
    msg += _T(" have been upgraded to Full Rejuvenation potions");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextGpsConvertor()
{
    CD2GemsForm dlg(*this);
    dlg.DoModal();
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2ItemsForm::OnItemContextGpsCreator()
{
    CD2AddGemsForm dlg(*this);
    dlg.DoModal();
    ClearCurrItemInfo();
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
