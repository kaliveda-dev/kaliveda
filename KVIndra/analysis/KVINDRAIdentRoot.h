/*
$Id: KVINDRAIdentRoot.h,v 1.2 2006/10/19 14:32:43 franklan Exp $
$Revision: 1.2 $
$Date: 2006/10/19 14:32:43 $
*/

#ifndef KVINDRAIdentRoot_h
#define KVINDRAIdentRoot_h

#include "KVSelector.h"


class TFile;
class TTree;

class KVINDRAIdentRoot: public KVSelector {

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
   KVINDRAIdentRoot()
   {
      fIdentFile = 0;
      fIdentTree = 0;
   };
   virtual ~ KVINDRAIdentRoot()
   {
   };

   virtual void InitRun();
   virtual void EndRun();
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   void CountStatus();
   void CountCodes();

   ClassDef(KVINDRAIdentRoot, 0);//Generation of fully-identified and calibrated INDRA data files
};

#endif
