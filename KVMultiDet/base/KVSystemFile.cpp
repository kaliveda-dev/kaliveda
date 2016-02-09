//Created by KVClassFactory on Mon Sep 24 10:45:24 2012
//Author: John Frankland,,,

#include "KVSystemFile.h"
#include "KVBase.h"
#include "TROOT.h"

ClassImp(KVSystemFile)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSystemFile</h2>
<h4>TSystemFile with added info on file size etc.</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

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
   TString path;
   AssignAndDelete(path, gSystem->ConcatFileName(dirname, filename));
   gSystem->GetPathInfo(path, fFileInfos);
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
