//Created by KVClassFactory on Thu Mar 23 14:02:40 2017
//Author: John Frankland,,,

#include "KVSimDirFilterAnalyser.h"
#include <KVClassFactory.h>
#include <KVDataSetManager.h>
#include <KVExpDB.h>
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
                     kTRUE, "FilteredEventAnalysisTemplate");
   cf.AddImplIncludeFile("KVReconstructedNucleus.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");

   cf.GenerateCode();
}

//____________________________________________________________________________//

