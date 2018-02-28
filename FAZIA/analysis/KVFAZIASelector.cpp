//Created by KVClassFactory on Wed Jan 28 09:54:34 2015
//Author: Eric Bonnet

#include "KVFAZIASelector.h"
#include "KVReconstructedNucleus.h"
#include "KVBatchSystem.h"
#include "KVFAZIA.h"
#include "KVDataSet.h"
#include "KVDataAnalyser.h"
#include "KVFAZIAReconDataAnalyser.h"
#include "KVFAZIADetector.h"
#include "KVSignal.h"
#include "KVClassFactory.h"

#include "KVDataRepositoryManager.h"
#include "KVDataSetManager.h"
#include "KVFAZIADBRun.h"
#include "KVFAZIADB.h"
#include "KVFileReader.h"

ClassImp(KVFAZIASelector)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIASelector</h2>
<h4>selector to analyze FAZIA reconstructed data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

// This class is derived from the KaliVeda class KVSelector.
// The following members functions are called by the TTree::Process() functions:
//    InitRun():       called everytime a run starts
//    EndRun():        called everytime a run ends
//    InitAnalysis():  called at the beginning of the analysis
//                     a convenient place to create your histograms.
//    Analysis():      called for each event. In this function you
//                     fill your histograms.
//    EndAnalysis():   called at the end of a loop on the tree,
//                     a convenient place to draw/fit your histograms.
//
// Modify these methods as you wish in order to create your analysis class.
// Don't forget that for every class used in the analysis, you must put a
// line '#include' at the beginning of this file.
// E.g. if you want to access informations on the FAZIA multidetector
// through the global pointer gIndra, you must AT LEAST add the line
//   #include "KVFAZIA.h"

void KVFAZIASelector::SlaveBegin(TTree* tree)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).

   SetReadingOfRawData(kFALSE);
   gDataAnalyser->preInitAnalysis();

   //
   // ParseOptions : Manage options passed as arguments
   // done in KVEventSelector::SlaveBegin
   KVEventSelector::SlaveBegin(tree);

}

void KVFAZIASelector::Init(TTree* tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer

   Event = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(1);

   if (strcmp(GetBranchName(), "")  && fChain->GetBranch(GetBranchName())) {
      Info("Init", "Analysing data in branch : %s", GetBranchName());
      fChain->SetBranchAddress(GetBranchName(), &Event, &b_Event);
   }
   if (NeedToReadRawData())
      LinkRawData();

   //user additional branches addressing
   SetAdditionalBranchAddress();
   fEventsRead = 0;
   fOutputFile = 0;

}

Bool_t KVFAZIASelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   if (fNotifyCalled) return kTRUE; // avoid multiple calls at beginning of analysis
   fNotifyCalled = kTRUE;

   Info("Notify", "Beginning analysis of file %s (%lld events)", fChain->GetCurrentFile()->GetName(), fChain->GetTree()->GetEntries());
   gDataAnalyser->preInitRun();
   fCurrentRun = ((KVFAZIAReconDataAnalyser*)gDataAnalyser)->GetCurrentRunNumber();
   printf("KVFAZIASelector-LOG: Start run : %d\n", fCurrentRun);
   if (fAutomatic)
      CreateOutputFile();
   InitRun();  //user initialisations for run

   return kTRUE;
}

//_____________________________________
void KVFAZIASelector::LinkRawData()
{
   Info("LinkRawData", "Enable reading of raw data : %s\n", rawdatabranchname.Data());
   RawEvent = 0;
   fChain->SetBranchAddress(rawdatabranchname.Data(), &RawEvent);
}

//_____________________________________
void KVFAZIASelector::ConnectSignalsToDetectors()
{
   KVFAZIADetector* det = 0;
   KVSignal* sig = 0;

   TIter next_s(RawEvent->GetSignals());
   while ((sig = (KVSignal*)next_s())) {
      sig->DeduceFromName();
      det = (KVFAZIADetector*)gFazia->GetDetector(sig->GetDetectorName());
      if (!det)
         det = (KVFAZIADetector*)gFazia->GetDetector(KVFAZIADetector::GetNewName(sig->GetDetectorName()));
      if (det)
         det->SetSignal(sig, sig->GetType());
      else
         Warning("ConnectSignalsToDetectors", "Unknown detector %s", sig->GetDetectorName());
   }

}

//_____________________________________
void KVFAZIASelector::ParseOptions()
{
   KVEventSelector::ParseOptions();

   //check if the reading of raw data is needed
   if (IsOptGiven("ReadRawData")) SetReadingOfRawData(GetOpt("ReadRawData").Atoi());

   //Force the name of the KVEvent branch after the call to ParseOption
   SetBranchName("FAZIAReconEvent");
}

//_____________________________________
void KVFAZIASelector::Make(const Char_t* kvsname)
{
   // Automatic generation of KVSelector-derived class for KaliVeda analysis
   KVClassFactory cf(kvsname, "User analysis class", Class_Name(), kTRUE);
   cf.AddImplIncludeFile("KVReconstructedNucleus.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");
   cf.AddImplIncludeFile("KVFAZIA.h");
   cf.GenerateCode();
}

//_____________________________________
void KVFAZIASelector::SetAutomaticManagementOfOutputs(Bool_t automatic)
{
   //indicate if outputs (histograms, trees or other objects
   //has to be managed automatically by the selector
   //
   //to be active FAZIA_SELECTOR_OUTPUTS shell variable
   //has to be defined in your .cshrc type configuration file
   //the format of the output file will be :
   // FAZIA_SELECTOR_OUTPUTS/nameofthekvfaziaselector/dataset/R%d.root
   fOutputPath = "";
   if (automatic) {
      TString output_path = gSystem->Getenv("FAZIASELECTOR_OUTPUTS");
      fOutputPath.Form("%s/%s/%s", output_path.Data(), GetName(), gDataSet->GetName());
      if (output_path == "") {
         Error("CreateOutputFile", "FAZIASELECTOR_OUTPUTS shell variable has to defined first");
         fAutomatic = kFALSE;
         fOutputPath = "";
         return;
      } else if (gSystem->Exec(Form("test -d %s", fOutputPath.Data())) != 0) {
         Info("SetAutomaticManagementOfOutputs", "Creation of %s directory", fOutputPath.Data());
         if (gSystem->mkdir(fOutputPath.Data(), kTRUE) != 0) {
            Info("SetAutomaticManagementOfOutputs", "Error in the %s directory creation", fOutputPath.Data());
            fAutomatic = kFALSE;
            fOutputPath = "";
            return;
         }
      }
   }
   fAutomatic = automatic;
}
//_____________________________________
Bool_t KVFAZIASelector::CreateOutputFile()
{
   //create ROOT file where all outputs coming from
   //the selector will be stores
   //to be active FAZIA_SELECTOR_OUTPUTS shell variable
   //has to be defined in your .cshrc type configuration file
   //the format of the output file will be :
   // FAZIA_SELECTOR_OUTPUTS/nameofthekvfaziaselector/dataset/R%d.root
   //When it is created, the output file name is tmp_R%d.root
   //and renamed at the end of the run R%d.root
   //This helps to know if the entire run has been read
   //
   //
   fOutputFile = new TFile(Form("%s/tmp_R%d.root", fOutputPath.Data(), GetCurrentRunNumber()), "recreate");
   if (fOutputFile && fOutputFile->IsOpen())
      return kTRUE;
   return kFALSE;
}
//_____________________________________
Bool_t KVFAZIASelector::SaveAndCloseOutputFile()
{
   //write in and close the ROOT file created by the CreateOutputFile
   //method
   //
   if (!fOutputFile) {
      Error("SaveAndCloseOutputFile", "No file open for writing outputs");
      return kFALSE;
   }

   fOutputFile->cd();
   fOutputFile->Write();
   fOutputFile->Close();

   TString fmt1, fmt2;
   fmt1.Form("%s/tmp_R%d.root", fOutputPath.Data(), GetCurrentRunNumber());
   fmt2.Form("%s/R%d.root", fOutputPath.Data(), GetCurrentRunNumber());

   gROOT->ProcessLine(Form(".! mv %s %s", fmt1.Data(), fmt2.Data()));
   return kTRUE;
}

//_____________________________________
void KVFAZIASelector::CheckEndOfRun()
{
   // Testing whether EndRun() should be called
   if (AtEndOfRun()) {
      Info("Process", "End of file reached after %lld events", fEventsRead);
      EndRun();
      printf("KVFAZIASelector-LOG: End run : %d\n", fCurrentRun);
      if (fAutomatic)
         SaveAndCloseOutputFile();
      fNotifyCalled = kFALSE;//Notify will be called when next file is opened (in TChain)
   }

}

//_____________________________________
void KVFAZIASelector::CheckListOfWellEndedRuns(const Char_t* kvsname, const Char_t* dsname)
{
   //static method
   //List well ended files in the corresponding directory :
   //FAZIA_SELECTOR_OUTPUTS/[kvsname]/[dsname]/
   //Only available if the selector you ran
   //had the option SetAutomaticManagementOfOutputs(kTRUE)
   //
   TString output_path = gSystem->Getenv("FAZIASELECTOR_OUTPUTS");
   TString op;
   op.Form("%s/%s/%s", output_path.Data(), kvsname, dsname);
   if (output_path == "") {
      printf("Error in CheckListOfEndedRuns : FAZIASELECTOR_OUTPUTS shell variable is not defined\n");
      return;
   }
   if (gSystem->Exec(Form("test -d %s", op.Data())) != 0) {
      printf("directory %s does not exist\n", op.Data());
      return;
   }
   KVNumberList lr;
   KVString lruns = gSystem->GetFromPipe(Form("ls %s/R*.root", op.Data()));
   lruns.Begin("\n");
   Int_t run = -1;
   while (!lruns.End()) {
      KVString srun = lruns.Next();
      srun = gSystem->BaseName(srun.Data());
      Int_t rep = sscanf(srun.Data(), "R%d.root", &run);
      if (rep == 1)
         lr.Add(run);
   }
   if (lr.GetNValues() > 0)
      printf("Ended runs for the selector %s and the dataset %s are:\n%s\n", kvsname, dsname, lr.AsString());
   else
      printf("No runs ended for the selector %s and the dataset %s\n", kvsname, dsname);
}
//_____________________________________
void KVFAZIASelector::MergeListOfEndedRuns(const Char_t* kvsname, const Char_t* dsname)
{
   //static method
   //Gather well ended files in the corresponding directory :
   //FAZIA_SELECTOR_OUTPUTS/[kvsname]/[dsname]/
   //Files are merged using the hadd command
   //the -f option is set, so previous output file is deleted
   //
   //To avoid mixing, one merging is done for each system
   //
   //The output file is deduced from system name
   //Example for "48Ca + 48Ca 35.0 MeV/A" gives 48Ca48Ca35.0
   //
   //An additional TEnv file is created with the format
   // listruns_48Ca48Ca35.0.env to follow the previous example
   //It is a summary of info coming from the merging :
   //       System:                                  48Ca + 48Ca 35.0 MeV/A
   //       DataSet:                                 FAZIASYM
   //       Selector:                                CheckCsIIdentification
   //       Number:                                  34
   //       RunList:                                 14588-14621
   //
   //Only available if the selector you ran
   //had the option SetAutomaticManagementOfOutputs(kTRUE)
   //
   TString output_path = gSystem->Getenv("FAZIASELECTOR_OUTPUTS");
   TString op;
   op.Form("%s/%s/%s", output_path.Data(), kvsname, dsname);
   if (output_path == "") {
      printf("Error in CheckListOfEndedRuns : FAZIASELECTOR_OUTPUTS shell variable is not defined\n");
      return;
   }
   if (gSystem->Exec(Form("test -d %s", op.Data())) != 0) {
      printf("directory %s does not exist\n", op.Data());
      return;
   }

   if (!(gDataSet)) {
      new KVDataRepositoryManager();
      gDataRepositoryManager->Init();
      gDataSetManager->GetDataSet(dsname)->cd();
   }

   KVNumberList lall;
   TList* lsyst = new TList();
   lsyst->SetOwner(kTRUE);
   KVNumberList* lpersyst = 0;
   KVString lruns = gSystem->GetFromPipe(Form("ls %s/R*.root", op.Data()));
   lruns.Begin("\n");
   Int_t run = -1;
   while (!lruns.End()) {
      KVString srun = lruns.Next();
      srun = gSystem->BaseName(srun.Data());
      Int_t rep = sscanf(srun.Data(), "R%d.root", &run);
      if (rep == 1) {
         KVFAZIADBRun* fr =  gFaziaDB->GetRun(run);
         lall.Add(run);
         if (!(lpersyst = (KVNumberList*)lsyst->FindObject(fr->GetSystemName()))) {
            lpersyst = new KVNumberList();
            lpersyst->SetName(fr->GetSystemName());
            printf("creation de la liste associee a : %s\n", fr->GetSystemName());
            lsyst->Add(lpersyst);
         }
         lpersyst->Add(run);
      }
   }


   TIter next(lsyst);
   while ((lpersyst = (KVNumberList*)next())) {
      TEnv fruns;

      TString slist(lpersyst->GetExpandedList());
      slist.ReplaceAll(" ", ",");
      TString sname(lpersyst->GetName());
      sname.ReplaceAll("+ ", "");
      sname.ReplaceAll(" MeV/A", "");
      sname.ReplaceAll(" ", "");
      TString inst = "";
      if (lpersyst->GetNValues() > 1) {
         inst.Form("hadd -f -k -v 0 %s/%s.root %s/R{%s}.root",
                   op.Data(),
                   sname.Data(),
                   op.Data(),
                   slist.Data()
                  );
      } else if (lpersyst->GetNValues() == 1) {
         inst.Form("hadd -f -k -v 0 %s/%s.root %s/R%d.root",
                   op.Data(),
                   sname.Data(),
                   op.Data(),
                   lpersyst->First()
                  );
      }


      if (inst != "") {
         std::cout << "Creation of output file " << sname.Data() << ".root from run list:" << std::endl;
         std::cout << lpersyst->AsString() << " ..." << std::endl;
         KVString sroot;
         sroot.Form("%s/%s.root",
                    op.Data(),
                    sname.Data()
                   );

         fruns.SetValue("System", lpersyst->GetName());
         fruns.SetValue("DataSet", dsname);
         fruns.SetValue("Selector", kvsname);
         fruns.SetValue("Number", lpersyst->GetNValues());
         fruns.SetValue("RunList", lpersyst->AsString());
         gSystem->Exec(inst.Data());
         std::cout << " ... done" << std::endl;
         fruns.WriteFile(
            Form("%s/listruns_%s.env",
                 op.Data(),
                 sname.Data()
                )
         );
      }

   }


   delete lsyst;

}
//_____________________________________
void KVFAZIASelector::GetMissingRunsListing(const Char_t* kvsname, const Char_t* dsname)
{
   //static method
   //
   //Read the TEnv file created by the MergeListOfEndedRuns method with the format
   // listruns_[SYSTEM].env
   //extract the system name and the run list of the well ended file
   //and compare this list to the all run list of the system for the considering dataset
   //The output is a printing of the missing runs
   //compare to the system run list
   //
   TString output_path = gSystem->Getenv("FAZIASELECTOR_OUTPUTS");
   TString op;
   op.Form("%s/%s/%s", output_path.Data(), kvsname, dsname);
   if (output_path == "") {
      printf("Error in CheckListOfEndedRuns : FAZIASELECTOR_OUTPUTS shell variable is not defined\n");
      return;
   }
   if (gSystem->Exec(Form("test -d %s", op.Data())) != 0) {
      printf("directory %s does not exist\n", op.Data());
      return;
   }

   if (!(gDataSet)) {
      new KVDataRepositoryManager();
      gDataRepositoryManager->Init();
      gDataSetManager->GetDataSet(dsname)->cd();
   }

   KVString lenv = gSystem->GetFromPipe(Form("ls %s/*env", op.Data()));
   lenv.Begin("\n");
   while (!lenv.End()) {
      TEnv env;
      KVString senv = lenv.Next();
      env.ReadFile(senv.Data(), kEnvAll);
      TString syst(env.GetValue("System", ""));
      TString sruns(env.GetValue("RunList", ""));
      KVDBSystem* dbsys = gFaziaDB->GetSystem(syst.Data());
      if (dbsys) {
         KVNumberList ltot;
         dbsys->GetRunList(ltot);
         ltot.Remove(sruns.Data());
         if (ltot.GetNValues() > 0)
            printf("System: %s, Missing Runs %s\n", dbsys->GetName(), ltot.AsString());
         else
            printf("System: %s, All runs done\n", dbsys->GetName());
      }
   }

}
//_____________________________________
void KVFAZIASelector::CheckBatchOutputFiles(const Char_t* kvsname, Bool_t eraseafter)
{
   //static method
   //
   //Get the list of output batch files
   //with the format [kvsname]_R*.o*
   //for example :
   // CheckCsIIdentification_R14748-14767.o110904
   // Extract information on CPU, Memory consuming
   // and the list of ended/not ended files
   //
   // It is more adapted than the analyse_logs
   // script when several runs are in a same job
   //
   // output TEnv file is produced with the following information
   // with the following format : [kvsname].joblog
   // example with CheckCsIIdentification.joblog
   //
   // Nfiles:                                  21
   // Runlist:                                 14588-14905
   // CpuSys(s):                               4.51931
   // CpuUser(s):                              494.547
   // CpuPerRun:                               24.7273
   // ResMem(MB):                              474.09
   // VirtMem(MB):                             1870.77
   // MissingRuns:
   //
   // if eraseafter=kTRUE, (kFALSE by default),
   // log files are erased after the checking
   //
   KVFileReader fr;
   KVString lfiles = gSystem->GetFromPipe(Form("ls %s_R*.o*", kvsname));
   lfiles += "\n";
   lfiles += gSystem->GetFromPipe(Form("ls %s.o*", kvsname));

   lfiles.Begin("\n");
   Double_t max_sys_time = 0, max_cpu_time = 0, max_res_mem = 0, max_virt_mem = 0, max_ratio = 0;
   Double_t sys_time = 0, cpu_time = 0, res_mem = 0, virt_mem = 0;
   KVNumberList linit_all;
   KVNumberList lstarted_all;
   KVNumberList lended_all;
   Int_t nf = 0;
   while (!lfiles.End()) {
      KVString file = lfiles.Next();
      fr.OpenFileToRead(file.Data());

      KVNumberList linit;
      KVNumberList lstarted;
      KVNumberList lended;
      Int_t num = -1;
      while (fr.IsOK()) {
         fr.ReadLine(0);
         KVString line = fr.GetCurrentLine();
         //Info in <KVDataAnalyser::SetRuns>: Accepted runs : 14788-14807
         if (line.BeginsWith("Info in <KVFAZIAReconDataAnalyser::SetRuns>: Accepted runs :")) {
            line.ReplaceAll("Info in <KVFAZIAReconDataAnalyser::SetRuns>: Accepted runs :", "");
            linit.Add(line.Data());
            linit_all.Add(line.Data());
         } else if (line.BeginsWith("Info in <KVFAZIAReconDataAnalyser::SubmitTask>: treatment of run#")) {
            line.ReplaceAll("Info in <KVFAZIAReconDataAnalyser::SubmitTask>: treatment of run#", "");
            num = line.Atoi();
            lstarted.Add(num);
            lstarted_all.Add(num);
         } else if (line.BeginsWith(Form("Info in <%s::Process>: End of file reached after ", kvsname))) {
            lended.Add(num);
            lended_all.Add(num);
         } else if (line.BeginsWith("CpuSys = ")) {
            sscanf(line.Data(), " CpuSys = %lf  s.    CpuUser = %lf s.    ResMem = %lf MB   VirtMem = %lf MB",
                   &sys_time, &cpu_time, &res_mem, &virt_mem);

         }
      }
      fr.CloseFile();
      if (linit.GetNValues()) {
         Double_t ratio = 0;
         if (lended.GetNValues() > 0)
            ratio = cpu_time / lended.GetNValues();
         if (ratio > max_ratio) max_ratio = ratio;

         if (sys_time > max_sys_time) max_sys_time = sys_time;
         if (cpu_time > max_cpu_time) max_cpu_time = cpu_time;
         if (res_mem > max_res_mem)   max_res_mem = res_mem;
         if (virt_mem > max_virt_mem) max_virt_mem = virt_mem;

         linit.Remove(lstarted);
         if (linit.GetNValues() > 0)
            printf("in %s : runs not started : %s\n", file.Data(), linit.AsString());

         lstarted.Remove(lended);
         if (lstarted.GetNValues() > 0)
            printf("in %s : runs not ended : %s\n", file.Data(), lstarted.AsString());
      } else {
         printf("in %s : initial run list is empty\n", file.Data());
      }
      nf += 1;

   }

   if (nf > 0) {
      TEnv env;
      env.SetValue("Nfiles", nf);
      env.SetValue("Runlist", linit_all.AsString());
      env.SetValue("CpuSys(s)", max_sys_time);
      env.SetValue("CpuUser(s)", max_cpu_time);
      env.SetValue("CpuPerRun", max_ratio);
      env.SetValue("ResMem(MB)", max_res_mem);
      env.SetValue("VirtMem(MB)", max_virt_mem);

      printf("%d log files read\n", nf);
      printf("initial runlist : %s\n", linit_all.AsString());
      printf("Maximum values\n");
      printf("CpuSys=%lf s\nCpuUser=%lf s (%lf s/run)\nResMem=%lf MB\nVirtMem=%lf MB\n",
             max_sys_time,
             max_cpu_time,
             max_ratio,
             max_res_mem,
             max_virt_mem
            );

      KVNumberList lmissing;

      linit_all.Remove(lstarted_all);
      if (linit_all.GetNValues() > 0)
         printf("\truns not started : %s\n", linit_all.AsString());
      lmissing.Add(linit_all);
      lstarted_all.Remove(lended_all);
      if (lstarted_all.GetNValues() > 0)
         printf("\truns not ended : %s\n", lstarted_all.AsString());
      lmissing.Add(lstarted_all);
      if (lmissing.GetNValues() > 0)
         printf("\t%d runs to be done : %s with a total time of %lf\n", lmissing.GetNValues(), lmissing.AsString(), max_ratio * lmissing.GetNValues());
      env.SetValue("MissingRuns", lmissing.AsString());
      env.WriteFile(Form("%s.joblog", kvsname));

      if (eraseafter) {
         gROOT->ProcessLine(Form(".! rm  %s_R*.o*", kvsname));
         gROOT->ProcessLine(Form(".! rm  %s.o*", kvsname));
      }

   }
}
