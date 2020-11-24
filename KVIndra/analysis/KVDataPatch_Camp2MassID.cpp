//Created by KVClassFactory on Sat Jun  9 20:07:03 2018
//Author: John Frankland,,,

#include "KVDataPatch_Camp2MassID.h"
#include "KVINDRAReconNuc.h"
#include "TClass.h"

ClassImp(KVDataPatch_Camp2MassID)

KVDataPatch_Camp2MassID::KVDataPatch_Camp2MassID()
   : KVDataPatch()
{
   // Default constructor
   SetName(ClassName());
   SetTitle(Class()->GetTitle());
}

//____________________________________________________________________________//

KVDataPatch_Camp2MassID::~KVDataPatch_Camp2MassID()
{
   // Destructor
}

void KVDataPatch_Camp2MassID::ApplyToParticle(KVReconstructedNucleus* n)
{
   KVINDRAReconNuc* irn = dynamic_cast<KVINDRAReconNuc*>(n);
   if (irn->GetRingNumber() == 1) {
      //phoswich
      irn->SetAMeasured(kFALSE);
   }
   if (!irn->IsAMeasured()) {
      irn->SetMassFormula(KVNucleus::kVedaMass);
      irn->SetZ(irn->GetZ());
   }
}

void KVDataPatch_Camp2MassID::PrintPatchInfo() const
{
   std::cout << "Correct bad attribution of isotopic resolution & mass in 2nd campaign data" << std::endl;
   std::cout << std::endl;
   std::cout << "All particles identified in Phoswich detectors (Ring 1) were" << std::endl;
   std::cout << "labelled as being isotopically identified." << std::endl;
   std::cout << "Calculated masses were not consistent: several A for some Z" << std::endl;
   std::cout << "We give single KVNucleus::kVedaMass A to all" << std::endl;
}

//____________________________________________________________________________//

