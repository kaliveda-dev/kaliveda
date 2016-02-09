//Created by KVClassFactory on Mon Mar 16 11:45:14 2015
//Author: ,,,

#include "KVFAZIAIDSiSi.h"
#include "KVIDZAGrid.h"
#include "KVFAZIADetector.h"


ClassImp(KVFAZIAIDSiSi)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDSiSi</h2>
<h4>identification telescope for FAZIA Si-Si idcards</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAIDSiSi::KVFAZIAIDSiSi()
{
   // Default constructor
   fSiSiGrid = 0;
   SetType("Si-Si");
   fSi1 = 0;
   fSi2 = 0;
   SetHasMassID(kTRUE);
}

KVFAZIAIDSiSi::~KVFAZIAIDSiSi()
{
   // Destructor
}

void KVFAZIAIDSiSi::Initialize()
{
   // Initialize telescope for current run.
   // If there is at least 1 grid, we set fCanIdentify = kTRUE
   // "Natural" line widths are calculated for KVIDZAGrids.


   fSiSiGrid = (KVIDZAGrid*) GetIDGrid();
   fSi1 = (KVFAZIADetector*)GetDetector(1);
   fSi2 = (KVFAZIADetector*)GetDetector(2);

   if (fSiSiGrid) {
      SetBit(kReadyForID);
      fSiSiGrid->Initialize();
   } else {
      ResetBit(kReadyForID);
   }
}

//________________________________________________________________
Bool_t KVFAZIAIDSiSi::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   // Particle identification and code setting using identification grids.
   // perform identification in Si(GG) - CsI(H) map
   // Sets idr->deltaEpedestal according to position in GG map

   idr->SetIDType(GetType());
   idr->IDattempted = kTRUE;

   Double_t si1 = (y < 0. ? GetIDMapY() : y);
   Double_t si2 = (x < 0. ? GetIDMapX() : x);

   fSiSiGrid->Identify(si2, si1, idr);

   idr->IDcode = GetIDCode();

   return kTRUE;

}
//____________________________________________________________________________________

Double_t KVFAZIAIDSiSi::GetIDMapX(Option_t*)
{
   //X-coordinate for CsI identification map is raw "L" coder value
   //Info("GetIDMapX","%lf",fSi2->GetQ2Amplitude());
   return fSi2->GetQ2Amplitude();
}

//____________________________________________________________________________________

Double_t KVFAZIAIDSiSi::GetIDMapY(Option_t*)
{
   //Y-coordinate for CsI identification map is raw "R" coder value
   //Info("GetIDMapY","%lf",fSi1->GetQH1Amplitude());
   return fSi1->GetQH1Amplitude();
}
