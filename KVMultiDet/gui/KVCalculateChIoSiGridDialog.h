#include "TGFrame.h"
#include "TGMenu.h"
#include "TGListBox.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "KVIDGrid.h"
#include "RQ_OBJECT.h"
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGProgressBar.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// KVCalculateChIoSiGridDialog
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

class KVCalculateChIoSiGridDialog {

   RQ_OBJECT("KVCalculateChIoSiGridDialog")

   TGTransientFrame* fMain;
   TGHorizontalFrame* fHframe2;
   TGLabel* fZmaxLabel;
   TGNumberEntry* fZmax;

   TGHorizontalFrame* fHFa;
   TGHorizontalFrame* fHFb;
   TGHorizontalFrame* fHFc;
   TGHorizontalFrame* fHFd;

   TGCheckButton* fSetPHD;
   TGNumberEntry* fPHDparam;
   TGCheckButton* fSetPressure;
   TGNumberEntry* fChIoPressure;
   TGCheckButton* fSetPoints;
   TGNumberEntry* fNumberPoints;
   TGComboBox* fMassFormula;
   TGTextButton* fCalculate;
   TGTextButton* fApply;
   TGTextButton* fCancel;

   static Int_t zmax_grid;
   static Double_t phd_param;
   static Double_t chio_press;
   static Int_t npoints;

   static Bool_t set_phd;
   static Bool_t set_pressure;
   static Bool_t set_npoints;

   enum {
      CHK_BUT_PHD,
      CHK_BUT_PRESSURE,
      CHK_BUT_NPOINTS,
   };
   enum {
      NE_ZMAX,
      NE_PHD,
      NE_PRESSURE,
      NE_NPOINTS,
   };

   KVIDGrid* fSelectedGrid;

public:

   KVCalculateChIoSiGridDialog(const TGWindow* p =
                                  0, const TGWindow* main = 0, UInt_t w =
                                  1, UInt_t h = 1, KVIDGrid* g = 0);
   virtual ~ KVCalculateChIoSiGridDialog();

   void DoClose();
   void CloseWindow();
   void CalculateGrid();

   void SetPHD(Bool_t);
   void ReadPHD();
   void SetPressure(Bool_t);
   void ReadPressure();
   void SetNpoints(Bool_t);
   void ReadNpoints();
   void ReadZmax();
   //void GetNominalChIoPressure();
   void SetMassFormula(Int_t mf);

   ClassDef(KVCalculateChIoSiGridDialog, 0)    //Dialog box for calculation of new chio-si grid in KVIDGridManagerGUI
};
