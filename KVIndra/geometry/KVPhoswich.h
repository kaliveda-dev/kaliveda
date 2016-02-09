/***************************************************************************
$Id: KVPhoswich.h,v 1.9 2006/10/19 14:32:43 franklan Exp $
                          kvphoswich.h  -  description
                             -------------------
    begin                : Fri Oct 4 2002
    copyright            : (C) 2002 by A. Mignon & J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVPHOSWICH_H
#define KVPHOSWICH_H

#include "KVINDRADetector.h"

class KVPhoswich: public KVINDRADetector {
public:
   KVPhoswich();
   KVPhoswich(Float_t);
   virtual ~ KVPhoswich();

   const Char_t* GetArrayName();
   //void Print(Option_t * opt="") const;
   Double_t GetEnergy();
   void SetEnergy(Double_t e);
   void SetACQParams();

   ClassDef(KVPhoswich, 2)     //Phoswich telescopes of the INDRA array
};

#endif
