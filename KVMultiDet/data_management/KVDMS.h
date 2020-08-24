//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#ifndef __KVDMS_H
#define __KVDMS_H

#include "KVBase.h"
#include "KVDatime.h"

class KVUniqueNameList;

/**
\class KVDMS
\brief Abstract base class for interfaces to Data Management Systems (SRB, IRODS, etc.)
\ingroup Infrastructure
*/


class DMSFile_t : public KVBase {
   KVDatime fModTime; // last modification
   Bool_t fIsCont;// kTRUE if is container

public:
   DMSFile_t() : KVBase()
   {
      fIsCont = kFALSE;
   };
   virtual ~DMSFile_t() {};

   KVDatime GetModTime() const
   {
      return fModTime;
   };
   void SetModTime(KVDatime& d)
   {
      fModTime = d;
   };
   UInt_t GetSize() const
   {
      return GetNumber();
   };
   void SetSize(UInt_t s)
   {
      SetNumber(s);
   };
   void SetIsContainer(Bool_t yes = kTRUE)
   {
      fIsCont = yes;
   };
   Bool_t IsContainer() const
   {
      return fIsCont;
   };
   void ls(Option_t* opt = "") const;

   ClassDef(DMSFile_t, 1) //Attributes of Data Management Service file/container
};

class KVDMS : public KVBase {
   TString fexec;//string holding full path to executable
   TString fcmd;//string holding full path to command line with arguments
   TString fout;//string holding results of command

protected:
   Bool_t   buildCommand(const Char_t* scmd, const Char_t* args = "", Option_t* opts = "");
   Int_t    execCommand();
   TString  pipeCommand();

   /* take line from directory listing and extract infos on file size, modification date, etc. */
   virtual void ExtractFileInfos(TString&, DMSFile_t*) const = 0;

public:
   virtual Int_t init() = 0;
   virtual Int_t exit() = 0;

   /* produce simple listing of directory, with just the name of each file/container on a separate line */
   virtual TString list(const Char_t* directory = "") = 0;

   /* produce full listing of directory, with for each file/container the name, size, owner, date, etc. on a separate line */
   virtual TString longlist(const Char_t* directory = "") = 0;

   virtual Int_t cd(const Char_t* directory = "") = 0;
   virtual Int_t chmod(const Char_t* path, UInt_t mode) = 0;
   virtual Int_t put(const Char_t* source, const Char_t* target = ".") = 0;
   virtual Int_t get(const Char_t* source, const Char_t* target = ".") = 0;
   virtual TString info(const Char_t* file, Option_t* opt = "") = 0;
   virtual Int_t forcedelete(const Char_t* path) = 0;
   virtual Int_t mkdir(const Char_t* path, Option_t* opt = "") = 0;

   virtual Bool_t IsContainer(TString&) const;

   KVDMS(const Char_t* name = "DMS", const Char_t* title = "Data Management System");
   virtual ~KVDMS();

   virtual KVUniqueNameList* GetFullListing(const Char_t* directory = "");
   virtual TList* GetListing(const Char_t* directory = "");
   virtual Bool_t DirectoryContains(const Char_t* name, const Char_t* directory = "");
   virtual Bool_t GetPathInfo(const Char_t* path, DMSFile_t& fs);

   ClassDef(KVDMS, 1) //Interface to Data Management Service (SRB, IRODS, etc.)
};

#endif
