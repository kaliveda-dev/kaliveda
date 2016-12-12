//Created by KVClassFactory on Wed Feb  2 15:49:27 2011
//Author: frankland,,,,

#include "KVedaLoss.h"
#include "KVedaLossMaterial.h"
#include <TString.h>
#include <TSystem.h>
#include <TEnv.h>
#include "TGeoMaterial.h"

ClassImp(KVedaLoss)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVedaLoss</h2>
<h4>C++ implementation of VEDALOSS stopping power calculation</h4>
See documentation <a href="KVedaLossDoc/KVedaLoss.html">here</a>.
<!-- */
// --> END_HTML
// Energy limits
// Normally all range, dE, Eres functions are limited to range 0<=E<=Emax,
// where Emax is nominal max energy for which range tables are valid
// (usually 400MeV/u for Z<3, 250MeV/u for Z>3)
// If higher energies are required, call static method KVedaLoss::SetIgnoreEnergyLimits()
// BEFORE ANY MATERIALS ARE CREATED
// in order to recalculate the Emax limits in such a way that:
//     range function is always monotonically increasing function of Einc
//     stopping power is concave (i.e. no minimum of stopping power followed by an increase)
// at the most, the new limit will be 1 GeV/nucleon.
// at the least, it will remain at the nominal (400 or 250 MeV/nucleon) level.
////////////////////////////////////////////////////////////////////////////////

KVHashList* KVedaLoss::fMaterials = 0x0;
Bool_t KVedaLoss::fgNewRangeInversion = kTRUE;

void KVedaLoss::SetIgnoreEnergyLimits(Bool_t yes)
{
   // Call this static method with yes=kTRUE in order to recalculate the nominal limits
   // on incident ion energies for which the range tables are valid.
   //
   // Normally all range, dE, Eres functions are limited to range 0<=E<=Emax,
   // where Emax is nominal max energy for which range tables are valid
   // (usually 400MeV/u for Z<3, 250MeV/u for Z>3)
   // If higher energies are required, call this static method in order to recalculate the Emax limits
   // in such a way that:
   //     range function is always monotonically increasing function of Einc
   //     stopping power is concave (i.e. no minimum of stopping power followed by an increase)
   // at the most, the new limit will be 1 GeV/nucleon.
   // at the least, it will remain at the nominal (400 or 250 MeV/nucleon) level.
   KVedaLossMaterial::SetNoLimits(yes);
}

Bool_t KVedaLoss::CheckIon(Int_t Z, Int_t) const
{
   return KVedaLossMaterial::CheckIon(Z);
}

KVedaLoss::KVedaLoss()
   : KVIonRangeTable("VEDALOSS",
                     "Calculation of range and energy loss of charged particles in matter using VEDALOSS range tables")
{
   // Default constructor
   if (!CheckMaterialsList()) {
      Error("KVedaLoss", "Problem reading range tables. Do not use.");
   }
}

KVedaLoss::~KVedaLoss()
{
   // Destructor
}

Bool_t KVedaLoss::init_materials() const
{
   // PRIVATE method - called to initialize fMaterials list of all known materials
   // properties, read from file given by TEnv variable KVedaLoss.RangeTables

   Info("init_materials", "Initialising KVedaLoss...");
//    printf("\n");
//    printf("\t*************************************************************************\n");
//    printf("\t*                VEDALOSS STOPPING POWER & RANGE TABLES                 *\n");
//    printf("\t*                                                                       *\n");
   int mat_count = 0;
   fMaterials = new KVHashList;
   fMaterials->SetName("VEDALOSS materials list");
   fMaterials->SetOwner();

   TString DataFilePath;
   if (!KVBase::SearchKVFile(gEnv->GetValue("KVedaLoss.RangeTables", "kvloss.data"), DataFilePath, "data")) {
      Error("init_materials()", "Range tables file %s not found", gEnv->GetValue("KVedaLoss.RangeTables", "kvloss.data"));
      return kFALSE;
   }

   Char_t name[25], gtype[25], state[10];
   Float_t Amat = 0.;
   Float_t Dens = 0.;
   Float_t MoleWt = 0.;
   Float_t Temp = 19.;
   Float_t Zmat = 0.;

   FILE* fp;
   if (!(fp = fopen(DataFilePath.Data(), "r"))) {
      Error("init_materials()", "Range tables file %s cannot be opened", DataFilePath.Data());
      return kFALSE;
   } else {
      char line[132];
      while (fgets(line, 132, fp)) {    // read lines from file

         switch (line[0]) {

            case '/':             // ignore comment lines
               break;

            case '+':             // header lines

               if (sscanf(line, "+ %s %s %s %f %f %f %f %f",
                          gtype, name, state, &Dens, &Zmat, &Amat,
                          &MoleWt, &Temp)
                     != 8) {
                  Error("init_materials()", "Problem reading file %s", DataFilePath.Data());
                  fclose(fp);
                  return kFALSE;
               }
//found a new material
               KVedaLossMaterial* tmp_mat = new KVedaLossMaterial(this, name, gtype, state, Dens,
                     Zmat, Amat, MoleWt);
               fMaterials->Add(tmp_mat);
               if (!tmp_mat->ReadRangeTable(fp)) return kFALSE;
               tmp_mat->Initialize();
               ++mat_count;
//               Double_t rho = 0.;
               if (tmp_mat->IsGas()) tmp_mat->SetTemperatureAndPressure(19., 1.*KVUnits::atm);
//               rho = tmp_mat->GetDensity();
//                printf("\t*  %2d.  %-7s %-18s  Z=%2d A=%5.1f  rho=%6.3f g/cm**3    *\n",
//                       mat_count, tmp_mat->GetType(), tmp_mat->GetName(),
//                       (int)tmp_mat->GetZ(), tmp_mat->GetMass(),
//                       rho);
               break;
         }
      }
      fclose(fp);
   }
//    printf("\t*                                                                       *\n");
//    printf("\t*     TF1::Range::Npx = %4d            TF1::EnergyLoss::Npx = %4d     *\n",
//           gEnv->GetValue("KVedaLoss.Range.Npx", 100), gEnv->GetValue("KVedaLoss.EnergyLoss.Npx", 100));
//    printf("\t*                      TF1::ResidualEnergy::Npx = %4d                  *\n",
//           gEnv->GetValue("KVedaLoss.ResidualEnergy.Npx", 100));
//    printf("\t*                                                                       *\n");
//    printf("\t*                       INITIALISATION COMPLETE                         *\n");
//    printf("\t*************************************************************************\n");
   return kTRUE;
}

//________________________________________________________________________________//

KVIonRangeTableMaterial* KVedaLoss::GetMaterialWithNameOrType(const Char_t* material)
{
   // Returns pointer to material of given name or type.
   KVIonRangeTableMaterial* M = (KVIonRangeTableMaterial*)fMaterials->FindObject(material);
   if (!M) {
      M = (KVIonRangeTableMaterial*)fMaterials->FindObjectByType(material);
   }
   return M;
}

void KVedaLoss::Print(Option_t*) const
{
   printf("KVedaLoss::%s\n%s\n", GetName(), GetTitle());
   printf("\nEnergy loss & range tables loaded for %d materials:\n\n", fMaterials->GetEntries());
   fMaterials->ls();
}

TObjArray* KVedaLoss::GetListOfMaterials()
{
   // Create and fill a list of all materials for which range tables exist.
   // Each entry is a TNamed with the name and type (title) of the material.
   // User's responsibility to delete list after use (it owns its objects).

   if (CheckMaterialsList()) {
      TObjArray* list = new TObjArray(fMaterials->GetEntries());
      list->SetOwner(kTRUE);
      TIter next(fMaterials);
      KVedaLossMaterial* mat;
      while ((mat = (KVedaLossMaterial*)next())) {
         list->Add(new TNamed(mat->GetName(), mat->GetType()));
      }
      return list;
   }
   return 0;
}
