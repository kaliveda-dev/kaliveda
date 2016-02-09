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
#include <Riostream.h>
using namespace std;

ClassImp(KVRangeYanez)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVRangeYanez</h2>
<h4>Interface to Range dE/dx and range library (Ricardo Yanez)</h4>
See <a href="http://www.calel.org/range.html">here</a> for details.
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVHashList* KVRangeYanez::fMaterials = 0x0;

KVRangeYanez::KVRangeYanez()
   : KVIonRangeTable("RANGE",
                     "Interface to Range dE/dx and range library (Ricardo Yanez)")
{
   // Default constructor
   // Predefined materials are created based on the contents of the file(s) whose
   // names are given as values of the variable KVRangeYanez.PredefMaterials.
   // A default file is specified in the main .kvrootrc file.
   // If you want to add your own definitions, just put in your .kvrootrc:
   //+RANGE.PredefMaterials: myfile1.dat
   //+RANGE.PredefMaterials: myfile2.dat
   // If you want to override the default definitions:
   //RANGE.PredefMaterials: myfile1.dat
   //+RANGE.PredefMaterials: myfile2.dat

   KVString DataFilePaths = gEnv->GetValue("RANGE.PredefMaterials", "");
   DataFilePaths.Begin(" ");
   KVString nextPath;
   KVString lastPath;
   while (!DataFilePaths.End()) {
      nextPath = DataFilePaths.Next();
      if (nextPath == lastPath) break; //check for double occurrence of last file : TEnv bug?
      lastPath = nextPath;
      ReadPredefinedMaterials(nextPath);
   }
}

//________________________________________________________________

KVRangeYanez::KVRangeYanez(const KVRangeYanez& obj)  : KVIonRangeTable()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVRangeYanez::~KVRangeYanez()
{
   // Destructor
}

//________________________________________________________________

void KVRangeYanez::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVRangeYanez::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVIonRangeTable::Copy(obj);
   //KVRangeYanez& CastedObj = (KVRangeYanez&)obj;
}

KVIonRangeTableMaterial* KVRangeYanez::GetMaterialWithNameOrType(const Char_t* material)
{
   // Returns pointer to material of given name or type.
   // Note that any request for an element of the periodic table will cause
   // the corresponding material to be created (if not already done).
   // If given in the form of an isotope ("48Ca", "124Sn", etc.) this material will
   // be isotopically pure. If not ("Ca", "natSn", etc.) the material generated will be
   // a mixture of the most abundant naturally-occurring isotopes.

   CheckMaterialsList();
   KVIonRangeTableMaterial* M = (KVIonRangeTableMaterial*)fMaterials->FindObject(material);
   if (!M) {
      M = (KVIonRangeTableMaterial*)fMaterials->FindObjectByType(material);
   }
   if (!M) {
      // is the requested material an atomic element?
      Int_t A = KVNucleus::IsMassGiven(material);
      if (A > -1) {
         KVNucleus n(material);
         AddElementalMaterial(n.GetZ(), A);
         M = (KVIonRangeTableMaterial*)fMaterials->FindObjectByType(material);
      }
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
   } else
      printf("\nEnergy loss & range tables loaded for 0 materials.\n");
}

TObjArray* KVRangeYanez::GetListOfMaterials()
{
   // Create and fill a list of all materials for which range tables exist.
   // Each entry is a TNamed with the name and type (title) of the material.
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
void KVRangeYanez::CheckMaterialsList()
{
   if (!fMaterials) {
      fMaterials = new KVHashList;
      fMaterials->SetName("RANGE materials list");
      fMaterials->SetOwner();
   }
}
void KVRangeYanez::AddElementalMaterial(Int_t z, Int_t a)
{
   // Adds a material composed of a single isotope of a chemical element.
   // If the isotope (a) is not specified, we create a material containing the naturally
   // occuring isotopes of the given element, weighted according to their abundance.
   // If the mass is given, the material symbol will be "AX" where X is the symbol for the element
   //    e.g. "48Ca",  "124Sn", etc.
   // and the material name will be "Xxx-A" where Xxx is the name of the element
   //    e.g. "Calcium-48", "Tin-124", etc.
   // Otherwise, we just use the element symbol and name for naturally-occurring
   // mixtures of atomic elements ("Ca", "Calcium", etc.).

   KVIonRangeTableMaterial* mat;
   if (!a) {
      mat = MakeNaturallyOccuringElementMixture(z);
   } else {
      if (!gNDTManager) {
         Error("AddElementalMaterial",
               "Nuclear data tables have not been initialised");
         return;
      }
      KVElementDensity* ed = (KVElementDensity*)gNDTManager->GetData(z, a, "ElementDensity");
      if (!ed) {
         Error("AddElementalMaterial",
               "No element found in ElementDensity NDT-table with Z=%d", z);
         return;
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
   mat->ls();
}

void KVRangeYanez::AddCompoundMaterial(
   const Char_t* name, const Char_t* symbol,
   Int_t nelem, Int_t* z, Int_t* a, Int_t* natoms, Double_t density)
{
   // Adds a compound material with a simple formula composed of different elements
   // For solids, give the density (in g/cm**3)

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
   mat->ls();
}

void KVRangeYanez::AddMixedMaterial(
   const Char_t* name, const Char_t* symbol,
   Int_t nelem, Int_t* z, Int_t* a, Int_t* natoms, Double_t* proportion, Double_t density)
{
   // Adds a material which is a mixture of either elements or compounds:
   //   nelem = number of elements in mixture
   //   z[],a[],natoms[],proportion[]: arrays with atomic number, mass, number of atoms
   //            and proportion of each element
   //   if mixture is a solid, give density in g/cm**3
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
   mat->ls();
}

KVIonRangeTableMaterial* KVRangeYanez::MakeNaturallyOccuringElementMixture(Int_t z)
{
   // create a material containing the naturally occuring isotopes of the given element,
   // weighted according to their abundance.

   if (!gNDTManager) {
      Error("MakeNaturallyOccuringElementMixture",
            "Nuclear data tables have not been initialised");
      return NULL;
   }
   KVElementDensity* ed = (KVElementDensity*)gNDTManager->GetData(z, z, "ElementDensity");
   if (!ed) {
      Error("AddElementalMaterial",
            "No element found in ElementDensity NDT-table with Z=%d", z);
      return 0x0;
   }
   TString state = "solid";
   if (ed->IsGas()) state = "gas";
   KVRangeYanezMaterial* mat =
      new KVRangeYanezMaterial(this,
                               ed->GetElementName(),
                               ed->GetElementSymbol(),
                               state, ed->GetValue());
   KVNucleus nuc(z);
   KVNumberList isotopes = nuc.GetKnownARange();
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

void KVRangeYanez::ReadPredefinedMaterials(const Char_t* filename)
{
   // Read materials from file whose name is given

   TString DataFilePath = filename;

   ifstream filestream;
   if (!SearchAndOpenKVFile(DataFilePath, filestream, "data")) {
      Error("ReadPredefinedMaterials", "Cannot open %s for reading", DataFilePath.Data());
      return;
   }
   Info("ReadPredefinedMaterials", "Reading materials in file : %s", filename);

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
         } else if (line.BeginsWith("MIXTURE")) {
            compound = kFALSE;
            mixture = kTRUE;
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
      }
   }
}
