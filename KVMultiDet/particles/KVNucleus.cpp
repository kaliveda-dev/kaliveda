/***************************************************************************
$Id: KVNucleus.cpp,v 1.48 2009/04/02 09:32:55 ebonnet Exp $
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVNucleus.h"
#include "KVString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "Riostream.h"
#include "TSystem.h"
#include "TEnv.h"
#include "KVParticleCondition.h"
#include "Riostream.h"
#include "TMethodCall.h"
#include "TPluginManager.h"
#include "KVNDTManager.h"

#include "KVLifeTime.h"
#include "KVMassExcess.h"
#include "KVAbundance.h"
#include "KVChargeRadius.h"
#include "KVSpinParity.h"

//Atomic mass unit in MeV
//Reference: 2002 CODATA recommended values Reviews of Modern Physics 77, 1-107 (2005)
Double_t KVNucleus::kAMU = 9.31494043e02;
Double_t KVNucleus::kMe = 0.510998;
// hbar*c in MeV.fm = 197.33....
Double_t KVNucleus::hbar = TMath::Hbarcgs() * TMath::Ccgs() / TMath::Qe();
// e^2/(4.pi.epsilon_0) in MeV.fm = 1.44... = hbar*alpha (fine structure constant)
Double_t KVNucleus::e2 = KVNucleus::hbar / 137.035999074;

using namespace std;

ClassImp(KVNucleus);



UInt_t KVNucleus::fNb_nuc = 0;

#define MAXZ_ELEMENT_SYMBOL 118
Char_t KVNucleus::fElements[][3] = {
   "n", "H", "He", "Li", "Be", "B", "C", "N", "O",
   "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
   "Sc",
   "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As",
   "Se", "Br",
   "Kr", "Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag",
   "Cd",
   "In", "Sn", "Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd",
   "Pm",
   "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf", "Ta",
   "W",
   "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",
   "Fr",
   "Ra", "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es",
   "Fm", "Md",
   "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn", "Ed",
   "Fl", "Ef",
   "Lv", "Eh", "Ei"
};

const Char_t* KVNucleus::GetSymbol(Option_t* opt) const
{
   // Returns symbol of isotope corresponding to this nucleus,
   // i.e. "238U", "12C", "3He" etc.
   // Neutrons are represented by "n".
   // In order to have just the symbol of the chemical element
   // (e.g. "Pt", "Zn", "Fe"), call with opt="EL".

   Int_t a = GetA();
   Int_t z = GetZ();
   TString& symname = (TString&)fSymbolName;
   Bool_t Mpfx = strcmp(opt, "EL"); // kTRUE if mass prefix required
   if (0 <= GetZ() && GetZ() <= MAXZ_ELEMENT_SYMBOL) {
      if (Mpfx) symname.Form("%d%s", a, fElements[z]);
      else symname = fElements[z];
   }
   else
      symname = "";

   return symname.Data();
}
const Char_t* KVNucleus::GetLatexSymbol(Option_t* opt) const
{
   // Returns symbol of isotope corresponding to this nucleus,
   // suitable for latex format in ROOT TLatex type class
   // i.e. "^{238}U", "^{12}C", "^{3}He" etc.
   // Neutrons are represented by "^{1}n".
   // In order to have also the charge printed like this : ^{12}_{6}C
   // call with opt="ALL".

   Int_t a = GetA();
   Int_t z = GetZ();
   TString& symname = (TString&)fSymbolName;
   if (0 <= GetZ() && GetZ() <= MAXZ_ELEMENT_SYMBOL) {
      if (!strcmp(opt, "ALL")) symname.Form("{}^{%d}_{%d}%s", a, z, fElements[z]);
      else                    symname.Form("^{%d}%s", a, fElements[z]);
   }
   else {
      symname = "";
   }
   return symname.Data();
}

Int_t KVNucleus::IsMassGiven(const Char_t* isotope)
{
   // test if the given string corresponds to the name of an isotope/element,
   // and whether or not a mass is specified.
   // isotope = symbol for element isotope, "C", "natSn", "13N", etc.
   // if the mass of the isotope is given ("13N", "233U") we return the given mass
   // if this is a valid element but no mass is given we return 0
   // if this is not a valid isotope/element, we return -1

   Int_t A;
   Char_t name[5];
   TString tmp(isotope);
   if (tmp.BeginsWith("nat"))
      tmp.Remove(0, 3);
   if (sscanf(tmp.Data(), "%d%s", &A, name) == 2) {
      //name given in form "208Pb"
      Int_t z = GetZFromSymbol(name);
      if (z < 0) return z;
      return A;
   }
   Int_t z = GetZFromSymbol(tmp);
   if (z < 0) return z;
   return 0;
}

void KVNucleus::Set(const Char_t* isotope)
{
   // Set nucleus' Z & A using chemical symbol e.g. Set("12C") or Set("233U") etc.
   //
   // Any failure to deduce Z from the symbol will result in this object being made
   // a zombie i.e. IsZombie() will return kTRUE

   Int_t A;
   Char_t name[255];
   TString tmp(isotope);
   if (tmp.BeginsWith("nat"))
      tmp.Remove(0, 3);
   if (sscanf(tmp.Data(), "%d%s", &A, name) == 2) {
      //name given in form "208Pb"
      if (SetZFromSymbol(name) > -1) SetA(A);
      else MakeZombie();
   }
   else if (sscanf(tmp.Data(), "%s", name) == 1) {
      //name given in form "Pb"
      if (SetZFromSymbol(name) == -1) MakeZombie();
   }
}

Int_t KVNucleus::GetZFromSymbol(const Char_t* sym)
{
   //Returns Z of nucleus with given symbol i.e. "C" => Z=6, "U" => Z=92
   //if unknown, returns -1
   for (int i = 0; i <= MAXZ_ELEMENT_SYMBOL; i++) {
      if (!strcmp(sym, fElements[i])) {
         return i;
      }
   }
   return -1;
}

int KVNucleus::SetZFromSymbol(const Char_t* sym)
{
   // Set Z of nucleus with given symbol i.e. "C" => Z=6, "U" => Z=92
   //
   // Returns Z found, or -1 if symbol is unknown

   Int_t z = GetZFromSymbol(sym);
   if (z > -1) SetZ(z);
   else Error("SetZFromSymbol", "%s is unknown", sym);
   return z;
}

//_________________________________________________________________________________

void KVNucleus::init()
{
   // Default intialisations
   // The mass formula is kBetaMass, i.e. the formula for the valley of beta-stability.
   // Set up nuclear data table manager if not done already

   fZ = fA = 0;
   if (!fNb_nuc) {
      KVBase::InitEnvironment(); // initialise environment i.e. read .kvrootrc
      if (!gNDTManager) gNDTManager = new KVNDTManager;
   }
   fMassFormula = kBetaMass;
   fNb_nuc++;
}

KVNucleus::KVNucleus()
{
   //
   //Default constructor.
   //

   init();
}

KVNucleus::KVNucleus(const KVNucleus& obj) : KVParticle()
{
   //copy ctor
   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   obj.Copy(*this);
#else
   ((KVNucleus&) obj).Copy(*this);
#endif
}

//___________________________________________________________________________________________
void KVNucleus::Clear(Option_t* opt)
{
   // Reset nucleus' properties: set A and Z to zero.
   // For other properties, see KVParticle::Clear

   KVParticle::Clear(opt);
   ResetBit(kIsHeavy);
   fZ = fA = 0;
}

//___________________________________________________________________________________________
KVNucleus::KVNucleus(Int_t z, Int_t a, Double_t ekin)
{
   //Create a nucleus with atomic number Z.
   //If the mass number A is not given, A is calculated using the
   //parametrisation determined by the value of fMassFormula (see KVNucleus::GetAFromZ).
   //ekin is the kinetic energy in MeV

   init();
   fZ = (UChar_t) z;
   if (z != 0 && a == 0) {
      SetA(GetAFromZ(z, fMassFormula));
   }
   else {
      SetA(a);
   }
   SetKE(ekin);
}

//___________________________________________________________________________________________
KVNucleus::KVNucleus(const Char_t* symbol, Double_t EperA)
{
   // Create a nucleus defined by symbol e.g. "12C", "34Mg", "42Si" etc. etc.
   //
   // If symbol is not valid, will be made a zombie (IsZombie() returns kTRUE)
   //
   // The second argument is the kinetic energy per nucleon (E/A) in MeV/A unit

   init();
   Set(symbol);
   if (!IsZombie()) SetKE(EperA * GetA());
}

//___________________________________________________________________________________________
KVNucleus::KVNucleus(Int_t z, Double_t t, TVector3& p)
{

   //Create nucleus with given Z, kinetic energy t and direction p
   //(p is a unit vector in the desired direction. See KVPosition for methods
   //for generating such vectors).
   //The mass number A is calculated from Z. See KVNucleus::GetAFromZ.
   //
   init();
   fZ = (UChar_t) z;
   SetA(GetAFromZ(z, fMassFormula));
   SetMomentum(t, p);
}

//___________________________________________________________________________________________
KVNucleus::KVNucleus(Int_t z, Int_t a, TVector3 p)
{
   //
   //Create nucleus with given Z, A, and 3-momentum p
   //
   init();
   fZ = (UChar_t) z;
   SetA(a);
   SetMomentum(&p);
}

//___________________________________________________________________________________________
KVNucleus::~KVNucleus()
{
   fNb_nuc--;
   fZ = fA = 0;
}

//___________________________________________________________________________________________

Double_t KVNucleus::GetRealAFromZ(Double_t Z, Char_t mt)
{
   //Calculate nuclear mass number from the element's atomic number Z.
   //This value is not rounded off, we just return the result of one of the following formulae:
   //
   //mt = KVNucleus::kVedaMass
   //__________________________
   //Veda - A calculated using the formula
   //      fA = (1.867*fZ+.016*fZ*fZ-1.07E-4*fZ*fZ*fZ);
   //      This corresponds to the amass.f subroutine of the old INDRA Veda
   //      calibration programme. This formula was supposed to represent
   //      the Z-dependence of isotope masses in the beta-stability valley,
   //      but is in fact a rather poor approximation, especially for large Z.
   //
   //mt = KVNucleus::kBetaMass
   //_________________________
   //Beta (default) - An improved parametrisation of the beta-stability valley,
   //   correct even for heavy nuclei up to 238U. The formula is the result
   //   of a fit to 8 stable nuclear masses from Ne20 up to U238.
   //   fA = (.2875 + 1.7622 *Z + .013879 * Z  * Z - .000054875 * Z * Z * Z);
   //
   //mt = KVNucleus::kEALMass
   //________________________
   //EAL - parametrisation of the Evaporation Attractor Line (residue corridor)
   //    due to R.J. Charity (PRC 58(1998)1073) (eq 2)
   //    fA = (2.072*Z + 2.32E-03 * Z*Z) ;
   //
   //mt = KVNucleus::kEALResMass
   //________________________
   //EALRes - R.J. Charity ---- improvement of EAL parametrisation for
   // Heavy Residue (QP for instance) (PRC 58(1998)1073) (eq 7)
   //    fA = (2.045*Z + 3.57E-03 * Z*Z) ;
   //
   //mt = any other value: A=2*Z

   Double_t A;
   switch (mt) {

      case kVedaMass:
         A = (1.867 * Z + .016 * TMath::Power(Z, 2.) -
              1.07E-4 * TMath::Power(Z, 3.));
         break;

      case kBetaMass:
         A = (.2875 + 1.7622 * Z + .013879 * TMath::Power(Z, 2.) -
              .000054875 * TMath::Power(Z, 3.));
         break;

      case kEALMass:
         A = (2.072 * Z + 2.32E-03 * TMath::Power(Z, 2.));
         break;

      case kEALResMass:
         A = (2.045 * Z + 3.57E-03 * TMath::Power(Z, 2.));
         break;

      default:
         A = 2. * Z;
   }

   return A;
}
//___________________________________________________________________________________________

Double_t KVNucleus::GetRealNFromZ(Double_t Z, Char_t mt)
{
   //Calculate neutron number from the element's atomic number Z.
   //This value is not rounded off, we just return the result
   //obtain from the chosen mass formula (mt)
   return GetRealAFromZ(Z, mt) - Z;

}
//___________________________________________________________________________________________
Int_t KVNucleus::GetAFromZ(Double_t Z, Char_t mt)
{
   //Calculate nuclear mass number from the element's atomic number Z.
   //Used by default to set fA and fMass if fA not given.
   //For light nuclei (Z<6) the values are given (not calculated) and
   //correspond to: p, alpha, 7Li, 9Be, 11B.
   //For heavier nuclei, several prescriptions are available
   //by giving one of the following values to argument mt:
   //
   //mt = KVNucleus::kVedaMass
   //__________________________
   //Veda - A calculated using the formula
   //      fA = TMath::Nint(1.867*fZ+.016*fZ*fZ-1.07E-4*fZ*fZ*fZ);
   //      This corresponds to the amass.f subroutine of the old INDRA Veda
   //      calibration programme. These are the masses used in the first
   //      INDRA campaigns.
   //      For light nuclei (Z<6) the values are given (not calculated) and
   //         correspond to: p, alpha, 6Li, 8Be, 10B.
   //
   //mt = KVNucleus::kBetaMass
   //_________________________
   //Beta (default) - An improved parametrisation of the beta-stability valley,
   //   correct even for heavy nuclei up to 238U. The formula is the result
   //   of a fit to 8 stable nuclear masses from Ne20 up to U238. From carbon-12 onwards,
   //   the mass is calculated using
   //   fA = (Int_t) (.2875 + 1.7622 *Z + .013879 * Z  * Z - .000054875 * Z * Z * Z) + 1;
   //
   //mt = KVNucleus::kEALMass
   //________________________
   //EAL - parametrisation of the Evaporation Attractor Line (residue corridor)
   //    due to R.J. Charity (PRC 58(1998)1073).
   //    fA = (Int_t)(2.072*Z + 2.32E-03 * Z*Z) + 1; (eq 2)
   //
   //mt = KVNucleus::kEALResMass
   //________________________
   //EALRes - R.J. Charity ---- improvement of EAL parametrisation for
   // Heavy Residues (QP for instance) (PRC 58(1998)1073) (eq 7)
   //    fA = (Int_t)(2.045*Z + 3.57E-03 * Z*Z) + 1 ;
   //
   //mt = any other value: A=2*Z

   Int_t A = 0;
   Int_t z = (Int_t) Z;
   switch (z) {                 // masses for lightest nuclei
      case 1:
         A = 1;
         break;
      case 2:
         A = 4;
         break;
      case 3:
         A = (mt == kVedaMass ? 6 : 7);
         break;
      case 4:
         A = (mt == kVedaMass ? 8 : 9);
         break;
      case 5:
         A = (mt == kVedaMass ? 10 : 11);
         break;
      default:
         if (mt == kVedaMass)
            A = TMath::Nint(KVNucleus::GetRealAFromZ(Z, mt));
         else
            A = (Int_t) KVNucleus::GetRealAFromZ(Z, mt) + 1;
   }
   return A;
}
//___________________________________________________________________________________________
Int_t KVNucleus::GetNFromZ(Double_t Z, Char_t mt)
{
   //Calculate neutron number from the element's atomic number Z.
   return GetAFromZ(Z, mt) - Int_t(Z);

}

//___________________________________________________________________________________________
void KVNucleus::SetA(Int_t a)
{
   //Set mass number
   //Be careful not to call SetZ() after SetA(), as SetZ() will
   //reset the mass number according to one of the available
   //parametrisations of A as a function of Z.
   //
   //For A>255 the kIsHeavy flag is set. Then fA will equal A-255,
   //and GetA will return fA+255.
   //If A<=255 the flag is reset.

   if (a > 255) {
      fA = (UChar_t)(a - 255);
      SetBit(kIsHeavy);
   }
   else {
      fA = (UChar_t) a;
      ResetBit(kIsHeavy);
   }
   SetMass(GetMassGS());
}
//___________________________________________________________________________________________
void KVNucleus::SetN(Int_t n)
{
   //Set mass number
   //Be careful not to call SetZ() after SetN(), as SetZ() will
   //reset the neutron number according to one of the available
   //parametrisations of A (N+Z) as a function of Z.
   //
   Int_t z = GetZ();
   SetA(z + n);
}

//___________________________________________________________________________________________
void KVNucleus::SetZ(Int_t z, Char_t mt)
{
   //Set atomic number
   //The mass number fA is automatically calculated and set using GetAFromZ().
   //The optional EMassType argument allows to change the default parametrisation
   //used for calculating A from Z.
   fZ = (UChar_t) z;
   if (mt > -1)
      fMassFormula = mt;
   SetA(GetAFromZ(z, fMassFormula));
}

//___________________________________________________________________________________________
void KVNucleus::SetZandA(Int_t z, Int_t a)
{
   //Set atomic number and mass number
   SetZ(z);
   SetA(a);
}

//___________________________________________________________________________________________
void KVNucleus::SetZAandE(Int_t z, Int_t a, Double_t ekin)
{
   //Set atomic number, mass number, and kinetic energy in MeV
   SetZ(z);
   SetA(a);
   SetKE(ekin);
}

//___________________________________________________________________________________________
void KVNucleus::SetZandN(Int_t z, Int_t n)
{
   //Set atomic number and mass number
   SetZ(z);
   SetN(n);
}

//___________________________________________________________________________________________
void KVNucleus::Print(Option_t* t) const
{
   // Display nucleus parameters
   cout << "KVNucleus Z=" << GetZ() << " A=" << GetA()  << " E*=" << GetExcitEnergy() << endl;
   KVParticle::Print(t);
}

//___________________________________________________________________________________________
Int_t KVNucleus::GetZ() const
{
   //Return the number of proton / atomic number
   return (Int_t) fZ;
}

//___________________________________________________________________________________________
Int_t KVNucleus::GetN() const
{
   //Return the number of neutron
   return (Int_t)(GetA() - GetZ());
}

//___________________________________________________________________________________________
Int_t KVNucleus::GetA() const
{
   //Returns mass number (A) of nucleus.
   //
   //The actual member variable (fA) is a UChar_t and so limited to values 0-255.
   //In case nuclei with larger A are needed (for example in calculations of 2-body
   //scattering, a temporary nucleus corresponding to the sum of the entrance channel
   //nuclei is used in order to find the outgoing target-like from the outgoing
   //projectile-like) the flag "kIsHeavy" is set and GetA returns the value (fA+255).
   //For this reason you should always use GetA and not fA.

   if (TestBit(kIsHeavy))
      return ((Int_t) fA + 255);
   return (Int_t) fA;
}
//___________________________________________________________________________________________
Int_t KVNucleus::GetNpairs(Int_t type) const
{

   if (type == kNN)       return GetA() * (GetA() - 1) / 2;
   else if (type == knn)  return GetN() * (GetN() - 1) / 2;
   else if (type == kpp)  return GetZ() * (GetZ() - 1) / 2;
   else if (type == knp)  return GetZ() * GetN();
   else return 0;
}

//_______________________________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVNucleus::Copy(TObject& obj) const
#else
void KVNucleus::Copy(TObject& obj)
#endif
{
   //Copy this KVNucleus into the KVNucleus object referenced by "obj"
   KVParticle::Copy(obj);
   ((KVNucleus&) obj).SetZ(GetZ());
   ((KVNucleus&) obj).SetMassFormula(fMassFormula);
   ((KVNucleus&) obj).SetA(((KVNucleus*) this)->GetA());
   ((KVNucleus&) obj).SetExcitEnergy(((KVNucleus*) this)->
                                     GetExcitEnergy());
}

//________________________________________________________________________________________
void  KVNucleus::CheckZAndA(Int_t& z, Int_t& a) const
{
   if (z == -1)   z = GetZ();
   if (a == -1)   a = GetA();

}

//________________________________________________________________________________________
void KVNucleus::SetExcitEnergy(Double_t ex)
{
   // Define excitation energy of nucleus in MeV.
   // The rest mass of the nucleus is changed: m0 -> m0 + E*

   SetMass(GetMassGS() + ex);
}

//________________________________________________________________________________________

Double_t KVNucleus::GetMassExcess(Int_t z, Int_t a) const
{
   //Returns mass excess value in MeV for this nucleus.
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.
   //If the nucleus is not included in the mass table, an extrapolated value
   //using KVNucleus::LiquidDrop_BrackGuet is returned.

   CheckZAndA(z, a);

   Double_t val = gNDTManager->GetValue(z, a, "MassExcess");
   if (val == -555) return GetExtraMassExcess(z, a);
   else           return val;

}
//________________________________________________________________________________________

Double_t KVNucleus::GetExtraMassExcess(Int_t z, Int_t a) const
{
   //Calculate the extrapoled mass excess value
   // from the LiquidDrop_BrackGuet formula
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.

   CheckZAndA(z, a);
   return (LiquidDrop_BrackGuet(a, z) - a * kAMU);

}

//_____________________________________________
Double_t KVNucleus::GetAtomicMass(Int_t zz, Int_t aa) const
{
   // Returns the mass of an isotope in unified atomic mass units
   // (KVNucleus::u() MeV/c**2).
   // This number is also the mass in grammes of 1 mole of this isotope.
   CheckZAndA(zz, aa);
   return aa + GetMassExcess(zz, aa) / u();
}

//________________________________________________________________________________________

KVMassExcess* KVNucleus::GetMassExcessPtr(Int_t z, Int_t a) const
{
   //Returns pointer of corresponding KVMassExcess object
   //0 if the Z,A couple is not in the table
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.
   CheckZAndA(z, a);
   return (KVMassExcess*)gNDTManager->GetData(z, a, "MassExcess");

}

//________________________________________________________________________________________

KVSpinParity* KVNucleus::GetSpinParityPtr(Int_t z, Int_t a) const
{
   //Returns pointer of corresponding KVSpinParity object
   //0 if the Z,A couple is not in the table
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.
   CheckZAndA(z, a);
   return (KVSpinParity*)gNDTManager->GetData(z, a, "SpinParity");

}

//________________________________________________________________________________________

Double_t KVNucleus::GetSpin(Int_t z, Int_t a) const
{
   //Returns spin value for this nucleus.
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.
   //If the nucleus is not included in the mass table, -1 is returned

   CheckZAndA(z, a);

   Double_t val = gNDTManager->GetValue(z, a, "SpinParity");
   if (val == -555)
      return -1;
   return TMath::Abs(val);

}

//________________________________________________________________________________________

Double_t KVNucleus::GetParity(Int_t z, Int_t a) const
{
   //Returns parity value (-1 or +1) for this nucleus.
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.
   //If the nucleus is not included in the mass table, O is returned

   CheckZAndA(z, a);
   Double_t val = gNDTManager->GetValue(z, a, "SpinParity");
   if (val == -555)
      return 0;
   return TMath::Sign(-1.0, val);

}

//________________________________________________________________________________________

Double_t KVNucleus::GetLifeTime(Int_t z, Int_t a) const
{
   //Returns life time in seconds (see KVLifeTime class for unit details).
   //For 'stable' nuclei (for which the abundance is known),
   //if no lifetime exists in the table we return 1.e+100.
   //For other "nuclei" with no known lifetime we return -1.
   //For resonances (IsResonance() returns kTRUE) we calculate the lifetime
   //from the width of the resonance, t = hbar/W.
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.

   CheckZAndA(z, a);
   KVLifeTime* lf = GetLifeTimePtr(z, a);
   if (!lf) {
      if (GetAbundance(z, a) > 0) return 1.e+100;
      return -1.0;
   }
   if (!lf->IsAResonance()) {
      Double_t life = lf->GetValue();
      return (life < 0. ? 1.e+100 : life);
   }
   Double_t life = ((hbar / TMath::Ccgs()) * 1.e-13) / lf->GetValue();
   return life;
}


//________________________________________________________________________________________

KVLifeTime* KVNucleus::GetLifeTimePtr(Int_t z, Int_t a) const
{
   //Returns the pointeur of the life time object associated to this nucleus
   //If optional arguments (z,a) are given we return object for the
   //required nucleus.

   CheckZAndA(z, a);
   return (KVLifeTime*)gNDTManager->GetData(z, a, "LifeTime");

}

//________________________________________________________________________________________

Double_t KVNucleus::GetChargeRadius(Int_t z, Int_t a) const
{
   //Returns charge radius in fm for tabulated nuclei
   //if not tabulated returns the extrapolated radius
   //calculate in GetExtraChargeRadius
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.

   CheckZAndA(z, a);
   KVChargeRadius* cr = GetChargeRadiusPtr(z, a);
   if (!cr) {
      return GetExtraChargeRadius(z, a);
   }
   return cr->GetValue();

}

//________________________________________________________________________________________

Double_t KVNucleus::GetExtraChargeRadius(Int_t z, Int_t a, Int_t rct) const
{
   //Calculate the extrapoled charge radius
   // Three formulae taken from Atomic Data and Nuclear Data Tables 87 (2004) 185-201
   // are proposed:
   // rct=2 (kELTON)take into account the finite surfacethickness
   // This rct=2 is set by default because it has the best reproduction of exp data
   //
   // rct=1 (kEMPFunc) is a purely emperical function re*A**ee
   // rct=0 (kLDModel) is the standard Liquid Drop model approximation
   //
   // Those formulae are valid for nuclei near the stability valley
   // other parametrization for xotic nuclei are proposed in the same reference
   // but needed extrapolation from given nuclei and I don't have time
   // to do it now
   //
   // If optional arguments (z,a) are given we return the value for the
   // required nucleus.

   CheckZAndA(z, a);
   Double_t R = 0;
   Double_t A = Double_t(a);

   Double_t rLD = 0.9542; //for kLDModel

   Double_t re = 1.153; //for kEMPFunc
   Double_t ee = 0.2938; //for kEMPFunc

   Double_t r0 = 0.9071; //for kELTON
   Double_t r1 = 1.105;
   Double_t r2 = -0.548;

   switch (rct) {

      case kLDModel:
         R = rLD * TMath::Power(A, 1. / 3.);
         break;

      case kEMPFunc:
         R = re * TMath::Power(A, ee);
         break;

      case kELTON:
         R = (r0 * TMath::Power(A, 1. / 3.) + r1 / TMath::Power(A, 1. / 3.) + r2 / A);
         break;

   }

   return R;

}

//________________________________________________________________________________________

KVChargeRadius* KVNucleus::GetChargeRadiusPtr(Int_t z, Int_t a) const
{
   //Returns the pointeur of charge radius object associated to this nucleus
   //If optional arguments (z,a) are given we return object for the
   //required nucleus.

   CheckZAndA(z, a);
   return (KVChargeRadius*)gNDTManager->GetData(z, a, "ChargeRadius");

}

//________________________________________________________________________________________

Double_t KVNucleus::GetAbundance(Int_t z, Int_t a) const
{
   //Returns relative abundance value (see KVAbundance class for unit details).
   //If optional arguments (z,a) are given we return the value for the
   //required nucleus.

   CheckZAndA(z, a);
   return TMath::Max(0.0, gNDTManager->GetValue(z, a, "Abundance"));
}

//________________________________________________________________________________________

Int_t KVNucleus::GetMostAbundantA(Int_t z) const
{
   //Returns for a the Z of the current nucleus (z=-1) or the given z
   // most abundant A.
   //return -1 if no isotope of the given z have an abundance
   Int_t amost = -1;
   if (z == -1) z = GetZ();
   KVNumberList ll = GetKnownARange(z);
   ll.Begin();
   Double_t abmax = 0;
   while (!ll.End()) {
      Int_t a = ll.Next();
      Double_t abund = GetAbundance(z, a);
      if (abund > abmax) {
         abmax = abund;
         amost = a;
      }
   }
   return amost;
}

//________________________________________________________________________________________

KVAbundance* KVNucleus::GetAbundancePtr(Int_t z, Int_t a) const
{
   //Returns the pointeur of the abundance object associated to this nucleus
   //If optional arguments (z,a) are given we return the object for the
   //required nucleus.

   CheckZAndA(z, a);
   return (KVAbundance*)gNDTManager->GetData(z, a, "Abundance");

}

//________________________________________________________________________________________

Bool_t KVNucleus::IsKnown(int z, int a) const
{
   //Old method, the answer is only valid for the mass excess table
   //Returns kTRUE if this nucleus or (z,a) is included in the mass table.
   //
   //We kept it for backward compatibility :

   CheckZAndA(z, a);
   //return fMassTable->IsKnown(z,a);
   return gNDTManager->IsInTable(z, a, "MassExcess");
}

//________________________________________________________________________________________

Double_t KVNucleus::GetBindingEnergy(Int_t z, Int_t a) const
{
   //Returns ground state binding energy in MeV for this nucleus.
   //The convention is : binding energy is positive if nucleus is bound.
   //If optional arguments (z,a) are given we return the binding energy for the
   //required nucleus.
   //If the nucleus is not included in the mass table, an extrapolated value
   //using KVNucleus::LiquidDrop_BrackGuet is returned.

   CheckZAndA(z, a);

   return a ==
          0 ? 0. : (z * GetMassExcess(1, 1) + (a - z) * GetMassExcess(0, 1) -
                    GetMassExcess(z, a));
}

Double_t KVNucleus::GetLiquidDropBindingEnergy(Int_t z, Int_t a) const
{
   // Returns ground state binding energy in MeV for this nucleus calculated from Brack & Guet
   // liquid drop formula (see KVNucleus::LiquiDrop_BrackGuet).
   // The convention is : binding energy is positive if nucleus is bound.
   // If optional arguments (z,a) are given we return the binding energy for the
   // required nucleus.

   CheckZAndA(z, a);

   return a ==
          0 ? 0. : (z * GetMassExcess(1, 1) + (a - z) * GetMassExcess(0, 1) -
                    GetExtraMassExcess(z, a));
}

//________________________________________________________________________________________

Double_t KVNucleus::GetBindingEnergyPerNucleon(Int_t z, Int_t a) const
{
   //Returns binding energy in MeV/A for this nucleus.

   CheckZAndA(z, a);

   if (a == 0) return 0;
   return GetBindingEnergy(z, a) / a;
}
//________________________________________________________________________________________

Double_t KVNucleus::GetEnergyPerNucleon() const
{
   //
   //Returns kinetic energy of nucleus per nucleon (in MeV/nucleon, donc)
   //
   return GetA() ? GetEnergy() / GetA() : GetEnergy();
}

//________________________________________________________________________________________

Double_t KVNucleus::GetAMeV() const
{
   //
   //Returns kinetic energy of nucleus per nucleon (in MeV/nucleon, donc)
   //
   return GetEnergyPerNucleon();
}

//________________________________________________________________________________________

KVNumberList KVNucleus::GetKnownARange(Int_t zz, Double_t tmin) const
{
   //returns range of a known mass for a given element
   //according to the lifetime in seconds
   //tmin=0 (default) include all nuclei with known lifetime
   //tmin=-1 include also nuclei for which lifetime is unknown
   if (zz == -1) zz = GetZ();
   KVNumberList nla;
   if (zz == 0)
      nla.Add(1);
   else
      nla.SetMinMax(TMath::Max(zz, 1), 6 * TMath::Max(zz, 1));
   KVNumberList nlb;
   nla.Begin();
   while (!nla.End()) {
      Int_t aa = nla.Next();
      if (IsKnown(zz, aa) && (GetLifeTime(zz, aa) >= tmin)) nlb.Add(aa);
   }
   return nlb;
}

KVNumberList KVNucleus::GetMeasuredARange(Int_t zz) const
{
   //returns range of a measured mass for a given element

   if (zz == -1) zz = GetZ();
   KVNumberList nla;
   if (zz == 0)
      nla.Add(1);
   else
      nla.SetMinMax(TMath::Max(zz, 1), 6 * TMath::Max(zz, 1));
   KVNumberList nlb;
   nla.Begin();
   while (!nla.End()) {
      Int_t aa = nla.Next();
      if (GetMassExcessPtr(zz, aa) &&  GetMassExcessPtr(zz, aa)->IsMeasured())
         nlb.Add(aa);
   }
   return nlb;

}

const Char_t* KVNucleus::GetIsotopesList(Int_t zmin, Int_t zmax, Double_t tmin) const
{
   //returns list of isotopes separated by commas
   //for exemple 1H,2H,3H
   //according to the charge range and the lifetime in seconds
   //
   static KVString list;
   KVNucleus nn;
   KVNumberList nla;
   list = "";
   for (Int_t zz = zmin; zz <= zmax; zz += 1) {
      nla = GetKnownARange(zz, tmin);
      nla.Begin();
      while (!nla.End()) {
         Int_t aa = nla.Next();
         nn.SetZandA(zz, aa);
         list += nn.GetSymbol();
         list += ",";
      }
   }
   return list.Data();
}

//________________________________________________________________________________________

Int_t KVNucleus::GetAWithMaxBindingEnergy(Int_t zz)
{

   if (zz == -1) zz = GetZ();
   KVNumberList nla = GetKnownARange(zz);
   nla.Begin();
   Double_t emax = 0;
   Int_t amax = 0;
   while (!nla.End()) {
      Int_t aa = nla.Next();
      if (GetBindingEnergyPerNucleon(zz, aa) > emax) {
         emax = GetBindingEnergyPerNucleon(zz, aa);
         amax = aa;
      }
   }
   return amax;

}
//___________________________________________________________________________//

void KVNucleus::AddGroup_Withcondition(const Char_t* groupname, KVParticleCondition* pc)
{
   // implementation of AddGroup(const Char_t* groupname,KVParticleCondition* pc)
   // Can be overriden in child classes [unlike
   // KVParticle::AddGroup(const Char_t* groupname,KVParticleCondition* pc), which cannot]

   if (pc) {
      //pc->SetParticleClassName(this->IsA()->GetName());
      if (pc->Test(this)) AddGroup_Sanscondition(groupname);
   }
}

//________________________________________________________________________________________

KVNucleus& KVNucleus::operator=(const KVNucleus& rhs)
{
   //KVNucleus assignment operator.

   if (&rhs != this) {
      rhs.Copy(*this);
   }
   return *this;
}

//________________________________________________________________________________________

KVNucleus KVNucleus::operator+(const KVNucleus& rhs)
{
   // KVNucleus addition operator.
   // Add two nuclei together to form a compound nucleus whose Z, A, momentum
   // and excitation energy are calculated from energy and momentum conservation.

   KVNucleus& lhs = *this;
   Int_t ztot = lhs.GetZ() + rhs.GetZ();
   Int_t atot = lhs.GetA() + ((KVNucleus&) rhs).GetA();
   KVNucleus CN(ztot, atot);

   Double_t etot = lhs.E() + rhs.E();
   TVector3 ptot = lhs.GetMomentum() + rhs.GetMomentum();
   TLorentzVector q(ptot, etot);
   CN.Set4Mom(q);

   return CN;

}

//________________________________________________________________________________________

KVNucleus KVNucleus::operator-(const KVNucleus& rhs)
{
   // KVNucleus subtraction operator.
   // If the LHS is a compound nucleus and the RHS an emitted nucleus
   // (which may or may not be excited) then the result of the subtraction
   // is the residual nucleus, with recoil and residual excitation calculated
   // by conservation laws.

   KVNucleus& lhs = *this;
   Int_t zres = lhs.GetZ() - rhs.GetZ();
   Int_t ares = lhs.GetA() - ((KVNucleus&) rhs).GetA();
   Double_t eres = lhs.E() - rhs.E();
   TVector3 pres = lhs.GetMomentum() - rhs.GetMomentum();

   if (zres < 0 || ares < 0 || eres < 0) {
      Warning("operator-(const KVNucleus &rhs)",
              "Cannot subtract nuclei, resulting Z=%d A=%d E=%lf", zres, ares, eres);
      KVNucleus RES;
      RES.Clear();
      return RES;
   }
   else {
      KVNucleus RES(zres, ares);       //mass of nucleus includes mass excess
      TLorentzVector q(pres, eres);
      RES.Set4Mom(q);
      return RES;
   }
}

//________________________________________________________________________________________

KVNucleus& KVNucleus::operator+=(const KVNucleus& rhs)
{
   //KVNucleus addition and assignment operator.

   KVNucleus temp = (*this) + rhs;
   (*this) = temp;
   return *this;
}

//________________________________________________________________________________________

KVNucleus& KVNucleus::operator-=(const KVNucleus& rhs)
{
   //KVNucleus subtraction and assignment operator.

   KVNucleus temp = (*this) - rhs;
   (*this) = temp;
   return *this;
}

//________________________________________________________________________________________

Double_t KVNucleus::LiquidDrop_BrackGuet(UInt_t aa, UInt_t zz)
{
   //Liquid drop mass formula used for nuclei not in mass table (extrapolation).
   //Parameters are from Brack and Guet (copied from Simon code)

   Double_t A = (Double_t) aa;
   Double_t Z = (Double_t) zz;
   Double_t AVOL = 15.776;
   Double_t ASUR = -17.22;
   Double_t AC = -10.24;
   Double_t AZER = 8.;
   Double_t XJJ = -30.03;
   Double_t QQ = -35.4;
   Double_t C1 = -.737;
   Double_t C2 = 1.28;

   Double_t XNEU = A - Z;
   Double_t SI = (XNEU - Z) / A;
   Double_t X13 = TMath::Power(A, 1. / 3.);
   Double_t EE1 = C1 * Z * Z / X13;
   Double_t EE2 = C2 * Z * Z / A;
   Double_t AUX = 1. + (9. * XJJ / 4. / QQ / X13);
   Double_t EE3 = XJJ * A * SI * SI / AUX;
   Double_t EE4 =
      AVOL * A + ASUR * TMath::Power(A, 2. / 3.) + AC * X13 + AZER;
   Double_t TOTA = EE1 + EE2 + EE3 + EE4;
   return (939.55 * XNEU + 938.77 * Z - TOTA);
}

//________________________________________________________________________________________

Double_t KVNucleus::LiquidDrop_Weizsacker()
{
   //Liquid drop mass formula used for nuclei not in mass table (extrapolation).
   //Parameters are from Brack and Guet (copied from Simon code)

   Double_t av   = 1.531e+01;
   Double_t as   = 1.654e+01;
   Double_t ac   = 6.882e-01;
   Double_t aa   = 2.225e+01;
   Double_t ap   = 9.399e+00;
   Double_t kap  = 6.056e-01;

   Double_t eb = 0;
   eb += av * GetA();
   eb -= as * TMath::Power(GetA(), 2. / 3.);
   eb -= ac * GetZ() * (GetZ() - 1) / TMath::Power(GetA(), 1. / 3.);
   eb -= aa * TMath::Power(GetN() - GetZ(), 2.) / GetA();

   if (TMath::Even(GetA()))
      eb += ap * (TMath::Power(-1, GetN()) + TMath::Power(-1, GetZ())) / TMath::Power(GetA(), kap);

   return eb;

}

//_______________________________________________________________________________________

Int_t KVNucleus::Compare(const TObject* obj) const
{
   //For sorting lists of nuclei according to their Z
   //Largest Z appears first in list

   if (GetZ() > ((KVNucleus*) obj)->GetZ()) {
      return -1;
   }
   else if (GetZ() < ((KVNucleus*) obj)->GetZ()) {
      return 1;
   }
   else {
      if (GetA() == ((KVNucleus*) obj)->GetA()) return 0;
      else if (GetA() > ((KVNucleus*) obj)->GetA()) return -1;
      else  return 1;
   }
}
//_______________________________________________________________________________________

/*
TH2F* KVNucleus::GetKnownNucleiChart(KVString method)
{
        //Draw nuclei chart of tabulated nuclei and tagged as known in KaliVeda
        //The 2D histogram (AvsZ) has to be deleted by the user
        //Each content cell correponds to the method passed in argument of nucleus in MeV
        // Method Pattern has to be Double_t Method() or Double_t Method(obs = default value) in KVNucleus.h
TH2F* chart = new TH2F("nuclei_known_charts",method.Data(),
                                        121,-0.5,120.5,
                                        351,-0.5,350.5);
chart->SetXTitle("Atomic Number");
chart->SetYTitle("Mass Number");

TMethodCall *mt = new TMethodCall();
mt->InitWithPrototype(this->IsA(),Form("%s",method.Data()),"");
if (! mt->IsValid()) { delete mt; return 0; }
delete mt;
KVNucleus* ntemp = new KVNucleus();
for (Int_t zz=0;zz<120;zz+=1){
        for (Int_t aa=0;aa<350;aa+=1){
                if (this->IsKnown(zz,aa)){
                        mt = new TMethodCall();
                        mt->InitWithPrototype(ntemp->IsA(),Form("%s",method.Data()),"");
                        if (mt->ReturnType()==TMethodCall::kDouble){
                                ntemp->SetZ(zz); ntemp->SetA(aa);
                                Double_t ret; mt->Execute(ntemp,"",ret);
                                chart->Fill(zz,aa,ret);
                        }
                        delete mt;
                }
        }
}
delete ntemp;
return chart;

}
*/
//_______________________________________________________________________________________

Double_t KVNucleus::u(void)
{
   //Atomic mass unit in MeV
   //Reference: 2002 CODATA recommended values Reviews of Modern Physics 77, 1-107 (2005)
   return kAMU;
};

//_______________________________________________________________________________________

Double_t KVNucleus::DeduceEincFromBrho(Double_t Brho, Int_t ChargeState)
{
   //Retourne l'energie cintetique totale (MeV) du noyau pour
   //une valeur de Brho et d'etat de charge (Si 0-> Etat de charge=Z)
   Double_t C_mparns = KVNucleus::C() * 10;

   if (ChargeState == 0) ChargeState = GetZ();

   Double_t X = Brho * C_mparns * ChargeState;

   Double_t MassIon = GetMass() - ChargeState * KVNucleus::kMe;

   Double_t Result = TMath::Sqrt(MassIon * MassIon + X * X) - MassIon;

   return Result;

}

Double_t KVNucleus::GetRelativeVelocity(KVNucleus* nuc)
{
   // Return the reltive velocity between nuc and this in cm/ns.
   if (!nuc) return 0.;
   return (GetVelocity() - nuc->GetVelocity()).Mag();
}

Double_t KVNucleus::GetFissionTKE(const KVNucleus* nuc, Int_t formula) const
{
   // Average or most probable Total Kinetic Energy [MeV] expected for fission based on various systematics
   // for fission of highly-excited nuclei produced in heavy-ion reactions.
   // If nuc=0, this method returns the TKE for symmetric fission of this nucleus.
   // Else, it returns the expected TKE considering that nuc and the current nucleus arise
   // from the fisson of a compound nucleus.
   // - kItkis1998: M.G. Itkis & A. Ya. Rusanov, Phys. Part. Nucl. 29, 160 (1998)
   // - kDefaultFormula = kHinde1987: D. Hinde, J. Leigh, J. Bokhorst, J. Newton, R. Walsh, and J. Boldeman, Nuclear Physics A 472, 318 (1987).
   // - kViola1985: V. E. Viola, K. Kwiatkowski, and M. Walker, Physical Review C 31, 1550 (1985).
   // - kViola1966: V. E. Viola, Jr. , Nuclear Data Sheets. Section A 1, 391 (1965).

   Double_t Ztot = GetZ();
   Double_t Atot = GetA();
   if (nuc) {
      Ztot += nuc->GetZ();
      Atot += nuc->GetA();
   }
   Double_t tke = 0;
   switch (formula) {
      case kDefaultFormula:
      case kHinde1987:
         if (nuc) tke = TKE_Hinde1987(GetZ(), GetA(), nuc->GetZ(), nuc->GetA());
         else    tke = TKE_Hinde1987(GetZ() * 0.5, GetA() * 0.5, GetZ() - (GetZ() * 0.5), GetA() - (GetA() * 0.5));
         break;

      case kViola1985:
         tke = TKE_Viola1985(Ztot, Atot);
         break;

      case kViola1966:
         tke = TKE_Viola1966(Ztot, Atot);
         break;

      case kItkis1998:
         tke = TKE_Itkis1998(Ztot, Atot);
         break;
   }

   return tke;
}

Double_t KVNucleus::GetQFasymTKE(KVNucleus* target)
{
   // <TKE> of asymmetric QuasiFission fragments (for the fragment mass where the QFasym yield is maximal)
   // E.M. Kozulin et al PHYSICAL REVIEW C 90, 054608 (2014)
   // This depends on the entrance channel: this nucleus is assumed to be the projectile,
   // while the target is given as argument.

   return TKE_Kozulin2014(GetZ(), target->GetZ(), GetA(), target->GetA());
}

Double_t KVNucleus::GetFissionVelocity(KVNucleus* nuc, Int_t formula)
{
   // Average/most probable relative velocity [cm/ns] expected for fission based on various systematics
   // for fission of highly-excited nuclei produced in heavy-ion reactions.
   // If nuc=0, this method returns the relative velocity expected for the symmetric fission of this nucleus.
   // Else, it returns the expected relative velocity considering that nuc and the current nucleus arise
   // from the fisson of a compound nucleus.
   // - kItkis1998: M.G. Itkis & A. Ya. Rusanov, Phys. Part. Nucl. 29, 160 (1998)
   // - kDefaultFormula = kHinde1987: D. Hinde, J. Leigh, J. Bokhorst, J. Newton, R. Walsh, and J. Boldeman, Nuclear Physics A 472, 318 (1987).
   // - kViola1985: V. E. Viola, K. Kwiatkowski, and M. Walker, Physical Review C 31, 1550 (1985).
   // - kViola1966: V. E. Viola, Jr. , Nuclear Data Sheets. Section A 1, 391 (1965).

   Double_t vrel = 0;
   Double_t mu = 0;
   if (nuc) {
      mu = nuc->GetMass() * GetMass() / (nuc->GetMass() + GetMass());
   }
   else {
      KVNucleus ff1(0.5 * GetZ(), 0.5 * GetA());
      KVNucleus ff2(GetZ() - ff1.GetZ(), GetA() - ff1.GetA());
      mu = ff1.GetMass() * ff2.GetMass() / (ff1.GetMass() + ff2.GetMass());
   }

   Double_t TKE = GetFissionTKE(nuc, formula);
   vrel = sqrt(2 * TKE / mu) * C();

   return vrel;
}

Double_t KVNucleus::TKE_Hinde1987(Double_t z1, Double_t a1, Double_t z2, Double_t a2)
{
   // from: D. Hinde, J. Leigh, J. Bokhorst, J. Newton, R. Walsh, and J. Boldeman, Nuclear Physics A 472, 318 (1987)
   // According to the authors, an extension to asymmetric fission based on TKE_Viola1985
   return 0.755 * z1 * z2 / (pow(a1, 1 / 3.) + pow(a2, 1 / 3.)) + 7.3;
}

Double_t KVNucleus::TKE_Viola1985(Double_t z, Double_t a)
{
   // from: V. E. Viola, K. Kwiatkowski, and M. Walker, Physical Review C 31, 1550 (1985).
   Double_t za = pow(z, 2) / pow(a, 1. / 3.);
   return 0.1189 * za + 7.3;
}

Double_t KVNucleus::TKE_Viola1966(Double_t z, Double_t a)
{
   // from: V. E. Viola, Jr., Nuclear Data Sheets. Section A 1, 391 (1965).
   Double_t za = pow(z, 2) / pow(a, 1. / 3.);
   return 0.1071 * za + 22.2;
}

Double_t KVNucleus::TKE_Itkis1998(Double_t z, Double_t a)
{
   // from: M.G. Itkis & A. Ya. Rusanov, Phys. Part. Nucl. 29, 160 (1998)
   //  Compared to Viola systematics, only heavy-ion induced fission is considered
   // A change of slope is observed for Z**2/A**1/3 > 900

   Double_t za = pow(z, 2) / pow(a, 1. / 3.);
   if (za < 900)
      return 0.131 * za;
   return 0.104 * za + 24.3;
}

Double_t KVNucleus::TKE_Kozulin2014(Double_t zp, Double_t zt, Double_t ap, Double_t at)
{
   // <TKE> of asymmetric QuasiFission fragments (for the fragment mass where the QFasym yield is maximal)
   // E.M. Kozulin et al PHYSICAL REVIEW C 90, 054608 (2014)

   return 39.43 + .085 * pow(zp + zt, 2) / pow(ap + at, 1. / 3.);
}

//_______________________________________________________________________________________

Bool_t KVNucleus::IsStable(Double_t min_lifetime) const
{
   // Returns kTRUE if this nucleus is stable.
   // Definition of stable:
   //   if the natural abundance is defined (look up in Abundance table)
   // OR
   //   if lifetime is > min_lifetime
   if (GetAbundance() > 0.) return kTRUE;
   KVLifeTime* ptr = GetLifeTimePtr();
   return (ptr && !ptr->IsAResonance() && ptr->GetValue() > min_lifetime);
}

//_______________________________________________________________________________________

Bool_t KVNucleus::IsResonance() const
{
   // Returns kTRUE if this nucleus is a resonance.
   // In this case GetWidth() returns the width in MeV.
   KVLifeTime* ptr = GetLifeTimePtr();
   return (ptr && ptr->IsAResonance());
}

//_______________________________________________________________________________________

Double_t KVNucleus::GetWidth() const
{
   // Returns width of resonance in MeV, if this nucleus
   // is indeed a resonance (IsResonance() returns kTRUE).
   KVLifeTime* ptr = GetLifeTimePtr();
   return ((ptr && ptr->IsAResonance()) ? ptr->GetValue() : 0.0);
}

//_______________________________________________________________________________________

Double_t KVNucleus::GetNaturalA(Int_t Z) const
{
   // Calculate and return the effective mass number of element Z
   // taking into account the abundance of naturally-occurring isotopes

   KVNumberList isotopes = GetKnownARange(Z);
   isotopes.Begin();
   Double_t Aeff = 0, wtot = 0;
   while (!isotopes.End()) {

      int A = isotopes.Next();
      Double_t abundance = GetAbundance(Z, A) / 100.;
      if (abundance > 0.) {
         Aeff += A * abundance;
         wtot += abundance;
      }

   }
   if (wtot > 0) Aeff /= wtot;
   return Aeff;
}

//-------------------------
Double_t KVNucleus::ShimaChargeState(Int_t Ztarget) const
//-------------------------
{
   //Nuclear Instruments and Methods 200 (1982) 605-608
   //Shima et al
   // "The present formula is useful for the collision range"
   // Zprojectile>=8
   // 4<=Ztarget<=79
   // Eproj<=6 MeV/A
   // Precision DeltaQ/Zproj <0.04.
   //

   //v=sqrt((2*E*1.6022)/(A*1.66054))*10.;
   //X=v/((3.6)*pow(Z,0.45));

   Double_t vel = GetVelocity().Mag();   // (cm/ns)
   vel *= 10; //  (mm/ns)
   Double_t X = vel / ((3.6) * pow(GetZ(), 0.45));

   Double_t Q = GetZ() * (1 - exp(-1.25 * X + 0.32 * TMath::Power(X, 2.) - 0.11 * TMath::Power(X, 3.)));
   Q *= (1 - 0.0019 * (Ztarget - 6) * TMath::Sqrt(X) + 0.00001 * TMath::Power(Ztarget - 6, 2.) * X); //Correction respect to the carbon

   return Q;

}

//-------------------------
Double_t KVNucleus::ShimaChargeStatePrecision() const
//-------------------------
{
   return 0.04 * GetZ();
}
