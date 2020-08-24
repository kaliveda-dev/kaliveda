/*
$Id: KVRawDataReader.cpp,v 1.2 2007/05/31 09:59:22 franklan Exp $
$Revision: 1.2 $
$Date: 2007/05/31 09:59:22 $
*/

//Created by KVClassFactory on Wed May 16 15:52:21 2007
//Author: franklan

#include "KVRawDataReader.h"
#include "KVBase.h"
#include "TPluginManager.h"
#include "TError.h"

ClassImp(KVRawDataReader)


KVRawDataReader::KVRawDataReader()
{
   //Default constructor
}

KVRawDataReader::~KVRawDataReader()
{
   //Destructor
}

KVRawDataReader* KVRawDataReader::OpenFile(const TString& type, const TString& filename)
{
   // Generate an instance of a class to read raw data of given type
   // corresponding to a defined plugin for KVRawDataReader

   TString base;
   if (IsThisAPlugin(type, base)) {
      if (base != "KVRawDataReader") {
         ::Error("KVRawDataReader::OpenFile", "%s is plugin of wrong type (%s)", type.Data(), base.Data());
         return nullptr;
      }
      TPluginHandler* ph = LoadPlugin(base, type);
      if (ph) {
         return (KVRawDataReader*)ph->ExecPlugin(1, filename.Data());
      }
      else {
         ::Error("KVRawDataReader::OpenFile", "Failed to load plugin handler for %s", type.Data());
      }
   }
   return nullptr;
}

