//Created by KVClassFactory on Thu Jan 19 15:37:43 2017
//Author: John Frankland,,,

#ifndef __KVFRAMETRANSFORM_H
#define __KVFRAMETRANSFORM_H

#include "TLorentzRotation.h"

/**
\class KVFrameTransform
\brief Utility class for kinematical transformations of KVParticle class
\ingroup NucEvents

 This class is used as a generic argument for the KVEvent::SetFrame() and
 KVParticle::SetFrame() methods (amongst others).

It is not always necessary to explicitly use this class when calling these methods. Any class for which
 a single-argument KVFrameTransform constructor is defined can be used, even
 a temporary (unnamed) object:

  e.g. for KVParticle::SetFrame(const Char_t* frame, const KVFrameTransform& ft)
       the following calls are valid:

~~~~~~~~~~{.cpp}
          KVParticle p;
          TVector3 boost(0,0,3.0);// in cm/ns
          p.SetFrame("toto", boost);
          p.SetFrame("toto", TVector3(0,0,3.0)); // same effect as previous
          TRotation rot;
          rot.RotateZ(TMath::PiOver2()); // 90 deg. rotation around beam
          p.SetFrame("titi", rot);
~~~~~~~~~~

 On the other hand, when more than one argument to the KVFrameTransform constructor
 is required, before C++11 you had to explicitly call the constructor in question, although
 once again it is sufficient to use a temporary object in the method call:

~~~~~~~~~~{.cpp}
          p.SetFrame("tata", KVFrameTransform(TVector3(0,0,0.1),kTRUE));
           --> boost to frame moving at 0.1c in beam direction

          p.SetFrame("tutu", KVFrameTransform(rot, TVector3(0,0,0.1), kTRUE));
           --> rotation around beam axis followed by boost 0.1c
~~~~~~~~~~

 With C++11 this becomes
~~~~~~~~~~{.cpp}
          p.SetFrame("tata", {TVector3(0,0,0.1),kTRUE});
           --> boost to frame moving at 0.1c in beam direction

          p.SetFrame("tutu", {rot, TVector3(0,0,0.1), kTRUE});
           --> rotation around beam axis followed by boost 0.1c
~~~~~~~~~~

\note for boosts, default units are cm/ns
\note for rotations (TRotation), default units are radians

*/

class KVFrameTransform : public TLorentzRotation {
public:
   KVFrameTransform() {}
   KVFrameTransform(const TVector3& boost, Bool_t beta = kFALSE);
   KVFrameTransform(const TRotation& r);
   KVFrameTransform(const TLorentzRotation& r);
   KVFrameTransform(const KVFrameTransform& r);
   KVFrameTransform(const TVector3& boost, const TRotation& r, Bool_t beta = kFALSE);
   KVFrameTransform(const TRotation& r, const TVector3& boost, Bool_t beta = kFALSE);

   virtual ~KVFrameTransform() {}

   KVFrameTransform& operator=(const KVFrameTransform&);

   ClassDef(KVFrameTransform, 1) //Utility class for kinematical transformations of KVParticle class
};

#endif
