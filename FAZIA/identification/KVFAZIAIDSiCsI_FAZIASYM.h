//Created by KVClassFactory on Tue Sep  8 16:14:25 2015
//Author: ,,,

#ifndef __KVFAZIAIDSICSI_FAZIASYM_H
#define __KVFAZIAIDSICSI_FAZIASYM_H

#include "KVFAZIAIDTelescope.h"
#include "KVIDZAGrid.h"
#include "KVFAZIADetector.h"
#include "KVIDCutLine.h"
#include "KVIDCutContour.h"


class KVFAZIAIDSiCsI_FAZIASYM : public KVFAZIAIDTelescope {

protected:

   KVIDZAGrid* TheGrid;    //! telescope's grid for principal identification (from Z=1 up to Z=20)
   KVIDZAGrid* TheGrid_lcp;    //! telescope's grid for LCP identification (from Z=1 and Z=2)
   KVFAZIADetector* fCsI;//!
   KVFAZIADetector* fSi2;//!
   KVIDCutContour* fBelowProton; //;KVIDCutLine* fBelowProton; //;
   KVIDCutLine* fSiThreshold; //;


public:

   KVFAZIAIDSiCsI_FAZIASYM();
   virtual ~KVFAZIAIDSiCsI_FAZIASYM();

   virtual UShort_t GetIDCode()
   {
      return kSi2CsI;
   };
   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);

   virtual void Initialize();

   KVIDCutContour* GetBelowProtonLine() const //KVIDCutLine* GetBelowProtonLine() const
   {
      return fBelowProton;
   }
   KVIDCutLine* GetSiThresholdLine() const
   {
      return fSiThreshold;
   }
   void SetIdentificationStatus(KVReconstructedNucleus* n);

   ClassDef(KVFAZIAIDSiCsI_FAZIASYM, 1) //id telescope to manage identification telescope for FAZIASYM experiment
};

#endif
