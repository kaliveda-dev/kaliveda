#ifndef __KVVGVECTORSUM_H
#define __KVVGVECTORSUM_H

#include "KVVGObjectSum.h"
#include "TVector3.h"

/**
 \class KVVGVectorSum
 \brief Global variable calculating sum of 3-dimensional vectors
 \ingroup GlobalVariables

The components of the summed vector can be retrieved either by name
("Z", "X", "Y") or by index:

Component | index
--------- | -----
Z         | 0
X         | 1
Y         | 2

 \author John Frankland
 \date Sun Aug  9 16:22:24 2020
*/

class KVVGVectorSum : public KVVGObjectSum<TVector3> {
protected:
   void init();
   Double_t getvalue_int(Int_t) const;

public:
   KVVGVectorSum()
      : KVVGObjectSum<TVector3>()
   {
      init();
   }
   KVVGVectorSum(const Char_t* name)
      : KVVGObjectSum<TVector3>(name)
   {
      init();
   }
   virtual ~KVVGVectorSum() {}

   void Init() {}
   void Calculate() {}

   ClassDef(KVVGVectorSum, 1) //Global variable calculating sum of vectors
};

#endif
