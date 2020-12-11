//Created by KVClassFactory on Tue Jan 26 16:01:47 2016
//Author: bonnet,,,

#include "KVFAZIARawDataAnalyser.h"
#include "KVMultiDetArray.h"
#include "KVDataSet.h"

ClassImp(KVFAZIARawDataAnalyser)

KVFAZIARawDataAnalyser::KVFAZIARawDataAnalyser()
{
   // Default constructor
}

KVFAZIARawDataAnalyser::~KVFAZIARawDataAnalyser()
{
   // Destructor
}

void KVFAZIARawDataAnalyser::preInitAnalysis()
{
   //called by the KVFAZIAReader::SlaveBegin() derived analysis class
   Info("preInitAnalysis", "Appel");
   if (gDataSet && !gMultiDetArray) {
      Info("preInitAnalysis", "Building of FAZIA array");
      KVMultiDetArray::MakeMultiDetector(gDataSet->GetName());
   }
}

void KVFAZIARawDataAnalyser::preInitRun()
{
   //called by the KVFAZIAReader::Notify() derived analysis class
   Info("preInitRun", "Appel");
   gMultiDetArray->SetParameters(fRunNumber);

}

void KVFAZIARawDataAnalyser::SubmitTask()
{
   // Perform analysis of chosen runs
   // Before beginning the loop over the runs, the user's InitAnalysis() method is called.
   // After completing the analysis of all runs, the user's EndAnalysis() method is called.
   //
   // Further customisation of the event loop is possible by overriding the methods
   //   preInitAnalysis()
   //   postInitAnalysis()
   //   preEndAnalysis()
   //   postEndAnalysis()
   // which are executed respectively just before and just after the methods.

   if (gDataSet != GetDataSet()) GetDataSet()->cd();

   //loop over runs
   GetRunList().Begin();
   while (!GetRunList().End() && !AbortProcessingLoop()) {
      fRunNumber = GetRunList().Next();
      Info("SubmitTask", "treatment of run# %d", fRunNumber);
      ProcessRun();
   }

}

void KVFAZIARawDataAnalyser::ProcessRun()
{
   // Perform treatment of a given run
   // Before processing each run, after opening the associated file, user's InitRun() method is called.
   // After each run, user's EndRun() is called.
   // For each event of each run, user's Analysis() method is called.
   //
   // For further customisation, the pre/post-methods are called just before and just after
   // each of these methods (preInitRun(), postAnalysis(), etc. etc.)
   TString fullPathToRunfile = gDataSet->GetFullPathToRunfile(GetDataType(), fRunNumber);
   TFile* f = gDataSet->OpenRunfile<TFile>(GetDataType(), fRunNumber);
   if (!(f && !f->IsZombie())) {
      Error("ProcessRun", "file %s does not exist or is made zombie", fullPathToRunfile.Data());
      return;
   }

   theTree = (TTree*)f->Get("FAZIA");

   TString option("");
   Info("SubmitTask", "Beginning TTree::Process...");
   if (GetNbEventToRead()) {
      theTree->Process(GetUserClass(), option.Data(), GetNbEventToRead());
   }
   else {
      theTree->Process(GetUserClass(), option.Data());
   }

   f->Close();

}
