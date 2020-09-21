//Created by KVClassFactory on Wed Feb 23 16:10:14 2011
//Author: bonnet

#include "KVAbundanceTable.h"
#include "KVFileReader.h"
#include "TEnv.h"
#include "KVBase.h"

ClassImp(KVAbundanceTable)

KVAbundanceTable::KVAbundanceTable(): KVNuclDataTable("KVAbundance")
{
   // Default constructor
   init();
}

//_____________________________________________
KVAbundanceTable::~KVAbundanceTable()
{
   // Destructor
   init();
}

//_____________________________________________
void KVAbundanceTable::init()
{
   SetName("Abundance");
}

//_____________________________________________
void KVAbundanceTable::Initialize()
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
   /*
   Int_t zmax=-1;
   Int_t amax=-1;
   */
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
         Int_t aa = fr->GetIntReadPar(1);
         /*
         if (zz>zmax) zmax=zz;
         if (aa>amax) amax=aa;
         */
         GiveIndexToNucleus(zz, aa, ntot);
         ntot += 1;
      }
   }

   //CreateTableRange(zmax+1);
   /*
   tobj_rangeA = new TObjArray(zmax+1);
   tobj_rangeZ = new TObjArray(amax+1);
   KVIntegerList* il=0;
   */
   if (!fr->PreparForReadingAgain()) return;

   // Info("Initialize","Set up map for %d nuclei", ntot);
   CreateTable(ntot);

   KVNuclData* lf = 0;
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
         lf = GetCurrent();

         /*
         Int_t zz = fr->GetIntReadPar(0);
         Int_t aa = fr->GetIntReadPar(1);


         if ( !(il = (KVIntegerList* )tobj_rangeA->At(zz)) ) {
            il = new KVIntegerList();
            tobj_rangeA->AddAt(il,zz);
         }
         il->Add(aa);

         if ( !(il = (KVIntegerList* )tobj_rangeZ->At(aa)) ) {
            il = new KVIntegerList();
            tobj_rangeZ->AddAt(il,aa);
         }
         il->Add(zz);
         */
         Double_t val = fr->GetDoubleReadPar(2);
         lf->SetValue(val);
         //lf->SetMeasured(kTRUE);

         ntot += 1;
      }

   }

// Info("Initialize","table initialised correctly for %d/%d nuclei", ntot,GetNumberOfNuclei());
   fr->CloseFile();
   delete fr;

   /*
   for (Int_t zz=0;zz<=zmax;zz+=1){
      if ( il = (KVIntegerList* )tobj_rangeA->At(zz) ){
         il->CheckForUpdate();
         //printf("%d %s\n",zz,tobj_rangeA->At(zz)->GetName());

      }
   }

   for (Int_t aa=0;aa<=amax;aa+=1){
      if ( il = (KVIntegerList* )tobj_rangeZ->At(aa) ){
         il->CheckForUpdate();
         //printf("%d %s\n",aa,tobj_rangeZ->At(aa)->GetName());

      }
   }
   */

}

//_____________________________________________
KVAbundance* KVAbundanceTable::GetAbundance(Int_t zz, Int_t aa) const
{

   return (KVAbundance*)GetData(zz, aa);

}
