/*
$Id: KVRemoteAvailableRunsFile.h,v 1.3 2008/02/08 08:19:59 franklan Exp $
$Revision: 1.3 $
$Date: 2008/02/08 08:19:59 $
*/

//Created by KVClassFactory on Mon May 22 00:14:11 2006
//Author: John Frankland

#ifndef __KVREMOTEAVAILABLERUNSFILE_H
#define __KVREMOTEAVAILABLERUNSFILE_H

#include <KVAvailableRunsFile.h>

class KVRemoteAvailableRunsFile: public KVAvailableRunsFile {
protected:

   TString fFilePath;           //full path to copy of remote file kept in temp dir
   TString fCurl;               //full path to executable used to copy files

   virtual Bool_t OpenAvailableRunsFile();
   virtual void CloseAvailableRunsFile();
   void init();

public:

   KVRemoteAvailableRunsFile();
   KVRemoteAvailableRunsFile(const Char_t* type, KVDataSet* ds);
   virtual ~ KVRemoteAvailableRunsFile();

   void Remove(Int_t, const Char_t* /*filename*/ = "")
   {
      Info("Remove", "Not possible for remote available runs files");
   };
   void Update(Bool_t /*no_existing_file*/ = kFALSE)
   {
      Info("Update", "Not possible for remote available runs files");
   };
   void Add(Int_t, const Char_t*)
   {
      Info("Add", "Not possible for remote available runs files");
   };

   ClassDef(KVRemoteAvailableRunsFile, 1)       //Access to lists of available runfiles in remote data repository via HTTP
};

#endif
