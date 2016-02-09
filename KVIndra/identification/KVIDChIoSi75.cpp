/***************************************************************************
                          KVIDChIoSi75.cpp  -  description
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

#include "KVIDChIoSi75.h"
#include "KVINDRACodes.h"

ClassImp(KVIDChIoSi75)
//////////////////////////////////////////////////////////////////////////////
//KVIDChIoSi75
//
//Identification in ChIo-Si75 matrices of INDRA
//
//Identification subcodes are written in bits 12-15 of KVIDSubCodeManager
//(see KVINDRACodes)
KVIDChIoSi75::KVIDChIoSi75()
{
   fECode = kECode1;
   SetSubCodeManager(4, 15);
}

KVIDChIoSi75::~KVIDChIoSi75()
{
}
