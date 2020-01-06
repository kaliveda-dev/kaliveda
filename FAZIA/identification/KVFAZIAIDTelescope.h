//Created by KVClassFactory on Mon Feb 17 13:51:39 2014
//Author: John Frankland,,,

#ifndef __KVFAZIAIDTELESCOPE_H
#define __KVFAZIAIDTELESCOPE_H

#include "KVIDTelescope.h"
#include "KVReconstructedNucleus.h"
#include "TF1.h"

#include <KVFAZIADetector.h>

class KVFAZIAIDTelescope : public KVIDTelescope {

protected:
   static TF1* fMassIDProb;

public:
   enum {                       //determine identification code
      kSi1Si2 = 12,
      kSi1 = 11,
      kSi2 = 22,
      kCsI = 33,
      kSi2CsI = 23
   };

   KVFAZIAIDTelescope();
   virtual ~KVFAZIAIDTelescope();
   virtual void AddDetector(KVDetector* d);
   static const Char_t* GetNewName(KVString oldname);
   void SetIdentificationStatus(KVReconstructedNucleus* n);

   ClassDef(KVFAZIAIDTelescope, 1) //Identification for FAZIA array
};

#endif
