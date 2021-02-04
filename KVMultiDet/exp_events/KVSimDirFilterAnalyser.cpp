//Created by KVClassFactory on Thu Mar 23 14:02:40 2017
//Author: John Frankland,,,

#include "KVSimDirFilterAnalyser.h"
#include <KVClassFactory.h>
#include <KVDataSetManager.h>
#include <KVExpDB.h>
#include <KVTriggerConditions.h>
#include "KVMultiDetArray.h"

ClassImp(KVSimDirFilterAnalyser)

KVSimDirFilterAnalyser::KVSimDirFilterAnalyser()
   : KVSimDirAnalyser()
{
   // Default constructor
}

//____________________________________________________________________________//

KVSimDirFilterAnalyser::~KVSimDirFilterAnalyser()
{
   // Destructor

   if (fKinematics->TestBit(kCanDelete)) delete fKinematics;
}

void KVSimDirFilterAnalyser::preInitAnalysis()
{
   // Use options passed to KVEventSelector to initialise dataset used for filter

   if (!gDataSetManager) {
      gDataSetManager = new KVDataSetManager;
      gDataSetManager->Init();
   }
   if (!gDataSet) gDataSetManager->GetDataSet(fAnalysisClass->GetOpt("DataSet"))->cd();
}

void KVSimDirFilterAnalyser::preInitRun()
{
   // Use options passed to KVEventSelector to initialise kinematics for reaction
   // and build experimental set-up

   KVString system = fAnalysisClass->GetOpt("System");
   KVDBSystem* sys = gExpDB->GetSystem(system);
   if (sys) {
      fKinematics = sys->GetKinematics();
   }
   else {
      fKinematics = new KV2Body(system);
      fKinematics->SetBit(kCanDelete); // clean up in destructor
      fKinematics->CalculateKinematics();
   }
   KVString run = fAnalysisClass->GetOpt("Run");
   KVMultiDetArray::MakeMultiDetector(gDataSet->GetName(), run.Atoi());
}

void KVSimDirFilterAnalyser::preAnalysis()
{
   // Called by KVEventSelector just after reading new event and just before
   // calling user analysis.
   // Set minimum acceptable multiplicity for event.
   gMultiDetArray->SetMinimumOKMultiplicity(fAnalysisClass->GetEvent());
}

void KVSimDirFilterAnalyser::Make(const Char_t* kvsname)
{
   // Generate a new filtered analysis selector class

   KVClassFactory cf(kvsname, "Analysis of filtered simulated events", "",
#ifndef USING_ROOT6
                     kTRUE, "FilteredEventAnalysisTemplate");
#else
                     kTRUE, "ROOT6FilteredEventAnalysisTemplate");
#endif
   cf.AddImplIncludeFile("KVReconstructedNucleus.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");

   cf.GenerateCode();
}

#ifdef USING_ROOT6
void KVSimDirFilterAnalyser::SetTriggerConditionsForRun(int run)
{
   // When called from the InitRun() method of a user's analysis class, this method will ensure that only data
   // compatible with the experimental trigger will be provided for analysis in the user's Analysis() method.
   //
   // This will be done by searching for a KVTriggerConditions plugin class defined for the currently-analysed
   // dataset, defined like so:
   //
   //~~~~
   //+Plugin.KVTriggerConditions:   [dataset]   [classname]  [libname]   "[default constructor]()"
   //~~~~

   TPluginHandler* ph = KVBase::LoadPlugin("KVTriggerConditions", gDataSet->GetName());
   if (!ph) {
      Info("SetTriggerConditionsForRun",
           "No definition of trigger conditions available for dataset %s",
           gDataSet->GetName());
      return;
   }
   std::unique_ptr<KVTriggerConditions> trig((KVTriggerConditions*) ph->ExecPlugin(0));
   trig->SetTriggerConditionsForRun(fAnalysisClass, run);
}
#endif
//____________________________________________________________________________//

