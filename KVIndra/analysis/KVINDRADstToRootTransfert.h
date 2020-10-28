/*
$Id: KVINDRADstToRootTransfert.h,v 1.3 2007/05/31 09:59:22 franklan Exp $
$Revision: 1.3 $
$Date: 2007/05/31 09:59:22 $
*/

//Created by KVClassFactory on Fri May  4 15:26:02 2007
//Author: franklan

#ifndef __KVINDRADSTTOROOTTRANSFERT_H
#define __KVINDRADSTTOROOTTRANSFERT_H

#include "KVString.h"
#include "KVNumberList.h"
#include "KVIntegerList.h"
#include "KVDataSet.h"
#include "KVINDRADSTReader.h"
#include "TString.h"

class KVINDRAReconEvent;
class KVIDTelescope;
class KVDetector;
class TTree;

/**
\class KVINDRADstToRootTransfert
\brief Conversion of INDRA DST file to KaliVeda ROOT format
\ingroup INDRAnalysis
*/

class KVINDRADstToRootTransfert : public KVINDRADSTReader {

protected:
   Int_t fEventNumber;//!
   Int_t NbParFired;//!
   UShort_t ParVal[3000];//!
   UInt_t ParNum[3000];//!
   const KVSeqCollection* params;//!

   virtual KVNumberList PrintAvailableRuns(KVString& datatype);

   Int_t mt;
   // de1: energie Phoswich pour couronne1 campagne 1,2,3
   //      energie ChIo pour couronne > 1
   // de2: energie Si
   // de3: energie CsI
   // de4: energie Si75 pour etalons
   // de5: energie SiLi pour etalons
   // de_mylar: contribution mylar pour la ChIo

   Double_t de1, de2, de3, de4, de5, de_mylar;
   Double_t canal[16];
   Int_t code_idf[4];
   KVIDTelescope* identifying_telescope;

   Int_t events_in_file;//total number of events read from each file - for tests

   Int_t events_good, events_read;//totals of events read and trees filled

   TTree* data_tree;//tree for writing events
   TTree* rawtree;//raw data tree

   Int_t fRunNumber;//run number of current file
   Int_t fCampNumber;

   TString req_time, req_mem, req_scratch;
   TString cur_time, cur_mem, cur_scratch;


   virtual void ProcessRun();

public:

   KVINDRADstToRootTransfert();
   virtual ~KVINDRADstToRootTransfert();

   virtual void InitRun();
   virtual void EndRun();

   virtual void SubmitTask();

   void DefineSHELLVariables();
   void ReadDST();

   KVDetector* Code2and9and10(int ring, int mod);
   KVDetector* Code3(int ring, int mod);
   KVDetector* Code0(int ring, int mod);
   KVDetector* Code1(int ring, int mod);
   KVDetector* Code7(int ring, int mod);
   KVDetector* Code4and5and6and8(int ring, int mod);

   void lire_evt(std::ifstream& f_in, KVINDRAReconEvent* evt);

   void FillRawTree();

   ClassDef(KVINDRADstToRootTransfert, 1) //Conversion of INDRA DST to KaliVeda ROOT format
};

#endif
