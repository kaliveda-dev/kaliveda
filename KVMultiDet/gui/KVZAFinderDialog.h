//Created by KVClassFactory on Wed Dec 12 10:11:10 2012
//Author: dgruyer

#ifndef __KVZAFINDERDIALOG_H
#define __KVZAFINDERDIALOG_H

#include "RQ_OBJECT.h"

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

class KVZAFinderDialog {
   RQ_OBJECT("KVZAFinderDialog")

protected:
   KVIDGraph* fGrid;
   TH2*       fHisto;

   static Int_t fZmin;
   static Int_t fZmax;
   static TString fAList;
   static Int_t fBinsByZ;
   static Int_t fStepsByLine;

   TGTransientFrame* fMain;

   TGGroupFrame* fInitFrame;
   TGHorizontalFrame* fZpFrame;
   TGLabel* fZminLabel;
   TGNumberEntry* fZminEntry;
   TGLabel* fZmaxLabel;
   TGNumberEntry* fZmaxEntry;
   TGLabel* fZbinLabel;
   TGNumberEntry* fZbinEntry;
   TGHorizontalFrame* fApFrame;
   TGLabel* fALabel;
   TGTextEntry* fAEntry;

   TGGroupFrame*   fLinearFrame;
   TGHProgressBar* fLinearBar;
   TGGroupFrame*   fProgressFrame;
   TGHProgressBar* fProgressBar;

   TGHorizontalFrame* fButtonsFrame;
   TGTextButton* fTestButton;
   TGTextButton* fCloseButton;

public:
   KVZAFinderDialog(KVIDGraph* g = 0, TH2* data_histo = 0);
   KVZAFinderDialog(const KVZAFinderDialog&);
   virtual ~KVZAFinderDialog();

   void DoClose();
   void CloseWindow();
   void ProcessIdentification();

   ClassDef(KVZAFinderDialog, 1) //dialog box for KVZALineFinder classe
};

#endif
