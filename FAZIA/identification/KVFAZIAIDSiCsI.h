//Created by KVClassFactory on Tue Sep  8 16:14:25 2015
//Author: ,,,

#ifndef __KVFAZIAIDSICSI_H
#define __KVFAZIAIDSICSI_H

#include "KVFAZIAIDTelescope.h"

class KVFAZIAIDSiCsI : public KVFAZIAIDTelescope {

public:
   KVFAZIAIDSiCsI();
   virtual ~KVFAZIAIDSiCsI();
public:
   void Copy(TObject& obj) const;

   ClassDef(KVFAZIAIDSiCsI, 1) //id telescope to manage FAZIA Si-CsI identification
};

#endif
