/***************************************************************************
$Id: KVINDRADB2.h,v 1.6 2006/10/19 14:32:43 franklan Exp $
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
#ifndef KV_DATA_BASE_2_H
#define KV_DATA_BASE_2_H

#include "KVINDRADB.h"
#include "KVDBRun.h"
#include "KVDBSystem.h"
#include "KVDBTape.h"

/**
  \class KVINDRADB2
  \ingroup INDRADB
  \brief DataBase of parameters for 2nd campaign of INDRA
 */

class KVINDRADB2: public KVINDRADB {

public:
   KVINDRADB2()
   {
   };
   KVINDRADB2(const Char_t* name);
   virtual ~ KVINDRADB2();

   virtual void Build();
   virtual void GoodRunLine();

//.............inline functions

   ClassDef(KVINDRADB2, 3)      // DataBase of parameters for 2nd campaign of Indra
};


#endif
