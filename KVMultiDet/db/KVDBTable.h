/***************************************************************************
$Id: KVDBTable.h,v 1.18 2007/05/31 09:59:22 franklan Exp $
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
#ifndef __KVDBTABLE_H
#define __KVDBTABLE_H

#include "TFolder.h"

#include <KVSeqCollection.h>

class KVDBRecord;

/**
  \class KVDBTable
  \brief Table in a database
  \ingroup Core
 */

class KVDBTable: public TFolder {

private:
   TObject* FindObject(const Char_t*) const
   {
      return nullptr;   // do not use
   }
   TObject* FindObject(const TObject*) const
   {
      return nullptr;   // do not use
   }

protected:

   Bool_t fIsUnique;            // Must each record name be unique ?
   TString fFullPath;           //full path to table in folder structure
   TString fDefFormatNumRec;    // default formatting for names of numbered records

public:

   KVDBTable();
   KVDBTable(const Char_t* name, const Char_t* title = "", Bool_t unique = kFALSE);
   virtual ~ KVDBTable();

   virtual KVDBRecord* GetRecord(const Char_t* rec_name) const
   {
      return (KVDBRecord*) GetRecords()->FindObject(rec_name);
   }
   virtual KVDBRecord* GetRecord(Int_t n) const;
   virtual KVSeqCollection* GetRecords() const
   {
      return (KVSeqCollection*)GetListOfFolders();
   }
   virtual Bool_t AddRecord(KVDBRecord* add);
   virtual void RemoveRecord(KVDBRecord* add);
   virtual void ls(Option_t* option = "*") const;

   virtual void SetFullPath(const Char_t* path)
   {
      fFullPath = path;
   };
   virtual const Char_t* GetFullPath() const
   {
      return fFullPath.Data();
   };

   void SetDefaultFormat(const TString&);
   Bool_t HasDefaultFormat() const
   {
      return fDefFormatNumRec != "";
   }
   void Rehash(void);

   ClassDef(KVDBTable, 3)      //Table object for database
};

#endif
