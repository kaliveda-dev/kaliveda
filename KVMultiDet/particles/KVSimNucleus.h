//Created by KVClassFactory on Mon Jun 28 15:02:00 2010
//Author: bonnet

#ifndef __KVSIMNUCLEUS_H
#define __KVSIMNUCLEUS_H

#include "KVNucleus.h"
#include "KVNameValueList.h"

class TVector3;

/**
   \class KVSimNucleus
\brief Nucleus in a simulated event
\ingroup NucEvents
\ingroup Simulation

This class is used to represent nuclei in events produced by models and simulations.
In addition to the properties handled by parent class KVNucleus, a KVSimNucleus can also have:
  - a position in space (methods SetPosition(), GetPosition())
  - an angular momentum (spin) (methods SetAngMom(), GetAngMom())
  - a density (SetDensity(), GetDensity())

Methods are also provided for simple calculations of nuclear radii, (spherical) moment of inertia,
and rotation energy corresponding to the nucleus' angular momentum.

\sa KVNucleus, KVSimEvent, NucEvents
*/


class KVSimNucleus : public KVNucleus {

protected:
   TVector3 position;   // vector position of the particle in fm
   TVector3 angmom;  // angular momentum of the particle in units
   Double_t fDensity;   //density of the nucleus in nuc.fm-3
public:

   KVSimNucleus() : KVNucleus(), fDensity(0.0) {}
   KVSimNucleus(Int_t z, Int_t a = 0, Double_t ekin = 0) : KVNucleus(z, a, ekin), fDensity(0.0) {}
   KVSimNucleus(Int_t z, Double_t t, TVector3& p) : KVNucleus(z, t, p), fDensity(0.0) {}
   KVSimNucleus(Int_t z, Int_t a, TVector3 p) : KVNucleus(z, a, p), fDensity(0.0) {}
   KVSimNucleus(const Char_t* sym, Double_t EperA = 0) : KVNucleus(sym, EperA), fDensity(0.0) {}
   KVSimNucleus(const KVSimNucleus&);
   KVSimNucleus(const KVNucleus& n) : KVNucleus(n), fDensity(0.0) {}

   virtual ~KVSimNucleus() {}

   void Copy(TObject& obj) const;

   void SetPosition(Double_t rx, Double_t ry, Double_t rz);
   void SetPosition(const TVector3&);
   void SetDensity(Double_t);
   Double_t GetDensity() const;
   const TVector3* GetPosition() const
   {
      return &position;
   }
   TVector3& GetPosition()
   {
      return position;
   }

   Double_t GetRx() const
   {
      return position.X();
   }
   Double_t GetRy() const
   {
      return position.Y();
   }
   Double_t GetRz() const
   {
      return position.Z();
   }
   Double_t GetRmag() const
   {
      return position.Mag();
   }

   Double_t GetLx() const
   {
      return angmom.X();
   }
   Double_t GetLy() const
   {
      return angmom.Y();
   }
   Double_t GetLz() const
   {
      return angmom.Z();
   }
   Double_t GetLmag() const
   {
      return angmom.Mag();
   }

   void SetAngMom(Double_t lx, Double_t ly, Double_t lz);
   void SetSpin(Double_t x, Double_t y, Double_t z)
   {
      SetAngMom(x, y, z);
   }
   const TVector3* GetAngMom() const
   {
      return &angmom;
   }
   TVector3& GetAngMom()
   {
      return angmom;
   }
   Double_t GetRadius() const
   {
      // Spherical nuclear radius 1.2*A**(1/3)
      return 1.2 * pow(GetA(), 1. / 3.);
   }
   Double_t GetMomentOfInertia() const
   {
      // Moment of inertia for spherical nucleus of radius 1.2*A**(1/3)
      return 0.4 * GetMass() * pow(GetRadius(), 2);
   }
   Double_t GetRotationalEnergy() const
   {
      // Rotational energy for spherical nucleus
      Double_t s = angmom.Mag();
      return 0.5 * pow(hbar, 2) * (s * (s + 1.)) / GetMomentOfInertia();
   }

   Double_t GetEnergyLoss(const TString& detname) const;
   TVector3 GetEntrancePosition(const TString& detname) const;
   TVector3 GetExitPosition(const TString& detname) const;

   void Print(Option_t* t = "") const;

   KVSimNucleus operator+(const KVSimNucleus& rhs) const;
   KVSimNucleus& operator+=(const KVSimNucleus& rhs);
   KVSimNucleus& operator=(const KVSimNucleus&);

   ClassDef(KVSimNucleus, 4) //Nuclear particle in a simulated event

};

#endif
