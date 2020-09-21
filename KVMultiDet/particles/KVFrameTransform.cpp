//Created by KVClassFactory on Thu Jan 19 15:37:43 2017
//Author: John Frankland,,,

#include "KVFrameTransform.h"
#include "KVParticle.h"

ClassImp(KVFrameTransform)


KVFrameTransform::KVFrameTransform(const TVector3& boost, Bool_t beta)
   : TLorentzRotation()
{
   // Construct frame transformation using velocity boost vector
   //   if beta=kTRUE, velocity given in light speed units
   //   if beta=kFALSE [default], velocity given in cm/ns units.
   if (beta) {
      Boost(boost);
   }
   else {
      Boost(boost.X() / KVParticle::C(), boost.Y() / KVParticle::C(),
            boost.Z() / KVParticle::C());
   }
}

//____________________________________________________________________________//

KVFrameTransform::KVFrameTransform(const TRotation& arg1)
   : TLorentzRotation(arg1)
{
   // Construct frame transformation using rotation of axes
}

//____________________________________________________________________________//

KVFrameTransform::KVFrameTransform(const TLorentzRotation& r)
   : TLorentzRotation(r)
{
   // Construct frame transformation using a TLorentzRotation
}

KVFrameTransform::KVFrameTransform(const KVFrameTransform& r)
   : TLorentzRotation((const TLorentzRotation&)r)
{
   // Copy constructor
}

//____________________________________________________________________________//

KVFrameTransform::KVFrameTransform(const TVector3& boost, const TRotation& r, Bool_t beta)
   : TLorentzRotation()
{
   // Construct frame transformation using rotation & velocity boost vector
   // in the order: BOOST then ROTATION
   //   if beta=kTRUE, velocity given in light speed units
   //   if beta=kFALSE [default], velocity given in cm/ns units.
   if (beta) {
      Boost(boost);
   }
   else {
      Boost(boost.X() / KVParticle::C(), boost.Y() / KVParticle::C(),
            boost.Z() / KVParticle::C());
   }
   Transform(r);
}

KVFrameTransform::KVFrameTransform(const TRotation& r, const TVector3& boost, Bool_t beta)
   : TLorentzRotation(r)
{
   // Construct frame transformation using rotation & velocity boost vector
   // in the order: ROTATION then BOOST
   //   if beta=kTRUE, velocity given in light speed units
   //   if beta=kFALSE [default], velocity given in cm/ns units.
   if (beta) {
      Boost(boost);
   }
   else {
      Boost(boost.X() / KVParticle::C(), boost.Y() / KVParticle::C(),
            boost.Z() / KVParticle::C());
   }
}

KVFrameTransform& KVFrameTransform::operator=(const KVFrameTransform& p)
{
   // assignment by copy

   return dynamic_cast<KVFrameTransform&>(TLorentzRotation::operator=(p));
}
