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
#include "D2MainForm.h"


//---------------------------------------------------------------------------
class CD2SkillToolTipCtrlCallback
{
public:
    virtual ~CD2SkillToolTipCtrlCallback() = default;
    virtual const std::uint16_t* InvHitTest(UINT id, CPoint point, std::uint8_t& base, std::uint16_t& bonus, bool& enabled, TOOLINFO* pTI = nullptr) const = 0;
};

//---------------------------------------------------------------------------
class CD2SkillTreeForm : public CDialogEx, public CD2SkillToolTipCtrlCallback
{
    DECLARE_DYNAMIC(CD2SkillTreeForm)

public:
    CD2SkillTreeForm(CD2MainForm& form);
    virtual ~CD2SkillTreeForm();

    const std::uint16_t* InvHitTest(CPoint point, TOOLINFO* pTI = nullptr) const; // get skill at point
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;  // get tool at point

    // Dialog Data
    enum { IDD = IDD_SKILLS_DIALOG };

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    void OnTab1SkillKillFocus(UINT nID);
    void OnTab2SkillKillFocus(UINT nID);
    void OnTab3SkillKillFocus(UINT nID);
    void OnTab1SkillBnClicked(UINT nID);
    void OnTab2SkillBnClicked(UINT nID);
    void OnTab3SkillBnClicked(UINT nID);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedSetAll();
    DECLARE_MESSAGE_MAP()
    BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

    void CheckTab1Skills();
    void CheckTab2Skills();
    void CheckTab3Skills();

    void CheckToolTipCtrl();

public:
    virtual BOOL OnInitDialog();
    bool isSkillChoicesChanged() const;

private:
    CString ToText(CWnd* Sender);
    std::string ToStdString(CWnd* Sender);
    std::uint32_t ToInt(UINT nID);
    void SetInt(UINT nID, std::uint32_t newValue);
    void SaveSkills();
    void UpdateCaption();
    std::uint16_t GetBonusSkillPoints(std::uint16_t idx) const;
    std::uint8_t GetSkillPoints(std::uint16_t idx, bool addBonusPts = false) const;
    bool SetSkillPoints(std::uint16_t idx, std::uint8_t value);

private:
    std::array<std::uint8_t, d2ce::NUM_OF_SKILLS> Skills;
    std::vector<std::uint16_t> BonusSkillPoints;
    bool SkillsChanged = false;
    CD2MainForm& MainForm;
    d2ce::EnumCharClass Class = d2ce::EnumCharClass::Amazon;
    std::uint16_t Level = 1;
    std::uint32_t SkillChoices = 0;
    std::uint32_t SkillsUsed = 0;
    std::uint32_t EarnedSkillPoints = 0;
    CString OrigCaption;

    struct SkillCtrlsType
    {
        CBitmap bitmap;
        UINT buttonId = 0;
        UINT editId = 0;
        std::uint16_t skillId = MAXUINT16;
        std::uint16_t skillIdx = MAXUINT16;
        std::vector<std::uint16_t> reqSkills; // map to index in Skill array
        bool levelReqMet = true;
    };
    std::map<UINT, SkillCtrlsType> Tab1SkillMap;
    std::map<UINT, SkillCtrlsType> Tab2SkillMap;
    std::map<UINT, SkillCtrlsType> Tab3SkillMap;
    std::map<UINT, UINT> SkillBnMap;
    std::map<std::uint16_t, UINT> SkillIdxBnMap;

    // Inherited via CD2SkillToolTipCtrlCallback
    virtual const std::uint16_t* InvHitTest(UINT id, CPoint point, std::uint8_t& base, std::uint16_t& bonus, bool& enabled, TOOLINFO* pTI = nullptr) const override;
public:
    afx_msg void OnPaint();
};
//---------------------------------------------------------------------------
