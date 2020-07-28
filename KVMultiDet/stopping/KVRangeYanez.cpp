//Created by KVClassFactory on Thu Sep 27 14:48:55 2012
//Author: John Frankland,,,

#include "KVRangeYanez.h"
#include "KVRangeYanezMaterial.h"
#include "KVElementDensity.h"
#include "KVElementDensityTable.h"
#include "KVNDTManager.h"
#include "TString.h"
#include "KVNucleus.h"
#include "KVNumberList.h"
#include <KVSystemDirectory.h>
#include <KVSystemFile.h>
#include <Riostream.h>
using namespace std;

ClassImp(KVRangeYanez)

KVHashList* KVRangeYanez::fMaterials = 0x0;

KVRangeYanez::KVRangeYanez()
   : KVIonRangeTable("RANGE",
                     "Interface to Range dE/dx and range library (Ricardo Yanez)")
{
   // Default constructor
   //
   // Predefined materials are created based on the contents of the file(s) whose
   // names are given as values of the variable `RANGE.PredefMaterials`
   //
   // A default file is specified in the main `.kvrootrc` file.
   //
   // If you want to add your own definitions, just put in your `.kvrootrc` file:
   //~~~~~~~~~~~
   //+RANGE.PredefMaterials: myfile1.dat
   //+RANGE.PredefMaterials: myfile2.dat
   //~~~~~~~~~~~
   // If you want to override the default definitions:
   //~~~~~~~~~~~
   //RANGE.PredefMaterials: myfile1.dat
   //+RANGE.PredefMaterials: myfile2.dat
   //~~~~~~~~~~~

   KVString DataFilePaths = gEnv->GetValue("RANGE.PredefMaterials", "");
   DataFilePaths.Begin(" ");
   KVString nextPath;
   KVString lastPath;
   while (!DataFilePaths.End()) {
      nextPath = DataFilePaths.Next();
      if (nextPath == lastPath) break; //check for double occurrence of last file : TEnv bug?
      lastPath = nextPath;
      ReadMaterials(nextPath);
   }

   // directory where any materials defined by user are stored
   fLocalMaterialsDirectory = GetWORKDIRFilePath("RANGE");
   if (!gSystem->AccessPathName(fLocalMaterialsDirectory)) {
      // read all materials in directory if it exists
      KVSystemDirectory matDir("matDir", fLocalMaterialsDirectory);
      TIter nxtfil(matDir.GetListOfFiles());
      KVSystemFile* fil;
      while ((fil = (KVSystemFile*)nxtfil())) {
         if (TString(fil->GetName()).EndsWith(".dat")) ReadMaterials(fil->GetFullPath());
      }
   }
   fDoNotSaveMaterials = kFALSE;
}

//________________________________________________________________

KVRangeYanez::KVRangeYanez(const KVRangeYanez& obj)  : KVIonRangeTable()
{
   obj.Copy(*this);
}

//________________________________________________________________

void KVRangeYanez::Copy(TObject& obj) const
{
   KVIonRangeTable::Copy(obj);
   KVRangeYanez& CastedObj = (KVRangeYanez&)obj;
   CastedObj.fLocalMaterialsDirectory = fLocalMaterialsDirectory;
   CastedObj.fDoNotSaveMaterials = fDoNotSaveMaterials;
}

KVIonRangeTableMaterial* KVRangeYanez::GetMaterialWithNameOrType(const Char_t* material) const
{
   // Returns pointer to material of given name or type if it has been defined.
   //
   // \param[in] material name or type of material to retrieve

   CheckMaterialsList();
   KVIonRangeTableMaterial* M = (KVIonRangeTableMaterial*)fMaterials->FindObject(material);
   if (!M) {
      M = (KVIonRangeTableMaterial*)fMaterials->FindObjectByType(material);
   }
   return M;
}

void KVRangeYanez::Print(Option_t*) const
{
   printf("KVRangeYanez::%s\n%s\n", GetName(), GetTitle());
   Int_t n = (fMaterials ? fMaterials->GetEntries() : 0);
   if (n) {
      printf("\nEnergy loss & range tables loaded for %d materials:\n\n", fMaterials->GetEntries());
      fMaterials->Print();
   }
   else
      printf("\nEnergy loss & range tables loaded for 0 materials.\n");
}

TObjArray* KVRangeYanez::GetListOfMaterials()
{
   // Create and fill a list of all materials for which range tables exist.
   //
   // Each entry is a TNamed with the name and type (title) of the material.
   //
   // User's responsibility to delete list after use (it owns its objects).

   TObjArray* list = new TObjArray(fMaterials->GetEntries());
   list->SetOwner(kTRUE);
   TIter next(fMaterials);
   KVIonRangeTableMaterial* mat;
   while ((mat = (KVIonRangeTableMaterial*)next())) {
      list->Add(new TNamed(mat->GetName(), mat->GetType()));
   }
   return list;
}
void KVRangeYanez::CheckMaterialsList() const
{
   if (!fMaterials) {
      fMaterials = new KVHashList;
      fMaterials->SetName("RANGE materials list");
      fMaterials->SetOwner();
   }
}
KVIonRangeTableMaterial* KVRangeYanez::AddElementalMaterial(Int_t z, Int_t a) const
{
   // Adds a material composed of a single isotope of a chemical element.
   //
   // \param[in] z atomic number of element
   // \param[in] a mass number of element
   //
   // If the mass number of the isotope \f$A\f$ is not specified, we create a material containing the naturally
   // occuring isotopes of the given element, weighted according to natural abundance.
   //
   // If the mass is given, the material symbol will be `"AX"` where `X` is the symbol for the element
   //     e.g. `"48Ca"`,  `"124Sn"`, etc.
   // and the material name will be `"Xxx-A"` where `Xxx` is the name of the element
   //     e.g. `"Calcium-48"`, `"Tin-124"`, etc.
   //
   // Otherwise, we just use the element symbol and name for naturally-occurring
   // mixtures of atomic elements (`"Ca"`, `"Calcium"`, etc.).

   KVIonRangeTableMaterial* mat;
   if (!a) mat = MakeNaturallyOccuringElementMixture(z, a); // this may set a!=0 if only one isotope exists in nature
   if (a) {
      if (!gNDTManager) {
         Error("AddElementalMaterial",
               "Nuclear data tables have not been initialised");
         return nullptr;
      }
      KVElementDensity* ed = (KVElementDensity*)gNDTManager->GetData(z, a, "ElementDensity");
      if (!ed) {
         Error("AddElementalMaterial",
               "No element found in ElementDensity NDT-table with Z=%d", z);
         return nullptr;
      }
      TString state = "solid";
      if (ed->IsGas()) state = "gas";
      mat = new KVRangeYanezMaterial(this, Form("%s-%d", ed->GetElementName(), a),
                                     Form("%d%s", a, ed->GetElementSymbol()),
                                     state, ed->GetValue(), z, a);
      mat->Initialize();
   }
   CheckMaterialsList();
   fMaterials->Add(mat);
   if (!fDoNotSaveMaterials) SaveMaterial(mat);
   return mat;
}

KVIonRangeTableMaterial* KVRangeYanez::AddCompoundMaterial(
   const Char_t* name, const Char_t* symbol,
   Int_t nelem, Int_t* z, Int_t* a, Int_t* natoms, Double_t density) const
{
   // Adds a compound material with a simple formula composed of different elements
   //
   // \param[in] name name for the new compound (no spaces)
   // \param[in] symbol chemical symbol for compound
   // \param[in] nelem number of elements in compound
   // \param[in] z[nelem] atomic numbers of elements
   // \param[in] a[nelem] mass numbers of elements
   // \param[in] natoms[nelem] number of atoms of each element
   // \param[in] density in \f$g/cm^{3}\f$, if compound is a solid

   TString state = "gas";
   if (density > 0) state = "solid";
   KVRangeYanezMaterial* mat =
      new KVRangeYanezMaterial(this, name, symbol, state, density);
   for (int i = 0; i < nelem; i++) {
      mat->AddCompoundElement(z[i], a[i], natoms[i]);
   }
   mat->Initialize();
   CheckMaterialsList();
   fMaterials->Add(mat);
   if (!fDoNotSaveMaterials) SaveMaterial(mat);
   return mat;
}

KVIonRangeTableMaterial* KVRangeYanez::AddMixedMaterial(
   const Char_t* name, const Char_t* symbol,
   Int_t nelem, Int_t* z, Int_t* a, Int_t* natoms, Double_t* proportion, Double_t density) const
{
   // Adds a material which is a mixture of either elements or compounds:
   //
   // \param[in] name name for the new mixture (no spaces)
   // \param[in] symbol chemical symbol for mixture
   // \param[in] nelem number of elements in mixture
   // \param[in] z[nelem] atomic numbers of elements
   // \param[in] a[nelem] mass numbers of elements
   // \param[in] natoms[nelem] number of atoms of each element
   // \param[in] proportion[nelem] proportion by mass in mixture of element
   // \param[in] density in \f$g/cm^{3}\f$, if mixture is a solid

   TString state = "gas";
   if (density > 0) state = "solid";
   KVRangeYanezMaterial* mat =
      new KVRangeYanezMaterial(this, name, symbol, state, density);
   for (int i = 0; i < nelem; i++) {
      mat->AddMixtureElement(z[i], a[i], natoms[i], proportion[i]);
   }
   mat->Initialize();
   CheckMaterialsList();
   fMaterials->Add(mat);
   if (!fDoNotSaveMaterials) SaveMaterial(mat);
   return mat;
}

KVIonRangeTableMaterial* KVRangeYanez::MakeNaturallyOccuringElementMixture(Int_t z, Int_t& a) const
{
   // Create a material containing the naturally occuring isotopes of the given element,
   // weighted according to their abundance.
   //
   // \param[in] z atomic number of element
   // \param[out] a mass number of unique isotope, if 100% abundance
   //
   // if there is only one naturally occurring isotope of the element we set `a` to this isotope
   // and don't create any material

   if (!gNDTManager) {
      Error("MakeNaturallyOccuringElementMixture",
            "Nuclear data tables have not been initialised");
      return nullptr;
   }
   KVElementDensity* ed = (KVElementDensity*)gNDTManager->GetData(z, z, "ElementDensity");
   if (!ed) {
      Error("AddElementalMaterial",
            "No element found in ElementDensity NDT-table with Z=%d", z);
      return nullptr;
   }

   KVNucleus nuc(z);
   KVNumberList isotopes = nuc.GetKnownARange();
   isotopes.Begin();
   while (!isotopes.End()) {
      nuc.SetA(isotopes.Next());
      if (nuc.GetAbundance() == 100.) {
         a = nuc.GetA();
         return nullptr;
      }
   }

   TString state = "solid";
   if (ed->IsGas()) state = "gas";
   KVRangeYanezMaterial* mat =
      new KVRangeYanezMaterial(this,
                               ed->GetElementName(),
                               ed->GetElementSymbol(),
                               state, ed->GetValue());
   isotopes.Begin();
   while (!isotopes.End()) {
      nuc.SetA(isotopes.Next());
      Double_t abundance = nuc.GetAbundance() / 100.;
      if (abundance > 0.) mat->AddMixtureElement(z, nuc.GetA(), 1, abundance);
   }
   mat->Initialize();
   return (KVIonRangeTableMaterial*)mat;
}
//____________________________________________________________________________

Bool_t KVRangeYanez::ReadMaterials(const Char_t* filename) const
{
   // Read materials from file whose name is given

   TString DataFilePath = filename;

   ifstream filestream;
   if (!SearchAndOpenKVFile(DataFilePath, filestream, "data")) {
      Error("ReadPredefinedMaterials", "Cannot open %s for reading", DataFilePath.Data());
      return kFALSE;
   }
   Info("ReadPredefinedMaterials", "Reading materials in file : %s", filename);

   fDoNotSaveMaterials = kTRUE; //don't write what we just read!!

   Bool_t compound, mixture;
   compound = mixture = kFALSE;

   KVString line;
   while (filestream.good()) {
      line.ReadLine(filestream);
      if (filestream.good()) {
         if (line.BeginsWith("//")) continue;
         if (line.BeginsWith("COMPOUND")) {
            compound = kTRUE;
            mixture = kFALSE;
         }
         else if (line.BeginsWith("MIXTURE")) {
            compound = kFALSE;
            mixture = kTRUE;
         }
         else if (line.BeginsWith("ELEMENT")) {
            compound = mixture = kFALSE;
         }
         if (compound || mixture) {
            // new compound or mixed material
            KVString name, symbol, state;
            Double_t density = -1;
            KVString element[10];
            Int_t natoms[10];
            Int_t z[10], a[10];
            Double_t proportion[10];
            Int_t nelem = 0;
            line.ReadLine(filestream);
            while (filestream.good() && !line.IsWhitespace() && line != "\n") {
               line.Begin("=");
               KVString next = line.Next();
               if (next == "name") name = line.Next();
               else if (next == "symbol") symbol = line.Next();
               else if (next == "state") state = line.Next();
               else if (next == "density") density = line.Next().Atof();
               else if (next == "nelem") {
                  nelem = line.Next().Atoi();
                  for (int i = 0; i < nelem; i++) {
                     line.ReadLine(filestream);
                     line.Begin(" ");
                     element[i] = line.Next();
                     a[i] = KVNucleus::IsMassGiven(element[i]);
                     KVNucleus n(element[i]);
                     z[i] = n.GetZ();
                     if (!a[i]) a[i] = TMath::Nint(n.GetNaturalA());
                     natoms[i] = line.Next().Atoi();
                     if (mixture) proportion[i] = line.Next().Atof();
                  }
               }
               line.ReadLine(filestream, kFALSE); //do not skip 'whitespace'
            }
            if (compound) AddCompoundMaterial(name, symbol, nelem, z, a, natoms, density);
            else if (mixture) AddMixedMaterial(name, symbol, nelem, z, a, natoms, proportion, density);
            compound = mixture = kFALSE;
         }
         else {
            // new isotopically pure material
            KVString name, symbol, state;
            line.ReadLine(filestream);
            while (filestream.good() && !line.IsWhitespace() && line != "\n") {
               line.Begin("=");
               KVString next = line.Next();
               if (next == "name") name = line.Next();
               else if (next == "symbol") symbol = line.Next();
               else if (next == "state") state = line.Next();
               line.ReadLine(filestream, kFALSE); //do not skip 'whitespace'
            }
            KVNucleus nuc(symbol);
            AddElementalMaterial(nuc.GetZ(), nuc.GetA());
         }
      }
   }
   fDoNotSaveMaterials = kFALSE;
   return kTRUE;
}

void KVRangeYanez::SaveMaterial(KVIonRangeTableMaterial* mat) const
{
   // Write definition of material in a file in the directory
   //
   //  $(WORKING_DIR)/RANGE
   //
   // All files in this directory are read when the table is initialised

   // make directory if needed
   if (gSystem->AccessPathName(fLocalMaterialsDirectory)) {
      gSystem->mkdir(fLocalMaterialsDirectory, true);
      gSystem->Chmod(fLocalMaterialsDirectory, 0755);
   }
   TString matfilename(mat->GetName());
   matfilename.ReplaceAll(" ", "_"); // no spaces in filenames
   matfilename += ".dat";
   ofstream matfil;
   if (SearchAndOpenKVFile(matfilename, matfil, fLocalMaterialsDirectory)) {
      dynamic_cast<KVRangeYanezMaterial*>(mat)->SaveMaterial(matfil);
      matfil.close();
   }
}
