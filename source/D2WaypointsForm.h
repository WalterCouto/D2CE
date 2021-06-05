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

#ifndef D2WaypointsFormH
#define D2WaypointsFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <CheckLst.hpp>
#include <ExtCtrls.hpp>
#include <bitset>
#include "WaypointConstants.h"
//---------------------------------------------------------------------------
class TWaypointsForm : public TForm
{
__published:	// IDE-managed Components
   TButton *CloseButton;
   TPanel *DifficultyPanel;
   TPageControl *WPPageCtrl;
   TTabSheet *Act1Tab;
   TCheckListBox *Act1WP;
   TTabSheet *Act2Tab;
   TCheckListBox *Act2WP;
   TTabSheet *Act3Tab;
   TCheckListBox *Act3WP;
   TTabSheet *Act4Tab;
   TCheckListBox *Act4WP;
   TTabSheet *Act5Tab;
   TCheckListBox *Act5WP;
   TRadioGroup *ViewOptions;
   TButton *ActivateAllButton;
   void __fastcall WPClickCheck(TObject *Sender);
   void __fastcall FormShow(TObject *Sender);
   void __fastcall ViewOptionsClick(TObject *Sender);
   void __fastcall ButtonClick(TObject *Sender);
private:	// User declarations
   std::bitset<NUM_OF_WAYPOINTS> NormalWP[2], NightmareWP[2], HellWP[2];
   bool NormalChanged, NightmareChanged, HellChanged;

   void InitializeWaypoints();
   void SaveWaypoints();
   void UpdateTabs();
public:		// User declarations
   __fastcall TWaypointsForm(TComponent* Owner);
   bool isWaypointsChanged() const;
};
//---------------------------------------------------------------------------
extern PACKAGE TWaypointsForm *WaypointsForm;
//---------------------------------------------------------------------------
#endif

