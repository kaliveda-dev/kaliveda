//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#include "KVI2.h"
#include "KVPSAResult.h"
#include "TMath.h"

ClassImp(KVI2)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVI2</h2>
<h4>digitized intensity signal</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
void KVI2::init()
{
   SetDefaultValues();
   fChannel = kI2;
   SetType("I2");
   LoadPSAParameters();
}

KVI2::KVI2()
{
   init();
}

//________________________________________________________________

KVI2::KVI2(const char* name) : KVSignal(name, "Current")
{
   init();
}

//________________________________________________________________

KVI2::~KVI2()
{
   // Destructor
}

//________________________________________________________________

void KVI2::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVI2::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSignal::Copy(obj);
   //KVI2& CastedObj = (KVI2&)obj;
}

void KVI2::SetDefaultValues()
{
   SetChannelWidth(4);
   SetBaseLineLength(30);
}

void KVI2::LoadPSAParameters()
{
   Double_t val = GetPSAParameter("BaseLineLength");
   SetBaseLineLength(val);

   val = GetPSAParameter("ChannelWidth");
   SetChannelWidth(val);

   val = GetPSAParameter("InterpolatedChannelWidth");
   SetInterpolatedChannelWidth(val);

   val = GetPSAParameter("Interpolation");
   SetInterpolation((val == 1));

}
Double_t KVI2::ComputeBaseLine()
{
   // special case for current signal
   // in case the pulse start to early,
   // base line is calculated at the end of the signal
   KVSignal::ComputeBaseLine();

   if (fBaseLine <= fEndLine) {
      //do nothing baseline is kept as calculated by default KVSignal method
   } else {
      //compute the base line at the end of the signal
      //baseline length is taken as for the standard base line calculation
      //
      ComputeEndLine();
      fBaseLine = fEndLine;
      fSigmaBase = fSigmaEnd;
   }

   return fBaseLine;

}


void KVI2::TreateSignal()
{
   if (GetN() == 0) return;
   if (!TestWidth())
      ChangeChannelWidth(GetChannelWidth());

//   FIR_ApplyMovingAverage(4);

   Add(-1.*ComputeBaseLine());
   if (fWithInterpolation) {
      BuildSmoothingSplineSignal();
      SetNSamples(GetNSamples() - 5 * (fChannelWidth / fInterpolatedChannelWidth)); // because we use a 3th order interpolation...
   }
   fAmplitude = ComputeAmplitude();
   fPSAIsDone = kTRUE;
}

KVPSAResult* KVI2::GetPSAResult() const
{
   if (!fPSAIsDone) return 0;

   KVPSAResult* psa = new KVPSAResult();
   psa->SetValue(Form("%s.%s.BaseLine", fDetName.Data(), fType.Data()), fBaseLine);
   psa->SetValue(Form("%s.%s.SigmaBaseLine", fDetName.Data(), fType.Data()), fSigmaBase);
   psa->SetValue(Form("%s.%s.Amplitude", fDetName.Data(), fType.Data()), fAmplitude);
   psa->SetValue(Form("%s.%s.RawAmplitude", fDetName.Data(), fType.Data()), GetRawAmplitude());
   return psa;

}
