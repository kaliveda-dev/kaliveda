//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#include "IRODS.h"
#include "TSystem.h"
#include "TList.h"
#include "TObjArray.h"
#include "TObjString.h"

ClassImp(IRODS)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>IRODS</h2>
<h4>Interface to IRODS commands</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

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
   buildCommand("iput", args.Data(), "-N0");
   return execCommand();
}

Int_t IRODS::get(const Char_t* source, const Char_t* target)
{
   // copy a file from IRODS space.

   TString args;
   args.Form("%s %s", source, target);
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
   //  frankland         0 demoResc1                       0 2012-10-18.17:30 & CheckChIoSi_129Xe119Sn27_R8129.o5553080
   //Info("ExtractFileInfos","%s",s.Data());
   TObjArray* fstats = s.Tokenize(" ");
   //Info("ExtractFileInfos","%d",fstats->GetEntries());
   f->SetName(((TObjString*)(*fstats)[fstats->GetEntries() - 1])->String().Remove(TString::kBoth, ' ').Data());
   f->SetSize((UInt_t)((TObjString*)(*fstats)[3])->String().Remove(TString::kBoth, ' ').Atoi());
   KVDatime mt(((TObjString*)(*fstats)[4])->String().Remove(TString::kBoth, ' ').Data(), KVDatime::kIRODS);
   f->SetModTime(mt);
   delete fstats;
}
