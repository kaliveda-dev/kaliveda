//Created by KVClassFactory on Wed Nov 28 13:11:32 2012
//Author: dgruyer

#ifndef __KVSPIDGUI_H
#define __KVSPIDGUI_H

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
#include <TGComboBox.h>
#include "KVCanvas.h"
#include "KVIDZALine.h"
#include "KVIDZAGrid.h"
#include "KVSpiderIdentificator.h"

class KVSpIdGUI {
   RQ_OBJECT("KVSpIdGUI")

public:
   enum MatrixType {
      kSiCsI,
      kSiSi,
      kChIoSi,
      kOther
   };
   enum PiedestalType {
      kUser,
      kAuto,
      kNone
   };

protected:
   TGTransientFrame* fMain;

   TGHorizontalFrame* fHdataFrame;
   TGLabel* fHdataNameLabel;
   TGLabel* fGNameLabel;

   // initialisation parameters frame
   TGGroupFrame* fInitFrame;

   TGHorizontalFrame* fZpFrame;
   TGLabel* fZpLabel;
   TGNumberEntry* fZpEntry;
   TGCheckButton* fZpRadio;

   TGHorizontalFrame* fSpiderFactorFrame;
   TGLabel* fSpiderFactorLabel;
   TGNumberEntry* fSpiderFactorEntry;
   TGCheckButton* fSpiderFactorRadio;

   // options
   TGGroupFrame* fOptFrame;

   TGHorizontalFrame* fTypeFrame;
   TGLabel* fTypeLabel;
   TGComboBox* fTypeChoice;

   TGHorizontalFrame* fPiedFrame;
   TGLabel* fPiedLabel;
   TGComboBox* fPiedChoice;

   TGHorizontalFrame* fDebugFrame;
   TGCheckButton* fDebugBut;
   TGCheckButton* fCutBut;

   // advanced options
   TGGroupFrame* fAdOptFrame;

   TGHorizontalFrame* fAngleUpFrame;
   TGLabel* fAngleUpLabel;
   TGNumberEntry* fAngleUpEntry;

   TGLabel* fAngleDownLabel;
   TGNumberEntry* fAngleDownEntry;

   TGLabel* fApertureLabel;
   TGNumberEntry* fApertureUpEntry;

   // Progress Bar and buttons
   TGHProgressBar* fProgressBar;

   TGHorizontalFrame* fButtonsFrame;
   TGTextButton* fTestButton;
   TGTextButton* fCloseButton;

   KVIDGraph* fGrid;
   TH2*       fHisto;
   TH2F*      fScaledHisto;

   KVSpiderIdentificator* fIdentificator;

   static Int_t    fZp;
   static Double_t fSpFactor;
   static Double_t fAlpha;
   static Bool_t   fDebug;
   static Bool_t   fPiedestal;
   static Bool_t   fUseZp;
   static Bool_t   fUseCut;
   static Int_t    fAnglesUp;
   static Int_t    fAnglesDown;
   static Int_t    fMatrixType;
   static Int_t    fPiedType;

   Double_t fXm;
   Double_t fYm;
   Double_t fPdx;
   Double_t fPdy;

   Double_t fSfx;
   Double_t fSfy;

   Bool_t   fUserParameter;
   TString  fOption;

public:
   KVSpIdGUI(KVIDGraph* g = 0, TH2* data_histo = 0, Double_t xm = -1.,
             Double_t ym = -1., Double_t pdx = -1., Double_t pdy = -1., const char* opt = "DRLF");
   KVSpIdGUI(const KVSpIdGUI&) ;
   virtual ~KVSpIdGUI();

   void DoClose();
   void CloseWindow();
   void SpiderIdentification();

   void SetDebug(Bool_t deb)
   {
      fDebug = deb;
   }
   void SetUseCut(Bool_t uc)
   {
      fUseCut = uc;
   }
   void SetPiedestal(Bool_t pied)
   {
      fPiedestal = pied;
   }
   void SetUserParameter(Bool_t par);

   Double_t GetFactor();

   ClassDef(KVSpIdGUI, 1) //
};

#endif
