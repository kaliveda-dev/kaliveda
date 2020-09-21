//Created by KVClassFactory on Wed Feb 23 16:10:14 2011
//Author: bonnet

#include "KVSpinParityTable.h"
#include "KVFileReader.h"
#include "TEnv.h"
#include "KVBase.h"

ClassImp(KVSpinParityTable)


KVSpinParityTable::KVSpinParityTable(): KVNuclDataTable("KVSpinParity")
{
   // Default constructor
   init();
}

//_____________________________________________
KVSpinParityTable::~KVSpinParityTable()
{
   // Destructor
   init();
}

//_____________________________________________
void KVSpinParityTable::init()
{
   SetName("SpinParity");
}

//_____________________________________________
void KVSpinParityTable::Initialize()
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
   //Lecture du nombre de noyaux a enregistrer

   while (fr->IsOK()) {

      fr->ReadLine(" ");
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
         Int_t zz = fr->GetIntReadPar(0);
         Int_t nn = fr->GetIntReadPar(1);
         GiveIndexToNucleus(zz, nn + zz, ntot);
         ntot += 1;
      }
   }

   if (!fr->PreparForReadingAgain()) return;
   CreateTable(ntot);

   KVSpinParity* lf = 0;
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

         CreateElement(ntot);
         lf = (KVSpinParity*)GetCurrent();
         Double_t spin = -555;
         Double_t parity = 0;
         KVString sp = fr->GetReadPar(2);
         if (sp == "NA") {
            lf->SetMeasured(kFALSE);
            lf->SetMultiple(kFALSE);
         }
         else if (sp.BeginsWith("(") || sp.Contains(",")) {

            if (sp.Contains("(")) {
               sp.ReplaceAll("(", "");
               sp.ReplaceAll(")", "");
               lf->SetMeasured(kFALSE);
            }

            KVString stmp = sp;
            if (sp.Contains(",")) {
               lf->SetMultiple(kTRUE);
               sp.Begin(",");
               stmp = sp.Next();
            }
            else {
               lf->SetMultiple(kFALSE);
            }
            //traitement de la parite
            if (stmp.Contains("-")) {
               parity = -1;
               stmp.ReplaceAll("-", "");
            }
            else  if (stmp.Contains("+")) {
               parity = 1;
               stmp.ReplaceAll("+", "");
            }

            //traitement du spin
            if (stmp.Contains("/")) {
               //fraction
               stmp.Begin("/");
               Int_t num = stmp.Next().Atoi();
               Int_t den = stmp.Next().Atoi();
               spin = Double_t(num) / den;
            }
            else {
               //valeur entiere
               spin = stmp.Atof();
            }
         }
         else {
            lf->SetMultiple(kFALSE);
            lf->SetMeasured(kTRUE);

            //traitement de la parite
            if (sp.Contains("-")) {
               sp.ReplaceAll("-", "");
               parity = -1;
            }
            if (sp.Contains("+")) {
               sp.ReplaceAll("+", "");
               parity = 1;
            }

            //traitement du spin
            if (sp.Contains("/")) {
               sp.Begin("/");
               Int_t num = sp.Next().Atoi();
               Int_t den = sp.Next().Atoi();
               spin = Double_t(num) / den;
            }
            else {
               spin = sp.Atof();
            }
         }

         if (parity != 0)
            spin *= parity;
         lf->SetValue(spin);

         ntot += 1;
      }

   }

   fr->CloseFile();
   delete fr;

}

//_____________________________________________
KVSpinParity* KVSpinParityTable::GetSpinParity(Int_t zz, Int_t aa) const
{

   return (KVSpinParity*)GetData(zz, aa);

}

//_____________________________________________
Bool_t KVSpinParityTable::IsMultiple(Int_t zz, Int_t aa) const
{
   // Returns bit of the registered KVNuclData object associated to the couple (Z,A).
   // indicating if the nucleus as multiple possible values or a certain one
   // returns kFALSE if no such object is present

   KVSpinParity* nd = 0;
   if ((nd = GetSpinParity(zz, aa)))
      return nd->IsMultiple();
   else
      return kFALSE;


}
