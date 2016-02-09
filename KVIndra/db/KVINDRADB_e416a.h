/*
$Id: KVINDRADB_e416a.h,v 1.1 2007/02/13 18:18:18 franklan Exp $
$Revision: 1.1 $
$Date: 2007/02/13 18:18:18 $
*/

//Created by KVClassFactory on Tue Feb 13 19:00:55 2007
//Author: franklan

#ifndef __KVINDRADB_E416A_H
#define __KVINDRADB_E416A_H

#include <KVINDRADB.h>

class KVINDRADB_e416a : public KVINDRADB {
   KVDBTable* fBICPressures;   //-> table of BIC pressures
public:

   KVINDRADB_e416a()
   {
      fBICPressures = AddTable("BIC Pressures", "Pressures of BIC");
   };
   KVINDRADB_e416a(const Char_t* name);
   virtual ~KVINDRADB_e416a();
   void Build();
   virtual void ReadBICPressures();

   ClassDef(KVINDRADB_e416a, 1) //Database for E416a experiment
};

#endif
