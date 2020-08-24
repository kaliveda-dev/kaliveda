/*
$Id: KVRemoteAvailableRunsFile.cpp,v 1.4 2008/02/08 08:19:59 franklan Exp $
$Revision: 1.4 $
$Date: 2008/02/08 08:19:59 $
*/

//Created by KVClassFactory on Mon May 22 00:14:11 2006
//Author: John Frankland

#include "KVRemoteAvailableRunsFile.h"
#include "KVDataRepository.h"
#include "KVDataSet.h"
#include "TSystem.h"
#include "TEnv.h"

using namespace std;

ClassImp(KVRemoteAvailableRunsFile)

KVRemoteAvailableRunsFile::KVRemoteAvailableRunsFile()
{
   //default Constructor
   init();
}

KVRemoteAvailableRunsFile::KVRemoteAvailableRunsFile(const Char_t*
      type,
      KVDataSet*
      ds): KVAvailableRunsFile
   (type, ds)
{
   //Constructor with name of datatype
   //and pointer to parent dataset
   init();
}

KVRemoteAvailableRunsFile::~KVRemoteAvailableRunsFile()
{
   //Destructor
   //Delete the runlist file from the temp directory if it has been opened
   if (IsFileOpen()) {
      if (runlist_lock.Lock(fFilePath.Data())) {
         gSystem->Unlink(fFilePath);
         runlist_lock.Release();
      }
   }
}

//__________________________________________________________________________________________________________________

void KVRemoteAvailableRunsFile::init()
{
   //Find executable 'curl' or programme indicated by
   //value of DataRepository.RemoteAvailableRuns.protocol
   fCurl = gEnv->GetValue(Form
                          ("%s.DataRepository.RemoteAvailableRuns.protocol",
                           GetDataSet()->GetRepository()->GetName()), "curl");
   KVBase::FindExecutable(fCurl);
}

//__________________________________________________________________________________________________________________

Bool_t KVRemoteAvailableRunsFile::OpenAvailableRunsFile()
{
   //Initialise fRunlist so that it can be used to read the available runs file from the beginning.
   //
   //If the file has already been opened, this means resetting the ifstream to the start of the file.
   //
   //If not already open, open the file containing the list of available runs for the dataset.
   //What this actually means is:
   //      - read value of DataRepository.RemoteAvailableRuns.url for the data repository
   //      - use 'curl' or programme indicated by value of
   //      DataRepository.RemoteAvailableRuns.protocol to make local copy
   //      of file
   //      - open copy
   //Returns kFALSE in case of problems.

   //already open ?
   if (IsFileOpen()) {
      fRunlist.clear();         // clear any error flags (EOF etc.)
      fRunlist.seekg(0, ios::beg);      // set file buffer pointer to beginning of file
      //if we can get a lock on the file, all is well. if not, we need to fetch a new
      //copy
      if (runlist_lock.Lock()) return kTRUE;
   }
   TString http =
      gEnv->GetValue(Form("%s.DataRepository.RemoteAvailableRuns.url",
                          GetDataSet()->GetRepository()->GetName()), "");
   if (http == "") {
      Warning("OpenAvailableRunsFile(ifstream& runlist)",
              "%s.DataRepository.RemoteAvailableRuns.url is not defined. See $KVROOT/KVFiles/.kvrootrc",
              GetDataSet()->GetRepository()->GetName());
      return kFALSE;
   }
   TString url;
   url.Form("%s/%s", http.Data(), GetFileName());
   //remote file will be copied to $TEMP directory with name
   //repository.available_runs....
   fFilePath = GetFileName();
   KVBase::GetTempFileName(fFilePath);
   TString cmd;
   cmd.Form("%s -o%s %s", fCurl.Data(), fFilePath.Data(), url.Data());
   //lock temp file - just in case
   runlist_lock.Lock(fFilePath.Data());
   if (gSystem->Exec(cmd.Data())) {
      runlist_lock.Release();
      return kFALSE;            // problem with curl
   }
   fRunlist.open(fFilePath.Data());
   if (!fRunlist.good() || !fRunlist.is_open()) {
      Error("OpenAvailableRunsFile",
            "Cannot open temp file to copy remote runlist file");
      runlist_lock.Release();
      return kFALSE;
   }
   return kTRUE;
}

//__________________________________________________________________________________________________________________

void KVRemoteAvailableRunsFile::CloseAvailableRunsFile()
{
   //Redefines KVAvailableRunsFile::CloseAvailableRunsFile
   //We do not want to transfer the file again every time we need to read it,
   //therefore we do not close the file, simply remove the lock.
   runlist_lock.Release();
}

