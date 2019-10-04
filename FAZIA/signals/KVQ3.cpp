//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#include "KVQ3.h"
#include "KVDBParameterList.h"

#include <KVDetector.h>

ClassImp(KVQ3)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVQ3</h2>
<h4>digitized charge signal</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
void KVQ3::init()
{
   SetDefaultValues();
   fChannel = kQ3;
   SetType("Q3");
   fFPGAOutputNumbers = 2;
   LoadPSAParameters();

}

KVQ3::KVQ3()
{
   init();
}

//________________________________________________________________

KVQ3::KVQ3(const char* name) : KVSignal(name, "Charge")
{
   init();
}

//________________________________________________________________

KVQ3::~KVQ3()
{
   // Destructor
}

//________________________________________________________________

void KVQ3::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVQ3::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSignal::Copy(obj);
   //KVQ3& CastedObj = (KVQ3&)obj;
}

void KVQ3::SetDefaultValues()
{
   SetChannelWidth(10);
   SetBaseLineLength(250);
}

void KVQ3::LoadPSAParameters()
{

   Double_t val = GetPSAParameter("BaseLineLength");
   SetBaseLineLength(val);

   val = GetPSAParameter("ChannelWidth");
   SetChannelWidth(val);

   val = GetPSAParameter("TauRC");
   SetTauRC(val);

   Double_t rise = GetPSAParameter("ShaperRiseTime");
   val = GetPSAParameter("ShaperFlatTop");
   SetTrapShaperParameters(rise, val);

   val = GetPSAParameter("PZCorrection");
   SetPoleZeroCorrection((val == 1));

   rise = GetPSAParameter("FastShaperRiseTime");
   val = GetPSAParameter("FastShaperFlatTop");
   SetFastTrapShaperParameters(rise, val);

   val = GetPSAParameter("MinimumAmplitude");
   SetAmplitudeTriggerValue(val);

}

//________________________________________________________________
void KVQ3::UpdatePSAParameter(KVDBParameterList* par)
{
   for (Int_t ii = 0; ii < par->GetParameters()->GetNpar(); ii += 1) {
      TString nameat(par->GetParameters()->GetNameAt(ii));
      if (nameat == "BaseLineLength") SetBaseLineLength(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "ChannelWidth")    SetChannelWidth(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "ShaperRiseTime") SetShaperRiseTime(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "ShaperFlatTop")   SetShaperFlatTop(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "FastShaperRiseTime") SetFastShaperRiseTime(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "FastShaperFlatTop")  SetFastShaperFlatTop(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "TauRC")        SetTauRC(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "MinimumAmplitude")   SetAmplitudeTriggerValue(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "InterpolatedChannelWidth")    SetInterpolatedChannelWidth(par->GetParameters()->GetDoubleValue(ii));
      else if (nameat == "Interpolation")   SetInterpolation((par->GetParameters()->GetDoubleValue(ii) == 1));
      else if (nameat == "PZCorrection")    SetPoleZeroCorrection((par->GetParameters()->GetDoubleValue(ii) == 1));
      else {
         if (nameat == "Detector" || nameat == "Signal" || nameat == "RunRange") {

         }
         else {
            Warning("UpdatePSAParameter", "Not supported PSA parameter : %d %s\n", ii, nameat.Data());
         }
      }
   }
}


void KVQ3::TreateSignal()
{
   if (!IsLongEnough()) return;
   if (!TestWidth())
      ChangeChannelWidth(GetChannelWidth());

   RemoveBaseLine();

   FIR_ApplyTrapezoidal(fFastTrapRiseTime, fFastTrapFlatTop);
   ComputeAmplitude();
   fFastAmplitude = GetAmplitude();

   SetADCData();

   FIR_ApplyTrapezoidal(fTrapRiseTime, fTrapFlatTop);
   ComputeAmplitude();
   fAmplitude = GetAmplitude();// - (sQ3fast[fMtot-1])*0.5;

   ComputeRiseTime();

   fPSAIsDone = kTRUE;

}


void KVQ3::GetPSAResult(KVDetector* d) const
{
   if (!fPSAIsDone) return;

//   d->SetDetectorSignalValue(Form("%s.TrapRiseTime", fType.Data()), fTrapRiseTime);
//   d->SetDetectorSignalValue(Form("%s.TrapFlatTop", fType.Data()), fTrapFlatTop);
//   d->SetDetectorSignalValue(Form("%s.FastTrapRiseTime", fType.Data()), fFastTrapRiseTime);
//   d->SetDetectorSignalValue(Form("%s.FastTrapFlatTop", fType.Data()), fFastTrapFlatTop);
   d->SetDetectorSignalValue(Form("%s.BaseLine", fType.Data()), fBaseLine);
   d->SetDetectorSignalValue(Form("%s.SigmaBaseLine", fType.Data()), fSigmaBase);
   d->SetDetectorSignalValue(Form("%s.Amplitude", fType.Data()), fAmplitude);
   d->SetDetectorSignalValue(Form("%s.FastAmplitude", fType.Data()), fFastAmplitude);
   d->SetDetectorSignalValue(Form("%s.RiseTime", fType.Data()), fRiseTime);
   d->SetDetectorSignalValue(Form("%s.RawAmplitude", fType.Data()), GetRawAmplitude());
}
