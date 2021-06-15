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

Revision History
================
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
            have maximum durablity without being indestructable.
   - Added: Reset Stats menu item under Options which will do exactly what
            Akara does after completing The Den of Evil.

   - Removed: Upgrade Gems button from main form. There is now a Upgrade submenu
              under options which contains menu items to upgrade Gems and Potions.
   - Removed: "Remove New Skills" menu item was remove as that functionality is
              now part of the "Check Character" menu item.

Version 1.94 (April 17, 2003)
   - Added: You can now convert potions to gems/skulls and vice versa. (Note:
            The Gem converter is now called the GPS converter. GPS stands for
            Gems, Potions, Skulls.)

   - Fixed: Gems could not be converted to regular skulls.


Version 1.93 (March 30, 2003)
   - Added: Option to convert gems from one type and condition to another.

   - Updated: The Adobe Acrobat PDF help file has been updated and renamed to
              UserGuide.pdf.


Version 1.92 (March 15, 2003)
   - Added: Titles for hardcore and expansion characters.  They were not in
            prior versions.


Version 1.91 (January 10, 2003)
   - Added: Some functions to the Character and Item classes that may be
            useful for other programmers.

   - Removed: Non-portable code that was added in v1.90


Version 1.90 (January 9, 2003)
   - Fixed: Web address in About box and README.TXT was incorrect

   - Removed: Refresh action has been removed since it can't be used with the
              ANSI C++ code as mentioned in the "Known Issues" section above.


Version 1.89 (January 3, 2003)
   - Fixed: Quests not reading properly.

   - Added: Editor will notify you if it cannot open a character file.
            (Make sure the file isn't in use by another application.)


Version 1.88 (April 3, 2002)
   - Fixed: Sometimes non-perfect gems were not detected correctly.

   - Removed: Second URL link in the About box.


Version 1.87 (February 12, 2002)
   - Fixed: A minor bug which caused character stats to not read correctly.


Version 1.861 (November 21, 2001)
   - Fixed: A rare bug where certain character names may cause the editor to
            not display the correct stats when the character is first loaded.


Version 1.86 (November 7, 2001)
   - Updated: Checksum calculation code has been reworked to be more portable
              and is also easier to understand than before.

   - Removed: Redundant code that was not used in the earlier 1.86 version.


Version 1.86 (November 5, 2001)
   - Updated: The editor now works with Diablo II v1.09.  Thanks goes to
              Stoned2000 for making his checksum calculation source
              available to the public.


Version 1.85 (October 13, 2001)
   - New: Editor is now open source and this is the last release.

          The main portion of the code has been converted to ANSI C++ (with
          the binary file read/write code in ANSI C) for portability.  The
          front end (GUI) was created with Borland's C++ Builder 5.


Version 1.84 (August 17, 2001)
   - Fixed: Flawless amethyst not upgraded by Upgrade Gems feature.


Version 1.83 (July 20, 2001)
   - Fixed: Upgrading gems sometimes messed up the characters.


Version 1.82 (July 17, 2001)
   - Fixed: Quest values were not properly updated when using the Quest form
            for the first time, changing quest values and then switching between
            acts or closing the form.


Version 1.81 (July 15, 2001)
   - Fixed: Quests weren't updating correctly on the form.

   - Updated: Two more reward options added to Act 5.
   - Updated: Two more character titles were added to account for the Nightmare
              and Hell difficulty levels.


Version 1.8 (July 14, 2001)
   - Updated: The editor has been updated to fully support the new Lord of
              Destruction (LOD) expansion set.  You will only see the
              corresponding LOD options (i.e. Act 5 classes, waypoints, etc.)
              if the character being edited is an expansion set character.

   - Added: Batch option for the various forms in the editor.

   - Changed: Level Requirements form can now be viewed at the same time you
              edit your character.


Version 1.7 (June 30, 2001)
   - Updated: The editor now works with Diablo II v1.07+ including the Lord of
              Destruction expansion pack.  The editor has not been updated to
              support the new classes and their skills or Act 5 as of yet but
              those will be in the next version.

   - Fixed: A bug where decreasing your character's level (don't know why you
            would want to) did not reduce the experience accordingly.


Version 1.6f (May 14, 2001)
   - Changed: Quests dialog code have been reworked again.  Hopefully, it
              should work better now.

   - Fixed: A bug where changing the name of your character did not rename the
            corresponding character files.


Version 1.6e (April 27, 2001)
   - Fixed: A bug where if you changed any quest settings, the program didn't
            let you save.


Version 1.6d (April 13, 2001)
   - Added: Option to enable/disable an act manually.  Before, the next act
            would be enabled based on whether you completed the last quest for
            the previous act.

   - Changed: Quests code has been reworked which results in a smaller
              executable than the previous version.

   - Fixed: A bug where if you had an edited character and opened a new file,
            without first saving the current one, and selected Yes, the stats
            would overwrite the newly opened file.


Version 1.6c (March 8, 2001)
   - Fixed: The experience would always reset to experience requirement value
            based on your characters level.  Current life, mana, and stamina
            values would be changed to match their maximum values,
            respectively.

   - Changed: Updated the PDF help file a bit.


Version 1.6b (January 10, 2001)
   - Fixed: Current life, mana, or stamina values did not change when their
            respective maximum values changed.


Version 1.6a (January 8, 2001)
   - Changed: Upgrade Gems option now works with Diablo II v1.04.

   - Fixed: Could not tab to the current life, mana, and stamina edit boxes.


Version 1.6 (January 7, 2001)
   - Updated: The editor now works with the latest version -- v1.04.

   - Changed: Backup option is now default to on.
   - Changed: Character file you are editing is also displayed on the title
              bar.
   - Changed: Can now edit current life, mana, stamina.  If current life, mana,
              or stamina is less than maximum life, mana, or stamina, they will
              be upgraded to match their respective maximum values.
   - Changed: Maximum values for life, mana, and stamina have been increased to
              5000.
   - Changed: When using v1.04, Upgrade Gems is not available.  I will fix this
              as soon as possible.
   - Changed: A few changes have been made to the About box.


Version 1.5f (October 16, 2000)
   - Changed: Updated the web link in the About box to reflect the new home
              page


Version 1.5e (October 9, 2000)
   - Changed: Fixed a bug where save option was not activated when you edited
              your character's waypoints twice.
   - Changed: Updated the D2Help.pdf file.


Version 1.5d (September 4, 2000)
   - Changed: Fixed a display problem for the Imbue option.  Act 1, Quest 5
              would not display "Imbue" even though you can imbue in the game.


Version 1.5c (September 1, 2000)
   - Changed: Item bug has been fixed.  All items are now recorded correctly.
   - Changed: Improved quest status detection (hopefully).
   - Changed: Made web page and email addresses clickable links in the About
              box and increased the font size for readability.


Version 1.5b (August 26, 2000)
   - Changed: You can now edit experience and gold without having to acquire
              some before editing.
   - Changed: The editor will now start in your Diablo II save directory if
              Diablo II is installed correctly (i.e. game information is stored
              in the registry).  If Diablo II is not installed or there is no
              registry entry for Diablo II then the editor will default to the
              current directory it is located in.
   - Changed: Changed Act 1, Quest 5 status information to show as completed
              after you have used your imbue but did not press Q twice.
   - Changed: Renamed the "Edit Skill Trees" button to "Skill Trees".
   - Changed: More thorough error checking.
   - Changed: Updated the screenshots in the D2Help.pdf file


Version 1.5a (August 15, 2000)
   - Changed: Fixed act statuses not being saved if you select to complete all
              quests for an act.  Town waypoints will also be activated for any
              of the acts in which a quest is set to "Started/In Progress" or
              "Completed".
   - Changed: Upgrade Gems button will only be enabled if your character has
              any non-perfect gems.  If your character has no gems or all gems
              are perfect then this button will not be enabled.
   - Changed: Removed some redundant code and improved memory usage.


Version 1.5 (August 12, 2000)
   - Added: Option to change quest status.  You can turn on the imbue option by
            going to Act 1, Quest 5, and selecting the Imbue status.

            Note:  If quest status is not correctly displayed, please email me
            your character file and state what quests you have completed so I
            may write down the values.
   - Added: Upgrade Gems option.  Upgrades all gems to their perfect state.

   - Changed: Fixed incorrect values for Level Information form and improved
              the layout.
   - Changed: Moved the Refresh option to the View menu.
   - Changed: Character status has been updated more accurately.
   - Changed: D2Help.pdf has been updated.


Version 1.4 (July 31, 2000)
   - Added: Option to activate/deactivate waypoints.  You will only be able to
            access waypoints for later acts if you have finished the required
            previous acts.
   - Added: Level Information Option under View menu.  Displays experience
            required and max gold limits for given level.
   - Added: Restore character option.  This will allow you to restore a
            character from a previous backup (i.e. character file with
            extension .bak must exist).

   - Changed: Renamed "Fix Skill Choices Remaining" button to "Remove New
              Skills Button".
   - Changed: Fixed some of the skills names which did not match the names in
              the game.
   - Changed: Values for various stats will no longer be changed, when saving,
              if you edited a character manually (i.e. hex editor) and you have
              not changed any of the values.
   - Changed: Updated D2Help.pdf file


Version 1.3b (July 26, 2000)
   - Added: Ability to change the Starting Act (i.e. Act 1 through 4) though
            you will not be able to complete any quests for later acts if any
            previous act(s) have not been completed.
   - Added: Ability to change the difficulty level the character was last
            played at.

   - Changed: Fixed a bug with the level-experience check.
   - Changed: Gold In Belt max limit fixed (only affected characters with level
              less than 16)
   - Changed: Can now change character's name but only one '_' character is
              allowed.  If you add more than one, the character's name will be
              reset to the previous name and the character files will not be
              renamed.


Version 1.3a (July 19, 2000)
   - Added: "Check character" option.  This is for people who edited their own
            character and finding it doesn't work in Diablo II.  This option
            checks for any limit violations and corrects them.

   - Changed: Improved checking for character title status and character state
   - Changed: Improved level-experience checking and fixed a bug where changing
              the level would change the experience even if you had more than
              the minimum required for any level
   - Changed: Updated the PDF help file a bit.


Version 1.3 (July 17, 2000)
   - Added: You can now edit character title
   - Added: Hotkey to the File, Close menu item (can use CTRL-C to close the
            currently opened file).
   - Added: A Refresh option (under the Edit menu or press F5) to refresh the
            stats.
   - Added: Option to backup character files located under the Options menu
   - Added: Level-Experience check.  When you edit your character's level and
            you currently do not have enough experience to meet that level,
            your experience points will automatically be updated to meet the
            minimum required.
   - Added: A basic help file is included in Adobe Acrobat (PDF) format.  The
            file name is D2Help.pdf

   - Changed: The character level is now the same on the character selection
              screen as in the game.
   - Changed: You can now edit skills individually with two view options.
   - Changed: You can now have your character file opened at the same time you
              play Diablo II though the stats are only updated when you select
              "Save and Exit Game".
        NOTE: Character stats are not updated if you do not select "Save and
              Exit Game"
   - Changed: stamina limit remains the same as before but the life and mana
              limits have been increased

   - Removed: max level limit.  Max level is now 99 (game limit)
   - Removed: "Max All Skills" button


Version 1.2b (July 9, 2000)
   - added: an Undo option (under the Edit menu) which undoes all changes
   - added: Character State to select whether your character is normal
            (singleplayer and multiplayer), hardcore or dead :)
   - changed: you can now edit character level but there is a max limit set
   - changed: "Fix Skills Choices Remaining" button now only active if you have
              skill choices left
   - changed: "Max All Skills" button is only active if your skills aren't
              already at max
   - fixed bug: if your character had new stats (from a level up) and you saved
                the character without using them, then when you go edit the
                file with the editor the stats would be incorrect
   - fixed bug: if your character is dead (from playing with hardcore setting),
                then stats would be shifted

Version 1.2a (July 3, 2000)
   - changed max limits for max life, mana, and stamina so stamina bar would
     not constantly change (when you are playing as the barbarian class) and
     you have maxed all skills
   - added a "Fix Skill Choices Remaining" button; whenever you attain a new
     level, you also get more skill choices and a New Skills button would
     remain onscreen unless this value in your character file was cleared

Version 1.2 (July 3, 2000)
   - set maximum limits for character stats
   - can now change character class
   - can now enter four digits for max life, max mana, and max stamina
   - added "Max Everything" button so you don't have to do edit all stats
     manually; maxes all stats including all skills

Version 1.1 (June 30, 2000)
   - minor bug fixes
   - no longer requires VCL50.BPL runtime library

Version 1.0 (June 29, 2000)
   - written in C++ Builder 5 in two days
   - you can edit character strength, energy, dexterity, vitality, max
     life/mana/stamina, max all skill trees, gold, and experience
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

BEGIN_MESSAGE_MAP(CD2EditorApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
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

    CWinApp::InitInstance();

    AfxEnableControlContainer();

    // Create the shell manager, in case the dialog contains
    // any shell tree view or shell list view controls.
    CShellManager* pShellManager = new CShellManager;

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

    LoadStdProfileSettings();  // Load standard INI file options (including MRU)

    m_haccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

    CD2MainForm dlg(CalculateFontPointSize());
    m_pMainWnd = &dlg;
    auto nResponse = dlg.DoModal();
    if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
        TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
    }

    // Delete the shell manager created above.
    if (pShellManager != nullptr)
    {
        delete pShellManager;
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
