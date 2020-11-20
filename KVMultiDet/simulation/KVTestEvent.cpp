#include <iostream>
#include <string>
#include <TMath.h>
#include <TRandom3.h>
#include "KVTestEvent.h"
#include "KVNucleus.h"

using std::cout;
using std::cerr;
using std::endl;

ClassImp(KVTestEvent);


KVTestEvent::KVTestEvent(): KVSimEvent()
{
   //Default constructor

   init();
}

void KVTestEvent::init()
{
   //Default initialisations

   fLower_Limit_theta = 0.;
   fLower_Limit_phi = 0.;
   fUpper_Limit_theta = 180.;
   fUpper_Limit_phi = 360.;
   fLower_Limit_Z = 0;
   fUpper_Limit_Z = 0;
   fLower_Limit_E = 0.;
   fUpper_Limit_E = 0.;
   fGenMult = 0;
   kAMeV = kFALSE;
   strcpy(fOption, "");
}

KVTestEvent::KVTestEvent(UInt_t mult, Option_t* t): KVSimEvent(mult)
{
   //Constructor with arguments
   //
   //mult = multiplicity of event
   //Option string can be
   //       "isotropic":  particle directions generated isotropically (1/sin Theta law)
   //       "random":  particle directions generated equiprobably

   init();
   fGenMult = mult;
   strcpy(fOption, t);
}

KVTestEvent::~KVTestEvent()
{
}

void KVTestEvent::SetZRange(UInt_t zlo, UInt_t zhi)
{
//
//Set range in Z of nuclei
//
   fLower_Limit_Z = zlo;
   fUpper_Limit_Z = zhi;
}

void KVTestEvent::SetERange(Float_t elo, Float_t ehi)
{
//
//Set range in kinetic energy of nuclei (MeV)
//
   fLower_Limit_E = elo;
   fUpper_Limit_E = ehi;
   kAMeV = kFALSE;
}

void KVTestEvent::SetERangeAMeV(Float_t elo, Float_t ehi)
{
//
//Set range in kinetic energy of nuclei (MeV/nucleon)
//
   fLower_Limit_E = elo;
   fUpper_Limit_E = ehi;
   kAMeV = kTRUE;
}

void KVTestEvent::SetThetaRange(Float_t thmin, Float_t thmax)
{
//
//Set polar angular range for nuclei
//
   fLower_Limit_theta = thmin;
   fUpper_Limit_theta = thmax;
}

void KVTestEvent::SetPhiRange(Float_t phmin, Float_t phmax)
{
//
//Set azimuthal angular range for nuclei
//
   fLower_Limit_phi = phmin;
   fUpper_Limit_phi = phmax;
}

void KVTestEvent::Generate()
{
//
//Generate an event based on given parameters.
//Before using, at least set multiplicity (using constructor with arguments or
//KVTestEvent::SetMult), range in Z (KVTestEvent::SetZRange) and range
//in energy (KVTestEvent::SetERange or KVTestEvent::SetERangeAMeV).
//

   for (UInt_t i = 1; i <= fGenMult; i++) {
      UInt_t z =
         gRandom->Integer(fUpper_Limit_Z - fLower_Limit_Z + 1) +
         fLower_Limit_Z;
      Float_t ke =
         gRandom->Uniform(fUpper_Limit_E - fLower_Limit_E) +
         fLower_Limit_E;
      KVNucleus* kvn = AddParticle();
      kvn->SetZ(z);
      if (kAMeV)
         ke *= kvn->GetA();
      kvn->SetRandomMomentum(ke, fLower_Limit_theta, fUpper_Limit_theta,
                             fLower_Limit_phi, fUpper_Limit_phi, fOption);
   }
}

void KVTestEvent::Print(Option_t*) const
{
//
//Print out list of all particles in event
//
   cout << "\nKVTestEvent with " << GetMult() << " nuclei:" << endl;
   cout << "----------------------------------------" << endl;
   cout << "Zmin=" << fLower_Limit_Z << " Zmax=" << fUpper_Limit_Z << endl;
   cout << "Emin=" << fLower_Limit_E << " Emax=" << fUpper_Limit_E;
   if (kAMeV)
      cout << " (MeV/nucleon)" << endl;
   else
      cout << " (MeV)" << endl;
   cout << "Thetamin=" << fLower_Limit_theta << " Thetamax=" <<
        fUpper_Limit_theta << endl;
   cout << "Phimin=" << fLower_Limit_phi << " Phimax=" << fUpper_Limit_phi
        << endl;
   cout << "----------------------------------------" << endl;
#ifdef WITH_CPP11
   for (auto& n : *this) n.Print();
#else
   for (KVEvent::Iterator it = begin(); it != end(); ++it)(*it).Print();
#endif
}
