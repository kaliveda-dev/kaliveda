//Created by KVClassFactory on Fri Nov 30 09:01:11 2012
//Author: dgruyer

#ifndef __KVSpiderLineSiCsI_H
#define __KVSpiderLineSiCsI_H

#include "KVSpiderLine.h"

/**
\class KVSpiderLineSiCsI
\brief KVSpiderLine specialized for PSA matrix
\ingroup Identification
*/

class KVSpiderLineSiCsI : public KVSpiderLine {

public:
   KVSpiderLineSiCsI();
   KVSpiderLineSiCsI(int z_, Double_t pdy_ = 0.);
   KVSpiderLineSiCsI(int z_, int a_);
//   KVSpiderLineSiCsI(const KVSpiderLineSiCsI&) ;
   virtual ~KVSpiderLineSiCsI();
//   void Copy(TObject&) const;

   virtual TF1* GetFunction(double min_ = -1., double max_ = -1.);


   ClassDef(KVSpiderLineSiCsI, 1) //KVSpiderLine specialized for PSA matrix
};

#endif
