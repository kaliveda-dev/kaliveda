#ifndef __KVTRIGGERCONDITIONS_H
#define __KVTRIGGERCONDITIONS_H

#include "KVBase.h"
class KVEventSelector;
/**
 \class KVTriggerConditions
 \brief Set trigger conditions for analysis of reconstructed data
\ingroup Analysis

 Write a detailed documentation for your class here, see doxygen manual for help.

 \author John Frankland
 \date Thu Jan 28 16:32:51 2021
*/

class KVTriggerConditions : public KVBase {
public:
   KVTriggerConditions() {}
   virtual ~KVTriggerConditions() {}

   virtual void SetTriggerConditionsForRun(KVEventSelector*, int) = 0;

   ClassDef(KVTriggerConditions, 1) //Set trigger conditions for analysis of reconstructed data
};

#endif
