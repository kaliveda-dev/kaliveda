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

/**
   \class KVPosition
  \brief Base class used for handling geometry in a multidetector array.
  \ingroup Geometry

Each KVPosition object represents an element of solid angle, with a domain in
polar angle (theta) and azimuthal angle (phi). All methods necessary to
inquire about relative positions of such elements (overlaps, inclusion, etc.)
are provided.

The coordinate system has the target position for its centre. The beam
direction defines the positive z axis, with respect to which polar angles are
measured. The +ve x-axis is taken to be vertical, and azimuthal angles
increase when going clockwise from the +ve x-axis, looking towards the +ve
z direction:

~~~~
               phi=0
    (x) beam    |
                |
                |
                +------- phi=90
~~~~

All angles are in degrees.

Polar angles (theta) vary between 0 and 180 degrees
Azimuthal angles (phi) vary between 0 and 359.999... degrees.

An element is defined by theta-min/max and phi-min/max:
 - Theta-min is the polar angle of the edge closest the beam (+ve z) axis
 - Theta-max is the polar angle of the edge furthest from the beam (+ve z) axis
 - Phi-min is the phi angle of the edge which is the most anticlockwise when
    looking as in the figure shown above
 - Phi-max is the phi angle of the edge which is the most clockwise when
    looking as in the figure shown above

The above definitions mean that phi-min is not necessarily smaller than
phi-max: we redefine "smaller" for phi angles to mean "more anticlockwise than"
or "less clockwise than". Some examples:
 - an element with phi-min = 20 and phi-max = 40 has an azimuthal width of 20
 degrees;
 - an element with phi-min = 40 and phi-max = 20 has an azimuthal width of 340
 degrees;
 - an element with phi-min = 350 and phi-max = 10 has an azimuthal width of
 20 degrees.

#### Examples of use of KVPosition objects

Generating a random unit vector within a given angular range

~~~~{.cpp}
  KVPosition pos;                 |
  pos.SetPolarMinMax(20, 30);     |  --  or:  KVPosition pos(20, 30, 0, 90);
  pos.SetAzimuthalMinMax(0, 90);  |
  TVector3 dir1 = pos.GetDirection();              //unit vector in direction theta=25, phi=45
  TVector3 dir2 = pos.GetRandomDirection();        //isotropic direction within angular limits of pos
  TVector3 dir3 = pos.GetRandomDirection("random");//random direction
~~~~
*/

class KVPosition {
private:
   Double_t fTheta;              // polar angle in degrees with respect to beam axis, corresponds to centre of telescope
   mutable Double_t fPhi;                // azimuthal angle in degrees with respect to 12 o'clock (=0 deg.), corresponds to centre of telescope
   Double_t fTheta_min;          // polar angle in degrees corresponding to edge of telescope closest to beam axis
   Double_t fTheta_max;          // polar angle in degrees of the edge furthest from the beam axis
   Double_t fPhi_min;            // azimuthal angle in degrees corresponding to most anticlockwise edge of telescope
   Double_t fPhi_max;            // azimuthal angle in degrees corresponding to most clockwise edge of telescope
   Double_t fDistance;           // distance in cm from centre of solid angle element to coordinate system origin (target)

   /* ROOT Geometry */
   TGeoHMatrix* fMatrix;        // transform world<->detector coordinates
   TGeoBBox*    fShape;         // shape of detector volume
   mutable Double_t     fSolidAngle;    // solid angle = area of entrance window / distance**2

   TVector3 GetSCVector() const
   {
      return GetVolumeCentre() - GetSurfaceCentre();
   }
   Double_t GetOC_SC_CosAngle() const
   {
      return GetVolumeCentre().Unit().Dot(GetSCVector().Unit());
   }

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
   }
   virtual TVector3 GetRandomDirection(Option_t* t = "isotropic");
   TRotation GetRandomIsotropicRotation();
   virtual void GetRandomAngles(Double_t& th, Double_t& ph, Option_t* t = "isotropic");
   virtual TVector3 GetDirection();

   Bool_t IsInPolarRange(const Double_t theta);
   Bool_t IsInPhiRange(const Double_t phi);
   Bool_t IsSmallerThan(KVPosition* pos);
   Bool_t IsAlignedWith(KVPosition* pos);
   Bool_t IsOverlappingWith(KVPosition* pos);
   Bool_t IsAzimuthallyWiderThan(KVPosition* pos);

   inline Double_t GetPhiMin() const
   {
      return fPhi_min;
   }
   inline Double_t GetPhiMax() const
   {
      return fPhi_max;
   }
   inline Double_t GetThetaMin() const
   {
      return fTheta_min;
   }
   inline Double_t GetThetaMax() const
   {
      return fTheta_max;
   }
   Double_t GetAzimuthalWidth(Double_t phmin = -1., Double_t phimax =
                                 -1.) const;
   virtual Double_t GetTheta() const
   {
      return fTheta;
   }
   virtual Double_t GetSinTheta() const
   {
      return TMath::Sin(fTheta * TMath::DegToRad());
   }
   virtual Double_t GetCosTheta() const
   {
      return TMath::Cos(fTheta * TMath::DegToRad());
   }
   virtual Double_t GetPhi() const
   {
      if (fPhi >= 360.0)
         fPhi -= 360.;
      return fPhi;
   }
   void SetTheta(Double_t t)
   {
      SetPolarAngle(t);
   }
   void SetPhi(Double_t p)
   {
      SetAzimuthalAngle(p);
   }
   void SetDistance(Double_t d)
   {
      fDistance = d;
   }
   virtual Double_t GetDistance(void) const
   {
      return fDistance;
   }
   void GetCornerCoordinates(TVector3*, Double_t /*depth*/ = 0);
   void GetCornerCoordinatesInOwnFrame(TVector3*, Double_t /*depth*/ = 0);
   void GetWidthsFromDimension(Double_t lin_dim);

   virtual Double_t GetSolidAngle(void) const;

   /* ROOT Geometry */
   Bool_t ROOTGeo() const;
   virtual void SetMatrix(const TGeoHMatrix*);
   virtual void SetShape(TGeoBBox*);
   virtual TGeoHMatrix* GetMatrix() const;
   virtual TGeoBBox* GetShape() const;
   virtual TVector3 GetRandomPointOnSurface() const;
   virtual TVector3 GetSurfaceCentre() const;
   virtual TVector3 GetVolumeCentre() const;
   virtual TVector3 GetSurfaceNormal() const;
   virtual Double_t GetSurfaceArea(int npoints = 100000) const;
   virtual Double_t GetMisalignmentAngle() const;

   ClassDef(KVPosition, 1)//Class handling geometry of detectors in a multidetector array
};

#endif
