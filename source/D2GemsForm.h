//---------------------------------------------------------------------------

#ifndef D2GemsFormH
#define D2GemsFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TConvertGemsForm : public TForm
{
__published:	// IDE-managed Components
   TPanel *ButtonPanel;
   TButton *ConvertButton;
   TButton *CloseButton;
   TPanel *Panel1;
   TLabel *Label1;
   TComboBox *OldGem;
   TLabel *Label2;
   TComboBox *NewGem;
   TStatusBar *GemStatusBar;
   void __fastcall CloseButtonClick(TObject *Sender);
   void __fastcall ConvertButtonClick(TObject *Sender);
   void __fastcall FormShow(TObject *Sender);
private:	// User declarations
public:		// User declarations
   __fastcall TConvertGemsForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConvertGemsForm *ConvertGemsForm;
//---------------------------------------------------------------------------
#endif

