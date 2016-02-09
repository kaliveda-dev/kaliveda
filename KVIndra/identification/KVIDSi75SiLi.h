/***************************************************************************
                          KVIDSi75SiLi.h  -  description
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

#ifndef KVIDSi75SiLi_H
#define KVIDSi75SiLi_H

#include "KVINDRAIDTelescope.h"


class KVIDSi75SiLi: public KVINDRAIDTelescope {

public:

   KVIDSi75SiLi();
   virtual ~ KVIDSi75SiLi();

   ClassDef(KVIDSi75SiLi, 1)    //INDRA identification using Si75-Si(Li) matrices
};

#endif
