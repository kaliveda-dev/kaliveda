/***************************************************************************
$Id: KVDBRecord.cpp,v 1.23 2007/05/31 09:59:22 franklan Exp $
                          KVDBRecord.cpp  -  description
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
#include "Riostream.h"
#include "KVDBRecord.h"
#include "KVDBTable.h"
#include "KVDBKey.h"
#include "TROOT.h"

using namespace std;

ClassImp(KVDBRecord);

//__________________________________________________________________________
KVDBRecord::KVDBRecord()
{
   SetOwner(kTRUE);
}

//__________________________________________________________________________
KVDBRecord::KVDBRecord(const Char_t* name,
                       const Char_t* title): TFolder(name, title)
{
   SetOwner(kTRUE);
}

//__________________________________________________________________________
KVDBRecord::~KVDBRecord()
{
   gROOT->GetListOfCleanups()->Remove(this);
}

//___________________________________________________________________________
Bool_t KVDBRecord::AddKey(KVDBKey* key, Bool_t check)
{
// Add a key to the list of available keys and return kTRUE
// if it is added. If "check" is kTRUE , we check if the new key's name
// already exists, if not the key is added otherwise it's not added
// and the method return kFALSE.
//


   if (check && GetKey(key->GetName())) {
      TObject::Warning("AddKey(KVDBKey*,Bool_t)",
                       "A key named %s already exists.", key->GetName());
      return kFALSE;
   }

   Add(key);
   key->SetParent(this);

   return kTRUE;
}

//___________________________________________________________________________
KVDBKey* KVDBRecord::AddKey(const Char_t* name, const Char_t* title,
                            Bool_t check)
{
// Add a key to the list of available keys and return kTRUE
// it's added. If "check" is kTRUE , we check if the new key's name
// already exists, if not the key is added otherwise it's not added
// and the method return kFALSE.
//

   if (check && GetKey(name)) {
      TObject::Warning("AddKey(const Char_t*, const Char_t*,Bool_t)",
                       "A key named %s already exists.", name);
      return NULL;
   }

   KVDBKey* key = new KVDBKey(name, title, this);
   Add(key);

   return key;
}

//___________________________________________________________________________
Bool_t KVDBRecord::AddLink(const Char_t* key_name, KVDBRecord* rec,
                           Bool_t linkback)
{
   //Link this record to the record "rec" in the DB table with name "key_name"
   //The record will be added to the list of records in KVDBKey "key_name"

   KVDBKey* key = GetKey(key_name);
   if (key)
      return key->LinkTo(rec, linkback);
   else {
      TObject::Warning("AddLink(const Char_t*,KVDBRecord*)",
                       "No key named %s found.", key_name);
      return kFALSE;
   }

}

//___________________________________________________________________________

void KVDBRecord::RemoveLink(const Char_t* key_name, KVDBRecord* rec,
                            Bool_t linkback)
{
   //Remove the link between this record and the record "rec" in the DB table"key_name"

   KVDBKey* key = GetKey(key_name);
   if (key)
      key->Unlink(rec, linkback);
   else {
      TObject::Warning("RemoveLink(const Char_t*,KVDBRecord*)",
                       "No key named %s found.", key_name);
   }

}

//___________________________________________________________________________

void KVDBRecord::RemoveAllLinks(const Char_t* key_name)
{
   //Remove all links between this record and the records in the DB table"key_name"

   KVDBKey* key = GetKey(key_name);
   if (key)
      key->UnlinkAll();
   else {
      TObject::Warning("RemoveAllLinks(const Char_t*)",
                       "No key named %s found.", key_name);
   }

}

//___________________________________________________________________________

KVDBRecord* KVDBRecord::GetLink(const Char_t* key, const Char_t* link) const
{
   //Returns the record named "link" in the table named "key"

   KVDBKey* key_ = GetKey(key);
   if (key_)
      return key_->GetLink(link);
   else {
      TObject::Warning("GetLink(const Char_t*,const Char_t*)const",
                       "No key named %s found.", key);
      return NULL;
   }
}

//___________________________________________________________________________

KVRList* KVDBRecord::GetLinks(const Char_t* key) const
{
   //Returns the list of records linked to this record in table "key"

   KVDBKey* key_ = GetKey(key);
   if (key_)
      return key_->GetLinks();

   return NULL;
}

//___________________________________________________________________________

void KVDBRecord::Print(Option_t*) const
{

   cout << "_______________________________________________________" <<
        endl;
   cout << GetName() << " " << GetTitle() << endl;
   cout << "Available Keys :" << endl;
   TIter next(GetKeys());
   KVDBKey* key;
   while ((key = (KVDBKey*) next())) {
      cout << "  " << key->GetName() << endl;
   }
   cout << "_______________________________________________________" <<
        endl;
}

//___________________________________________________________________________

void KVDBRecord::ls(Option_t*) const
{
   TNamed::ls();
}


//___________________________________________________________________________

Int_t KVDBRecord::Compare(const TObject* obj) const
{
   // Compare two record numbers for sorting lists.
   // Lists will be sorted in ascending order.

   KVDBRecord* dbobj =
      dynamic_cast < KVDBRecord* >(const_cast < TObject* >(obj));
   return (dbobj->GetNumber() ==
           GetNumber() ? 0 : (dbobj->GetNumber() > GetNumber() ? -1 : 1));
}

KVDBTable* KVDBRecord::GetTable() const
{
   return (KVDBTable*)gROOT->FindObject(fFullPathTable.Data());
}

void KVDBRecord::SetTable(const KVDBTable* table)
{
   fFullPathTable = table->GetFullPath();
}

//_____________________________________________________________________________//

KVDBKey* KVDBRecord::GetKey(const Char_t* key) const
{
   TString knom(key);
   knom.Prepend("Key:");
   return (KVDBKey*) FindObject(knom.Data());
}

TList* KVDBRecord::GetKeys() const
{
   return (TList*) GetListOfFolders();
}


