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
class KVNumberList;

/**
  \class KVDataBase
  \brief Simple cross-referenced database structure
  \ingroup Core

KVDataBase, along with KVDBKey, KVDBRecord and KVDBTable, are base classes for
the implementation of simple cross-referenced database structures. The database is made up of tables with
unique names, each table contains a list of records. Records in different tables may be
linked together using keys, providing cross-referenced entries.

All these objects are ROOT TFolder objects. The KVDataBase object is added
to the ROOT folder and may be browsed in the TBrowser.

               An example of use is given here:
~~~~{.cpp}
//create new database
KVDataBase my_db("my_db","Beatles' discography");

//create tables in database
my_db.AddTable("Albums","All the albums made by The Beatles",kTRUE);
my_db.AddTable("Songs","All the songs written or recorded by The Beatles",kTRUE);
my_db.AddTable("Years","For chronological information",kTRUE);

//fill tables with data
my_db.GetTable("Years")->AddRecord(new KVDBRecord("1966","The Year 1966"));
my_db.GetTable("Years")->AddRecord(new KVDBRecord("1967","The Year 1967"));
my_db.GetTable("Years")->AddRecord(new KVDBRecord("1968","The Year 1968"));

my_db.GetTable("Songs")->AddRecord(new KVDBRecord("Taxman","1. Taxman"));
my_db.GetTable("Songs")->AddRecord(new KVDBRecord("Eleanor","2. Eleanor Rigby"));
my_db.GetTable("Songs")->AddRecord(new KVDBRecord("LSD","3. Lucy in the Sky with Diamonds"));
my_db.GetTable("Songs")->AddRecord(new KVDBRecord("Better","4. Getting Better"));
my_db.GetTable("Songs")->AddRecord(new KVDBRecord("USSR","1. Back In The USSR"));
my_db.GetTable("Songs")->AddRecord(new KVDBRecord("Prudence","2. Dear Prudence"));

my_db.GetTable("Albums")->AddRecord(new KVDBRecord("Revolver","Revolver"));
my_db.GetTable("Albums")->AddRecord(new KVDBRecord("Pepper","Sgt. Pepper's Lonely Hearts' Club Band"));
my_db.GetTable("Albums")->AddRecord(new KVDBRecord("White","The Beatles"));

//cross-referencing
//add a list of songs to each album
my_db.GetTable("Albums")->GetRecord("Revolver")->AddKey("Songs","Songs on the album Revolver");
my_db.GetTable("Albums")->GetRecord("Pepper")->AddKey("Songs","Songs on the album Sgt. Pepper");
my_db.GetTable("Albums")->GetRecord("White")->AddKey("Songs","Songs on the album The Beatles");
//add songs to list for each album
//notice that at the same time, a link is added to each song indicating which album they are from
my_db.GetTable("Albums")->GetRecord("Revolver")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Taxman"));
my_db.GetTable("Albums")->GetRecord("Revolver")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Eleanor"));
my_db.GetTable("Albums")->GetRecord("Pepper")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("LSD"));
my_db.GetTable("Albums")->GetRecord("Pepper")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Better"));
my_db.GetTable("Albums")->GetRecord("White")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("USSR"));
my_db.GetTable("Albums")->GetRecord("White")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Prudence"));
//make a list of songs for each year
my_db.GetTable("Years")->GetRecord("1966")->AddKey("Songs","Songs from 1966");
my_db.GetTable("Years")->GetRecord("1967")->AddKey("Songs","Songs from 1967");
my_db.GetTable("Years")->GetRecord("1968")->AddKey("Songs","Songs from 1968");
//fill lists for each year - this will also add a link to each song indicating its year
my_db.GetTable("Years")->GetRecord("1966")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Taxman"));
my_db.GetTable("Years")->GetRecord("1966")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Eleanor"));
my_db.GetTable("Years")->GetRecord("1967")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("LSD"));
my_db.GetTable("Years")->GetRecord("1967")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Better"));
my_db.GetTable("Years")->GetRecord("1968")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("USSR"));
my_db.GetTable("Years")->GetRecord("1968")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Prudence"));
}
~~~~
After setting up the database in this way, some typical output would be:
~~~~{.cpp}
root [12] my_db.Print()
_______________________________________________________
my_db Beatles' discography
Available Tables :
  Albums
  Songs
  Years
_______________________________________________________

root [14] my_db.GetTable("Songs")->ls()
OBJ: KVDBRecord Taxman  1. Taxman : 0 at: 0x89b5910
OBJ: KVDBRecord Eleanor 2. Eleanor Rigby : 0 at: 0x89acb50
OBJ: KVDBRecord LSD     3. Lucy in the Sky with Diamonds : 0 at: 0x89b59f0
OBJ: KVDBRecord Better  4. Getting Better : 0 at: 0x89b5c08
OBJ: KVDBRecord USSR    1. Back In The USSR : 0 at: 0x89b5c40
OBJ: KVDBRecord Prudence        2. Dear Prudence : 0 at: 0x89b5cd8

root [15] my_db.GetTable("Albums")->GetRecord("White")->Print()
_______________________________________________________
White The Beatles
Available Keys :
  Songs
_______________________________________________________

root [16] my_db.GetTable("Songs")->GetRecord("LSD")->Print()
_______________________________________________________
LSD 3. Lucy in the Sky with Diamonds
Available Keys :
  Albums
  Years
_______________________________________________________

root [18] my_db.GetTable("Songs")->GetRecord("LSD")->GetKey("Albums")->ls()
OBJ: KVDBRecord Pepper  Sgt. Pepper's Lonely Hearts' Club Band : 0 at: 0x89b5b00

root [19] my_db.GetTable("Songs")->GetRecord("LSD")->GetKey("Years")->ls()
OBJ: KVDBRecord 1967    The Year 1967 : 0 at: 0x89b57a0
~~~~
*/

class KVDataBase: public TFolder {

   TString fFolderName;

public:
   KVDataBase();
   KVDataBase(const Char_t* name);
   KVDataBase(const Char_t* name, const Char_t* title);
   virtual ~ KVDataBase();

   inline virtual KVDBTable* GetTable(const Char_t* table) const;
   inline virtual TList* GetTables() const;
   virtual Bool_t AddTable(KVDBTable* table);
   virtual KVDBTable* AddTable(const Char_t* name, const Char_t* title,
                               Bool_t unique = kFALSE);
   virtual KVDBRecord* GetRecord(const Char_t* table_name,
                                 const Char_t* rec_name) const;
   virtual void Print(Option_t* option = "") const;

   ClassDef(KVDataBase, 3)     // Base Class for a database of parameters
};

KVDBTable* KVDataBase::GetTable(const Char_t* table) const
{
   return (KVDBTable*) FindObject(table);
}

TList* KVDataBase::GetTables() const
{
   return (TList*) GetListOfFolders();
}

#endif
