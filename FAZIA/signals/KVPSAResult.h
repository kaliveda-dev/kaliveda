//Created by KVClassFactory on Tue Jan 13 15:06:39 2015
//Author: ,,,

#ifndef __KVPSARESULT_H
#define __KVPSARESULT_H

#include "KVNameValueList.h"

class KVPSAResult : public KVNameValueList {

public:
   KVPSAResult();
   KVPSAResult(const Char_t* name, const Char_t* title = "");
   virtual ~KVPSAResult();
   void Copy(TObject& obj) const;

   ClassDef(KVPSAResult, 1) //class to store PSA parameters and results
};

#endif
