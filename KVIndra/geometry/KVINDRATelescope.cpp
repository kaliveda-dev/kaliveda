//Created by KVClassFactory on Thu Jun  9 10:34:54 2011
//Author: frankland,,,,

#include "KVINDRATelescope.h"
#include "KVRing.h"

ClassImp(KVINDRATelescope)

KVINDRATelescope::KVINDRATelescope()
{
   // Default constructor
}

KVINDRATelescope::~KVINDRATelescope()
{
   // Destructor
}

//_________________________________________________________________________
const Char_t* KVINDRATelescope::GetName() const
{
   // Name of telescope given in the form Det1_Det2_..._Ring-numberTelescope-number
   // where Det1 etc. are the ACTIVE detector layers of the telescope
   // The detectors are signified by their TYPE names i.e. KVDetector::GetType
   //
   //Just a wrapper for GetArrayName to allow polymorphism
   return const_cast<KVINDRATelescope*>(this)->GetArrayName();
}

const Char_t* KVINDRATelescope::GetArrayName()
{
   // Name of telescope given in the form Det1_Det2_..._Ring-numberTelescope-number
   // where Det1 etc. are the ACTIVE detector layers of the telescope
   // The detectors are signified by their TYPE names i.e. KVDetector::GetType

   TIter next_det(GetDetectors());
   KVDetector* kdet;
   TString dummy;
   while ((kdet = (KVDetector*) next_det())) {  //loop over detectors in telescope
      if (dummy == "")
         dummy = kdet->GetType();
      else
         dummy += kdet->GetType();
      dummy += "_";
   }
   fName.Form("%s%02d%02d", dummy.Data(), GetRingNumber(), GetNumber());
   return fName.Data();
}

Int_t KVINDRATelescope::GetRingNumber() const
{
   return (GetRing() ? GetRing()->GetNumber() : 0);
}

KVRing* KVINDRATelescope::GetRing() const
{
   return (KVRing*)GetParentStructure("RING");
}

KVGroup* KVINDRATelescope::GetGroup() const
{
   return (KVGroup*)GetParentStructure("GROUP");
}


//__________________________________________________________________________
Int_t KVINDRATelescope::Compare(const TObject* obj) const
{
   // telescopes are sorted
   // (i) according to layer number if they are in different layers
   // (ii) according to lower edge polar angle if they belong to the same layer
   // (iii) according to telescope number if they belong to the same ring

   KVINDRATelescope* tobj = (KVINDRATelescope*)obj;

   Int_t this_layer_number = GetParentStructure("RING")->GetParentStructure("LAYER")->GetNumber();
   Int_t obj_layer_number = tobj->GetParentStructure("RING")->GetParentStructure("LAYER")->GetNumber();
   Int_t this_ring_number = GetParentStructure("RING")->GetNumber();
   Int_t obj_ring_number = tobj->GetParentStructure("RING")->GetNumber();

   if (this_layer_number != obj_layer_number) {
// not in the same layer - sort according to layer number, smallest first
//(closest to target)
      if (this_layer_number < obj_layer_number)
         return -1;
      else if (this_layer_number > obj_layer_number)
         return 1;
      else
         return 0;
   }
   if (this_ring_number != obj_ring_number) {
// not in the same ring - sort according to lower edge polar angle
      if (GetThetaMin() < tobj->GetThetaMin())
         return -1;
      else if (GetThetaMin() > tobj->GetThetaMin())
         return 1;
      else
         return 0;
   }
   if (this_ring_number == obj_ring_number) {
// same ring - sort according to telescope number
      if (GetNumber() < tobj->GetNumber())
         return -1;
      else if (GetNumber() > tobj->GetNumber())
         return 1;
      else
         return 0;
   }
   return 0;
}
