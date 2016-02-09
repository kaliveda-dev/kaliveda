//Created by KVClassFactory on Thu Dec 12 16:37:53 2013
//Author: Guilain ADEMARD

#include "KVIDHarpeeSiCsI.h"

ClassImp(KVIDHarpeeSiCsI)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDHarpeeSiCsI</h2>
<h4>DeltaE-E identification telescope in VAMOS with Harpee's Si-CsI detectors</h4>
<!-- */
// --> END_HTML
//
// This telescope has to be exclusively composed of detectors derived from
// KVVAMOSDetector.
//
// The code corresponding to correct identification by this type of telescope
// is kIDCode3 ( the same as for INDRA's telescope SI_CSI )
//
////////////////////////////////////////////////////////////////////////////////

KVIDHarpeeSiCsI::KVIDHarpeeSiCsI()
{
   // Default constructor

   fIDCode = kIDCode3;
}
//________________________________________________________________

KVIDHarpeeSiCsI::~KVIDHarpeeSiCsI()
{
   // Destructor
}
//________________________________________________________________

Double_t KVIDHarpeeSiCsI::GetIDMapX(Option_t* opt)
{
   // Override the VAMOS default X component of the IDMAP. We require CsI total
   // light output.

   UNUSED(opt);

   KVHarpeeCsI* csi(static_cast<KVHarpeeCsI*>(fEdet));
   assert(csi);

   Double_t csi_light(
      csi->GetACQData(csi->GetEBaseName())
      - csi->GetPedestal(csi->GetEBaseName())
   );

   return csi_light;
}

Double_t KVIDHarpeeSiCsI::GetIDMapY(Option_t* opt)
{
   UNUSED(opt);

   KVHarpeeSi* si(static_cast<KVHarpeeSi*>(fDEdet));
   assert(si);

   Double_t si_energy(si->GetCalibE());
   return si_energy;
}

