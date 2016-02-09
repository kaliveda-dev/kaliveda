/*
$Id: KVINDRARawDataAnalyser.cpp,v 1.4 2009/01/14 16:13:49 franklan Exp $
$Revision: 1.4 $
$Date: 2009/01/14 16:13:49 $
*/

//Created by KVClassFactory on Fri May  4 15:26:02 2007
//Author: franklan

#include "KVINDRARawDataAnalyser.h"
#include "KVString.h"
#include "KVINDRA.h"
#include "KVDataSet.h"
#include "KVClassFactory.h"
#include "KVINDRADB.h"
#include <KVINDRADBRun.h>

using namespace std;

ClassImp(KVINDRARawDataAnalyser)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRARawDataAnalyser</h2>
<h4>Analysis of raw INDRA data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVINDRARawDataAnalyser::KVINDRARawDataAnalyser()
{
   //Default constructor
}

KVINDRARawDataAnalyser::~KVINDRARawDataAnalyser()
{
   //Destructor
}

void KVINDRARawDataAnalyser::postInitRun()
{
   // Initialise counters for INDRA events, INDRA-gene events, and others

   INDRA_events = gene_events = other_events = 0;
}

void KVINDRARawDataAnalyser::preAnalysis()
{
   // Count numbers of INDRA events, INDRA-gene events, and others

   if (gIndra->GetTriggerInfo()->IsINDRAEvent()) {
      INDRA_events++;
      if (gIndra->GetTriggerInfo()->IsGene()) gene_events++;
   } else {
      other_events++;
   }
}

void KVINDRARawDataAnalyser::preEndRun()
{
   // Print numbers of INDRA events, INDRA-gene events, and others

   cout << " +++ INDRA events : " << INDRA_events << endl;
   cout << " +++ INDRA gene events : " << gene_events << endl;
   cout << " +++ Other (non-INDRA) events : " << other_events << endl << endl;
}
//_______________________________________________________________________//

void KVINDRARawDataAnalyser::Make(const Char_t* kvsname)
{
   //Automatic generation of derived class for raw data analysis

   KVClassFactory cf(kvsname, "User INDRA raw data analysis class", "KVINDRARawDataAnalyser");
   cf.AddMethod("InitAnalysis", "void");
   cf.AddMethod("InitRun", "void");
   cf.AddMethod("Analysis", "Bool_t");
   cf.AddMethod("EndRun", "void");
   cf.AddMethod("EndAnalysis", "void");
   KVString body;
   //initanalysis
   body = "   //Initialisation of e.g. histograms, performed once at beginning of analysis";
   cf.AddMethodBody("InitAnalysis", body);
   //initrun
   body = "   //Initialisation performed at beginning of each run\n";
   body += "   //  GetRunNumber() returns current run number";
   cf.AddMethodBody("InitRun", body);
   //Analysis
   body = "   //Analysis method called for each event\n";
   body += "   //  GetEventNumber() returns current event number\n";
   body += "   //  gIndra->GetTriggerInfo() returns pointer to object\n";
   body += "   //     (KVINDRATriggerInfo) with informations on INDRA trigger for event\n";
   body += "   //  GetDetectorEvent() gives pointer to list of hit groups (KVDetectorEvent) for current event\n";
   body += "   //  Processing will stop if this method returns kFALSE\n";
   body += "   return kTRUE;";
   cf.AddMethodBody("Analysis", body);
   //endrunù
   body = "   //Method called at end of each run";
   cf.AddMethodBody("EndRun", body);
   //endanalysis
   body = "   //Method called at end of analysis: save/display histograms etc.";
   cf.AddMethodBody("EndAnalysis", body);
   cf.GenerateCode();
}

KVNumberList KVINDRARawDataAnalyser::PrintAvailableRuns(KVString& datatype)
{
   //Prints list of available runs, sorted according to multiplicity
   //trigger, for selected dataset, data type/analysis task, and system
   //Returns list containing all run numbers

   KVNumberList all_runs =
      fDataSet->GetRunList(datatype.Data(), fSystem);
   KVINDRADBRun* dbrun;

   //first read list and find what triggers are available
   vector<int> triggers;
   all_runs.Begin();
   while (!all_runs.End()) {
      dbrun = (KVINDRADBRun*)fDataSet->GetDataBase()->GetTable("Runs")->GetRecord(all_runs.Next());
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
         dbrun = (KVINDRADBRun*)fDataSet->GetDataBase()->GetTable("Runs")->GetRecord(all_runs.Next());
         if (dbrun->GetTrigger() == *it) {
            cout << "    " << Form("%4d", dbrun->GetNumber());
            cout << Form("\t(%7d events)", dbrun->GetEvents());
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

void KVINDRARawDataAnalyser::CalculateTotalEventsToRead()
{
   //loop over runs and calculate total events
   TotalEntriesToRead = 0;
   GetRunList().Begin();
   while (!GetRunList().End()) {
      Int_t r = GetRunList().Next();
      TotalEntriesToRead += gIndraDB->GetRun(r)->GetEvents();
   }
}
