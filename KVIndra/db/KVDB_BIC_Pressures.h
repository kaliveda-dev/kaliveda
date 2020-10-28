/*
$Id: KVDB_BIC_Pressures.h,v 1.3 2007/02/14 14:13:05 franklan Exp $
$Revision: 1.3 $
$Date: 2007/02/14 14:13:05 $
*/

//Created by KVClassFactory on Tue Feb 13 18:37:09 2007
//Author: franklan

#ifndef __KVDB_BIC_PRESSURES_H
#define __KVDB_BIC_PRESSURES_H

#include <KVDBChIoPressures.h>

#define BIC_1 0
#define BIC_2 1
#define BIC_3 2

/**
\class KVDB_BIC_Pressures
\brief Database entry for BIC (Blocking Ionisation Chamber) pressures (experiment E416A)
\ingroup INDRADB
*/

class KVDB_BIC_Pressures : public KVDBChIoPressures {
   void init();

public:

   KVDB_BIC_Pressures();
   KVDB_BIC_Pressures(Float_t*);
   virtual ~KVDB_BIC_Pressures();

   void SetPressures(Float_t* pressures)
   {
      for (UInt_t i = BIC_1; i <= BIC_3; i++) {
         fPressures[i] = pressures[i];
      }
   };

   void Print(Option_t* opt = "") const;

   ClassDef(KVDB_BIC_Pressures, 1) //Database entry for BIC pressures (experiment E416A)
};

#endif
