/***************************************************************************
                          KVIDSi75SiLi.cpp  -  description
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

#include "KVIDSi75SiLi.h"
#include "KVINDRACodes.h"

ClassImp(KVIDSi75SiLi)
///////////////////////////////////////////////////////////////////////////
//KVIDSi75SiLi
//
//Identification in Si75-SiLI matrices of INDRA
//
//Identification subcodes are written in bits 4-7 of KVIDSubCodeManager
//(see KVINDRACodes)
KVIDSi75SiLi::KVIDSi75SiLi()
{
   fIDCode = kIDCode_Si75SiLi;
   fZminCode = kIDCode_Zmin;
   fECode = kECode1;
   SetSubCodeManager(4, 7);
}

KVIDSi75SiLi::~KVIDSi75SiLi()
{
}
