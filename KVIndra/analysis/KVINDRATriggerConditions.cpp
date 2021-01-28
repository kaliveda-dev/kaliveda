#include "KVINDRATriggerConditions.h"
#include "KVEventSelector.h"
#include <KVVarGlob.h>
#include "KVINDRA.h"

void KVINDRATriggerConditions::SetTriggerConditionsForRun(KVEventSelector* Selec, int)
{
   // For (classic) INDRA data, respecting the trigger conditions means rejecting all events with
   // a multiplicity of correctly-identified & calibrated particles less than the trigger
   // multiplicity for the run

   KVVarGlob* vg = Selec->GetGVList()->AddGVFirst("KVArrayMult", "INDRA");
   vg->SetMaxNumBranches(0);
   Int_t trig = gIndra->GetTrigger();
   Info("SetTriggerConditionsForRun", "Minimum multiplicity for run : %d", trig);
   vg->SetEventSelection([ = ](const KVVarGlob * V) {
      return V->GetValue() >= trig;
   });
   vg->Init();//initialise by hand, list will have been 'Init'ed already
}

ClassImp(KVINDRATriggerConditions)


