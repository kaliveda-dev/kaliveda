#ifndef __KVTestIDGridDialog_H
#define __KVTestIDGridDialog_H

#include "TGFrame.h"
#include "TGMenu.h"
#include "TGListBox.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "KVIDGraph.h"
#include "RQ_OBJECT.h"
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGProgressBar.h>
#include "KVCanvas.h"

/**
  \class KVTestIDGridDialog
  \brief GUI for testing identification grids
  \ingroup GUI
 */
class KVTestIDGridDialog {

   RQ_OBJECT("KVTestIDGridDialog")

   TGTransientFrame* fMain;

   TGHorizontalFrame* fHdataFrame;
   TGLabel* fHdataNameLabel;
   TGTextEntry* fHdataNameEntry;

   TGGroupFrame* fHzrealFrame;
   TGHorizontalFrame* fHzrealNameFrame;
   TGLabel* fHzrealNameLabel;
   TGTextEntry* fHzrealNameEntry;
   TGHorizontalFrame* fHzrealBinsFrame;
   TGLabel* fHzrealBinsLabel;
   TGLabel* fHzrealXminLabel;
   TGLabel* fHzrealXmaxLabel;
   TGNumberEntry* fHzrealBinsEntry;
   TGNumberEntry* fHzrealXminEntry;
   TGNumberEntry* fHzrealXmaxEntry;

   TGGroupFrame* fHzvseFrame;
   TGHorizontalFrame* fHzvseNameFrame;
   TGLabel* fHzvseNameLabel;
   TGTextEntry* fHzvseNameEntry;
   TGHorizontalFrame* fHzvseXBinsFrame;
   TGLabel* fHzvseXBinsLabel;
   TGLabel* fHzvseXminLabel;
   TGLabel* fHzvseXmaxLabel;
   TGNumberEntry* fHzvseXBinsEntry;
   TGNumberEntry* fHzvseXminEntry;
   TGNumberEntry* fHzvseXmaxEntry;
   TGHorizontalFrame* fHzvseYBinsFrame;
   TGLabel* fHzvseYBinsLabel;
   TGLabel* fHzvseYminLabel;
   TGLabel* fHzvseYmaxLabel;
   TGNumberEntry* fHzvseYBinsEntry;
   TGNumberEntry* fHzvseYminEntry;
   TGNumberEntry* fHzvseYmaxEntry;

   TGHProgressBar* fProgressBar;

   TGHorizontalFrame* fButtonsFrame;
   TGTextButton* fTestButton;
   TGTextButton* fCloseButton;

   KVIDGraph* fSelectedGrid;

   static TString fNameData;
   static TString fNameZreal;
   static TString fNameZvsE;
   static Int_t hzrealbins;
   static Int_t hzrealxmin;
   static Int_t hzrealxmax;
   static Int_t hzvsexbins;
   static Double_t hzvsexmin;
   static Double_t hzvsexmax;
   static Int_t hzvseybins;
   static Int_t hzvseymin;
   static Int_t hzvseymax;

public:
   KVTestIDGridDialog(const TGWindow* p = 0, const TGWindow* main =
                         0, UInt_t w = 1, UInt_t h = 1, KVIDGraph* g = 0, TH2* data_histo = 0);
   virtual ~ KVTestIDGridDialog();

   void DoClose();
   void CloseWindow();
   void TestGrid();
   void DrawChart(KVCanvas* cc, Int_t zmin, Int_t zmax, Int_t nmin, Int_t nmax);

   Bool_t CheckNameOK(const TString& name)
   {
      return (name != "");
   };

   ClassDef(KVTestIDGridDialog, 0)      //dialog box for testing an ID grid
};
#endif
