/*
$Id: KVDatedFileManager.h,v 1.2 2007/05/03 11:05:47 franklan Exp $
$Revision: 1.2 $
$Date: 2007/05/03 11:05:47 $
*/

//Created by KVClassFactory on Wed May  2 14:50:40 2007
//Author: franklan

#ifndef __KVDatedFileManager_H
#define __KVDatedFileManager_H

#include "TNamed.h"
#include "KVDatime.h"
#include "KVString.h"
#include "KVList.h"

class KVSortableDatedFile : public TNamed {
   KVDatime fTimestamp;//timestamp extracted from filename

public:

   KVSortableDatedFile() {};
   KVSortableDatedFile(const Char_t* filename, const Char_t* basename);
   virtual ~KVSortableDatedFile() {};
   virtual Int_t Compare(const TObject* obj) const;
   virtual Bool_t IsEqual(const TObject* obj) const;
   KVDatime GetTimeStamp() const
   {
      return fTimestamp;
   };

   ClassDef(KVSortableDatedFile, 1)//A filename with an SQL-format date extension which can be used to sort a list according to date
};

//__________________________________________________________________________________________________//

class KVDatedFileManager {
   KVString fBaseName;//the base name of the file
   KVString fDirectory;//the directory containg the files
   KVList     fFileList;//list of files sorted by timestamp

   void ReadFiles();

public:

   KVDatedFileManager() {};
   KVDatedFileManager(const Char_t* base, const Char_t* dir);
   virtual ~KVDatedFileManager() {};

   const Char_t* GetPreviousVersion(const Char_t* name);
   const Char_t* GetOldestVersion();
   void Update();

   ClassDef(KVDatedFileManager, 1) //Handles a set of different versions of files in a directory with the same base name and a timestamp
};

#endif
