//Created by KVClassFactory on Thu Jan 27 17:23:54 2011
//Author: Eric Bonnet,,,

#ifndef __KVMASSEXCESS_H
#define __KVMASSEXCESS_H

#include "KVNuclData.h"

/**
\class KVMassExcess
\brief Simple class for store life time information of nucleus
\ingroup NucProp
*/

class KVMassExcess : public KVNuclData {

public:
   KVMassExcess();
   KVMassExcess(const Char_t* name);
   virtual ~KVMassExcess();

   ClassDef(KVMassExcess, 1) //Simple class to store mass information of nucleus
};

#endif
