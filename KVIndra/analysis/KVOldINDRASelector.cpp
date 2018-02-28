#define KVOldINDRASelector_cxx
#include "KVOldINDRASelector.h"
#include "TH2.h"
#include "TStyle.h"
#include "TString.h"
#include "TEnv.h"
#include "TSystem.h"
#include "TClass.h"
#include "TMacro.h"
#include "TROOT.h"
#include "KVBase.h"
#include "KVDataSet.h"
#include "KVAvailableRunsFile.h"
#include "TPluginManager.h"
#include "KVClassFactory.h"
#include "KVDataAnalyser.h"
#include "KVDataAnalysisTask.h"
#include "KVINDRAReconNuc.h"
#include "KVINDRAReconDataAnalyser.h"

#include <KVINDRADB.h>

KVString KVOldINDRASelector::fBranchName = "INDRAReconEvent";
using namespace std;

ClassImp(KVOldINDRASelector)

//______________________________________________________________________________
//KVOldINDRASelector
//
//Basic analysis class derived from TSelector for the analysis of TChains of
//KVINDRAReconEvent objects (ROOT files containing INDRA data).
//
//The user's analysis class, derived from KVOldINDRASelector, must define the following
//methods:
//
//      void    InitAnalysis()
//
//              Called once at the beginning of the analysis.
//              Can be used to define histograms, TTrees, etc.
//              Also the place to define the list of global variables to be used.
//
//      void    InitRun();
//
//              Called at the beginning of each new run.
//              Define code masks for "good" particles here.
//
//      Bool_t  Analysis();
//
//              The analysis of the event.
//              Return value: kTRUE normally, kFALSE skips to the next run (?)
//
//      void    EndRun();
//
//              Called at the end of each run.
//
//      void    EndAnalysis();
//
//              Called at the end of the analysis.
//              Write histograms, TTrees to TFiles, etc.
//
//Use the static method KVOldINDRASelector::Make("MySelector") in order to generate a
//template for your analysis class with the correct declarations for the above
//methods. MySelector.C and MySelector.h will be generated.
//
//Using Options
//=============
//
//Using Global Variables
//======================
//Many of the most frequently used global variables have been implemented as
//KaliVeda classes, all of which derive from the base class KVVarGlob. The KVOldINDRASelector
//class provides a semi-automatic way to calculate and use these variables, via
//an internal list of global variables (KVGVList class). In order to use a
//global variable in your analysis, declare it in your InitAnalysis() method,
//its value will automatically be calculated for each event analysed.
//Note that only particles with KVINDRAReconNuc::IsOK()==kTRUE are included in the calculation.
//
//For example, let us suppose you want to calculate the isotropy ratio Riso for
//each event. In InitAnalysis() you should add the line:
//
//     AddGV( "KVRiso", "riso" );
//
//This will create a new KVRiso global variable object with the name "riso"
//(used to retrieve the object from the list when you want to know its value)
//and add it to the internal list of global variables to be calculated.
//
//In your event-by-event Analysis() method, you can retrieve the value of
//the global variable using e.g.
//
//     Double_t rapport = GetGV("riso")->GetValue();
//
//This gives the value of the isotropy ratio for the currently analysed event.
//
//
//More examples can be found in the AnalyseCamp1 analysis class.
//
//Manage histograms
//======================
//Few methods are defined to make easier the management
//of histograms
//Redefine the method CreateHistos() and create the histograms
//like this
//lhisto->Add(new TH1F(...));
//lhisto->Add(new TH2F(...));
//lhisto->Add(new TProfile(...));
//All histograms are stored in an "owner" KVHashList "lhisto"
//
//The method FillHisto(KVString sname,Double_t one,Double_t two,Double_t three,Double_t four)
//allows to fill any histogram declared as shown above
//you only need to give the histogram name followed by the arguments you use normally in the Fill method
//of TH1::Fill() and derivated
//
//Finally, to write the list of histograms in a file
//Just call the WriteHistoToFile method indicating the name of the file (or not)
//
//
//Manage trees
//======================
//Few methods are defined to make easier the management
//of trees
//Redefine the method CreateTrees() and create the histograms
//like this
//TTree* tt = 0;
//lhisto->Add(new TTree(...));
//tt = (TTree* )lhisto->Last()
//tt->Branch(...)
//tt->Branch(...)
//All histograms are stored in an "owner" KVHashList "ltree"
//
//The method FillTree(KVString sname="")
//allows to fill tree(s) according to the name passed as argument
//if "" is given all trees are filled
//
//Finally, to write the list of trees in a file
//Just call the WriteTreeToFile method indicating the name of the file (or not)
//
//////////////////////////////////////////////////////////////////

KVOldINDRASelector::KVOldINDRASelector(TTree*)
{
   //ctor
   fChain = 0;
   callnotif = 0;
   gvlist = 0;                  // Global variable list set to nul.
   lhisto = new KVHashList();
   ltree = new KVHashList();
   //create stopwatch
   fTimer = new TStopwatch;
   // event list
   fEvtList = 0;
   fTEVLexist = 0;
   fKVDataSelector = 0;
   fDataSelector = "";
   needToSelect = kFALSE;
   needToCallEndRun = kFALSE;
   fCurrentRun = 0;
   fPartCond = 0;
   data = 0;
   dataselector_lock.SetTimeout(60);   // 60-second timeout in case of problems
   dataselector_lock.SetSuspend(5);   // suspension after timeout
   dataselector_lock.SetSleeptime(1);   // try lock every second
}

KVOldINDRASelector::~KVOldINDRASelector()
{
   //dtor
   //delete global variable list if it belongs to us, i.e. if created by a
   //call to GetGVList
   if (gvlist && TestBit(kDeleteGVList)) {
      delete gvlist;
      gvlist = 0;
      ResetBit(kDeleteGVList);
   }
   delete fTimer;
   SafeDelete(fPartCond);
   lhisto->Clear();
   delete lhisto;
   ltree->Clear();
   delete ltree;
}

void KVOldINDRASelector::Init(TTree* tree)
{
   if (fChain) return; //Init has already been called

   //delete any status file from previous job with same name from launch directory
   gDataAnalyser->DeleteBatchStatusFile();

   if (!tree) return;

   //   Set branch addresses
   fChain = tree;
   fChain->SetMakeClass(1);

   if (fChain->InheritsFrom("TChain"))
      fTreeOffset = ((TChain*) fChain)->GetTreeOffset();
   else
      fTreeOffset = 0;

   data = 0;
   b_data = 0;
   fChain->SetBranchAddress(fBranchName.Data(), &data, &b_data);

//
// Builds a TEventList by adding the contents of the lists for each run
//
   BuildEventList();
   if (fKVDataSelector) {       // Init of the KVDataSelector if needed
      fKVDataSelector->Init();
   }
   // tell the data analyser who we are
   gDataAnalyser->RegisterUserClass(this);
   gDataAnalyser->preInitAnalysis();
   InitAnalysis();              //user initialisations for analysis
   gDataAnalyser->postInitAnalysis();

   if (gvlist) {
      gvlist->Init();
   }
}

Bool_t KVOldINDRASelector::Notify()
{
   // Called when loading a new file.
   // Get branch pointers.

   cout << "Analyse du fichier " << fChain->GetCurrentFile()->GetName()
        << " : " << fChain->GetTree()->GetEntries() << endl;
   NbTreeEntry = (Int_t) fChain->GetTree()->GetEntries();
   fCurrentTreeNumber = fChain->GetTreeNumber();

   needToCallEndRun = kTRUE;

   if (fEvtList)
      needToSelect = !(fTEVLexist[fCurrentTreeNumber]);
   else
      needToSelect = kFALSE;

   gDataAnalyser->preInitRun(); // will initialize fCurrentRun

   if (needToSelect) {
      if (!fKVDataSelector) {
         LoadDataSelector();
      }
      fKVDataSelector->Reset(fCurrentRun->GetNumber());
      cout << " Building new TEventList : " << fKVDataSelector->
           GetTEventList()->GetName()
           << endl;
   }

   InitRun();                   //user initialisations for run
   gDataAnalyser->postInitRun();
   return kTRUE;
}

void KVOldINDRASelector::Begin(TTree*)
{
   // Function called before starting the event loop.
   // When running with PROOF Begin() is only called in the client.

   // Get the option and the name of the DataSelector if needed
   ParseOptions();

   if (IsOptGiven("DataSelector")) SetDataSelector(GetOpt("DataSelector"));
   else
      SetDataSelector();

   totentry = 0;

   //start stopwatch, after first resetting it (in case this is not the first time the analysis is run)
   fTimer->Start(kTRUE);
}

void KVOldINDRASelector::SlaveBegin(TTree*)
{
   // Function called before starting the event loop.
   // When running with PROOF SlaveBegin() is called in each slave
   // Initialize the tree branches.

   /*    Init(tree);

      TString option = GetOption();
    */
}

#ifdef __WITHOUT_TSELECTOR_LONG64_T
Bool_t KVOldINDRASelector::Process(Int_t entry) //for ROOT versions < 4.01/01
#else
Bool_t KVOldINDRASelector::Process(Long64_t entry)      //for ROOT versions > 4.00/08
#endif
{
   //Here the event is read into memory and checks are made on the number
   //of events read.
   //Particles not having the correct identification and calibration quality codes
   //are excluded from the analysis (IsOK()=kFALSE).
   //Particles which do not satisfy the additional selection criteria set with
   //SetParticleConditions() are excluded from the analysis (IsOK()=kFALSE).
   //If ChangeFragmentMasses() was called in InitAnalysis() or InitRun()
   //then the non-measured fragment masses will be recalculated with the
   //new mass formula and laboratory energies adjusted accordingly.
   //Then the reaction CM momenta of all "OK" particles are calculated and
   //the list of global variables (if one is defined) is filled.

   if (gDataAnalyser->AbortProcessingLoop()) {
      // abort requested by batch system
      Abort(Form("Aborting analysis after %d events", totentry), kAbortFile);
      return kFALSE;
   }

   fTreeEntry = entry;

   if (gDataAnalyser->CheckStatusUpdateInterval(totentry)) gDataAnalyser->DoStatusUpdate(totentry);

   // read event
   fChain->GetTree()->GetEntry(fTreeEntry);
   // read raw data associated to event
   gDataAnalyser->preAnalysis();

   //additional selection criteria ?
   if (fPartCond) {
      KVNucleus* part = 0;
      while ((part = (KVNucleus*)GetEvent()->GetNextParticle("ok"))) {

         part->SetIsOK(fPartCond->Test(part));

      }
   }

   //change masses ?
   if (TestBit(kChangeMasses)) GetEvent()->ChangeFragmentMasses(fNewMassFormula);

   totentry++;

   //calculate momenta of particles in reaction cm frame
   if (fCurrentRun->GetSystem() && fCurrentRun->GetSystem()->GetKinematics()) {

      GetEvent()->SetFrame("CM",
                           fCurrentRun->GetSystem()->GetKinematics()->GetCMVelocity());

   }

   RecalculateGlobalVariables();


   Bool_t ok_anal = kTRUE;

   if (needToSelect) {
      ok_anal = fKVDataSelector->ProcessCurrentEntry(); //Data Selection and user analysis
   } else
      ok_anal = Analysis();     //user analysis
   gDataAnalyser->postAnalysis();

   // Testing whether EndRun() should be called
   if (AtEndOfRun()) {
      TString mes("End of run after ");
      mes += (totentry);
      mes += " events.";
      Info("Process", "%s", mes.Data());

      gDataAnalyser->preEndRun();
      EndRun();                 //user routine end of run
      gDataAnalyser->postEndRun();
      needToCallEndRun = kFALSE;

      // save the new Built TEventList
      if (needToSelect) {
         SaveCurrentDataSelection();
         needToSelect = kFALSE;
      }
   }

   return ok_anal;
}

void KVOldINDRASelector::SlaveTerminate()
{
   // Function called at the end of the event loop in each PROOF slave.


}

void KVOldINDRASelector::Terminate()
{
   // Function called at the end of the event loop.
   // When running with PROOF Terminate() is only called in the client.

   fTimer->Stop();//stop stopwatch straight away so that 'Events/CPU sec' etc. only
   //includes time actually spent analysing data

   if (needToCallEndRun) {
      gDataAnalyser->preEndRun();
      EndRun();
      gDataAnalyser->postEndRun();
   }

   if (needToSelect) {
      SaveCurrentDataSelection();
   }

   cout << endl << " ====================== END ====================== " <<
        endl << endl;
   cout << "   Total number of events read     = " << totentry << endl;
   cout << "   Real time = " << fTimer->RealTime() << " sec." << endl;
   cout << "    CPU time = " << fTimer->CpuTime() << " sec." << endl;
   cout << "   Events/Real sec. = " << totentry /
        fTimer->RealTime() << endl;
   cout << "    Events/CPU sec. = " << totentry /
        fTimer->CpuTime() << endl;
   cout << endl << " ====================== END ====================== " <<
        endl << endl;

   if (fEvtList) {
      fChain->SetEventList(0);
      delete fEvtList;
      fEvtList = 0;
   }

   gDataAnalyser->preEndAnalysis();
   EndAnalysis();               //user end of analysis routine
   gDataAnalyser->postEndAnalysis();

   //delete job status file from $HOME directory
   gDataAnalyser->DeleteBatchStatusFile();
}

void KVOldINDRASelector::Make(const Char_t* kvsname)
{
   // Automatic generation of KVOldINDRASelector-derived class for KaliVeda analysis
   KVClassFactory cf(kvsname, "User analysis class", "KVOldINDRASelector", kTRUE);
   cf.AddImplIncludeFile("KVINDRAReconNuc.h");
   cf.AddImplIncludeFile("KVBatchSystem.h");
   cf.AddImplIncludeFile("KVINDRA.h");
   cf.GenerateCode();
}

void KVOldINDRASelector::SetGVList(KVGVList* list)
{
   //Use a user-defined list of global variables for the analysis.
   //In this case it is the user's responsibility to delete the list
   //at the end of the analysis.
   gvlist = list;
}

KVGVList* KVOldINDRASelector::GetGVList(void)
{
   //Access to the internal list of global variables
   //If the list does not exist, it is created.
   //In this case it will be automatically deleted with the KVOldINDRASelector object.
   if (!gvlist) {
      gvlist = new KVGVList;
      SetBit(kDeleteGVList);
   }
   return gvlist;
}

void KVOldINDRASelector::AddGV(KVVarGlob* vg)
{
   //Add the global variable "vg" to the list of variables for the analysis.
   //This is equivalent to GetGVList()->Add( vg ).
   if (!vg)
      Error("AddGV(KVVarGlob*)", "KVVarGlob pointer is null");
   else
      GetGVList()->Add(vg);
}

KVVarGlob* KVOldINDRASelector::GetGV(const Char_t* name) const
{
   //Access the global variable with name "name" in the list of variables
   //for the analysis.
   //This is equivalent to GetGVList()->GetGV( name ).

   return (const_cast < KVOldINDRASelector* >(this)->GetGVList()->GetGV(name));
}

KVVarGlob* KVOldINDRASelector::AddGV(const Char_t* class_name,
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

void KVOldINDRASelector::RecalculateGlobalVariables()
{
   //Use this method if you change e.g. the acceptable particle identification codes in your
   //Analysis() method and want to recalculate the values of all global variables
   //for your new selection.
   //
   //WARNING: the global variables are calculated automatically for you for each event
   //before method Analysis() is called. In order for the correct particles to be included in
   //this calculation, make sure that at the END of Analysis() you reset the selection
   //criteria.
   //
   //i.e. if in your InitAnalysis() you have:
   //      GetEvent()->AcceptIDCodes(kIDCode2|kIDCode3|kIDCode4);
   //
   //at the beginning of Analysis() the variables have been calculated for all particles
   //with codes 2-4. If you store these values somewhere and then change the code mask:
   //
   //      GetEvent()->AcceptIDCodes(kIDCode2|kIDCode3|kIDCode4|kIDCode6);
   //
   //you can recalculate the global variable list for particles with codes 2-4 & 6:
   //
   //      RecalculateGlobalVariables();
   //
   //then at the end of Analysis(), after storing the new values of the variables, you
   //should reset the code mask:
   //
   //      GetEvent()->AcceptIDCodes(kIDCode2|kIDCode3|kIDCode4);
   //
   //which will be used for the next event read for processing.

   if (gvlist) gvlist->CalculateGlobalVariables(GetEvent());
}

//____________________________________________________________________________

#ifdef __WITHOUT_TSELECTOR_LONG64_T
Int_t KVOldINDRASelector::GetTreeEntry() const
#else
Long64_t KVOldINDRASelector::GetTreeEntry() const
#endif
{
   //During event analysis (i.e. inside the Analysis() method), this gives the current TTree
   //entry number, in other words the argument passed to TSelector::Process(Long64_t entry).

   return fTreeEntry;
}

//____________________________________________________________________________
void KVOldINDRASelector::BuildEventList(void)
{
//
// Builds the event list of the TChain by adding the event lists of each TTree.
// The event list contain the entry number of the TChain
//

   if (fEvtList) {
      fEvtList->Reset();
      delete fEvtList;
      fEvtList = 0;
   }
   if (fKVDataSelector) {
      delete fKVDataSelector;
      fKVDataSelector = 0;
   }
   if (fTEVLexist) {
      delete[]fTEVLexist;
      fTEVLexist = 0;
   }

   if (fDataSelector.Length()) {
      Info("BuildEventList()",
           "Building TEventList for the KVDataSelector \"%s\".",
           GetDataSelector());
      TObjArray* lof = 0;
      Long64_t* toff = 0;
      if (fTreeOffset) {
         cout << "Analysis from a chain of trees." << endl;
         lof = ((TChain*) fChain)->GetListOfFiles();
         toff = fTreeOffset;
      } else {
         cout << "Analysis from a single tree." << endl;
         lof = new TObjArray();
         lof->
         Add(new
             TNamed("SingleRun", fChain->GetCurrentFile()->GetName()));
         toff = new Long64_t[2];
         toff[0] = 0;
         toff[1] = fChain->GetEntries();
      }

      fEvtList =
         new TEventList("chainEventList", "TEvent list for this TChain");


      // Opening the file which contains the TEventLists
      TString searchname(GetDataSelectorFileName());
      TString fname = searchname;
      cout << "File to open : " << fname.Data() << endl;
      TFile* fileDataSelector = 0;
      if (KVBase::FindFile("", searchname)) {
         //get lock on file to avoid interference with other processes
         if (dataselector_lock.Lock(fname.Data())) fileDataSelector = new TFile(fname.Data());
      } else {
         cout << "The file \"" << fname.
              Data() << "\" does not exist." << endl;
      }

      fTEVLexist = new Bool_t[lof->GetEntries()];
      for (Int_t tn = 0; tn < lof->GetEntries(); tn++) {
         fTEVLexist[tn] = kFALSE;
         TString fname(((TNamed*) lof->At(tn))->GetTitle());
         cout << fname.Data() << endl;
         Int_t nrun = dynamic_cast<KVDataSetAnalyser*>(gDataAnalyser)->GetRunNumberFromFileName(fname.Data());
         if (nrun) {
            cout << "Numero de run " << nrun << endl;
            cout << "Recherche de " << Form("%s_run%d;1",
                                            GetDataSelector(),
                                            nrun) << endl;
            TEventList* revtList = 0;
            if (fileDataSelector) {
               revtList =
                  (TEventList*) fileDataSelector->
                  Get(Form("%s_run%d;1", GetDataSelector(), nrun));
            }
            if (revtList) {
               for (Int_t i = 0; i < revtList->GetN(); i++) {
                  fEvtList->Enter(fTreeOffset[tn] + revtList->GetEntry(i));
               }
               fTEVLexist[tn] = kTRUE;
            } else {
               cout << "Liste introuvable..." << endl;
               for (Int_t i = fTreeOffset[tn]; i < fTreeOffset[tn + 1];
                     i++) {
                  fEvtList->Enter(i);
               }
            }
         } else {
            cout << "Run inconnu..." << endl;
         }
      }
      if (fileDataSelector) {
         cout << "Closing " << fileDataSelector->GetName() << endl;
         fileDataSelector->Close();
         dataselector_lock.Release();//unlock file
      }

      if (!fTreeOffset) {
         delete[]toff;
         lof->SetOwner(kTRUE);
         delete lof;
      }
      // Check if the TEventList is empty. If yes, all events will be read
      if (!fEvtList->GetN()) {
         Warning("BuildEventList()", "The TEventList is empty...");
         delete fEvtList;
         fEvtList = 0;
      }
   }
   fChain->SetEventList(fEvtList);
}

//____________________________________________________________________________
Bool_t KVOldINDRASelector::AtEndOfRun(void)
{
//
// Check whether the end of run is reached for the current tree
//

   Bool_t ok = (fTreeEntry + 1 == fChain->GetTree()->GetEntries());

   if (fEvtList) {
      Long64_t globEntry = 0;
      if (fTreeOffset)
         globEntry = ((TChain*) fChain)->GetChainEntryNumber(fTreeEntry);
      else
         globEntry = fTreeEntry;
      Int_t index = fEvtList->GetIndex(globEntry);
      Long64_t nextEntry = (fEvtList->GetEntry(index + 1));
      ok = ok || (nextEntry == -1);
      if (fTreeOffset) {
         ok = ok || (nextEntry >= fTreeOffset[fCurrentTreeNumber + 1]);
      }
   }

   return ok;
}

//____________________________________________________________________________
void KVOldINDRASelector::LoadDataSelector(void)
{
//
// Set the pointer of the KVDataSelector according to its name
//
   cout << "Loading KVDataSelector..." << endl;
   Bool_t deleteSources = kFALSE;
   if (fDataSelector.Length()) {
      fKVDataSelector = 0;
      TClass* clas = gROOT->GetClass(fDataSelector.Data());
      cout << clas << " / " << fDataSelector.Data() << endl;
      if (!clas) {
         //if the class is not present, first check whether the declaration and
         //the implementation files are present in the user's working directory
         TString fileC(Form("%s.cpp", fDataSelector.Data()));
         TString fileh(Form("%s.h", fDataSelector.Data()));

#ifdef __WITHOUT_TMACRO
         if (gSystem->AccessPathName(fileC.Data()) || gSystem->AccessPathName(fileh.Data())) {
            Warning("LoadDataSelector(void)",
                    Form
                    ("No implementation and/or declaration file found for \"%s\".",
                     fDataSelector.Data()));
#else
         TMacro mC;
         if (!mC.ReadFile(fileC.Data()) || !mC.ReadFile(fileh.Data())) {

            // Load .cpp and .h files from the TEventList's root file
            TString searchname(GetDataSelectorFileName());
            TString fname = searchname;
            TFile* fileDataSelector = 0;

            if (KVBase::FindFile("", fname)) {
               if (dataselector_lock.Lock(fname.Data())) fileDataSelector = new TFile(fname.Data());
            } else {
               cout << "The file \"" << fname.
                    Data() << "\" does not exist." << endl;
            }
            TMacro* macC = 0;
            TMacro* mach = 0;
            if (fileDataSelector) {
               macC = (TMacro*) fileDataSelector->Get(fileC.Data());
               mach = (TMacro*) fileDataSelector->Get(fileh.Data());
            }
            if (macC && mach) {
               macC->SaveSource(fileC.Data());
               cout << fileC.Data() << " re-generated" << endl;
               mach->SaveSource(fileh.Data());
               cout << fileh.Data() << " re-generated." << endl;
               deleteSources = kTRUE;
            } else {
               Warning("LoadDataSelector(void)",
                       "No implementation and/or declaration file found for \"%s\".",
                       fDataSelector.Data());
            }
            if (fileDataSelector) {
               fileDataSelector->Close();
               dataselector_lock.Release();
            }
#endif
         } else {
            cout << "Files " << fileC.Data() << " and " << fileh.Data() <<
                 " found." << endl;
         }
         //class not in dictionary - user-defined class ? Add a plugin.
         gROOT->GetPluginManager()->AddHandler("KVDataSelector",
                                               fDataSelector.Data(),
                                               fDataSelector.Data(),
                                               Form("%s.cpp+",
                                                     fDataSelector.Data()),
                                               Form("%s()",
                                                     fDataSelector.Data()));
         TPluginHandler* ph =
            KVBase::LoadPlugin("KVDataSelector", fDataSelector.Data());
         if (!ph) {
            //not found
            Error("LoadDataSelector(void)",
                  "Called with class_name=%s.\nClass is unknown: not in standard libraries, and plugin (user-defined class) not found",
                  fDataSelector.Data());
         } else {
            fKVDataSelector = (KVDataSelector*) ph->ExecPlugin(0);
            fKVDataSelector->SetKVSelector(this);
            cout << "Apres Plugin " << fKVDataSelector->IsA()->
                 GetName() << " : " << fKVDataSelector->
                 GetKVSelector() << endl;
         }

      } else if (!clas->InheritsFrom("KVDataSelector")) {
         Error("LoadDataSelector(void)",
               "%s is not a valid class deriving from KVDataSelector.",
               fDataSelector.Data());
      } else {
         fKVDataSelector = (KVDataSelector*) clas->New();
         fKVDataSelector->SetKVSelector(this);
         cout << "Apres clas->New() " << fKVDataSelector->IsA()->
              GetName() << " : " << fKVDataSelector->
              GetKVSelector() << endl;
      }
      if (deleteSources) {
         cout << "Removing files \"" << fDataSelector.
              Data() << "*.*\" ..." << endl;
         gSystem->
         Exec(Form
              ("rm %s.[c,h]*", fDataSelector.Data()));
      }
   } else {
      fKVDataSelector = 0;
   }
   if (fKVDataSelector)
      fKVDataSelector->Init();
}

//____________________________________________________________________________
void KVOldINDRASelector::SaveCurrentDataSelection(void)
{
//
// Save the newly built TEventList in the DataSelection root file
//
   if (fKVDataSelector) {
      TFile* curfile = gFile;
      TString option;
      TString searchname(GetDataSelectorFileName());
      TString fname = searchname;
      if (KVBase::FindFile("", searchname)) {
         option = "update";
      } else {
         option = "recreate";
      }
      cout << "File to open : " << fname.Data() << " : " << option.
           Data() << endl;
      TFile* fileDataSelector = 0;
      if (dataselector_lock.Lock(fname.Data())) fileDataSelector = new TFile(fname.Data(), option.Data());
      if (!fileDataSelector || (fileDataSelector && fileDataSelector->IsZombie())) {
         Error("SaveCurrentDataSelection(void)",
               "Opening the file \"%s\" with the option \"%s\" is not possible.\nNothing saved.",
               fname.Data(), option.Data());
         gFile = curfile;
         return;
      }
      cout << "Saving the TEventList \"" << fKVDataSelector->
           GetTEventList()->GetName() << "\"..." << endl;
      cout << fKVDataSelector->GetTEventList()->
           GetN() << " entries selected." << endl;
      fKVDataSelector->GetTEventList()->Write();

#ifndef __WITHOUT_TMACRO
      TObject
      * oC =
         fileDataSelector->
         Get(Form("%s.cpp;1", fKVDataSelector->IsA()->GetName()));
      TObject* oh =
         fileDataSelector->
         Get(Form("%s.h;1", fKVDataSelector->IsA()->GetName()));
      if (!oC || !oh) {
         cout << "Saving the source files..." << endl;
         TMacro mC(Form("%s.cpp", fKVDataSelector->IsA()->GetName()));
         mC.SetName(Form("%s.cpp", fKVDataSelector->IsA()->GetName()));
         mC.SetTitle(Form
                     ("Implementation file for the KVDataSelector \"%s\".",
                      fKVDataSelector->IsA()->GetName()));
         mC.Write();
         TMacro mh(Form("%s.h", fKVDataSelector->IsA()->GetName()));
         mh.SetName(Form("%s.h", fKVDataSelector->IsA()->GetName()));
         mh.SetTitle(Form
                     ("Declaration file for the KVDataSelector \"%s\".",
                      fKVDataSelector->IsA()->GetName()));
         mh.Write();
      }
#endif

      cout << "Done" << endl;
      fileDataSelector->Close();
      dataselector_lock.Release();
      gFile = curfile;
   } else {
      cout << "No TEventList to save..." << endl;
   }
}

//____________________________________________________________________________
const Char_t* KVOldINDRASelector::GetDataSelectorFileName(void)
{
//
// Gets the name of the file where the TEventLists and the KVDataSelectors
// are stored.
//
   static TString fname;
   fname = "";
   fname =
      gEnv->GetValue("DataSelector.directory",
                     gSystem->WorkingDirectory());
   fname += "/";
   fname +=
      gEnv->GetValue("DataSelector.fileName", "ListOfDataSelector.root");
   cout << "List Of Data Selectors : " << fname.Data() << endl;
   return fname.Data();
}

//____________________________________________________________________________

void KVOldINDRASelector::ChangeFragmentMasses(UInt_t mass_formula)
{
   //Call this method in your InitAnalysis() if you want to replace the masses of nuclei
   //in each event with masses calculated from the given formula (see KVNucleus::GetAFromZ).
   //
   //As each event is read, the masses of nuclei with calculated (not measured) masses
   //will be replaced with the new value, and the energy of the nucleus will be replaced with
   //a new value taking into account the effect of the change in mass on the CsI energy
   //calibration (if the particle stops in a CsI detector).
   //
   //NB: the "CM" frame momenta/energies/angles calculated automatically before your
   //Analysis() method is called will be calculated using the new masses and lab energies.
   //Any subsequent change to the masses and/or energies you make in Analysis() will not
   //change the "CM" momenta. If you change the laboratory energies etc. you must recalculate
   //the c.m. momenta by calling e.g. GetEvent()->SetFrame("CM", ... )
   SetBit(kChangeMasses);
   fNewMassFormula = mass_formula;
}

//____________________________________________________________________________

void KVOldINDRASelector::SetParticleConditions(const KVParticleCondition& cond)
{
   //Use this method to set criteria for selecting particles (other than the identification
   //or calibration quality codes - see KVINDRAReconEvent::AcceptIDCodes and
   //KVINDRAReconEvent::AcceptECodes).
   //
   //The criteria defined in the KVParticleCondition object will be applied to every
   //particle which has the right quality codes and if they are not satisfied the particle's
   //"OK" flag will be set to false, i.e. the particle's IsOK() method will return kFALSE,
   //and the particle will not be included in iterations such as GetEvent()->GetNextParticle("OK").
   //Neither will the particle be included in the evaluation of any global variables or event
   //selection criteria (see KVDataSelector).
   //
   //This method must be called in the user's InitAnalysis() or InitRun() method.
   //
   //EXAMPLES:
   //To include only particles having a time marker between 90 and 110:
   //
   //       KVParticleCondition cd("_NUC_->GetTimeMarker()>=90 && _NUC_->GetTimeMarker()<=110");
   //       SetParticleConditions( cd );
   //
   //To include only Z=1 particles having a time marker between 90 and 110, Z>1 particles
   //with time markers between 80 and 120:
   //
   //       KVParticleCondition tm1("_NUC_->GetTimeMarker()>=90 && _NUC_->GetTimeMarker()<=110");
   //       KVParticleCondition tm2("_NUC_->GetTimeMarker()>=80 && _NUC_->GetTimeMarker()<=120");
   //       KVParticleCondition z1("_NUC_->GetZ()==1");
   //       KVParticleCondition zgt1("_NUC_->GetZ()>1");
   //       KVParticleCondition cd = (z1 && tm1) || (zgt1 && tm2);
   //       SetParticleConditions( cd );
   if (!fPartCond) fPartCond = new KVParticleCondition(cond);
   else *fPartCond = cond;
   //set name of class to which we cast. this is for optimization to work
   fPartCond->SetParticleClassName("KVINDRAReconNuc");
}

//____________________________________________________________________________

KVHashList* KVOldINDRASelector::GetHistoList()
{

   return lhisto;

}

//____________________________________________________________________________

TH1* KVOldINDRASelector::GetHisto(const Char_t* histo_name)
{

   return (TH1*)lhisto->FindObject(histo_name);

}

//____________________________________________________________________________

void KVOldINDRASelector::FillHisto(KVString sname, Double_t one, Double_t two, Double_t three, Double_t four)
{

   //Find in the list, if there is an histogram named "sname"
   //If not print an error message
   //If yes redirect to the right method according to its closest mother class
   //to fill it
   TH1* h1 = 0;
   if ((h1 = GetHisto(sname.Data()))) {
      if (h1->InheritsFrom("TH3"))
         FillTH3((TH3*)h1, one, two, three, four);
      else if (h1->InheritsFrom("TProfile2D"))
         FillTProfile2D((TProfile2D*)h1, one, two, three, four);
      else if (h1->InheritsFrom("KVDalitzPlot"))
         FillKVDalitz((KVDalitzPlot*)h1, one, two, three);
      else if (h1->InheritsFrom("TH2"))
         FillTH2((TH2*)h1, one, two, three);
      else if (h1->InheritsFrom("TProfile"))
         FillTProfile((TProfile*)h1, one, two, three);
      else if (h1->InheritsFrom("TH1"))
         FillTH1(h1, one, two);
      else
         Warning("FillHisto", "%s -> Classe non prevue ...", lhisto->FindObject(sname.Data())->ClassName());
   } else {
      Warning("FillHisto", "%s introuvable", sname.Data());
   }

}

//____________________________________________________________________________

void KVOldINDRASelector::FillTH1(TH1* h1, Double_t one, Double_t two)
{

   h1->Fill(one, two);

}

//____________________________________________________________________________

void KVOldINDRASelector::FillTProfile(TProfile* h1, Double_t one, Double_t two, Double_t three)
{

   h1->Fill(one, two, three);

}

//____________________________________________________________________________

void KVOldINDRASelector::FillTH2(TH2* h2, Double_t one, Double_t two, Double_t three)
{

   h2->Fill(one, two, three);

}

//____________________________________________________________________________

void KVOldINDRASelector::FillTProfile2D(TProfile2D* h2, Double_t one, Double_t two, Double_t three, Double_t four)
{

   h2->Fill(one, two, three, four);
}

//____________________________________________________________________________

void KVOldINDRASelector::FillKVDalitz(KVDalitzPlot* h2, Double_t one, Double_t two, Double_t three)
{

   h2->FillAsDalitz(one, two, three);
}

//____________________________________________________________________________

void KVOldINDRASelector::FillTH3(TH3* h3, Double_t one, Double_t two, Double_t three, Double_t four)
{

   h3->Fill(one, two, three, four);
}


//____________________________________________________________________________

void KVOldINDRASelector::CreateHistos()
{

   Warning("CreateHistos", "To be redefined child class");

}

//____________________________________________________________________________

void KVOldINDRASelector::WriteHistoToFile(KVString filename, Option_t* option)
{

   //If no filename is specified, assume that the current directory is writable
   //if filename correspond to an already opened file, write in it
   //if not open/create it, depending on the option ("recreate" by default)
   //and write in it
   Bool_t IsCreated = kFALSE;
   if (filename == "") {
      GetHistoList()->Write();
   } else {
      TFile* file = 0;
      if (!(file = (TFile*)gROOT->GetListOfFiles()->FindObject(filename.Data()))) {
         IsCreated = kTRUE;
         file = new TFile(filename.Data(), option);
      }
      file->cd();
      GetHistoList()->Write();
      if (IsCreated) file->Close();
   }

}

//____________________________________________________________________________

KVHashList* KVOldINDRASelector::GetTreeList()
{

   return ltree;

}

//____________________________________________________________________________

TTree* KVOldINDRASelector::GetTree(const Char_t* tree_name)
{

   return (TTree*)ltree->FindObject(tree_name);

}
//____________________________________________________________________________

void KVOldINDRASelector::CreateTrees()
{

   Warning("CreateTrees", "To be redefined child class");

}

//____________________________________________________________________________

void KVOldINDRASelector::FillTree(KVString sname)
{

   if (sname == "") {
      ltree->Execute("Fill", "");
   } else {
      TTree* tt = 0;
      if ((tt = GetTree(sname.Data()))) {
         tt->Fill();
      } else {
         Warning("FillTree", "%s introuvable", sname.Data());
      }
   }

}

//____________________________________________________________________________

void KVOldINDRASelector::WriteTreeToFile(KVString filename, Option_t* option)
{

   //If no filename is specified, assume that the current directory is writable
   Bool_t IsCreated = kFALSE;
   if (filename == "") {
      GetTreeList()->Write();
   } else {
      TFile* file = 0;
      //if filename correspond to an already opened file, write in it
      //if not open/create it, depending on the option ("recreate" by default)
      //and write in it
      if (!(file = (TFile*)gROOT->GetListOfFiles()->FindObject(filename.Data()))) {
         IsCreated = kTRUE;
         file = new TFile(filename.Data(), option);
      }
      file->cd();
      GetTreeList()->Write();
      if (IsCreated) file->Close();
   }

}

void KVOldINDRASelector::SetOpt(const Char_t* option, const Char_t* value)
{
   //Set a value for an option
   KVString tmp(value);
   fOptionList.SetValue(option, tmp);
}

//_________________________________________________________________

Bool_t KVOldINDRASelector::IsOptGiven(const Char_t* opt)
{
   // Returns kTRUE if the option 'opt' has been set

   return fOptionList.HasParameter(opt);
}

//_________________________________________________________________

TString KVOldINDRASelector::GetOpt(const Char_t* opt) const
{
   // Returns the value of the option
   // Only use after checking existence of option with IsOptGiven(const Char_t* opt)

   return fOptionList.GetTStringValue(opt);
}

//_________________________________________________________________

void KVOldINDRASelector::UnsetOpt(const Char_t* opt)
{
   // Removes the option 'opt' from the internal lists, as if it had never been set

   fOptionList.RemoveParameter(opt);
}

void KVOldINDRASelector::ParseOptions()
{
   // Analyse comma-separated list of options given to TTree::Process
   // and store all "option=value" pairs in fOptionList.
   // Options can then be accessed using IsOptGiven(), GetOptString(), etc.
   // This method is called by Begin

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
}
