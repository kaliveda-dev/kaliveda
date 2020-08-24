/***************************************************************************
$Id: KVDBKey.cpp,v 1.23 2007/04/19 12:41:54 franklan Exp $
                          KVDBKey.cpp  -  description
                             -------------------
    begin                : jeu f� 6 2003
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
#include "KVDBKey.h"
#include "KVDBRecord.h"
#include "Riostream.h"
#include "KVDBTable.h"
#include "TObjString.h"
#include "TROOT.h"

ClassImp(KVDBKey)


//____________________________________________________________________________
KVDBKey::KVDBKey()
{
   fIsUnique = fSingle = kFALSE;
   fLinks = new KVRList;
   fRecord = 0;
}

//_____________________________________________________________________________
KVDBKey::KVDBKey(const Char_t* name, const Char_t* title,
                 KVDBRecord* parent)
   : KVBase(name, title)
{
   fIsUnique = fSingle = kFALSE;
   fRecord = 0;
   SetParent(parent);
   fLinks = new KVRList;
   TString knom(name);
   knom.Prepend("Key:");
   SetName(knom.Data());
   SetTitle(title);
}

//_____________________________________________________________________________
KVDBKey::~KVDBKey()
{
   if (fLinks) {
      fLinks->Clear();
      delete fLinks;
      fLinks = 0;
   }
   fRecord = 0;
}

//_____________________________________________________________________________
Bool_t KVDBKey::LinkTo(KVDBRecord* rec, Bool_t linkback)
{
// This function adds a record to the list of cross-references
// if fIsUnique is kTRUE we check if there is already an object with the same name:
//    if so we return kFALSE otherwise the record is added and kTRUE is returned.
// if fSingle is kTRUE the list can handle only one object:
//    if the list is empty the record is added and kTRUE is returned, otherwise kFALSE.

   // check if more than one object is allowed
   if (fSingle && (GetLinks()->GetSize() > 0) && (GetLinks()->First())) {
      Warning("LinkTo(KVDBRecord*)",
              "%s : Only one cross-reference allowed.\nCurrent cross-reference is:%s\nCan't add new record \"%s\" in list",
              GetName(), GetLinks()->First()->GetName(), rec->GetName());
      return kFALSE;
   }
   // check if identical names are allowed
   if (fIsUnique) {
      if (fLinks->FindObject(rec->GetName())) {
         Warning("LinkTo(KVDBRecord*)",
                 "%s : Cross-references must have different names.\nA record named %s already exists.\nCan't add new record \"%s\" in list",
                 GetName(), rec->GetName(), rec->GetName());
         return kFALSE;
      }
   }
   AddLink(rec);

   if (linkback) {
      //if linkback is kTRUE then a mirror link to this record will be made in record "rec"
      //if a key with the same name as the table containing the current record is found
      //in the record to link , then this key is linked back to the current record.
      //if no key is found, one will be created, and then the linkback performed.

      //check parent is set
      if (!GetParent()) {
         Error("LinkTo", "Parent table not set for key %s", GetName());
         return kFALSE;
      }
      const Char_t* table_name = GetParent()->GetTable()->GetName();

      KVDBKey* key = rec->GetKey(table_name);
      if (!key)
         key = rec->AddKey(table_name, GetParent()->GetName());
      key->LinkTo(GetParent(), kFALSE); //linkback=kFALSE otherwise infinite circular linkage results !!!
   }
   return kTRUE;
}

//_____________________________________________________________________________

void KVDBKey::Unlink(KVDBRecord* rec, Bool_t linkback)
{
   //This function removes a record from the list of cross-references
   //if linkback is kTRUE then the reference in record "rec" is also removed.

   RemoveLink(rec);

   if (linkback) {

      //check parent is set
      if (!GetParent()) {
         Error("Unlink", "Parent not set for key %s", GetName());
         return;
      }
      const Char_t* table_name = GetParent()->GetTable()->GetName();

      KVDBKey* key = rec->GetKey(table_name);
      if (key) key->Unlink(GetParent(), kFALSE); //linkback=kFALSE otherwise infinite circular linkage results !!!
   }
}


//_____________________________________________________________________________

void KVDBKey::UnlinkAll()
{
   //This function recursively removes all cross-references
   //The corresponding references to the owner of this key in all of the referecnced
   //objects will also be removed.

   KVDBRecord* rec = (KVDBRecord*)fLinks->Last();
   if (rec) {
      Unlink(rec);
      UnlinkAll();
   }
   fLinks->Clear();
}

//_____________________________________________________________________________

void KVDBKey::AddLink(KVDBRecord* rec)
{
   //Add a link to the list of links

   fLinks->Add(rec);
}

void KVDBKey::RemoveLink(KVDBRecord* rec)
{
   //Remove a link from the list of links

   fLinks->Remove(rec);
}

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
