//Created by KVClassFactory on Sat Oct  3 14:18:09 2009
//Author: John Frankland,,,

#include "KVINDRADetector.h"
#include "KVGroup.h"
#include "KVDataSet.h"

ClassImp(KVINDRADetector)

const Char_t* KVINDRADetector::GetArrayName()
{
   // Name of detector given in the form
   //     CI_0213, SI_0911, CSI_1705
   // to be compatible with GANIL acquisition parameters
   //
   // The root of the name is the detector type

   fFName =
      Form("%s_%02d%02d", GetType(), GetRingNumber(),
           GetModuleNumber());
   return fFName.Data();
}


KVINDRADetector* KVINDRADetector::FindChIo()
{
   //PRIVATE METHOD
   //Used when GetChIo is called the first time to retrieve the
   //pointer to the ChIo of the group associated to this detector
   if (GetTelescope()) {
      KVGroup* kvgr = GetTelescope()->GetGroup();
      if (kvgr) {
         const KVSeqCollection* dets = kvgr->GetDetectors();
         TIter next_det(dets);
         KVINDRADetector* dd;
         while ((dd = (KVINDRADetector*) next_det())) {
            if (dd->InheritsFrom("KVChIo"))
               fChIo = dd;
         }
      }
   }
   else
      fChIo = 0;
   return fChIo;
}

void KVINDRADetector::SetThickness(Double_t thick)
{
   // Overrides KVDetector::SetThickness
   // If using ROOT geometry, print warning that any change in detector thickness will not
   // be taken into account in the geometry.
   // All thicknesses have to be set before the ROOT geometry is generated.

   if (ROOTGeo()) {
      Warning("SetThickness", "Using ROOT geometry. Changes to detector thickness will not be taken into account in geometry.");
   }
   KVMaterial::SetThickness(thick);
}
