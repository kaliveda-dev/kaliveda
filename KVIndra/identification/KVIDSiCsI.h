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
#include "KVIDCutLine.h"
/**
  \class KVIDSiCsI
  \ingroup INDRAIdent
  \brief Identification in Si-CsI telescopes of INDRA
 */
class KVIDSiCsI: public KVINDRAIDTelescope {

protected:
   KVIDCutLine* fPIEDESTAL;      //! Upper limit of Silicon pedestal zone (neutral particles)

public:

   KVIDSiCsI();
   virtual ~ KVIDSiCsI() {}

   void Initialize();
   Bool_t Identify(KVIdentificationResult* idr, Double_t x = -1, Double_t y = -1);

   ClassDef(KVIDSiCsI, 1)       //INDRA identification using Si-CsI matrices
};

#endif
