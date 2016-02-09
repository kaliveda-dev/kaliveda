//Created by KVClassFactory on Tue Jan 13 15:11:11 2015
//Author: ,,,

#ifndef __KVCURRENTSIGNAL_H
#define __KVCURRENTSIGNAL_H

#include "KVSignal.h"
#include "KVPSAResult.h"

class KVCurrentSignal : public KVSignal {

protected:
   void init();

public:
   KVCurrentSignal();
   KVCurrentSignal(const char* name);

   virtual ~KVCurrentSignal();
   void Copy(TObject& obj) const;

   virtual void SetDefaultValues();
   virtual void LoadPSAParameters();

   //virtual KVPSAResult* TreateSignal();

   ClassDef(KVCurrentSignal, 1) //digitized intensity signal
};

#endif
