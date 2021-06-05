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

#include "D2QuestsForm.h"
#include "D2MainForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TQuestsForm *QuestsForm;
//---------------------------------------------------------------------------
__fastcall TQuestsForm::TQuestsForm(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TQuestsForm::ActStatusEnter(TObject *Sender)
{
   actStatus_entered = true;
}
//---------------------------------------------------------------------------
void __fastcall TQuestsForm::ActStatusExit(TObject *Sender)
{
   actStatus_entered = false;
}
//---------------------------------------------------------------------------
void __fastcall TQuestsForm::ButtonClick(TObject *Sender)
{
   if (Sender == CloseButton)
   {
      if (NormalChanged || NightmareChanged || HellChanged)
      {
         MainForm->CharInfo->updateQuests(qi);
         MainForm->JustOpened = false;
         MainForm->StatsChanged();
      }

      Close();
   }
   else if (Sender == CompleteAllButton)
   {
      CompleteAllQuests();

      A1Q1->ItemIndex = 2;
      A1Q2->ItemIndex = 2;
      A1Q3->ItemIndex = 2;
      A1Q4->ItemIndex = 2;
      A1Q5->ItemIndex = 3;
      A1Q6->ItemIndex = 2;

      A2Q1->ItemIndex = 2;
      A2Q2->ItemIndex = 2;
      A2Q3->ItemIndex = 2;
      A2Q4->ItemIndex = 2;
      A2Q5->ItemIndex = 2;
      A2Q6->ItemIndex = 2;
      Act2Status->ItemIndex = 1;

      A3Q1->ItemIndex = 2;
      A3Q2->ItemIndex = 2;
      A3Q3->ItemIndex = 2;
      A3Q4->ItemIndex = 2;
      A3Q5->ItemIndex = 2;
      A3Q6->ItemIndex = 2;
      Act3Status->ItemIndex = 1;

      A4Q1->ItemIndex = 2;
      A4Q2->ItemIndex = 2;
      A4Q3->ItemIndex = 2;
      Act4Status->ItemIndex = 1;

      if (MainForm->CharInfo->isExpansionCharacter())
      {
         A5Q1->ItemIndex = 3;
         A5Q2->ItemIndex = 3;
         A5Q3->ItemIndex = 3;
         A5Q4->ItemIndex = 3;
         A5Q5->ItemIndex = 2;
         A5Q6->ItemIndex = 2;
         Act5Status->ItemIndex = 1;
      }

      UpdateQuestValues();

      switch(Difficulty->ItemIndex)
      {
         case 0: NormalChanged = true;
                 break;
         case 1: NightmareChanged = true;
                 break;
         case 2: HellChanged = true;
                 break;
      }
   }
}
//---------------------------------------------------------------------------
void TQuestsForm::CompleteAllQuests()
{
   for (int j = 0; j < NUM_OF_ACTS; ++j)
   {
      if ((j == NUM_OF_ACTS-1) && !MainForm->CharInfo->isExpansionCharacter())
         break;

      qi[Difficulty->ItemIndex].Completed[j] = 1;
      for (int k = 0; k < NUM_OF_QUESTS; ++k)
         qi[Difficulty->ItemIndex].Quests[j][k] |= Completed;
   }
}
//---------------------------------------------------------------------------
void __fastcall TQuestsForm::FormShow(TObject *Sender)
{
   // if expansion set character, activate Act 5 tab
   Act5Tab->TabVisible = MainForm->CharInfo->isExpansionCharacter();

   qi = MainForm->CharInfo->getQuests();

   actStatus_entered = false;
   NormalChanged = NightmareChanged = HellChanged = false;

   // initialize questStatus array
   memset(questStatus, 0, sizeof(questStatus));

   QuestsPage->ActivePageIndex = 0;
   Difficulty->ItemIndex = 0;

   UpdatePage(QuestsPage->ActivePageIndex);
}
//---------------------------------------------------------------------------
bool TQuestsForm::isQuestsChanged() const
{
   return (NormalChanged || NightmareChanged || HellChanged);
}
//---------------------------------------------------------------------------
void __fastcall TQuestsForm::QuestsChange(TObject *Sender)
{
   switch(QuestsPage->ActivePageIndex)
   {
      // act 1
      case 0: if (Sender == A1Q1)
                 switch(A1Q1->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[0][0] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[0][0] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[0][0] = Completed;
                            break;
                 }
              else if (Sender == A1Q2)
                 switch(A1Q2->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[0][1] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[0][1] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[0][1] = Completed;
                            break;
                 }
              else if (Sender == A1Q3)
                 switch(A1Q3->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[0][3] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[0][3] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[0][3] = Completed;
                            break;
                 }
              else if (Sender == A1Q4)
                 switch(A1Q4->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[0][4] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[0][4] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[0][4] = Completed;
                            break;
                 }
              else if (Sender == A1Q5)
                 switch(A1Q5->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[0][2] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[0][2] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[0][2] = Imbue;
                            break;
                    case 3: qi[Difficulty->ItemIndex].Quests[0][2] = Completed;
                            break;
                 }
              else if (Sender == A1Q6)
                 switch(A1Q6->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[0][5] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[0][5] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[0][5] = Completed;
                            break;
                 }
              break;

      // act 2
      case 1: if (Sender == A2Q1)
                 switch(A2Q1->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[1][0] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[1][0] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[1][0] = Completed;
                            break;
                 }
              else if (Sender == A2Q2)
                 switch(A2Q2->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[1][1] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[1][1] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[1][1] = Completed;
                            break;
                 }
              else if (Sender == A2Q3)
                 switch(A2Q3->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[1][2] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[1][2] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[1][2] = Completed;
                            break;
                 }
              else if (Sender == A2Q4)
                 switch(A2Q4->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[1][3] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[1][3] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[1][3] = Completed;
                            break;
                 }
              else if (Sender == A2Q5)
                 switch(A2Q5->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[1][4] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[1][4] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[1][4] = Completed;
                            break;
                 }
              else if (Sender == A2Q6)
                 switch(A2Q6->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[1][5] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[1][5] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[1][5] = Completed;
                            break;
                 }
              break;

      // act 3
      case 2: if (Sender == A3Q1)
                 switch(A3Q1->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[2][3] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[2][3] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[2][3] = Completed;
                            break;
                 }
              else if (Sender == A3Q2)
                 switch(A3Q2->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[2][2] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[2][2] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[2][2] = Completed;
                            break;
                 }
              else if (Sender == A3Q3)
                 switch(A3Q3->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[2][1] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[2][1] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[2][1] = Completed;
                            break;
                 }
              else if (Sender == A3Q4)
                 switch(A3Q4->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[2][0] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[2][0] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[2][0] = Completed;
                            break;
                 }
              else if (Sender == A3Q5)
                 switch(A3Q5->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[2][4] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[2][4] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[2][4] = Completed;
                            break;
                 }
              else if (Sender == A3Q6)
                 switch(A3Q6->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[2][5] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[2][5] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[2][5] = Completed;
                            break;
                 }
              break;

      // act 4
      case 3: if (Sender == A4Q1)
                 switch(A4Q1->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[3][0] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[3][0] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[3][0] = Completed;
                            break;
                 }
              else if (Sender == A4Q2)
                 switch(A4Q2->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[3][2] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[3][2] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[3][2] = Completed;
                            break;
                 }
              else if (Sender == A4Q3)
                 switch(A4Q3->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[3][1] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[3][1] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[3][1] = Completed;
                            break;
                 }
              break;

      // act 5
      case 4: if (Sender == A5Q1)
                 switch(A5Q1->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[4][0] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[4][0] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[4][0] = AddSocket;
                            break;
                    case 3: qi[Difficulty->ItemIndex].Quests[4][0] = Completed;
                            break;
                 }
              else if (Sender == A5Q2)
                 switch(A5Q2->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[4][1] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[4][1] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[4][1] = Runes;
                            break;
                    case 3: qi[Difficulty->ItemIndex].Quests[4][1] = Completed;
                            break;
                 }
              else if (Sender == A5Q3)
                 switch(A5Q3->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[4][2] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[4][2] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[4][2] = Scroll;
                            break;
                    case 3: qi[Difficulty->ItemIndex].Quests[4][2] = Completed;
                            break;
                 }
              else if (Sender == A5Q4)
                 switch(A5Q4->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[4][3] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[4][3] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[4][3] = Personalize;
                            break;
                    case 3: qi[Difficulty->ItemIndex].Quests[4][3] = Completed;
                            break;
                 }
              else if (Sender == A5Q5)
                 switch(A5Q5->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[4][4] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[4][4] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[4][4] = Completed;
                            break;
                 }
              else if (Sender == A5Q6)
                 switch(A5Q6->ItemIndex)
                 {
                    case 0: qi[Difficulty->ItemIndex].Quests[4][5] = NotStarted;
                            break;
                    case 1: qi[Difficulty->ItemIndex].Quests[4][5] = Started;
                            break;
                    case 2: qi[Difficulty->ItemIndex].Quests[4][5] = Completed;
                            break;
                 }
              break;
   }

   switch(Difficulty->ItemIndex)
   {
      case 0: NormalChanged = true;
              break;
      case 1: NightmareChanged = true;
              break;
      case 2: HellChanged = true;
              break;
   }
}
//---------------------------------------------------------------------------
void __fastcall TQuestsForm::QuestsPageChange(TObject *Sender)
{
   UpdatePage(QuestsPage->ActivePageIndex);
}
//---------------------------------------------------------------------------
// this function updates the combo boxes when the difficulty has been changed
void __fastcall TQuestsForm::RadioClick(TObject *Sender)
{
   if (Sender == Act2Status || Sender == Act3Status || Sender == Act4Status
       || Sender == Act5Status)
   {
      if (!actStatus_entered) return;

      switch(QuestsPage->ActivePageIndex)
      {
         case 1: qi[Difficulty->ItemIndex].Completed[0] = Act2Status->ItemIndex;
                 break;
         case 2: qi[Difficulty->ItemIndex].Completed[1] = Act3Status->ItemIndex;
                 break;
         case 3: qi[Difficulty->ItemIndex].Completed[2] = Act4Status->ItemIndex;
                 break;
         case 4: qi[Difficulty->ItemIndex].Completed[3] = Act5Status->ItemIndex;
                 break;
      }

      switch(Difficulty->ItemIndex)
      {
         case 0: NormalChanged = true;
                 break;
         case 1: NightmareChanged = true;
                 break;
         case 2: HellChanged = true;
                 break;
      }
   }
   else if (Sender == Difficulty)
      UpdatePage(QuestsPage->ActivePageIndex);
}
//---------------------------------------------------------------------------
// makes sure the combo boxes are displaying the right values
void __fastcall TQuestsForm::UpdatePage(int currentAct)
{
   UpdateQuestValues();

   switch(currentAct)
   {
      case 0: A1Q1->ItemIndex = questStatus[0][0];
              A1Q2->ItemIndex = questStatus[0][1];
              A1Q3->ItemIndex = questStatus[0][3];
              A1Q4->ItemIndex = questStatus[0][4];
              A1Q5->ItemIndex = questStatus[0][2];
              A1Q6->ItemIndex = questStatus[0][5];
              break;

      case 1: A2Q1->ItemIndex = questStatus[1][0];
              A2Q2->ItemIndex = questStatus[1][1];
              A2Q3->ItemIndex = questStatus[1][2];
              A2Q4->ItemIndex = questStatus[1][3];
              A2Q5->ItemIndex = questStatus[1][4];
              A2Q6->ItemIndex = questStatus[1][5];
              Act2Status->ItemIndex = qi[Difficulty->ItemIndex].Completed[0];
              break;

      case 2: A3Q1->ItemIndex = questStatus[2][3];
              A3Q2->ItemIndex = questStatus[2][2];
              A3Q3->ItemIndex = questStatus[2][1];
              A3Q4->ItemIndex = questStatus[2][0];
              A3Q5->ItemIndex = questStatus[2][4];
              A3Q6->ItemIndex = questStatus[2][5];
              Act3Status->ItemIndex = qi[Difficulty->ItemIndex].Completed[1];
              break;

      case 3: A4Q1->ItemIndex = questStatus[3][0];
              A4Q2->ItemIndex = questStatus[3][2];
              A4Q3->ItemIndex = questStatus[3][1];
              Act4Status->ItemIndex = qi[Difficulty->ItemIndex].Completed[2];
              break;

      case 4: A5Q1->ItemIndex = questStatus[4][0];
              A5Q2->ItemIndex = questStatus[4][1];
              A5Q3->ItemIndex = questStatus[4][2];
              A5Q4->ItemIndex = questStatus[4][3];
              A5Q5->ItemIndex = questStatus[4][4];
              A5Q6->ItemIndex = questStatus[4][5];
              Act5Status->ItemIndex = qi[Difficulty->ItemIndex].Completed[3];
              break;
   }
}
//---------------------------------------------------------------------------
void TQuestsForm::UpdateQuestValues()
{
   for (int j = 0; j < NUM_OF_QUESTS; j++)
   {
      // skip unnecessary processing of quests for act 4
      if (QuestsPage->ActivePageIndex == 3 && j > 2) break;

      // not started
      if (qi[Difficulty->ItemIndex].Quests[QuestsPage->ActivePageIndex][j] == 0)
         questStatus[QuestsPage->ActivePageIndex][j] = 0;
      // started / in progress
      else if ((qi[Difficulty->ItemIndex].Quests[QuestsPage->ActivePageIndex][j] & 0x0001) == 0)
         questStatus[QuestsPage->ActivePageIndex][j] = 1;
      // completed
      else if ((qi[Difficulty->ItemIndex].Quests[QuestsPage->ActivePageIndex][j] & 0x0001) == 1)
         questStatus[QuestsPage->ActivePageIndex][j] = 2;
   }

   // special cases
   // act 1, quest 5: imbue
   if ((qi[Difficulty->ItemIndex].Quests[0][2] & 0x000F) == 0x000E)
      questStatus[0][2] = 2;
   // act 1, quest 5: completed
   else if ((qi[Difficulty->ItemIndex].Quests[0][2] & 0x0001) == 1)
      questStatus[0][2] = 3;

   // act 5, quest 1: add sockets
   if ((qi[Difficulty->ItemIndex].Quests[4][0] & 0x00FF) == AddSocket)
      questStatus[4][0] = 2;
   // act 5, quest 1: completed
   else if ((qi[Difficulty->ItemIndex].Quests[4][0] & 0x0001) == 0x0001)
      questStatus[4][0] = 3;

   // act 5, quest 2: rune reward
   if (MainForm->CharInfo->maskMSB(qi[Difficulty->ItemIndex].Quests[4][1]) == Runes)
      questStatus[4][1] = 2;
   // act 5, quest 3: completed
   else if ((qi[Difficulty->ItemIndex].Quests[4][1] & 0x0001) == 0x0001)
      questStatus[4][1] = 3;

   // act 5, quest 3: scroll of resistance
   if ((qi[Difficulty->ItemIndex].Quests[4][0] & 0x0008) == Scroll)
      questStatus[4][2] = 2;
   // act 5, quest 3: completed
   else if ((qi[Difficulty->ItemIndex].Quests[4][0] & 0x0001) == 0x0001)
      questStatus[4][2] = 3;

   // act 5, quest 4: personalize item
   if ((qi[Difficulty->ItemIndex].Quests[4][3] & 0x00FF) == Personalize)
      questStatus[4][3] = 2;
   // act 5, quest 4: completed
   else if ((qi[Difficulty->ItemIndex].Quests[4][3] & 0x0001) == 0x0001)
      questStatus[4][3] = 3;
}
//---------------------------------------------------------------------------

