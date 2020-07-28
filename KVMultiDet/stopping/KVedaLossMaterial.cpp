//Created by KVClassFactory on Wed Feb  2 16:13:08 2011
//Author: frankland,,,,

#include "KVedaLossMaterial.h"
#include <TMath.h>
#include "KVIonRangeTable.h"
#include <TEnv.h>
#include <TF1.h>
#include "KVedaLossInverseRangeFunction.h"
#include "KVedaLoss.h"

ClassImp(KVedaLossMaterial)

Bool_t KVedaLossMaterial::fNoLimits = kFALSE;
KVedaLoss* KVedaLossMaterial::fgTable = nullptr;

KVedaLossMaterial::KVedaLossMaterial()
   : KVIonRangeTableMaterial(), fInvRange(ZMAX_VEDALOSS, 1),
     fEmin(ZMAX_VEDALOSS), fEmax(ZMAX_VEDALOSS), fCoeff(ZMAX_VEDALOSS, std::vector<Double_t>(14))
{
   // Default constructor
   for (int i = 0; i < ZMAX_VEDALOSS; i++) {
      fEmin[i] = 0.0;
      fEmax[i] = 500.0;
   }
   fInvRange.SetOwner();
}

KVedaLossMaterial::KVedaLossMaterial(const KVIonRangeTable* t, const Char_t* name, const Char_t* type, const Char_t* state,
                                     Double_t density, Double_t Z, Double_t A, Double_t)
   : KVIonRangeTableMaterial(t, name, type, state, density, Z, A), fInvRange(ZMAX_VEDALOSS, 1),
     fEmin(ZMAX_VEDALOSS), fEmax(ZMAX_VEDALOSS), fCoeff(ZMAX_VEDALOSS, std::vector<Double_t>(14))
{
   // create new material
   fgTable = static_cast<KVedaLoss*>(const_cast<KVIonRangeTable*>(t));
   for (int i = 0; i < ZMAX_VEDALOSS; i++) {
      fEmin[i] = 0.0;
      fEmax[i] = 500.0;
   }
   fInvRange.SetOwner();
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
   //   - KVedaLossMaterial:[type]:Range                -  gives range in g/cm**2 as a function of particle energy
   //   - KVedaLossMaterial:[type]:StoppingPower        -  gives dE/dx in MeV/(g/cm**2) as a function of particle energy
   //   - KVedaLossMaterial:[type]:EnergyLoss           -  gives dE as a function of particle energy
   //   - KVedaLossMaterial:[type]:ResidualEnergy       -  gives energy after material (0 if particle stops)
   //
   // The TF1::fNpx parameter for these functions is defined by the environment variables
   //
   //   - KVedaLoss.Range.Npx:         20      /* also used for StoppingPower */
   //   - KVedaLoss.EnergyLoss.Npx:         50
   //   - KVedaLoss.ResidualEnergy.Npx:         20
   //

   char line[132];

   //look for energy limits to calculation validity
   if (!fgets(line, 132, fp)) {
      Warning("ReadRangeTable", "Problem reading energy limits in range table file for %s (%s)",
              GetName(), GetType());
      return kFALSE;
   }
   else {
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
      }
      else if (!strncmp(line, "MIXTURE", 7)) {
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
   }
   else {
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

   for (int count = 0; count < ZMAX_VEDALOSS; count++) {

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
      }
      else {
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
   //
   // \param E[0] incident energy given in MeV.
   // \returns  energy loss calculated in MeV.

   return (E[0] - EResFunc(E, nullptr));
}

Double_t KVedaLossMaterial::EResFunc(Double_t* E, Double_t*)
{
   // Function parameterising the residual energy of charged particles in this material.
   //
   // \param E[0] incident energy given in MeV.
   // \returns  residual energy calculated in MeV.

   Double_t R0 = RangeFunc(E, nullptr);
   if (R0 < thickness) {// if range < thickness, particle stops: Eres=0
      fRangeOfLastDE = R0;
      return 0.0;
   }
   else
      fRangeOfLastDE = thickness;

   // calculate energy after absorber - invert range function to find Eres corresponding to (R0 - thickness)
   R0 -= thickness;

   // invert range function to get energy after absorber
   if (fgTable->IsUseNewRangeInversion()) {
      return static_cast<KVedaLossInverseRangeFunction*>(fInvRange[RF_Z])->GetEnergyPerNucleon(R0, riso) * RF_A;
   }
   return fRange->GetX(R0);
}


TF1* KVedaLossMaterial::GetRangeFunction(Int_t Z, Int_t A, Double_t isoAmat)
{
   // Return function giving range (in \f$g/cm^2\f$) as a function of energy (in MeV) for
   // charged particles \f$Z,A\f$ in this material.
   // \param isoAmat If required, the isotopic mass of the material can be given.

   RF_Z = Z;
   RF_A = A;
   // get parameters for this Z
   par = &fCoeff[Z - 1];
   // set up polynomial
   Double_t x1 = TMath::Log(0.1);
   Double_t x2 = TMath::Log(0.2);
   ran = 0.0;
   for (int j = 2; j < 7; j++)
      ran += (*par)[j + 1] * TMath::Power(x2, (Double_t)(j - 1));
   ran += (*par)[2];
   Double_t y2 = ran;
   ran = 0.0;
   for (int jj = 2; jj < 7; jj++)
      ran += (*par)[jj + 1] * TMath::Power(x1, (Double_t)(jj - 1));
   ran += (*par)[2];
   Double_t y1 = ran;
   adm = (y2 - y1) / (x2 - x1);
   adn = (y1 - adm * x1);
   riso = RF_A / (*par)[1];
   if (isoAmat > 0.0) riso *= (isoAmat / fAmat);

   fRange->SetRange(0., GetEmaxValid(Z, A));

   /*
    * New inversion of range-energy curve (if KVedaLoss::fgNewRangeInversion=kTRUE)
    */
   if (fgTable->IsUseNewRangeInversion()) {
      if (!fInvRange[Z]) {
         fInvRange[Z] = new KVedaLossInverseRangeFunction(fRange, A, riso);
      }
   }

   return fRange;
}

TF1* KVedaLossMaterial::GetStoppingFunction(Int_t Z, Int_t A, Double_t isoAmat)
{
   // Return function giving stopping power (in \f$MeV/(g/cm^2)\f$) as a function of energy (in MeV) for
   // charged particles \f$Z,A\f$ in this material.
   // \param isoAmat If required, the isotopic mass of the material can be given.

   RF_Z = Z;
   RF_A = A;
   // get parameters for this Z
   par = &fCoeff[Z - 1];
   // set up polynomial
   Double_t x1 = TMath::Log(0.1);
   Double_t x2 = TMath::Log(0.2);
   ran = 0.0;
   for (int j = 2; j < 7; j++)
      ran += (*par)[j + 1] * TMath::Power(x2, (Double_t)(j - 1));
   ran += (*par)[2];
   Double_t y2 = ran;
   ran = 0.0;
   for (int jj = 2; jj < 7; jj++)
      ran += (*par)[jj + 1] * TMath::Power(x1, (Double_t)(jj - 1));
   ran += (*par)[2];
   Double_t y1 = ran;
   adm = (y2 - y1) / (x2 - x1);
   adn = (y1 - adm * x1);
   riso = RF_A / (*par)[1];
   if (isoAmat > 0.0) riso *= (isoAmat / fAmat);
   fStopping->SetRange(0., GetEmaxValid(Z, A));
   return fStopping;
}

Double_t KVedaLossMaterial::RangeFunc(Double_t* E, Double_t*)
{
   // Function parameterising the range of charged particles in this material.
   // \param E[0] energy is given in MeV.
   // \returns range calculated in units of \f$g/cm^2\f$

   eps = E[0] / RF_A;
   dleps = TMath::Log(eps);
   if (eps < 0.1)
      ran = adm * dleps + adn;
   else {
      DLEP = dleps;
      ran = (*par)[2] + (*par)[3] * DLEP;
      ran += (*par)[4] * (DLEP *= dleps);
      ran += (*par)[5] * (DLEP *= dleps);
      ran += (*par)[6] * (DLEP *= dleps);
      ran += (*par)[7] * (DLEP *= dleps);
   }

   // range in g/cm**2
   return riso * TMath::Exp(ran) * KVUnits::mg;
}

Double_t KVedaLossMaterial::StoppingFunc(Double_t* E, Double_t*)
{
   // Function parameterising the stopping power of charged particles in this material.
   // \param E[0] energy is given in MeV.
   // \returns stopping power calculated in units of \f$Mev/(g/cm^2)\f$

   eps = E[0] / RF_A;
   dleps = TMath::Log(eps);
   if (eps < 0.1) {
      ran = adm * dleps + adn;
      drande = E[0] / (riso * TMath::Exp(ran) * KVUnits::mg) / adm;
      return drande;
   }
   DLEP = dleps;
   ran = (*par)[2] + (*par)[3] * DLEP;
   drande = (*par)[3];
   for (int i = 4; i < 8; i++) {
      drande += (i - 2) * (*par)[i] * DLEP;
      ran += (*par)[i] * (DLEP *= dleps);
   }
   // range in g/cm**2
   return E[0] / (riso * TMath::Exp(ran) * KVUnits::mg) / drande;
}

TF1* KVedaLossMaterial::GetDeltaEFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat)
{
   // Return function giving energy loss (in MeV) as a function of incident energy (in MeV) for
   // charged particles (Z,A) traversing (or not) material
   // \param e thickness (in \f$g/cm^2\f$) of this material.
   // \param isoAmat If required, the isotopic mass of the material can be given.

   GetRangeFunction(Z, A, isoAmat);
   thickness = e;
   fDeltaE->SetRange(0., GetEmaxValid(Z, A));
   return fDeltaE;
}

TF1* KVedaLossMaterial::GetEResFunction(Double_t e, Int_t Z, Int_t A, Double_t isoAmat)
{
   // Return function giving residual energy (in MeV) as a function of incident energy (in MeV) for
   // charged particles (Z,A) traversing (or not) material
   // \param e thickness (in \f$g/cm^2\f$) of this material.
   // \param isoAmat If required, the isotopic mass of the material can be given.

   GetRangeFunction(Z, A, isoAmat);
   thickness = e;
   fEres->SetRange(0., GetEmaxValid(Z, A));
   return fEres;
}

Double_t KVedaLossMaterial::GetRangeOfIon(Int_t Z, Int_t A, Double_t E, Double_t isoAmat)
{
   // \returns range (in \f$g/cm^2\f$) of ion (Z,A) with energy E (MeV) in material.
   // \param isoAmat change default (isotopic) mass of material,

   if (Z == 0) return 0.0; //only charged particles
   TF1* f = GetRangeFunction(Z, A, isoAmat);
   return f->Eval(E);
}

Double_t KVedaLossMaterial::GetDeltaEOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat)
{
   // \returns energy lost (in MeV) by ion (Z,A)
   // \param E incident energy (MeV)
   // \param e thickness (in \f$g/cm^2\f$)
   // \param isoAmat change default (isotopic) mass of material,

   if (Z == 0) return 0.0; //only charged particles
   TF1* f = GetDeltaEFunction(e, Z, A, isoAmat);
   return f->Eval(E);
}

Double_t KVedaLossMaterial::GetEResOfIon(Int_t Z, Int_t A, Double_t E, Double_t e,
      Double_t isoAmat)
{
   // \returns energy lost (in MeV) by ion (Z,A)
   // \param E incident energy (MeV)
   // \param e thickness (in \f$g/cm^2\f$)
   // \param isoAmat change default (isotopic) mass of material,

   if (Z == 0) return 0.0; //only charged particles
   TF1* f = GetEResFunction(e, Z, A, isoAmat);
   return f->Eval(E);
}

Double_t KVedaLossMaterial::GetPunchThroughEnergy(Int_t Z, Int_t A, Double_t e, Double_t isoAmat)
{
   // Calculate incident energy (in MeV) for ion (Z,A) for which the range is equal to the
   // given thickness e (in \f$g/cm^2\f$). At this energy the residual energy of the ion is (just) zero,
   // for all energies above this energy the residual energy is > 0.
   // \param isoAmat change default (isotopic) mass of material,

   if (Z == 0) return 0.0; //only charged particles
   GetRangeFunction(Z, A, isoAmat);
   if (fgTable->IsUseNewRangeInversion()) {
      return static_cast<KVedaLossInverseRangeFunction*>(fInvRange[Z])->GetEnergyPerNucleon(e, riso) * A;
   }
   return fRange->GetX(e);
}

Double_t KVedaLossMaterial::GetEIncFromEResOfIon(Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) with residual energy Eres (MeV) after thickness e (in \f$g/cm^2\f$).
   // \param isoAmat change default (isotopic) mass of material,

   if (Z == 0) return 0.0; //only charged particles
   GetRangeFunction(Z, A, isoAmat);
   Double_t R0 = fRange->Eval(Eres) + e;
   if (fgTable->IsUseNewRangeInversion()) {
      return static_cast<KVedaLossInverseRangeFunction*>(fInvRange[Z])->GetEnergyPerNucleon(R0, riso) * A;
   }
   return fRange->GetX(R0);
}

void KVedaLossMaterial::GetParameters(Int_t Zion, Int_t& Aion, std::vector<Double_t>& rangepar)
{
   // For the given ion atomic number, give the reference mass used and the six
   // parameters for the range function fit
   // \param[in] Zion ion atomic number
   // \param[out] Aion reference mass used by table
   // \param[out] rangepar vector containing the six parameters for the range function fit

   rangepar = std::vector<Double_t>(fCoeff[Zion - 1].begin() + 2, fCoeff[Zion - 1].end());
   Aion = fCoeff[Zion - 1][1];
}

