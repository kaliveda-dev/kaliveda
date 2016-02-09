//Created by KVClassFactory on Wed Feb  2 16:13:08 2011
//Author: frankland,,,,

#include "KVedaLossMaterial.h"
#include <TMath.h>
#include "KVIonRangeTable.h"
#include <TEnv.h>
#include <TF1.h>

ClassImp(KVedaLossMaterial)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVedaLossMaterial</h2>
<h4>Description of material properties used by KVedaLoss range calculation</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Bool_t KVedaLossMaterial::fNoLimits = kFALSE;

KVedaLossMaterial::KVedaLossMaterial()
   : KVIonRangeTableMaterial()
{
   // Default constructor
   for (int i = 0; i < 100; i++) {
      fEmin[i] = 0.0;
      fEmax[i] = 500.0;
   }
}

KVedaLossMaterial::KVedaLossMaterial(const KVIonRangeTable* t, const Char_t* name, const Char_t* type, const Char_t* state,
                                     Double_t density, Double_t Z, Double_t A, Double_t)
   : KVIonRangeTableMaterial(t, name, type, state, density, Z, A)
{
   // create new material
   for (int i = 0; i < 100; i++) {
      fEmin[i] = 0.0;
      fEmax[i] = 500.0;
   }
}

KVedaLossMaterial::~KVedaLossMaterial()
{
   // Destructor
}

Bool_t KVedaLossMaterial::ReadRangeTable(FILE* fp)
{
   // Read Z- & A-dependent range parameters for material
   //
   // For each material we create 4 TF1 objects:
   //   KVedaLossMaterial:[type]:Range                -  gives range in g/cm**2 as a function of particle energy
   //   KVedaLossMaterial:[type]:StoppingPower           -  gives dE/dx in MeV/(g/cm**2) as a function of particle energy
   //   KVedaLossMaterial:[type]:EnergyLoss           -  gives dE as a function of particle energy
   //   KVedaLossMaterial:[type]:ResidualEnergy       -  gives energy after material (0 if particle stops)
   //
   // The TF1::fNpx parameter for these functions is defined by the environment variables
   //
   //   KVedaLoss.Range.Npx:         20      /* also used for StoppingPower */
   //   KVedaLoss.EnergyLoss.Npx:         50
   //   KVedaLoss.ResidualEnergy.Npx:         20
   //

   char line[132];

   //look for energy limits to calculation validity
   if (!fgets(line, 132, fp)) {
      Warning("ReadRangeTable", "Problem reading energy limits in range table file for %s (%s)",
              GetName(), GetType());
      return kFALSE;
   } else {
      if (!strncmp(line, "COMPOUND", 8)) {
         // material is compound. read composition for TGeoMixture.
         if (fgets(line, 132, fp)) {}
         int nel = atoi(line);  // read number of elements
         for (int el = 0; el < nel; el++) {
            if (fgets(line, 132, fp)) {}
            int z, a, w;
            sscanf(line, "%d %d %d", &z, &a, &w);
            AddCompoundElement(z, a, w);
         }
      } else if (!strncmp(line, "MIXTURE", 7)) {
         // material is mixture. read composition for TGeoMixture.
         if (fgets(line, 132, fp)) {}
         int nel = atoi(line);  // read number of elements
         for (int el = 0; el < nel; el++) {
            if (fgets(line, 132, fp)) {}
            int z, a, nat;
            float w;
            sscanf(line, "%d %d %d %f", &z, &a, &nat, &w);
            AddMixtureElement(z, a, nat, w);
         }
      }
   }
   if (!fgets(line, 132, fp)) {
      Warning("ReadRangeTable", "Problem reading energy limits in range table file for %s (%s)",
              GetName(), GetType());
      return kFALSE;
   } else {
      while (line[0] == 'Z') {
         Int_t z1, z2;
         Float_t e1, e2;
         sscanf(line, "Z = %d,%d     %f < E/A  <  %f MeV", &z1,
                &z2, &e1, &e2);
         if (fgets(line, 132, fp)) {}
         for (int i = z1; i <= z2; i++) {
            fEmin[i - 1] = e1;
            fEmax[i - 1] = e2;
         }
      }
   }

   // get require Npx value from (user-defined) environment variables
   Int_t my_npx = gEnv->GetValue("KVedaLoss.Range.Npx", 100);

   fRange = new TF1(Form("KVedaLossMaterial:%s:Range", GetType()), this, &KVedaLossMaterial::RangeFunc,
                    0., 1.e+03, 0, "KVedaLossMaterial", "RangeFunc");
   fRange->SetNpx(my_npx);

   fStopping = new TF1(Form("KVedaLossMaterial:%s:StoppingPower", GetType()), this, &KVedaLossMaterial::StoppingFunc,
                       0., 1.e+03, 0, "KVedaLossMaterial", "StoppingFunc");
   fStopping->SetNpx(my_npx);

   my_npx = gEnv->GetValue("KVedaLoss.EnergyLoss.Npx", 100);
   fDeltaE = new TF1(Form("KVedaLossMaterial:%s:EnergyLoss", GetType()), this, &KVedaLossMaterial::DeltaEFunc,
                     0., 1.e+03, 0, "KVedaLossMaterial", "DeltaEFunc");
   fDeltaE->SetNpx(my_npx);

   my_npx = gEnv->GetValue("KVedaLoss.ResidualEnergy.Npx", 100);
   fEres = new TF1(Form("KVedaLossMaterial:%s:ResidualEnergy", GetType()), this, &KVedaLossMaterial::EResFunc,
                   0., 1.e+03, 0, "KVedaLossMaterial", "EResFunc");
   fEres->SetNpx(my_npx);

   for (register int count = 0; count < ZMAX_VEDALOSS; count++) {

      if (sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf",
                 &fCoeff[count][0], &fCoeff[count][1],
                 &fCoeff[count][2], &fCoeff[count][3],
                 &fCoeff[count][4], &fCoeff[count][5],
                 &fCoeff[count][6], &fCoeff[count][7])
            != 8) {
         Error("ReadRangeTable", "problem reading coeffs 0-7 in range table for %s (%s)", GetName(), GetType());
         return kFALSE;
      }
      if (!fgets(line, 132, fp)) {
         Warning("ReadRangeTable", "file too short ??? %s (%s)", GetName(), GetType());
         return kFALSE;
      } else {
         if (sscanf(line, "%lf %lf %lf %lf %lf %lf",
                    &fCoeff[count][8], &fCoeff[count][9],
                    &fCoeff[count][10], &fCoeff[count][11],
                    &fCoeff[count][12], &fCoeff[count][13])
               != 6) {
            Error("ReadRangeTable", "problem reading coeffs 8-13 in range table for %s (%s)", GetName(), GetType());
            return kFALSE;
         }
      }
      if (fNoLimits) {
         // if we ignore nominal validity limits on incident energy, we must still use energy limits
         // such that all range functions increase monotonically in the energy interval
         GetRangeFunction(fCoeff[count][0], fCoeff[count][1])->SetRange(GetEminValid(fCoeff[count][0], fCoeff[count][1]), VERY_BIG_ENERGY);
         Double_t emax = fRange->GetMaximumX() - 1;
         emax /= fCoeff[count][1];
         Double_t original_emax = fEmax[count];
         // the new emax is only accepted if it is > than the nominal emax (400 or 250 AMeV),
         // and at most 1 GeV/nucleon
         fEmax[count] = TMath::Min(TMath::Max(original_emax, emax), 1000.);
         // we may further reduce the upper limit to correspond to the minimum of stopping,
         // if one exists
         GetStoppingFunction(fCoeff[count][0], fCoeff[count][1])->SetRange(GetEminValid(fCoeff[count][0], fCoeff[count][1]), GetEmaxValid(fCoeff[count][0], fCoeff[count][1]));
         emax = fStopping->GetMinimumX();
         emax /= fCoeff[count][1];
         // again, the new emax is only accepted if it is > than the nominal emax (400 or 250 AMeV),
         // and at most 1 GeV/nucleon
         fEmax[count] = TMath::Min(TMath::Max(original_emax, emax), 1000.);
         //if(fEmax[count]!=original_emax) Info("ReadRangeTable", "Max. incident E for Z=%d  ===>  E/A = %f", count+1, fEmax[count]);
      }
      if (fgets(line, 132, fp)) {}
   }

   return kTRUE;
}

Double_t KVedaLossMaterial::DeltaEFunc(Double_t* E, Double_t*)
{
   // Function parameterising the energy loss of charged particles in this material.
   // The incident energy E[0] is given in MeV.
   // The energy loss is calculated in MeV.

   // if range < thickness, particle stops: dE = E0
   Double_t R0 = RangeFunc(E, 0);
   if (R0 < thickness) {
      return E[0];
   }

   // calculate energy loss - invert range function to find E corresponding to (R0 - thickness)
   R0 -= thickness;

   // invert range function to get energy after absorber
   return (E[0] - fRange->GetX(R0));

   /*
      // VEDALOSS inversion of R(E)
      // range in mg/cm**2 for VEDALOSS
      R0 /= (KVUnits::mg / pow(KVUnits::cm,2));
      // get parameters for this Z
      Int_t Z = (Int_t)Mypar[1];
      Double_t A = Mypar[2];
      Double_t *par = fCoeff[Z - 1];
      Double_t ranx = TMath::Log(R0);
      Double_t ranx1 = ranx - riso;
      Double_t depsx;
      if (ranx1 < arm)
         depsx = (ranx1 - adn) / adm;
      else {
         depsx = 0.0;
         for (register int j = 2; j < 7; j++)
            depsx += par[j + 7] * TMath::Power(ranx1, (Double_t) (j - 1));
         depsx += par[8];
      }

      const Double_t PERC = 0.02;

      Double_t eps1 = depsx + TMath::Log(1 - PERC);
      Double_t eps2 = depsx + TMath::Log(1 + PERC);
      Double_t rap = TMath::Log((1 + PERC) / (1 - PERC));

      Double_t rn1 = 0.0;
      if (TMath::Exp(eps1) < 0.1)
         rn1 = adm * eps1 + adn;
      else {
         for (register int j = 1; j < 7; j++)
            rn1 += par[j + 1] * TMath::Power(eps1, (Double_t) (j - 1));
      }
      Double_t rn2 = 0.0;
      if (TMath::Exp(eps2) < 0.1)
         rn2 = adm * eps2 + adn;
      else {
         for (register int j = 1; j < 7; j++)
            rn2 += par[j + 1] * TMath::Power(eps2, (Double_t) (j - 1));
      }

      Double_t epres = eps1 + (rap / (rn2 - rn1)) * (ranx1 - rn1);
      epres = TMath::Exp(epres);
      Double_t eres = A * epres;

      // garde-fou - calculated energy after absorber > incident energy ?!!
      //if(eres > E[0]) return 0.0;
      return E[0] - eres;
      */
}

Double_t KVedaLossMaterial::EResFunc(Double_t* E, Double_t*)
{
   // Function parameterising the residual energy of charged particles in this material.
   // The incident energy E[0] is given in MeV.
   // The residual energy is calculated in MeV.

   // if range < thickness, particle stops: Eres=0
   Double_t R0 = RangeFunc(E, 0);
   if (R0 < thickness) {
      return 0.0;
   }

   // calculate energy after absorber - invert range function to find Eres corresponding to (R0 - thickness)
   R0 -= thickness;

   // invert range function to get energy after absorber
   return fRange->GetX(R0);
}


TF1* KVedaLossMaterial::GetRangeFunction(Int_t Z, Int_t A, Double_t isoAmat)
{
   // Return function giving range (in g/cm**2) as a function of energy (in MeV) for
   // charged particles (Z,A) in this material.
   // If required, the isotopic mass of the material can be given.

   //fRange->SetParameters(Z, A, isoAmat);
   RF_Z = Z;
   RF_A = A;
   // get parameters for this Z
   par = fCoeff[Z - 1];
   // set up polynomial
   Double_t x1 = TMath::Log(0.1);
   Double_t x2 = TMath::Log(0.2);
   ran = 0.0;
   for (register int j = 2; j < 7; j++)
      ran += par[j + 1] * TMath::Power(x2, (Double_t)(j - 1));
   ran += par[2];
   Double_t y2 = ran;
   ran = 0.0;
   for (register int jj = 2; jj < 7; jj++)
      ran += par[jj + 1] * TMath::Power(x1, (Double_t)(jj - 1));
   ran += par[2];
   Double_t y1 = ran;
   adm = (y2 - y1) / (x2 - x1);
   adn = (y1 - adm * x1);
   riso = RF_A / par[1];
   if (isoAmat > 0.0) riso *= (isoAmat / fAmat);

   fRange->SetRange(0., GetEmaxValid(Z, A));
   return fRange;
}

TF1* KVedaLossMaterial::GetStoppingFunction(Int_t Z, Int_t A, Double_t isoAmat)
{
   // Return function giving stopping power (in MeV/(g/cm**2)) as a function of energy (in MeV) for
   // charged particles (Z,A) in this material.
   // If required, the isotopic mass of the material can be given.

   RF_Z = Z;
   RF_A = A;
   // get parameters for this Z
   par = fCoeff[Z - 1];
   // set up polynomial
   Double_t x1 = TMath::Log(0.1);
   Double_t x2 = TMath::Log(0.2);
   ran = 0.0;
   for (register int j = 2; j < 7; j++)
      ran += par[j + 1] * TMath::Power(x2, (Double_t)(j - 1));
   ran += par[2];
   Double_t y2 = ran;
   ran = 0.0;
   for (register int jj = 2; jj < 7; jj++)
      ran += par[jj + 1] * TMath::Power(x1, (Double_t)(jj - 1));
   ran += par[2];
   Double_t y1 = ran;
   adm = (y2 - y1) / (x2 - x1);
   adn = (y1 - adm * x1);
   riso = RF_A / par[1];
   if (isoAmat > 0.0) riso *= (isoAmat / fAmat);
   fStopping->SetRange(0., GetEmaxValid(Z, A));
   return fStopping;
}

Double_t KVedaLossMaterial::RangeFunc(Double_t* E, Double_t*)
{
   // Function parameterising the range of charged particles in this material.
   // The energy E[0] is given in MeV.
   // The range is calculated in units of g/cm**2

   eps = E[0] / RF_A;
   dleps = TMath::Log(eps);
   if (eps < 0.1)
      ran = adm * dleps + adn;
   else {
      DLEP = dleps;
      ran = par[2] + par[3] * DLEP;
      ran += par[4] * (DLEP *= dleps);
      ran += par[5] * (DLEP *= dleps);
      ran += par[6] * (DLEP *= dleps);
      ran += par[7] * (DLEP *= dleps);
   }

   // range in g/cm**2
   return riso * TMath::Exp(ran) * KVUnits::mg;
}

Double_t KVedaLossMaterial::StoppingFunc(Double_t* E, Double_t*)
{
   // Function parameterising the stopping of charged particles in this material.
   // The energy E[0] is given in MeV.
   // The stopping power is calculated in units of MeV/(g/cm**2)

   eps = E[0] / RF_A;
   dleps = TMath::Log(eps);
   if (eps < 0.1) {
      ran = adm * dleps + adn;
      drande = E[0] / (riso * TMath::Exp(ran) * KVUnits::mg) / adm;
      return drande;
   }
   DLEP = dleps;
   ran = par[2] + par[3] * DLEP;
   drande = par[3];
   for (register int i = 4; i < 8; i++) {
      drande += (i - 2) * par[i] * DLEP;
      ran += par[i] * (DLEP *= dleps);
   }
   // range in g/cm**2
   return E[0] / (riso * TMath::Exp(ran) * KVUnits::mg) / drande;
}

TF1* KVedaLossMaterial::GetDeltaEFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat)
{
   // Return function giving energy loss (in MeV) as a function of incident energy (in MeV) for
   // charged particles (Z,A) traversing (or not) the thickness e (in g/cm**2) of this material.
   // If required, the isotopic mass of the material can be given.

   GetRangeFunction(Z, A, isoAmat);
   thickness = e;
   fDeltaE->SetRange(0., GetEmaxValid(Z, A));
   return fDeltaE;
}

TF1* KVedaLossMaterial::GetEResFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat)
{
   // Return function giving residual energy (in MeV) as a function of incident energy (in MeV) for
   // charged particles (Z,A) traversing (or not) the thickness e (in g/cm**2) of this material.
   // If required, the isotopic mass of the material can be given.

   GetRangeFunction(Z, A, isoAmat);
   thickness = e;
   fEres->SetRange(0., GetEmaxValid(Z, A));
   return fEres;
}

Double_t KVedaLossMaterial::GetRangeOfIon(Int_t Z, Int_t A, Double_t E, Double_t isoAmat)
{
   // Returns range (in g/cm**2) of ion (Z,A) with energy E (MeV) in material.
   // Give Amat to change default (isotopic) mass of material,

   /*if(E>GetEmaxValid(Z,A))
      Warning("GetRangeOfIon", "Incident energy of (%d,%d) > limit of validity of KVedaLoss (Emax=%f)",
            Z,A,GetEmaxValid(Z,A));*/
   TF1* f = GetRangeFunction(Z, A, isoAmat);
   return f->Eval(E);
}

Double_t KVedaLossMaterial::GetDeltaEOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat)
{
   // Returns energy lost (in MeV) by ion (Z,A) with energy E (MeV) after thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,

   /* if(E>GetEmaxValid(Z,A))
      Warning("GetDeltaEOfIon", "Incident energy of (%d,%d) > limit of validity of KVedaLoss (Emax=%f)",
            Z,A,GetEmaxValid(Z,A)); */
   TF1* f = GetDeltaEFunction(e, Z, A, isoAmat);
   return f->Eval(E);
}

Double_t KVedaLossMaterial::GetEResOfIon(Int_t Z, Int_t A, Double_t E, Double_t e,
      Double_t isoAmat)
{
   // Returns residual energy (in MeV) of ion (Z,A) with energy E (MeV) after thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material

   /* if(E>(GetEmaxValid(Z,A)+0.1))
      Warning("GetEResOfIon", "Incident energy of (%d,%d) %f MeV/A > limit of validity of KVedaLoss (Emax=%f MeV/A)",
            Z,A,E/A,GetEmaxValid(Z,A)/A); */
   TF1* f = GetEResFunction(e, Z, A, isoAmat);
   return f->Eval(E);
}

void KVedaLossMaterial::GetParameters(Int_t Zion, Int_t& Aion, Double_t*& rangepar)
{
   // For the given ion atomic number, give the reference mass used and the six
   // parameters for the range function fit

   rangepar = &fCoeff[Zion - 1][2];
   Aion = fCoeff[Zion - 1][1];
}

