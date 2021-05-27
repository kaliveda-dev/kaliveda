//Created by KVClassFactory on Thu Jun 21 17:01:26 2012
//Author: John Frankland,,,

#include "KVEventFiltering.h"
#include "KVMultiDetArray.h"
#include "KV2Body.h"
#include "KVDBSystem.h"
#include "KVDBRun.h"
#include "KVDataSet.h"
#include "KVDataSetManager.h"
#include "KVGeoNavigator.h"

ClassImp(KVEventFiltering)



KVEventFiltering::KVEventFiltering()
{
   // Default constructor
   fTransformKinematics = kTRUE;
   fNewFrame = "";
   fRotate = kTRUE;
#ifdef WITH_GEMINI
   fGemini = kFALSE;
   fGemDecayPerEvent = 1;
#endif
#ifdef DEBUG_FILTER
   memory_check = KVClassMonitor::GetInstance();
   SetEventsReadInterval(100);
#endif
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
#ifdef DEBUG_FILTER
   delete memory_check;
#endif
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
   KVEventFiltering& CastedObj = (KVEventFiltering&)obj;
   CastedObj.fRotate = fRotate;
#ifdef WITH_GEMINI
   CastedObj.fGemini = fGemini;
   CastedObj.fGemDecayPerEvent = fGemDecayPerEvent;
#endif
}

void KVEventFiltering::RandomRotation(KVEvent* to_rotate, const TString& frame_name) const
{
   // do random phi rotation around z-axis
   // if frame_name is given, apply rotation to that frame
   //
   // store phi rotation angle [radians] in event parameter "RANDOM_PHI"
   TRotation r;
   Double_t phi = gRandom->Uniform(TMath::TwoPi());
   r.RotateZ(phi);
   if (frame_name != "") to_rotate->SetFrame("rotated_frame", frame_name, r);
   else to_rotate->SetFrame("rotated_frame", r);
   to_rotate->SetParameter("RANDOM_PHI", phi);
}

Bool_t KVEventFiltering::Analysis()
{
   // Event-by-event filtering of simulated data.
   // If needed (fTransformKinematics = kTRUE), kinematics of event are transformed
   // to laboratory frame using C.M. velocity calculated in InitAnalysis().
   // Detection of particles in event is simulated with KVMultiDetArray::DetectEvent,
   // then the reconstructed detected event is treated by the same identification and calibration
   // procedures as for experimental data.

   // few event counter print at the beginning to be sure the process started properly
   // because in case GEMINI decay is used, it sometimes stops (randomly) after few events
   if ((fEVN <= 10)) Info("Analysis", "%d events processed", (int)fEVN);
   else if ((fEVN <= 1000) && !(fEVN % 100)) Info("Analysis", "%d events processed", (int)fEVN);

#ifdef DEBUG_FILTER
   if (fEVN == 2500) memory_check->SetInitStatistics();
   else if (fEVN > 4995) memory_check->CompareToInit();
#endif
   KVEvent* to_be_detected = GetEvent();
#ifdef WITH_GEMINI
   Int_t iterations = 1;
   if (fGemini) iterations = fGemDecayPerEvent;
#endif
   if (to_be_detected->GetNumber()) to_be_detected->SetParameter("SIMEVENT_NUMBER", (int)to_be_detected->GetNumber());
   to_be_detected->SetParameter("SIMEVENT_TREE_ENTRY", (int)fTreeEntry);
#ifdef WITH_GEMINI
   do {
      if (fGemini) {
         try {
            GEM.DecayEvent((KVSimEvent*)GetEvent(), &fGemEvent, fGemAddRotEner);
         }
         catch (...) {
            continue;
         }
         //Copy any parameters associated with simulated event into the Gemini-decayed event
         GetEvent()->GetParameters()->Copy(*(fGemEvent.GetParameters()));
         to_be_detected = &fGemEvent;
      }
#endif
      if (fTransformKinematics) {
         if (fNewFrame == "proj")   to_be_detected->SetFrame("lab", fProjVelocity);
         else                    to_be_detected->SetFrame("lab", fCMVelocity);
         if (fRotate) {
            RandomRotation(to_be_detected, "lab");
            gMultiDetArray->DetectEvent(to_be_detected, fReconEvent, "rotated_frame");
         }
         else {
            gMultiDetArray->DetectEvent(to_be_detected, fReconEvent, "lab");
         }
      }
      else {
         if (fRotate) {
            RandomRotation(to_be_detected);
            gMultiDetArray->DetectEvent(to_be_detected, fReconEvent, "rotated_frame");
         }
         else {
            gMultiDetArray->DetectEvent(to_be_detected, fReconEvent);
         }
      }
      fReconEvent->SetNumber(fEVN++);
      fReconEvent->SetFrameName("lab");
      fTree->Fill();
#ifdef WITH_GEMINI
   }
   while (fGemini && --iterations);
#endif


   return kTRUE;
}

void KVEventFiltering::EndAnalysis()
{
   gEnv->SetValue(Form("%s.HasCalibIdentInfos", GetOpt("Dataset").Data()), fIdCalMode);
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

   if (fDisableCreateTreeFile) return; //when running with PROOF, only execute on workers

   TString dataset = GetOpt("Dataset").Data();
   if (!gDataSetManager) {
      gDataSetManager = new KVDataSetManager;
      gDataSetManager->Init();
   }
   gDataSetManager->GetDataSet(dataset)->cd();

   TString system = GetOpt("System").Data();
   KVDBSystem* sys = (gExpDB ? gExpDB->GetSystem(system) : nullptr);
   KV2Body* tb = 0;

   Bool_t justcreated = kFALSE;
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
   if (IsOptGiven("Run")) {
      run = GetOpt("Run").Atoi();
      Info("InitAnalysis", "Run given in options = %d", run);
   }
   if (!run) {
      if (sys) {
         run = ((KVDBRun*)sys->GetRuns()->First())->GetNumber();
         Info("InitAnalysis", "Using first run for system = %d", run);
      }
      else {
         Info("InitAnalysis", "No run information");
         run = -1;
      }
   }

   KVMultiDetArray::MakeMultiDetector(dataset, run);
   if (run == -1) gMultiDetArray->InitializeIDTelescopes();
   gMultiDetArray->SetSimMode();

   fIdCalMode = gEnv->GetValue(Form("%s.HasCalibIdentInfos", dataset.Data()), "no");

   TString geo = GetOpt("Geometry").Data();
   if (geo == "ROOT") {
      gMultiDetArray->CheckROOTGeometry();
      Info("InitAnalysis", "Filtering with ROOT geometry");
      Info("InitAnalysis", "Navigator detector name format = %s", gMultiDetArray->GetNavigator()->GetDetectorNameFormat());
   }
   else {
      gMultiDetArray->SetROOTGeometry(kFALSE);
      Info("InitAnalysis", "Filtering with KaliVeda geometry");
   }

   TString filt = GetOpt("Filter").Data();
   if (filt == "Geo") {
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_Geo);
      Info("InitAnalysis", "Geometric filter");
   }
   else if (filt == "GeoThresh") {
      gEnv->SetValue(Form("%s.HasCalibIdentInfos", dataset.Data()), "no");
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_GeoThresh);
      Info("InitAnalysis", "Geometry + thresholds filter");
   }
   else if (filt == "Full") {
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_GeoThresh);
      Info("InitAnalysis", "Geometry + thresholds filter + implemented identifications and calibrations.");
//      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_Full);
//      Info("InitAnalysis", "Full simulation of detector response & calibration");
   }
   TString kine = GetOpt("Kinematics").Data();
   if (kine == "lab") {
      fTransformKinematics = kFALSE;
      Info("InitAnalysis", "Simulation is in laboratory/detector frame");
   }
   else {
      fNewFrame = kine;
      Info("InitAnalysis", "Simulation will be transformed to laboratory/detector frame");
   }

   if (IsOptGiven("PhiRot")) {
      if (GetOpt("PhiRot") == "no") fRotate = kFALSE;
   }
   if (fRotate) Info("InitAnalysis", "Random phi rotation around beam axis performed for each event");
#ifdef WITH_GEMINI
   if (IsOptGiven("Gemini")) {
      if (GetOpt("Gemini") == "yes") fGemini = kTRUE;
      if (IsOptGiven("GemDecayPerEvent")) fGemDecayPerEvent = GetOpt("GemDecayPerEvent").Atoi();
      else fGemDecayPerEvent = 1;
      if (IsOptGiven("GemAddRotEner")) {
         if (GetOpt("GemAddRotEner") == "yes") fGemAddRotEner = kTRUE;
         else fGemAddRotEner = kFALSE;
      }
   }
   if (fGemini) Info("InitAnalysis", "Statistical decay with Gemini++ for each event before detection");
   if (fGemDecayPerEvent > 1) Info("InitAnalysis", "  -- %d decays per primary event", fGemDecayPerEvent);
   if (fGemAddRotEner) Info("InitAnalysis", "  -- Rotational energy will be added to excitation energy");
#endif
   if (!gDataSet->HasCalibIdentInfos()) {
      // for old data without implementation of calibration/identification
      // we set status of identifications according to experimental informations in IdentificationBilan.dat
      // this "turns off" any telescopes which were not working during the experimental run

      // beforehand we have to "turn on" all identification telescopes by initializing them
      // as this will not have been done yet
      gMultiDetArray->InitializeIDTelescopes();
      // this will also set informations on (simulated) mass identification capabilities for
      // certain telescopes

      TString fullpath;
      if (sys && KVBase::SearchKVFile("IdentificationBilan.dat", fullpath, gDataSet->GetName())) {
         Info("InitAnalysis", "Setting identification bilan...");
         TString sysname = sys->GetBatchName();
         KVIDTelescope::OpenIdentificationBilan(fullpath);
         TIter next(gMultiDetArray->GetListOfIDTelescopes());
         KVIDTelescope* idt;
         while ((idt = (KVIDTelescope*)next())) {
            idt->CheckIdentificationBilan(sysname);
         }
      }
   }
   gMultiDetArray->PrintStatusOfIDTelescopes();

   OpenOutputFile(sys, run);
   if (sys) fTree = new TTree("ReconstructedEvents", Form("%s filtered with %s (%s)", GetOpt("SimTitle").Data(), gMultiDetArray->GetTitle(), sys->GetName()));
   else fTree = new TTree("ReconstructedEvents", Form("%s filtered with %s", GetOpt("SimTitle").Data(), gMultiDetArray->GetTitle()));

   TString reconevclass = gDataSet->GetReconstructedEventClassName();
   fReconEvent = (KVReconstructedEvent*)TClass::GetClass(reconevclass)->New();
   KVEvent::MakeEventBranch(fTree, "ReconEvent", fReconEvent);

   AddTree(fTree);
}

void KVEventFiltering::InitRun()
{
   fEVN = 0;
#ifdef DEBUG_FILTER
   SetEventsReadInterval(100);
#endif
}

void KVEventFiltering::OpenOutputFile(KVDBSystem* S, Int_t run)
{
   // Open ROOT file for new filtered events TTree.
   // The file will be written in the directory given as option "OutputDir".
   // The filename is built up from the original simulation filename and the values
   // of various options:
   //
   //       [simfile]_[Gemini]_geo=[geometry]_filt=[filter-type]_[dataset]_[system]_run=[run-number].root
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
   // KEY: TNamed RandomPhi;1 title=[yes/no, random rotation about beam axis]
   // KEY: TNamed Gemini++;1 title=[yes/no, Gemini++ decay before detection]
   // KEY: TNamed GemDecayPerEvent;1 title=[number of Gemini++ decays per primary event]
   // KEY: TNamed GemAddRotEner;1 title=[Enable or not the addition of the rotational energy to the excitation energy]
   //
   TString basefile = GetOpt("SimFileName");
   basefile.Remove(basefile.Index(".root"), 5);
   TString outfile = basefile;
#ifdef WITH_GEMINI
   if (fGemini) {
      outfile += "_Gemini";
      if (fGemDecayPerEvent > 1) outfile += fGemDecayPerEvent;
      if (fGemAddRotEner) outfile += "AddedRotEner";
   }
#endif
   outfile += "_geo=";
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
   }
   else if (GetOpt("System")) {
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
   }
   else if (GetOpt("System"))(new TNamed("System", GetOpt("System").Data()))->Write();

   (new TNamed("Dataset", gDataSet->GetName()))->Write();
   if (S)(new TNamed("Run", Form("%d", run)))->Write();
   if (gMultiDetArray->IsROOTGeometry()) {
      (new TNamed("Geometry", "ROOT"))->Write();
   }
   else {
      (new TNamed("Geometry", "KV"))->Write();
   }
   (new TNamed("Filter", GetOpt("Filter").Data()))->Write();
   (new TNamed("Origin", (basefile + ".root").Data()))->Write();
   (new TNamed("RandomPhi", (fRotate ? "yes" : "no")))->Write();
#ifdef WITH_GEMINI
   (new TNamed("Gemini++", (fGemini ? "yes" : "no")))->Write();
   (new TNamed("GemDecayPerEvent", Form("%d", fGemDecayPerEvent)))->Write();
   (new TNamed("GemAddRotEner", (fGemAddRotEner ? "yes" : "no")))->Write();
#endif
   curdir->cd();
}
