#ifndef KVEventSelector_h
#define KVEventSelector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include "KVEvent.h"
#include "KVGVList.h"
#include "KVString.h"
#include "KVParticleCondition.h"
#include "TDatime.h"
#include "KVHashList.h"
#include <TH3.h>
#include <TH2.h>
#include <TProfile2D.h>
#include "KVNameValueList.h"
#include "TProofOutputFile.h"
#include "KVDataAnalyser.h"

/**
\class KVEventSelector
\brief General purpose analysis class for TTree containing KVEvent objects
\ingroup Analysis

 Use this TSelector to analyse data in TTrees containing a branch with KVEvent-derived objects:

~~~~~~~~~~~~~~~~
    my_tree->Process("KVEventSelector", "[options]");
~~~~~~~~~~~~~~~~

 where `"[options]"` is the list of options in the form `"BranchName=toto, ..."`.

 The following options MUST be given:

~~~~~~~~~~~~~~~~
    BranchName:                name of branch containing the events
~~~~~~~~~~~~~~~~

 The following are optional options:

~~~~~~~~~~~~~~~~
    EventsReadInterval:     print info on analysis every N events instead of default value
    AuxFiles:               list of files containing "friend" TTrees to be made available during analysis. Separate filenames with '|'.
    AuxDir:                 directory in which to find AuxFiles
    AuxTreeName:            name of tree in AuxFiles containing KVEvent objects
    AuxBranchName:          name of branch in AuxFiles containing KVEvent objects
~~~~~~~~~~~~~~~~

 When `AuxFiles` is used, the user can access the events in these files in her `Analysis()` method
 by doing the following:

~~~~~~~~~~~~
   GetFriendTreeEntry(entry_number);
   KVEvent* friend_event = GetFriendEvent();
~~~~~~~~~~~~

 Any other options can be defined by the user and parsed in her analysis class
 with methods IsOptGiven() and GetOpt()

## Usage
### Global Variables
Global variables for the analysis should be declared in InitAnalysis() using method AddGV().
These variables are calculated automatically for each event __before__ user's Analysis()
method is called.

 If the user needs to define new reference frames for the data which must exist before global variables
 are automatically calculated in the analysis loop, she can do so by overriding the method
 SetAnalysisFrame(), like so:

~~~~~~~~~~~~~~~~
 MyAnalysis::SetAnalysisFrame()
 {
     static TVector3 cmvelocity(0,0,3.0);
     GetEvent()->SetFrame("CM", cmvelocity);
 }
~~~~~~~~~~~~~~~~

Note that the global variables are only calculated using particles which have their "OK" status set,
for example because they correspond to the global particle selection criteria given to SetParticleConditions().
Any further particle selections applied to individual global variables will then select from
among these "OK" particles.

If one or more global variables have event selection conditions defined (i.e. cuts on the values of the
global variables) - see KVVarGlob::SetEventSelection() and KVGVList - then for each event which does not
satisfy all conditions the Analysis() method will __not__ be called.

### Use with PROOF(lite)
 In order to use a KVEventSelector with PROOF:
  - declare any histograms with method AddHisto(TH1*)
    e.g. in InitAnalysis() do:


~~~~~~~~~~~~~~~~
 void MySelector::InitAnalysis()
 {
     AddHisto( new TH2F("toto", "tata", 100, 0, 0, 500, 0, 0) );
 }
~~~~~~~~~~~~~~~~

 Histograms can also be declared 'on the fly' in the Analysis() method in
 the same way;

 - for TTrees, first call CreateTreeFile() with name of file for TTree(s)
 (by default, histograms and TTrees are written in different files - but see below),
 then declare all trees using method AddTree(TTree*)
 e.g. in InitAnalysis() do:

~~~~~~~~~~~~~~~~
 void MySelector::InitAnalysis()
 {
     CreateTreeFile("MyTrees.root");
     TTree* aTree = new TTree("t1", "Some Tree");
     aTree->Branch(...) etc.
     AddTree(aTree);
 }
~~~~~~~~~~~~~~~~

 - if you want (not obligatory), you can use methods FillHisto() and
  FillTree() in your Analysis() method;

 - to save histograms to file in EndAnalysis(), call method
  SaveHistos() in EndAnalysis() with the name of the file to be created

 -  the file declared with CreateTreeFile() will be automatically written
 to disk at the end of the analysis.

### Histograms & TTrees in the same output file
 If you want all results of your analysis to be written in a single file
 containing both histos and trees, call method SetJobOutputFileName() with the required filename in your InitAnalysis() method;

 - do not call SaveHistos() in EndAnalysis(), and make sure you call CreateTreeFile() without giving a name (the
 resulting intermediate file will have a default name allowing it to be found at the end of the analysis)

### Generating & saving profiles, divided histograms, etc.
If, at the end of processing, you want to generate a histogram from one or more histograms filled in your analysis,
for example generate a TProfile from a 2D histogram, or store the result of dividing one histogram by the other,
you need to do the following:

  - do the processing in the EndRun() method, not in EndAnalysis() - by the time EndAnalysis() is called it is too late,
    the output file has been closed and written to disk
  - add the resulting histogram to the output list like so:

~~~~~~~~~~
   MyEventSelector::EndRun()
   {
      TProfile* my_profile = ((TH2*)GetHisto("my_histo_2D"))->ProfileX("name_of_my_profile");
      GetOutputList()->Add(my_profile);
   }
~~~~~~~~~~
*/
////////////////////////////////////////////////////////////////////////////////
class KVEventSelector : public TSelector {

protected :
   TTree*          fChain;   //!pointer to the analyzed TTree or TChain
   TTree*          fAuxChain;//![optional] pointer to another TTree or TChain which may be used during analysis

   // Declaration of leaf types
   KVEvent*     Event;//the events to be analysed
   KVEvent*     AuxEvent;//[optional] events in fAuxChain

   // List of branches
   TBranch*        b_Event;   //!

   //List of global variables
   KVGVList gvlist;            //!

   KVString fBranchName; //name of branch which contains events to analyse

   KVParticleCondition fPartCond;//(optional) conditions for selecting particles
   KVString fPartName;//(optional) classname for upcasting in KVParticleCondition::Optimize

   Bool_t fFirstEvent;//set to kFALSE after first event is read

   Long64_t fEventsRead;//number of events read
   Long64_t fEventsReadInterval;//interval at which to print number of events read
   Long64_t fTreeEntry;//current tree entry number

   KVHashList* lhisto;           //->!
   KVHashList* ltree;            //->!

   Bool_t fNotifyCalled; // avoid multiple calls to Notify/InitRun
   KVString fCombinedOutputFile;// optional name for single results file with trees and histos

   //parsed list of options given to TTree::Process
   KVNameValueList fOptionList;

   Bool_t fDisableCreateTreeFile;//used with PROOF

   void FillTH1(TH1* h1, Double_t one, Double_t two);
   void FillTProfile(TProfile* h1, Double_t one, Double_t two, Double_t three);
   void FillTH2(TH2* h2, Double_t one, Double_t two, Double_t three);
   void FillTProfile2D(TProfile2D* h2, Double_t one, Double_t two, Double_t three, Double_t four);
   void FillTH3(TH3* h3, Double_t one, Double_t two, Double_t three, Double_t four);

   void SetUpAuxEventChain();
   void SetCombinedOutputFile(const TString& filename)
   {
      // Call in InitAnalysis() to set the name of the single output file
      // containing all histograms and TTrees produced by analysis (but see also
      // SetJobOutputFileName).
      //
      // This is equivalent to running the analysis with option
      //
      //~~~~~~~~~~~~~~~
      //    CombinedOutputFile=[filename]
      //~~~~~~~~~~~~~~~
      //
      // but setting this option in InitAnalysis() will not work.
      //
      // Note that if this method is not called/the option is not given,
      // histograms and TTrees will be written in separate files.
      fCombinedOutputFile = filename;
   }
public:
   TFile* writeFile;//!
   TProofOutputFile* mergeFile;//! for merging with PROOF
   Bool_t CreateTreeFile(const Char_t* filename = "");

   virtual void ParseOptions();

   KVEventSelector(TTree* /*tree*/ = 0) : fChain(0), fAuxChain(0), fBranchName("data"), fFirstEvent(kTRUE),
      fEventsRead(0), fEventsReadInterval(100), fNotifyCalled(kFALSE), fDisableCreateTreeFile(kFALSE)
   {
      lhisto = new KVHashList();
      ltree = new KVHashList();
   }
   virtual ~KVEventSelector()
   {
      lhisto->Clear();
      delete lhisto;
      lhisto = 0;
      ltree->Clear();
      delete ltree;
      ltree = 0;
   }
   void SetEventsReadInterval(Long64_t N)
   {
      fEventsReadInterval = N;
   }
   virtual Int_t   Version() const
   {
      return 3;
   }
   virtual void    Begin(TTree* tree);
   virtual void    SlaveBegin(TTree* tree);
   virtual void    Init(TTree* tree);
   void    InitFriendTree(TTree* tree, const TString& branchname);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual void   CheckEndOfRun();
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0)
   {
      return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;
   }
   Int_t GetFriendTreeEntry(Long64_t entry, Int_t getall = 0)
   {
      return fAuxChain ? fAuxChain->GetTree()->GetEntry(entry, getall) : 0;
   }
   KVEvent* GetFriendEvent() const
   {
      return AuxEvent;
   }
   virtual void    SetObject(TObject* obj)
   {
      fObject = obj;
   }
   virtual void    SetInputList(TList* input)
   {
      fInput = input;
   }
   virtual TList*  GetOutputList() const
   {
      return fOutput;
   }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   void SetBranchName(const Char_t* n)
   {
      fBranchName = n;
   }
   const Char_t* GetBranchName() const
   {
      return fBranchName.Data();
   }
   virtual void SetAnalysisFrame()
   {
   }

   KVEvent* GetEvent() const
   {
      return Event;
   }
   void SetEvent(KVEvent* e)
   {
      Event = e;
   }

   /* user entry points */
   virtual void InitAnalysis()
   {
      AbstractMethod("InitAnalysis");
   }
   virtual void InitRun()
   {
      AbstractMethod("InitRun");
   }
   virtual Bool_t Analysis()
   {
      //AbstractMethod("Analysis");
      return kTRUE;
   }
   virtual void EndRun()
   {
      AbstractMethod("EndRun");
   }
   virtual void EndAnalysis()
   {
      AbstractMethod("EndAnalysis");
   }
   KVGVList* GetGVList(void)
   {
      //Access to the internal list of global variables
      return &gvlist;
   }
   const KVGVList* GetGVList(void) const
   {
      //Access to the internal list of global variables
      return &gvlist;
   }
   void AddGV(KVVarGlob* vg)
   {
      //Add the global variable "vg" to the list of variables for the analysis.
      //This is equivalent to GetGVList()->Add( vg ).
      if (!vg)
         Error("AddGV(KVVarGlob*)", "KVVarGlob pointer is null");
      else
         GetGVList()->Add(vg);
   }
   KVVarGlob* AddGV(const Char_t* class_name, const Char_t* name);
   KVVarGlob* GetGV(const Char_t* name) const
   {
      //Access the global variable with name "name" in the list of variables
      //for the analysis.
      //This is equivalent to GetGVList()->GetGV( name ).

      return GetGVList()->GetGV(name);
   }
   virtual void RecalculateGlobalVariables();
   Bool_t AtEndOfRun(void)
   {
      // Check whether the end of run is reached for the current tree
      return (fTreeEntry + 1 == fChain->GetTree()->GetEntries());
   }

   void SetParticleConditions(const KVParticleCondition&, const KVString& = "");
   void SetParticleConditionsParticleClassName(const KVString& t)
   {
      fPartName = t;
   }

   void AddHisto(TH1* histo);
   void AddTree(TTree* tree);

   void FillHisto(const Char_t* sname, Double_t one, Double_t two = 1, Double_t three = 1, Double_t four = 1);
   void FillHisto(const Char_t* sname, const Char_t* label, Double_t weight = 1);
   void FillTree(const Char_t* sname = "");

   const KVHashList* GetHistoList() const;
   const KVHashList* GetTreeList() const;

   TH1* GetHisto(const Char_t* name) const;
   TTree* GetTree(const Char_t* name) const;

   virtual void SaveHistos(const Char_t* filename = "", Option_t* option = "recreate", Bool_t onlyfilled = kFALSE);

   virtual void SetOpt(const Char_t* option, const Char_t* value);
   virtual Bool_t IsOptGiven(const Char_t* option);
   virtual TString GetOpt(const Char_t* option) const;
   virtual void UnsetOpt(const Char_t* opt);

   virtual void SetAdditionalBranchAddress()
   {
      //if user wants to read additional branches of the tree
      //
   }

   void SetJobOutputFileName(const TString& filename)
   {
      // Call in InitAnalysis() to set the name of the single output file
      // containing all histograms and TTrees produced by analysis.
      //
      // For interactive jobs or jos using PROOF, filename will be used for
      // the ROOT file. For jobs using a batch system to execute many
      // jobs in parallel, we use the job name with the '.root' extension.

#ifdef WITH_CPP11
      if (KVDataAnalyser::IsRunningBatchAnalysis() && (gDataAnalyser->GetProofMode() == KVDataAnalyser::EProofMode::None))
#else
      if (KVDataAnalyser::IsRunningBatchAnalysis() && (gDataAnalyser->GetProofMode() == KVDataAnalyser::None))
#endif
         SetCombinedOutputFile(Form("%s.root", gDataAnalyser->GetBatchSystem()->GetJobName()));

      else
         SetCombinedOutputFile(filename);
   }
#ifdef USING_ROOT6
   void SetTriggerConditionsForRun(int);
#endif
   ClassDef(KVEventSelector, 0)//General purpose analysis class for TTrees containing KVEvent objects
};

//! Use with TTree pointer to avoid repetitive strain injury when defining branches
///
/// tree->AddVar(Mult,I);  => tree->Branch("Mult", &Mult, "Mult/I");
/// tree->AddVarBranch(Mult,multiplicity,I);  => tree->Branch("multiplicity", &Mult, "multiplicity/I");
#define AddVar(var,type) Branch(dadastr(var), &var, didixstr(duduvartype(var,type)))
#define AddVarBranch(var,branch,type) Branch(dadastr(branch), &var, didixstr(duduvartype(branch,type)))
#define duduvartype(var,type) var/type
#define didixstr(s) dadastr(s)
#define dadastr(s) #s

#endif
