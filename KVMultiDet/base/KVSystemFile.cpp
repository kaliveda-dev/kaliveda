//Created by KVClassFactory on Mon Sep 24 10:45:24 2012
//Author: John Frankland,,,

#include "KVSystemFile.h"
#include "KVBase.h"
#include "TROOT.h"

ClassImp(KVSystemFile)


KVSystemFile::KVSystemFile()
{
   // Default constructor
   fUserInfo = 0;
}

//________________________________________________________________

KVSystemFile::KVSystemFile(const Char_t* filename, const Char_t* dirname)
   : TSystemFile(filename, dirname)
{
   // Get file infos
   AssignAndDelete(fFullPath, gSystem->ConcatFileName(dirname, filename));
   gSystem->GetPathInfo(fFullPath, fFileInfos);
   fUserInfo = gSystem->GetUserInfo(fFileInfos.fUid);
}

KVSystemFile::~KVSystemFile()
{
   // Destructor
   SafeDelete(fUserInfo);
}


void KVSystemFile::ls(Option_t*) const
{
   TROOT::IndentLevel();
   printf("[%s:%s]\t%s\t%12lld\t%s\n",
          GetUser(), GetGroup(), GetDate(), GetSize(), GetName());
}
