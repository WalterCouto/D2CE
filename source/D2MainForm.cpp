/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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
#include "framework.h"
#include "D2Editor.h"
#include "D2MainForm.h"
#include "D2SkillTreeForm.h"
#include "D2LevelInfoForm.h"
#include "D2WaypointsForm.h"
#include "D2QuestsForm.h"
#include "D2GemsForm.h"
#include "d2ce\ExperienceConstants.h"
#include "d2ce\Constants.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    void ScaleImage(CDC* pDC, CBitmap& image, const CRect& rect)
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
        image2.CreateCompatibleBitmap(&memSrcDc, rect.Width(), rect.Height());
        memDestDC.SelectObject(&image2);

        // StretchBlt from src to dest
        memDestDC.StretchBlt(0, 0, rect.Width(), rect.Height(), & memSrcDc, 0, 0, bmp.bmWidth - 1, bmp.bmHeight - 1, SRCCOPY);

        HGDIOBJ hbitmap_detach = image.Detach();
        if (hbitmap_detach)
        {
            DeleteObject(hbitmap_detach);
        }

        image.Attach(image2.Detach());
    }

    void SetScaledButtonImage(CDC* pDC, CButton& button, CBitmap& image)
    {
        CRect buttonRect;
        button.GetClientRect(&buttonRect);
        buttonRect.bottom += 4;
        buttonRect.right += 10;
        ScaleImage(pDC, image, buttonRect);
        button.SetBitmap(image);
    }

    static CD2LevelInfoForm s_levelInfo;
    CString ExtractFilePath(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        return p.remove_filename().c_str();
    }

    CString ExtractFileName(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        return p.filename().c_str();
    }

    CString ExtractFileExt(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        if (!p.has_extension())
        {
            return _T("");
        }

        return p.extension().c_str();
    }

    CString ChangeFileExt(LPCTSTR fullPath, LPCTSTR ext)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        if (ext == nullptr)
        {
            // remove the extension
            return p.replace_extension().c_str();
        }

        return p.replace_extension(ext).c_str();
    }

    CString RemoveFileExtFromPath(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return _T("");
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return _T("");
        }

        std::filesystem::path p = fullPath;
        return p.replace_extension().c_str();
    }

    bool FileExists(LPCTSTR fullPath)
    {
        if (fullPath == nullptr)
        {
            return false;
        }

        CString temp = (LPCTSTR)fullPath;
        if (temp.IsEmpty())
        {
            return false;
        }

        std::filesystem::path p = fullPath;
        return std::filesystem::exists(p);
    }
}

//---------------------------------------------------------------------------
// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    static CString GetModuleVersionInfoString(LPCTSTR lpszVariable, const std::vector<std::uint8_t>& pbData, LPCTSTR languageString)
    {
        if (pbData.empty() || lpszVariable == nullptr || languageString == nullptr)
        {
            return _T("");
        }

        CString sFieldPath(languageString);
        sFieldPath += lpszVariable;

        std::uint32_t  cbValue = 0;
        LPTSTR lpszValue = nullptr;

        VerQueryValue(&pbData[0], sFieldPath, (VOID**)&lpszValue, &cbValue);

        CString sVersion;
        if (lpszValue != nullptr)
        {
            sVersion = lpszValue;
            sVersion.Trim();
        }
        else if (sFieldPath.CompareNoCase(_T("FileVersion")) == 0)
        {
            VS_FIXEDFILEINFO* fi = NULL;
            UINT uLen = 0;
            VerQueryValue(&pbData[0], _T("\\"), (LPVOID*)&fi, &uLen);
            sVersion.Format(_T("%d.%d.%d"), HIWORD(fi->dwFileVersionMS), LOWORD(fi->dwFileVersionMS), HIWORD(fi->dwFileVersionLS));
        }
        else if (sFieldPath.CompareNoCase(_T("ProductVersion")) == 0)
        {
            VS_FIXEDFILEINFO* fi = NULL;
            UINT uLen = 0;
            VerQueryValue(&pbData[0], _T("\\"), (LPVOID*)&fi, &uLen);
            sVersion.Format(_T("%d.%d.%d"), HIWORD(fi->dwProductVersionMS), LOWORD(fi->dwProductVersionMS), HIWORD(fi->dwProductVersionLS));
        }

        return sVersion;
    }

    // Implementation
protected:
    DECLARE_MESSAGE_MAP()

protected:
    CString ProductNameAndVersion = _T("Diablo II Character Editor, Version 2.0.0");
    CString LegalCopyright = _T("Copyright (c) 2021 By Walter Couto\nCopyright (c) 2000-2003 By Burton Tsang");
};

//---------------------------------------------------------------------------
CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
    TCHAR appName[MAX_PATH];
    ::GetModuleFileName(NULL, appName, MAX_PATH);
    CString sFileName = appName;
    if (!sFileName.IsEmpty())
    {
        CString languageString;
        std::vector<std::uint8_t> pbData;

        DWORD dwHandle = 0;	// unused parameter for some APIs,must be 0

        // Get the size of the Version info block
        DWORD cbBuf = ::GetFileVersionInfoSize(sFileName, &dwHandle);

        pbData.resize(cbBuf, 0);

        // Get the file version info
        dwHandle = 0;	// unused parameter for some APIs,must be 0
        BOOL bRet = ::GetFileVersionInfo(sFileName, dwHandle, cbBuf, &pbData[0]);
        if (bRet)
        {
            DWORD dwTranslation = 0;
            DWORD* pdwTranslation = nullptr;
            UINT  cbValue = 0;

            bRet = VerQueryValue(&pbData[0], _T("\\VarFileInfo\\Translation"), (void**)&pdwTranslation, &cbValue);
            if (bRet)
            {
                dwTranslation = *pdwTranslation;
            }

            TCHAR buffer[256];
            _stprintf_s(buffer, _countof(buffer), _T("%04x%04x"), LOWORD(dwTranslation), HIWORD(dwTranslation));

            languageString = _T("\\StringFileInfo\\");
            languageString += buffer;
            languageString += _T("\\");
        }

        CString sProductName = GetModuleVersionInfoString(_T("ProductName"), pbData, languageString);;
        CString sProductVersion = GetModuleVersionInfoString(_T("ProductVersion"), pbData, languageString);
        ProductNameAndVersion.Format(_T("%s, Version %s"), (LPCTSTR)sProductName, (LPCTSTR)sProductVersion);
        LegalCopyright = GetModuleVersionInfoString(_T("LegalCopyright"), pbData, languageString);
    }
}
//---------------------------------------------------------------------------
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_ABOUT_VERSION, ProductNameAndVersion);
    DDX_Text(pDX, IDC_APP_COPYRIGHT, LegalCopyright);
}
//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CCharNameEdit
CCharNameEdit::CCharNameEdit()
{
}
//---------------------------------------------------------------------------
CCharNameEdit::~CCharNameEdit()
{
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CCharNameEdit, CEdit)
    //{{AFX_MSG_MAP(CCharNameEdit)
    ON_WM_CHAR()
    ON_WM_KILLFOCUS()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_PASTE, OnPaste)
    ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

//---------------------------------------------------------------------------
void CCharNameEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (isalpha(nChar) || nChar == '_' || nChar == '-')
    {
        if (nChar == '_' || nChar == '-')
        {
            CString sWindowText;
            GetWindowText(sWindowText);
            if (sWindowText.IsEmpty())
            {
                return;
            }

            for (int i = 1; i < sWindowText.GetLength(); i++)
            {
                if (sWindowText[i] == '_' || sWindowText[i] == '-')
                {
                    // don't bother renaming if more than 1 underscore
                    return;
                }
            }
        }
        __super::OnChar(nChar, nRepCnt, nFlags);
    }
    else if (nChar == VK_BACK)
    {
        __super::OnChar(nChar, nRepCnt, nFlags);
    }
}
//---------------------------------------------------------------------------
void CCharNameEdit::OnKillFocus(CWnd* pNewWnd)
{
    __super::OnKillFocus(pNewWnd);

    CString sWindowText;
    GetWindowText(sWindowText);
    CString strText = GetValidText(sWindowText);
    if (strText != sWindowText)
    {
        SetWindowText(strText);
    }
}
//---------------------------------------------------------------------------
// Pastes the text from the clipboard onto the current selection.
LRESULT CCharNameEdit::OnPaste(WPARAM, LPARAM)
{
    int nStart, nEnd;
    GetSel(nStart, nEnd);

    Default();

    CString sWindowText;
    GetWindowText(sWindowText);
    CString strText = GetValidText(sWindowText);
    if (strText != sWindowText)
    {
        SetWindowText(strText);
        SetSel(nStart, nEnd);
    }

    return 0;
}
//---------------------------------------------------------------------------
// Handles the WM_SETTEXT message to ensure that text (set via SetWindowText) is valid.
LRESULT CCharNameEdit::OnSetText(WPARAM /*wParam*/, LPARAM lParam)
{
    LRESULT nResult = Default();

    CString strText = GetValidText((LPCTSTR)lParam);
    if (strText != (LPCTSTR)lParam)
    {
        SetWindowText(strText);
    }

    return nResult;
}
//---------------------------------------------------------------------------
// Returns the control's value in a valid format.
CString CCharNameEdit::GetValidText(LPCTSTR value) const
{
    if (value == nullptr)
    {
        return _T("");
    }

    CString strNewText;
    CString strText(value);
    UINT maxSize = GetLimitText();

    // Remove any invalid characters from the number
    for (UINT iPos = 0, nNewLen = 0, NumberOfUnderscores = 0, nLen = strText.GetLength(); iPos < nLen && nNewLen < maxSize; ++iPos)
    {
        TCHAR c = strText[iPos];

        if (isalpha(c))
        {
            strNewText += c;
            ++nNewLen;
        }
        else if ((c == '_' || c == '-') && nNewLen != 0 && NumberOfUnderscores < 1)
        {
            strNewText += c;
            ++nNewLen;
            ++NumberOfUnderscores;
        }
    }

    return strNewText.TrimRight(_T("_-"));
}
//---------------------------------------------------------------------------
// CStatsLeftImage
CStatsLeftImage::CStatsLeftImage()
{
    m_bitmap.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_STATS_LEFT_BITMAP)));
}
//---------------------------------------------------------------------------
CStatsLeftImage::~CStatsLeftImage()
{
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CStatsLeftImage, CStatic)
    //{{AFX_MSG_MAP(CStatsLeftImage)
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CStatsLeftImage::PreSubclassWindow()
{
    __super::PreSubclassWindow();
    ModifyStyle(0, SS_OWNERDRAW);
}

void CStatsLeftImage::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}

BOOL CStatsLeftImage::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(rect);

    // If no bitmap selected, simply erase the background as per normal and return
    if (!m_bitmap.GetSafeHandle())
    {
        CBrush backBrush(::GetSysColor(COLOR_3DFACE)); // (this is meant for dialogs)
        CBrush* pOldBrush = pDC->SelectObject(&backBrush);

        pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
        pDC->SelectObject(pOldBrush);

        return TRUE;
    }

    // We have a bitmap - draw it.
    BITMAP bmp;
    m_bitmap.GetBitmap(&bmp);

    // Create compatible memory DC using the controls DC
    CDC dcMem;
    VERIFY(dcMem.CreateCompatibleDC(pDC));

    // Select bitmap into memory DC.
    CBitmap* pBmpOld = dcMem.SelectObject(&m_bitmap);

    // StretchBlt bitmap onto static's client area
    pDC->StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dcMem, 0, 0, bmp.bmWidth - 1, bmp.bmHeight - 1, SRCCOPY);

    dcMem.SelectObject(pBmpOld);

    return TRUE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// CD2MainForm dialog
IMPLEMENT_DYNAMIC(CD2MainForm, CDialogEx)
//---------------------------------------------------------------------------
CD2MainForm::CD2MainForm(WORD fontSize, CWnd* pParent /*=nullptr*/)
    : CDialogEx()
    , IsExpansionCharacter(FALSE)
    , IsHardcoreCharacter(FALSE)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_nIDTracking = 0;
    m_nIDLastMessage = 0;

    SetStartDir();

    BackupChar = AfxGetApp()->GetProfileInt(SettingsSection, BackupCharacterOption, 1) != 0 ? true : false;

    UINT nIDTemplate = IDD_D2EDITOR_DIALOG;
    m_pParentWnd = pParent;
    m_nIDHelp = nIDTemplate;

    LPCTSTR lpszTemplateName = MAKEINTRESOURCE(IDD_D2EDITOR_DIALOG);
    HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
    HRSRC hResource = ::FindResource(hInst, lpszTemplateName, RT_DIALOG);
    if (hResource == NULL)
    {
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    DWORD dwszDialogTemplate = SizeofResource(hInst, hResource);
    HGLOBAL hTemplate = LoadResource(hInst, hResource);
    LPCDLGTEMPLATE lpTemplate = static_cast<LPCDLGTEMPLATE>(::LockResource(hTemplate));
    if (lpTemplate == nullptr || dwszDialogTemplate == 0)
    {
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    // check if we have may contain font information
    const WORD* pwPtr = reinterpret_cast<const WORD*>(lpTemplate);

    // dlgVer
    size_t idx = 0;
    if (pwPtr[idx] != 1)
    {
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    // signature
    ++idx;
    if (pwPtr[idx] != 0xFFFF)
    {
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    // skip over helpID

    // skip over exStyle
    idx += 2;

    // style
    idx += 2;
    DWORD style = ((DWORD)pwPtr[idx] << 16) | pwPtr[idx + 1];

    // skip over cDlgItems
    idx += 2;

    // skip over  x
    idx += 2;

    // skip over  y
    idx += 2;

    // skip over  cx
    idx += 2;

    // skip over  cx
    idx += 2;

    // Now skip over extended information until we get to font size
    // Skip over menu and class
    for (size_t i = 0; i < 2; ++i)
    {
        if (pwPtr[idx] == 0)
        {
            // no other elements
            ++idx;
        }
        else if (pwPtr[idx] == 0xFFFF)
        {
            //Next element is menu ID
            idx += 2;
        }
        else
        {
            //Unicode ASIIZ string
            WORD z = 0;
            do
            {
                z = pwPtr[idx];
                ++idx;
            } while (z != 0);
        }
    }

    // we are at style
    if ((style & (DS_SETFONT | DS_SHELLFONT)) == 0)
    {
        // no font information
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    WORD templateFontSize = pwPtr[idx];
    if (templateFontSize == fontSize)
    {
        // Font size already correct
        m_lpszTemplateName = lpszTemplateName;
        return;
    }

    m_vecTemplate.resize(dwszDialogTemplate, 0);
    std::memcpy(&m_vecTemplate[0], lpTemplate, dwszDialogTemplate);
    m_vecTemplate[idx] = fontSize;
    m_lpDialogTemplate = (LPCDLGTEMPLATE)&m_vecTemplate[0];

    // look for any initialization data
    HRSRC hDlgInit = ::FindResource(hInst, lpszTemplateName, RT_DLGINIT);
    if (hDlgInit == NULL)
    {
        return;
    }

    // load it
    HGLOBAL hDlgInitResource = ::LoadResource(hInst, hDlgInit);
    if (hDlgInitResource == NULL)
    {
        return;
    }

    m_lpDialogInit = ::LockResource(hDlgInitResource);
}

CD2MainForm::CD2MainForm(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_D2EDITOR_DIALOG, pParent)
    , IsExpansionCharacter(FALSE)
    , IsHardcoreCharacter(FALSE)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_nIDTracking = 0;
    m_nIDLastMessage = 0;

    SetStartDir();

    BackupChar = AfxGetApp()->GetProfileInt(SettingsSection, BackupCharacterOption, 1) != 0 ? true : false;
}
//---------------------------------------------------------------------------
void CD2MainForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHAR_NAME, CharName);
    DDX_Control(pDX, IDC_HARDCORE_CHECK, CharStatusHardcore);
    DDX_Control(pDX, IDC_RESURRECTED_CHECK, CharStatusResurrected);
    DDX_Control(pDX, IDC_EXPANSION_CHECK, CharStatusExpansion);
    DDX_Control(pDX, IDC_LADDER_CHECK, CharStatusLadder);
    DDX_Control(pDX, IDC_CHAR_CLASS_CMB, CharClass);
    DDX_Control(pDX, IDC_DIFFICULTY, Difficulty);
    DDX_Control(pDX, IDC_STARTING_ACT_CMB, StartingAct);
    DDX_Control(pDX, IDC_CHAR_LEVEL, CharLevel);
    DDX_Control(pDX, IDC_CHAR_STRENGTH, CharStrength);
    DDX_Control(pDX, IDC_STRENGTH_PLUS_BTN, StrengthPlusButton);
    DDX_Control(pDX, IDC_CHAR_ENERGY, CharEnergy);
    DDX_Control(pDX, IDC_ENERGY_PLUS_BTN, EnergyPlusButton);
    DDX_Control(pDX, IDC_CHAR_DEXTERITY, CharDexterity);
    DDX_Control(pDX, IDC_DEXTERITY_PLUS_BTN, DexterityPlusButton);
    DDX_Control(pDX, IDC_CHAR_VITALITY, CharVitality);
    DDX_Control(pDX, IDC_VITALITY_PLUS_BTN, VitalityPlusButton);
    DDX_Control(pDX, IDC_CUR_LIFE, CurLife);
    DDX_Control(pDX, IDC_MAX_LIFE, MaxLife);
    DDX_Control(pDX, IDC_CUR_MANA, CurMana);
    DDX_Control(pDX, IDC_MAX_MANA, MaxMana);
    DDX_Control(pDX, IDC_CUR_STAMINA, CurStamina);
    DDX_Control(pDX, IDC_MAX_STAMINA, MaxStamina);
    DDX_Control(pDX, IDC_CHAR_EXPERIENCE, Experience);
    DDX_Control(pDX, IDC_CHAR_NEXT_EXPERIENCE, NextExperience);
    DDX_Control(pDX, IDC_STATS_LEFT, StatsLeft);
    DDX_Control(pDX, IDC_STATS_LEFT_IMAGE, StatsLeftCtrl);
    DDX_Control(pDX, IDC_GOLD_IN_BELT, GoldInBelt);
    DDX_Control(pDX, IDC_GOLD_IN_STASH, GoldInStash);
    DDX_Control(pDX, IDC_CHAR_TITLE_CMB, CharTitle);
    DDX_Control(pDX, IDC_EDIT_SKILLS_BTN, EditSkillsButton);
    DDX_Control(pDX, IDC_MAX_ALL_BTN, MaxAllButton);
    DDX_Control(pDX, IDC_QUESTS_BTN, QuestsButton);
    DDX_Control(pDX, IDC_WAYPOINTS_BTN, WaypointsButton);
    DDX_Check(pDX, IDC_EXPANSION_CHECK, IsExpansionCharacter);
    DDX_Check(pDX, IDC_HARDCORE_CHECK, IsHardcoreCharacter);
    DDX_Control(pDX, IDC_QUIT_BTN, QuitButton);
    DDX_Control(pDX, IDC_BACKGROUND_BOX, BackgroundBox);
    
}
//---------------------------------------------------------------------------
BOOL CD2MainForm::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if (pMsg->wParam == VK_RETURN)
        {
            TCHAR szClass[10];
            CWnd* pWndFocus = GetFocus();
            if (((pWndFocus = GetFocus()) != NULL) &&
                IsChild(pWndFocus) &&
                GetClassName(pWndFocus->m_hWnd, szClass, 10) &&
                (lstrcmpi(szClass, _T("EDIT")) == 0))
            {
                // pressing the ENTER key will take the focus to the next control
                pMsg->wParam = VK_TAB;
            }
        }
    }
    return __super::PreTranslateMessage(pMsg);
}

//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2MainForm, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_CTLCOLOR()
    ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
    ON_MESSAGE(WM_POPMESSAGESTRING, OnPopMessageString)
    ON_WM_MENUSELECT()
    ON_WM_MOUSEMOVE()
    ON_WM_INITMENUPOPUP()
    ON_COMMAND(ID_VIEW_SKILL_TREE, &CD2MainForm::OnViewSkillTree)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SKILL_TREE, &CD2MainForm::OnUpdateViewSkillTree)
    ON_BN_CLICKED(IDC_EDIT_SKILLS_BTN, &CD2MainForm::OnBnClickedEditSkillsBtn)
    ON_COMMAND(ID_OPTIONS_MAX_EVERYTHING, &CD2MainForm::OnOptionsMaxEverything)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_MAX_EVERYTHING, &CD2MainForm::OnUpdateOptionsMaxEverything)
    ON_BN_CLICKED(IDC_MAX_ALL_BTN, &CD2MainForm::OnBnClickedMaxAllBtn)
    ON_COMMAND(ID_VIEW_QUESTS, &CD2MainForm::OnViewQuests)
    ON_UPDATE_COMMAND_UI(ID_VIEW_QUESTS, &CD2MainForm::OnUpdateViewQuests)
    ON_BN_CLICKED(IDC_QUESTS_BTN, &CD2MainForm::OnBnClickedQuestsBtn)
    ON_COMMAND(ID_OPTIONS_UPGRADE_GEMS, &CD2MainForm::OnOptionsUpgradeGems)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_UPGRADE_GEMS, &CD2MainForm::OnUpdateOptionsUpgradeGems)
    ON_COMMAND(ID_VIEW_WAYPOINTS, &CD2MainForm::OnViewWaypoints)
    ON_UPDATE_COMMAND_UI(ID_VIEW_WAYPOINTS, &CD2MainForm::OnUpdateViewWaypoints)
    ON_BN_CLICKED(IDC_WAYPOINTS_BTN, &CD2MainForm::OnBnClickedWaypointsBtn)
    ON_EN_SETFOCUS(IDC_CHAR_NAME, &CD2MainForm::OnEnSetfocusCharName)
    ON_EN_CHANGE(IDC_CHAR_NAME, &CD2MainForm::OnEnChangeCharName)
    ON_EN_KILLFOCUS(IDC_CHAR_NAME, &CD2MainForm::OnEnKillfocusCharName)
    ON_EN_SETFOCUS(IDC_CHAR_LEVEL, &CD2MainForm::OnEnSetfocusCharLevel)
    ON_EN_CHANGE(IDC_CHAR_LEVEL, &CD2MainForm::OnEnChangeCharLevel)
    ON_EN_KILLFOCUS(IDC_CHAR_LEVEL, &CD2MainForm::OnEnKillfocusCharLevel)
    ON_EN_SETFOCUS(IDC_CHAR_STRENGTH, &CD2MainForm::OnEnSetfocusCharStrength)
    ON_EN_CHANGE(IDC_CHAR_STRENGTH, &CD2MainForm::OnEnChangeCharStrength)
    ON_EN_KILLFOCUS(IDC_CHAR_STRENGTH, &CD2MainForm::OnEnKillfocusCharStrength)
    ON_BN_CLICKED(IDC_STRENGTH_PLUS_BTN, &CD2MainForm::OnBnClickedStrengthPlus)
    ON_EN_SETFOCUS(IDC_CHAR_DEXTERITY, &CD2MainForm::OnEnSetfocusCharDexterity)
    ON_EN_CHANGE(IDC_CHAR_DEXTERITY, &CD2MainForm::OnEnChangeCharDexterity)
    ON_EN_KILLFOCUS(IDC_CHAR_DEXTERITY, &CD2MainForm::OnEnKillfocusCharDexterity)
    ON_BN_CLICKED(IDC_DEXTERITY_PLUS_BTN, &CD2MainForm::OnBnClickedDexterityPlus)
    ON_EN_SETFOCUS(IDC_CHAR_VITALITY, &CD2MainForm::OnEnSetfocusCharVitality)
    ON_EN_CHANGE(IDC_CHAR_VITALITY, &CD2MainForm::OnEnChangeCharVitality)
    ON_EN_KILLFOCUS(IDC_CHAR_VITALITY, &CD2MainForm::OnEnKillfocusCharVitality)
    ON_BN_CLICKED(IDC_VITALITY_PLUS_BTN, &CD2MainForm::OnBnClickedVitalityPlus)
    ON_EN_SETFOCUS(IDC_CHAR_ENERGY, &CD2MainForm::OnEnSetfocusCharEnergy)
    ON_EN_CHANGE(IDC_CHAR_ENERGY, &CD2MainForm::OnEnChangeCharEnergy)
    ON_EN_KILLFOCUS(IDC_CHAR_ENERGY, &CD2MainForm::OnEnKillfocusCharEnergy)
    ON_BN_CLICKED(IDC_ENERGY_PLUS_BTN, &CD2MainForm::OnBnClickedEnergyPlus)
    ON_EN_SETFOCUS(IDC_CHAR_EXPERIENCE, &CD2MainForm::OnEnSetfocusCharExperience)
    ON_EN_CHANGE(IDC_CHAR_EXPERIENCE, &CD2MainForm::OnEnChangeCharExperience)
    ON_EN_KILLFOCUS(IDC_CHAR_EXPERIENCE, &CD2MainForm::OnEnKillfocusCharExperience)
    ON_EN_SETFOCUS(IDC_CUR_LIFE, &CD2MainForm::OnEnSetfocusCurLife)
    ON_EN_CHANGE(IDC_CUR_LIFE, &CD2MainForm::OnEnChangeCurLife)
    ON_EN_KILLFOCUS(IDC_CUR_LIFE, &CD2MainForm::OnEnKillfocusCurLife)
    ON_EN_SETFOCUS(IDC_MAX_LIFE, &CD2MainForm::OnEnSetfocusMaxLife)
    ON_EN_CHANGE(IDC_MAX_LIFE, &CD2MainForm::OnEnChangeMaxLife)
    ON_EN_KILLFOCUS(IDC_MAX_LIFE, &CD2MainForm::OnEnKillfocusMaxLife)
    ON_EN_SETFOCUS(IDC_CUR_MANA, &CD2MainForm::OnEnSetfocusCurMana)
    ON_EN_CHANGE(IDC_CUR_MANA, &CD2MainForm::OnEnChangeCurMana)
    ON_EN_KILLFOCUS(IDC_CUR_MANA, &CD2MainForm::OnEnKillfocusCurMana)
    ON_EN_SETFOCUS(IDC_MAX_MANA, &CD2MainForm::OnEnSetfocusMaxMana)
    ON_EN_CHANGE(IDC_MAX_MANA, &CD2MainForm::OnEnChangeMaxMana)
    ON_EN_KILLFOCUS(IDC_MAX_MANA, &CD2MainForm::OnEnKillfocusMaxMana)
    ON_EN_SETFOCUS(IDC_CUR_STAMINA, &CD2MainForm::OnEnSetfocusCurStamina)
    ON_EN_CHANGE(IDC_CUR_STAMINA, &CD2MainForm::OnEnChangeCurStamina)
    ON_EN_KILLFOCUS(IDC_CUR_STAMINA, &CD2MainForm::OnEnKillfocusCurStamina)
    ON_EN_SETFOCUS(IDC_MAX_STAMINA, &CD2MainForm::OnEnSetfocusMaxStamina)
    ON_EN_CHANGE(IDC_MAX_STAMINA, &CD2MainForm::OnEnChangeMaxStamina)
    ON_EN_KILLFOCUS(IDC_MAX_STAMINA, &CD2MainForm::OnEnKillfocusMaxStamina)
    ON_EN_SETFOCUS(IDC_GOLD_IN_BELT, &CD2MainForm::OnEnSetfocusGoldInBelt)
    ON_EN_CHANGE(IDC_GOLD_IN_BELT, &CD2MainForm::OnEnChangeGoldInBelt)
    ON_EN_KILLFOCUS(IDC_GOLD_IN_BELT, &CD2MainForm::OnEnKillfocusGoldInBelt)
    ON_EN_SETFOCUS(IDC_GOLD_IN_STASH, &CD2MainForm::OnEnSetfocusGoldInStash)
    ON_EN_CHANGE(IDC_GOLD_IN_STASH, &CD2MainForm::OnEnChangeGoldInStash)
    ON_EN_KILLFOCUS(IDC_GOLD_IN_STASH, &CD2MainForm::OnEnKillfocusGoldInStash)
    ON_CBN_SELCHANGE(IDC_CHAR_CLASS_CMB, &CD2MainForm::OnCbnSelchangeCharClassCmb)
    ON_BN_CLICKED(IDC_HARDCORE_CHECK, &CD2MainForm::OnBnClickedHardcoreCheck)
    ON_BN_CLICKED(IDC_RESURRECTED_CHECK, &CD2MainForm::OnBnClickedResurrectedCheck)
    ON_BN_CLICKED(IDC_EXPANSION_CHECK, &CD2MainForm::OnBnClickedExpansionCheck)
    ON_BN_CLICKED(IDC_LADDER_CHECK, &CD2MainForm::OnBnClickedLadderCheck)
    ON_CBN_SELCHANGE(IDC_CHAR_TITLE_CMB, &CD2MainForm::OnCbnSelchangeCharTitleCmb)
    ON_CBN_SELCHANGE(IDC_STARTING_ACT_CMB, &CD2MainForm::OnCbnSelchangeStartingActCmb)
    ON_COMMAND(ID_FILE_SAVE, &CD2MainForm::OnFileSave)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CD2MainForm::OnUpdateFileSave)
    ON_COMMAND(ID_FILE_CLOSE, &CD2MainForm::OnFileClose)
    ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, &CD2MainForm::OnUpdateFileClose)
    ON_COMMAND(ID_FILE_OPEN, &CD2MainForm::OnFileOpen)
    ON_MESSAGE(WM_OPEN_DLG_FILE, &CD2MainForm::OnMRUFileOpen)
    ON_COMMAND(ID_VIEW_REFRESH, &CD2MainForm::OnViewRefresh)
    ON_UPDATE_COMMAND_UI(ID_VIEW_REFRESH, &CD2MainForm::OnUpdateViewRefresh)
    ON_CBN_SELCHANGE(IDC_DIFFICULTY, &CD2MainForm::OnCbnSelchangeDifficultyCmb)
    ON_BN_CLICKED(IDC_QUIT_BTN, &CD2MainForm::OnAppExit)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
    ON_COMMAND(ID_OPTIONS_BACKUP_CHAR, &CD2MainForm::OnOptionsBackupChar)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_BACKUP_CHAR, &CD2MainForm::OnUpdateOptionsBackupChar)
    ON_COMMAND(ID_OPTIONS_RESTORE_CHAR, &CD2MainForm::OnOptionsRestoreChar)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_RESTORE_CHAR, &CD2MainForm::OnUpdateOptionsRestoreChar)
    ON_COMMAND(ID_OPTIONS_CHECK_CHAR, &CD2MainForm::OnOptionsCheckChar)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_CHECK_CHAR, &CD2MainForm::OnUpdateOptionsCheckChar)
    ON_COMMAND(ID_APP_ABOUT, &CD2MainForm::OnAppAbout)
    ON_COMMAND(ID_APP_EXIT, &CD2MainForm::OnAppExit)
    ON_COMMAND(ID_VIEW_LEVEL_REQ, &CD2MainForm::OnViewLevelReq)
    ON_COMMAND(ID_OPTIONS_UPGRADE_POTIONS, &CD2MainForm::OnOptionsUpgradePotions)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_UPGRADE_POTIONS, &CD2MainForm::OnUpdateOptionsUpgradePotions)
    ON_COMMAND(ID_OPTIONS_UPGRADE_REJUVENATION, &CD2MainForm::OnOptionsUpgradeRejuvenation)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_UPGRADE_REJUVENATION, &CD2MainForm::OnUpdateOptionsUpgradeRejuvenation)
    ON_COMMAND(ID_OPTIONS_GPS_CONVERTOR, &CD2MainForm::OnOptionsGpsConvertor)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_GPS_CONVERTOR, &CD2MainForm::OnUpdateOptionsGpsConvertor)
    ON_COMMAND(ID_OPTIONS_MAXFILLSTACKABLES, &CD2MainForm::OnOptionsMaxfillstackables)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_MAXFILLSTACKABLES, &CD2MainForm::OnUpdateOptionsMaxfillstackables)
    ON_COMMAND(ID_OPTIONS_MAXDURABILITYFORALLITEMS, &CD2MainForm::OnOptionsMaxdurabilityforallitems)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_MAXDURABILITYFORALLITEMS, &CD2MainForm::OnUpdateOptionsMaxdurabilityforallitems)
        ON_COMMAND(ID_OPTIONS_RESET_STATS, &CD2MainForm::OnOptionsResetStats)
        ON_UPDATE_COMMAND_UI(ID_OPTIONS_RESET_STATS, &CD2MainForm::OnUpdateOptionsResetStats)
        END_MESSAGE_MAP()

//---------------------------------------------------------------------------
// CD2MainForm message handlers

static UINT indicators[] =
{
    ID_SEPARATOR           // status line indicator
};

//---------------------------------------------------------------------------
BOOL CD2MainForm::OnInitDialog()
{
    __super::OnInitDialog();

    EnableToolTips(TRUE);

    CString appTitle;
    if (appTitle.LoadString(AFX_IDS_APP_TITLE) != 0)
    {
        SetWindowText(appTitle);
    }

    BlackBrush.CreateSolidBrush(RGB(0, 0, 0)); // black background color

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    if (!StatusBar.Create(this) ||
        !StatusBar.SetIndicators(indicators,
            sizeof(indicators) / sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
    }
    OnSetMessageString(AFX_IDS_IDLEMESSAGE);
    

    CDC* pDC = GetDC();
    CRect rect;
    BackgroundBox.GetClientRect(&rect);
    BackgroundImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_CHAR_BITMAP)));
    ScaleImage(pDC, BackgroundImage, rect);
    SetBackgroundImage(BackgroundImage, BACKGR_TOPLEFT, TRUE, FALSE);

    QuestsImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_QUEST_BITMAP)));
    SetScaledButtonImage(pDC, QuestsButton, QuestsImage);

    EditSkillsImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_SKILL_BITMAP)));
    SetScaledButtonImage(pDC, EditSkillsButton, EditSkillsImage);

    QuitImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_EXIT_BITMAP)));
    SetScaledButtonImage(pDC, QuitButton, QuitImage);

    WaypointsImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_WAYPOINT_BITMAP)));
    SetScaledButtonImage(pDC, WaypointsButton, WaypointsImage);

    MaxAllImage.Attach(::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_STATS_BITMAP)));
    SetScaledButtonImage(pDC, MaxAllButton, MaxAllImage);
    StrengthPlusButton.SetBitmap(MaxAllImage);
    EnergyPlusButton.SetBitmap(MaxAllImage);
    DexterityPlusButton.SetBitmap(MaxAllImage);
    VitalityPlusButton.SetBitmap(MaxAllImage);

    //------------------
    // Create bold font:
    //------------------
    CFont* pWndFont = CharName.GetFont();
    LOGFONT lf;
    pWndFont->GetLogFont(&lf);
    lf.lfWeight = FW_BOLD;
    BoldFont.CreateFontIndirect(&lf);

    //This is where we actually draw it on the screen
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
        ID_SEPARATOR);

    // Set text limite on edit boxes
    CharName.SetLimitText(15);
    CharLevel.SetLimitText(2);
    CharStrength.SetLimitText(5);
    CharEnergy.SetLimitText(5);
    CharDexterity.SetLimitText(5);
    CharVitality.SetLimitText(5);
    CurLife.SetLimitText(4);
    MaxLife.SetLimitText(4);
    CurMana.SetLimitText(4);
    MaxMana.SetLimitText(4);
    CurStamina.SetLimitText(4);
    MaxStamina.SetLimitText(4);
    Experience.SetLimitText(10);
    GoldInBelt.SetLimitText(6);
    GoldInStash.SetLimitText(7);

    UpdateCharInfoBox(FALSE);
    return TRUE;  // return TRUE  unless you set the focus to a control
}
//---------------------------------------------------------------------------
void CD2MainForm::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
        return;
    }

    if ((nID & 0xFFF0) == SC_CLOSE)
    {
        if (CharInfo != nullptr && CharInfo->is_open() && DoFileCloseAction() == IDCANCEL)
        {
            return;
        }

        if (::IsWindow(s_levelInfo.GetSafeHwnd()))
        {
            s_levelInfo.DestroyWindow();
        }
    }

    __super::OnSysCommand(nID, lParam);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnAppAbout()
{
    SendMessage(WM_SYSCOMMAND, IDM_ABOUTBOX, 0);
}
//---------------------------------------------------------------------------
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CD2MainForm::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        __super::OnPaint();
    }
}
//---------------------------------------------------------------------------
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CD2MainForm::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}
//---------------------------------------------------------------------------
HBRUSH CD2MainForm::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    if (CTLCOLOR_EDIT == nCtlColor || CTLCOLOR_LISTBOX == nCtlColor ||
        (pWnd->IsKindOf(RUNTIME_CLASS(CEdit)) || pWnd->IsKindOf(RUNTIME_CLASS(CComboBox))))
    {
        pDC->SetTextColor(RGB(255, 255, 255));
        pDC->SetBkColor(RGB(0, 0, 0));
        pDC->SetBkMode(TRANSPARENT);
        if (ctrlEditted.find(pWnd->GetDlgCtrlID()) != ctrlEditted.end())
        {
            pDC->SelectObject(&BoldFont);
        }

        return (HBRUSH)BlackBrush;
    }

    switch (pWnd->GetDlgCtrlID())
    {
    case IDC_HARDCORE_CHECK:
    case IDC_RESURRECTED_CHECK:
    case IDC_EXPANSION_CHECK:
    case IDC_LADDER_CHECK:
        pDC->SetTextColor(RGB(255, 255, 255));
        pDC->SetBkColor(RGB(0, 0, 0));
        pDC->SetBkMode(TRANSPARENT);
        if (ctrlEditted.find(pWnd->GetDlgCtrlID()) != ctrlEditted.end())
        {
            pDC->SelectObject(&BoldFont);
        }

        return (HBRUSH)BlackBrush;
    }

    if (CTLCOLOR_STATIC == nCtlColor)
    {
        switch (pWnd->GetDlgCtrlID())
        {
        case IDC_HARDCORE_CHECK:
        case IDC_RESURRECTED_CHECK:
        case IDC_EXPANSION_CHECK:
        case IDC_LADDER_CHECK:
            pDC->SetBkMode(TRANSPARENT);
            return reinterpret_cast<HBRUSH>(::GetStockObject(NULL_BRUSH));
        }

        HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
        pDC->SetTextColor(RGB(255, 255, 255));
        pDC->SetBkColor(RGB(0, 0, 0));
        return hbr;
    }

    return __super::OnCtlColor(pDC, pWnd, nCtlColor);;

}
//---------------------------------------------------------------------------
// OnSetMessageString updates the status bar text.
//
// This code is based on CFrameWnd::OnSetMessageString.  We assume
// a string ID is always passed in wParam.
LRESULT CD2MainForm::OnSetMessageString(WPARAM wParam, LPARAM /*lParam*/)
{
    UINT nIDMsg = (UINT)wParam;
    CString strMsg;

    if (nIDMsg)
    {
        if (nIDMsg == AFX_IDS_IDLEMESSAGE && CharInfo == nullptr)
        {
            StatusBar.SetWindowText(_T("Click on File, Open or press CTRL-O to start"));
        }
        else if (strMsg.LoadString(nIDMsg) != 0)
        {
            CString strStatusText;
            AfxExtractSubString(strStatusText, strMsg, 0, '\n');
            switch (nIDMsg)
            {
            case IDC_GOLD_IN_BELT:
                strStatusText.Format(strStatusText, MAXGOLD);
                break;
            case IDC_GOLD_IN_STASH:
                strStatusText.Format(strStatusText, MAXSTASHGOLD);
                break;
            }
            StatusBar.SetWindowText(strStatusText);
        }
        else
        {
            if (CharInfo == nullptr)
            {
                StatusBar.SetWindowText(_T("Click on File, Open or press CTRL-O to start"));
            }
            else if (strMsg.LoadString(AFX_IDS_IDLEMESSAGE) != 0)
            {
                StatusBar.SetWindowText(strMsg);
            }
            TRACE1("Warning: no message line prompt for ID %x%04X\n", nIDMsg);
        }
    }

    UINT nIDLast = m_nIDTracking;
    m_nIDTracking = nIDMsg;
    m_nIDLastMessage = nIDMsg;
    return nIDLast;
}
//---------------------------------------------------------------------------
// Resets status bar message string.  This code is based on
// CFrameWnd::OnPopMessageString
LRESULT CD2MainForm::OnPopMessageString(WPARAM wParam, LPARAM lParam)
{
    if (m_nFlags & WF_NOPOPMSG)
        return 0;

    return SendMessage(WM_SETMESSAGESTRING, wParam, lParam);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
    __super::OnMenuSelect(nItemID, nFlags, hSysMenu);

    if (nFlags == 0xFFFF)
    {
        m_nIDTracking = AFX_IDS_IDLEMESSAGE;
        OnSetMessageString(m_nIDTracking);
        ASSERT(m_nIDTracking == m_nIDLastMessage);
    }
    else if (nItemID == 0 || nFlags & (MF_SEPARATOR | MF_POPUP | MF_MENUBREAK | MF_MENUBARBREAK))
    {
        m_nIDTracking = 0;
    }
    else if (nItemID >= 0xF000 && nItemID < 0xF1F0)
    {
        m_nIDTracking = ID_COMMAND_FROM_SC(nItemID);
        ASSERT(m_nIDTracking >= AFX_IDS_SCFIRST && m_nIDTracking < AFX_IDS_SCFIRST + 31);
    }
    else
    {
        m_nIDTracking = nItemID;
        OnSetMessageString(m_nIDTracking);
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!LastRect.IsRectEmpty() && !LastRect.PtInRect(point))
    {
        OnSetMessageString(AFX_IDS_IDLEMESSAGE);
        LastRect.SetRectEmpty();
    }

    __super::OnMouseMove(nFlags, point);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    __super::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
    if (bSysMenu)
        return;     // don't support system menu

    ASSERT(pPopupMenu != NULL);
    // Check the enabled state of various menu items.

    OnSetMessageString(AFX_IDS_IDLEMESSAGE);
    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
    {
        CWnd* pParent = this;
        // Child windows don't have menus--need to go to the top!
        if (pParent != NULL &&
            (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
        {
            int nIndexMax = ::GetMenuItemCount(hParentMenu);
            for (int nIndex2 = 0; nIndex2 < nIndexMax; nIndex2++)
            {
                if (::GetSubMenu(hParentMenu, nIndex2) == pPopupMenu->m_hMenu)
                {
                    // When popup is found, m_pParentMenu is containing menu.
                    state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
                    break;
                }
            }
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
        state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
            continue; // Menu separator or invalid cmd - ignore it.

        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
            // Possibly a popup menu, route to first item of that popup.
            state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
            if (state.m_pSubMenu == NULL ||
                (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
                state.m_nID == (UINT)-1)
            {
                continue;       // First item of popup can't be routed to.
            }
            state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
        }
        else
        {
            // Normal menu item.
            // Auto enable/disable if frame window has m_bAutoMenuEnable
            // set and command is _not_ a system command.
            state.m_pSubMenu = NULL;
            state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
            state.m_nIndex -= (state.m_nIndexMax - nCount);
            while (state.m_nIndex < nCount &&
                pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
            {
                state.m_nIndex++;
            }
        }
        state.m_nIndexMax = nCount;
    }
}
//---------------------------------------------------------------------------
BOOL CD2MainForm::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    TCHAR szFullText[256];
    CString strTipText;
    CString strPromtpText;
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
        AfxExtractSubString(strPromtpText, szFullText, 0, '\n');
        switch (nID)
        {
        case IDC_GOLD_IN_BELT:
            strTipText.Format(strTipText, MAXGOLD);
            strPromtpText.Format(strPromtpText, MAXGOLD);
            break;
        case IDC_GOLD_IN_STASH:
            strTipText.Format(strTipText, MAXSTASHGOLD);
            strPromtpText.Format(strPromtpText, MAXSTASHGOLD);
            break;
        case IDC_CHAR_STRENGTH:
            strTipText.Format(strTipText, MINSTRENGTH);
            strPromtpText.Format(strPromtpText, MINSTRENGTH);
            break;
        case IDC_CHAR_ENERGY:
            strTipText.Format(strTipText, MINENERGY);
            strPromtpText.Format(strPromtpText, MINENERGY);
            break;
        case IDC_CHAR_DEXTERITY:
            strTipText.Format(strTipText, MINDEXTERITY);
            strPromtpText.Format(strPromtpText, MINDEXTERITY);
            break;
        case IDC_CHAR_VITALITY:
            strTipText.Format(strTipText, MINVITALITY);
            strPromtpText.Format(strPromtpText, MINVITALITY);
            break;
        case IDC_MAX_LIFE:
            strTipText.Format(strTipText, MINHITPOINTS >> 8);
            strPromtpText.Format(strPromtpText, MINHITPOINTS >> 8);
            break;
        case IDC_MAX_MANA:
            strTipText.Format(strTipText, MINMANA >> 8);
            strPromtpText.Format(strPromtpText, MINMANA >> 8);
            break;
        case IDC_MAX_STAMINA:
            strTipText.Format(strTipText, MINSTAMINA >> 8);
            strPromtpText.Format(strPromtpText, MINSTAMINA >> 8);
            break;
        }

        if (!strPromtpText.IsEmpty())
        {
            StatusBar.SetWindowText(strPromtpText);
            LastRect = rect;
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
void CD2MainForm::OnOptionsCheckChar()
{
    if (CharInfo == nullptr)
    {
        return;
    }

    bool statChanged = false;
    bool bFoundIssue = false;

    // does a valid level range check
    std::uint32_t curLevel = ToInt(&CharLevel);
    if (curLevel < 1 || curLevel > d2ce::NUM_OF_LEVELS)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("\"Level\" amount exceeds the recommended maximum limit.\n")
            _T("Would you like the amount changed to the recommended maximum limit?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            curLevel = d2ce::NUM_OF_LEVELS;
            SetInt(&CharLevel, curLevel);
            ctrlEditted.insert(CharLevel.GetDlgCtrlID());
            std::uint32_t reqExpereince = d2ce::MinExpRequired[curLevel - 1];
            if ((std::uint32_t)ToInt(&Experience) < reqExpereince)
            {
                SetInt(&Experience, reqExpereince);
                ctrlEditted.insert(Experience.GetDlgCtrlID());
            }
            UpdateMaxGold();
            UpdateMinStats();
            statChanged = true;
        }
        else
        {
            // Can't proceed with an invalid level
            StatusBar.SetWindowText(_T("Character stats have been checked"));
            return;
        }
    }

    // does a level-experience check
    std::uint32_t expLevel = d2ce::NUM_OF_LEVELS;
    std::uint32_t value = (std::uint32_t)ToInt(&Experience);
    // find the correct level
    while ((expLevel > 1) && (value < d2ce::MinExpRequired[expLevel - 1]))
        --expLevel;

    if (expLevel > curLevel)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Your character's level is too low for the experience achieved.\n")
            _T("Would you like the amount changed to match your character's experience?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            std::uint32_t oldLevel = curLevel;
            curLevel = expLevel;
            SetInt(&CharLevel, curLevel);
            ctrlEditted.insert(CharLevel.GetDlgCtrlID());
            UpdateMaxGold();
            UpdateMinStats();
            CheckStatsSkillsLevel(oldLevel);
            statChanged = true;
        }
    }
    else if (expLevel < curLevel)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient experience based on current level.\n")
            _T("Would you like experience changed to match your character's level?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&Experience, d2ce::MinExpRequired[curLevel - 1]);
            ctrlEditted.insert(Experience.GetDlgCtrlID());
            statChanged = true;
        }
    }

    // does a level-gold check
    if (ToInt(&GoldInBelt) > MAXGOLD)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("\"Gold In Belt\" amount exceeds the maximum limit.\n")
            _T("Would you like the amount changed to match your character's level?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&GoldInBelt, MAXGOLD);
            ctrlEditted.insert(GoldInBelt.GetDlgCtrlID());
            statChanged = true;
        }
    }

    if (ToInt(&GoldInStash) > MAXSTASHGOLD)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("\"Gold In Stash\" amount exceeds the maximum limit.\n")
            _T("Would you like the amount changed to match your character's level?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&GoldInStash, MAXSTASHGOLD);
            ctrlEditted.insert(GoldInStash.GetDlgCtrlID());
            statChanged = true;
        }
    }

    if (ToInt(&CharVitality) < MINVITALITY)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Vitality\" based on the character's class.\n")
            _T("Would you like the amount changed to match your character's class?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&CharVitality, MINVITALITY);
            ctrlEditted.insert(CharVitality.GetDlgCtrlID());
            statChanged = true;
        }
    }

    if (ToInt(&CharEnergy) < MINENERGY)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Energy\" based for the character's class.\n")
            _T("Would you like the amount changed to match your character's class?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&CharEnergy, MINENERGY);
            ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
            statChanged = true;
        }
    }

    if (ToInt(&CharDexterity) < MINDEXTERITY)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Dexterity\" based for the character's class.\n")
            _T("Would you like the amount changed to match your character's class?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&CharEnergy, MINDEXTERITY);
            ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
            statChanged = true;
        }
    }

    if (ToInt(&CharStrength) < MINSTRENGTH)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Strength\" based for the character's class.\n")
            _T("Would you like the amount changed to match your character's class?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&CharEnergy, MINSTRENGTH);
            ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
            statChanged = true;
        }
    }

    UpdateMinStats(false);
    if (ToInt(&MaxLife) < (MINHITPOINTS & d2ce::LMS_MASK))
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Hit Points\" based on current level, class and stats.\n")
            _T("Would you like the amount changed to match your character's level, class and stats?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&MaxLife, MINHITPOINTS);
            SetInt(&CurLife, MINHITPOINTS);
            ctrlEditted.insert(MaxLife.GetDlgCtrlID());
            ctrlEditted.insert(CurLife.GetDlgCtrlID());
            statChanged = true;
        }
    }

    if (ToInt(&MaxStamina) < (MINSTAMINA & d2ce::LMS_MASK))
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Stamina\" based on current level and stats.\n")
            _T("Would you like the amount changed to match your character's level and stats?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&MaxStamina, MINSTAMINA);
            SetInt(&CurStamina, MINSTAMINA);
            ctrlEditted.insert(MaxStamina.GetDlgCtrlID());
            ctrlEditted.insert(CurStamina.GetDlgCtrlID());
            statChanged = true;
        }
    }

    if (ToInt(&MaxMana) < (MINMANA & d2ce::LMS_MASK))
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Mana\" based on current level and stats.\n")
            _T("Would you like the amount changed to match your character's level and stats?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            SetInt(&MaxMana, MINMANA);
            SetInt(&CurMana, MINMANA);
            ctrlEditted.insert(MaxMana.GetDlgCtrlID());
            ctrlEditted.insert(CurMana.GetDlgCtrlID());
            statChanged = true;
        }
    }

    // Check Stat points
    std::uint32_t curTotalStatPoints = ToInt(&CharStrength) + ToInt(&CharDexterity) + ToInt(&CharVitality) + ToInt(&CharEnergy);
    std::uint32_t earnedStatPoints = CharInfo->getStatPointsEarned(curLevel) + MINSTRENGTH + MINDEXTERITY + MINVITALITY + MINENERGY;
    if ((curTotalStatPoints >= 4 * d2ce::MAX_BASICSTATS) && (Cs.StatsLeft > 0))
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("No more stat points can be assigned but the amount left to assign is non-zero.\n")
            _T("Would you like the amount of stat points remaining to be set to zero?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            Cs.StatsLeft = 0;
            StatsLeftChanged = true;
            statChanged = true;
        }
    }

    if ((curTotalStatPoints + Cs.StatsLeft) < earnedStatPoints)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Total Stat Points\" based on current level and quests completed.\n")
            _T("Would you like the amount changed to match your character's level and quests completed?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            Cs.StatsLeft = std::min(earnedStatPoints - curTotalStatPoints, d2ce::MAX_BASICSTATS);
            StatsLeftChanged = true;
            statChanged = true;
        }
    }
    else if ((curTotalStatPoints + Cs.StatsLeft) > earnedStatPoints)
    {
        expLevel = CharInfo->getLevelFromStatPointsEarned(curTotalStatPoints - MINSTRENGTH - MINDEXTERITY - MINVITALITY - MINENERGY);
        if (expLevel > curLevel)
        {
            bFoundIssue = true;
            if (AfxMessageBox(_T("Your character's level is too low for the value of \"Total Stat Points\".\n")
                _T("Would you like the amount changed to match your character's \"Total Stat Points\"?"),
                MB_ICONQUESTION | MB_YESNO) == IDYES)
            {
                std::uint32_t oldLevel = curLevel;
                curLevel = expLevel;
                SetInt(&CharLevel, curLevel);
                ctrlEditted.insert(CharLevel.GetDlgCtrlID());
                std::uint32_t reqExpereince = d2ce::MinExpRequired[curLevel - 1];
                if ((std::uint32_t)ToInt(&Experience) < reqExpereince)
                {
                    SetInt(&Experience, reqExpereince);
                    ctrlEditted.insert(Experience.GetDlgCtrlID());
                }
                UpdateMaxGold();
                UpdateMinStats();
                CheckStatsSkillsLevel(oldLevel);
                statChanged = true;
            }
        }
    }

    // Check Skill points
    if ((Cs.SkillChoices > 0) && CharInfo->areSkillsMaxed())
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("No more skill points can be assigned but the amount left to assign is non-zero.\n")
            _T("Would you like the amount of skill points remaining to be set to zero?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            Cs.SkillChoices = 0;
            SkillChoicesChanged = true;
            statChanged = true;
        }
    }

    std::uint32_t curPoints = CharInfo->getSkillPointUsed();
    std::uint32_t earnedPoints = CharInfo->getSkillPointsEarned(curLevel);
    if ((curPoints + Cs.SkillChoices) < earnedPoints)
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Insufficient \"Total Skill Points\" based on current level and quests completed.\n")
            _T("Would you like the amount changed to match your character's level and quests completed?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            Cs.SkillChoices = std::min(earnedPoints - curPoints, d2ce::MAX_SKILL_CHOICES);
            SkillChoicesChanged = true;
            statChanged = true;
        }
    }
    else if ((curPoints + Cs.SkillChoices) > earnedPoints)
    {
        expLevel = CharInfo->getLevelFromSkillPointsEarned(curPoints);
        if (expLevel > curLevel)
        {
            bFoundIssue = true;
            if (AfxMessageBox(_T("Your character's level is too low for the value of \"Total Skill Points\".\n")
                _T("Would you like the amount changed to match your character's \"Total Skill Points\"?"),
                MB_ICONQUESTION | MB_YESNO) == IDYES)
            {
                std::uint32_t oldLevel = curLevel;
                curLevel = expLevel;
                SetInt(&CharLevel, curLevel);
                ctrlEditted.insert(CharLevel.GetDlgCtrlID());
                std::uint32_t reqExpereince = d2ce::MinExpRequired[curLevel - 1];
                if ((std::uint32_t)ToInt(&Experience) < reqExpereince)
                {
                    SetInt(&Experience, reqExpereince);
                    ctrlEditted.insert(Experience.GetDlgCtrlID());
                }
                UpdateMaxGold();
                UpdateMinStats();
                CheckStatsSkillsLevel(oldLevel);
                statChanged = true;
            }
        }
    }

    // Check Hardcore character
    bool deadStatChanged = false;
    if (CheckIsHardcoreDead(deadStatChanged))
    {
        bFoundIssue = true;
        if (deadStatChanged)
        {
            statChanged = true;
        }
    }

    if (bFoundIssue)
    {
        StatusBar.SetWindowText(_T("Character stats have been checked"));
        if (statChanged)
        {
            CheckStatsLeft();
            StatsChanged();
        }
    }
    else
    {
        CString msg(_T("No discrepancies found"));
        StatusBar.SetWindowText(msg);
        AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
    }
}

void CD2MainForm::OnUpdateOptionsCheckChar(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(TRUE);
}
//---------------------------------------------------------------------------
bool CD2MainForm::CheckIsHardcoreDead(bool& bStatChanged)
{
    bStatChanged = false;
    bool bFoundIssue = false;
    if (CharInfo == nullptr)
    {
        return bFoundIssue;
    }

    if (CharInfo->isHardcoreCharacter() && CharInfo->isDeadCharacter())
    {
        bFoundIssue = true;
        if (AfxMessageBox(_T("Your \"Hardcore\" character is dead and is no longer playable!\n")
            _T("Would you like to resurrect this character?"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            CharInfo->setIsDeadCharacter(false);
            bStatChanged = true;
            StatusChanged = true;
        }
    }

    return bFoundIssue;
}
//---------------------------------------------------------------------------
void CD2MainForm::ClearAllBoolVars()
{
    Editted = false;
    ItemsChanged = false;
    StatusChanged = false;
    StatsLeftChanged = false;
    SkillChoicesChanged = false;
    WaypointsChanged = false;
    QuestsChanged = false;
    UpdateAppTitle();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnCbnSelchangeCharClassCmb()
{
    UpdateTitleDisplay();
    UpdateMinStats();
    d2ce::BasicStats bs;
    CharInfo->fillBasicStats(bs);
    if (static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel()) == bs.Class)
    {
        auto iter = ctrlEditted.find(CharClass.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CharClass.GetDlgCtrlID());
        StatsChanged();
    }

    UpdateStartStats();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedHardcoreCheck()
{
    UpdateData(TRUE);
    UpdateTitleDisplay();

    if (NewStatusSelected())
    {
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedResurrectedCheck()
{
    if (NewStatusSelected())
    {
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedExpansionCheck()
{
    UpdateData(TRUE);
    UpdateTitleDisplay();

    bool bCharClassChanged = false;
    if (!IsExpansionCharacter)
    {
        CString curSel = ToText(&CharClass);
        if (curSel == _T("Druid") || curSel == _T("Assassin"))
        {
            if (!CharInfo->isExpansionCharacter())
            {
                // Restore original character
                CharClass.SetCurSel(static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(CharInfo->getClass()));
            }
            else
            {
                CharClass.SetCurSel(0);
            }
            bCharClassChanged = true;
        }

        // remove expansion set classes from combo box component
        auto pos = CharClass.FindStringExact(0, _T("Druid"));
        if (pos != CB_ERR)
        {
            CharClass.DeleteString(pos);
        }

        pos = CharClass.FindStringExact(0, _T("Assassin"));
        if (pos != CB_ERR)
        {
            CharClass.DeleteString(pos);
        }
    }
    else if (CharClass.GetCount() < d2ce::NUM_OF_CLASSES)
    {
        // add the expansion set characters to combo box component
        CharClass.AddString(_T("Druid"));
        CharClass.AddString(_T("Assassin"));

        if (CharInfo->isExpansionCharacter())
        {
            // Restore original character if it was an expansion character
            switch (CharInfo->getClass())
            {
            case d2ce::EnumCharClass::Druid:
            case d2ce::EnumCharClass::Assassin:
                bCharClassChanged = true;
                CharClass.SetCurSel(static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(CharInfo->getClass()));
                break;
            }
        }
    }

    if (NewStatusSelected())
    {
        if (bCharClassChanged)
        {
            OnCbnSelchangeCharClassCmb();
        }
        else
        {
            StatsChanged();
        }
    }
    else if (bCharClassChanged)
    {
        OnCbnSelchangeCharClassCmb();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedLadderCheck()
{
    if (NewStatusSelected())
    {
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnCbnSelchangeCharTitleCmb()
{
    if (NewTitleSelected())
    {
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnCbnSelchangeStartingActCmb()
{
    d2ce::BasicStats bs;
    CharInfo->fillBasicStats(bs);
    if (static_cast<d2ce::EnumAct>(StartingAct.GetCurSel()) == bs.StartingAct)
    {
        auto iter = ctrlEditted.find(StartingAct.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(StartingAct.GetDlgCtrlID());
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnCbnSelchangeDifficultyCmb()
{
    d2ce::BasicStats bs;
    CharInfo->fillBasicStats(bs);
    if (bs.DifficultyLastPlayed != static_cast<d2ce::EnumDifficulty>(Difficulty.GetCurSel()))
    {
        StatsChanged();
    }
}
//---------------------------------------------------------------------------
/*
   Fills in all the components to display the character's stats
*/
void CD2MainForm::DisplayCharInfo()
{
    IsExpansionCharacter = CharInfo->isExpansionCharacter() ? TRUE : FALSE;
    IsHardcoreCharacter = CharInfo->isHardcoreCharacter() ? TRUE : FALSE;
    CharTitle.SetCurSel(-1);
    SetupBasicStats();

    hasUpgradableRejuvenations = CharInfo->anyUpgradableRejuvenations();
    if (!hasUpgradableRejuvenations)
    {
        // implies no potions or all Full Rejuvenation potions so nothing to upgrade
        hasUpgradablePotions = false;
    }
    else
    {
        hasUpgradablePotions = CharInfo->anyUpgradablePotions();
    }

    hasUpgradableGems = CharInfo->anyUpgradableGems();

    d2ce::CharStats cs;
    CharInfo->fillCharacterStats(cs);

    SetInt(&CharLevel, cs.Level);

    SetInt(&CharStrength, cs.Strength);
    SetInt(&CharEnergy, cs.Energy);
    SetInt(&CharDexterity, cs.Dexterity);
    SetInt(&CharVitality, cs.Vitality);

    SetInt(&CurLife, cs.CurLife);
    SetInt(&MaxLife, cs.MaxLife);
    SetInt(&CurMana, cs.CurMana);
    SetInt(&MaxMana, cs.MaxMana);
    SetInt(&CurStamina, cs.CurStamina);
    SetInt(&MaxStamina, cs.MaxStamina);

    SetInt(&Experience, cs.Experience);
    SetInt(&GoldInBelt, cs.GoldInBelt);
    SetInt(&GoldInStash, cs.GoldInStash);
    UpdateMaxGold();
    UpdateStartStats();
    UpdateMinStats(false);
    CheckStatsLeft();

    EarnedStatPoints = CharInfo->getStatPointsEarned(cs.Level) + MINSTRENGTH + MINDEXTERITY + MINVITALITY + MINENERGY;
    EarnedSkillPoints = CharInfo->getSkillPointsEarned(cs.Level);

    CharStatusLadder.EnableWindow(CharInfo->getVersion() >= d2ce::EnumCharVersion::v110 ? TRUE : FALSE);
    s_levelInfo.ResetVersion(CharInfo->getVersion());
}
//---------------------------------------------------------------------------
void CD2MainForm::OnFileOpen()
{
    CString	defaultDirectory = InitialDir;
    if (!curPathName.IsEmpty())
    {
        defaultDirectory = ExtractFilePath(curPathName);
        if (defaultDirectory.IsEmpty())
        {
            defaultDirectory = InitialDir;
        }
    }

    // close any open files
    if (CharInfo != nullptr && CharInfo->is_open() && DoFileCloseAction() == IDCANCEL)
    {
        return;
    }

    CFileDialog fileDialog(TRUE, _T("d2s"), NULL,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
        _T("Diablo II Character Files (*.d2s)|*.d2s|All Files (*.*)|*.*||"), this);

    fileDialog.m_ofn.lpstrInitialDir = (LPCTSTR)defaultDirectory;
    if (fileDialog.DoModal() != IDOK)
    {
        return;
    }

    AfxGetApp()->AddToRecentFileList(fileDialog.GetPathName());

    OpenFile(fileDialog.GetPathName().GetString());
}
//---------------------------------------------------------------------------
LRESULT CD2MainForm::OnMRUFileOpen(WPARAM, LPARAM lParam)
{
    OpenFile((LPCTSTR)lParam);
    return 0;
}
//---------------------------------------------------------------------------
void CD2MainForm::OpenFile(LPCTSTR filename)
{
    // close any open files
    if (CharInfo != nullptr && CharInfo->is_open() && DoFileCloseAction() == IDCANCEL)
    {
        return;
    }

    // allocate memory for character to be loaded
    if (CharInfo == nullptr)
    {
        CharInfo.reset(new d2ce::Character());
    }

    CStringA newPathNameA(filename);

    // return if open not successful
    if (!CharInfo->open(newPathNameA), false)
    {
        CString errorMsg(CharInfo->getLastError().message().c_str());
        if (errorMsg.IsEmpty())
        {
            errorMsg = _T("Not a valid Diablo II save file.");
        }
        AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

        Editted = false;
        OnFileClose();
        return;
    }

    if (static_cast<d2ce::CharacterErrc>(CharInfo->getLastError().value()) == d2ce::CharacterErrc::InvalidChecksum)
    {
        // Checksum is invalid
        if (AfxMessageBox(_T("Character File checksum is not valid.\nDo you wish to correct it now?"), MB_ICONERROR | MB_YESNO) == IDYES)
        {
            // The checksum was updated on load, so just save the file
            CharInfo->save();
        }
    }

    curPathName = filename;
    UpdateCharInfoBox(TRUE);

    if (FileExists(ChangeFileExt(curPathName, _T(".bak"))))
    {
        RestoreActionEnabled = TRUE;
    }

    CharInfo->fillCharacterStats(Cs);
    DisplayCharInfo();
    ctrlEditted.clear();
    StatusBar.SetWindowText(_T("Character stats have been refreshed"));
    UpdateAppTitle();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnFileClose()
{
    DoFileCloseAction();
}

void CD2MainForm::OnUpdateFileClose(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(TRUE);
}
//---------------------------------------------------------------------------
int CD2MainForm::DoFileCloseAction()
{
    int ret = IDYES;
    if (Editted)
    {
        ret = AfxMessageBox(_T("Character has been modified.\nSave file?"), MB_ICONQUESTION | MB_YESNOCANCEL);
        switch (ret)
        {
        case IDYES:
            OnFileSave();
            break;
        case IDCANCEL:
            return ret;
        }
    }

    CharInfo.reset();
    s_levelInfo.ResetVersion(d2ce::EnumCharVersion::v110);

    MAXGOLD = d2ce::GOLD_IN_BELT_LIMIT;
    MAXSTASHGOLD = d2ce::GOLD_IN_STASH_LIMIT;
    MINHITPOINTS = 0x100;
    MINSTAMINA = 0x100;
    MINMANA = 0x100;
    MINVITALITY = 1;
    MINENERGY = 1;
    MINDEXTERITY = 1;
    MINSTRENGTH = 1;
    EarnedStatPoints = 0;
    EarnedSkillPoints = 0;

    curPathName.Empty();
    Initialize();

    RestoreActionEnabled = FALSE;

    UpdateCharInfoBox(FALSE);
    OnSetMessageString(AFX_IDS_IDLEMESSAGE);
    return ret;
}
//---------------------------------------------------------------------------
void CD2MainForm::OnFileSave()
{
    if (BackupChar)
        WriteBackupFile();

    d2ce::BasicStats bs;
    strcpy_s(bs.Name, 16, ToTextA(&CharName));
    bs.Status = getStatus();
    bs.Title = d2ce::EnumCharTitle::None;
    switch (CharTitle.GetCurSel())
    {
    case 1:
        bs.Title = d2ce::EnumCharTitle::SirDame;
        if (isExpansionCharacter())
        {
            bs.Title |= d2ce::EnumCharTitle::Slayer;
        }
        break;

    case 2:
        bs.Title = d2ce::EnumCharTitle::LordLady;
        if (isExpansionCharacter())
        {
            bs.Title |= d2ce::EnumCharTitle::Champion;
        }
        break;

    case 3:
        bs.Title = d2ce::EnumCharTitle::BaronBaroness;
        if (isExpansionCharacter())
        {
            bs.Title |= d2ce::EnumCharTitle::MPatriarch;
        }
        break;
    }

    bs.Class = static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel());
    bs.DifficultyLastPlayed = static_cast<d2ce::EnumDifficulty>(Difficulty.GetCurSel());
    bs.StartingAct = static_cast<d2ce::EnumAct>(StartingAct.GetCurSel());
    CharInfo->updateBasicStats(bs);

    std::uint32_t totalStatPoints = 0;
    Cs.Dexterity = ToInt(&CharDexterity);
    totalStatPoints += Cs.Dexterity;
    Cs.Energy = ToInt(&CharEnergy);
    totalStatPoints += Cs.Energy;
    Cs.Strength = ToInt(&CharStrength);
    totalStatPoints += Cs.Strength;
    Cs.Vitality = ToInt(&CharVitality);
    totalStatPoints += Cs.Vitality;
    if (totalStatPoints < 4 * d2ce::MAX_BASICSTATS)
    {
        if (totalStatPoints + Cs.StatsLeft < EarnedStatPoints)
        {
            Cs.StatsLeft = std::min(EarnedStatPoints - totalStatPoints, d2ce::MAX_BASICSTATS);
        }
    }
    else
    {
        Cs.StatsLeft = 0;
    }

    if (!CharInfo->areSkillsMaxed())
    {
        std::uint32_t totalSkillPoints = CharInfo->getSkillPointUsed();
        if (totalSkillPoints  + Cs.SkillChoices < EarnedSkillPoints)
        {
            Cs.SkillChoices = std::min(EarnedSkillPoints - totalSkillPoints, d2ce::MAX_SKILL_CHOICES);
        }
    }
    else
    {
        Cs.SkillChoices = 0;
    }

    Cs.CurLife = ToInt(&CurLife);
    Cs.MaxLife = ToInt(&MaxLife);
    Cs.CurMana = ToInt(&CurMana);
    Cs.MaxMana = ToInt(&MaxMana);
    Cs.CurStamina = ToInt(&CurStamina);
    Cs.MaxStamina = ToInt(&MaxStamina);

    Cs.Level = ToInt(&CharLevel);
    Cs.Experience = ToInt(&Experience);
    Cs.GoldInBelt = ToInt(&GoldInBelt);
    Cs.GoldInStash = ToInt(&GoldInStash);

    CharInfo->updateCharacterStats(Cs);
    if (!CharInfo->save())
    {
        CString errorMsg(CharInfo->getLastError().message().c_str());
        if (errorMsg.IsEmpty())
        {
            errorMsg = _T("Corrupted Diablo II save file discovered!");
        }

        AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

        Editted = false;
        OnFileClose();
        return;
    }

    CharInfo->fillCharacterStats(Cs);

    if (ctrlEditted.find(CharName.GetDlgCtrlID()) != ctrlEditted.end())
    {
        Editted = false;
        CharInfo->close();
        RenameCharacterFiles(ToStdString(&CharName));

        // return if open not successful
        CStringA newPathNameA(curPathName);
        if (!CharInfo->open(newPathNameA))
        {
            CString errorMsg(CharInfo->getLastError().message().c_str());
            if (errorMsg.IsEmpty())
            {
                errorMsg = _T("Not a valid Diablo II save file.");
            }
            AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

            Editted = false;
            OnFileClose();
            return;
        }
    }

    Initialize();
    CheckStatsLeft();

    CString msg(_T("Character stats saved"));
    StatusBar.SetWindowText(msg);
    AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
}

void CD2MainForm::OnUpdateFileSave(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(Editted ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnAppExit()
{
    SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
}
//---------------------------------------------------------------------------
void CD2MainForm::Initialize()
{
    ClearAllBoolVars();
    ctrlEditted.clear();
}
//---------------------------------------------------------------------------
/*
   Returns true if the user selected a new character status
*/
bool CD2MainForm::NewStatusSelected()
{
    StatusChanged = CharInfo->getStatus() != getStatus() ? true : false;
    return StatusChanged;
}
//---------------------------------------------------------------------------
bitmask::bitmask<d2ce::EnumCharStatus> CD2MainForm::getStatus()
{
    auto status = CharInfo->getStatus();
    if (CharStatusResurrected.GetCheck() == 1)
    {
        status |= d2ce::EnumCharStatus::Resurrected;
    }
    else
    {
        status &= ~d2ce::EnumCharStatus::Resurrected;
    }

    if (CharStatusHardcore.GetCheck() == 1)
    {
        status |= d2ce::EnumCharStatus::Hardcore;
        status &= ~d2ce::EnumCharStatus::Resurrected; // can't be resurrected
    }
    else
    {
        status &= ~d2ce::EnumCharStatus::Hardcore;
    }

    if (CharStatusExpansion.GetCheck() == 1)
    {
        status |= d2ce::EnumCharStatus::Expansion;
    }
    else
    {
        status &= ~d2ce::EnumCharStatus::Expansion;
    }

    if (CharStatusLadder.GetCheck() == 1)
    {
        status |= d2ce::EnumCharStatus::Ladder;
    }
    else
    {
        status &= ~d2ce::EnumCharStatus::Ladder;
    }

    return status;
}
//---------------------------------------------------------------------------
/*
   Returns true if the user selected a new character title
*/
bool CD2MainForm::NewTitleSelected()
{
    // Get previous Selections
    int prevSel = (CharInfo->getTitle().bits() & 0x0C) >> 2;

    if (CharTitle.GetCurSel() != prevSel)
    {
        ctrlEditted.insert(CharTitle.GetDlgCtrlID());
        return true;
    }

    auto iter = ctrlEditted.find(CharTitle.GetDlgCtrlID());
    if (iter != ctrlEditted.end())
    {
        ctrlEditted.erase(iter);
    }
    return false;
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewRefresh()
{
    if (Editted)
    {
        if (AfxMessageBox(_T("Character has been modified.\nAre you sure you want to refresh and lose all your changes?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
        {
            return;
        }
    }

    Initialize();

    CharInfo->refresh();
    CharInfo->fillCharacterStats(Cs);
    DisplayCharInfo();

    ctrlEditted.clear();

    StatusBar.SetWindowText(_T("Character stats have been refreshed"));
}

void CD2MainForm::OnUpdateViewRefresh(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(TRUE);
}
//---------------------------------------------------------------------------
void CD2MainForm::RenameCharacterFiles(const std::string& newNameA)
{
    CString curName = ExtractFileName(curPathName);
    CString curPath = ExtractFilePath(curPathName) + _T("\\");
    CString fullFileName = curPathName;
    CString newName(newNameA.c_str());

    // remove file extension
    CString sExt = ExtractFileExt(curName);
    if (!sExt.IsEmpty())
    {
        curName.Delete(curName.GetLength() - sExt.GetLength(), sExt.GetLength());
    }

    // do nothing if name hasn't changed
    if (curName.CompareNoCase(newName) == 0)
    {
        return;
    }

    // otherwise rename the other files that belong to this character
    curName = curPathName;
    if (!sExt.IsEmpty())
    {
        curName.Delete(curName.GetLength() - sExt.GetLength(), sExt.GetLength());
    }
    curName += ".*";

    CString existingFileName;
    CString newFileName;
    WIN32_FIND_DATA wfd;
    HANDLE hFind = ::FindFirstFile(curName, &wfd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                existingFileName = curPath + wfd.cFileName;
                sExt = ExtractFileExt(existingFileName);
                newFileName = curPath + newName + sExt;
                auto result = MoveFile(existingFileName, newFileName);
                if (result != 0)
                {
                    AfxMessageBox(_T("Problem renaming: ") + existingFileName, MB_OK | MB_ICONERROR);
                }
            }
        } while (::FindNextFile(hFind, &wfd));

        ::FindClose(hFind);
    }

    curPathName = curPath + newName + _T(".d2s");
    UpdateAppTitle();
}
//-------------------------------------------------------------------------
void CD2MainForm::OnBnClickedMaxAllBtn()
{
    OnOptionsMaxEverything();
}
void CD2MainForm::OnOptionsMaxEverything()
{
    if (CharInfo == nullptr)
    {
        return;
    }

    ctrlEditted.clear();
    SetInt(&CharLevel, d2ce::NUM_OF_LEVELS);
    ctrlEditted.insert(CharLevel.GetDlgCtrlID());

    EarnedStatPoints = MINSTRENGTH + MINDEXTERITY + MINVITALITY + MINENERGY + d2ce::MAX_STAT_POINTS;
    std::uint32_t curTotalStatPoints = ToInt(&CharStrength) + ToInt(&CharDexterity) + ToInt(&CharVitality) + ToInt(&CharEnergy);
    if ((curTotalStatPoints + Cs.StatsLeft) < EarnedStatPoints)
    {
        // we have more stats to give
        Cs.StatsLeft = std::min(EarnedStatPoints - curTotalStatPoints, d2ce::MAX_BASICSTATS);
        StatsLeftChanged = true;
    }

    SetInt(&Experience, d2ce::MAX_EXPERIENCE);
    ctrlEditted.insert(Experience.GetDlgCtrlID());
    SetText(&NextExperience, _T("NONE"));

    UpdateMaxGold();
    SetInt(&GoldInBelt, MAXGOLD);
    ctrlEditted.insert(GoldInBelt.GetDlgCtrlID());
    SetInt(&GoldInStash, MAXSTASHGOLD);
    ctrlEditted.insert(GoldInStash.GetDlgCtrlID());

    EarnedSkillPoints = d2ce::MAX_SKILL_CHOICES_EARNED;
    std::uint32_t curTotalSkillPoints = CharInfo->getSkillPointUsed();
    if ((curTotalSkillPoints + Cs.SkillChoices) < EarnedSkillPoints)
    {
        // we have more skills to aquire
        Cs.SkillChoices = std::min(EarnedSkillPoints - curTotalSkillPoints, d2ce::MAX_SKILL_CHOICES);
        SkillChoicesChanged = true;
    }

    CharInfo->upgradeGems();
    CharInfo->upgradePotions();
    CharInfo->fillAllStackables();
    CharInfo->maxDurabilityAllItems();
    ItemsChanged = true;

    UpdateMinStats();
    CheckStatsLeft();
    StatsChanged();
}
void CD2MainForm::OnUpdateOptionsMaxEverything(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(MaxAllButton.IsWindowEnabled());
}
//---------------------------------------------------------------------------
/*
   Looks in the Registry for the location of where Diablo II is installed
   and sets the Open Dialog to start in the "save" directory
*/
#define MAX_VALUE_NAME 4096
void CD2MainForm::SetStartDir()
{
    // Check to see if this is the one to use by camparing the driver's path
    HKEY regKey = 0;
    if (::RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Blizzard Entertainment\\Diablo II"), 0, KEY_QUERY_VALUE, &regKey) != ERROR_SUCCESS)
    {
        if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Blizzard Entertainment\\Diablo II"), 0, KEY_QUERY_VALUE, &regKey) != ERROR_SUCCESS)
        {
            InitialDir.Empty();
            return;
        }
    }

    std::vector<wchar_t> valueName(MAX_VALUE_NAME + 1, 0);
    DWORD valueNameLength = MAX_VALUE_NAME;
    DWORD valueType = 0;
    if ((RegQueryValueEx(regKey, _T("NewSavePath"), 0, &valueType, (LPBYTE)&valueName[0], &valueNameLength) == ERROR_SUCCESS) && (valueType == REG_SZ))
    {
        InitialDir = &valueName[0];
    }
    else if ((RegQueryValueEx(regKey, _T("Save Path"), 0, &valueType, (LPBYTE)&valueName[0], &valueNameLength) == ERROR_SUCCESS) && (valueType == REG_SZ))
    {
        InitialDir = &valueName[0];
    }

    RegCloseKey(regKey);
    regKey = 0;
}
//---------------------------------------------------------------------------
void CD2MainForm::SetupBasicStats()
{
    d2ce::BasicStats bs;
    CharInfo->fillBasicStats(bs);

    // display character stats
    SetText(&CharName, &bs.Name[0]);

    if (IsHardcoreCharacter)
    {
        if (CharInfo->isResurrectedCharacter())
        {
            // Hardcore characters can't ever have been ressurrected
            CharInfo->setIsResurrectedCharacter(false);
            StatusChanged = true;
            Editted = true;
        }

        bool deadStatChanged = false;
        if (CheckIsHardcoreDead(deadStatChanged))
        {
            Editted = true;
        }
    }

    // set state of Status checkbox
    CharStatusHardcore.SetCheck(IsHardcoreCharacter ? 1 : 0);
    CharStatusResurrected.SetCheck(CharInfo->isResurrectedCharacter() ? 1 : 0);
    CharStatusExpansion.SetCheck(IsExpansionCharacter ? 1 : 0);
    CharStatusLadder.SetCheck(CharInfo->isLadderCharacter() ? 1 : 0);

    if (!IsExpansionCharacter)
    {
        // remove expansion set classes from combo box component
        auto pos = CharClass.FindStringExact(0, _T("Druid"));
        if (pos != CB_ERR)
        {
            CharClass.DeleteString(pos);
        }

        pos = CharClass.FindStringExact(0, _T("Assassin"));
        if (pos != CB_ERR)
        {
            CharClass.DeleteString(pos);
        }
    }
    else if (CharClass.GetCount() < d2ce::NUM_OF_CLASSES)
    {
        // add the expansion set characters to combo box component
        CharClass.AddString(_T("Druid"));
        CharClass.AddString(_T("Assassin"));
    }

    CharClass.SetCurSel(static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(bs.Class));
    UpdateTitleDisplay();
    Difficulty.SetCurSel(static_cast<std::underlying_type_t<d2ce::EnumDifficulty>>(bs.DifficultyLastPlayed));

    if (!IsExpansionCharacter)
    {
        auto pos = StartingAct.FindStringExact(0, _T("V"));
        if (pos != CB_ERR)
        {
            StartingAct.DeleteString(pos);
        }
    }
    else if (StartingAct.GetCount() < d2ce::NUM_OF_ACTS)
    {
        StartingAct.AddString(_T("V"));
    }

    StartingAct.SetCurSel(static_cast <std::underlying_type_t<d2ce::EnumAct>>(bs.StartingAct));
}
//---------------------------------------------------------------------------
void CD2MainForm::StatsChanged()
{
    if (!Editted && (ItemsChanged || !ctrlEditted.empty() || StatusChanged ||
        StatsLeftChanged || SkillChoicesChanged || WaypointsChanged || QuestsChanged))
    {
        Editted = true;
        UpdateAppTitle();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::CheckStatsSkillsLevel(std::uint32_t prevLevel)
{
    std::uint32_t curLevel = std::min(ToInt(&CharLevel), d2ce::NUM_OF_LEVELS);

    // Check Stats Left 
    std::uint32_t curTotalStatPoints = ToInt(&CharStrength) + ToInt(&CharDexterity) + ToInt(&CharVitality) + ToInt(&CharEnergy);
    if ((prevLevel > curLevel) && (Cs.StatsLeft > 0))
    {
        // we went down so reduce by that amount StatsLeft and check if there is any missing
        std::uint32_t statsLeftReduced = (prevLevel - curLevel) * 5;
        if (Cs.StatsLeft > statsLeftReduced)
        {
            Cs.StatsLeft -= statsLeftReduced;
        }
        else
        {
            Cs.StatsLeft = 0;
        }
        StatsLeftChanged = true;
    }

    // Make sure we have enough Stat Points for this level
    EarnedStatPoints = CharInfo->getStatPointsEarned(curLevel) + MINSTRENGTH + MINDEXTERITY + MINVITALITY + MINENERGY;
    if (curTotalStatPoints < 4 * d2ce::MAX_BASICSTATS)
    {
        if (curTotalStatPoints + Cs.StatsLeft < EarnedStatPoints)
        {
            Cs.StatsLeft = std::min(EarnedStatPoints - curTotalStatPoints, d2ce::MAX_BASICSTATS);
            StatsLeftChanged = true;
        }
    }
    else if(Cs.StatsLeft > 0)
    {
        Cs.StatsLeft = 0;
        StatsLeftChanged = true;
    }

    // Adjust Skills Left
    std::uint32_t curPoints = CharInfo->getSkillPointUsed();
    if ((prevLevel > curLevel) && (Cs.SkillChoices > 0))
    {
        // we went down so reduce by that amount StatsLeft and check if there is any missing
        std::uint32_t skillChoicesReduced = prevLevel - curLevel;
        if (Cs.SkillChoices > skillChoicesReduced)
        {
            Cs.SkillChoices -= skillChoicesReduced;
        }
        else
        {
            Cs.SkillChoices = 0;
        }
        SkillChoicesChanged = true;
    }

    EarnedSkillPoints = CharInfo->getSkillPointsEarned(curLevel);
    if (!CharInfo->areSkillsMaxed())
    {
        if (curPoints + Cs.SkillChoices < EarnedSkillPoints)
        {
            Cs.SkillChoices = std::min(EarnedSkillPoints - curPoints, d2ce::MAX_SKILL_CHOICES);
            SkillChoicesChanged = true;
        }
    }
    else if (Cs.SkillChoices > 0)
    {
        Cs.SkillChoices = 0;
        SkillChoicesChanged = true;
    }

    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateStatsLeft()
{
    std::uint32_t statsLeft = Cs.StatsLeft;
    std::uint32_t totalStatPoints = ToInt(&CharStrength) + ToInt(&CharDexterity) + ToInt(&CharVitality) + ToInt(&CharEnergy);
    EarnedStatPoints = CharInfo->getStatPointsEarned(Cs.Level) + MINSTRENGTH + MINDEXTERITY + MINVITALITY + MINENERGY;
    if (totalStatPoints < 4 * d2ce::MAX_BASICSTATS)
    {
        if (totalStatPoints + Cs.StatsLeft < EarnedStatPoints)
        {
            statsLeft = std::min(EarnedStatPoints - totalStatPoints, d2ce::MAX_BASICSTATS);
        }
    }
    else
    {
        statsLeft = 0;
    }

    if (statsLeft != Cs.StatsLeft)
    {
        Cs.StatsLeft = statsLeft;
        StatsLeftChanged = true;
        CheckStatsLeft();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::CheckStatsLeft()
{
    if (Cs.StatsLeft > 0)
    {
        if (ToInt(&CharStrength) < d2ce::MAX_BASICSTATS)
        {
            StrengthPlusButton.ShowWindow(SW_SHOWNORMAL);
            StrengthPlusButton.EnableWindow(TRUE);
        }

        if (ToInt(&CharDexterity) < d2ce::MAX_BASICSTATS)
        {
            DexterityPlusButton.ShowWindow(SW_SHOWNORMAL);
            DexterityPlusButton.EnableWindow(TRUE);
        }

        if (ToInt(&CharVitality) < d2ce::MAX_BASICSTATS)
        {
            VitalityPlusButton.ShowWindow(SW_SHOWNORMAL);
            VitalityPlusButton.EnableWindow(TRUE);
        }

        if (ToInt(&CharEnergy) < d2ce::MAX_BASICSTATS)
        {
            EnergyPlusButton.ShowWindow(SW_SHOWNORMAL);
            EnergyPlusButton.EnableWindow(TRUE);
        }

        StatsLeft.ShowWindow(SW_SHOWNORMAL);
        StatsLeftCtrl.ShowWindow(SW_SHOWNORMAL);
        SetInt(&StatsLeft, Cs.StatsLeft);
    }
    else
    {
        StrengthPlusButton.EnableWindow(FALSE);
        StrengthPlusButton.ShowWindow(SW_HIDE);
        DexterityPlusButton.EnableWindow(FALSE);
        DexterityPlusButton.ShowWindow(SW_HIDE);
        VitalityPlusButton.EnableWindow(FALSE);
        VitalityPlusButton.ShowWindow(SW_HIDE);
        VitalityPlusButton.EnableWindow(FALSE);
        EnergyPlusButton.ShowWindow(SW_HIDE);

        StatsLeft.SetWindowText(_T(""));
        StatsLeft.ShowWindow(SW_HIDE);
        StatsLeftCtrl.ShowWindow(SW_HIDE);
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateMaxGold()
{
    std::uint32_t curLevel = std::min(ToInt(&CharLevel), d2ce::NUM_OF_LEVELS);
    MAXGOLD = curLevel * 10000;

    if (CharInfo->getVersion() >= d2ce::EnumCharVersion::v110) // 1.10+ character
    {
        MAXSTASHGOLD = d2ce::GOLD_IN_STASH_LIMIT;
    }
    else if (curLevel < 31) // 1.00 - 1.09 character
    {
        MAXSTASHGOLD = (curLevel / 10 + 1) * 50000;
    }
    else if (CharInfo->getVersion() >= d2ce::EnumCharVersion::v107) // 1.07 - 1.09 character
    {
        MAXSTASHGOLD = (curLevel / 2 + 1) * 50000;
    }
    else // pre 1.07 character
    {
        if (curLevel < 90)
        {
            MAXSTASHGOLD = (curLevel / 10 + 1) * 50000;
        }
        else
        {
            MAXSTASHGOLD = 2000000;
        }
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateStartStats()
{
    switch (static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel()))
    {
    case d2ce::EnumCharClass::Amazon:
        MINVITALITY = d2ce::AMAZON_VITALITY_MIN;
        MINENERGY = d2ce::AMAZON_ENERGY_MIN;
        MINDEXTERITY = d2ce::AMAZON_DEXTERITY_MIN;
        MINSTRENGTH = d2ce::AMAZON_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Assassin:
        MINVITALITY = d2ce::ASSASSIN_VITALITY_MIN;
        MINENERGY = d2ce::ASSASSIN_ENERGY_MIN;
        MINDEXTERITY = d2ce::ASSASSIN_DEXTERITY_MIN;
        MINSTRENGTH = d2ce::ASSASSIN_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Sorceress:
        MINVITALITY = d2ce::SORCERESS_VITALITY_MIN;
        MINENERGY = d2ce::SORCERESS_ENERGY_MIN;
        MINDEXTERITY = d2ce::SORCERESS_DEXTERITY_MIN;
        MINSTRENGTH = d2ce::SORCERESS_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Barbarian:
        MINVITALITY = d2ce::BARBARIAN_VITALITY_MIN;
        MINENERGY = d2ce::BARBARIAN_ENERGY_MIN;
        MINDEXTERITY = d2ce::BARBARIAN_DEXTERITY_MIN;
        MINSTRENGTH = d2ce::BARBARIAN_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Druid:
        MINVITALITY = d2ce::DRUID_VITALITY_MIN;
        MINENERGY = d2ce::DRUID_ENERGY_MIN;
        MINDEXTERITY = d2ce::DRUID_DEXTERITY_MIN;
        MINSTRENGTH = d2ce::DRUID_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Necromancer:
        MINVITALITY = d2ce::NECROMANCER_VITALITY_MIN;
        MINENERGY = d2ce::NECROMANCER_ENERGY_MIN;
        MINDEXTERITY = d2ce::NECROMANCER_DEXTERITY_MIN;
        MINSTRENGTH = d2ce::NECROMANCER_STRENGTH_MIN;
        break;

    case d2ce::EnumCharClass::Paladin:
        MINVITALITY = d2ce::PALADIN_VITALITY_MIN;
        MINENERGY = d2ce::PALADIN_ENERGY_MIN;
        MINDEXTERITY = d2ce::PALADIN_DEXTERITY_MIN;
        MINSTRENGTH = d2ce::PALADIN_STRENGTH_MIN;
        break;
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateMinStats(bool applyChanges)
{
    std::uint32_t curLevel = std::min(ToInt(&CharLevel), d2ce::NUM_OF_LEVELS);
    std::uint32_t curVitality = std::min(ToInt(&CharVitality), d2ce::MAX_BASICSTATS);
    std::uint32_t curEnergy = std::min(ToInt(&CharEnergy), d2ce::MAX_BASICSTATS);
    switch (static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel()))
    {
    case d2ce::EnumCharClass::Amazon:
        curVitality = std::max(curVitality, d2ce::AMAZON_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::AMAZON_ENERGY_MIN); // start stat
        MINHITPOINTS = 0x3200 + ((curLevel - 1) << 8) * 2 + (((curVitality - d2ce::AMAZON_VITALITY_MIN) << 8) * 3);
        MINSTAMINA = 0x5400 + ((curLevel + curVitality - d2ce::AMAZON_VITALITY_MIN - 1) << 8);
        MINMANA = 0x0F00 + std::uint32_t(((curLevel + curEnergy - d2ce::AMAZON_ENERGY_MIN - 1) << 8) * 1.5);
        break;

    case d2ce::EnumCharClass::Assassin:
        curVitality = std::max(curVitality, d2ce::ASSASSIN_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::ASSASSIN_ENERGY_MIN); // start stat
        MINHITPOINTS = 0x5400 + ((curLevel - 1) << 8) * 2 + (((curVitality - d2ce::ASSASSIN_VITALITY_MIN) << 8) * 3);
        MINSTAMINA = 0x5F00 + ((curLevel - 1) << 8) + std::uint32_t((((curVitality - d2ce::ASSASSIN_VITALITY_MIN) << 8) * 1.25));
        MINMANA = 0x1900 + std::uint32_t(((curLevel - 1) << 8) * 1.5) + std::uint32_t((((curEnergy - d2ce::ASSASSIN_ENERGY_MIN) << 8) * 1.75));
        break;

    case d2ce::EnumCharClass::Sorceress:
        curVitality = std::max(curVitality, d2ce::SORCERESS_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::SORCERESS_ENERGY_MIN); // start stat
        MINHITPOINTS = 0x5400 + ((curLevel - 1) << 8) + (((curVitality - d2ce::SORCERESS_VITALITY_MIN) << 8) * 2);
        MINSTAMINA = 0x4A00 + ((curLevel + curVitality - d2ce::SORCERESS_VITALITY_MIN - 1) << 8);
        MINMANA = 0x2300 + ((curLevel + curEnergy - d2ce::SORCERESS_ENERGY_MIN - 1) << 8) * 2;
        break;

    case d2ce::EnumCharClass::Barbarian:
        curVitality = std::max(curVitality, d2ce::BARBARIAN_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::BARBARIAN_ENERGY_MIN); // start stat
        MINHITPOINTS = 0x3700 + ((curLevel - 1) << 8) * 2 + (((curVitality - d2ce::BARBARIAN_VITALITY_MIN) << 8) * 4);
        MINSTAMINA = 0x5C00 + ((curLevel + curVitality - d2ce::BARBARIAN_VITALITY_MIN - 1) << 8);
        MINMANA = 0x0A00 + ((curLevel + curEnergy - d2ce::BARBARIAN_ENERGY_MIN - 1) << 8);
        break;

    case d2ce::EnumCharClass::Druid:
        curVitality = std::max(curVitality, d2ce::DRUID_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::DRUID_ENERGY_MIN); // start stat
        MINHITPOINTS = 0x3700 + std::uint32_t((((curLevel - 1) << 8) * 1.5)) + (((curVitality - d2ce::DRUID_VITALITY_MIN) << 8) * 2);
        MINSTAMINA = 0x5400 + ((curLevel + curVitality - d2ce::DRUID_VITALITY_MIN - 1) << 8);
        MINMANA = 0x0A00 + ((curLevel - 1) << 8) + (((curEnergy - d2ce::DRUID_ENERGY_MIN) << 8) * 2);
        break;

    case d2ce::EnumCharClass::Necromancer:
        curVitality = std::max(curVitality, d2ce::NECROMANCER_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::NECROMANCER_ENERGY_MIN); // start stat
        MINHITPOINTS = 0x2D00 + std::uint32_t((((curLevel - 1) << 8) * 1.5)) + (((curVitality - d2ce::NECROMANCER_VITALITY_MIN) << 8) * 2);
        MINSTAMINA = 0x4F00 + ((curLevel + curVitality - d2ce::NECROMANCER_VITALITY_MIN - 1) << 8);
        MINMANA = 0x1900 + ((curLevel + curEnergy - d2ce::NECROMANCER_ENERGY_MIN - 1) << 8) * 2;
        break;

    case d2ce::EnumCharClass::Paladin:
        curVitality = std::max(curVitality, d2ce::PALADIN_VITALITY_MIN); // start stat
        curEnergy = std::max(curEnergy, d2ce::PALADIN_ENERGY_MIN); // start stat
        MINHITPOINTS = 0x3700 + ((curLevel - 1) << 8) * 2 + (((curVitality - d2ce::PALADIN_VITALITY_MIN) << 8) * 3);
        MINSTAMINA = 0x5900 + ((curLevel + curVitality - d2ce::PALADIN_VITALITY_MIN - 1) << 8);
        MINMANA = 0x0F00 + std::uint32_t(((curLevel + curEnergy - d2ce::PALADIN_ENERGY_MIN - 1) << 8) * 1.5);
        break;
    }

    // you get +20 Life for completing Act 3 Quest 1 (The Golden Bird) for each difficulty level
    MINHITPOINTS += CharInfo->getLifePointsEarned() << 8;

    if (applyChanges)
    {
        // Make sure the values make sense
        bool statsChanged = false;
        std::uint32_t value = std::min(ToInt(&MaxLife), d2ce::MAX_LMS);
        if (value < MINHITPOINTS)
        {
            SetInt(&MaxLife, MINHITPOINTS);
            SetInt(&CurLife, MINHITPOINTS);
            ctrlEditted.insert(MaxLife.GetDlgCtrlID());
            ctrlEditted.insert(CurLife.GetDlgCtrlID());
            statsChanged = true;
        }

        value = std::min(ToInt(&MaxStamina), d2ce::MAX_LMS);
        if (value < MINSTAMINA)
        {
            SetInt(&MaxStamina, MINSTAMINA);
            SetInt(&CurStamina, MINSTAMINA);
            ctrlEditted.insert(MaxStamina.GetDlgCtrlID());
            ctrlEditted.insert(CurStamina.GetDlgCtrlID());
            statsChanged = true;
        }

        value = std::min(ToInt(&MaxMana), d2ce::MAX_LMS);
        if (value < MINMANA)
        {
            SetInt(&MaxMana, MINMANA);
            SetInt(&CurMana, MINMANA);
            ctrlEditted.insert(MaxMana.GetDlgCtrlID());
            ctrlEditted.insert(CurMana.GetDlgCtrlID());
            statsChanged = true;
        }

        if (statsChanged)
        {
            StatsChanged();
        }
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateAppTitle()
{
    CString newAppTitle;
    if (newAppTitle.LoadString(AFX_IDS_APP_TITLE) != 0)
    {
        if (!curPathName.IsEmpty())
        {
            newAppTitle += _T(" - ") + ExtractFileName(curPathName);
            if (Editted)
            {
                newAppTitle += "*";
            }
            switch (CharInfo->getVersion())
            {
            case d2ce::EnumCharVersion::v110:
                newAppTitle += _T(" (Version 1.10-1.14d)");
                break;
            case d2ce::EnumCharVersion::v109:
                newAppTitle += _T(" (Version 1.09)");
                break;
            case d2ce::EnumCharVersion::v108:
                newAppTitle += _T(" (Version Standard 1.08)");
                break;
            case d2ce::EnumCharVersion::v107:
                newAppTitle += _T(" (Version 1.07/Expansion 1.08)");
                break;
            case d2ce::EnumCharVersion::v100:
                newAppTitle += _T(" (Version 1.00 - 1.06)");
                break;
            }
        }
        SetWindowText(newAppTitle);
    }
}
//---------------------------------------------------------------------------
/*
   Makes sure the correct gender titles are displayed based on the
   character's class
*/
void CD2MainForm::UpdateTitleDisplay()
{
    int curSel = CharTitle.GetCurSel();
    if (curSel < 0)
    {
        curSel = (CharInfo->getTitle().bits() & 0x0C) >> 2;
    }

    CharTitle.ResetContent();

    const char** pValidTitles = nullptr;
    switch (static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel()))
    {
    case d2ce::EnumCharClass::Amazon:
    case d2ce::EnumCharClass::Assassin:
    case d2ce::EnumCharClass::Sorceress: // add titles for female characters
        if (IsExpansionCharacter)
        {
            if (IsHardcoreCharacter)
            {
                pValidTitles = HardcoreExpansionTitle;
            }
            else
            {
                pValidTitles = FemaleHardcoreTitle;
            }
        }
        else if (IsHardcoreCharacter)
        {
            pValidTitles = FemaleHardcoreTitle;
        }
        else
        {
            pValidTitles = FemaleTitle;
        }
        break;
    case d2ce::EnumCharClass::Barbarian:
    case d2ce::EnumCharClass::Druid:
    case d2ce::EnumCharClass::Necromancer:
    case d2ce::EnumCharClass::Paladin: // add titles for male characters
    default:
        if (IsExpansionCharacter)
        {
            if (IsHardcoreCharacter)
            {
                pValidTitles = HardcoreExpansionTitle;
            }
            else
            {
                pValidTitles = MaleHardcoreTitle;
            }
        }
        else if (IsHardcoreCharacter)
        {
            pValidTitles = MaleHardcoreTitle;
        }
        else
        {
            pValidTitles = MaleTitle;
        }
        break;
    }

    if (pValidTitles != nullptr)
    {
        for (std::uint32_t i = 0; i < NUM_OF_TITLES; i++)
        {
            CharTitle.AddString(CString(pValidTitles[i]));
        }
    }

    CharTitle.SetCurSel(curSel);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsUpgradeGems()
{
    if (CharInfo == nullptr)
    {
        return;
    }

    auto numConverted = CharInfo->upgradeGems();
    CString msg;
    msg.Format(_T("%zd gem(s) have been upgraded to perfect state"), numConverted);
    if (numConverted > 0)
    {
        hasUpgradableGems = false;
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}
void CD2MainForm::OnUpdateOptionsUpgradeGems(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(hasUpgradableGems ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsUpgradePotions()
{
    if (CharInfo == nullptr)
    {
        return;
    }

    auto numConverted = CharInfo->upgradePotions();
    CString msg;
    msg.Format(_T("%zd potion(s) have been upgraded to their highest level"), numConverted);
    if (numConverted > 0)
    {
        hasUpgradablePotions = false;
        hasUpgradableRejuvenations = CharInfo->anyUpgradableRejuvenations();
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsUpgradePotions(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(hasUpgradablePotions ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsUpgradeRejuvenation()
{
    if (CharInfo == nullptr)
    {
        return;
    }

    auto numConverted = CharInfo->upgradeRejuvenationPotions();
    CString msg;
    msg.Format(_T("%zd potion(s) have been upgraded to Full Rejuvenation potions"), numConverted);
    if (numConverted > 0)
    {
        hasUpgradableRejuvenations = false;
        hasUpgradablePotions = false;
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsUpgradeRejuvenation(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(hasUpgradableRejuvenations ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
/*
   Makes a backup copy of the character file and overwrites any
   existing backup file
*/
void CD2MainForm::WriteBackupFile()
{
    CStringA oldPathNameA(curPathName);
    CString backupname = ChangeFileExt(curPathName, _T(".bak"));
    if (CopyFile(curPathName, backupname, false))
    {
        RestoreActionEnabled = TRUE;
    }
}

//---------------------------------------------------------------------------
void CD2MainForm::OnViewSkillTree()
{
    CD2SkillTreeForm dlg(*this);
    dlg.DoModal();
}

void CD2MainForm::OnUpdateViewSkillTree(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(EditSkillsButton.IsWindowEnabled());
}

void CD2MainForm::OnBnClickedEditSkillsBtn()
{
    OnViewSkillTree();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewQuests()
{
    CD2QuestsForm dlg(*this);
    dlg.DoModal();
}

void CD2MainForm::OnUpdateViewQuests(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(QuestsButton.IsWindowEnabled());
}

void CD2MainForm::OnBnClickedQuestsBtn()
{
    OnViewQuests();
}
//---------------------------------------------------------------------------
const d2ce::ActsInfo& CD2MainForm::getQuests()
{
    if (CharInfo == nullptr)
    {
        static d2ce::ActsInfo dummuy;
        return dummuy;
    }

    return CharInfo->getQuests();
}
//---------------------------------------------------------------------------
void CD2MainForm::updateQuests(const d2ce::ActsInfo& qi)
{
    if (CharInfo == nullptr)
    {
        return;
    }

    CharInfo->updateQuests(qi);
    QuestsChanged = true;
    StatsChanged();
}
//---------------------------------------------------------------------------
d2ce::EnumCharClass CD2MainForm::getCharacterClass()
{
    if (CharInfo == nullptr)
    {
        return d2ce::EnumCharClass::Amazon;
    }

    return static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel());
}
//---------------------------------------------------------------------------
std::uint8_t(&CD2MainForm::getSkills())[d2ce::NUM_OF_SKILLS]
{
    static std::uint8_t dummy[d2ce::NUM_OF_SKILLS] = {0};
    if (CharInfo == nullptr)
    {
        return dummy;
    }

    return CharInfo->getSkills();
}
std::uint32_t CD2MainForm::getSkillChoices()
{
    return Cs.SkillChoices;
}
//---------------------------------------------------------------------------
void CD2MainForm::updateSkills(const std::uint8_t (&updated_skills)[d2ce::NUM_OF_SKILLS])
{
    if (CharInfo == nullptr)
    {
        return;
    }

    CharInfo->updateSkills(updated_skills);
    if (!CharInfo->areSkillsMaxed())
    {
        std::uint32_t totalSkillPoints = CharInfo->getSkillPointUsed();
        if (totalSkillPoints + Cs.SkillChoices < EarnedSkillPoints)
        {
            Cs.SkillChoices = std::min(EarnedSkillPoints - totalSkillPoints, d2ce::MAX_SKILL_CHOICES);
        }
    }
    else
    {
        Cs.SkillChoices = 0;
    }

    SkillChoicesChanged = true;
    StatsChanged();
}
//---------------------------------------------------------------------------
const std::vector<std::reference_wrapper<d2ce::Item>>& CD2MainForm::getGPSs()
{
    static std::vector<std::reference_wrapper<d2ce::Item>> dummy;

    if (CharInfo == nullptr)
    {
        return dummy;
    }

    return CharInfo->getGPSs();
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems, potions or skulls to the specified
   final gem, potion or skull.
   Returns the number of gems converted.
*/
size_t CD2MainForm::convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4])
{
    if (CharInfo == nullptr)
    {
        return 0;
    }

    auto numConverted = CharInfo->convertGPSs(existingGem, desiredGem);
    if (numConverted > 0)
    {
        hasUpgradableRejuvenations = CharInfo->anyUpgradableRejuvenations();
        if (!hasUpgradableRejuvenations)
        {
            // implies no potions or all Full Rejuvenation potions so nothing to upgrade
            hasUpgradablePotions = false;
        }
        else
        {
            hasUpgradablePotions = CharInfo->anyUpgradablePotions();
        }

        hasUpgradableGems = CharInfo->anyUpgradableGems();

        ItemsChanged = true;
        StatsChanged();
    }

    return numConverted;
}
//---------------------------------------------------------------------------
uint32_t CD2MainForm::getSkillPointsEarned() const
{
    if (CharInfo == nullptr)
    {
        return 0;
    }

    std::uint32_t curLevel = std::min(ToInt(&CharLevel), d2ce::NUM_OF_LEVELS);
    return CharInfo->getSkillPointsEarned(curLevel);
}
//---------------------------------------------------------------------------
uint32_t CD2MainForm::getStatPointsEarned() const
{
    if (CharInfo == nullptr)
    {
        return 0;
    }

    std::uint32_t curLevel = std::min(ToInt(&CharLevel), d2ce::NUM_OF_LEVELS);
    return CharInfo->getStatPointsEarned(curLevel);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewWaypoints()
{
    CD2WaypointsForm dlg(*this);
    dlg.DoModal();
}

void CD2MainForm::OnUpdateViewWaypoints(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(WaypointsButton.IsWindowEnabled());
}

void CD2MainForm::OnBnClickedWaypointsBtn()
{
    OnViewWaypoints();
}
//---------------------------------------------------------------------------
std::uint64_t CD2MainForm::getWaypoints(d2ce::EnumDifficulty difficulty) const
{
    if (CharInfo == nullptr)
    {
        return 0;
    }

    return CharInfo->getWaypoints(difficulty);
}
//---------------------------------------------------------------------------
void CD2MainForm::setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue)
{
    if (CharInfo == nullptr)
    {
        return;
    }

    CharInfo->setWaypoints(difficulty, newvalue);
    WaypointsChanged = true;
    StatsChanged();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharName()
{
    prev_name = ToStdString(&CharName);
}
void CD2MainForm::OnEnChangeCharName()
{
    ctrlEditted.insert(CharName.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharName()
{
    auto newName = ToStdString(&CharName);
    if (newName != prev_name)
    {
        CString newFileName = ExtractFilePath(curPathName) + _T("\\") + ToText(&CharName) + _T(".d2s");
        if (FileExists(newFileName) && Editted)
        {
            AfxMessageBox(_T("A file with that name already exists.  Please select another name."), MB_OK | MB_ICONEXCLAMATION);
            SetText(&CharName, prev_name.c_str());

            if (_stricmp(prev_name.c_str(), CharInfo->getName()) == 0)
            {
                auto iter = ctrlEditted.find(CharName.GetDlgCtrlID());
                if (iter != ctrlEditted.end())
                {
                    ctrlEditted.erase(iter);
                }
            }
            else
            {
                ctrlEditted.insert(CharName.GetDlgCtrlID());
            }
        }
        else
        {
            ctrlEditted.insert(CharName.GetDlgCtrlID());
        }

        StatsChanged();
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharDexterity()
{
    OrigValue = ToInt(&CharDexterity);
}
void CD2MainForm::OnEnChangeCharDexterity()
{
    ctrlEditted.insert(CharDexterity.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharDexterity()
{
    std::uint32_t value = ToInt(&CharDexterity);
    if (value > d2ce::MAX_BASICSTATS)
    {
        value = d2ce::MAX_BASICSTATS;
        SetInt(&CharDexterity, value);
    }
    else if (value < MINDEXTERITY)
    {
        value = MINDEXTERITY;
        SetInt(&CharDexterity, value);
    }

    if (OrigValue != value)
    {
        SetInt(&CharDexterity, value);
        ctrlEditted.insert(CharDexterity.GetDlgCtrlID());
        UpdateStatsLeft();
        StatsChanged();
    }
    else if (value == Cs.Dexterity)
    {
        auto iter = ctrlEditted.find(CharDexterity.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CharDexterity.GetDlgCtrlID());
    }
}
void CD2MainForm::OnBnClickedDexterityPlus()
{
    std::uint32_t value = ToInt(&CharDexterity);
    if (value < d2ce::MAX_BASICSTATS && Cs.StatsLeft > 0)
    {
        ++value;
        --Cs.StatsLeft;
        StatsLeftChanged = true;
        ctrlEditted.insert(CharDexterity.GetDlgCtrlID());
        SetInt(&CharDexterity, value);
        CheckStatsLeft();
        StatsChanged();
        return;
    }

    // should not happen
    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnChangeCharEnergy()
{
    ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
}
void CD2MainForm::OnEnSetfocusCharEnergy()
{
    OrigValue = ToInt(&CharEnergy);
}
void CD2MainForm::OnEnKillfocusCharEnergy()
{
    std::uint32_t value = ToInt(&CharEnergy);
    if (value > d2ce::MAX_BASICSTATS)
    {
        value = d2ce::MAX_BASICSTATS;
        SetInt(&CharEnergy, value);
        UpdateMinStats();
    }
    else if (value < MINENERGY)
    {
        value = MINENERGY;
        SetInt(&CharEnergy, value);
        UpdateMinStats();
    }

    if (OrigValue != value)
    {
        SetInt(&CharEnergy, value);
        UpdateMinStats();
        ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
        UpdateStatsLeft();
        StatsChanged();
    }
    else if (value == Cs.Energy)
    {
        auto iter = ctrlEditted.find(CharEnergy.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedEnergyPlus()
{
    std::uint32_t value = ToInt(&CharEnergy);
    if (value < d2ce::MAX_BASICSTATS && Cs.StatsLeft > 0)
    {
        ++value;
        --Cs.StatsLeft;
        StatsLeftChanged = true;
        ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
        SetInt(&CharEnergy, value);
        UpdateMinStats();
        CheckStatsLeft();
        StatsChanged();
        return;
    }

    // should not happen
    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharLevel()
{
    OrigValue = ToInt(&CharLevel);
}
void CD2MainForm::OnEnChangeCharLevel()
{
    ctrlEditted.insert(CharLevel.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharLevel()
{
    std::uint32_t newLevel = ToInt(&CharLevel);
    if (newLevel < 1 || newLevel > d2ce::NUM_OF_LEVELS)
    {
        newLevel = CharInfo->getLevel();
        SetInt(&CharLevel, newLevel);
        SetInt(&Experience, d2ce::MinExpRequired[newLevel - 1]);
        UpdateMaxGold();
        UpdateMinStats();
    }

    if (OrigValue != newLevel)
    {
        SetInt(&CharLevel, newLevel);
        ctrlEditted.insert(CharLevel.GetDlgCtrlID());

        SetInt(&Experience, d2ce::MinExpRequired[newLevel - 1]);
        ctrlEditted.insert(Experience.GetDlgCtrlID());

        UpdateMaxGold();
        UpdateMinStats();
        CheckStatsSkillsLevel(OrigValue);
        StatsChanged();
    }
    else if (newLevel == Cs.Level)
    {
        auto iter = ctrlEditted.find(CharLevel.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CharLevel.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharStrength()
{
    OrigValue = ToInt(&CharStrength);
}
void CD2MainForm::OnEnChangeCharStrength()
{
    ctrlEditted.insert(CharStrength.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharStrength()
{
    std::uint32_t value = ToInt(&CharStrength);
    if (value > d2ce::MAX_BASICSTATS)
    {
        value = d2ce::MAX_BASICSTATS;
        SetInt(&CharStrength, value);
    }
    else if (value < MINSTRENGTH)
    {
        value = MINSTRENGTH;
        SetInt(&CharStrength, value);
    }

    if (OrigValue != value)
    {
        SetInt(&CharStrength, value);
        ctrlEditted.insert(CharStrength.GetDlgCtrlID());
        UpdateStatsLeft();
        StatsChanged();
    }
    else if (value == Cs.Strength)
    {
        auto iter = ctrlEditted.find(CharStrength.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CharStrength.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnBnClickedStrengthPlus()
{
    std::uint32_t value = ToInt(&CharStrength);
    if (value < d2ce::MAX_BASICSTATS && Cs.StatsLeft > 0)
    {
        ++value;
        --Cs.StatsLeft;
        StatsLeftChanged = true;
        ctrlEditted.insert(CharStrength.GetDlgCtrlID());
        SetInt(&CharStrength, value);
        CheckStatsLeft();
        StatsChanged();
        return;
    }

    // should not happen
    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharVitality()
{
    OrigValue = ToInt(&CharVitality);
}
void CD2MainForm::OnEnChangeCharVitality()
{
    ctrlEditted.insert(CharVitality.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharVitality()
{
    std::uint32_t value = ToInt(&CharVitality);
    if (value > d2ce::MAX_BASICSTATS)
    {
        value = d2ce::MAX_BASICSTATS;
        SetInt(&CharVitality, value);
        UpdateMinStats();
    }
    else if (value < MINVITALITY)
    {
        value = MINVITALITY;
        SetInt(&CharVitality, value);
        UpdateMinStats();
    }

    if (OrigValue != value)
    {
        SetInt(&CharVitality, value);
        UpdateMinStats();
        ctrlEditted.insert(CharVitality.GetDlgCtrlID());
        UpdateStatsLeft();
        StatsChanged();
    }
    else if (value == Cs.Vitality)
    {
        auto iter = ctrlEditted.find(CharVitality.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CharVitality.GetDlgCtrlID());
    }
}
void CD2MainForm::OnBnClickedVitalityPlus()
{
    std::uint32_t value = ToInt(&CharVitality);
    if (value < d2ce::MAX_BASICSTATS && Cs.StatsLeft > 0)
    {
        ++value;
        --Cs.StatsLeft;
        StatsLeftChanged = true;
        ctrlEditted.insert(CharVitality.GetDlgCtrlID());
        SetInt(&CharVitality, value);
        UpdateMinStats();
        CheckStatsLeft();
        StatsChanged();
        return;
    }

    // should not happen
    CheckStatsLeft();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCurLife()
{
    OrigValue = ToInt(&CurLife);
}
void CD2MainForm::OnEnChangeCurLife()
{
    ctrlEditted.insert(CurLife.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCurLife()
{
    std::uint32_t value = ToInt(&CurLife);
    if (value > d2ce::MAX_LMS)
    {
        value = d2ce::MAX_LMS;
        SetInt(&CurLife, value);
    }

    if (OrigValue != value)
    {
        SetInt(&CurLife, value);
        ctrlEditted.insert(CurLife.GetDlgCtrlID());

        if (ToInt(&MaxLife) < value)
        {
            SetInt(&MaxLife, value);
            UpdateMinStats();
            ctrlEditted.insert(MaxLife.GetDlgCtrlID());
        }

        StatsChanged();
    }
    else if (value == Cs.CurLife)
    {
        auto iter = ctrlEditted.find(CurLife.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CurLife.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusMaxLife()
{
    OrigValue = ToInt(&MaxLife);
}
void CD2MainForm::OnEnChangeMaxLife()
{
    ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusMaxLife()
{
    UpdateMinStats();
    std::uint32_t value = ToInt(&MaxLife);
    if (value > d2ce::MAX_LMS)
    {
        value = d2ce::MAX_LMS;
        SetInt(&MaxLife, value);
    }

    if (OrigValue != value)
    {
        SetInt(&MaxLife, value);
        ctrlEditted.insert(MaxLife.GetDlgCtrlID());

        if (ToInt(&CurLife) > value)
        {
            SetInt(&CurLife, value);
            ctrlEditted.insert(CurLife.GetDlgCtrlID());
        }

        StatsChanged();
    }
    else if (value == Cs.MaxLife)
    {
        auto iter = ctrlEditted.find(MaxLife.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(MaxLife.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCurMana()
{
    OrigValue = ToInt(&CurMana);
}
void CD2MainForm::OnEnChangeCurMana()
{
    ctrlEditted.insert(CurMana.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCurMana()
{
    std::uint32_t value = ToInt(&CurMana);
    if (value > d2ce::MAX_LMS)
    {
        value = d2ce::MAX_LMS;
        SetInt(&CurMana, value);
    }

    if (OrigValue != value)
    {
        SetInt(&CurMana, value);
        ctrlEditted.insert(CurMana.GetDlgCtrlID());

        if (ToInt(&MaxMana) < value)
        {
            SetInt(&MaxMana, value);
            UpdateMinStats();
            ctrlEditted.insert(MaxMana.GetDlgCtrlID());
        }

        StatsChanged();
    }
    else if (value == Cs.CurMana)
    {
        auto iter = ctrlEditted.find(CurMana.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CurMana.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusMaxMana()
{
    OrigValue = ToInt(&CurLife);
}
void CD2MainForm::OnEnChangeMaxMana()
{
    ctrlEditted.insert(MaxMana.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusMaxMana()
{
    UpdateMinStats();
    std::uint32_t value = ToInt(&MaxMana);
    if (value > d2ce::MAX_LMS)
    {
        value = d2ce::MAX_LMS;
        SetInt(&MaxMana, value);
    }

    if (OrigValue != value)
    {
        SetInt(&MaxMana, value);
        ctrlEditted.insert(MaxMana.GetDlgCtrlID());

        if (ToInt(&CurMana) > value)
        {
            SetInt(&CurMana, value);
            ctrlEditted.insert(CurMana.GetDlgCtrlID());
        }

        StatsChanged();
    }
    else if (value == Cs.MaxMana)
    {
        auto iter = ctrlEditted.find(MaxMana.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(MaxMana.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCurStamina()
{
    OrigValue = ToInt(&CurStamina);
}
void CD2MainForm::OnEnChangeCurStamina()
{
    ctrlEditted.insert(CurStamina.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCurStamina()
{
    std::uint32_t value = ToInt(&CurStamina);
    if (value > d2ce::MAX_LMS)
    {
        value = d2ce::MAX_LMS;
        SetInt(&CurStamina, value);
    }

    if (OrigValue != value)
    {
        SetInt(&CurStamina, value);
        ctrlEditted.insert(CurStamina.GetDlgCtrlID());

        if (ToInt(&MaxStamina) < value)
        {
            SetInt(&MaxStamina, value);
            UpdateMinStats();
            ctrlEditted.insert(MaxStamina.GetDlgCtrlID());
        }

        StatsChanged();
    }
    else if (value == Cs.CurStamina)
    {
        auto iter = ctrlEditted.find(CurStamina.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(CurStamina.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusMaxStamina()
{
    OrigValue = ToInt(&MaxStamina);
}
void CD2MainForm::OnEnChangeMaxStamina()
{
    ctrlEditted.insert(MaxStamina.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusMaxStamina()
{
    UpdateMinStats();
    std::uint32_t value = ToInt(&MaxStamina);
    if (value > d2ce::MAX_LMS)
    {
        value = d2ce::MAX_LMS;
        SetInt(&MaxStamina, value);
    }

    if (OrigValue != value)
    {
        SetInt(&MaxStamina, value);
        ctrlEditted.insert(MaxStamina.GetDlgCtrlID());

        if (ToInt(&CurStamina) > value)
        {
            SetInt(&CurStamina, value);
            ctrlEditted.insert(CurStamina.GetDlgCtrlID());
        }

        StatsChanged();
    }
    else if (value == Cs.MaxStamina)
    {
        auto iter = ctrlEditted.find(MaxStamina.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(MaxStamina.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusCharExperience()
{
    OrigValue = ToInt(&Experience);
}
void CD2MainForm::OnEnChangeCharExperience()
{
    ctrlEditted.insert(Experience.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusCharExperience()
{
    std::uint32_t value = (std::uint32_t)ToInt(&Experience);
    if (value > d2ce::MAX_EXPERIENCE)
    {
        value = d2ce::MAX_EXPERIENCE;
        SetInt(&Experience, value);
    }

    if (OrigValue != value)
    {
        SetInt(&Experience, value);
        ctrlEditted.insert(Experience.GetDlgCtrlID());

        // otherwise updates level if experience has been reduced
        if (value < OrigValue)
        {
            int oldLevel = ToInt(&CharLevel);
            int newLevel = oldLevel;

            // find the correct level
            while ((newLevel > 1) && (value < d2ce::MinExpRequired[newLevel - 1]))
                --newLevel;

            if (newLevel != oldLevel)
            {
                SetInt(&CharLevel, newLevel);
                ctrlEditted.insert(CharLevel.GetDlgCtrlID());
                UpdateMaxGold();
            }
        }

        StatsChanged();
    }
    else if (value == Cs.Experience)
    {
        auto iter = ctrlEditted.find(Experience.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(Experience.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusGoldInBelt()
{
    OrigValue = ToInt(&GoldInBelt);
}
void CD2MainForm::OnEnChangeGoldInBelt()
{
    ctrlEditted.insert(GoldInBelt.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusGoldInBelt()
{
    std::uint32_t value = ToInt(&GoldInBelt);
    if (value > MAXGOLD)
    {
        value = MAXGOLD;
        SetInt(&GoldInBelt, value);
    }

    if (OrigValue != value)
    {
        SetInt(&GoldInBelt, value);
        ctrlEditted.insert(GoldInBelt.GetDlgCtrlID());
        StatsChanged();
    }
    else if (value == Cs.GoldInBelt)
    {
        auto iter = ctrlEditted.find(GoldInBelt.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(GoldInBelt.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::OnEnSetfocusGoldInStash()
{
    OrigValue = ToInt(&GoldInStash);
}
void CD2MainForm::OnEnChangeGoldInStash()
{
    ctrlEditted.insert(GoldInStash.GetDlgCtrlID());
}
void CD2MainForm::OnEnKillfocusGoldInStash()
{
    std::uint32_t value = ToInt(&GoldInStash);
    if (value > MAXSTASHGOLD)
    {
        value = MAXSTASHGOLD;
        SetInt(&GoldInStash, value);
    }

    if (OrigValue != value)
    {
        SetInt(&GoldInStash, value);
        ctrlEditted.insert(GoldInStash.GetDlgCtrlID());
        StatsChanged();
    }
    else if (value == Cs.GoldInBelt)
    {
        auto iter = ctrlEditted.find(GoldInStash.GetDlgCtrlID());
        if (iter != ctrlEditted.end())
        {
            ctrlEditted.erase(iter);
        }
    }
    else
    {
        ctrlEditted.insert(GoldInStash.GetDlgCtrlID());
    }
}
//---------------------------------------------------------------------------
void CD2MainForm::UpdateCharInfoBox(BOOL bEnable)
{
    CharName.EnableWindow(bEnable);
    CharStatusHardcore.EnableWindow(bEnable);
    CharStatusResurrected.EnableWindow(bEnable);
    CharStatusExpansion.EnableWindow(bEnable);
    CharStatusLadder.EnableWindow(bEnable);
    CharClass.EnableWindow(bEnable);
    Difficulty.EnableWindow(bEnable);
    StartingAct.EnableWindow(bEnable);
    CharLevel.EnableWindow(bEnable);
    CharStrength.EnableWindow(bEnable);
    CharEnergy.EnableWindow(bEnable);
    CharDexterity.EnableWindow(bEnable);
    CharVitality.EnableWindow(bEnable);
    CurLife.EnableWindow(bEnable);
    MaxLife.EnableWindow(bEnable);
    CurMana.EnableWindow(bEnable);
    MaxMana.EnableWindow(bEnable);
    CurStamina.EnableWindow(bEnable);
    MaxStamina.EnableWindow(bEnable);
    Experience.EnableWindow(bEnable);
    NextExperience.EnableWindow(bEnable);
    GoldInBelt.EnableWindow(bEnable);
    GoldInStash.EnableWindow(bEnable);
    CharTitle.EnableWindow(bEnable);
    EditSkillsButton.EnableWindow(bEnable);
    MaxAllButton.EnableWindow(bEnable);
    QuestsButton.EnableWindow(bEnable);
    WaypointsButton.EnableWindow(bEnable);

    if (!bEnable)
    {
        CharName.SetWindowText(_T(""));
        CharStatusHardcore.SetCheck(0);
        CharStatusResurrected.SetCheck(0);
        CharStatusExpansion.SetCheck(0);
        CharStatusHardcore.SetCheck(0);
        CharStatusLadder.SetCheck(0);
        CharClass.SetCurSel(-1);
        Difficulty.SetCurSel(-1);
        StartingAct.SetCurSel(-1);
        CharLevel.SetWindowText(_T(""));
        CharStrength.SetWindowText(_T(""));
        CharEnergy.SetWindowText(_T(""));
        CharDexterity.SetWindowText(_T(""));
        CharVitality.SetWindowText(_T(""));
        CurLife.SetWindowText(_T(""));
        MaxLife.SetWindowText(_T(""));
        CurMana.SetWindowText(_T(""));
        MaxMana.SetWindowText(_T(""));
        CurStamina.SetWindowText(_T(""));
        MaxStamina.SetWindowText(_T(""));
        Experience.SetWindowText(_T(""));
        GoldInBelt.SetWindowText(_T(""));
        GoldInStash.SetWindowText(_T(""));
        CharTitle.SetCurSel(-1);
        CharTitle.ResetContent();
        IsExpansionCharacter = FALSE;
        IsHardcoreCharacter = FALSE;

        // Update Static so we refresh properly
        SetText(&NextExperience, _T(""));
        SetText(&StatsLeft, _T(""));

        StrengthPlusButton.EnableWindow(FALSE);
        StrengthPlusButton.ShowWindow(SW_HIDE);
        DexterityPlusButton.EnableWindow(FALSE);
        DexterityPlusButton.ShowWindow(SW_HIDE);
        VitalityPlusButton.EnableWindow(FALSE);
        VitalityPlusButton.ShowWindow(SW_HIDE);
        EnergyPlusButton.EnableWindow(FALSE);
        EnergyPlusButton.ShowWindow(SW_HIDE);
        StatsLeft.ShowWindow(SW_HIDE);
        StatsLeftCtrl.ShowWindow(SW_HIDE);
    }
}
//---------------------------------------------------------------------------
std::string CD2MainForm::ToStdString(const CWnd* Sender) const
{
    return (LPCSTR)CStringA(ToText(Sender));
}
//---------------------------------------------------------------------------
CString CD2MainForm::ToText(const CWnd* Sender) const
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
CStringA CD2MainForm::ToTextA(const CWnd* Sender) const
{
    return CStringA(ToText(Sender));
}
//---------------------------------------------------------------------------
void CD2MainForm::SetText(CWnd* Sender, const char* newValue)
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
void CD2MainForm::SetText(CWnd* Sender, const wchar_t* newValue)
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
std::uint32_t CD2MainForm::ToInt(const CWnd* Sender) const
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        std::string sWindowText(ToStdString(Sender));
        char* ptr = nullptr;
        std::uint32_t value = std::strtoul(sWindowText.c_str(), &ptr, 10);
        switch (Sender->GetDlgCtrlID())
        {
            // shift right 8 bits for actual value
            // Only update these values if the value had changed as
            // you may loose the lower 8 bits from the original value
        case IDC_CUR_LIFE:
            if (value == (Cs.CurLife >> 8))
            {
                return Cs.CurLife;
            }
            value <<= 8;
            break;
        case IDC_MAX_LIFE:
            if (value == (Cs.MaxLife >> 8))
            {
                return Cs.MaxLife;
            }
            value <<= 8;
            break;
        case IDC_CUR_MANA:
            if (value == (Cs.CurMana >> 8))
            {
                return Cs.CurMana;
            }
            value <<= 8;
            break;
        case IDC_MAX_MANA:
            if (value == (Cs.MaxMana >> 8))
            {
                return Cs.MaxMana;
            }
            value <<= 8;
            break;
        case IDC_CUR_STAMINA:
            if (value == (Cs.CurStamina >> 8))
            {
                return Cs.CurStamina;
            }
            value <<= 8;
            break;
        case IDC_MAX_STAMINA:
            if (value == (Cs.MaxStamina >> 8))
            {
                return Cs.MaxStamina;
            }
            value <<= 8;
            break;
        }

        return value;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        return ((CComboBox*)Sender)->GetCurSel();
    }

    return 0;
}
//---------------------------------------------------------------------------
void CD2MainForm::SetInt(CWnd* Sender, std::uint32_t newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        switch (Sender->GetDlgCtrlID())
        {
        case IDC_CUR_LIFE:
        case IDC_MAX_LIFE:
        case IDC_CUR_MANA:
        case IDC_MAX_MANA:
        case IDC_CUR_STAMINA:
        case IDC_MAX_STAMINA:
            newValue >>= 8; // shift right 8 bits for actual value
            break;
        case IDC_CHAR_LEVEL:
            if (newValue + 1 <= d2ce::NUM_OF_LEVELS)
            {
                SetInt(&NextExperience, d2ce::MinExpRequired[newValue]);
            }
            else
            {
                SetText(&NextExperience, _T("NONE"));
            }
            break;
        }

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
bool CD2MainForm::isExpansionCharacter() const
{
    return IsExpansionCharacter ? true : false;
}
//---------------------------------------------------------------------------
d2ce::EnumDifficulty CD2MainForm::getDifficultyLastPlayed()
{
    return CharInfo != nullptr ? CharInfo->getDifficultyLastPlayed() : d2ce::EnumDifficulty::Normal;
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsBackupChar()
{
    BackupChar = !BackupChar;
    AfxGetApp()->WriteProfileInt(SettingsSection, BackupCharacterOption, BackupChar ? 1 : 0);
}

void CD2MainForm::OnUpdateOptionsBackupChar(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(BackupChar ? TRUE : FALSE);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsRestoreChar()
{
    if (!RestoreActionEnabled)
    {
        return;
    }

    RestoreActionEnabled = FALSE;
    CStringA origNameA(ExtractFileName(RemoveFileExtFromPath(curPathName)));
    CStringA oldPathNameA(curPathName);
    CStringA backupname(ChangeFileExt(curPathName, _T(".bak")));

    Editted = false;
    DoFileCloseAction();

    // rename temp file to character file
    if (std::remove((LPCSTR)oldPathNameA))
    {
        CString msg;
        msg.Format(_T("Failed to delete existing character file: %s"), curPathName.GetString());
        AfxMessageBox(msg, MB_OK | MB_ICONERROR);

        // just reopen it again
    }
    else if (std::rename((LPCSTR)backupname, (LPCSTR)oldPathNameA))
    {
        CStringA msg;
        msg.Format("Failed to rename backup character file: %s", backupname.GetString());
        AfxMessageBox(CString(msg), MB_OK | MB_ICONERROR);
        return;
    }

    // allocate memory for character to be loaded
    if (CharInfo == nullptr)
    {
        CharInfo.reset(new d2ce::Character());
    }

    // return if open not successful
    if (!CharInfo->open(oldPathNameA))
    {
        CString errorMsg(CharInfo->getLastError().message().c_str());
        if (errorMsg.IsEmpty())
        {
            errorMsg = _T("Not a valid Diablo II save file.");
        }
        AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

        Editted = false;
        OnFileClose();
        return;
    }

    // check if we have to rename
    if (_stricmp((LPCSTR)origNameA, CharInfo->getName()) != 0)
    {
        Editted = false;
        CharInfo->close();

        RenameCharacterFiles(ToStdString(&CharName));

        // return if open not successful
        CStringA newPathNameA(curPathName);
        if (!CharInfo->open(newPathNameA))
        {
            CString errorMsg(CharInfo->getLastError().message().c_str());
            if (errorMsg.IsEmpty())
            {
                errorMsg = _T("Not a valid Diablo II save file.");
            }
            AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

            Editted = false;
            OnFileClose();
            return;
        }
    }

    Initialize();
    UpdateCharInfoBox(TRUE);

    CharInfo->fillCharacterStats(Cs);
    DisplayCharInfo();
    ctrlEditted.clear();
    CString msg(_T("Character stats have been restored"));
    StatusBar.SetWindowText(msg);
    UpdateAppTitle();
    AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
}

void CD2MainForm::OnUpdateOptionsRestoreChar(CCmdUI* pCmdUI)
{
    if (CharInfo == nullptr)
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    pCmdUI->Enable(RestoreActionEnabled);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnViewLevelReq()
{
    s_levelInfo.Show(this);
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsGpsConvertor()
{
    CD2GemsForm dlg(*this);
    dlg.DoModal();
}

void CD2MainForm::OnUpdateOptionsGpsConvertor(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo == nullptr ? FALSE : (CharInfo->getNumberOfGPSs() == 0 ? FALSE : TRUE));
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsMaxfillstackables()
{
    if (CharInfo == nullptr)
    {
        return;
    }

    auto numConverted = CharInfo->fillAllStackables();
    CString msg;
    msg.Format(_T("%zd stackable item(s) have been fully filled"), numConverted);
    if (numConverted > 0)
    {
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsMaxfillstackables(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo == nullptr ? FALSE : (CharInfo->getNumberOfStackables() == 0 ? FALSE : TRUE));
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsMaxdurabilityforallitems()
{
    if (CharInfo == nullptr)
    {
        return;
    }

    auto numConverted = CharInfo->maxDurabilityAllItems();
    CString msg;
    msg.Format(_T("%zd item(s) have been given the highest durability value"), numConverted);
    if (numConverted > 0)
    {
        StatusBar.SetWindowText(msg);

        ItemsChanged = true;
        StatsChanged();
    }

    AfxMessageBox(msg, MB_ICONINFORMATION | MB_OK);
}

void CD2MainForm::OnUpdateOptionsMaxdurabilityforallitems(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo == nullptr ? FALSE : (CharInfo->getNumberOfArmor() == 0 && CharInfo->getNumberOfWeapons() == 0 ? FALSE : TRUE));
}
//---------------------------------------------------------------------------
void CD2MainForm::OnOptionsResetStats()
{
    std::uint32_t curLevel = std::min(ToInt(&CharLevel), d2ce::NUM_OF_LEVELS);

    // Reset Character Stats
    SetInt(&CharVitality, MINVITALITY);
    ctrlEditted.insert(CharVitality.GetDlgCtrlID());
    SetInt(&CharEnergy, MINENERGY);
    ctrlEditted.insert(CharEnergy.GetDlgCtrlID());
    SetInt(&CharDexterity, MINDEXTERITY);
    ctrlEditted.insert(CharDexterity.GetDlgCtrlID());
    SetInt(&CharStrength, MINSTRENGTH);
    ctrlEditted.insert(CharStrength.GetDlgCtrlID());
    Cs.StatsLeft = CharInfo->getStatPointsEarned(curLevel);
    StatsLeftChanged = true;
    CheckStatsLeft();

    // Reset Character Skills
    CharInfo->resetSkills();
    Cs.SkillChoices = CharInfo->getSkillPointsEarned(curLevel);
    SkillChoicesChanged = true;

    // update min/max stats
    UpdateMinStats(false);
    SetInt(&MaxLife, MINHITPOINTS);
    SetInt(&CurLife, MINHITPOINTS);
    SetInt(&MaxStamina, MINSTAMINA);
    SetInt(&CurStamina, MINSTAMINA);
    SetInt(&MaxMana, MINMANA);
    SetInt(&CurMana, MINMANA);

    StatsChanged();
}
//---------------------------------------------------------------------------
void CD2MainForm::OnUpdateOptionsResetStats(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CharInfo == nullptr ? FALSE : TRUE);
}
//---------------------------------------------------------------------------