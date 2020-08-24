/***************************************************************************
$Id: KVDBRecord.h,v 1.19 2007/05/31 09:59:22 franklan Exp $
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

#ifndef __KVDBRECORD_H
#define __KVDBRECORD_H

#include "TFolder.h"
#include "TList.h"
#include "TRef.h"
#include "KVRList.h"

class KVDBKey;
class KVDBTable;

/**
  \class KVDBRecord
  \brief Record folder for the database
  \ingroup Core

  it's a base class, which suppose that it must be derived to have
  the proper caracteristics for each type of record
  it holds a list of keys.
  Each key must have the name of a table of the data base
      The folder owns the list of keys.
*/

class KVDBRecord: public TFolder {

protected:

   TString fFullPathTable;  //full path to parent table in folder structure
   Int_t fNumber;               //number which can be used to identify/sort record

public:

   KVDBRecord();
   KVDBRecord(const Char_t* name, const Char_t* title = "");
   virtual ~ KVDBRecord();

   virtual KVDBKey* GetKey(const Char_t* key) const;
   virtual TList* GetKeys() const;
   virtual Bool_t AddLink(const Char_t* key_name, KVDBRecord* rec,
                          Bool_t linkback = kTRUE);
   virtual void RemoveLink(const Char_t* key_name, KVDBRecord* rec,
                           Bool_t linkback = kTRUE);
   virtual Bool_t AddKey(KVDBKey* key, Bool_t check = kTRUE);
   virtual KVDBKey* AddKey(const Char_t* name, const Char_t* title,
                           Bool_t check = kTRUE);
   virtual KVDBRecord* GetLink(const Char_t* key,
                               const Char_t* link) const;
   virtual KVRList* GetLinks(const Char_t* key) const;
   virtual void RemoveAllLinks(const Char_t* key);
   virtual KVDBTable* GetTable() const;
   virtual void SetTable(const KVDBTable* table);
   virtual void Print(Option_t* option = "") const;
   virtual void ls(Option_t* option = "*") const;
   virtual Int_t GetNumber() const
   {
      return fNumber;
   };
   virtual void SetNumber(Int_t n)
   {
      fNumber = n;
   };
   virtual Int_t Compare(const TObject* obj) const;

   ClassDef(KVDBRecord, 3)//Base Class for a record
};

#endif
