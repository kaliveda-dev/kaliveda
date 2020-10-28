/***************************************************************************
$Id: KVDBTape.cpp,v 1.7 2006/10/19 14:32:43 franklan Exp $

 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KVDBTape.h"
#include "KVDBRun.h"

ClassImp(KVDBTape);


KVDBTape::KVDBTape()
{
}

//_____________________________________________________________________________
KVDBTape::KVDBTape(Int_t tape_number)
{

   SetNumber(tape_number);
   Char_t name[80];
   sprintf(name, "Tape %d", tape_number);
   SetName(name);
   SetTitle("Data storage tape");
   KVDBKey* key = AddKey("Runs", "List Of Runs");
   key->SetUniqueStatus(kTRUE);
}

//_____________________________________________________________________________
KVDBTape::~KVDBTape()
{
}

void KVDBTape::AddRun(KVDBRun* run)
{
   //add run to list of runs stored on this tape
   AddLink("Runs", run);
}
