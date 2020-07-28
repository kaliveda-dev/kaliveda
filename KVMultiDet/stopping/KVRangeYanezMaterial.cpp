//Created by KVClassFactory on Thu Sep 27 14:50:08 2012
//Author: John Frankland,,,

#include "KVRangeYanezMaterial.h"
#include "TF1.h"
#include "TEnv.h"
#include "KVUnits.h"
#include "KVNameValueList.h"
#include "Riostream.h"
#include "KVConfig.h"

#include <KVNucleus.h>
using namespace std;

namespace range {
# define NELMAX 10
   int nelem;
   int is_gas = 0;
   struct elem {
      int z;
      int a;
      double w;
   } absorb[NELMAX];

   double passage(int icorr, int zp, int ap, int iabso, int zt, int at,
                  double ein, double t, double* err);

   double egassap(int icorr, int zp, int ap, int iabso, int zt, int at,
                  double t, double eut, double* err);

   double thickn(int icorr, int zp, int ap, int iabso, int zt, int at,
                 double ein, double delen);

   double rangen(int icorr, int zp, int ap, int iabso, int zt, int at,
                 double ein);
}
//Int_t KVRangeYanezMaterial::fTableType = 1;//Hubert-Bimbot-Gauvin, valid for 2.5<E/A<100 MeV

ClassImp(KVRangeYanezMaterial)

using namespace range;

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
   is_gas = (int)IsGas(); // special treatment for effective charge in gases (M.F. Rivet, R. Bimbot et al)
   if (iabso < 0) {
      //cout << "nelem="<<nelem<<endl;
      for (int k = 0; k < fNelem; k++) {
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
   }
   else if (IsMixture()) {
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

void KVRangeYanezMaterial::SaveMaterial(ofstream& matfile)
{
   // Write definition of material in a file in the directory
   //
   //  $(WORKING_DIR)/range_yanez/[name].dat
   //
   // All files in this directory are read when the table is initialised

   matfile << "// " << GetName() << " (" << GetSymbol() << ") generated ";
   TDatime now;
   matfile << now.AsString() << endl << endl;
   if (IsCompound()) matfile << "COMPOUND" << endl;
   else if (IsMixture()) matfile << "MIXTURE" << endl;
   else matfile << "ELEMENT" << endl;
   matfile << "name=" << GetName() << endl;
   matfile << "symbol=" << GetSymbol() << endl;
   matfile << "state=";
   if (IsGas()) matfile << "gas" << endl;
   else {
      matfile << "solid" << endl;
      matfile << "density=" << GetDensity() << endl;
   }
   if (IsCompound() || IsMixture()) {
      matfile << "nelem=" << GetNElem() << endl;
      TString listname = (IsCompound() ? "Compound element %d" : "Mixture element %d");

      for (int nel = 1; nel <= GetNElem(); ++nel) {
         KVNameValueList* compo = (KVNameValueList*)fComposition->FindObject(Form(listname.Data(), nel));
         KVNucleus nuc(compo->GetIntValue("Z"), compo->GetIntValue("A"));
         matfile << nuc.GetSymbol() << " " << compo->GetIntValue("Natoms");
         if (IsMixture()) matfile << " " << compo->GetDoubleValue("Proportion");
         matfile << endl;
      }
   }
   matfile << endl;
}

namespace range {
   /*
     Author: Ricardo Yanez
     Copyright (c) 2005-2011 Ricardo Yanez <ricardo.yanez@calel.org>

     Calculates energy loss of an ion in an absorber by constructing
     range tables based on either the,

     Northcliffe-Schilling correlations  (E/A < 12 MeV/A)
     (L.C. Northcliffe, R.F. Schilling, Nucl. Data Tables A7, 233, 1970).

     or,

     Hubert-Bimbot-Gauvin correlations (2.5 < E/A < 100 MeV/A)
     (Atomic Data and Nuclear Data Tables, 1990, 46, pp. 1-213).

     License:

      This program is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 2 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program; if not, write to the Free Software
      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

   */
#define NMAX 4000
#define NSAV 2000
#define FMT 0.005

   struct elem cmpnd[NELMAX];
   int numel;

   int isw1 = 0;
   int isw2 = 0;
   int isw3 = 0;
   int isw4 = 0;
   int isw5 = 0;

   /*
     Calculate energy of ion after passage through an absorber foil.
   */
   double passage(int icorr, int zp, int ap, int iabso, int zt, int at,
                  double ein, double t, double* err)
   {

      void rangetab(int icorr, int zp, int ap, int iabso, int zt, int at,
                    double * em, double * r, int* n);
      unsigned int locate(double y[], int n, double x);
      double polint(double * xa, double * ya, int n, double x, double * dy);

      double em[NMAX], r[NMAX];

      double elin, elut, rin, rut, lerr;
      int n;

      int jj, jjj;

      /* check correlation */
      if (icorr == 0 && ein / ap > 12.0)
         icorr = 1;  /* switch to H-B-G */
      if (icorr == 1 && ein / ap <= 2.5)
         icorr = 0;  /* switch to N-S */

      rangetab(icorr, zp, ap, iabso, zt, at, &em[0], &r[0], &n);
      jjj = n - 3;

      elin = log10(ein / (double)ap);
      jj = locate(em, n, elin);
      if (jj > jjj) jj = jjj;
      rin = polint(&em[jj], &r[jj], 3, elin, &lerr);
      rut = rin - t;
      if (rut <= 0.0) {
         *err = 0.0;
         return 0.0;
      }
      else {
         jj = locate(r, n, rut);
         if (jj > jjj) jj = jjj;
         elut = polint(&r[jj], &em[jj], 3, rut, &lerr);
         *err = fabs(pow(10.0, elut - lerr * 3) - pow(10.0, elut + lerr * 3)) / pow(10.0, elut);
         return pow(10.0, elut) * ap;
      }
   }


   /*
     Calculate incoming energy of ion before passage
     through an absorber of thickness t.
   */
   double egassap(int icorr, int zp, int ap, int iabso, int zt, int at,
                  double t, double eut, double* err)
   {

      void rangetab(int icorr, int zp, int ap, int iabso, int zt, int at,
                    double * em, double * r, int* n);
      unsigned int locate(double y[], int n, double x);
      double polint(double * xa, double * ya, int n, double x, double * dy);

      double em[NMAX], r[NMAX];

      double elut, elin, eaut, rut, rin, lerr;
      int n;

      int jj, jjj;

      if (eut / (double)ap != 0.0) {
         if (icorr == 0 && eut / ap > 12.0)
            icorr = 1;  /* switch to H-B-G */
      }

      rangetab(icorr, zp, ap, iabso, zt, at, &em[0], &r[0], &n);
      jjj = n - 3;

      if (eut / (double)ap != 0.0) {
         elut = log10(eut / (double)ap);
         jj = locate(em, n, elut);
         if (jj > jjj) jj = jjj;
         rut = polint(&em[jj], &r[jj], 3, elut, &lerr);
      }
      else
         rut = 0.0;

      rin = rut + t;
      jj = locate(r, n, rin);
      if (jj > jjj) jj = jjj;
      elin = polint(&r[jj], &em[jj], 3, rin, &lerr);
      *err = fabs(pow(10.0, elin - lerr * 3) - pow(10.0, elin + lerr * 3)) / pow(10.0, elin);
      eaut = pow(10.0, elin);

      if (icorr == 0 && eaut > 12.0)
         printf("warning: Hubert-Bimbot-Gauvin correlations should be used in this case.\n");
      if (icorr == 1 && eaut <= 2.5)
         printf("Warning: Northcliffe-Schilling correlations should be used in this case.\n");

      return eaut * ap;
   }


   /*
     Calculate absorber thickness for a given energy decrement
   */
   double thickn(int icorr, int zp, int ap, int iabso, int zt, int at,
                 double ein, double delen)
   {

      void rangetab(int icorr, int zp, int ap, int iabso, int zt, int at,
                    double * em, double * r, int* n);
      unsigned int locate(double y[], int n, double x);
      double polint(double * xa, double * ya, int n, double x, double * dy);

      double em[NMAX], r[NMAX];

      double elin, elut, rin, rut, rerr;
      int n;

      int jj, jjj;

      if (icorr == 0 && ein / (double)ap > 12.0)
         icorr = 1;  /* switch to H-B-G */
      if (icorr == 1 && ein / (double)ap <= 2.5)
         icorr = 0;  /* switch to N-S */

      rangetab(icorr, zp, ap, iabso, zt, at, &em[0], &r[0], &n);
      jjj = n - 3;

      elin = log10(ein / (double)ap);
      jj = locate(em, n, elin);
      if (jj > jjj) jj = jjj;
      rin = polint(&em[jj], &r[jj], 3, elin, &rerr);
      if (ein - delen <= 0.0)
         rut = 0.0;
      else {
         elut = log10((ein - delen) / (double)ap);
         jj = locate(em, n, elut);
         if (jj > jjj) jj = jjj;
         rut = polint(&em[jj], &r[jj], 3, elut, &rerr);
      }
      return rin - rut;
   }


   /*
     Calculate the range of a projectile
   */
   double rangen(int icorr, int zp, int ap, int iabso, int zt, int at,
                 double ein)
   {

      void rangetab(int icorr, int zp, int ap, int iabso, int zt, int at,
                    double * em, double * r, int* n);
      unsigned int locate(double y[], int n, double x);
      double polint(double * xa, double * ya, int n, double x, double * dy);

      double em[NMAX], r[NMAX];

      double elin, rerr;
      int n;

      int jj, jjj;

      if (icorr == 0 && ein / (double)ap > 12.0)
         icorr = 1;  /* switch to H-B-G */
      if (icorr == 1 && ein / (double)ap <= 2.5)
         icorr = 0;  /* switch to N-S */

      rangetab(icorr, zp, ap, iabso, zt, at, &em[0], &r[0], &n);
      jjj = n - 3;

      elin = log10(ein / (double)ap);
      jj = locate(em, n, elin);
      if (jj > jjj) jj = jjj;
      return polint(&em[jj], &r[jj], 3, elin, &rerr);

   }


   /*
     Calculates a range table given projectile and absorber. Tables are
     saved in memory to speed up calculations. Up to NSAV such tables are
     saved.
   */
   void rangetab(int icorr, int zp, int ap, int iabso, int zt, int at,
                 double* em, double* r, int* n)
   {

      double dedx(int icorr, double e, int zp, int ap, int zt, int at);
      void def_absorber(int zt, int at, int iabso);

      double elog[62] = {
         -2.0000000000, -1.9030899870, -1.7958800173, -1.6989700043, -1.6020599913,
            -1.4948500217, -1.3979400087, -1.3010299957, -1.2218487496, -1.1549019600,
            -1.0969100130, -1.0457574906, -1.0000000000, -0.9030899870, -0.7958800173,
            -0.6989700043, -0.6020599913, -0.4948500217, -0.3979400087, -0.3010299957,
            -0.2218487496, -0.1549019600, -0.0969100130, -0.0457574906, 0.0000000000,
            0.0969100130, 0.2041199827, 0.3010299957, 0.3979400087, 0.5051499783,
            0.6020599913, 0.6989700043, 0.7781512504, 0.8450980400, 0.9030899870,
            0.9542425094, 1.0000000000, 1.0413926852, 1.0791812460, 1.1760912591,
            1.3010299957, 1.3979400087, 1.4771212547, 1.5440680444, 1.6020599913,
            1.6532125138, 1.6989700043, 1.7781512504, 1.8450980400, 1.9030899870,
            1.9542425094, 2.0000000000, 2.0413926852, 2.0791812460, 2.1760912591,
            2.3010299957, 2.3979400087, 2.4771212547, 2.5440680444, 2.6020599913,
            2.6532125138, 2.6989700043
         };

      int ntalel;
      double dedxt[NELMAX][NMAX];
      double wtot;

      int i, j, k;
      double est, elg, e;
      double rng, rval, rnow, rold;
      double etot, eold;
      double dedxnow;

      int jsav;
      static int isav = 0;
      static int icc[NSAV], ijj[NSAV], iab[NSAV];
      static int izp[NSAV], iap[NSAV], izt[NSAV], iat[NSAV];
      static double esav[NSAV][NMAX], rsav[NSAV][NMAX];


      /* check if table saved */
      if (isav > 0) {
         for (i = 0 ; i < isav ; i++) {
            jsav = 0;
            if (icorr == icc[i]) jsav++;
            if (iabso == iab[i]) jsav++;
            if (zp == izp[i]) jsav++;
            if (ap == iap[i]) jsav++;
            if (zt == izt[i]) jsav++;
            if (at == iat[i]) jsav++;
            if (jsav == 6) {
               k = ijj[i];
               for (j = 0 ; j < k ; j++) {
                  *(em + j) = esav[i][j];
                  *(r + j) = rsav[i][j];
               }
               *n = k;
               return;
            }
         }
      }

      if (isav > NSAV) {
         printf("WARNING in rangetab: NSAV too small\n");
         exit(0);
      }
      /* Save this call */
      icc[isav] = icorr;
      iab[isav] = iabso;
      izp[isav] = zp;
      iap[isav] = ap;
      izt[isav] = zt;
      iat[isav] = at;

      switch (icorr) {
         case 0:
            ntalel = 38;
            break;
         case 1:
            ntalel = 61;
            break;
         case 2:
            ntalel = 61;
            break;
         default:
            printf("No valid range correlation.\n");
            exit(0);
      }

      /* define absorber */
      def_absorber(zt, at, iabso);

      /* Compute a range table */
      est = 3 * elog[0];
      wtot = 0.0;
      for (i = 0 ; i < numel ; i++) {
         isw1 = isw2 = 0;
         zt = cmpnd[i].z;
         at = cmpnd[i].a;
         wtot += cmpnd[i].w;
         *n = 0;
         for (j = 1 ; j <= 7000 ; j++) {
            elg = FMT * (j - 1200);
            e = pow(exp(elg), log(10.0));
            if (elg >= est) {
               if (elg <= elog[ntalel]) {
                  dedxt[i][*n] = dedx(icorr, e, zp, ap, zt, at);
                  *(em + (*n)) = elg;
                  (*n)++;
               }
               else
                  break;
               if (*n > 3999) break;
            }
         }
      }

      ijj[isav] = *n;

      rng = 0.0;
      rold = 0.0;
      eold = 0.0;
      for (j = 0 ; j < *n ; j++) {
         elg = *(em + j);
         e = pow(exp(elg), log(10.0));
         etot = e * ap;
         dedxnow = 0.0;
         for (i = 0 ; i < numel ; i++)
            dedxnow += dedxt[i][j] * cmpnd[i].w;
         dedxnow /= wtot;
         rnow = 1.0 / dedxnow;
         rval = 0.5 * (rold + rnow) * (etot - eold);
         rng += rval;
         *(r + j) = rng;
         eold = etot;
         rold = rnow;
         esav[isav][j] = *(em + j);
         rsav[isav][j] = *(r + j);
      }
      isav++;
   }

   void dedxtab(int icorr, int zp, int ap, int iabso, int zt, int at,
                double e, double* tdedxe, double* tdedxn)
   {

      double ededx(double e, int zp, int zt);
      double ndedx(double e, int zp, int ap, int zt, int at);
      double ededxh(double e, int zp, int zt);
      void def_absorber(int zt, int at, int iabso);

      double dedxn[NELMAX], dedxe[NELMAX];
      double tw;
      double e_int;
      int i;

      def_absorber(zt, at, iabso);

      for (i = 0 ; i < numel ; i++) {
         isw1 = isw2 = 0;
         zt = cmpnd[i].z;
         at = cmpnd[i].a;
         if (e < 2.5) {
            dedxn[i] = ndedx(e, zp, ap, zt, at) * pow(zp, 2);
            dedxe[i] = ededx(e, zp, zt) * pow(zp, 2);
         }
         else {
            if (icorr == 1) {
               dedxn[i] = 0.0;
               dedxe[i] = ededxh(e, zp, zt);
            }
            else if (icorr == 0) {
               dedxn[i] = ndedx(e, zp, ap, zt, at) * pow(zp, 2);
               dedxe[i] = ededx(e, zp, zt) * pow(zp, 2);
            }
            else if (icorr == 2) {
               if (e < 12) {
                  /* interpolate */
                  e_int = (e - 2.5) / 9.5;
                  dedxn[i] = (1. - e_int) * ndedx(e, zp, ap, zt, at) * pow(zp, 2);
                  dedxe[i] = e_int * ededxh(e, zp, zt) + (1. - e_int) * ededx(e, zp, zt) * pow(zp, 2);
               }
               else {
                  dedxn[i] = 0.0;
                  dedxe[i] = ededxh(e, zp, zt);
               }
            }
         }
      }
      *tdedxn = *tdedxe = 0.0;
      tw = 0.0;
      for (i = 0 ; i < numel ; i++) {
         *tdedxn += dedxn[i] * cmpnd[i].w;
         *tdedxe += dedxe[i] * cmpnd[i].w;
         tw += cmpnd[i].w;
      }
      *tdedxn /= tw;
      *tdedxe /= tw;
   }


   void def_absorber(int zt, int at, int iabso)
   {

      int i;

      switch (iabso) {

         /* User defined */
         case -1:
            for (i = 0 ; i < nelem ; i++) {
               cmpnd[i].z = absorb[i].z;
               cmpnd[i].a = absorb[i].a;
               cmpnd[i].w = absorb[i].w;
            }
            numel = nelem;
            break;

         /* Single element */

         case 0:
            numel = 1;
            cmpnd[0].z = zt;
            cmpnd[0].a = at;
            cmpnd[0].w = at * 1;
            break;

         /* Solids */

         case 1: /* Mylar (C-10, H-8, O-4) */
            numel = 3;
            cmpnd[0].z = 6;
            cmpnd[0].a = 12;
            cmpnd[0].w = cmpnd[0].a * 10.;
            cmpnd[1].z = 1;
            cmpnd[1].a = 1;
            cmpnd[1].w = cmpnd[1].a * 8.;
            cmpnd[2].z = 8;
            cmpnd[2].a = 16;
            cmpnd[2].w = cmpnd[2].a * 4.;
            break;
         case 2: /* Polyethylene (C-2, H-4) */
            numel = 2;
            cmpnd[0].z = 6;
            cmpnd[0].a = 12;
            cmpnd[0].w = cmpnd[0].a * 2.;
            cmpnd[1].z = 1;
            cmpnd[1].a = 1;
            cmpnd[1].w = cmpnd[1].a * 4.;
            break;
         case 3: /* Polypropylene (C-3, H-6) */
            numel = 2;
            cmpnd[0].z = 6;
            cmpnd[0].a = 12;
            cmpnd[0].w = cmpnd[0].a * 3.;
            cmpnd[1].z = 1;
            cmpnd[1].a = 1;
            cmpnd[1].w = cmpnd[1].a * 6.;
            break;
         case 4: /* Kapton (H-10, C-22, N-2, O-5) */
            numel = 4;
            cmpnd[0].z = 1;
            cmpnd[0].a = 1;
            cmpnd[0].w = cmpnd[0].a * 10.;
            cmpnd[1].z = 6;
            cmpnd[1].a = 12;
            cmpnd[1].w = cmpnd[1].a * 22.;
            cmpnd[2].z = 7;
            cmpnd[2].a = 14;
            cmpnd[2].w = cmpnd[2].a * 2.;
            cmpnd[3].z = 8;
            cmpnd[3].a = 16;
            cmpnd[3].w = cmpnd[3].a * 5.;
            break;
         case 5: /* Cesium Iodine (CsI) */
            numel = 2;
            cmpnd[0].z = 55;
            cmpnd[0].a = 133;
            cmpnd[0].w = cmpnd[0].a * 1.;
            cmpnd[1].z = 53;
            cmpnd[1].a = 127;
            cmpnd[1].w = cmpnd[1].a * 1.;
            break;
         case 6: /* Sodium Iodine (NaI) */
            numel = 2;
            cmpnd[0].z = 11;
            cmpnd[0].a = 23;
            cmpnd[0].w = cmpnd[0].a * 1.;
            cmpnd[1].z = 53;
            cmpnd[1].a = 127;
            cmpnd[1].w = cmpnd[1].a * 1.;
            break;
         case 7: /* Aluminum Oxide (Al2O3) */
            numel = 2;
            cmpnd[0].z = 13;
            cmpnd[0].a = 27;
            cmpnd[0].w = cmpnd[0].a * 2.;
            cmpnd[1].z = 8;
            cmpnd[1].a = 16;
            cmpnd[1].w = cmpnd[1].a * 3.;
            break;
         case 8: /* Tin-Lead (Sn60/Pb40) */
            numel = 4;
            cmpnd[0].z = 50;
            cmpnd[0].a = 116;
            cmpnd[0].w = 0.206 * 60.;
            cmpnd[1].z = 50;
            cmpnd[1].a = 118;
            cmpnd[1].w = 0.340 * 60.;
            cmpnd[2].z = 50;
            cmpnd[2].a = 120;
            cmpnd[2].w = 0.454 * 60.;
            cmpnd[3].z = 82;
            cmpnd[3].a = 208;
            cmpnd[3].w = 40.;
            break;
         case 9: /* Natural Ni (Ni-nat) */
            numel = 5;
            cmpnd[0].z = 28;
            cmpnd[0].a = 58;
            cmpnd[0].w = cmpnd[0].a * 0.680769;
            cmpnd[1].z = 28;
            cmpnd[1].a = 60;
            cmpnd[1].w = cmpnd[1].a * 0.262231;
            cmpnd[2].z = 28;
            cmpnd[2].a = 61;
            cmpnd[2].w = cmpnd[2].a * 0.011399;
            cmpnd[3].z = 28;
            cmpnd[3].a = 62;
            cmpnd[3].w = cmpnd[3].a * 0.036345;
            cmpnd[4].z = 28;
            cmpnd[4].a = 64;
            cmpnd[4].w = cmpnd[4].a * 0.009256;
            break;

         /* Gases */

         case 100: /* Carbon Tetrafluoride (CF4) */
            numel = 2;
            cmpnd[0].z = 6;
            cmpnd[0].a = 12;
            cmpnd[0].w = cmpnd[0].a * 1.;
            cmpnd[1].z = 9;
            cmpnd[1].a = 19;
            cmpnd[1].w = cmpnd[1].a * 4.;
            is_gas = 1;
            break;
         case 101: /* Propane (H-8, C-3) */
            numel = 2;
            cmpnd[0].z = 1;
            cmpnd[0].a = 1;
            cmpnd[0].w = cmpnd[0].a * 8.;
            cmpnd[1].z = 6;
            cmpnd[1].a = 12;
            cmpnd[1].w = cmpnd[1].a * 3.;
            is_gas = 1;
            break;
         case 102: /* Butane (H-10, C-4) */
            numel = 2;
            cmpnd[0].z = 1;
            cmpnd[0].a = 1;
            cmpnd[0].w = cmpnd[0].a * 10.;
            cmpnd[1].z = 6;
            cmpnd[1].a = 12;
            cmpnd[1].w = cmpnd[1].a * 4.;
            is_gas = 1;
            break;
         case 103: /* Octane (H-18, C-8) */
            numel = 2;
            cmpnd[0].z = 1;
            cmpnd[0].a = 1;
            cmpnd[0].w = cmpnd[0].a * 18.;
            cmpnd[1].z = 6;
            cmpnd[1].a = 12;
            cmpnd[1].w = cmpnd[1].a * 8.;
            is_gas = 1;
            break;
         default:
            printf("No valid absorber data.\n");
            exit(0);
      }
   }


   /*
     Compute 1/(dE/dx) for a given energy E/A and projectile and target.
   */

   /* SMOOTHERSTEP = interpolating function with zero first- and
      second- derivatives at the end points */
#define SMOOTHERSTEP(x) ((x) * (x) * (x) * (3*(x)*(2*(x) - 5) + 10))

   double dedx(int icorr, double e, int zp, int ap, int zt, int at)
   {

      double ededx(double e, int zp, int zt);
      double ndedx(double e, int zp, int ap, int zt, int at);
      double ededxh(double e, int zp, int zt);

      double dedxn, dedxe, e_int, smoothe;
      static double emin = 0.1, emax = 2.5;

      if (icorr == 1 && e < 2.5) icorr = 0;
      if (icorr == 2) {
         if (e < emin) icorr = 0;
         else if (e > emax) icorr = 1;
      }
      switch (icorr) {
         case 0:
            dedxn = ndedx(e, zp, ap, zt, at);
            dedxe = ededx(e, zp, zt);
            return (dedxn + dedxe) * pow(zp, 2);
            break;
         case 1:
            return ededxh(e, zp, zt);
            break;
         case 2:   /* interpolate in range emin<e<emax */
            e_int = (e - emin) / (emax - emin);
            smoothe = SMOOTHERSTEP(e_int);
            return (smoothe * ededxh(e, zp, zt) +
                    (1. - smoothe) * (ndedx(e, zp, ap, zt, at) + ededx(e, zp, zt)) * pow(zp, 2));
            break;
         default:
            exit(0);
      }
   }


   /*
     Compute the "electrical" energy loss rate in any material (-dE/dx)/Z2
   */
   double ededx(double e, int zp, int zt)
   {

      void alion(int zp, double * dedxz2);
      void gfact(double * le, int zt, double * f);
      void mpyers(double * le, int zt, double * f);

      unsigned int locate(double y[], int n, double x);
      double polint(double * xa, double * ya, int n, double x, double * dy);

      double elog[42] = {-1.903090, -1.795880, -1.698970, -1.602060, -1.494850,
                         -1.397940, -1.301030, -1.221849, -1.154902, -1.096910,
                         -1.045757, -1.000000, -0.903090, -0.795880, -0.698970,
                         -0.602060, -0.494850, -0.397940, -0.301030, -0.221849,
                         -0.154902, -0.096910, -0.045757, 0.000000, +0.096910,
                         +0.204120, +0.301030, +0.397940, +0.505150, +0.602060,
                         +0.698970, +0.778151, +0.845098, +0.903090, +0.954243,
                         +1.000000, +1.041393, +1.079181, +1.301030, +1.602060,
                         +1.845098, +2.000000
                        };

      int zgases[11] = {1, 2, 7, 8, 9, 10, 17, 18, 36, 54, 86};

      int i, j;
      unsigned int jj;
      static double dedxz2[42];
      static double ak, a, ftargl;
      double b, ftarg, el, err;
      int gas;

      if (!isw1) {
         alion(zp, &dedxz2[0]);
         ak = (dedxz2[2] - dedxz2[0]) / (elog[2] - elog[0]);
         a = dedxz2[0] - ak * elog[0];
         //ftarg = 1.0;

         /* Is it a gas? */
         gas = 0;
         for (i = 0 ; i < 11 ; i++)
            if (zt == zgases[i]) gas = 1;

         /* Special case for gases */
         if (gas) {
            gfact(&elog[0], zt, &ftargl);
            dedxz2[0] += ftargl;
            for (j = 1 ; j < 42 ; j++) {
               gfact(&elog[j], zt, &ftarg);
               dedxz2[j] += ftarg;
            }
         }
         /* It is a solid */
         else {
            if (zt != 13) {
               mpyers(&elog[0], zt, &ftargl);
               dedxz2[0] += ftargl;
               for (j = 1 ; j < 42 ; j++) {
                  mpyers(&elog[j], zt, &ftarg);
                  dedxz2[j] += ftarg;
               }
            }
         }
         isw1 = 1;
      }
      el = log10(e);
      if (el < elog[0])
         b = a + ak * el + ftargl;
      else {
         jj = locate(elog, 42, el);
         if (jj > 39) jj = 39;
         b = polint(&elog[jj], &dedxz2[jj], 3, el, &err);
      }
      return pow(10.0, b);
   }


   /*
     Computes current value of nuclear stopping power for a
     Projectile of energy E (MeV/A), mass ap and charge zp in an
     absorber with mass at and charge zt using a function fit to
     the LSS universal nuclear stopping power curve.
   */
   double ndedx(double e, int zp, int ap, int zt, int at)
   {

      double zexpo, dedr;
      double x, y, xl, yl;

      /* Compute current x-coordinate from projectile and target data */
      zexpo = sqrt(pow(zp, 2.0 / 3.0) + pow(zt, 2.0 / 3.0));
      x = sqrt((e * ap * at / (double)(ap + at)) / (zp * zt * zexpo));

      /* Get log for polynomial fit */
      xl = log10(x);
      yl = -6.80959 + xl *
           (-6.60315 + xl * (-1.73474 + xl * xl * (0.04937 + xl * 0.00486)));
      y = pow(10.0, yl);

      /* Convert y to -dEPS/dRHO */
      dedr = y * zt * ap / (zp * at * (ap + at) * zexpo);
      return dedr;
   }

   /*
     Compute the "electrical" energy loss rate in any material
     (-dE/dx) above 2.5 MeV/A according to Hubert et al.
   */
   double ededxh(double e, int zp, int zt)
   {

      double s2az(double e, int zt);

      static double b, c, d;
      static double x1, x2, x3, x4;
      double xg1;

      /* Special case for He */
      if (zp == 2) {
         isw2 = 1;
         return s2az(e, zt) * 4.0;
      }

      if (!isw2) {
         if (is_gas) {
            /* If the element or compound is a gas
            we use the (unpublished) effective charge
            parametrisation for gaseous absorbers
            (M.F. Rivet, IPN Orsay, private communication).
             */
            b = 2.6878;
            c = 0.07452;
            d = 0.51216 + 0.5693 * exp(-0.002158 * zt);
            x2 = 11.109 + 0.1057 * log(zt);
            x3 = 0.5622 - 0.07901 * log(zt);
            x4 = 0.8276 - 0.03059 * log(zt);
         }
         else {
            if (zt == 4) {
               b = 2.0;
               c = 0.04369;
               d = 2.045;
               x2 = 7.0;
               x3 = 0.2643;
               x4 = 0.4171;
            }
            else {
               if (zt == 6) {
                  b = 1.91;
                  c = 0.03958;
                  d = 2.584;
                  x2 = 6.933;
                  x3 = 0.2433;
                  x4 = 0.3969;
               }
               else {
                  b = 1.658;
                  c = 0.0517;
                  d = 1.164 + 0.2319 * exp(-0.004302 * zt);
                  x2 = 8.144 + 0.09876 * log(zt);
                  x3 = 0.314 + 0.01072 * log(zt);
                  x4 = 0.5218 + 0.02521 * log(zt);
               }
            }
         }
         x1 = d + b * exp(-c * zp);
         isw2 = 1;
      }
      xg1 = 1.0 - x1 * exp(-x2 * pow(e, x3) / pow(zp, x4));
      return s2az(e, zt) * pow((xg1 * zp), 2);
   }


   /*
     Given E/A (MeV/A) and the material atomic number Z, A
     logarithm F is calculated that should be used to convert
     stopping power in Aluminium to stopping power of the material
     with atomic number Z by adding it to the log of -(1/Z2)(dE/dx)
     This routine is for solids only.
   */
   void mpyers(double* le, int zt, double* f)
   {

      void splie2(double x1a[], double x2a[], double * ya[],
                  int m, int n, double * y2a[]);
      void splin2(double x1a[], double x2a[], double * ya[], double * y2a[],
                  int m, int n, double x1, double x2, double * y);

      double za[12] = {4., 6., 13., 22., 28., 32., 40., 47., 63., 73., 79., 92.};

      double ea[38] = {0.0125, .016, .02, .025, .032, .04, .05, .06, .07, .08, .09, .1,
                       .125, .16, .2, .25, .32, .4, .5, .6, .7, .8, .9, 1., 1.25, 1.6, 2.,
                       2.5, 3.2, 4., 5., 6., 7., 8., 9.0, 10., 11., 12.0
                      };

      double fb[38][12] = {
         {1.6499, 1.3651, 1., .665, .5395, .4901, .454, .42, .2669, .2284, .21, .1799},
         {1.6501, 1.3652, 1., .6651, .54, .4901, .4541, .4201, .2671, .2285, .2099, .1801},
         {1.65, 1.365, 1., .6649, .54, .49, .454, .4199, .267, .2285, .2101, .1801},
         {1.65, 1.365, 1., .6649, .5401, .49, .4541, .42, .2669, .2286, .2105, .1801},
         {1.6482, 1.3662, 1., .6651, .541, .492, .454, .42, .267, .229, .2111, .1811},
         {1.6441, 1.3691, 1., .667, .544, .496, .457, .4231, .269, .2316, .2129, .1831},
         {1.638, 1.3729, 1., .67, .549, .4999, .461, .427, .271, .2354, .217, .1865},
         {1.632, 1.381, 1., .674, .554, .503, .465, .431, .276, .24, .2215, .1905},
         {1.625, 1.3901, 1., .678, .56, .508, .47, .436, .28, .245, .227, .195},
         {1.6181, 1.4021, 1., .683, .565, .5121, .473, .44, .285, .25, .231, .199},
         {1.611, 1.414, 1., .688, .57, .518, .477, .444, .291, .2545, .236, .2035},
         {1.6049, 1.427, 1., .693, .576, .522, .48, .449, .296, .259, .24, .207},
         {1.59, 1.467, 1., .704, .588, .533, .49, .459, .3075, .269, .2515, .2175},
         {1.569, 1.5251, 1., .716, .602, .546, .501, .47, .32, .281, .2635, .23},
         {1.548, 1.571, 1., .727, .615, .557, .511, .479, .332, .293, .275, .24},
         {1.523, 1.6021, 1., .739, .628, .57, .522, .489, .345, .305, .286, .25},
         {1.492, 1.621, 1., .753, .643, .587, .535, .5, .358, .318, .2985, .2625},
         {1.462, 1.623, 1., .762, .656, .6, .547, .511, .372, .331, .311, .273},
         {1.43, 1.607, 1., .773, .67, .615, .56, .522, .386, .344, .3235, .285},
         {1.402, 1.579, 1., .783, .682, .628, .57, .531, .397, .355, .3345, .297},
         {1.379, 1.548, 1., .79, .691, .637, .577, .538, .406, .363, .343, .304},
         {1.358, 1.514, 1., .797, .699, .647, .585, .545, .414, .372, .351, .312},
         {1.342, 1.483, 1., .802, .707, .656, .592, .551, .423, .38, .359, .32},
         {1.327, 1.453, 1., .809, .714, .663, .599, .557, .43, .387, .366, .327},
         {1.297, 1.389, 1., .82, .73, .68, .613, .569, .447, .403, .381, .341},
         {1.266, 1.327, 1., .835, .746, .697, .628, .58, .465, .421, .398, .357},
         {1.239, 1.293, 1., .847, .761, .714, .64, .593, .48, .438, .414, .373},
         {1.213, 1.271, 1., .859, .777, .73, .653, .606, .498, .453, .43, .39},
         {1.189, 1.256, 1., .87, .792, .745, .668, .618, .516, .472, .448, .407},
         {1.17, 1.246, 1., .879, .805, .758, .683, .63, .533, .488, .465, .423},
         {1.154, 1.237, 1., .886, .816, .77, .694, .642, .55, .502, .481, .439},
         {1.144, 1.23, 1., .892, .823, .78, .704, .651, .558, .515, .493, .453},
         {1.134, 1.225, 1., .896, .828, .787, .712, .659, .57, .524, .503, .463},
         {1.127, 1.22, 1., .9, .833, .792, .72, .665, .578, .533, .512, .472},
         {1.123, 1.215, 1., .903, .837, .797, .725, .671, .585, .54, .519, .48},
         {1.118, 1.21, 1., .907, .84, .801, .729, .677, .59, .547, .527, .487},
         {1.114, 1.207, 1., .91, .844, .805, .733, .683, .596, .554, .535, .493},
         {1.111, 1.203, 1., .912, .847, .809, .737, .688, .601, .56, .54, .498}
      };

      int i, j;
      static double lfb[38][12], y2b[38][12];
      static double* pfb[38], *py2b[38];
      static double lza[12], lea[38];
      double zl;

      if (!isw4) {
         for (i = 0 ; i < 38 ; i++) {
            lea[i] = log10(ea[i]);
            for (j = 0 ; j < 12 ; j++)
               lfb[i][j] = log10(fb[i][j]);
         }
         for (j = 0 ; j < 12 ; j++)
            lza[j] = log10(za[j]);
         for (i = 0 ; i < 38 ; i++) {
            pfb[i] = &(lfb[i])[0];
            py2b[i] = &(y2b[i])[0];
         }
         splie2(lza, lea, &pfb[0], 12, 38, &py2b[0]);
         isw4 = 1;
      }

      zl = log10(zt);
      if (*le < lea[0])
         *le = lea[0];
      splin2(lza, lea, &pfb[0], &py2b[0], 12, 38, zl, *le, f);
   }

   /*
     Given E/A (MeV/A) and the gas material atomic number Z, A
     logarithm F is calculated that should be used to convert
     stopping power in Aluminium to stopping power of the gas
     with atomic number Z by adding it to the log of -(1/Z2)(dE/dx)
     This routine is for gases only.
   */
   void gfact(double* le, int zt, double* f)
   {

      void splie2(double x1a[], double x2a[], double * ya[],
                  int m, int n, double * y2a[]);
      void splin2(double x1a[], double x2a[], double * ya[], double * y2a[],
                  int m, int n, double x1, double x2, double * y);
      unsigned int locate(double y[], int n, double x);
      double polint(double * xa, double * ya, int n, double x, double * dy);

      double za[9] = {1., 2., 7., 8., 10., 18., 36., 54., 86.};

      double ea[38] = {0.0125, .016, .02, .025, .032, .04, .05, .06, .07, .08, .09, .1,
                       .125, .16, .2, .25, .32, .4, .5, .6, .7, .8, .9, 1., 1.25, 1.6, 2.,
                       2.5, 3.2, 4., 5., 6., 7., 8., 9.0, 10., 11., 12.0
                      };

      double far[38] = {0.584, .5891, .595, .607, .6251, .6461, .671, .6970, .722, .747,
                        .771, .795, .851, .923, .985, 1.034, 1.0550, 1.051, 1.031, 1.001,
                        .964, .928, .894, .871, .843, .8350, .842, .862, .885, .9, .909,
                        .914, .918, .919, .9200, .92, .92, .9200
                       };

      double fa[38][9] = {
         {6.4213, 2.7396, 1.8149, 1.7055, 1.5153, 1., .6132, .4349, .2868},
         {6.2480, 2.6351, 1.7709, 1.6654, 1.4804, 1., .6195, .4415, .2931},
         {6.0166, 2.4705, 1.7091, 1.6102, 1.4403, 1., .6235, .4536, .3036},
         {5.7334, 2.2751, 1.6345, 1.5404, 1.3939, 1., .6393, .4661, .3378},
         {5.4557, 2.0702, 1.5344, 1.4608, 1.3343, 1., .6495, .4895, .3408},
         {5.1854, 1.8853, 1.4441, 1.3900, 1.2800, 1., .6625, .5078, .3607},
         {4.9329, 1.7348, 1.3695, 1.3323, 1.2400, 1., .6736, .5246, .3800},
         {4.7917, 1.6397, 1.3299, 1.2854, 1.2152, 1., .6800, .5351, .3931},
         {4.6953, 1.5900, 1.3020, 1.2645, 1.1994, 1., .6815, .5415, .4003},
         {4.6454, 1.5596, 1.2866, 1.2504, 1.1901, 1., .6855, .5462, .4056},
         {4.6042, 1.5446, 1.2775, 1.2451, 1.1854, 1., .6873, .5499, .4072},
         {4.6038, 1.5447, 1.2805, 1.2403, 1.1799, 1., .6906, .5497, .4076},
         {4.6652, 1.5571, 1.2868, 1.2397, 1.1845, 1., .6886, .5488, .4078},
         {4.7995, 1.6002, 1.3131, 1.2654, 1.2004, 1., .6858, .5460, .4052},
         {4.9848, 1.6548, 1.3553, 1.3015, 1.2305, 1., .6802, .5401, .3990},
         {5.2418, 1.7351, 1.4072, 1.3549, 1.2650, 1., .6760, .5300, .3897},
         {5.6304, 1.8379, 1.4948, 1.4265, 1.3052, 1., .6673, .5204, .3801},
         {5.9373, 1.9601, 1.5566, 1.4796, 1.3397, 1., .6613, .5148, .3749},
         {6.1881, 2.0563, 1.5975, 1.5199, 1.3598, 1., .6595, .5150, .3754},
         {6.3436, 2.1279, 1.6104, 1.5305, 1.3676, 1., .6633, .5175, .3786},
         {6.3693, 2.1888, 1.6100, 1.5301, 1.3693, 1., .6691, .5207, .3848},
         {6.3254, 2.2198, 1.6045, 1.5301, 1.3674, 1., .6767, .5280, .3922},
         {6.2639, 2.2326, 1.5995, 1.5246, 1.3557, 1., .6823, .5391, .4005},
         {6.1883, 2.2377, 1.5970, 1.5155, 1.3502, 1., .6889, .5488, .4076},
         {5.8955, 2.2076, 1.5718, 1.5006, 1.3333, 1., .7022, .5670, .4282},
         {5.4133, 2.1198, 1.5306, 1.4635, 1.3102, 1., .7174, .5832, .4515},
         {4.9170, 2.0155, 1.4941, 1.4228, 1.2874, 1., .7304, .5998, .4715},
         {4.3851, 1.8851, 1.4397, 1.3701, 1.2645, 1., .7424, .6183, .4919},
         {3.9549, 1.7571, 1.3933, 1.3299, 1.2350, 1., .7605, .6328, .5073},
         {3.6667, 1.6578, 1.3555, 1.3000, 1.2222, 1., .7667, .6456, .5222},
         {3.4983, 1.5896, 1.3300, 1.2805, 1.2068, 1., .7778, .6567, .5390},
         {3.4027, 1.5471, 1.3151, 1.2648, 1.1980, 1., .7812, .6641, .5471},
         {3.3442, 1.5185, 1.3050, 1.2582, 1.1895, 1., .7865, .6710, .5545},
         {3.3080, 1.4962, 1.3003, 1.2546, 1.1850, 1., .7867, .6746, .5604},
         {3.2717, 1.4870, 1.2946, 1.2500, 1.1826, 1., .7881, .6772, .5652},
         {3.2500, 1.4837, 1.2946, 1.2500, 1.1826, 1., .7902, .6793, .5685},
         {3.2282, 1.4804, 1.2945, 1.2500, 1.1826, 1., .7924, .6815, .5707},
         {3.2066, 1.4772, 1.2946, 1.2500, 1.1826, 1., .7946, .6837, .5728}
      };

      int i, j;
      unsigned int jj;
      static double lfa[38][9], y2a[38][9];
      static double* pfa[38], *py2a[38];
      static double lza[9], lea[38], lfar[38];
      double zl, fgl, fgal, err;

      if (!isw3) {
         for (i = 0 ; i < 38 ; i++) {
            lea[i] = log10(ea[i]);
            lfar[i] = log10(far[i]);
            for (j = 0 ; j < 9 ; j++)
               lfa[i][j] = log10(fa[i][j]);
         }
         for (j = 0 ; j < 9 ; j++)
            lza[j] = log10(za[j]);
         for (i = 0 ; i < 38 ; i++) {
            pfa[i] = &(lfa[i])[0];
            py2a[i] = &(y2a[i])[0];
         }
         splie2(lza, lea, &pfa[0], 9, 38, &py2a[0]);
         isw3 = 1;
      }
      zl = log10(zt);
      if (*le < lea[0])
         *le = lea[0];
      splin2(lza, lea, &pfa[0], &py2a[0], 9, 38, zl, *le, &fgl);
      jj = locate(lea, 38, *le);
      if (jj > 35) jj = 35;
      fgal = polint(&lea[jj], &lfar[jj], 3, *le, &err);
      *f = fgl + fgal;
   }

   /*
     Get corresponding vectors of log(E/A) and log((dE/dx)/Z2) for a
     specific ion with charge Z in Aluminium
   */
   void alion(int zp, double* dedxz2)
   {

      void alref(double e, double * lz, double * dedx);
      unsigned int locate(double y[], int n, double x);
      double polint(double * xa, double * ya, int n, double x, double * dy);

      double elog[42] = {-1.903090, -1.795880, -1.698970, -1.602060, -1.494850,
                         -1.397940, -1.301030, -1.221849, -1.154902, -1.096910,
                         -1.045757, -1.000000, -0.903090, -0.795880, -0.698970,
                         -0.602060, -0.494850, -0.397940, -0.301030, -0.221849,
                         -0.154902, -0.096910, -0.045757, 0.000000, +0.096910,
                         +0.204120, +0.301030, +0.397940, +0.505150, +0.602060,
                         +0.698970, +0.778151, +0.845098, +0.903090, +0.954243,
                         +1.000000, +1.041393, +1.079181, +1.301030, +1.602060,
                         +1.845098, +2.000000
                        };

      int j;
      unsigned int jj;
      double dedx[22], lz[22];
      double zlog, err;

      zlog = log10(zp);

      for (j = 0; j < 42 ; j++) {
         alref(elog[j], &lz[0], &dedx[0]);
         jj = locate(lz, 22, zlog);
         if (jj > 19) jj = 19;
         *(dedxz2 + j) = polint(&lz[jj], &dedx[jj], 3, zlog, &err);
      }
   }

   /*
     Data pool for aluminium -dE/dx/Z2.
     Call with energy E/A and get a series of log(Z) and log((-dE/dx)/Z2)
   */
   void alref(double le, double* lz, double* dedx)
   {

      unsigned int locate(double y[], int n, double x);
      double polint(double * xa, double * ya, int n, double x, double * dy);

      double zval[22] = {1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 16.,
                         20., 25., 32., 40., 50., 61., 79., 100.
                        };

      double elog[42] = {-1.903090, -1.795880, -1.698970, -1.602060, -1.494850,
                         -1.397940, -1.301030, -1.221849, -1.154902, -1.096910,
                         -1.045757, -1.000000, -0.903090, -0.795880, -0.698970,
                         -0.602060, -0.494850, -0.397940, -0.301030, -0.221849,
                         -0.154902, -0.096910, -0.045757, 0.000000, +0.096910,
                         +0.204120, +0.301030, +0.397940, +0.505150, +0.602060,
                         +0.698970, +0.778151, +0.845098, +0.903090, +0.954243,
                         +1.000000, +1.041393, +1.079181, +1.301030, +1.602060,
                         +1.845098, +2.000000
                        };

      /* H; Z=1 */
      double h[42] = {-0.67572, -0.62160, -0.57349, -0.52143, -0.46980, -0.43063,
                      -0.39794, -0.37779, -0.36653, -0.35952, -0.35655, -0.35655,
                      -0.36351, -0.38195, -0.40782, -0.44129, -0.48545, -0.53018,
                      -0.58004, -0.62709, -0.66555, -0.70115, -0.73049, -0.75945,
                      -0.82102, -0.88941, -0.95468, -1.02228, -1.10237, -1.17393,
                      -1.24413, -1.30103, -1.35655, -1.39794, -1.43180, -1.46852,
                      -1.49485, -1.52288, -1.72584, -1.95861, -2.14874, -2.25181
                     };

      /* He; Z=2 */
      double he[42] = {-0.87615, -0.82246, -0.77404, -0.72584, -0.67162, -0.62663,
                       -0.58503, -0.55479, -0.53276, -0.51606, -0.50376, -0.49485,
                       -0.48247, -0.48050, -0.48845, -0.50585, -0.53387, -0.56623,
                       -0.60380, -0.64589, -0.68194, -0.71388, -0.74232, -0.76828,
                       -0.82536, -0.89279, -0.95664, -1.02342, -1.09963, -1.17070,
                       -1.24222, -1.30103, -1.35164, -1.39523, -1.43474, -1.46852,
                       -1.49826, -1.53018, -1.70997, -1.94310, -2.13077, -2.23657
                      };

      /* Li; Z=3 */
      double li[42] = {-1.03990, -0.98623, -0.93763, -0.88904, -0.83532, -0.78870,
                       -0.74473, -0.71145, -0.68566, -0.66577, -0.65018, -0.63827,
                       -0.61939, -0.60985, -0.61103, -0.61979, -0.63618, -0.65561,
                       -0.67778, -0.70358, -0.72816, -0.75175, -0.77440, -0.79588,
                       -0.84568, -0.90658, -0.96658, -1.02996, -1.10360, -1.17249,
                       -1.24328, -1.30200, -1.35218, -1.39674, -1.43573, -1.47137,
                       -1.50246, -1.53264, -1.70333, -1.93554, -2.11919, -2.22915
                      };

      /* Be; Z=4 */
      double be[42] = {-1.16630, -1.11280, -1.06424, -1.01604, -0.96232, -0.91498,
                       -0.86967, -0.83472, -0.80740, -0.78615, -0.76955, -0.75696,
                       -0.73785, -0.72830, -0.72801, -0.73283, -0.74172, -0.75187,
                       -0.76321, -0.77875, -0.79385, -0.80879, -0.82373, -0.83863,
                       -0.87533, -0.92445, -0.97675, -1.03533, -1.10582, -1.17352,
                       -1.24365, -1.30266, -1.35347, -1.39794, -1.43771, -1.47334,
                       -1.50515, -1.53480, -1.70115, -1.92812, -2.11919, -2.22915
                      };

      /* B; Z=5 */
      double b[42] = {-1.26857, -1.21496, -1.16673, -1.11827, -1.06449, -1.01827,
                      -0.97469, -0.94157, -0.91578, -0.89551, -0.87943, -0.86646,
                      -0.84430, -0.82786, -0.81976, -0.81667, -0.81793, -0.82206,
                      -0.82822, -0.83791, -0.84783, -0.85811, -0.86864, -0.87943,
                      -0.90714, -0.94692, -0.99140, -1.04383, -1.10969, -1.17496,
                      -1.24413, -1.30277, -1.35379, -1.39881, -1.43890, -1.47470,
                      -1.50808, -1.53820, -1.69897, -1.92812, -2.11351, -2.22915
                     };

      /* C; Z=6 */
      double c[42] = {-1.36597, -1.31227, -1.26382, -1.21546, -1.16168, -1.11335,
                      -1.06803, -1.03230, -1.00327, -0.97939, -0.95960, -0.94310,
                      -0.91315, -0.88941, -0.87642, -0.86967, -0.86708, -0.86753,
                      -0.86967, -0.87751, -0.88587, -0.89477, -0.90406, -0.91364,
                      -0.93825, -0.97356, -1.01323, -1.06048, -1.12094, -1.18192,
                      -1.24795, -1.30491, -1.35491, -1.39915, -1.43903, -1.47496,
                      -1.50786, -1.53843, -1.69897, -1.92812, -2.10791, -2.23657
                     };

      /* N; Z=7 */
      double n[42] = {-1.43468, -1.38099, -1.33264, -1.28417, -1.23065, -1.18207,
                      -1.13789, -1.10316, -1.07498, -1.05171, -1.03209, -1.01543,
                      -0.98331, -0.95348, -0.93242, -0.91721, -0.90694, -0.90295,
                      -0.90309, -0.90873, -0.91546, -0.92289, -0.93091, -0.93930,
                      -0.96160, -0.99419, -1.03152, -1.07625, -1.13389, -1.19230,
                      -1.25579, -1.31053, -1.35877, -1.40150, -1.44002, -1.47509,
                      -1.50693, -1.53638, -1.69897, -1.92812, -2.11351, -2.23657
                     };

      /* O; Z=8 */
      double o[42] = {-1.51481, -1.46120, -1.41260, -1.36417, -1.31064, -1.26211,
                      -1.21679, -1.18066, -1.15095, -1.12594, -1.10461, -1.08619,
                      -1.04977, -1.01456, -0.98855, -0.96859, -0.95321, -0.94441,
                      -0.93930, -0.94119, -0.94554, -0.95157, -0.95873, -0.96658,
                      -0.98809, -1.01971, -1.05552, -1.09793, -1.15220, -1.20728,
                      -1.26761, -1.32032, -1.36701, -1.40876, -1.44672, -1.48149,
                      -1.51348, -1.54302, -1.70115, -1.92996, -2.11919, -2.23657
                     };

      /* F; Z=9 */
      double f[42] = {-1.59335, -1.53983, -1.49135, -1.44295, -1.38931, -1.34087,
                      -1.29243, -1.25489, -1.22382, -1.19744, -1.17473, -1.15490,
                      -1.11504, -1.07534, -1.04469, -1.02002, -0.99984, -0.98727,
                      -0.97881, -0.97881, -0.98183, -0.98680, -0.99298, -1.00000,
                      -1.01946, -1.04827, -1.08092, -1.11968, -1.16939, -1.22033,
                      -1.27653, -1.32619, -1.37067, -1.41086, -1.44759, -1.48149,
                      -1.51281, -1.54206, -1.70333, -1.93554, -2.11919, -2.23657
                     };

      /* Ne; Z=10 */
      double ne[42] = {-1.667562, -1.614036, -1.565431, -1.516984, -1.463442,
                       -1.414991, -1.366532, -1.327440, -1.294821, -1.267044,
                       -1.242984, -1.221849, -1.178814, -1.134837, -1.099851,
                       -1.070581, -1.045661, -1.029374, -1.017729, -1.016554,
                       -1.018544, -1.022505, -1.027751, -1.033858, -1.051294,
                       -1.077638, -1.107905, -1.144118, -1.190912, -1.239050,
                       -1.292430, -1.339704, -1.382161, -1.420559, -1.455684,
                       -1.488117, -1.518128, -1.546223, -1.709965, -1.939302,
                       -2.124939, -2.244125
                      };

      /* Na; Z=11 */
      double na[42] = {-1.721058, -1.667311, -1.618892, -1.570501, -1.516820,
                       -1.468416, -1.419933, -1.380786, -1.348226, -1.320407,
                       -1.296389, -1.275318, -1.232446, -1.188746, -1.153929,
                       -1.124556, -1.098528, -1.080058, -1.065126, -1.059368,
                       -1.057930, -1.059286, -1.062507, -1.067048, -1.081744,
                       -1.106069, -1.135107, -1.170563, -1.216675, -1.264098,
                       -1.316521, -1.362626, -1.403721, -1.440816, -1.474580,
                       -1.505523, -1.534028, -1.560602, -1.712198, -1.939302,
                       -2.130768, -2.244125
                      };

      /* Mg; Z=13 */
      double mg[42] = {-1.772578, -1.719030, -1.670517, -1.622183, -1.568525,
                       -1.520073, -1.471637, -1.432043, -1.398770, -1.370336,
                       -1.345650, -1.324005, -1.280013, -1.235689, -1.201234,
                       -1.172622, -1.146496, -1.126147, -1.107635, -1.098269,
                       -1.093830, -1.092708, -1.093905, -1.096722, -1.108137,
                       -1.129466, -1.156326, -1.189926, -1.234445, -1.280588,
                       -1.331705, -1.376751, -1.416896, -1.453012, -1.485895,
                       -1.515997, -1.543782, -1.569643, -1.716699, -1.943095,
                       -2.130768, -2.244125
                      };

      /* Al; Z=13 */
      double al[42] = {-1.819477, -1.765788, -1.717342, -1.668938, -1.615315,
                       -1.566832, -1.518362, -1.478769, -1.445056, -1.416178,
                       -1.391120, -1.368989, -1.324092, -1.279083, -1.244712,
                       -1.216274, -1.189346, -1.167302, -1.146395, -1.134325,
                       -1.127585, -1.124494, -1.123946, -1.125247, -1.133765,
                       -1.152303, -1.177082, -1.208978, -1.251858, -1.296734,
                       -1.346616, -1.390614, -1.429789, -1.465058, -1.497104,
                       -1.526405, -1.553485, -1.578552, -1.721246, -1.946922,
                       -2.130768, -2.251812
                      };

      /* S; Z=16 */
      double s[42] = {-1.940188, -1.886579, -1.838047, -1.789669, -1.736050,
                      -1.687585, -1.639158, -1.599556, -1.566068, -1.536375,
                      -1.510448, -1.487543, -1.440552, -1.393635, -1.358479,
                      -1.328625, -1.298392, -1.272186, -1.246453, -1.229580,
                      -1.218301, -1.211042, -1.206734, -1.204636, -1.206133,
                      -1.217544, -1.236718, -1.263853, -1.302185, -1.343333,
                      -1.389623, -1.430608, -1.467176, -1.500023, -1.529833,
                      -1.557043, -1.582165, -1.605329, -1.739929, -1.954677,
                      -2.142668, -2.251812
                     };

      /* Ca; Z=20 */
      double ca[42] = {-2.075979, -2.021819, -1.972854, -1.923997, -1.869827,
                       -1.820951, -1.771985, -1.732066, -1.698265, -1.668978,
                       -1.643162, -1.619608, -1.570934, -1.521578, -1.483795,
                       -1.450506, -1.415782, -1.385129, -1.354774, -1.334630,
                       -1.320095, -1.309649, -1.302291, -1.297333, -1.292494,
                       -1.296881, -1.310092, -1.331777, -1.364667, -1.401237,
                       -1.443065, -1.480402, -1.513818, -1.543900, -1.571217,
                       -1.596151, -1.619111, -1.640402, -1.761954, -1.968592,
                       -2.148742, -2.259637
                      };

      /* Mn; Z=25 */
      double mn[42] = {-2.225571, -2.170053, -2.119827, -2.069642, -2.014125,
                       -1.963946, -1.913754, -1.872740, -1.838033, -1.807990,
                       -1.781528, -1.757816, -1.707638, -1.655498, -1.613580,
                       -1.575563, -1.535642, -1.500335, -1.465385, -1.442445,
                       -1.425311, -1.412424, -1.402779, -1.395653, -1.385879,
                       -1.384429, -1.392159, -1.408383, -1.435381, -1.466787,
                       -1.503646, -1.537003, -1.567095, -1.594346, -1.619152,
                       -1.641882, -1.662852, -1.682271, -1.787812, -1.982967,
                       -2.161151, -2.267606
                      };

      /* Ge; Z=32 */
      double ge[42] = {-2.400492, -2.343408, -2.291715, -2.240111, -2.182995,
                       -2.131319, -2.079657, -2.037496, -2.001785, -1.970886,
                       -1.943639, -1.919231, -1.867598, -1.810463, -1.763088,
                       -1.718798, -1.672302, -1.631539, -1.591419, -1.564711,
                       -1.544545, -1.529158, -1.517344, -1.508296, -1.494333,
                       -1.487817, -1.490406, -1.500990, -1.521326, -1.546631,
                       -1.577491, -1.606185, -1.632484, -1.656595, -1.678751,
                       -1.699203, -1.718177, -1.735865, -1.823909, -2.002177,
                       -2.187087, -2.283997
                      };

      /* Zr; Z=40 */
      double zr[42] = {-2.561853, -2.503243, -2.450231, -2.397194, -2.338601,
                       -2.285565, -2.232566, -2.189264, -2.152620, -2.120904,
                       -2.092925, -2.067907, -2.014882, -1.956245, -1.903242,
                       -1.852826, -1.799560, -1.753563, -1.709214, -1.677858,
                       -1.654381, -1.636459, -1.622625, -1.611899, -1.594536,
                       -1.584078, -1.582591, -1.588464, -1.602995, -1.622728,
                       -1.648011, -1.672309, -1.695106, -1.716345, -1.736142,
                       -1.754611, -1.771888, -1.788112, -1.861697, -2.026872,
                       -2.200659, -2.301030
                      };

      /* Sn; Z=50 */
      double sn[42] = {-2.721064, -2.660906, -2.606460, -2.552036, -2.491821,
                       -2.437422, -2.383000, -2.338528, -2.300926, -2.268347,
                       -2.239608, -2.213930, -2.159492, -2.099283, -2.044871,
                       -1.990430, -1.930228, -1.878243, -1.830878, -1.794081,
                       -1.766699, -1.745819, -1.729629, -1.716934, -1.695613,
                       -1.680761, -1.675002, -1.676195, -1.685072, -1.699405,
                       -1.719194, -1.739042, -1.758185, -1.776379, -1.793595,
                       -1.809859, -1.825243, -1.839808, -1.913640, -2.060481,
                       -2.229148, -2.318759
                      };

      /* Pm; Z=61 */
      double pm[42] = {-2.859781, -2.798118, -2.742451, -2.686714, -2.625043,
                       -2.569315, -2.513602, -2.468054, -2.429555, -2.396193,
                       -2.366784, -2.340466, -2.284742, -2.223095, -2.167368,
                       -2.111644, -2.049993, -1.993513, -1.942557, -1.901296,
                       -1.870337, -1.846490, -1.827754, -1.812816, -1.786842,
                       -1.766889, -1.756499, -1.752995, -1.756612, -1.766145,
                       -1.781107, -1.797027, -1.812861, -1.828225, -1.842956,
                       -1.857026, -1.870441, -1.883229, -1.958607, -2.096910,
                       -2.259637, -2.337242
                      };

      /* Au; Z=79 */
      double au[42] = {-3.042131, -2.978549, -2.921062, -2.863644, -2.800058,
                       -2.742599, -2.685136, -2.638190, -2.598525, -2.564142,
                       -2.533801, -2.506670, -2.449214, -2.385659, -2.328194,
                       -2.270741, -2.207173, -2.149714, -2.092264, -2.047594,
                       -2.012868, -1.985183, -1.962713, -1.944210, -1.910215,
                       -1.881059, -1.862441, -1.851053, -1.846528, -1.849244,
                       -1.857867, -1.868886, -1.880736, -1.892718, -1.904509,
                       -1.915963, -1.927015, -1.937638, -2.017729, -2.154902,
                       -2.288193, -2.361511
                      };

      /* Fm; Z=100 */
      double fm[42] = {-3.208520, -3.143211, -3.084231, -3.025212, -2.959912,
                       -2.900872, -2.841879, -2.793633, -2.752862, -2.717559,
                       -2.686407, -2.658546, -2.599514, -2.534231, -2.475215,
                       -2.416189, -2.350899, -2.291885, -2.232866, -2.188767,
                       -2.152835, -2.122876, -2.097497, -2.075726, -2.033033,
                       -1.992295, -1.962577, -1.940611, -1.925985, -1.921442,
                       -1.924464, -1.931844, -1.941035, -1.950879, -1.960828,
                       -1.970612, -1.980107, -1.989259, -2.070581, -2.193820,
                       -2.309804, -2.371611
                      };

      double err;
      int j;
      unsigned int jj;

      for (j = 0; j < 22 ; j++)
         *(lz + j) = log10(zval[j]);

      jj = locate(elog, 42, le);
      if (jj > 39) jj = 39;

      /* Interpolate data for Al to given energy for all standard ions */

      /* First hydrogen ions */
      *(dedx++) = polint(&elog[jj], &h[jj], 3, le, &err);

      /* Then Helium ions */
      *(dedx++) = polint(&elog[jj], &he[jj], 3, le, &err);

      /* Then Lithium ions */
      *(dedx++) = polint(&elog[jj], &li[jj], 3, le, &err);

      /* Then Beryllium ions */
      *(dedx++) = polint(&elog[jj], &be[jj], 3, le, &err);

      /* Then Bor ions */
      *(dedx++) = polint(&elog[jj], &b[jj], 3, le, &err);

      /* Then Carbon ions */
      *(dedx++) = polint(&elog[jj], &c[jj], 3, le, &err);

      /* Then Nitrogen ions */
      *(dedx++) = polint(&elog[jj], &n[jj], 3, le, &err);

      /* Then Oxygen ions */
      *(dedx++) = polint(&elog[jj], &o[jj], 3, le, &err);

      /* Then Fluorine ions */
      *(dedx++) = polint(&elog[jj], &f[jj], 3, le, &err);

      /* Then Neon ions */
      *(dedx++) = polint(&elog[jj], &ne[jj], 3, le, &err);

      /* Then Sodium ions */
      *(dedx++) = polint(&elog[jj], &na[jj], 3, le, &err);

      /* Then Magnesium ions */
      *(dedx++) = polint(&elog[jj], &mg[jj], 3, le, &err);

      /* Then Aluminium ions */
      *(dedx++) = polint(&elog[jj], &al[jj], 3, le, &err);

      /* Then Sulfur ions */
      *(dedx++) = polint(&elog[jj], &s[jj], 3, le, &err);

      /* Then Calcium ions */
      *(dedx++) = polint(&elog[jj], &ca[jj], 3, le, &err);

      /* Then Manganese ions */
      *(dedx++) = polint(&elog[jj], &mn[jj], 3, le, &err);

      /* Then Germanium ions */
      *(dedx++) = polint(&elog[jj], &ge[jj], 3, le, &err);

      /* Then Zirconium ions */
      *(dedx++) = polint(&elog[jj], &zr[jj], 3, le, &err);

      /* Then Tin ions */
      *(dedx++) = polint(&elog[jj], &sn[jj], 3, le, &err);

      /* Then Prometium ions */
      *(dedx++) = polint(&elog[jj], &pm[jj], 3, le, &err);

      /* Then Gold ions */
      *(dedx++) = polint(&elog[jj], &au[jj], 3, le, &err);

      /* Then Fermium ions */
      *(dedx++) = polint(&elog[jj], &fm[jj], 3, le, &err);
   }


   /*
     Interpolate for current value of s(2,a) function as given by
     F.Hubert, R.Rimbot and H.Gauvin, Atomic Data and Nuclear Data
     Tables, 1990, 46, pp. 1-213.
   */
   double s2az(double e, int zt)
   {

      void splie2(double x1a[], double x2a[], double * ya[],
                  int m, int n, double * y2a[]);
      void splin2(double x1a[], double x2a[], double * ya[], double * y2a[],
                  int m, int n, double x1, double x2, double * y);

      double za[18] = {4.0, 6.0, 13.0, 14.0, 22.0, 26.0, 28.0, 29.0, 32.0, 34.0, 40.0,
                       47.0, 50.0, 64.0, 73.0, 79.0, 82.0, 92.0
                      };

      double el[38] = {0.916291, 1.098612, 1.252763, 1.386294, 1.504077, 1.609438,
                       1.704748, 1.791759, 1.871802, 1.945910, 2.079442, 2.197225,
                       2.302585, 2.397895, 2.484907, 2.708050, 2.995732, 3.218876,
                       3.401197, 3.555348, 3.688879, 3.806662, 3.912023, 4.007333,
                       4.094345, 4.174387, 4.248495, 4.382027, 4.499810, 4.605170,
                       5.010635, 5.298317, 5.521461, 5.703782, 5.857933, 5.991465,
                       6.109248, 6.214608
                      };

      static double sa2[18][38] = {
         {
            2.19060, 2.32662, 2.44357, 2.54625, 2.63806, 2.72038, 2.79565, 2.86470, 2.92901,
            2.98826, 3.09555, 3.19114, 3.27677, 3.35455, 3.42575, 3.60822, 3.84436, 4.02575,
            4.17501, 4.29953, 4.40693, 4.50104, 4.58488, 4.66015, 4.72819, 4.79060, 4.84820,
            4.95048, 5.04019, 5.11892, 5.41429, 5.61371, 5.75009, 5.85432, 5.93698, 6.00455,
            6.06082, 6.10800
         },
         {
            2.12549, 2.25929, 2.37462, 2.47575, 2.56623, 2.64754, 2.72190, 2.79035, 2.85337,
            2.91231, 3.01849, 3.11283, 3.19785, 3.27479, 3.34529, 3.52676, 3.76038, 3.94119,
            4.08936, 4.21313, 4.31999, 4.41372, 4.49699, 4.57174, 4.63976, 4.70168, 4.75890,
            4.86103, 4.94978, 5.02829, 5.32210, 5.51089, 5.64575, 5.75009, 5.83275, 5.89980,
            5.95610, 6.00253
         },
         {
            2.35942, 2.48561, 2.59461, 2.69082, 2.77700, 2.85467, 2.92574, 2.99124, 3.05177,
            3.10834, 3.21078, 3.30158, 3.38360, 3.45777, 3.52591, 3.70095, 3.92841, 4.10440,
            4.24925, 4.37010, 4.47392, 4.56547, 4.64677, 4.72002, 4.78639, 4.84724, 4.90290,
            5.00267, 5.09008, 5.16685, 5.45439, 5.64292, 5.77635, 5.87814, 5.95900, 6.02606,
            6.08139, 6.12728
         },
         {
            2.33666, 2.46187, 2.57047, 2.66607, 2.75161, 2.82895, 2.89951, 2.96472, 3.02516,
            3.08129, 3.18327, 3.27346, 3.35527, 3.42883, 3.49661, 3.67202, 3.89837, 4.07454,
            4.21821, 4.33897, 4.44241, 4.53378, 4.61522, 4.68828, 4.75454, 4.81527, 4.87109,
            4.97081, 5.05812, 5.13492, 5.42275, 5.61783, 5.75324, 5.85519, 5.93603, 6.00152,
            6.05654, 6.10240
         },
         {
            2.52604, 2.64649, 2.75161, 2.84387, 2.92667, 3.00175, 3.07046, 3.13385, 3.19236,
            3.24740, 3.34671, 3.43501, 3.51409, 3.58723, 3.65351, 3.82470, 4.04698, 4.21991,
            4.36027, 4.47854, 4.58194, 4.67184, 4.75222, 4.82426, 4.88952, 4.94942, 5.00490,
            5.10316, 5.18946, 5.26537, 5.54999, 5.73837, 5.87013, 5.97166, 6.05122, 6.11703,
            6.17179, 6.21711
         },
         {
            2.58296, 2.70008, 2.80222, 2.89273, 2.97348, 3.04703, 3.11452, 3.17666, 3.23399,
            3.28809, 3.38582, 3.47216, 3.55086, 3.62216, 3.68788, 3.85730, 4.07601, 4.24750,
            4.38805, 4.50533, 4.60667, 4.69592, 4.77537, 4.84692, 4.91204, 4.97154, 5.02600,
            5.12394, 5.20939, 5.28491, 5.56816, 5.75324, 5.88351, 5.98449, 6.06404, 6.12958,
            6.18384, 6.22972
         },
         {
            2.59931, 2.71394, 2.81383, 2.90270, 2.98232, 3.05442, 3.12073, 3.18206, 3.23844,
            3.29145, 3.38803, 3.47377, 3.55173, 3.62216, 3.68688, 3.85493, 4.07307, 4.24227,
            4.38203, 4.49856, 4.60018, 4.68910, 4.76828, 4.83931, 4.90425, 4.96328, 5.01804,
            5.11558, 5.20028, 5.27557, 5.55709, 5.74071, 5.87102, 5.97166, 6.05122, 6.11590,
            6.17059, 6.21586
         },
         {
            2.65962, 2.77379, 2.87307, 2.96133, 3.04073, 3.11227, 3.17845, 3.23972, 3.29616,
            3.34884, 3.44515, 3.53102, 3.60822, 3.67794, 3.74334, 3.91077, 4.12738, 4.29769,
            4.43543, 4.55400, 4.65384, 4.74271, 4.82177, 4.89285, 4.95721, 5.01653, 5.07078,
            5.16817, 5.25334, 5.32826, 5.60961, 5.79425, 5.92474, 6.02399, 6.10351, 6.16820,
            6.22214, 6.26722
         },
         {
            2.71018, 2.82304, 2.92155, 3.00932, 3.08785, 3.15943, 3.22515, 3.28542, 3.34175,
            3.39397, 3.49003, 3.57466, 3.65158, 3.72140, 3.78649, 3.95285, 4.16853, 4.33897,
            4.47634, 4.59275, 4.69373, 4.78221, 4.86103, 4.93194, 4.99636, 5.05537, 5.10977,
            5.20665, 5.29134, 5.36660, 5.64717, 5.80914, 5.93982, 6.03961, 6.12044, 6.18505,
            6.23993, 6.28584
         },
         {
            2.75436,
            2.86558, 2.96278, 3.04913, 3.12641, 3.19724, 3.26231, 3.32216, 3.37773, 3.42960,
            3.52421, 3.60914, 3.68489, 3.75502, 3.81899, 3.98459, 4.19971, 4.36812, 4.50533,
            4.62283, 4.72283, 4.81097, 4.88986, 4.96042, 5.02486, 5.08401, 5.13790, 5.23534,
            5.32005, 5.39483, 5.67520, 5.84305, 5.97264, 6.07268, 6.15281, 6.21837, 6.27118,
            6.31720
         },
         {
            2.77780, 2.88822, 2.98479, 3.07046, 3.14714, 3.21763, 3.28208, 3.34104, 3.39621,
            3.44750, 3.54132, 3.62497, 3.70095, 3.77009, 3.83275, 3.99676, 4.20976, 4.37605,
            4.51442, 4.62793, 4.72819, 4.81558, 4.89352, 4.96363, 5.02753, 5.08603, 5.14003,
            5.23628, 5.32056, 5.39483, 5.67374, 5.85956, 5.98847, 6.08798, 6.16582, 6.23099,
            6.28449, 6.32974
         },
         {
            2.86734, 2.97446, 3.06884, 3.15239, 3.22766, 3.29616, 3.35886, 3.41733, 3.47135,
            3.52167, 3.61377, 3.69590, 3.77009, 3.83854, 3.90084, 4.06139, 4.27228, 4.43543,
            4.57077, 4.68313, 4.78161, 4.86783, 4.94485, 5.01389, 5.07678, 5.13450, 5.18767,
            5.28244, 5.36553, 5.43873, 5.71383, 5.89797, 6.02502, 6.12385, 6.20219, 6.26590,
            6.31858, 6.36398
         },
         {
            2.92667, 3.03240, 3.12527, 3.20769, 3.28208, 3.34955, 3.41201, 3.46974, 3.52337,
            3.57288, 3.66419, 3.74651, 3.82013, 3.88733, 3.94895, 4.10895, 4.31811, 4.48141,
            4.61522, 4.72847, 4.82644, 4.91238, 4.98900, 5.05812, 5.12101, 5.17876, 5.23159,
            5.32620, 5.40925, 5.48284, 5.75718, 5.94077, 6.06835, 6.16701, 6.24507, 6.30892,
            6.36253, 6.40698
         },
         {
            3.03812, 3.14018, 3.22956, 3.30907, 3.38140, 3.44672, 3.50739, 3.56313, 3.61563,
            3.66419, 3.75289, 3.83275, 3.90455, 3.97124, 4.03137, 4.18827, 4.39369, 4.55448,
            4.68638, 4.79815, 4.89485, 4.97986, 5.05537, 5.12394, 5.18588, 5.24288, 5.29532,
            5.38934, 5.47089, 5.54358, 5.81583, 6.00051, 6.12958, 6.22719, 6.30481, 6.36834,
            6.42071, 6.46467
         },
         {
            3.15063, 3.24612, 3.32981, 3.40521, 3.47377, 3.53702, 3.59448, 3.64870, 3.69893,
            3.74545, 3.83160, 3.90828, 3.97790, 4.04270, 4.10137, 4.25416, 4.45524, 4.61295,
            4.74271, 4.85235, 4.94766, 5.03135, 5.10605, 5.17345, 5.23487, 5.29134, 5.34279,
            5.43586, 5.51710, 5.58867, 5.85781, 6.04066, 6.17299, 6.26986, 6.34671, 6.40850,
            6.46147, 6.50563
         },
         {
            3.21016, 3.30294, 3.38508, 3.45856, 3.52591, 3.58723, 3.64391, 3.69590, 3.74545,
            3.79091, 3.87521, 3.95154, 4.02017, 4.08341, 4.14144, 4.29182, 4.49006, 4.64573,
            4.77389, 4.88257, 4.97660, 5.05969, 5.13365, 5.20028, 5.26150, 5.31699, 5.36820,
            5.46025, 5.54103, 5.61234, 5.88082, 6.06189, 6.19358, 6.28987, 6.36543, 6.42842,
            6.47923, 6.52420
         },
         {
            3.21763, 3.31113, 3.39323, 3.46734, 3.53530, 3.59630, 3.65351, 3.70603, 3.75609,
            3.80205, 3.88733, 3.96332, 4.03137, 4.09535, 4.15250, 4.30451, 4.50329, 4.65963,
            4.78819, 4.89720, 4.99157, 5.07477, 5.14904, 5.21582, 5.27656, 5.33239, 5.38388,
            5.47625, 5.55644, 5.62821, 5.89615, 6.07811, 6.20962, 6.30481, 6.38155, 6.44402,
            6.49565, 6.53965
         },
         {
            3.26035, 3.35455, 3.43812, 3.51325, 3.58092, 3.64391, 3.70095, 3.75395, 3.80429,
            3.85022, 3.93478, 4.01184, 4.08044, 4.14459, 4.20304, 4.35363, 4.55234, 4.70859,
            4.83679, 4.94555, 5.03942, 5.12227, 5.19666, 5.26343, 5.32415, 5.37953, 5.43071,
            5.52271, 5.60349, 5.67447, 5.94172, 6.12044, 6.24507, 6.34102, 6.41611, 6.47923,
            6.53103, 6.57307
         }
      };

      int i;
      static double y2a[18][38];
      static double* psa2[18], *py2a[18];
      double sa2ln;
      double le;

      if (!isw5) {
         for (i = 0 ; i < 18 ; i++) {
            psa2[i] = &(sa2[i])[0];
            py2a[i] = &(y2a[i])[0];
         }
         splie2(el, za, &psa2[0], 38, 18, &py2a[0]);
         isw5 = 1;
      }
      le = log(e);
      splin2(el, za, &psa2[0], &py2a[0], 38, 18, le, zt, &sa2ln);
      return exp(-sa2ln);
   }


   double polint(double* xa, double* ya, int n, double x, double* dy)
   {

      double y, c[n], d[n];
      int i, m, ns = 1;
      double den, dif, dift, ho, hp, w;

      dif = fabs(x - *xa);

      for (i = 0 ; i < n ; i++) {
         dift = fabs(x - * (xa + i));
         if (dift < dif) {
            ns = i + 1;
            dif = dift;
         }
         c[i] = d[i] = *(ya + i);
      }
      y = *(ya + ns - 1);
      ns--;
      for (m = 0 ; m < n - 1 ; m++) {
         for (i = 0 ; i < n - m - 1 ; i++) {
            ho = *(xa + i) - x;
            hp = *(xa + i + m + 1) - x;
            w = c[i + 1] - d[i];
            den = ho - hp;
            if (den == 0.0)
               exit(0);
            den = w / den;
            d[i] = hp * den;
            c[i] = ho * den;
         }
         if (2 * ns < n - m - 1)
            *dy = c[ns];
         else {
            *dy = d[ns - 1];
            ns--;
         }
         y += *dy;
      }
      *dy = fabs(*dy);
      return y;
   }


   unsigned int locate(double y[], int n, double x)
   {

      unsigned int jl, jm, ju;

      jl = 0;
      ju = n;

      while ((ju - jl) > 1) {
         jm = (ju + jl) / 2;
         if ((y[n - 1] > y[1]) && (x > y[jm]))
            jl = jm;
         else
            ju = jm;
      }
      return jl;
   }


   void splie2(double [], double x2a[], double* ya[],
               int m, int n, double* y2a[])
   {

      void spline(double x[], double y[], int n, double yp1, double ypn, double * y2);

      double ytmp[n], y2tmp[n];
      int i, j;

      for (j = 0 ; j < m ; j++) {
         for (i = 0 ; i < n ; i++) {
            ytmp[i] = *(ya[i] + j);
         }

         spline(x2a, ytmp, n, 1.0e30, 1.0e30, &y2tmp[0]);
         for (i = 0 ; i < n ; i++) {
            *(y2a[i] + j) = y2tmp[i];
         }
      }
   }


   void splin2(double x1a[], double x2a[], double* ya[], double* y2a[],
               int m, int n, double x1, double x2, double* y)
   {

      void spline(double x[], double y[], int n, double yp1, double ypn, double * y2);
      void splint(double xa[], double ya[], double y2a[],
                  int n, double x, double * y);

      double ytmp[n], y2tmp[n], yytmp[m];
      int i, j;

      for (j = 0 ; j < m ; j++) {
         for (i = 0 ; i < n ; i++) {
            ytmp[i] = *(ya[i] + j);
            y2tmp[i] = *(y2a[i] + j);
         }
         splint(x2a, ytmp, y2tmp, n, x2, &yytmp[j]);
      }
      spline(x1a, yytmp, m, 1.0e30, 1.0e30, &y2tmp[0]);
      splint(x1a, yytmp, y2tmp, m, x1, y);
   }


   void spline(double x[], double y[], int n, double yp1, double ypn, double* y2)
   {

      int k;
      double p, qn, sig, un, u[200];
      int m = n - 1;

      if (yp1 > 0.99e30)
         *y2 = u[0] = 0.0;
      else {
         *y2 = -0.5;
         u[0] = (3.0 / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
      }
      for (k = 1 ; k < n - 1 ; k++) {
         sig = (x[k] - x[k - 1]) / (x[k + 1] - x[k - 1]);
         p = *(y2 + k - 1) * sig + 2.0;
         *(y2 + k) = (sig - 1.0) / p;
         u[k] = (6.0 * ((y[k + 1] - y[k]) / (x[k + 1] - x[k]) -
                        (y[k] - y[k - 1]) / (x[k] - x[k - 1])) /
                 (x[k + 1] - x[k - 1]) - sig * u[k - 1]) / p;
      }
      if (ypn > 0.99e30)
         qn = un = 0.0;
      else {
         qn = 0.5;
         un = (3.0 / (x[m] - x[m - 1])) *
              (ypn - (y[m] - y[m - 1]) / (x[m] - x[m - 1]));
      }
      *(y2 + m) = (un - qn * u[m - 1]) / (*(y2 + m - 1) * qn + 1.0);
      for (k = m - 1 ; k >= 0 ; k--) {
         *(y2 + k) *= *(y2 + k + 1);
         *(y2 + k) += u[k];
      }
   }


   void splint(double xa[], double ya[], double y2a[],
               int n, double x, double* y)
   {

      int k, klo, khi;
      double a, b, h;

      klo = 1;
      khi = n;

      while (khi - klo > 1) {
         k = (khi + klo) >> 1;
         if (xa[k - 1] > x)
            khi = k;
         else
            klo = k;
      }
      klo--;
      khi--;
      h = xa[khi] - xa[klo];
      if (h == 0.0) {
         printf("Bad xa input in splint.\n");
         exit(0);
      }
      a = (xa[khi] - x) / h;
      b = (x - xa[klo]) / h;
      *y = a * ya[klo] + b * ya[khi] +
           ((pow(a, 3) - a) * y2a[klo] + (pow(b, 3) - b) * y2a[khi]) * (h * h) / 6.0;
   }
}
