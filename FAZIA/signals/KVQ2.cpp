//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#include "KVQ2.h"
#include "KVPSAResult.h"

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

}

KVQ2::KVQ2()
{
   init();
}

//________________________________________________________________

KVQ2::KVQ2(const char* name) : KVSignal(name, "Charge")
{
   SetType(name);
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
   if (GetN() == 0) return;
   if (!TestWidth())
      ChangeChannelWidth(GetChannelWidth());

   ComputeBaseLine();
   fBaseLine  = GetBaseLine();
   fSigmaBase = GetSigmaBaseLine();
   Add(-1.*fBaseLine);
   ApplyModifications();
   if (fWithPoleZeroCorrection)
      PoleZeroSuppression(fTauRC);
   FIR_ApplyTrapezoidal(fTrapRiseTime, fTrapFlatTop);
   ComputeAmplitude();
   fAmplitude = GetAmplitude();
   SetADCData();
   ComputeRiseTime();
   fRiseTime = GetRiseTime();

   fPSAIsDone = kTRUE;
}

KVPSAResult* KVQ2::GetPSAResult() const
{
   if (!fPSAIsDone) return 0;

   KVPSAResult* psa = new KVPSAResult();
   psa->SetValue(Form("%s.%s.BaseLine", fDetName.Data(), fType.Data()), fBaseLine);
   psa->SetValue(Form("%s.%s.SigmaBaseLine", fDetName.Data(), fType.Data()), fSigmaBase);
   psa->SetValue(Form("%s.%s.Amplitude", fDetName.Data(), fType.Data()), fAmplitude);
   psa->SetValue(Form("%s.%s.RiseTime", fDetName.Data(), fType.Data()), fRiseTime);
   psa->SetValue(Form("%s.%s.RawAmplitude", fDetName.Data(), fType.Data()), GetRawAmplitude());
   return psa;

}

