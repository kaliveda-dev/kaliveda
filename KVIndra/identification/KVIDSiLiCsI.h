/***************************************************************************
                          KVIDSiLiCsI.h  -  description
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

#ifndef KVIDSiLiCsI_H
#define KVIDSiLiCsI_H

#include "KVIDSiCsI.h"


class KVIDSiLiCsI: public KVIDSiCsI {

public:

   KVIDSiLiCsI();
   virtual ~ KVIDSiLiCsI();

   ClassDef(KVIDSiLiCsI, 1)    //INDRA identification using SiLi-CsI matrices
};

#endif
