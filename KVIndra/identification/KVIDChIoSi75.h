/***************************************************************************
                          KVIDChIoSi75.h  -  description
                             -------------------
    begin                : Fri Feb 20 2004
    copyright            : (C) 2004 by J.D. Frankland
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

#ifndef KVIDChIoSi75_H
#define KVIDChIoSi75_H

#include "KVIDChIoSi.h"


class KVIDChIoSi75: public KVIDChIoSi {

public:

   KVIDChIoSi75();
   virtual ~ KVIDChIoSi75();

   ClassDef(KVIDChIoSi75, 1)    //INDRA identification using ChIo-Si75 matrices
};

#endif
