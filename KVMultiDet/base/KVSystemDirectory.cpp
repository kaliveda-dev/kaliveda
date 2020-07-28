//Created by KVClassFactory on Mon Sep 24 10:45:52 2012
//Author: John Frankland,,,

#include "KVSystemDirectory.h"
#include "TSystem.h"
#include "TList.h"
#include "TROOT.h"
#include "KVSystemFile.h"
#include <iostream>

ClassImp(KVSystemDirectory)


KVSystemDirectory::KVSystemDirectory()
{
   // Default constructor
   fContents = 0;
   fSubdirs = 0;
   fTotFiles = 0;
   fTotDirs = 0;
}

KVSystemDirectory::KVSystemDirectory(const Char_t* dirname, const Char_t* path)
   : TSystemDirectory(dirname, path)
{
   // Constructor with name for directory and path.
   // Path can be given as "." for current working directory.
   // Shell variables can be used if written as "$(VARIABLE)"
   fContents = 0;
   fSubdirs = 0;
   fTotFiles = 0;
   fTotDirs = 0;
   TString Path = path;
   if (Path == ".") Path = "$(PWD)";
   gSystem->ExpandPathName(Path);
   SetTitle(Path);
   GetListings();
}

//________________________________________________________________

KVSystemDirectory::~KVSystemDirectory()
{
   // Destructor
   SafeDelete(fContents);
   SafeDelete(fSubdirs);
}

//______________________________________________________________________________
TList* KVSystemDirectory::GetListOfFiles() const
{
   // Returns a TList of KVSystemFile objects representing the contents
   // of the directory.
   // Returns 0 in case of errors.

   return fContents;
}

//______________________________________________________________________________
TList* KVSystemDirectory::GetListOfDirectories() const
{
   // Returns a TList of KVSystemDirectory objects representing the contents
   // of the directory.
   // Returns 0 in case of errors.

   return fSubdirs;
}

void KVSystemDirectory::GetListings()
{
   // Fill lists of files and directories

   void* dir = gSystem->OpenDirectory(GetTitle());
   if (!dir) {
      Error("GetListings", "Cannot open directory %s", GetTitle());
      return;
   }

   const char* file = 0;
   fContents  = new TList;
   fContents->SetOwner();
   fSubdirs  = new TList;
   fSubdirs->SetOwner();
   while ((file = gSystem->GetDirEntry(dir))) {
      if (IsItDirectory(file)) {
         TString sdirpath = "";
         if (file[0] == '.' && file[1] == '\0')
            continue;
         else if (file[0] == '.' && file[1] == '.' && file[2] == '\0')
            continue;
         else {
            sdirpath = GetTitle();
            if (!sdirpath.EndsWith("/"))
               sdirpath += "/";
            sdirpath += file;
         }
         if (sdirpath != "") {
            KVSystemDirectory* sd = new KVSystemDirectory(file, sdirpath.Data());
            fSubdirs->Add(sd);
            fTotDirs += sd->GetTotalSize();
         }
      }
      else {
         KVSystemFile* sf = new KVSystemFile(file, GetTitle());
         fContents->Add(sf);
         fTotFiles += sf->GetSize();
      }
   }
   gSystem->FreeDirectory(dir);
}

void KVSystemDirectory::ls(Option_t* opt) const
{
   // if opt="nosubdirs", do not list contents of subdirectories
   // default is to list recursively contents of all subdirectories

   TROOT::IndentLevel();
   printf("%s :\n[total : %lld / files : %lld bytes]\n\n",
          GetTitle(), GetTotalSize(), GetTotalFiles());
   TROOT::IncreaseDirLevel();
   TROOT::IncreaseDirLevel();
   GetListOfFiles()->R__FOR_EACH(KVSystemFile, ls)();
   if (!strcmp(opt, "nosubdirs")) {
      TIter next(GetListOfDirectories());
      KVSystemDirectory* sd;
      while ((sd = (KVSystemDirectory*)next())) {
         TROOT::IndentLevel();
         printf("%-60s [total : %lld / files : %lld bytes]\n",
                sd->GetTitle(), sd->GetTotalSize(), sd->GetTotalFiles());
      }
   }
   else {
      GetListOfDirectories()->R__FOR_EACH(KVSystemDirectory, ls)();
   }
   TROOT::DecreaseDirLevel();
   TROOT::DecreaseDirLevel();
   printf("\n");
}
