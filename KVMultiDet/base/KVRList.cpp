/***************************************************************************
                          kvrlist.cpp  -  description
                             -------------------
    begin                : Thu Mar 13 2003
    copyright            : (C) 2003 by J.D Frankland & Alexis Mignon
    email                : frankland@ganil.fr, mignon@ganil.fr

$Id: KVRList.cpp,v 1.20 2009/01/14 11:46:30 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVRList.h"
#include "TObject.h"
#include "Riostream.h"
#include "KVError.h"

ClassImp(KVRList);

KVBase* KVRList::FindObjectByName(const Char_t* name) const
{
   //Look for object with name "name" in the list
   return (KVBase*) TRefArray::FindObject(name);
}

//_______________________________________________________________________________
KVBase* KVRList::FindObjectByType(const Char_t* type) const
{
   //
   // Find an object in this list using its type. Requires a sequential
   // scan till the object has been found. Returns 0 if object with specified
   // name is not found.
   //

   if (type) {
      if (GetSize()) {          //make sure list contains some objects
         for (Int_t i = 0; i < GetSize(); i++) {
            KVBase* obj = (KVBase*) At(i);
            if (obj->GetType() && !strcmp(type, obj->GetType()))
               return obj;
         }
      }
   }
//   Warning("GetObjectByType",KVRLIST_NOT_WITH_THIS_TYPE,type);
   return 0;
}

//_______________________________________________________________________________
KVBase* KVRList::FindObjectByLabel(const Char_t* type) const
{
   //
   // Find an object in this list using its label. Requires a sequential
   // scan till the object has been found. Returns 0 if object with specified
   // label is not found.
   //

   if (type) {
      if (GetSize()) {          //make sure list contains some objects
         for (Int_t i = 0; i < GetSize(); i++) {
            KVBase* obj = (KVBase*) At(i);
            if (obj->GetLabel() && !strcmp(type, obj->GetLabel()))
               return obj;
         }
      }
   }
//   Warning("GetObjectByType",KVRLIST_NOT_WITH_THIS_TYPE,type);
   return 0;
}

//_______________________________________________________________________________
KVBase* KVRList::FindObject(const Char_t* name, const Char_t* type) const
{
//
// Find an object in this list using its type and name. Requires a sequential
// scan till the object has been found. Returns 0 if object with specified
// name is not found.
//

   if (type && name) {
      if (GetSize()) {          //make sure list contains some objects
         for (Int_t i = 0; i < GetSize(); i++) {
            KVBase* obj = (KVBase*) At(i);
            if (!strcmp(type, obj->GetType())
                  && !strcmp(name, obj->GetName()))
               return obj;
         }
      }
   }
//                                                                                                                                                                                                                                                             Warning("GetObject",KVRLIST_NOT_WITH_THIS_NAME_AND_TYPE,name,type);
   return 0;
}

//_______________________________________________________________________________

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVRList::Copy(TObject& obj) const
#else
void KVRList::Copy(TObject& obj)
#endif
{
   //
   //Copy this list to obj
   //As a KVRList is a list of references to objects, here we are simply
   //copying the references, i.e. no new objects are created
   //
   obj.Clear();
   if (this->GetSize()) {       //make sure list contains some objects
      for (Int_t i = 0; i < this->GetSize(); i++) {
         ((KVRList&) obj).Add(this->At(i));
      }
   }
}

void KVRList::Print(Option_t* opt) const
{
   //Print description of all objects in list
#ifdef KV_DEBUG
   Info("Print", "List size: %d elements", this->GetSize());
#endif
   if (this->GetSize()) {       //make sure list contains some objects
      for (Int_t i = 0; i < this->GetSize(); i++) {
         if (At(i))
            ((KVBase*) At(i))->Print(opt);
         else
            Error("Print", "Object %d does not exist", i);
      }
   }
}

void KVRList::Execute(const char* method, const char* params,
                      Int_t* error)
{
   //Redefinition of TObject::Execute method.
   //TObject::Execute is called for each object in the list in order, meaning that for each
   //object the method "method" is executed with arguments "params".
   TIter next(this);
   TObject* obj;
   while ((obj = next())) {
      obj->Execute(method, params, error);
   }
}


KVBase* KVRList::FindObject(KVBase* obj) const
{
   return (KVBase*) TRefArray::FindObject(obj);
}
