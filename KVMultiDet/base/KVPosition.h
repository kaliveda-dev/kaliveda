/***************************************************************************
                          kvposition.h  -  description
                             -------------------
    begin                : Sun May 19 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVPosition.h,v 1.20 2009/01/19 14:36:57 ebonnet Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVPOSITION_H
#define KVPOSITION_H
#include "TMath.h"
#include "KVBase.h"
#include "TVector3.h"
#include "TRotation.h"
class TGeoHMatrix;
class TGeoBBox;

class KVPosition {
private:
   Double_t fTheta;              // polar angle in degrees with respect to beam axis, corresponds to centre of telescope
   Double_t fPhi;                // azimuthal angle in degrees with respect to 12 o'clock (=0 deg.), corresponds to centre of telescope
   Double_t fTheta_min;          // polar angle in degrees corresponding to edge of telescope closest to beam axis
   Double_t fTheta_max;          // polar angle in degrees of the edge furthest from the beam axis
   Double_t fPhi_min;            // azimuthal angle in degrees corresponding to most anticlockwise edge of telescope
   Double_t fPhi_max;            // azimuthal angle in degrees corresponding to most clockwise edge of telescope
   Double_t fDistance;           // distance in cm from centre of solid angle element to coordinate system origin (target)

   /* ROOT Geometry */
   TGeoHMatrix* fMatrix;        // transform world<->detector coordinates
   TGeoBBox*    fShape;         // shape of detector volume
   Double_t     fSolidAngle;    // solid angle = area of entrance window / distance**2

public:
   KVPosition();
   KVPosition(Double_t thmin, Double_t thmax, Double_t phmin, Double_t phmax, Double_t dist = 0.0);
   void init();
   virtual ~KVPosition();
   virtual void SetAzimuthalAngle(Double_t ph);
   virtual void SetPolarAngle(Double_t th);
   virtual void SetPolarWidth(Double_t pw);
   virtual void SetPolarMinMax(Double_t min, Double_t max);
   virtual void SetAzimuthalWidth(Double_t aw);
   virtual void SetAzimuthalMinMax(Double_t min, Double_t max);
   void SetPhiMinMax(Double_t min, Double_t max)
   {
      SetAzimuthalMinMax(min, max);
   };
   TVector3 GetRandomDirection(Option_t* t = "isotropic");
   TRotation GetRandomIsotropicRotation();
   void GetRandomAngles(Double_t& th, Double_t& ph, Option_t* t = "isotropic");
   TVector3 GetDirection();

   Bool_t IsInPolarRange(const Double_t theta);
   Bool_t IsInPhiRange(const Double_t phi);
   Bool_t IsSmallerThan(KVPosition* pos);
   Bool_t IsAlignedWith(KVPosition* pos);
   Bool_t IsOverlappingWith(KVPosition* pos);
   Bool_t IsAzimuthallyWiderThan(KVPosition* pos);

   inline Double_t GetPhiMin() const
   {
      return fPhi_min;
   };
   inline Double_t GetPhiMax() const
   {
      return fPhi_max;
   };
   inline Double_t GetThetaMin() const
   {
      return fTheta_min;
   };
   inline Double_t GetThetaMax() const
   {
      return fTheta_max;
   };
   Double_t GetAzimuthalWidth(Double_t phmin = -1., Double_t phimax =
                                 -1.) const;
   Double_t GetTheta() const
   {
      return fTheta;
   };
   Double_t GetSinTheta() const
   {
      return TMath::Sin(fTheta * TMath::DegToRad());
   };
   Double_t GetCosTheta() const
   {
      return TMath::Cos(fTheta * TMath::DegToRad());
   };
   Double_t GetPhi()
   {
      if (fPhi >= 360.0)
         fPhi -= 360.;
      return fPhi;
   };
   void SetTheta(Double_t t)
   {
      SetPolarAngle(t);
   };
   void SetPhi(Double_t p)
   {
      SetAzimuthalAngle(p);
   };
   void SetDistance(Double_t d)
   {
      fDistance = d;
   };
   Double_t GetDistance(void) const
   {
      return fDistance;
   };
   void GetCornerCoordinates(TVector3*, Double_t /*depth*/ = 0);
   void GetCornerCoordinatesInOwnFrame(TVector3*, Double_t /*depth*/ = 0);
   void GetWidthsFromDimension(Double_t lin_dim);

   virtual Double_t GetSolidAngle(void);

   /* ROOT Geometry */
   Bool_t ROOTGeo() const;
   void SetMatrix(const TGeoHMatrix*);
   void SetShape(TGeoBBox*);
   TGeoHMatrix* GetMatrix() const;
   TGeoBBox* GetShape() const;
   TVector3 GetRandomPoint() const;
   TVector3 GetCentre() const;

   ClassDef(KVPosition, 1)//Class handling geometry of detectors in a multidetector array
};

#endif
