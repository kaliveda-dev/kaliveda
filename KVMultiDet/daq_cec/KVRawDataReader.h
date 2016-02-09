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

class KVRawDataReader : public KVBase {
public:

   KVRawDataReader();
   virtual ~KVRawDataReader();

   virtual const KVSeqCollection* GetUnknownParameters() const
   {
      return 0;
   };
   virtual const KVSeqCollection* GetRawDataParameters() const
   {
      return 0;
   };
   virtual Bool_t GetNextEvent() = 0;
   virtual KVSeqCollection* GetFiredDataParameters() const = 0;

   virtual Int_t GetStatus() const
   {
      return 0;
   };

   ClassDef(KVRawDataReader, 0) //Base class for reading raw data
};

#endif
