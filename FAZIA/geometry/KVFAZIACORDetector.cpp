//Created by KVClassFactory on Tue Aug 29 16:12:18 2017
//Author: gruyer diego

#include "KVFAZIACORDetector.h"
#include "KVFAZIACalibrator.h"
#include "KVIDTelescope.h"
#include "KVFAZIA.h"
#include "KVSignal.h"
#include "TClass.h"
#include "KVLightEnergyCsI.h"

ClassImp(KVFAZIACORDetector)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIACORDetector</h2>
<h4>re-implementation due to special CSI calibration</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIACORDetector::KVFAZIACORDetector()
   : KVFAZIADetector()
{
   // Default constructor
}

//____________________________________________________________________________//

KVFAZIACORDetector::KVFAZIACORDetector(const Char_t* type, const Float_t thick)
   : KVFAZIADetector(type, thick)
{
   // Constructor inherited from KVFAZIADetector
}

//____________________________________________________________________________//

KVFAZIACORDetector::~KVFAZIACORDetector()
{
   // Destructor
}

//________________________________________________________________
void KVFAZIACORDetector::SetCalibrators()
{
   // Change CSI calibrator to KVLightEnergyCsI

   if (GetListOfCalibrators())
      return;

   KVFAZIADetector::SetCalibrators();

   if (fIdentifier == kCSI) {
      KVCalibrator* fzcal = new KVLightEnergyCsI(this);
      fzcal->SetType("Channel-Energy");
      if (!ReplaceCalibrator("Channel-Energy", fzcal)) delete fzcal;
   }
}
