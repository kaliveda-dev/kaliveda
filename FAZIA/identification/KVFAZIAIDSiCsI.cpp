//Created by KVClassFactory on Tue Sep  8 16:14:25 2015
//Author: ,,,

#include "KVFAZIAIDSiCsI.h"
#include "KVDataSet.h"

ClassImp(KVFAZIAIDSiCsI)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDSiCsI</h2>
<h4>id telescope to manage FAZIA Si-CsI identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAIDSiCsI::KVFAZIAIDSiCsI()
{
   // Default constructor
   SetType("Si-CsI");
   fBelowProton = 0;
   fSiThreshold = 0;

}

KVFAZIAIDSiCsI::~KVFAZIAIDSiCsI()
{
   // Destructor
}

//____________________________________________________________________________________

Double_t KVFAZIAIDSiCsI::GetIDMapX(Option_t*)
{
   //X-coordinate for Si2-CsI identification map :
   // computed fast componment
   // of the charge signal of CsI detector
   return fCsI->GetQ3Amplitude();
}

//____________________________________________________________________________________

Double_t KVFAZIAIDSiCsI::GetIDMapY(Option_t*)
{
   //Y-coordinate for Si2-CsI identification map :
   // computed amlpitude
   // of the charge signal od Si2 detector
   return fSi2->GetQ2Amplitude();
}

//____________________________________________________________________________________

Bool_t KVFAZIAIDSiCsI::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid (class KVIDZAGrid).

   idr->SetIDType(GetType());
   idr->IDattempted = kTRUE;

   //perform identification
   Double_t si2 = (y < 0. ? GetIDMapY() : y);
   Double_t csi = (x < 0. ? GetIDMapX() : x);

   //test if line below proton and si threshold are there
   //if yes test the position of the point respect to
   //these lines
   //Bool_t OKproton = ((fBelowProton && fBelowProton->TestPoint(csi, si2)) || !fBelowProton);
   //Bool_t OKthreshold = ((fSiThreshold && fSiThreshold->TestPoint(csi, si2)) || !fSiThreshold);
   //if (OKproton && OKthreshold){
   if (TheGrid->IsIdentifiable(csi, si2)) {
      TheGrid->Identify(csi, si2, idr);
   } else {
      idr->IDOK = kFALSE;
      idr->IDquality = KVIDZAGrid::kICODE8;
   }

   // set general ID code
   idr->IDcode = GetIDCode();

   return kTRUE;

}

//____________________________________________________________________________________

void KVFAZIAIDSiCsI::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run.

   TheGrid = (KVIDZAGrid*) GetIDGrid();
   fSi2 = (KVFAZIADetector*)GetDetector(1);
   fCsI = (KVFAZIADetector*)GetDetector(2);
   if (TheGrid) {
      SetHasMassID(TheGrid->IsOnlyZId());
      TheGrid->Initialize();
      fBelowProton = (KVIDCutLine*)TheGrid->GetCut("below_proton");
      fSiThreshold = (KVIDCutLine*)TheGrid->GetCut("threshold");
      SetBit(kReadyForID);
   } else {
      ResetBit(kReadyForID);
   }
   if (!gDataSet->HasCalibIdentInfos()) SetBit(kReadyForID);
}
