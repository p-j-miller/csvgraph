//---------------------------------------------------------------------------
// This version by Peter Miller 18/7/2019 as csv file grapher
//
//  Loosely based on an original public domain version by  Frank heinrich, mail@frank-heinrich.de
//---------------------------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef UScalesWindowH
#define UScalesWindowH
//----------------------------------------------------------------------------
#include <System.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Graphics.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Controls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
//----------------------------------------------------------------------------
class TScalesWindow : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TBevel *Bevel1;
        TEdit *Edit1;
        TEdit *Edit2;
        TEdit *Edit3;
        TEdit *Edit4;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        void __fastcall OKBtnClick(TObject *Sender);
        void __fastcall CancelBtnClick(TObject *Sender);
private:
public:
        TScientificGraph *pScientificGraph;
	virtual __fastcall TScalesWindow(TComponent* AOwner,
                           TScientificGraph *pScientificGraphK);
};
//----------------------------------------------------------------------------
extern PACKAGE TScalesWindow *ScalesWindow;
//----------------------------------------------------------------------------
#endif    
