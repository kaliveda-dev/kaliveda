//Created by KVClassFactory on Tue Jan 13 15:10:39 2015
//Author: ,,,

#include "KVChargeSignal.h"
#include "TMath.h"
#include "TH1F.h"
#include "TEnv.h"
#include "KVDataSet.h"

ClassImp(KVChargeSignal)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVChargeSignal</h2>
<h4>digitized charge signal</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
void KVChargeSignal::init()
{

   fFunc1 = fFunc2 = 0;
   bidim = 0;
   SetDefaultValues();

}

KVChargeSignal::KVChargeSignal()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVChargeSignal::KVChargeSignal(const char* name) : KVSignal(name, "Charge")
{
   // Write your code here
   SetType(name);
   init();
}

void KVChargeSignal::SetDefaultValues()
{

   if (fType.Contains("QL")) {
      SetChannelWidth(4.);
      SetTauRC(250.);
   }
   else                    {
      SetChannelWidth(10.);
      SetTauRC(40.);
   }
   SetBaseLineLength(100);

   SetTrapShaperParameters(2., 1.);
   SetPoleZeroCorrection();
}

void KVChargeSignal::LoadPSAParameters()
{

//    TString spar;
//    Double_t lval;
//    //--- BaseLineLength
//    spar.Form("%s.%s.BaseLineLength",dettype,GetName());
//    if (gDataSet) lval = gDataSet->GetDataSetEnv(spar.Data(),0.0);
//    else              lval = gEnv->GetValue(spar.Data(),0.0);
//    SetBaseLineLength(lval);
//    //--- ChannelWidth
//    spar.Form("%s.%s.ChannelWidth",dettype,GetName());
//    if (gDataSet) lval = gDataSet->GetDataSetEnv(spar.Data(),0.0);
//    else              lval = gEnv->GetValue(spar.Data(),0.0);
//    SetChannelWidth(lval);
//    //--- TauRC
//    spar.Form("%s.%s.TauRC",dettype,GetName());
//    if (gDataSet) lval = gDataSet->GetDataSetEnv(spar.Data(),0.0);
//    else              lval = gEnv->GetValue(spar.Data(),0.0);
//    SetTauRC(lval);
//
//    //--- ShaperRiseTime for trapezoidal filter
//    spar.Form("%s.%s.ShaperRiseTime",dettype,GetName());
//    if (gDataSet) lval = gDataSet->GetDataSetEnv(spar.Data(),0.0);
//    else              lval = gEnv->GetValue(spar.Data(),0.0);
//    Double_t risetime = lval;
//    //--- ShaperFlatTop for trapezoidal filter
//    spar.Form("%s.%s.ShaperFlatTop",dettype,GetName());
//    if (gDataSet) lval = gDataSet->GetDataSetEnv(spar.Data(),0.0);
//    else              lval = gEnv->GetValue(spar.Data(),0.0);
//    Double_t flattop = lval;
//    SetTrapShaperParameters(risetime,flattop);
//    //--- Pole-Zero Correction
//    spar.Form("%s.%s.PZCorrection",dettype,GetName());
//    if (gDataSet) lval = gDataSet->GetDataSetEnv(spar.Data(),0.0);
//    else              lval = gEnv->GetValue(spar.Data(),0.0);
//    if (lval==1) SetPoleZeroCorrection(kTRUE);
//    else           SetPoleZeroCorrection(kFALSE);


}
//________________________________________________________________

KVChargeSignal::~KVChargeSignal()
{
   // Destructor
}

//________________________________________________________________

void KVChargeSignal::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVChargeSignal::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSignal::Copy(obj);
   //KVChargeSignal& CastedObj = (KVChargeSignal&)obj;
}

// //________________________________________________________________
// KVPSAResult* KVChargeSignal::TreateSignal()
// {
//    //to be implemented in child class
//    if (GetN()==0) {
//       //Info("TreateSignal","Empty signal %s",GetName());
//       return 0;
//    }
//    KVPSAResult* psa = new KVPSAResult(GetName());
//    psa->SetValue(Form("%s.%s.RawAmplitude",fDetName.Data(),fType.Data()),GetRawAmplitude());
//    if (fAdc.fN==0) SetADCData();
//
//    ComputeBaseLine();
//    Add(-1.*fBaseLine);
//    ApplyModifications();
//
//    if(fWithPoleZeroCorrection) PoleZeroSuppression(fTauRC);
//    FIR_ApplyTrapezoidal(fTrapRiseTime,fTrapFlatTop);
//
//    ComputeAmplitude();
//
//    //Init(); remplacer par SetADCData()
//    //
//    SetADCData();
//    ComputeRiseTime();
//
//    // storing result
//    psa->SetValue(Form("%s.%s.BaseLine",fDetName.Data(),fType.Data()),fBaseLine);
//    psa->SetValue(Form("%s.%s.SigmaBaseLine",fDetName.Data(),fType.Data()),fSigmaBase);
//    psa->SetValue(Form("%s.%s.Amplitude",fDetName.Data(),fType.Data()),fAmplitude);
//    psa->SetValue(Form("%s.%s.RiseTime",fDetName.Data(),fType.Data()),fRiseTime);
//
//    return psa;
//
//
//    /*
//    Int_t xmin_bl = 0;
//    Int_t xmax_bl = 200;
//    psa->SetValue("BaseLine.Xmin",xmin_bl);
//    psa->SetValue("BaseLine.Xmax",xmax_bl);
//    Double_t xx,yy;
//    GetPoint(0,xx,yy);
//    */
//    Int_t nn = GetN();
//    if (!fFunc1){
//       //Info("TreateSignal","Creation de la fonction de fit");
//       fFunc1 = new TF1("KVChargeSignal1","[0]+[1]/(1+TMath::Exp([2]-x))",0,nn-1); fFunc1->SetNpx(nn);
//       fFunc2 = new TF1("KVChargeSignal2","[0]+([1]+[3]*x)/(1+TMath::Exp([2]-x))",0,nn-2); fFunc2->SetNpx(nn);
//    }
//
//    Double_t par0 = GetMean(2)-2*GetRMS(2);   //BaseLine
//    Double_t par1 = GetRMS(2); //Amplitude
//    Double_t par2 = GetMean(1);   //Front
//
//    fFunc1->SetParameters(par0,par1,par2);
//    fFunc1->SetParLimits(1,0,1e5);
//    fFunc1->SetParLimits(2,50,nn);
//
//    //Determination ligne de base par fit pol0
//    //
//    Int_t times=0;
//    Int_t nres1 = Int_t(Fit(fFunc1,"0WRQ"));
//    if (nres1!=0 &&  times<3){
//       nres1 = Int_t(Fit(fFunc1,"0WRQ"));
//       times+=1;
//    }
//    if (nres1!=0){
//     Error("TreateSignal","%s:%s : Fit #1 of the signal failed",fDetName.Data(),fType.Data());
//       return 0;
//    }
//    for (Int_t nn=0;nn<fFunc1->GetNpar();nn+=1)
//       fFunc2->SetParameter(nn,fFunc1->GetParameter(nn));
//    fFunc2->SetParameter(fFunc2->GetNpar()-1,0.);
//
//    Int_t nres2 = Int_t(Fit(fFunc2,"0WRQ"));
//    if (nres2!=0){
//     Warning("TreateSignal","%s:%s : Fit #2 of the signal failed",fDetName.Data(),fType.Data());
//       psa->SetValue("BaseLine.Fit",fFunc1->GetParameter(0));
//       psa->SetValue("Amplitude.Fit",fFunc1->GetParameter(1));
//       psa->SetValue("Front0.Fit",fFunc1->GetParameter(2));
//       psa->SetValue("Front1.Fit",fFunc1->GetParameter(3));
//    }
//    else{
//       psa->SetValue("BaseLine.Fit",fFunc2->GetParameter(0));
//       psa->SetValue("Amplitude.Fit",fFunc2->GetParameter(1));
//       psa->SetValue("Front0.Fit",fFunc2->GetParameter(2));
//       psa->SetValue("Front1.Fit",fFunc2->GetParameter(3));
//    }
//    //Min et Max de la ligne de base
//    /*
//    Double_t blmin = 1e6;
//    Double_t blmax = -1e6;
//    for (Int_t nn=xmin_bl;nn<xmax_bl;nn+=1)
//    {
//       GetPoint(nn,xx,yy);
//       yy-=ybl;
//       if (yy>blmax) blmax=yy;
//       if (yy<blmin) blmin=yy;
//    }
//
//    Double_t trigger = 0;
//    Double_t threshold = blmax;
//
//    psa->SetValue("Noise.Threshold",blmax);
//
//    Double_t max_signal=-1e6;
//    Double_t tmax_signal=-1;
//
//    for (Int_t nn=0;nn<GetN();nn+=1)
//    {
//       GetPoint(nn,xx,yy);
//       yy-=ybl;
//       if (yy>threshold){
//          trigger += 1;
//          if (yy>max_signal){
//             max_signal = yy;
//             tmax_signal = xx;
//          }
//       }
//       SetPoint(nn,xx,yy);
//    }
//    psa->SetValue("Noise.Trigger",trigger/GetN());
//    psa->SetValue("Signal.Max",max_signal);
//    psa->SetValue("Signal.tMax",tmax_signal);
//    */
//    delete fFunc1;
//    delete fFunc2;
//    return psa;
//
// }
// //________________________________________________________________
// KVPSAResult* KVChargeSignal::TreateSignal(TF1* filter)
// {
//    //to be implemented in child class
//    KVPSAResult* psa = new KVPSAResult(GetName());
//    Int_t nn = GetN();
//    if (!filter){
//       return 0;
//    }
//
//    Double_t xx,yy,y0;
//    Double_t ymin=1e6,ymax=-1e6;
//    GetPoint(0,xx,y0);
//    for (Int_t nn=0;nn<GetN();nn+=1){
//       GetPoint(nn,xx,yy);
//       if (yy>ymax) ymax=yy;
//       if (yy<ymin) ymin=yy;
//    }
//
//    Double_t width = ymax-ymin;
//
//    Double_t par0 = y0; //GetMean(2)-3*GetRMS(2);   //BaseLine
//    Double_t par1 = width; //2*GetRMS(2);     //Amplitude
//    Double_t par2 = GetMean(1);   //Front
//
//    filter->SetParameters(par0,par1,par2,40);
//    //filter->SetParameters(-7350,par1,590,70);
//    filter->SetParLimits(1,0,width*1.1);
//    filter->SetParLimits(2,50,nn);
//    filter->SetParLimits(3,0.01,nn);
//    //filter->SetParLimits(Parameter(3,1);
//    //filter->FixParameter(4,1);
//
//    /*
//    filter->SetParLimits(3,-1,0.0001);
//    filter->SetParLimits(4,0.000,100);
//    */
//    //Determination ligne de base par fit pol0
//    //
//    Int_t times=0;
//    Int_t nres = Int_t(Fit(filter,"WRQ"));
//    while (nres!=0 &&  times<10){
//       nres = Int_t(Fit(filter,"WRQ"));
//       times+=1;
//       //printf("on recommence Fit #1 %d/10\n",times);
//    }
//    if (nres!=0){
//     Error("TreateSignal","%s:%s : Fit #1 of the signal failed",fDetName.Data(),fType.Data());
//       return 0;
//    }
//
//
//    /*
//    filter->ReleaseParameter(3);
//    filter->SetParLimits(3,-1,0.000);
//    */
//    /*
//    filter->ReleaseParameter(4);
//    filter->SetParLimits(4, 0,10);
//    */
//    /*
//    times=0;
//    nres = Int_t(Fit(filter,"WRQ"));
//    while (nres!=0 &&  times<10){
//       nres = Int_t(Fit(filter,"WRQ"));
//       times+=1;
//       printf("on recommence Fit #2 %d/10\n",times);
//    }
//    if (nres!=0){
//     Error("TreateSignal","%s : Fit #2 of the signal failed",fDetName.Data(),fType.Data());
//       return 0;
//    }
//    */
//    //Int_t np=0;
//    psa->SetValue("width",width);
//
//    for (Int_t nn=0;nn<filter->GetNpar();nn+=1)
//       filter->ReleaseParameter(nn);
//
//    return psa;
//
// }

Double_t KVChargeSignal::GetMaxFluctuationsWindow(Double_t* window, Int_t width)
{
   if (!window) return 0;
   TH1F* h2 = new TH1F("windows", "windows", (TMath::Nint(GetAmplitude()) + 1), GetYmin() - 0.5, GetYmax() + 0.5);
   TGraph* gmoy = new TGraph();
   TGraph* grms = new TGraph();
   if (bidim) delete bidim;
   bidim = new TGraph();
   Double_t xx, yy;
   Int_t nsamples = GetN() / width;

   printf("npoints=%d - nsamples=%d - width=%d\n", GetN(), nsamples, width);

   Double_t rms_max = 0;
   Int_t irms_max = -1;
   for (Int_t ii = 0; ii < nsamples; ii += 1) {
      h2->Reset();
      Double_t xinf = 0;
      for (Int_t nn = 0; nn < width; nn += 1) {
         GetPoint(width * ii + nn, xx, yy);
         if (nn == 0) xinf = xx;
         h2->Fill(yy);
      }
      Double_t xmoy = xinf;
      Double_t rms = h2->GetRMS();
      Double_t mean = h2->GetMean();
      printf("%d %lf %lf %lf\n", ii, xmoy, rms, mean);
      gmoy->SetPoint(ii, xmoy, mean);
      grms->SetPoint(ii, xmoy, rms);
      bidim->SetPoint(ii, mean, rms);
      if (rms > rms_max) {
         //endroit ou les fluctuations sont maximums
         rms_max = h2->GetRMS();
         irms_max = ii;
      }
   }

   Int_t ideb = irms_max;
   Double_t ybefore = rms_max;
   Double_t yafter = -1;
   Int_t iparcours = ideb - 1;

   grms->GetPoint(iparcours, xx, yafter);
   while (yafter < ybefore) {
      ybefore = yafter;
      iparcours -= 1;
      grms->GetPoint(iparcours, xx, yafter);
   }
   Double_t xgauche = 0;
   grms->GetPoint(iparcours + 1, xgauche, yy);
   //xgauche-=width/2.;
   Double_t ybas = 0;
   gmoy->GetPoint(iparcours + 1, xx, ybas);

   ideb = irms_max;
   ybefore = rms_max;
   iparcours = ideb + 1;

   grms->GetPoint(iparcours, xx, yafter);
   while (yafter < ybefore) {
      ybefore = yafter;
      iparcours += 1;
      grms->GetPoint(iparcours, xx, yafter);
   }
   Double_t xdroite = 0;
   grms->GetPoint(iparcours - 1, xdroite, yy);
   //xdroite+=width/2.;
   Double_t yhaut = 0;
   gmoy->GetPoint(iparcours - 1, xx, yhaut);

   window[0] = xgauche;
   window[1] = xdroite;
   window[2] = ybas;
   window[3] = yhaut;

   delete h2;
   delete gmoy;
   delete grms;
   return rms_max;
}
