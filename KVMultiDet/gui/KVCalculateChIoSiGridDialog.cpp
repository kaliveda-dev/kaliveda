#include "KVCalculateChIoSiGridDialog.h"
#include "Riostream.h"
#include "KVIDGridManager.h"
#include "KVIDGrid.h"
#include "TGFileDialog.h"
#include "TPad.h"
#include <TGMsgBox.h>
#include <TTimer.h>
#include <TClass.h>
#include <TROOT.h>
#include <TGLayout.h>
#include <TCanvas.h>

Int_t KVCalculateChIoSiGridDialog::zmax_grid = 93;
Int_t KVCalculateChIoSiGridDialog::npoints = 20;
Bool_t KVCalculateChIoSiGridDialog::set_phd = kFALSE;
Bool_t KVCalculateChIoSiGridDialog::set_pressure = kFALSE;
Bool_t KVCalculateChIoSiGridDialog::set_npoints = kFALSE;
Double_t KVCalculateChIoSiGridDialog::phd_param = -1;
Double_t KVCalculateChIoSiGridDialog::chio_press = -1;

using namespace std;

ClassImp(KVCalculateChIoSiGridDialog)
////////////////////////////////////////////////////////////////
// KVCalculateChIoSiGridDialog
////////////////////////////////////////////////////////////////
KVCalculateChIoSiGridDialog::
KVCalculateChIoSiGridDialog(const TGWindow* p, const TGWindow* main,
                            UInt_t w, UInt_t h, KVIDGrid* g)
{
   //Dialog box for calculating new grid
   fMain = new TGTransientFrame(p, main, w, h);
   fMain->Connect("CloseWindow()", "KVCalculateChIoSiGridDialog", this,
                  "DoClose()");
   fMain->DontCallClose();      // to avoid double deletions.

   fSelectedGrid = g;
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);

   /**************Select Zmax for grid************/
   fHFa = new TGHorizontalFrame(fMain);

   fZmaxLabel = new TGLabel(fHFa, "Calculate lines from Z=1 to Zmax= ");

   fZmax =
      new TGNumberEntry(fHFa, zmax_grid, 5, NE_ZMAX,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);

   fHFa->AddFrame(fZmax, new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHFa->AddFrame(fZmaxLabel, new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));

   /**************Change ChIo pressure************/
   fHFb = new TGHorizontalFrame(fMain);
   fSetPressure =
      new TGCheckButton(fHFb, "Change ChIo pressure :", CHK_BUT_PRESSURE);
   //set checked according to last known state
   fSetPressure->SetDown(set_pressure);
   fSetPressure->Connect("Toggled(Bool_t)", "KVCalculateChIoSiGridDialog",
                         this, "SetPressure(Bool_t)");
   fChIoPressure =
      new TGNumberEntry(fHFb, chio_press, 5, NE_PRESSURE,
                        TGNumberFormat::kNESReal,
                        TGNumberFormat::kNEAPositive);

   fHFb->AddFrame(fChIoPressure,
                  new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHFb->AddFrame(fSetPressure,
                  new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));

   /**************Change Si PHD************/
   fHFc = new TGHorizontalFrame(fMain);
   fSetPHD =
      new TGCheckButton(fHFc, "Simulate Silicon PHD", CHK_BUT_PHD);
   //set checked according to last known state
   fSetPHD->SetDown(set_phd);
   fSetPHD->Connect("Toggled(Bool_t)", "KVCalculateChIoSiGridDialog", this,
                    "SetPHD(Bool_t)");
   /*    fPHDparam =
          new TGNumberEntry(fHFc, phd_param, 5, NE_PHD,
                            TGNumberFormat::kNESReal,
                            TGNumberFormat::kNEAPositive);
    */
//   fHFc->AddFrame(fPHDparam, new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fMassFormula = new TGComboBox(fHFc);
   fMassFormula->AddEntry("Stable", 0);
   fMassFormula->AddEntry("Veda ", 1);
   fMassFormula->AddEntry("EAL", 2);
   fMassFormula->AddEntry("EPAX", 3);
   fMassFormula->AddEntry("2*Z", 4);
   fMassFormula->Resize(104, 22);
   fMassFormula->Select(0);
   fSelectedGrid->SetMassFormula(0);
   fMassFormula->Connect("Selected(Int_t)", "KVCalculateChIoSiGridDialog", this,
                         "SetMassFormula(Int_t)");
   fHFc->AddFrame(fMassFormula, new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));

   fHFc->AddFrame(fSetPHD, new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));

   /**************Change points/line************/
   fHFd = new TGHorizontalFrame(fMain);
   fSetPoints =
      new TGCheckButton(fHFd, "Number of points/line :", CHK_BUT_NPOINTS);
   //set checked according to last known state
   fSetPoints->SetDown(set_npoints);
   fSetPoints->Connect("Toggled(Bool_t)", "KVCalculateChIoSiGridDialog",
                       this, "SetNpoints(Bool_t)");
   fNumberPoints =
      new TGNumberEntry(fHFd, npoints, 5, NE_PHD,
                        TGNumberFormat::kNESInteger,
                        TGNumberFormat::kNEAPositive);

   fHFd->AddFrame(fNumberPoints,
                  new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));
   fHFd->AddFrame(fSetPoints, new TGLayoutHints(kLHintsRight, 5, 5, 2, 2));

   /**************OK, apply & cancel buttons************/
   fHframe2 = new TGHorizontalFrame(fMain, 150, 50);
   fCancel = new TGTextButton(fHframe2, "&Cancel");
   fCancel->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                    "DoClose()");
   fCalculate = new TGTextButton(fHframe2, "&OK");
   fCalculate->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                       "ReadPressure()");
   fCalculate->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                       "ReadPHD()");
   fCalculate->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                       "ReadNpoints()");
   fCalculate->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                       "ReadZmax()");
   fCalculate->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                       "CalculateGrid()");
   fCalculate->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                       "DoClose()");
   fApply = new TGTextButton(fHframe2, "&Apply");
   fApply->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                   "ReadPressure()");
   fApply->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                   "ReadPHD()");
   fApply->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                   "ReadNpoints()");
   fApply->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                   "ReadZmax()");
   fApply->Connect("Clicked()", "KVCalculateChIoSiGridDialog", this,
                   "CalculateGrid()");

   fHframe2->AddFrame(fCalculate,
                      new TGLayoutHints(kLHintsExpandX | kLHintsCenterX |
                                        kLHintsCenterY, 2, 2, 5, 5));
   fHframe2->AddFrame(fApply,
                      new TGLayoutHints(kLHintsExpandX | kLHintsCenterX |
                                        kLHintsCenterY, 2, 2, 5, 5));
   fHframe2->AddFrame(fCancel,
                      new TGLayoutHints(kLHintsExpandX | kLHintsCenterX |
                                        kLHintsCenterY, 2, 2, 5, 5));

   fMain->AddFrame(fHFa,
                   new TGLayoutHints(kLHintsRight | kLHintsTop, 2, 2, 15,
                                     5));
   fMain->AddFrame(fHFb,
                   new TGLayoutHints(kLHintsRight | kLHintsTop, 2, 2, 5,
                                     5));
   fMain->AddFrame(fHFc,
                   new TGLayoutHints(kLHintsRight | kLHintsTop, 2, 2, 5,
                                     5));
   fMain->AddFrame(fHFd,
                   new TGLayoutHints(kLHintsRight | kLHintsTop, 2, 2, 5,
                                     5));
   fMain->AddFrame(fHframe2,
                   new TGLayoutHints(kLHintsCenterX | kLHintsTop, 2, 2, 15,
                                     10));

   //layout and display window
   fMain->MapSubwindows();
   fMain->Resize(fMain->GetDefaultSize());

   // position relative to the parent's window
   fMain->CenterOnParent();

   fMain->SetWindowName("Calculate ID grid");
   fMain->MapWindow();

   //set initial state
   SetPHD(set_phd);
   SetPressure(set_pressure);
   SetNpoints(set_npoints);
}

KVCalculateChIoSiGridDialog::~KVCalculateChIoSiGridDialog()
{
   //Delete all widgets

   delete fMain;
}

void KVCalculateChIoSiGridDialog::DoClose()
{
   TTimer::SingleShot(150, "KVCalculateChIoSiGridDialog", this,
                      "CloseWindow()");
}

void KVCalculateChIoSiGridDialog::CloseWindow()
{
   delete this;
}

void KVCalculateChIoSiGridDialog::SetPressure(Bool_t yes)
{
   set_pressure = yes;
   if (yes) {
      fChIoPressure->SetState(kTRUE);
   } else {
      fChIoPressure->SetState(kFALSE);
      chio_press = -1.0;
   }
}

void KVCalculateChIoSiGridDialog::ReadPressure()
{
   chio_press = fChIoPressure->GetNumber();
}

void KVCalculateChIoSiGridDialog::SetPHD(Bool_t yes)
{
   set_phd = yes;
   if (yes) {
      //fPHDparam->SetState(kTRUE);
   } else {
      //fPHDparam->SetState(kFALSE);
      phd_param = -1.0;
   }
}

void KVCalculateChIoSiGridDialog::ReadPHD()
{
   //phd_param = fPHDparam->GetNumber();
}

void KVCalculateChIoSiGridDialog::SetNpoints(Bool_t yes)
{
   set_npoints = yes;
   if (yes) {
      fNumberPoints->SetState(kTRUE);
   } else {
      fNumberPoints->SetState(kFALSE);
      npoints = 20;
   }
}

void KVCalculateChIoSiGridDialog::ReadNpoints()
{
   npoints = fNumberPoints->GetIntNumber();
}

void KVCalculateChIoSiGridDialog::ReadZmax()
{
   zmax_grid = fZmax->GetIntNumber();
}

void KVCalculateChIoSiGridDialog::CalculateGrid()
{
   //Generate a new grid according to current parameters and draw it

   ReadZmax();
   if (set_phd)
      ReadPHD();
   else
      phd_param = -1.0;
   if (set_pressure)
      ReadPressure();
   else
      chio_press = -1.0;
   if (set_npoints)
      ReadNpoints();
   else
      npoints = 20;
   cout << "Calculate grid with zmax=" << zmax_grid << " ChIo press=" <<
        chio_press << " PHD param=" << phd_param << " npoints=" << npoints
        << endl;
   //fSelectedGrid->MakeELossGrid(zmax_grid, set_phd, chio_press, npoints);
   fSelectedGrid->Draw();
   //update all canvases
   TSeqCollection* Clist = gROOT->GetListOfCanvases();
   if (Clist->GetSize() > 0) {
      Clist->R__FOR_EACH(TCanvas, Modified)();
      Clist->R__FOR_EACH(TCanvas, Update)();
   }
   gIDGridManager->Modified();
//      DoClose();
}

void KVCalculateChIoSiGridDialog::SetMassFormula(Int_t mf)
{
   //Called when user selects a mass formula in the combo box
   cout << "KVCalculateChIoSiGridDialog::SetMassFormula called with mf=" << mf << endl;
   if (mf == 3) {
      //EPAX not implemented
      Warning("SetMassFormula", "EPAX mass formula not implemented. Will use Charity EAL formula.");
      fSelectedGrid->SetMassFormula(KVNucleus::kEALMass);
   } else {
      Info("SetMassFormula", "Setting grid mass formula");
      fSelectedGrid->SetMassFormula(mf);
   }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
