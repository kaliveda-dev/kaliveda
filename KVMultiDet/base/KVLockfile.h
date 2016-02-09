/*
$Id: KVLockfile.h,v 1.1 2008/02/07 09:25:40 franklan Exp $
$Revision: 1.1 $
$Date: 2008/02/07 09:25:40 $
*/

//Created by KVClassFactory on Wed Feb  6 12:39:42 2008
//Author: franklan

#ifndef __KVLOCKFILE_H
#define __KVLOCKFILE_H

#include "KVString.h"

class KVLockfile {
   KVString fFile;//name of file
   KVString fLockfile;//full path to lockfile executable (if defined)
   bool have_exec;//kTRUE if lockfile found on system
   int sleeptime;//time to wait before retrying lock
   int retries;//number of times to retry
   int locktimeout;//time after which lock automatically opens
   int suspend;//suspend time after timeout
   KVString cmd;//command to execute
   bool locked;//kTRUE when Lock() has been called successfully

   void init();
   int testlock();
   void writecmd();
   Bool_t FindExecutable(TString& exec, const Char_t* path = "$(PATH)");

public:
   KVLockfile(const Char_t* filename = "");
   virtual ~KVLockfile();

   void SetSleeptime(int s)
   {
      sleeptime = s;
   };
   void SetRetries(int r)
   {
      retries = r;
   };
   void SetTimeout(int t)
   {
      locktimeout = t;
   };
   void SetSuspend(int s)
   {
      suspend = s;
   };

   Bool_t Lock(const Char_t* filename = "");
   Bool_t Release();

   Bool_t IsLocked() const
   {
      return locked;
   };

   ClassDef(KVLockfile, 1) //Interface to (Linux) system lockfile command
};

#endif
