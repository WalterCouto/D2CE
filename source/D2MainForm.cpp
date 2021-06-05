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

#include <vcl.h>
#pragma hdrstop

#include <cmath>
#include <registry.hpp>
#include "D2MainForm.h"
#include "D2AboutForm.h"
#include "D2SkillTreeForm.h"
#include "D2LevelInfoForm.h"
#include "D2WaypointsForm.h"
#include "D2QuestsForm.h"
#include "MainFormConstants.h"
#include "ExperienceConstants.h"
#include "Constants.h"
#include "D2GemsForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
   : TForm(Owner)
{
   Application->OnHint = DisplayHint;

   BackupChar = BackupAction->Checked;

   CharDexterity->Hint = HINTTEXT + MAX_BASICSTATS;
   CharEnergy->Hint = HINTTEXT + MAX_BASICSTATS;
   CharLevel->Hint = HINTTEXT + NUM_OF_LEVELS;
   CharStrength->Hint = HINTTEXT + MAX_BASICSTATS;
   CharVitality->Hint = HINTTEXT + MAX_BASICSTATS;

   MaxLife->Hint = HINTTEXT + MAX_LMS;
   MaxMana->Hint = HINTTEXT + MAX_LMS;
   MaxStamina->Hint = HINTTEXT + MAX_LMS;

   Experience->Hint = HINTTEXT + MAXEXP_TEXT;
   GoldInBelt->Hint = HINTTEXT + "based on your level";
   GoldInStash->Hint = HINTTEXT + "based on your level";

   SetStartDir();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BackupActionExecute(TObject *Sender)
{
   if (BackupCharItem->Checked)  // turning off backup option
      BackupChar = false;
   else                          // turning on backup option
      BackupChar = true;

   BackupCharItem->Checked = !BackupCharItem->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ButtonClick(TObject *Sender)
{
   if (Sender == EditSkillsButton)
      SkillTrees->ShowModal();
   else if (Sender == FixSkillButton)
   {
      CharInfo->clearSkillChoices();
      JustOpened = false;
      SkillChoicesChanged = true;
      FixSkillButton->Enabled = false;
      StatusBar->Panels->Items[0]->Text = "Skill choices remaining has been reset to zero";

      StatsChanged();
   }
   else if (Sender == MaxAllButton)
   {
      CharLevel->Text = NUM_OF_LEVELS;

      CharStrength->Text = MAX_BASICSTATS;
      CharDexterity->Text = MAX_BASICSTATS;
      CharVitality->Text = MAX_BASICSTATS;
      CharEnergy->Text = MAX_BASICSTATS;

      MaxLife->Text = MAX_LMS;
      if (CurLife->Text.ToInt() < MaxLife->Text.ToInt())
         CurLife->Text = MaxLife->Text;

      MaxMana->Text = MAX_LMS;
      if (CurMana->Text.ToInt() < MaxMana->Text.ToInt())
         CurMana->Text = MaxMana->Text;

      MaxStamina->Text = MAX_LMS;
      if (CurStamina->Text.ToInt() < MaxStamina->Text.ToInt())
         CurStamina->Text = MaxStamina->Text;

      Experience->Text = MAX_EXPERIENCE;

      StatsEnter(GoldInBelt);
      GoldInBelt->Text = MAXGOLD;

      StatsEnter(GoldInStash);
      GoldInStash->Text = MAXSTASHGOLD;

      CharInfo->maxSkills();

      JustOpened = false;
      OthersChanged = true;

      SetColours();
      StatsChanged();
   }
   else if (Sender == QuestsButton)
      QuestsForm->ShowModal();
   else if (Sender == UpgradeGemsButton)
      UpgradeGems();
//      ConvertGemsForm->ShowModal();
   else if (Sender == WaypointsButton)
      WaypointsForm->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CharNameEnter(TObject *Sender)
{
   prev_name = CharName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CharNameExit(TObject *Sender)
{
   if (FileExists(CharName->Text + ".d2s") && CharName->Text != prev_name && Edited)
   {
      ShowMessage("A file with that name already exists.  Please select another name.");
      CharName->Text = prev_name;
   }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CharNameKeyPress(TObject *Sender, char &Key)
{
   if (!(toupper(Key) >= 'A' && toupper(Key) <= 'Z') && Key != '_' &&
       Key != VK_BACK)
      Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CharStatsChange(TObject *Sender)
{
   if (Sender == CharName && CharName->Modified)
   {
      CharName->Color = EDITED_COLOUR;
      JustOpened = false;
      NameChanged = true;
   }
   else if (Sender == CharDexterity && CharDexterity->Modified)
   {
      CharDexterity->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == CharEnergy && CharEnergy->Modified)
   {
      CharEnergy->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == CharLevel && CharLevel->Modified)
   {
      CharLevel->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == CharStrength && CharStrength->Modified)
   {
      CharStrength->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == CharVitality && CharVitality->Modified)
   {
      CharVitality->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == CurLife && CurLife->Modified)
   {
      CurLife->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == MaxLife && MaxLife->Modified)
   {
      MaxLife->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == CurMana && CurMana->Modified)
   {
      CurMana->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == MaxMana && MaxMana->Modified)
   {
      MaxMana->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == CurStamina && CurStamina->Modified)
   {
      CurStamina->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == MaxStamina && MaxStamina->Modified)
   {
      MaxStamina->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == Experience && Experience->Modified)
   {
      Experience->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == GoldInBelt && GoldInBelt->Modified)
   {
      GoldInBelt->Color = EDITED_COLOUR;
      JustOpened = false;
   }
   else if (Sender == GoldInStash && GoldInStash->Modified)
   {
      GoldInStash->Color = EDITED_COLOUR;
      JustOpened = false;
   }

   OthersChanged = true;
   StatsChanged();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CharStatsExitCheck(TObject *Sender)
{
   int value;

   if (Sender == CharDexterity && CharDexterity->Modified)
   {
      value = CharDexterity->Text.ToInt();

      if (value > MAX_BASICSTATS)
         CharDexterity->Text = MAX_BASICSTATS;
      else
         CharDexterity->Text = value;
   }
   else if (Sender == CharEnergy && CharEnergy->Modified)
   {
      value = CharEnergy->Text.ToInt();

      if (value > MAX_BASICSTATS)
         CharEnergy->Text = MAX_BASICSTATS;
      else
         CharEnergy->Text = value;
   }
   else if (Sender == CharLevel && CharLevel->Modified)
   {
      if (CharLevel->Text.ToIntDef(-1) <= 0)
         CharLevel->Text = CharInfo->getLevel();

      SetExperience();
      OthersChanged = true;
   }
   else if (Sender == CharStrength && CharStrength->Modified)
   {
      value = CharStrength->Text.ToInt();

      if (value > MAX_BASICSTATS)
         CharStrength->Text = MAX_BASICSTATS;
      else
         CharStrength->Text = value;
   }
   else if (Sender == CharVitality && CharVitality->Modified)
   {
      value = CharVitality->Text.ToInt();

      if (value > MAX_BASICSTATS)
         CharVitality->Text = MAX_BASICSTATS;
      else
         CharVitality->Text = value;
   }
   else if (Sender == CurLife && CurLife->Modified)
   {
      value = CurLife->Text.ToInt();

      if (value > MAX_LMS)
         CurLife->Text = MAX_LMS;
      else
         CurLife->Text = value;

      if (MaxLife->Text.ToInt() < CurLife->Text.ToInt())
      {
         MaxLife->Text = CurLife->Text;
         MaxLife->Color = EDITED_COLOUR;
      }
   }
   else if (Sender == MaxLife && MaxLife->Modified)
   {
      value = MaxLife->Text.ToInt();

      if (value > MAX_LMS)
         MaxLife->Text = MAX_LMS;
      else
         MaxLife->Text = value;

      if (CurLife->Text.ToInt() < MaxLife->Text.ToInt())
      {
         CurLife->Text = MaxLife->Text;
         CurLife->Color = EDITED_COLOUR;
      }
   }
   else if (Sender == CurMana && CurMana->Modified)
   {
      value = CurMana->Text.ToInt();

      if (value > MAX_LMS)
         CurMana->Text = MAX_LMS;
      else
         CurMana->Text = value;

      if (MaxMana->Text.ToInt() < CurMana->Text.ToInt())
      {
         MaxMana->Text = CurMana->Text;
         MaxMana->Color = EDITED_COLOUR;
      }
   }
   else if (Sender == MaxMana && MaxMana->Modified)
   {
      value = MaxMana->Text.ToInt();

      if (value > MAX_LMS)
         MaxMana->Text = MAX_LMS;
      else
         MaxMana->Text = value;

      if (CurMana->Text.ToInt() < MaxMana->Text.ToInt())
      {
         CurMana->Text = MaxMana->Text;
         CurMana->Color = EDITED_COLOUR;
      }
   }
   else if (Sender == CurStamina && CurStamina->Modified)
   {
      value = CurStamina->Text.ToInt();

      if (value > MAX_LMS)
         CurStamina->Text = MAX_LMS;
      else
         CurStamina->Text = value;

      if (MaxStamina->Text.ToInt() < CurStamina->Text.ToInt())
      {
         MaxStamina->Text = CurStamina->Text;
         MaxStamina->Color = EDITED_COLOUR;
      }
   }
   else if (Sender == MaxStamina && MaxStamina->Modified)
   {
      value = MaxStamina->Text.ToInt();

      if (value > MAX_LMS)
         MaxStamina->Text = MAX_LMS;
      else
         MaxStamina->Text = value;

      if (CurStamina->Text.ToInt() < MaxStamina->Text.ToInt())
      {
         CurStamina->Text = MaxStamina->Text;
         CurStamina->Color = EDITED_COLOUR;
      }
   }
   else if (Sender == Experience && Experience->Modified)
   {
      SetLevel();
      OthersChanged = true;
   }
   else if (Sender == GoldInBelt && GoldInBelt->Modified)
   {
      value = GoldInBelt->Text.ToInt();

      if (value > MAXGOLD)
         GoldInBelt->Text = MAXGOLD;
      else
         GoldInBelt->Text = value;
   }
   else if (Sender == GoldInStash && GoldInStash->Modified)
   {
      value = GoldInStash->Text.ToInt();

      if (value > MAXSTASHGOLD)
         GoldInStash->Text = MAXSTASHGOLD;
      else
         GoldInStash->Text = value;
   }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CharStatsKeyPress(TObject *Sender, char &Key)
{
   // only allow numerals
   if (!(Key >= '0' && Key <= '9') && !(Key == VK_BACK))
      Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CheckCharActionExecute(TObject *Sender)
{
   // only check if a character file has been opened
   // and the current stats have not been edited
   if (!CharInfo->isFileOpen() || Edited) return;

   // does a level-experience, level-gold, and max life/mana/stamina check
   if (Experience->Text.ToDouble() < MinExpRequired[CharLevel->Text.ToInt()-1])
   {
      if (MessageDlg("Insufficient experience based on current level.\n"
                 "Would you like experience changed to match your character's level?",
                 mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
      {
         Experience->Text = MinExpRequired[CharLevel->Text.ToInt()-1];
         Experience->Color = EDITED_COLOUR;
         JustOpened = false;
         OthersChanged = true;
      }
   }

   StatsEnter(GoldInBelt);
   if (GoldInBelt->Text.ToInt() > MAXGOLD)
   {
      if (MessageDlg("\"Gold In Belt\" amount exceeds the maximum limit.\n"
                 "Would you like the amount changed to match your character's level?",
                 mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
      {
         GoldInBelt->Text = MAXGOLD;
         GoldInBelt->Color = EDITED_COLOUR;
         JustOpened = false;
         OthersChanged = true;
      }
   }

   StatsEnter(GoldInStash);
   if (GoldInStash->Text.ToInt() > MAXSTASHGOLD)
   {
      if (MessageDlg("\"Gold In Stash\" amount exceeds the maximum limit.\n"
                 "Would you like the amount changed to match your character's level?",
                 mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
      {
         GoldInStash->Text = MAXSTASHGOLD;
         GoldInStash->Color = EDITED_COLOUR;
         JustOpened = false;
         OthersChanged = true;
      }
   }

   if (MaxLife->Text.ToInt() > MAX_LMS)
   {
      if (MessageDlg("\"Maximum Life\" amount exceeds the recommended maximum limit.\n"
                 "Would you like the amount changed to the recommended maximum limit?",
                 mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
      {
         MaxLife->Text = MAX_LMS;
         MaxLife->Color = EDITED_COLOUR;
         JustOpened = false;
         OthersChanged = true;
      }
   }

   if (MaxMana->Text.ToInt() > MAX_LMS)
   {
      if (MessageDlg("\"Maximum Mana\" amount exceeds the recommended maximum limit.\n"
                 "Would you like the amount changed to the recommended maximum limit?",
                 mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
      {
         MaxMana->Text = MAX_LMS;
         MaxMana->Color = EDITED_COLOUR;
         JustOpened = false;
         OthersChanged = true;
      }
   }

   if (MaxStamina->Text.ToInt() > MAX_LMS)
   {
      if (MessageDlg("\"Maximum Stamina\" amount exceeds the recommended maximum limit.\n"
                 "Would you like the amount changed to the recommended maximum limit?",
                 mtWarning, TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
      {
         MaxStamina->Text = MAX_LMS;
         MaxStamina->Color = EDITED_COLOUR;
         JustOpened = false;
         OthersChanged = true;
      }
   }

   if (OthersChanged)
      StatusBar->Panels->Items[0]->Text = "Character stats have been checked";
   else
      StatusBar->Panels->Items[0]->Text = "No discrepancies found";

   StatsChanged();
}
//---------------------------------------------------------------------------
void TMainForm::CheckName()
{
   int NumberOfUnderscores = 0;

   // find how many underscores there are
   for (int i = 1; i < CharName->Text.Length(); i++)
      if (CharName->Text[i] == '_')
         ++NumberOfUnderscores;

   // don't bother renaming if more than 1 underscore
   if (NumberOfUnderscores > 1)
   {
      CharName->Text = CharInfo->getName();
      NameChanged = false;
   }
}
//---------------------------------------------------------------------------
void TMainForm::ClearAllBoolVars()
{
   ActChanged = false;
   ClassChanged = false;
   Edited = false;
   ItemsChanged = false;
   JustOpened = true;
   NameChanged = false;
   OthersChanged = false;
   SkillChoicesChanged = false;
   StatusChanged = false;
   TitleChanged = false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ComboBoxChange(TObject *Sender)
{
   BasicStats bs;
   CharInfo->fillBasicStats(bs);

   if (Sender == CharClass)
   {
      if (CharClass->ItemIndex == bs.Class)
      {
         ClassChanged = false;
         CharClass->Color = NORMAL_COLOUR;
      }
      else
      {
         JustOpened = false;
         ClassChanged = true;
         CharClass->Color = EDITED_COLOUR;
      }
      UpdateTitleDisplay();
   }
   else if (Sender == CharStatus)
   {
      if ((StatusChanged = NewStateSelected()) == true)
      {
         JustOpened = false;
         CharStatus->Color = EDITED_COLOUR;
      }
      else
         CharStatus->Color = NORMAL_COLOUR;

      UpdateTitleDisplay();
   }
   else if (Sender == CharTitle)
   {
      if ((TitleChanged = NewTitleSelected()) == true)
      {
         JustOpened = false;
         CharTitle->Color = EDITED_COLOUR;
      }
      else
         CharTitle->Color = NORMAL_COLOUR;
   }
   else if (Sender == StartingAct)
   {
      if (StartingAct->ItemIndex == bs.StartingAct)
      {
         ActChanged = false;
         StartingAct->Color = NORMAL_COLOUR;
      }
      else
      {
         ActChanged = true;
         JustOpened = false;
         StartingAct->Color = EDITED_COLOUR;
      }
   }

   StatsChanged();
}
//---------------------------------------------------------------------------
/*
   Fills in all the components to display the character's stats
*/
void TMainForm::DisplayCharInfo()
{
   SetupBasicStats();

   if (CharInfo->getSkillChoices() > 0 && CharInfo->areSkillsMaxed())
      FixSkillButton->Enabled = true;
   else
      FixSkillButton->Enabled = false;

   UpgradeGemsButton->Enabled = CharInfo->anyNonPerfectGems();

   if (CharInfo->getNumberOfGems() > 0)
      GemConverterItem->Enabled = true;
   else
      GemConverterItem->Enabled = false;

   CharacterStats cs;
   CharInfo->fillCharacterStats(cs);

   CharLevel->Text = cs.Level;
   CharStrength->Text = cs.Strength;
   CharEnergy->Text = cs.Energy;
   CharDexterity->Text = cs.Dexterity;
   CharVitality->Text = cs.Vitality;

   CurLife->Text = cs.CurLife;
   MaxLife->Text = cs.MaxLife;
   CurMana->Text = cs.CurMana;
   MaxMana->Text = cs.MaxMana;
   CurStamina->Text = cs.CurStamina;
   MaxStamina->Text = cs.MaxStamina;

   Experience->Text = cs.Experience;
   GoldInBelt->Text = cs.GoldInBelt;
   GoldInStash->Text = cs.GoldInStash;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DisplayHint(TObject * Sender)
{
  StatusBar->Panels->Items[0]->Text = GetLongHint(Application->Hint);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditUndoActionExecute(TObject *Sender)
{
   CharInfo->read();
   DisplayCharInfo();
   ClearAllBoolVars();
   ResetColours();

   FileSaveAction->Enabled = false;
   EditUndoAction->Enabled = false;
   CheckCharAction->Enabled = true;

   StatusBar->Panels->Items[1]->Text = "";
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ExperienceEnter(TObject *Sender)
{
   prevExp = Experience->Text.ToDouble();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileCloseActionExecute(TObject *Sender)
{
   if (Edited)
      switch(GetCloseResponse())
      {
         case mrYes: FileSaveAction->Execute();
                     break;
         case mrCancel: return;
      }

   if (CharInfo)
   {
      delete CharInfo;
      CharInfo = NULL;
   }

   ClearAllBoolVars();
   FileSaveAction->Enabled = false;
   FileCloseAction->Enabled = false;

   EditUndoAction->Enabled = false;

   RestoreAction->Enabled = false;
   CheckCharAction->Enabled = false;
   GemConverterItem->Enabled = false;

   CharInfoBox->Enabled = false;
   CharInfoBox->Visible = false;

   MainForm->Caption = Application->Title;
   StatusBar->Panels->Items[0]->Text = "";
   StatusBar->Panels->Items[1]->Text = "";
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileExitActionExecute(TObject *Sender)
{
   Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileOpenActionExecute(TObject *Sender)
{
   if (OpenCharDlg->Execute())
   {
      // close any open files
      if (CharInfo && CharInfo->isFileOpen())
         FileCloseAction->Execute();

      // allocate memory for character to be loaded
      if (CharInfo == NULL)
         CharInfo = new Character();

      // return if open not successful
      if (!CharInfo->open(OpenCharDlg->FileName.c_str()))
      {
         int err = CharInfo->getErrorCode();

         if (err == INVALID_HEADER)
            MessageDlg("Not a valid Diablo 2 character file",
                       mtError, TMsgDlgButtons() << mbOK, 0);
         else if (err == CANNOT_OPEN_FILE)
            MessageDlg("Could not open " + OpenCharDlg->FileName + "\nVerify that file is not in use by another application.",
                       mtError, TMsgDlgButtons() << mbOK, 0);

         return;
      }

      FileCloseAction->Enabled = true;
      CheckCharAction->Enabled = true;
      CharInfoBox->Enabled = true;
      CharInfoBox->Visible = true;
      JustOpened = true;

      if (FileExists(ChangeFileExt(OpenCharDlg->FileName, ".bak")))
         RestoreAction->Enabled = true;

      UpdateAppTitle(OpenCharDlg->FileName);
      DisplayCharInfo();
   }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileSaveActionExecute(TObject *Sender)
{
   if (BackupChar)
      WriteBackupFile();

   StatsCheck();
   CharInfo->save();

   if (NameChanged)
      RenameCharacterFiles(CharName->Text);

   FileSaveAction->Enabled = false;
   EditUndoAction->Enabled = false;
   CheckCharAction->Enabled = true;

   ResetColours();
   ClearAllBoolVars();

   StatusBar->Panels->Items[0]->Text = "Character stats saved";
   StatusBar->Panels->Items[1]->Text = "";
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
   if (Edited)
      switch(GetCloseResponse())
      {
         case mrYes: FileSaveAction->Execute();
                     break;
         case mrCancel: Action = caNone;
                        return;
      }

   if (CharInfo)
   {
      delete CharInfo;
      CharInfo = NULL;
   }

   Action = caFree;
}
//---------------------------------------------------------------------------
int TMainForm::GetCloseResponse()
{
   return MessageDlg("Character has been modified.\nSave file?",
                        mtConfirmation,
                        TMsgDlgButtons() << mbYes << mbNo
                        << mbCancel, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::HelpAboutActionExecute(TObject *Sender)
{
   AboutBox->ShowModal();
}
//---------------------------------------------------------------------------
void TMainForm::Initialize()
{
   ClearAllBoolVars();
   ResetColours();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LevelInfoActionExecute(TObject *Sender)
{
   LevelInfoForm->Show();
}
//---------------------------------------------------------------------------
/*
   Returns true if the user selected a new character status
*/
bool TMainForm::NewStateSelected()
{
   switch(CharStatus->ItemIndex)
   {
      case 0: if (CharInfo->getStatus() == NoDeaths) return false;
              break;
      case 1: if (CharInfo->getStatus() == Hardcore) return false;
              break;
      case 2: if (CharInfo->getStatus() == Resurrected) return false;
              break;
      case 3: if (CharInfo->getStatus() == Dead) return false;
              break;
   }

   return true;
}
//---------------------------------------------------------------------------
/*
   Returns true if the user selected a new character title
*/
bool TMainForm::NewTitleSelected()
{
   char prevTitle = CharInfo->getTitle();
   char version = CharInfo->getVersion();

   switch(CharTitle->ItemIndex)
   {
      case 0: if ((version < VERSION_109 && prevTitle < SirDame) ||
                  (version >= VERSION_109 && prevTitle < Slayer))
                 return false;
              break;
      case 1: if ((version < VERSION_109 && prevTitle >= SirDame && prevTitle < LordLady) ||
                  (version >= VERSION_109 && prevTitle >= Slayer && prevTitle < Champion))
                 return false;
              break;
      case 2: if ((version < VERSION_109 && prevTitle >= LordLady && prevTitle < BaronBaroness) ||
                  (version >= VERSION_109 && prevTitle >= Champion && prevTitle < MPatriarch))
                 return false;
              break;
      case 3: if ((version < VERSION_109 && prevTitle >= BaronBaroness) ||
                  (version >= VERSION_109 && prevTitle == MPatriarch))
                 return false;
              break;
   }

   return true;
}
//---------------------------------------------------------------------------
void TMainForm::RenameCharacterFiles(const AnsiString& newName)
{
   AnsiString curName = ExtractFileName(OpenCharDlg->FileName);
   OpenCharDlg->FileName = ExtractFilePath(OpenCharDlg->FileName);

   // remove file extension
   curName.Delete(curName.Length()-3, 4);

   // do nothing if name hasn't changed
   if (newName.AnsiCompareIC(curName) == 0)
   {
      NameChanged = false;
      return;
   }

   // otherwise rename the other files that belong to this character
   curName += ".*";
   TSearchRec fileInfo;

   if (FindFirst(curName, faAnyFile, fileInfo) == 0)
   {
      do
      {
         if (!RenameFile(fileInfo.Name, newName + ExtractFileExt(fileInfo.Name)))
            MessageDlg("Problem renaming: " + fileInfo.Name,
                       mtError, TMsgDlgButtons() << mbOK, 0);
      }
      while(FindNext(fileInfo) == 0);

      FindClose(fileInfo);
   }

   UpdateAppTitle(newName + ".d2s");
   OpenCharDlg->FileName = OpenCharDlg->FileName + newName + ".d2s";
}
//---------------------------------------------------------------------------
void TMainForm::ResetColours()
{
   CharName->Color = NORMAL_COLOUR;
   CharClass->Color = NORMAL_COLOUR;
   CharStatus->Color = NORMAL_COLOUR;
   CharTitle->Color = NORMAL_COLOUR;

   StartingAct->Color = NORMAL_COLOUR;

   CharLevel->Color = NORMAL_COLOUR;
   CharStrength->Color = NORMAL_COLOUR;
   CharEnergy->Color = NORMAL_COLOUR;
   CharDexterity->Color = NORMAL_COLOUR;
   CharVitality->Color = NORMAL_COLOUR;

   CurLife->Color = NORMAL_COLOUR;
   MaxLife->Color = NORMAL_COLOUR;
   CurMana->Color = NORMAL_COLOUR;
   MaxMana->Color = NORMAL_COLOUR;
   CurStamina->Color = NORMAL_COLOUR;
   MaxStamina->Color = NORMAL_COLOUR;

   Experience->Color = NORMAL_COLOUR;
   GoldInBelt->Color = NORMAL_COLOUR;
   GoldInStash->Color = NORMAL_COLOUR;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::RestoreActionExecute(TObject *Sender)
{
   CharInfo->close();

   AnsiString origName = OpenCharDlg->FileName,
              backupName = ChangeFileExt(origName, ".bak");

   if (!DeleteFile(origName))
   {
      MessageDlg("Backup file does not exist", mtError,
                  TMsgDlgButtons() << mbOK, 0);
      return;
   }

   if (!RenameFile(backupName, origName))
   {
      MessageDlg("File could not be restored", mtError,
                  TMsgDlgButtons() << mbOK, 0);
      return;
   }

   CharInfo->open(origName.c_str());
   AnsiString oldname = CharInfo->getName(true);

   // rename character files if necessary
   origName = ExtractFileName(origName);
   origName.Delete(origName.Length()-3, 4);
   if (origName.AnsiCompareIC(oldname) != 0)
   {
      CharInfo->close();
      RenameCharacterFiles(oldname);
      CharInfo->open(/*(oldname + ".d2s")*/OpenCharDlg->FileName.c_str());
   }

   RestoreAction->Enabled = false;
   DisplayCharInfo();
}
//---------------------------------------------------------------------------
/*
   Called only when the "Max Everything" button is clicked
*/
void TMainForm::SetColours()
{
   CharLevel->Color = EDITED_COLOUR;

   CharStrength->Color = EDITED_COLOUR;
   CharEnergy->Color = EDITED_COLOUR;
   CharDexterity->Color = EDITED_COLOUR;
   CharVitality->Color = EDITED_COLOUR;

   CurLife->Color = EDITED_COLOUR;
   MaxLife->Color = EDITED_COLOUR;
   CurMana->Color = EDITED_COLOUR;
   MaxMana->Color = EDITED_COLOUR;
   CurStamina->Color = EDITED_COLOUR;
   MaxStamina->Color = EDITED_COLOUR;

   Experience->Color = EDITED_COLOUR;

   GoldInBelt->Color = EDITED_COLOUR;
   GoldInStash->Color = EDITED_COLOUR;
}
//---------------------------------------------------------------------------
/*
   Only called when character level has been changed and sets the character's
   experience to match required level if it has been increased
*/
void TMainForm::SetExperience()
{
   int curLevel;

   try
   {
      curLevel = CharLevel->Text.ToInt();
   }
   catch(const EConvertError&)
   {
      // if there was a problem converting the string to a value
      // then use the old level value
      CharLevel->Text = curLevel = CharInfo->getLevel();
   }

   if (curLevel != prevLevel)
   {
      Experience->Text = MinExpRequired[curLevel-1];
      Experience->Color = EDITED_COLOUR;
   }
}
//---------------------------------------------------------------------------
/*
   Changes the character's level based on experience if required
   called when the user changes the character's experience
*/
void TMainForm::SetLevel()
{
   double newExp;

   try
   {
      newExp = Experience->Text.ToDouble();
   }
   catch(const EConvertError&)
   {
      // if there was aa problem converting the string to a value
      // then use the old experience value
      Experience->Text = newExp = CharInfo->getExperience();
   }

   // set the new experience to the max limit if it is greater than the max limit
   if (newExp > MAX_EXPERIENCE)
      Experience->Text = MAX_EXPERIENCE;

   // otherwise updates level if experience has been reduced
   else if (newExp < prevExp)
   {
      int newLevel = CharLevel->Text.ToInt();

      // find the correct level
      while (newExp < MinExpRequired[newLevel-1])
         --newLevel;

      if (newLevel != CharLevel->Text.ToInt())
      {
         CharLevel->Text = newLevel;
         CharLevel->Color = EDITED_COLOUR;
      }
   }
}
//---------------------------------------------------------------------------
/*
   Looks in the Registry for the location of where Diablo II is installed
   and sets the Open Dialog to start in the "save" directory 
*/
void TMainForm::SetStartDir()
{
   TRegistry *Registry = new TRegistry;
   try
   {
      Registry->RootKey = HKEY_LOCAL_MACHINE;

      // make sure open dialog box uses Diablo II's save path if it exists
      if (Registry->OpenKeyReadOnly("\\Software\\Blizzard Entertainment\\Diablo II"))
         OpenCharDlg->InitialDir = Registry->ReadString("Save Path");

      // otherwise, make sure open dialog box starts in current directory
      else
         ForceCurrentDirectory = true;
   }
   __finally
   {
      delete Registry;
   }
}
//---------------------------------------------------------------------------
/*
   Fills in the components with the character's basic stats
*/
void TMainForm::SetupBasicStats()
{
   BasicStats bs;
   CharInfo->fillBasicStats(bs);

   // display character stats
   CharName->Text = bs.Name;

   // set state of Status checkbox
   switch(bs.Status)
   {
      case NoDeaths: CharStatus->ItemIndex = 0;
                     break;
      case Hardcore: CharStatus->ItemIndex = 1;
                     break;
      case Resurrected: CharStatus->ItemIndex = 2;
                        break;
      case Dead: CharStatus->ItemIndex = 3;
                 break;
   }

   if (!CharInfo->isExpansionCharacter())
   {
      // remove expansion set classes from combo box component
      CharClass->Items->Delete(CharClass->Items->IndexOf("Druid"));
      CharClass->Items->Delete(CharClass->Items->IndexOf("Assassin"));
   }
   else if (CharClass->Items->Count < NUM_OF_CLASSES)
   {
      // add the expansion set characters to combo box component
      CharClass->Items->Add("Druid");
      CharClass->Items->Add("Assassin");
   }

   CharClass->ItemIndex = bs.Class;
   UpdateTitleDisplay();

   switch(bs.DifficultyLastPlayed)
   {
      case NORMAL: Difficulty->Caption = "Normal";
                   break;
      case NIGHTMARE: Difficulty->Caption = "Nightmare";
                      break;
      case HELL: Difficulty->Caption = "Hell";
                 break;
   }

   if (!CharInfo->isExpansionCharacter())
      StartingAct->Items->Delete(4);
   else if (StartingAct->Items->Count < NUM_OF_ACTS)
      StartingAct->Items->Add("Act V -- Harrogath");

   StartingAct->ItemIndex = bs.StartingAct;
}
//---------------------------------------------------------------------------
void TMainForm::StatsChanged()
{
   if (!Edited && !JustOpened &&
       (ActChanged || ClassChanged || ItemsChanged ||
       NameChanged || OthersChanged || StatusChanged ||
       SkillChoicesChanged || TitleChanged || QuestsForm->isQuestsChanged() ||
       WaypointsForm->isWaypointsChanged()))
   {
      Edited = true;
      FileSaveAction->Enabled = true;
      EditUndoAction->Enabled = true;
      CheckCharAction->Enabled = false;
      StatusBar->Panels->Items[1]->Text = "*";
   }
}
//---------------------------------------------------------------------------
void TMainForm::StatsCheck()
{
   if (NameChanged)
      CheckName();

   // check character level
   if (CharLevel->Text.ToIntDef(-1) <= 0)
      CharLevel->Text = CharInfo->getLevel();

   if (CharLevel->Modified)
      SetExperience();

   if (CharDexterity->Modified &&  CharDexterity->Text.ToInt() > MAX_BASICSTATS)
      CharDexterity->Text = MAX_BASICSTATS;

   if (CharEnergy->Modified && CharEnergy->Text.ToInt() > MAX_BASICSTATS)
      CharEnergy->Text = MAX_BASICSTATS;

   if (CharStrength->Modified && CharStrength->Text.ToInt() > MAX_BASICSTATS)
      CharStrength->Text = MAX_BASICSTATS;

   if (CharVitality->Modified && CharVitality->Text.ToInt() > MAX_BASICSTATS)
      CharVitality->Text = MAX_BASICSTATS;

   if (MaxLife->Modified && MaxLife->Text.ToInt() > MAX_LMS)
      MaxLife->Text = MAX_LMS;

   if (CurLife->Text.ToInt() < MaxLife->Text.ToInt())
      CurLife->Text = MaxLife->Text;

   if (MaxMana->Modified && MaxMana->Text.ToInt() > MAX_LMS)
      MaxMana->Text = MAX_LMS;

   if (CurMana->Text.ToInt() < MaxMana->Text.ToInt())
      CurMana->Text = MaxMana->Text;

   if (MaxStamina->Modified && MaxStamina->Text.ToInt() > MAX_LMS)
      MaxStamina->Text = MAX_LMS;

   if (CurStamina->Text.ToInt() < MaxStamina->Text.ToInt())
      CurStamina->Text = MaxStamina->Text;

   StatsEnter(GoldInBelt);
   if (GoldInBelt->Text.ToInt() > MAXGOLD)
      GoldInBelt->Text = MAXGOLD;

   StatsEnter(GoldInStash);
   if (GoldInStash->Text.ToInt() > MAXSTASHGOLD)
      GoldInStash->Text = MAXSTASHGOLD;

   BasicStats bs;
   strcpy(bs.Name, CharName->Text.c_str());
   bs.Status = CharStatus->ItemIndex;
   bs.Title = CharTitle->ItemIndex;
   bs.Class = CharClass->ItemIndex;

   if (Difficulty->Caption.AnsiCompareIC("Normal") == 0)
      bs.DifficultyLastPlayed = NORMAL;
   else if (Difficulty->Caption.AnsiCompareIC("Nightmare") == 0)
      bs.DifficultyLastPlayed = NIGHTMARE;
   else if (Difficulty->Caption.AnsiCompareIC("Hell") == 0)
      bs.DifficultyLastPlayed = HELL;

   bs.StartingAct = StartingAct->ItemIndex;
   CharInfo->updateBasicStats(bs);

   CharacterStats cs;
   cs.Dexterity = CharDexterity->Text.ToInt();
   cs.Energy = CharEnergy->Text.ToInt();
   cs.Strength = CharStrength->Text.ToInt();
   cs.Vitality = CharVitality->Text.ToInt();

   cs.CurLife = CurLife->Text.ToInt();
   cs.MaxLife = MaxLife->Text.ToInt();
   cs.CurMana = CurMana->Text.ToInt();
   cs.MaxMana = MaxMana->Text.ToInt();
   cs.CurStamina = CurStamina->Text.ToInt();
   cs.MaxStamina = MaxStamina->Text.ToInt();

   cs.Level = CharLevel->Text.ToInt();
   cs.Experience = Experience->Text.ToDouble();
   cs.GoldInBelt = GoldInBelt->Text.ToInt();
   cs.GoldInStash = GoldInStash->Text.ToInt();

   CharInfo->updateCharacterStats(cs);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::StatsEnter(TObject *Sender)
{
   int curLevel = CharLevel->Text.ToInt();

   if (Sender == CharLevel)
      prevLevel = CharLevel->Text.ToInt();
   else if (Sender == GoldInBelt)
      MAXGOLD = curLevel * 10000;
   else if (Sender == GoldInStash)
   {
      if (curLevel >= 1 && curLevel <= 9)
         MAXSTASHGOLD = 50000;
      else if (curLevel >= 10 && curLevel <= 19)
         MAXSTASHGOLD = 100000;
      else if (curLevel >= 20 && curLevel <= 29)
         MAXSTASHGOLD = 150000;
      else if (CharInfo->getVersion() < 0x50)       // pre 1.07 character
      {
         if (curLevel >= 30 && curLevel <= 39)
            MAXSTASHGOLD = 200000;
         else if (curLevel >= 40 && curLevel <= 49)
            MAXSTASHGOLD = 250000;
         else if (curLevel >= 50 && curLevel <= 59)
            MAXSTASHGOLD = 300000;
         else if (curLevel >= 60 && curLevel <= 69)
            MAXSTASHGOLD = 350000;
         else if (curLevel >= 70 && curLevel <= 79)
            MAXSTASHGOLD = 400000;
         else if (curLevel >= 80 && curLevel <= 89)
            MAXSTASHGOLD = 450000;
         else if (curLevel >= 90 && curLevel <= 99)
            MAXSTASHGOLD = 500000;
      }
      else if (CharInfo->getVersion() >= 0x50)      // 1.07+ character
      {
         switch(curLevel)
         {
            case 30: MAXSTASHGOLD = 200000;
                     break;
            default: static long goldlimit = 2500000;
                     MAXSTASHGOLD = goldlimit -
                                    ((NUM_OF_LEVELS - curLevel) / 2) * 50000;
                     break;
         }
      }
   }
}
//---------------------------------------------------------------------------
void TMainForm::UpdateAppTitle(const AnsiString& filename)
{
   MainForm->Caption = Application->Title + " - "
                       + filename.SubString(filename.LastDelimiter("\\")+1,
                       filename.Length());
}
//---------------------------------------------------------------------------
/*
   Makes sure the correct gender titles are displayed based on the
   character's class
*/
void TMainForm::UpdateTitleDisplay()
{
   CharTitle->Items->BeginUpdate();
   CharTitle->Items->Clear();

   if (CharStatus->ItemIndex == 1)
   {
      for (int i = 0; i < NUM_OF_TITLES; ++i)
         CharTitle->Items->Add(AnsiString(HardcoreExpansionTitle[i]));
   }
   else
      switch(CharClass->ItemIndex)
      {
         case Amazon:
         case Assassin:
         case Sorceress: // add titles for female characters
                         if (CharInfo->isExpansionCharacter())
                            for (int i = 0; i < NUM_OF_TITLES; ++i)
                               CharTitle->Items->Add(AnsiString(FemaleExpansionTitle[i]));
                         else
                            for (int i = 0; i < NUM_OF_TITLES; ++i)
                               CharTitle->Items->Add(AnsiString(FemaleTitle[i]));

                         break;

         case Barbarian:
         case Druid:
         case Necromancer:
         case Paladin: // add titles for male characters
                       if (CharInfo->isExpansionCharacter())
                          for (int i = 0; i < NUM_OF_TITLES; i++)
                             CharTitle->Items->Add(AnsiString(MaleExpansionTitle[i]));
                       else
                          for (int i = 0; i < NUM_OF_TITLES; i++)
                             CharTitle->Items->Add(AnsiString(MaleTitle[i]));

                       break;
      }

   CharTitle->Items->EndUpdate();

   switch(CharInfo->getTitle())
   {
      case None: CharTitle->ItemIndex = 0;
                 break;
      case SirDame:
      case Slayer: CharTitle->ItemIndex = 1;
                   break;
      case LordLady:
      case Champion: CharTitle->ItemIndex = 2;
                     break;
      case BaronBaroness:
      case MPatriarch: CharTitle->ItemIndex = 3;
                       break;
   }
}
//---------------------------------------------------------------------------
void TMainForm::UpgradeGems()
{
   CharInfo->upgradeGems();
   UpgradeGemsButton->Enabled = false;
   StatusBar->Panels->Items[0]->Text = "All gems have been upgraded to perfect state";

   ItemsChanged = true;
   JustOpened = false;
   StatsChanged();
}
//---------------------------------------------------------------------------
/*
   Makes a backup copy of the character file and overwrites any
   existing backup file
*/
void TMainForm::WriteBackupFile()
{
   AnsiString backupname = ChangeFileExt(OpenCharDlg->FileName, ".bak");
   CopyFile(OpenCharDlg->FileName.c_str(), backupname.c_str(), false);
   RestoreAction->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::GemConverterItemClick(TObject *Sender)
{
   ConvertGemsForm->ShowModal();
}
//---------------------------------------------------------------------------

