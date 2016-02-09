//Created by KVClassFactory on Sun Jun 21 10:34:54 2015
//Author: Dijon Aurore

#include "KVChargeStateDistrib.h"
#include "TF1.h"

ClassImp(KVChargeStateDistrib)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVChargeStateDistrib</h2>
<h4>Calculate charge state distribution</h4>
<!-- */
// --> END_HTML
//  Ref: Baron et al NIMA 328(1-2):177-182
////////////////////////////////////////////////////////////////////////////////

KVChargeStateDistrib::KVChargeStateDistrib() : fNuc(NULL), fDistrib(NULL)
{
   // Default constructor
}
//________________________________________________________________
KVChargeStateDistrib::KVChargeStateDistrib(KVNucleus* nuc) : fNuc(nuc), fDistrib(NULL)

{
   // constructor
}
//________________________________________________________________

KVChargeStateDistrib::~KVChargeStateDistrib()
{
   // Destructor
}
//________________________________________________________________

void KVChargeStateDistrib::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVChargeStateDistrib::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVChargeStateDistrib& CastedObj = (KVChargeStateDistrib&)obj;
}
//________________________________________________________________

Double_t KVChargeStateDistrib::GetQmean0()
{

   if (!fNuc) return 0.;

   Double_t W = fNuc->GetEnergyPerNucleon();
   Double_t C = 1.;
   if (W <= 1.3) C = 0.9 + 0.0769 * W;
   Int_t Z = fNuc->GetZ();
   Double_t beta = fNuc->Beta();
   Double_t Qmean = 1.*Z * (1. - C * TMath::Exp(-83.275 * beta / TMath::Power(1.*Z, 0.447)));

   return Qmean;



}
//________________________________________________________________

Double_t KVChargeStateDistrib::GetQmean()
{

   if (!fNuc) return 0.;

   Int_t Z = fNuc->GetZ();
   Double_t Qmean = GetQmean0() * (1. - TMath::Exp(-12.905 + 0.2124 * Z - 0.00122 * Z * Z));

   return Qmean;
}
//________________________________________________________________

Double_t KVChargeStateDistrib::GetQsig()
{

   if (!fNuc) return 0.;

   Int_t Z = fNuc->GetZ();
   Double_t W = fNuc->GetEnergyPerNucleon();
   Double_t Qmean0 = GetQmean0();
   Double_t Y = Qmean0 / Z;
   Double_t Qsig = 0.;

   if (Z >= 54 && W > 1.3) Qsig = TMath::Sqrt(Qmean0 * (0.07535 + 0.19 * Y - 0.2654 * Y * Y));
   else Qsig = 0.5 * TMath::Sqrt(Qmean0 * (1. - TMath::Power(Y, 1.67)));

   return Qsig;
}
//________________________________________________________________

TF1* KVChargeStateDistrib::GetDistribution(KVNucleus* nuc)
{
   if (nuc) SetNucleus(nuc);
   if (!fNuc) return NULL;
   if (!fDistrib) {
      //fDistrib = new TF1("KVChargeStateDistrib", this, &KVChargeStateDistrib::Distrib,
      fDistrib = new TF1("KVChargeStateDistrib", "gaus", 0., 150.);
      fDistrib->SetNpx(150);
   }
   fDistrib->SetParameters(1., GetQmean(), GetQsig());
   fDistrib->SetRange(0., fNuc->GetZ());
   fDistrib->SetTitle(Form("Charge state distribution for %s and E= %f A.MeV", fNuc->GetSymbol(), fNuc->GetEnergyPerNucleon()));

   return fDistrib;
}
//________________________________________________________________

Int_t KVChargeStateDistrib::GetRandomQ(KVNucleus* nuc)
{
   // Draw randomly an integer charge state Q and returns the value

   TF1* dist = GetDistribution(nuc);
   if (!dist) return 0;
   return  Int_t(dist->GetRandom());
}
