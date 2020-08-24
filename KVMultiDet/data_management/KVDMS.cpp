//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#include "KVDMS.h"
#include "TSystem.h"
#include "TList.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TROOT.h"
#include "Riostream.h"

#include "KVUniqueNameList.h"

using namespace std;

ClassImp(KVDMS)

KVDMS::KVDMS(const Char_t* name, const Char_t* title)
   : KVBase(name, title)
{
   // Default ctor with name and title
}

KVDMS::~KVDMS()
{
}

Bool_t KVDMS::buildCommand(const Char_t* scmd, const Char_t* args, Option_t* opts)
{
   // scmd = one of the DMS commands (init, ls, get, etc.)
   // args = (optional) list of arguments for command (filename, etc.)
   // opts = (optional) list of options for command
   // If the command give in 'scmd' is found in the path specified by the
   // user's PATH environment variables, this method returns kTRUE and
   // private member variables fexec and fcmd will hold the full path to
   // the executable and the full command line to pass to gSystem->Exec.
   // If executable is not found, returns kFALSE.

   fexec = scmd;
   if (!FindExecutable(fexec)) {
      fexec = "";
      fcmd = "";
      return kFALSE;
   }
   fcmd = fexec + " ";
   fcmd += opts;
   fcmd += " ";
   fcmd += args;
   return kTRUE;
}

Int_t KVDMS::execCommand()
{
   // Execute last command initialised with buildCommand() and return
   // the value given by the operating system.
   // Returns -1 if no command has been defined.

   if (fcmd == "") return -1;
   return gSystem->Exec(fcmd.Data());
}

TString KVDMS::pipeCommand()
{
   // Execute last command initialised with buildCommand() and return
   // the value given by the operating system in a string.

   if (fcmd == "") return TString("");
   fout = gSystem->GetFromPipe(fcmd.Data());
   return fout;
}

Bool_t KVDMS::IsContainer(TString& line) const
{
   // 'line' is a line in a directory listing
   // This method should return kTRUE if the line corresponds to a container

   return (line.BeginsWith("C-"));
}

KVUniqueNameList* KVDMS::GetFullListing(const Char_t* directory)
{
   // Create and fill TList with info (name, size, modification date)
   // on all files & containers in current directory
   // (default) or in given directory.
   // TList is filled with DMSFile_t objects which belong to the list, list must be deleted after use.

   longlist(directory);
   if (fout == "") {
      Error("GetListing", "Unknown directory %s", directory);
      return 0;
   }

   TObjArray* toks = fout.Tokenize("\n");
   KVUniqueNameList* list = new KVUniqueNameList(kTRUE);
   list->SetOwner(kTRUE);
   list->SetName(((TObjString*)(*toks)[0])->String().Remove(TString::kBoth, ' ').Data());
   for (int i = 1; i < toks->GetEntries(); i++) {
      TString tmp = ((TObjString*)(*toks)[i])->String().Remove(TString::kBoth, ' ');
      DMSFile_t* f = new DMSFile_t;
      if (IsContainer(tmp)) { // container
         f->SetName(gSystem->BaseName(tmp.Data()));
         f->SetIsContainer();
      }
      else {
         ExtractFileInfos(tmp, f);
      }
      list->Add(f);
   }
   delete toks;
   return list;
}

Bool_t KVDMS::GetPathInfo(const Char_t* path, DMSFile_t& fs)
{
   // Returns kTRUE if 'path' exists (file or directory, absolute or relative pathname).
   // If so, KVDMSFile_t object will be filled with information on file/container

   TString filename = gSystem->BaseName(path);
   TString dirname = gSystem->DirName(path);
   if (DirectoryContains(filename.Data(), dirname.Data())) {
      longlist(path);
      fout.Remove(TString::kBoth, ' ');
      fs.SetName(filename.Data());
      if (IsContainer(fout)) { // container
         fs.SetIsContainer();
      }
      else {
         ExtractFileInfos(fout, &fs);
      }
      return kTRUE;
   }
   return kFALSE;
}

TList* KVDMS::GetListing(const Char_t* directory)
{
   // Create and fill TList with just the names of files & containers in current directory
   // (default) or in given directory.
   // TList is filled with DMSFile_t objects which belong to the list, list must be deleted after use.

   list(directory);
   if (fout == "") {
      Error("GetListing", "Unknown directory %s", directory);
      return 0;
   }

   TObjArray* toks = fout.Tokenize("\n");
   TList* list = new TList;
   list->SetOwner(kTRUE);
   list->SetName(((TObjString*)(*toks)[0])->String().Remove(TString::kBoth, ' ').Data());
   for (int i = 1; i < toks->GetEntries(); i++) {
      TString tmp = ((TObjString*)(*toks)[i])->String().Remove(TString::kBoth, ' ');
      DMSFile_t* f = new DMSFile_t;
      if (IsContainer(tmp)) { // container
         f->SetName(gSystem->BaseName(tmp.Data()));
         f->SetIsContainer();
      }
      else {
         f->SetName(tmp.Data());
      }
      list->Add(f);
   }
   delete toks;
   return list;
}

Bool_t KVDMS::DirectoryContains(const Char_t* name, const Char_t* directory)
{
   // Returns true if the current directory (default) or the given directory
   // contains a file or a container with given name.

   TList* list = GetListing(directory);
   if (!list)
      return kFALSE;
   Bool_t ok = list->FindObject(name);
   delete list;
   return ok;
}

ClassImp(DMSFile_t)

/////////////////////////////////////////////
// DMSFile_t
//
// Describes Data Management System file/container attributes
/////////////////////////////////////////////

void DMSFile_t::ls(Option_t* /*opt*/) const
{
   // List file/container attributes

   TROOT::IndentLevel();
   if (IsContainer())
      cout << GetName() << "/" << endl;
   else
      cout << GetName() << "\t" << GetSize() << "\t" << GetModTime().AsString() << endl;
}
