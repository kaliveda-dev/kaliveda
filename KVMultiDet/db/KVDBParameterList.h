//Created by KVClassFactory on Fri Sep 11 15:25:54 2015
//Author: ,,,

#ifndef __KVDBPARAMETERLIST_H
#define __KVDBPARAMETERLIST_H

#include "KVDBRecord.h"
#include "KVNameValueList.h"

/**
  \class KVDBParameterList
  \brief To store calibration parameters in a database
  \ingroup Calibration,Core
 */
class KVDBParameterList : public KVDBRecord {

   KVNameValueList fParameters;
public:

   KVDBParameterList() : KVDBRecord() {}
   KVDBParameterList(const Char_t* name, const Char_t* title = "")
      : KVDBRecord(name, title)
   {}
   KVNameValueList* GetParameters()
   {
      return &fParameters;
   }
   const KVNameValueList* GetParameters() const
   {
      return &fParameters;
   }
   virtual ~KVDBParameterList() {}
   void Print(Option_t* option = "") const;

   ClassDef(KVDBParameterList, 1) //To store in a database some parameters list
};

#endif
