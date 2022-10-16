/*
    Diablo II Character Editor
    Copyright (C) 2022 Walter Couto

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
#include "D2SharedStashForm.h"
#include "afxdialogex.h"
#include "D2MainForm.h"
#include "D2GemsForm.h"
#include "D2AddGemsForm.h"
#include "D2MercenaryForm.h"
#include "D2NewItemForm.h"
#include <deque>
#include <utf8/utf8.h>

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

    CString BuildNumItemsChangedMessage(size_t numChanged)
    {
        CString msg;
        msg.Format(_T("%zd item(s)"), numChanged);
        return msg;
    }

    CString BuildNumStatckedItemsChangedMessage(size_t numChanged)
    {
        CString msg;
        msg.Format(_T("%zd stackable item(s)"), numChanged);
        return msg;
    }

    CString BuildNumPotionsChangedMessage(size_t numChanged)
    {
        CString msg;
        msg.Format(_T("%zd potion(s)"), numChanged);
        return msg;
    }

    CString BuildNumGemsChangedMessage(size_t numChanged)
    {
        CString msg;
        msg.Format(_T("%zd gem(s)"), numChanged);
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

        if (numSlots <= 3)
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
            else // 3 slots with item being 2 slots high
            {
                CSize slotGridSize((dimension.Width * slotSize.cx) / 2, (dimension.Height * slotSize.cy) / 2);
                CSize slotGridOffset((slotGridSize.cx - slotSize.cx) / 2, (slotGridSize.cy - slotSize.cy) / 2);
                CRect slotRect;
                slotRect.top = slotGridOffset.cy;
                slotRect.bottom = slotRect.top + slotSize.cy;
                for (size_t i = 0; i < numSlots - 1; ++i)
                {
                    slotRect.left = LONG(slotGridSize.cx * i + slotGridOffset.cx);
                    slotRect.right = slotRect.left + slotSize.cx;
                    slotRectMap.push_back(slotRect);
                }

                slotRect.top = slotGridSize.cy + slotGridOffset.cy;
                slotRect.bottom = slotRect.top + slotSize.cy;
                slotRect.left = ((dimension.Width - 1) * slotSize.cx) / 2;
                slotRect.right = slotRect.left + slotSize.cx;
                slotRectMap.push_back(slotRect);
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
            for (size_t i = 0; i < numSlots; ++i)
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

    bool CalcItemRect(const d2ce::Item& item, CStatic& invBox, CRect& rect, CSize& slotSize, UINT id)
    {
        CSize slots;
        switch (id)
        {
        case IDC_INV_HEAD:
        case IDC_INV_CORPSE_HEAD:
        case IDC_INV_MERC_HEAD:
        case IDC_INV_GLOVES:
        case IDC_INV_CORPSE_GLOVES:
        case IDC_INV_FEET:
        case IDC_INV_CORPSE_FEET:
            slots = CSize(2, 2);
            break;

        case IDC_INV_NECK:
        case IDC_INV_CORPSE_NECK:
        case IDC_INV_RIGHT_RING:
        case IDC_INV_CORPSE_RIGHT_RING:
        case IDC_INV_LEFT_RING:
        case IDC_INV_CORPSE_LEFT_RING:
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
// CD2SharedStashForm dialog

IMPLEMENT_DYNAMIC(CD2SharedStashForm, CDialogEx)

//---------------------------------------------------------------------------
CD2SharedStashForm::CD2SharedStashForm(CD2MainForm& form)
    : CDialogEx(CD2SharedStashForm::IDD, (CWnd*)&form),
    MainForm(form), Stash(form.getSharedStash())
{

}
//---------------------------------------------------------------------------
CD2SharedStashForm::CD2SharedStashForm(CD2ItemsForm& form)
    : CDialogEx(CD2SharedStashForm::IDD, (CWnd*)&form),
    MainForm(form.MainForm), Stash(form.MainForm.getSharedStash())
{
}
//---------------------------------------------------------------------------
CD2SharedStashForm::~CD2SharedStashForm()
{
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_INV_STASH_GRID, InvStashGrid);
    DDX_Control(pDX, IDC_PAGE_COMBO, StashPage);
    DDX_Control(pDX, IDC_GOLD_IN_STASH, GoldInStash);
}
//---------------------------------------------------------------------------
BOOL CD2SharedStashForm::PreTranslateMessage(MSG* pMsg)
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
BEGIN_MESSAGE_MAP(CD2SharedStashForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2SharedStashForm::OnBnClickedOk)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
    ON_WM_CONTEXTMENU()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_COMMAND(ID_ITEM_CONTEXT_FIX, &CD2SharedStashForm::OnItemContextFix)
    ON_COMMAND(ID_ITEM_CONTEXT_FIXALLITEMS, &CD2SharedStashForm::OnItemContextFixallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_LOAD, &CD2SharedStashForm::OnItemContextLoad)
    ON_COMMAND(ID_ITEM_CONTEXT_FIXALLITEMS, &CD2SharedStashForm::OnItemContextMaxfillstackables)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXDURABILITY, &CD2SharedStashForm::OnItemContextMaxdurability)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXDURABILITYFORALLITEMS, &CD2SharedStashForm::OnItemContextMaxdurabilityforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_INDESTRUCTIBLE, &CD2SharedStashForm::OnItemContextIndestructible)
    ON_COMMAND(ID_ITEM_CONTEXT_INDESTRUCTIBLEFORALLITEMS, &CD2SharedStashForm::OnItemContextIndestructibleforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_ADDSOCKET, &CD2SharedStashForm::OnItemContextAddsocket)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXSOCKETS, &CD2SharedStashForm::OnItemContextMaxsockets)
    ON_COMMAND(ID_ITEM_CONTEXT_UNSOCKET, &CD2SharedStashForm::OnItemContextUnsocket)
    ON_COMMAND(ID_ITEM_CONTEXT_MAKESUPERIORQUALITY, &CD2SharedStashForm::OnItemContextMakesuperiorquality)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADEITEMTIER, &CD2SharedStashForm::OnItemContextUpgradehighertier)
    ON_COMMAND(ID_ITEM_CONTEXT_PERSONALIZE, &CD2SharedStashForm::OnItemContextPersonalize)
    ON_COMMAND(ID_ITEM_CONTEXT_REMOVE_PERSONALIZATION, &CD2SharedStashForm::OnItemContextRemovePersonalization)
    ON_COMMAND(ID_ITEM_CONTEXT_APPLY_RUNEWORD, &CD2SharedStashForm::OnItemContextApplyruneword)
    ON_COMMAND(ID_ITEM_CONTEXT_CREATE_ITEM, &CD2SharedStashForm::OnItemContextCreateitem)
    ON_COMMAND(ID_ITEM_CONTEXT_IMPORT_ITEM, &CD2SharedStashForm::OnItemContextImportitem)
    ON_COMMAND(ID_ITEM_CONTEXT_EXPORT_ITEM, &CD2SharedStashForm::OnItemContextExportitem)
    ON_COMMAND(ID_ITEM_CONTEXT_REMOVE_ITEM, &CD2SharedStashForm::OnItemContextRemoveitem)
    ON_COMMAND(ID_ITEM_CONTEXT_MAXSOCKETSFORALLITEMS, &CD2SharedStashForm::OnItemContextMaxsocketsforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_SUPERIORQUALITYALLITEMS, &CD2SharedStashForm::OnItemContextSuperiorforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_ALLITEMSMAXTIER, &CD2SharedStashForm::OnItemContextHigherTierforallitems)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_GEM, &CD2SharedStashForm::OnItemContextUpgradeGem)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_GEMS, &CD2SharedStashForm::OnItemContextUpgradeGems)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_POTION, &CD2SharedStashForm::OnItemContextUpgradePotion)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_POTIONS, &CD2SharedStashForm::OnItemContextUpgradePotions)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_REJUVENATION, &CD2SharedStashForm::OnItemContextUpgradeRejuvenation)
    ON_COMMAND(ID_ITEM_CONTEXT_UPGRADE_REJUVENATIONS, &CD2SharedStashForm::OnItemContextUpgradeRejuvenations)
    ON_COMMAND(ID_ITEM_CONTEXT_GPS_CONVERTOR, &CD2SharedStashForm::OnItemContextGpsConvertor)
    ON_COMMAND(ID_ITEM_CONTEXT_GPS_CREATOR, &CD2SharedStashForm::OnItemContextGpsCreator)
    ON_EN_KILLFOCUS(IDC_GOLD_IN_STASH, &CD2SharedStashForm::OnEnKillfocusGoldInStash)
    ON_CBN_SELCHANGE(IDC_PAGE_COMBO, &CD2SharedStashForm::OnCbnSelchangeStashPageCmb)
    ON_BN_CLICKED(IDOK, &CD2SharedStashForm::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CD2SharedStashForm::OnBnClickedCancel)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// D2ItemsForm message handlers
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void CD2SharedStashForm::LoadGridItemImages()
{
    InvStashGrid.LoadItemImages();
    SetInt(&GoldInStash, Stash.getGoldInStash(getCurrentPage()));
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::CheckToolTipCtrl()
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
size_t CD2SharedStashForm::getCurrentPage() const
{
    return size_t(ToInt(&StashPage));
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem)
{
    auto numCoverted = Stash.convertGPSs(existingGem, desiredGem);
    if (numCoverted > 0)
    {
        refreshGrid();
    }

    return numCoverted;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::convertGPSs(const std::array<std::uint8_t, 4>& existingGem, const std::array<std::uint8_t, 4>& desiredGem, size_t page)
{
    auto numCoverted = Stash.convertGPSs(existingGem, desiredGem, page);
    if (numCoverted > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }

    return numCoverted;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::updateGem(d2ce::Item& item, const std::array<std::uint8_t, 4>& newgem)
{
    if (!item.updateGem(newgem))
    {
        return false;
    }

    refreshGrid();
    return true;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::upgradeGem(d2ce::Item& item)
{
    if (!item.upgradeGem())
    {
        return false;
    }

    refreshGrid();
    return true;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::upgradeGems()
{
    auto numUpgraded = Stash.upgradeGems();
    if (numUpgraded > 0)
    {
        refreshGrid();
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::upgradeGems(size_t page)
{
    auto numUpgraded = Stash.upgradeGems(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::upgradePotion(d2ce::Item& item)
{
    if (!item.upgradePotion())
    {
        return false;
    }

    refreshGrid();
    return true;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::upgradePotions()
{
    auto numUpgraded = Stash.upgradePotions();
    if (numUpgraded > 0)
    {
        refreshGrid();
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::upgradePotions(size_t page)
{
    auto numUpgraded = Stash.upgradePotions(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::upgradeToFullRejuvenationPotion(d2ce::Item& item)
{
    if (!item.upgradeToFullRejuvenationPotion())
    {
        return false;
    }

    refreshGrid();
    return true;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::upgradeRejuvenationPotions()
{
    auto numUpgraded = Stash.upgradeRejuvenationPotions();
    if (numUpgraded > 0)
    {
        refreshGrid();
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::upgradeRejuvenationPotions(size_t page)
{
    auto numUpgraded = Stash.upgradeRejuvenationPotions(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
size_t CD2SharedStashForm::fillAllStackables()
{
    auto numUpgraded = Stash.fillAllStackables();
    if (numUpgraded > 0)
    {
        refreshGrid();
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::fillAllStackables(size_t page)
{
    auto numUpgraded = Stash.fillAllStackables(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::repairAllItems()
{
    auto numUpgraded = Stash.repairAllItems();
    if (numUpgraded > 0)
    {
        refreshGrid();
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::repairAllItems(size_t page)
{
    auto numUpgraded = Stash.repairAllItems(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::maxDurabilityAllItems()
{
    auto numUpgraded = Stash.maxDurabilityAllItems();
    if (numUpgraded > 0)
    {
        refreshGrid();
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::maxDurabilityAllItems(size_t page)
{
    auto numUpgraded = Stash.maxDurabilityAllItems(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::setIndestructibleAllItems()
{
    auto numUpgraded = Stash.setIndestructibleAllItems();
    if (numUpgraded > 0)
    {
        refreshGrid();
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::setIndestructibleAllItems(size_t page)
{
    auto numUpgraded = Stash.setIndestructibleAllItems(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::maxSocketCountAllItems()
{
    auto numUpgraded = Stash.maxSocketCountAllItems();
    if (numUpgraded > 0)
    {
        refreshGrid();
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::maxSocketCountAllItems(size_t page)
{
    auto numUpgraded = Stash.maxSocketCountAllItems(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::setSuperiorAllItems(size_t page)
{
    auto numUpgraded = Stash.setSuperiorAllItems(page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::upgradeTierAllItems(size_t page)
{
    auto numUpgraded = Stash.upgradeTierAllItems(MainForm.getCharacterInfo(), page);
    if (numUpgraded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }
    return numUpgraded;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::repairItem(d2ce::Item& item)
{
    return item.fixDurability();
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemMaxQuantity(d2ce::Item& item)
{
    return item.setMaxQuantity();
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemMaxDurability(d2ce::Item& item)
{
    return item.setMaxDurability();
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::addItemSocket(d2ce::Item& item)
{
    return item.addSocket();
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemMaxSocketCount(d2ce::Item& item)
{
    return item.addMaxSocketCount();
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::removeSocketedItems(d2ce::Item& item)
{
    return Stash.removeSocketedItems(item);
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::personalizeItem(d2ce::Item& item)
{
    return item.addPersonalization(MainForm.getCharacterName());
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::removeItemPersonalization(d2ce::Item& item)
{
    return item.removePersonalization();
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemIndestructible(d2ce::Item& item)
{
    return item.setIndestructible();
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::makeItemSuperior(d2ce::Item& item)
{
    return item.makeSuperior();
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::upgradeItemTier(d2ce::Item& item)
{
    return MainForm.upgradeItemTier(item);
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::addItem(std::array<std::uint8_t, 4>& strcode, size_t page)
{
    if (!Stash.addItem(strcode, page))
    {
        return false;
    }

    if (page == getCurrentPage())
    {
        refreshGrid();
    }
    return true;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::fillEmptySlots(std::array<std::uint8_t, 4>& strcode)
{
    auto numAdded = Stash.fillEmptySlots(strcode);
    if (numAdded > 0)
    {
        refreshGrid();
    }

    return numAdded;
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::fillEmptySlots(std::array<std::uint8_t, 4>& strcode, size_t page)
{
    auto numAdded = Stash.fillEmptySlots(strcode, page);
    if (numAdded > 0)
    {
        if (page == getCurrentPage())
        {
            refreshGrid();
        }
    }

    return numAdded;
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::refreshGrid()
{
    CWaitCursor wait;
    InvStashGrid.LoadItemImages();
    SetInt(&GoldInStash, Stash.getGoldInStash(getCurrentPage()));
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::getNumberOfItems() const
{
    return Stash.getNumberOfItems();
}
//---------------------------------------------------------------------------
size_t CD2SharedStashForm::getNumberOfItems(size_t page) const
{
    return Stash.getNumberOfItems(page);
}
//---------------------------------------------------------------------------
const std::vector<std::vector<std::reference_wrapper<d2ce::Item>>> CD2SharedStashForm::getItems() const
{
    return Stash.getItems();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2SharedStashForm::getItems(size_t page) const
{
    return Stash.getItems(page);
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::getDimensions(d2ce::ItemDimensions& dimensions) const
{
    return Stash.getDimensions(dimensions);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::SetCurrItemInfo(CPoint point)
{
    CurrItem = const_cast<d2ce::Item*>(InvHitTest(point));
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::ClearCurrItemInfo()
{
    CurrItem = nullptr;
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::ResetCursor()
{
    if (ItemCursor != NULL)
    {
        ::DestroyIcon(ItemCursor);
        ItemCursor = NULL;
        CurrCursor = NULL;
        ::SetCursor(LoadCursor(NULL, IDC_ARROW));
        CurrDragItem = nullptr;
        CurrDragPage = 0;
    }
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::GetInvBitmap(UINT id, CBitmap& image, CPoint point, TOOLINFO* pTI) const
{
    ScreenToClient(&point);

    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_STASH_GRID:
        return InvStashGrid.GetInvBitmap(image, point, pTI);
    }

    return false;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::CanPlaceItem(UINT id, const d2ce::Item& item, CPoint point)
{
    ScreenToClient(&point);

    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_STASH_GRID:
        return InvStashGrid.CanPlaceItem(item, point);
    }

    return false;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2SharedStashForm::PlaceItem(UINT id, d2ce::Item& item, CPoint point, CBitmap& bitmap)
{
    ScreenToClient(&point);

    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_STASH_GRID:
        return InvStashGrid.PlaceItem(item, point, bitmap);
    }

    return false;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemRuneword(d2ce::Item& item, std::uint16_t id)
{
    if (Stash.setItemRuneword(item, id))
    {
        refreshGrid();
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2SharedStashForm::GetInvItem(UINT id, UINT offset) const
{
    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_STASH_GRID:
        return InvStashGrid.GetInvItem(offset);
    }

    return nullptr;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2SharedStashForm::InvHitTest(UINT id, CPoint point, TOOLINFO* pTI) const
{
    ScreenToClient(&point);

    // Make sure we have hit an item
    switch (id)
    {
    case IDC_INV_STASH_GRID:
        return InvStashGrid.InvHitTest(point, pTI);
    }

    return nullptr;
}
//---------------------------------------------------------------------------
std::optional<d2ce::EnumCharClass> CD2SharedStashForm::getCharClass() const
{
    return std::optional<d2ce::EnumCharClass>();
}
//---------------------------------------------------------------------------
std::optional<d2ce::CharStats> CD2SharedStashForm::getDisplayedCharStats() const
{
    return std::optional<d2ce::CharStats>();
}
//---------------------------------------------------------------------------
std::optional<d2ce::Mercenary*> CD2SharedStashForm::getMercInfo() const
{
    return std::optional<d2ce::Mercenary*>();
}
//---------------------------------------------------------------------------
CSize CD2SharedStashForm::getInvGridSize(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId) const
{
    d2ce::ItemDimensions dimensions;
    switch (locationId)
    {
    case d2ce::EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case d2ce::EnumAltItemLocation::STASH:
            getDimensions(dimensions);
            break;
        }
    }

    return CSize(dimensions.InvWidth, dimensions.InvHeight);
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2SharedStashForm::getInvGridItems(d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId) const
{
    static std::vector<std::reference_wrapper<d2ce::Item>> s_empty;
    switch (locationId)
    {
    case d2ce::EnumItemLocation::STORED:
        switch (altPositionId)
        {
        case d2ce::EnumAltItemLocation::STASH:
            return getItems(getCurrentPage());
        }
    }

    return s_empty;
}
//---------------------------------------------------------------------------
const d2ce::Item* CD2SharedStashForm::getInvEquippedItem(d2ce::EnumEquippedId /*equippedId*/, d2ce::EnumItemInventory /*invType*/) const
{
    return nullptr;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::getItemBitmap(const d2ce::Item& item, CBitmap& bitmap) const
{
    return MainForm.getItemBitmap(item, bitmap);
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    if ((locationId != d2ce::EnumItemLocation::STORED) ||
        (altPositionId != d2ce::EnumAltItemLocation::STASH))
    {
        return false;
    }

    return setItemLocation(item, altPositionId, positionX, positionY, invType, pRemovedItem);
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemLocation(d2ce::Item& item, d2ce::EnumItemLocation locationId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory invType, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    switch (locationId)
    {
    case d2ce::EnumItemLocation::STORED:
        return setItemLocation(item, d2ce::EnumAltItemLocation::STASH, positionX, positionY, invType, pRemovedItem);

    case d2ce::EnumItemLocation::BUFFER:
        if (Stash.removeItem(item))
        {
            refreshGrid();
            return true;
        }
        return false;
    }
    return false;
    
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemLocation(d2ce::Item& item, d2ce::EnumAltItemLocation altPositionId, std::uint16_t positionX, std::uint16_t positionY, d2ce::EnumItemInventory /*invType*/, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    if (altPositionId != d2ce::EnumAltItemLocation::STASH)
    {
        return false;
    }

    auto currPage = getCurrentPage();
    auto itemPage = currPage;
    if (CurrDragItem == &item)
    {
        itemPage = CurrDragPage;
    }

    if (Stash.setItemLocation(item, itemPage, positionX, positionY, getCurrentPage(), pRemovedItem))
    {
        if (itemPage != currPage)
        {
            CurrDragPage = currPage; // would have moved pages after the above call
        }

        refreshGrid();
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool CD2SharedStashForm::setItemLocation(d2ce::Item& /*item*/, d2ce::EnumEquippedId /*equippedId*/, d2ce::EnumItemInventory /*invType*/, const d2ce::Item*& pRemovedItem)
{
    pRemovedItem = nullptr;
    return false;
}
//---------------------------------------------------------------------------
BOOL CD2SharedStashForm::OnInitDialog()
{
    __super::OnInitDialog();

    EnableToolTips(TRUE);
    CheckToolTipCtrl();

    {
        CWaitCursor wait;
        Stash.refresh();
        StashPage.ResetContent();
        if (Stash.size() > 0)
        {
            for (size_t i = 0; i < Stash.size(); ++i)
            {
                StashPage.AddString(std::to_wstring(i + 1).c_str());
            }

            StashPage.SetCurSel(0);
        }
        else
        {
            AfxMessageBox(L"Bad shared stash", MB_ICONERROR | MB_OK);
            EndDialog(IDCANCEL);
            return TRUE;
        }

        LoadGridItemImages();
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
BOOL CD2SharedStashForm::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    if (ItemCursor != NULL)
    {
        *pResult = 0;
        return FALSE;
    }

    ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    TCHAR szFullText[256];
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
        AfxLoadString((UINT)nID, szFullText);
        AfxExtractSubString(strTipText, szFullText, 1, '\n');
        switch (nID)
        {
        case IDC_INV_STASH_GRID:
            strTipText = _T("N/A");
            break;

        case IDC_GOLD_IN_STASH:
            strTipText.Format(strTipText, d2ce::GOLD_IN_STASH_LIMIT);
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
const d2ce::Item* CD2SharedStashForm::InvHitTest(CPoint point, TOOLINFO* pTI) const
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
INT_PTR CD2SharedStashForm::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
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
void CD2SharedStashForm::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CPoint hitTestPoint = point;
    ScreenToClient(&hitTestPoint);
    SetCurrItemInfo(hitTestPoint);
    if (CurrItem == nullptr)
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_ITEM_MENU));

        CMenu* pPopup = FindPopup(menu, 2);
        ENSURE(pPopup != NULL);

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

        if(!isStackable)
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
        removeQualityMenu = !canAddRareAffixes && !canAddMagicalAffixes && !canMakeSuperior;

        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_ITEM_MENU));

        CMenu* pPopup = FindPopup(menu, 3);
        ENSURE(pPopup != NULL);

        auto pos = FindPopupPosition(*pPopup, ID_ITEM_CONTEXT_ADDMAGICALAFFIXES);
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
void CD2SharedStashForm::OnLButtonDown(UINT nFlags, CPoint point)
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
    case IDC_INV_STASH_GRID:
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

        CurrDragPage = getCurrentPage();
        ItemCursor = CreateItemCursor(image);
        CurrCursor = ItemCursor;
        ::SetCursor(CurrCursor);
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
void CD2SharedStashForm::OnMouseMove(UINT nFlags, CPoint point)
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
BOOL CD2SharedStashForm::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if (nHitTest == HTCLIENT)
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
void CD2SharedStashForm::OnItemContextFix()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    repairItem(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextFixallitems()
{
    auto numChanged = repairAllItems(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged));
    msg += _T(" have been fixed");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextLoad()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    setItemMaxQuantity(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextMaxfillstackables()
{
    auto numChanged = fillAllStackables(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumStatckedItemsChangedMessage(numChanged));
    msg += _T(" have been fully filled");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextMaxdurability()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    setItemMaxDurability(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextMaxdurabilityforallitems()
{
    auto numChanged = maxDurabilityAllItems(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged));
    msg += _T(" have been given the highest durability value");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextIndestructible()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    setItemIndestructible(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextIndestructibleforallitems()
{
    auto numChanged = setIndestructibleAllItems(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged));
    msg += _T(" have been change to be indestructible");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextAddsocket()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (addItemSocket(*CurrItem))
    {
        refreshGrid();
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextMaxsockets()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (setItemMaxSocketCount(*CurrItem))
    {
        refreshGrid();
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextUnsocket()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (removeSocketedItems(*CurrItem))
    {
        refreshGrid();
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextMakesuperiorquality()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (makeItemSuperior(*CurrItem))
    {
        refreshGrid();
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextUpgradehighertier()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    if (upgradeItemTier(*CurrItem))
    {
        refreshGrid();
    }
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextMaxsocketsforallitems()
{
    auto numChanged = maxSocketCountAllItems(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged));
    msg += _T(" have been given the highest number of sockets");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextSuperiorforallitems()
{
    auto numChanged = setSuperiorAllItems(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged));
    msg += _T(" have been given Superior quality");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextHigherTierforallitems()
{
    auto numChanged = upgradeTierAllItems(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumItemsChangedMessage(numChanged));
    msg += _T(" have been upgraded to a higher tier");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextPersonalize()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    personalizeItem(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextRemovePersonalization()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    removeItemPersonalization(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextApplyruneword()
{
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextCreateitem()
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
void CD2SharedStashForm::OnItemContextImportitem()
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
        if (!Stash.importItem(fileDialog.GetPathName().GetString(), pImportedItem, bRandomizeId) || (pImportedItem == nullptr))
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
void CD2SharedStashForm::OnItemContextExportitem()
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
void CD2SharedStashForm::OnItemContextRemoveitem()
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
void CD2SharedStashForm::OnItemContextUpgradeGem()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    upgradeGem(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextUpgradeGems()
{
    auto numChanged = upgradeGems(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumGemsChangedMessage(numChanged));
    msg += _T(" have been upgraded to perfect state");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextUpgradePotion()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    upgradePotion(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextUpgradePotions()
{
    auto numChanged = upgradePotions(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumPotionsChangedMessage(numChanged));
    msg += _T(" have been upgraded to their highest level");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextUpgradeRejuvenation()
{
    if (CurrItem == nullptr)
    {
        return;
    }

    upgradeToFullRejuvenationPotion(*CurrItem);
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextUpgradeRejuvenations()
{
    auto numChanged = upgradeRejuvenationPotions(getCurrentPage());
    ClearCurrItemInfo();

    CString msg(BuildNumPotionsChangedMessage(numChanged));
    msg += _T(" have been upgraded to Full Rejuvenation potions");
    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextGpsConvertor()
{
    CD2GemsForm dlg(*this);
    dlg.DoModal();
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnItemContextGpsCreator()
{
    CD2AddGemsForm dlg(*this);
    dlg.DoModal();
    ClearCurrItemInfo();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnEnKillfocusGoldInStash()
{
    Stash.setGoldInStash(ToInt(&GoldInStash), getCurrentPage());
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnCbnSelchangeStashPageCmb()
{
    refreshGrid();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnBnClickedOk()
{
    {
        CWaitCursor wait;
        if (!Stash.save())
        {
            CStringW msg;
            msg.Format(L"Failed to savefile: %s", Stash.getPath().wstring().c_str());
            AfxMessageBox(CString(msg), MB_OK | MB_ICONERROR);
            return;
        }
        Stash.clear(true); // force items to be read later
    }

    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::OnBnClickedCancel()
{
    {
        CWaitCursor wait;
        Stash.clear(true); // force items to be read later
    }
    __super::OnCancel();
}
//---------------------------------------------------------------------------
std::string CD2SharedStashForm::ToStdString(const CWnd* Sender) const
{
    CStringW wValue(ToText(Sender));
    return utf8::utf16to8(reinterpret_cast<const char16_t*>(wValue.GetString()));
}
//---------------------------------------------------------------------------
CString CD2SharedStashForm::ToText(const CWnd* Sender) const
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
std::uint32_t CD2SharedStashForm::ToInt(const CWnd* Sender) const
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        std::string sWindowText(ToStdString(Sender));
        char* ptr = nullptr;
        return std::strtoul(sWindowText.c_str(), &ptr, 10);
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        return ((CComboBox*)Sender)->GetCurSel();
    }

    return 0;
}
//---------------------------------------------------------------------------
void CD2SharedStashForm::SetInt(CWnd* Sender, std::uint32_t newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText;
        sWindowText.Format(_T("%lu"), newValue);
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
