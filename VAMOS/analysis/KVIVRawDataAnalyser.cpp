/*
$Id: KVIVRawDataAnalyser.cpp,v 1.1 2009/01/14 15:59:49 franklan Exp $
$Revision: 1.1 $
$Date: 2009/01/14 15:59:49 $
*/

//Created by KVClassFactory on Thu Jan  8 11:46:27 2009
//Author: John Frankland,,,

#include "KVIVRawDataAnalyser.h"
#include "KVClassFactory.h"
#include "KVGANILDataReader.h"
#include "GTGanilData.h"

ClassImp(KVIVRawDataAnalyser)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIVRawDataAnalyser</h2>
<h4>Analysis of raw INDRA-VAMOS data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIVRawDataAnalyser::KVIVRawDataAnalyser()
{
   // Default constructor
   // Create dummy TTree for VAMOS Parameters class
   fBidonTree = new TTree("Bidon", "Dummy TTree for VAMOS Parameters class");
   fBidonTree->SetDirectory(0); // do not associate to currently open file, we do not want to write it
}

KVIVRawDataAnalyser::~KVIVRawDataAnalyser()
{
   // Destructor
   // Delete dummy TTree
   delete fBidonTree;
}

void KVIVRawDataAnalyser::postInitRun()
{
   KVINDRARawDataAnalyser::postInitRun(); // initialise event counters
   ((KVGANILDataReader*)fRunFile)->GetGanTapeInterface()->SetUserTree(fBidonTree);
}

//_______________________________________________________________________//

void KVIVRawDataAnalyser::Make(const Char_t* kvsname)
{
   //Automatic generation of derived class for raw data analysis

   KVClassFactory cf(kvsname, "User raw data analysis class", "KVIVRawDataAnalyser");
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
   body += "   //  Int_t fRunNumber contains current run number";
   cf.AddMethodBody("InitRun", body);
   //Analysis
   body = "   //Analysis method called for each event\n";
   body += "   //  Long64_t fEventNumber contains current event number\n";
   body += "   //  KVINDRATriggerInfo* fTrig contains informations on INDRA trigger for event\n";
   body += "   //  KVDetectorEvent* fDetEv gives list of hit groups for current event\n";
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
