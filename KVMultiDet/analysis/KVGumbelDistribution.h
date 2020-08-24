//Created by KVClassFactory on Mon Mar 19 12:14:55 2012
//Author: John Frankland,,,

#ifndef __KVGUMBELDISTRIBUTION_H
#define __KVGUMBELDISTRIBUTION_H

#include "TF1.h"

/**
\class KVGumbelDistribution
\brief Gumbel distributions for rank-ordered extremal variables
\ingroup Math

Consider a probability distribution \f$f(Z)\f$ which is used to generate
m values of the random variable Z, which can then be ordered
in terms of decreasing size
\f[
Z_{1}>Z_{2}> ... >Z_{m}
\f]
If such
a procedure is repeated a sufficiently large number of times, it can
be shown that the resulting probability distributions of the \f$Z_k\f$
are given by the Gumbel distributions:
\f[
\phi_{k}(s_{k}) = \frac{k^{k}}{(k-1)!}\frac{1}{b_{m}}e^{-ks_{k}-ke^{-s_{k}}}
\f]
where
\f[
s_{k}  =  \frac{Z_{k}-a_{k}}{b_{k}}
\f]
*/

class KVGumbelDistribution : public TF1 {
   Int_t fRank;//rank of distribution
   Double_t fkFac;
   Bool_t fNormalised;//=kTRUE if distribution is normalised

public:
   KVGumbelDistribution();
   KVGumbelDistribution(const Char_t* name, Int_t k = 1, Bool_t norm = kTRUE, Double_t xmin = 0., Double_t xmax = 100.);
   KVGumbelDistribution(const KVGumbelDistribution&) ;
   virtual ~KVGumbelDistribution();
   void Copy(TObject&) const;

   Double_t GDk(Double_t* x, Double_t* p);

   ClassDef(KVGumbelDistribution, 1) //Gumbel distributions for rank-ordered extremal variables
};

#endif
