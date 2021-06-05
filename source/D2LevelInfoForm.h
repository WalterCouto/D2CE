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

#ifndef D2LevelInfoFormH
#define D2LevelInfoFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <ExtCtrls.hpp>
#include "ExperienceConstants.h"
//---------------------------------------------------------------------------
class TLevelInfoForm : public TForm
{
__published:	// IDE-managed Components
   TStringGrid *LevelInfoGrid;
   TButton *CloseButton;
   TRadioGroup *D2Version;
   void __fastcall CloseButtonClick(TObject *Sender);
   void __fastcall D2VersionClick(TObject *Sender);
private:
   AnsiString Exp[NUM_OF_LEVELS],
              MaxBeltGold[NUM_OF_LEVELS],
              MaxStashGold[NUM_OF_LEVELS];

   void FillArrays();
   void FillCells();
   void __fastcall FormatStrArray(AnsiString *strArray);
public:		// User declarations
   __fastcall TLevelInfoForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TLevelInfoForm *LevelInfoForm;
//---------------------------------------------------------------------------
#endif

