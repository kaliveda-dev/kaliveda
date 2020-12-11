//Created by KVClassFactory on Thu Jan 22 16:02:02 2015
//Author: ,,,

#ifndef __KVISOFAZIA_H
#define __KVISOFAZIA_H

#include "KVFAZIA.h"

/**
   \class KVISOFAZIA
\brief FAZIA set-up with two blocks used in LNS 2014 commissioning
\ingroup FAZIAGeo
*/

class KVISOFAZIA : public KVFAZIA {

   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();

public:
   KVISOFAZIA();
   virtual ~KVISOFAZIA();

   ClassDef(KVISOFAZIA, 1) //FAZIA set-up with two blocks used in LNS 2014 commissioning
};

#endif
