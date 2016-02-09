//Created by KVClassFactory on Thu Dec 12 16:37:34 2013
//Author: Guilain ADEMARD

#include "KVIDHarpeeICSi.h"
#include "KVVAMOSCodes.h"

ClassImp(KVIDHarpeeICSi)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDHarpeeICSi</h2>
<h4>DeltaE-E identification telescope in VAMOS with Harpee's IC-Si detectors</h4>
<!-- */
// --> END_HTML
//
// This telescope has to be exclusively composed of detectors derived from
// KVVAMOSDetector.
//
// The code corresponding to correct identification by this type of telescope
// is kIDCode4 ( the same as for INDRA's telescope_CSI )
//
////////////////////////////////////////////////////////////////////////////////

KVIDHarpeeICSi::KVIDHarpeeICSi()
{
   // Default constructor

   fIDCode = kIDCode4;
}
//________________________________________________________________

KVIDHarpeeICSi::~KVIDHarpeeICSi()
{
   // Destructor
}

