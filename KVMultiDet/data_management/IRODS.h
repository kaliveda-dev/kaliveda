//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#ifndef __IRODS_H
#define __IRODS_H

#include "KVDMS.h"

class IRODS : public KVDMS {
protected:
   virtual void ExtractFileInfos(TString&, DMSFile_t*) const;

public:
   IRODS();
   virtual ~IRODS();
   virtual Int_t init();
   virtual Int_t exit();
   virtual TString list(const Char_t* directory = "");
   virtual TString longlist(const Char_t* directory = "");
   virtual Int_t cd(const Char_t* directory = "");
   virtual Int_t put(const Char_t* source, const Char_t* target = ".");
   virtual Int_t get(const Char_t* source, const Char_t* target = ".");
   virtual TString info(const Char_t* file, Option_t* opt = "");
   virtual Int_t forcedelete(const Char_t* path);
   virtual Int_t mkdir(const Char_t* path, Option_t* opt = "");
   virtual Int_t chmod(const Char_t* path, UInt_t mode);

   ClassDef(IRODS, 1) //Interface to IRODS commands
};

#endif
