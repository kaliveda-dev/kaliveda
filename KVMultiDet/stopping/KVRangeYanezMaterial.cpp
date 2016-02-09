//Created by KVClassFactory on Thu Sep 27 14:50:08 2012
//Author: John Frankland,,,

#include "KVRangeYanezMaterial.h"
#include "TF1.h"
#include "TEnv.h"
#include "KVUnits.h"
#include "KVNameValueList.h"
#include "range.h"
#include "Riostream.h"
#include "KVConfig.h"
using namespace std;

//Int_t KVRangeYanezMaterial::fTableType = 1;//Hubert-Bimbot-Gauvin, valid for 2.5<E/A<100 MeV

ClassImp(KVRangeYanezMaterial)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVRangeYanezMaterial</h2>
<h4>Description of absorber for the Range dE/dx and range library (Ricardo Yanez)</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVRangeYanezMaterial::KVRangeYanezMaterial()
{
   // Default constructor
}

KVRangeYanezMaterial::KVRangeYanezMaterial(
   const KVIonRangeTable* t, const Char_t* name, const Char_t* symbol, const Char_t* state, Double_t density,
   Int_t Z, Int_t A)
   :    KVIonRangeTableMaterial(t, name, symbol, state, density, Z, A)
{
   // Create material (single-element absorber)
   fTableType = 1; // Hubert for E/A>2.5MeV, switches to Northcliffe for <2.5AMeV.
   fNelem = 1;
   iabso = 0;
   fAbsorb[0].z  = Z;
   fAbsorb[0].a = A;
   fAbsorb[0].w = A;

   MakeFunctionObjects();
}

//________________________________________________________________

KVRangeYanezMaterial::KVRangeYanezMaterial(const KVRangeYanezMaterial& obj)
   : KVIonRangeTableMaterial()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVRangeYanezMaterial::~KVRangeYanezMaterial()
{
   // Destructor
}

//________________________________________________________________

void KVRangeYanezMaterial::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVRangeYanezMaterial::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVIonRangeTableMaterial::Copy(obj);
   //KVRangeYanezMaterial& CastedObj = (KVRangeYanezMaterial&)obj;
}

void KVRangeYanezMaterial::MakeFunctionObjects()
{
   fDeltaE = new TF1(Form("KVRangeYanezMaterial:%s:EnergyLoss", GetType()), this,
                     &KVRangeYanezMaterial::DeltaEFunc,
                     0., 1.e+03, 0, "KVRangeYanezMaterial", "DeltaEFunc");
   fDeltaE->SetNpx(100);
   fRange = new TF1(Form("KVRangeYanezMaterial:%s:Range", GetType()), this,
                    &KVRangeYanezMaterial::RangeFunc,
                    0., 1.e+03, 0, "KVRangeYanezMaterial", "RangeFunc");
   fRange->SetNpx(100);
   fEres = new TF1(Form("KVRangeYanezMaterial:%s:ResidualEnergy", GetType()), this,
                   &KVRangeYanezMaterial::EResFunc,
                   0., 1.e+03, 0, "KVRangeYanezMaterial", "EResFunc");
   fEres->SetNpx(100);
}

Double_t KVRangeYanezMaterial::DeltaEFunc(Double_t* E, Double_t* p)
{
   // Function parameterising the energy loss of charged particles in this material.
   // This is simply an interface to the passage() function of the Range C library.
   // The incident energy E[0] is given in MeV.
   // The energy loss is calculated in MeV.
   // To avoid divergences as E->0, for any incident energy E<=1.e-3MeV (i.e. 1keV)
   // we return dE=E i.e. all particles with E<=1keV are stopped.

   return (E[0] - EResFunc(E, p));
}

Double_t KVRangeYanezMaterial::EResFunc(Double_t* E, Double_t*)
{
   // Function parameterising the residual energy of charged particles after traversing this material.
   // This is simply an interface to the passage() function of the Range C library.
   // The incident energy E[0] is given in MeV.
   // The residual energy is calculated in MeV.
   // To avoid divergences as E->0, for any incident energy E<=1.e-3MeV (i.e. 1keV)
   // we return Eres=0 i.e. all particles with E<=1keV are stopped.

   if (E[0] < 1.e-3) return 0.0;
   return passage(fTableType, Zp, Ap, iabso, fAbsorb[0].z, fAbsorb[0].a, E[0], thickness / KVUnits::mg, &error);
}

Double_t KVRangeYanezMaterial::RangeFunc(Double_t* E, Double_t*)
{
   // Function parameterising the range of charged particles in this material.
   // This is simply an interface to the rangen() function of the Range C library.
   // The incident energy E[0] is given in MeV.
   // The range is calculated in g/cm**2.
   // To avoid divergences as E->0, for any incident energy E<=1.e-3MeV (i.e. 1keV)
   // we return range=0 i.e. all particles with E<=1keV are stopped.

   if (E[0] < 1.e-3) return 0.;
   Double_t R = rangen(fTableType, Zp, Ap, iabso, fAbsorb[0].z, fAbsorb[0].a, E[0]);
   return (R * KVUnits::mg);
}

void KVRangeYanezMaterial::PrepareRangeLibVariables(Int_t Z, Int_t A)
{
   nelem = fNelem;
#ifdef WITH_MODIFIED_RANGE_YANEZ
   is_gas = (int)IsGas(); // special treatment for effective charge in gases (M.F. Rivet, R. Bimbot et al)
#endif
   if (iabso < 0) {
      //cout << "nelem="<<nelem<<endl;
      for (register int k = 0; k < fNelem; k++) {
         absorb[k].z = fAbsorb[k].z;
         absorb[k].a = fAbsorb[k].a;
         absorb[k].w = fAbsorb[k].w;
         //cout << k << " " << absorb[k].z << " " << absorb[k].a << " " << absorb[k].w << endl;
      }
   }
   Zp = Z;
   Ap = A;
}

TF1* KVRangeYanezMaterial::GetDeltaEFunction(Double_t e, Int_t Z, Int_t A, Double_t)
{
   // Return function giving energy loss (in MeV) as a function of incident energy (in MeV) for
   // charged particles (Z,A) traversing (or not) the thickness e (in g/cm**2) of this material.
   // isotopic mass isoAmat argument is not used.

   PrepareRangeLibVariables(Z, A);
   thickness = e;
   fDeltaE->SetRange(0, 400 * Ap);
   return fDeltaE;
}

TF1* KVRangeYanezMaterial::GetEResFunction(Double_t e, Int_t Z, Int_t A, Double_t)
{
   // Return function giving residual energy (in MeV) as a function of incident energy (in MeV) for
   // charged particles (Z,A) traversing (or not) the thickness e (in g/cm**2) of this material.
   // isotopic mass isoAmat argument is not used.

   PrepareRangeLibVariables(Z, A);
   thickness = e;
   fEres->SetRange(0, 400 * Ap);
   return fEres;
}

TF1* KVRangeYanezMaterial::GetRangeFunction(Int_t Z, Int_t A, Double_t)
{
   // Return function giving range (in g/cm**2) as a function of incident energy (in MeV) for
   // charged particles (Z,A) traversing this material.
   // isotopic mass isoAmat argument is not used.

   PrepareRangeLibVariables(Z, A);
   // Yanez' range functions tend to have a (negative) minimum at very low energy,
   // below which the range diverges towards +infty at E=0.
   // Therefore we limit the range of the function to E>=Emin where Emin
   // is the energy for which the minimum occurs
   Double_t minX = fRange->GetMinimumX(1.e-6, 400 * Ap);
   fRange->SetRange(TMath::Max(1.e-6, minX), 400 * Ap);
   return fRange;
}

void KVRangeYanezMaterial::Initialize()
{
   KVIonRangeTableMaterial::Initialize();
   if (IsCompound()) {
      fNelem = 0;
      iabso = -1;
      TIter next(fComposition);
      KVNameValueList* nvl;
      while ((nvl = (KVNameValueList*)next())) {
         fAbsorb[fNelem].z = nvl->GetIntValue("Z");
         fAbsorb[fNelem].a = nvl->GetIntValue("A");
         fAbsorb[fNelem].w = nvl->GetDoubleValue("Ar*Weight");
         fNelem++;
      }
   } else if (IsMixture()) {
      fNelem = 0;
      iabso = -1;
      TIter next(fComposition);
      KVNameValueList* nvl;
      while ((nvl = (KVNameValueList*)next())) {
         fAbsorb[fNelem].z = nvl->GetIntValue("Z");
         fAbsorb[fNelem].a = nvl->GetIntValue("A");
         fAbsorb[fNelem].w = nvl->GetDoubleValue("Ar*Weight");
         fNelem++;
      }
   }
}

Double_t KVRangeYanezMaterial::GetEIncFromEResOfIon(Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t)
{
   // Overrides KVIonRangeTableMaterial method to use the egassap() function of  the Range C library.
   // Calculates incident energy (in MeV) of an ion (Z,A) with residual energy Eres (MeV) after thickness e (in g/cm**2).
   // isotopic mass isoAmat argument is not used.
   PrepareRangeLibVariables(Z, A);
   return egassap(fTableType, Zp, Ap, iabso, fAbsorb[0].z, fAbsorb[0].a, e / KVUnits::mg, Eres, &error);
}
