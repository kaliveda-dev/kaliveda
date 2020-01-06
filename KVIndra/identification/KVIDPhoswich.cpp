/***************************************************************************
                          KVIDPhoswich.cpp  -  description
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

#include "KVIDPhoswich.h"
#include "KVTelescope.h"
#include "KVINDRACodes.h"
#include <KVINDRATelescope.h>
#include <KVINDRADetector.h>

ClassImp(KVIDPhoswich)
///////////////////////////////////////////////////////////////////////////
//KVIDPhoswich
//
//Identification in Phoswich R-L matrices of INDRA
//

KVIDPhoswich::KVIDPhoswich()
{
   set_id_code(kIDCode_Phoswich);
   fZminCode = kIDCode_ZminCsI;
   fECode = kECode1;
}

KVIDPhoswich::~KVIDPhoswich()
{
}

const Char_t* KVIDPhoswich::GetArrayName()
{
   // Name of telescope given in the form PHOS_R_L_Telescope-number

   //in order to access angular dimensions of detectors, we need their KVTelescopes
   KVINDRATelescope* de_det = dynamic_cast<KVINDRADetector*>(GetDetector(1))->GetTelescope();
   UInt_t mod;
   mod = de_det->GetNumber();
   TString dummy;
   dummy.Form("%s_R_L", GetDetector(1)->GetType());
   SetType(dummy.Data());
   dummy.Form("%s_%02d", GetType(), mod);
   SetName(dummy.Data());

   return fName.Data();
}
