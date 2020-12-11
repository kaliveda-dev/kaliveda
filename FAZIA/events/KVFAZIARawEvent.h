//Created by KVClassFactory on Wed Jul 15 11:54:35 2015
//Author: ,,,

#ifndef __KVFAZIARAWEVENT_H
#define __KVFAZIARAWEVENT_H

#include "TObject.h"
#include "TClonesArray.h"

#include "KVSignal.h"
#include "KVString.h"
#include "KVNameValueList.h"
/**
  \class KVFAZIARawEvent
  \ingroup FAZIAnal
  \brief Obsolete raw data class for FAZIA
 */
class KVFAZIARawEvent : public TObject {
protected:
   TClonesArray* fSignals;//-> array where signals are storred
   KVNameValueList* fValues;//-> list of values
   Int_t fNumber; //event number
   Int_t fNumberOfBlocks; //number of blocks stored in the acquisition event
public:
   KVFAZIARawEvent(Int_t ntot);
   KVFAZIARawEvent();
   virtual ~KVFAZIARawEvent();
public:
   void Copy(TObject& obj) const;
   void Clear(Option_t* = "");
   KVSignal* AddNewSignal(KVString name, KVString title);
   TClonesArray* GetSignals() const
   {
      return fSignals;
   }
   void SetNumber(Int_t num)
   {
      fNumber = num;
   }
   Int_t GetNumber() const
   {
      return fNumber;
   }
   void SetNumberOfBlocks(Int_t num)
   {
      fNumberOfBlocks = num;
   }
   Int_t GetNumberOfBlocks() const
   {
      return fNumberOfBlocks;
   }
   KVNameValueList* GetValues() const
   {
      return fValues;
   }
   const Char_t* GetFPGAEnergy(Int_t blk, Int_t qua, Int_t tel, TString signaltype, Int_t idx = 0);

   ClassDef(KVFAZIARawEvent, 2) //Handle list of signals and additional information coming from acquisition file
};

#endif
