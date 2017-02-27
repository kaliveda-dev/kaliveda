//Created by KVClassFactory on Tue Feb 14 11:13:53 2017
//Author: John Frankland,,,

#ifndef __KVSIMDIRANALYSER_H
#define __KVSIMDIRANALYSER_H

#include "KVDataAnalyser.h"
#include "TChain.h"

#include <KVSimDir.h>

class KVSimDirAnalyser : public KVDataAnalyser {

   TList* fListOfSimFiles;//!    list of files/trees to analyse
   TChain* fAnalysisChain;//!    TChain for analysis
   KVSimDir* fSimDir;//!         used for batch analysis

private:
   void DeleteSimFilesListIfOurs();

protected:
   void BuildChain();
   virtual Bool_t NeedToChooseWhatToAnalyse() const
   {
      return !(fListOfSimFiles && fListOfSimFiles->GetEntries());
   }

public:
   KVSimDirAnalyser();
   virtual ~KVSimDirAnalyser();

   void SetFileList(TList* l)
   {
      fListOfSimFiles = l;
   }
   Int_t GetNumberOfFilesToAnalyse() const
   {
      return fListOfSimFiles ? fListOfSimFiles->GetEntries() : 0;
   }

   void SubmitTask();
   KVString GetRootDirectoryOfDataToAnalyse() const;

   void WriteBatchEnvFile(const Char_t*, Bool_t sav = kTRUE);
   Bool_t ReadBatchEnvFile(const Char_t*);

   ClassDef(KVSimDirAnalyser, 1) //Analysis of trees containing simulated events
};

#endif
