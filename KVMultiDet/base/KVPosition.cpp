/***************************************************************************
$Id: KVPosition.cpp,v 1.21 2008/12/17 13:01:26 franklan Exp $
                          kvposition.cpp  -  description
                             -------------------
    begin                : Sun May 19 2002
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

#include "KVPosition.h"
#include "TRandom3.h"
#include "Riostream.h"
#include "TMath.h"
#include "TGeoMatrix.h"
#include "TGeoBBox.h"

#define ROOT_GEO_DO_NOT_USE(method,retval) \
   if(ROOTGeo()){ \
      ::Warning(method, "Does not work with ROOT geometries. Do not use."); \
      return retval; \
   }

ClassImp(KVPosition);


void KVPosition::init()
{
   //default initialiser

   fTheta = fPhi = -1.0;
   fTheta_min = fTheta_max = fPhi_min = fPhi_max = fDistance = 0.;
   fMatrix = 0;
   fShape = 0;
   fSolidAngle = 0;
}

KVPosition::~KVPosition()
{
   if (fMatrix) {
      delete fMatrix;
      fMatrix = 0;
   }
}

KVPosition::KVPosition()
{
   init();
}

KVPosition::KVPosition(Double_t thmin, Double_t thmax, Double_t phmin,
                       Double_t phmax, Double_t dist)
{
   init();
   SetPolarMinMax(thmin, thmax);
   SetAzimuthalMinMax(phmin, phmax);
   SetDistance(dist);
};


//_____________________________________________________________________________
void KVPosition::SetPolarAngle(Double_t th)
{
   //Sets the polar angle corresponding to the centre of this telescope/solid angle element/etc.
   //If the polar width has been set already (KVPosition::SetPolarWidth),
   //the limits theta-min and theta-max are calculated.

   fTheta = th;
   if (fTheta_min == -180.) {   // polar width of detector already set, calculate theta_min
      // and theta_max
      fTheta_min = fTheta - .5 * (fTheta_max + 180.);
      fTheta_max = 2. * fTheta - fTheta_min;
   }
}

//___________________________________________________________________________
void KVPosition::SetAzimuthalAngle(Double_t ph)
{
   //Sets the azimuthal angle corresponding to the centre of this telescope/solid angle element/etc.
   //If the azimuthal width has been set already (KVPosition::SetAzimuthalWidth),
   //the limits phi-min and phi-max are calculated.

   fPhi = ph;
   //make sure phi between 0 and 360
   if (fPhi < 0.)
      fPhi += 360.;
   if (fPhi_min == -180.) {     // azimuthal width of detector already set, calculate phi_min
      // and phi_max
      fPhi_min = fPhi - .5 * (fPhi_max + 180.);
      fPhi_max = 2. * fPhi - fPhi_min;
      //make sure phimin and phimax are between 0 and 360
      if (fPhi_min < 0.)
         fPhi_min += 360.;
      if (fPhi_max >= 360.)
         fPhi_max -= 360.;
   }
}

//___________________________________________________________________________
void KVPosition::SetPolarWidth(Double_t pw)
{
   //Set theta_min and theta_max from width (in degrees).
   //If theta is already known, use to set theta_min and theta_max.
   //If not, keep relative values (negative) of theta_min and theta_max,
   //to be used when theta is set

   ROOT_GEO_DO_NOT_USE("SetPolarWidth",)

   if (fTheta > -1.0) {
      fTheta_min = fTheta - .5 * pw;
      fTheta_max = fTheta + .5 * pw;
   }
   else {
      fTheta_min = -180.;
      fTheta_max = fTheta_min + pw;
   }
}

//___________________________________________________________________________
void KVPosition::SetPolarMinMax(Double_t min, Double_t max)
{
   //Set min and max polar angles and calculate (mean) theta

   ROOT_GEO_DO_NOT_USE("SetPolarMinMax",)
   fTheta_min = min;
   fTheta_max = max;
   fTheta = .5 * (min + max);
}

//___________________________________________________________________________
void KVPosition::SetAzimuthalWidth(Double_t aw)
{
   //Set phi_min and phi_max from width (in degrees)
   //  If phi is already known, use to set phi_min and phi_max
   //  If not, keep relative values (negative) of phi_min and phi_max,
   //  to be used when phi is set

   ROOT_GEO_DO_NOT_USE("SetAzimuthalWidth",)
   if (fPhi > -1.0) {
      fPhi_min = fPhi - .5 * aw;
      if (fPhi_min < 0.)
         fPhi_min += 360.;
      fPhi_max = fPhi + .5 * aw;
      if (fPhi_max >= 360.)
         fPhi_max -= 360.;
   }
   else {
      fPhi_min = -180.;
      fPhi_max = fPhi_min + aw;
   }
}

//___________________________________________________________________________
void KVPosition::SetAzimuthalMinMax(Double_t min, Double_t max)
{
   //Set min and max azimuthal angles and calculate (mean) phi

   ROOT_GEO_DO_NOT_USE("SetAzimuthalMinMax",)
   fPhi_min = min;
   fPhi_max = max;
   if (min > max)
      max += 360;
   fPhi = .5 * (min + max);
   if (fPhi >= 360.)
      fPhi -= 360.;
}

//___________________________________________________________________________
TVector3 KVPosition::GetRandomDirection(Option_t* t)
{
   // Returns a unit vector in a random direction corresponding to this detector.
   // Depending on the optional option string, the direction is either drawn at
   // "random" among the corresponding angles, or "isotropic".
   // By default, the direction is "isotropic".
   //
   // * ROOT Geometry *
   // Direction corresponds to a random position on the entrance surface of the volume.
   // The "isotropic" option has no effect.

   if (ROOTGeo()) {
      // * ROOT Geometry *
      TVector3 r = GetRandomPointOnSurface();
      return r.Unit();
   }

   Double_t  dtor = TMath::DegToRad();
   Double_t th, ph;
   GetRandomAngles(th, ph, t);
   TVector3 dummy;
   dummy.SetMagThetaPhi(1.0, th * dtor, ph * dtor); // a unit vector

   return dummy;
}
//___________________________________________________________________________
void KVPosition::GetRandomAngles(Double_t& th, Double_t& ph, Option_t* t)
{
   // Set th and ph to random values between the max and min limits defining the
   // solid angle element.
   // Depending on the optional option string, the direction is either drawn at
   // "random" among the corresponding angles, or "isotropic".
   // By default, the direction is "isotropic".
   //
   // * ROOT Geometry *
   // th and ph correspond to a random position on the detector's entrance window.
   // The "isotropic" option has no effect.

   Double_t dtor = TMath::DegToRad();

   if (ROOTGeo()) {
      // * ROOT Geometry *
      TVector3 r = GetRandomPointOnSurface();
      th = r.Theta() / dtor;
      ph = r.Phi() / dtor;
      if (ph < 0) ph += 360.;
      return;
   }

   Double_t phmin = fPhi_min;     //phimin in radians
   Double_t phmax = fPhi_max;     //phimax in radians
   if (phmax < phmin)
      phmax += 360;

   Double_t dphi = phmax - phmin;
   Double_t thmin, thmax, dtheta;

   if (!strcmp(t, "random")) {
      thmin = fTheta_min * dtor;
      thmax = fTheta_max * dtor;
      dtheta = thmax - thmin;
      th = gRandom->Uniform(dtheta) + thmin;
   }
   else {
      thmin = TMath::Cos(fTheta_min * dtor);
      thmax = TMath::Cos(fTheta_max * dtor);
      dtheta = thmin - thmax;
      th = TMath::ACos(gRandom->Uniform(dtheta) + thmax);
   }
   ph = gRandom->Uniform(dphi) + phmin;
   if (ph > 360)
      ph -= 360;
   th /= dtor;

}

//___________________________________________________________________________
Bool_t KVPosition::IsInPhiRange(const Double_t phi)
{
   //kTRUE if given angle phi is within the azimuthal range of this solid
   //angle element

   ROOT_GEO_DO_NOT_USE("IsInPhiRange", kFALSE)

   Double_t phimintest = fPhi_min;
   Double_t phimaxtest = fPhi_max;
   if (phimintest > phimaxtest) {
      phimaxtest += 360.;
   }
   if (phi >= phimintest && phi <= phimaxtest) {
      return kTRUE;
   }
   if ((phi + 360.) >= phimintest && (phi + 360.) <= phimaxtest) {
      return kTRUE;
   }
   return kFALSE;
}

//___________________________________________________________________________
Bool_t KVPosition::IsInPolarRange(const Double_t theta)
{
   //kTRUE if given angle theta is within the polar range of this solid angle element

   ROOT_GEO_DO_NOT_USE("IsInPolarRange", kFALSE)

   if (theta >= fTheta_min && theta <= fTheta_max)
      return kTRUE;
   else
      return kFALSE;
}

//_____________________________________________________________________________________
Bool_t KVPosition::IsSmallerThan(KVPosition* pos)
{
   // kTRUE if "this" is entirely contained within "pos"

   ROOT_GEO_DO_NOT_USE("IsSmallerThan", kFALSE)
   return (pos->IsInPolarRange(GetThetaMin())
           && pos->IsInPolarRange(GetThetaMax())
           && pos->IsInPhiRange(GetPhiMin())
           && pos->IsInPhiRange(GetPhiMax()));
}

//___________________________________________________________________________
Bool_t KVPosition::IsAlignedWith(KVPosition* pos)
{
   //kTRUE if one of the two solid angle elements is completely contained within the other.

   ROOT_GEO_DO_NOT_USE("IsAlignedWith", kFALSE)
   return (IsSmallerThan(pos) || pos->IsSmallerThan(this));
}

//___________________________________________________________________________
Bool_t KVPosition::IsOverlappingWith(KVPosition* other)
{
   //kTRUE if there is at least partial overlap between two solid angle elements

   ROOT_GEO_DO_NOT_USE("IsOverlappingWith", kFALSE)
   return (
             //overlapping corners - case 1
             (IsInPolarRange(other->GetThetaMin())
              || IsInPolarRange(other->GetThetaMax()))
             && (IsInPhiRange(other->GetPhiMin())
                 || IsInPhiRange(other->GetPhiMax()))
          ) || (
             //overlapping corners - case 2
             (other->IsInPolarRange(GetThetaMin())
              || other->IsInPolarRange(GetThetaMax()))
             && (other->IsInPhiRange(GetPhiMin())
                 || other->IsInPhiRange(GetPhiMax()))
          ) || (
             //case where this is phi-contained by the other, but the other is theta-contained by this
             (other->IsInPolarRange(GetThetaMin())
              || other->IsInPolarRange(GetThetaMax()))
             && (IsInPhiRange(other->GetPhiMin())
                 || IsInPhiRange(other->GetPhiMax()))
          ) || (
             //case where this is phi-contained by the other, but the other is theta-contained by this
             (IsInPolarRange(other->GetThetaMin())
              || IsInPolarRange(other->GetThetaMax()))
             && (other->IsInPhiRange(GetPhiMin())
                 || other->IsInPhiRange(GetPhiMax()))
          );
}

//___________________________________________________________________________
Bool_t KVPosition::IsAzimuthallyWiderThan(KVPosition* pos)
{
   //kTRUE if "this" has larger azimuthal width than "pos".
   //Takes care of cases where the solid angle straddles 0 degrees

   ROOT_GEO_DO_NOT_USE("IsAzimuthallyWiderThan", kFALSE)
   if (GetAzimuthalWidth() > pos->GetAzimuthalWidth())
      return kTRUE;
   return kFALSE;
}

//___________________________________________________________________________
TVector3 KVPosition::GetDirection()
{
   //Returns a unit vector corresponding to the direction of fTheta, fPhi
   //i.e. the centre of the solid angle element.

   TVector3 tmp(1.0, 0.0, 0.0);
   tmp.SetTheta(fTheta * TMath::Pi() / 180.);
   tmp.SetPhi(fPhi * TMath::Pi() / 180.);
   return tmp;
}

//___________________________________________________________________________

void KVPosition::GetCornerCoordinates(TVector3* corners, Double_t depth)
{
   // Fill the array (TVector3 corner[4]) with the coordinates of the 4 'corners' of the solid angle element.
   //
   // These 'corners' are the points of intersection between the plane defined by the normal
   // to the centre of the solid angle (direction: theta,phi), at a distance fDistance [cm] from the
   // origin, and the four lines starting at the origin with directions (thetamin,phimin),
   // (thetamax,phimin), (thetamax,phimax), (thetamin,phimax).
   //
   // If optional argument 'depth' [cm] is given, the coordinates are calculated for the plane
   // situated at distance (fDistance+depth) from the origin.
   //
   // The order of the 4 corners is as follows:
   //       corners[3] : theta-min, phi-min
   //       corners[2] : theta-max, phi-min
   //       corners[1] : theta-max, phi-max
   //       corners[0] : theta-min, phi-max
   //
   // Coordinates are in CENTIMETRES

   ROOT_GEO_DO_NOT_USE("GetCornerCoordinates",)

   // calculate unit vector normal to solid angle
   Double_t pthR = GetTheta() * TMath::DegToRad();
   Double_t pphR = GetPhi() * TMath::DegToRad();
   TVector3 normal_to_plane(sin(pthR)*cos(pphR), sin(pthR)*sin(pphR), cos(pthR));

   // the four directions/lines
   corners[3].SetMagThetaPhi(1.0, TMath::DegToRad()*GetThetaMin(), TMath::DegToRad()*GetPhiMin());
   corners[2].SetMagThetaPhi(1.0, TMath::DegToRad()*GetThetaMax(), TMath::DegToRad()*GetPhiMin());
   corners[1].SetMagThetaPhi(1.0, TMath::DegToRad()*GetThetaMax(), TMath::DegToRad()*GetPhiMax());
   corners[0].SetMagThetaPhi(1.0, TMath::DegToRad()*GetThetaMin(), TMath::DegToRad()*GetPhiMax());

   // calculate intersection points
   for (int i = 0; i < 4; i++) {
      Double_t t = corners[i] * normal_to_plane;
      if (t <= 0.0) corners[i].SetXYZ(0, 0, 0);
      else corners[i] *= (fDistance + depth) / t;
   }
}

//___________________________________________________________________________

void KVPosition::GetCornerCoordinatesInOwnFrame(TVector3* corners, Double_t depth)
{
   // Like GetCornerCoordinates(), except that the coordinates correspond to a reference frame
   // in which the +ve z-axis goes through the centre of the solid angle

   ROOT_GEO_DO_NOT_USE("GetCornerCoordinatesInOwnFrame",)
   GetCornerCoordinates(corners, depth);
   TRotation rot_to_frame;
   rot_to_frame.SetYEulerAngles(-GetPhi()*TMath::DegToRad(), -GetTheta()*TMath::DegToRad(), 0.);
   TVector3 displZ(0, 0, fDistance + depth);
   for (int i = 0; i < 4; i++) {
      corners[i] = rot_to_frame * corners[i] - displZ;
   }
}

//___________________________________________________________________________
Double_t KVPosition::GetSolidAngle(void) const
{
   // Return values of the solid angle (in msr) seen by the geometric ensemble
   // For simple geometries defined by theta_min/max etc., this is exact.
   // For ROOT geometries we calculate the area of the entrance window and divide
   // it by the square of the distance to the detector.

   if (ROOTGeo()) {
      if (!fSolidAngle) {
         // TGeoArb8 shapes' solid angles calculated on demand as this requires
         // a monte carlo calculation
         // this takes into account any eventual "misalignment" i.e. if the vector from the
         // origin to the centre of the volume is not perpendicular to the entrance surface,
         // which reduces the effective area "seen" from the target
         Double_t area = GetSurfaceArea() * GetOC_SC_CosAngle();
         fSolidAngle = area / pow(GetDistance(), 2.) * 1.e3;
      }
      return fSolidAngle;
   }

   return (-1.*cos(GetThetaMax() * TMath::DegToRad()) +
           cos(GetThetaMin() * TMath::DegToRad()))
          * (GetAzimuthalWidth() * TMath::DegToRad()) * 1.e3;

}

Bool_t KVPosition::ROOTGeo() const
{
   // Returns kTRUE if ROOT geometry is used, kFALSE if not
   return (fMatrix && fShape);
}

//_________________________________________________________________________________________
Double_t KVPosition::GetAzimuthalWidth(Double_t phmin, Double_t phmax) const
{
   //Calculate azimuthal width taking phi-min as the most anticlockwise point of the
   //element and phi-max the most clockwise.
   //If no arguments are given, width calculated for this object
   //Otherwise, width calculated for given phi-min and phi-max

   ROOT_GEO_DO_NOT_USE("GetAzimuthalWidth", 0.)

   if (phmin == -1.)
      phmin = GetPhiMin();
   if (phmax == -1.)
      phmax = GetPhiMax();
   if (phmin < phmax)
      return (phmax - phmin);
   else
      return (phmax - phmin + 360.);
}

//__________________________________________________________________________________

void KVPosition::GetWidthsFromDimension(Double_t lin_dim)
{
   //Calculate azimuthal and polar widths for a square element placed at a
   //given distance from the origin with linear dimension 'lin_dim' (in mm).
   //SetDistance, SetTheta and SetPhi must already have been called.

   if (GetDistance() <= 0.0) {
      Error("GetWidthsFromDimension", "Distance not set");
      return;
   }
   Double_t d__th =
      2. * TMath::RadToDeg() * TMath::ATan(lin_dim /
                                           (2. * GetDistance()));
   Double_t d__ph =
      TMath::RadToDeg() * lin_dim / (GetDistance() * GetSinTheta());
   SetPolarWidth(d__th);
   SetAzimuthalWidth(d__ph);
}

TRotation KVPosition::GetRandomIsotropicRotation()
{
   // Generates a rotation which, if applied to a unit vector in the Z-direction,
   // will transform it into an isotropically-distributed vector in this
   // angular range.

   TRotation rr2;
   ROOT_GEO_DO_NOT_USE("GetRandomIsotropicRotation", rr2)
   Double_t a1min, a1max, a2min, a2max, a3min, a3max;
   a1min = 0;
   a1max = 2 * TMath::Pi();
   a2min = GetThetaMin() * TMath::DegToRad();
   a2max = GetThetaMax() * TMath::DegToRad();
   a3min = GetPhiMin() * TMath::DegToRad();
   a3max = GetPhiMax() * TMath::DegToRad();
   a3min += TMath::Pi() / 2.;
   a3max += TMath::Pi() / 2.;
   rr2.SetXEulerAngles(gRandom->Uniform(a1min, a1max),
                       TMath::ACos(gRandom->Uniform(cos(a2max), cos(a2min))),
                       gRandom->Uniform(a3min, a3max));
   return rr2;
}

/////////////////////////////////////////////////////////////////////////////

void KVPosition::SetMatrix(const TGeoHMatrix* m)
{
   // * ROOT Geometry *
   // Set the global transformation matrix for this volume
   // If shape has been set, we set the (theta,phi) angles
   // corresponding to the centre of the volume, and
   // the distance from the target corresponding to the distance
   // along (theta,phi) to the entrance surface of the volume
   // (not necessarily the same as the distance from the target to
   // the centre of the entrance surface)

   if (fMatrix) delete fMatrix;
   fMatrix = new TGeoHMatrix(*m);
   if (ROOTGeo()) {
      TVector3 centre = GetVolumeCentre();
      SetTheta(centre.Theta()*TMath::RadToDeg());
      SetPhi(centre.Phi()*TMath::RadToDeg());
      SetDistance(GetVolumeCentre().Mag() - GetSCVector().Mag() / GetOC_SC_CosAngle());
      // solid angle calculated and set here only for non-TGeoArb8 shapes
      // this takes into account any eventual "misalignment" i.e. if the vector from the
      // origin to the centre of the volume is not perpendicular to the entrance surface,
      // which reduces the effective area "seen" from the target
      if (!GetShape()->InheritsFrom("TGeoArb8")) {
         Double_t area = GetShape()->GetFacetArea(1) * GetOC_SC_CosAngle();
         fSolidAngle = area / pow(GetDistance(), 2.) * 1.e3;
      }
   }
}

void KVPosition::SetShape(TGeoBBox* b)
{
   // * ROOT Geometry *
   // Set the shape of this detector
   // If matrix has been set, we set the (theta,phi) angles
   // corresponding to the centre of the volume, and
   // the distance from the target corresponding to the distance
   // along (theta,phi) to the entrance surface of the volume
   // (not necessarily the same as the distance from the target to
   // the centre of the entrance surface)

   fShape = b;
   if (ROOTGeo()) {
      TVector3 centre = GetVolumeCentre();
      SetTheta(centre.Theta()*TMath::RadToDeg());
      SetPhi(centre.Phi()*TMath::RadToDeg());
      SetDistance(GetVolumeCentre().Mag() - GetSCVector().Mag() / GetOC_SC_CosAngle());
      // solid angle calculated and set here only for non-TGeoArb8 shapes
      // this takes into account any eventual "misalignment" i.e. if the vector from the
      // origin to the centre of the volume is not perpendicular to the entrance surface,
      // which reduces the effective area "seen" from the target
      if (!GetShape()->InheritsFrom("TGeoArb8")) {
         Double_t area = GetShape()->GetFacetArea(1) * GetOC_SC_CosAngle();
         fSolidAngle = area / pow(GetDistance(), 2.) * 1.e3;
      }
   }
}

TGeoHMatrix* KVPosition::GetMatrix() const
{
   // * ROOT Geometry *
   // Return global transformation matrix for this detector
   return fMatrix;
}

TGeoBBox* KVPosition::GetShape() const
{
   // * ROOT Geometry *
   // Return shape of this detector
   return fShape;
}

TVector3 KVPosition::GetRandomPointOnSurface() const
{
   // * ROOT Geometry *
   // Generate a vector in the world (laboratory) frame from the origin
   // to a random point on the entrance surface of this volume.
   //
   // It is assumed that the volume was defined in such a way
   // that the entrance window corresponds to the facet in the X-Y plane
   // placed at -dZ.
   //
   // NOTE: we force the use of TGeoBBox::GetPointsOnFacet.
   // For TGeoArb8, the method has been overridden and does nothing.
   // We use the TGeoBBox method, and then use TGeoShape::Contains
   // to check that the point does actually correspond to the TGeoArb8.

   if (!ROOTGeo()) {
      ::Error("KVPosition::GetRandomPointOnSurface",
              "ROOT Geometry has not been initialised");
      return TVector3();
   }
   Double_t master[3 * 50];
   Double_t points[3 * 50];
   const Double_t* origin = GetShape()->GetOrigin();
   Double_t dz = GetShape()->GetDZ();
   // This will generate a point on the (-DZ) face of the bounding box of the shape
   Bool_t ok1 = GetShape()->TGeoBBox::GetPointsOnFacet(1, 50, points);
   // We move the point slightly inside the volume to test if it actually corresponds
   // to a point on the shape's facet
   points[2] += dz / 100.;
   // Correct for offset of centre of shape
   for (int i = 0; i < 3; i++) points[i] += origin[i];
   Bool_t ok2 = GetShape()->Contains(points);
   if (!ok1) {
      ::Error("KVPosition::GetRandomPoint",
              "TGeoBBox::GetPointsOnFacet returns kFALSE for shape %s. Returning coordinates of centre.", GetShape()->ClassName());
      return GetSurfaceCentre();
   }
   Int_t np = 0;
   if (!ok2) {
      // try to find a point that works
      np++;
      while (np < 50) {
         Double_t* npoint = points + 3 * np;
         npoint[2] += dz / 100.;
         // Correct for offset of centre of shape
         for (int i = 0; i < 3; i++) npoint[i] += origin[i];
         ok2 = GetShape()->Contains(npoint);
         if (ok2) break;
         np++;
      }
      if (!ok2) {
         ::Error("KVPosition::GetRandomPointOnSurface",
                 "Cannot generate points for shape %s. Returning coordinates of surface centre.", GetShape()->ClassName());
         return GetSurfaceCentre();
      }
   }
   Double_t* npoint = points + 3 * np;
   npoint[2] -= dz / 100.;
   GetMatrix()->LocalToMaster(npoint, master);
   return TVector3(master);
}

TVector3 KVPosition::GetSurfaceCentre() const
{
   // * ROOT Geometry *
   // Generate a vector in the world (laboratory) frame from the origin
   // to the centre of the entrance surface of the volume.
   //
   // It is assumed that the volume was defined in such a way
   // that the entrance surface corresponds to the facet in the X-Y plane
   // placed at -dZ.

   if (!ROOTGeo()) {
      ::Error("KVPosition::GetSurfaceCentre",
              "ROOT Geometry has not been initialised");
      return TVector3();
   }
   Double_t master[3];
   const Double_t* origin = GetShape()->GetOrigin();
   Double_t points[] = {origin[0], origin[1], origin[2] - GetShape()->GetDZ()};
   GetMatrix()->LocalToMaster(points, master);
   return TVector3(master);
}

TVector3 KVPosition::GetVolumeCentre() const
{
   // * ROOT Geometry *
   // Generate a vector in the world (laboratory) frame from the origin
   // to the centre of the volume.

   if (!ROOTGeo()) {
      ::Error("KVPosition::GetVolumeCentre",
              "ROOT Geometry has not been initialised");
      return TVector3();
   }
   Double_t master[3];
   const Double_t* origin = GetShape()->GetOrigin();
   Double_t points[] = {origin[0], origin[1], origin[2]};
   GetMatrix()->LocalToMaster(points, master);
   return TVector3(master);
}

TVector3 KVPosition::GetSurfaceNormal() const
{
   // * ROOT Geometry *
   // Generate a vector in the world (laboratory) frame representing
   // the normal to the entrance surface of the volume (pointing away
   // from the target, i.e. towards the inside of the volume)
   //
   // It is assumed that the volume was defined in such a way
   // that the entrance surface corresponds to the facet in the X-Y plane
   // placed at -dZ.

   if (!ROOTGeo()) {
      ::Error("KVPosition::GetSurfaceNormal",
              "ROOT Geometry has not been initialised");
      return TVector3();
   }
   return GetSCVector().Unit();
}

Double_t KVPosition::GetSurfaceArea(int npoints) const
{
   // Monte Carlo calculation of entrance surface area for TGeoArb8 shapes
   // Area is calculated as area of bounding box facet multiplied by the ratio between
   // number of random points actually on the shape surface to the number of points
   // npoints generated over the surface of the bounding box facet
   //
   // npoints is the number of points to test

   Double_t* points = new Double_t[3 * npoints];
   const Double_t* origin = GetShape()->GetOrigin();
   Double_t dz = GetShape()->GetDZ();
   // This will generate npoints points on the (-DZ) face of the bounding box of the shape
   Bool_t ok1 = GetShape()->TGeoBBox::GetPointsOnFacet(1, npoints, points);
   if (!ok1) {
      ::Error("KVPosition::GetEntranceWindowArea",
              "TGeoBBox::GetPointsOnFacet returns kFALSE for shape %s. Returning 0.0", GetShape()->ClassName());
      delete [] points;
      return 0.0;
   }
   Double_t points_on_facet = 0;
   for (Int_t np = 0; np < npoints; ++np) {
      Double_t* npoint = points + 3 * np;
      // We move the point slightly inside the volume to test if it actually corresponds
      // to a point on the shape's facet
      npoint[2] += dz / 100.;
      // Correct for offset of centre of shape
      for (int i = 0; i < 3; i++) npoint[i] += origin[i];
      if (GetShape()->Contains(npoint)) ++points_on_facet;
   }
   delete [] points;
   return GetShape()->GetFacetArea(1) * (points_on_facet / Double_t(npoints));
}

Double_t KVPosition::GetMisalignmentAngle() const
{
   // Return angle (in deg.) between the vector from the target to the volume centre
   // and the normal to the volume surface
   return TMath::RadToDeg() * TMath::ACos(GetOC_SC_CosAngle());
}
