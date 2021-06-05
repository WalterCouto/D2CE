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

#ifndef D2SkillTreeFormH
#define D2SkillTreeFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSkillTrees : public TForm
{
__published:	// IDE-managed Components
   TButton *CloseButton;
   TRadioGroup *ViewOption;
   TStringGrid *SkillsGrid;
   TPageControl *TabPageCtrl;
   TTabSheet *SkillsTab1;
   TTabSheet *SkillsTab2;
   TTabSheet *SkillsTab3;
   TStringGrid *TabSkillsGrid1;
   TStringGrid *TabSkillsGrid2;
   TStringGrid *TabSkillsGrid3;
   TGroupBox *SetSkillsBox;
   TEdit *SkillValue;
   TButton *GoButton;
   TUpDown *UpDownControl;
   void __fastcall ButtonClick(TObject *Sender);
   void __fastcall FormShow(TObject *Sender);
   void __fastcall SkillsGridSetEditText(TObject *Sender, int ACol,
          int ARow, const AnsiString Value);
   void __fastcall ComponentKeyPress(TObject *Sender, char &Key);
   void __fastcall SkillsGridGetEditText(TObject *Sender, int ACol,
          int ARow, AnsiString &Value);
   void __fastcall ViewOptionClick(TObject *Sender);
   void __fastcall TabSkillsGridGetEditText(TObject *Sender, int ACol,
          int ARow, AnsiString &Value);
   void __fastcall TabSkillsGridSetEditText(TObject *Sender,
          int ACol, int ARow, const AnsiString Value);
   void __fastcall FormCreate(TObject *Sender);
   void __fastcall FormDestroy(TObject *Sender);
   void __fastcall SkillValueExit(TObject *Sender);
private:	   // User declarations
   TStringList *SkillsList;
   bool SkillsChanged;
   char *Skills;

   void AssignValues();
   void SetGrid();
   void SetTabs();
public:		// User declarations
   __fastcall TSkillTrees(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSkillTrees *SkillTrees;
//---------------------------------------------------------------------------
#endif

