/***************************************************************************
                          KVIDBlocking.h  -  description
                             -------------------
    begin                : Thu Oct 13 2005
    copyright            : (C) 2005 by J.D. Frankland
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

#ifndef KVIDBlock_H
#define KVIDBlock_H

#include "KVIDChIoSi.h"

class KVIDGrid;

class KVIDBlocking: public KVIDChIoSi {

public:

   KVIDBlocking()
   {
   };
   virtual ~ KVIDBlocking()
   {
   };

   Bool_t SetIDGrid(KVIDGrid*);

   Double_t GetIDMapX(Option_t* opt = "");
   Double_t GetIDMapY(Option_t* opt = "");

   ClassDef(KVIDBlocking, 1)    //Identification in FNL blocking telescopes BIC-SiB
};

#endif
