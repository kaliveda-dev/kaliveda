/*
$Id: KVCsI_e475s.cpp,v 1.2 2009/04/15 09:49:19 ebonnet Exp $
$Revision: 1.2 $
$Date: 2009/04/15 09:49:19 $
*/

//Created by KVClassFactory on Thu Apr  9 09:22:46 2009
//Author: eric bonnet,,,

#include "KVCsI_e475s.h"
#include "KVCalibrator.h"
#include "TClass.h"

ClassImp(KVCsI_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVCsI_e475s</h2>
<h4>derivation of KVCsI class for E475s experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________

void KVCsI_e475s::init()
{

   //initialise non-persistent pointers
   fcalibLT = 0;

}

//______________________________________________________________________________

KVCsI_e475s::KVCsI_e475s()
{
   //Default ctor
   //This ctor (which in turn calls the KVDetector default ctor) must exist in
   //order for Cloning of detectors to work (as used in KVTelescope::AddDetector).
   //Do not replace this ctor by giving a default value for the argument of KVCsI(Float_t).
   //
   init();
}

//______________________________________________________________________________
KVCsI_e475s::KVCsI_e475s(Float_t thick): KVCsI(thick)
{
   //Make a CsI detector "thick" cm long
   //Set type of detector to "CSI"
   //By default 'thick'=0

   SetType("CSI");
   init();

}

//------------------------------
void KVCsI_e475s::SetCalibrator(KVDBParameterSet* kvdbps)
//------------------------------
{
   KVFunctionCal* cali = new KVFunctionCal(kvdbps);
   if (!AddCalibrator(cali)) {
      Warning("SetCalibrator(KVDBParameterSet*)",
              "Addition of Calibrator %s %s failed !", kvdbps->GetName(), kvdbps->GetTitle());
      delete cali;
   } else {
      if (TString(cali->GetType()).Contains("(LT)"))  fcalibLT = cali;
      else fcalibLT = 0;
   }
}

//------------------------------
void KVCsI_e475s::ChangeCalibParameters(KVDBParameterSet* kvdbps)
//------------------------------
{
   if (TString(kvdbps->GetTitle()).Contains("LT")) {
      fcalibLT->ChangeCalibParameters(kvdbps);
   } else {
      printf("KVCsI_e475s::ChangeCalibParameters(KVDBParameterSet *kvdbps) : no calibrator defined for %s\n", kvdbps->GetTitle());
   }

}

//------------------------------
Double_t KVCsI_e475s::GetOriginalValue(Float_t to, TString signal)
//------------------------------
{
   if (IsCalibratedBySignal(signal)) return GetCalibratorBySignal(signal)->Invert(to);
   else return 0.;
}

//------------------------------
Double_t KVCsI_e475s::GetCalibratedValue(Float_t from, TString signal)
//------------------------------
{
   if (IsCalibratedBySignal(signal)) return GetCalibratorBySignal(signal)->Compute(from);
   else return 0.;
}

//------------------------------
KVFunctionCal* KVCsI_e475s::GetCalibratorBySignal(TString signal) const
//------------------------------
{
   if (signal == "LT") return fcalibLT;
   else              return 0;
}

//------------------------------
Bool_t  KVCsI_e475s::IsCalibrated() const
//------------------------------
{
   return IsCalibratedBySignal("LT");
}

//------------------------------
Bool_t  KVCsI_e475s::IsCalibratedBySignal(TString signal) const
//------------------------------
{
   return (GetCalibratorBySignal(signal));

}

//------------------------------
Double_t  KVCsI_e475s::GetCalibratedEnergy()
//------------------------------
{
   return GetCalibratedValue(TMath::Max(0., GetLumiereTotale()), "LT");
}

//------------------------------
Double_t KVCsI_e475s::GetCorrectedEnergy(KVNucleus*, Double_t, Bool_t)
//------------------------------
{
   //Do nothing more
   return GetEnergy();

}

//------------------------------
Double_t KVCsI_e475s::GetEnergy()
//------------------------------
{
   Double_t ELoss = KVDetector::GetEnergy();
   if (ELoss > 0)    return KVDetector::GetEnergy();
   else              ELoss = 0;
   SetEnergy(ELoss);
   return ELoss;
}

//______________________________________________________________________________

Short_t KVCsI_e475s::GetCalcACQParam(KVACQParam*, Double_t) const
{
   // Calculates & returns value of given acquisition parameter corresponding to the
   // current value of fEcalc, i.e. the calculated residual energy loss in the detector
   // after subtraction of calculated energy losses corresponding to each identified
   // particle crossing this detector.
   // Returns -1 if fEcalc = 0 or if detector is not calibrated

   return -1;
   /*
   if(!IsCalibratedBySignal(ACQ->GetType()) || GetECalc()==0) return -1;
   Double_t orig = const_cast<KVCsI_e475s*>(this)->GetOriginalValue((Float_t)GetECalc(), ACQ->GetType())
         + const_cast<KVCsI_e475s*>(this)->GetPedestal(ACQ->GetType());
   return (Short_t)orig;
   */
}

//______________________________________________________________________________

void KVCsI_e475s::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVCsI.
   // We set the pointers to the calibrator objects

   if (R__b.IsReading()) {
      KVCsI_e475s::Class()->ReadBuffer(R__b, this);
      fcalibLT  = (KVFunctionCal*)GetCalibrator("Channel->MeV(LT)");
      //printf("")
   } else {
      KVCsI_e475s::Class()->WriteBuffer(R__b, this);
   }
}

