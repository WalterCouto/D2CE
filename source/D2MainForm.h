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

#pragma once

#include "d2ce\Character.h"
#include "MainFormConstants.h"
#include <memory>
#include <set>

class CCharNameEdit : public CEdit
{
    // Construction
public:
    CCharNameEdit();

public:
    virtual ~CCharNameEdit();

    // Generated message map functions
protected:
    //{{AFX_MSG(CCharNameEdit)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    //}}AFX_MSG
    afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

protected:
    CString GetValidText(LPCTSTR value) const;
}; 

class CStatsLeftImage : public CStatic
{
    // Construction
public:
    CStatsLeftImage();

public:
    virtual ~CStatsLeftImage();

    // Generated message map functions
protected:
    //{{AFX_MSG(CCharNameEdit)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

protected:
    virtual void PreSubclassWindow();
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
    CBitmap  m_bitmap;
};

// CD2MainForm dialog
class CD2MainForm : public CDialogEx
{
    DECLARE_DYNAMIC(CD2MainForm)

    // Construction
public:
    CD2MainForm(WORD fontSize, CWnd* pParent = nullptr);
    CD2MainForm(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_D2EDITOR_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
    HICON m_hIcon = NULL;

    UINT m_nIDTracking = 0;
    UINT m_nIDLastMessage = 0;
    double m_dpiFactor = 1.0;

    CFont BoldFont;
    std::uint32_t OrigValue = 0; // used for set focus and kill focus

    BOOL RestoreActionEnabled = FALSE;
    BOOL IsExpansionCharacter = FALSE;
    BOOL IsHardcoreCharacter = FALSE;

    std::uint32_t EarnedStatPoints = 0;
    std::uint32_t EarnedSkillPoints = 0;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam = 0L);
    afx_msg LRESULT OnPopMessageString(WPARAM wParam, LPARAM lParam);
    afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
    afx_msg void OnViewSkillTree();
    afx_msg void OnUpdateViewSkillTree(CCmdUI* pCmdUI);
    afx_msg void OnBnClickedEditSkillsBtn();
    afx_msg void OnOptionsMaxEverything();
    afx_msg void OnUpdateOptionsMaxEverything(CCmdUI* pCmdUI);
    afx_msg void OnBnClickedMaxAllBtn();
    afx_msg void OnViewQuests();
    afx_msg void OnUpdateViewQuests(CCmdUI* pCmdUI);
    afx_msg void OnBnClickedQuestsBtn();
    afx_msg void OnOptionsUpgradeGems();
    afx_msg void OnUpdateOptionsUpgradeGems(CCmdUI* pCmdUI);
    afx_msg void OnViewWaypoints();
    afx_msg void OnUpdateViewWaypoints(CCmdUI* pCmdUI);
    afx_msg void OnBnClickedWaypointsBtn();
    afx_msg void OnEnChangeCharName();
    afx_msg void OnEnSetfocusCharName();
    afx_msg void OnEnKillfocusCharName();
    afx_msg void OnEnSetfocusCharLevel();
    afx_msg void OnEnChangeCharLevel();
    afx_msg void OnEnKillfocusCharLevel();
    afx_msg void OnEnSetfocusCharStrength();
    afx_msg void OnEnChangeCharStrength();
    afx_msg void OnEnKillfocusCharStrength();
    afx_msg void OnBnClickedStrengthPlus();
    afx_msg void OnEnSetfocusCharDexterity();
    afx_msg void OnEnChangeCharDexterity();
    afx_msg void OnEnKillfocusCharDexterity();
    afx_msg void OnBnClickedDexterityPlus();
    afx_msg void OnEnSetfocusCharVitality();
    afx_msg void OnEnChangeCharVitality();
    afx_msg void OnEnKillfocusCharVitality();
    afx_msg void OnBnClickedVitalityPlus();
    afx_msg void OnEnSetfocusCharEnergy();
    afx_msg void OnEnChangeCharEnergy();
    afx_msg void OnEnKillfocusCharEnergy();
    afx_msg void OnBnClickedEnergyPlus();
    afx_msg void OnEnSetfocusCharExperience();
    afx_msg void OnEnChangeCharExperience();
    afx_msg void OnEnKillfocusCharExperience();
    afx_msg void OnEnSetfocusCurLife();
    afx_msg void OnEnChangeCurLife();
    afx_msg void OnEnKillfocusCurLife();
    afx_msg void OnEnSetfocusMaxLife();
    afx_msg void OnEnChangeMaxLife();
    afx_msg void OnEnKillfocusMaxLife();
    afx_msg void OnEnSetfocusCurMana();
    afx_msg void OnEnChangeCurMana();
    afx_msg void OnEnKillfocusCurMana();
    afx_msg void OnEnSetfocusMaxMana();
    afx_msg void OnEnChangeMaxMana();
    afx_msg void OnEnKillfocusMaxMana();
    afx_msg void OnEnSetfocusCurStamina();
    afx_msg void OnEnChangeCurStamina();
    afx_msg void OnEnKillfocusCurStamina();
    afx_msg void OnEnSetfocusMaxStamina();
    afx_msg void OnEnChangeMaxStamina();
    afx_msg void OnEnKillfocusMaxStamina();
    afx_msg void OnEnSetfocusGoldInBelt();
    afx_msg void OnEnChangeGoldInBelt();
    afx_msg void OnEnKillfocusGoldInBelt();
    afx_msg void OnEnSetfocusGoldInStash();
    afx_msg void OnEnChangeGoldInStash();
    afx_msg void OnEnKillfocusGoldInStash();
    afx_msg void OnCbnSelchangeCharClassCmb();
    afx_msg void OnBnClickedHardcoreCheck();
    afx_msg void OnBnClickedResurrectedCheck();
    afx_msg void OnBnClickedExpansionCheck();
    afx_msg void OnBnClickedLadderCheck();
    afx_msg void OnCbnSelchangeCharTitleCmb();
    afx_msg void OnCbnSelchangeStartingActCmb();
    afx_msg void OnFileSave();
    afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
    afx_msg void OnFileClose();
    afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);
    afx_msg void OnFileOpen();
    LRESULT OnMRUFileOpen(WPARAM wParam, LPARAM lParam);
    afx_msg void OnViewRefresh();
    afx_msg void OnUpdateViewRefresh(CCmdUI* pCmdUI);
    afx_msg void OnCbnSelchangeDifficultyCmb();
    afx_msg void OnOptionsBackupChar();
    afx_msg void OnUpdateOptionsBackupChar(CCmdUI* pCmdUI);
    afx_msg void OnOptionsRestoreChar();
    afx_msg void OnUpdateOptionsRestoreChar(CCmdUI* pCmdUI);
    afx_msg void OnOptionsCheckChar();
    afx_msg void OnUpdateOptionsCheckChar(CCmdUI* pCmdUI);
    afx_msg void OnAppAbout();
    afx_msg void OnAppExit();
    afx_msg void OnViewLevelReq();
    afx_msg void OnOptionsUpgradePotions();
    afx_msg void OnUpdateOptionsUpgradePotions(CCmdUI* pCmdUI);
    afx_msg void OnOptionsUpgradeRejuvenation();
    afx_msg void OnUpdateOptionsUpgradeRejuvenation(CCmdUI* pCmdUI);
    afx_msg void OnOptionsGpsConvertor();
    afx_msg void OnUpdateOptionsGpsConvertor(CCmdUI* pCmdUI);
    afx_msg void OnOptionsMaxfillstackables();
    afx_msg void OnUpdateOptionsMaxfillstackables(CCmdUI* pCmdUI);
    afx_msg void OnOptionsMaxdurabilityforallitems();
    afx_msg void OnUpdateOptionsMaxdurabilityforallitems(CCmdUI* pCmdUI);
    afx_msg void OnOptionsResetStats();
    afx_msg void OnUpdateOptionsResetStats(CCmdUI* pCmdUI);
    DECLARE_MESSAGE_MAP()
    BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

private:	// User declarations
    std::string  prev_name;
    bool BackupChar = true,          // specifies whether backup should be created
        Editted = false,             // has stats been changed
        ItemsChanged = false,        // has items been changed
        StatusChanged = false,       // has hardcore value been changed
        StatsLeftChanged = false,    // has the number of stats point left changed
        SkillChoicesChanged = false, // has skill choices remaining been changed
        WaypointsChanged = false,    // has list of active waypoints changed
        QuestsChanged = false;       // has the list of completed quests changed
    std::uint32_t MAXGOLD = d2ce::GOLD_IN_BELT_LIMIT, MAXSTASHGOLD = d2ce::GOLD_IN_STASH_LIMIT;
    std::uint32_t MINHITPOINTS = 0x100, MINSTAMINA = 0x100, MINMANA = 0x100;
    std::uint32_t MINVITALITY = 1, MINENERGY = 1, MINDEXTERITY = 1, MINSTRENGTH = 1;
    bool hasUpgradableGems = false;
    bool hasUpgradablePotions = false;
    bool hasUpgradableRejuvenations = false;
    d2ce::CharStats Cs;
    std::vector<WORD> m_vecTemplate;

    CCharNameEdit CharName;
    CString InitialDir;
    CButton CharStatusHardcore;
    CButton CharStatusResurrected;
    CButton CharStatusExpansion;
    CButton CharStatusLadder;
    CComboBox CharClass;
    CComboBox Difficulty;
    CComboBox StartingAct;
    CEdit CharLevel;
    CEdit CharStrength;
    CButton StrengthPlusButton;
    CEdit CharEnergy;
    CButton EnergyPlusButton;
    CEdit CharDexterity;
    CButton DexterityPlusButton;
    CEdit CharVitality;
    CButton VitalityPlusButton;
    CEdit CurLife;
    CEdit MaxLife;
    CEdit CurMana;
    CEdit MaxMana;
    CEdit CurStamina;
    CEdit MaxStamina;
    CEdit Experience;
    CEdit GoldInBelt;
    CEdit GoldInStash;
    CStatic NextExperience;
    CStatic StatsLeft;
    CStatsLeftImage StatsLeftCtrl;
    CComboBox CharTitle;
    CButton EditSkillsButton;
    CBitmap EditSkillsImage;
    CButton MaxAllButton;
    CBitmap MaxAllImage;
    CButton QuestsButton;
    CBitmap QuestsImage;
    CButton WaypointsButton;
    CBitmap WaypointsImage;
    CButton QuitButton;
    CBitmap QuitImage;
    CBitmap BackgroundImage;
    CStatusBar StatusBar;
    CBrush BlackBrush;
    CRect LastRect;
    CStatic BackgroundBox;

    std::set<UINT> ctrlEditted;
    CString curPathName;

    void OpenFile(LPCTSTR filename);
    void ClearAllBoolVars();
    void DisplayCharInfo();
    int DoFileCloseAction();
    void Initialize();
    bitmask::bitmask<d2ce::EnumCharStatus> getStatus();
    bool NewStatusSelected();
    bool NewTitleSelected();
    void RenameCharacterFiles(const std::string& newName);
    void SetStartDir();
    void UpdateAppTitle();
    void UpdateTitleDisplay();
    void WriteBackupFile();
    void SetupBasicStats();
    void StatsChanged();
    bool CheckIsHardcoreDead(bool& bStatChanged);

    void CheckStatsSkillsLevel(std::uint32_t prevLevel);
    void UpdateStatsLeft();
    void CheckStatsLeft();
    void UpdateMaxGold();
    void UpdateStartStats();
    void UpdateMinStats(bool applyChanges = true);
    void UpdateCharInfoBox(BOOL bEnable);
    std::string ToStdString(const CWnd* Sender) const;
    CString ToText(const CWnd* Sender) const;
    CStringA ToTextA(const CWnd* Sender) const;
    void SetText(CWnd* Sender, const char* newValue);
    void SetText(CWnd* Sender, const wchar_t* newValue);
    std::uint32_t ToInt(const CWnd* Sender) const;
    void SetInt(CWnd* Sender, std::uint32_t newValue);

public:
    std::unique_ptr<d2ce::Character> CharInfo;

    bool isExpansionCharacter() const;
    d2ce::EnumDifficulty getDifficultyLastPlayed();
    std::uint64_t getWaypoints(d2ce::EnumDifficulty difficulty) const;
    void setWaypoints(d2ce::EnumDifficulty difficulty, std::uint64_t newvalue);
    const d2ce::ActsInfo& getQuests();
    void updateQuests(const d2ce::ActsInfo& qi);
    d2ce::EnumCharClass getCharacterClass();
    std::uint8_t (&getSkills())[d2ce::NUM_OF_SKILLS];
    std::uint32_t getSkillChoices();
    void updateSkills(const std::uint8_t(&updated_skills)[d2ce::NUM_OF_SKILLS]);
    const std::vector<std::reference_wrapper<d2ce::Item>>& getGPSs();
    size_t convertGPSs(const std::uint8_t(&existingGem)[4], const std::uint8_t(&desiredGem)[4]);
    uint32_t getSkillPointsEarned() const;
    uint32_t getStatPointsEarned() const;
};
//---------------------------------------------------------------------------
