//Created by KVClassFactory on Tue Jan 20 16:29:33 2015
//Author: ,,,

#ifndef __KVFAZIADBRUN_H
#define __KVFAZIADBRUN_H

#include "KVDBRun.h"

class KVFAZIADBRun : public KVDBRun {

public:
   KVFAZIADBRun();
   KVFAZIADBRun(Int_t number, const Char_t* title);
   virtual ~KVFAZIADBRun();

   void SetTrigger(Int_t mt);
   Int_t GetTrigger() const;
   const Char_t* GetTriggerString() const;

   void SetACQStatus(const KVString& status);
   const Char_t* GetACQStatus() const;

   void SetGoodEvents(Int_t);
   Int_t GetGoodEvents() const;

   void SetError_WrongNumberOfBlocks(Int_t);
   Int_t GetError_WrongNumberOfBlocks() const;

   void SetError_InternalBlockError(Int_t);
   Int_t GetError_InternalBlockError() const;

   void SetNumberOfAcqFiles(Int_t);
   Int_t GetNumberOfAcqFiles() const;

   void SetDuration(Double_t);
   Double_t GetDuration() const;

   void SetFrequency(Double_t);
   Double_t GetFrequency() const;

   void SetTriggerRate(Double_t);
   Double_t GetTriggerRate() const;

   void SetDeadTime(Double_t);
   Double_t GetDeadTime() const;

   void SetNumberOfTriggerBlocks(Double_t);
   Double_t GetNumberOfTriggerBlocks() const;

   ClassDef(KVFAZIADBRun, 1) //run description for FAZIA experiment
};

#endif
