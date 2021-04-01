#ifndef KVACQPARAM_H
#define KVACQPARAM_H

#include "KVBase.h"
#include "TRef.h"

/**
 \class KVEBYEDAT_ACQParam
 \ingroup DAQ
 \brief GANIL VXI/VME 16 bit (maximum) EBYEDAT acquisition parameter
*/

class KVDetector;

class KVEBYEDAT_ACQParam: public KVBase {

private:
   UShort_t* fChannel;          //!pointer to raw data i.e. value read from coder (channel number)
   UShort_t fData;              //!Dummy used when reading back events from a TTree etc.
   UChar_t fNbBits;         //number of bits (<=16) actually used by parameter
   Bool_t* fFired;            // set when present in raw event

public:
   void init();
   KVEBYEDAT_ACQParam();
   KVEBYEDAT_ACQParam(const TString& name, const TString& type = "");
   KVEBYEDAT_ACQParam(const KVEBYEDAT_ACQParam&);
   ROOT_COPY_ASSIGN_OP(KVEBYEDAT_ACQParam)
   virtual ~ KVEBYEDAT_ACQParam() {}

   void Clear(Option_t* = "")
   {
   }

   UShort_t** ConnectData()
   {
      //used with GTGanilData::Connect() in order to prepare for
      //reading of data from DLT's
      return &fChannel;
   }
   Bool_t** ConnectFired()
   {
      //used with GTGanilData::Connect() in order to prepare for
      //reading of data from DLT's
      return &fFired;
   }

   Bool_t Fired() const
   {
      return (fFired ? *fFired : false);
   }
   void SetData(UShort_t val)
   {
      fData = val;
      if (fChannel)
         *fChannel = val;
   }
   UShort_t GetData() const
   {
      //Returns value of parameter as read back from raw acquisition event
      if (fChannel)
         return (UShort_t)(*fChannel);
      return (UShort_t) fData;
   }

   void ls(Option_t* option = "") const;

   UShort_t* GetDataAddress()
   {
      return &fData;
   }

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif
   virtual void Print(Option_t* opt = "") const;

   void SetNbBits(UChar_t n)
   {
      fNbBits = n;
   }
   UChar_t GetNbBits() const
   {
      return fNbBits;
   }
   void UseInternalDataMember()
   {
      // Call this method to make the fChannel pointer reference the fData member
      fChannel = &fData;
   }

   ClassDef(KVEBYEDAT_ACQParam, 4) //Data acquisition parameters read from GANIL EBYEDAT data
};

#endif
