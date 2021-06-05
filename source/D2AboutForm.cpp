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
//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "D2AboutForm.h"
#include "D2MainForm.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TAboutBox *AboutBox;
//---------------------------------------------------------------------
__fastcall TAboutBox::TAboutBox(TComponent* AOwner)
	: TForm(AOwner)
{
   SetupVersionInfo();
}
//---------------------------------------------------------------------
void __fastcall TAboutBox::OKButtonClick(TObject *Sender)
{
   Close();
}
//---------------------------------------------------------------------------
void __fastcall TAboutBox::LabelClick(TObject *Sender)
{
   if (Sender == HomePage1)
      ShellExecute(NULL, "open", HomePage1->Caption.c_str(), NULL, NULL,
                   SW_SHOWDEFAULT);
   else if (Sender == EmailAddress)
      ShellExecute(NULL, "open", "mailto:diablo2@urbancities.net?subject=Diablo 2 Character Editor",
                   NULL, NULL, SW_SHOWDEFAULT);
}
//---------------------------------------------------------------------------
/*
   This function retrieves the version information stored in the
   executable file.

   This is a slightly modified version of the code found in C++ Builder 5
   Developer's Guide, Copyright 2001 by Sams Publishing
*/
void TAboutBox::SetupVersionInfo()
{
   struct TransArray
   {
      WORD LanguageID, CharacterSet;
   };

   DWORD VerInfo, VerSize;
   HANDLE MemHandle;
   LPVOID MemPtr, BufferPtr;
   UINT BufferLength;
   TransArray *Array;
   char QueryBlock[40];

   // Get the product name and version from the
   // application's version information.
   String Path(Application->ExeName);
   VerSize = GetFileVersionInfoSize(Path.c_str(), &VerInfo);
   if (VerSize > 0)
   {
      MemHandle = GlobalAlloc(GMEM_MOVEABLE, VerSize);
      MemPtr = GlobalLock(MemHandle);
      GetFileVersionInfo(Path.c_str(), VerInfo, VerSize, MemPtr);
      VerQueryValue(MemPtr, "\\VarFileInfo\\Translation", &BufferPtr,
                   &BufferLength);
      Array = (TransArray *)BufferPtr;

      // Get the product name.
      wsprintf(QueryBlock, "\\StringFileInfo\\%04x%04x\\ProductName",
              Array[0].LanguageID, Array[0].CharacterSet);
      VerQueryValue(MemPtr, QueryBlock, &BufferPtr, &BufferLength);
      AnsiString productname((char *)BufferPtr);

      // Get the product version.
      wsprintf(QueryBlock, "\\StringFileInfo\\%04x%04x\\ProductVersion",
              Array[0].LanguageID, Array[0].CharacterSet);
      VerQueryValue(MemPtr, QueryBlock, &BufferPtr, &BufferLength);
      // Set the version caption.
      AnsiString version((char *)BufferPtr);
      Application->Title = productname + " " + version;
      MainForm->Caption = Application->Title;
      lblProductName->Caption = Application->Title;

      // Get legal copyright.
      wsprintf(QueryBlock, "\\StringFileInfo\\%04x%04x\\LegalCopyright",
              Array[0].LanguageID, Array[0].CharacterSet);
      VerQueryValue(MemPtr, QueryBlock, &BufferPtr, &BufferLength);
      // Set the copyright caption.
      lblCopyrightDate->Caption = (char *)BufferPtr;

      GlobalUnlock(MemPtr);
      GlobalFree(MemHandle);
   }
}
//---------------------------------------------------------------------------

