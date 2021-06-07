//Created by KVClassFactory on Fri Oct 14 15:38:36 2016
//Author: John Frankland,,,

#ifndef __KVVALUERANGE_H
#define __KVVALUERANGE_H

#include "KVBase.h"

/**
  \class  KVValueRange
 \brief Range of values specified by minimum, maximum
\ingroup Core
*/


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
      : KVBase()
   {
      r.Copy(*this);
   }
   void Copy(TObject& o) const
   {
      KVBase::Copy(o);
      KVValueRange& vr = static_cast<KVValueRange&>(o);
      vr.fMin = fMin;
      vr.fMax = fMax;
      vr.fSize = fSize;
   }
   ROOT_COPY_ASSIGN_OP(KVValueRange)
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
   TString GetExpression(const TString& obs_name)
   {
      // Return cut expressed as "Xmin \leq X \leq Xmax" with "X" given by obs_name
      return Form("%g #leq %s #leq %g", Min(), obs_name.Data(), Max());
   }

   ClassDef(KVValueRange, 1) //Range of values specified by minimum, maximum
};
#endif
