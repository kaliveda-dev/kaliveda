/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Wed Jun  3 08:40:52 2009
//Author: John Frankland,,,

#ifndef __KVTARARCHIVE_H
#define __KVTARARCHIVE_H

#include "KVBase.h"

class KVTarArchive : public KVBase {
   void init();

protected:
   Bool_t fOK;//set to kTRUE if directory/archive is found
   Bool_t fTGZ;//set to kTRUE if directory is extracted from '.tgz' archive
   KVString fFullpath;//full path to directory if found/extracted

   void CheckDirectory(const Char_t* dirname, const Char_t* path);
   void DeleteDirectory(const Char_t* dirpath);

public:
   KVTarArchive();
   KVTarArchive(const Char_t* dirname, const Char_t* path);
   virtual ~KVTarArchive();
   Bool_t IsOK() const
   {
      // Returns kTRUE if directory/archive found
      return fOK;
   };
   Bool_t IsArchive() const
   {
      // Returns kTRUE if directory extracted from archive file
      return fTGZ;
   };
   const Char_t* GetFullPath() const
   {
      // Returns full path to extracted directory if found (IsOK()==kTRUE)
      return fFullpath.Data();
   };


   ClassDef(KVTarArchive, 1) //Handles directories stored in .tgz archive files
};

#endif
