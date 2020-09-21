/***************************************************************************
$Id: KVLayer.cpp,v 1.17 2006/10/19 14:32:43 franklan Exp $
                          kvlayer.cpp  -  description
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
#include "KVLayer.h"
#include "KVRing.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"

using namespace std;

ClassImp(KVLayer)

KVLayer::KVLayer()
{
   //default ctor
   SetType("LAYER");
}


//______________________________________________________________________________
KVLayer::~KVLayer()
{
}

//_______________________________________________________________________________________

Int_t KVLayer::Compare(const TObject* obj) const
{
   //For sorting lists of layer according to layer number.
   if (GetNumber() < ((KVLayer*) obj)->GetNumber())
      return -1;
   else if (GetNumber() > ((KVLayer*) obj)->GetNumber())
      return 1;
   else
      return 0;
}

//____________________________________________________________________________________
const Char_t* KVLayer::GetName() const
{
   //If name of layer has not been explicitly set with SetName(Char_t*),
   //return name as Layer 1, Layer 2 etc.

   static TString fDyName;
   if (!strcmp(fName.Data(), "")) {
      fDyName.Form("Layer %d", GetNumber());
      return fDyName;
   }
   else
      return fName.Data();
}


//___________________________________________________________________________________________

TGeoVolume* KVLayer::GetGeoVolume()
{
   // Create and return TGeoVolume representing detectors in this layer

   TGeoVolume* mother_vol = gGeoManager->MakeVolumeAssembly(GetName());
   //**** BUILD & ADD Rings ****
   TIter next(GetStructures());
   KVRing* det;
   while ((det = (KVRing*)next())) {
      TGeoVolume* det_vol = det->GetGeoVolume();
      // position ring in layer
      TGeoTranslation* tr = new TGeoTranslation(0, 0, det->GetDistance()); //distance set in KVRing::GetGeoVolume()
      mother_vol->AddNode(det_vol, 1, tr);
   }
   return mother_vol;
}

void KVLayer::AddToGeometry()
{
   // Construct and position a TGeoVolume shape to represent this layer in the current geometry
   if (!gGeoManager) return;

   // get volume for layer
   TGeoVolume* vol = GetGeoVolume();

   // add to geometry
   TGeoTranslation* tr = new TGeoTranslation(0, 0, 0);

   // add ring volume to geometry
   gGeoManager->GetTopVolume()->AddNode(vol, 1, tr);
}


