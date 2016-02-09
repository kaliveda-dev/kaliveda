//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#include "KVQH1.h"
#include "KVPSAResult.h"

ClassImp(KVQH1)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVQH1</h2>
<h4>digitized charge signal</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
void KVQH1::init()
{
   SetDefaultValues();
   fChannel = kQH1;
}

KVQH1::KVQH1()
{
   init();
}

//________________________________________________________________

KVQH1::KVQH1(const char* name) : KVSignal(name, "Charge")
{
   SetType(name);
   init();
}

//________________________________________________________________

KVQH1::~KVQH1()
{
   // Destructor
}

//________________________________________________________________

void KVQH1::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVQH1::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSignal::Copy(obj);
   //KVQH1& CastedObj = (KVQH1&)obj;
}

void KVQH1::SetDefaultValues()
{
   SetChannelWidth(10);
   SetBaseLineLength(500);
}

void KVQH1::LoadPSAParameters()
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


void KVQH1::TreateSignal()
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

KVPSAResult* KVQH1::GetPSAResult() const
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
