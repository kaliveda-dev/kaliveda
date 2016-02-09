//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#include "KVCurrentSignal.h"
#include "TMath.h"
#include "TEnv.h"
#include "KVDataSet.h"

ClassImp(KVCurrentSignal)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVCurrentSignal</h2>
<h4>digitized intensity signal</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
void KVCurrentSignal::init()
{
   SetDefaultValues();
}

KVCurrentSignal::KVCurrentSignal()
{
   init();
}

//________________________________________________________________

KVCurrentSignal::KVCurrentSignal(const char* name) : KVSignal(name, "Current")
{
   SetType(name);
   init();
}

//________________________________________________________________

KVCurrentSignal::~KVCurrentSignal()
{
   // Destructor
}

//________________________________________________________________

void KVCurrentSignal::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVCurrentSignal::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSignal::Copy(obj);
   //KVCurrentSignal& CastedObj = (KVCurrentSignal&)obj;
}

void KVCurrentSignal::SetDefaultValues()
{
   SetChannelWidth(4.);
   SetBaseLineLength(30);
}

void KVCurrentSignal::LoadPSAParameters()
{

//    TString spar;
//    Double_t lval;
//    //BaseLineLength
//    spar.Form("%s.%s.BaseLineLength",dettype,GetName());
//    if (gDataSet) lval = gDataSet->GetDataSetEnv(spar.Data(),0.0);
//    else              lval = gEnv->GetValue(spar.Data(),0.0);
//    SetBaseLineLength(lval);
//    //ChannelWidth
//    spar.Form("%s.%s.ChannelWidth",dettype,GetName());
//    if (gDataSet) lval = gDataSet->GetDataSetEnv(spar.Data(),0.0);
//    else              lval = gEnv->GetValue(spar.Data(),0.0);
//    SetChannelWidth(lval);
}

/*
KVPSAResult *KVCurrentSignal::TreateSignal()
{
    if (GetN()==0) {
      //Info("TreateSignal","Empty signal %s",GetName());
      return 0;
   }
   KVPSAResult* psa = new KVPSAResult(GetName());
   psa->SetValue(Form("%s.%s.RawAmplitude",fDetName.Data(),fType.Data()),GetRawAmplitude());
   if (fAdc.fN==0) SetADCData();

   fBaseLine  = FindMedia(fFirstBL, fLastBL);
   fSigmaBase = FindSigma2(fFirstBL, fLastBL);

   Add(-1.*fBaseLine);
   ComputeAmplitude();

   // storing result
   psa->SetValue(Form("%s.%s.BaseLine", fDetName.Data(), fType.Data()), fBaseLine);
   psa->SetValue(Form("%s.%s.SigmaBaseLine", fDetName.Data(), fType.Data()), TMath::Sqrt(fSigmaBase));
   psa->SetValue(Form("%s.%s.Amplitude", fDetName.Data(), fType.Data()), fAmplitude);

   return psa;

}
*/
