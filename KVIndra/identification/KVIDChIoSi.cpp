/***************************************************************************
                          KVIDChIoSi.cpp  -  description
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

#include "KVIDChIoSi.h"
#include "KVIdentificationResult.h"
#include "KVINDRACodeMask.h"
#include "KVDataSet.h"

ClassImp(KVIDChIoSi)
/////////////////////////////////////////////////////////////////////////////
//KVIDChIoSi
//
//Identification in ChIo-Si matrices of INDRA
//
/////////////////////////////////////////////////////////////////////////////

KVIDChIoSi::KVIDChIoSi()
{
   set_id_code(kIDCode_ChIoSi);
   fZminCode = kIDCode_ArretChIo;
   fECode = kECode1;
   ChIoSiGrid = 0;
   fchio = fsi = 0;
}

KVIDChIoSi::~KVIDChIoSi()
{
}

//________________________________________________________________________________________//

Bool_t KVIDChIoSi::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid KVIDGChIoSi

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;
   //identification
   Double_t chio = (y < 0. ? GetIDMapY() : y);
   Double_t si = (x < 0. ? GetIDMapX() : x);

   if (ChIoSiGrid->IsIdentifiable(si, chio))
      ChIoSiGrid->Identify(si, chio, IDR);
   Int_t quality = ChIoSiGrid->GetQualityCode();
   IDR->IDquality = quality;

   // set general ID code
   IDR->IDcode = kIDCode4;
   //if point lies above Zmax line, we give Zmax as Z of particle (real Z is >= Zmax)
   //general ID code = kIDCode5 (Zmin)
   if (quality == KVIDZAGrid::kICODE7) {
      IDR->IDcode = kIDCode5;
   }
   //Identified particles with subcode kID_LeftOfBragg are given
   //general ID code kIDCode5 (Zmin).
   if (quality == KVIDGChIoSi::k_LeftOfBragg) {
      IDR->IDcode = kIDCode5;
   }
   //unidentifiable particles with subcode kID_BelowSeuilSi are given
   //general ID code kIDCode5 (Zmin) and we estimate Zmin from energy
   //loss in ChIo
   if (quality == KVIDGChIoSi::k_BelowSeuilSi) {
      IDR->IDcode = kIDCode5;
      IDR->Z = fchio->FindZmin();
      IDR->SetComment("point to identify left of Si threshold line (bruit/arret ChIo?)");
   }
   if (quality == KVIDGChIoSi::k_RightOfEmaxSi) IDR->SetComment("point to identify has E_Si > Emax_Si i.e. codeur is saturated. Unidentifiable");

   return kTRUE;
}

//____________________________________________________________________________________

void KVIDChIoSi::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run
   // or if no calib/ident parameters are available for current dataset.

   fchio = GetDetector(1);
   fsi = GetDetector(2);
//   fsipgped = fsi->GetPedestal("PG");
//   fchiopgped = fchio->GetPedestal("PG");
   ChIoSiGrid = (KVIDGChIoSi*) GetIDGrid();
   if (ChIoSiGrid) {
      ChIoSiGrid->Initialize();
      SetBit(kReadyForID);
   }
   else
      ResetBit(kReadyForID);
   if (!gDataSet->HasCalibIdentInfos()) SetBit(kReadyForID);
}

Bool_t KVIDChIoSi::CheckTheoreticalIdentificationThreshold(KVNucleus* ION, Double_t)
{
   // Overrides KVIDTelescope method
   // We lower the calculated energy threshold to be better in agreement with data
   // EINC argument not used as ChIo-Si is always first telescope

   Double_t emin = (1.9 / 2.5) * (fchio->GetEIncOfMaxDeltaE(ION->GetZ(), ION->GetA()));
   return (ION->GetEnergy() > emin);

}
