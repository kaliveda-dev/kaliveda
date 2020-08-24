//Created by KVClassFactory on Wed Jun 13 11:49:37 2012
//Author: dgruyer

#ifndef __KVGUMGUMDISTRIBUTION_H
#define __KVGUMGUMDISTRIBUTION_H

#include "TF1.h"
#include "KVGumbelDistribution.h"

/**
\class KVGumGumDistribution
\brief Sum of normalised Gumbel first asymptote and Gumbel last asymptote
\ingroup Math
*/

class KVGumGumDistribution : public TF1 {
protected:
   Int_t fRank;//rank of Gumbel distribution
   Double_t fkFac;
   Double_t fkGaussNor;

   KVGumbelDistribution* fFirstGumbel;//! non-persistent (not written to disk)
   KVGumbelDistribution* fLastGumbel;//! non-persistent (not written to disk)

public:
   KVGumGumDistribution();
   KVGumGumDistribution(const Char_t* name, Int_t k = 1, Double_t xmin = 0., Double_t xmax = 100.);
   KVGumGumDistribution(const KVGumGumDistribution&) ;
   virtual ~KVGumGumDistribution();
   void Copy(TObject&) const;

   void  Paint(Option_t* option = "");
   Double_t GDk(Double_t* x, Double_t* p);

   ClassDef(KVGumGumDistribution, 1) //Sum of normalised Gumbel first asymptote and Gumbel last asymptote
};

#endif
