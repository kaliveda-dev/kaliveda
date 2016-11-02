//Created by KVClassFactory on Mon Jun 28 15:02:00 2010
//Author: bonnet

#ifndef __KVSIMNUCLEUS_H
#define __KVSIMNUCLEUS_H

#include "KVNucleus.h"
#include "KVNameValueList.h"

class TVector3;

class KVSimNucleus : public KVNucleus {

protected:
   TVector3 position;   // vector position of the particle in fm
   TVector3 angmom;  // angular momentum of the particle in units
   Double_t fDensity;   //density of the nucleus in nuc.fm-3
public:

   KVSimNucleus() : KVNucleus() {}
   KVSimNucleus(Int_t z, Int_t a = 0, Double_t ekin = 0) : KVNucleus(z, a, ekin) {}
   KVSimNucleus(Int_t z, Double_t t, TVector3& p) : KVNucleus(z, t, p) {}
   KVSimNucleus(Int_t z, Int_t a, TVector3 p) : KVNucleus(z, a, p) {}
   KVSimNucleus(const Char_t* sym, Double_t EperA = 0) : KVNucleus(sym, EperA) {}
   KVSimNucleus(const KVSimNucleus&);
   KVSimNucleus(const KVNucleus& n) : KVNucleus(n) {}

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
   TVector3& GetSpin()
   {
      return GetAngMom();
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

   KVSimNucleus operator+(const KVSimNucleus& rhs);
   KVSimNucleus& operator+=(const KVSimNucleus& rhs);

   ClassDef(KVSimNucleus, 4) //Nuclear particle in a simulated event

};

#endif
