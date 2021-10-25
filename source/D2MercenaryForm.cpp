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
#include "D2MercenaryForm.h"
#include "afxdialogex.h"
#include "D2MainForm.h"
#include "D2ItemToolTipCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
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
        auto rectBoxWidth = rectLocation.Width() / 2.0;

        BITMAP bmp;
        image.GetBitmap(&bmp);

        auto imageNumBoxesWide = std::max(1L, bmp.bmWidth / 30);
        auto imageNumBoxesHeight = std::max(1L, bmp.bmHeight / 30);
        CRect rect;
        rect.top = 0;
        rect.left = 0;
        rect.right = (LONG)(imageNumBoxesWide * rectBoxWidth);
        rect.bottom = (LONG)(imageNumBoxesHeight * rectBoxWidth);

        // select the source CBitmap in a memory DC;
        CDC memSrcDc;
        memSrcDc.CreateCompatibleDC(pDC);
        memSrcDc.SelectObject(&image); //now bitmap is an instance of CBitmap class

        // Create your new CBitmap with the new desired size and select it into a destination memory DC
        CDC memDestDC;
        CBitmap image2;
        memDestDC.CreateCompatibleDC(pDC);
        image2.CreateCompatibleBitmap(&memSrcDc, rect.Width(), rect.Height());
        memDestDC.SelectObject(&image2);

        // StretchBlt from src to dest
        memDestDC.StretchBlt(0, 0, rect.Width(), rect.Height(), &memSrcDc, 0, 0, bmp.bmWidth - 1, bmp.bmHeight - 1, SRCCOPY);

        HGDIOBJ hbitmap_detach = image.Detach();
        if (hbitmap_detach)
        {
            DeleteObject(hbitmap_detach);
        }

        image.Attach(image2.Detach());
    }

    void MergeImage(CDC* pDC, CBitmap& image, CBitmap& overlay)
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

        auto top = (bmp.bmHeight - bmp2.bmHeight) / 2;
        auto left = (bmp.bmWidth - bmp2.bmWidth) / 2;

        // StretchBlt from src to dest
        memDestDC.BitBlt(0, 0, bmp.bmWidth, bmp.bmHeight, &memSrcDc, 0, 0, SRCCOPY);

        // combine bmps
        memDestDC.TransparentBlt(left, top, bmp.bmWidth - left, bmp.bmHeight - top, &memSrcDc2, 0, 0, bmp2.bmWidth, bmp2.bmHeight, ::GetPixel(memSrcDc2, 0, 0));

        HGDIOBJ hbitmap_detach = image.Detach();
        if (hbitmap_detach)
        {
            DeleteObject(hbitmap_detach);
        }

        image.Attach(image2.Detach());
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
}

//---------------------------------------------------------------------------
// CD2MercenaryForm dialog

IMPLEMENT_DYNAMIC(CD2MercenaryForm, CDialogEx)

//---------------------------------------------------------------------------
CD2MercenaryForm::CD2MercenaryForm(CD2MainForm& form)
	: CDialogEx(IDD_MERC_DIALOG, (CWnd*)&form), MainForm(form), Merc(form.getMercenaryInfo())
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
    return ctrlEditted.empty() ? false : true;
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
    DDX_Control(pDX, IDC_CUR_LIFE, MercLife);
    DDX_Control(pDX, IDC_CHAR_EXPERIENCE, Experience);
    DDX_Control(pDX, IDC_INV_HEAD, InvHeadBox);
    DDX_Control(pDX, IDC_INV_HAND_RIGHT, InvHandRightBox);
    DDX_Control(pDX, IDC_INV_TORSO, InvTorsoBox);
    DDX_Control(pDX, IDC_INV_HAND_LEFT, InvHandLeftBox);
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
    ON_WM_PAINT()
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
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// D2MercenaryForm message handlers
void CD2MercenaryForm::OnPaint()
{
    CPaintDC dc(this);

    DrawItem(dc, InvHeadImage, InvHeadBox, *this);
    DrawItem(dc, InvHandRightImage, InvHandRightBox, *this);
    DrawItem(dc, InvTorsoImage, InvTorsoBox, *this);
    DrawItem(dc, InvHandLeftImage, InvHandLeftBox, *this);
}

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
    UpdateModified();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::EnableMercInfoBox()
{
    auto bEnable = Merc.isHired();
    auto bHasItem = Merc.getNumberOfItems() > 0 ? true : false;
    auto bIsBarbarian = Merc.getClass() == d2ce::EnumMercenaryClass::Barbarian ? true : false;

    MercHired.EnableWindow(!bEnable || !bHasItem);
    MercDead.EnableWindow(bEnable);
    MercName.EnableWindow(bEnable);
    MercClass.EnableWindow(bEnable && !bHasItem);
    MercLevel.EnableWindow(bEnable);
    Difficulty.EnableWindow(bEnable);
    Attribute.EnableWindow(bEnable && !bIsBarbarian);
    Attribute.ShowWindow(bIsBarbarian ? SW_HIDE : SW_SHOW);
    AttributeDash.ShowWindow(bIsBarbarian ? SW_HIDE : SW_SHOW);
    Experience.EnableWindow(bEnable);
    MercStrength.EnableWindow(bEnable);
    MercDexterity.EnableWindow(bEnable);
    MercLife.EnableWindow(bEnable);
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
        Experience.SetWindowText(_T(""));
        MercLife.SetWindowText(_T(""));
    }
    else if (bIsBarbarian)
    {
        Attribute.ResetContent();
        Attribute.SetCurSel(-1);
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

    MercName.ResetContent();
    CurMercNameClass = curClass;
    switch (CurMercNameClass)
    {
    case d2ce::EnumMercenaryClass::RogueScout:
        for (const auto& name : d2ce::RogueMercNames)
        {
            MercName.AddString(CString(name.c_str()));
        }
        break;

    case d2ce::EnumMercenaryClass::DesertMercenary:
        for (const auto& name : d2ce::DesertMercenaryNames)
        {
            MercName.AddString(CString(name.c_str()));
        }
        break;

    case d2ce::EnumMercenaryClass::IronWolf:
        for (const auto& name : d2ce::IronWolfNames)
        {
            MercName.AddString(CString(name.c_str()));
        }
        break;

    case d2ce::EnumMercenaryClass::Barbarian:
        for (const auto& name : d2ce::BarbarianMercNames)
        {
            MercName.AddString(CString(name.c_str()));
        }
        break;
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

    Attribute.ResetContent();
    CurAttributeClass = curClass;
    switch (CurAttributeClass)
    {
    case d2ce::EnumMercenaryClass::RogueScout:
        for (const auto& name : d2ce::RogueMercAttributes)
        {
            Attribute.AddString(CString(name.c_str()));
        }
        break;

    case d2ce::EnumMercenaryClass::DesertMercenary:
        for (const auto& name : d2ce::DesertMercenaryAttributes)
        {
            Attribute.AddString(CString(name.c_str()));
        }
        break;

    case d2ce::EnumMercenaryClass::IronWolf:
        for (const auto& name : d2ce::IronWolfAttributes)
        {
            Attribute.AddString(CString(name.c_str()));
        }
        break;

    case d2ce::EnumMercenaryClass::Barbarian:
        break;
    }

    if (Attribute.GetCount() <= 0)
    {
        Attribute.EnableWindow(FALSE);
        Attribute.ShowWindow(SW_HIDE);
        return;
    }

    Attribute.EnableWindow(TRUE);
    Attribute.ShowWindow(SW_SHOW);
    Attribute.SetCurSel(Merc.getAttributeId());
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::UpdateModified()
{
    d2ce::MercInfo merc;
    Merc.fillMercInfo(merc);

    if (merc.Id != OrigMerc.Id)
    {
        ctrlEditted.insert(MercHired.GetDlgCtrlID());
    }
    else
    {
        auto iter = ctrlEditted.find(MercHired.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }

    if (merc.Dead != OrigMerc.Dead)
    {
        ctrlEditted.insert(MercDead.GetDlgCtrlID());
    }
    else
    {
        auto iter = ctrlEditted.find(MercDead.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }

    if (merc.NameId != OrigMerc.NameId)
    {
        ctrlEditted.insert(MercName.GetDlgCtrlID());
    }
    else
    {
        auto iter = ctrlEditted.find(MercName.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }

    if (merc.Type != OrigMerc.Type)
    {
        ctrlEditted.insert(MercName.GetDlgCtrlID());
        ctrlEditted.insert(MercClass.GetDlgCtrlID());
        ctrlEditted.insert(Difficulty.GetDlgCtrlID());
        ctrlEditted.insert(Attribute.GetDlgCtrlID());
    }
    else
    {
        auto iter = ctrlEditted.find(MercClass.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
        
        iter = ctrlEditted.find(Difficulty.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }

        iter = ctrlEditted.find(Attribute.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }

    if (merc.Experience != OrigMerc.Experience)
    {
        ctrlEditted.insert(Experience.GetDlgCtrlID());
        ctrlEditted.insert(MercLevel.GetDlgCtrlID());
    }
    else
    {
        auto iter = ctrlEditted.find(Experience.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }

        iter = ctrlEditted.find(MercLevel.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::LoadMercItemImages()
{
    auto pToolTip = AfxGetModuleThreadState()->m_pToolTip;

    // load base images
    CDC* pDC = GetDC();
    CRect rect;
    InvHeadBox.GetClientRect(&rect);
    InvHeadImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_HELM_GLOVE_0_1)));
    ScaleImage(pDC, InvHeadImage, rect);
    pToolTip->DelTool(this, IDC_INV_HEAD);

    InvHandRightBox.GetClientRect(&rect);
    InvHandRightImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_WEAPONS)));
    ScaleImage(pDC, InvHandRightImage, rect);
    pToolTip->DelTool(this, IDC_INV_HAND_RIGHT);

    InvTorsoBox.GetClientRect(&rect);
    InvTorsoImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_ARMOR)));
    ScaleImage(pDC, InvTorsoImage, rect);
    pToolTip->DelTool(this, IDC_INV_TORSO);

    InvHandLeftBox.GetClientRect(&rect);
    InvHandLeftImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_INV_WEAPONS)));
    ScaleImage(pDC, InvHandLeftImage, rect);
    pToolTip->DelTool(this, IDC_INV_HAND_LEFT);

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

            InvHeadBox.GetClientRect(&rect);
            ScaleImage(pDC, bitmap, rect);
            MergeImage(pDC, InvHeadImage, bitmap);

            InvHeadBox.GetWindowRect(&rect);
            ScreenToClient(&rect);
            pToolTip->AddTool(this, _T("Head"), rect, IDC_INV_HEAD);
            break;

        case d2ce::EnumEquippedId::HAND_RIGHT:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvHandRightBox.GetClientRect(&rect);
            ScaleImage(pDC, bitmap, rect);
            MergeImage(pDC, InvHandRightImage, bitmap);

            InvHandRightBox.GetWindowRect(&rect);
            ScreenToClient(&rect);
            pToolTip->AddTool(this, _T("Right Hand"), rect, IDC_INV_HAND_RIGHT);
            break;

        case d2ce::EnumEquippedId::TORSO:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvTorsoBox.GetClientRect(&rect);
            ScaleImage(pDC, bitmap, rect);
            MergeImage(pDC, InvTorsoImage, bitmap);

            InvTorsoBox.GetWindowRect(&rect);
            ScreenToClient(&rect);
            pToolTip->AddTool(this, _T("Torso"), rect, IDC_INV_TORSO);
            break;

        case d2ce::EnumEquippedId::HAND_LEFT:
            if (!MainForm.getItemBitmap(item, bitmap))
            {
                continue;
            }

            InvHandLeftBox.GetClientRect(&rect);
            ScaleImage(pDC, bitmap, rect);
            MergeImage(pDC, InvHandLeftImage, bitmap);

            InvHandLeftBox.GetWindowRect(&rect);
            ScreenToClient(&rect);
            pToolTip->AddTool(this, _T("Left Hand"), rect, IDC_INV_HAND_LEFT);
            break;
        }
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
        pToolTip = new CD2ItemToolTipCtrl(MainForm.getCharacterInfo(), true, &ttParams);
        if (pToolTip->Create(this, TTS_ALWAYSTIP))
        {
            pToolTip->SetDelayTime(TTDT_AUTOPOP, 0x7FFF);
            pToolTip->SendMessage(TTM_ACTIVATE, FALSE);
            AfxGetModuleThreadState()->m_pToolTip = pToolTip;
        }
    }
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
    m_boldFont.CreateFontIndirect(&lf);

    // Set text limite on edit boxes
    MercLevel.SetLimitText(2);
    Experience.SetLimitText(10);

    DisplayMercInfo();
    LoadMercItemImages();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnEnChangeMercLevel()
{
    ctrlEditted.insert(MercLevel.GetDlgCtrlID());
}
void CD2MercenaryForm::OnEnKillfocusMercLevel()
{
    std::uint32_t level = std::min(std::max(ToInt(&MercLevel), std::uint32_t(1)), std::min(MainForm.getCharacterLevel(), d2ce::NUM_OF_LEVELS - 1));
    if (level != Merc.getLevel())
    {
        Merc.setLevel(level);
    }

    DisplayMercInfo();
}
//---------------------------------------------------------------------------
void CD2MercenaryForm::OnEnChangeMercExperience()
{
    ctrlEditted.insert(Experience.GetDlgCtrlID());
}
void CD2MercenaryForm::OnEnKillfocusMercExperience()
{
    std::uint32_t experience = std::min(std::max(ToInt(&Experience), std::uint32_t(1)), d2ce::MAX_EXPERIENCE);
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
HBRUSH CD2MercenaryForm::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    if (CTLCOLOR_EDIT == nCtlColor || CTLCOLOR_LISTBOX == nCtlColor ||
        (pWnd->IsKindOf(RUNTIME_CLASS(CEdit)) || pWnd->IsKindOf(RUNTIME_CLASS(CComboBox))))
    {
        if (ctrlEditted.find(pWnd->GetDlgCtrlID()) != ctrlEditted.end())
        {
            pDC->SelectObject(&m_boldFont);
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
    UpdateModified();
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
INT_PTR CD2MercenaryForm::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
    // check child windows first by calling CMFCBaseToolBar
    INT_PTR nHit = __super::OnToolHitTest(point, pTI);
    if (nHit != -1)
    {
        return nHit;
    }

    TOOLINFO ti;
    auto pToolTip = AfxGetModuleThreadState()->m_pToolTip;
    if (pToolTip != NULL && (pToolTip->GetOwner() == this))
    {
        if (pToolTip->HitTest((CWnd*)this, point, &ti))
        {
            return ti.uId;
        }
    }

    return nHit;
}
