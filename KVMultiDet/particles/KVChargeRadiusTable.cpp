//Created by KVClassFactory on Wed Jan 30 12:13:08 2013
//Author: bonnet

#include "KVChargeRadiusTable.h"
#include "TEnv.h"
#include "KVFileReader.h"
#include "KVBase.h"

ClassImp(KVChargeRadiusTable)

KVChargeRadiusTable::KVChargeRadiusTable() : KVNuclDataTable("KVChargeRadius")
{
   // Default constructor
   init();

}
//_____________________________________________
void KVChargeRadiusTable::init()
{

   SetName("ChargeRadius");

}

//________________________________________________________________
KVChargeRadiusTable::~KVChargeRadiusTable()
{
   // Destructor
}

//_____________________________________________
void KVChargeRadiusTable::Initialize()
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
         Int_t aa = fr->GetIntReadPar(0);
         Int_t zz = fr->GetIntReadPar(1);
         GiveIndexToNucleus(zz, aa, ntot);
         ntot += 1;
      }
   }

   if (!fr->PreparForReadingAgain()) return;

   //Info("Initialize","Set up map for %d nuclei", ntot);
   CreateTable(ntot);

   KVChargeRadius* cr = 0;
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
         Double_t eval = fr->GetDoubleReadPar(3);

         CreateElement(ntot);
         cr = (KVChargeRadius*)GetCurrent();

         cr->SetValue(val);
         cr->SetError(eval);

         ntot += 1;
      }

   }

   //Info("Initialize","table initialised correctly for %d nuclei", ntot);
   fr->CloseFile();
   delete fr;

}

//_____________________________________________
KVChargeRadius* KVChargeRadiusTable::GetChargeRadius(Int_t zz, Int_t aa) const
{

   return (KVChargeRadius*)GetData(zz, aa);

}
