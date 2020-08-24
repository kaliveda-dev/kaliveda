//Created by KVClassFactory on Tue Feb 14 11:13:53 2017
//Author: John Frankland,,,

#ifndef __KVSIMDIRANALYSER_H
#define __KVSIMDIRANALYSER_H

#include "KVDataAnalyser.h"
#include "TChain.h"

#include <KVSimDir.h>

/**
  \class KVSimDirAnalyser
  \brief Class piloting analyses of simulated data
  \ingroup Analysis
 */

class KVSimDirAnalyser : public KVDataAnalyser {

   TList* fListOfSimFiles;//!    list of files/trees to analyse
   TList* fListOfAuxFiles;//!    [optional] list of original simulated data to be used during filtered data analysis
   TChain* fAnalysisChain;//!    TChain for analysis
   KVSimDir* fSimDir;//!         used for batch analysis
   Bool_t fCopyFilesToWorkDir;//! if true, files to be analysed are copied to working directory first

private:
   void DeleteSimFilesListIfOurs();
   void DeleteAuxFilesListIfOurs();

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
   void SetAuxFileList(TList* l)
   {
      fListOfAuxFiles = l;
   }
   TList* GetFileList() const
   {
      return fListOfSimFiles;
   }
   Int_t GetNumberOfFilesToAnalyse() const
   {
      return fListOfSimFiles ? fListOfSimFiles->GetEntries() : 0;
   }

   void SubmitTask();
   KVString GetRootDirectoryOfDataToAnalyse() const;

   void WriteBatchEnvFile(const Char_t*, Bool_t sav = kTRUE);
   Bool_t ReadBatchEnvFile(const Char_t*);

   static void Make(const Char_t* kvsname = "MySimulatedAnalysis");

   void SetCopyFilesToWorkDir(Bool_t on = kTRUE)
   {
      fCopyFilesToWorkDir = on;
   }
   Bool_t IsCopyFilesToWorkDir() const
   {
      return fCopyFilesToWorkDir;
   }

   ClassDef(KVSimDirAnalyser, 1) //Analysis of trees containing simulated events
};

#endif
