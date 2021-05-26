//Created by KVClassFactory on Wed Feb  2 15:49:27 2011
//Author: frankland,,,,

#include "KVedaLoss.h"
#include "KVedaLossMaterial.h"
#include "KVedaLossRangeFitter.h"
#include <TString.h>
#include <TSystem.h>
#include <TEnv.h>
#include <KVSystemDirectory.h>
#include <KVSystemFile.h>
#include "TGeoMaterial.h"

ClassImp(KVedaLoss)

KVHashList* KVedaLoss::fMaterials = 0x0;
Bool_t KVedaLoss::fgNewRangeInversion = kTRUE;

void KVedaLoss::SetIgnoreEnergyLimits(Bool_t yes)
{
   // Call this static method with yes=kTRUE in order to recalculate the nominal limits
   // on incident ion energies for which the range tables are valid.
   //
   // Normally all range, \f$dE\f$, \f$E_{res}\f$ functions are limited to range \f$0\leq E\leq E_{max}\f$,
   // where \f$E_{max}\f$ is nominal maximum energy for which range tables are valid
   // (usually 400MeV/u for \f$Z<3\f$, 250MeV/u for \f$Z>3\f$).
   //
   // If higher energies are required, call static method KVedaLoss::SetIgnoreEnergyLimits() **BEFORE ANY MATERIALS ARE CREATED**
   // in order to recalculate the \f$E_{max}\f$ limits in such a way that:
   //   -  range function is always monotonically increasing function of \f$E_{inc}\f$;
   //   -  stopping power is concave (i.e. no minimum of stopping power followed by an increase)
   //
   // Then, at the most, the new limit will be 1 GeV/nucleon, or
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

   fLocalMaterialsDirectory = GetWORKDIRFilePath("VEDALOSS");
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
   //
   // any files in $(WORKING_DIR)/vedaloss/*.dat will also be read, these contain
   // materials added by the user(s)

   fMaterials = new KVHashList;
   fMaterials->SetName("VEDALOSS materials list");
   fMaterials->SetOwner();

   TString DataFilePath;
   if (!KVBase::SearchKVFile(gEnv->GetValue("KVedaLoss.RangeTables", "kvloss.data"), DataFilePath, "data")) {
      Error("init_materials()", "Range tables file %s not found", gEnv->GetValue("KVedaLoss.RangeTables", "kvloss.data"));
      return kFALSE;
   }

   bool ok = ReadMaterials(DataFilePath);

   if (!gSystem->AccessPathName(fLocalMaterialsDirectory)) {
      // read all user materials in directory
      KVSystemDirectory matDir("matDir", fLocalMaterialsDirectory);
      TIter nxtfil(matDir.GetListOfFiles());
      KVSystemFile* fil;
      while ((fil = (KVSystemFile*)nxtfil())) {
         if (TString(fil->GetName()).EndsWith(".dat")) ok = ok && ReadMaterials(fil->GetFullPath());
      }
   }
   return ok;
}

Bool_t KVedaLoss::ReadMaterials(const Char_t* DataFilePath) const
{
   // Read and add range tables for materials in file

   Char_t name[25], gtype[25], state[10];
   Float_t Amat = 0.;
   Float_t Dens = 0.;
   Float_t MoleWt = 0.;
   Float_t Temp = 19.;
   Float_t Zmat = 0.;
   int mat_count = 0;

   FILE* fp;
   if (!(fp = fopen(DataFilePath, "r"))) {
      Error("init_materials()", "Range tables file %s cannot be opened", DataFilePath);
      return kFALSE;
   }
   else {
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
                  Error("init_materials()", "Problem reading file %s", DataFilePath);
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
               if (tmp_mat->IsGas()) tmp_mat->SetTemperatureAndPressure(19., 1.*KVUnits::atm);
               break;
         }
      }
      fclose(fp);
   }
   return kTRUE;
}

KVIonRangeTableMaterial* KVedaLoss::AddElementalMaterial(Int_t Z, Int_t A) const
{
   // Use the RANGE tables to generate a new material of a given element.
   // If A is given only one isotope will be used, by default the natural abundance
   // of each isotope is used to generate a mixture

   unique_ptr<KVIonRangeTable> yanez(KVIonRangeTable::GetRangeTable("RANGE"));
   KVIonRangeTableMaterial* mat = yanez->AddElementalMaterial(Z, A);
   AddMaterial(mat);
   return GetMaterial(mat->GetName());
}

Bool_t KVedaLoss::AddRANGEMaterial(const Char_t* name) const
{
   // If the given material is defined in the RANGE tables, import it into VEDALOSS

   unique_ptr<KVIonRangeTable> yanez(KVIonRangeTable::GetRangeTable("RANGE"));
   if (yanez->GetMaterial(name)) {
      AddMaterial(yanez->GetMaterial(name));
      return kTRUE;
   }
   return kFALSE;
}

KVIonRangeTableMaterial* KVedaLoss::AddCompoundMaterial(const Char_t* name, const Char_t* symbol, Int_t nel, Int_t* Z, Int_t* A, Int_t* nat, Double_t dens) const
{
   // Use the RANGE tables to generate a new compound material

   unique_ptr<KVIonRangeTable> yanez(KVIonRangeTable::GetRangeTable("RANGE"));
   KVIonRangeTableMaterial* mat = yanez->AddCompoundMaterial(name, symbol, nel, Z, A, nat, dens);
   AddMaterial(mat);
   return GetMaterial(mat->GetName());
}

KVIonRangeTableMaterial* KVedaLoss::AddMixedMaterial(const Char_t* name, const Char_t* symbol, Int_t nel, Int_t* Z, Int_t* A, Int_t* nat, Double_t* prop, Double_t dens) const
{
   // Use the RANGE tables to generate a new mixed material

   unique_ptr<KVIonRangeTable> yanez(KVIonRangeTable::GetRangeTable("RANGE"));
   KVIonRangeTableMaterial* mat = yanez->AddMixedMaterial(name, symbol, nel, Z, A, nat, prop, dens);
   AddMaterial(mat);
   return GetMaterial(mat->GetName());
}

//________________________________________________________________________________//

KVIonRangeTableMaterial* KVedaLoss::GetMaterialWithNameOrType(const Char_t* material) const
{
   // Returns pointer to material of given name or type.
   KVIonRangeTableMaterial* M = (KVIonRangeTableMaterial*)fMaterials->FindObject(material);
   if (!M) {
      M = (KVIonRangeTableMaterial*)fMaterials->FindObjectByType(material);
   }
   return M;
}

void KVedaLoss::AddMaterial(KVIonRangeTableMaterial* mat) const
{
   // Add a material (taken from a different range table) to VEDALOSS
   // This means fitting the ranges for Z=1-100 and writing the parameters in a
   // file which will be stored in
   //
   //    $(WORKING_DIR)/VEDALOSS/[name].dat
   //
   // which will be read at each initialisation to include the new material

   KVedaLossRangeFitter vlfit;
   vlfit.SetMaterial(mat);
   TString matname = mat->GetName();
   matname.ReplaceAll(" ", "_"); //no spaces in filename
   matname += ".dat";
   // check directory exists & make it if necessary
   if (gSystem->AccessPathName(fLocalMaterialsDirectory)) {
      gSystem->mkdir(fLocalMaterialsDirectory, true);
      gSystem->Chmod(fLocalMaterialsDirectory, 0755);
   }
   vlfit.DoFits(Form("%s/%s", fLocalMaterialsDirectory.Data(), matname.Data()));
   ReadMaterials(Form("%s/%s", fLocalMaterialsDirectory.Data(), matname.Data()));
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
