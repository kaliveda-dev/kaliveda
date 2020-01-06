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
#include "KVIDGraph.h"

ClassImp(KVIDSiCsI)
//////////////////////////////////////////////////////////////////////////////
//KVIDSiCsI
//
//Identification in Si-CsI matrices of INDRA
//
////////////////////////////////////////////////////////////////////////////////

KVIDSiCsI::KVIDSiCsI()
{
   //set IDCode
   set_id_code(kIDCode_SiCsI);
   fZminCode = kIDCode_ArretSi;
   fECode = kECode1;
   fPIEDESTAL = nullptr;
}

void KVIDSiCsI::Initialize()
{
   // perform standard initialization plus initialize Silicon pedestal cut "PIEDESTAL" (if defined
   // - by default we look in the first grid, assumed to be the only one)

   KVINDRAIDTelescope::Initialize();
   fPIEDESTAL = nullptr;
   if (GetIDGrid()) fPIEDESTAL = (KVIDCutLine*)GetIDGrid()->GetCut("PIEDESTAL");
}

Bool_t KVIDSiCsI::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   // Perform identification attempt for (x,y) and check position compared to silicon pedestal
   // (used to set idr->deltaEpedestal in order to identify neutral particles identified in CsI)

   Bool_t ok = KVINDRAIDTelescope::Identify(idr, x, y);

   Double_t de, e;
   GetIDGridCoords(e, de, GetIDGrid(), x, y);

   // check if silicon is in pedestal region (possible neutron)
   if (fPIEDESTAL) {
      if (fPIEDESTAL->TestPoint(e, de)) idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_NO;
      else idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_YES;
   }
   else {
      idr->deltaEpedestal = KVIdentificationResult::deltaEpedestal_UNKNOWN;
   }

   return ok;
}

