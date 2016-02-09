//Created by KVClassFactory on Fri Jan 23 15:43:30 2015
//Author: ademard

#include "KVIDQA.h"
#include "KVVAMOSCodes.h"
ClassImp(KVIDQA)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDQA</h2>
<h4>ID telescope used to identify Q and A with VAMOS</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDQA::KVIDQA() : fQAGrid(NULL)
{
   // Default constructor

   // fIDCode = kIDCode???;
}
//________________________________________________________________

KVIDQA::~KVIDQA()
{
   // Destructor
}
//________________________________________________________________

const Char_t* KVIDQA::GetArrayName()
{
   // Name of identification telescope given in the form IDV_Det1_Det2.
   // Prefix VID for Vamos IDentification.
   // The detectors are signified by their names i.e. KVDetector::GetName


   KVDetector* det  = (KVDetector*)GetDetectors()->Last();
   if (det) {
      SetName(Form("VID_QA_%s", det->GetName()));
      SetType(Form("%s"    , det->GetType()));
   } else SetName("VID_QA_EMPTY");

   return fName.Data();
}
//________________________________________________________________

Bool_t KVIDQA::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Q and A identification and code setting using identification grid,
   //a KVIDQAGrid object loaded in fQAGrid

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;
   //identification
   y = (y < 0. ? GetIDMapY() : y);
   x = (x < 0. ? GetIDMapX() : x);

   if (fQAGrid->IsIdentifiable(x, y)) {
      fQAGrid->Identify(x, y, IDR);
      IDR->IDcode = KVVAMOSCodes::GetTCode(fQAGrid->GetToF());
   }
   return kTRUE;
}
//________________________________________________________________

Bool_t KVIDQA::Identify(KVIdentificationResult* IDR, const Char_t* tof_name, Double_t realAoQ, Double_t realA)
{
   //Q and A identification and code setting using identification grid,
   // corresponding to Q and A/Q quantities calculated from the given
   // time of flight (tof_name). First this grid is loaded in fQAGrid
   // before identification.

   if (IDR && (realAoQ > 0.) && (realA > 0.)) {
      //look for a grid for the given ToF
      fQAGrid = (KVIDQAGrid*)GetListOfIDGrids()->FindObjectWithMethod(tof_name, "GetToF");
      if (fQAGrid) {
         Double_t Y = (fQAGrid->IsAvsAoQ() ? realA : realA / realAoQ);
         return Identify(IDR, realAoQ, Y);
      }
      //else Error("Identify","no grid available for Q-A identification with %s time of flight",tof_name);
   }
   return kFALSE;
}
//________________________________________________________________

void KVIDQA::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run.

   ResetBit(kReadyForID);
   TIter next(fIDGrids);
   TObject* obj = NULL;
   while ((obj = next())) {
      if (!obj->InheritsFrom(KVIDQAGrid::Class())) fIDGrids->Remove(obj);
      else {
         fQAGrid = (KVIDQAGrid*)obj;
         fQAGrid->Initialize();
         SetBit(kReadyForID);
      }
   }
}
