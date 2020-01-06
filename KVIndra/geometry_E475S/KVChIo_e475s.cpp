/*
$Id: KVChIo_e475s.cpp,v 1.8 2008/03/13 17:51:19 ebonnet Exp $
$Revision: 1.8 $
$Date: 2008/03/13 17:51:19 $
*/

//Created by KVClassFactory on Thu Sep 20 09:46:32 2007
//Author: Eric Bonnet

#include "KVChIo_e475s.h"
#include "KVCalibrator.h"
#include "KVFunctionCal.h"
#include "KVDBParameterSet.h"
#include "TClass.h"

ClassImp(KVChIo_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVChIo_e475s</h2>
<h4>derivation of KVChIo class for E475s experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//------------------------------
KVChIo_e475s::KVChIo_e475s()
//------------------------------
{
   //Default constructor
   fSegment = 0;

   //initialise non-persistent pointers
   fcalibPG = 0;
   fcalibGG = 0;

}

//------------------------------
KVChIo_e475s::KVChIo_e475s(Float_t pressure, Float_t thick): KVChIo(pressure, thick)
//------------------------------
{
   fcalibPG = 0;
   fcalibGG = 0;
}

//------------------------------
void KVChIo_e475s::SetCalibrator(KVDBParameterSet* kvdbps)
//------------------------------
{
   KVFunctionCal* cali = new KVFunctionCal(kvdbps);
   if (!AddCalibrator(cali)) {
      Warning("SetCalibrator(KVDBParameterSet*)",
              "Addition of Calibrator %s %s failed !", kvdbps->GetName(), kvdbps->GetTitle());
      delete cali;
   }
   else {
      if (TString(cali->GetType()).Contains("(PG)"))  fcalibPG = cali;
      else                                            fcalibGG = cali;
   }
}
//------------------------------
void KVChIo_e475s::ChangeCalibParameters(KVDBParameterSet* kvdbps)
//------------------------------
{
   if (TString(kvdbps->GetTitle()).Contains("PG")) {
      fcalibPG->ChangeCalibParameters(kvdbps);
   }
   else {
      fcalibGG->ChangeCalibParameters(kvdbps);
   }

}

//------------------------------
Double_t KVChIo_e475s::GetOriginalValue(Float_t to, TString signal)
//------------------------------
{
   if (IsCalibratedBySignal(signal)) return GetCalibratorBySignal(signal)->Invert(to);
   else return 0.;
}

//------------------------------
Double_t KVChIo_e475s::GetCalibratedValue(Float_t from, TString signal) const
//------------------------------
{
   if (IsCalibratedBySignal(signal)) return GetCalibratorBySignal(signal)->Compute(from);
   else return 0.;
}

//------------------------------
KVFunctionCal* KVChIo_e475s::GetCalibratorBySignal(TString signal) const
//------------------------------
{
   if (signal == "PG") return fcalibPG;
   else              return fcalibGG;
}


//------------------------------
Bool_t  KVChIo_e475s::IsCalibrated() const
//------------------------------
{
   if (IsCalibratedBySignal("PG") || IsCalibratedBySignal("GG")) return kTRUE;
   else return kFALSE;

}

//------------------------------
Bool_t  KVChIo_e475s::IsCalibratedBySignal(TString signal) const
//------------------------------
{
   return (GetCalibratorBySignal(signal));

}

//------------------------------
Double_t  KVChIo_e475s::GetCalibratedEnergy() const
//------------------------------
{
   return GetCalibratedValue(TMath::Max(Float_t(0.), GetPG() - GetPedestal("PG")), "PG");
}

//------------------------------
Double_t KVChIo_e475s::GetEnergy() const
//------------------------------
{

   //Redefinition of KVDetector::GetEnergy().
   //If energy lost in active (gas) layer is already set (e.g. by calculation of energy loss
   //of charged particles), return its value.
   //
   //Returns 0 if (i) no calibration present (ii) calibration present but no data in acquisition parameters
   Double_t ELoss = KVDetector::GetEnergy();
   if (ELoss > 0) return KVDetector::GetEnergy();
   ELoss = GetCalibratedEnergy();
   if (ELoss < 0) ELoss = 0;
   SetEnergy(ELoss);
   return ELoss;

}

//______________________________________________________________________________

Short_t KVChIo_e475s::GetCalcACQParam(KVACQParam* ACQ, Double_t ECalc) const
{
   // Calculates & returns value of given acquisition parameter corresponding to
   // given calculated energy loss in the detector
   // Returns -1 if detector is not calibrated

   if (!IsCalibratedBySignal(ACQ->GetType())) return -1;
   Double_t orig = const_cast<KVChIo_e475s*>(this)->GetOriginalValue((Float_t)ECalc, ACQ->GetType())
                   + const_cast<KVChIo_e475s*>(this)->GetPedestal(ACQ->GetType());
   return (Short_t)orig;
}

//______________________________________________________________________________

void KVChIo_e475s::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVChIo_e475s.
   // We set the pointers to the calibrator objects

   if (R__b.IsReading()) {
      KVChIo_e475s::Class()->ReadBuffer(R__b, this);
      fcalibPG  = (KVFunctionCal*)GetCalibrator("Channel->MeV(PG)");
      fcalibGG  = (KVFunctionCal*)GetCalibrator("Channel->MeV(GG)");
   }
   else {
      KVChIo_e475s::Class()->WriteBuffer(R__b, this);
   }
}
