/***************************************************************************
$Id: KVDBTape.h,v 1.13 2006/10/19 14:32:43 franklan Exp $
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KV_DB_TAPE_H
#define KV_DB_TAPE_H

#include "KVDBRecord.h"
#include "KVDBKey.h"

class KVDBRun;

/**
\class KVDBTape
\brief Database entry describing a data storage tape used to store raw data.
\ingroup INDRADB
*/

class KVDBTape: public KVDBRecord {

public:
   KVDBTape();
   KVDBTape(Int_t tape_number);
   virtual ~ KVDBTape();

   virtual KVRList* GetRuns()
   {
      if (GetKey("Runs")) {
         return GetKey("Runs")->GetLinks();
      }
      return 0;
   }
   void AddRun(KVDBRun* run);

   ClassDef(KVDBTape, 3)       // Class describing a DLT tape
};

#endif
