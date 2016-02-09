/***************************************************************************
$Id: KVDataBase.h,v 1.20 2009/01/22 13:55:00 franklan Exp $
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
#ifndef KV_DATA_BASE_H
#define KV_DATA_BASE_H
#include "TFolder.h"
#include "TString.h"
#include "KVList.h"
#include "KVDBTable.h"
#include "KVDBRecord.h"

class TFile;

class KVDataBase: public TFolder {


protected:

   TString fDataSet;//the name of the dataset to which this database is associated
   TString fDataSetDir;//the directory containing the dataset files

public:
   KVDataBase();
   KVDataBase(const Char_t* name, const Char_t* title);
   virtual ~ KVDataBase();

   inline virtual KVDBTable* GetTable(const Char_t* table) const;
   inline virtual TList* GetTables() const;
   virtual Bool_t AddTable(KVDBTable* table);
   virtual KVDBTable* AddTable(const Char_t* name, const Char_t* title,
                               Bool_t unique = kFALSE);
   virtual KVDBRecord* GetRecord(const Char_t* table_name,
                                 const Char_t* rec_name) const;
   virtual void Build();
   virtual void Print(Option_t* option = "") const;
   inline virtual void cd();

   static KVDataBase* MakeDataBase(const Char_t* name, const Char_t* datasetdir);

   virtual void Save(const Char_t*)
   {
      ;
   };

   virtual void WriteObjects(TFile*);
   virtual void ReadObjects(TFile*);

   const Char_t* GetDataSetDir() const
   {
      return fDataSetDir;
   }
   void SetDataSetDir(const Char_t* d)
   {
      fDataSetDir = d;
   }

   ClassDef(KVDataBase, 2)     // Base Class for a database of parameters
};

//........ global variable
R__EXTERN KVDataBase* gDataBase;

KVDBTable* KVDataBase::GetTable(const Char_t* table) const
{
   return (KVDBTable*) FindObject(table);
}

TList* KVDataBase::GetTables() const
{
   return (TList*) GetListOfFolders();
}

void KVDataBase::cd()
{
   gDataBase = this;
}

#endif
