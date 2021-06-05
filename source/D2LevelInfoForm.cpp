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
#include "D2LevelInfoForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TLevelInfoForm *LevelInfoForm;
//---------------------------------------------------------------------------
__fastcall TLevelInfoForm::TLevelInfoForm(TComponent* Owner)
   : TForm(Owner)
{
   LevelInfoGrid->Cells[0][0] = "LEVEL";
   LevelInfoGrid->ColWidths[0] = 46;

   LevelInfoGrid->Cells[1][0] = "MIN EXP REQ";
   LevelInfoGrid->ColWidths[1] = 90;

   LevelInfoGrid->Cells[2][0] = "BELT MAX GOLD";
   LevelInfoGrid->ColWidths[2] = 110;

   LevelInfoGrid->Cells[3][0] = "STASH MAX GOLD";
   LevelInfoGrid->ColWidths[3] = 122;

   FillArrays();
   FillCells();
}
//---------------------------------------------------------------------------
void __fastcall TLevelInfoForm::CloseButtonClick(TObject *Sender)
{
   Close();
}
//---------------------------------------------------------------------------
void __fastcall TLevelInfoForm::D2VersionClick(TObject *Sender)
{
   FillArrays();
   FillCells();
}
//---------------------------------------------------------------------------
void TLevelInfoForm::FillArrays()
{
   int goldValue = 50000;

   for (int i = 1; i <= NUM_OF_LEVELS; ++i)
   {
      // store gold values
      MaxBeltGold[i-1] = i * 10000;

      if (i > 30 && D2Version->ItemIndex == 1)
      {
         long goldlimit = 2500000;
         goldValue = goldlimit - std::floor(static_cast<double>((NUM_OF_LEVELS - i) / 2)) * 50000;
      }
      else if (i > 9 && (i % 10 == 0))
         goldValue += 50000;

      MaxStashGold[i-1] = goldValue;

      // store experience values
      Exp[i-1] = MinExpRequired[i-1];
   }
}
//---------------------------------------------------------------------------
void TLevelInfoForm::FillCells()
{
   FormatStrArray(Exp);
   FormatStrArray(MaxBeltGold);
   FormatStrArray(MaxStashGold);

   for (int i = 1; i < LevelInfoGrid->RowCount; ++i)
   {
      LevelInfoGrid->Cells[0][i] = i;
      LevelInfoGrid->Cells[1][i] = Exp[i-1];
      LevelInfoGrid->Cells[2][i] = MaxBeltGold[i-1];
      LevelInfoGrid->Cells[3][i] = MaxStashGold[i-1];
   }
}
//---------------------------------------------------------------------------
void __fastcall TLevelInfoForm::FormatStrArray(AnsiString *strArray)
{
   int strLength;

   for (int i = 0; i < NUM_OF_LEVELS; ++i)
   {
      strLength = strArray[i].Length();

      while(strLength > 3)
      {
         strLength -= 3;
         strArray[i] = strArray[i].Insert(",", strLength+1);
      }
   }
}
//---------------------------------------------------------------------------

