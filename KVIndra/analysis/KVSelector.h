//////////////////////////////////////////////////////////
//   This class has been automatically generated
//     (Mon Feb 23 13:13:30 2004 by ROOT version3.10/02)
//   from TChain tree/
//////////////////////////////////////////////////////////


#ifndef KVSelector_h
#define KVSelector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include "KVINDRAReconEvent.h"
#include "KVINDRA.h"
#include "KV2Body.h"
#include "KVGVList.h"
#include "TStopwatch.h"
#include "TEventList.h"
#include "KVDataSelector.h"
#include "KVDataSet.h"
#include "KVDataBase.h"
#include "KVParticleCondition.h"
#include "KVString.h"
#include "KVLockfile.h"
#include "KVConfig.h"
#include "KVDataAnalyser.h"
#include "KVHashList.h"
#include "KVNameValueList.h"
#include "KVINDRADBRun.h"

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TProfile2D.h"
#include "TProfile.h"
#include "KVDalitzPlot.h"

class KVSelector: public TSelector {

protected:

   enum {
      kDeleteGVList = BIT(14),
      kChangeMasses = BIT(15)
   };

   Int_t callnotif;
   Int_t totentry;
   Int_t NbTreeEntry;
//Declaration of leaves types
   KVINDRAReconEvent* data;

//List of branches
   TBranch* b_data;             //!

//List of global variables
   KVGVList* gvlist;            //!
   KVHashList* lhisto;           //->!
   KVHashList* ltree;            //->!

   TStopwatch* fTimer;          //!used to time analysis

#ifdef __WITHOUT_TSELECTOR_LONG64_T
   Int_t fTreeEntry;            //!this is the current TTree entry number, i.e. the argument passed to TSelector::Process(Long64_t entry)
#else
   Long64_t fTreeEntry;         //!this is the current TTree entry number, i.e. the argument passed to TSelector::Process(Long64_t entry)
#endif

   TEventList* fEvtList;        //!this is the current event list.
#ifdef __WITHOUT_TSELECTOR_LONG64_T
   Int_t* fTreeOffset;          //!this is the current TTree offset table
#else
   Long64_t* fTreeOffset;       //!this is the current TTree offset table
#endif
   Int_t fCurrentTreeNumber;    //!This is the current tree number

   TString fDataSelector;       //! Name of the KVDataSelector
   KVDataSelector* fKVDataSelector;     //! KVDataSelector
   Bool_t needToSelect;         //! tells if one needs to build the TEventList
   Bool_t* fTEVLexist;          //! tells if the TEventList exist for each run
   Bool_t needToCallEndRun;     //! tells if one needs to call EndRun
   KVINDRADBRun* fCurrentRun;   //! current run

   UInt_t fNewMassFormula;//new mass formula to apply to fragments, if required

   KVParticleCondition* fPartCond;//(optional) conditions for selecting particles

   static KVString fBranchName; //name of branch which contains KVINDRAReconEvent objects in the TTree

   TTree* fChain;              //!pointer to the analyzed TTree or TChain

   TTree* fGeneData; //!pointer to tree containing pulser and laser data for run
   TTree* fRawData; //!pointer to tree containing raw data for run

   //parsed list of options given to TTree::Process
   KVNameValueList fOptionList;
   void ParseOptions();

   KVINDRAReconEvent** GetEventReference()
   {
      return &data;
   };

   virtual KVINDRADBRun* GetCurrentRun() const
   {
      return fCurrentRun;
   };

   void SetINDRAReconEventBranchName(const Char_t* br_name)
   {
      fBranchName = br_name;
   };

   KVLockfile dataselector_lock;//for locking user's data selector file

   void FillTH1(TH1* h1, Double_t one, Double_t two);
   void FillTProfile(TProfile* h1, Double_t one, Double_t two, Double_t three);
   void FillTH2(TH2* h2, Double_t one, Double_t two, Double_t three);
   void FillKVDalitz(KVDalitzPlot* h2, Double_t one, Double_t two, Double_t three);
   void FillTProfile2D(TProfile2D* h2, Double_t one, Double_t two, Double_t three, Double_t four);
   void FillTH3(TH3* h3, Double_t one, Double_t two, Double_t three, Double_t four);

public:

   KVSelector(TTree* tree = 0);
   virtual ~ KVSelector();

   KVINDRAReconEvent* GetEvent()
   {
      return data;
   };
   Int_t GetEventNumber()
   {
      // returns number of currently analysed event
      // N.B. this may be different to the TTree/TChain entry number etc.
      return data->GetNumber();
   };

   Int_t Version() const
   {
      return 2;
   };

   void Begin(TTree* tree);
   void SlaveBegin(TTree* tree);
   void Init(TTree* tree);
   Bool_t Notify();
#ifdef __WITHOUT_TSELECTOR_LONG64_T
   Bool_t Process(Int_t entry);
#else
   Bool_t Process(Long64_t entry);
#endif
   void SetOption(const char* option)
   {
      fOption = option;
   }
   void SetObject(TObject* obj)
   {
      fObject = obj;
   }
   void SetInputList(TList* input)
   {
      fInput = input;
   }
   TList* GetOutputList() const
   {
      return fOutput;
   }
   void SlaveTerminate();
   void Terminate();

   /* user entry points */
   virtual void InitAnalysis()
   {
   };
   virtual void InitRun()
   {
   };
   virtual Bool_t Analysis()
   {
      return kTRUE;
   };
   virtual void EndRun()
   {
   };
   virtual void EndAnalysis()
   {
   };

   //handling global variables for analysis
   virtual void SetGVList(KVGVList* list);
   virtual KVGVList* GetGVList(void);
   virtual void AddGV(KVVarGlob*);
   virtual KVVarGlob* AddGV(const Char_t* class_name,
                            const Char_t* name);
   virtual KVVarGlob* GetGV(const Char_t*) const;
   virtual void RecalculateGlobalVariables();

#ifdef __WITHOUT_TSELECTOR_LONG64_T
   virtual Int_t GetTreeEntry() const;
#else
   virtual Long64_t GetTreeEntry() const;
#endif

   virtual void BuildEventList(void);
   virtual Bool_t AtEndOfRun(void);

   virtual void SetDataSelector(const Char_t* dataSel = "")
   {
      fDataSelector = dataSel;
   }
   virtual const Char_t* GetDataSelector(void)
   {
      return fDataSelector.Data();
   }

   virtual void LoadDataSelector(void);
   virtual void SaveCurrentDataSelection(void);

   virtual const Char_t* GetDataSelectorFileName(void);

   virtual void ChangeFragmentMasses(UInt_t mass_formula);

   virtual void SetParticleConditions(const KVParticleCondition&);

   //Return pointer to tree containing pulser and laser events for the current run
   TTree* GetGeneData()
   {
      return fGeneData;
   };

   //Return pointer to tree containing raw data for the current run
   TTree* GetRawData()
   {
      return fRawData;
   };

   static void Make(const Char_t* kvsname = "MyOwnKVSelector");

   virtual void CreateHistos();
   virtual void CreateTrees();

   void FillHisto(KVString sname, Double_t one, Double_t two = 1, Double_t three = 1, Double_t four = 1);
   void FillTree(KVString sname = "");

   KVHashList* GetHistoList();
   KVHashList* GetTreeList();

   TH1* GetHisto(const Char_t* name);
   TTree* GetTree(const Char_t* name);

   virtual void WriteHistoToFile(KVString filename = "FileFromKVSelector.root", Option_t* option = "recreate");
   virtual void WriteTreeToFile(KVString filename = "FileFromKVSelector.root", Option_t* option = "recreate");

   virtual void SetOpt(const Char_t* option, const Char_t* value);
   virtual Bool_t IsOptGiven(const Char_t* option);
   virtual const TString& GetOpt(const Char_t* option) const;
   virtual void UnsetOpt(const Char_t* opt);

   ClassDef(KVSelector, 0);     //Analysis class for TChains of KVINDRAReconEvents
};

#endif


