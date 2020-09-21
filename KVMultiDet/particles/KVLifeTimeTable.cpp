//Created by KVClassFactory on Wed Feb  9 13:04:29 2011
//Author: bonnet

#include "KVLifeTimeTable.h"
#include "KVFileReader.h"
#include "TEnv.h"
#include "KVBase.h"

ClassImp(KVLifeTimeTable)

KVLifeTimeTable::KVLifeTimeTable() : KVNuclDataTable("KVLifeTime")
{
   // Default constructor
   init();
}

//_____________________________________________
KVLifeTimeTable::~KVLifeTimeTable()
{
   // Destructor

}

//_____________________________________________

// suppress spurious warnings about out-of-bounds indices in
// the 2 lines
//      conversion_t[ii][jj]=conversion_t[ii][jj-1]*conversion_t[jj-1][jj];
#pragma GCC diagnostic ignored "-Warray-bounds"

void KVLifeTimeTable::init()
{

   //Initialisation de la table de conversion en unite de temps
   list_t = "y d h m s ms micros ns ps fs";
   list_vt = "365 24 60 60 1.e3 1.e3 1.e3 1.e3 1.e3";

   kNu_t = 0;
   list_t.Begin(" ");
   while (!list_t.End()) {
      lu_t.SetValue(list_t.Next().Data(), kNu_t++);
   }

   if (kNu_t > 15)
      Warning("init", "Pb de dimensionnement de tableau conversion_t");

   for (Int_t jj = 0; jj < kNu_t; jj += 1)
      for (Int_t ii = 0; ii < kNu_t; ii += 1)
         conversion_t[ii][jj] = 1.;

   Int_t nn = 0;
   list_vt.Begin(" ");
   while (!list_vt.End()) {
      Double_t coef = list_vt.Next().Atof();
      //cout << nn << " " << nn+1 << " " << coef << endl;
      conversion_t[nn][nn + 1] = coef;
      nn += 1;
   }
   for (Int_t ii = 0; ii < kNu_t; ii += 1) {
      for (Int_t jj = ii + 2; jj < kNu_t; jj += 1) {
         conversion_t[ii][jj] = conversion_t[ii][jj - 1] * conversion_t[jj - 1][jj];
      }
   }
   for (Int_t ii = 0; ii < kNu_t; ii += 1) {
      for (Int_t jj = ii; jj < kNu_t; jj += 1) {
         conversion_t[jj][ii] = 1. / conversion_t[ii][jj];
      }
   }

   //Initialisation de la table de conversion en unite d'energie

   list_e = "MeV keV eV";
   list_ve = "1.e3 1.e3";

   kNu_e = 0;
   list_e.Begin(" ");
   while (!list_e.End()) {
      lu_e.SetValue(list_e.Next().Data(), kNu_e++);
   }

   if (kNu_e > 15)
      Warning("init", "Pb de dimensionnement de tableau conversion_e");

   for (Int_t jj = 0; jj < kNu_e; jj += 1)
      for (Int_t ii = 0; ii < kNu_e; ii += 1)
         conversion_e[ii][jj] = 1.;

   nn = 0;
   list_ve.Begin(" ");
   while (!list_ve.End()) {
      Double_t coef = list_ve.Next().Atof();
      //cout << nn << " " << nn+1 << " " << coef << endl;
      conversion_e[nn][nn + 1] = coef;
      nn += 1;
   }

   for (Int_t ii = 0; ii < kNu_e; ii += 1) {
      for (Int_t jj = ii + 2; jj < kNu_e; jj += 1) {
         conversion_e[ii][jj] = conversion_e[ii][jj - 1] * conversion_e[jj - 1][jj];
      }
   }
   for (Int_t ii = 0; ii < kNu_e; ii += 1) {
      for (Int_t jj = ii; jj < kNu_e; jj += 1) {
         conversion_e[jj][ii] = 1. / conversion_e[ii][jj];
      }
   }

   SetName("LifeTime");

}


//_____________________________________________
void KVLifeTimeTable::Initialize()
{

   TString dfile;
   dfile.Form("%s.DataFile", GetName());
   TString cl_path;
   if (!KVBase::SearchKVFile(gEnv->GetValue(dfile.Data(), ""), cl_path, "data")) {
      Error("Initialize", "No file found for %s", GetName());
      return;
   }
   else {
      //Info("Initialize","%s will be read",gEnv->GetValue(dfile.Data(),""));
   }
   SetTitle(gEnv->GetValue(dfile.Data(), ""));

   Int_t ntot = 0;
   nucMap = new TMap(50, 2);
   KVFileReader* fr = new KVFileReader();
   fr->OpenFileToRead(cl_path.Data());

   //Premier passage
   while (fr->IsOK()) {

      fr->ReadLine(" ");
      if (fr->GetNparRead() == 0) break;
      else if (fr->GetReadPar(0).BeginsWith("//")) {

         kcomments += fr->GetCurrentLine();
         kcomments += "\n";

      }
      else {
         Int_t zz = fr->GetIntReadPar(0);
         Int_t aa = fr->GetIntReadPar(1);
         GiveIndexToNucleus(zz, aa, ntot);
         ntot += 1;
      }
   }

   if (!fr->PreparForReadingAgain()) return;

   //Info("Initialize","Set up map for %d nuclei", ntot);
   CreateTable(ntot);

   KVLifeTime* lf = 0;
   ntot = 0;
   while (fr->IsOK()) {

      fr->ReadLine(" ");
      if (fr->GetCurrentLine().IsNull()) {
         break;
      }
      else if (fr->GetNparRead() == 0) {
         break;
      }
      else if (fr->GetReadPar(0).BeginsWith("//")) { }
      else {
         Double_t val = fr->GetDoubleReadPar(2);

         CreateElement(ntot);
         lf = (KVLifeTime*)GetCurrent();

         if (fr->GetNparRead() > 3) {
            TString unit = fr->GetReadPar(3);
            if (unit.EndsWith("eV")) {
               lf->SetResonance(kTRUE);
               val *= ToMeV(unit);
            }
            else {
               lf->SetResonance(kFALSE);
               val *= ToSecond(unit);
            }
            if (fr->GetNparRead() > 4) {
               Int_t meas = fr->GetReadPar(4).Atoi();
               lf->SetMeasured(meas);
            }
            else {
               if (ntot == 0) InfoOnMeasured();
            }
         }
         lf->SetValue(val);

         ntot += 1;
      }

   }

   //Info("Initialize","table initialised correctly for %d nuclei", ntot);
   fr->CloseFile();
   delete fr;

}

//_____________________________________________
Double_t KVLifeTimeTable::ToSecond(TString unit)
{

   //Allow conversion from a given time unit to second
   Int_t from = lu_t.GetIntValue(unit.Data());
   Int_t to = lu_t.GetIntValue("s");
   Double_t coef = conversion_t[from][to];
   return coef;
}

//_____________________________________________
Double_t KVLifeTimeTable::ToMeV(TString unit)
{

   //Allow conversion from a given energy unit to MeV
   Int_t from = lu_e.GetIntValue(unit.Data());
   Int_t to = lu_e.GetIntValue("MeV");
   Double_t coef = conversion_e[from][to];
   return coef;
}

//_____________________________________________
KVLifeTime* KVLifeTimeTable::GetLifeTime(Int_t zz, Int_t aa) const
{

   return (KVLifeTime*)GetData(zz, aa);

}

//_____________________________________________
Bool_t KVLifeTimeTable::IsAResonance(Int_t zz, Int_t aa) const
{
   // Returns bit of the registered KVNuclData object associated to the couple (Z,A).
   // indicating if the nucleus as lifetime (in second) or a resonnance width (in MeV)
   // returns kFALSE if no such object is present

   KVLifeTime* nd = 0;
   if ((nd = GetLifeTime(zz, aa)))
      return nd->IsAResonance();
   else
      return kFALSE;


}
