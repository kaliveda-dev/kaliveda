/*
$Id: KVIDSiCsI_e475s.cpp,v 1.2 2009/04/15 11:45:14 ebonnet Exp $
$Revision: 1.2 $
$Date: 2009/04/15 11:45:14 $
*/

//Created by KVClassFactory on Wed Apr 15 11:41:21 2009
//Author: eric bonnet,,,

#include "KVIDSiCsI_e475s.h"
#include "KVReconstructedNucleus.h"
#include "KVCsI_e475s.h"
#include "KVIdentificationResult.h"
#include "KVINDRACodeMask.h"

ClassImp(KVIDSiCsI_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDSiCsI_e475s</h2>
<h4>derivation of KVIDSiCsI class for E475s experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDSiCsI_e475s::KVIDSiCsI_e475s()
{
   // Default constructor
   fidgrid = 0;
}

KVIDSiCsI_e475s::~KVIDSiCsI_e475s()
{
   // Destructor
}

//________________________________________________________________________________________//

void KVIDSiCsI_e475s::Initialize(void)
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run
   // AND both detectors are calibrated

   Bool_t ready = kTRUE;
   fidgrid = 0;
   ready &= GetListOfIDGrids()->GetEntries();
   if (ready) fidgrid = (KVIDZAGrid*)GetListOfIDGrids()->First();
   if (fidgrid) fidgrid->Initialize();
   ready &= GetDetector(1)->IsCalibrated();
   ready &= GetDetector(2)->IsCalibrated();

   SetBit(kReadyForID, ready);

}
//________________________________________________________________________________________//

Double_t KVIDSiCsI_e475s::GetIDMapX(Option_t*)
{

   return ((KVCsI_e475s*)GetDetector(2))->GetCalibratedEnergy();
}
//________________________________________________________________________________________//

Double_t KVIDSiCsI_e475s::GetIDMapY(Option_t*)
{

   return GetDetector(1)->GetEnergy();

}
//________________________________________________________________________________________//

Bool_t KVIDSiCsI_e475s::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid KVIDGChIoSi
   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;

   //identification
   Double_t varX = (x < 0. ? GetIDMapX() : x);
   Double_t varY = (y < 0. ? GetIDMapY() : y);

   if (fidgrid->IsIdentifiable(varX, varY))
      fidgrid->Identify(varX, varY, IDR);

   Int_t quality = fidgrid->GetQualityCode();

   if (quality == KVIDZAGrid::kICODE8) {
      // only if the final quality code is kICODE8 do we consider that it is
      // worthwhile looking elsewhere. In all other cases, the particle has been
      // "identified", even if we still don't know its Z and/or A (in this case
      // we consider that we have established that they are unknowable).
      return kFALSE;
   }

   if (quality  == KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE7 (above last line in grid) then the estimated
      // Z is only a minimum value (Zmin)
      IDR->IDcode = kIDCode5;
      return kTRUE;
   }

   if (KVIDZAGrid::kICODE3 < quality && quality < KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE4, kICODE5 or kICODE6 then this "nucleus"
      // corresponds to a point which is inbetween the lines, i.e. noise
      IDR->IDcode = kIDCode10;
      return kTRUE;
   }

   // set general ID code Si-CsI
   IDR->IDcode = kIDCode3;
   return kTRUE;


}

//________________________________________________________________________________________//
void KVIDSiCsI_e475s::CalculateParticleEnergy(KVReconstructedNucleus* nuc)
{

   // The energy of each particle is calculated as follows:
   //
   //      E = dE_1 + dE_2 + ... + dE_N
   //
   // dE_1, dE_2, ... = energy losses measured in each detector through which
   //                          the particle has passed (or stopped, in the case of dE_N).
   //                         These energy losses are corrected for (Z,A)-dependent effects
   //                          such as pulse-heigth defect in silicon detectors, losses in
   //                          windows of gas detectors, etc.
   //
   // Whenever possible, the energy loss for fired detectors which are uncalibrated
   // or not functioning is calculated. In this case the status returned by GetCalibStatus()
   // will be KVIDTelescope::kCalibStatus_Calculated.
   // If none of the detectors is calibrated, the particle's energy cannot be calculated &
   // the status will be KVIDTelescope::kCalibStatus_NoCalibrations.
   // Otherwise, the status code will be KVIDTelescope::kCalibStatus_OK.

   //status code
   fCalibStatus = kCalibStatus_NoCalibrations;

   if (nuc->GetZ() == 0) return;

   KVDetector* det_dE = GetDetector(1);
   KVDetector* det_Eres = GetDetector(2);

   if (!det_dE->IsCalibrated()) return;

   Double_t dE = det_dE->GetEnergy();
   Double_t Eres = det_dE->GetEResFromDeltaE(nuc->GetZ(), nuc->GetA(), dE);
   det_Eres->SetEnergyLoss(Eres);

   Double_t Einc = det_dE->GetCorrectedEnergy(nuc, dE, kTRUE) + Eres;

   fCalibStatus = kCalibStatus_Calculated;

   //Now we have to work our way up the list of detectors from which the particle was
   //reconstructed. For each fired & calibrated detector which is only associated with
   //one particle in the events, we add the corrected measured energy loss
   //to the particle. For uncalibrated, unfired detectors and detectors through which
   //more than one particle has passed, we calculate the corrected energy loss and add it
   //to the particle.
   int ndets = nuc->GetNumDet();
   if (ndets > (int)GetSize()) { //particle passed through other detectors before this idtelesocpe
      //look at detectors not in this id telescope
      int idet = GetSize();//next detector after delta-e member of IDTelescope (stopping detector = 0)
      while (idet < ndets) {

         KVDetector* det = nuc->GetDetector(idet);
         if (det->Fired() && det->IsCalibrated() && det->GetNHits() == 1) {
            Einc += det->GetCorrectedEnergy(nuc);
         }
         idet++;
      }
   }
   //einc is now the energy of the particle before crossing the first detector
   nuc->SetEnergy(Einc);
   //set angles from the dimensions of the telescope in which particle detected
   nuc->GetAnglesFromStoppingDetector();

}

