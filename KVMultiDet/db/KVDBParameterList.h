//Created by KVClassFactory on Fri Sep 11 15:25:54 2015
//Author: ,,,

#ifndef __KVDBPARAMETERLIST_H
#define __KVDBPARAMETERLIST_H

#include "KVDBRecord.h"
#include "KVNameValueList.h"

class KVDBParameterList : public KVDBRecord {

   KVNameValueList* fParameters;
public:

   KVDBParameterList();
   KVDBParameterList(const Char_t* name, const Char_t* title = "");
   KVNameValueList* GetParameters() const;
   virtual ~KVDBParameterList();
   void Print(Option_t* option = "") const;

   ClassDef(KVDBParameterList, 1) //To store in a database some parameters list
};

#endif
