/***************************************************************************
                          KVIDBlocking.cpp  -  description
                             -------------------
    begin                : Thu Oct 13 2005
    copyright            : (C) 2005 by J.D. Frankland
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

#include "KVIDBlocking.h"
#include "TString.h"
#include "KVDetector.h"

ClassImp(KVIDBlocking)
///////////////////////////////////////////////////////////////////////////////////////////
//KVIDBlocking
//
//Identification in BIC-SiB blocking telescopes of E416(a)
Bool_t KVIDBlocking::SetIDGrid(KVIDGrid*)
{
   //No grids for the moment
   return kFALSE;
}

//________________________________________________________________________________________//

Double_t KVIDBlocking::GetIDMapX(Option_t* gain)
{
   //X coordinate for BIC-SiB identification is energy parameter of SiB
   //"gain" can be "GG" (we use value of SIB_x_E_GG) or "" (SIB_x_E)

   TString type("E");
   if (!strcmp(gain, "GG") || !strcmp(gain, "gg"))
      type += "_GG";
   return GetDetector(2)->GetACQData(type.Data());
}

Double_t KVIDBlocking::GetIDMapY(Option_t* gain)
{
   //Y coordinate for BIC-SIB identification is BIC PG or GG
   TString _gain(gain);
   _gain.ToUpper();
   return GetDetector(1)->GetACQData(_gain.Data());
}
