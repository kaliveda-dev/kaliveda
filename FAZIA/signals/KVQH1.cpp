//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#include "KVQH1.h"

#include <KVDetector.h>

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
   SetType("QH1");
   fFPGAOutputNumbers = 1;
   LoadPSAParameters();
}

KVQH1::KVQH1()
{
   init();
}

//________________________________________________________________

KVQH1::KVQH1(const char* name) : KVSignal(name, "Charge")
{
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

void KVQH1::GetPSAResult(KVDetector* d) const
{
   // store results of PSA in detector signals

   if (!fPSAIsDone) return;

   d->SetDetectorSignalValue(Form("%s.BaseLine", fType.Data()), fBaseLine);
   d->SetDetectorSignalValue(Form("%s.SigmaBaseLine", fType.Data()), fSigmaBase);
   d->SetDetectorSignalValue(Form("%s.Amplitude", fType.Data()), fAmplitude);
   d->SetDetectorSignalValue(Form("%s.RiseTime", fType.Data()), fRiseTime);
   d->SetDetectorSignalValue(Form("%s.RawAmplitude", fType.Data()), GetRawAmplitude());
}
