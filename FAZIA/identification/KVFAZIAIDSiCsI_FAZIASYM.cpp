//Created by KVClassFactory on Tue Sep  8 16:14:25 2015
//Author: ,,,

#include "KVFAZIAIDSiCsI_FAZIASYM.h"
#include "KVDataSet.h"

ClassImp(KVFAZIAIDSiCsI_FAZIASYM)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDSiCsI</h2>
<h4>id telescope to manage FAZIA Si-CsI identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAIDSiCsI_FAZIASYM::KVFAZIAIDSiCsI_FAZIASYM()
{
   // Default constructor
   SetType("Si-CsI");
   fBelowProton = 0;
   fSiThreshold = 0;
   // printf("Allocating new telescope of type KVFAZIAIDSiCsI_FAZIASYM\n");
}

KVFAZIAIDSiCsI_FAZIASYM::~KVFAZIAIDSiCsI_FAZIASYM()
{
   // Destructor
}

//____________________________________________________________________________________

Double_t KVFAZIAIDSiCsI_FAZIASYM::GetIDMapX(Option_t*)
{
   //X-coordinate for Si2-CsI identification map :
   // computed fast componment
   // of the charge signal of CsI detector
   return fCsI->GetQ3Amplitude();
}

//____________________________________________________________________________________

Double_t KVFAZIAIDSiCsI_FAZIASYM::GetIDMapY(Option_t*)
{
   //Y-coordinate for Si2-CsI identification map :
   // computed amlpitude
   // of the charge signal od Si2 detector
   return fSi2->GetQ2Amplitude();
}

//____________________________________________________________________________________

Bool_t KVFAZIAIDSiCsI_FAZIASYM::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
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


   if (fBelowProton) {
      if (fBelowProton->TestPoint(csi, si2)) idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_NO;
      else idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_YES;
   }
   else {
      idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_UNKNOWN;
   }

   if (TheGrid->IsIdentifiable(csi, si2)) {
      // cout << "Identifico con TheGrid" << endl;
      TheGrid->Identify(csi, si2, idr);
      //  cout << "Ho identificato con TheGrid" << endl;
      //   printf ("The Grid: %d %d %f\n", idr->Z, idr->A, idr->PID);

      //utilizzo TheGrid_lcp per identificare nuovamente Z=1 e Z=2
      if (TheGrid_lcp->IsIdentifiable(csi, si2) && idr->Z <= 2 && idr->Z >= 0) {
         //cout << "Identifico con TheGrid_lcp" << endl;
         TheGrid_lcp->Identify(csi, si2, idr);
         // cout << "Ho identificato con TheGrid_lcp" << endl;
         //printf ("The Grid_lcp: %d %d %f\n", idr->Z, idr->A, idr->PID);
      }
   }
   else {
      idr->IDOK = kFALSE;
      idr->IDquality = KVIDZAGrid::kICODE8;
   }



   // set general ID code
   idr->IDcode = GetIDCode();

   return kTRUE;

}

//____________________________________________________________________________________

void KVFAZIAIDSiCsI_FAZIASYM::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run.

   //TheGrid = (KVIDZAGrid*) GetIDGrid();
   TheGrid = (KVIDZAGrid*) GetListOfIDGrids()->At(0);
   TheGrid_lcp = (KVIDZAGrid*) GetListOfIDGrids()->At(1);
   fSi2 = (KVFAZIADetector*)GetDetector(1);
   fCsI = (KVFAZIADetector*)GetDetector(2);
   if (TheGrid) {
      //   cout << "Inizializzo TheGrid" << endl;
      SetHasMassID(TheGrid->IsOnlyZId());
      TheGrid->Initialize();
      fBelowProton = (KVIDCutContour*)TheGrid->GetCut("PIEDESTAL");//fBelowProton = (KVIDCutLine*)TheGrid->GetCut("PIEDESTAL");
      fSiThreshold = (KVIDCutLine*)TheGrid->GetCut("threshold");
      SetBit(kReadyForID);
   }
   else {
      ResetBit(kReadyForID);
   }

   if (TheGrid_lcp) {
      //  cout << "Inizializzo TheGrid_lcp" << endl;
      SetHasMassID(TheGrid_lcp->IsOnlyZId());
      TheGrid_lcp->Initialize();
      fBelowProton = (KVIDCutContour*)TheGrid->GetCut("PIEDESTAL");//fBelowProton = (KVIDCutLine*)TheGrid->GetCut("PIEDESTAL");
      fSiThreshold = (KVIDCutLine*)TheGrid_lcp->GetCut("threshold");
      SetBit(kReadyForID);
   }
   else {
      ResetBit(kReadyForID);
   }

   if (!gDataSet->HasCalibIdentInfos()) {// for filtering simulations
      SetBit(kReadyForID);
      SetHasMassID();// in principle mass identification always possible
   }
   fMassIDProb->SetParameters(17.7, 1.66);
}

//void KVFAZIAIDSiCsI_FAZIASYM::SetIdentificationStatus(KVReconstructedNucleus* n)
//{
//    // For filtering simulations
//    //
//    // Z-dependence of A identification:
//    // fMassIDProb parameters has to set in the Initialize method

//    n->SetZMeasured();
//    Bool_t okmass = (gRandom->Uniform() < fMassIDProb->Eval(n->GetZ()));

//    if (okmass) {
//        //reset the original mass in case of multiple call of this method
//        if(n->GetParameters()->HasParameter("OriginalMass")) n->SetA(n->GetParameters()->GetIntValue("OriginalMass"));
//        n->SetAMeasured(kTRUE);
//    }
//    else {
//        //save the original mass in the parameter list in case of multiple call of this method
//        n->GetParameters()->SetValue("OriginalMass",n->GetA());
//        double e = n->GetE();
//        n->SetZ(n->GetZ());
//        n->SetE(e);
//        n->SetAMeasured(kFALSE);
//    }
//}
