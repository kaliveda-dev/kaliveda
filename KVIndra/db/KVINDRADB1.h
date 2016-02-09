/***************************************************************************
$Id: KVINDRADB1.h,v 1.9 2006/10/19 14:32:43 franklan Exp $
                          kvdatabase1.h  -  description
                             -------------------
    begin                : 13/2/2004
    copyright            : (C) 2004 by Daniel Cussol
    email                : cussol@in2p3.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KV_DATA_BASE_1_H
#define KV_DATA_BASE_1_H

#include "KVINDRADB.h"
#include "KVDBRun.h"
#include "KVDBSystem.h"
#include "KVDBTape.h"

class KVINDRADB1: public KVINDRADB {

public:

   KVINDRADB1()
   {
   };
   KVINDRADB1(const Char_t* name);
   virtual ~ KVINDRADB1()
   {
   };

   virtual void Build();
   virtual void GoodRunLine();

//.............inline functions

   ClassDef(KVINDRADB1, 3)      // DataBase of parameters for 1st campaign of Indra
};


#endif
