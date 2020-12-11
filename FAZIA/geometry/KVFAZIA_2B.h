//Created by KVClassFactory on Thu Jan 22 16:02:02 2015
//Author: ,,,

#ifndef __KVFAZIA_2B_H
#define __KVFAZIA_2B_H

#include "KVFAZIA.h"

/**
\class KVFAZIA_2B
\brief FAZIA set-up with two blocks used in LNS 2014 commissioning
\ingroup FAZIAGeo
*/

class KVFAZIA_2B : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();

public:
   KVFAZIA_2B();
   virtual ~KVFAZIA_2B();

   ClassDef(KVFAZIA_2B, 1) //FAZIA set-up with two blocks used in LNS 2014 commissioning
};

#endif
