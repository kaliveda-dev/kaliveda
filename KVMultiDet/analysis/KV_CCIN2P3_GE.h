//Created by KVClassFactory on Wed Apr 27 15:43:08 CEST 2011
//Author: John Frankland

#ifndef __KV_CCIN2P3_GE_H
#define __KV_CCIN2P3_GE_H

#include <KVBatchSystem.h>

class KV_CCIN2P3_GE: public KVBatchSystem {

protected:

   Int_t fDefJobTime; // default job length (in seconds)
   KVString fDefJobMem; // default job memory allocation (with units, e.g. "512MB")
   KVString fDefJobDisk; // default job disk space (with units, e.g. "1GB")
   Bool_t fTimeSet;
   Bool_t fDiskSet;
   Bool_t fMemSet;

   virtual void ChangeDefJobOpt(KVDataAnalyser*);

public:

   KV_CCIN2P3_GE(const Char_t* name);
   virtual ~ KV_CCIN2P3_GE();

   virtual void SetJobTimeString(const Char_t* h = "");      /* Set CPU time for batch job */
   virtual void SetJobTime(Int_t, Int_t = 0, Int_t = 0);        /* Set CPU time for batch job */
   virtual void SetJobMemory(const Char_t* h = ""); /* Set max memory for batch job */
   virtual void SetJobDisk(const Char_t* h = "");   /* Set max disk space for batch job */
   virtual void PrintJobs(Option_t* opt = "");  /* Print list of all jobs submitted to system */

   virtual Bool_t CheckJobParameters();        /* Checks job parameters */

   virtual const Char_t* GetJobTime(void);    /* Get CPU time for batch job */
   virtual const Char_t* GetJobMemory(void);       /* Get max memory for batch job */
   virtual const Char_t* GetJobDisk(void); /* Get max disk space for batch job */

   virtual void ChooseJobTime(void);    /* Choose CPU time for batch job */
   virtual void ChooseJobMemory(void);       /* Choose max memory for batch job */
   virtual void ChooseJobDisk(void); /* Choose max disk space for batch job */

   virtual void Clear(Option_t* opt = "");

   virtual void WriteBatchEnvFile(TEnv*);
   virtual void ReadBatchEnvFile(TEnv*);
   virtual void Print(Option_t* /*option*/ = "") const;

   virtual void SanitizeJobName();

   virtual KVList* GetListOfJobs();

   ClassDef(KV_CCIN2P3_GE, 1)  //Interface to CCIN2P3-GE batch job management system
};

#endif
