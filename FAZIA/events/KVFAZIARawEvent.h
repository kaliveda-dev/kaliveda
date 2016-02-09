//Created by KVClassFactory on Wed Jul 15 11:54:35 2015
//Author: ,,,

#ifndef __KVFAZIARAWEVENT_H
#define __KVFAZIARAWEVENT_H

#include "TObject.h"
#include "TClonesArray.h"

#include "KVSignal.h"
#include "KVString.h"
#include "KVNameValueList.h"

class KVFAZIARawEvent : public TObject {
protected:
   TClonesArray* fSignals;//-> array where signals are storred
   KVNameValueList* fValues;//-> list of values
   Int_t fNumber; //event number

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
   KVNameValueList* GetValues() const
   {
      return fValues;
   }

   ClassDef(KVFAZIARawEvent, 1) //Handle list of signals and additional information coming from acquisition file
};

#endif
