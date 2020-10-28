/*
$Id: KVINDRAGeneDataAnalyser.cpp,v 1.4 2007/11/15 14:59:45 franklan Exp $
$Revision: 1.4 $
$Date: 2007/11/15 14:59:45 $
*/

//Created by KVClassFactory on Mon Jun  4 19:24:11 2007
//Author: e_ivamos

#include "KVINDRAGeneDataAnalyser.h"
#include "TChain.h"
#include "KVDataSet.h"
#include "KVINDRAGeneDataSelector.h"
#include <KVINDRADBRun.h>

using namespace std;

ClassImp(KVINDRAGeneDataAnalyser)


KVINDRAGeneDataAnalyser::KVINDRAGeneDataAnalyser()
{
   //Default constructor
}

KVINDRAGeneDataAnalyser::~KVINDRAGeneDataAnalyser()
{
   //Destructor
}

//_________________________________________________________________

void KVINDRAGeneDataAnalyser::SubmitTask()
{
   //Run the interactive analysis

   //make the chosen dataset the active dataset ( = gDataSet; note this also opens database
   //and positions gDataBase & gIndraDB).
   GetDataSet()->cd();

   TChain* t = new TChain("GeneData");
   t->SetDirectory(0);//we handle delete

   GetRunList().Begin();
   Int_t run;
   while (!GetRunList().End()) {

      run = GetRunList().Next();
      cout << "Adding file " << gDataSet->GetFullPathToRunfile(GetDataType(),
            run);
      cout << " to the TChain." << endl;
      t->Add(gDataSet->GetFullPathToRunfile(GetDataType(), run), -1);
   }

   KVINDRAGeneDataSelector* selector = (KVINDRAGeneDataSelector*)GetInstanceOfUserClass();

   if (!selector || !selector->InheritsFrom("KVINDRAGeneDataSelector")) {
      cout << "The selector \"" << GetUserClass() << "\" is not valid." << endl;
      cout << "Process aborted." << endl;
   }
   else {
      if (GetNbEventToRead()) {
         t->Process(GetUserClass(), "", GetNbEventToRead());
      }
      else {
         t->Process(GetUserClass());
      }
   }
   if (selector) delete selector;
   delete t;
}

KVNumberList KVINDRAGeneDataAnalyser::PrintAvailableRuns(KVString& datatype)
{
   //Prints list of available runs, sorted according to multiplicity
   //trigger, for selected dataset, data type/analysis task, and system
   //Returns list containing all run numbers

   KVNumberList all_runs =
      GetDataSet()->GetRunList(datatype.Data(), GetSystem());
   KVINDRADBRun* dbrun;

   //first read list and find what triggers are available
   vector<int> triggers;
   all_runs.Begin();
   while (!all_runs.End()) {
      dbrun = (KVINDRADBRun*)GetDataSet()->GetDataBase()->GetDBRun(all_runs.Next());
      if (triggers.size() == 0
            || std::find(triggers.begin(), triggers.end(), dbrun->GetTrigger()) != triggers.end()) {
         triggers.push_back(dbrun->GetTrigger());
      }
   }
   //sort triggers in ascending order
   std::sort(triggers.begin(), triggers.end());


   for (std::vector<int>::iterator it = triggers.begin(); it != triggers.end(); ++it) {
      cout << " ---> Trigger M>" << *it << endl;
      all_runs.Begin();
      while (!all_runs.End()) {
         dbrun = (KVINDRADBRun*)GetDataSet()->GetDataBase()->GetDBRun(all_runs.Next());
         if (dbrun->GetTrigger() == *it) {
            cout << "    " << Form("%4d", dbrun->GetNumber());
            cout << Form("\t(%9llu events)", dbrun->GetEvents());
            cout << "\t[File written: " << dbrun->GetDatime().
                 AsString() << "]";
            if (dbrun->GetComments())
               cout << "\t" << dbrun->GetComments();
            cout << endl;
         }
      }
      cout << endl;
   }
   return all_runs;
}
