#ifndef KVINDRAIDTelescope_H
#define KVINDRAIDTelescope_H

#include "KVIDTelescope.h"
/**
  \class KVINDRAIDTelescope
  \ingroup INDRAIdent
  \brief Base class for identification telescopes of INDRA array
 */
class KVINDRAIDTelescope: public KVIDTelescope {

protected:
   UShort_t fZminCode;          //!code corresponding to particle stopping in first member of this type of telescope
   UChar_t fECode;              //!code corresponding to correct calibration by this type of telescope

public:

   KVINDRAIDTelescope();
   virtual ~ KVINDRAIDTelescope();
   void init();

   UShort_t GetZminCode()
   {
      return fZminCode;
   };
   UChar_t GetECode()
   {
      return fECode;
   };
   virtual UShort_t GetBadIDCode();
   virtual UShort_t GetCoherencyIDCode();
   virtual  UShort_t GetMultiHitFirstStageIDCode();
   virtual void SetIDCode(KVReconstructedNucleus*, UShort_t);

   ClassDef(KVINDRAIDTelescope, 1)   //A delta-E - E identification telescope in INDRA

};

#endif
