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

#include "D2WaypointsForm.h"
#include "D2MainForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TWaypointsForm *WaypointsForm;
//---------------------------------------------------------------------------
__fastcall TWaypointsForm::TWaypointsForm(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TWaypointsForm::ButtonClick(TObject *Sender)
{
   if (Sender == ActivateAllButton)
   {
      switch(ViewOptions->ItemIndex)
      {
         case 0: NormalWP[0].set();

                 if (MainForm->CharInfo->isExpansionCharacter())
                    for (int i = 0; i < TOTAL_NUM_OF_WAYPOINTS - NUM_OF_WAYPOINTS; i++)
                       NormalWP[1].set(i, 1);

                 NormalChanged = true;
                 break;

         case 1: NightmareWP[0].set();

                 if (MainForm->CharInfo->isExpansionCharacter())
                    for (int i = 0; i < TOTAL_NUM_OF_WAYPOINTS - NUM_OF_WAYPOINTS; i++)
                       NightmareWP[1].set(i, 1);

                 NightmareChanged = true;
                 break;

         case 2: HellWP[0].set();

                 if (MainForm->CharInfo->isExpansionCharacter())
                    for (int i = 0; i < TOTAL_NUM_OF_WAYPOINTS - NUM_OF_WAYPOINTS; i++)
                       HellWP[1].set(i, 1);

                 HellChanged = true;
                 break;
      }

      UpdateTabs();
   }
   else if (Sender == CloseButton)
   {
      SaveWaypoints();

      if (isWaypointsChanged())
      {
         MainForm->JustOpened = false;
         MainForm->StatsChanged();
      }

      Close();
   }
}
//---------------------------------------------------------------------------
void __fastcall TWaypointsForm::FormShow(TObject *Sender)
{
   NormalChanged = NightmareChanged = HellChanged = false;

   InitializeWaypoints();

   if (!MainForm->CharInfo->isExpansionCharacter())
   {
      Act5Tab->Enabled = false;
      Act5Tab->TabVisible = false;
   }
   else
   {
      Act5Tab->Enabled = true;
      Act5Tab->TabVisible = true;
   }

   WPPageCtrl->ActivePage = Act1Tab;
   ViewOptions->ItemIndex = 0;
   UpdateTabs();
}
//---------------------------------------------------------------------------
void TWaypointsForm::InitializeWaypoints()
{
   for (int i = 0; i < NUM_OF_WP_SETS; i++)
   {
      NormalWP[i] = MainForm->CharInfo->getWaypoints(NORMAL, i);
      NightmareWP[i] = MainForm->CharInfo->getWaypoints(NIGHTMARE, i);
      HellWP[i] = MainForm->CharInfo->getWaypoints(HELL, i);
   }
}
//---------------------------------------------------------------------------
bool TWaypointsForm::isWaypointsChanged() const
{
   return (NormalChanged || NightmareChanged || HellChanged);
}
//---------------------------------------------------------------------------
void TWaypointsForm::SaveWaypoints()
{
   for (int i = 0; i < 2; i++)
   {
      if (NormalChanged)
         MainForm->CharInfo->setWaypoints(NORMAL, i, NormalWP[i].to_ulong());

      if (NightmareChanged)
         MainForm->CharInfo->setWaypoints(NIGHTMARE, i, NightmareWP[i].to_ulong());

      if (HellChanged)
         MainForm->CharInfo->setWaypoints(HELL, i, HellWP[i].to_ulong());
   }
}
//---------------------------------------------------------------------------
void TWaypointsForm::UpdateTabs()
{
   for (int i = 0; i < 9; i++)
      switch(ViewOptions->ItemIndex)
      {
         case 0: Act1WP->Checked[i] = NormalWP[0][i];
                 Act2WP->Checked[i] = NormalWP[0][i+9];
                 Act3WP->Checked[i] = NormalWP[0][i+18];

                 if (i < 3)
                    Act4WP->Checked[i] = NormalWP[0][i+27];

                 if (MainForm->CharInfo->isExpansionCharacter())
                    if (i < 2)
                       Act5WP->Checked[i] = NormalWP[0][i+30];
                    else
                       Act5WP->Checked[i] = NormalWP[1][i-2];
                 break;

         case 1: Act1WP->Checked[i] = NightmareWP[0][i];
                 Act2WP->Checked[i] = NightmareWP[0][i+9];
                 Act3WP->Checked[i] = NightmareWP[0][i+18];

                 if (i < 3)
                    Act4WP->Checked[i] = NightmareWP[0][i+27];

                 if (MainForm->CharInfo->isExpansionCharacter())
                    if (i < 2)
                       Act5WP->Checked[i] = NightmareWP[0][i+30];
                    else
                       Act5WP->Checked[i] = NightmareWP[1][i-2];
                 break;

         case 2: Act1WP->Checked[i] = HellWP[0][i];
                 Act2WP->Checked[i] = HellWP[0][i+9];
                 Act3WP->Checked[i] = HellWP[0][i+18];

                 if (i < 3)
                    Act4WP->Checked[i] = HellWP[0][i+27];

                 if (MainForm->CharInfo->isExpansionCharacter())
                    if (i < 2)
                       Act5WP->Checked[i] = HellWP[0][i+30];
                    else
                       Act5WP->Checked[i] = HellWP[1][i-2];
                 break;
      }
}
//---------------------------------------------------------------------------
void __fastcall TWaypointsForm::ViewOptionsClick(TObject *Sender)
{
   UpdateTabs();
}
//---------------------------------------------------------------------------
void __fastcall TWaypointsForm::WPClickCheck(TObject *Sender)
{
   switch(ViewOptions->ItemIndex)
   {
      case 0: if (Sender == Act1WP)
                 NormalWP[0][Act1WP->ItemIndex].flip();
              else if (Sender == Act2WP)
                 NormalWP[0][Act2WP->ItemIndex + 9].flip();
              else if (Sender == Act3WP)
                 NormalWP[0][Act3WP->ItemIndex + 18].flip();
              else if (Sender == Act4WP)
                 NormalWP[0][Act4WP->ItemIndex + 27].flip();
              else if (Sender == Act5WP && Act5Tab->Enabled)
              {
                 if (Act5WP->ItemIndex < 2)
                    NormalWP[0][Act5WP->ItemIndex + 30].flip();
                 else
                    NormalWP[1][Act5WP->ItemIndex - 2].flip();
              }

              NormalChanged = true;
              break;

      case 1: if (Sender == Act1WP)
                 NightmareWP[0][Act1WP->ItemIndex].flip();
              else if (Sender == Act2WP)
                 NightmareWP[0][Act2WP->ItemIndex + 9].flip();
              else if (Sender == Act3WP)
                 NightmareWP[0][Act3WP->ItemIndex + 18].flip();
              else if (Sender == Act4WP)
                 NightmareWP[0][Act4WP->ItemIndex + 27].flip();
              else if (Sender == Act5WP && Act5Tab->Enabled)
              {
                 if (Act5WP->ItemIndex < 2)
                    NightmareWP[0][Act5WP->ItemIndex + 30].flip();
                 else
                    NightmareWP[1][Act5WP->ItemIndex - 2].flip();
              }

              NightmareChanged = true;
              break;

      case 2: if (Sender == Act1WP)
                 HellWP[0][Act1WP->ItemIndex].flip();
              else if (Sender == Act2WP)
                 HellWP[0][Act2WP->ItemIndex + 9].flip();
              else if (Sender == Act3WP)
                 HellWP[0][Act3WP->ItemIndex + 18].flip();
              else if (Sender == Act4WP)
                 HellWP[0][Act4WP->ItemIndex + 27].flip();
              else if (Sender == Act5WP && Act5Tab->Enabled)
              {
                 if (Act5WP->ItemIndex < 2)
                    HellWP[0][Act5WP->ItemIndex + 30].flip();
                 else
                    HellWP[1][Act5WP->ItemIndex - 2].flip();
              }

              HellChanged = true;
              break;
   }
}
//---------------------------------------------------------------------------

