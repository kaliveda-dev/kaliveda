/***************************************************************************
                          KVIDSiLiCsI.cpp  -  description
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

#include "KVIDSiLiCsI.h"
#include "KVINDRACodes.h"

ClassImp(KVIDSiLiCsI)
//////////////////////////////////////////////////////////////////////////////
//KVIDSiLiCsI
//
//Identification in SiLi-CsI matrices of INDRA
//

KVIDSiLiCsI::KVIDSiLiCsI()
{
   set_id_code(kIDCode_SiLiCsI);
   fZminCode = kIDCode_ArretSi;
   fECode = kECode1;
}

KVIDSiLiCsI::~KVIDSiLiCsI()
{
}
