/***************************************************************************
                          KVIDSiCsI.h  -  description
                             -------------------
    begin                : Fri Feb 20 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr
$Id: KVIDSiCsI.h,v 1.10 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVIDSICSI_H
#define KVIDSICSI_H

#include "KVINDRAIDTelescope.h"

class KVIDSiCsI: public KVINDRAIDTelescope {

public:

   KVIDSiCsI();
   virtual ~ KVIDSiCsI();

   ClassDef(KVIDSiCsI, 1)       //INDRA identification using Si-CsI matrices
};

#endif
