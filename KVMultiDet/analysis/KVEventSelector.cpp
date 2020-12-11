#define KVEventSelector_cxx
#include "KVEventSelector.h"
#include <KVClassMonitor.h>
#include <TStyle.h>
#include "TPluginManager.h"
#include "TSystem.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataSetManager.h"
#include "TProof.h"
#include "KVDataSetAnalyser.h"

using namespace std;

ClassImp(KVEventSelector)



void KVEventSelector::Begin(TTree* /*tree*/)
{
   // Need to parse options here for use in Terminate
   // Also, on PROOF, any KVDataAnalyser instance has to be passed to the workers
   // via the TSelector input list.

   ParseOptions();

   if (IsOptGiven("CombinedOutputFile")) {
      fCombinedOutputFile = GetOpt("CombinedOutputFile");
   }
   else if (gProof) {
      // when running with PROOF, if the user calls SetCombinedOutputFile()
      // in InitAnalysis(), it will only be executed on the workers (in SlaveBegin()).
      // therefore we call InitAnalysis() here, but deactivate CreateTreeFile(),
      // AddTree() and AddHisto() in order to avoid interference with workers
      fDisableCreateTreeFile = kTRUE;
      if (gDataAnalyser) {
         gDataAnalyser->RegisterUserClass(this);
         gDataAnalyser->preInitAnalysis();
      }
      InitAnalysis();              //user initialisations for analysis
      if (gDataAnalyser) gDataAnalyser->postInitAnalysis();
      fDisableCreateTreeFile = kFALSE;
   }

   if (gDataAnalyser) {
      if (GetInputList()) {
         gDataAnalyser->AddJobDescriptionList(GetInputList());
         //GetInputList()->ls();
      }
   }
   if (IsOptGiven("AuxFiles")) {
      SetUpAuxEventChain();
      if (GetInputList()) GetInputList()->Add(fAuxChain);
   }
}

void KVEventSelector::SlaveBegin(TTree* /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).
   //
   // ParseOptions : Manage options passed as arguments
   //
   // Called user method InitAnalysis where users can create trees or histos
   // using the appropiate methods :
   // CreateTrees and CreateMethods
   //
   // Test the presence or not of such histo or tree
   // to manage it properly

   if (GetInputList() && GetInputList()->FindObject("JobDescriptionList")) {
      KVNameValueList* jdl = dynamic_cast<KVNameValueList*>(GetInputList()->FindObject("JobDescriptionList"));
      if (jdl) {
         KVDataAnalysisTask* task = nullptr;
         if (jdl->HasParameter("DataRepository")) {
            if (!gDataRepositoryManager) {
               new KVDataRepositoryManager;
               gDataRepositoryManager->Init();
            }
            gDataRepositoryManager->GetRepository(jdl->GetStringValue("DataRepository"))->cd();
            gDataSetManager->GetDataSet(jdl->GetStringValue("DataSet"))->cd();
            task = gDataSet->GetAnalysisTask(jdl->GetStringValue("AnalysisTask"));
         }
         else {
            if (!gDataSetManager) {
               gDataSetManager = new KVDataSetManager;
               gDataSetManager->Init();
            }
            task = gDataSetManager->GetAnalysisTaskAny(jdl->GetStringValue("AnalysisTask"));
         }
         gDataAnalyser = KVDataAnalyser::GetAnalyser(task->GetDataAnalyser());
         if (gDataSet && gDataAnalyser->InheritsFrom("KVDataSetAnalyser"))
            dynamic_cast<KVDataSetAnalyser*>(gDataAnalyser)->SetDataSet(gDataSet);
         gDataAnalyser->SetAnalysisTask(task);
         gDataAnalyser->RegisterUserClass(this);
         gDataAnalyser->SetProofMode((KVDataAnalyser::EProofMode)jdl->GetIntValue("PROOFMode"));
      }
   }

   ParseOptions();

   if (IsOptGiven("CombinedOutputFile")) {
      fCombinedOutputFile = GetOpt("CombinedOutputFile");
      Info("SlaveBegin", "Output file name = %s", fCombinedOutputFile.Data());
   }

   // tell the data analyser who we are
   if (gDataAnalyser) {
      gDataAnalyser->RegisterUserClass(this);
      gDataAnalyser->preInitAnalysis();
   }
   // make sure histo/tree creation are enabled
   fDisableCreateTreeFile = kFALSE;
   InitAnalysis();              //user initialisations for analysis
   if (gDataAnalyser) gDataAnalyser->postInitAnalysis();

   if (ltree->GetEntries() > 0) {
      for (Int_t ii = 0; ii < ltree->GetEntries(); ii += 1) {
         TTree* tt = (TTree*)ltree->At(ii);
         tt->SetDirectory(writeFile);
         tt->AutoSave();
      }
   }

   if (IsOptGiven("AuxFiles")) {
      // auxiliary files with PROOF
      if (GetInputList()) {
         fAuxChain = (TTree*)GetInputList()->FindObject(GetOpt("AuxTreeName"));
         InitFriendTree(fAuxChain, GetOpt("AuxBranchName"));
      }
   }
   Info("SlaveBegin", "fOutput->ls()");
   GetOutputList()->ls();
}
Bool_t KVEventSelector::CreateTreeFile(const Char_t* filename)
{
   // For PROOF:
   // This method must be called before creating any user TTree in InitAnalysis().
   // If no filename is given, default name="TreeFileFrom[name of selector class].root"

   if (fDisableCreateTreeFile) return kTRUE;

   TString tree_file_name;
   if (!strcmp(filename, ""))
      tree_file_name.Form("TreeFileFrom%s.root", ClassName());
   else
      tree_file_name = filename;

   mergeFile = new TProofOutputFile(tree_file_name.Data(), "M");
   mergeFile->SetOutputFileName(tree_file_name.Data());

   TDirectory* savedir = gDirectory;
   writeFile = mergeFile->OpenFile("RECREATE");
   if (writeFile && writeFile->IsZombie()) SafeDelete(writeFile);
   savedir->cd();

   // Cannot continue
   if (!writeFile) {
      Info("CreateTreeFile", "could not create '%s': instance is invalid!", filename);
      return kFALSE;
   }
   return kTRUE;

}

Bool_t KVEventSelector::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either KVEventSelector::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // Processing will abort cleanly if static flag fCleanAbort has been set
   // by some external controlling process.
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used.

   if (gDataAnalyser && gDataAnalyser->AbortProcessingLoop()) {
      // abort requested by external process
      Abort(Form("Job received KILL signal from batch system after %lld events - batch job probably needs more CPU time (see end of job statistics)", fEventsRead), kAbortFile);
      return kFALSE;
   }

   fTreeEntry = entry;

   if (gDataAnalyser && gDataAnalyser->CheckStatusUpdateInterval(fEventsRead))
      gDataAnalyser->DoStatusUpdate(fEventsRead);

   GetEntry(entry);
   if (gDataAnalyser) gDataAnalyser->preAnalysis();
   fEventsRead++;
   if (GetEvent()) {
      KVNucleus* part = 0;
      //apply particle selection criteria
      if (fPartCond.IsSet()) {
         part = 0;
         GetEvent()->ResetGetNextParticle();
         while ((part = GetEvent()->GetNextParticle("ok"))) {
            part->SetIsOK(fPartCond.Test(part));
         }
      }
      GetEvent()->ResetGetNextParticle();
      SetAnalysisFrame();//let user define any necessary reference frames for OK particles

      // initialise global variables at first event
      if (fFirstEvent && gvlist) {
         gvlist->Init();
         fFirstEvent = kFALSE;
      }
      RecalculateGlobalVariables();
   }

   Bool_t ok_anal = kTRUE;
   if (!gvlist || (gvlist && !gvlist->AbortEventAnalysis())) {
      // if global variables are defined, events are only analysed if no global variables
      // in the list have failed an event selection condition
      ok_anal = Analysis();     //user analysis
      if (gDataAnalyser) gDataAnalyser->postAnalysis();
   }
   CheckEndOfRun();

   return ok_anal;
}

void KVEventSelector::CheckEndOfRun()
{
   // Testing whether EndRun() should be called
   if (AtEndOfRun()) {
      Info("Process", "End of file reached after %lld events", fEventsRead);
      if (gDataAnalyser) gDataAnalyser->preEndRun();
      EndRun();
      if (gDataAnalyser) gDataAnalyser->postEndRun();
      fNotifyCalled = kFALSE;//Notify will be called when next file is opened (in TChain)
   }

}

void KVEventSelector::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.
   // if tree have been defined in the CreateTrees method
   // manage the merge of them in ProofLite session
   //

   if (ltree->GetEntries() > 0) {

      if (writeFile) {
         TDirectory* savedir = gDirectory;
         TTree* tt = 0;
         for (Int_t ii = 0; ii < ltree->GetEntries(); ii += 1) {
            tt = (TTree*)ltree->At(ii);
            writeFile->cd();
            tt->Write();
         }
         mergeFile->Print();
         fOutput->Add(mergeFile);

         for (Int_t ii = 0; ii < ltree->GetEntries(); ii += 1) {
            tt = (TTree*)ltree->At(ii);
            tt->SetDirectory(0);
         }

         gDirectory = savedir;
         writeFile->Close();
      }

   }

   fOutput->ls();
}

void KVEventSelector::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
   //
   // This method call the user defined EndAnalysis
   // where user can do what she wants
   //

   TDatime now;
   Info("Terminate", "Analysis ends at %s", now.AsString());

   if (fCombinedOutputFile != "") {
      Info("Terminate", "combine = %s", fCombinedOutputFile.Data());
      // combine histograms and trees from analysis into one file
      TString file1, file2;
      file1.Form("HistoFileFrom%s.root", ClassName());
      file2.Form("TreeFileFrom%s.root", ClassName());
      GetOutputList()->ls();
      if (GetOutputList()->FindObject("ThereAreHistos")) {
         if (GetOutputList()->FindObject(file2)) {
            Info("Terminate", "both");
            SaveHistos();
            KVBase::CombineFiles(file1, file2, fCombinedOutputFile, kFALSE);
         }
         else {
            // no trees - just rename histo file
            Info("Terminate", "histo");
            SaveHistos(fCombinedOutputFile);
         }
      }
      else if (GetOutputList()->FindObject(file2)) {
         // no histos - just rename tree file
         Info("Terminate", "tree");
         gSystem->Rename(file2, fCombinedOutputFile);
      }
      else  Info("Terminate", "none");
   }

   if (gDataAnalyser) gDataAnalyser->preEndAnalysis();
   EndAnalysis();               //user end of analysis routine
   if (gDataAnalyser) gDataAnalyser->postEndAnalysis();

   if (GetInputList() && fAuxChain) GetInputList()->Remove(fAuxChain);
   SafeDelete(fAuxChain);
}

KVVarGlob* KVEventSelector::AddGV(const Char_t* class_name,
                                  const Char_t* name)
{
   //Add a global variable to the list of variables for the analysis.
   //
   //`"class_name"` must be the name of a valid class inheriting from KVVarGlob, e.g. any of the default global
   //variable classes defined as part of the standard KaliVeda package (see the [global variables module](group__Globvars.html)),
   //or the name of a user-defined class (see below).
   //
   //`"name"` is a unique name for the new global variable object which will be created and added to the internal
   //list of global variables. This name can be used to retrieve the object (see GetGV()) in the user's analysis.
   //
   //Returns pointer to new global variable object in case more than the usual default initialisation is necessary.
   //
   //### User-defined global variables
   //The user may use her own global variables in an analysis class, without having to add them to the main libraries.
   //If the given class name is not known, it is assumed to be a user-defined class and we attempt to compile and load
   //the class from the user's source code. For this to work, the user must:
   //
   //  1. add to the ROOT macro path the directory where her class's source code is kept, e.g. in `$HOME/.rootrc` add the following line:
   //
   //~~~~~~~~~~~~~~~
   //              +Unix.*.Root.MacroPath:      $(HOME)/myVarGlobs
   //~~~~~~~~~~~~~~~
   //
   //  2. for each user-defined class, add a line to $HOME/.kvrootrc to define a "plugin". E.g. for a class called MyNewVarGlob,
   //
   //~~~~~~~~~~~~~~~
   //              +Plugin.KVVarGlob:    MyNewVarGlob    MyNewVarGlob     MyNewVarGlob.cpp+   "MyNewVarGlob()"
   //~~~~~~~~~~~~~~~
   //
   //  It is assumed that `MyNewVarGlob.h` and `MyNewVarGlob.cpp` will be found in `$HOME/myVarGlobs` (in this example).

   KVVarGlob* vg = GetGVList()->AddGV(class_name, name);
   return vg;
}

//____________________________________________________________________________

void KVEventSelector::RecalculateGlobalVariables()
{
   //Use this method if you change e.g. the particle selection criteria in your
   //Analysis() method and want to recalculate the values of all global variables
   //for your new selection.
   //
   //WARNING: the global variables are calculated automatically for you for each event
   //before method Analysis() is called. In order for the correct particles to be included in
   //this calculation, make sure that at the END of Analysis() you reset the selection
   //criteria.

   if (gvlist) gvlist->CalculateGlobalVariables(GetEvent());
}

//____________________________________________________________________________

void KVEventSelector::SetParticleConditions(const KVParticleCondition& cond, const KVString& upcast_class)
{
   //Use this method to set criteria for selecting particles to include in analysis.
   //The criteria defined in the KVParticleCondition object will be applied to every
   //particle and if they are not satisfied the particle's
   //"OK" flag will be set to false, i.e. the particle's IsOK() method will return kFALSE,
   //and the particle will not be included in iterations such as GetEvent()->GetNextParticle("OK").
   //Neither will the particle be included in the evaluation of any global variables.
   //
   //This method must be called in the user's InitAnalysis() or InitRun() method.
   //
   //If the methods used in the condition are not defined for KVNucleus, you can give the
   //name of the class to which the methods refer (upcast_class), or you can set it before
   //hand (SetParticleConditionsParticleClassName)

   fPartCond = cond;
   if (upcast_class != "") fPartCond.SetParticleClassName(upcast_class);
   else if (fPartName != "") fPartCond.SetParticleClassName(fPartName);
}

//____________________________________________________________________________

const KVHashList* KVEventSelector::GetHistoList() const
{

   //return the list of created trees
   return lhisto;

}

//____________________________________________________________________________

TH1* KVEventSelector::GetHisto(const Char_t* histo_name) const
{

   return lhisto->get_object<TH1>(histo_name);

}

//____________________________________________________________________________

void KVEventSelector::AddHisto(TH1* histo)
{
   // Declare a histogram to be used in analysis.
   // This method must be called when using PROOF.

   if (fDisableCreateTreeFile) return;
   histo->SetDirectory(nullptr);
   lhisto->Add(histo);
   fOutput->Add(histo);
   if (!fOutput->FindObject("ThereAreHistos")) fOutput->Add(new TNamed("ThereAreHistos", "...so save them!"));
}

void KVEventSelector::AddTree(TTree* tree)
{
   // Declare a TTree to be used in analysis.
   // This method must be called when using PROOF.

   if (fDisableCreateTreeFile) return;

   ltree->Add(tree);
}

//____________________________________________________________________________

void KVEventSelector::FillHisto(const Char_t* histo_name, Double_t one, Double_t two, Double_t three, Double_t four)
{

   //Find in the list, if there is an histogram named "sname"
   //If not print an error message
   //If yes redirect to the right method according to its closest mother class
   //to fill it
   TH1* h1 = 0;
   if ((h1 = GetHisto(histo_name))) {
      if (h1->InheritsFrom("TH3"))
         FillTH3((TH3*)h1, one, two, three, four);
      else if (h1->InheritsFrom("TProfile2D"))
         FillTProfile2D((TProfile2D*)h1, one, two, three, four);
      else if (h1->InheritsFrom("TH2"))
         FillTH2((TH2*)h1, one, two, three);
      else if (h1->InheritsFrom("TProfile"))
         FillTProfile((TProfile*)h1, one, two, three);
      else if (h1->InheritsFrom("TH1"))
         FillTH1(h1, one, two);
      else
         Warning("FillHisto", "%s -> Classe non prevue ...", lhisto->FindObject(histo_name)->ClassName());
   }
   else {
      Warning("FillHisto", "%s introuvable", histo_name);
   }

}

void KVEventSelector::FillHisto(const Char_t* histo_name, const Char_t* label, Double_t weight)
{
   // Fill 1D histogram with named bins
   TH1* h1 = 0;
   if ((h1 = GetHisto(histo_name))) {
      h1->Fill(label, weight);
   }
   else {
      Warning("FillHisto", "%s introuvable", histo_name);
   }

}

//____________________________________________________________________________

void KVEventSelector::FillTH1(TH1* h1, Double_t one, Double_t two)
{

   h1->Fill(one, two);

}

//____________________________________________________________________________

void KVEventSelector::FillTProfile(TProfile* h1, Double_t one, Double_t two, Double_t three)
{

   h1->Fill(one, two, three);

}

//____________________________________________________________________________

void KVEventSelector::FillTH2(TH2* h2, Double_t one, Double_t two, Double_t three)
{

   h2->Fill(one, two, three);

}

//____________________________________________________________________________

void KVEventSelector::FillTProfile2D(TProfile2D* h2, Double_t one, Double_t two, Double_t three, Double_t four)
{

   h2->Fill(one, two, three, four);
}

//____________________________________________________________________________

void KVEventSelector::FillTH3(TH3* h3, Double_t one, Double_t two, Double_t three, Double_t four)
{

   h3->Fill(one, two, three, four);
}

void KVEventSelector::SetUpAuxEventChain()
{
   // Called by SlaveBegin() when user gives the following options:
   //
   //~~~~~~~~~~~~~~~~
   //    AuxFiles:               list of files containing "friend" TTrees to be made available during analysis
   //    AuxDir:                 directory in which to find AuxFiles
   //    AuxTreeName:            name of tree in AuxFiles containing KVEvent objects
   //    AuxBranchName:          name of branch in AuxFiles containing KVEvent objects
   //~~~~~~~~~~~~~~~~

   if (!IsOptGiven("AuxDir") || !IsOptGiven("AuxTreeName") || !IsOptGiven("AuxBranchName")) {
      Error("SetUpAuxEventChain", "if AuxFiles option given, you must define AuxDir, AuxTreeName and AuxBranchName");
      return;
   }
   KVString filelist = GetOpt("AuxFiles");
   KVString filedir = GetOpt("AuxDir");
   if (!filedir.EndsWith("/")) filedir += "/";
   TChain* auxchain = new TChain(GetOpt("AuxTreeName"));
   filelist.Begin("|");
   while (!filelist.End()) {
      KVString path = filedir + filelist.Next();
      auxchain->Add(path);
   }
   InitFriendTree(auxchain, GetOpt("AuxBranchName"));
}

//____________________________________________________________________________

void KVEventSelector::SaveHistos(const Char_t* filename, Option_t* option, Bool_t onlyfilled)
{
   // Write in file all histograms declared with AddHisto(TH1*)
   // This method works with PROOF.
   //
   // If no filename is specified, set default name : HistoFileFrom[KVEvenSelector::GetName()].root
   //
   // If a filename is specified, search in gROOT->GetListOfFiles() if
   // this file has been already opened
   //  - if yes write in it
   //  - if not, create it with the corresponding option, write in it
   // and close it just after
   //
   // onlyfilled flag allow to write all (onlyfilled=kFALSE, default)
   // or only histograms (onlyfilled=kTRUE) those have been filled

   TString histo_file_name = "";
   if (!strcmp(filename, ""))
      histo_file_name.Form("HistoFileFrom%s.root", GetName());
   else
      histo_file_name = filename;

   Bool_t justopened = kFALSE;

   TFile* file = 0;
   TDirectory* pwd = gDirectory;
   //if filename correspond to an already opened file, write in it
   //if not open/create it, depending on the option ("recreate" by default)
   //and write in it
   if (!(file = (TFile*)gROOT->GetListOfFiles()->FindObject(histo_file_name.Data()))) {
      file = new TFile(histo_file_name.Data(), option);
      justopened = kTRUE;
   }
   file->cd();
   TIter next(GetOutputList());
   TObject* obj = 0;
   while ((obj = next())) {
      if (obj->InheritsFrom("TH1")) {
         if (onlyfilled) {
            if (((TH1*)obj)->GetEntries() > 0) {
               obj->Write();
            }
         }
         else {
            obj->Write();
         }
      }
   }
   if (justopened)
      file->Close();
   pwd->cd();

}

//____________________________________________________________________________

const KVHashList* KVEventSelector::GetTreeList() const
{
   //return the list of created trees
   return ltree;

}

//____________________________________________________________________________

TTree* KVEventSelector::GetTree(const Char_t* tree_name) const
{
   //return the tree named tree_name
   TTree* t = ltree->get_object<TTree>(tree_name);
   if (!t) Fatal("GetTree", "Tree %s not found: is this the right name?", tree_name);
   return t;
}

//____________________________________________________________________________

void KVEventSelector::FillTree(const Char_t* tree_name)
{
   //Filltree method, the tree named tree_name
   //has to be declared with AddTTree(TTree*) method
   //
   //if no sname="", all trees in the list is filled
   //
   if (!strcmp(tree_name, "")) {
      ltree->Execute("Fill", "");
   }
   else {
      TTree* tt = 0;
      if ((tt = GetTree(tree_name))) {
         tt->Fill();
      }
      else {
         Warning("FillTree", "%s introuvable", tree_name);
      }
   }

}

void KVEventSelector::SetOpt(const Char_t* option, const Char_t* value)
{
   //Set a value for an option
   KVString tmp(value);
   fOptionList.SetValue(option, tmp);
}

//_________________________________________________________________

Bool_t KVEventSelector::IsOptGiven(const Char_t* opt)
{
   // Returns kTRUE if the option 'opt' has been set

   return fOptionList.HasParameter(opt);
}

//_________________________________________________________________

TString KVEventSelector::GetOpt(const Char_t* opt) const
{
   // Returns the value of the option
   //
   // Only use after checking existence of option with IsOptGiven(const Char_t* opt)

   return fOptionList.GetTStringValue(opt);
}

//_________________________________________________________________

void KVEventSelector::UnsetOpt(const Char_t* opt)
{
   // Removes the option 'opt' from the internal lists, as if it had never been set

   fOptionList.RemoveParameter(opt);
}

void KVEventSelector::ParseOptions()
{
   // Analyse comma-separated list of options given to TTree::Process
   // and store all `"option=value"` pairs in fOptionList.
   // Options can then be accessed using IsOptGiven(), GetOptString(), etc.
   //
   //~~~~~~~~~~~~~~~
   //     BranchName=xxxx  :  change name of branch in TTree containing data
   //     EventsReadInterval=N: print "+++ 12345 events processed +++" every N events
   //~~~~~~~~~~~~~~~
   //
   // This method is called by SlaveBegin
   //

   fOptionList.Clear(); // clear list
   KVString option = GetOption();
   option.Begin(",");
   while (!option.End()) {

      KVString opt = option.Next();
      opt.Begin("=");
      KVString param = opt.Next();
      KVString val = opt.Next();
      while (!opt.End()) {
         val += "=";
         val += opt.Next();
      }

      SetOpt(param.Data(), val.Data());
   }

   fOptionList.Print();
   // check for branch name
   if (IsOptGiven("BranchName")) SetBranchName(GetOpt("BranchName"));
   // check for events read interval
   if (IsOptGiven("EventsReadInterval")) SetEventsReadInterval(GetOpt("EventsReadInterval").Atoi());
}

void KVEventSelector::Init(TTree* tree)
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
   //user additional branches addressing
   SetAdditionalBranchAddress();
   fEventsRead = 0;

}

void KVEventSelector::InitFriendTree(TTree* tree, const TString& branchname)
{
   // Set up a "friend" TTree/TChain containing KVEvent-derived objects in branch 'branchname'
   // N.B. this is not a "friend" in the sense of TTree::AddFriend, the main TTree and the
   // "friend" TTree can have different numbers of entries
   //
   // After calling this method at the beginning of the analysis, you can
   // access any of the events stored in the "friend" by doing:
   //
   //~~~~~~~~~~~~
   //   GetFriendTreeEntry(entry_number);
   //   KVEvent* friend_event = GetFriendEvent();
   //~~~~~~~~~~~~

   AuxEvent = 0;
   fAuxChain = tree;
   fAuxChain->SetBranchAddress(branchname, &AuxEvent);
   fAuxChain->Print();
   fAuxChain->GetEntry(0);
   fAuxChain->GetTree()->GetEntry(0);
}

Bool_t KVEventSelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   if (fNotifyCalled) return kTRUE; // avoid multiple calls at beginning of analysis
   fNotifyCalled = kTRUE;

   Info("Notify", "Beginning analysis of file %s (%lld events)", fChain->GetCurrentFile()->GetName(), fChain->GetTree()->GetEntries());

   if (gDataAnalyser) gDataAnalyser->preInitRun();
   InitRun();                   //user initialisations for run
   if (gDataAnalyser) gDataAnalyser->postInitRun();

   KVClassMonitor::GetInstance()->SetInitStatistics();

   return kTRUE;
}

/** \example ExampleSimDataAnalysis.cpp
# Example of an analysis class for simulated data

A simple example of analysis of simulated data, for use with kaliveda-sim

\include ExampleSimDataAnalysis.h
*/


/** \example ExampleFilteredSimDataAnalysis.cpp
# Example of an analysis class for filtered simulated data

A simple example of analysis of filtered simulated data, for use with kaliveda-sim.
Note that this class was written specially for filtering simulations with the
coupled INDRA and FAZIA arrays.

\include ExampleFilteredSimDataAnalysis.h
*/

