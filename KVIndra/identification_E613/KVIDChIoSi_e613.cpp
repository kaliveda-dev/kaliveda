//Created by KVClassFactory on Wed Mar 12 14:49:28 2014
//Author: bonnet

#include "KVIDChIoSi_e613.h"
#include "KVINDRACodes.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDChIoSi_e613)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoSi_e613</h2>
<h4>modification of the sequence of identification for e613</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoSi_e613::KVIDChIoSi_e613()
{
   // Default constructor
}

KVIDChIoSi_e613::~KVIDChIoSi_e613()
{
   // Destructor
}
//________________________________________________________________________________________//

Double_t KVIDChIoSi_e613::GetIDMapX(Option_t*)
{
   //Default X coordinate for ChIo-Si identification is KVSilicon::GetEnergy
   //if silicon is not calibrated, return 0
   return fsi->GetEnergy();

}

Double_t KVIDChIoSi_e613::GetIDMapY(Option_t*)
{
   //Default Y coordinate for ChIo-Si identification is KVChIo::GetEnergy
   //if chio is not calibrated, return 0
   return fchio->GetEnergy();
}

//____________________________________________________________________________________

void KVIDChIoSi_e613::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run.

   fchio = GetDetector(1);
   fsi = GetDetector(2);

   ChIoSiGrid = FromFitChIoSiGrid = 0;
   TIter next(GetListOfIDGrids());
   KVIDGraph* grid;
   while ((grid = (KVIDGraph*)next())) {
      if (!strcmp(grid->GetVarY(), "CI-E")) ChIoSiGrid = (KVIDGChIoSi*)grid;
      else if (!strcmp(grid->GetVarY(), "CI-E-Fit")) FromFitChIoSiGrid = (KVIDGChIoSi*)grid;
   }
   if (ChIoSiGrid) {
      SetBit(kReadyForID);
      ChIoSiGrid->Initialize();
      if (FromFitChIoSiGrid) FromFitChIoSiGrid->Initialize();
   }
   else {
      ResetBit(kReadyForID);
   }

}

//____________________________________________________________________________________

Bool_t KVIDChIoSi_e613::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid KVIDGChIoSi

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;
   //identification
   Double_t chio = (y < 0. ? GetIDMapY() : y);
   Double_t si = (x < 0. ? GetIDMapX() : x);

   KVIDZAGrid* theIdentifyingGrid = 0;
   theIdentifyingGrid = ChIoSiGrid;
   if (ChIoSiGrid->IsIdentifiable(si, chio)) {
      ChIoSiGrid->Identify(si, chio, IDR);
      if (IDR->IDOK) {  // < quality code 4
         /*
         Info("Identify","Cas0 identification OK avec grille std, IDRcode=%d IDRz=%d IDRq=%d Grilleq=%d",
            IDR->IDcode,
            IDR->Z,
            IDR->IDquality,
            theIdentifyingGrid->GetQualityCode()
         );
         */
         //theIdentifyingGrid = ChIoSiGrid;
      }
      else {
         if (FromFitChIoSiGrid) {
            FromFitChIoSiGrid->Identify(si, chio, IDR);
            if (IDR->IDOK) {
               theIdentifyingGrid = FromFitChIoSiGrid;
               /*
               Info("Identify","Cas1 identification OK avec grille fitte, IDRcode=%d IDRz=%d IDRq=%d Grilleq=%d",
                  IDR->IDcode,
                  IDR->Z,
                  IDR->IDquality,
                  theIdentifyingGrid->GetQualityCode()
               );
               */
            }
         }
      }
   }
   else if (FromFitChIoSiGrid->IsIdentifiable(si, chio)) {
      FromFitChIoSiGrid->Identify(si, chio, IDR);
      theIdentifyingGrid = FromFitChIoSiGrid;
      Info("Identify", "Cas2 identification avec grille fitte, IDRcode=%d IDRz=%d IDRq=%d Grilleq=%d",
           IDR->IDcode,
           IDR->Z,
           IDR->IDquality,
           theIdentifyingGrid->GetQualityCode()
          );
   }

   Int_t quality = theIdentifyingGrid->GetQualityCode();
   IDR->IDquality = quality;

   // set general ID code
   IDR->IDcode = kIDCode4;
   //if point lies above Zmax line, we give Zmax as Z of particle (real Z is >= Zmax)
   //general ID code = kIDCode5 (Zmin)
   if (quality == KVIDZAGrid::kICODE7) {
      IDR->IDcode = kIDCode5;
   }
   //Identified particles with subcode kID_LeftOfBragg are given
   //general ID code kIDCode5 (Zmin).
   if (quality == KVIDGChIoSi::k_LeftOfBragg) {
      IDR->IDcode = kIDCode5;
   }
   //unidentifiable particles with subcode kID_BelowSeuilSi are given
   //general ID code kIDCode5 (Zmin) and we estimate Zmin from energy
   //loss in ChIo
   if (quality == KVIDGChIoSi::k_BelowSeuilSi) {
      IDR->IDcode = kIDCode5;
      IDR->Z = fchio->FindZmin();
      IDR->SetComment("point to identify left of Si threshold line (bruit/arret ChIo?)");
   }
   if (quality == KVIDGChIoSi::k_RightOfEmaxSi) IDR->SetComment("point to identify has E_Si > Emax_Si i.e. codeur is saturated. Unidentifiable");


   //Info("Identify","au final ... z=%d IDcode=%d Quality=%d\nComment=%s",IDR->Z,IDR->IDcode,IDR->IDquality,IDR->GetComment());
   return kTRUE;

}
