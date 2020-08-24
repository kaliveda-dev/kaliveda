//Created by KVClassFactory on Fri Jul 17 15:10:29 2009
//Author: John Frankland

#include "SRB.h"
#include "TList.h"
#include "TObjArray.h"
#include "TObjString.h"

ClassImp(SRB)

SRB::SRB()
   : KVDMS("SRB", "Storage Resource Broker")
{
}

SRB::~SRB()
{
}

Int_t SRB::init()
{
   // Call in interactive session if you want to use relative pathnames
   buildCommand("Sinit");
   return execCommand();
}

TString SRB::list(const Char_t* directory)
{
   // returns simple listing of current directory (default) or given directory.
   // returns empty string if directory is unknown.

   buildCommand("Sls", directory);
   return pipeCommand();
}

TString SRB::longlist(const Char_t* directory)
{
   // returns long-format listing of current directory (default) or given directory.
   // returns empty string if directory is unknown.

   buildCommand("Sls", directory, "-l");
   return pipeCommand();
}

Int_t SRB::cd(const Char_t* directory)
{
   // change directory

   buildCommand("Scd", directory);
   return execCommand();
}

TString SRB::info(const Char_t* file, Option_t* opt)
{
   // returns meta-infos on given file.

   buildCommand("SgetD", file, opt);
   return pipeCommand();
}

Int_t SRB::put(const Char_t* source, const Char_t* target)
{
   // put a new file into SRB space.
   // options "-v -M" are used by default.

   TString args;
   args.Form("%s %s", source, target);
   buildCommand("Sput", args.Data(), "-v -M");
   return execCommand();
}

Int_t SRB::get(const Char_t* source, const Char_t* target)
{
   // copy a file from SRB space.
   // options "-v -M" are used by default.

   TString args;
   args.Form("%s %s", source, target);
   buildCommand("Sget", args.Data(), "-v -M");
   return execCommand();
}

Int_t SRB::forcedelete(const Char_t* path)
{
   // Delete a file
   // WARNING: THIS WILL DELETE THE PHYSICAL FILE,
   // not just the catalogue entry!!!

   buildCommand("Srm", path, "-f");
   return execCommand();
}

Int_t SRB::mkdir(const Char_t* path, Option_t* opt)
{
   // Create new directory in SRB space

   buildCommand("Smkdir", path, opt);
   return execCommand();
}

Int_t SRB::exit()
{
   // Call at end of interactive session (i.e. if you previously called Sinit())
   buildCommand("Sexit");
   return execCommand();
}

void SRB::ExtractFileInfos(TString& s, DMSFile_t* f) const
{
   // extract info (name, size, modification date) from listing line
   // such as :
   //   nief        0 Lyon                    230663725 2008-12-19-15.21   run788.root.2006-10-30_08:03:15
   TObjArray* fstats = s.Tokenize(" ");
   f->SetName(((TObjString*)(*fstats)[5])->String().Remove(TString::kBoth, ' ').Data());
   f->SetSize((UInt_t)((TObjString*)(*fstats)[3])->String().Remove(TString::kBoth, ' ').Atoi());
   KVDatime mt(((TObjString*)(*fstats)[4])->String().Remove(TString::kBoth, ' ').Data(), KVDatime::kSRB);
   f->SetModTime(mt);
   delete fstats;
}
