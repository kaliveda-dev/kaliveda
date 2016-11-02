//Created by KVClassFactory on Fri Oct 14 15:38:36 2016
//Author: John Frankland,,,

#ifndef __KVVALUERANGE_H
#define __KVVALUERANGE_H

#include "KVBase.h"

template< typename ValueType >
class KVValueRange : public KVBase {
   ValueType fMin;
   ValueType fMax;
   ValueType fSize;

public:
   KVValueRange(ValueType min, ValueType max)
      : fMin(min), fMax(max), fSize(fMax - fMin)
   {}
   KVValueRange(const KVValueRange& r)
      : KVBase(), fMin(r.fMin), fMax(r.fMax), fSize(fMax - fMin)
   {}
   ValueType Min() const
   {
      return fMin;
   }
   ValueType Max() const
   {
      return fMax;
   }
   ValueType Size() const
   {
      return fSize;
   }
   ValueType Random() const
   {
      return gRandom->Uniform(fMin, fMax);
   }
   ValueType ValueIofN(Int_t i, Int_t n) const
   {
      // Return i-th value (i=0,1,...,n-1) in sequence of n values
      // beginning with (i=0) fMin and ending with (i=n-1) fMax

      i >= n ? i = n - 1 : (i < 0 ? i = 0 : 1);
      return fMin + i * fSize / (n - 1.);
   }

   virtual ~KVValueRange() {}

   ClassDef(KVValueRange, 1) //Range of values specified by minimum, maximum
};
#endif
