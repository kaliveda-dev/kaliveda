//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#include "KVQ2.h"

#include <KVDetector.h>

ClassImp(KVQ2)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVQ2</h2>
<h4>digitized charge signal</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
void KVQ2::init()
{
   SetDefaultValues();
   fChannel = kQ2;
   fFPGAOutputNumbers = 1;
   SetType("Q2");
   LoadPSAParameters();

}

KVQ2::KVQ2()
{
   init();
}

//________________________________________________________________

KVQ2::KVQ2(const char* name) : KVSignal(name, "Charge")
{
   init();
}

//________________________________________________________________

KVQ2::~KVQ2()
{
   // Destructor
}

//________________________________________________________________

void KVQ2::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVQ2::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSignal::Copy(obj);
   //KVQ2& CastedObj = (KVQ2&)obj;
}

void KVQ2::SetDefaultValues()
{
   SetChannelWidth(10);
   SetBaseLineLength(500);
}

void KVQ2::LoadPSAParameters()
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

   val = GetPSAParameter("MinimumAmplitude");
   SetAmplitudeTriggerValue(val);
}


//________________________________________________________________
void KVQ2::TreateSignal()
{
   if (!IsLongEnough()) return;
   if (!TestWidth())
      ChangeChannelWidth(GetChannelWidth());

   RemoveBaseLine();

   if (fWithPoleZeroCorrection)
      PoleZeroSuppression(fTauRC);
   FIR_ApplyTrapezoidal(fTrapRiseTime, fTrapFlatTop);

   ComputeAmplitude();

   SetADCData();
   ComputeRiseTime();

   fPSAIsDone = kTRUE;
}

void KVQ2::GetPSAResult(KVDetector* d) const
{
   if (!fPSAIsDone) return;

   d->SetDetectorSignalValue(Form("%s.BaseLine", fType.Data()), fBaseLine);
   d->SetDetectorSignalValue(Form("%s.SigmaBaseLine", fType.Data()), fSigmaBase);
   d->SetDetectorSignalValue(Form("%s.Amplitude", fType.Data()), fAmplitude);
   d->SetDetectorSignalValue(Form("%s.RiseTime", fType.Data()), fRiseTime);
   d->SetDetectorSignalValue(Form("%s.RawAmplitude", fType.Data()), GetRawAmplitude());
}

