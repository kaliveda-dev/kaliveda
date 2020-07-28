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

/**
\class KVLockfile
\brief Interface to (Linux) system lockfile command
\ingroup Core

`lockfile`  can  be used to create one or more semaphore files, used to make sure that only one process
can write to a given file at any one time.

If `lockfile` can't create all the specified files (in the specified order), because another process has
 already 'locked' the file, it waits `sleeptime` (defaults to 8)
seconds and retries the last file that didn't succeed.

You can specify  the  number  of retries to do until failure is returned.
If the number of retries is -1 (default) `lockfile` will retry forever.

If the number of retries expires before all files have been created,
`lockfile` returns failure and removes all the files it created up till that point.

If you specify a `locktimeout` then a `lockfile` will be removed by force after `locktimeout` seconds
have passed  since the `lockfile` was last modified/created (most likely by some other program that
unexpectedly died a long time ago, and hence could not clean up any leftover lockfiles).

`lockfile` is clock skew immune. After a lockfile has been removed by force,
a suspension of `suspend` seconds (defaults to 16) is taken into account, in order to prevent the inadvertent
immediate removal of any newly created lockfile by another program (compare SUSPEND in procmail(1)).

To use a lockfile, create a KVLockfile object either using the constructor without arguments
or by giving the name of the file to lock:
~~~~{.cpp}
KVLockfile lox;
KVLockfile lox2("important.dat");
~~~~
Then set the values you want for the number of retries, the sleeptime etc. Default values are as for
Linux 'lockfile':
~~~~
sleeptime = 8 sec.
retries = -1 (keep trying for ever)
suspend = 16 sec. (in case of timeout)
~~~~
There is no timeout by default. If you want one, use SetTimeout() with timeout>0 seconds.

To lock the file:
~~~~{.cpp}
lox.Lock("important.dat"); // filename not given to ctor
lox2.Lock(); // filename given to ctor
~~~~

To unlock the file:
~~~~{.cpp}
lox.Release();
lox2.Release();
~~~~
Note that Release() is called automatically in the destructor in case e.g. the KVLockfile goes out of scope.
*/
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
