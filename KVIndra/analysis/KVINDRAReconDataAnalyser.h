/*
$Id: KVINDRAReconDataAnalyser.h,v 1.2 2007/05/31 09:59:22 franklan Exp $
$Revision: 1.2 $
$Date: 2007/05/31 09:59:22 $
*/

//Created by KVClassFactory on Wed Apr  5 23:50:04 2006
//Author: John Frankland

#ifndef __KVINDRAReconDataAnalyser_H
#define __KVINDRAReconDataAnalyser_H

#include "KVDataSetAnalyser.h"
#include "KVINDRAEventSelector.h"
#include "KVOldINDRASelector.h" // backwards compatibility
#include <KVDataPatchList.h>
class TChain;

/**
  \class KVINDRAReconDataAnalyser
  \ingroup INDRAnalysis
  \brief Manage analysis of reconstructed INDRA data
 */
class KVINDRAReconDataAnalyser: public KVDataSetAnalyser {

protected:
   virtual KVNumberList PrintAvailableRuns(KVString& datatype);
   KVINDRAEventSelector* fSelector;// the data analysis class
   KVOldINDRASelector* fOldSelector;// backwards compatibility
   TTree* theChain;//chain of TTrees to be analysed
   TTree* theRawData;//raw data TTree in recon file
   TTree* theGeneData;//gene data TTree in recon file
   Int_t NbParFired;
   UShort_t* ParVal;
   UInt_t* ParNum;
   TObjArray* parList;
   Long64_t Entry;
   void ConnectRawDataTree();
   void ConnectGeneDataTree();

   Long64_t TotalEntriesToRead;
   KVString fDataVersion;//KV version used to write analysed data
   KVString fDataSeries;//KV series used to write analysed data
   Int_t fDataReleaseNum;//KV release number used to write analysed data

   KVDataPatchList fRustines;//patches to be applied to correct data before analysis

public:

   Long64_t GetTotalEntriesToRead() const
   {
      return TotalEntriesToRead;
   }

   KVINDRAReconDataAnalyser();
   virtual ~ KVINDRAReconDataAnalyser();

   TTree* GetTree() const
   {
      return theChain;
   }
   void SetTree(TTree* t)
   {
      theChain = t;
   }

   virtual void Reset();

   virtual Bool_t CheckTaskVariables(void);
   virtual void SubmitTask();
   virtual void WriteBatchEnvFile(const Char_t*, Bool_t sav = kTRUE);

   void preInitAnalysis();
   void preAnalysis();
   void preInitRun();
   virtual void RegisterUserClass(TObject* obj)
   {
      // The user class may inherit from KVINDRAEventSelector or KVOldINDRASelector
      // Only one of the two pointers will be valid
      fSelector = dynamic_cast<KVINDRAEventSelector*>(obj);
      fOldSelector = dynamic_cast<KVOldINDRASelector*>(obj);
   }
   void PrintTreeInfos();
   TEnv* GetReconDataTreeInfos() const;

   KVString GetDataVersion() const
   {
      return fDataVersion;
   }
   KVString GetDataSeries() const
   {
      return fDataSeries;
   }
   Int_t GetDataReleaseNumber() const
   {
      return fDataReleaseNum;
   }
   TTree* GetRawDataTree() const
   {
      return theRawData;
   }
   TTree* GetGeneDataTree() const
   {
      return theGeneData;
   }
   void CloneRawAndGeneTrees();
   virtual Bool_t CheckIfUserClassIsValid(const KVString& = "");
   void SetSelectorCurrentRun(KVINDRADBRun* CurrentRun);
   Long64_t GetRawEntryNumber();
   KVReconstructedEvent* GetReconstructedEvent();
#ifdef USING_ROOT6
   void SetTriggerConditionsForRun(int);
#endif
   ClassDef(KVINDRAReconDataAnalyser, 0) //For analysing reconstructed INDRA data
};

#endif
