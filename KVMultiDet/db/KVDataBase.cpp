/***************************************************************************
$Id: KVDataBase.cpp,v 1.26 2009/01/22 13:55:00 franklan Exp $
                          KVDataBase.cpp  -  description
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

#include "KVDataBase.h"
#include "TROOT.h"
#include "Riostream.h"
#include "TPluginManager.h"
#include "KVNumberList.h"

using namespace std;

ClassImp(KVDataBase)
KVDataBase::KVDataBase()
{
   gROOT->GetRootFolder()->Add(this);
   SetOwner(kTRUE);
}

//_______________________________________________________________________
KVDataBase::KVDataBase(const Char_t* name, const Char_t* title)
   : TFolder(name, title)
{
   gROOT->GetRootFolder()->Add(this);
   SetOwner(kTRUE);
   fFolderName = name;
}

//_______________________________________________________________________
KVDataBase::KVDataBase(const Char_t* name)
   : TFolder(name, "database")
{
   gROOT->GetRootFolder()->Add(this);
   SetOwner(kTRUE);
   fFolderName = name;
}

//_______________________________________________________________________
KVDataBase::~KVDataBase()
{
   //reset global database pointer if it was pointing to this DB
   gROOT->GetRootFolder()->Remove(this);
   gROOT->GetListOfCleanups()->Remove(this);
   Info("~KVDataBase", "%s", GetName());
}

//_______________________________________________________________________
Bool_t KVDataBase::AddTable(KVDBTable* tab)
{
// Add a table to the list of available tables and return kTRUE
// it's added. As each table must have a unique name we check if the new table's name
// already exists, if not the table is added otherwise it's not added
// and the method return kFALSE.
//

   //table with this name already in DB ?
   if (GetTable(tab->GetName()))
      return kFALSE;

   Add(tab);
   tab->SetFullPath(Form("//root/%s/%s", fFolderName.Data(), tab->GetName()));
   return kTRUE;
}

//_______________________________________________________________________
KVDBTable* KVDataBase::AddTable(const Char_t* name, const Char_t* title,
                                Bool_t unique)
{
// Add a table to the list of tables and check if the new table's name already exists.
// If it exists the table is not added and null pointer is returned, otherwise a new
// table is created and added to the list and its address is returned.

   //Table with this name already in DB ?
   if (GetTable(name))
      return 0;

   KVDBTable* table = new KVDBTable(name, title, unique);
   Add(table);
   table->SetFullPath(Form("//root/%s/%s", fFolderName.Data(), name));
   return table;
}

//__________________________________________________________________________
KVDBRecord* KVDataBase::GetRecord(const Char_t* table_name,
                                  const Char_t* rec_name) const
{
//
// Return the KVDBRecord rec_name found in the table table_name
//
   KVDBTable* table = GetTable(table_name);
   if (!table) {
      TObject::Warning("GetRecord(const Char_t*,const Char_t*)",
                       "No Table named %s found.", table_name);
      return NULL;
   }
   return table->GetRecord(rec_name);
}

//__________________________________________________________________________
void KVDataBase::Print(Option_t*) const
{


   cout << "_______________________________________________________" <<
        endl;
   cout << ClassName() << " : " << GetName() << " <---> " << GetTitle() <<
        endl;
   cout << "Available Tables :" << endl;
   TIter next(GetTables());
   KVDBTable* table;
   while ((table = (KVDBTable*) next())) {
      cout << "    " << table->GetName() << endl;
   }
   cout << "_______________________________________________________" <<
        endl;
}


