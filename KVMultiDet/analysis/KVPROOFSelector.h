//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Apr  1 10:33:24 2011 by ROOT version 5.29/01
// from TTree events/events
// found on file: events.root
//////////////////////////////////////////////////////////

#ifndef KVPROOFSelector_h
#define KVPROOFSelector_h

#include <KVHashList.h>
#include <KVNameValueList.h>
#include <KVString.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TSelector.h>
#include "TProofOutputFile.h"

/**
\class KVPROOFSelector
\brief General purpose class for running parallel tasks with PROOF
\ingroup Infrastructure

 Generate a class derived from this one using static method

    KVPROOFSelector::Make("MySelector");

 then you can use PROOF in order to run tasks in parallel:

    TProof::Open("");           // open PROOFLite session
    gProof->Process("MySelector.cpp+", [ncycles], "[options]");

 This will execute MySelector::Process(Long64_t) ncycles times.

 USING TREES & HISTOGRAMS TO SAVE DATA
 - declare any histograms with method AddHisto(TH1*)
 e.g. in InitAnalysis:
 void MySelector::InitAnalysis()
 {
     AddHisto( new TH2F("toto", "tata", 100, 0, 0, 500, 0, 0) );
 }
 Histograms can also be declared 'on the fly' in Analysis() method in
 the same way;

 - for TTrees, first call CreateTreeFile("...") with name of file for TTree(s)
 (by default, histograms and TTrees are written in different files - but see below),
 then declare all trees using method AddTree(TTree*)
 e.g. in InitAnalysis:
 void MySelector::InitAnalysis()
 {
     CreateTreeFile("MyTrees.root");
     TTree* aTree = new TTree("t1", "Some Tree");
     aTree->Branch(...) etc.
     AddTree(aTree);
 }

 - if you want (not obligatory), you can use methods FillHisto(...) and
  FillTree(...) in your Analysis() method;

 - to save histograms to file in EndAnalysis(), call method
  SaveHistos(const Char_t* filename) in EndAnalysis()

 -  the file declared with CreateTreeFile will be automatically written
 to disk at the end of the analysis.

 HISTOS & TREES IN SAME FILE
 If you want all results of your analysis to be written in a single file
 containing both histos and trees, put the following in the list of options:
      CombinedOutputFile=myResults.root
 or call method SetCombinedOutputFile("myResults.root") in your InitAnalysis();
 do not call SaveHistos() in EndAnalysis(), and make
 sure you call CreateTreeFile() without giving a name (the
 resulting intermediate file will have a default name
 allowing it to be found at the end of the analysis)
*/

class KVPROOFSelector : public TSelector {

protected :
   Long64_t fEventsRead;//cycle number (argument 'entry' passed to Process(Long64_t))
   Long64_t fEventsReadInterval;//interval at which to print number of events read

   KVHashList* lhisto;           //->!
   KVHashList* ltree;            //->!

   KVString fCombinedOutputFile;// optional name for single results file with trees and histos

   //parsed list of options given to TTree::Process
   KVNameValueList fOptionList;

   Bool_t fDisableCreateTreeFile;//used with PROOF

   void FillTH1(TH1* h1, Double_t one, Double_t two);
   void FillTProfile(TProfile* h1, Double_t one, Double_t two, Double_t three);
   void FillTH2(TH2* h2, Double_t one, Double_t two, Double_t three);
   void FillTProfile2D(TProfile2D* h2, Double_t one, Double_t two, Double_t three, Double_t four);
   void FillTH3(TH3* h3, Double_t one, Double_t two, Double_t three, Double_t four);

public:
   TFile* writeFile;//!
   TProofOutputFile* mergeFile;//! for merging with PROOF
   TString tree_file_name;
   Bool_t CreateTreeFile(const Char_t* filename = "");

   virtual void ParseOptions();

   KVPROOFSelector(TTree* /*tree*/ = 0) :
      fEventsRead(0), fEventsReadInterval(100), fDisableCreateTreeFile(kFALSE)
   {
      lhisto = new KVHashList();
      ltree = new KVHashList();
   }
   virtual ~KVPROOFSelector()
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
   virtual Bool_t  Process(Long64_t entry);
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   /* user entry points */
   virtual void InitAnalysis() = 0;
   virtual Bool_t Analysis() = 0;
   virtual void EndAnalysis() = 0;

   void AddHisto(TH1* histo);
   void AddTree(TTree* tree);

   void FillHisto(const Char_t* sname, Double_t one, Double_t two = 1, Double_t three = 1, Double_t four = 1);
   void FillTree(const Char_t* sname = "");

   KVHashList* GetHistoList() const;
   KVHashList* GetTreeList() const;

   TH1* GetHisto(const Char_t* name) const;
   TTree* GetTree(const Char_t* name) const;

   virtual void SaveHistos(const Char_t* filename = "", Option_t* option = "recreate", Bool_t onlyfilled = kFALSE);

   virtual void SetOpt(const Char_t* option, const Char_t* value);
   virtual Bool_t IsOptGiven(const Char_t* option);
   virtual TString GetOpt(const Char_t* option) const;
   virtual void UnsetOpt(const Char_t* opt);

   void SetCombinedOutputFile(const TString& filename)
   {
      // Call in InitAnalysis() to set the name of the single output file
      // containing all histograms and TTrees produced by analysis.
      // This is equivalent to running the analysis with option
      //    CombinedOutputFile=[filename]
      // but setting this option in InitAnalysis() will not work.
      // Note that if this method is not called/the option is not given,
      // histograms and TTrees will be written in separate files.
      fCombinedOutputFile = filename;
   }
   Long64_t GetCycleNumber() const
   {
      // Returns the argument 'entry' passed to Process(Long64_t entry)
      return fEventsRead;
   }
   static void Make(const Char_t* classname);

   ClassDef(KVPROOFSelector, 1)//General purpose class for performing parallel tasks with PROOF
};

#endif
