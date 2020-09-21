//Created by KVClassFactory on Wed Nov 28 13:11:32 2012
//Author: dgruyer

#include "KVSpIdGUI.h"
#include "Riostream.h"
#include "KVIDGridManager.h"
#include "TGFileDialog.h"
#include "TPad.h"
#include <TGMsgBox.h>
#include <TTimer.h>
#include <TClass.h>
#include <TROOT.h>
#include <TGLayout.h>
#include <TCanvas.h>
#include "KVConfig.h"
#include "KVNucleus.h"
#include "TStyle.h"
#include "TBox.h"
#include "TLine.h"
#include "KVNuclearChart.h"
#include "KVHistoManipulator.h"

Int_t  KVSpIdGUI::fZp = -1;
Double_t KVSpIdGUI::fSpFactor = 0.;
Double_t KVSpIdGUI::fAlpha = 1.7;
Bool_t   KVSpIdGUI::fDebug = kFALSE;
Bool_t   KVSpIdGUI::fPiedestal = kTRUE;
Bool_t   KVSpIdGUI::fUseCut = kFALSE;
Int_t  KVSpIdGUI::fAnglesUp = 20;
Int_t  KVSpIdGUI::fAnglesDown = 40;
Int_t  KVSpIdGUI::fMatrixType = 0;
Int_t  KVSpIdGUI::fPiedType = 0;
Int_t  KVSpIdGUI::fNPoints = 50;

ClassImp(KVSpIdGUI)


KVSpIdGUI::KVSpIdGUI(KVIDGraph* g, TH2* data_histo, Double_t xm, Double_t ym, Double_t pdx, Double_t pdy, const char* opt)
{
   fOption = opt;
   fHisto = data_histo;
   fGrid = g;
   fScaledHisto   = 0;
   fIdentificator = 0;

   fXm  = xm;
   fYm  = ym;
   fPdx = pdx;
   fPdy = pdy;

   fUserParameter = (fZp > 0);
   fSfx = 1.;
   fSfy = 1.;

   if (fHisto) {
      fSfx = fHisto->GetNbinsX() * 1. / (fHisto->GetXaxis()->GetXmax());
      fSfy = fHisto->GetNbinsY() * 1 / (fHisto->GetYaxis()->GetXmax());
   }

   //Dialog box for testing identification grid
   fMain = new TGTransientFrame(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), 10, 10);
   fMain->Connect("CloseWindow()", "KVSpIdGUI", this, "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.

   fGrid = g;
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   /******* frame for initialization **************/
   fInitFrame = new TGGroupFrame(fMain, "Initialization");

   fZpFrame = new TGHorizontalFrame(fInitFrame);
   fZpLabel = new TGLabel(fZpFrame, "Z  ");
   fZpEntry = new TGNumberEntry(fZpFrame, fZp, 6, 0,
                                TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);
   fZpEntry->SetState(kTRUE);

   fZpFrame->AddFrame(fZpLabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fZpFrame->AddFrame(fZpEntry, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

   fSpiderFactorLabel = new TGLabel(fZpFrame, "Parameter  ");
   fSpiderFactorEntry = new TGNumberEntry(fZpFrame, fSpFactor, 6, 0,
                                          TGNumberFormat::kNESRealTwo, TGNumberFormat::kNEAAnyNumber);
   fSpiderFactorRadio = new TGCheckButton(fZpFrame);
   fSpiderFactorRadio->Connect("Toggled(Bool_t)", "KVSpIdGUI", this, "SetUserParameter(Bool_t)");

   SetUserParameter(fUserParameter);

   fZpFrame->AddFrame(fSpiderFactorRadio, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));
   fZpFrame->AddFrame(fSpiderFactorEntry, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));
   fZpFrame->AddFrame(fSpiderFactorLabel, new TGLayoutHints(kLHintsRight, 5, 2, 2, 2));
   fInitFrame->AddFrame(fZpFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));


   /******* frame for options **************/
   fOptFrame = new TGGroupFrame(fMain, "Options");

   fTypeFrame = new TGHorizontalFrame(fOptFrame);
   fTypeLabel = new TGLabel(fTypeFrame, "Grid   ");

   fTypeChoice = new TGComboBox(fTypeFrame);
   fTypeChoice->AddEntry("Si-CsI", 0);
   fTypeChoice->AddEntry("Si-Si", 1);
   fTypeChoice->AddEntry("ChIo-Si", 2);
   fTypeChoice->AddEntry("Other", 3);
   fTypeChoice->Resize(80, 22);
   fTypeChoice->Select(fMatrixType);

   fTypeFrame->AddFrame(fTypeLabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fTypeFrame->AddFrame(fTypeChoice, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));


   fNPointsLabel = new TGLabel(fTypeFrame, "Np : ");
   fNPointsEntry = new TGNumberEntry(fTypeFrame, fNPoints, 4, 0,
                                     TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);

   fTypeFrame->AddFrame(fNPointsEntry, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));
   fTypeFrame->AddFrame(fNPointsLabel, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));

   fPiedLabel = new TGLabel(fTypeFrame, "  Piedestal   ");

   fPiedChoice = new TGComboBox(fTypeFrame);
   fPiedChoice->AddEntry("User", 0);
   fPiedChoice->AddEntry("Auto", 1);
   fPiedChoice->AddEntry("None", 2);
   fPiedChoice ->Resize(80, 22);
   fPiedChoice->Select(fPiedType);

   fTypeFrame->AddFrame(fPiedChoice, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));
   fTypeFrame->AddFrame(fPiedLabel, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));

   fOptFrame->AddFrame(fTypeFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

   fDebugFrame = new TGHorizontalFrame(fOptFrame);
   fDebugBut = new TGCheckButton(fDebugFrame, "Debug");
   fDebugBut->SetState((!fDebug ? kButtonUp : kButtonDown), kFALSE);
   fDebugBut->Connect("Toggled(Bool_t)", "KVSpIdGUI", this, "SetDebug(Bool_t)");

   fCutBut = new TGCheckButton(fDebugFrame, "Keep cuts");
   fCutBut->SetState((!fUseCut ? kButtonUp : kButtonDown), kFALSE);
   fCutBut->Connect("Toggled(Bool_t)", "KVSpIdGUI", this, "SetUseCut(Bool_t)");

   fDebugFrame->AddFrame(fCutBut, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   fDebugFrame->AddFrame(fDebugBut, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
   fOptFrame->AddFrame(fDebugFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));


   /******* frame for advanced options **************/
   fAdOptFrame = new TGGroupFrame(fMain, "Advanced options");

   fAngleUpFrame = new TGHorizontalFrame(fAdOptFrame);
   fAngleUpLabel = new TGLabel(fAngleUpFrame, "Angles : above");
   fAngleUpEntry = new TGNumberEntry(fAngleUpFrame, fAnglesUp, 4, 0,
                                     TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);

   fAngleUpFrame->AddFrame(fAngleUpLabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fAngleUpFrame->AddFrame(fAngleUpEntry, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

   fAngleDownLabel = new TGLabel(fAngleUpFrame, " below");
   fAngleDownEntry = new TGNumberEntry(fAngleUpFrame, fAnglesDown, 4, 0,
                                       TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber);

   fAngleUpFrame->AddFrame(fAngleDownLabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fAngleUpFrame->AddFrame(fAngleDownEntry, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

   fApertureLabel = new TGLabel(fAngleUpFrame, " alpha");
   fApertureUpEntry = new TGNumberEntry(fAngleUpFrame, fAlpha, 4, 0,
                                        TGNumberFormat::kNESRealOne, TGNumberFormat::kNEAAnyNumber);

   fAngleUpFrame->AddFrame(fApertureLabel, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fAngleUpFrame->AddFrame(fApertureUpEntry, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   fAdOptFrame->AddFrame(fAngleUpFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

   /********** progress bar **********/
   fProgressBar = new TGHProgressBar(fMain, TGProgressBar::kFancy, 300);
   fProgressBar->SetFillType(TGProgressBar::kBlockFill);
   fProgressBar->ShowPosition();
   fProgressBar->SetBarColor("red");

   /************buttons***************/
   fButtonsFrame = new TGHorizontalFrame(fMain, 100, 50);
   fTestButton = new TGTextButton(fButtonsFrame, "&Spider");
   fTestButton->Connect("Clicked()", "KVSpIdGUI", this,
                        "SpiderIdentification()");
   fCloseButton = new TGTextButton(fButtonsFrame, "&Close");
   fCloseButton->Connect("Clicked()", "KVSpIdGUI", this,
                         "DoClose()");

   fButtonsFrame->AddFrame(fTestButton,
                           new TGLayoutHints(kLHintsExpandX |
                                 kLHintsCenterX |
                                 kLHintsCenterY, 2, 2, 5, 5));
   fButtonsFrame->AddFrame(fCloseButton,
                           new TGLayoutHints(kLHintsExpandX |
                                 kLHintsCenterX |
                                 kLHintsCenterY, 2, 2, 5, 5));


   fMain->AddFrame(fInitFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 2, 2, 10, 2));
   fMain->AddFrame(fOptFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
   fMain->AddFrame(fAdOptFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
   fMain->AddFrame(fProgressBar, new TGLayoutHints(kLHintsCenterX | kLHintsTop, 5, 5, 5, 5));
   fMain->AddFrame(fButtonsFrame, new TGLayoutHints(kLHintsCenterX | kLHintsTop, 2, 2, 2, 2));

   //layout and display window
   fMain->MapSubwindows();
   fMain->Resize(fMain->GetDefaultSize());

   // position relative to the parent's window
   fMain->CenterOnParent();

   fMain->SetWindowName("Spider Identification");
   fMain->MapWindow();

}

void KVSpIdGUI::DoClose()
{
   TTimer::SingleShot(150, "KVSpIdGUI", this, "CloseWindow()");
}

void KVSpIdGUI::CloseWindow()
{
   delete this;
}

KVSpIdGUI::~KVSpIdGUI()
{
   //Delete all widgets
   if ((fIdentificator) && (!fDebug)) delete fIdentificator;
   if ((fScaledHisto) && (!fDebug))   delete fScaledHisto;
   delete fMain;
}

//________________________________________________________________

KVSpIdGUI::KVSpIdGUI(const KVSpIdGUI&)
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

}

void KVSpIdGUI::SetUserParameter(Bool_t par)
{
   fUserParameter = par;
   fZpEntry->SetState(!fUserParameter);
   fSpiderFactorRadio->SetState((!fUserParameter ? kButtonUp : kButtonDown));
   fSpiderFactorEntry->SetState(fUserParameter);
}

Double_t KVSpIdGUI::GetFactor()
{
   Double_t fact = TMath::Sqrt((fXm - fPdx) * (fXm - fPdx) * (fSfx * fSfx) + (fYm - fPdy) * (fYm - fPdy) * (fSfy * fSfy)) / (20.*fZp * 12.);
   return fact;
}

void KVSpIdGUI::SpiderIdentification()
{
   if ((!fHisto) || (!fGrid)) return;

   TVirtualPad* pad = fGrid->GetPad();
   fGrid->UnDraw();

   fZp = fZpEntry->GetIntNumber();
   if (!fUserParameter) fSpFactor = GetFactor();
   else fSpFactor = fSpiderFactorEntry->GetNumber();

   fNPoints    = fNPointsEntry->GetIntNumber();
   fAnglesUp   = fAngleUpEntry->GetIntNumber();
   fAnglesDown = fAngleDownEntry->GetIntNumber();
   fAlpha      = fApertureUpEntry->GetNumber();
   fPiedType   = fPiedChoice->GetSelected();
   fMatrixType = fTypeChoice->GetSelected();
   Int_t type  = fMatrixType;

   TH2*   tmpHisto = fHisto;
   TList* tmpCut   = 0;
   if (fUseCut) {
      tmpHisto = (TH2*)fHisto->Clone(Form("%s_cut", fHisto->GetName()));
      tmpHisto->Reset();
      for (int i = 1; i <= fHisto->GetNbinsX(); i++) {
         for (int j = 1; j <= fHisto->GetNbinsY(); j++) {
            Stat_t ww = fHisto->GetBinContent(i, j);
            Axis_t x0 = fHisto->GetXaxis()->GetBinCenter(i);
            Axis_t y0 = fHisto->GetYaxis()->GetBinCenter(j);
            if (fGrid->IsIdentifiable(x0, y0)) tmpHisto->Fill(x0, y0, ww);
         }
      }
      tmpCut = (TList*)fGrid->GetCuts()->Clone("tmpCuts");
   }

   fGrid->Clear();

   if (fScaledHisto) delete fScaledHisto;
   KVHistoManipulator hm;

   TF1 RtLt("RtLt", Form("x*%lf", fSfx), 0, tmpHisto->GetXaxis()->GetXmax());
   TF1 RtLty("RtLty", Form("x*%lf", fSfy), 0, tmpHisto->GetXaxis()->GetXmax());
   fScaledHisto = (TH2F*)hm.ScaleHisto(tmpHisto, &RtLt, &RtLty);

   if (fIdentificator) delete fIdentificator;
   fIdentificator = new KVSpiderIdentificator(fScaledHisto, fXm * fSfx, fYm * fSfy, -1, -1, fMatrixType);

   switch (fPiedType) {
      case kUser:
         fIdentificator->SetX0(fPdx * fSfx);
         fIdentificator->SetY0(fPdy * fSfy);
         break;
      case kAuto:
         break;
      case kNone:
         fIdentificator->SetX0(0.);
         fIdentificator->SetY0(0.);
   }

   fIdentificator->SetParameters(fSpFactor);
   fIdentificator->SetNangles(fAnglesUp, fAnglesDown);
   fIdentificator->SetAlpha(fAlpha);

   fProgressBar->SetRange(0, fAnglesUp + fAnglesDown + 1);
   fProgressBar->Reset();
   fIdentificator->Connect("Increment(Float_t)", "TGHProgressBar", fProgressBar, "SetPosition(Float_t)");

   fTestButton->SetEnabled(kFALSE);
   fCloseButton->SetEnabled(kFALSE);
   fIdentificator->ProcessIdentification();
   fTestButton->SetEnabled(kTRUE);
   fCloseButton->SetEnabled(kTRUE);

   fIdentificator->Disconnect("Increment(Float_t)", fProgressBar, "SetPosition(Float_t)");
   fProgressBar->Reset();

   if (fDebug) fIdentificator->Draw(fOption.Data());

   TList* ll = (TList*)fIdentificator->GetListOfLines();

   KVIDZALine* TheLine = 0;
   int zmax = 0;

   KVSpiderLine* spline = 0;
   TIter next_line(ll);
   while ((spline = (KVSpiderLine*)next_line())) {
      if ((spline->GetN() > 10)) { //&&(spline->GetX(0)<=fIdentificator->GetX0()+200.))
         TF1* ff1 = 0;

//       printf("#%s# z=%d\n",spline->GetName(),spline->GetZ());

         if (type == KVSpiderIdentificator::kSiCsI) ff1 = spline->GetFunction(fPdx * fSfx, TMath::Max(fScaledHisto->GetXaxis()->GetXmax() * 0.9, spline->GetX(spline->GetN() - 1)));
         else if (type == KVSpiderIdentificator::kSiSi)  ff1 = spline->GetFunction(fPdx * fSfx, TMath::Min(fScaledHisto->GetXaxis()->GetXmax() * 0.9, spline->GetX(spline->GetN() - 1) * 1.5));
         else if (type == KVSpiderIdentificator::kChIoSi)  ff1 = spline->GetFunction(fPdx * fSfx, TMath::Min(fScaledHisto->GetXaxis()->GetXmax() * 0.9, spline->GetX(spline->GetN() - 1) * 1.5));
         else ff1 = spline->GetFunction();

         /*
         if ((type == kSiCsI) && (ff1->GetParameter(1) >= 3000. || (ff1->GetParameter(2) <= 0.35) || (ff1->GetParameter(2) >= 1.))) {
            Info("SpiderIdentification", "Z = %d has been rejected (fit parameters)", spline->GetZ());
            continue;
         }
         */
         TheLine = (KVIDZALine*)((KVIDZAGrid*)fGrid)->NewLine("ID");
         TheLine->SetZ(spline->GetZ());
         double min, max;
         ff1->GetRange(min, max);
         max = TMath::Min(max, fHisto->GetXaxis()->GetXmax() * 0.95);
//         double step = TMath::Min((max - min) /fNPoints, 20.); //20.;
         double step = (max - min) / fNPoints; //20.;
         double stepmax = 5.*step; //800.;

         double x = 0.;
         for (x = min + 1; x < max; x += step) {
            if (step < stepmax) step *= 1.2;
            TheLine->SetPoint(TheLine->GetN(), x, ff1->Eval(x));
         }
         if (max > x) TheLine->SetPoint(TheLine->GetN(), max, ff1->Eval(max));

         fGrid->Add("ID", TheLine);
         if (spline->GetZ() >= zmax) zmax = spline->GetZ();
      }
      else {
         Info("SpiderIdentification", "Z = %d has been rejected (too few points)", spline->GetZ());
      }
   }

   TF1 fx("fx12", Form("x/%lf", fSfx), 0., fScaledHisto->GetNbinsX() * 1.);
   TF1 fy("fy12", Form("x/%lf", fSfy), 0., fScaledHisto->GetNbinsY() * 1.);
   fGrid->Scale(&fx, &fy);

   if (fUseCut) delete tmpHisto;

   if (tmpCut) fGrid->GetCuts()->AddAll(tmpCut);
   pad->cd();
   fGrid->Draw();
   pad->Modified();
   pad->Update();

   DoClose();
}


