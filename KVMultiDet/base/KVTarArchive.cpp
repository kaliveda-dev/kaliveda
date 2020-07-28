/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Wed Jun  3 08:40:52 2009
//Author: John Frankland,,,

#include "KVTarArchive.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TList.h"

ClassImp(KVTarArchive)



KVTarArchive::KVTarArchive()
{
   // Default constructor
   init();
}

KVTarArchive::KVTarArchive(const Char_t* dirname, const Char_t* path)
{
   // Open directory/archive with name 'dirname'[.tgz] in directory 'path'

   init();
   CheckDirectory(dirname, path);
}

KVTarArchive::~KVTarArchive()
{
   // Destructor
   // If the directory was extracted from an archive file, we delete the extracted
   // directory and all files in it. This is so that the next time we need files from
   // the archive, we reopen the archive (in case the archive has been updated).

   if (fOK && fTGZ) DeleteDirectory(fFullpath.Data());
}

void KVTarArchive::init()
{
   // Default initialisations for ctors
   fOK = kFALSE;
   fTGZ = kFALSE;
   fFullpath = "";
}

void KVTarArchive::CheckDirectory(const Char_t* dirname, const Char_t* path)
{
   // We look for the directory 'dirname' in the directory 'path', i.e. we look for
   //   path/dirname
   //
   // If not found, we look for the following compressed archive:
   //
   //   path/dirname.tgz
   //
   // and if found, uncompress it (with 'tar -zxf').
   //
   // Sets fOK to kTRUE if directory/archive file exists.Full path to directory returned in 'fullpath'.
   //
   // Sets fOK to kFALSE if directory does not exist and no archive file found.

   fTGZ = kFALSE;
   fOK = kFALSE;
   KVString search = dirname;
   if (!SearchFile(search.Data(), fFullpath, 1, path)) {
      // directory not found; look for .tgz
      search += ".tgz";
      if (SearchFile(search.Data(), fFullpath, 1, path)) {
         // uncompress in directory where tgz is found
         TString pwd = gSystem->pwd();
         gSystem->cd(gSystem->DirName(fFullpath.Data()));
         TString cmd;
         cmd.Form("tar -zxf %s", gSystem->BaseName(fFullpath.Data()));
         Info("CheckDirectory", "Opening archive file %s", fFullpath.Data());
         if (gSystem->Exec(cmd.Data())) {
            Error("CheckDirectory", "Problem executing %s", cmd.Data());
            fFullpath = "";
         }
         else {
            fOK = kTRUE;
            // strip '.tgz' to leave just full path to directory
            fFullpath.Remove(fFullpath.Length() - 4);
            fTGZ = kTRUE;
         }
         gSystem->cd(pwd.Data());
      }
   }
   else {
      fOK = kTRUE;
      Info("CheckDirectory", "Found directory %s", fFullpath.Data());
   }
}

void KVTarArchive::DeleteDirectory(const Char_t* dirpath)
{
   // Delete all files in directory 'dirpath' (give full path) then delete the directory itself.
   // This is to get round the fact that gSystem->Unlink(dirname) does not work if the
   // directory contains any files.

   Info("DeleteDirectory", "Deleting %s", dirpath);
   TString pwd = gSystem->WorkingDirectory();//keep working directory
   TSystemDirectory dir("dir", dirpath);
   // get list of files (and directories)
   TList* files = dir.GetListOfFiles();
   TIter nxtFile(files);
   TSystemFile* fil = 0;
   // delete all files (and directories)
   while ((fil = (TSystemFile*)nxtFile())) {
      if (fil->IsDirectory()) {
         // recursively remove subdirectories if necessary
         if (strcmp(fil->GetName(), "..") && strcmp(fil->GetName(), ".")) {
            DeleteDirectory(fil->GetTitle());
         }
      }
      else {
         if (gSystem->Unlink(Form("%s/%s", fil->GetTitle(), fil->GetName())) == -1) {
            Info("DeleteDirectory", "Cannot delete file %s in %s", fil->GetName(), fil->GetTitle());
         }
      }
   }
   delete files;
   gSystem->cd(pwd.Data());//change back to working directory
   // delete directory
   if (gSystem->Unlink(dirpath) == 0)
      Info("DeleteDirectory", "Directory %s deleted", dirpath);
   else
      Info("DeleteDirectory", "Cannot delete directory %s", dirpath);
}
