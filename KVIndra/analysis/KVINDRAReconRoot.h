#ifndef KVINDRAReconRoot_h
#define KVINDRAReconRoot_h

#include "KVSelector.h"

class TFile;
class TTree;

class KVINDRAReconRoot: public KVSelector {
   int codes[15];
   int status[4];
   int Acodes[15];
   int Astatus[4];

protected:
   TFile* fIdentFile;           //new file
   TTree* fIdentTree;           //new tree
   Int_t fRunNumber;
   Int_t fEventNumber;

public:
   KVINDRAReconRoot()
   {
      fIdentFile = 0;
      fIdentTree = 0;
   };
   virtual ~ KVINDRAReconRoot()
   {
   };

   virtual void InitRun();
   virtual void EndRun();
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   void CountStatus();
   void CountCodes();

   ClassDef(KVINDRAReconRoot, 0) //Generation of fully-identified and calibrated INDRA data files
};

#endif
