/***************************************************************************
$Id: KVINDRAReconNuc.cpp,v 1.61 2009/04/03 14:28:37 franklan Exp $
                          kvdetectedparticle.cpp  -  description
                             -------------------
    begin                : Thu Oct 10 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Riostream.h"
#include "TBuffer.h"
#include "TEnv.h"
#include "KVDataSet.h"
#include "KVINDRAReconNuc.h"
#include "KVList.h"
#include "KVGroup.h"
#include "KVLayer.h"
#include "KVINDRA.h"
#include "KVTelescope.h"
#include "KVIDTelescope.h"
#include "TCollection.h"
#include "KVChIo.h"
#include "KVACQParam.h"
#include "KVSilicon.h"
#include "KVCsI.h"
/******* for Identify **********/
#include "KVIDGCsI.h"
#include "KVIDChIoSi.h"
#include "KVIDCsI.h"

using namespace std;

ClassImp(KVINDRAReconNuc);

////////////////////////////////////////////////////////////////////////////
//KVINDRAReconNuc
//
//Nuclei reconstructed from data measured in the INDRA array.
//Most useful methods are already defined in parent classes KVReconstructedNucleus,
//KVNucleus and KVParticle. Here we add a few useful INDRA-specific methods :
//
//   GetRingNumber(), GetModuleNumber()
//   GetTimeMarker()  -  returns time-marker of detector in which the particle stopped
//
//   StoppedInChIo(), StoppedInSi(), StoppedInCsI()  -  information on the detector in
//                                                      which the particle stopped
//       ** Note1: to access the detector in question, use GetStoppingDetector() (see KVReconstructedNucleus)
//       ** Note2: for a general test of the type of detector in which the particle stopped,
//                 you can do one of the following e.g. to test if it stopped in an
//                 ionisation chamber (assuming a particle pointer 'part'):
//
//        if( !strcmp(part->GetStoppingDetector()->GetType(), "CI") ) { ... }
//               //test the type of the detector - see INDRA detector classes for the different types
//        if( part->GetStoppingDetector()->InheritsFrom("KVChIo") ) { ... }
//               //test the inheritance of the class of the stopping detector
//
//    GetChIo(), GetSi(), GetCsI()    -  return pointers to the detectors through which the particle passed
//    GetEnergyChIo(), GetEnergySi(), GetEnergyCsI()   -  return the calculated contribution of each detector to the
//                                                                                      particle's energy
//
//Identification/Calibration status and codes
//-------------------------------------------
//
//Identified particles have IsIdentified()=kTRUE.
//Calibrated particles have IsCalibrated()=kTRUE.
//
//The KVINDRACodes object fCodes is used to store information on the identification and
//calibration of the particle. You can access this object using GetCodes() and then use the
//methods of KVINDRACodes/KVINDRACodeMask to obtain the information.
//
//For example, you can obtain the VEDA identification code of a particle 'part' using
//
//       part.GetCodes().GetVedaIDCode()  [ = 0, 1, 2, etc.]
//
//Information on whether the particle's mass was measured :
//
//       part.IsAMeasured()  [ = kTRUE or kFALSE]
//
//Information on whether the particle's charge was measured :
//
//       part.IsZMeasured()  [ = kTRUE or kFALSE]
//
//You can also access information on the status codes returned
//by the different identification telescopes used to identify the particle:
//see GetIDSubCode() and GetIDSubCodeString().
//
//Masses of reconstructed nuclei
//------------------------------
//
//When the nucleus' A is not measured, we estimate it from the identified Z.
//By default the mass formula used is that of R.J. Charity (see KVNucleus::GetAFromZ).
//     ** Note: for data previous to the 5th campaign converted to the KaliVeda format
//     ** from old DSTs, we keep the masses from the Veda programme, corresponding to
//     ** KVNucleus mass option kVedaMass.
//IN ALL CASES THE RETURNED VALUE OF GetA() IS POSITIVE
//

void KVINDRAReconNuc::init()
{
   //default initialisations
   if (gDataSet)
      SetMassFormula(UChar_t(gDataSet->GetDataSetEnv("KVINDRAReconNuc.MassFormula", Double_t(kEALMass))));
   fCoherent = kTRUE;
   fPileup = kFALSE;
   fUseFullChIoEnergyForCalib = kTRUE;
   fECsI = fESi = fEChIo = fESi75 = fESiLi = 0.;
   fCorrectCalib = kFALSE;
   fPileupChIo = kFALSE;
   fPileupSi75 = kFALSE;
   fPileupSiLi = kFALSE;
   fIncludeEtalonsInCalibration = kFALSE;
}

KVINDRAReconNuc::KVINDRAReconNuc(): fCodes()
{
   //default ctor
   init();
}

KVINDRAReconNuc::KVINDRAReconNuc(const KVINDRAReconNuc& obj):
   KVReconstructedNucleus(), fCodes()
{
   //copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVINDRAReconNuc&) obj).Copy(*this);
#endif
}

KVINDRAReconNuc::~KVINDRAReconNuc()
{
   //dtor
   init();
   fCodes.Clear();
}

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVINDRAReconNuc::Copy(TObject& obj) const
#else
void KVINDRAReconNuc::Copy(TObject& obj)
#endif
{
   //
   //Copy this to obj
   //
   KVReconstructedNucleus::Copy(obj);
}


void KVINDRAReconNuc::Print(Option_t* option) const
{

   KVReconstructedNucleus::Print(option);

   cout << "KVINDRAReconNuc: fRing=" << GetRingNumber() << " fModule=" <<
        GetModuleNumber() << endl;
   if (IsIdentified()) {
      if (GetRingNumber() < 10) {
         cout << "  -- RESULTS OF COHERENCY TESTS (RINGS 1-9) -- " << endl;
         if (fCoherent) cout << "    CsI-R/L & Si-CsI identifications COHERENT" << endl;
         else cout << "    CsI-R/L & Si-CsI identifications NOT COHERENT" << endl;
         if (fPileup)   cout << "    Si energy loss indicates PILEUP" << endl;
         else cout << "    Si energy loss indicates single particle" << endl;
         if (GetRingNumber() > 1) {
            if (fUseFullChIoEnergyForCalib)   cout << "    ChIo-Si identification indicates single particle" << endl;
            else cout << "    ChIo-Si identification indicates MULTIPLE CHIO CONTRIBUTIONS" << endl;
         }
         cout << endl;
      } else {
         cout << "  -- RESULTS OF COHERENCY TESTS (RINGS 10-17) -- " << endl;
         if (fIncludeEtalonsInCalibration) cout << "    WITH Etalon telescopes included in particle trajectory" << endl;
         else cout << "    WITHOUT etalon telescopes included in particle trajectory" << endl;
         if (fPileupChIo) {
            cout << "    Analysis indicates PILEUP in ChIo" << endl;
         }
         if (fPileupSi75) {
            cout << "    Analysis indicates PILEUP in Si75" << endl;
         }
         if (fPileupSiLi) {
            cout << "    Analysis indicates PILEUP in SiLi" << endl;
         }
         if (fUseFullChIoEnergyForCalib) cout << "    ChIo energy assumed due solely to this particle" << endl;
         else cout << "    ChIo energy contribution calculated assuming MULTIPLE CHIO CONTRIBUTIONS" << endl;
      }
      cout << " =======> ";
      cout << " Z=" << GetZ() << " A=" << ((KVINDRAReconNuc*) this)->
           GetA();
      if (((KVINDRAReconNuc*) this)->IsAMeasured()) cout << " Areal=" << ((KVINDRAReconNuc*) this)->GetRealA();
      else cout << " Zreal=" << GetRealZ();
      cout << endl << " Identification code = " << ((KVINDRAReconNuc*) this)->
           GetCodes().GetIDStatus() << endl;
   } else {
      cout << "(unidentified)" << endl;
   }
   if (IsCalibrated()) {
      cout << " Total Energy = " << GetEnergy() << " MeV,  Theta=" << GetTheta() << " Phi=" << GetPhi() << endl;
      if (GetRingNumber() < 10) {
         if (GetRingNumber() > 1) {
            cout << "    EChIo = " << TMath::Abs(fEChIo) << " MeV";
            if (fEChIo < 0) cout << " (calculated)";
            cout << endl;
         }
         cout << "    ESi = " << TMath::Abs(fESi) << " MeV";
         if (fESi < 0) cout << " (calculated)";
         cout << endl;
         cout << "    ECsI = " << TMath::Abs(fECsI) << " MeV";
         if (fECsI < 0) cout << " (calculated)";
         cout << endl;
      } else {
         cout << "    EChIo = " << TMath::Abs(fEChIo) << " MeV";
         if (fEChIo < 0) cout << " (calculated)";
         cout << endl;
         if (fIncludeEtalonsInCalibration) {
            cout << "    ESi75 = " << TMath::Abs(fESi75) << " MeV";
            if (fESi75 < 0) cout << " (calculated)";
            cout << endl;
            cout << "    ESiLi = " << TMath::Abs(fESiLi) << " MeV";
            if (fESiLi < 0) cout << " (calculated)";
            cout << endl;
         }
         cout << "    ECsI = " << TMath::Abs(fECsI) << " MeV";
         if (fECsI < 0) cout << " (calculated)";
         cout << endl;
      }
      cout << "    Target energy loss correction :  " << GetTargetEnergyLoss() << " MeV" << endl;
      cout << " Calibration code = " << ((KVINDRAReconNuc*) this)->
           GetCodes().GetEStatus() << endl;
   } else {
      cout << "(uncalibrated)" << endl;
   }
//   if (!IsIdentified()) {
   cout << "Analysis : ";
   switch (GetStatus()) {
      case 0:
         cout <<
              "Particle alone in group, or identification independently of other"
              << endl;
         cout << "particles in group is directly possible." << endl;
         break;

      case 1:
         cout <<
              "Particle reconstructed after identification of others in group"
              << endl;
         cout <<
              "and subtraction of their calculated energy losses in ChIo."
              << endl;
         break;

      case 2:
         cout <<
              "Particle identification estimated after arbitrary sharing of"
              << endl;
         cout <<
              "energy lost in ChIo between several reconstructed particles."
              << endl;
         break;

      case 3:
         cout <<
              "Particle stopped in first stage of telescope. Estimation of minimum Z."
              << endl;
         break;

      default:
         cout << GetStatus() << endl;
         break;
   }
   //}
   cout <<
        "-------------------------------------------------------------------------------"
        << endl;
}

//________________________________________________________________________________________

void KVINDRAReconNuc::Clear(Option_t* t)
{
   //reset nucleus' properties
   KVReconstructedNucleus::Clear(t);
   init();
   fCodes.Clear();
}

KVChIo* KVINDRAReconNuc::GetChIo()
{
   //Return pointer to the ChIo the particle passed through.
   //Pointer is null if not.
   KVChIo* chio = (KVChIo*) GetDetectorList()->FindObjectByType("CI");
   return chio;
}

KVSilicon* KVINDRAReconNuc::GetSi()
{
   //Return pointer to the Silicon the particle passed through.
   //Pointer is null if not.
   KVSilicon* chio =
      (KVSilicon*) GetDetectorList()->FindObjectByType("SI");
   return chio;
}

KVSi75* KVINDRAReconNuc::GetSi75()
{
   //Return pointer to the Silicon-75 the particle passed through.
   //Pointer is null if not.
   KVSi75* chio =
      (KVSi75*) GetDetectorList()->FindObjectByType("SI75");
   return chio;
}

KVSiLi* KVINDRAReconNuc::GetSiLi()
{
   //Return pointer to the Silicon-Lithium the particle passed through.
   //Pointer is null if not.
   KVSiLi* chio =
      (KVSiLi*) GetDetectorList()->FindObjectByType("SILI");
   return chio;
}

KVCsI* KVINDRAReconNuc::GetCsI()
{
   //Return pointer to the CsI the particle passed through.
   //Pointer is null if not.
   KVCsI* csi = (KVCsI*) GetDetectorList()->FindObjectByType("CSI");
   return csi;
}


Bool_t KVINDRAReconNuc::StoppedInChIo()
{
   //Returns kTRUE if particle stopped in ChIo detector

   if (!strcmp(GetStoppingDetector()->GetType(), "CI")) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

Bool_t KVINDRAReconNuc::StoppedInSi()
{
   //Returns kTRUE if particle stopped in Si detector

   if (!strcmp(GetStoppingDetector()->GetType(), "SI")) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}
Bool_t KVINDRAReconNuc::StoppedInSi75()
{
   //Returns kTRUE if particle stopped in Si75 detector

   if (!strcmp(GetStoppingDetector()->GetType(), "SI75")) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}
Bool_t KVINDRAReconNuc::StoppedInSiLi()
{
   //Returns kTRUE if particle stopped in Si detector

   if (!strcmp(GetStoppingDetector()->GetType(), "SILI")) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

Bool_t KVINDRAReconNuc::StoppedInCsI()
{
   //Returns kTRUE if particle stopped in CsI detector

   if (!strcmp(GetStoppingDetector()->GetType(), "CSI")) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

Int_t KVINDRAReconNuc::GetIDSubCode(const Char_t* id_tel_type) const
{
   //Returns subcode/status code from identification performed in ID telescope of given type.
   //i.e. to obtain CsI R-L subcode use GetIDSubCode("CSI_R_L").
   //
   //The meaning of the subcodes is defined in the corresponding KVIDTelescope class description
   //i.e. for CsI R-L, look at KVIDCsI.
   //
   //The subcode is set for any attempted identification, not necessarily that which eventually
   //leads to the definitive identification of the particle.
   //i.e. in the example of the CsI R-L subcode, the particle in question may in fact be identified
   //by a Si-CsI telescope, because the CsI identification routine returned e.g. KVIDGCsI::kICODE7
   //(a gauche de la ligne fragment, Z est alors un Zmin et le plus probable).
   //
   //calling GetIDSubCode() with no type returns the identification subcode corresponding
   //to the identifying telescope (whose pointer is given by GetIdentifyingTelescope()).
   //
   //If no subcode exists (identification in given telescope type was not attempted, etc.) value returned is -1

   KVIdentificationResult* ir = 0;
   if (strcmp(id_tel_type, "")) ir = GetIdentificationResult(id_tel_type);
   else ir = GetIdentificationResult(GetIdentifyingTelescope());
   if (!ir) return -1;
   return ir->IDquality;
   //return GetIDSubCode(id_tel_type,const_cast <KVINDRAReconNuc *>(this)->GetCodes().GetSubCodes());
}

const Char_t* KVINDRAReconNuc::GetIDSubCodeString(const Char_t*
      id_tel_type) const
{
   //Returns explanatory message concerning identification performed in ID telescope of given type.
   //(see GetIDSubCode())
   //
   //The subcode is set for any attempted identification, not necessarily that which eventually
   //leads to the definitive identification of the particle.
   //
   //calling GetIDSubCodeString() with no type returns the identification subcode message corresponding
   //to the identifying telescope (whose pointer is given by GetIdentifyingTelescope()).
   //
   //In case of problems:
   //       no ID telescope of type 'id_tel_type' :  "No identification attempted in id_tel_type"
   //       particle not identified               :  "Particle unidentified. Identifying telescope not set."

   KVIdentificationResult* ir = 0;
   if (strcmp(id_tel_type, "")) ir = GetIdentificationResult(id_tel_type);
   else ir = GetIdentificationResult(GetIdentifyingTelescope());
   if (!ir) {
      if (strcmp(id_tel_type, ""))
         return Form("No identification attempted in %s", id_tel_type);
      else
         return
            Form("Particle unidentified. Identifying telescope not set.");
   }
   return ir->GetComment();
}

//____________________________________________________________________________________________

Bool_t KVINDRAReconNuc::CoherencyChIoSiCsI(KVIdentificationResult theID)
{
   // we check that the ChIo contribution is sane:
   // if no other particles hit this group, the Z given by the ChIoSi
   // must be <= the Z found from Si-CsI or CsI-RL identification
   //
   // in this case the measured energy loss of the ChIo can be solely attributed to this particle
   // and we return kTRUE;

   // ChIo was hit by more than one particle in group
   if (GetChIo() && GetChIo()->GetNHits() > 1) {
      return kFALSE;
   }

   KVIdentificationResult* IDchiosi = GetIdentificationResult(3);
   if (!IDchiosi->IDattempted) {
      // no ChIo-Si identification ? assume coherency ?
      return kTRUE;
   }

   // if we have a successful ChIo-Si id with a Z > Z given by CsI or Si-CsI id,
   // then we consider that there is a pile-up in the ChIo
   // note that we consider Z_ChIoSi = 1 to mean the ChIo is in the pedestal i.e. nothing seen
   // we also require the chio-si identification to be above the punch-through line
   if (IDchiosi->IDOK && IDchiosi->Z > 1 && IDchiosi->IDquality != KVIDGChIoSi::k_BelowPunchThrough && IDchiosi->Z > theID.Z) {
      return kFALSE;
   }

   return kTRUE;
}

//____________________________________________________________________________________________

Bool_t KVINDRAReconNuc::CoherencySiCsI(KVIdentificationResult& theID)
{
   // Called by Identify() for particles stopping in CsI detectors on rings 1-9,
   // which have a Silicon detector just in front of them.
   //
   // coherent = kFALSE if CsI-R/L and Si-CsI identifications are not coherent.
   //                 this is a warning, the CsI identification is kept, either the Si signal
   //                  was not good (particle hitting dead zone), or it actually corresponds
   //                  to two particles reaching the CsI at the same time
   // pileup = kTRUE means that the particle identified in CsI-R/L is correct,
   //              and there is probably a second particle which stopped in the silicon
   //              detector at the same time, to be identified in ChIo-Si after
   //              subtraction of the Silicon contribution

   KVIdentificationResult* IDcsi = GetIdentificationResult(1);
   KVIdentificationResult* IDsicsi = GetIdentificationResult(2);
   fCoherent = kTRUE;
   fPileup = kFALSE;

   // Unsuccessful/no CsI id attempt with successful Si-CsI id ?
   // Then use Si-CsI identification result
   if (IDsicsi->IDOK && !IDcsi->IDOK) {
      theID = *IDsicsi;
      return kTRUE;
   }

   // check coherency of CsI-R/L and Si-CsI identifications
   if (IDcsi->IDOK) {
      // gammas
      if (IDcsi->IDcode == kIDCode0) {
         theID = *IDcsi;
         return kTRUE;
      }
      // Neutrons have no energy loss in Si detector (pedestal), have a reaction in the CsI and create a Z=1
      // or Z=2 which is identified in CsI R-L,  while they show up in Si-CsI maps as a horizontal
      // band around the Si pedestal for low energies (energies where proton dE is significantly larger than
      // the pedestal).
      if ((IDcsi->Z == 1 || IDcsi->Z == 2) && GetSi()) {
         if (IDsicsi->deltaEpedestal == KVIdentificationResult::deltaEpedestal_UNKNOWN) {
            // no explicit treatment of 'neutron-like' particles with a cut in Si-CsI id grid
            // First we check that we are in the domain where proton dE can be distinguished from pedestal.
            // If so, if the measured dE is below [ped + factor*(dE_exp - ped)], then we label the particle as a neutron.
            // 'factor' depends on the Si-CsI telescope: if it has mass identification, factor=0.3; if not, factor=0.1
            // (these empirical values correspond to 5th campaign data)
            KVIDTelescope* idt = (KVIDTelescope*)GetIDTelescopes()->FindObjectByType(IDsicsi->GetType());
            if (idt) {
               Double_t ped = idt->GetPedestalY();
               Int_t status;
               Double_t dE_exp = idt->GetMeanDEFromID(status, 1, 1);
               if (status == KVIDTelescope::kMeanDE_OK) { // proton/Z=1 line exists, and we are in its energy range
                  if (dE_exp > ped + 5.) { // arbitrary choice, must have expected dE at least 5 channels above pedestal

                     // if Si-CsI has no isotopic identification, reduce factor
                     Double_t factor = (idt->HasMassID() ? 0.3 : 0.1);
                     if (idt->GetIDMapY() < (ped + factor * (dE_exp - ped))) {
                        theID = *IDsicsi;
                        theID.IDOK = kTRUE;
                        theID.Zident = kTRUE;
                        theID.Aident = kTRUE;
                        theID.Z = 0;
                        theID.A = 1;
                        theID.IDcode = kIDCode1; // general code for neutrons
                        return kTRUE;
                     }
                  }
               }
            }
         } else if (IDsicsi->deltaEpedestal == KVIdentificationResult::deltaEpedestal_YES) {
            // explicit treatment of 'neutron-like' particles with a cut in Si-CsI id grid
            theID = *IDsicsi;
            theID.IDOK = kTRUE;
            theID.Zident = kTRUE;
            theID.Aident = kTRUE;
            theID.Z = 0;
            theID.A = 1;
            theID.IDcode = kIDCode1; // general code for neutrons
            return kTRUE;
         }
      }

      // We check the coherency of the mass and charge identifications
      // If a successful Si-CsI identification is available we check:
      //   if Si-CsI gives A & Z - must have same Z, A within +/-1 unit
      //                                      if Z or A smaller => incoherency : pile-up of two particles in CsI ?
      //                                               or bad signal from Si detector (dead zones) ?
      //                                      if Z or A larger, CsI identification is good,
      //                                      assume another particle stopped in Si (identify in ChIo-Si)
      //   if Si-CsI gives just Z - must have same Z
      //                                      if Z smaller => incoherency : pile-up of two particles in CsI ?
      //                                               or bad signal from Si detector (dead zones) ?
      //                                      if Z larger => CsI identification is good,
      //                                      assume another particle stopped in Si (identify in ChIo-Si)
      //
      // If CsI identification gives code KVIDGCsI::kICode1 ou KVIDGCsI::kICode3 and the
      // Si-CsI gives the same Z but A = Acsi + 1, we use the Si-CsI identification.
      //
      // If CsI identification gives code KVIDGCsI::kICode2 ou KVIDGCsI::kICode3 and the
      // Si-CsI gives the same Z but A = Acsi - 1, we use the Si-CsI identification.
      //
      // N.B. if CsI-R/L identification gives "8Be" (2 alpha particles) then there are two correct possibilities:
      //     1) Si-CsI identification gives 7Li => CsI identification is correct ("8Be")
      //     2) Si-CsI identification gives 8He => the particle is 8He (falls on same R/L line as 2*alpha)
      // Seeing the very low statistics for 8He compared to 8Be/2alpha, we assume that if Si-CsI id
      // gives something above 8He it is either incoherent (below 7Li) or 8Be + something else in ChIo-Si
      // (above 7Li).
      if (IDsicsi->IDOK) {
         theID = *IDcsi;
         Int_t Zref = IDcsi->Z;
         Int_t Aref = IDcsi->A;
         if (IDsicsi->Aident) { // Si-CsI provides mass identification

            if (IDcsi->Z == 4 && IDcsi->A == 8) {
               // traitement special 8Be
               // if sicsi => 7Li, it is 8Be (2alpha)
               // if sicsi => 8He, it is 8He
               if (IDsicsi->Z < 2 || (IDsicsi->Z == 2 && IDsicsi->A < 7)) {
                  fCoherent = kFALSE;
                  IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
                  return kTRUE;
               } else if (IDsicsi->Z == 2 && IDsicsi->A > 6 && IDsicsi->A < 10) {
                  // accept helium-7,8,9 as 8He
                  theID = *IDsicsi;
                  return kTRUE;
               } else if ((IDsicsi->Z == 2 && IDsicsi->A > 9) || (IDsicsi->Z == 3 && IDsicsi->A < 6)) {
                  fCoherent = kFALSE;
                  IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
                  return kTRUE;
               } else if (IDsicsi->Z == 3 && IDsicsi->A > 5 && IDsicsi->A < 9) {
                  // accept lithium-6,7,8 as 7Li
                  return kTRUE;
               } else if ((IDsicsi->Z == 3 && IDsicsi->A > 8) || IDsicsi->Z > 3) {
                  fPileup = kTRUE;
                  IDsicsi->SetComment("Second particle stopping in Si, identification ChIo-Si required");
                  return kTRUE;
               }
            }
            // if CsI says A could be bigger and Si-CsI gives same Z and A+1, use Si-CsI
            if ((IDsicsi->Z == Zref) && (IDsicsi->A == (Aref + 1))
                  && (IDcsi->IDquality == KVIDGCsI::kICODE1 || IDcsi->IDquality == KVIDGCsI::kICODE3)) {
               theID = *IDsicsi;
               return kTRUE;
            }
            // if CsI says A could be smaller and Si-CsI gives same Z and A-1, use Si-CsI
            if ((IDsicsi->Z == Zref) && (IDsicsi->A == (Aref - 1))
                  && (IDcsi->IDquality == KVIDGCsI::kICODE2 || IDcsi->IDquality == KVIDGCsI::kICODE3)) {
               theID = *IDsicsi;
               return kTRUE;
            }
            // everything else - Z must be same, A +/- 1 unit
            if (IDsicsi->Z == Zref && TMath::Abs(IDsicsi->A - Aref) < 2) {
               return kTRUE;
            } else if (IDsicsi->Z < Zref || (IDsicsi->Z == Zref && IDsicsi->A < (Aref - 1))) {
               fCoherent = kFALSE;
               IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
               return kTRUE;
            } else  if (IDsicsi->Z > Zref || (IDsicsi->Z == Zref && IDsicsi->A > (Aref + 1))) {
               fPileup = kTRUE;
               IDsicsi->SetComment("Second particle stopping in Si, identification ChIo-Si required");
               return kTRUE;
            }
         } else { // only Z identification from Si-CsI
            if (IDcsi->Z == 4 && IDcsi->A == 8) {
               // traitement special 8Be
               // we ask for Z to be equal 3 in SiCsI, but with no mass identification
               // we do not try for 8He identification
               if (IDsicsi->Z < 3) {
                  fCoherent = kFALSE;
                  IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
                  return kTRUE;
               } else if (IDsicsi->Z == 3) {
                  return kTRUE;
               } else {
                  fPileup = kTRUE;
                  IDsicsi->SetComment("Second particle stopping in Si, identification ChIo-Si required");
                  return kTRUE;
               }
            }
            // everything else
            if (IDsicsi->Z == Zref) {
               return kTRUE;
            } else if (IDsicsi->Z < Zref) {
               fCoherent = kFALSE;
               IDsicsi->SetComment("CsI-R/L & Si-CsI identifications not coherent");
               return kTRUE;
            } else {
               fPileup = kTRUE;

               IDsicsi->SetComment("Second particle stopping in Si, identification ChIo-Si required");
               return kTRUE;
            }
         }
      }
      // in all other cases accept CsI identification
      theID = *IDcsi;
      return kTRUE;
   }
   return kFALSE;
}

//____________________________________________________________________________________________

Bool_t KVINDRAReconNuc::CoherencyChIoCsI(KVIdentificationResult& theID)
{
   // Called by Identify() for particles stopping in CsI detectors on rings 10-17,
   // which have a ChIo detector just in front of them.
   //
   // fPileupChIo = kTRUE if ChIo-CsI identification gives Z >> CsI-R/L identification
   //              this means that the particle identified in CsI-R/L is correct,
   //              and there is probably a second particle which stopped in the ChIo
   //              detector at the same time (will be added as a Zmin/code5)

   KVIdentificationResult* IDcsi = GetIdentificationResult("CSI_R_L");
   KVIdentificationResult* IDcicsi = GetIdentificationResult("CI_CSI");

   fPileupChIo = kFALSE;
   fIncludeEtalonsInCalibration = kFALSE;

   // Unsuccessful/no CsI id attempt with successful ChIo-CsI id ?
   // Then use ChIo-CsI identification result
   if (IDcsi && !IDcsi->IDOK) {
      if (IDcicsi && IDcicsi->IDOK) {
         theID = *IDcicsi;
         return kTRUE;
      }
   }

   // check coherency of CsI-R/L and ChIo-CsI identifications
   if (IDcsi && IDcsi->IDOK) {
      // gammas
      if (IDcsi->IDcode == kIDCode0) {
         theID = *IDcsi;
         return kTRUE;
      }

      // We check the coherency of the identifications
      // Because ChIo-Csi identification is not of very high quality (compared to CsI R-L),
      // we only check that the Z given by ChIo-CsI is < Zref+1
      // If not, we can suspect the presence of another particle in the ChIo

      if (IDcicsi && IDcicsi->IDOK) {
         Int_t Zref = IDcsi->Z;
         if (IDcicsi->Z > (Zref + 1) && fUseFullChIoEnergyForCalib
               && !fPileupSi75) { // do not add pile-up if one already suspected in Si75
            fPileupChIo = kTRUE;
            IDcicsi->SetComment("Possible pile-up in ChIo");
         }
      }

      // in all other cases accept CsI identification
      theID = *IDcsi;
      return kTRUE;
   }
   return kFALSE;
}

Bool_t KVINDRAReconNuc::CoherencyEtalons(KVIdentificationResult& theID)
{
   // Called by Identify() for particles stopping in etalon modules of Rings 10-17.

   KVIdentificationResult* IDcsi, *IDsilicsi, *IDsi75sili, *IDcisi75, *IDcicsi;
   IDcsi = IDsilicsi = IDsi75sili = IDcisi75 = IDcicsi = 0;
   if (StoppedInCsI()) {
      IDcsi = GetIdentificationResult("CSI_R_L");
      IDcicsi = GetIdentificationResult("CI_CSI");
      IDsilicsi = GetIdentificationResult("SILI_CSI");
   }
   IDsi75sili = GetIdentificationResult("SI75_SILI");
   IDcisi75 = GetIdentificationResult("CI_SI75");
   fPileupChIo = kFALSE;
   fIncludeEtalonsInCalibration = kTRUE;

   Bool_t haveCsI = IDcsi && IDcsi->IDOK;
   Bool_t haveSiLiCsI = IDsilicsi && IDsilicsi->IDOK;
   Bool_t haveChIoCsI = IDcicsi && IDcicsi->IDOK;
   Bool_t haveSi75SiLi = IDsi75sili && IDsi75sili->IDOK;
   Bool_t haveChIoSi75 = IDcisi75 && IDcisi75->IDOK;

   // Etalon telescope Si75/SiLi not concerned, only ChIo and CsI.
   // Use standard coherency for rings 10-17
   if ((!GetSiLi() || !GetSiLi()->Fired("Pany")) && (!GetSi75() || !GetSi75()->Fired("Pany")))
      return CoherencyChIoCsI(theID);
   if (haveCsI && !haveSiLiCsI && !haveSi75SiLi)
      return CoherencyChIoCsI(theID);

   // Treat cases where particle hit etalon telescope
   if (StoppedInCsI()) {
      if (haveCsI) {
         // gammas
         if (IDcsi->IDcode == kIDCode0) {
            theID = *IDcsi;
            return kTRUE;
         }

         // successful CsI identification
         // check (again) if particle passed through SiLi
         // if SiLi did not fire but Si75 did, then we assume this particle
         // just passed through ChIo-CsI while another particle stopped in Si75
         if (!GetSiLi()->Fired("Pany") && GetSi75()->Fired("all")) {
            fPileupSi75 = kTRUE; // need to create particle in ChIo-Si75
            fUseFullChIoEnergyForCalib = kFALSE; // calculate ChIo energy for this particle
            return CoherencyChIoCsI(theID);
         }
         theID = *IDcsi;
         if (haveSi75SiLi) {
            // check for heavy fragment in Si75-SiLi
            if (IDsi75sili->Z > theID.Z) {
               if (haveChIoSi75) {
                  // check we don't have a better identification in ChIo-Si75
                  if (IDcisi75->IDquality < IDsi75sili->IDquality && IDcisi75->Z > theID.Z) {
                     fPileupSi75 = kTRUE;
                     IDcisi75->SetComment("CsI identification with another particle stopped in Si75");
                     fUseFullChIoEnergyForCalib = kFALSE; // calculate ChIo energy for this particle
                  } else {
                     fPileupSiLi = kTRUE;
                     IDsi75sili->SetComment("CsI identification with another particle stopped in SiLi");
                     fUseFullChIoEnergyForCalib = kFALSE; // calculate ChIo energy for this particle
                  }
               }
            }
         } else if (haveChIoSi75) {
            // check for heavy fragment in ChIo-Si75
            if (IDcisi75->Z > theID.Z) {
               fPileupSi75 = kTRUE;
               IDcisi75->SetComment("CsI identification with another particle stopped in Si75");
               fUseFullChIoEnergyForCalib = kFALSE; // calculate ChIo energy for this particle
            }
         }
         return kTRUE;
      } else if (haveSiLiCsI) {
         theID = *IDsilicsi;
         if (haveChIoSi75) {
            // check for heavy fragment in ChIo-Si75
            if (IDcisi75->Z > theID.Z) {
               fPileupSi75 = kTRUE;
               IDcisi75->SetComment("CsI identification with another particle stopped in Si75");
               fUseFullChIoEnergyForCalib = kFALSE; // calculate ChIo energy for this particle
            }
         }
         return kTRUE;
      } else if (haveChIoCsI) {
         theID = *IDcicsi;
         // fragment identified in ChIo-CsI
         // therefore it did not cross the etalon telescopes
         // (only Z=1 & 2 pass through SiLi)
         fIncludeEtalonsInCalibration = kFALSE;
         return kTRUE;
      }
   } else if (StoppedInSiLi()) {
      if (haveSi75SiLi) {
         theID = *IDsi75sili;
         // check ChIo-Si75 id is coherent (either no id or Z<=this one)
         if (haveChIoSi75) {
            if (IDcisi75->Z > theID.Z) fPileupChIo = kTRUE;
         }
         return kTRUE;
      } else if (haveChIoSi75) {
         theID = *IDcisi75;
         return kTRUE;
      }
   } else if (StoppedInSi75()) { // MFR march 2014 SiLi is not the real stopping detector
      if (haveChIoSi75) {
         theID = *IDcisi75;
         return kTRUE;
      }                       // end MFR march 2014
   }

   return kFALSE;
}
//____________________________________________________________________________________________

void KVINDRAReconNuc::Identify()
{
   // INDRA-specific particle identification.
   // Here we attribute the Veda6-style general identification codes depending on the
   // result of KVReconstructedNucleus::Identify and the subcodes from the different
   // identification algorithms:
   // If the particle's mass A was NOT measured, we make sure that it is calculated
   // from the measured Z using the mass formula defined by default
   //
   //IDENTIFIED PARTICLES
   //Identified particles with ID code = 2 with subcodes 4 & 5
   //(masse hors limite superieure/inferieure) are relabelled
   //with kIDCode10 (identification entre les lignes CsI)
   //
   //UNIDENTIFIED PARTICLES
   //Unidentified particles receive the general ID code for non-identified particles (kIDCode14)
   //EXCEPT if their identification in CsI R-L gave subcodes 6 or 7
   //(Zmin) then they are relabelled "Identified" with IDcode = 9 (ident. incomplete dans CsI ou Phoswich (Z.min))
   //Their "identifying" telescope is set to the CsI ID telescope

   KVReconstructedNucleus::Identify();

   KVIdentificationResult partID;
   Bool_t ok = kFALSE;

   // INDRA coherency treatment
   if (GetRingNumber() < 10) {
      if (StoppedInCsI()) {
         // particles stopping in CsI detectors on rings 1-9
         // check coherency of CsI-R/L and Si-CsI identifications
         ok = CoherencySiCsI(partID);
         // we check that the ChIo contribution is sane:
         // if no other particles hit this group, the Z given by the ChIoSi
         // must be <= the Z found from Si-CsI or CsI-RL identification

         //if(fCoherent && !fPileup)
         fUseFullChIoEnergyForCalib = CoherencyChIoSiCsI(partID);
      } else {
         // particle stopped in Si (=> ChIo-Si) or ChIo (=> Zmin)
         Int_t id_no = 1;
         KVIdentificationResult* pid = GetIdentificationResult(id_no);
         while (pid->IDattempted) {
            if (pid->IDOK) {
               ok = kTRUE;
               partID = *pid;
               break;
            }
            ++id_no;
            pid = GetIdentificationResult(id_no);
         }
         fUseFullChIoEnergyForCalib = !(GetChIo() && GetChIo()->GetNHits() > 1);
      }
   } else {
      //identification couronne 10 a 17
      //Arret dans les CsI, coherence entre identification CsI RL et ChIo CsI

      // if particle is alone in group, we can in principle attribute the ChIo energy
      // to the energy lost by this particle alone
      fUseFullChIoEnergyForCalib = !(GetChIo() && GetChIo()->GetNHits() > 1);
      if (StoppedInCsI()) {

         if (GetSiLi() || GetSi75()) /* etalon module */
            ok = CoherencyEtalons(partID);
         else
            ok = CoherencyChIoCsI(partID);

      } else if (StoppedInChIo()) {
         // particle stopped in ChIo (=> Zmin)
         Int_t id_no = 1;
         KVIdentificationResult* pid = GetIdentificationResult(id_no);
         while (pid->IDattempted) {
            if (pid->IDOK) {
               ok = kTRUE;
               partID = *pid;
               break;
            }
            ++id_no;
            pid = GetIdentificationResult(id_no);
         }
         partID.Print();
      } else {
         // particle stopped in SiLi or Si75 (etalon modules)
         ok = CoherencyEtalons(partID);
      }

   }
   if (ok) {
      SetIsIdentified();
      KVIDTelescope* idt = (KVIDTelescope*)GetIDTelescopes()->FindObjectByType(partID.GetType());
      if (!idt) {
         Warning("Identify", "cannot find ID telescope with type %s", partID.GetType());
         GetIDTelescopes()->ls();
         partID.Print();
      }
      SetIdentifyingTelescope(idt);
      SetIdentification(&partID);
   }

   if (IsIdentified()) {

      /******* IDENTIFIED PARTICLES *******/
      if (GetIdentifyingTelescope()->InheritsFrom("KVIDCsI")) {     /**** CSI R-L IDENTIFICATION ****/

         //Identified particles with ID code = 2 with subcodes 4 & 5
         //(masse hors limite superieure/inferieure) are relabelled
         //with kIDCode10 (identification entre les lignes CsI)

         Int_t csi_subid = GetIDSubCode();
         if (csi_subid == KVIDGCsI::kICODE4 || csi_subid == KVIDGCsI::kICODE5) {
            SetIDCode(kIDCode10);
         }

      }

   } else {

      /******* UNIDENTIFIED PARTICLES *******/

      /*** general ID code for non-identified particles ***/
      SetIDCode(kIDCode14);

      KVIDCsI* idtel = (KVIDCsI*)GetIDTelescopes()->FindObjectByType("CSI_R_L");
      if (idtel) {
         //Particles remaining unidentified are checked: if their identification in CsI R-L gave subcodes 6 or 7
         //(Zmin) then they are relabelled "Identified" with IDcode = 9 (ident. incomplete dans CsI ou Phoswich (Z.min))
         //Their "identifying" telescope is set to the CsI ID telescope
         Int_t csi_subid = GetIDSubCode("CSI_R_L");
         if (csi_subid == KVIDGCsI::kICODE6 || csi_subid == KVIDGCsI::kICODE7) {
            SetIsIdentified();
            SetIDCode(kIDCode9);
            SetIdentifyingTelescope(idtel);
         }
      }

   }
}

//_________________________________________________________________________________

void KVINDRAReconNuc::Calibrate()
{
   // Calculate and set the energy of a (previously identified) reconstructed particle.
   // For particles in rings 10-17
   // For particles in rings 1-9, we use the results of the ChIo-Si-CsI coherency tests in order
   // to calculate their energy.


   if (GetRingNumber() <= 9)
      CalibrateRings1To9();
   else
      CalibrateRings10To17();


   SetIsCalibrated();
   //add correction for target energy loss - moving charged particles only!
   Double_t E_targ = 0.;
   if (GetZ() && GetEnergy() > 0) {
      E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
      SetTargetEnergyLoss(E_targ);
   }
   Double_t E_tot = GetEnergy() + E_targ;
   SetEnergy(E_tot);
   // set particle momentum from telescope dimensions (random)
   GetAnglesFromStoppingDetector();
   CheckCsIEnergy();

}

void KVINDRAReconNuc::DoGammaCalibration()
{
   // no calibration is performed for gammas
   SetNoCalibrationStatus();
   return;
}

void KVINDRAReconNuc::DoNeutronCalibration()
{
   // use energy of CsI calculated using the Z & A of the CsI identification
   // to calculate the energy deposited by the neutron
   KVIdentificationResult* IDcsi = GetIdentificationResult(1);
   KVNucleus tmp(IDcsi->Z, IDcsi->A);
   if (GetCsI() && GetCsI()->IsCalibrated()) {
      fECsI = GetCsI()->GetCorrectedEnergy(&tmp, -1., kFALSE);
      SetEnergy(fECsI);
      SetECode(kECode2); // not a real energy measure
   } else {
      SetNoCalibrationStatus();
   }
}

void KVINDRAReconNuc::DoBeryllium8Calibration()
{
   // Beryllium-8 = 2 alpha particles of same energy
   // We halve the total light output of the CsI to calculate the energy of 1 alpha
   // Then multiply resulting energy by 2
   // Note: fECsI is -ve, because energy is calculated not measured
   Double_t half_light = GetCsI()->GetLumiereTotale() * 0.5;
   KVNucleus tmp(2, 4);
   fECsI = -2.*GetCsI()->GetCorrectedEnergy(&tmp, half_light, kFALSE);
   SetECode(kECode2);
}

Bool_t KVINDRAReconNuc::CalculateSiliconDEFromResidualEnergy()
{
   // calculate fESi from fECsI
   // returns kTRUE if OK
   Double_t e0 = GetSi()->GetDeltaEFromERes(GetZ(), GetA(), TMath::Abs(fECsI));
   GetSi()->SetEResAfterDetector(TMath::Abs(fECsI));
   fESi = GetSi()->GetCorrectedEnergy(this, e0);
   if (fESi <= 0) {
      // can't calculate fESi from CsI energy - bad
      SetBadCalibrationStatus();
      return kFALSE;
   } else {
      // calculated energy: negative
      fESi = -TMath::Abs(fESi);
      SetECode(kECode2);
   }
   return kTRUE;
}

void KVINDRAReconNuc::CalibrateRings1To9()
{
   // Special calibration for particles in rings 1 to 9
   // We set the energy calibration code for the particle here
   //    kECode0 = no calibration (e.g. gammas)
   //    kECode1 = everything OK
   //    kECode2 = small warning, for example if energy loss in a detector is calculated
   //    kECode15 = bad, calibration is no good
   // The contributions from ChIo, Si, and CsI are stored in member variables fEChIo, fESi, fECsI
   // If the contribution is calculated rather than measured (see below), it is stored as a negative value.
   // NOTE: in no case do we ever calculate an energy for uncalibrated detector using measured energy
   //       loss in the detector placed in front (i.e. calculate ECsI from deSi, or calculate ESi
   //       from deChIo) as this gives wildly varying (mostly false) results.
   //
   // For gammas (IDCODE=0): no calibration performed, energy will be zero, ECODE=kECode0
   // For neutrons (IDCODE=1): if CsI is calibrated, we use the CsI light response to calculate
   //                          the equivalent energy for a proton.
   // For particles stopping in the CsI detector (IDCODE=2 or 3):
   //    - for "8Be" we use half the CsI light output to calculate 4He kinetic energy which is then doubled
   //      (we assume two 4He of identical energy), and ECODE=kECode2 (calculated)
   //    - if no calibration exists for the CsI, we cannot calibrate the particle (ECODE=kECode0)
   //    - if calibrated CsI energy is bizarre (<=0), we cannot calibrate the particle (ECODE=kECode15)
   //    - if the Silicon detector is not calibrated, or if coherency analysis indicates a pile-up
   //      in the silicon or other inconsistency, the Silicon energy is calculated from the calibrated
   //      CsI energy (ECODE=kECode2)
   //    - if the Ionisation Chamber is not calibrated, or if coherency analysis indicates a pile-up
   //      in the ChIo or other inconsistency, this contribution is calculated using the total
   //      cumulated energy measured in the CsI and/or Silicon detectors (ECODE=kECode2)
   //
   // For particles stopping in the Silicon detector (IDCODE=3, 4 or 5):
   //    - if no calibration exists for the Silicon, we cannot calibrate the particle (ECODE=kECode0)
   //    - if the Ionisation Chamber is not calibrated, or if coherency analysis indicates a pile-up
   //      in the ChIo or other inconsistency, this contribution is calculated using the total
   //      cumulated energy measured in the CsI and/or Silicon detectors (ECODE=kECode2)

   fECsI = fESi = fEChIo = 0;

   if (GetCodes().TestIDCode(kIDCode_Gamma)) {
      DoGammaCalibration();
      return;
   }
   if (GetCodes().TestIDCode(kIDCode_Neutron)) {
      DoNeutronCalibration();
      return;
   }

   SetECode(kECode1);

   Bool_t stopped_in_silicon = kTRUE;

   if (GetCsI()) {
      stopped_in_silicon = kFALSE;
      if (GetCsI()->IsCalibrated()) {
         /* CSI ENERGY CALIBRATION */
         if (GetCodes().TestIDCode(kIDCode_CsI) && GetZ() == 4 && GetA() == 8) {
            DoBeryllium8Calibration();
         } else
            fECsI = GetCsI()->GetCorrectedEnergy(this, -1., kFALSE);
      } else {
         SetNoCalibrationStatus();// no CsI calibration - no calibration performed
         return;
      }
      if (GetCodes().TestECode(kECode1) && fECsI <= 0) { // for kECode2, fECsI is always <0
         //Info("Calib", "ECsI = %f",fECsI);
         SetBadCalibrationStatus();// problem with CsI energy - no calibration
         return;
      }
   }
   if (GetSi()) {
      /* SILICIUM ENERGY CONTRIBUTION */
      // if fPileup = kTRUE, the Silicon energy appears to include a contribution from another particle
      //     therefore we have to estimate the silicon energy for this particle using the CsI energy
      // if fCoherent = kFALSE, the Silicon energy is too small to be consistent with the CsI identification,
      //     therefore we have to estimate the silicon energy for this particle using the CsI energy

      if (!fPileup && fCoherent && GetSi()->IsCalibrated()) {
         // all is apparently well. use full energy deposited in silicon.
         Bool_t si_transmission = kTRUE;
         if (stopped_in_silicon) {
            si_transmission = kFALSE;
         } else {
            GetSi()->SetEResAfterDetector(TMath::Abs(fECsI));
         }
         fESi = GetSi()->GetCorrectedEnergy(this, -1., si_transmission);
         if (fESi <= 0) {
            // problem with silicon calibration
            if (!stopped_in_silicon && (TMath::Abs(fECsI) > 0.0)) {
               if (!CalculateSiliconDEFromResidualEnergy()) return;
            } else {
               // stopped in Si with bad Si calibration - no good
               SetBadCalibrationStatus();
               return;
            }
         }
      } else {
         if (!stopped_in_silicon) {
            if (!CalculateSiliconDEFromResidualEnergy()) return;
         } else {
            // cannot calibrate particle stopping in silicon in this case
            SetNoCalibrationStatus();
            return;
         }
      }

   }
   if (GetChIo()) {
      /* CHIO ENERGY CONTRIBUTION */
      // if fUseFullChIoEnergyForCalib = kFALSE, we have to estimate the ChIo energy for this particle
      Double_t ERES = TMath::Abs(fESi) + TMath::Abs(fECsI);
      if (fUseFullChIoEnergyForCalib && GetChIo()->IsCalibrated()) {
         // all is apparently well
         if (!StoppedInChIo()) {
            GetChIo()->SetEResAfterDetector(ERES);
            fEChIo = GetChIo()->GetCorrectedEnergy(this);
         } else {
            // particle stopped in chio, not in transmission
            fEChIo = GetChIo()->GetCorrectedEnergy(this, -1., kFALSE);
         }
         if (fEChIo < 0.) {
            // bad chio calibration
            if (!StoppedInChIo() && (ERES > 0.0)) {
               CalculateChIoDEFromResidualEnergy(ERES);
            }
         }
      } else {
         if (!StoppedInChIo()) {
            CalculateChIoDEFromResidualEnergy(ERES);
         } else {
            // particle stopped in ChIo, no calibration available
            SetNoCalibrationStatus();
            return;
         }
      }
   }
   SetEnergy(TMath::Abs(fECsI) + TMath::Abs(fESi) + TMath::Abs(fEChIo));
}

void KVINDRAReconNuc::CalculateSiLiDEFromResidualEnergy(Double_t ERES)
{
   // Etalon modules
   // calculate fESiLi from residual CsI energy
   Double_t e0 = GetSiLi()->GetDeltaEFromERes(GetZ(), GetA(), ERES);
   GetSiLi()->SetEResAfterDetector(ERES);
   fESiLi = GetSiLi()->GetCorrectedEnergy(this, e0);
   fESiLi = -TMath::Abs(fESiLi);
   SetECode(kECode2);
}

void KVINDRAReconNuc::CalculateSi75DEFromResidualEnergy(Double_t ERES)
{
   // Etalon modules
   // calculate fESi75 from residual CsI+SiLi energy
   Double_t e0 = GetSi75()->GetDeltaEFromERes(GetZ(), GetA(), ERES);
   GetSi75()->SetEResAfterDetector(ERES);
   fESi75 = GetSi75()->GetCorrectedEnergy(this, e0);
   fESi75 = -TMath::Abs(fESi75);
   SetECode(kECode2);
}

void KVINDRAReconNuc::CalculateChIoDEFromResidualEnergy(Double_t ERES)
{
   // calculate fEChIo from residual energy
   Double_t e0 = GetChIo()->GetDeltaEFromERes(GetZ(), GetA(), ERES);
   GetChIo()->SetEResAfterDetector(ERES);
   fEChIo = GetChIo()->GetCorrectedEnergy(this, e0);
   fEChIo = -TMath::Abs(fEChIo);
   SetECode(kECode2);
}

//_________________________________________________________________________________
void KVINDRAReconNuc::CalibrateRings10To17()
{
   // Special calibration for particles in rings 10 to 17
   // We set the energy calibration code for the particle here
   //    kECode0 = no calibration (e.g. gammas)
   //    kECode1 = everything OK
   //    kECode2 = small warning, for example if energy loss in a detector is calculated
   //    kECode15 = bad, calibration is no good
   // The contributions from ChIo & CsI are stored in member variables fEChIo, fECsI
   // If the contribution is calculated rather than measured, it is stored as a negative value

   fECsI = fEChIo = fESi75 = fESiLi = 0;
   if (GetCodes().TestIDCode(kIDCode_Gamma)) {
      DoGammaCalibration();
      return;
   }
   // change fUseFullChioenergyforcalib for "coherency" particles
   // we assume they are calibrated after all other particles in group have
   // been identified, calibrated, and their energy contributions removed
   // from the ChIo
   if (GetCodes().TestIDCode(kIDCode6) || GetCodes().TestIDCode(kIDCode7) || GetCodes().TestIDCode(kIDCode8))
      fUseFullChIoEnergyForCalib = kTRUE;

   SetECode(kECode1);
   Bool_t stopped_in_chio = kTRUE;
   if (GetCsI()) {
      stopped_in_chio = kFALSE;
      /* CSI ENERGY CALIBRATION */
      if (GetCodes().TestIDCode(kIDCode_CsI) && GetZ() == 4 && GetA() == 8) {
         DoBeryllium8Calibration();
      } else
         fECsI = GetCsI()->GetCorrectedEnergy(this, -1., kFALSE);

      if (fECsI <= 0) {
         SetBadCalibrationStatus();// bad - no CsI energy
         return;
      }
   }
   if (fIncludeEtalonsInCalibration) {
      if (GetSiLi()) {
         Double_t ERES = fECsI;
         if (!fPileupSiLi && GetSiLi()->IsCalibrated()) {
            Bool_t si_transmission = kTRUE;
            if (StoppedInSiLi()) {
               si_transmission = kFALSE;
            } else {
               GetSiLi()->SetEResAfterDetector(ERES);
            }
            fESiLi = GetSiLi()->GetCorrectedEnergy(this, -1., si_transmission);
            if (fESiLi <= 0) {
               if (!StoppedInSiLi() && ERES > 0.0) {
                  CalculateSiLiDEFromResidualEnergy(ERES);
               } else {
                  SetBadCalibrationStatus();
                  return;
               }
            }
         } else {
            CalculateSiLiDEFromResidualEnergy(ERES);
         }
      }
      if (GetSi75()) {
         Double_t ERES = fECsI + TMath::Abs(fESiLi);
         if (!fPileupSi75 && !fPileupSiLi && GetSi75()->IsCalibrated()) {
            Bool_t si_transmission = kTRUE;
            if (StoppedInSi75()) {
               si_transmission = kFALSE;
            } else {
               GetSi75()->SetEResAfterDetector(ERES);
            }
            fESi75 = GetSi75()->GetCorrectedEnergy(this, -1., si_transmission);
            if (fESi75 <= 0) {
               if (!StoppedInSi75() && ERES > 0.0) {
                  CalculateSi75DEFromResidualEnergy(ERES);
               } else {
                  SetBadCalibrationStatus();
                  return;
               }
            }
         } else {
            CalculateSi75DEFromResidualEnergy(ERES);
         }
      }
   }
   if (GetChIo()) {
      /* IONISATION CHAMBER ENERGY CONTRIBUTION */
      // if fUseFullChIoEnergyForCalib = kFALSE, ChIo was hit by other particles in group
      //     therefore we have to estimate the ChIo energy for this particle using the CsI energy
      // if fPileupChIo = kTRUE, there appears to be another particle stopped in the ChIo
      //     therefore we have to estimate the ChIo energy for this particle using the CsI energy
      Double_t ERES = fECsI + TMath::Abs(fESiLi) + TMath::Abs(fESi75);
      if (!fPileupChIo && fUseFullChIoEnergyForCalib && GetChIo()->IsCalibrated()) {
         // all is apparently well
         Bool_t ci_transmission = kTRUE;
         if (stopped_in_chio) {
            ci_transmission = kFALSE;
         } else {
            GetChIo()->SetEResAfterDetector(ERES);
         }
         fEChIo = GetChIo()->GetCorrectedEnergy(this, -1., ci_transmission);
         if (fEChIo <= 0) {
            if (!stopped_in_chio && ERES > 0) {
               CalculateChIoDEFromResidualEnergy(ERES);
            }
         }
      } else {
         if (!stopped_in_chio && ERES > 0) {
            CalculateChIoDEFromResidualEnergy(ERES);
         }
      }
   }


   SetEnergy(fECsI + TMath::Abs(fESiLi) + TMath::Abs(fESi75) + TMath::Abs(fEChIo));
}

//________________________________________________________________________________//

void KVINDRAReconNuc::CheckCsIEnergy()
{
   // Check calculated CsI energy loss of particle.
   // If it is greater than the maximum theoretical energy loss
   // (depending on the length of CsI, the Z & A of the particle)
   // we set the energy calibration code to kECode3 (historical VEDA code
   // for particles with E_csi > E_max_csi)

   KVDetector* csi = GetCsI();
   if (csi && GetZ() > 0 && GetZ() < 3 && (csi->GetEnergy() > csi->GetMaxDeltaE(GetZ(), GetA()))) SetECode(kECode3);
}

Int_t KVINDRAReconNuc::GetIDSubCode(const Char_t* id_tel_type, KVIDSubCode&) const
{
   // OBSOLETE METHOD

   KVINDRAIDTelescope* idtel;
   if (strcmp(id_tel_type, ""))
      idtel =
         (KVINDRAIDTelescope*) GetIDTelescopes()->
         FindObjectByType(id_tel_type);
   else
      idtel = (KVINDRAIDTelescope*)GetIdentifyingTelescope();
   if (!idtel)
      return -65535;
//    return idtel->GetIDSubCode(code);
   return -65535;
}

//______________________________________________________________________________________________//

const Char_t* KVINDRAReconNuc::GetIDSubCodeString(const Char_t* id_tel_type, KVIDSubCode&) const
{
   // OBSOLETE METHOD

   KVINDRAIDTelescope* idtel;
   if (strcmp(id_tel_type, ""))
      idtel =
         (KVINDRAIDTelescope*) GetIDTelescopes()->
         FindObjectByType(id_tel_type);
   else
      idtel = (KVINDRAIDTelescope*)GetIdentifyingTelescope();
   if (!idtel) {
      if (strcmp(id_tel_type, ""))
         return Form("No identification attempted in %s", id_tel_type);
      else
         return
            Form("Particle unidentified. Identifying telescope not set.");
   }
   //return idtel->GetIDSubCodeString(code);
   return id_tel_type;
}
