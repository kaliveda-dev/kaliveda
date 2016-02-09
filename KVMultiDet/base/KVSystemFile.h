//Created by KVClassFactory on Mon Sep 24 10:45:24 2012
//Author: John Frankland,,,

#ifndef __KVSYSTEMFILE_H
#define __KVSYSTEMFILE_H

#include "TSystemFile.h"
#include "TSystem.h"
#include "TDatime.h"

class KVSystemFile : public TSystemFile {
   FileStat_t fFileInfos;
   UserGroup_t* fUserInfo;

public:
   KVSystemFile();
   KVSystemFile(const Char_t* filename, const Char_t* dirname);
   virtual ~KVSystemFile();

   void ls(Option_t* option = "") const;

   const Char_t* GetUser() const
   {
      if (fUserInfo) return fUserInfo->fUser;
      return "";
   };
   const Char_t* GetGroup() const
   {
      if (fUserInfo) return fUserInfo->fGroup;
      return "";
   };
   Long64_t GetSize() const
   {
      return fFileInfos.fSize;
   };
   const Char_t* GetDate() const
   {
      TDatime when(fFileInfos.fMtime);
      return when.AsSQLString();
   };

   ClassDef(KVSystemFile, 1) //TSystemFile with added info on file size etc.
};

#endif
