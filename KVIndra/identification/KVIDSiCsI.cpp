/***************************************************************************
                          KVIDSiCsI.cpp  -  description
                             -------------------
    begin                : Fri Feb 20 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr
$Id: KVIDSiCsI.cpp,v 1.15 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVIDSiCsI.h"
#include "KVINDRACodeMask.h"

ClassImp(KVIDSiCsI)
//////////////////////////////////////////////////////////////////////////////
//KVIDSiCsI
//
//Identification in Si-CsI matrices of INDRA
//
//Identification subcodes are written in bits 8-11 of KVIDSubCodeManager
//(see KVINDRACodes)
KVIDSiCsI::KVIDSiCsI()
{
   //set IDCode
   fIDCode = kIDCode_SiCsI;
   fZminCode = kIDCode_ArretSi;
   fECode = kECode1;
   SetSubCodeManager(4, 11);
}

KVIDSiCsI::~KVIDSiCsI()
{
}
