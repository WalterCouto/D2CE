/*
    Diablo 2 Character Editor
    Copyright (C) 2000-2003  Burton Tsang

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//---------------------------------------------------------------------------

#ifndef D2MainFormH
#define D2MainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ActnList.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
#include <sys\types>
#include "Character.h"
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
   TOpenDialog *OpenCharDlg;
   TMainMenu *MainMenu;
   TMenuItem *OpenItem;
   TMenuItem *SaveItem;
   TMenuItem *ExitItem;
   TMenuItem *HelpMenuItem;
   TMenuItem *AboutItem;
   TActionList *D2ActionList;
   TAction *FileOpenAction;
   TAction *FileExitAction;
   TAction *FileSaveAction;
   TAction *HelpAboutAction;
   TGroupBox *CharInfoBox;
   TLabel *lblName;
   TEdit *CharName;
   TGroupBox *StatsBox;
   TLabel *lblLevel;
   TLabel *lblStrength;
   TLabel *lblEnergy;
   TLabel *lblDexterity;
   TLabel *lblVitality;
   TEdit *CharLevel;
   TEdit *CharStrength;
   TEdit *CharEnergy;
   TEdit *CharDexterity;
   TEdit *CharVitality;
   TAction *FileCloseAction;
   TGroupBox *LifeBox;
   TLabel *lblCurLife;
   TEdit *CurLife;
   TLabel *lblMaximum;
   TEdit *MaxLife;
   TGroupBox *ManaBox;
   TLabel *lblCurMana;
   TLabel *lblMaxMana;
   TEdit *CurMana;
   TEdit *MaxMana;
   TGroupBox *StaminaBox;
   TLabel *lblCurStamina;
   TLabel *lblMaxStamina;
   TEdit *CurStamina;
   TEdit *MaxStamina;
   TMenuItem *CloseItem;
   TStatusBar *StatusBar;
   TAction *FileSeparatorAction;
   TMenuItem *N1;
   TMenuItem *FileMenuItem;
   TLabel *lblClass;
   TComboBox *CharClass;
   TMenuItem *EditMenuItem;
   TMenuItem *Undo;
   TAction *EditUndoAction;
   TComboBox *CharStatus;
   TLabel *lblStatus;
   TLabel *lblTitle;
   TComboBox *CharTitle;
   TMenuItem *OptionsMenuItem;
   TAction *BackupAction;
   TMenuItem *BackupCharItem;
   TAction *CheckCharAction;
   TMenuItem *CheckCharItem;
   TMenuItem *N3;
   TComboBox *StartingAct;
   TLabel *lblStartingAct;
   TLabel *lblDifficulty;
   TMenuItem *ViewMenuItem;
   TAction *LevelInfoAction;
   TMenuItem *LevelReqItem;
   TLabel *lblExperience;
   TEdit *Experience;
   TButton *MaxAllButton;
   TAction *RestoreAction;
   TMenuItem *RestoreCharacterItem;
   TButton *FixSkillButton;
   TButton *WaypointsButton;
   TGroupBox *GoldBox;
   TLabel *lblBelt;
   TLabel *lblStash;
   TEdit *GoldInBelt;
   TEdit *GoldInStash;
   TButton *EditSkillsButton;
   TButton *QuestsButton;
   TButton *UpgradeGemsButton;
   TLabel *Difficulty;
   TMenuItem *GemConverterItem;
   void __fastcall FileOpenActionExecute(TObject *Sender);
   void __fastcall FileExitActionExecute(TObject *Sender);
   void __fastcall HelpAboutActionExecute(TObject *Sender);
   void __fastcall FileCloseActionExecute(TObject *Sender);
   void __fastcall FileSaveActionExecute(TObject *Sender);
   void __fastcall CharStatsKeyPress(TObject *Sender, char &Key);
   void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
   void __fastcall ComboBoxChange(TObject *Sender);
   void __fastcall CharStatsExitCheck(TObject *Sender);
   void __fastcall EditUndoActionExecute(TObject *Sender);
   void __fastcall ButtonClick(TObject *Sender);
   void __fastcall CharStatsChange(TObject *Sender);
   void __fastcall StatsEnter(TObject *Sender);
   void __fastcall BackupActionExecute(TObject *Sender);
   void __fastcall ExperienceEnter(TObject *Sender);
   void __fastcall CheckCharActionExecute(TObject *Sender);
   void __fastcall CharNameKeyPress(TObject *Sender, char &Key);
   void __fastcall LevelInfoActionExecute(TObject *Sender);
   void __fastcall RestoreActionExecute(TObject *Sender);
   void __fastcall CharNameExit(TObject *Sender);
   void __fastcall CharNameEnter(TObject *Sender);
   void __fastcall GemConverterItemClick(TObject *Sender);
private:	// User declarations
   AnsiString prev_filename,
              prev_name;
   bool ActChanged,              // has starting act been changed
        BackupChar,              // specifies whether backup should be created
        ClassChanged,            // has class been changed
        Edited,                  // has stats been changed
        ItemsChanged,            // has items been changed
        NameChanged,             // has character name been changed
        StatusChanged,           // has hardcore value been changed
        SkillChoicesChanged,     // has skill choices remaining been changed
        TitleChanged;
   long MAXGOLD, MAXSTASHGOLD;
   unsigned char prevLevel,
                 *fileData;
   unsigned long prevExp;

   void CheckName();
   void ClearAllBoolVars();
   void DisplayCharInfo();
   void __fastcall DisplayHint(TObject * Sender);
   int GetCloseResponse();
   void Initialize();
   bool NewStateSelected();
   bool NewTitleSelected();
   void RenameCharacterFiles(const AnsiString& newName);
   void ResetColours();
   void SetColours();
   void SetExperience();
   void SetLevel();
   void SetStartDir();
   void StatsCheck();
   void UpdateAppTitle(const AnsiString& filename);
   void UpdateTitleDisplay();
   void UpgradeGems();
   void WriteBackupFile();
   void SetupBasicStats();
public:		// User declarations
   Character *CharInfo;
   bool JustOpened,
        OthersChanged;           // rest of stats changed

   __fastcall TMainForm(TComponent* Owner);
   void StatsChanged();
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif

