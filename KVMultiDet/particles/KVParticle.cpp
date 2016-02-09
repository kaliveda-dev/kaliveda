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

Double_t KVParticle::kSpeedOfLight = TMath::C() * 1.e-07;

using namespace std;

ClassImp(KVParticle);

////////////////////////////////////////////////////////////////////////////
//KVParticle
//
//A general base class for massive particles.
//Implements all kinematical manipulations necessary for obtaining angles, kinetic energies, transverse energies etc. etc.
//
//Unless otherwise stated,
//      - all energies are in MeV
//      - all velocities are in cm/ns
//      - all angles are in degrees (polar angles between 0 and 180 degrees; azimuthal angles between 0 and 360 degrees).
//      - all momenta are in MeV/c
//
//This class derives from TLorentzVector - a particle is basically a Lorentz 4-vector with added attributes - and therefore any methods which are not
//documented here will be found in that class. This also means that all particles and kinematics are relativistic in KaliVeda.
//
//Methods defined/redefined in this class:
//
//      GetMass()       return mass of particle in MeV/c**2 (same as TLorentzVector::M())
//      GetMomentum()   returns momentum 3-vector (TVector3) of particle (units MeV/c) (same as TLorentzVector::Vect())
//      GetKE()/GetEnergy()     returns kinetic energy in MeV (same as TLorentzVector::E()-TLorentzVector::M())
//      GetTransverseEnergy()/GetEtran()        see below
//      GetRTransverseEnergy()/GetREtran()      see below
//      GetVelocity()/GetV()    returns velocity 3-vector (TVector3) of particle (units cm/ns)
//      GetVpar()       returns velocity component in beam (z) direction (cm/ns) (same as GetV().Z())
//      GetVperp()      same as GetV().Perp(), but sign is same as y-component of velocity, GetV().Y()
//      GetTheta()      same as TLorentzVector::Theta() but in degrees, not radians
//      GetPhi()        same as TLorentzVector::Phi() but in degrees, not radians, and always positive, between 0 and TMath::TwoPi()
//
//Particle properties can be defined either using one of the constructors :
//
//      KVParticle part(Double_t m, TVector3 & p);                              // rest mass and momentum 3-vector
//      KVParticle part(Double_t m, Double_t px, Double_t py, Double_t pz);     // rest mass and Cartesian components of momentum 3-vector
//
//or with the usual 'Set' methods:
//
//      part.SetMass(Double_t m);                                               // changes rest mass, leaves momentum unchanged
//
//      part.SetMomentum(const TVector3 & v);                                   // changes momentum, leaves rest mass unchanged
//      part.SetMomentum(const TVector3 * v);
//
//      part.SetMomentum(Double_t px, Double_t py, Double_t pz, Option_t * opt ="cart");
//              // set momentum 3-vector, leaving rest mass unchanged, with :
//              // if opt = "cart" or "cartesian" : using cartesian components (px,py,pz)
//              // if opt = "spher" or "spherical" : set momentum 3-vector using spherical coordinates,
//              //                                   in this case px = magnitude, py = theta [degrees], pz = phi [degrees].
//
//      part.SetMomentum(Double_t T, TVector3 dir);                             // set kinetic energy to T [MeV] and direction given by 'dir' unit vector
//
//      part.SetRandomMomentum(Double_t T, Double_t thmin, Double_t thmax, Double_t phmin, Double_t phmax, Option_t * opt = "isotropic");
//              // a handy tool for giving random momenta to particles.
//
//"Transverse energy"
//---------------------
//      GetEtran()/GetTransverseEnergy() return the "transverse energy" defined as the kinetic energy multiplied by the squared sinus of the polar angle:
//              Etran = KE * sin(theta) * sin(theta)
//      where 'theta' is the polar angle of the particle measured with respect to the beam (z-) axis
//
//      GetREtran()/GetRTransverseEnergy() return the "relativistic transverse energy" which is frame invariant
//
//"Definition of groups"
//---------------------
//      AddGroup() and BelongsToGroup() methods allow to store and access to information about particles
//      such as belonging to the QP, to the backward of events or as to be taken into account in the
//      calorimetry
//      For KVNucleus and derived classes group can be defined using KVParticleCondition.
//      For the same particle, number of groups is unlimited.
//      All these groups are stored in the fGroups pointeur (KVUniqueNameList of TObjString objects)
//
//      The name of the frame which particle as been created via the SetFrame() method is now stored
//      in the non persistent field fFrameName
//      Two WARNINGS :
//            - SetName and GetName Methods now are related to the fName field which is the name of the particle
//            - All group names are not case sensitive
//             Ex : KVNucleus nn; nn.AddGroup("forward"); nn.BelongsToGroup("ForWaRD") -> return kTRUE
//      When new KVParticle is defined using SetFrame() Method, the list of group names is already stored in it
//      In the same way, when some change is made on the "principal" KVParticle, if some "secondary" particles
//      have been already stored in fBoosted list, the change is also apply
//      On the contrary if some change is made in the following way :
//      par->GetFrame(framename).AddGroup(groupname)
//      the group groupname is only stored for "par->GetFrame(framename) particle"
//      ie :   par->BelongsToGroup(groupname) = kFALSE
//        and  par->GetFrame(framename).BelongsToGroup(groupname) = kTRUE
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
void KVParticle::Print(Option_t*) const
{
   // print out characteristics of particle

   cout << "KVParticle mass=" << M() <<
        " Theta=" << GetTheta() << " Phi=" << GetPhi()
        << " KE=" << GetKE() << endl;
   if (fBoosted.GetEntries()) {
      TIter next(&fBoosted);
      KVParticle* part;
      while ((part = (KVParticle*)next())) {
         cout << "\t " << part->GetFrameName() << ": " <<
              " Theta=" << part->GetTheta() << " Phi=" << part-> GetPhi()
              << " KE=" << part->GetKE() << endl;
      }
   }
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
         } else {
            newp.SetXYZ(0, 0, 1);
            newp.SetMag(pmod);
         }
         SetMomentum(newp);
      } else {
         SetMomentum(0., 0., 0.);
      }
   } else
      SetMomentum(0., 0., 0.);
}

//______________________________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVParticle::Copy(TObject& obj) const
#else
void KVParticle::Copy(TObject& obj)
#endif
{
   //Copy this to obj
   //l'operateur d assignation est celui du TLorentzVector
   //avec un rajout pour le champs fE0
   //La KVList des KVParticle deduite de la methode SetFrame
   //n est pas copiee
   //Info("Copy","je rentre");
   TLorentzVector::Copy(obj);
   ((KVParticle&) obj) = *this;
   ((KVParticle&) obj).SetGroups(this->GetGroups());
   ((KVParticle&) obj).SetName(this->GetName());
   fParameters.Copy(((KVParticle&) obj).fParameters);
   //((KVParticle &) obj).SetFrameName(GetFrameName());
   //Info("Copy","je sort");
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

   TString sfrom(from);
   sfrom.ToUpper();
   TString sgroupname(groupname);
   sgroupname.ToUpper();

   if (BelongsToGroup(sfrom.Data()) && !BelongsToGroup(sgroupname.Data())) {
      fGroups.Add(new TObjString(sgroupname.Data()));
      if (fBoosted.GetEntries()) {
         TString inst;
         inst.Form("\"%s\"", sgroupname.Data());
         fBoosted.Execute("AddGroup", inst.Data());
      }
   }
}

//___________________________________________________________________________//
void KVParticle::AddGroup(const Char_t* groupname, const Char_t* from)
{
   // Associate this particle with the given named group.
   // Optional argument "from" allows to put a condition on the already stored
   // group list, is set to "" by default
   //
   // Apply the method to all particles stored in fBoosted
   //Info("AddGroup","%s",groupname);
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
         TString inst;
         inst.Form("\"%s\"", sgroupname.Data());
         fBoosted.Execute("RemoveGroup", inst.Data());
      }
   }
}

//___________________________________________________________________________//
void KVParticle::RemoveAllGroups()
{
   //Remove all groups
   // Apply the method to all particles stored in fBoosted
   fGroups.Clear();
   if (fBoosted.GetEntries()) fBoosted.Execute("RemoveAllGroups", "");
}

//___________________________________________________________________________//

void KVParticle::ListGroups(void) const
{
   //List all stored groups
   if (!fGroups.GetEntries()) {
      cout << "Cette particle n appartient a aucun groupe" << endl;
      return;
   } else {
      cout << "--------------------------------------------------" << endl;
      cout << "Liste des groupes auxquels la particule appartient" << endl;
   }
   TObjString* os = 0;
   TIter no(GetGroups());
   while ((os = (TObjString*)no.Next())) cout << os->GetName() << endl;
   cout << "--------------------------------------------------" << endl;
}

//___________________________________________________________________________//
KVParticle* KVParticle::GetFrame(const Char_t* frame)
{
   //Return the momentum of the particle in the Lorentz-boosted frame corresponding to the name
   //"frame" given as argument (see SetFrame() for definition of different frames).
   //
   //The returned pointer corresponds to a "pseudoparticle" in the desired frame,
   //therefore you can use any KVParticle method in order to access the kinematics of the
   //particle in the boosted frame, e.g.
   //
   //      (...supposing a valid pointer KVParticle* my_part...)
   //      my_part->GetFrame("cm_frame")->GetVpar();// //el velocity in "cm_frame"
   //      my_part->GetFrame("QP_frame")->GetTheta();// polar angle in "QP_frame"
   //      etc. etc.
   //
   //If frame="", or if the frame is unknown, this just returns a pointer to the particle
   //i.e. you will access the particle's "default" frame, which usually corresponds to the
   //'laboratory' or 'detector' frame.

   if (!fBoosted.GetEntries() || !strcmp(frame, ""))
      return this;

   KVParticle* f = (KVParticle*) fBoosted.FindObjectWithMethod(frame, "GetFrameName");
   if (!f) {
      //Warning("GetFrame","Frame %s does not defined for this particle",frame);
      return this;
   } else {
      return f;
   }
}

//___________________________________________________________________________//
Bool_t KVParticle::HasFrame(const Char_t* frame)
{
   //Check if a given frame has been defined

   if (!fBoosted.GetEntries() || !strcmp(frame, "")) {
      return kFALSE;
   }
   KVParticle* f = (KVParticle*) fBoosted.FindObjectWithMethod(frame, "GetFrameName");

   return (f ? kTRUE : kFALSE);
}

//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* frame, const TVector3& boost,
                          Bool_t beta)
{
   //Define a Lorentz-boosted frame in which to calculate the particle's momentum and energy.
   //
   //The new frame will have the name given in the string "frame", which can then be used to
   //access the kinematics of the particle in different frames using GetFrame().
   //
   //The boost velocity vector is that of the boosted frame with respect to the original frame of the particles in the event.
   //The velocity vector can be given either in cm/ns units (default) or in units of 'c' (beta=kTRUE).
   //
   //E.g. to define a frame moving at 0.1c in the +ve z-direction with respect to the original
   //event frame:
   //
   //      (...supposing a valid pointer KVParticle* my_part...)
   //      TVector3 vframe(0,0,0.1);
   //      my_part->SetFrame("my_frame", vframe, kTRUE);
   //
   //In order to access the kinematics of the particle in the new frame:
   //
   //      my_part->GetFrame("my_frame")->GetTransverseEnergy();// transverse energy in "my_frame"

   //set up TLorentzRotation corresponding to boosted frame
   TLorentzRotation tmp;
   if (beta) {
      tmp.Boost(boost);
   } else {
      tmp.Boost(boost.X() / kSpeedOfLight, boost.Y() / kSpeedOfLight,
                boost.Z() / kSpeedOfLight);
   }
   SetFrame(frame, tmp);
}

//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* frame, const TLorentzRotation& rot)
{
   //Define a Lorentz-rotated frame in which to calculate the particle's momentum.
   //
   //The new frame will have the name given in the string "frame", which can then be used to
   //access the kinematics of the particle in different frames using GetFrame().
   //
   //According to the conventions adopted for the TRotation and TLorentzRotation classes,
   //we actually use the inverse of the TLorentzRotation to make the transformation,
   //i.e. to get the coordinates corresponding to a rotated coordinate system, not the coordinates
   //of a rotated vector in the same coordinate system.
   //
   //The transformed coordinates of the particle are calculated using
   //      v *= rot.Inverse();
   //where v = particle's original momentum 4-vector and rot = TLorentzRotation

   if (!strcmp(frame, "")) return;

   KVParticle* tmp = 0;
   if (HasFrame(frame)) {
      tmp = GetFrame(frame);
   } else {
      //if this frame has not already been defined, create a new particle
      tmp = (KVParticle*)this->IsA()->New();
      tmp->SetFrameName(frame);
      fBoosted.Add(tmp);
   }

   //copy all information on particle
   this->Copy(*tmp);
   //transform to boosted frame
   (*tmp) *= rot.Inverse();
}

//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* frame, const TRotation& rot)
{
   //Define a rotated frame in which to calculate the particle's momentum and energy.
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
   //N.B. you do not need to invert the transformation matrix (cf. TRotation), this is handled by
   //SetFrame(const Char_t* frame, TLorentzRotation& rot)
   //
   //The new frame will have the name given in the string "frame", which can then be used to
   //access the kinematics of the particle in different frames using GetFrame().
   //
   //In order to access the kinematics of the particle in the new frame:
   //
   //      my_part->GetFrame("my_frame")->GetTransverseEnergy();// transverse energy in "my_frame"

   //set up corresponding TLorentzRotation
   TLorentzRotation tmp(rot);
   SetFrame(frame, tmp);
}

//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* frame, const TVector3& boost,
                          TRotation& rot, Bool_t beta)
{
   //Define a Lorentz-boosted and rotated frame in which to calculate the particle's momentum and energy.
   //
   //The new frame will have the name given in the string "frame", which can then be used to
   //access the kinematics of the particle in different frames using GetFrame().
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
   //N.B. you do not need to invert the transformation matrix (cf. TRotation), this is handled by
   //SetFrame(const Char_t* frame, TLorentzRotation& rot)
   //
   //The boost velocity vector is that of the boosted frame with respect to the original frame of the particles in the event.
   //The velocity vector can be given either in cm/ns units (default) or in units of 'c' (beta=kTRUE).
   //
   //E.g. to define a frame moving at 0.1c in the +ve z-direction with respect to the original
   //event frame:
   //
   //      (...supposing a valid pointer KVParticle* my_part...)
   //      TVector3 vframe(0,0,0.1);
   //      my_part->SetFrame("my_frame", vframe, kTRUE);
   //
   //In order to access the kinematics of the particle in the new frame:
   //
   //      my_part->GetFrame("my_frame")->GetTransverseEnergy();// transverse energy in "my_frame"

   //set up corresponding TLorentzRotation
   TLorentzRotation tmp(rot);
   if (beta) {
      tmp.Boost(boost);
   } else {
      tmp.Boost(boost.X() / kSpeedOfLight, boost.Y() / kSpeedOfLight,
                boost.Z() / kSpeedOfLight);
   }
   SetFrame(frame, tmp);
}

//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* newframe, const Char_t* oldframe,
                          const TVector3& boost, Bool_t beta)
{
   GetFrame(oldframe)->SetFrame(newframe, boost, beta);
   //Duplicate the transformed particle to access by the method GetFrame("newframe")
   //without the chain : particle->GetFrame("oldframe")->GetFrame("newframe");
   //Specially usefull if the new frame is used in KVVarGlob derived classes
   KVParticle* tmp = (KVParticle*)this->IsA()->New();
   tmp->SetBit(kCanDelete);
   tmp->SetFrameName(newframe);
   GetFrame(oldframe)->GetFrame(newframe)->Copy(*tmp);
   GetListOfFrames()->Add(tmp);

}
//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* newframe, const Char_t* oldframe,
                          const TLorentzRotation& rot)
{
   GetFrame(oldframe)->SetFrame(newframe, rot);
   //Duplicate the transformed particle to access by the method GetFrame("newframe")
   //without the chain : particle->GetFrame("oldframe")->GetFrame("newframe");
   //Specially usefull if the new frame is used in KVVarGlob derived classes
   KVParticle* tmp = (KVParticle*)this->IsA()->New();
   tmp->SetBit(kCanDelete);
   tmp->SetFrameName(newframe);
   GetFrame(oldframe)->GetFrame(newframe)->Copy(*tmp);
   GetListOfFrames()->Add(tmp);

}

//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* newframe, const Char_t* oldframe,
                          const TRotation& rot)
{

   GetFrame(oldframe)->SetFrame(newframe, rot);
   //Duplicate the transformed particle to access by the method GetFrame("newframe")
   //without the chain : particle->GetFrame("oldframe")->GetFrame("newframe");
   //Specially usefull if the new frame is used in KVVarGlob derived classes
   KVParticle* tmp = (KVParticle*)this->IsA()->New();
   tmp->SetBit(kCanDelete);
   tmp->SetFrameName(newframe);
   GetFrame(oldframe)->GetFrame(newframe)->Copy(*tmp);
   GetListOfFrames()->Add(tmp);

}

//___________________________________________________________________________//

void KVParticle::SetFrame(const Char_t* newframe, const Char_t* oldframe,
                          const TVector3& boost, TRotation& rot, Bool_t beta)
{

   GetFrame(oldframe)->SetFrame(newframe, boost, rot, beta);
   //Duplicate the transformed particle to access by the method GetFrame("newframe")
   //without the chain : particle->GetFrame("oldframe")->GetFrame("newframe");
   //Specially usefull if the new frame is used in KVVarGlob derived classes
   KVParticle* tmp = (KVParticle*)this->IsA()->New();
   tmp->SetBit(kCanDelete);
   tmp->SetFrameName(newframe);
   GetFrame(oldframe)->GetFrame(newframe)->Copy(*tmp);
   GetListOfFrames()->Add(tmp);

}

//___________________________________________________________________________//

TVector3 KVParticle::GetVelocity() const
{
   //returns velocity vector in cm/ns units
   TVector3 beta;
   if (E()) {
      beta = GetMomentum() * (1. / E());
   } else {
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
   } else {
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
