//Created by KVClassFactory on Fri Jul 17 15:10:29 2009
//Author: John Frankland

#ifndef __SRB_H
#define __SRB_H

#include "KVDMS.h"

/**
  \class SRB
  \ingroup Infrastructure
  \brief Interface to SRB (Storage Resource Broker) DMS client
 */
class SRB : public KVDMS {
protected:
   virtual void ExtractFileInfos(TString&, DMSFile_t*) const;

public:
   SRB();
   virtual ~SRB();
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
   virtual Int_t chmod(const Char_t*, UInt_t)
   {
      // not implemented for SRB
      return 0;
   };

   ClassDef(SRB, 1) //Interface to SRB commands
};

#endif
