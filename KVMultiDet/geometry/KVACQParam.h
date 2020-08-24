#ifndef KVACQPARAM_H
#define KVACQPARAM_H

#include "KVBase.h"
#include "TRef.h"

/**
 \class KVACQParam
 \ingroup DAQ
 \brief GANIL VXI/VME acquisition parameter
*/

class KVDetector;

class KVACQParam: public KVBase {

private:
   UShort_t* fChannel;          //!pointer to raw data i.e. value read from coder (channel number)
   KVDetector* fDet;            //Detector associated with this acquisition parameter
   UShort_t fData;              //!Dummy used when reading back events from a TTree etc.
   Float_t fPied;               //Pedestal value for the current run
   Float_t fDeltaPied;          //Pedestal correction value for the current run
   Bool_t fWorks;            //kFALSE if acquisition parameter was not working
   UChar_t fNbBits;         //number of bits (<=16) actually used by parameter

public:
   void init();
   KVACQParam();
   KVACQParam(const TString& name, const TString& type = "");
   KVACQParam(const KVACQParam&);
   ROOT_COPY_ASSIGN_OP(KVACQParam)
   virtual ~ KVACQParam() {}

   void SetDetector(KVDetector* kd)
   {
      //set detector associated with ACQParam.
      fDet = kd;
   }

   KVDetector* GetDetector() const
   {
      return fDet;
   }
   void Clear(Option_t* = "")
   {
      //Reset value of parameter (put equal to -1)
      SetData((UShort_t) - 1);
   }


   UShort_t** ConnectData()
   {
      //used with GTGanilData::Connect() in order to prepare for
      //reading of data from DLT's
      return &fChannel;
   }

   void SetData(UShort_t val)
   {
      fData = val;
      if (fChannel)
         *fChannel = val;
   }
   Short_t GetCoderData() const
   {
      //Returns value of parameter as read back from raw acquisition event
      //GetCoderData() = -1 if the parameter was not present in the event (coder not fired)
      if (fChannel)
         return (Short_t)(*fChannel);
      return (Short_t) fData;
   }
   Double_t GetData() const
   {
      //Return value of data as floating-point number, by taking the value of
      //GetCoderData() and adding a random number in the range [-0.5,+0.5]
      //The range of values returned will thus go from -0.5 to MAX+0.5, where MAX is the maximum
      //of the coder's range.
      //GetData() = -1 if the parameter was not present in the event (coder not fired)

      if (GetCoderData() >= 0)
         return (Double_t)(GetCoderData()) + gRandom->Uniform(-0.5, 0.5);
      return -1.;
   }

   Bool_t Fired(Option_t* what = "") const
   {
      // Returns kTRUE if acquisition parameter fired in event.
      // Different definitions of "fired" can be used:
      //
      //  what = "" (default) :  acquisition parameter present in event (i.e. not equal to -1)
      //  what = "P"  :  acquisition parameter present in event and greater than currently set pedestal

      return (what[0] ? ((Float_t)GetCoderData() > fPied) : (GetCoderData() != -1));
   }

   void ls(Option_t* option = "") const;

   void SetPedestal(Float_t ped)
   {
      fPied = ped;
   }
   Float_t GetPedestal() const
   {
      return fPied;
   }
   void SetDeltaPedestal(Float_t delta)
   {
      fDeltaPied = delta;
   }
   Float_t GetDeltaPedestal() const
   {
      return fDeltaPied;
   }
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

   Bool_t IsWorking() const
   {
      return fWorks;
   }
   void SetWorking(Bool_t on = kTRUE)
   {
      //if(!on) Info("SetWorking", "Acquisition parameter %s not working", GetName());
      fWorks = on;
   }

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

   ClassDef(KVACQParam, 4)      //Data acquisition parameters read from raw DLT's
};

#endif
