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

#include "D2SkillTreeForm.h"
#include "D2MainForm.h"
#include "SkillConstants.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSkillTrees *SkillTrees;
//---------------------------------------------------------------------------
__fastcall TSkillTrees::TSkillTrees(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TSkillTrees::AssignValues()
{
   AnsiString temp;
   SkillsList->Clear();

   for (int i = 0; i < NUM_OF_SKILLS; i++)
   {
      temp = SkillsNames[MainForm->CharClass->ItemIndex][i];
      temp += "=";
      temp += i;
      SkillsList->Add(temp);
   }
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::ButtonClick(TObject *Sender)
{
   if (Sender == CloseButton)
   {
      if (SkillsChanged)
      {
         MainForm->CharInfo->updateSkills(Skills);
         SkillsChanged = false;
         MainForm->JustOpened = false;
         MainForm->OthersChanged = true;
         MainForm->StatsChanged();
      }

      Close();
   }
   else if (Sender == GoButton)
   {
      memset(Skills, SkillValue->Text.ToInt(), NUM_OF_SKILLS);

      SkillsChanged = true;

      AssignValues();
      SetGrid();
      SetTabs();
   }
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::FormCreate(TObject *Sender)
{
   SkillsGrid->Cells[0][0] = "SKILL";
   SkillsGrid->Cells[1][0] = "VALUE";
   SkillsGrid->ColWidths[1] = 58;

   TabSkillsGrid1->Cells[0][0] = "SKILL (sorted by level)";
   TabSkillsGrid1->Cells[1][0] = "VALUE";
   TabSkillsGrid1->ColWidths[1] = 66;

   TabSkillsGrid2->Cells[0][0] = "SKILL (sorted by level)";
   TabSkillsGrid2->Cells[1][0] = "VALUE";
   TabSkillsGrid2->ColWidths[1] = 66;

   TabSkillsGrid3->Cells[0][0] = "SKILL (sorted by level)";
   TabSkillsGrid3->Cells[1][0] = "VALUE";
   TabSkillsGrid3->ColWidths[1] = 66;

   SkillsChanged = false;

   SkillsList = new TStringList;
   SkillsList->Sorted = true;
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::FormDestroy(TObject *Sender)
{
   delete SkillsList;
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::FormShow(TObject *Sender)
{
   ViewOption->ItemIndex = 0;
   TabPageCtrl->ActivePage = SkillsTab1;
   SkillValue->Text = "";

   Skills = MainForm->CharInfo->getSkills();
   AssignValues();
   SetGrid();
   SetTabs();
}
//---------------------------------------------------------------------------
void TSkillTrees::SetGrid()
{
   for (int i = 0; i < SkillsGrid->RowCount-1; ++i)
   {
      SkillsGrid->Cells[0][i+1] = SkillsList->Names[i];
      SkillsGrid->Cells[1][i+1] = IntToStr(Skills[SkillsList->Values[
                                         SkillsList->Names[i]].ToInt()]);
   }
}
//---------------------------------------------------------------------------
void TSkillTrees::SetTabs()
{
   SkillsTab1->Caption = TabNames[MainForm->CharClass->ItemIndex][0];
   SkillsTab2->Caption = TabNames[MainForm->CharClass->ItemIndex][1];
   SkillsTab3->Caption = TabNames[MainForm->CharClass->ItemIndex][2];

   // fill all 10 cells for each skill tab
   for (int i = 0; i < 10; i++)
   {
      TabSkillsGrid1->Cells[0][i+1] = TabSkills[MainForm->CharClass->ItemIndex][i];
      TabSkillsGrid1->Cells[1][i+1] = IntToStr(Skills[TabSkillPos[
                                             MainForm->CharClass->ItemIndex][i]]);

      TabSkillsGrid2->Cells[0][i+1] = TabSkills[MainForm->CharClass->ItemIndex][i+10];
      TabSkillsGrid2->Cells[1][i+1] = IntToStr(Skills[TabSkillPos[
                                             MainForm->CharClass->ItemIndex][i+10]]);

      TabSkillsGrid3->Cells[0][i+1] = TabSkills[MainForm->CharClass->ItemIndex][i+20];
      TabSkillsGrid3->Cells[1][i+1] = IntToStr(Skills[TabSkillPos[
                                             MainForm->CharClass->ItemIndex][i+20]]);
   }
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::SkillsGridGetEditText(TObject *Sender,
      int ACol, int ARow, AnsiString &Value)
{
   Value = Skills[SkillsList->Values[SkillsList->Names[ARow-1]].ToInt()];
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::ComponentKeyPress(TObject *Sender, char &Key)
{
   // only allow numerals
   if (!(Key >= '0' && Key <= '9') && !(Key == VK_BACK))
      Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::SkillsGridSetEditText(TObject *Sender,
      int ACol, int ARow, const AnsiString Value)
{
   int skillValue;
   try
   {
      skillValue = Value.ToInt();
   }
   catch(const EConvertError& e)
   {
      skillValue = Skills[SkillsList->Values[SkillsList->Names[ARow-1]].ToInt()];
   }

      if (skillValue > MAX_SKILL_VALUE)
         skillValue = MAX_SKILL_VALUE;

      // value actually changed?
      if (skillValue != Skills[SkillsList->Values[SkillsList->Names[ARow-1]].ToInt()])
      {
         Skills[SkillsList->Values[SkillsList->Names[ARow-1]].ToInt()] = skillValue;
         SkillsChanged = true;
      }
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::SkillValueExit(TObject *Sender)
{
   if (SkillValue->Text.IsEmpty())
      return;
   else if (SkillValue->Text.ToInt() > MAX_SKILL_VALUE)
      SkillValue->Text = MAX_SKILL_VALUE;
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::TabSkillsGridGetEditText(TObject *Sender,
      int ACol, int ARow, AnsiString &Value)
{
   if (Sender == TabSkillsGrid1)
      Value = Skills[TabSkillPos[MainForm->CharClass->ItemIndex][ARow-1]];
   else if (Sender == TabSkillsGrid2)
      Value = Skills[TabSkillPos[MainForm->CharClass->ItemIndex][ARow+10-1]];
   else if (Sender== TabSkillsGrid3)
      Value = Skills[TabSkillPos[MainForm->CharClass->ItemIndex][ARow+20-1]];
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::TabSkillsGridSetEditText(TObject *Sender,
      int ACol, int ARow, const AnsiString Value)
{
   int Pos, skillValue;

   // find correct position in Skill array
   if (Sender == TabSkillsGrid1)
      Pos = TabSkillPos[MainForm->CharClass->ItemIndex][ARow-1];
   else if (Sender == TabSkillsGrid2)
      Pos = TabSkillPos[MainForm->CharClass->ItemIndex][ARow+10-1];
   else if (Sender == TabSkillsGrid3)
      Pos = TabSkillPos[MainForm->CharClass->ItemIndex][ARow+20-1];

   try
   {
      skillValue = Value.ToInt();
   }
   catch(const EConvertError& e)
   {
      skillValue = Skills[Pos];
   }

   if (skillValue > MAX_SKILL_VALUE)
      skillValue = MAX_SKILL_VALUE;

   // value actually changed?
   if (skillValue != Skills[Pos])
   {
      Skills[Pos] = skillValue;
      SkillsChanged = true;
   }
}
//---------------------------------------------------------------------------
void __fastcall TSkillTrees::ViewOptionClick(TObject *Sender)
{
   switch(ViewOption->ItemIndex)
   {
      // view all
      case 0: TabPageCtrl->Enabled = false;
              TabPageCtrl->Visible = false;
              SkillsGrid->Enabled = true;
              SkillsGrid->Visible = true;
              SetGrid();
              break;

      // view by type
      case 1: SkillsGrid->Enabled = false;
              SkillsGrid->Visible = false;
              TabPageCtrl->Enabled = true;
              TabPageCtrl->Visible = true;
              SetTabs();
              break;
   }
}
//---------------------------------------------------------------------------

