/*
$Id: KVIDChIoCsI_e475s.cpp,v 1.1 2009/04/17 08:50:02 ebonnet Exp $
$Revision: 1.1 $
$Date: 2009/04/17 08:50:02 $
*/

//Created by KVClassFactory on Thu Apr  9 14:59:44 2009
//Author: eric bonnet,,,

#include "KVIDChIoCsI_e475s.h"
#include "KVReconstructedNucleus.h"
#include "KVINDRACodeMask.h"
#include "KVCsI_e475s.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDChIoCsI_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoCsI_e475s</h2>
<h4>derivation of KVIDChIoCsI class for E475s experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoCsI_e475s::KVIDChIoCsI_e475s()
{
   // Default constructor
   fidgrid = 0;

}
//________________________________________________________________________________________//

KVIDChIoCsI_e475s::~KVIDChIoCsI_e475s()
{
   // Destructor
}
//________________________________________________________________________________________//

void KVIDChIoCsI_e475s::Initialize(void)
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

Double_t KVIDChIoCsI_e475s::GetIDMapX(Option_t*)
{

   return ((KVCsI_e475s*)GetDetector(2))->GetCalibratedEnergy();
}
//________________________________________________________________________________________//

Double_t KVIDChIoCsI_e475s::GetIDMapY(Option_t*)
{

   return GetDetector(1)->GetEnergy();

}
//________________________________________________________________________________________//

Bool_t KVIDChIoCsI_e475s::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
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

   // set general ID code ChIo-CsI
   IDR->IDcode = kIDCode4;
   return kTRUE;


}

//________________________________________________________________________________________//
void KVIDChIoCsI_e475s::CalculateParticleEnergy(KVReconstructedNucleus* nuc)
{

   //Cette methode sert a affilier / calculer l energie incidente de la particule
   //pour les telescopes ChIo-CsI des couronnes au-dessus de 10
   //Les CsI n'ayant pas de calibration propre
   //Le calcul de l'énergie n'est possible seulement si le Delta E (ChIo) est calibree en MeV
   //Si le Delta E n'est pas calibre on ressort de la routine sans avoir rien fait

   //status code
   fCalibStatus = kCalibStatus_NoCalibrations;

   if (nuc->GetZ() == 0) return;

   KVDetector* det_dE = GetDetector(1);
   KVDetector* det_Eres = GetDetector(2);


   if (!det_dE->IsCalibrated()) return;

   Double_t dE = det_dE->GetEnergy();
   //Calcul de la perte d'energie dans les CsI
   //A partir de la perte dans la ChIo, en utilisant les resultats de l identification (A,Z)
   Double_t Eres = det_dE->GetEResFromDeltaE(nuc->GetZ(), nuc->GetA(), dE);
   //On affecte l'energie calculee au detecteur CsI
   det_Eres->SetEnergyLoss(Eres);

   Double_t Einc = det_dE->GetCorrectedEnergy(nuc, dE, kTRUE) + Eres;

   fCalibStatus = kCalibStatus_Calculated;

   //La somme des deux energies devient l energie incidente de
   //la particule identifiee / calibree
   nuc->SetEnergy(Einc);
   //set angles from the dimensions of the telescope in which particle detected
   nuc->GetAnglesFromStoppingDetector();

}
