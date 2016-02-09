/***************************************************************************
                          kvrlist.h  -  description
                             -------------------
    begin                : Thu Mar 13 2003
    copyright            : (C) 2003 by J.D Frankland & Alexis Mignon
    email                : frankland@ganil.fr, mignon@ganil.fr

$Id: KVRList.h,v 1.15 2009/01/19 12:03:10 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVRLIST_H
#define KVRLIST_H

#define KVRLIST_NOT_WITH_THIS_TYPE "No object found with type %s"
#define KVRLIST_NOT_WITH_THIS_NAME_AND_TYPE "No object found with name %s and type %s"

#include "KVBase.h"
#include "RVersion.h"
#include "TRefArray.h"
#include "KVConfig.h"

class KVRList: public TRefArray {

public:
#ifdef __WITH_OLD_TREFARRAY
   KVRList() : TRefArray() {};
   KVRList(Int_t size, Int_t lowerBound = 0) : TRefArray(size, lowerBound) {};
#else
   KVRList(TProcessID* pid = 0) : TRefArray(pid) {};
   KVRList(Int_t s, TProcessID* pid) : TRefArray(s, pid) {};
   KVRList(Int_t size, Int_t lowerBound = 0, TProcessID* pid = 0) : TRefArray(size, lowerBound, pid) {};
#endif

   virtual ~ KVRList() {};
   virtual KVBase* FindObjectByName(const Char_t*) const;
   virtual KVBase* FindObjectByType(const Char_t*) const;
   virtual KVBase* FindObjectByLabel(const Char_t*) const;
   virtual KVBase* FindObject(const Char_t* name,
                              const Char_t* type) const;
   virtual KVBase* FindObject(KVBase* obj) const;
   virtual TObject* FindObject(const TObject* obj) const
   {
      return TCollection::FindObject(obj);
   };
   virtual TObject* FindObject(const char* obj) const
   {
      return TCollection::FindObject(obj);
   };

   virtual void Execute(const char* method, const char* params,
                        Int_t* error = 0);
   virtual void Execute(TMethod* m, TObjArray* o, Int_t* i)
   {
      TObject::Execute(m, o, i);
   };

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif
   virtual Int_t GetSize() const
   {
      return GetLast() + 1;
   };
   virtual void Print(Option_t* opt = "") const;
#ifdef __WITH_NEW_TCOLLECTION_PRINT
   virtual void       Print(Option_t* option, Int_t recurse) const
   {
      TCollection::Print(option, recurse);
   };
   virtual void       Print(Option_t* option, const char* wildcard, Int_t recurse = 1) const
   {
      TCollection::Print(option, wildcard, recurse);
   };
   virtual void       Print(Option_t* option, TPRegexp& regexp, Int_t recurse = 1) const
   {
      TCollection::Print(option, regexp, recurse);
   };
#else
#ifdef __WITH_TCOLLECTION_PRINT_WILDCARD
   virtual void Print(Option_t* wildcard, Option_t* opt) const
   {
      TCollection::Print(wildcard, opt);
   };
#endif
#endif

   ClassDef(KVRList, 1)         //KV wrapper for ROOT TRefArray class
};

#endif
