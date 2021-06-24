/***************************************************************************
$Id: KVNucleus.h,v 1.40 2009/04/02 09:32:55 ebonnet Exp $
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVNUCLEUS_H
#define KVNUCLEUS_H

#include "TVector3.h"
#include "TEnv.h"
#include "KVParticle.h"
#include "KVNumberList.h"
#include "TLorentzRotation.h"
#include "KVString.h"

class KVLifeTime;
class KVMassExcess;
class KVAbundance;
class KVChargeRadius;
class KVSpinParity;

/**
  \class KVNucleus
  \brief Description of properties and kinematics of atomic nuclei
  \ingroup NucEvents
  \ingroup NucProp

### Examples of use:
Create an alpha particle, and find its binding energy, mass, and mass excess:
~~~~{.cpp}
 KVNucleus n(2,4); // create nucleus with Z=2 and A=4
 cout << "Binding energy of alpha = " << n.GetBindingEnergy() << " MeV" << endl;
 cout << "Mass = " << n.GetMass() << " = 4 * " << KVNucleus::kAMU << " + " << n.GetMassExcess() << endl;
~~~~
Using the same object we can also obtain binding energies and mass excesses
for all other nuclei:
~~~~{.cpp}
  n.GetBindingEnergy(1,2);//binding energy of deuteron
  n.GetMassExcess(0,1);//mass excess of neutron
~~~~

### Nuclear masses
We can also create nuclei by only specifying the atomic number Z.
In this case the mass number A is calculated from Z:
~~~~{.cpp}
    KVNucleus nuc(82);//create Pb nucleus
    nuc.GetA();//by default this is the A corresponding to beta-stability, i.e. 208
    nuc.SetMassFormula(KVNucleus::kVedaMass);//change the mass formula used for this nucleus
    nuc.GetA();//now the value is that calculated according to the Veda formula, 202
    nuc.SetMassFormula(KVNucleus::kEALMass);//Evaporation Attractor Line from R.J. Charity
    nuc.GetA();//gives 186
    nuc.SetMassFormula(KVNucleus::kBetaMass);//restore the default mass formula
~~~~
Z and A can be specified separately:
~~~~{.cpp}
   KVNucleus a;//no A or Z specified
   a.SetZ(10);//at this moment the nucleus' mass number is 20 (beta-stability)
   a.SetA(24);//now this represents a 24Ne nucleus
~~~~
\note be careful not to use SetZ() AFTER SetA(), because the mass number
is always automatically calculated from Z after a call to SetZ().

The value of the atomic mass unit, u, is given by KVNucleus::kAMU or KVNucleus::u()
It is 931.494 043 x 10**6 eV, as per the 2002 CODATA recommended values (Reviews of Modern Physics 77, 1-107 (2005)).

### Nuclear Arithmetic & Calorimetry
The '+', '-' and '=' operators have been redefined for the KVNucleus class.
One can therefore perform "nuclear arithmetic".
Example:
~~~~{.cpp}
  KVNucleus c = a + b; //'a' and 'b' are also KVNucleus objects
~~~~
The Z, A, momentum and excitation energy of 'c' are calculated from the
appropriate conservation laws. The mass excesses of the 3 nuclei are obviously
taken into consideration.

If 'a' and 'b' are projectile and target, 'c' is the compound nucleus after
fusion.

In order to perform calorimetry (calculation of source characteristics from
daughter nuclei) one need only sum all nuclei associated with the source.
The resulting nucleus is the source nucleus with its Z, A, momentum and
excitation energy.

 \note if one defines an excitation energy for a nucleus
 using the SetExcitEnergy() method, the mass and the total energy is modified (M = Mgs + Ex)
 when excitation energy is set, one can access to the ground state mass via GetMassGS()

The subtraction operator allows to perform energy balance for a binary
splitting of a nucleus.
Example:
~~~~{.cpp}
      KVNucleus d = c - b;
~~~~
In this case, the resulting nucleus 'd' should be identical to 'a' in the first
example. One could also imagine
~~~~{.cpp}
      KVNucleus e = c - alpha;
~~~~
where 'alpha' is a KVNucleus alpha-particle, for which we specify the
momentum after emission. The resulting nucleus 'e' is the residue of the
fusion compound after evaporation of an alpha particle.

\note It is the user's responsiblitly to handle excitation energy sharing between the outgoing nuclei.

The operators '+=' and '-=' also exist. 'a+=b' means 'a = a + b' etc.

### Mass Excess Table
Different mass tables can be implemented using classes derived from
KVMassTable. The mass table to be used is defined by environment variable
~~~~
  KVNucleus.MassExcessTable:        MyMassExcessTable
~~~~
where 'MyMassExcessTable' must be defined in terms of a KVNuclDataTable plugin:
~~~~
+Plugin.KVNuclDataTable: MyMassExcessTable  MyMassExcessTable  MyMassExcessTable.cpp+  " MyMassExcessTable()"
~~~~
*/

class KVNucleus: public KVParticle {


private:
   UChar_t fA;                  //nuclear mass number
   UChar_t fZ;                  //nuclear charge number (atomic number)
   UChar_t fMassFormula;        //mass formula for calculating A from Z
   static UInt_t fNb_nuc;       //!counts number of existing KVNucleus objects
   static Char_t fElements[][3];        //!symbols of chemical elements
   TString fSymbolName;        //!

   enum {
      kIsHeavy = BIT(17)        //flag when mass of nucleus is > 255
   };

protected:
   virtual void AddGroup_Withcondition(const Char_t* groupname, KVParticleCondition*) const;

public:
   enum {                       //determines how to calculate mass from Z
      kBetaMass,
      kVedaMass,
      kEALMass,
      kEALResMass,
      kEPAXMass
   };
   enum {                       //determines how to calculate radius from Mass
      kLDModel,
      kEMPFunc,
      kELTON
   };

   enum {
      kDefaultFormula,
      kItkis1998,
      kHinde1987,
      kViola1985,
      kViola1966
   };

   enum {
      kNN,
      knn,
      kpp,
      knp
   };

   static Double_t kAMU;        //atomic mass unit in MeV
   static Double_t kMe;        //electron mass in MeV/c2
   static Double_t u(void);
   static Double_t hbar;   // hbar*c in MeV.fm
   static Double_t e2;    // e^2/(4.pi.epsilon_0) in MeV.fm

   inline void SetMassFormula(UChar_t mt);
   inline Int_t GetMassFormula() const
   {
      return (Int_t)fMassFormula;
   }

   void init();
   KVNucleus();
   KVNucleus(const KVNucleus&);
   virtual void Clear(Option_t* opt = "");
   KVNucleus(Int_t z, Int_t a = 0, Double_t ekin = 0);
   KVNucleus(Int_t z, Double_t t, TVector3& p);
   KVNucleus(Int_t z, Int_t a, TVector3 p);
   //KVNucleus(const Char_t *);
   KVNucleus(const Char_t*, Double_t EperA = 0);

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif
   Bool_t IsSortable() const
   {
      return kTRUE;
   }
   Int_t Compare(const TObject* obj) const;
   Bool_t IsDefined() const
   {
      // Returns kTRUE if the Z and/or A of the nucleus have been set.
      //
      // 'Blank' nuclei created by the default constructor have Z=A=0 until a method such as SetZ() is called.
      // In this case IsDefined() returns kFALSE.
      return !(fZ == 0 && fA == 0);
   }

   virtual ~ KVNucleus();
   static Int_t GetAFromZ(Double_t, Char_t mt);
   static Int_t GetNFromZ(Double_t, Char_t mt);
   static Double_t GetRealAFromZ(Double_t, Char_t mt);
   static Double_t GetRealNFromZ(Double_t, Char_t mt);
   const Char_t* GetSymbol(Option_t* opt = "") const;
   const Char_t* GetLatexSymbol(Option_t* opt = "") const;

   static Int_t GetZFromSymbol(const Char_t*);
   int SetZFromSymbol(const Char_t*);
   void Set(const Char_t*);
   static Int_t IsMassGiven(const Char_t*);

   void SetZ(Int_t z, Char_t mt = -1);
   void SetA(Int_t a);
   void SetN(Int_t n);
   void SetZandA(Int_t z, Int_t a);
   void SetZandN(Int_t z, Int_t n);
   void SetZAandE(Int_t z, Int_t a, Double_t ekin);

   virtual void Print(Option_t* t = "") const;
   Int_t GetZ() const;
   Int_t GetA() const;
   Int_t GetN() const;

   Int_t GetNpairs(Int_t type = kNN) const;

   Double_t GetAsurZ() const
   {
      return Double_t(GetA()) / GetZ();
   }
   Double_t GetNsurZ() const
   {
      return Double_t(GetN()) / GetZ();
   }
   Double_t GetChargeAsymetry() const
   {
      //The charge asymertry  = (neutrons-protons)/nucleons
      //
      return Double_t(GetN() - GetZ()) / GetA();
   }
   Double_t GetEnergyPerNucleon() const;
   Double_t GetAMeV() const;

   void CheckZAndA(Int_t& z, Int_t& a) const;

   Double_t GetMassExcess(Int_t z = -1, Int_t a = -1) const;
   Double_t GetExtraMassExcess(Int_t z = -1, Int_t a = -1) const;
   KVMassExcess* GetMassExcessPtr(Int_t z = -1, Int_t a = -1) const;
   Double_t GetAtomicMass(Int_t zz = -1, Int_t aa = -1) const ;
   Double_t GetNaturalA(Int_t zz = -1) const ;

   Double_t GetBindingEnergy(Int_t z = -1, Int_t a = -1) const;
   Double_t GetLiquidDropBindingEnergy(Int_t z = -1, Int_t a = -1) const;
   Double_t GetBindingEnergyPerNucleon(Int_t z = -1, Int_t a = -1) const;

   Double_t LiquidDrop_Weizsacker();

   KVNumberList GetKnownARange(Int_t z = -1, Double_t tmin = 0) const;
   KVNumberList GetMeasuredARange(Int_t z = -1) const;
   const Char_t* GetIsotopesList(Int_t zmin, Int_t zmax, Double_t tmin = 0) const;
   Int_t GetAWithMaxBindingEnergy(Int_t z = -1);

   static Double_t LiquidDrop_BrackGuet(UInt_t A, UInt_t Z);

   Bool_t IsKnown(int z = -1, int a = -1) const;
   Bool_t IsStable(Double_t min_lifetime = 1.0e+15/*seconds*/) const;
   Bool_t IsResonance() const;
   Double_t GetWidth() const;

   void SetExcitEnergy(Double_t e);

   Double_t GetExcitEnergy() const
   {
      // Return the excitation energy of the nucleus in MeV.
      // This is the difference between the (relativistic) rest mass
      // and the ground state mass of the nucleus
      return GetMass() - GetMassGS();
   }
   Double_t GetMassGS() const
   {
      // Return the ground state mass of the nucleus in MeV.
      return (kAMU * GetA() + GetMassExcess());
   }

   Double_t GetLifeTime(Int_t z = -1, Int_t a = -1) const;
   KVLifeTime* GetLifeTimePtr(Int_t z = -1, Int_t a = -1) const;

   Double_t GetSpin(Int_t z = -1, Int_t a = -1) const;
   Double_t GetParity(Int_t z = -1, Int_t a = -1) const;
   KVSpinParity* GetSpinParityPtr(Int_t z = -1, Int_t a = -1) const;

   Double_t GetAbundance(Int_t z = -1, Int_t a = -1) const;
   KVAbundance* GetAbundancePtr(Int_t z = -1, Int_t a = -1) const;
   Int_t GetMostAbundantA(Int_t z = -1) const;

   Double_t GetChargeRadius(Int_t z = -1, Int_t a = -1) const;
   KVChargeRadius* GetChargeRadiusPtr(Int_t z = -1, Int_t a = -1) const;
   Double_t GetExtraChargeRadius(Int_t z = -1, Int_t a = -1, Int_t rct = 2) const;

   KVNucleus& operator=(const KVNucleus& rhs);
   KVNucleus operator+(const KVNucleus& rhs);
   KVNucleus operator-(const KVNucleus& rhs);
   KVNucleus& operator+=(const KVNucleus& rhs);
   KVNucleus& operator-=(const KVNucleus& rhs);

   // TH2F* GetKnownNucleiChart(KVString method="GetBindingEnergyPerNucleon");
   Double_t DeduceEincFromBrho(Double_t Brho, Int_t ChargeState = 0);
   Double_t GetRelativeVelocity(KVNucleus* nuc);
   Double_t GetFissionTKE(const KVNucleus* nuc = 0, Int_t formula = kDefaultFormula) const;
   Double_t GetQFasymTKE(KVNucleus* target);
   Double_t GetFissionVelocity(KVNucleus* nuc = 0, Int_t formula = kDefaultFormula);

   static Double_t TKE_Hinde1987(Double_t z1, Double_t a1, Double_t z2, Double_t a2);
   static Double_t TKE_Viola1985(Double_t z, Double_t a);
   static Double_t TKE_Viola1966(Double_t z, Double_t a);
   static Double_t TKE_Itkis1998(Double_t z, Double_t a);
   static Double_t TKE_Kozulin2014(Double_t zp, Double_t zt, Double_t ap, Double_t at);

   Bool_t IsElement(Int_t Z) const
   {
      return GetZ() == Z;
   }
   Bool_t IsIsotope(Int_t Z, Int_t A) const
   {
      return (GetZ() == Z && GetA() == A);
   }

   Double_t ShimaChargeState(Int_t) const;
   Double_t ShimaChargeStatePrecision() const;

   ClassDef(KVNucleus, 7)      //Class describing atomic nuclei
};

inline void KVNucleus::SetMassFormula(UChar_t mt)
{
   //Set mass formula used for calculating A from Z for this nucleus
   fMassFormula = mt;
   SetA(GetAFromZ(GetZ(), fMassFormula));       //recalculate A and mass
}

#endif
