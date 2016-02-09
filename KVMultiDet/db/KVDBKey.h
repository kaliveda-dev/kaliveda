/***************************************************************************
$Id: KVDBKey.h,v 1.22 2009/03/11 14:19:50 franklan Exp $
                          KVDataBase.h  -  description
                             -------------------
    begin                : jeu fév 6 2003
    copyright            : (C) 2003 by Alexis Mignon
    email                : mignon@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __KVDBKEY_H
#define __KVDBKEY_H

#include "KVBase.h"
#include "KVRList.h"
#include "TRef.h"

class KVDBRecord;
class KVDBTable;

class KVDBKey: public KVBase {

protected:

   Bool_t fIsUnique;            // Can the list contains more than 1 object the same name
   Bool_t fSingle;              // Can the list contain more than 1 object in the list
   TRef fRecord;                //direct pointer to parent record
   KVRList* fLinks;             //->list of cross-referenced records

   virtual void AddLink(KVDBRecord* rec);

   virtual void RemoveLink(KVDBRecord* rec);

public:

   KVDBKey();
   KVDBKey(const Char_t* name, const Char_t* title =
              "", KVDBRecord* parent = 0);
   virtual ~ KVDBKey();

   virtual Bool_t LinkTo(KVDBRecord* rec, Bool_t linkback = kTRUE);
   virtual void Unlink(KVDBRecord* rec, Bool_t linkback = kTRUE);
   virtual void UnlinkAll();

   inline virtual KVDBRecord* GetLink(const Char_t* link) const;

   //return the list of cross-referenced objects
   virtual KVRList* GetLinks() const
   {
      return fLinks;
   };
   inline virtual KVDBRecord* GetParent();
   inline virtual KVDBRecord* GetRecord();
   inline virtual void SetParent(KVDBRecord* parent);
   inline virtual void SetRecord(KVDBRecord* parent);
   inline virtual void SetUniqueStatus(Bool_t unique);
   inline virtual void SetSingleStatus(Bool_t single);

   ClassDef(KVDBKey, 1)        // Key in a Record
};
//___________________________________________________________________________//

KVDBRecord* KVDBKey::GetLink(const Char_t* link) const
{
   return (KVDBRecord*) fLinks->FindObjectByName(link);
}

KVDBRecord* KVDBKey::GetParent()
{
   return (KVDBRecord*) fRecord.GetObject();
}

void KVDBKey::SetParent(KVDBRecord* parent)
{
   fRecord = (TObject*) parent;
}

void KVDBKey::SetRecord(KVDBRecord* parent)
{
   SetParent(parent);
}


KVDBRecord* KVDBKey::GetRecord()
{
   return GetParent();
}

void KVDBKey::SetUniqueStatus(Bool_t unique)
{
   fIsUnique = unique;
}

void KVDBKey::SetSingleStatus(Bool_t single)
{
   fSingle = single;
}


#endif
