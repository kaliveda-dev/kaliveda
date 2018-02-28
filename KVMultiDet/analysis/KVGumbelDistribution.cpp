//Created by KVClassFactory on Mon Mar 19 12:14:55 2012
//Author: John Frankland,,,

#include "KVGumbelDistribution.h"
#include "TMath.h"

ClassImp(KVGumbelDistribution)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGumbelDistribution</h2>
<h4>Gumbel distributions for rank-ordered extremal variables</h4>
Consider a probability distribution f(Z) which is used to generate
m values of the random variable Z, which can then be ordered
in terms of decreasing size
<!-- */
// --> END_HTML
/*
BEGIN_LATEX
Z_{1}>Z_{2}> ... >Z_{m}
END_LATEX
*/
// BEGIN_HTML <!--
/* -->
If such
a procedure is repeated a sufficiently large number of times, it can
be shown that the resulting probability distributions of the Z_k
are given by the Gumbel distributions:
<!-- */
// --> END_HTML
/*
BEGIN_LATEX
\phi_{k}(s_{k}) = \frac{k^{k}}{(k-1)!}\frac{1}{b_{m}}e^{-ks_{k}-ke^{-s_{k}}}
END_LATEX
*/
// BEGIN_HTML <!--
/* -->
where
<!-- */
// --> END_HTML
/*
BEGIN_LATEX
s_{k}  =  \frac{Z_{k}-a_{k}}{b_{k}}
END_LATEX
*/
////////////////////////////////////////////////////////////////////////////////

KVGumbelDistribution::KVGumbelDistribution()
   : TF1()
{
   // default ctor
}

//________________________________________________________________

KVGumbelDistribution::KVGumbelDistribution(const Char_t* name, Int_t k, Bool_t norm,
      Double_t xmin, Double_t xmax)
   : TF1(name, this, &KVGumbelDistribution::GDk, xmin, xmax, 3 - (int)norm,
         "KVGumbelDistribution", "GDk")
{
   // Gumbel distribution of k-th rank
   // if norm=kTRUE: normalised PDF, 2 free parameters (a,b)
   // if norm=kFALSE: 3 parameters (a,b,Integral)
   fRank = k;
   fkFac = TMath::Power(k, k) / TMath::Factorial(k - 1);
   fNormalised = norm;
   SetParName(0, "a_{m}");
   SetParLimits(0, 0, 100);
   SetParName(1, "b_{m}");
   SetParLimits(1, 0, 100);
   if (!norm) SetParName(2, "Integral");
}

//________________________________________________________________

KVGumbelDistribution::KVGumbelDistribution(const KVGumbelDistribution& obj)  : TF1()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVGumbelDistribution::~KVGumbelDistribution()
{
   // Destructor
}

//________________________________________________________________

void KVGumbelDistribution::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVGumbelDistribution::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TF1::Copy(obj);
   KVGumbelDistribution& CastedObj = (KVGumbelDistribution&)obj;
   CastedObj.fRank = fRank;
   CastedObj.fkFac = fkFac;
   CastedObj.fNormalised = fNormalised;
}

//________________________________________________________________

Double_t KVGumbelDistribution::GDk(Double_t* x, Double_t* p)
{
   // Evaluate Gumbel distribution of rank fRank for x
   // with parameters
   // par[0] = a
   // par[1] = b
   // If distribution is not normalised, then we have
   // par[2] = normalisation

   if (p[1] == 0.) return 0.;
   Double_t s = (*x - p[0]) / p[1];
   Double_t gum = (fkFac / p[1]);
   Double_t es = -fRank * (s + TMath::Exp(-s));
   gum *= TMath::Exp(es);
   if (!fNormalised) gum *= p[2];
   return gum;
}
