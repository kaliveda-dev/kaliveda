#ifndef __KVINDRATRIGGERCONDITIONS_H
#define __KVINDRATRIGGERCONDITIONS_H

#include "KVTriggerConditions.h"

/**
 \class KVINDRATriggerConditions
 \brief Set trigger conditions for analysis of reconstructed INDRA data
\ingroup INDRAnalysis

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author John Frankland
 \date Thu Jan 28 16:49:21 2021
*/

class KVINDRATriggerConditions : public KVTriggerConditions {
public:
   KVINDRATriggerConditions() {}
   virtual ~KVINDRATriggerConditions() {}

   void SetTriggerConditionsForRun(KVEventSelector*, int);

   ClassDef(KVINDRATriggerConditions, 1) //Set trigger conditions for analysis of reconstructed INDRA data
};

#endif
