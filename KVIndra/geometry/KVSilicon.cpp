/***************************************************************************
$Id: KVSilicon.cpp,v 1.55 2009/04/15 09:49:19 ebonnet Exp $
                          kvsilicon.cpp  -  description
                             -------------------
    begin                : Thu May 16 2002
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
#include "KVSilicon.h"
#include "TClass.h"

ClassImp(KVSilicon)


void KVSilicon::init()
{
   fSegment = 1;
}

//______________________________________________________

KVSilicon::KVSilicon(): KVINDRADetector("Si")
{
   //Default ctor
   //This ctor (which in turn calls the KVDetector default ctor) must exist in
   //order for Cloning of detectors to work (as used in KVTelescope::AddDetector).
   //Do not replace this ctor by giving default arguments to KVSilicon(Float_t).
   //
   init();
}

//______________________________________________________________________________
KVSilicon::KVSilicon(Float_t thick): KVINDRADetector("Si", thick * KVUnits::um)
{
   // constructor for silicon detector, thickness in microns
   // Type of detector: "SI"
   SetType("SI");
   SetLabel("SI");//for use with KVReconNucTrajectory
   init();
}

//____________________________________________________________________________________________

KVSilicon::~KVSilicon()
{
}

//void KVSilicon::SetACQParams()
//{
//   //Setup acquistion parameters for this Silicon.
//   //Do not call before ChIo name has been set.

//   AddACQParamType("GG");
//   AddACQParamType("PG");
//   AddACQParamType("T");

//}

//__________________________________________________________________________________________

Double_t KVSilicon::GetPHD(Double_t, UInt_t)
{
   //Calculate Pulse Height Defect in MeV for a given energy loss dE(MeV) and Z.
   //The formula of Moulton is used (see class KVPulseHeightDefect).
   //
   //Returns 0 if PHD is not defined.

   AbstractMethod("GetPHD");
   return 0;
}


void KVSilicon::SetMoultonPHDParameters(Double_t, Double_t, Double_t, Double_t)
{
   //Sets parameters of Moulton formula used to calculate PHD for particles
   //stopping in this detector. The parameters are as in the following:
   //
   // log_10(PHD) = b(Z) + a(Z)*log_10(E)
   //
   //  with  a(Z) = a_1*(Z**2/1000) + a_2
   //          b(Z) = b_1*(100/Z) + b_2
   //            E = energy lost by particle
   //
   //See class KVPulseHeightDefect

   AbstractMethod("SetMoultonPHDParameters");
}

//______________________________________________________________________________

//Short_t KVSilicon::GetCalcACQParam(KVACQParam*, Double_t) const
//{
//   // Calculates & returns value of given acquisition parameter corresponding to
//   // given calculated energy loss in the detector
//   // Returns -1 if detector is not calibrated

//   AbstractMethod("GetCalcACQParam");
//   return -1;
//}

//______________________________________________________________________________

TF1* KVSilicon::GetELossFunction(Int_t Z, Int_t A)
{
   // Overrides KVDetector::GetELossFunction
   // If the pulse height deficit (PHD) has been set for this detector,
   // we return an energy loss function which takes into account the PHD,
   // i.e. for an incident energy E we calculate
   //
   //      dEphd(E,Z,A) = dE(E,Z,A) - PHD(dE,Z)
   //
   // If no PHD is set, we return the usual KVDetector::GetELossFunction
   // which calculates dE(E,Z,A)

   //if (fPHD && fPHD->GetStatus()) return fPHD->GetELossFunction(Z, A);

   return KVDetector::GetELossFunction(Z, A);
}

void KVSilicon::DeduceACQParameters(KVEvent*, KVNumberList&)
{

   AbstractMethod("DeduceACQParameters");
//   Double_t volts = GetVoltsFromEnergy(GetEnergy());
//   Int_t cipg = (Int_t)GetCanalPGFromVolts(volts);
//   Int_t cigg = (Int_t)GetCanalGGFromVolts(volts);
//   //cout << "chio: pg = " << cipg << " gg = " << cigg << endl;
//   GetACQParam("PG")->SetData((UShort_t)TMath::Min(4095, cipg));
//   GetACQParam("GG")->SetData((UShort_t)TMath::Min(4095, cigg));
//   GetACQParam("T")->SetData(110);
}

Double_t KVSilicon::GetDeltaE(Int_t Z, Int_t A, Double_t Einc)
{
   // Overrides KVDetector::GetDeltaE
   // If no PHD is set, we use the optimized KVMaterial::GetDeltaE

   //if (fPHD && fPHD->GetStatus()) return fPHD->GetELossFunction(Z, A)->Eval(Einc);

   return KVDetector::GetDeltaE(Z, A, Einc);
}


//__________________________________________________________________________________________

ClassImp(KVSi75)
//_________________________________________
//Etalon telescope detector Si75
//(actually an 80 micron thick silicon detector)
//Type of detector: "SI75"
//Array naming convention: "SI75_RR" with RR=ring number

KVSi75::KVSi75(): KVSilicon()
{
   //Default ctor
   //For cloning
}

KVSi75::KVSi75(Float_t thick): KVSilicon(thick)
{
   //Default ctor
   //80 micron silicon detector with type "SI75"
   SetType("SI75");
   SetLabel("SI75");
}

KVSi75::~KVSi75()
{
   //default dtor
}

const Char_t* KVSi75::GetArrayName()
{
   //Redefinition of KVDetector method.
   //Name given as SI75_xx with xx=Ring number

   fFName = Form("SI75_%02d", GetRingNumber());
   return fFName.Data();
}

//__________________________________________________________________________________________

ClassImp(KVSiLi)
//_________________________________________
//Etalon telescope detector Si(Li)
//Nominal thickness: 2mm active layer + 40um dead layer (both silicon)
//Type of detector: "SILI"
//Array naming convention: "SILI_RR" with RR=ring number

KVSiLi::KVSiLi(): KVSilicon()
{
   //Default ctor
   //For cloning
}

KVSiLi::KVSiLi(Float_t thick): KVSilicon(thick)
{
   //Default ctor
   // first layer (active) : 2mm silicon (nominal)
   // second layer (dead) : 40um silicon (nominal)
   AddAbsorber(new KVMaterial("Si", 40.0 * KVUnits::um));
   SetType("SILI");
   SetLabel("SILI");
}

KVSiLi::~KVSiLi()
{
   //default dtor
}

const Char_t* KVSiLi::GetArrayName()
{
   //Redefinition of KVDetector method.
   //Name given as SILI_xx with xx=Ring number

   fFName = Form("SILI_%02d", GetRingNumber());
   return fFName.Data();
}
