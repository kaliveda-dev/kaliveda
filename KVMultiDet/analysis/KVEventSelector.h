//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Apr  1 10:33:24 2011 by ROOT version 5.29/01
// from TTree events/events
// found on file: events.root
//////////////////////////////////////////////////////////

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

class KVEventSelector : public TSelector {

protected :
   enum {
      kDeleteGVList = BIT(14),
   };

   TTree*          fChain;   //!pointer to the analyzed TTree or TChain
   TTree*          fAuxChain;//![optional] pointer to another TTree or TChain which may be used during analysis

   // Declaration of leaf types
   KVEvent*     Event;//the events to be analysed
   KVEvent*     AuxEvent;//[optional] events in fAuxChain

   // List of branches
   TBranch*        b_Event;   //!

   //List of global variables
   KVGVList* gvlist;            //!

   KVString fBranchName; //name of branch which contains events to analyse

   KVParticleCondition* fPartCond;//(optional) conditions for selecting particles
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

public:
   TFile* writeFile;//!
   TProofOutputFile* mergeFile;//! for merging with PROOF
   Bool_t CreateTreeFile(const Char_t* filename = "");

   virtual void ParseOptions();

   KVEventSelector(TTree* /*tree*/ = 0) : fChain(0), fAuxChain(0), gvlist(0), fBranchName("data"), fPartCond(0), fFirstEvent(kTRUE),
      fEventsRead(0), fEventsReadInterval(100), fNotifyCalled(kFALSE), fDisableCreateTreeFile(kFALSE)
   {
      lhisto = new KVHashList();
      ltree = new KVHashList();
   }
   virtual ~KVEventSelector()
   {
      //delete global variable list if it belongs to us, i.e. if created by a call to GetGVList
      if (TestBit(kDeleteGVList)) {
         delete gvlist;
         gvlist = 0;
         ResetBit(kDeleteGVList);
      }
      SafeDelete(fPartCond);
      lhisto->Clear();
      delete lhisto;
      lhisto = 0;
      ltree->Clear();
      delete ltree;
      ltree = 0;
   };
   void SetEventsReadInterval(Long64_t N)
   {
      fEventsReadInterval = N;
   };
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
   };
   virtual void InitRun()
   {
      AbstractMethod("InitRun");
   };
   virtual Bool_t Analysis()
   {
      //AbstractMethod("Analysis");
      return kTRUE;
   };
   virtual void EndRun()
   {
      AbstractMethod("EndRun");
   };
   virtual void EndAnalysis()
   {
      AbstractMethod("EndAnalysis");
   };
   //handling global variables for analysis
   virtual void SetGVList(KVGVList* list)
   {
      //Use a user-defined list of global variables for the analysis.
      //In this case it is the user's responsibility to delete the list
      //at the end of the analysis.
      gvlist = list;
   };
   virtual KVGVList* GetGVList(void) const
   {
      //Access to the internal list of global variables
      //If the list does not exist, it is created.
      //In this case it will be automatically deleted with the KVSelector object.
      if (!gvlist) {
         const_cast < KVEventSelector*>(this)->gvlist = new KVGVList;
         const_cast < KVEventSelector*>(this)->SetBit(kDeleteGVList);
      }
      return gvlist;
   };
   virtual void AddGV(KVVarGlob* vg)
   {
      //Add the global variable "vg" to the list of variables for the analysis.
      //This is equivalent to GetGVList()->Add( vg ).
      if (!vg)
         Error("AddGV(KVVarGlob*)", "KVVarGlob pointer is null");
      else
         GetGVList()->Add(vg);
   };
   virtual KVVarGlob* AddGV(const Char_t* class_name, const Char_t* name);
   virtual KVVarGlob* GetGV(const Char_t* name) const
   {
      //Access the global variable with name "name" in the list of variables
      //for the analysis.
      //This is equivalent to GetGVList()->GetGV( name ).

      return GetGVList()->GetGV(name);
   };
   virtual void RecalculateGlobalVariables();
   Bool_t AtEndOfRun(void)
   {
      // Check whether the end of run is reached for the current tree
      return (fTreeEntry + 1 == fChain->GetTree()->GetEntries());
   };

   virtual void SetParticleConditions(const KVParticleCondition&, const KVString& = "");
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
