//---------------------------------------------------------------------------
// This version by Peter Miller 18/7/2019 as csv file grapher
//
//  Loosely based on an original public domain version by  Frank heinrich, mail@frank-heinrich.de
//---------------------------------------------------------------------------
/*----------------------------------------------------------------------------
 * Copyright (c) 2019 Peter Miller
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *--------------------------------------------------------------------------*/
//---------------------------------------------------------------------
#include <vcl.h>
#include <dstring.h>
#pragma hdrstop

#include "UScientificGraph.h"
#include "UScalesWindow.h"

//---------------------------------------------------------------------
#pragma resource "*.dfm"

#define P_UNUSED(x) (void)x; /* a way to avoid warning unused parameter messages from the compiler */

TScalesWindow *ScalesWindow;
//---------------------------------------------------------------------
__fastcall TScalesWindow::TScalesWindow(TComponent* AOwner,
           TScientificGraph *pScientificGraphK) : TForm(AOwner)
{
  pScientificGraph = pScientificGraphK;

  Edit1->Text=FloatToStrF(pScientificGraph->fnGetScaleXMin(),ffGeneral,8,4);
  Edit2->Text=FloatToStrF(pScientificGraph->fnGetScaleXMax(),ffGeneral,8,4);
  Edit3->Text=FloatToStrF(pScientificGraph->fnGetScaleYMin(),ffGeneral,8,4);
  Edit4->Text=FloatToStrF(pScientificGraph->fnGetScaleYMax(),ffGeneral,8,4);
}
//---------------------------------------------------------------------




void __fastcall TScalesWindow::OKBtnClick(TObject *Sender)
{
  double dEdit1, dEdit2, dEdit3, dEdit4, d;
  P_UNUSED(Sender);
  try
  {
    dEdit1 = Edit1->Text.ToDouble();          //convert user input
    dEdit2 = Edit2->Text.ToDouble();
    dEdit3 = Edit3->Text.ToDouble();
    dEdit4 = Edit4->Text.ToDouble();

    if (dEdit1>dEdit2)                        //swap limits if necessary
    {
    d=dEdit1; dEdit1=dEdit2; dEdit2=d;
    }

    if (dEdit3>dEdit4)                        //swap other limits
    {
    d=dEdit3; dEdit3=dEdit4; dEdit4=d;
    }

    if ((dEdit1!=dEdit2) & (dEdit3!=dEdit4))   //interval length <> zero
    {
    pScientificGraph->fnSetScales(dEdit1,dEdit2,dEdit3,dEdit4);
    }
	else Application->MessageBox(L"Interval length zero!", L"Input Error", MB_OK);

  }
  catch(const EConvertError &e)                //wrong input
  {
   Application->MessageBox(L"Not a Number.", L"Input Error", MB_OK);
  }


  Close();
}
//---------------------------------------------------------------------------

void __fastcall TScalesWindow::CancelBtnClick(TObject *Sender)
{ P_UNUSED(Sender);
  Close();                                    //cancel button
}
//---------------------------------------------------------------------------


