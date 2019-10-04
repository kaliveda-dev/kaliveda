//Created by KVClassFactory on Wed Jun  5 17:00:08 2019
//Author: John Frankland,,,

#include "KVDetectorSignal.h"
#include "KVDetector.h"

ClassImp(KVDetectorSignal)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDetectorSignal</h2>
<h4>Signal associated with detector</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVDetectorSignal::KVDetectorSignal(const Char_t* type, KVDetector* det)
   : KVBase(type), fDetector(det), fValue(0)
{
   if (det) SetTitle(Form("Signal %s of detector %s", type, det->GetName()));
   else SetTitle(Form("Detector signal %s", type));
}

void KVDetectorSignal::ls(Option_t*) const
{
   printf(" %s 	%s	  %s   [%lf]\n", ClassName(), GetName(), GetType(), GetValue());
}

