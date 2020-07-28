/*
$Id: KVLockfile.cpp,v 1.2 2008/03/12 11:21:58 franklan Exp $
$Revision: 1.2 $
$Date: 2008/03/12 11:21:58 $
*/

//Created by KVClassFactory on Wed Feb  6 12:39:42 2008
//Author: franklan

#include "KVLockfile.h"
#include "TSystem.h"
#include "Riostream.h"
#include "KVError.h"

using namespace std;

ClassImp(KVLockfile)



KVLockfile::KVLockfile(const Char_t* filename): fFile(filename), fLockfile("lockfile")
{
   // Default constructor
   init();
}

////////////////////////////////////////////////////////////////////////////////

KVLockfile::~KVLockfile()
{
   // Destructor
   // If file is still locked, we call Release
   if (locked) Release();
}

////////////////////////////////////////////////////////////////////////////////

void KVLockfile::init()
{
   //Initialisation
   //Look for 'lockfile' executable and store path if found
   //Set value of have_exec accordingly
   have_exec = FindExecutable(fLockfile);
   if (!have_exec) {
      Warning(KV__ERROR(init), "Unix 'lockfile' command not found on system. You should install it.");
   }
   sleeptime = 8; //time to wait before retrying lock
   retries = -1; //number of times to retry
   locktimeout = 0; //time after which lock automatically opens
   suspend = 16; //suspend time after timeout
   locked = kFALSE;
}
Bool_t KVLockfile::FindExecutable(TString& exec, const Char_t* path)
{
   //copied from KVBase to avoid circular dependency

   TString spath(path), backup(exec), backup2(exec), expandexec(exec);
   gSystem->ExpandPathName(expandexec);
   if (gSystem->IsAbsoluteFileName(expandexec.Data())) {
      //executable given as absolute path
      //we check if it exists
      if (!gSystem->AccessPathName(expandexec)) {
         exec = expandexec;
         return kTRUE;
      }
      else {
         //try with ".exe" in case of Windows system
         if (!expandexec.EndsWith(".exe")) {
            expandexec += ".exe";
            if (!gSystem->AccessPathName(expandexec)) {
               exec = expandexec;
               return kTRUE;
            }
         }
      }
      exec = backup;
      return kFALSE;
   }
   gSystem->ExpandPathName(spath);
   if (gSystem->FindFile(spath.Data(), exec))
      return kTRUE;
   if (!backup.EndsWith(".exe")) {
      backup += ".exe";
      if (gSystem->FindFile(spath.Data(), backup)) {
         exec = backup;
         return kTRUE;
      }
   }
   exec = backup2;
   return kFALSE;
}

////////////////////////////////////////////////////////////////////////////////

void KVLockfile::writecmd()
{
   //Writes lockfile command with current values of parameters
   if (locktimeout)
      cmd.Form("%s -%d -r%d -l%d -s%d %s.lock",
               fLockfile.Data(),
               sleeptime,
               retries,
               locktimeout,
               suspend,
               fFile.Data()
              );
   else
      cmd.Form("%s -%d -r%d -s%d %s.lock",
               fLockfile.Data(),
               sleeptime,
               retries,
               suspend,
               fFile.Data()
              );
}

////////////////////////////////////////////////////////////////////////////////

int KVLockfile::testlock()
{
   //Executes lockfile command with current values of parameters
   //(call KVLockfile::writecmd() before)
   return gSystem->Exec(cmd.Data());
}

////////////////////////////////////////////////////////////////////////////////

Bool_t KVLockfile::Lock(const Char_t* filename)
{
   if (!have_exec) {
      locked = kTRUE;
      return kTRUE;
   }
   if (locked) {
      cout << "<Error in KVLockfile::Lock: file " << fFile.Data() << " is already locked. Release it first>" << endl;
      return kFALSE;
   }
   if (strcmp(filename, "")) fFile = filename;
   writecmd();
   if (!testlock()) {
      //cout << "<Info in KVLockfile::Lock : Locked " << fFile.Data() << ">" << endl;
      locked = kTRUE;
      return kTRUE;
   }
   else {
      cout << "<Error in KVLockfile::Lock: can't get a lock for file " << fFile.Data() << ">" << endl;
      locked = kFALSE;
      return kFALSE;
   }
}

////////////////////////////////////////////////////////////////////////////////

Bool_t KVLockfile::Release()
{
   if (!have_exec) {
      locked = kFALSE;
      return kTRUE;
   }
   if (!locked) {
      cout << "<Error in KVLockfile::Release: file is not locked. Lock it first>" << endl;
      return kFALSE;
   }
   //cout << "<Info in KVLockfile::Release : Released " << fFile.Data() << ">" << endl;
   locked = kFALSE;
   return (gSystem->Unlink(Form("%s.lock", fFile.Data())) != -1);
}
