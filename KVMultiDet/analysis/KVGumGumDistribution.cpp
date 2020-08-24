//Created by KVClassFactory on Wed Jun 13 11:49:37 2012
//Author: dgruyer

#include "KVGumGumDistribution.h"
#include "TMath.h"
#include "Riostream.h"

ClassImp(KVGumGumDistribution)


KVGumGumDistribution::KVGumGumDistribution(): TF1(), fFirstGumbel(0), fLastGumbel(0)
{
   // Default constructor
   SetLineColor(kBlack);
   SetNpx(2000);
   fkFac = 0.;
}

//________________________________________________________________

KVGumGumDistribution::KVGumGumDistribution(const Char_t* name, Int_t k, Double_t xmin, Double_t xmax)
   : TF1(name, this, &KVGumGumDistribution::GDk, xmin, xmax, 5,
         "KVGumGumDistribution", "GDk"), fFirstGumbel(0), fLastGumbel(0)
{
   // normalised sum of Gumbel distribution and Gumbel distribution of k-th rank
   // f = eta*Gumbel(mu,sigma) + (1-eta)*Gumbel(ap,bp)
   // free parameters: gauss: (mean,sigma) gumbel: (a,b) eta
   fRank = k;
   fkFac = TMath::Power(k, k) / TMath::Factorial(k - 1);

   SetParName(0, "#eta");
   SetParName(1, "#mu");
   SetParName(2, "#sigma");
   SetParName(3, "#mu-a_{m}");
   SetParName(4, "b_{m}");

   SetParLimits(0, 0., 1.);
   SetParLimits(1, GetXmin(), GetXmax());
   SetParLimits(2, 0.01, 200.);
   SetParLimits(3, 0.01, 300.);
   SetParLimits(4, 0.01, 200.);

   SetLineColor(kBlack);
   SetNpx(2000);
}

//________________________________________________________________

KVGumGumDistribution::KVGumGumDistribution(const KVGumGumDistribution& obj)  : TF1()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVGumGumDistribution::~KVGumGumDistribution()
{
   // Destructor
   SafeDelete(fFirstGumbel);
   SafeDelete(fLastGumbel);
}

//________________________________________________________________

void KVGumGumDistribution::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVGumGumDistribution::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TF1::Copy(obj);
   KVGumGumDistribution& CastedObj = (KVGumGumDistribution&)obj;
   CastedObj.fRank = fRank;
   CastedObj.fkFac = fkFac;
}

//________________________________________________________________

void KVGumGumDistribution::Paint(Option_t* option)
{
   // Draw total distribution and the two component distributions

   TF1::Paint(option);

   if (!fLastGumbel)  fLastGumbel = new KVGumbelDistribution("GumbelLast", fRank, false, GetXmin(), GetXmax());
   if (!fFirstGumbel) fFirstGumbel = new KVGumbelDistribution("GumbelFirst", fRank, false, GetXmin(), GetXmax());
   fLastGumbel->SetParameters(GetParameter(1), -GetParameter(2), -fkFac * GetParameter(0));
   fFirstGumbel->SetParameters(GetParameter(1) - GetParameter(3), GetParameter(4), fkFac * (1. - GetParameter(0)));

   fLastGumbel->SetLineColor(kBlue);
   fLastGumbel->SetLineStyle(9);
   fLastGumbel->SetLineWidth(1);
   fLastGumbel->SetNpx(2000);

   fFirstGumbel->SetLineColor(kRed);
   fFirstGumbel->SetLineWidth(1);
   fFirstGumbel->SetNpx(2000);

   fLastGumbel->Paint(option);
   fFirstGumbel->Paint(option);
}


//________________________________________________________________

Double_t KVGumGumDistribution::GDk(Double_t* x, Double_t* p)
{
   // Evaluate normalised sum of Gaussian and Gumbel distribution of rank fRank for x
   // with parameters
   // par[0] = eta
   // par[1] = mu
   // par[2] = sigma
   // par[3] = distance between gaussian and gumbel >=0
   // par[4] = b

//   if(p[4]==0) return 0;

   Double_t am = p[1] - p[3];
   Double_t s = (*x - am) / p[4];
   Double_t gum = (fkFac / p[4]);
   Double_t es = -fRank * (s + TMath::Exp(-s));
   gum *= TMath::Exp(es);

   Double_t am2 = p[1];
   Double_t bm2 = -p[2];
   Double_t s2 = (*x - am2) / bm2;
   Double_t gum2 = -(fkFac / bm2);
   Double_t es2 = -fRank * (s2 + TMath::Exp(-s2));
   gum2 *= TMath::Exp(es2);

   Double_t gg = p[0] * gum2 + (1. - p[0]) * gum;
   return gg;
}

