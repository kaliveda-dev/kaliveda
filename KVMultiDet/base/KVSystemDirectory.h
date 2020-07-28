//Created by KVClassFactory on Mon Sep 24 10:45:52 2012
//Author: John Frankland,,,

#ifndef __KVSYSTEMDIRECTORY_H
#define __KVSYSTEMDIRECTORY_H

#include "TSystemDirectory.h"

/**
  \class KVSystemDirectory
  \brief Extension of ROOT TSystemDirectory class, handling browsing directories on disk
  \ingroup Core
 */
class KVSystemDirectory : public TSystemDirectory {
   TList* fContents;//contents of directory
   TList* fSubdirs;//subdirectories of directory
   Long64_t fTotFiles;//total size of files in directory
   Long64_t fTotDirs;//total size of files in subdirectories
   virtual void   GetListings();

public:
   KVSystemDirectory();
   KVSystemDirectory(const Char_t* dirname, const Char_t* path);
   virtual ~KVSystemDirectory();
   virtual TList* GetListOfFiles() const;
   virtual TList* GetListOfDirectories() const;
   Long64_t GetTotalFiles() const
   {
      return fTotFiles;
   }
   Long64_t GetTotalSubdirs() const
   {
      return fTotDirs;
   }
   Long64_t GetTotalSize() const
   {
      return (fTotDirs + fTotFiles);
   }
   void ls(Option_t* option = "") const;

   ClassDef(KVSystemDirectory, 1) //TSystemDirectory which uses KVSystemFile
};

#endif
