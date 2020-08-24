/***************************************************************************
$Id: KVDBTable.cpp,v 1.18 2007/04/27 14:46:29 franklan Exp $
                          KVTable.cpp  -  description
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
#include "KVDBTable.h"
#include "KVDBRecord.h"
#include "Riostream.h"
#include "TROOT.h"

#include <KVUniqueNameList.h>


using namespace std;

ClassImp(KVDBTable)

KVDBTable::KVDBTable()
{
   fIsUnique = kFALSE;
}

//__________________________________________________________________

KVDBTable::KVDBTable(const Char_t* name, const Char_t* type,
                     Bool_t unique)
   : TFolder() // do not call TFolder(name,type) ctor as it creates a default TList for fFolders
{
   SetName(name);
   SetTitle(type);
   fIsUnique = unique;
   if (unique) fFolders = new KVUniqueNameList;
   else fFolders = new KVHashList;
   SetOwner();
}

//__________________________________________________________________

KVDBTable::~KVDBTable()
{
   gROOT->GetListOfCleanups()->Remove(this);
}

//___________________________________________________________________

Bool_t KVDBTable::AddRecord(KVDBRecord* rec)
{
   // Add a KVDBRecord to the list of available records and return kTRUE
   // If a default formatting string for names of numbered records has been
   // set (see SetDefaultFormat(const TString&)) the name of the record
   // will be automatically set here according to the format and its number.

   if (HasDefaultFormat()) rec->SetName(Form(fDefFormatNumRec.Data(), rec->GetNumber()));
   Add(rec);
   rec->SetTable(this);
   return kTRUE;
}

//___________________________________________________________________

void KVDBTable::RemoveRecord(KVDBRecord* rec)
{
   //Remove a KVDBRecord from the list of available records

   Remove(rec);
   rec->SetTable(0);
}

//___________________________________________________________________________________//

void KVDBTable::ls(Option_t*) const
{
   cout << ClassName() << " : " << GetName() << " <---> " << GetTitle() << " ["
        << GetRecords()->GetSize() << " records]" << endl;
}

void KVDBTable::SetDefaultFormat(const TString& fmt)
{
   // Set a default formatting string for names of numbered records.
   // e.g. if fmt="Rec#%d", each record which is added to the table will have
   // its name automatically set to "Rec#1", "Rec#2", etc., according to the
   // record number.
   // This allows to replace a slow sequential scan for a record based on number
   // (GetRecord(Int_t)) with a fast hash-based look-up

   fDefFormatNumRec = fmt.Data();
}

//___________________________________________________________________________________//

KVDBRecord* KVDBTable::GetRecord(Int_t num) const
{
   // Sequential search for record using its number.
   // To keep things simple, we only search in the top level of the folder structure.
   //
   // NOTE: if there are many records, this sequential search can be long.
   // If a default format for the record name using its number has been defined
   // (see SetDefaultFormat(const TString&)) the search will be performed using
   // the resulting formatted object name, which is fast (hash list).

   if (HasDefaultFormat()) return GetRecord(Form(fDefFormatNumRec.Data(), num));

   TIter next(GetListOfFolders());
   KVDBRecord* obj = 0;
   while ((obj = (KVDBRecord*) next()) && (obj->GetNumber() != num));
   return (obj ? (obj->GetNumber() == num ? obj : 0) : 0);
}

//___________________________________________________________________________________//

void KVDBTable::Rehash(void)
{
   // The list of records is a KVHashList or KVUniqueNameList
   // Call this method if the names of the records change after having been
   // added to the table, otherwise GetRecord(const char*) will not work.

   ((KVHashList*)fFolders)->Rehash();
}
