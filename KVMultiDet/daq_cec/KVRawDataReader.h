/*
$Id: KVRawDataReader.h,v 1.3 2007/06/08 15:49:10 franklan Exp $
$Revision: 1.3 $
$Date: 2007/06/08 15:49:10 $
*/

//Created by KVClassFactory on Wed May 16 15:52:21 2007
//Author: franklan

#ifndef __KVRAWDATAREADER_H
#define __KVRAWDATAREADER_H

#include "KVBase.h"
#include "KVSeqCollection.h"

#include <KVNameValueList.h>

/**
\class KVRawDataReader
\brief Abstract base class for reading raw (DAQ) data
  \ingroup DAQ

Classes derived from this one must implement the methods

  - Bool_t GetNextEvent();
  - TString GetDataFormat() const;
  - Int_t GetRunNumberReadFromFile() const;
*/

class KVRawDataReader : public KVBase {

protected:
   KVNameValueList fRunInfos;//! informations on run extracted from file

public:

   KVRawDataReader();
   virtual ~KVRawDataReader();

   virtual Bool_t GetNextEvent() = 0;
   virtual Int_t GetStatus() const
   {
      return 0;
   }
   virtual TString GetDataFormat() const = 0;
   virtual Int_t GetRunNumberReadFromFile() const = 0;

   static KVRawDataReader* OpenFile(const TString& type, const TString& filename);
   const KVNameValueList& GetRunInfos() const
   {
      return fRunInfos;
   }

   ClassDef(KVRawDataReader, 0) //Base class for reading raw data
};

#endif
