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

#ifndef D2QuestsFormH
#define D2QuestsFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "Constants.h"
#include "DataTypes.h"
//---------------------------------------------------------------------------
class TQuestsForm : public TForm
{
__published:   // IDE-managed Components
   TButton *CloseButton;
   TPanel *DifficultyPanel;
   TRadioGroup *Difficulty;
   TButton *CompleteAllButton;
   TPageControl *QuestsPage;
   TTabSheet *Act1Tab;
   TPanel *Act1Panel;
   TLabel *A1Q1Label;
   TLabel *A1Q2Label;
   TLabel *A1Q3Label;
   TLabel *A1Q4Label;
   TLabel *A1Q5Label;
   TLabel *A1Q6Label;
   TComboBox *A1Q1;
   TComboBox *A1Q2;
   TComboBox *A1Q3;
   TComboBox *A1Q4;
   TComboBox *A1Q5;
   TComboBox *A1Q6;
   TTabSheet *Act2Tab;
   TPanel *Act2Panel;
   TLabel *Label1;
   TLabel *Label2;
   TLabel *Label3;
   TLabel *Label4;
   TLabel *Label5;
   TLabel *Label6;
   TComboBox *A2Q1;
   TComboBox *A2Q2;
   TComboBox *A2Q3;
   TComboBox *A2Q4;
   TComboBox *A2Q5;
   TComboBox *A2Q6;
   TRadioGroup *Act2Status;
   TTabSheet *Act3Tab;
   TPanel *Act3Panel;
   TLabel *Label7;
   TLabel *Label8;
   TLabel *Label9;
   TLabel *Label10;
   TLabel *Label11;
   TLabel *Label12;
   TComboBox *A3Q1;
   TComboBox *A3Q2;
   TComboBox *A3Q3;
   TComboBox *A3Q4;
   TComboBox *A3Q5;
   TComboBox *A3Q6;
   TRadioGroup *Act3Status;
   TTabSheet *Act4Tab;
   TPanel *Act4Panel;
   TLabel *Label13;
   TLabel *Label14;
   TLabel *Label15;
   TComboBox *A4Q1;
   TComboBox *A4Q2;
   TComboBox *A4Q3;
   TRadioGroup *Act4Status;
   TTabSheet *Act5Tab;
   TPanel *Act5Panel;
   TLabel *Label19;
   TLabel *Label20;
   TLabel *Label21;
   TLabel *Label22;
   TLabel *Label23;
   TLabel *Label24;
   TComboBox *A5Q1;
   TComboBox *A5Q2;
   TComboBox *A5Q3;
   TComboBox *A5Q4;
   TComboBox *A5Q5;
   TComboBox *A5Q6;
   TRadioGroup *Act5Status;
   void __fastcall RadioClick(TObject *Sender);
   void __fastcall FormShow(TObject *Sender);
   void __fastcall ButtonClick(TObject *Sender);
   void __fastcall QuestsPageChange(TObject *Sender);
   void __fastcall QuestsChange(TObject *Sender);
   void __fastcall ActStatusEnter(TObject *Sender);
   void __fastcall ActStatusExit(TObject *Sender);
private: // User declarations
   enum {NotStarted, Started = 4, Runes = 0x0E, Scroll = 0x0E, Personalize = 0x1E,
         AddSocket = 0x22, Imbue = 0x4E, Completed = 0x1001};

   ActInfo *qi;
   bool actStatus_entered;
   bool NormalChanged, NightmareChanged, HellChanged;
   int questStatus[NUM_OF_ACTS][NUM_OF_QUESTS];

   void CompleteAllQuests();
   void __fastcall UpdatePage(int currentAct);
   void UpdateQuestValues();
public:     // User declarations
   __fastcall TQuestsForm(TComponent* Owner);

   bool isQuestsChanged() const;
};
//---------------------------------------------------------------------------
extern PACKAGE TQuestsForm *QuestsForm;
//---------------------------------------------------------------------------
#endif

