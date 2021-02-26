/***************************************************************************
$Id: KVPhoswich.cpp,v 1.14 2007/01/04 16:38:50 franklan Exp $
                          kvphoswich.cpp  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
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
#include "Riostream.h"
#include "KVPhoswich.h"
#include "KVACQParam.h"

ClassImp(KVPhoswich);



//______________________________________________________________________________
KVPhoswich::KVPhoswich()
{
   //Default ctor

   fSegment = 2;
}

//______________________________________________________________________________
KVPhoswich::KVPhoswich(Float_t): KVINDRADetector("NE102", 0.05)
{
   //Build phoswich telescope from two layers of plastic - (NE102) and (NE115).
   // 'thick' is a dummy argument, it is not used
   //    thickness of NE102 = 0.05cm for standard INDRA phoswich
   //    thickness of NE115 = 25cm for standard INDRA phoswich
   //Both layers are "active" - this requires redefinition of GetEnergy etc. methods
   //Type of detector is "PHOS"

   KVMaterial* ne115 = new KVMaterial("NE102", 25);
   AddAbsorber(ne115);

   fSegment = 2;
   SetType("PHOS");
}

//____________________________________________________________________________________________

KVPhoswich::~KVPhoswich()
{
}


//___________________________________________________________________________________
Double_t KVPhoswich::GetEnergy() const
{
   //Override KVDetector::GetEnergy to return total energy lost in both layers

   return (GetAbsorber(0)->GetEnergyLoss() +
           GetAbsorber(1)->GetEnergyLoss());
}

//_____________________________________________________________________________________
void KVPhoswich::SetEnergy(Double_t e) const
{
   //Set energy lost in both layers

   GetAbsorber(0)->SetEnergyLoss(e / 2.);
   GetAbsorber(1)->SetEnergyLoss(e / 2.);
}

//_______________________________________________________________________________________
const Char_t* KVPhoswich::GetArrayName()
{
   //Overrides KVDetector method.
   //Gives name of phoswich as PHOS_01, PHOS_02, etc.

   fFName.Form("PHOS_%02d", GetTelescope()->GetNumber());
   return fFName.Data();
}

//void KVPhoswich::SetACQParams()
//{
//   //Set acquisition parameters for this phoswich.
//   //Do not call before detector's name has been set.

//   AddACQParamType("R");
//   AddACQParamType("L");
//   AddACQParamType("T");
//}
