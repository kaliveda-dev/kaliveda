#ifndef KVINDRAIDTelescope_H
#define KVINDRAIDTelescope_H

#include "KVIDTelescope.h"
#include "KVIDSubCodeManager.h"

class KVINDRAIDTelescope: public KVIDTelescope, public KVIDSubCodeManager {

protected:
   UShort_t fIDCode;            //!code corresponding to correct identification by this type of telescope
   UShort_t fZminCode;          //!code corresponding to particle stopping in first member of this type of telescope
   UChar_t fECode;              //!code corresponding to correct calibration by this type of telescope

public:

   KVINDRAIDTelescope();
   virtual ~ KVINDRAIDTelescope();
   void init();

   const Char_t* GetName() const;
   virtual const Char_t* GetArrayName();

   UShort_t GetIDCode()
   {
      return fIDCode;
   };
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
   inline void SetSubCodeManager(UChar_t n_bits,
                                 UChar_t most_significant_bit)
   {
      SetNbits(n_bits);
      SetMSB(most_significant_bit);
   };
   virtual void SetIDCode(KVReconstructedNucleus*, UShort_t);

   ClassDef(KVINDRAIDTelescope, 1)   //A delta-E - E identification telescope in INDRA

};

#endif
