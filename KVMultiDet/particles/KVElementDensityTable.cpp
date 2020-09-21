//Created by KVClassFactory on Thu Sep 27 17:23:50 2012
//Author: John Frankland,,,

#include "KVElementDensityTable.h"
#include "KVFileReader.h"
#include "TEnv.h"
#include "KVElementDensity.h"
#include "KVUnits.h"
#include "KVBase.h"

ClassImp(KVElementDensityTable)

KVElementDensityTable::KVElementDensityTable(): KVNuclDataTable("KVElementDensity")
{
   // Default constructor
   SetName("ElementDensity");
}

//________________________________________________________________

KVElementDensityTable::~KVElementDensityTable()
{
   // Destructor
}

void KVElementDensityTable::Initialize()
{

   TString dfile;
   dfile.Form("%s.DataFile", GetName());
   TString cl_path;
   if (!KVBase::SearchKVFile(gEnv->GetValue(dfile.Data(), ""), cl_path, "data")) {
      Error("Initialize", "No file found for %s", GetName());
      return;
   }
   else {
//    Info("Initialize","%s will be read",gEnv->GetValue(dfile.Data(),""));
   }
   SetTitle(gEnv->GetValue(dfile.Data(), ""));

   Int_t ntot = 0;
   nucMap = new TMap(50, 2);
   KVFileReader* fr = new KVFileReader();
   fr->OpenFileToRead(cl_path.Data());

   //Premier passage
   //Lecture du nombre de noyaux a enregistrer
   while (fr->IsOK()) {

      fr->ReadLine(" \t");
      if (fr->GetCurrentLine().IsNull()) {
         break;
      }
      else if (fr->GetNparRead() == 0) {
         break;
      }
      else if (fr->GetReadPar(0).BeginsWith("//")) {

         kcomments += fr->GetCurrentLine();
         kcomments += "\n";

      }
      else {
         Int_t zz = fr->GetIntReadPar(4);
         Int_t aa = 2 * zz + 1;
         GiveIndexToNucleus(zz, aa, ntot);
         ntot += 1;
      }
   }

   if (!fr->PreparForReadingAgain()) return;

// Info("Initialize","Set up map for %d nuclei", ntot);
   CreateTable(ntot);

   KVElementDensity* lf = 0;
   ntot = 0;
   while (fr->IsOK()) {
      fr->ReadLine(" \t");
      if (fr->GetCurrentLine().IsNull()) {
         break;
      }
      else if (fr->GetNparRead() == 0) {
         break;
      }
      else if (fr->GetReadPar(0).BeginsWith("//")) { }
      else {
         CreateElement(ntot);
         lf = (KVElementDensity*)GetCurrent();

         KVString unit = fr->GetReadPar(1);
         Double_t units = KVUnits::g / KVUnits::cc;
         if (unit == "g/L") {
            lf->SetIsGas();
            units = KVUnits::g / KVUnits::litre;
         }
         Double_t val = fr->GetDoubleReadPar(0);
         lf->SetValue(val * units);
         lf->SetElementSymbol(fr->GetReadPar(3));
         lf->SetElementName(fr->GetReadPar(2));
         lf->SetZ(fr->GetIntReadPar(4));
         ntot += 1;
      }

   }

// Info("Initialize","table initialised correctly for %d/%d nuclei", ntot,GetNumberOfNuclei());
   fr->CloseFile();
   delete fr;

}

//_____________________________________________
void KVElementDensityTable::GiveIndexToNucleus(Int_t zz, Int_t aa, Int_t ntot)
{

   //Add a new entry in the table
   //Masses are not important, we use aa=2*zz+1
   aa = 2 * zz + 1;
   KVNuclDataTable::GiveIndexToNucleus(zz, aa, ntot);
}

//_____________________________________________
NDT::value* KVElementDensityTable::getNDTvalue(Int_t zz, Int_t aa) const
{
   // Return NDT::value object pointer stored at map position (Z,A).
   //Masses are not important, we use aa=2*zz+1
   aa = 2 * zz + 1;
   return KVNuclDataTable::getNDTvalue(zz, aa);
}

KVElementDensity* KVElementDensityTable::FindElementByName(const Char_t* X) const
{
   // Search table for an element with the given name. Case-insensitive.
   TString x = X;
   x.ToUpper();
   TIter next(tobj);
   KVElementDensity* elem;
   while ((elem = (KVElementDensity*)next())) {
      TString el = elem->GetElementName();
      el.ToUpper();
      if (x == el) return elem;
   }
   return 0x0;
}

KVElementDensity* KVElementDensityTable::FindElementBySymbol(const Char_t* X) const
{
   // Search table for an element with the given symbol. Case-insensitive.
   TString x = X;
   x.ToUpper();
   TIter next(tobj);
   KVElementDensity* elem;
   while ((elem = (KVElementDensity*)next())) {
      TString el = elem->GetElementSymbol();
      el.ToUpper();
      if (x == el) return elem;
   }
   return 0x0;
}
