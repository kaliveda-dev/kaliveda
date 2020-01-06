/***************************************************************************
                          KVTelBlocking.h  -  description
                             -------------------
    begin                : Mon Nov 3 2003
    copyright            : (C) 2003 by John Frankland
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

#ifndef KVTELBLOCKING_H
#define KVTELBLOCKING_H

#include "KVTelescope.h"

class KVTelBlocking: public KVTelescope {

public:

   KVTelBlocking(UInt_t n = 0);
   virtual ~ KVTelBlocking();

   ClassDef(KVTelBlocking, 1)  //Blocking telescope for E416 experiment
};

#endif
