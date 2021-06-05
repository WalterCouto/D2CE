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

#include "D2GemsForm.h"
#include "D2MainForm.h"
//#include "RuneConstants.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConvertGemsForm *ConvertGemsForm;
//---------------------------------------------------------------------------
__fastcall TConvertGemsForm::TConvertGemsForm(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TConvertGemsForm::CloseButtonClick(TObject *Sender)
{
   Close();
}
//---------------------------------------------------------------------------
void __fastcall TConvertGemsForm::ConvertButtonClick(TObject *Sender)
{
   if (OldGem->ItemIndex == NewGem->ItemIndex)
      return;

   if (OldGem->ItemIndex < 0 || NewGem->ItemIndex < 0)
      return;

   char oldgem, newgem;
   GemTypes gt;

   if (OldGem->ItemIndex < 35)
   {
      gt.oldgem = (OldGem->ItemIndex / 7);
      gt.oldgem <<= 4;
      gt.oldgem += (OldGem->ItemIndex % 7);
      gt.original = -1;
   }
   else
   {
      gt.original = OldGem->ItemIndex - 34;
      gt.oldgem = -1;   // set to -1 to indicate that function should read original field
      gt.oldname = OldGem->Text.c_str();
   }

   if (NewGem->ItemIndex < 35)
   {
      gt.newgem = (NewGem->ItemIndex / 7);
      gt.newgem <<= 4;
      gt.newgem += (NewGem->ItemIndex % 7);
      gt.desired = -1;
   }
   else
   {
      gt.desired = NewGem->ItemIndex - 34;
      gt.newgem = -1;   // set to -1 to indicate that function should read desired field
      gt.newname = NewGem->Text.c_str();
   }

   int total = MainForm->CharInfo->convertGems(gt);
   GemStatusBar->SimpleText = "";
   GemStatusBar->SimpleText = AnsiString(total) + " item(s) converted";

   if (total > 0)
   {
      MainForm->OthersChanged = true;
      MainForm->JustOpened = false;
      MainForm->StatsChanged();
   }
}
//---------------------------------------------------------------------------
void __fastcall TConvertGemsForm::FormShow(TObject *Sender)
{
   MainForm->OthersChanged = false;
/*
   OldGem->Items->BeginUpdate();
   NewGem->Items->BeginUpdate();

   if (MainForm->CharInfo->isExpansionCharacter())
   {
      if (OldGem->Items->Count == 35)
         for (int i = 0; i < 33; ++i)
         {
            OldGem->Items->Add(runes[i].c_str());
            NewGem->Items->Add(runes[i].c_str());
         }
   }
   else
   {
      if (OldGem->Items->Count > 35)
         for (int i = 35; i < 68; ++i)
         {
            OldGem->Items->Delete(i);
            NewGem->Items->Delete(i);
         }
   }

   OldGem->Items->EndUpdate();
   NewGem->Items->EndUpdate();
*/
}
//---------------------------------------------------------------------------

