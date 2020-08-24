//Created by KVClassFactory on Tue Feb 14 11:13:53 2017
//Author: John Frankland,,,

#include "KVSimDirAnalyser.h"
#include "KVDataAnalysisTask.h"
#include <KVSimFile.h>
#include <KVSimDir.h>
#include <KVClassFactory.h>
#include <TStopwatch.h>
#include "TSystem.h"

ClassImp(KVSimDirAnalyser)

KVSimDirAnalyser::KVSimDirAnalyser()
   : KVDataAnalyser(), fListOfSimFiles(nullptr), fListOfAuxFiles(nullptr), fAnalysisChain(nullptr), fSimDir(nullptr), fCopyFilesToWorkDir(false)
{
   // Default constructor
}

//____________________________________________________________________________//

void KVSimDirAnalyser::DeleteSimFilesListIfOurs()
{
   if (fListOfSimFiles && TString(fListOfSimFiles->GetName()) == "toDelete")
      SafeDelete(fListOfSimFiles);
}

void KVSimDirAnalyser::DeleteAuxFilesListIfOurs()
{
   if (fListOfAuxFiles && TString(fListOfAuxFiles->GetName()) == "toDelete")
      SafeDelete(fListOfAuxFiles);
}

KVSimDirAnalyser::~KVSimDirAnalyser()
{
   // Destructor
   SafeDelete(fSimDir);
   DeleteSimFilesListIfOurs();
   DeleteAuxFilesListIfOurs();
}

void KVSimDirAnalyser::SubmitTask()
{
   // Set up and run the analysis

   BuildChain();

   Bool_t read_all_events = (GetNbEventToRead() == 0);
   Long64_t nevents = (!read_all_events ? GetNbEventToRead() : fAnalysisChain->GetEntries());
   Long64_t update_interval = (nevents > 10 ? nevents / 10 : 1);
   TString results_file_name;
   KVSimFile* first_file = (KVSimFile*)fListOfSimFiles->First();
   results_file_name.Form("%s_%s", GetUserClass(), first_file->GetName());
   TString options;
   options.Form("SimFileName=%s,SimTitle=%s,OutputDir=%s,EventsReadInterval=%lld,BranchName=%s,CombinedOutputFile=%s,SimulationInfos=%s",
                first_file->GetName(), fAnalysisChain->GetTitle(), (IsCopyFilesToWorkDir() ? "." : first_file->GetSimDir()->GetDirectory()),
                update_interval, first_file->GetBranchName(),
                results_file_name.Data(), first_file->GetTitle());
   if (first_file->IsFiltered()) options += Form(",DataSet=%s,System=%s,Run=%d",
            first_file->GetDataSet(), first_file->GetSystem(), first_file->GetRun());
   if (fListOfAuxFiles) {
      // Set up list of auxiliary files
      KVString auxfiles;
      TIter ifi(fListOfAuxFiles);
      KVSimFile* auxf;
      while ((auxf = (KVSimFile*)ifi())) {
         if (auxfiles != "") auxfiles += "|";
         auxfiles += auxf->GetName();
      }
      options += ",AuxFiles=";
      options += auxfiles;
      options += ",AuxDir=";
      KVSimFile* ffaux = (KVSimFile*)fListOfAuxFiles->First();
      options += ffaux->GetSimDir()->GetDirectory();
      options += ",AuxTreeName=";
      options += ffaux->GetTreeName();
      options += ",AuxBranchName=";
      options += ffaux->GetBranchName();
   }
   // Add any user-defined options
   if (GetUserClassOptions() != "") {
      options += ",";
      options += GetUserClassOptions();
   }

   // Check compilation of user class & run
   if (CheckIfUserClassIsValid()) {
      Info("SubmitTask", "Beginning TChain::Process...");
#ifdef WITH_CPP11
      if (GetProofMode() != KVDataAnalyser::EProofMode::None) fAnalysisChain->SetProof(kTRUE);
#else
      if (GetProofMode() != KVDataAnalyser::None) fAnalysisChain->SetProof(kTRUE);
#endif
      TString analysis_class;
      if (GetAnalysisTask()->WithUserClass()) analysis_class.Form("%s%s", GetUserClassImp().Data(), GetACliCMode());
      else analysis_class = GetUserClass();
      if (read_all_events) {
         fAnalysisChain->Process(analysis_class, options.Data());
      }
      else {
         fAnalysisChain->Process(analysis_class, options.Data(), GetNbEventToRead());
      }
   }
   delete fAnalysisChain;
   fAnalysisChain = nullptr;
}

KVString KVSimDirAnalyser::GetRootDirectoryOfDataToAnalyse() const
{
   // Returns path to data to be analysed
   if (!NeedToChooseWhatToAnalyse()) {
      KVSimFile* first_file = (KVSimFile*)fListOfSimFiles->First();
      return first_file->GetSimDir()->GetDirectory();
   }
   return "";
}

void KVSimDirAnalyser::WriteBatchEnvFile(const Char_t* jobname, Bool_t sav)
{
   //Save (in the TEnv fBatchEnv) all necessary information on analysis task which can be used to execute it later
   //(i.e. when batch processing system executes the job).
   //If save=kTRUE (default), write the information in a file whose name is given by ".jobname"
   //where 'jobname' is the name of the job as given to the batch system.

   KVDataAnalyser::WriteBatchEnvFile(jobname, kFALSE);
   KVSimFile* simF = (KVSimFile*)fListOfSimFiles->First();
   KVSimDir* simD = simF->GetSimDir();
   GetBatchInfoFile()->SetValue("SimDir", simD->GetDirectory());
   if (simF->IsFiltered()) GetBatchInfoFile()->SetValue("SimFile.Type", "filtered");
   else GetBatchInfoFile()->SetValue("SimFile.Type", "simulated");
   GetBatchInfoFile()->SetValue("SimFiles", simF->GetName());
   if (fListOfSimFiles->GetEntries() > 1) {
      TIter next(fListOfSimFiles);
      next();
      while ((simF = (KVSimFile*)next())) GetBatchInfoFile()->SetValue("+SimFiles", simF->GetName());
   }
   if (fListOfAuxFiles) {
      // auxiliary files for batch job
      GetBatchInfoFile()->SetValue("AuxFiles", fListOfAuxFiles->First()->GetName());
      if (fListOfAuxFiles->GetEntries() > 1) {
         TIter next(fListOfAuxFiles);
         next();
         while ((simF = (KVSimFile*)next())) GetBatchInfoFile()->SetValue("+AuxFiles", simF->GetName());
      }
   }
   if (sav) GetBatchInfoFile()->SaveLevel(kEnvUser);
}

Bool_t KVSimDirAnalyser::ReadBatchEnvFile(const Char_t* filename)
{
   //Read the batch env file "filename" and initialise the analysis task using the
   //informations in the file
   //Returns kTRUE if all goes well

   Bool_t ok = kFALSE;

   if (!KVDataAnalyser::ReadBatchEnvFile(filename)) return ok;

   KVString simdir = GetBatchInfoFile()->GetValue("SimDir", "");
   if (simdir == "") return ok;

   fSimDir = new KVSimDir("SIMDIR", simdir);
   fSimDir->AnalyseDirectory();

   KVString filetype = GetBatchInfoFile()->GetValue("SimFile.Type", "");
   if (filetype == "") return ok;

   KVString simfiles = GetBatchInfoFile()->GetValue("SimFiles", "");
   if (simfiles == "") return ok;

   DeleteSimFilesListIfOurs();
   fListOfSimFiles = new TList;
   fListOfSimFiles->SetName("toDelete");

   simfiles.Begin(" ");
   while (!simfiles.End()) {
      if (filetype == "simulated") fListOfSimFiles->Add(fSimDir->GetSimDataList()->FindObject(simfiles.Next()));
      else if (filetype == "filtered") fListOfSimFiles->Add(fSimDir->GetFiltDataList()->FindObject(simfiles.Next()));
   }

   // this option, if set, copies the files to be analysed to the current working directory
   SetCopyFilesToWorkDir(GetBatchInfoFile()->GetValue("SimDirAnalyser.CopyFilesToWorkingDirectory", false));

   KVString auxfiles = GetBatchInfoFile()->GetValue("AuxFiles", "");
   if (auxfiles == "") return (ok = kTRUE);

   DeleteAuxFilesListIfOurs();
   fListOfAuxFiles = new TList;
   fListOfAuxFiles->SetName("toDelete");

   auxfiles.Begin(" ");
   while (!auxfiles.End()) {
      fListOfAuxFiles->Add(fSimDir->GetSimDataList()->FindObject(auxfiles.Next()));
   }

   ok = kTRUE;

   return ok;
}

void KVSimDirAnalyser::BuildChain()
{
   // Build a TChain with all files/trees to be analysed
   //
   // If fCopyFilesToWorkDir==true, files are first copied to working directory

   TIter next(fListOfSimFiles);
   KVSimFile* file;
   while ((file = (KVSimFile*)next())) {
      if (!fAnalysisChain) {
         fAnalysisChain = new TChain(file->GetTreeName());
      }
      TString fullpath;
      AssignAndDelete(fullpath, gSystem->ConcatFileName(file->GetSimDir()->GetDirectory(), file->GetName()));
      if (IsCopyFilesToWorkDir()) {
         Info("BuildChain", "Copying file to analyse from %s to %s", fullpath.Data(), Form("./%s", gSystem->BaseName(fullpath)));
         TStopwatch timer;
         TFile::Cp(fullpath, Form("./%s", gSystem->BaseName(fullpath)), kFALSE, 1e+08);
         Info("BuildChain", "Copied file in %g seconds", timer.RealTime());
         fAnalysisChain->Add(gSystem->BaseName(fullpath));
      }
      else
         fAnalysisChain->Add(fullpath);
   }
   if (IsCopyFilesToWorkDir()) {
      // rescan the working directory to include the newly-added copies of the files to analyse
      ScanWorkingDirectory(&fWorkDirInit);
   }
}


void KVSimDirAnalyser::Make(const Char_t* kvsname)
{
   // Generate a new simulated analysis selector class

   KVClassFactory cf(kvsname, "Analysis of simulated events", "",
                     kTRUE, "SimulatedEventAnalysisTemplate");
   cf.AddImplIncludeFile("KVSimNucleus.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");

   cf.GenerateCode();
}
