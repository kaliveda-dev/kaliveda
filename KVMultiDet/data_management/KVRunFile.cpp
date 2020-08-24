//Created by KVClassFactory on Thu Aug  4 12:08:39 2016
//Author: John Frankland,,,

#include "KVRunFile.h"

ClassImp(KVRunFile)

KVRunFile::KVRunFile()
   : KVBase(), fRun(nullptr)
{
   // Default constructor
}

KVRunFile::KVRunFile(KVDBRun* r, const KVString& f, const KVDatime& d, const KVString& v, const KVString& w)
   : KVBase(f, "KVRunFile"), fRun(r), fFileWritten(d), fVersion(v), fUser(w)
{

}

//____________________________________________________________________________//

KVRunFile::~KVRunFile()
{
   // Destructor
}

//____________________________________________________________________________//

Int_t KVRunFile::Compare(const TObject* obj) const
{
   // Compare two run numbers for sorting lists.
   // Lists will be sorted in ascending order.

   KVRunFile* dbobj =
      dynamic_cast < KVRunFile* >(const_cast < TObject* >(obj));
   return (dbobj->GetRunNumber() ==
           GetRunNumber() ? 0 : (dbobj->GetRunNumber() > GetRunNumber() ? -1 : 1));
}
