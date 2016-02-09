/***************************************************************************
                          KVIDPhoswich.h  -  description
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

#ifndef KVIDPhoswich_H
#define KVIDPhoswich_H

#include "KVINDRAIDTelescope.h"


class KVIDPhoswich: public KVINDRAIDTelescope {

public:

   KVIDPhoswich();
   virtual ~ KVIDPhoswich();

   const Char_t* GetArrayName();

   ClassDef(KVIDPhoswich, 1)   //INDRA identification using Phoswich R-L matrices
};

#endif
