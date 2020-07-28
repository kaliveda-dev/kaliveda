//Created by KVClassFactory on Wed Mar 23 14:04:55 2016
//Author: John Frankland,,,

#ifndef __KVIDCSI_H
#define __KVIDCSI_H

#include "KVIDTelescope.h"

/**
  \class KVIDCsI
  \ingroup Identification
  \brief Generic CsI identification telescope
 */
class KVIDCsI : public KVIDTelescope {
public:
   KVIDCsI();

   virtual ~KVIDCsI();
   virtual Bool_t CanIdentify(Int_t Z, Int_t /*A*/)
   {
      // Used for filtering simulations
      // Returns kTRUE if this telescope is theoretically capable of identifying a given nucleus,
      // without considering thresholds etc.
      // For CsI Rapide-Lente detectors, identification is typically possible up to Z=4
      return (Z < 5);
   }
   virtual void Initialize(void);

   ClassDef(KVIDCsI, 1) //A generic 1-member CsI fast-slow identification telescope
};

#endif
