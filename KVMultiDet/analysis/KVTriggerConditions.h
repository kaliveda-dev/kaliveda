#ifndef __KVTRIGGERCONDITIONS_H
#define __KVTRIGGERCONDITIONS_H

#include "KVBase.h"
class KVEventSelector;
/**
 \class KVTriggerConditions
 \brief Set trigger conditions for analysis of reconstructed data
\ingroup Analysis

 Implements rejection of reconstructed events which are not consistent with the on-line acquisition trigger for the run.

 To use, put in the InitRun() method of your analysis class a call to KVEventSelector::SetTriggerConditionsForRun()
 with the current run number as argument.

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
