//Created by KVClassFactory on Wed May 17 12:26:57 2006
//Author: franklan

#ifndef __KVINDRAE503_H
#define __KVINDRAE503_H

#include "KVINDRA.h"

/**
\class KVINDRAe503
\brief INDRA multidetector array, configuration for experiments E503 & E494S
\ingroup INDRAGeometry
*/

class KVINDRAe503: public KVINDRA {

protected:

   virtual void SetGroupsAndIDTelescopes();

public:

   KVINDRAe503();
   virtual ~ KVINDRAe503();

   void SetDataSet(const Char_t* name)
   {
      fDataSet = name;
   }

   ClassDef(KVINDRAe503, 1)   //Experimental configuration for INDRA-VAMOS experiments
};

#endif
