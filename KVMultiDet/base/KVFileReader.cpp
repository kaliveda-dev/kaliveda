//Created by KVClassFactory on Tue Jul 13 11:52:58 2010
//Author: Eric Bonnet

#include "KVFileReader.h"

ClassImp(KVFileReader)

KVFileReader::KVFileReader()
{
   // Default constructor
   init();
}

//______________________
KVFileReader::KVFileReader(const KVFileReader& obj) : KVBase()
{
   //copy ctor
   init();
   obj.Copy(*this);
}

//___________________________________________________________________________________

void KVFileReader::Copy(TObject& obj) const
{
   //Copy this to obj
   //Redefinition of KVBase::Copy

   KVBase::Copy(obj);
}
