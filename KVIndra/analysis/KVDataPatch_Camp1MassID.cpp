//Created by KVClassFactory on Wed Jun 19 15:54:49 2019
//Author: John Frankland,,,

#include "KVDataPatch_Camp1MassID.h"

#include <KVINDRAReconNuc.h>
#include "TClass.h"

ClassImp(KVDataPatch_Camp1MassID)

KVDataPatch_Camp1MassID::KVDataPatch_Camp1MassID()
   : KVDataPatch()
{
   // Default constructor
   SetName(ClassName());
   SetTitle(Class()->GetTitle());
}

KVDataPatch_Camp1MassID::~KVDataPatch_Camp1MassID()
{
   // Destructor
}

void KVDataPatch_Camp1MassID::ApplyToParticle(KVReconstructedNucleus* n)
{
   // Correct the following errors in 1st campagn data:
   //   * ring=1 idcode=2 (phoswich): should have IsAMeasured=kFALSE
   //   * ring=2-9 idcode=3 (si-csi): should have IsAMeasured=kFALSE
   //   * give correct VEDA mass to all particles with IsAMeasured=kFALSE
   //   * etalons have many strange isotopes (with mass id upto 12~13)
   //      - set IsAMeasured=kFALSE for any unknown nucleus or with lifetime < 1ns

   KVINDRAReconNuc* irn = dynamic_cast<KVINDRAReconNuc*>(n);
   if (irn->IsAMeasured()) {
      // check apparently isotopically-identified nuclei
      if (irn->GetRingNumber() == 1) {
         // phoswich
         irn->SetAMeasured(kFALSE);
      }
      else if (irn->GetCodes().TestIDCode(kIDCode3)) {
         if (irn->GetRingNumber() < 10) {
            // si-csi rings 2-9
            irn->SetAMeasured(kFALSE);
         }
         else if (!irn->IsKnown() || (irn->GetLifeTime() < 1.e-09)) {
            // very exotic isotope in etalons
            irn->SetAMeasured(kFALSE);
         }
      }
   }
   // apply VEDA mass formula to all nuclei with unmeasured mass
   if (!irn->IsAMeasured()) {
      irn->SetMassFormula(KVNucleus::kVedaMass);
      irn->SetZ(irn->GetZ());//recalculate A
   }
}

void KVDataPatch_Camp1MassID::PrintPatchInfo() const
{
   std::cout <<  "Correct the following errors in 1st campagn data:" << std::endl;
   std::cout <<  "  * ring=1 idcode=2 (phoswich): should have IsAMeasured=kFALSE" << std::endl;
   std::cout <<  "  * ring=2-9 idcode=3 (si-csi): should have IsAMeasured=kFALSE" << std::endl;
   std::cout <<  "  * give correct VEDA mass to all particles with IsAMeasured=kFALSE" << std::endl;
   std::cout <<  "  * etalons have many strange isotopes (with mass id upto 12~13)" << std::endl;
   std::cout <<  "     - set IsAMeasured=kFALSE for any unknown nucleus or with lifetime < 1ns" << std::endl;

}

