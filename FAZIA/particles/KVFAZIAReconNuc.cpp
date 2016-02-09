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

using namespace std;

ClassImp(KVFAZIAReconNuc);

////////////////////////////////////////////////////////////////////////////
//KVFAZIAReconNuc
//
//Nuclei reconstructed from data measured in the FAZIA array.
//Most useful methods are already defined in parent classes KVReconstructedNucleus,
//KVNucleus and KVParticle.


void KVFAZIAReconNuc::init()
{
   //default initialisations
   if (gDataSet)
      SetMassFormula(UChar_t(gDataSet->GetDataSetEnv("KVFAZIAReconNuc.MassFormula", Double_t(kEALMass))));
   fECsI = fESi1 = fESi2 = 0.;
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

   //KVReconstructedNucleus::Print(option);

   if (IsIdentified()) {

      cout << " =======> ";
      cout << " Z=" << GetZ() << " A=" << ((KVFAZIAReconNuc*) this)->
           GetA();
      if (((KVFAZIAReconNuc*) this)->IsAMeasured()) cout << " Areal=" << ((KVFAZIAReconNuc*) this)->GetRealA();
      else cout << " Zreal=" << GetRealZ();

   } else {
      cout << "(unidentified)" << endl;
   }
   if (IsCalibrated()) {
      cout << " Total Energy = " << GetEnergy() << " MeV,  Theta=" << GetTheta() << " Phi=" << GetPhi() << endl;
      cout << "    Target energy loss correction :  " << GetTargetEnergyLoss() << " MeV" << endl;

   } else {
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
   KVFAZIADetector* det = (KVFAZIADetector*) GetDetectorList()->FindObjectByLabel(label);
   return det;
}

KVFAZIADetector* KVFAZIAReconNuc::GetCSI() const
{
   return Get("CSI");
}
KVFAZIADetector* KVFAZIAReconNuc::GetSI1() const
{
   return Get("SI1");
}
KVFAZIADetector* KVFAZIAReconNuc::GetSI2() const
{
   return Get("SI2");
}


Bool_t KVFAZIAReconNuc::StoppedIn(const Char_t* label) const
{
   //Returns kTRUE if particle stopped in the labelled detector
   return (GetStoppingDetector() == Get(label));
}

Bool_t KVFAZIAReconNuc::StoppedInSI2() const
{
   return StoppedIn("SI2");
}
Bool_t KVFAZIAReconNuc::StoppedInSI1() const
{
   return StoppedIn("SI1");
}
Bool_t KVFAZIAReconNuc::StoppedInCSI() const
{
   return StoppedIn("CSI");
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


   KVList* idt_list = GetStoppingDetector()->GetIDTelescopes();
   KVIdentificationResult* IDR = 0;
   Int_t idnumber = 1;
   if (idt_list && idt_list->GetSize() > 0) {

      KVIDTelescope* idt;
      TIter next(idt_list);
      while ((idt = (KVIDTelescope*) next()) && !IsIdentified()) {
         IDR = GetIdentificationResult(idnumber);
         IDR->SetName(idt->GetName());
         if (idt->IsReadyForID()) { // is telescope able to identify for this run ?

            IDR->IDattempted = kTRUE;
            IDR->IDOK = kFALSE;
            idt->Identify(IDR);
            if (IDR->IDOK) {  //Correspond to Quality code <=3
               SetIdentification(IDR);
               SetIdentifyingTelescope(idt);
               SetIsIdentified();
               //return;
            } else {
               SetIdentification(IDR);
               SetIdentifyingTelescope(idt);
               SetIDCode(0);
               SetZandA(0, 0);
               SetIsIdentified();
            }
         } else {
            IDR->IDattempted = kFALSE;
         }
         idnumber += 1;
      }

   }

}

//_________________________________________________________________________________

void KVFAZIAReconNuc::Calibrate()
{
   KVNucleus avatar;
   //printf("start Calibrate\n");
   Int_t ntot = GetDetectorList()->GetEntries();
   Double_t* eloss = new Double_t[ntot];
   TIter next(GetDetectorList());
   KVFAZIADetector* det = 0;
   Int_t ndet = 0;
   Int_t ndet_calib = 0;
   Double_t etot = 0;
   while ((det = (KVFAZIADetector*)next())) {
      //printf("\t%s\n",det->GetName());
      if (det->IsCalibrated()) {
         eloss[ntot - ndet - 1] = det->GetEnergy();
         //printf("\tdet=%s ndet=%d ntot-ndet-1=%d de=%lf\n",det->GetName(),ndet,ntot-ndet-1,eloss[ntot-ndet-1]);
         etot += eloss[ntot - ndet - 1];
         ndet_calib += 1;
      }
      ndet += 1;
   }
   if (ndet == ndet_calib) {
      Double_t E_targ = 0;
      SetEnergy(etot);

      Double_t etot_avatar = 0;
      Double_t chi2 = 0;
      avatar.SetZAandE(GetZ(), GetA(), GetKE());
      for (Int_t nn = ntot - 1; nn >= 0; nn -= 1) {
         Double_t temp = GetDetector(nn)->GetELostByParticle(&avatar);
         //printf("\t (avatar) det=%s de=%lf(%lf)\n",GetDetector(nn)->GetName(),temp,eloss[ntot-1-nn]);
         etot_avatar += temp;
         chi2 += TMath::Power(eloss[ntot - 1 - nn] - temp, 2.);

         avatar.SetKE(avatar.GetKE() - temp);
      }
      if (avatar.GetKE() > 0) {
         Warning("Calibrate", "Incoherence energie residuelle %lf (PUNCH THROUGH)", avatar.GetKE());
      } else if (TMath::Abs(etot - etot_avatar) > 1e-3) {
         Warning("Calibrate", "Incoherence %lf != %lf", etot, etot_avatar);
      } else {
         SetIsCalibrated();
         chi2 /= ndet;
         //Info("Calibrate","z=%d a=%d edet=%lf chi2=%lf ameasured=%d",GetZ(),GetA(),etot,chi2,Int_t(IsAMeasured()));
      }

      if (GetZ() && GetEnergy() > 0) {
         E_targ = gMultiDetArray->GetTargetEnergyLossCorrection(this);
         SetTargetEnergyLoss(E_targ);
      }
      Double_t E_tot = GetEnergy() + E_targ;
      SetEnergy(E_tot);
      // set particle momentum from telescope dimensions (random)
      GetAnglesFromStoppingDetector();

      //SetIsCalibrated();
   } else {
      //Info("Calibrate","Incomplete calibration");
   }

   delete [] eloss;
}


void KVFAZIAReconNuc::MakeDetectorList()
{
   // Protected method, called when required to fill fDetList with pointers to
   // the detectors whose names are stored in fDetNames.
   // If gMultiDetArray=0x0, fDetList list will be empty.
   KVFAZIADetector* det = 0;
   Double_t val = -1;
   fDetList.Clear();
   if (gMultiDetArray) {
      fDetNames.Begin("/");
      while (!fDetNames.End()) {
         det = (KVFAZIADetector*)gMultiDetArray->GetDetector(fDetNames.Next(kTRUE));
         if (det) {
            fDetList.Add(det);
            if (!strcmp(det->GetLabel(), "SI1")) {
               val = GetParameters()->GetDoubleValue(Form("%s.QL1.Amplitude", det->GetName()));
               det->SetQL1Amplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.QL1.RawAmplitude", det->GetName()));
               det->SetQL1RawAmplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.QL1.BaseLine", det->GetName()));
               det->SetQL1BaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.QL1.SigmaBaseLine", det->GetName()));
               det->SetQL1SigmaBaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.QL1.RiseTime", det->GetName()));
               det->SetQL1RiseTime(val);

               val = GetParameters()->GetDoubleValue(Form("%s.QH1.Amplitude", det->GetName()));
               det->SetQH1Amplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.QH1.RawAmplitude", det->GetName()));
               det->SetQH1RawAmplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.QH1.BaseLine", det->GetName()));
               det->SetQH1BaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.QH1.SigmaBaseLine", det->GetName()));
               det->SetQH1SigmaBaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.QH1.RiseTime", det->GetName()));
               det->SetQH1RiseTime(val);

               val = GetParameters()->GetDoubleValue(Form("%s.I1.Amplitude", det->GetName()));
               det->SetI1Amplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.I1.RawAmplitude", det->GetName()));
               det->SetI1RawAmplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.I1.BaseLine", det->GetName()));
               det->SetI1BaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.I1.SigmaBaseLine", det->GetName()));
               det->SetI1SigmaBaseLine(val);

            } else if (!strcmp(det->GetLabel(), "SI2")) {

               val = GetParameters()->GetDoubleValue(Form("%s.Q2.Amplitude", det->GetName()));
               det->SetQ2Amplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q2.RawAmplitude", det->GetName()));
               det->SetQ2RawAmplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q2.BaseLine", det->GetName()));
               det->SetQ2BaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q2.SigmaBaseLine", det->GetName()));
               det->SetQ2SigmaBaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q2.RiseTime", det->GetName()));
               det->SetQ2RiseTime(val);

               val = GetParameters()->GetDoubleValue(Form("%s.I2.Amplitude", det->GetName()));
               det->SetI2Amplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.I2.RawAmplitude", det->GetName()));
               det->SetI2RawAmplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.I2.BaseLine", det->GetName()));
               det->SetI2BaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.I2.SigmaBaseLine", det->GetName()));
               det->SetI2SigmaBaseLine(val);
            } else if (!strcmp(det->GetLabel(), "CSI")) {
               val = GetParameters()->GetDoubleValue(Form("%s.Q3.Amplitude", det->GetName()));
               det->SetQ3Amplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q3.RawAmplitude", det->GetName()));
               det->SetQ3RawAmplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q3.FastAmplitude", det->GetName()));
               det->SetQ3FastAmplitude(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q3.BaseLine", det->GetName()));
               det->SetQ3BaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q3.SigmaBaseLine", det->GetName()));
               det->SetQ3SigmaBaseLine(val);
               val = GetParameters()->GetDoubleValue(Form("%s.Q3.RiseTime", det->GetName()));
               det->SetQ3RiseTime(val);

            }

         }
      }
   }
}
