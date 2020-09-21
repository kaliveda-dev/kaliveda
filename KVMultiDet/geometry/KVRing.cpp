/***************************************************************************
$Id: KVRing.cpp,v 1.22 2007/05/31 09:59:22 franklan Exp $
                          kvring.cpp  -  description
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
#include "KVRing.h"
#include "KVLayer.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "KVTelescope.h"

using namespace std;

ClassImp(KVRing)


KVRing::KVRing()
{
   init();
}

void KVRing::init()
{
   SetType("RING");
}

//_______________________________________________________________________________________
KVRing::~KVRing()
{
}

void KVRing::Add(KVBase* b)
{
   // Only KVTelescope-derived structures can be placed in a KVRing

   if (!b->InheritsFrom("KVTelescope")) return;
   KVGeoStrucElement::Add(b);
}


//_______________________________________________________________________________________
KVTelescope* KVRing::GetTelescope(Float_t phi) const
{
   // give pointer to telescope in ring which covers azimuthal angle phi
   TIter next(GetTelescopes());
   KVTelescope* obj;
   while ((obj = (KVTelescope*) next())) {      // loop over telescopes in ring
      if (obj->IsInPhiRange(phi))
         return obj;
   }
   return NULL;
}

//_______________________________________________________________________________________
KVTelescope* KVRing::GetTelescope(const Char_t* name) const
{
   // give pointer to telescope in ring with name
   return (KVTelescope*)GetStructure("TELESCOPE", name);
}

//____________________________________________________________________________________________
Int_t KVRing::Compare(const TObject* obj) const
{
   // rings are sorted according to lower edge polar angle
   if (GetThetaMin() < ((KVRing*) obj)->GetThetaMin())
      return -1;
   else if (GetThetaMin() > ((KVRing*) obj)->GetThetaMin())
      return 1;
   else
      return 0;
}

Double_t KVRing::GetSolidAngle() const
{
   // redefinition of KVPosition::GetSolidAngle() to take into account
   // only KVTelescope of the considered KVLayer which define the KVRing
   Double_t sol_ang = 0;
   KVTelescope* tel;
   TIter nxttel(GetTelescopes());
   while ((tel = (KVTelescope*) nxttel())) sol_ang += tel->GetSolidAngle();
   return sol_ang;

}




//___________________________________________________________________________________________

TGeoVolume* KVRing::GetGeoVolume()
{
   // Create and return TGeoVolume representing detectors in this ring.

   TString name(GetName());
   name.ToUpper();
   name.ReplaceAll(" ", "_");
   TGeoVolume* mother_vol = gGeoManager->MakeVolumeAssembly(name.Data());
   //**** BUILD & ADD TELESCOPEs ****
   TIter next(GetTelescopes());
   KVTelescope* det;
   while ((det = (KVTelescope*)next())) {
      TGeoVolume* det_vol = det->GetGeoVolume();
      // position telescope in ring
      // rotate telescope to orientation corresponding to (theta,phi)
      Double_t theta = det->GetTheta();
      Double_t phi = det->GetPhi();
      TGeoRotation rot1, rot2;
      rot2.SetAngles(phi + 90., theta, 0.);
      rot1.SetAngles(-90., 0., 0.);
      Double_t tot_len_tel = det->GetTotalLengthInCM();
      // distance to telescope centre = distance to telescope + half total length of telescope
      Double_t dist = det->GetDistance() + tot_len_tel / 2.;
      // translate telescope to correct distance from target (note: reference is CENTRE of telescope)
      Double_t trans_1 = dist;
      // translate telescope so that centre of ring is on origin
      Double_t trans_2 = dist * TMath::Cos(theta * TMath::DegToRad());
      // set distance for ring = distance between origin and centre of ring
      SetDistance(trans_2);// distance in cm

      TGeoTranslation tran1(0, 0, trans_1);
      TGeoTranslation tran2(0, 0, -trans_2);
      TGeoHMatrix h = tran2 * rot2 * tran1 * rot1;
      TGeoHMatrix* ph = new TGeoHMatrix(h);

      mother_vol->AddNode(det_vol, 1, ph);
   }
   return mother_vol;
}

void KVRing::AddToGeometry()
{
   // Construct and position a TGeoVolume shape to represent this ring in the current geometry
   if (!gGeoManager) return;

   // get volume for ring
   TGeoVolume* vol = GetGeoVolume();

   // translate ring to correct distance
   TGeoTranslation* tr = new TGeoTranslation(0, 0, GetDistance()); //distance set in GetGeoVolume()

   // add ring volume to geometry
   gGeoManager->GetTopVolume()->AddNode(vol, 1, tr);
}


