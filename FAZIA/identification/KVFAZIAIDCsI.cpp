//Created by KVClassFactory on Tue Sep  8 16:00:28 2015
//Author: ,,,

#include "KVFAZIAIDCsI.h"
#include "KVIDGCsI.h"
#include "KVIdentificationResult.h"
#include "KVFAZIADetector.h"
#include "KVDataSet.h"

ClassImp(KVFAZIAIDCsI)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDCsI</h2>
<h4>id telescope to manage FAZIA CsI identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAIDCsI::KVFAZIAIDCsI()
{
   // Default constructor
   SetType("CsI");
   set_id_code(kCsI);
   CsIGrid = 0;
   fCsI = 0;
   /* in principle all CsI R-L telescopes can identify mass & charge */
   SetHasMassID(kTRUE);
}

KVFAZIAIDCsI::~KVFAZIAIDCsI()
{
   // Destructor
}

Bool_t KVFAZIAIDCsI::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid KVIDGCsI* fGrid.

   Bool_t ok = KVFAZIAIDTelescope::Identify(IDR, x, y);

   // general ID code for gammas
   if (IDR->IDquality == KVIDGCsI::kICODE10)
      IDR->IDcode = 0;

   return ok;

}

void KVFAZIAIDCsI::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run.

   CsIGrid = (KVIDGCsI*) GetIDGrid();
   fCsI = (KVFAZIADetector*)GetDetector(1);
   if (CsIGrid) {
      CsIGrid->Initialize();
      SetBit(kReadyForID);
   }
   else {
      ResetBit(kReadyForID);
   }
   if (!gDataSet->HasCalibIdentInfos()) SetBit(kReadyForID);
}
