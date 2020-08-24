//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#include "IRODS.h"
#include "TSystem.h"
#include "TList.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "KVConfig.h"

ClassImp(IRODS)

IRODS::IRODS()
   : KVDMS("IRODS", "Integrated Rule-Oriented Data System")
{
}

IRODS::~IRODS()
{
}

Int_t IRODS::init()
{
   // Call in interactive session if you want to use relative pathnames
   buildCommand("iinit");
   return execCommand();
}

TString IRODS::list(const Char_t* directory)
{
   // returns simple listing of current directory (default) or given directory.
   // returns empty string if directory is unknown.

   buildCommand("ils", directory);
   return pipeCommand();
}

TString IRODS::longlist(const Char_t* directory)
{
   // returns long-format listing of current directory (default) or given directory.
   // returns empty string if directory is unknown.
   buildCommand("ils", directory, "-l");
   return pipeCommand();
}

Int_t IRODS::cd(const Char_t* directory)
{
   // change directory

   buildCommand("icd", directory);
   return execCommand();
}

TString IRODS::info(const Char_t* file, Option_t* opt)
{
   // returns meta-infos on given file.

   buildCommand("imeta", file, opt);
   return pipeCommand();
}

Int_t IRODS::put(const Char_t* source, const Char_t* target)
{
   // put a new file into IRODS space.

   TString args;
   args.Form("%s %s", source, target);
   buildCommand("iput", args.Data(), "-KP");
   return execCommand();
}

Int_t IRODS::get(const Char_t* source, const Char_t* target)
{
   // copy a file from IRODS space.

   TString args;
   args.Form("-P %s %s", source, target);
   buildCommand("iget", args.Data());
   return execCommand();
}

Int_t IRODS::forcedelete(const Char_t* path)
{
   // Delete a file.
   // WARNING: THIS WILL DELETE THE PHYSICAL FILE,
   // not just the catalogue entry!!!

   buildCommand("irm", path, "-f");
   return execCommand();
}

Int_t IRODS::chmod(const Char_t*, UInt_t)
{
   // Change access permissions for file/directory

   //buildCommand("ichmod",path, Form("%ud",mode));
   return 0;//execCommand();
}

Int_t IRODS::mkdir(const Char_t* path, Option_t* opt)
{
   // Create new directory in IRODS space

   buildCommand("imkdir", path, opt);
   return execCommand();
}

Int_t IRODS::exit()
{
   // Call at end of interactive session (i.e. if you previously called iinit())
   buildCommand("iexit");
   return execCommand();
}

void IRODS::ExtractFileInfos(TString& s, DMSFile_t* f) const
{
   // extract info (name, size, modification date) from listing line
   // such as :
   //   indramgr          0 diskcache2              208207872 2009-10-06.16:56 & run_0002.dat.15-Feb-07.18:00:58
   //   indramgr          1 HPSS2                   208207872 2009-10-06.16:56 & run_0002.dat.15-Feb-07.18:00:58
   //   indramgr          0 diskcache2              208224256 2009-10-06.16:56 & run_0003.dat.15-Feb-07.20:21:50
   //   indramgr          1 HPSS2                   208224256 2009-10-06.16:56 & run_0003.dat.15-Feb-07.20:21:50
   //

   TObjArray* fstats = s.Tokenize(" ");
   f->SetName(((TObjString*)(*fstats)[fstats->GetEntries() - 1])->String().Remove(TString::kBoth, ' ').Data());
   f->SetSize((UInt_t)((TObjString*)(*fstats)[3])->String().Remove(TString::kBoth, ' ').Atoi());
   KVDatime mt(((TObjString*)(*fstats)[4])->String().Remove(TString::kBoth, ' ').Data(), KVDatime::kIRODS);
   f->SetModTime(mt);
   delete fstats;
}
