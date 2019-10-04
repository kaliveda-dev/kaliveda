//Created by KVClassFactory on Mon Mar 16 11:45:14 2015
//Author: ,,,

#include "KVFAZIAIDSiSi_e789.h"
#include "KVDataSet.h"


ClassImp(KVFAZIAIDSiSi_e789)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDSiSi_e789</h2>
<h4>identification telescope for E789 FAZIA Si-Si idcards</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////


KVFAZIAIDSiSi_e789::KVFAZIAIDSiSi_e789()
{
   fSiSiGrid = 0;
   fSiSiGridQL1 = 0;
   SetType("Si-Si");
   fSi1 = 0;
   fSi2 = 0;
   SetHasMassID(kTRUE);
}

void KVFAZIAIDSiSi_e789::Initialize()
{
   // Initialize telescope for current run.
   // If there is at least 1 grid, we set fCanIdentify = kTRUE

   fSiSiGrid    = nullptr;
   fSiSiGridQL1 = nullptr;

   int ig = 0;
   KVIDZAGrid*  dummy;
   while (++ig) {
      if (!(dummy = (KVIDZAGrid*) GetIDGrid(ig))) break;
      else if (!strcmp(dummy->GetVarY(), "QH1FPGAEnergy")) {
         fSiSiGrid    = dummy;
         fSiSiGrid->Initialize();
      }
      else if (!strcmp(dummy->GetVarY(), "QL1Amplitude"))  {
         fSiSiGridQL1 = dummy;
         fSiSiGridQL1->Initialize();
      }
   }


   fSi1 = (KVFAZIADetector*)GetDetector(1);
   fSi2 = (KVFAZIADetector*)GetDetector(2);

   if (fSiSiGrid || fSiSiGridQL1) SetBit(kReadyForID);
   else ResetBit(kReadyForID);

   if (!gDataSet->HasCalibIdentInfos()) SetBit(kReadyForID);
}


//________________________________________________________________
Bool_t KVFAZIAIDSiSi_e789::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   // Particle identification and code setting using identification grids.
   // perform identification in QH1-Q2 map / QL1-Q2 map

   idr->SetIDType(GetType());
   idr->IDattempted = kTRUE;

   Double_t si1 = (y < 0. ? GetIDMapY() : y);
   Double_t si2 = (x < 0. ? GetIDMapX() : x);

   // first try with high range grid
   if (fSiSiGrid && fSiSiGrid->IsIdentifiable(si2, si1)) {
      fSiSiGrid->Identify(si2, si1, idr);
   }
   else {
      idr->IDOK = kFALSE;
      idr->IDquality = KVIDZAGrid::kICODE8;
   }

   idr->IDcode = GetIDCode();

   return kTRUE;

}

