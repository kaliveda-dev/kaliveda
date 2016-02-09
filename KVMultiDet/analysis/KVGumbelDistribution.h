//Created by KVClassFactory on Mon Mar 19 12:14:55 2012
//Author: John Frankland,,,

#ifndef __KVGUMBELDISTRIBUTION_H
#define __KVGUMBELDISTRIBUTION_H

#include "TF1.h"

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
