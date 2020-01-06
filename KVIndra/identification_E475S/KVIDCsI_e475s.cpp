/*
$Id: KVIDCsI_e475s.cpp,v 1.4 2008/12/02 16:43:30 ebonnet Exp $
$Revision: 1.4 $
$Date: 2008/12/02 16:43:30 $
*/

//Created by KVClassFactory on Fri Jul  4 16:36:34 2008
//Author: Eric Bonnet

#include "KVIDCsI_e475s.h"
#include "KVChIo_e475s.h"
#include "KVSilicon_e475s.h"
#include "KVINDRACodeMask.h"
#include "KVReconstructedNucleus.h"

ClassImp(KVIDCsI_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDCsI_e475s</h2>
<h4>derivation of KVIDCsI class for E475s experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDCsI_e475s::KVIDCsI_e475s()
{
   // Default constructor
}

KVIDCsI_e475s::~KVIDCsI_e475s()
{
   // Destructor
}

void KVIDCsI_e475s::CalculateParticleEnergy(KVReconstructedNucleus* nuc)
{

   //status code
   fCalibStatus = kCalibStatus_NoCalibrations;

   UInt_t z = nuc->GetZ();
   //uncharged particles
   if (z == 0) {
      return;
   }

   // In e475s experiment no calibrations available for csi
   const KVSeqCollection* ll = nuc->GetIDTelescopes();
   // si il n y a ri1 avant le csi return pas de calibration possible
   if (ll->GetEntries() == 1) return;

   KVIDTelescope* idtel = 0;
   //Appel au telescope precedent pour realiser la reconstruction en energie
   if ((idtel = (KVIDTelescope*)nuc->GetIDTelescopes()->At(1))) {
      idtel->CalculateParticleEnergy(nuc);
      fCalibStatus = idtel->GetCalibStatus();
   }

   return;
}
