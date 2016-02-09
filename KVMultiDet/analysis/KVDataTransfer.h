/*
$Id: KVDataTransfer.h,v 1.5 2007/01/04 16:38:50 franklan Exp $
$Revision: 1.5 $
$Date: 2007/01/04 16:38:50 $
*/

//Created by KVClassFactory on Mon Sep 18 14:27:21 2006
//Author: franklan

#ifndef __KVDATATRANSFER_H
#define __KVDATATRANSFER_H

#include "KVDataAnalyser.h"
#include "TString.h"

class KVDataRepository;

class KVDataTransfer: public KVDataAnalyser {
protected:
   KVDataRepository* fSourceRep;        // repository containing source files
   KVDataRepository* fTargetRep;        // repository where files will be copied
   TString fCmdFile;            //name of command file given to transfer agent
   TString fTransferExec;      //full path to transfer client executable

   virtual void SetTransferExec(const Char_t* path)
   {
      fTransferExec = path;
   };
   virtual void ChooseAnalysisTask()
   {
      ;
   };
   virtual void SubmitTask()
   {
      ;
   };

   virtual void ExecuteCommand() = 0;
   virtual void WriteTransferScript() = 0;

   virtual void CheckTargetRepository();

   virtual void init();
   Bool_t fOK;                  //may be set to kFALSE by init(), in which case Run() will abort

public:

   KVDataTransfer();
   virtual ~ KVDataTransfer();
   virtual void Run();
   virtual void TransferRuns();
   virtual void SetDataSet(KVDataSet* ds);
   virtual void SetDataSet(const Char_t* name);

   static KVDataTransfer* NewTransfer(const Char_t* source_rep,
                                      const Char_t* target_rep);

   ClassDef(KVDataTransfer, 0)  //Transfers data between data repositories
};

#endif
