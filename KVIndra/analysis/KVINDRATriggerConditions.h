#ifndef __KVINDRATRIGGERCONDITIONS_H
#define __KVINDRATRIGGERCONDITIONS_H

#include "KVTriggerConditions.h"

/**
 \class KVINDRATriggerConditions
 \brief Set trigger conditions for analysis of reconstructed INDRA data
\ingroup INDRAnalysis

 Implements rejection of reconstructed events with a number of correctly-identified particles
 which is less than the on-line acquisition multiplicity trigger for the run.

 To use, put in the InitRun() method of your analysis class a call to KVEventSelector::SetTriggerConditionsForRun()
 with the current run number as argument.

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
