//Created by KVClassFactory on Thu Jun 21 17:01:26 2012
//Author: John Frankland,,,

#include "KVEventFiltering.h"
#include "KVMultiDetArray.h"
#include "KVDataBase.h"
#include "KV2Body.h"
#include "KVDBSystem.h"
#include "KVDBRun.h"
#include "KVDataSet.h"
#include "KVDataSetManager.h"
#include "KVDataRepositoryManager.h"
#include "KVGeoNavigator.h"

ClassImp(KVEventFiltering)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVEventFiltering</h2>
<h4>Filter simulated events with multidetector response</h4>
<!-- */
// --> END_HTML
// Use this KVEventSelector on simulated data TTrees containing a branch with KVSimEvent-derived objects:
//
//    my_tree->Process("KVEventFiltering", "[options]");
//
// where "[options]" is the list of options in the form "BranchName=toto,Dataset=titi,System=tata, ...".
//
// The following options MUST be given:
//
//    SimFileName: name of file containing simulated events
//    SimTitle: description of simulation
//    BranchName: name of branch containing simulated events
//    Dataset:    name of experimental dataset (defines multidetector array to use etc.)
//    System:     name of experimental dataset system (defines collision kinematics etc.)
//    Geometry:   type of geometry, either 'KV' (KaliVeda geometry) or 'ROOT' (ROOT TGeometry package)
//    Filter:     type of filter, either 'Geo' (geometric filter), 'GeoThresh' (geometry + detector thresholds),
//                or 'Full' (full simulation of detector response, including experimental identification
//                and calibration routines)
//    OutputDir:  directory path in which to write filtered data file
//    Kinematics:  kinematical frame for simulation, either "cm" or "lab". if "cm", we use the c.m. velocity
//                        of the selected System to transform events into the detector (laboratory) frame.
//                         if "lab" no transformation is performed: simulated events are already in laboratory frame.
//
// The following is an optional option:
//
//    Run:        run number to use for detector status, setup, parameters, etc.
//                if not given, first run of the given experimental system is used.
//
// The filtered data will be written in the directory given as option "OutputDir".
// The filename is built up from the original simulation filename and the values
// of various options:
//
//       [simfile]_geo=[geometry]_filt=[filter-type]_[dataset]_[system]_run=[run-number].root
//
// The data will be stored in a TTree with name 'ReconstructedEvents', in a branch with name
// 'ReconEvent'. The class used for reconstructed events depends on the dataset,
// it is given by KVDataSet::GetReconstructedEventClassName().
////////////////////////////////////////////////////////////////////////////////

KVEventFiltering::KVEventFiltering()
{
   // Default constructor
   fTransformKinematics = kTRUE;
   fNewFrame = "";
}

//________________________________________________________________

KVEventFiltering::KVEventFiltering(const KVEventFiltering& obj)  : KVEventSelector()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   fTransformKinematics = kTRUE;
   fNewFrame = "";
   obj.Copy(*this);
}

KVEventFiltering::~KVEventFiltering()
{
   // Destructor
}

//________________________________________________________________

void KVEventFiltering::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVEventFiltering::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVEventSelector::Copy(obj);
   //KVEventFiltering& CastedObj = (KVEventFiltering&)obj;
}


Bool_t KVEventFiltering::Analysis()
{
   // Event-by-event filtering of simulated data.
   // If needed (fTransformKinematics = kTRUE), kinematics of event are transformed
   // to laboratory frame using C.M. velocity calculated in InitAnalysis().
   // Detection of particles in event is simulated with KVMultiDetArray::DetectEvent,
   // then the reconstructed detected event is treated by the same identification and calibration
   // procedures as for experimental data.

   if (fTransformKinematics) {
      if (fNewFrame = "proj")   GetEvent()->SetFrame("lab", fProjVelocity);
      else                    GetEvent()->SetFrame("lab", fCMVelocity);
      gMultiDetArray->DetectEvent(GetEvent(), fReconEvent, "lab");
   } else {
      gMultiDetArray->DetectEvent(GetEvent(), fReconEvent);
   }
   fReconEvent->SetNumber(fEVN++);
   fTree->Fill();

   /*    if (!(fEventsRead % fEventsReadInterval) && fEventsRead) {
         memory_check.Check();
      }
    */
   return kTRUE;
}

void KVEventFiltering::EndAnalysis()
{
   // Write file containing filtered data to disk.
   //fFile->Write();
   //delete fFile;
}

void KVEventFiltering::EndRun()
{
}

void KVEventFiltering::InitAnalysis()
{
   // Select required dataset for filtering (option "Dataset")
   // Build the associated multidetector geometry.
   // Calculate C.M. velocity associated with required experimental collision
   // kinematics (option "System").
   //
   // Set the parameters of the detectors according to the required run
   // if given (option "Run"), or the first run of the given system otherwise.
   // If ROOT/TGeo geometry is required (option "Geometry"="ROOT"),
   // build the TGeometry representation of the detector array.
   //
   // Open file for filtered data (see KVEventFiltering::OpenOutputFile), which will
   // be stored in a TTree with name 'ReconstructedEvents', in a branch with name
   // 'ReconEvent'. The class used for reconstructed events depends on the dataset,
   // it is given by KVDataSet::GetReconstructedEventClassName().

   TString dataset = GetOpt("Dataset").Data();
   if (!gDataSetManager) {
      new KVDataRepositoryManager;
      gDataRepositoryManager->Init();
   }
   gDataSetManager->GetDataSet(dataset)->cd();

   TString system = GetOpt("System").Data();
   KVDBSystem* sys = (gDataBase ? (KVDBSystem*)gDataBase->GetTable("Systems")->GetRecord(system) : 0);
   KV2Body* tb = 0;

   Bool_t justcreated=kFALSE;
	if (sys) tb =  sys->GetKinematics();
   else if (system) {
      tb = new KV2Body(system);
      tb->CalculateKinematics();
   	justcreated = kTRUE;
	}

   fCMVelocity = (tb ? tb->GetCMVelocity() : TVector3(0, 0, 0));
   fCMVelocity *= -1.0;

	fProjVelocity = (tb ? tb->GetNucleus(1)->GetVelocity() : TVector3(0, 0, 0));
   fProjVelocity *= -1.0;
	
	if (justcreated)
		delete tb;
	
	Int_t run = 0;
   if (IsOptGiven("Run")) run = GetOpt("Run").Atoi();
   if (!run && sys) run = ((KVDBRun*)sys->GetRuns()->First())->GetNumber();

   KVMultiDetArray::MakeMultiDetector(dataset, run);
   gMultiDetArray->SetSimMode();

   TString geo = GetOpt("Geometry").Data();
   if (geo == "ROOT") {
      gMultiDetArray->SetROOTGeometry(kTRUE);
      Info("InitAnalysis", "Filtering with ROOT geometry");
      Info("InitAnalysis", "Navigator detector name format = %s", gMultiDetArray->GetNavigator()->GetDetectorNameFormat());
   } else {
      gMultiDetArray->SetROOTGeometry(kFALSE);
      Info("InitAnalysis", "Filtering with KaliVeda geometry");
   }

   TString filt = GetOpt("Filter").Data();
   if (filt == "Geo") {
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_Geo);
      Info("InitAnalysis", "Geometric filter");
   } else if (filt == "GeoThresh") {
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_GeoThresh);
      Info("InitAnalysis", "Geometry + thresholds filter");
   } else if (filt == "Full") {
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_Full);
      Info("InitAnalysis", "Full simulation of detector response & calibration");
   }
   TString kine = GetOpt("Kinematics").Data();
   if (kine == "lab") {
      fTransformKinematics = kFALSE;
      Info("InitAnalysis", "Simulation is in laboratory/detector frame");
   } else {
      fNewFrame = kine;
      Info("InitAnalysis", "Simulation will be transformed to laboratory/detector frame");
   }



   OpenOutputFile(sys, run);
   if (sys) fTree = new TTree("ReconstructedEvents", Form("%s filtered with %s (%s)", GetOpt("SimTitle").Data() , gMultiDetArray->GetTitle(), sys->GetName()));
   else fTree = new TTree("ReconstructedEvents", Form("%s filtered with %s", GetOpt("SimTitle").Data() , gMultiDetArray->GetTitle()));

   TString reconevclass = gDataSet->GetReconstructedEventClassName();
   fReconEvent = (KVReconstructedEvent*)TClass::GetClass(reconevclass)->New();
   KVEvent::MakeEventBranch(fTree, "ReconEvent", reconevclass, &fReconEvent);

   AddTree(fTree);
}

void KVEventFiltering::InitRun()
{
//   memory_check.SetInitStatistics();
   fEVN = 0;
}

void KVEventFiltering::OpenOutputFile(KVDBSystem* S, Int_t run)
{
   // Open ROOT file for new filtered events TTree.
   // The file will be written in the directory given as option "OutputDir".
   // The filename is built up from the original simulation filename and the values
   // of various options:
   //
   //       [simfile]_geo=[geometry]_filt=[filter-type]_[dataset]_[system]_run=[run-number].root
   //
   // In addition, informations on the filtered data are stored in the file as
   // TNamed objects. These can be read by KVSimDir::AnalyseFile:
   //
   // KEY: TNamed System;1 title=[full system name]
   // KEY: TNamed Dataset;1   title=[dataset name]
   // KEY: TNamed Run;1 title=[run-number]
   // KEY: TNamed Geometry;1  title=[geometry-type]
   // KEY: TNamed Filter;1 title=[filter-type]
   // KEY: TNamed Origin;1 title=[name of simulation file]
   //
   TString basefile = GetOpt("SimFileName");
   basefile.Remove(basefile.Index(".root"), 5);
   TString outfile = basefile + "_geo=";
   outfile += GetOpt("Geometry");
   outfile += "_filt=";
   outfile += GetOpt("Filter");
   outfile += "_";
   outfile += gDataSet->GetName();

   if (S) {
      outfile += "_";
      outfile += S->GetBatchName();
      outfile += "_run=";
      outfile += Form("%d", run);
   } else if (GetOpt("System")) {
      TString tmp = GetOpt("System");
      tmp.ReplaceAll(" ", "");
      tmp.ReplaceAll("/", "");
      tmp.ReplaceAll("@", "");
      tmp.ReplaceAll("MeV", "");
      tmp.ReplaceAll("A", "");
      tmp.ReplaceAll("+", "");
      outfile += "_";
      outfile += tmp.Data();
      outfile += "_run=0";
   }
   outfile += ".root";

   TString fullpath;
   AssignAndDelete(fullpath, gSystem->ConcatFileName(GetOpt("OutputDir").Data(), outfile.Data()));

   //fFile = new TFile(fullpath,"recreate");
   CreateTreeFile(fullpath);

   TDirectory* curdir = gDirectory;
   writeFile->cd();
   if (S) {
      TNamed* system = new TNamed("System", S->GetName());
      system->Write();
   } else if (GetOpt("System"))(new TNamed("System", GetOpt("System").Data()))->Write();

   (new TNamed("Dataset", gDataSet->GetName()))->Write();
   if (S)(new TNamed("Run", Form("%d", run)))->Write();
   if (gMultiDetArray->IsROOTGeometry()) {
      (new TNamed("Geometry", "ROOT"))->Write();
   } else {
      (new TNamed("Geometry", "KV"))->Write();
   }
   (new TNamed("Filter", GetOpt("Filter").Data()))->Write();
   (new TNamed("Origin", (basefile + ".root").Data()))->Write();
   curdir->cd();
}
