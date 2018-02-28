/*
$Id: KVSilicon_e475s.cpp,v 1.9 2009/04/09 09:25:14 ebonnet Exp $
$Revision: 1.9 $
$Date: 2009/04/09 09:25:14 $
*/

//Created by KVClassFactory on Wed Sep 19 13:46:35 2007
//Author: Eric Bonnet

#include "KVSilicon_e475s.h"
#include "KVCalibrator.h"
#include "KVFunctionCal.h"
#include "KVDBParameterSet.h"
#include "TClass.h"

ClassImp(KVSilicon_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSilicon_e475s</h2>
<h4>derivation of KVSilicon class for E475s experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//------------------------------
void KVSilicon_e475s::init()
//------------------------------
{
   fSegment = 1;
   //initialise non-persistent pointers
   fcalibPG = 0;
   fcalibGG = 0;

}

//------------------------------
KVSilicon_e475s::KVSilicon_e475s()
//------------------------------
{
   //Default constructor
   //This ctor (which in turn calls the KVSilicon default ctor) must exist in
   //order for Cloning of detectors to work (as used in KVTelescope::AddDetector).
   //Do not replace this ctor by giving default arguments to KVSilicon_e475s(Float_t).
   init();
}

//------------------------------
KVSilicon_e475s::KVSilicon_e475s(Float_t thick): KVSilicon(thick)
//------------------------------
{
   //Default constructor
   SetType("SI");
   init();
}

//------------------------------
void KVSilicon_e475s::SetCalibrator(KVDBParameterSet* kvdbps)
//------------------------------
{
   KVFunctionCal* cali = new KVFunctionCal(kvdbps);
   if (!AddCalibrator(cali)) {
      Warning("SetCalibrator(KVDBParameterSet*)",
              "Addition of Calibrator %s %s failed !", kvdbps->GetName(), kvdbps->GetTitle());
      delete cali;
   } else {
      if (TString(cali->GetType()).Contains("(PG)"))  fcalibPG = cali;
      else                                            fcalibGG = cali;
   }
}

//------------------------------
void KVSilicon_e475s::ChangeCalibParameters(KVDBParameterSet* kvdbps)
//------------------------------
{
   if (TString(kvdbps->GetTitle()).Contains("PG")) {
      fcalibPG->ChangeCalibParameters(kvdbps);
   } else {
      fcalibGG->ChangeCalibParameters(kvdbps);
   }

}

//------------------------------
Double_t KVSilicon_e475s::GetOriginalValue(Float_t to, TString signal)
//------------------------------
{
   if (IsCalibratedBySignal(signal)) return GetCalibratorBySignal(signal)->Invert(to);
   else return 0.;
}

//------------------------------
Double_t KVSilicon_e475s::GetCalibratedValue(Float_t from, TString signal)
//------------------------------
{
   if (IsCalibratedBySignal(signal)) return GetCalibratorBySignal(signal)->Compute(from);
   else return 0.;
}

//------------------------------
KVFunctionCal* KVSilicon_e475s::GetCalibratorBySignal(TString signal) const
//------------------------------
{
   if (signal == "PG") return fcalibPG;
   else              return fcalibGG;
}

//------------------------------
Bool_t  KVSilicon_e475s::IsCalibrated() const
//------------------------------
{
   return IsCalibratedBySignal("PG") || IsCalibratedBySignal("GG");
}

//------------------------------
Bool_t  KVSilicon_e475s::IsCalibratedBySignal(TString signal) const
//------------------------------
{
   return (GetCalibratorBySignal(signal));

}

//------------------------------
Double_t  KVSilicon_e475s::GetCalibratedEnergy()
//------------------------------
{
   return GetCalibratedValue(TMath::Max(Float_t(0.), GetPG() - GetPedestal("PG")), "PG");
}

//------------------------------
Double_t KVSilicon_e475s::GetCorrectedEnergy(KVNucleus*, Double_t, Bool_t)
//------------------------------
{
   //Do nothing more
   return GetEnergy();

}

//------------------------------
Double_t KVSilicon_e475s::GetEnergy()
//------------------------------
{
   Double_t ELoss = KVDetector::GetEnergy();
   if (ELoss > 0) return KVDetector::GetEnergy();
   ELoss = GetCalibratedEnergy();
   if (ELoss < 0) ELoss = 0;
   SetEnergy(ELoss);
   return ELoss;
}

//______________________________________________________________________________

Short_t KVSilicon_e475s::GetCalcACQParam(KVACQParam* ACQ, Double_t ECalc) const
{
   // Calculates & returns value of given acquisition parameter corresponding to
   // given calculated energy loss in the detector
   // Returns -1 if detector is not calibrated

   if (!IsCalibratedBySignal(ACQ->GetType())) return -1;
   Double_t orig = const_cast<KVSilicon_e475s*>(this)->GetOriginalValue((Float_t)ECalc, ACQ->GetType())
                   + const_cast<KVSilicon_e475s*>(this)->GetPedestal(ACQ->GetType());
   return (Short_t)orig;
}

//______________________________________________________________________________

void KVSilicon_e475s::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVSilicon_e475s.
   // We set the pointers to the calibrator objects

   if (R__b.IsReading()) {
      KVSilicon_e475s::Class()->ReadBuffer(R__b, this);
      fcalibPG  = (KVFunctionCal*)GetCalibrator("Channel->MeV(PG)");
      fcalibGG  = (KVFunctionCal*)GetCalibrator("Channel->MeV(GG)");
   } else {
      KVSilicon_e475s::Class()->WriteBuffer(R__b, this);
   }
}
