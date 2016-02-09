/***************************************************************************
                          KVTelBlocking.cpp  -  description
                             -------------------
    begin                : Mon Nov 3 2003
    copyright            : (C) 2003 by John Frankland
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

#include "KVTelBlocking.h"
#include "KVBIC.h"
#include "KVSiB.h"

ClassImp(KVTelBlocking)

KVTelBlocking::KVTelBlocking(UInt_t n): KVTelescope()
{
   //Create blocking telescope number 'n'.
   //By default the BIC pressure is 30 (Torr)

   if (n) {                     //default ctor must not add detectors, or Clone() will give a telescope with 2 more detectors!!
      SetNumber(n);
      Add(new KVBIC(30.));
      Add(new KVSiB(500.));
      SetDepth(1, 0.);
      SetDepth(2, 119.);        //SIB is 119mm behind BIC
   }
   SetName(Form("TEL_BLOCK_%d", (Int_t)n));
}

//__________________________________________________________________________

KVTelBlocking::~KVTelBlocking()
{
}

//__________________________________________________________________________
