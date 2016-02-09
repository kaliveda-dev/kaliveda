/***************************************************************************
$Id: KVDBChIoPressures.h,v 1.4 2007/02/14 14:11:15 franklan Exp $
                          KVDBChIoPressures5.h  -  description
                             -------------------
    begin                : mer mai 7 2003
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
#ifndef KVDB_CHIO_PRESSURES
#define KVDB_CHIO_PRESSURES

#define CHIO_2_3 0
#define CHIO_4_5 1
#define CHIO_6_7 2
#define CHIO_8_12 3
#define CHIO_13_17 4

#include "Riostream.h"
#include "KVDataBase.h"


class KVDBChIoPressures: public KVDBRecord {
private:
   static UInt_t fSetNumber;
protected:
   Float_t fPressures[5];      //[5]
   void InitCommon();

public:
   KVDBChIoPressures();
   KVDBChIoPressures(Float_t p1, Float_t p2, Float_t p3, Float_t p4,
                     Float_t p5);
   KVDBChIoPressures(Float_t* pressures);
   KVDBChIoPressures(KVDBChIoPressures& chiopres);
   ~KVDBChIoPressures();

   Float_t GetChIoPressures(UInt_t chio_number);
   Float_t* GetChIoPressures();
   virtual Float_t GetPressure(UInt_t chio_number);
   virtual Float_t* GetPressures();
   void SetPressures(Float_t p1 = 0, Float_t p2 = 0, Float_t p3 = 0,
                     Float_t p4 = 0, Float_t p5 = 0);
   void SetPressures(Float_t* pressures);
   virtual void SetPressure(UInt_t n, Float_t pressure);

   virtual void Print(Option_t* option = "") const;

   ClassDef(KVDBChIoPressures, 1)      //Database record with INDRA ChIo pressures for a set of runs
};

inline Float_t KVDBChIoPressures::GetChIoPressures(UInt_t chio_number)
{
   return fPressures[chio_number];
}

inline Float_t* KVDBChIoPressures::GetChIoPressures()
{
   return fPressures;
}

inline Float_t KVDBChIoPressures::GetPressure(UInt_t chio_number)
{
   return fPressures[chio_number];
}

inline Float_t* KVDBChIoPressures::GetPressures()
{
   return fPressures;
}

inline void KVDBChIoPressures::SetPressures(Float_t p1, Float_t p2,
      Float_t p3, Float_t p4,
      Float_t p5)
{
   fPressures[CHIO_2_3] = p1;
   fPressures[CHIO_4_5] = p2;
   fPressures[CHIO_6_7] = p3;
   fPressures[CHIO_8_12] = p4;
   fPressures[CHIO_13_17] = p5;
}

inline void KVDBChIoPressures::SetPressures(Float_t* pressures)
{
   for (UInt_t i = CHIO_2_3; i <= CHIO_13_17; i++) {
      fPressures[i] = pressures[i];
   }
}

inline void KVDBChIoPressures::SetPressure(UInt_t n, Float_t pressure)
{
   fPressures[n] = pressure;
}

#endif
