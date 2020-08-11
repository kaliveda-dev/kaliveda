/***************************************************************************
$Id: KVParticle.cpp,v 1.50 2009/04/28 08:59:05 franklan Exp $
                          kvparticle.cpp  -  description
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
#include "TMath.h"
#include "Riostream.h"
#include "KVPosition.h"
#include "KVParticle.h"
#include "KVList.h"
#include "TRotation.h"
#include "TLorentzRotation.h"
#include "TObjString.h"
#include "TClass.h"
#include "KVKinematicalFrame.h"

Double_t KVParticle::kSpeedOfLight = TMath::C() * 1.e-07;

using namespace std;

ClassImp(KVParticle)

////////////////////////////////////////////////////////////////////////////
/*
<h2>KVParticle</h2>
<h4>A general base class for relativistic kinematics of massive particles</h4>

Implements all kinematical manipulations necessary for obtaining angles, kinetic energies, transverse energies etc. etc.,
defining and changing relativistic reference frames.

Unless otherwise stated,
      - all energies are in MeV
      - all velocities are in cm/ns
      - all angles are in degrees (polar angles between 0 and \f$180^o\f$; azimuthal angles between 0 and \f$360^o\f$).
      - all momenta are in MeV/c

This class derives from [TLorentzVector](https://root.cern.ch/doc/master/classTLorentzVector.html) - a particle is basically a Lorentz 4-vector with added attributes - and therefore
any methods which are not documented here will be found in that class
  - [**N.B.** most 'Set'ter methods of TLorentzVector should not be used directly and have been made 'private' in KVParticle in order to prevent their use].

This also means that all particles and kinematics are relativistic in KaliVeda.

Methods defined/redefined in this class:

~~~~~~~~~~~~~~~~~~
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

~~~~~~~~~~~~~~~~~~
      KVParticle(Double_t m, TVector3 & p);                              // rest mass and momentum 3-vector
      KVParticle(Double_t m, Double_t px, Double_t py, Double_t pz);     // rest mass and Cartesian components of momentum 3-vector
~~~~~~~~~~~~~~~~~~

or with the usual 'Set' methods:

~~~~~~~~~~~~~~~~~~
      SetMass(Double_t m);                                               // changes rest mass, leaves momentum unchanged
      SetEnergy(Double_t e);                                             // set kinetic energy (in MeV)
      SetMomentum(const TVector3 & v);                                   // changes momentum, leaves rest mass unchanged
      SetMomentum(const TVector3 * v);
~~~~~~~~~~~~~~~~~~

or, for the following method:

~~~~~~~~~~~~~~~~~~
      SetMomentum(Double_t px, Double_t py, Double_t pz, Option_t * opt ="cart");
~~~~~~~~~~~~~~~~~~

Set momentum 3-vector, leaving rest mass unchanged, with :
  - if `opt = "cart"` or `"cartesian"`  : using cartesian components `(px,py,pz)`
  - if `opt = "spher"` or `"spherical"` : set momentum 3-vector using spherical coordinates
      - in this case `px` = magnitude, `py` = theta [degrees], `pz` = phi [degrees].

 Other methods:

~~~~~~~~~~~~~~~~~~
      SetMomentum(Double_t T, TVector3 dir);  // set kinetic energy to T [MeV] and direction given by 'dir' unit vector

      SetRandomMomentum(Double_t T, Double_t thmin, Double_t thmax, Double_t phmin, Double_t phmax, Option_t * opt = "isotropic");
              // a handy tool for giving random momenta to particles.
~~~~~~~~~~~~~~~~~~

GetEtran() and GetTransverseEnergy() return the non-relativistic transverse energy defined as the kinetic energy multiplied by the squared sinus of the polar angle:
i.e. \f$E_{tran} = E\sin^2(\theta)\f$ where \f$\theta\f$ is the polar angle of the particle measured with respect to the beam (z-) axis.

GetREtran() and GetRTransverseEnergy() return the relativistic transverse energy which is frame invariant.

## Kinematical reference frames
### 1. Accessing particle kinematics in different frames
Particle kinematics in different frames can be accessed with InFrame(const KVFrameTransform&).
This method does not modify the particle's kinematics or default reference frame (see 2 below).
If you want to access lots of information from this frame, it is probably more efficient to
define and store it in the particle's list of reference frames (see 3 below).

__Example:__ inspect kinematics of particle (accessed through pointer `KVParticle* p`)
 - in a frame moving at 5 cm/ns in the beam direction (velocities/boosts are described by [TVector3](https://root.cern.ch/doc/master/classTVector3.html) objects):

~~~~~~~~~~~~~~~~~~
            p->InFrame(TVector3(0,0,5)).GetVpar();
~~~~~~~~~~~~~~~~~~

 - in a frame rotated by \f$90^o\f$ clockwise around the +ve beam direction (rotations are described by [TRotation](https://root.cern.ch/doc/master/classTRotation.html) objects):

~~~~~~~~~~~~~~~~~~
            TRotation rot;
            rot.RotateZ(TMath::PiOver2());
            p->InFrame(rot).GetPhi();
~~~~~~~~~~~~~~~~~~

 - in a frame moving at \f$0.1c\f$ in the beam direction:

~~~~~~~~~~~~~~~~~~
            p->InFrame(KVFrameTransform(TVector3(0,0,0.1),kTRUE)).GetKE();

            // the following only works with C++11 and later
            p->InFrame({{0,0,0.1},kTRUE}).GetKE();
~~~~~~~~~~~~~~~~~~

In this case we need to pass two arguments to the KVFrameTransform(const TVector3&, Bool_t = kFALSE) constructor;
before C++11, this required an explicit call to the constructor.

### 2. Modifying particle kinematics
Particle kinematics can be modified using method ChangeFrame(const KVFrameTransform&):

__Example:__ change kinematics of particle (accessed through pointer `KVParticle* p`)
 - to a frame moving at 5 cm/ns in the beam direction:

~~~~~~~~~~~~~~~~~~
            p->ChangeFrame(TVector3(0,0,5));
~~~~~~~~~~~~~~~~~~

 - to a frame rotated by \f$90^o\f$ clockwise around the +ve beam direction:

~~~~~~~~~~~~~~~~~~
            TRotation rot;
            rot.RotateZ(TMath::PiOver2());
            p->ChangeFrame(rot);
~~~~~~~~~~~~~~~~~~

 - to a frame moving at \f$0.1c\f$ in the beam direction:

~~~~~~~~~~~~~~~~~~
            p->ChangeFrame(KVFrameTransform(TVector3(0,0,0.1),kTRUE));

            // the following only works with C++11 and later
            p->ChangeFrame({{0,0,0.1},kTRUE});
~~~~~~~~~~~~~~~~~~

### 3. Using several reference frames
Rather than changing the reference frame of the particle, you can define and use several
different reference frames while keeping the original kinematics as the default. Each
frame can be used independently, and new frames can be defined based on any of the
existing frames:

__Example:__ (for a particle accessed through pointer `KVParticle* p`):
 - define a new frame moving at 5 cm/ns in the beam direction:

~~~~~~~~~~~~~~~~~~
            p->SetFrame("moving_frame", TVector3(0,0,5));
~~~~~~~~~~~~~~~~~~

 - define a rotated coordinate frame in the "moving_frame", rotated by \f$90^o\f$ clockwise around the +ve beam direction:

~~~~~~~~~~~~~~~~~~
            TRotation rot;
            rot.RotateZ(TMath::PiOver2());
            p->SetFrame("rotated_moving_frame", "moving_frame", rot);
~~~~~~~~~~~~~~~~~~

  Note that the same frame can be defined directly from the original particle by using a combined boost-then-rotation transform:

~~~~~~~~~~~~~~~~~~
            p->SetFrame("rotated_moving_frame", KVFrameTransform(TVector3(0,0,5),rot));

            // the following only works with C++11 and later
            p->SetFrame("rotated_moving_frame", {{0,0,5},rot});
~~~~~~~~~~~~~~~~~~

 - define a similarly rotated coordinate frame in the original (default) reference frame:

~~~~~~~~~~~~~~~~~~
            p->SetFrame("rotated_frame", rot);
~~~~~~~~~~~~~~~~~~

 - access kinematical information in any of these frames:

~~~~~~~~~~~~~~~~~~
            p->GetFrame("moving_frame")->GetVpar();
            p->GetFrame("rotated_frame")->GetPhi();
            p->GetFrame("rotated_moving_frame")->GetTransverseEnergy();
~~~~~~~~~~~~~~~~~~

Note that the frame `"rotated_moving_frame"` is directly accessible even if it is defined in two
steps as a rotation of the `"moving_frame"`.

### 4. Changing the default kinematics
Let us consider a particle for which the different reference frames in the previous paragraph have been defined.
For an example, imagine that the default kinematics are that of particle with rest mass 939 MeV moving with
a momentum of 250 MeV/c at an angle of \f$45^o\f$ to the +ve z-direction:

~~~~~~~~~~~~~~~~~~
KVParticle p(939,0,0,250);
p.SetTheta(45);

p.SetFrame("moving_frame", TVector3(0,0,5));
TRotation rot;
rot.RotateZ(TMath::PiOver2());
p.SetFrame("rotated_moving_frame", "moving_frame", rot);
p.SetFrame("rotated_frame", rot);
~~~~~~~~~~~~~~~~~~

Calling Print() will show all reference frames defined for the particle:

~~~~~~~~~~~~~~~~~~
p.Print()

KVParticle mass=939 Theta=45 Phi=0 KE=32.7103 Vpar=5.45392
         moving_frame:  Theta=85.1751 Phi=0 KE=16.6117 Vpar=0.468125
                 rotated_moving_frame:  Theta=85.1751 Phi=270 KE=16.6117 Vpar=0.468125
         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392
~~~~~~~~~~~~~~~~~~

Indentation indicates the relationships between frames: `"rotated_moving_frame"` is a child frame of `"moving_frame"`.
The first line is the default kinematics. As yet it has no name:

~~~~~~~~~~~~~~~~~~
p.SetFrameName("lab");
~~~~~~~~~~~~~~~~~~

Now if we want to change the default kinematical frame for this particle:

~~~~~~~~~~~~~~~~~~
p.ChangeDefaultFrame("rotated_moving_frame");

p.Print();

KVParticle mass=939 Theta=85.1751 Phi=270 KE=16.6117 Vpar=0.468125
         moving_frame:  Theta=85.1751 Phi=0 KE=16.6117 Vpar=0.468125
                 lab:  Theta=45 Phi=0 KE=32.7103 Vpar=5.45392
                         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392
KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x7f5a1ff8b1b8)
 <frameName=rotated_moving_frame>
~~~~~~~~~~~~~~~~~~

Note that the name of the default kinematics is stored as a parameter `"frameName"` and can be retrieved with GetFrameName().
Note also how the relationships between frames are preserved, i.e. if we present the frames as graphs:

~~~~~~~~~~~~~~~~~~
with "lab" as default frame:

          lab
           |
           +--moving_frame
           |        |
           |        +--rotated_moving_frame
           |
           +--rotated_frame

with "rotated_moving_frame" as default frame:

   rotated_moving_frame
           |
           +--moving_frame
                    |
                    +--lab
                        |
                        +--rotated_frame
~~~~~~~~~~~~~~~~~~


### 5. Updating stored kinematical frames
If you call SetFrame() several times with the same frame name
[note that frame names are **case insensitive**], the existing reference frame will
be updated to use the new transformation, which will be applied to the kinematics
of the particle in the 'parent' frame used to define the frame. Any frames which
were defined based on the frame will be updated too.

__Example:__ for the previous particle & frame definitions, after resetting the default kinematics:

~~~~~~~~~~~~~~~~~~
           p.ChangeDefaultFrame("lab");
~~~~~~~~~~~~~~~~~~

 - change the angle of rotation in the moving rotated frame:

~~~~~~~~~~~~~~~~~~
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

~~~~~~~~~~~~~~~~~~
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

~~~~~~~~~~~~~~~~~~
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

## Definition of groups
AddGroup() and BelongsToGroup() methods allow to sort particles in an event into subsets
based on various criteria such as "emitted by QP", "backwards emitted", or "include in calorimetry".
The group attribution can be done 'by hand' or using a KVParticleCondition object to define a selection.
The number of such groups for a given particle is unlimited.

Group names are case _insensitive_. Example:

~~~~~~~~~~~~~~
KVNucleus nn;
nn.AddGroup("forward");
nn.BelongsToGroup("ForWaRD");// -> return kTRUE
~~~~~~~~~~~~~~

 Any groups added to the particle are propagated to all existing kinematical frames. In other words, if a particle has a kinematical frame `"CM"` defined when it is
 added to a group:

~~~~~~~~~~~~~~~
part.SetFrame("CM", [some transformation]);
part.AddGroup("toto");
part.BelongsToGroup("toto");// this returns kTRUE
part.GetFrame("CM")->BelongsToGroup("toto");// this returns kTRUE also
~~~~~~~~~~~~~~~

On the contrary if some change is made in the following way :

~~~~~~~~~~~~~~~
part.GetFrame("CM")->AddGroup("titi");
part.GetFrame("CM")->BelongsToGroup("titi");// this returns kTRUE
part.BelongsToGroup("titi");// this returns kFALSE
~~~~~~~~~~~~~~~

*/
///////////////////////////////////////////////////////////////////////////

KVParticle::KVParticle() : fParameters("ParticleParameters", "Parameters associated with a particle in an event")
{
   init();
}

//_________________________________________________________
void KVParticle::init()
{
   //default initialisation
   fE0 = 0;
   SetFrameName("");
   fGroups.SetOwner(kTRUE);
}

//_________________________________________________________
KVParticle::KVParticle(const KVParticle& obj) : TLorentzVector()
{
   //copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVParticle&) obj).Copy(*this);
#endif
}

//________________________________________________________
KVParticle::KVParticle(Double_t m, TVector3& p) : fParameters("ParticleParameters", "Parameters associated with a particle in an event")
{
   //create particle with given mass and momentum vector
   init();
   SetMass(m);
   SetMomentum(p);
}

//________________________________________________________
KVParticle::KVParticle(Double_t m, Double_t px, Double_t py, Double_t pz) : fParameters("ParticleParameters", "Parameters associated with a particle in an event")
{
   //create particle with given mass and momentum vector
   init();
   SetMass(m);
   SetMomentum(px, py, pz);
}

//________________________________________________________
KVParticle::~KVParticle()
{
   //Info("~KVParticle","%p",this);
   Clear();
}

//________________________________________________________
Double_t KVParticle::C()
{
   //Static function.
   //Returns speed of light in cm/ns units.
   return kSpeedOfLight;
}

//________________________________________________________
void KVParticle::SetRandomMomentum(Double_t T, Double_t thmin,
                                   Double_t thmax, Double_t phmin,
                                   Double_t phmax, Option_t* opt)
{
   //Give randomly directed momentum to particle with kinetic energy T
   //Direction will be between (thmin,thmax) [degrees] limits in polar angle,
   //and (phmin,phmax) [degrees] limits in azimuthal angle.
   //
   //If opt = "" or "isotropic" (default) : direction is isotropically distributed over the solid angle
   //If opt = "random"                    : direction is randomly distributed over solid angle
   //
   //Based on KVPosition::GetRandomDirection().

   Double_t p = (T + M()) * (T + M()) - M2();
   if (p > 0.)
      p = (TMath::Sqrt(p));     // calculate momentum
   else
      p = 0.;

   TVector3 dir;
   KVPosition pos(thmin, thmax, phmin, phmax);
   dir = pos.GetRandomDirection(opt);   // get isotropic unit vector dir
   if (p && dir.Mag())
      dir.SetMag(p);            // set magnitude of vector to momentum required
   SetMomentum(dir);            // set momentum 4-vector
}

//________________________________________________________
void KVParticle::SetMomentum(Double_t T, TVector3 dir)
{
   //set momentum with kinetic energy t and unit direction vector d
   //(d is normalised first in case it is not a unit vector)

   Double_t p = (T + M()) * (T + M()) - M2();
   TVector3 pdir;
   TVector3 unit_dir = dir.Unit();
   if (p > 0.) {
      p = (TMath::Sqrt(p));
      pdir = p * unit_dir;
   }
   SetMomentum(pdir);
};

//________________________________________________________________________________________
void KVParticle::print_frames(TString fmt) const
{
   // recursive print out of all defined kinematical frames

   fmt += "\t";
   if (fBoosted.GetEntries()) {
      TIter next(&fBoosted);
      KVKinematicalFrame* frame;
      while ((frame = (KVKinematicalFrame*)next())) {
         cout << fmt << " " << frame->GetName() << ": ";
         KVParticle* part = frame->GetParticle();
         cout << " Theta=" << part->GetTheta() << " Phi=" << part->GetPhi()
              << " KE=" << part->GetKE() << " Vpar=" << part->GetVpar() << endl;
         part->print_frames(fmt);
      }
   }
}

void KVParticle::Print(Option_t*) const
{
   // print out characteristics of particle

   cout << "KVParticle mass=" << M() <<
        " Theta=" << GetTheta() << " Phi=" << GetPhi()
        << " KE=" << GetKE() << " Vpar=" << GetVpar() << endl;
   print_frames();
   GetParameters()->Print();
}

//_________________________________________________________________________________________
void KVParticle::SetKE(Double_t ecin)
{
   //Change particle KE, keeping momentum direction constant
   //If momentum is zero (i.e. no direction defined) the particle will be given
   //a velocity in the positive z-direction

   if (ecin > 0.) {
      Double_t et = M() + ecin; // new total energy = KE + m
      Double_t pmod = 0;
      if (et * et > M2()) {
         pmod = TMath::Sqrt(et * et - M2());    // p**2 = E**2 - m**2
         TVector3 newp(Px(), Py(), Pz());
         if (pmod && newp.Mag()) {
            newp.SetMag(pmod);
         }
         else {
            newp.SetXYZ(0, 0, 1);
            newp.SetMag(pmod);
         }
         SetMomentum(newp);
      }
      else {
         SetMomentum(0., 0., 0.);
      }
   }
   else
      SetMomentum(0., 0., 0.);
}

//______________________________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVParticle::Copy(TObject& obj) const
#else
void KVParticle::Copy(TObject& obj)
#endif
{
   // Copy this to obj
   // Particle kinematics are copied using operator=(const KVParticle&)
   // List of particle's groups is copied
   // The particle's name is copied
   // The list of parameters associated with the particle is copied

   ((KVParticle&) obj) = *this;
   ((KVParticle&) obj).SetGroups(this->GetGroups());
   ((KVParticle&) obj).SetName(this->GetName());
   fParameters.Copy(((KVParticle&) obj).fParameters);
}


//______________________________________________________________________________________
void KVParticle::Clear(Option_t*)
{
   //Reset particle properties i.e. before creating/reading a new event

   SetXYZM(0., 0., 0., 0.);
   if (fE0) {
      delete fE0;
      fE0 = 0;
   }
   ResetIsOK();                 //in case IsOK() status was set "by hand" in previous event
   ResetBit(kIsDetected);
   fParameters.Clear();
   fGroups.Clear();
   fBoosted.Delete();
}

//_________________________________________________________________________________________________________
Bool_t KVParticle::IsOK()
{
   //Determine whether this particle is considered "good" or not for analysis,
   //depending on a previous call to SetIsOK(Bool_t flag).
   //If SetIsOK has not been called, IsOK returns kTRUE by default.

   if (TestBit(kIsOKSet)) {     //status set by SetIsOK()
      return TestBit(kIsOK);
   }
   //Default if SetIsOK not used: accept particle
   return kTRUE;
}

//________________________________________________________________________________________________________

void KVParticle::SetIsOK(Bool_t flag)
{
   //Set acceptation/rejection status for the particle.
   //
   //In order to 'forget' this status (accept all particles) use ResetIsOK()

   SetBit(kIsOK, flag);
   SetBit(kIsOKSet);
}

void KVParticle::ls(Option_t* option) const
{
   std::cout << option << GetName() << ":" << GetFrameName() << ":" << this << "\n";
   if (fBoosted.GetEntries()) {
      TString nopt = option;
      nopt += "   ";
      TIter next(&fBoosted);
      KVKinematicalFrame* p;
      while ((p = (KVKinematicalFrame*)next())) p->GetParticle()->ls(nopt.Data());
   }
}

//________________________________________________________________________________________________________

KVParticle& KVParticle::operator=(const KVParticle& rhs)
{
   //KVParticle assignment operator.

   TLorentzVector::operator=((TLorentzVector&) rhs);
   if (rhs.GetPInitial()) SetE0(rhs.GetPInitial());
   return *this;
}

//________________________________________________________________________________________________________

void KVParticle::ResetEnergy()
{
   //Used for simulated particles after passage through some absorber/detector.
   //The passage of a particle through the different absorbers modifies its
   //kinetic energies, indeed the particle may be stopped in the detector.
   //Calling this method will reset the particle's momentum to its
   //initial value i.e. before it entered the first absorber.
   //Particles which have not encountered any absorbers/detectors are left as they are.

   if (IsDetected())
      SetMomentum(GetPInitial());
}

//___________________________________________________________________________//

void KVParticle::SetName(const Char_t* nom)
{
   //Set Name of the particle
   fName.Form("%s", nom);

}

//___________________________________________________________________________//
const Char_t* KVParticle::GetName() const
{
   // return the field fName
   return fName.Data();
}

//___________________________________________________________________________//
void KVParticle::AddGroup_Withcondition(const Char_t*, KVParticleCondition*)
{
   // Dummy implementation of AddGroup(const Char_t* groupname, KVParticleCondition*)
   // Does nothing. Real implementation is in KVNucleus::AddGroup_Withcondition.
   Warning("AddGroup_Withcondition", "DUUUUUUUUUUUUUMYYYYYYY do nothing");
};

//___________________________________________________________________________//
void KVParticle::AddGroup_Sanscondition(const Char_t* groupname, const Char_t* from)
{
   // Implementation of AddGroup_Sansconditioncon(st Char_t*, const Char_t*)
   // Can be overridden in child classes [instead of AddGroup(const Char_t*, const Char_t*),
   // which cannot]
   // if this method is overridde in child class
   // the line
   //         if (!fGroups) CreateGroups();
   // has to be included
   // The group will be added to all 'particles' representing different kinematical frames
   // for this particle

   TString sfrom(from);
   sfrom.ToUpper();
   TString sgroupname(groupname);
   sgroupname.ToUpper();

   if (BelongsToGroup(sfrom.Data()) && !BelongsToGroup(sgroupname.Data())) {
      fGroups.Add(new TObjString(sgroupname.Data()));
      if (fBoosted.GetEntries()) {
         // recursively add to all boosted particles
         TIter it(&fBoosted);
         KVKinematicalFrame* f;
         while ((f = (KVKinematicalFrame*)it())) {
            f->GetParticle()->AddGroup(sgroupname);
         }
      }
   }
}

//___________________________________________________________________________//
void KVParticle::AddGroup(const Char_t* groupname, const Char_t* from)
{
   // Associate this particle with the given named group.
   // Optional argument "from" allows to put a condition on the already stored
   // group list, is set to "" by default

   AddGroup_Sanscondition(groupname, from);
}


//___________________________________________________________________________//

void KVParticle::AddGroup(const Char_t* groupname, KVParticleCondition* cond)
{
   //define and store a group name from a condition on the particle
   //
   // Apply the method to all particles stored in fBoosted
   // SetParticleClassName has to be set before using this method if you use
   // in the KVParticleCondistion a specific method of a derived KVNucleus class

   AddGroup_Withcondition(groupname, cond);
}

//___________________________________________________________________________//
void KVParticle::SetGroups(KVUniqueNameList* un)
{
   //Define for the particle a new list of groups
   //if there is an existing list, it's deleted
   fGroups.Clear();
   AddGroups(un);
}

//___________________________________________________________________________//
void KVParticle::AddGroups(KVUniqueNameList* un)
{
   //list of groups added to the current one
   TObjString* os = 0;
   TIter no(un);
   while ((os = (TObjString*)no.Next())) {
      AddGroup(os->GetName());
   }

}

Int_t KVParticle::GetNumberOfDefinedFrames()
{
   // Returns the total number of defined kinematical frames for this particle.
   // This includes all frames which are defined with respect to other frames
   // (i.e. we count recursively the contents of all fBoosted lists)

   if (!fBoosted.GetEntries()) return 0;
   TIter it(&fBoosted);
   KVKinematicalFrame* p;
   Int_t nf = 0;
   while ((p = (KVKinematicalFrame*)it())) {
      nf += (1 + p->GetParticle()->GetNumberOfDefinedFrames());
   }
   return nf;
}
//___________________________________________________________________________//
Int_t KVParticle::GetNumberOfDefinedGroups(void)
{
   //return the number of defined groups for the particle
   return fGroups.GetEntries();
}

//___________________________________________________________________________//
KVUniqueNameList* KVParticle::GetGroups() const
{
   //return the KVUniqueNameList pointeur where list of groups are stored
   return (KVUniqueNameList*)&fGroups;
}

//___________________________________________________________________________//
Bool_t KVParticle::BelongsToGroup(const Char_t* groupname) const
{
   //Check if particle belong to a given group
   //return kTRUE if groupname="".
   //return kFALSE if no group has be defined

   TString sgroupname(groupname);
   sgroupname.ToUpper();
   //Important for KVEvent::GetNextParticle()
   if (sgroupname.IsNull()) return kTRUE;
   //retourne kFALSE si aucun groupe n'est defini
   if (!fGroups.GetEntries()) return kFALSE;
   if (fGroups.FindObject(sgroupname.Data())) return kTRUE;
   return kFALSE;
}

//___________________________________________________________________________//
void KVParticle::RemoveGroup(const Char_t* groupname)
{
   // Remove group from list of groups
   // Apply the method to all particles stored in fBoosted
   if (!fGroups.GetEntries()) return;
   TString sgroupname(groupname);
   sgroupname.ToUpper();

   TObjString* os = 0;
   if ((os = (TObjString*)fGroups.FindObject(sgroupname.Data()))) {
      delete fGroups.Remove(os);
      if (fBoosted.GetEntries()) {
         TIter it(&fBoosted);
         KVKinematicalFrame* f;
         while ((f = (KVKinematicalFrame*)it())) {
            f->GetParticle()->RemoveGroup(sgroupname);
         }
      }
   }
}

//___________________________________________________________________________//
void KVParticle::RemoveAllGroups()
{
   //Remove all groups
   // Apply the method to all particles stored in fBoosted
   fGroups.Clear();
   if (fBoosted.GetEntries()) {
      TIter it(&fBoosted);
      KVKinematicalFrame* f;
      while ((f = (KVKinematicalFrame*)it())) {
         f->GetParticle()->RemoveAllGroups();
      }
   }
}

//___________________________________________________________________________//

void KVParticle::ListGroups(void) const
{
   //List all stored groups
   if (!fGroups.GetEntries()) {
      cout << "Cette particle n appartient a aucun groupe" << endl;
      return;
   }
   else {
      cout << "--------------------------------------------------" << endl;
      cout << "Liste des groupes auxquels la particule appartient" << endl;
   }
   TObjString* os = 0;
   TIter no(GetGroups());
   while ((os = (TObjString*)no.Next())) cout << os->GetName() << endl;
   cout << "--------------------------------------------------" << endl;
}

KVParticle KVParticle::InFrame(const KVFrameTransform& t)
{
   // Use this method to obtain 'on-the-fly' some information on particle kinematics
   // in a different reference frame. The default kinematics of the particle are unchanged.

   KVParticle p;
   p.Set4Mom(*this);
   KVKinematicalFrame(&p, t);
   return p;
}

void KVParticle::ChangeFrame(const KVFrameTransform& t, const KVString& name)
{
   // Permanently modify kinematics of particle according to the given transformation.
   // You can optionally set the name of this new default kinematics.
   // NB the current kinematics will be lost. If you want to keep it after changing the
   // default kinematics, define the new frame with SetFrame and then use ChangeDefaultFrame.

   KVKinematicalFrame(this, t);
   if (name != "") SetFrameName(name);
}

void KVParticle::ChangeDefaultFrame(const Char_t* newdef, const Char_t* defname)
{
   // Make existing reference frame 'newdef' the new default frame for particle kinematics.
   // The current default frame will then be accessible from the list of frames
   // using its name (if set with SetFrameName). You can change/set the name of the previous
   // default frame with 'defname'

   TString _defname(defname);
   if (_defname == "") _defname = GetFrameName();
   // get list of all parents of new default
   TList parents;
   TString ff = newdef;
   KVKinematicalFrame* f;
   do {
      f = get_parent_frame(ff);
      if (f) {
         parents.Add(f);
         ff = f->GetName();
      }
   }
   while (f);
   // modify tree structure
   TIter it(&parents);
   KVKinematicalFrame* newdframe = get_frame(newdef);
   KVKinematicalFrame* save_newdef = newdframe;
   KVFrameTransform trans, next_trans = newdframe->GetTransform();
   while ((f = (KVKinematicalFrame*)it())) {
      f->GetParticle()->GetListOfFrames()->Remove(newdframe);
      trans = next_trans;
      next_trans = f->GetTransform();
      newdframe->GetParticle()->GetListOfFrames()->Add(f);
      f->SetTransform(trans.Inverse());
      newdframe = f;
   }
   GetListOfFrames()->Remove(newdframe);
   // copy current default kinematics particle momentum/energy
   TLorentzVector old_def_p(*this);
   // set momentum/energy of new default kinematics
   Set4Mom(*(save_newdef->GetParticle()));
   save_newdef->GetParticle()->Set4Mom(old_def_p);
   save_newdef->SetTransform(next_trans.Inverse());
   save_newdef->SetName(_defname);
   newdframe->GetParticle()->GetListOfFrames()->Add(save_newdef);
   // copy frame list from new default frame
   TList frame_list;
   frame_list.AddAll(save_newdef->GetParticle()->GetListOfFrames());
   save_newdef->GetParticle()->GetListOfFrames()->Clear("nodelete");
   save_newdef->GetParticle()->GetListOfFrames()->AddAll(&fBoosted);
   fBoosted.Clear("nodelete");
   fBoosted.AddAll(&frame_list);
   SetFrameName(newdef);
}

void KVParticle::SetFrame(const Char_t* frame, const KVFrameTransform& ft)
{
   //Define a Lorentz-boosted and/or rotated frame in which to calculate this particle's momentum and energy.
   //
   //The new frame will have the name given in the string "frame", which can then be used to
   //access the kinematics of the particle in different frames using GetFrame() (frame names are case-insensitive).
   //Calling this method with the name of an existing frame will update the kinematics of the particle
   //in that frame using the given transform (note that kinematics in all frames defined as 'subframes' of
   //this frame will also be updated).
   //
   //USING BOOSTS
   //The boost velocity vector is that of the boosted frame with respect to the original frame of the particles in the event.
   //The velocity vector can be given either in cm/ns units (default) or in units of 'c' (beta=kTRUE).
   //
   //E.g. to define a frame moving at 0.1c in the +ve z-direction with respect to the original
   //event frame:
   //
   //      (...supposing a valid pointer KVParticle* my_part...)
   //      TVector3 vframe(0,0,0.1);
   //      my_part->SetFrame("my_frame", KVFrameTransform(vframe, kTRUE));
   //
   //or with velocity in cm/ns units (default):
   //      TVector3 vframe(0,0,3);
   //      my_part->SetFrame("my_frame", KVFrameTransform(vframe));
   // OR   my_part->SetFrame("my_frame", vframe);
   //
   //USING ROTATIONS
   //According to the conventions adopted for the TRotation and TLorentzRotation classes,
   //we actually use the inverse of the TLorentzRotation to make the transformation,
   //to get the coordinates corresponding to a rotated coordinate system, not the coordinates
   //of a rotated vector in the same coordinate system
   //=> you do not need to invert the transformation matrix
   //
   //E.g. if you want to define a new frame whose coordinate axes are rotated with respect
   //to the original axes, you can set up a TRotation like so:
   //
   //      TRotation rot;
   //      TVector3 newX, newY, newZ; // the new coordinate axes
   //      rot.RotateAxes(newX, newY, newZ);
   //
   //If you are using one of the two global variables which calculate the event tensor
   //(KVTensP and KVTensPCM) you can obtain the transformation to the tensor frame
   //using:
   //
   //      TRotation rot;
   //      KVTensP* tens_gv;// pointer to tensor global variable
   //      tens_gv->GetTensor()->GetRotation(rot);// see KVTenseur3::GetRotation
   //
   //Then the new frame can be defined by
   //      my_part->SetFrame("my_frame", KVFrameTransform(rot));
   //  OR  my_part->SetFrame("my_frame", rot);
   //
   //USING COMBINED BOOST AND ROTATION
   //You can define a frame using both a boost and a rotation like so:
   //      my_part->SetFrame("my_frame", KVFrameTransform(vframe,rot,kTRUE));
   //
   //ACCESSING KINEMATICS IN NEW FRAMES
   //In order to access the kinematics of the particle in the new frame:
   //
   //      my_part->GetFrame("my_frame")->GetTransverseEnergy();// transverse energy in "my_frame"

   if (!strcmp(frame, "")) return;

   KVKinematicalFrame* tmp = get_frame(frame);
   if (!tmp) {
      //if this frame has not already been defined, create a new one
      tmp = new KVKinematicalFrame(frame, this, ft);
      fBoosted.Add(tmp);
   }
   else
      tmp->ApplyTransform(this, ft);
}

//___________________________________________________________________________//
KVParticle const* KVParticle::GetFrame(const Char_t* frame, Bool_t warn_and_return_null_if_unknown) const
{
   // Return the momentum of the particle in the Lorentz-boosted frame corresponding to the name
   // "frame" given as argument (see SetFrame() for definition of different frames).
   // If the default frame name has been set (see KVEvent::SetFrameName) and 'frame' is the
   // name of this default frame (KVParticle::fFrameName), we return the address of the particle
   // itself.
   //
   // This frame may have been defined by a direct transformation of the original kinematics of the
   // particle (using SetFrame(newframe,...)) or by a transformation of the kinematics in another
   // user-defined frame (using SetFrame(newframe,oldframe,...)).
   //
   // Note that frames are not "dynamic": if any changes are made to the original particle's kinematics
   // after definition of a frame, if you want these changes to affect also the other frames you
   // need to update them by hand by calling KVParticle::UpdateAllFrames().
   //
   // Frame names are case insensitive: "CM" or "cm" or "Cm" are all good...
   //
   // By default, if no frame with the given name is found, we return nullptr and print a warning.
   // If `warn_and_return_null_if_unknown=kFALSE`, we return the address of the particle itself,
   // i.e. the original/default kinematics.
   // [Note that this is an inversion of the previous default behaviour]
   //
   // Note that the properties of the particle returned by this method can not be modified:
   //    this is deliberate, as any modifications e.g. to kinematics will have no effect
   //    in any other frames.
   //
   // The returned pointer corresponds to a "pseudoparticle" in the desired frame,
   // therefore you can use any KVParticle method in order to access the kinematics of the
   // particle in the boosted frame, e.g.
   //
   //~~~~~~~~~~~~~~~~~~~~
   //      (...supposing a valid pointer KVParticle* my_part...)
   //      my_part->GetFrame("cm_frame")->GetVpar();// //el velocity in "cm_frame"
   //      my_part->GetFrame("QP_frame")->GetTheta();// polar angle in "QP_frame"
   //      etc. etc.
   //~~~~~~~~~~~~~~~~~~~~
   //

   if (!fFrameName.CompareTo(frame, TString::kIgnoreCase)) return (KVParticle const*)this;
   KVKinematicalFrame* f = get_frame(frame);
   return f ? (KVParticle const*)f->GetParticle() :
          (warn_and_return_null_if_unknown ?
           Warning("GetFrame(const Char_t*)", "No frame \"%s\" defined for particle. 0x0 returned.",
#ifndef WITH_CPP11
                   frame), (KVParticle*)nullptr
#else
                   frame), nullptr
#endif
           : this);
}

void KVParticle::UpdateAllFrames()
{
   // Call this method to update particle kinematics in all defined frames if you change
   // the kinematics of the particle in its original/default frame.

   if (fBoosted.GetEntries()) {
      TIter it(&fBoosted);
      KVKinematicalFrame* f;
      while ((f = (KVKinematicalFrame*)it())) {
         f->ReapplyTransform(this);
         // recursively apply to all subframes
         f->GetParticle()->UpdateAllFrames();
      }
   }
}

KVKinematicalFrame* KVParticle::get_frame(const Char_t* frame) const
{
   // PRIVATE method for internal use only
   // This allows to modify the returned frame, i.e. in order to define
   // new frames in SetFrame(newframe,oldframe,...)

   if (!fBoosted.GetEntries() || !strcmp(frame, "")) {
      // no frames defined or no frame name given
      return nullptr;
   }
   TString _frame(frame);
   TIter it(&fBoosted);
   KVKinematicalFrame* p(nullptr), *f(nullptr);
   while ((p = f = (KVKinematicalFrame*)it())) {
      if (!_frame.CompareTo(p->GetName(), TString::kIgnoreCase)) break;
      // look for subframe
      if ((f = p->GetParticle()->get_frame(_frame))) break;
   }
   return f;
}

KVKinematicalFrame* KVParticle::get_parent_frame(const Char_t* f, KVKinematicalFrame* F) const
{
   // PRIVATE method for internal use only
   // Returns pointer to parent frame of 'f'
   if (!fBoosted.GetEntries() || !strcmp(f, "")) {
      // no frames defined or no frame name given
      return nullptr;
   }
   TString _frame(f);
   TIter it(&fBoosted);
   KVKinematicalFrame* p(nullptr), *r(nullptr);
   while ((p = (KVKinematicalFrame*)it())) {
      if (!_frame.CompareTo(p->GetName(), TString::kIgnoreCase)) return F;
      // look for subframe
      if ((r = p->GetParticle()->get_parent_frame(_frame, p))) return r;
   }
   return nullptr;
}


//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* newframe, const Char_t* oldframe, const KVFrameTransform& ft)
{
   // Define new kinematical frame by transformation from existing frame
   // See SetFrame(const Char_t*,const KVFrameTransform&) for details on
   // defining kinematically-transformed frames.

   KVKinematicalFrame* f = get_frame(oldframe);
   if (!f) {
      Error("SetFrame(newframe,oldframe)", "oldframe=%s does not exist!", oldframe);
      return;
   }
   f->GetParticle()->SetFrame(newframe, ft);
}

//___________________________________________________________________________//

TVector3 KVParticle::GetVelocity() const
{
   //returns velocity vector in cm/ns units
   TVector3 beta;
   if (E()) {
      beta = GetMomentum() * (1. / E());
   }
   else {
      beta.SetXYZ(0, 0, 0);
   }
   return (kSpeedOfLight * beta);
}

//___________________________________________________________________________//

Double_t KVParticle::GetVperp() const
{
   //returns transverse velocity in cm/ns units
   //sign is +ve if py>=0, -ve if py<0
   return (GetV().y() >= 0.0 ? GetV().Perp() : -(GetV().Perp()));
}

//___________________________________________________________________________//

void KVParticle::SetMomentum(Double_t px, Double_t py, Double_t pz,
                             Option_t* opt)
{
   // Set Momentum components (in MeV/c)
   // if option is "cart" or "cartesian" we give cartesian components (x,y,z)
   // if option is "spher" or "spherical" we give components (rho,theta,phi) in spherical system
   //   with theta, phi in DEGREES
   if (!strcmp("spher", opt) || !strcmp("spherical", opt)) {
      TVector3 pvec(0., 0., 1.);
      pvec.SetMag(px);
      pvec.SetTheta(TMath::Pi() * py / 180.);
      pvec.SetPhi(TMath::Pi() * pz / 180.);
      SetVectM(pvec, M());
   }
   else {
      if (strcmp("cart", opt) && strcmp("cartesian", opt)) {
         Warning("SetMomentum(Double_t,Double_t,Double_t,Option_t*)",
                 "Unkown coordinate system\n known system are :\n\t\"cartesian\" or \"cart\" (default)\n\t\"spherical\" or \"spher\"");
         Warning("SetMomentum(Double_t,Double_t,Double_t,Option_t*)",
                 "default used.");
      }
      TVector3 pvec(px, py, pz);
      SetVectM(pvec, M());
   }
}

void KVParticle::SetVelocity(const TVector3& vel)
{
   // Set velocity of particle (in cm/ns units)
   Double_t gamma = 1. / kSpeedOfLight / sqrt(1 - (vel.Mag2() / pow(kSpeedOfLight, 2)));
   TVector3 p = GetMass() * gamma * vel;
   SetMomentum(p);
}

void KVParticle::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVParticle.
   // When reading: If parameter "frameName" is set, use it to set non-persistent
   // fFrameName member (used by GetFrame)

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(KVParticle::Class(), this);
      if (GetParameters()->HasStringParameter("frameName")) fFrameName = GetParameters()->GetStringValue("frameName");
   }
   else {
      R__b.WriteClassBuffer(KVParticle::Class(), this);
   }
}
