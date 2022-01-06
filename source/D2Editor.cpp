/*
    Diablo II Character Editor
    Copyright (C) 2000-2003  Burton Tsang
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

Revision History
================
Version 2.08 (Jan 6, 2022)
   - Updated: logic for handling Character progressions properly. Now
              characer Title, Last Difficulty Played, Last Act Played,
              or Quest completed will be properly validated.
   - Updated: Quest and Waypoint dialogs now properly take into
              account Character progression.
    
Version 2.07 (Dec 6, 2021)
   - Updated: json export to use jsoncpp for easier export/import logic
   - Updated: fix bug where Hardcore characters would be detected as dead
   - Updated: get proper level requirement information for Set and Unique
              items

   - Added: tests for the serialized Json export/import functionality
   - Added: ability to directly load and edit Json files via the File
            Open and File Save commands.
   - Added: ability to convert a JSON character file to a D2S character
            file.
    
Version 2.06 (Nov 22, 2021)
   - Added: Context menu for Gem, Potion or Skull items in Items Form
   - Added: Reading/Writing and Json export of the menu appearance section
   - Added: New Serialized Json Export compatable with D2SLib format.
            This format contains only the information available in the d2s
            file without the detailed interpretation given by the existing
            Json export logic. The previous JSON export is now called 
            "Detailed" while the new JSON export is called "Serialized" in
            the menu.

Version 2.05 (Nov 14, 2021)
   - Updated: Fix up handling of reading files with a corpse
   - Updated: Fix up setting/clearing the is potion bit

   - Added: Items Dialog to allow viewing of all items in inventory
   - Added: Added unit tests to cover file with corpse data

Version 2.04 (Nov 5, 2021)
   - Updated: Fix up handling of reading pre 1.09 file items
   - Updated: Application Title will show "Diablo II: Resurrected Character Editor"
     unless an older Diablo II version file is loaded
   - Updated: Added current changes to user guide

   - Added: Mercenary class to encapsulate Mercenary information and items
   - Added: Mercenary Dialog to allow editing of Mercenary Info
   - Added: more unit tests to cover older file versions

Version 2.03 (Oct 2, 2021)
   - Updated: Fix up handling of Simple Quest Items
   - Updated: Properly identify "Scroll of Inifuss" and 
              "Key to the Cairn Stones" items

Version 2.02 (Oct 1, 2021)
   - Updated: Logic to account for Diablo II Ressurected Saved Games folder

Version 2.01 (Sept 25, 2021)
   - Updated: Moved logic in UI down to Character class for consistency.
              Refactored and cleaned up code to simplify logic
   - Updated: Fixed bug in About window to properly show version as 3 digits.
   - Updated: Fixed bug in renaming character files.
   - Updated: Max Durability For All Items now also updates Mercenary Items.

   - Added: Unit test project and tests.
   - Added: Reading of Merc and NPC sections
   - Added: logic to fetch magical properties for an item
   - Added: JSON export option in File menu that produces output compatible 
            with https://github.com/nokka/d2s

   - Removed: remove long revision history prior to 2.00 as it is not
              relevant any longer and you can get that detail from the
              1.9.6 branch

Version 2.00 (June 18, 2021)
   - Updated: Converted from a C++ Builder 5 project to a Visual Studio 2019
              project.
   - Updated: Changed Main Dialog to resemble the in-game Character Stats
              screen.
   - Updated: Changed Waypoint and Quest Dialogs to remove tabs and display
              all in initial screen
   - Updated: In the Quest dialog, changed to tri-state checkboxes so either a
              quest is not started, started or completed fully
   - Updated: Changed Skills dialog to show all the skill under their category
              as displayed in the game in one screen.
   - Updated: The logic to upgrade gems to the perfect quality to use same logic
              as the GPS converter code added in the 1.93 version for consistent
              handling of gem/potion conversions.
   - Updated: The editor now works with v1.10 of the character file. It now
              reads/writes all sections properly and also handles Corpse, Merc
              and Golem sections. The maximum character stats have also been
              adjusted to match the limits due to the file structure.
   - Updated: use std::error_code to hold error codes from file open or save
              operations.
   - Updated: The original Diablo II icon with transparant backgroud is used.
   - Updated: Level Requirements will show requirments using the version of
              from the character file. V1.10 is assumed to have the latest
              Level Requirements from Diable II 1.14b, while versions
              1.07-1.09 will level requirements for those versions and any
              version below 1.07 will show the level requirements tha
              existed since 1.00. When loading the Level Requirements dialog
              with no character file opened, it will assume the latest
              file version.
   - Updated: The GPS Converter Dialog is now fuly called Gems, Potions &
              Skulls Converter as is the menu item. The logic around converting
              GPS has been merged across supported character file versions and
              the upgrade potions and gems methods. Support added for Runes.
   - Updated: The Adobe Acrobat PDF help file has been updated.
   - Updated: Modfied item reading logic to be able to read v1.15 (D2R)
              character files.
   - Updated: File reading will now use read/write share mode again but cpp
              file has _MSC_VER guard around thus allowing the file to still
              be portable in theory.  This means you can once again open a
              file already opened by the game or something else but it is still
              recommended you do not have it open by anything else.
   - Updated: namespace d2ce is now part of all non-MFC classes, i.e. the
              portable code.  This quickly identifies what is part of UI logic
              and what is part of the core.
   - Updated: Max Everything menu item to also make all potions the best
              quality, all gems and skulls perfect, all stackable items fully
              filled and all weapons and armor have max durability.
   - Updated: Max Everything will not make all stats their maximum bit value
              and all Skill have the max value of 20. Instead it will change
              the stats points and skill point yet to distribute value to be
              what the maximum earned points would be in a game. The user can
              still make all skill points be 20 in the Skills dialog and can
              edit the stat values to be maximum if they wish to have values
              beyond of what is possible in a game.
   - Updated: Check Character now checks for more possible issues, like one
              of the stats having points lower then minimum expected value.

   - Added: Upgrades from potions of any quality to potions of highest quality
            as well as potions of any kind to Full Rejuvenation potions. These
            operations are available under the Upgrade submenu of the Options
            main menu.
   - Added: Hotkey to the View, Reload menu item (can use F5 to reload the
            currently opened file).
   - Added: Check for Dead Hardcore character added during opeing a Character
            file allowing user mark a dead hardcore character as alive again.
            The "Check character" option also performs this check.
   - Added: MRU support so you can easily open previously opened files.
   - Added: New ActInfo class seperates the logic of handling Quest and Act
            information to simplify and make less error prone other parts of
            the code.
   - Added: Max Quanity For All Stackables menu item that fully fills all
            stackables.
   - Added: Max Durability For All Items which makes all Armour and Weapons
            have maximum durablity without being Indestructible.
   - Added: Reset Stats menu item under Options which will do exactly what
            Akara does after completing The Den of Evil.

   - Removed: Upgrade Gems button from main form. There is now a Upgrade submenu
              under options which contains menu items to upgrade Gems and Potions.
   - Removed: "Remove New Skills" menu item was remove as that functionality is
              now part of the "Check Character" menu item.
*/
//---------------------------------------------------------------------------

#include "pch.h"
#include "framework.h"
#include "D2Editor.h"
#include "D2MainForm.h"
#include "shellscalingapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CD2EditorApp

BEGIN_MESSAGE_MAP(CD2EditorApp, CWinAppEx)
    ON_COMMAND(ID_HELP, &CWinAppEx::OnHelp)
END_MESSAGE_MAP()


// CD2EditorApp construction
namespace
{
    typedef HRESULT(WINAPI* LPGetDpiForMonitor)(_In_ HMONITOR hmonitor,
        _In_ MONITOR_DPI_TYPE dpiType,
        _Out_ UINT* dpiX,
        _Out_ UINT* dpiY);

    int GetDPIOfMonitor(HMONITOR hMonitor)
    {
        UINT dpix = 96, dpiy = 96;
        HMODULE hModule = ::LoadLibraryW(L"Shcore.dll");
        if (hModule != NULL)
        {
            LPGetDpiForMonitor GetDpiForMonitor =
                (LPGetDpiForMonitor)GetProcAddress(hModule, "GetDpiForMonitor");

            if (GetDpiForMonitor != NULL &&
                GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy) != S_OK)
            {
                return 96;
            }
        }
        else
        {
            HDC screen = GetDC(0);
            dpix = GetDeviceCaps(screen, LOGPIXELSX);
            ReleaseDC(0, screen);
        }

        return dpix;
    }

    BOOL CALLBACK Monitorenumproc(HMONITOR hMonitor, HDC /*hdc*/, LPRECT /*pRect*/, LPARAM arg)
    {
        std::vector<HMONITOR>* monitors = (std::vector<HMONITOR>*)arg;
        monitors->push_back(hMonitor);
        return TRUE;
    }

    double GetDPIZoomFactor()
    {
        std::vector<HMONITOR> monitors;
        if (!::EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)Monitorenumproc, (LPARAM)&monitors))
        {
            return 1.0;
        }

        double factorHeight = 1.0;
        double factorDpi = 1.0;
        double factor = 1.0;
        for (size_t i = 0; i < monitors.size(); i++)
        {
            MONITORINFOEX minfo{};

            minfo.cbSize = sizeof(MONITORINFOEX);
            if (GetMonitorInfo(monitors[i], &minfo))
            {
                factorHeight = double(minfo.rcMonitor.bottom - minfo.rcMonitor.top) / 1050.0;
                factorDpi = double(GetDPIOfMonitor(monitors[i])) / 96.0;
                if ((factorDpi > 1) && (factorDpi  > factorHeight))
                {
                    factor = std::max(factor, factorDpi / factorHeight);
                }
            }
        }

        return factor;
    }

    WORD CalculateFontPointSize()
    {
        return std::max(WORD(1), (WORD)std::round(8.0 / GetDPIZoomFactor()));
    }
}

CD2EditorApp::CD2EditorApp()
{
    // support Restart Manager
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

// The one and only CD2EditorApp object
CD2EditorApp theApp;

// CD2EditorApp initialization
BOOL CD2EditorApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls{};
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    __super::InitInstance();

    AfxEnableControlContainer();

    // Activate "Windows Native" visual manager for enabling themes in MFC controls
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    LoadStdProfileSettings();  // Load standard INI file options (including MRU)InitContextMenuManager();

    InitKeyboardManager();

    InitTooltipManager();
    CMFCToolTipInfo ttParams;
    ttParams.m_bVislManagerTheme = TRUE;
    theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
        RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

    m_haccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

    CD2MainForm dlg(CalculateFontPointSize());
    m_pMainWnd = &dlg;
    auto nResponse = dlg.DoModal();
    if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
        TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}
//---------------------------------------------------------------------------
BOOL CD2EditorApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
    // Handle dialog-based accelerator keys
    if (m_haccel)
    {
        if (::TranslateAccelerator(m_pMainWnd->m_hWnd, m_haccel, lpMsg))
        {
            return TRUE;
        }
    }

    return __super::ProcessMessageFilter(code, lpMsg);
}
//---------------------------------------------------------------------------
CDocument* CD2EditorApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
    m_pMainWnd->SendMessage(WM_OPEN_DLG_FILE, 0, (LPARAM)lpszFileName);

    /*
    We need to return a non null value otherwise the most recent file will be
    deleted from the menu.  If the calling function is the
    CWinApp::OnOpenRecentFile it will not use the return value, so it is safe to
    return a bogus value.
    */
    return((CDocument*)1); // CWinApp::OpenDocumentFile(lpszFileName);
}
//---------------------------------------------------------------------------
