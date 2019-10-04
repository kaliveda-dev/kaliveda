//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#include "KVI1.h"
#include "TMath.h"

#include <KVDetector.h>

ClassImp(KVI1)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVI1</h2>
<h4>digitized current signal</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
void KVI1::init()
{
   SetDefaultValues();
   fChannel = kI1;
   SetType("I1");
   LoadPSAParameters();

}

KVI1::KVI1()
{
   init();
}

//________________________________________________________________

KVI1::KVI1(const char* name) : KVSignal(name, "Current")
{
   init();
}

//________________________________________________________________

KVI1::~KVI1()
{
   // Destructor
}

//________________________________________________________________

void KVI1::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVI1::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSignal::Copy(obj);
   //KVI1& CastedObj = (KVI1&)obj;
}

void KVI1::SetDefaultValues()
{
   SetChannelWidth(4);
   SetBaseLineLength(30);
}

void KVI1::LoadPSAParameters()
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

Double_t KVI1::ComputeBaseLine()
{
   // special case for current signal
   // in case the pulse start to early,
   // base line is calculated at the end of the signal

   KVSignal::ComputeBaseLine();
   KVSignal::ComputeEndLine();

   if (fBaseLine <= fEndLine) {
      //do nothing baseline is kept as calculated by default KVSignal method
   }
   else {
      //compute the base line at the end of the signal
      //baseline length is taken as for the standard base line calculation
      //
      ComputeEndLine();
      fBaseLine = fEndLine;
      fSigmaBase = fSigmaEnd;
   }

   return fBaseLine;
}


void KVI1::TreateSignal()
{
   if (!IsLongEnough()) return;

   if (!TestWidth())
      ChangeChannelWidth(GetChannelWidth());

//   FIR_ApplyMovingAverage(4);
   Add(-1.*ComputeBaseLine());
//   ApplyModifications();

   if (fWithInterpolation) {
      BuildSmoothingSplineSignal();
      SetNSamples(GetNSamples() - 5 * (fChannelWidth / fInterpolatedChannelWidth)); // because we use a 3th order interpolation...
   }

   fAmplitude = ComputeAmplitude();
   fPSAIsDone = kTRUE;

}


void KVI1::GetPSAResult(KVDetector* d) const
{
   if (!fPSAIsDone) return;

   d->SetDetectorSignalValue(Form("%s.BaseLine", fType.Data()), fBaseLine);
   d->SetDetectorSignalValue(Form("%s.SigmaBaseLine", fType.Data()), fSigmaBase);
   d->SetDetectorSignalValue(Form("%s.Amplitude", fType.Data()), fAmplitude);
   d->SetDetectorSignalValue(Form("%s.RawAmplitude", fType.Data()), GetRawAmplitude());
}
