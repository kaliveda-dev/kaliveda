/***************************************************************************
                          KVIDChIoCsI.cpp  -  description
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

#include "KVIDChIoCsI.h"
#include "KVReconstructedNucleus.h"
#include "KVINDRACodes.h"

ClassImp(KVIDChIoCsI)
/////////////////////////////////////////////////////////////////////
//KVIDChIoCsI
//
//Identification in ChIo-CsI matrices of INDRA
//

KVIDChIoCsI::KVIDChIoCsI()
{
   //set ID code for telescope
   set_id_code(kIDCode_ChIoCsI);
   fZminCode = kIDCode_ArretChIo;
   fECode = kECode1;
}


Bool_t KVIDChIoCsI::CheckTheoreticalIdentificationThreshold(KVNucleus* ION, Double_t)
{
   // Overrides KVIDTelescope method
   // We lower the calculated energy threshold to be better in agreement with data
   // EINC argument not used as ChIo-CsI is always first telescope

   Double_t emin = (1.9 / 2.5) * (fChIo->GetEIncOfMaxDeltaE(ION->GetZ(), ION->GetA()));
   return (ION->GetEnergy() > emin);

}

void KVIDChIoCsI::Initialize()
{
   // Initialize telescope for current run.

   KVINDRAIDTelescope::Initialize();
   fChIo = (KVChIo*) GetDetector(1);
   fCsI = (KVCsI*) GetDetector(2);
}
