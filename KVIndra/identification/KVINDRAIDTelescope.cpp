#include "KVINDRAIDTelescope.h"
#include "KVINDRACodeMask.h"
#include "KVTelescope.h"
#include <KVINDRATelescope.h>
#include <KVINDRADetector.h>
#include <KVINDRACodes.h>
#include "KVReconstructedNucleus.h"

ClassImp(KVINDRAIDTelescope)
/////////////////////////////////////////////////////////////////////////////////////////
//KVINDRAIDTelescope
//
/////////////////////////////////////////////////////////////////////////////////////////
KVINDRAIDTelescope::KVINDRAIDTelescope()
{
   init();
}

void KVINDRAIDTelescope::init()
{
   //default init
   fECode = 1;                 //default
   fZminCode = kIDCode_Zmin;                 //default
}

UShort_t KVINDRAIDTelescope::GetBadIDCode()
{
   // return a general identification code (can be a bitmask) for particles badly identified
   // this corresponds to VEDA ID code 14
   return kIDCode14;
}

UShort_t KVINDRAIDTelescope::GetCoherencyIDCode()
{
   // return a general identification code (can be a bitmask) for particles identified
   // with this type of ID telescope after coherency analysis
   // this corresponds to VEDA ID code 6
   return kIDCode6;
}

UShort_t KVINDRAIDTelescope::GetMultiHitFirstStageIDCode()
{
   // return a general identification code (can be a bitmask) for particles which cannot
   // be identified correctly due to pile-up in a delta-E detector
   // this corresponds to VEDA ID code 8
   return kIDCode8;
}

void KVINDRAIDTelescope::SetIDCode(KVReconstructedNucleus* n, UShort_t c)
{
   // Overrides KVIDTelescope method
   // If 'n' is not a KVINDRAReconNuc, we use the 'simple' VEDA id codes
   // i.e. '2' for kIDCode2, '5' for kIDCode5, etc. instead of the bitmasks
   // used with KVINDRACodes

   if (n->InheritsFrom("KVINDRAReconNuc")) n->SetIDCode(c);
   else {
      KVINDRACodes indracode;
      indracode.SetIDCode(c);
      n->SetIDCode(indracode.GetVedaIDCode());
   }
}

KVINDRAIDTelescope::~KVINDRAIDTelescope()
{
}


//____________________________________________________________________________________
const Char_t* KVINDRAIDTelescope::GetName() const
{
   // Name of telescope given in the form Det1_Det2_Ring-numberTelescope-number
   // The detectors are signified by their TYPE names i.e. KVDetector::GetType
   //
   //Just a wrapper for GetArrayName to allow polymorphism
   return ((KVINDRAIDTelescope*) this)->GetArrayName();
}
const Char_t* KVINDRAIDTelescope::GetArrayName()
{
   // Name of telescope given in the form Det1_Det2_Ring-numberTelescope-number
   //where ring and telescope numbers are those of the smallest (in angular terms)
   //detector of the telescope (if both are the same size, either one will do).
   // The detectors are signified by their TYPE names i.e. KVDetector::GetType

   //in order to access angular dimensions of detectors, we need their KVTelescopes
   KVINDRATelescope* de_det = dynamic_cast<KVINDRADetector*>(GetDetector(1))->GetTelescope();
   KVINDRATelescope* e_det = 0;
   if (GetSize() > 1)
      e_det = dynamic_cast<KVINDRADetector*>(GetDetector(2))->GetTelescope();
   UInt_t ring, mod;
   if (!e_det || de_det->IsSmallerThan(e_det)) {
      ring = de_det->GetRingNumber();
      mod = de_det->GetNumber();
   }
   else {
      ring = e_det->GetRingNumber();
      mod = e_det->GetNumber();
   }
   SetName(GetDetector(1)->GetType());
   if (e_det) {
      fName.Append("_");
      fName.Append(GetDetector(2)->GetType());
   }
   SetType(fName.Data());
   fName.Append("_");
   TString num;
   num.Form("%02d%02d", ring, mod);
   fName += num;

   return fName.Data();
}
