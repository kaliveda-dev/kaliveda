/*
$Id: KVDataSelector.h,v 1.3 2006/11/03 08:01:32 franklan Exp $
$Revision: 1.3 $
$Date: 2006/11/03 08:01:32 $
*/

//Created by KVClassFactory on Fri Oct 20 13:58:16 2006
//Author: Daniel CUSSOL

#ifndef __KVDATASELECTOR_H
#define __KVDATASELECTOR_H

#include "KVBase.h"
class KVSelector;
#include "TEventList.h"

class KVDataSelector: public KVBase {

private:
   KVSelector* kvsel;
   TEventList* evtl;

public:

   KVDataSelector();
   virtual ~ KVDataSelector();

   virtual void SetKVSelector(KVSelector* kvs = 0)
   {
      kvsel = kvs;
   } virtual KVSelector* GetKVSelector(void)
   {
      return kvsel;
   }
   virtual void Reset(Int_t nrun);
   virtual Bool_t ProcessCurrentEntry(void);
   virtual const TEventList* GetTEventList(void)
   {
      return evtl;
   }
   virtual void Init(void);
   virtual Bool_t SelectCurrentEntry(void);

   static void MakeClass(const Char_t* classname,
                         const Char_t* classdesc);

   ClassDef(KVDataSelector, 1)  //Class handling event lists for KVSelector analysis of data chains
};

#endif
