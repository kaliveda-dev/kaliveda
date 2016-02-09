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
//Identification subcodes are written in bits 8-11 of KVIDSubCodeManager
//(see KVINDRACodes)
KVIDSiLiCsI::KVIDSiLiCsI()
{
   fIDCode = kIDCode_SiLiCsI;
   fZminCode = kIDCode_ArretSi;
   fECode = kECode1;
   SetSubCodeManager(4, 11);
}

KVIDSiLiCsI::~KVIDSiLiCsI()
{
}

void KVIDSiLiCsI::Calibrate(KVReconstructedNucleus*)
{
   //Redefinition of KVIDSiCsI method
   //Should take into account different structure of etalon telescope (CI - SI75 - SILI - CSI)
   //(not implemented)
   return;
}
