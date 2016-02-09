/***************************************************************************
$Id: KVDataBase.cpp,v 1.26 2009/01/22 13:55:00 franklan Exp $
                          KVDataBase.cpp  -  description
                             -------------------
    begin                : jeu f�v 6 2003
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

KVDataBase* gDataBase;

using namespace std;

ClassImp(KVDataBase)
//_______________________________________________________________________
//KVDataBase, along with KVDBKey, KVDBRecord and KVDBTable, are base classes for
//the implementation of simple cross-referenced database structures. The database is made up of tables with
//unique names, each table contains a list of records. Records in different tables may be
//linked together using keys, providing cross-referenced entries.
//
//All these objects are ROOT TFolder objects. The KVDataBase object is added
//to the ROOT folder and may be browsed in the TBrowser.
//
//               An example of use is given here:
//
//{
//gSystem->Load("libKVMultiDet.so");
//
////create new database
//KVDataBase my_db("my_db","Beatles' discography");
//
////create tables in database
//my_db.AddTable("Albums","All the albums made by The Beatles",kTRUE);
//my_db.AddTable("Songs","All the songs written or recorded by The Beatles",kTRUE);
//my_db.AddTable("Years","For chronological information",kTRUE);
//
////fill tables with data
//my_db.GetTable("Years")->AddRecord(new KVDBRecord("1966","The Year 1966"));
//my_db.GetTable("Years")->AddRecord(new KVDBRecord("1967","The Year 1967"));
//my_db.GetTable("Years")->AddRecord(new KVDBRecord("1968","The Year 1968"));
//
//my_db.GetTable("Songs")->AddRecord(new KVDBRecord("Taxman","1. Taxman"));
//my_db.GetTable("Songs")->AddRecord(new KVDBRecord("Eleanor","2. Eleanor Rigby"));
//my_db.GetTable("Songs")->AddRecord(new KVDBRecord("LSD","3. Lucy in the Sky with Diamonds"));
//my_db.GetTable("Songs")->AddRecord(new KVDBRecord("Better","4. Getting Better"));
//my_db.GetTable("Songs")->AddRecord(new KVDBRecord("USSR","1. Back In The USSR"));
//my_db.GetTable("Songs")->AddRecord(new KVDBRecord("Prudence","2. Dear Prudence"));
//
//my_db.GetTable("Albums")->AddRecord(new KVDBRecord("Revolver","Revolver"));
//my_db.GetTable("Albums")->AddRecord(new KVDBRecord("Pepper","Sgt. Pepper's Lonely Hearts' Club Band"));
//my_db.GetTable("Albums")->AddRecord(new KVDBRecord("White","The Beatles"));
//
////cross-referencing
////add a list of songs to each album
//my_db.GetTable("Albums")->GetRecord("Revolver")->AddKey("Songs","Songs on the album Revolver");
//my_db.GetTable("Albums")->GetRecord("Pepper")->AddKey("Songs","Songs on the album Sgt. Pepper");
//my_db.GetTable("Albums")->GetRecord("White")->AddKey("Songs","Songs on the album The Beatles");
////add songs to list for each album
////notice that at the same time, a link is added to each song indicating which album they are from
//my_db.GetTable("Albums")->GetRecord("Revolver")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Taxman"));
//my_db.GetTable("Albums")->GetRecord("Revolver")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Eleanor"));
//my_db.GetTable("Albums")->GetRecord("Pepper")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("LSD"));
//my_db.GetTable("Albums")->GetRecord("Pepper")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Better"));
//my_db.GetTable("Albums")->GetRecord("White")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("USSR"));
//my_db.GetTable("Albums")->GetRecord("White")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Prudence"));
////make a list of songs for each year
//my_db.GetTable("Years")->GetRecord("1966")->AddKey("Songs","Songs from 1966");
//my_db.GetTable("Years")->GetRecord("1967")->AddKey("Songs","Songs from 1967");
//my_db.GetTable("Years")->GetRecord("1968")->AddKey("Songs","Songs from 1968");
////fill lists for each year - this will also add a link to each song indicating its year
//my_db.GetTable("Years")->GetRecord("1966")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Taxman"));
//my_db.GetTable("Years")->GetRecord("1966")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Eleanor"));
//my_db.GetTable("Years")->GetRecord("1967")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("LSD"));
//my_db.GetTable("Years")->GetRecord("1967")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Better"));
//my_db.GetTable("Years")->GetRecord("1968")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("USSR"));
//my_db.GetTable("Years")->GetRecord("1968")->AddLink("Songs",my_db.GetTable("Songs")->GetRecord("Prudence"));
//}
//
//After setting up the database in this way, some typical output would be:
//
//root [12] my_db.Print()
//_______________________________________________________
//my_db Beatles' discography
//Available Tables :
//  Albums
//  Songs
//  Years
//_______________________________________________________
//
//root [14] my_db.GetTable("Songs")->ls()
//OBJ: KVDBRecord Taxman  1. Taxman : 0 at: 0x89b5910
//OBJ: KVDBRecord Eleanor 2. Eleanor Rigby : 0 at: 0x89acb50
//OBJ: KVDBRecord LSD     3. Lucy in the Sky with Diamonds : 0 at: 0x89b59f0
//OBJ: KVDBRecord Better  4. Getting Better : 0 at: 0x89b5c08
//OBJ: KVDBRecord USSR    1. Back In The USSR : 0 at: 0x89b5c40
//OBJ: KVDBRecord Prudence        2. Dear Prudence : 0 at: 0x89b5cd8
//
//root [15] my_db.GetTable("Albums")->GetRecord("White")->Print()
//_______________________________________________________
//White The Beatles
//Available Keys :
//  Songs
//_______________________________________________________
//
//root [16] my_db.GetTable("Songs")->GetRecord("LSD")->Print()
//_______________________________________________________
//LSD 3. Lucy in the Sky with Diamonds
//Available Keys :
//  Albums
//  Years
//_______________________________________________________
//
//root [18] my_db.GetTable("Songs")->GetRecord("LSD")->GetKey("Albums")->ls()
//OBJ: KVDBRecord Pepper  Sgt. Pepper's Lonely Hearts' Club Band : 0 at: 0x89b5b00
//
//root [19] my_db.GetTable("Songs")->GetRecord("LSD")->GetKey("Years")->ls()
//OBJ: KVDBRecord 1967    The Year 1967 : 0 at: 0x89b57a0
//
//_______________________________________________________________________
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
   fDataSet = name;
}

//_______________________________________________________________________
KVDataBase::~KVDataBase()
{
   //reset global database pointer if it was pointing to this DB
   if (gDataBase == this)
      gDataBase = 0;
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
   tab->SetFullPath(Form("//root/%s/%s", fDataSet.Data(), tab->GetName()));
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
   table->SetFullPath(Form("//root/%s/%s", fDataSet.Data(), name));
   return table;
}

//__________________________________________________________________________
void KVDataBase::Build()
{
//  Methode that builds the DataBase from the parameter files
//  It does nothing here. Must be derived for each Database.
//
//
   AbstractMethod("Build");

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


//_________________________________________________________________________________

KVDataBase* KVDataBase::MakeDataBase(const Char_t* name, const Char_t* datasetdir)
{
   //Static function which will create and 'Build' the database object corresponding to 'name'
   //These are defined as 'Plugin' objects in the file $KVROOT/KVFiles/.kvrootrc :
   //
   //      Plugin.KVDataBase:    INDRA_camp1    KVINDRADB1     KVIndra    "KVINDRADB1()"
   //      +Plugin.KVDataBase:    INDRA_camp2    KVINDRADB2     KVIndra    "KVINDRADB2()"
   //      +Plugin.KVDataBase:    INDRA_camp4    KVINDRADB4     KVIndra    "KVINDRADB4()"
   //      +Plugin.KVDataBase:    INDRA_camp5    KVINDRADB5     KVIndra5    "KVINDRADB5()"
   //
   //The 'name' ("INDRA_camp1" etc.) corresponds to the name of a dataset in $KVROOT/KVFiles/manip.list
   //This name is stored in member variable fDataSet.
   //The constructors/macros used have arguments (const Char_t* name)

   //does plugin exist for given name ?
   TPluginHandler* ph;
   if (!(ph = KVBase::LoadPlugin("KVDataBase", name))) {
      return 0;
   }
   //execute constructor/macro for database
   KVDataBase* mda = (KVDataBase*) ph->ExecPlugin(1, name);
   mda->SetDataSetDir(datasetdir);
   //call Build() method
   mda->Build();
   return mda;
}

//______________________________________________________________________________

void KVDataBase::WriteObjects(TFile*)
{
   // Abstract method. Can be overridden in child classes.
   // When the database is written to disk (by the currently active dataset, see
   // KVDataSet::WriteDBFile) any associated objects (histograms, trees, etc.)
   // can be written using this method.
   // The pointer to the file being written is passed as argument.

   AbstractMethod("WriteObjects");
}

//______________________________________________________________________________

void KVDataBase::ReadObjects(TFile*)
{
   // Abstract method. Can be overridden in child classes.
   // When the database is read from disk (by the currently active dataset, see
   // KVDataSet::OpenDBFile) any associated objects (histograms, trees, etc.)
   // stored in the file can be read using this method.
   // The pointer to the file being read is passed as argument.

   AbstractMethod("ReadObjects");
}

