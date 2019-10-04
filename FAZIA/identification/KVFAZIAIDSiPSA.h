//Created by KVClassFactory on Mon Feb 17 13:52:51 2014
//Author: John Frankland,,,

#ifndef __KVFAZIAIDSiPSA_H
#define __KVFAZIAIDSiPSA_H

#include "KVFAZIAIDTelescope.h"
#include "KVIDZAGrid.h"
#include "KVFAZIADetector.h"
#include "KVIDCutLine.h"
#include "TF1.h"

class KVFAZIAIDSiPSA : public KVFAZIAIDTelescope {

   static TF1* fZThreshold;//! empirical threshold for Z identification
   static TF1* fAThreshold;//! empirical threshold for A identification

   KVIDZAGrid* IGrid;    //! telescope's Imax E grid
   KVIDZAGrid* QGrid;    //! telescope's Qrise E grid
   KVFAZIADetector* fSi; //! the silicon detector

public:
   KVFAZIAIDSiPSA();
   virtual ~KVFAZIAIDSiPSA();
   virtual UShort_t GetIDCode()
   {
      return kSi1;
   }

   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual void Initialize();

   Bool_t CheckTheoreticalIdentificationThreshold(KVNucleus* /*ION*/, Double_t /*EINC*/ = 0.0);
   void SetIdentificationStatus(KVReconstructedNucleus*);

   ClassDef(KVFAZIAIDSiPSA, 1) //PSA identification in first silicon of FAZIA telescopes
};

#endif
