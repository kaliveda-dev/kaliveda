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
   set_id_code(kSi2CsI);
   fBelowProton = 0;
   fSiThreshold = 0;

}

KVFAZIAIDSiCsI::~KVFAZIAIDSiCsI()
{
   // Destructor
}

//____________________________________________________________________________________

Bool_t KVFAZIAIDSiCsI::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid (class KVIDZAGrid).

   Bool_t ok = KVFAZIAIDTelescope::Identify(idr, x, y);

   Double_t csi, si2;
   GetIDGridCoords(csi, si2, TheGrid, x, y);
   if (fBelowProton) {
      if (fBelowProton->TestPoint(csi, si2)) idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_NO;
      else idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_YES;
   }
   else {
      idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_UNKNOWN;
   }

   return ok;
}

//____________________________________________________________________________________

void KVFAZIAIDSiCsI::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run.

   ResetBit(kReadyForID);
   TheGrid = (KVIDZAGrid*) GetIDGrid();
   if (TheGrid) {
      //printf("Grid Found\n");
      SetHasMassID(TheGrid->HasMassIDCapability());
      TheGrid->Initialize();
      fBelowProton = (KVIDCutLine*)TheGrid->GetCut("PIEDESTAL");
      fSiThreshold = (KVIDCutLine*)TheGrid->GetCut("threshold");
      // make sure both x & y axes' signals are well set up
      if (!fGraphCoords[TheGrid].fVarX || !fGraphCoords[TheGrid].fVarY) {
         Warning("Initialize",
                 "ID tel. %s: grid %s has undefined VarX(%s:%p) or VarY(%s:%p) - WILL NOT USE",
                 GetName(), TheGrid->GetName(), TheGrid->GetVarX(), fGraphCoords[TheGrid].fVarX, TheGrid->GetVarY(), fGraphCoords[TheGrid].fVarY);
      }
      else
         SetBit(kReadyForID);
   }
   else {
      ResetBit(kReadyForID);
   }
   if (!gDataSet->HasCalibIdentInfos()) {// for filtering simulations
      SetBit(kReadyForID);
      SetHasMassID();// in principle mass identification always possible
   }
}

void KVFAZIAIDSiCsI::SetIdentificationStatus(KVReconstructedNucleus* n)
{
   // For filtering simulations
   //
   // Z-dependence of A identification:
   //    all ok if Z<=14, decreasing probability for 15<=Z<=18
   //    no A identification for Z>18
   //
   // If A is not measured, we make sure the KE of the particle corresponds to the simulated one

   n->SetZMeasured();
   fMassIDProb->SetParameters(16.5, .4);
   Bool_t okmass = (n->GetZ() <= 14) || (n->GetZ() < 19 && gRandom->Uniform() < fMassIDProb->Eval(n->GetZ()));
   if (okmass) {
      n->SetAMeasured();
   }
   else {
      double e = n->GetE();
      n->SetZ(n->GetZ());
      n->SetE(e);
   }
}
