/***************************************************************************
                          kvparticle.h  -  description
                             -------------------
    begin                : Sun May 19 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVParticle.h,v 1.41 2008/05/21 13:19:56 ebonnet Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVPARTICLE_H
#define KVPARTICLE_H

#include "TVector3.h"
#include "TLorentzVector.h"
#include "KVBase.h"
#include "TRef.h"
#include "TMath.h"
#include "KVList.h"
#include "KVUniqueNameList.h"
#include "TObjString.h"
#include "KVNameValueList.h"
#include "KVFrameTransform.h"

class KVKinematicalFrame;
class KVParticleCondition;

/**
  \class KVParticle
  \ingroup NucEvents
  \brief Base class for relativistic kinematics of massive particles

Implements all kinematical manipulations necessary for obtaining angles, kinetic energies, transverse energies etc. etc.,
defining and changing relativistic reference frames.

Unless otherwise stated,
      - all energies are in MeV
      - all velocities are in cm/ns
      - all angles are in degrees (polar angles between 0 and \f$180^o\f$; azimuthal angles between 0 and \f$360^o\f$).
      - all momenta are in MeV/c

This class derives from TLorentzVector - a particle is basically a Lorentz 4-vector with added attributes - and therefore
any methods which are not documented here will be found in that class
\note most 'Set'ter methods of TLorentzVector should not be used directly and have been made 'private' in KVParticle in order to prevent their misuse

This also means that all particles and kinematics are relativistic in KaliVeda.

Methods defined/redefined in this class:

~~~~~~~~~~~~~~~~~~{.cpp}
      GetMass()       return mass of particle in MeV/c**2 (same as TLorentzVector::M())
      GetMomentum()   returns momentum 3-vector (TVector3) of particle (units MeV/c) (same as TLorentzVector::Vect())
      GetKE()/GetEnergy()     returns kinetic energy in MeV (same as TLorentzVector::E()-TLorentzVector::M())
      GetTransverseEnergy()/GetEtran()        see below
      GetRTransverseEnergy()/GetREtran()      see below
      GetVelocity()/GetV()    returns velocity 3-vector (TVector3) of particle (units cm/ns)
      GetVpar()       returns velocity component in beam (z) direction (cm/ns) (same as GetV().Z())
      GetVperp()      same as GetV().Perp(), but sign is same as y-component of velocity, GetV().Y()
      GetTheta()      same as TLorentzVector::Theta() but in degrees, not radians
      GetPhi()        same as TLorentzVector::Phi() but in degrees, not radians, and always positive, between 0 and TMath::TwoPi()
~~~~~~~~~~~~~~~~~~

Particle properties can be defined either using one of the constructors :

~~~~~~~~~~~~~~~~~~{.cpp}
      KVParticle(Double_t m, TVector3 & p);                              // rest mass and momentum 3-vector
      KVParticle(Double_t m, Double_t px, Double_t py, Double_t pz);     // rest mass and Cartesian components of momentum 3-vector
~~~~~~~~~~~~~~~~~~

or with the usual 'Set' methods:

~~~~~~~~~~~~~~~~~~{.cpp}
      SetMass(Double_t m);                                               // changes rest mass, leaves momentum unchanged
      SetEnergy(Double_t e);                                             // set kinetic energy (in MeV)
      SetMomentum(const TVector3 & v);                                   // changes momentum, leaves rest mass unchanged
      SetMomentum(const TVector3 * v);
~~~~~~~~~~~~~~~~~~

or, for the following method:

~~~~~~~~~~~~~~~~~~{.cpp}
      SetMomentum(Double_t px, Double_t py, Double_t pz, Option_t * opt ="cart");
~~~~~~~~~~~~~~~~~~

Set momentum 3-vector, leaving rest mass unchanged, with :
  - if `opt = "cart"` or `"cartesian"`  : using cartesian components `(px,py,pz)`
  - if `opt = "spher"` or `"spherical"` : set momentum 3-vector using spherical coordinates
      - in this case `px` = magnitude, `py` = theta [degrees], `pz` = phi [degrees].

 Other methods:

~~~~~~~~~~~~~~~~~~{.cpp}
      SetMomentum(Double_t T, TVector3 dir);  // set kinetic energy to T [MeV] and direction given by 'dir' unit vector

      SetRandomMomentum(Double_t T, Double_t thmin, Double_t thmax, Double_t phmin, Double_t phmax, Option_t * opt = "isotropic");
              // a handy tool for giving random momenta to particles.
~~~~~~~~~~~~~~~~~~

GetEtran() and GetTransverseEnergy() return the non-relativistic transverse energy defined as the kinetic energy multiplied by the squared sinus of the polar angle:
i.e. \f$E_{tran} = E\sin^2(\theta)\f$ where \f$\theta\f$ is the polar angle of the particle measured with respect to the beam (z-) axis.

GetREtran() and GetRTransverseEnergy() return the relativistic transverse energy which is frame invariant.

### Kinematical reference frames
#### 1. Accessing particle kinematics in different frames
Particle kinematics in different frames can be accessed with method InFrame().
This method does not modify the particle's kinematics or default reference frame (see 2 below).
If you want to access lots of information from this frame, it is probably more efficient to
define and store it in the particle's list of reference frames (see 3 below).

__Example:__ inspect kinematics of particle (accessed through pointer `KVParticle* p`)
 - in a frame moving at 5 cm/ns in the beam direction (velocities/boosts are described by TVector3 objects):

~~~~~~~~~~~~~~~~~~{.cpp}
            p->InFrame(TVector3(0,0,5)).GetVpar();
~~~~~~~~~~~~~~~~~~

 - in a frame rotated by \f$90^o\f$ clockwise around the +ve beam direction (rotations are described by TRotation objects):

~~~~~~~~~~~~~~~~~~{.cpp}
            TRotation rot;
            rot.RotateZ(TMath::PiOver2());
            p->InFrame(rot).GetPhi();
~~~~~~~~~~~~~~~~~~

 - in a frame moving at \f$0.1c\f$ in the beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
p->InFrame(KVFrameTransform(TVector3(0,0,0.1),kTRUE)).GetKE();

p->InFrame({{0,0,0.1},kTRUE}).GetKE(); // C++11 equivalent of above
~~~~~~~~~~~~~~~~~~

In this case we need to pass two arguments to the KVFrameTransform constructor:
before C++11, this required an explicit call to the constructor (top line); since C++11, uniform initialization
allows to just give the expected arguments, using `{}` to enclose the arguments for any aggregate (class) types.

#### 2. Modifying particle kinematics
Particle kinematics can be modified using method ChangeFrame():

__Example:__ change kinematics of particle (accessed through pointer `KVParticle* p`)
 - to a frame moving at 5 cm/ns in the beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
            p->ChangeFrame(TVector3(0,0,5));
~~~~~~~~~~~~~~~~~~

 - to a frame rotated by \f$90^o\f$ clockwise around the +ve beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
            TRotation rot;
            rot.RotateZ(TMath::PiOver2());
            p->ChangeFrame(rot);
~~~~~~~~~~~~~~~~~~

 - to a frame moving at \f$0.1c\f$ in the beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
 p->ChangeFrame(KVFrameTransform(TVector3(0,0,0.1),kTRUE));

p->ChangeFrame({{0,0,0.1},kTRUE});  // C++11 equivalent of above
~~~~~~~~~~~~~~~~~~

#### 3. Using several reference frames
Rather than changing the reference frame of the particle, you can define and use several
different reference frames while keeping the original kinematics as the default. Each
frame can be used independently, and new frames can be defined based on any of the
existing frames:

__Example:__ (for a particle accessed through pointer `KVParticle* p`{.cpp}):
 - define a new frame moving at 5 cm/ns in the beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
            p->SetFrame("moving_frame", TVector3(0,0,5));
~~~~~~~~~~~~~~~~~~

 - define a rotated coordinate frame in the "moving_frame", rotated by \f$90^o\f$ clockwise around the +ve beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
            TRotation rot;
            rot.RotateZ(TMath::PiOver2());
            p->SetFrame("rotated_moving_frame", "moving_frame", rot);
~~~~~~~~~~~~~~~~~~

  Note that the same frame can be defined directly from the original particle by using a combined boost-then-rotation transform:

~~~~~~~~~~~~~~~~~~{.cpp}
   p->SetFrame("rotated_moving_frame", KVFrameTransform(TVector3(0,0,5),rot));

   p->SetFrame("rotated_moving_frame", {{0,0,5},rot});  // C++11 equivalent of above
~~~~~~~~~~~~~~~~~~

 - define a similarly rotated coordinate frame in the original (default) reference frame:

~~~~~~~~~~~~~~~~~~{.cpp}
            p->SetFrame("rotated_frame", rot);
~~~~~~~~~~~~~~~~~~

 - access kinematical information in any of these frames:

~~~~~~~~~~~~~~~~~~{.cpp}
            p->GetFrame("moving_frame")->GetVpar();
            p->GetFrame("rotated_frame")->GetPhi();
            p->GetFrame("rotated_moving_frame")->GetTransverseEnergy();
~~~~~~~~~~~~~~~~~~

Note that the frame `"rotated_moving_frame"` is directly accessible even if it is defined in two
steps as a rotation of the `"moving_frame"`.

The total number of defined frames (including the default frame) is given by GetNumberOfDefinedFrames().
Calling Print() will show all reference frames defined for the particle:

~~~~~~~~~~~~~~~~~~{.cpp}
p->Print()

KVParticle mass=0 Theta=0 Phi=0 KE=0 Vpar=0
    moving_frame:  Theta=0 Phi=0 KE=0 Vpar=0
       rotated_moving_frame:  Theta=0 Phi=0 KE=0 Vpar=0
    rotated_frame:  Theta=0 Phi=0 KE=0 Vpar=0
~~~~~~~~~~~~~~~~~~

Indentation indicates the relationships between frames: `"rotated_moving_frame"` is a child frame of `"moving_frame"`.
The first line is the default kinematics, which by default has no name (a name can be set using SetFrameName()).

The current default kinematics can always be accessed using GetCurrentDefaultKinematics(): this method returns the default
kinematical frame for the particle when accessed from any kinematical frame:

~~~~{.cpp}
p->SetFrameName("lab");    // set name for default kinematics

auto show_default_frame = [](const KVParticle* p){ std::cout << p->GetCurrentDefaultKinematics()->GetFrameName() << std::endl; };

show_default_frame( p->GetFrame("moving_frame") )

"lab"
~~~~

#### 4. Changing the default kinematics
Let us consider a particle for which the different reference frames in the previous paragraph have been defined.
For an example, imagine that the default kinematics are that of particle with rest mass 939 MeV moving with
a momentum of 250 MeV/c at an angle of \f$45^o\f$ to the +ve z-direction:

~~~~~~~~~~~~~~~~~~{.cpp}
KVParticle p(939,0,0,250);
p.SetTheta(45);

p.SetFrame("moving_frame", TVector3(0,0,5));
TRotation rot;
rot.RotateZ(TMath::PiOver2());
p.SetFrame("rotated_moving_frame", "moving_frame", rot);
p.SetFrame("rotated_frame", rot);
~~~~~~~~~~~~~~~~~~

Now if we want to change the default kinematical frame for this particle:

~~~~~~~~~~~~~~~~~~{.cpp}
p.ChangeDefaultFrame("rotated_moving_frame", "lab"); // second argument is name for previous default frame, if not defined before

p.Print();

KVParticle mass=939 Theta=85.1751 Phi=270 KE=16.6117 Vpar=0.468125
         moving_frame:  Theta=85.1751 Phi=0 KE=16.6117 Vpar=0.468125
                 lab:  Theta=45 Phi=0 KE=32.7103 Vpar=5.45392
                         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392
KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x7f5a1ff8b1b8)
 <frameName=rotated_moving_frame>
~~~~~~~~~~~~~~~~~~

The relationships between frames are preserved, i.e. if we present the frames as graphs:

with "lab" as default frame:
~~~~
          lab
           |
           +--moving_frame
           |        |
           |        +--rotated_moving_frame
           |
           +--rotated_frame
~~~~
with "rotated_moving_frame" as default frame:
~~~~
   rotated_moving_frame
           |
           +--moving_frame
                    |
                    +--lab
                        |
                        +--rotated_frame
~~~~

#### 5. Updating stored kinematical frames
If you call SetFrame() several times with the same frame name
[note that frame names are **case insensitive**], the existing reference frame will
be updated to use the new transformation, which will be applied to the kinematics
of the particle in the 'parent' frame used to define the frame. Any frames which
were defined based on the frame will be updated too.

__Example:__ for the previous particle & frame definitions, after resetting the default kinematics:

~~~~~~~~~~~~~~~~~~{.cpp}
           p.ChangeDefaultFrame("lab");
~~~~~~~~~~~~~~~~~~

 - change the angle of rotation in the moving rotated frame:

~~~~~~~~~~~~~~~~~~{.cpp}
           rot.RotateZ(-TMath::PiOver4());
           p.SetFrame("rotated_moving_frame", rot);
           p.Print();

KVParticle mass=939 Theta=45 Phi=0 KE=32.7103 Vpar=5.45392
         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392
         moving_frame:  Theta=85.1751 Phi=0 KE=16.6117 Vpar=0.468125
                 rotated_moving_frame:  Theta=45 Phi=315 KE=32.7103 Vpar=5.45392
KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x7faae41241b8)
 <frameName=lab>
~~~~~~~~~~~~~~~~~~

 - change the velocity of the moving frame to \f$0.1c\f$:

~~~~~~~~~~~~~~~~~~{.cpp}
           p.SetFrame("moving_frame", KVFrameTransform(TVector3(0,0,0.1),kTRUE));

           // the following only works with C++11 and later
           p.SetFrame("moving_frame", {{0,0,0.1},kTRUE});

           p.Print();
KVParticle mass=939 Theta=45 Phi=0 KE=32.7103 Vpar=5.45392
         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392
         moving_frame:  Theta=65.6491 Phi=0 KE=19.8389 Vpar=2.50151
                 rotated_moving_frame:  Theta=65.6491 Phi=315 KE=19.8389 Vpar=2.50151
KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x7faae41241b8)
 <frameName=lab>
~~~~~~~~~~~~~~~~~~

Note that in this case, the "rotated_moving_frame" is updated automatically to take account of the new velocity of "moving_frame".

However, if you change the kinematics of the particle in its original (default) frame,
you have to update the kinematics in all defined frames by hand, it does not occur automatically:

~~~~~~~~~~~~~~~~~~{.cpp}
p.SetTheta(30);
p.Print();

KVParticle mass=939 Theta=30 Phi=0 KE=32.7103 Vpar=6.67966
         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392
         moving_frame:  Theta=65.6491 Phi=0 KE=19.8389 Vpar=2.50151
                 rotated_moving_frame:  Theta=65.6491 Phi=315 KE=19.8389 Vpar=2.50151
KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x7faae41241b8)
 <frameName=lab>

p.UpdateAllFrames();
p.Print();

KVParticle mass=939 Theta=30 Phi=0 KE=32.7103 Vpar=6.67966
         rotated_frame:  Theta=30 Phi=270 KE=32.7103 Vpar=6.67966
         moving_frame:  Theta=46.1843 Phi=0 KE=15.8459 Vpar=3.76564
                 rotated_moving_frame:  Theta=46.1843 Phi=315 KE=15.8459 Vpar=3.76564
KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x7faae41241b8)
 <frameName=lab>
~~~~~~~~~~~~~~~~~~

### Definition of groups
AddGroup() and BelongsToGroup() methods allow to sort particles in an event into subsets
based on various criteria such as "emitted by QP", "backwards emitted", or "include in calorimetry".
The group attribution can be done 'by hand' or using a KVParticleCondition object to define a selection.
The number of such groups for a given particle is unlimited.

Group names are case _insensitive_. Example:

~~~~~~~~~~~~~~{.cpp}
KVParticle part;
part.AddGroup("forward");
part.BelongsToGroup("ForWaRD");// -> return kTRUE
~~~~~~~~~~~~~~

Groups are valid for all kinematical frames, and can be defined using a pointer to the nucleus in any of
its defined kinematical frames:

~~~~~~~~~~~~~~~{.cpp}
part.SetFrame("CM", [some transformation]);
part.AddGroup("toto");
part.GetFrame("CM")->BelongsToGroup("toto");// returns kTRUE

part.GetFrame("CM")->AddGroup("titi");
part.BelongsToGroup("titi");// returns kRUE
~~~~~~~~~~~~~~~
 */
class KVParticle: public TLorentzVector {

   friend class KVKinematicalFrame;

private:
   void print_frames(TString fmt = "") const;
   KVKinematicalFrame* get_frame(const Char_t*) const;
   KVKinematicalFrame* get_parent_frame(const Char_t*, KVKinematicalFrame* F = nullptr) const;

   TString fName;                       //!non-persistent name field - Is useful
   TString fFrameName;                  //!non-persistent frame name field, sets when calling SetFrame method

   class FrameList : public KVList {
      KVParticle* parent;
   public:
      FrameList(KVParticle* p) : KVList(), parent(p) {}
      void Add(TObject*);
      TObject* Remove(TObject*);
      void Clear(Option_t* = "");
      void AddAll(const TCollection*);
   };

   mutable FrameList fBoosted{this};          //!list of momenta of the particle in different Lorentz-boosted frames
   mutable KVUniqueNameList fGroups;            //!list of TObjString for manage different group name
   static Double_t kSpeedOfLight;       //speed of light in cm/ns

   // TLorentzVector setters should not be used
   void SetVect(const TVector3& vect3)
   {
      TLorentzVector::SetVect(vect3);
   }
   void SetVectM(const TVector3& spatial, Double_t mass)
   {
      TLorentzVector::SetVectM(spatial, mass);
   }
   void SetVectMag(const TVector3& spatial, Double_t magnitude)
   {
      TLorentzVector::SetVectMag(spatial, magnitude);
   }
   void SetPerp(Double_t p)
   {
      TLorentzVector::SetPerp(p);
   }
   void SetPtEtaPhiE(Double_t pt, Double_t eta, Double_t phi, Double_t e)
   {
      TLorentzVector::SetPtEtaPhiE(pt, eta, phi, e);
   }
   void SetPtEtaPhiM(Double_t pt, Double_t eta, Double_t phi, Double_t m)
   {
      TLorentzVector::SetPtEtaPhiM(pt, eta, phi, m);
   }
   void SetPx(Double_t a)
   {
      TLorentzVector::SetPx(a);
   }
   void SetPy(Double_t a)
   {
      TLorentzVector::SetPy(a);
   }
   void SetPz(Double_t a)
   {
      TLorentzVector::SetPz(a);
   }
   void SetPxPyPzE(Double_t px, Double_t py, Double_t pz, Double_t e)
   {
      TLorentzVector::SetPxPyPzE(px, py, pz, e);
   }
   void SetRho(Double_t rho)
   {
      TLorentzVector::SetRho(rho);
   }
   void SetT(Double_t a)
   {
      TLorentzVector::SetT(a);
   }
   void SetX(Double_t a)
   {
      TLorentzVector::SetX(a);
   }
   void SetY(Double_t a)
   {
      TLorentzVector::SetY(a);
   }
   void SetZ(Double_t a)
   {
      TLorentzVector::SetZ(a);
   }
   void SetXYZM(Double_t x, Double_t y, Double_t z, Double_t m)
   {
      TLorentzVector::SetXYZM(x, y, z, m);
   }
   void SetXYZT(Double_t x, Double_t y, Double_t z, Double_t t)
   {
      TLorentzVector::SetXYZT(x, y, z, t);
   }

protected:

   TVector3* fE0;              //the momentum of the particle before it is slowed/stopped by an absorber
   KVNameValueList fParameters;//a general-purpose list of parameters associated with this particle

private:
   KVParticle* fParentFrame{nullptr};//! parent kinematical frame
   KVParticle* fOriginal{nullptr};//! if != nullptr, this object is a representation of the particle in a kinematical frame
   mutable Bool_t fFrameCopyOnly{kFALSE};//! used to inhibit full copying of particles in different kinematical frames

   Int_t _GetNumberOfDefinedFrames() const;
   void SetFrameCopyOnly() const
   {
      fFrameCopyOnly = kTRUE;
   }
   void ResetFrameCopyOnly() const
   {
      fFrameCopyOnly = kFALSE;
   }
   KVParticle* GetParentFrame() const
   {
      return fParentFrame;
   }
   void SetParentFrame(KVParticle* p)
   {
      fParentFrame = p;
   }
   const KVParticle* GetTopmostParentFrame() const
   {
      // \return pointer to current default kinematics for particle, i.e. the one at the top of the graph of kinematical frames
      // which has no parent frame

      auto top_node = this;
      while (top_node->GetParentFrame()) {
         top_node = top_node->GetParentFrame();
      }
      return top_node;
   }
   const KVParticle* GetOriginal() const
   {
      return fOriginal ? fOriginal : this;
   }
   KVParticle* GetOriginal()
   {
      return fOriginal ? fOriginal : this;
   }
   void SetOriginal(KVParticle* p)
   {
      fOriginal = p;
   }

protected:
   virtual void AddGroup_Withcondition(const Char_t*, KVParticleCondition*) const;
   virtual void AddGroup_Sanscondition(const Char_t* groupname, const Char_t* from = "") const;
   void SetGroups(KVUniqueNameList* un);
   void AddGroups(KVUniqueNameList* un);

public:

   enum {
      kIsOK = BIT(14),          //acceptation/rejection flag
      kIsOKSet = BIT(15),       //flag to indicate flag is set
      kIsDetected = BIT(16)     //flag set when particle is slowed by some KVMaterial
   };

   static Double_t C();

   KVParticle();
   KVParticle(Double_t m, TVector3& p);
   KVParticle(Double_t m, Double_t px, Double_t py, Double_t pz);
   KVParticle(const KVParticle&);
   virtual ~ KVParticle();
   void init();
   virtual void Copy(TObject&) const;
   virtual void Clear(Option_t* opt = "");

   virtual void SetMass(Double_t m)
   {
      SetXYZM(Px(), Py(), Pz(), m);
   };
   Double_t GetMass() const
   {
      return M();
   };
   void SetMomentum(const TVector3& v)
   {
      SetXYZM(v(0), v(1), v(2), M());
   };
   void SetMomentum(const TVector3* v)
   {
      SetXYZM((*v)(0), (*v)(1), (*v)(2), M());
   };
   void SetMomentum(Double_t px, Double_t py, Double_t pz, Option_t* opt =
                       "cart");
   void SetMomentum(Double_t T, TVector3 dir);
   void SetRandomMomentum(Double_t T, Double_t thmin, Double_t thmax,
                          Double_t phmin, Double_t phmax,
                          Option_t* opt = "isotropic");
   virtual void Print(Option_t* t = "") const;
   void Set4Mom(const TLorentzVector& p)
   {
      SetVect(p.Vect());
      SetT(p.E());
   }
   void SetE(Double_t a)
   {
      SetKE(a);
   };
   void SetKE(Double_t ecin);
   void SetEnergy(Double_t e)
   {
      SetKE(e);
   };
   void SetVelocity(const TVector3&);
   TVector3 GetMomentum() const
   {
      return Vect();
   };
   TVector3 GetTransverseMomentum() const
   {
      TVector3 perp = GetMomentum();
      perp.SetZ(0);
      return perp;
   }
   Double_t GetKE() const
   {
      Double_t e = E();
      Double_t m = M();
      //return (E() - M());
      return e - m;
   };
   Double_t GetEnergy() const
   {
      return GetKE();
   };
   Double_t GetTransverseEnergy() const
   {
      return GetKE() * TMath::Power(TMath::Sin(Theta()), 2.0);
   }
   Double_t GetEtran() const
   {
      return GetTransverseEnergy();
   }

   Double_t GetLongitudinalEnergy() const
   {
      return GetKE() - GetTransverseEnergy();
   }
   Double_t GetElong() const
   {
      return GetLongitudinalEnergy();
   }

   Double_t GetRTransverseEnergy() const
   {
      Double_t etran = Mt() - GetMass();
      return etran;
   }
   Double_t GetREtran() const
   {
      return GetRTransverseEnergy();
   }
   Double_t GetE() const
   {
      return GetKE();
   };
   Double_t GetWaveLength() const
   {
      //should be in fm
      if (GetMomentum().Mag() == 0)
         return 0;
      Double_t h = TMath::H() * TMath::C() / TMath::Qe() * 1e9; //in MeV.fm
      return h / GetMomentum().Mag();
   };
   Double_t GetThermalWaveLength(Double_t temp) const
   {
      //should be in fm
      Double_t h = TMath::H() * TMath::C() / TMath::Qe() * 1e9; //in MeV.fm
      return h / TMath::Sqrt(TMath::TwoPi() * temp * GetMass());
   };
   TVector3 GetVelocity() const;
   TVector3 GetV() const
   {
      return GetVelocity();
   };
   Double_t GetVpar() const
   {
      return GetV().z();
   };
   Double_t GetVperp() const;
   Double_t GetTheta() const
   {
      return TMath::RadToDeg() * Theta();
   };
   Double_t GetCosTheta() const
   {
      return TMath::Cos(Theta());
   }
   Double_t GetPhi() const
   {
      Double_t phi = TMath::RadToDeg() * Phi();
      return (phi < 0 ? 360. + phi : phi);
   };
   void SetTheta(Double_t theta)
   {
      TLorentzVector::SetTheta(TMath::DegToRad() * theta);
   }
   void SetPhi(Double_t phi)
   {
      TLorentzVector::SetPhi(TMath::DegToRad() * phi);
   }

   virtual Bool_t IsOK();
   void SetIsOK(Bool_t flag = kTRUE);
   void ResetIsOK()
   {
      GetOriginal()->ResetBit(kIsOKSet);
   }

   KVList* GetListOfFrames() const
   {
      return &fBoosted;
   }
   void ls(Option_t* option = "") const;

   void SetE0(TVector3* e = 0)
   {
      if (!fE0)
         fE0 = new TVector3;
      if (!e) {
         *fE0 = GetMomentum();
      }
      else {
         *fE0 = *e;
      }
   }
   TVector3* GetPInitial() const
   {
      return fE0;
   }
   void SetIsDetected()
   {
      GetOriginal()->SetBit(kIsDetected);
   }
   Bool_t IsDetected() const
   {
      return GetOriginal()->TestBit(kIsDetected);
   }
   KVParticle& operator=(const KVParticle& rhs);

   void ResetEnergy();

   const Char_t* GetName() const;
   void SetName(const Char_t* nom);

   void AddGroup(const Char_t* groupname, const Char_t* from = "") const;
   void AddGroup(const Char_t* groupname, KVParticleCondition*) const;

   Bool_t BelongsToGroup(const Char_t* groupname) const;
   void RemoveGroup(const Char_t* groupname);
   void RemoveAllGroups();
   void ListGroups() const;
   Int_t GetNumberOfDefinedGroups() const
   {
      //\return the number of defined groups for the particle
      return GetGroups()->GetEntries();
   }
   KVUniqueNameList* GetGroups() const
   {
      //\return the list of groups
      return (KVUniqueNameList*)&GetOriginal()->fGroups;
   }


   KVParticle InFrame(const KVFrameTransform&);
   void ChangeFrame(const KVFrameTransform&, const KVString& = "");
   void ChangeDefaultFrame(const Char_t*, const Char_t* defname = "");
   void SetFrame(const Char_t* frame, const KVFrameTransform&);
   void SetFrame(const Char_t* newframe, const Char_t* oldframe, const KVFrameTransform&);
   void UpdateAllFrames();

   const KVParticle* GetCurrentDefaultKinematics() const
   {
      // \return pointer to current default kinematics for particle
      return GetTopmostParentFrame();
   }
   Bool_t IsDefaultKinematics() const
   {
      // \returns kTRUE if this particle is in its default kinematic frame
      return GetTopmostParentFrame() == this;
   }

   Bool_t HasFrame(const Char_t* frame) const
   {
      // \param[in] frame name of a previously defined kinematical frame
      // \returns kTRUE if a kinematical frame called \a frame has been defined for this particle
      //
      // \note Frame names are case-insensitive
      //
      // \note We search the entire hierarchy of kinematical frames, starting from the current default kinematics

      return (GetTopmostParentFrame()->get_frame(frame) != nullptr);
   }
   Int_t GetNumberOfDefinedFrames() const
   {
      // \returns the total number of kinematic frames defined for this particle
      return GetTopmostParentFrame()->_GetNumberOfDefinedFrames();
   }
   KVParticle const* GetFrame(const Char_t* frame, Bool_t warn_and_return_null_if_unknown = kTRUE) const;

   const Char_t* GetFrameName(void) const
   {
      // \returns name of the current kinematical frame
      return fFrameName;
   }
   void SetFrameName(const Char_t* framename)
   {
      // Set the (non-persistent) name of the reference frame for this particle's kinematics.
      // Also sets a (persistent) parameter "frameName"

      fFrameName = framename;
      if (fFrameName != "") SetParameter("frameName", framename);
      else GetParameters()->RemoveParameter("frameName");
   }

   KVNameValueList* GetParameters() const
   {
      return (KVNameValueList*)&fParameters;
   }
   template<typename ValType> void SetParameter(const Char_t* name, ValType value) const
   {
      GetParameters()->SetValue(name, value);
   }

   ClassDef(KVParticle, 8)      //General base class for all massive particles
};

#endif
