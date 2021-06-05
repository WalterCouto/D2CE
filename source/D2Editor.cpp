//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USEFORM("D2MainForm.cpp", MainForm);
USEFORM("D2AboutForm.cpp", AboutBox);
USEFORM("D2SkillTreeForm.cpp", SkillTrees);
USEFORM("D2LevelInfoForm.cpp", LevelInfoForm);
USEFORM("D2WaypointsForm.cpp", WaypointsForm);
USEFORM("D2QuestsForm.cpp", QuestsForm);
USEFORM("D2GemsForm.cpp", ConvertGemsForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
   try
   {
       Application->Initialize();
       Application->Title = "";
       Application->CreateForm(__classid(TMainForm), &MainForm);
       Application->CreateForm(__classid(TSkillTrees), &SkillTrees);
       Application->CreateForm(__classid(TWaypointsForm), &WaypointsForm);
       Application->CreateForm(__classid(TQuestsForm), &QuestsForm);
       Application->CreateForm(__classid(TLevelInfoForm), &LevelInfoForm);
       Application->CreateForm(__classid(TAboutBox), &AboutBox);
       Application->CreateForm(__classid(TConvertGemsForm), &ConvertGemsForm);
       Application->Run();
   }
   catch (Exception &exception)
   {
       Application->ShowException(&exception);
   }
   return 0;
}
//---------------------------------------------------------------------------
