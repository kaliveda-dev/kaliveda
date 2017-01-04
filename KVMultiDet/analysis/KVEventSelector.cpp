#define KVEventSelector_cxx
#include "KVEventSelector.h"
#include <TStyle.h>
#include "TPluginManager.h"
#include "TSystem.h"
#include "KVDataAnalyser.h"

using namespace std;

ClassImp(KVEventSelector)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVEventSelector</h2>
<h4>General purpose analysis class for TTrees containing KVEvent objects</h4>
<!-- */
// --> END_HTML
// Use this TSelector to analyse data in TTrees containing a branch with KVEvent-derived objects:
//
//    my_tree->Process("KVEventSelector", "[options]");
//
// where "[options]" is the list of options in the form "BranchName=toto, ...".
//
// The following options MUST be given:
//
//    BranchName:                name of branch containing the events
//
// The following is an optional option:
//
//    EventsReadInterval:     print info on analysis every N events instead of default value
//
// Any other options can be defined by the user and parsed in her analysis class
// with methods IsOptGiven(const Char_t*) and GetOpt(const Char_t*)
//
// If the user needs to define new reference frames for the data which must exist before global variables
// are automatically calculated in the analysis loop, she can do so by overriding the method
// SetAnalysisFrame(), like so:
// MyAnalysis::SetAnalysisFrame()
// {
//     static TVector3 cmvelocity(0,0,3.0);
//     GetEvent()->SetFrame("CM", cmvelocity);
// }
//
// *** USE WITH PROOF ***
// In order to use a KVEventSelector with PROOF:
//
// - declare any histograms with method AddHisto(TH1*)
// e.g. in InitAnalysis:
// void MySelector::InitAnalysis()
// {
//     AddHisto( new TH2F("toto", "tata", 100, 0, 0, 500, 0, 0) );
// }
// Histograms can also be declared 'on the fly' in Analysis() method in
// the same way;
//
// - for TTrees, first call CreateTreeFile("...") with name of file for TTree(s)
// (by default, histograms and TTrees are written in different files - but see below),
// then declare all trees using method AddTree(TTree*)
// e.g. in InitAnalysis:
// void MySelector::InitAnalysis()
// {
//     CreateTreeFile("MyTrees.root");
//     TTree* aTree = new TTree("t1", "Some Tree");
//     aTree->Branch(...) etc.
//     AddTree(aTree);
// }
//
// - if you want (not obligatory), you can use methods FillHisto(...) and
//  FillTree(...) in your Analysis() method;
//
// - to save histograms to file in EndAnalysis(), call method
//  SaveHistos(const Char_t* filename) in EndAnalysis()
//
// -  the file declared with CreateTreeFile will be automatically written
// to disk at the end of the analysis.
//
// HISTOS & TREES IN SAME FILE
// If you want all results of your analysis to be written in a single file
// containing both histos and trees, put the following in the list of options:
//      CombinedOutputFile=myResults.root
// do not call SaveHistos() in EndAnalysis(), and make
// sure you call CreateTreeFile() without giving a name (the
// resulting intermediate file will have a default name
// allowing it to be found at the end of the analysis)
////////////////////////////////////////////////////////////////////////////////

void KVEventSelector::Begin(TTree* /*tree*/)
{
   // Need to parse options here for use in Terminate

   ParseOptions();

   if (IsOptGiven("CombinedOutputFile")) {
      fCombinedOutputFile = GetOpt("CombinedOutputFile");
      Info("Begin", "Output file name = %s", fCombinedOutputFile.Data());
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

   ParseOptions();

   if (IsOptGiven("CombinedOutputFile")) {
      fCombinedOutputFile = GetOpt("CombinedOutputFile");
      Info("SlaveBegin", "Output file name = %s", fCombinedOutputFile.Data());
   }

   InitAnalysis();

   if (ltree->GetEntries() > 0)
      for (Int_t ii = 0; ii < ltree->GetEntries(); ii += 1) {
         TTree* tt = (TTree*)ltree->At(ii);
         tt->SetDirectory(writeFile);
         tt->AutoSave();
      }

}
Bool_t KVEventSelector::CreateTreeFile(const Char_t* filename)
{
   // For PROOF:
   // This method must be called before creating any user TTree in InitAnalysis().
   // If no filename is given, default name="TreeFileFrom[name of selector class].root"

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
      Abort(Form("Aborting analysis after %lld events", fEventsRead), kAbortFile);
      return kFALSE;
   }

   fTreeEntry = entry;

   if (!(fEventsRead % fEventsReadInterval) && fEventsRead) {
      Info("Process", " +++ %lld events processed +++ ", fEventsRead);
      ProcInfo_t pid;
      if (gSystem->GetProcInfo(&pid) == 0) {
         cout << "     ------------- Process infos -------------" << endl;
         printf(" CpuSys = %f  s.    CpuUser = %f s.    ResMem = %f MB   VirtMem = %f MB\n",
                pid.fCpuSys, pid.fCpuUser, pid.fMemResident / 1024., pid.fMemVirtual / 1024.);
      }
   }
   GetEntry(entry);
   fEventsRead++;
   if (GetEvent()) {
      SetAnalysisFrame();//let user define any necessary reference frames
      KVNucleus* part = 0;
      //apply particle selection criteria
      if (fPartCond) {
         part = 0;
         while ((part = GetEvent()->GetNextParticle("ok"))) {
            part->SetIsOK(fPartCond->Test(part));
         }
      }

      // initialise global variables at first event
      if (fFirstEvent && gvlist) {
         gvlist->Init();
         fFirstEvent = kFALSE;
      }
      RecalculateGlobalVariables();
   }

   Bool_t ok_anal = kTRUE;
   ok_anal = Analysis();     //user analysis

   CheckEndOfRun();

   return ok_anal;
}

void KVEventSelector::CheckEndOfRun()
{
   // Testing whether EndRun() should be called
   if (AtEndOfRun()) {
      Info("Process", "End of file reached after %lld events", fEventsRead);

      EndRun();
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
      if (GetOutputList()->FindObject("ThereAreHistos")) {
         if (GetOutputList()->FindObject(file2)) {
            Info("Terminate", "both");
            SaveHistos();
            KVBase::CombineFiles(file1, file2, fCombinedOutputFile, kFALSE);
         } else {
            // no trees - just rename histo file
            Info("Terminate", "histo");
            SaveHistos(fCombinedOutputFile);
         }
      } else if (GetOutputList()->FindObject(file2)) {
         // no histos - just rename tree file
         Info("Terminate", "tree");
         gSystem->Rename(file2, fCombinedOutputFile);
      } else  Info("Terminate", "none");
   }


   EndAnalysis();               //user end of analysis routine

}

KVVarGlob* KVEventSelector::AddGV(const Char_t* class_name,
                                  const Char_t* name)
{
   //Add a global variable to the list of variables for the analysis.
   //
   //"class_name" must be the name of a valid class inheriting from KVVarGlob, e.g. any of the default global
   //variable classes defined as part of the standard KaliVeda package (in libKVvVarGlob.so). See
   //"Class Reference" page on website for the available classes (listed by category under "Global Variables: ...").
   //
   //USER-DEFINED GLOBAL VARIABLES
   //The user may use her own global variables in an analysis class, without having to add them to the main libraries.
   //If the given class name is not known, it is assumed to be a user-defined class and we attempt to compile and load
   //the class from the user's source code. For this to work, the user must:
   //
   //      (1) add to the ROOT macro path the directory where her class's source code is kept, e.g. in $HOME/.rootrc
   //              add the following line:
   //
   //              +Unix.*.Root.MacroPath:      $(HOME)/myVarGlobs
   //
   //      (2) for each user-defined class, add a line to $HOME/.kvrootrc to define a "plugin". E.g. for a class called MyNewVarGlob,
   //
   //              +Plugin.KVVarGlob:    MyNewVarGlob    MyNewVarGlob     MyNewVarGlob.cpp+   "MyNewVarGlob()"
   //
   //      It is assumed that MyNewVarGlob.h and MyNewVarGlob.cpp will be found in $HOME/myVarGlobs (in this example).
   //
   //"name" is a unique name for the new global variable object which will be created and added to the internal
   //list of global variables. This name can be used to retrieve the object (see GetGV) in the user's analysis.
   //
   //Returns pointer to new global variable object in case more than the usual default initialisation is necessary.

   KVVarGlob* vg = 0;
   TClass* clas = gROOT->GetClass(class_name);
   if (!clas) {
      //class not in dictionary - user-defined class ? Look for plugin.
      TPluginHandler* ph = KVBase::LoadPlugin("KVVarGlob", class_name);
      if (!ph) {
         //not found
         Error("AddGV(const Char_t*,const Char_t*)",
               "Called with class_name=%s.\nClass is unknown: not in standard libraries, and plugin (user-defined class) not found",
               class_name);
         return 0;
      } else {
         vg = (KVVarGlob*) ph->ExecPlugin(0);
      }
   } else if (!clas->InheritsFrom("KVVarGlob")) {
      Error("AddGV(const Char_t*,const Char_t*)",
            "%s is not a valid class deriving from KVVarGlob.",
            class_name);
      return 0;
   } else {
      vg = (KVVarGlob*) clas->New();
   }
   vg->SetName(name);
   AddGV(vg);
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

   if (gvlist) {
      // 1st step: Reset global variables
      gvlist->Reset();

      //2nd step: loop over particles with correct codes
      //          and fill global variables
      KVNucleus* n1 = 0;
      // calculate 1-body variables
      if (gvlist->Has1BodyVariables()) {
         while ((n1 = GetEvent()->GetNextParticle("ok"))) {
            gvlist->Fill(n1);
         }
      }
      KVNucleus* n2 = 0;
      // calculate 2-body variables
      // we use every pair of particles (including identical pairs) in the event
      if (gvlist->Has2BodyVariables()) {
         Int_t N = GetEvent()->GetMult();
         for (int i1 = 1; i1 <= N ; i1++) {
            for (int i2 = 1 ; i2 <= N ; i2++) {
               n1 = GetEvent()->GetParticle(i1);
               n2 = GetEvent()->GetParticle(i2);
               if (n1->IsOK() && n2->IsOK())
                  gvlist->Fill2(n1, n2);
            }
         }
      }
      // calculate N-body variables
      if (gvlist->HasNBodyVariables()) gvlist->FillN(GetEvent());
   }
}

//____________________________________________________________________________

void KVEventSelector::SetParticleConditions(const KVParticleCondition& cond)
{
   //Use this method to set criteria for selecting particles to include in analysis.
   //The criteria defined in the KVParticleCondition object will be applied to every
   //particle and if they are not satisfied the particle's
   //"OK" flag will be set to false, i.e. the particle's IsOK() method will return kFALSE,
   //and the particle will not be included in iterations such as GetEvent()->GetNextParticle("OK").
   //Neither will the particle be included in the evaluation of any global variables.
   //
   //This method must be called in the user's InitAnalysis() or InitRun() method.

   if (!fPartCond) fPartCond = new KVParticleCondition(cond);
   else *fPartCond = cond;
}

//____________________________________________________________________________

KVHashList* KVEventSelector::GetHistoList() const
{

   //return the list of created trees
   return lhisto;

}

//____________________________________________________________________________

TH1* KVEventSelector::GetHisto(const Char_t* histo_name) const
{

   return (TH1*)lhisto->FindObject(histo_name);

}

//____________________________________________________________________________

void KVEventSelector::AddHisto(TH1* histo)
{
   // Declare a histogram to be used in analysis.
   // This method must be called when using PROOF.
   lhisto->Add(histo);
   fOutput->Add(histo);
   if (!fOutput->FindObject("ThereAreHistos")) fOutput->Add(new TNamed("ThereAreHistos", "...so save them!"));
}

void KVEventSelector::AddTree(TTree* tree)
{
   // Declare a TTree to be used in analysis.
   // This method must be called when using PROOF.
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
   } else {
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
         } else {
            obj->Write();
         }
      }
   }
   if (justopened)
      file->Close();
   pwd->cd();

}

//____________________________________________________________________________

KVHashList* KVEventSelector::GetTreeList() const
{
   //return the list of created trees
   return ltree;

}

//____________________________________________________________________________

TTree* KVEventSelector::GetTree(const Char_t* tree_name) const
{
   //return the tree named tree_name
   return (TTree*)ltree->FindObject(tree_name);

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
   } else {
      TTree* tt = 0;
      if ((tt = GetTree(tree_name))) {
         tt->Fill();
      } else {
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

const TString& KVEventSelector::GetOpt(const Char_t* opt) const
{
   // Returns the value of the option
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
   // and store all "option=value" pairs in fOptionList.
   // Options can then be accessed using IsOptGiven(), GetOptString(), etc.
   //
   //     BranchName=xxxx  :  change name of branch in TTree containing data
   //     EventsReadInterval=N: print "+++ 12345 events processed +++" every N events
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
