//Created by KVClassFactory on Wed Jun  5 17:00:08 2019
//Author: John Frankland,,,

#include "KVDetectorSignal.h"
#include "KVDetector.h"

ClassImp(KVDetectorSignal)

KVDetectorSignal::KVDetectorSignal(const Char_t* type, const KVDetector* det)
   : KVBase(type), fDetector(det), fValue(0), fFired(false)
{
   if (det) SetTitle(Form("Signal %s of detector %s", type, det->GetName()));
   else SetTitle(Form("Detector signal %s", type));
}

void KVDetectorSignal::ls(Option_t*) const
{
   printf(" %s \t\t %s \t\t %s \t\t [%lf] %s\n", ClassName(), GetName(), GetType(), GetValue(), IsFired() ? "*FIRED*" : "");
}

Int_t KVDetectorSignal::GetStatus(const TString&) const
{
   // Override in child classes to report on the 'status' of the signal.
   return -1;
}

TString KVDetectorSignal::GetFullName() const
{
   // Return full name of detector signal, i.e. [detector].[signal-type] if
   // associated to a detector, or [signal-type] if not.
   if (fDetector)
      return Form("%s.%s", fDetector->GetName(), GetName());
   return GetName();
}

