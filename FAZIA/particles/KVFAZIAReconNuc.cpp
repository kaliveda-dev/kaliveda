/***************************************************************************
$Id: KVFAZIAReconNuc.cpp,v 1.61 2009/04/03 14:28:37 franklan Exp $
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
#include "KVFAZIAReconNuc.h"
#include "KVList.h"
#include "TCollection.h"
#include "KVDetector.h"
#include "KVMultiDetArray.h"
#include "KVFAZIADetector.h"
#include "KVIDZAGrid.h"
#include "KVLightEnergyCsIFull.h"
#include "KVLightEnergyCsI.h"
#include "KVIDGCsI.h"

using namespace std;

ClassImp(KVFAZIAReconNuc);

////////////////////////////////////////////////////////////////////////////
//KVFAZIAReconNuc
//
//Nuclei reconstructed from data measured in the FAZIA array.
//Most useful methods are already defined in parent classes KVReconstructedNucleus,
//KVNucleus and KVParticle.
////////////////////////////////////////////////////////////////////////////


void KVFAZIAReconNuc::init()
{
   //default initialisations
   if (gDataSet)
      SetMassFormula(UChar_t(gDataSet->GetDataSetEnv("KVFAZIAReconNuc.MassFormula", Double_t(kEALMass))));
   fECSI = fESI1 = fESI2 = 0.;
}

KVFAZIAReconNuc::KVFAZIAReconNuc()
{
   //default ctor
   init();
}

KVFAZIAReconNuc::KVFAZIAReconNuc(const KVFAZIAReconNuc& obj):
   KVReconstructedNucleus()
{
   //copy ctor
   init();
   obj.Copy(*this);

}

KVFAZIAReconNuc::~KVFAZIAReconNuc()
{
   //dtor
   init();
}

void KVFAZIAReconNuc::Copy(TObject& obj) const
{
   //
   //Copy this to obj
   //
   KVReconstructedNucleus::Copy(obj);
}


void KVFAZIAReconNuc::Print(Option_t*) const
{
   // Print information on particle

   if (IsIdentified()) {

      cout << " =======> ";
      cout << " Z=" << GetZ() << " A=" << ((KVFAZIAReconNuc*) this)->
           GetA();
      if (((KVFAZIAReconNuc*) this)->IsAMeasured()) cout << " Areal=" << ((KVFAZIAReconNuc*) this)->GetRealA();
      else cout << " Zreal=" << GetRealZ();

   }
   else {
      cout << "(unidentified)" << endl;
   }
   if (IsCalibrated()) {
      cout << " Total Energy = " << GetEnergy() << " MeV,  Theta=" << GetTheta() << " Phi=" << GetPhi() << endl;
      cout << "    Target energy loss correction :  " << GetTargetEnergyLoss() << " MeV" << endl;

   }
   else {
      cout << "(uncalibrated)" << endl;
   }

   cout << "Analysis : ";
   switch (GetStatus()) {
      case 0:
         cout <<
              "Particle alone in group, or identification independently of other"
              << endl;
         cout << "particles in group is directly possible." << endl;
         break;

      default:
         cout << "Particle status code" << GetStatus() << endl;
         break;
   }

   cout <<
        "-------------------------------------------------------------------------------"
        << endl;
}

//________________________________________________________________________________________

void KVFAZIAReconNuc::Clear(Option_t* t)
{
   //reset nucleus' properties
   KVReconstructedNucleus::Clear(t);
   init();

}

KVFAZIADetector* KVFAZIAReconNuc::Get(const Char_t* label) const
{
   // Access one of the detectors hit by this particle: "SI1", "SI2", or "CSI"
   // If the requested detector type was not hit (i.e. requesting "CSI" for a particle
   // which stopped in "SI1" or "SI2") then a null pointer will be returned.

   KVFAZIADetector* det = (KVFAZIADetector*) GetDetectorList()->FindObjectByLabel(label);
   return det;
}

KVFAZIADetector* KVFAZIAReconNuc::GetCSI() const
{
   // Access CSI detector hit by particle. If particle stopped before CSI, returns null pointer.
//   return Get("CSI");
   int IDSTOP = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
   // if(IDSTOP==KVFAZIADetector::kSI1) return (KVFAZIADetector*)GetDetector(0);
   // if(IDSTOP==KVFAZIADetector::kSI2) return (KVFAZIADetector*)GetDetector(1);
   if (IDSTOP == KVFAZIADetector::kCSI) return (KVFAZIADetector*)GetDetector(0);
   return nullptr;
}

KVFAZIADetector* KVFAZIAReconNuc::GetSI1() const
{
   // Access SI1 detector hit by particle.
   //return Get("SI1");
   int IDSTOP = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
   if (IDSTOP == KVFAZIADetector::kSI1) return (KVFAZIADetector*)GetDetector(0);
   if (IDSTOP == KVFAZIADetector::kSI2) return (KVFAZIADetector*)GetDetector(1);
   if (IDSTOP == KVFAZIADetector::kCSI) return (KVFAZIADetector*)GetDetector(2);
   return nullptr;
}

KVFAZIADetector* KVFAZIAReconNuc::GetSI2() const
{
   // Access SI2 detector hit by particle. If particle stopped in SI1, returns null pointer.
   //return Get("SI2");
   int IDSTOP = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
   // if(IDSTOP==KVFAZIADetector::kSI1) return (KVFAZIADetector*)GetDetector(0);
   if (IDSTOP == KVFAZIADetector::kSI2) return (KVFAZIADetector*)GetDetector(0);
   if (IDSTOP == KVFAZIADetector::kCSI) return (KVFAZIADetector*)GetDetector(1);
   return nullptr;
}

Int_t KVFAZIAReconNuc::GetIndex() const
{
   // Return index of dectector in which particle stopped
   // (see KVFaziaDetector::GetIndex for meaning of index).
   // If no stopping detector defined (weird?), returns -1.

   KVFAZIADetector* det(nullptr);
   if (!(det = (KVFAZIADetector*)GetStoppingDetector())) return -1;
   return det->GetIndex();
}


Bool_t KVFAZIAReconNuc::StoppedIn(const Char_t* label) const
{
   //Returns kTRUE if particle stopped in the given detector: "SI1", "SI2" or "CSI"
   return (GetStoppingDetector() == Get(label));
}

Int_t KVFAZIAReconNuc::GetIdentifierOfStoppingDetector() const
{
   // Return the identifier of the detector in which particle stopped.
   // For the definition of possible returned values, see KVFAZIADetector::GetIdentifier

   return ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
}

Bool_t KVFAZIAReconNuc::StoppedInSI2() const
{
   return ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier() == KVFAZIADetector::kSI2;
//   return StoppedIn("SI2");
}
Bool_t KVFAZIAReconNuc::StoppedInSI1() const
{
   return ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier() == KVFAZIADetector::kSI1;
   //return StoppedIn("SI1");
}
Bool_t KVFAZIAReconNuc::StoppedInCSI() const
{
   return ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier() == KVFAZIADetector::kCSI;
   //return StoppedIn("CSI");
}



void KVFAZIAReconNuc::Identify()
{
   // Try to identify this nucleus by calling the Identify() function of each
   // ID telescope crossed by it, starting with the telescope where the particle stopped, in order
   //      -  attempt identification only in ID telescopes containing the stopping detector
   //      -  only telescopes which have been correctly initialised for the current run are used,
   //         i.e. those for which KVIDTelescope::IsReadyForID() returns kTRUE.
   // This continues until a successful identification is achieved or there are no more ID telescopes to try.
   // The identification code corresponding to the identifying telescope is set as the identification code of the particle.

   //cout << "Dentro il mio Identify" << endl;

   KVList* idt_list = GetStoppingDetector()->GetIDTelescopes();

   // GetStoppingDetector()->Print();
   int STOPID = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();

   //idt_list->Print();

   KVIdentificationResult* IDR = 0;
   Int_t idnumber = 1;

   if (idt_list && idt_list->GetSize() > 0) {
      // cout << "LOOP over telescopes\n";
      KVIDTelescope* idt;
      TIter next(idt_list);

      while ((idt = (KVIDTelescope*) next())) { // && !IsIdentified()) {
         //cout << "NExt Telescope\n";
         //idt->Print();
         //printf("Checking stopped detector %s\n", idt->GetType());
         if (STOPID == KVFAZIADetector::kSI1 && !strcmp(idt->GetType(), "Si-Si")) continue; // why ?
         if (STOPID == KVFAZIADetector::kSI2 && !strcmp(idt->GetType(), "Si-CsI")) continue; // why ?

         //printf("Initializing IDR\n");
         IDR = GetIdentificationResult(idnumber);
         //printf("Initialized IDR=%p\n", IDR);
         IDR->SetName(idt->GetName());
         IDR->SetType(idt->GetType());
         /* if(!strcmp(idt->GetType(), "Si-CsI")){
             printf("Attempting id with %s tel\n",idt->GetType());
             if(!idt->IsReadyForID()) printf("iDT not ready\n");
          }*/
         if (idt->IsReadyForID()) { // is telescope able to identify for this run ?
            // printf("Attempting id with telescope:\n");
            // idt->Print();
            IDR->IDattempted = kTRUE;
            IDR->IDOK = kFALSE;
            idt->Identify(IDR);
            //  if (!strcmp(idt->GetType(), "Si-CsI")) IDR->Print();
            if (IDR->IDOK && !(IDR->Z >= 3 && IDR->IDcode == 33)) { //Correspond to Quality code <=3 !!!!!Condizione rigetto Z>3 da CSI
               SetIdentification(IDR);
               SetIdentifyingTelescope(idt);
               SetIsIdentified();
               //return;
            }
            else {
               SetIdentification(IDR);
               SetIdentifyingTelescope(idt);
               //SetIDCode(0);
               //SetZandA(0, 0);
               //SetIsIdentified();
            }
         }
         else {
            IDR->IDattempted = kFALSE;
         }
         idnumber += 1;
      }

      KVIdentificationResult partID;
      Bool_t ok = kFALSE;
      //printf("Beginning Coherency check\n");
      if (STOPID == KVFAZIADetector::kSI1) {
         ok = CoherencySi(partID);
      }
      else if (STOPID == KVFAZIADetector::kSI2) {
         ok = CoherencySiSi(partID);
      }
      else if (STOPID == KVFAZIADetector::kCSI) {
         ok = CoherencySiCsI(partID);
      }
      if (ok) {
         SetIsIdentified();
         KVIDTelescope* idt = (KVIDTelescope*)GetIDTelescopes()->FindObjectByType(partID.GetType());
         // if (!idt) {
         //    Warning("Identify", "cannot find ID telescope with type %s", partID.GetType());
         //    GetIDTelescopes()->ls();
         //    partID.Print();
         // }
         SetIdentifyingTelescope(idt);
         SetIdentification(&partID);
         //if(GetZ()==3 && IDCode==33) cout << "fine " << GetZ() << endl;
      }



   }

}

Bool_t KVFAZIAReconNuc::CoherencySi(KVIdentificationResult& theID)
{
   KVIdentificationResult* IDsi = GetIdentificationResult("SiPSA");
   if (IDsi && IDsi->IDOK) {
      theID = *IDsi;
      //cout << "PSA IDCode" << theID.IDcode << endl;
      return kTRUE;
   }
   else return kFALSE;
}

Bool_t KVFAZIAReconNuc::CoherencySiSi(KVIdentificationResult& theID)
{
   KVIdentificationResult* IDsisi = GetIdentificationResult("Si-Si");
   KVIdentificationResult* IDsi = GetIdentificationResult("SiPSA");
   if (IDsisi && IDsisi->IDOK)    {
      theID = *IDsisi;
      //cout << "SiSi IDCode" << theID.IDcode << endl;
      return kTRUE;
   }
   else if (IDsi && IDsi->IDOK) {
      theID = *IDsi;
      // cout << "Si (daSiSi) IDCode" << theID.IDcode << endl;
      return kTRUE;
   }
   else return kFALSE;
}

Bool_t KVFAZIAReconNuc::CoherencySiCsI(KVIdentificationResult& theID)
{
   // printf("Check CSI\n");
   KVIdentificationResult* IDcsi = GetIdentificationResult("CsI");
   KVIdentificationResult* IDsicsi = GetIdentificationResult("Si-CsI");
   fCoherent = kTRUE;
   fPileup = kFALSE;

   // the following procedure has been copied/adapted from KVINDRAReconNuc::CoherencySiCsI()

   // Unsuccessful/no CsI id attempt with successful Si-CsI id
   // Then use Si-CsI identification result

   //Standard

   if (IDsicsi && IDsicsi->IDOK)    {
      theID = *IDsicsi;
      return kTRUE;
   }
   else if (IDcsi && IDcsi->IDOK && IDcsi->Z <= 2) {
      theID = *IDcsi;
      return kTRUE;
   }



   /*    if (IDsicsi->IDOK && !IDcsi->IDOK) {
         theID = *IDsicsi;
         return kTRUE;
      }

      // check coherency of CsI-R/L and Si-CsI identifications
      if (IDcsi->IDOK) {
         // gammas: if sicsi ok->sicsi; else keep csi id
         if (IDcsi->IDcode == 0) {
            theID = *IDcsi;
            return kTRUE;
         }


         if (IDsicsi->IDOK) {

            // priority to Si-CsI identification (if any) for Z>=5
            if (IDsicsi->Z >= 5) {
               theID = *IDsicsi;
               return kTRUE;
            }

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
    */
   return kFALSE;
}

//_________________________________________________________________________________

void KVFAZIAReconNuc::Calibrate()
{
   Int_t STOPID = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
   if (STOPID == KVFAZIADetector::kSI1) CalibrateSi1();
   if (STOPID == KVFAZIADetector::kSI2) CalibrateSi2();
   if (STOPID == KVFAZIADetector::kCSI) CalibrateCsI();
}

void KVFAZIAReconNuc::CalibrateSi1()
{
   KVNucleus avatar;
   Int_t STOPID = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
   Int_t ntot = 0;

   if (STOPID == KVFAZIADetector::kSI1) ntot = 1;
   if (STOPID == KVFAZIADetector::kSI2) return;
   if (STOPID == KVFAZIADetector::kCSI) return;

   if (ntot < 1) {
      return;
   }

   Bool_t punch_through = kFALSE;
   Bool_t incoherency = kFALSE;
   Bool_t pileup = kFALSE;
   Bool_t check_error = kFALSE;
   Bool_t Si1Calib = false;


   // printf("Init variables, stopped in %s\n", ((KVFAZIADetector*)GetStoppingDetector())->GetType());
   double error_si1 = 0;
   Double_t* eloss = new Double_t[ntot];
   for (Int_t ii = 0; ii < ntot; ii += 1) eloss[ii] = 0;
   //TIter next(GetDetectorList());
   KVFAZIADetector* det = 0;
   Int_t idet = 0;
   Int_t ndet = 1;
   Int_t ndet_calib = 0;
   Double_t etot = 0;

   fESI1 = fESI2 = fECSI = 0;

   det = GetSI1();
   if (det->IsCalibrated()) {
      eloss[0] = det->GetEnergy();
      etot = eloss[0];
      ndet_calib = 1;
   }

   //printf("Starting Eloss recon\n");
   if (ndet_calib == 1) {
      Double_t E_targ = 0;
      SetEnergy(etot);
      if (IsAMeasured()) {
         Double_t etot_avatar = 0;
         Double_t chi2 = 0;
         avatar.SetZAandE(GetZ(), GetA(), GetKE());

         det = (KVFAZIADetector*)GetDetector(0);
         Double_t temp = det->GetELostByParticle(&avatar);
         etot_avatar += temp;
         chi2 += TMath::Power((eloss[0] - temp) / eloss[0], 2.);
         avatar.SetKE(avatar.GetKE() - temp);
         error_si1 = (fESI1 - temp) / fESI1;

         chi2 /= ndet;

         if ((avatar.GetKE() / GetKE()) > 0.0) {
            punch_through = kTRUE;
         }
         else if (chi2 > 10.) {
            incoherency = kTRUE;
         }
         else if (TMath::Abs(error_si1) > 0.15) {
            if (StoppedInCSI() && (fECSI / etot) < 0.03) pileup = kTRUE;
            else check_error = kTRUE;
         }
         else {
            // if(avatar.GetZ()==15 && avatar.GetA()==32 && detname==242  && sono_dentro==1) {cout << "CODE 0!!!!!!\n\n\n\n"; getchar();}
            //chi2 /= ndet;
         }

      }
      if (GetZ() && GetEnergy() > 0) {
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         SetTargetEnergyLoss(E_targ);
      }

      Double_t E_tot = GetEnergy() + E_targ;
      SetEnergy(E_tot);
      // set particle momentum from telescope dimensions (random)
      GetAnglesFromStoppingDetector();
      SetECode(0);
      if (punch_through)   SetECode(2);
      if (incoherency)     SetECode(3);
      if (check_error)     SetECode(5); //
      if (pileup)          SetECode(4); //

      SetIsCalibrated();
   }
   delete [] eloss;
}

void KVFAZIAReconNuc::CalibrateSi2()
{
   // Perform energy calibration of (previously identified) particle
   //printf("In Calibrate\n");
   KVNucleus avatar;
   //printf("start Calibrate\n");
   Int_t STOPID = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
   Int_t ntot = 0;

   if (STOPID == KVFAZIADetector::kSI1) return;
   if (STOPID == KVFAZIADetector::kSI2) ntot = 2;
   if (STOPID == KVFAZIADetector::kCSI) return;

   Bool_t punch_through = kFALSE;
   Bool_t incoherency = kFALSE;
   Bool_t pileup = kFALSE;
   Bool_t check_error = kFALSE;
   Bool_t Si1Calib = false;
   Bool_t Si2Calib = false;
   Bool_t CsICalib = false;

   // printf("Init variables, stopped in %s\n", ((KVFAZIADetector*)GetStoppingDetector())->GetType());
   double error_si1 = 0, error_si2 = 0; // error_csi=0;
   Double_t* eloss = new Double_t[ntot];
   for (Int_t ii = 0; ii < ntot; ii += 1) eloss[ii] = 0;
   //TIter next(GetDetectorList());
   KVFAZIADetector* det = 0;
   Int_t idet = 0;
   Int_t ndet = 0;
   Int_t ndet_calib = 0;
   Double_t etot = 0;

   fESI1 = fESI2 = fECSI = 0;

   while (det = (KVFAZIADetector*)GetDetector(idet)) {
      // printf("Det %d of %d (%s)\n",idet,ntot,det->GetType());
      if (det->IsCalibrated()) {
         eloss[ntot - ndet - 1] = det->GetEnergy();
         if (det->GetIdentifier() == KVFAZIADetector::kSI1) {
            fESI1 = eloss[ntot - ndet - 1];
            Si1Calib = true;
         }
         else if (det->GetIdentifier() == KVFAZIADetector::kSI2) {
            fESI2 = eloss[ntot - ndet - 1];
            Si2Calib = true;
         }
         etot += eloss[ntot - ndet - 1];
         ndet_calib += 1;
      }
      ndet += 1;
      idet += 1;
      if (idet == ntot) break;
   }

   if (ndet == ndet_calib) {
      Double_t E_targ = 0;
      SetEnergy(etot);

      if (IsAMeasured()) {
         Double_t etot_avatar = 0;
         Double_t chi2 = 0;
         avatar.SetZAandE(GetZ(), GetA(), GetKE());
         for (Int_t nn = ntot - 1; nn >= 0; nn -= 1) {
            det = (KVFAZIADetector*)GetDetector(nn);
            Double_t temp = det->GetELostByParticle(&avatar);
            etot_avatar += temp;
            chi2 += TMath::Power((eloss[ntot - 1 - nn] - temp) / eloss[ntot - 1 - nn], 2.);
            avatar.SetKE(avatar.GetKE() - temp);
            if (det->GetIdentifier() == KVFAZIADetector::kSI1)      error_si1 = (fESI1 - temp) / fESI1;
            else if (det->GetIdentifier() == KVFAZIADetector::kSI2) error_si2 = (fESI2 - temp) / fESI2;
         }

         chi2 /= ndet;

         if ((avatar.GetKE() / GetKE()) > 0.0) {
            punch_through = kTRUE;
         }
         else if (chi2 > 10.) {
            incoherency = kTRUE;
         }
         else if (TMath::Abs(error_si1) > 0.15 || TMath::Abs(error_si1) + TMath::Abs(error_si2) > 0.15) {
            if (StoppedInCSI() && (fECSI / etot) < 0.03) pileup = kTRUE;
            else check_error = kTRUE;
         }
         else {
            // if(avatar.GetZ()==15 && avatar.GetA()==32 && detname==242  && sono_dentro==1) {cout << "CODE 0!!!!!!\n\n\n\n"; getchar();}
            //chi2 /= ndet;
         }
      }
      if (GetZ() && GetEnergy() > 0) {
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         SetTargetEnergyLoss(E_targ);
      }

      Double_t E_tot = GetEnergy() + E_targ;
      SetEnergy(E_tot);
      // set particle momentum from telescope dimensions (random)
      GetAnglesFromStoppingDetector();
      SetECode(0);
      if (punch_through)   SetECode(2);
      if (incoherency)     SetECode(3);
      if (check_error)     SetECode(5); //
      if (pileup)          SetECode(4); //

      SetIsCalibrated();
   }
   else if (ndet_calib == 1) {
      if (Si1Calib) {
         //try to recover Si2 Energy loss from Si1
         //Assign mass if not present
         if (!IsAMeasured()) {
            if (GetZ() == 1)       SetA(1);
            else if (GetZ() == 2)  SetA(4);
            else if (GetZ() == 20) SetA(48);
            else {
               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
            }
         }
         Double_t E_targ = 0;
         fESI2 = GetSI1()->GetEResFromDeltaE(GetZ(), GetA(), fESI1);
         SetEnergy(fESI1 + fESI2);
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         Double_t E_tot = GetEnergy() + E_targ;
         SetECode(1);
         SetIsCalibrated();
         SetEnergy(E_tot);
         GetAnglesFromStoppingDetector();
      }
      else {
         if (!IsAMeasured()) {
            if (GetZ() == 1)       SetA(1);
            else if (GetZ() == 2)  SetA(4);
            else if (GetZ() == 20) SetA(48);
            else {
               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
            }
         }
         Double_t E_targ = 0;
         fESI1 = GetSI2()->GetDeltaEFromERes(GetZ(), GetA(), fESI2);
         SetEnergy(fESI1 + fESI2);
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         Double_t E_tot = GetEnergy() + E_targ;
         SetECode(1);
         SetIsCalibrated();
         SetEnergy(E_tot);
         GetAnglesFromStoppingDetector();
      }
   }
   delete [] eloss;
}

void KVFAZIAReconNuc::CalibrateCsI()
{
   if (GetZ() <= 2) CalibrateCsI_Light();
   else CalibrateCsI_Heavy();
}

void KVFAZIAReconNuc::CalibrateCsI_Heavy()
{
   // Perform energy calibration of (previously identified) particle
   //printf("In Calibrate\n");
   KVNucleus avatar;
   //printf("start Calibrate\n");
   Int_t STOPID = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
   Int_t ntot = 0;

   if (STOPID == KVFAZIADetector::kSI1) return;
   if (STOPID == KVFAZIADetector::kSI2) return;
   if (STOPID == KVFAZIADetector::kCSI) ntot = 3;

   if (ntot < 1) {
      return;
   }
   Bool_t punch_through = kFALSE;
   Bool_t incoherency = kFALSE;
   Bool_t pileup = kFALSE;
   Bool_t check_error = kFALSE;
   Bool_t Si1Calib = false;
   Bool_t Si2Calib = false;
   Bool_t CsICalib = false;

   // printf("Init variables, stopped in %s\n", ((KVFAZIADetector*)GetStoppingDetector())->GetType());
   double error_si1 = 0, error_si2 = 0; // error_csi=0;
   Double_t* eloss = new Double_t[ntot];
   for (Int_t ii = 0; ii < ntot; ii += 1) eloss[ii] = 0;
   //TIter next(GetDetectorList());
   KVFAZIADetector* det = 0;
   Int_t idet = 0;
   Int_t ndet = 0;
   Int_t ndet_calib = 0;
   Double_t etot = 0;

   fESI1 = fESI2 = fECSI = 0;

   while (det = (KVFAZIADetector*)GetDetector(idet)) {
      // printf("Det %d of %d (%s)\n",idet,ntot,det->GetType());
      if (det->IsCalibrated()) {
         //Csi calibration will always be ignored
         if (det->GetIdentifier() != KVFAZIADetector::kCSI) {
            eloss[ntot - ndet - 1] = det->GetEnergy();
            if (det->GetIdentifier() == KVFAZIADetector::kSI1) {
               fESI1 = eloss[ntot - ndet - 1];
               Si1Calib = true;
            }
            else if (det->GetIdentifier() == KVFAZIADetector::kSI2) {
               fESI2 = eloss[ntot - ndet - 1];
               Si2Calib = true;
            }
            etot += eloss[ntot - ndet - 1];
            ndet_calib += 1;
         }
      }
      ndet += 1;
      idet += 1;
      if (idet == ntot) break;
   }
   //printf("Starting Eloss recon\n");
   //this will never happen
   if (ndet == ndet_calib) {
      Double_t E_targ = 0;
      SetEnergy(etot);

      if (IsAMeasured()) {
         Double_t etot_avatar = 0;
         Double_t chi2 = 0;
         avatar.SetZAandE(GetZ(), GetA(), GetKE());
         for (Int_t nn = ntot - 1; nn >= 0; nn -= 1) {
            det = (KVFAZIADetector*)GetDetector(nn);
            Double_t temp = det->GetELostByParticle(&avatar);
            etot_avatar += temp;
            chi2 += TMath::Power((eloss[ntot - 1 - nn] - temp) / eloss[ntot - 1 - nn], 2.);
            avatar.SetKE(avatar.GetKE() - temp);
            if (det->GetIdentifier() == KVFAZIADetector::kSI1)      error_si1 = (fESI1 - temp) / fESI1;
            else if (det->GetIdentifier() == KVFAZIADetector::kSI2) error_si2 = (fESI2 - temp) / fESI2;
         }

         chi2 /= ndet;

         if ((avatar.GetKE() / GetKE()) > 0.0) {
            punch_through = kTRUE;
         }
         else if (chi2 > 10.) {
            incoherency = kTRUE;
         }
         else if (TMath::Abs(error_si1) > 0.15 || TMath::Abs(error_si1) + TMath::Abs(error_si2) > 0.15) {
            if (StoppedInCSI() && (fECSI / etot) < 0.03) pileup = kTRUE;
            else check_error = kTRUE;
         }
         else {
            // if(avatar.GetZ()==15 && avatar.GetA()==32 && detname==242  && sono_dentro==1) {cout << "CODE 0!!!!!!\n\n\n\n"; getchar();}
            //chi2 /= ndet;
         }

      }
      if (GetZ() && GetEnergy() > 0) {
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         SetTargetEnergyLoss(E_targ);
      }

      Double_t E_tot = GetEnergy() + E_targ;
      SetEnergy(E_tot);
      // set particle momentum from telescope dimensions (random)
      GetAnglesFromStoppingDetector();
      SetECode(0);
      if (punch_through)   SetECode(2);
      if (incoherency)     SetECode(3);
      if (check_error)     SetECode(5); //
      if (pileup)          SetECode(4); //

      SetIsCalibrated();
   }
   else if (ndet_calib == 2) {
      //both Si1 and Si2 are calibrated
      if (!IsAMeasured()) {

         if (GetZ() == 1)       SetA(1);
         else if (GetZ() == 2)  SetA(4);
         else if (GetZ() == 20) SetA(48);
         else {
            SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
         }
      }

      Double_t E_targ = 0;
      fECSI = GetSI2()->GetEResFromDeltaE(GetZ(), GetA(), fESI2);
      SetEnergy(fECSI + fESI1 + fESI2);

      E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
      Double_t E_tot = GetEnergy() + E_targ;
      SetECode(1);
      SetIsCalibrated();
      SetEnergy(E_tot);
      GetAnglesFromStoppingDetector();
   }
   else if (ndet_calib == 1) {
      if (Si2Calib) { //recover Esi1 and EcsI from Esi2
         if (!IsAMeasured()) {

            if (GetZ() == 1)       SetA(1);
            else if (GetZ() == 2)  SetA(4);
            else if (GetZ() == 20) SetA(48);
            else {
               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
            }
         }

         Double_t E_targ = 0;
         fECSI = GetSI2()->GetEResFromDeltaE(GetZ(), GetA(), fESI2);
         fESI1 = GetSI1()->GetDeltaEFromERes(GetZ(), GetA(), fESI2 + fECSI);
         SetEnergy(fECSI + fESI2 + fESI1);

         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         Double_t E_tot = GetEnergy() + E_targ;
         SetECode(1);
         SetIsCalibrated();
         SetEnergy(E_tot);
         GetAnglesFromStoppingDetector();
      }
      else { //recover Esi2 and ECSI from ESi1
         if (!IsAMeasured()) {

            if (GetZ() == 1)       SetA(1);
            else if (GetZ() == 2)  SetA(4);
            else if (GetZ() == 20) SetA(48);
            else {
               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
            }
         }
         Double_t E_targ = 0;
         Double_t Eres = GetSI1()->GetEResFromDeltaE(GetZ(), GetA(), fESI1);
         avatar.SetZAandE(GetZ(), GetA(), Eres);
         fESI2 = GetSI2()->GetELostByParticle(&avatar);
         fECSI = Eres - fESI2;
         SetEnergy(fECSI + fESI2 + fESI1);
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         Double_t E_tot = GetEnergy() + E_targ;
         SetECode(6);
         SetIsCalibrated();
         SetEnergy(E_tot);
         GetAnglesFromStoppingDetector();
      }
   }
   delete [] eloss;
}

void KVFAZIAReconNuc::CalibrateCsI_Light()
{
   // Perform energy calibration of (previously identified) particle
   //printf("In Calibrate\n");

   Obsolete("CalibrateCsI_Light", "1.11", "1.12");

//   KVNucleus avatar;
//   //printf("start Calibrate\n");
//   Int_t STOPID = ((KVFAZIADetector*)GetStoppingDetector())->GetIdentifier();
//   Int_t ntot = 0;

//   if (STOPID == KVFAZIADetector::kSI1) return;
//   if (STOPID == KVFAZIADetector::kSI2) return;
//   if (STOPID == KVFAZIADetector::kCSI) ntot = 3;

//   if (ntot < 1) {
//      return;
//   }
//   Bool_t punch_through = kFALSE;
//   Bool_t incoherency = kFALSE;
//   Bool_t pileup = kFALSE;
//   Bool_t check_error = kFALSE;
//   Bool_t Si1Calib = false;
//   Bool_t Si2Calib = false;
//   Bool_t CsICalib = false;

//   // printf("Init variables, stopped in %s\n", ((KVFAZIADetector*)GetStoppingDetector())->GetType());
//   double error_si1 = 0, error_si2 = 0; // error_csi=0;
//   Double_t* eloss = new Double_t[ntot];
//   for (Int_t ii = 0; ii < ntot; ii += 1) eloss[ii] = 0;
//   //TIter next(GetDetectorList());
//   KVFAZIADetector* det = 0;
//   Int_t idet = 0;
//   Int_t ndet = 0;
//   Int_t ndet_calib = 0;
//   Double_t etot = 0;

//   fESI1 = fESI2 = fECSI = 0;

//   while (det = (KVFAZIADetector*)GetDetector(idet)) {
//      // printf("Det %d of %d (%s)\n",idet,ntot,det->GetType());
//      if (det->IsCalibrated()) {
//         if (det->GetIdentifier() == KVFAZIADetector::kCSI) {
//            CsICalib = true;
//            if (det->GetCalibrator("Channel-Energy")->InheritsFrom("KVLightEnergyCsIFull")) {
//               KVLightEnergyCsIFull* calib = (KVLightEnergyCsIFull*)det->GetCalibrator("Channel-Energy");
//               calib->SetZ(GetZ());
//               calib->SetA(GetA());
//               eloss[ntot - ndet - 1] = calib->Compute(det->GetDetectorSignalValue("Q3.Amplitude"));
//               //cout << detname << " " << calib->GetParameter(0) << endl;

//            }
//            else if (det->GetCalibrator("Channel-Energy")->InheritsFrom("KVLightEnergyCsI") && GetZ()) {
//               KVLightEnergyCsI* calib = (KVLightEnergyCsI*)det->GetCalibrator("Channel-Energy");
//               calib->SetZ(GetZ());
//               calib->SetA(GetA());
//               //cout << detname << " " << calib->GetParameter(0) << endl;
//               eloss[ntot - ndet - 1] = calib->Compute(det->GetDetectorSignalValue("Q3.Amplitude"));
//            }
//         }
//         else eloss[ntot - ndet - 1] = det->GetEnergy();

//         if (det->GetIdentifier() == KVFAZIADetector::kSI1) {
//            fESI1 = eloss[ntot - ndet - 1];
//            Si1Calib = true;
//         }
//         else if (det->GetIdentifier() == KVFAZIADetector::kSI2) {
//            fESI2 = eloss[ntot - ndet - 1];
//            Si2Calib = true;
//         }
//         else if (det->GetIdentifier() == KVFAZIADetector::kCSI) fECSI = eloss[ntot - ndet - 1];
//         etot += eloss[ntot - ndet - 1];
//         ndet_calib += 1;

//      }
//      ndet += 1;
//      idet += 1;
//      if (idet == ntot) break;
//      /*if(GetZ()==8&& (detname==234||detname==221)){
//      *     cout << det->GetName() << " " <<GetZ() << " " << GetA() << " " << fESI1 << " " << fESI2 << " " << fECSI << endl;
//      *     cout << ndet << " = " << ndet_calib << endl;
//      }*/
//   }
//   /* if (GetZ() == 3) {
//       cout << ndet << " " << ndet_calib << endl;
//    }*/
//   //printf("Starting Eloss recon\n");
//   if (ndet == ndet_calib) {
//      Double_t E_targ = 0;
//      SetEnergy(etot);

//      if (IsAMeasured()) {
//         Double_t etot_avatar = 0;
//         Double_t chi2 = 0;
//         avatar.SetZAandE(GetZ(), GetA(), GetKE());
//         for (Int_t nn = ntot - 1; nn >= 0; nn -= 1) {
//            det = (KVFAZIADetector*)GetDetector(nn);
//            Double_t temp = det->GetELostByParticle(&avatar);
//            etot_avatar += temp;
//            chi2 += TMath::Power((eloss[ntot - 1 - nn] - temp) / eloss[ntot - 1 - nn], 2.);
//            avatar.SetKE(avatar.GetKE() - temp);
//            if (det->GetIdentifier() == KVFAZIADetector::kSI1)      error_si1 = (fESI1 - temp) / fESI1;
//            else if (det->GetIdentifier() == KVFAZIADetector::kSI2) error_si2 = (fESI2 - temp) / fESI2;
//         }

//         chi2 /= ndet;

//         if ((avatar.GetKE() / GetKE()) > 0.0) {
//            punch_through = kTRUE;
//         }
//         else if (chi2 > 10.) {
//            incoherency = kTRUE;
//         }
//         else if (TMath::Abs(error_si1) > 0.15 || TMath::Abs(error_si1) + TMath::Abs(error_si2) > 0.15) {
//            if (StoppedInCSI() && (fECSI / etot) < 0.03) pileup = kTRUE;
//            else check_error = kTRUE;
//         }
//         else {
//            // if(avatar.GetZ()==15 && avatar.GetA()==32 && detname==242  && sono_dentro==1) {cout << "CODE 0!!!!!!\n\n\n\n"; getchar();}
//            //chi2 /= ndet;
//         }

//      }
//      if (GetZ() && GetEnergy() > 0) {
//         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
//         SetTargetEnergyLoss(E_targ);
//      }

//      Double_t E_tot = GetEnergy() + E_targ;
//      SetEnergy(E_tot);
//      // set particle momentum from telescope dimensions (random)
//      GetAnglesFromStoppingDetector();
//      SetECode(0);
//      if (punch_through)   SetECode(2);
//      if (incoherency)     SetECode(3);
//      if (check_error)     SetECode(5); //
//      if (pileup)          SetECode(4); //

//      SetIsCalibrated();
//   }
//   else if (ndet_calib == 2) {
//      if (!CsICalib) { //Si1 and Si2 are calibrated
//         if (!IsAMeasured()) {

//            if (GetZ() == 1)       SetA(1);
//            else if (GetZ() == 2)  SetA(4);
//            else if (GetZ() == 20) SetA(48);
//            else {
//               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
//            }
//         }

//         Double_t E_targ = 0;
//         fECSI = GetSI2()->GetEResFromDeltaE(GetZ(), GetA(), fESI2);
//         SetEnergy(fECSI + fESI1 + fESI2);

//         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
//         Double_t E_tot = GetEnergy() + E_targ;
//         SetECode(1);
//         SetIsCalibrated();
//         SetEnergy(E_tot);
//         GetAnglesFromStoppingDetector();
//      }
//      else if (!Si2Calib) {
//         if (!IsAMeasured()) {

//            if (GetZ() == 1)       SetA(1);
//            else if (GetZ() == 2)  SetA(4);
//            else if (GetZ() == 20) SetA(48);
//            else {
//               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
//            }
//         }
//         Double_t E_targ = 0;
//         Double_t Eres = GetSI1()->GetEResFromDeltaE(GetZ(), GetA(), fESI1);
//         avatar.SetZAandE(GetZ(), GetA(), Eres);
//         fESI2 = GetSI2()->GetELostByParticle(&avatar);
//         //  fECSI=Eres-fESI2;
//         SetEnergy(fECSI + fESI2 + fESI1);
//         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
//         Double_t E_tot = GetEnergy() + E_targ;
//         SetECode(8);
//         SetIsCalibrated();
//         SetEnergy(E_tot);
//         GetAnglesFromStoppingDetector();
//      }
//      else {
//         if (!IsAMeasured()) {

//            if (GetZ() == 1)       SetA(1);
//            else if (GetZ() == 2)  SetA(4);
//            else if (GetZ() == 20) SetA(48);
//            else {
//               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
//            }
//         }
//         Double_t E_targ = 0;
//         fESI1 = GetSI1()->GetDeltaEFromERes(GetZ(), GetA(), fESI2 + fECSI);
//         SetEnergy(fECSI + fESI2 + fESI1);

//         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
//         Double_t E_tot = GetEnergy() + E_targ;
//         SetECode(1);
//         SetIsCalibrated();
//         SetEnergy(E_tot);
//         GetAnglesFromStoppingDetector();




//      }
//   }
//   else if (ndet_calib == 1) {
//      if (Si2Calib) { //recover Esi1 and EcsI from Esi2
//         if (!IsAMeasured()) {

//            if (GetZ() == 1)       SetA(1);
//            else if (GetZ() == 2)  SetA(4);
//            else if (GetZ() == 20) SetA(48);
//            else {
//               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
//            }
//         }
//         Double_t E_targ = 0;
//         fECSI = GetSI2()->GetEResFromDeltaE(GetZ(), GetA(), fESI2);
//         fESI1 = GetSI1()->GetDeltaEFromERes(GetZ(), GetA(), fESI2 + fECSI);
//         SetEnergy(fECSI + fESI2 + fESI1);

//         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
//         Double_t E_tot = GetEnergy() + E_targ;
//         SetECode(1);
//         SetIsCalibrated();
//         SetEnergy(E_tot);
//         GetAnglesFromStoppingDetector();
//      }
//      else if (Si1Calib) { //recover Esi2 and ECSI from ESi1
//         if (!IsAMeasured()) {

//            if (GetZ() == 1)       SetA(1);
//            else if (GetZ() == 2)  SetA(4);
//            else if (GetZ() == 20) SetA(48);
//            else {
//               SetA(1.04735 + 1.99941 * GetZ() + 0.00683224 * TMath::Power(GetZ(), 2.));
//            }
//         }
//         Double_t E_targ = 0;
//         Double_t Eres = GetSI1()->GetEResFromDeltaE(GetZ(), GetA(), fESI1);
//         avatar.SetZAandE(GetZ(), GetA(), Eres);
//         fESI2 = GetSI2()->GetELostByParticle(&avatar);
//         fECSI = Eres - fESI2;
//         SetEnergy(fECSI + fESI2 + fESI1);
//         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
//         Double_t E_tot = GetEnergy() + E_targ;
//         SetECode(6);
//         SetIsCalibrated();
//         SetEnergy(E_tot);
//         GetAnglesFromStoppingDetector();
//      }
//      else {
//         //this will never happen since CSI calibration requires at least Si1 or Si2 to be calibrated

//      }
//   }
//   delete [] eloss;
}





void KVFAZIAReconNuc::ComputePSA()
{
   // Perform Pulse Shape Analysis for all detectors hit by this particle

   KVFAZIADetector* det = 0;

   TIter nextd(GetDetectorList());
   while ((det = (KVFAZIADetector*)nextd())) {
      det->ComputePSA();
   }

}

