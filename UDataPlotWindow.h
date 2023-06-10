//---------------------------------------------------------------------------
// This version by Peter Miller 18/7/2019 as csv file grapher
//
//  Loosely based on an original public domain version by  Frank heinrich, mail@frank-heinrich.de
//---------------------------------------------------------------------------
#ifndef UDataPlotWindowH
#define UDataPlotWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
//#include "cspin.h"
#include <ExtDlgs.hpp>
#include <System.Actions.hpp>
#include <System.ImageList.hpp>
//---------------------------------------------------------------------------
#include "multiple-lin-reg-fn.h"
#include <Vcl.Samples.Spin.hpp>

class TPlotWindow : public TForm
{
__published:	// IDE-managed Components
        TActionList *ActionList1;
        TImageList *ImageList1;
        TAction *ZoomInX;
        TAction *ZoomOutX;
        TAction *ZoomInY;
        TAction *ZoomOutY;
        TAction *ShiftXPlus;
        TAction *ShiftXMinus;
        TAction *ShiftYPlus;
        TAction *ShiftYMinus;
        TAction *Resize;
        TMainMenu *MainMenu1;
        TMenuItem *Scales1;
        TAction *AutoScale;
        TCheckBox *CheckBox1;
        TRadioGroup *RadioGroup2;
        TRadioGroup *RadioGroup3;
        TCheckBox *CheckBox3;
        TPanel *Panel1;
        TImage *Image1;
        TShape *Shape1;
        TBitBtn *BitBtn5;
        TBitBtn *BitBtn4;
        TBitBtn *BitBtn3;
        TBitBtn *BitBtn2;
        TBitBtn *BitBtn6;
        TBitBtn *BitBtn7;
        TBitBtn *BitBtn8;
        TBitBtn *BitBtn9;
        TBitBtn *BitBtn10;
        TLabel *Label1;
        TLabel *Label2;
        TAction *ReDraw;
        TOpenDialog *Opencsv;
        TLabel *Label3;
        TLabel *Label4;
        TEdit *Edit_xcol;
        TEdit *Edit_ycol;
        TStaticText *StatusText;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *Label7;
        TEdit *Edit_y;
        TEdit *Edit_x;
        TEdit *Edit_title;
        TLabel *Label8;
	TEdit *StaticText_filename;
        TRadioGroup *Xcol_type;
        TLabel *Label9;
        TEdit *Edit_Xoffset;
        TMenuItem *file1;
        TMenuItem *Open1;
        TMenuItem *Addtrace1;
        TMenuItem *Clearalltraces1;
        TLabel *Label10;
        TEdit *Edit_median_len;
        TMenuItem *Help1;
        TMenuItem *About1;
	TListBox *FilterType;
        TLabel *Label11;
        TMenuItem *Exit1;
        TSavePictureDialog *SavePictureDialog1;
        TMenuItem *Save1;
        TMenuItem *SavePlotAs1;
        TMenuItem *SaveDataAs1;
        TMenuItem *PlottoClipboard1;
        TListBox *ListBoxY;
        TListBox *ListBoxX;
        TCheckBox *CheckBox_Compress;
        TBitBtn *ButtonSave;
        TBitBtn *ButtonPlotToClipboard;
        TBitBtn *Button_Filename;
        TBitBtn *Button_add_trace;
        TBitBtn *Button_clear_all_traces;
	TOpenDialog *Opencsv1;
	TFileOpenDialog *FileOpenDialogCSV;
	TFileSaveDialog *FileSaveDialog1;
	TFileSaveDialog *FileSaveDialogCSV;
	TScrollBox *Panel2;
	TMenuItem *Panel3;
	TMenuItem *visible1;
	TColorDialog *ColourDialog1;
	TBitBtn *BitBtn_set_colour;
	TLabel *Label12;
	TLabel *Label13;
	TEdit *Polyorder;
	TLabel *Label14;
	TAction *Action1;
	TMenuItem *Action11;
	TEdit *Edit_skip_lines;
	TLabel *Label15;
	TLabel *Label16;
	TCheckBox *Time_from0;
	TEdit *Date_time_fmt;
	TMenuItem *Save2;
	TCheckBox *CheckBox_legend_add_filename;
	TSpinEdit *SpinEdit_Fontsize;
	TLabel *Label17;
	TLabel *Label18;
	TCheckBox *CheckBox_legend;
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall ResizeExecute(TObject *Sender);
        void __fastcall ShiftYMinusExecute(TObject *Sender);
        void __fastcall ShiftYPlusExecute(TObject *Sender);
        void __fastcall ShiftXMinusExecute(TObject *Sender);
        void __fastcall ShiftXPlusExecute(TObject *Sender);
        void __fastcall ZoomOutYExecute(TObject *Sender);
        void __fastcall ZoomInYExecute(TObject *Sender);
        void __fastcall ZoomOutXExecute(TObject *Sender);
        void __fastcall ZoomInXExecute(TObject *Sender);
        void __fastcall Scales1Click(TObject *Sender);
        void __fastcall AutoScaleExecute(TObject *Sender);
        void __fastcall Image1MouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
        void __fastcall Image1MouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
        void __fastcall Image1MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall Button_clear_all_traces1Click(TObject *Sender);
        void __fastcall Button_add_trace1Click(TObject *Sender);
        void __fastcall ReDrawExecute(TObject *Sender);
        void __fastcall Button_Filename1Click(TObject *Sender);
        void __fastcall Edit_yChange(TObject *Sender);
        void __fastcall Edit_xChange(TObject *Sender);
        void __fastcall Edit_XoffsetChange(TObject *Sender);
        void __fastcall About1Click(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall Edit_titleChange(TObject *Sender);
        void __fastcall Exit1Click(TObject *Sender);
        void __fastcall ButtonSave1Click(TObject *Sender);
        void __fastcall Button_PlotToClipboard1Click(TObject *Sender);
        void __fastcall SaveDataAs1Click(TObject *Sender);
        void __fastcall ListBoxYClick(TObject *Sender);
        void __fastcall ListBoxXClick(TObject *Sender);
        void __fastcall FormMouseWheelDown(TObject *Sender,
          TShiftState Shift, TPoint &MousePos, bool &Handled);
        void __fastcall FormMouseWheelUp(TObject *Sender,
          TShiftState Shift, TPoint &MousePos, bool &Handled);
        void __fastcall FormResize(TObject *Sender);
	void __fastcall Panel2EndDock(TObject *Sender, TObject *Target, int X, int Y);
	void __fastcall Panel2Resize(TObject *Sender);
	void __fastcall visible1Click(TObject *Sender);
	void __fastcall BitBtn_set_colourClick(TObject *Sender);
	void __fastcall Edit_xcolChange(TObject *Sender);
	void __fastcall Edit_ycolChange(TObject *Sender);
	void __fastcall Action1Execute(TObject *Sender);
	void __fastcall Time_from0Click(TObject *Sender);
	void __fastcall Save2Click(TObject *Sender);
	void __fastcall SpinEdit_FontsizeChange(TObject *Sender);
	void __fastcall SpinEdit_FontsizeKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall FormGetSiteInfo(TObject *Sender, TControl *DockClient, TRect &InfluenceRect,
          TPoint &MousePos, bool &CanDock);

private:	 // User declarations
        int iShape1X,iShape1Y;
        void virtual  __fastcall WmDropFiles(TWMDropFiles& Message);     // for drag n drop
        virtual void __fastcall CreateParams(TCreateParams &Params);     // to allow defined window types (to support proper minimise to icon)

protected:

public:		 // User declarations
  int iBitmapHeight, iBitmapWidth;           //Height and Width of the Bitmap
                                             //of the graph object
  TScientificGraph *pScientificGraph;        //graph object
  TScalesWindow *pScalesWindow;              //scales window object

  void fnReDraw();
  void gen_lin_reg(enum reg_types r,int N,bool write_y,int iGraph); // fit defined type of function with N variables (ie for y=m*x+c N=2)  for trace iGraph
  __fastcall TPlotWindow(TComponent* Owner);
BEGIN_MESSAGE_MAP
MESSAGE_HANDLER(WM_DROPFILES,TWMDropFiles,WmDropFiles)
END_MESSAGE_MAP (TForm)
};

//----------------------------------------------------------------------------
#endif
