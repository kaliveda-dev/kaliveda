//Created by KVClassFactory on Mon Jun 22 16:47:20 2009
//Author: John Frankland,,,

#include "KVIDChIoSi75_camp5.h"
#include "KVINDRACodeMask.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDChIoSi75_camp5)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoSi75_camp5</h2>
<h4>ChIo-Si(75) identification for INDRA 5th campaign data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoSi75_camp5::KVIDChIoSi75_camp5()
{
   // Default constructor
   fGGgrid = 0;
   fPGgrid = 0;
}

KVIDChIoSi75_camp5::~KVIDChIoSi75_camp5()
{
   // Destructor
}

//___________________________________________________________________________________________

void KVIDChIoSi75_camp5::Initialize()
{
   // Initialize telescope for current run.
   // If there is at least 1 grid, we set fCanIdentify = kTRUE
   // "Natural" line widths are calculated for KVIDZAGrids.

   fchio = GetDetector(1);
   fsi = GetDetector(2);
   fGGgrid = fPGgrid = 0;
   TIter next(GetListOfIDGrids());
   KVIDGrid* grid = 0;
   while ((grid = (KVIDGrid*)next())) {
      if (!strcmp(grid->GetVarY(), "CIGG")) fGGgrid = (KVIDZAGrid*)grid;
      else if (!strcmp(grid->GetVarY(), "CIPG")) fPGgrid = (KVIDZAGrid*)grid;
   }
   if (fGGgrid || fPGgrid) {
      SetBit(kReadyForID);
      if (fGGgrid) fGGgrid->Initialize();
      if (fPGgrid) fPGgrid->Initialize();
   }
   else
      ResetBit(kReadyForID);
}


Double_t KVIDChIoSi75_camp5::GetIDMapX(Option_t* opt)
{
   //Calculates current X coordinate for identification.

   return fsi->GetACQData(opt);
}

//__________________________________________________________________________//

Double_t KVIDChIoSi75_camp5::GetIDMapY(Option_t* opt)
{
   //Calculates current Y coordinate for identification.
   //It is the ionisation chamber's current grand gain (if opt="GG") or petit gain (opt != "GG")
   //coder data, without pedestal correction.

   return fchio->GetACQData(opt);
}

//________________________________________________________________________________________//

Bool_t KVIDChIoSi75_camp5::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grids.

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;
   KVIDGrid* theIdentifyingGrid = 0;

   if (fGGgrid) {
      Double_t cigg = (y < 0. ?       GetIDMapY("GG")   : y);
      Double_t sigg = (x < 0. ?       GetIDMapX("GG")   : x);
      fGGgrid->Identify(sigg, cigg, IDR);
      theIdentifyingGrid = (KVIDGrid*)fGGgrid;

   }
   if ((fGGgrid && fGGgrid->GetQualityCode() > KVIDZAGrid::kICODE6) || !fGGgrid) { //we have to try PG grid (if there is one)

      if (fPGgrid) {
         Double_t cipg = (y < 0. ?       GetIDMapY("PG")   : y);
         Double_t sipg = (x < 0. ?       GetIDMapX("PG")   : x);
         fPGgrid->Identify(sipg, cipg, IDR);
         theIdentifyingGrid = (KVIDGrid*)fPGgrid;
      }
   }

   if (theIdentifyingGrid->GetQualityCode() == KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE7 (above last line in grid) then the estimated
      // Z is only a minimum value (Zmin)
      IDR->IDcode = kIDCode5;
      return kTRUE;
   }

   if (theIdentifyingGrid->GetQualityCode() > KVIDZAGrid::kICODE3 &&
         theIdentifyingGrid->GetQualityCode() < KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE4, kICODE5 or kICODE6 then this "nucleus"
      // corresponds to a point which is inbetween the lines, i.e. noise
      IDR->IDcode = kIDCode10;
      return kTRUE;
   }

   // set general ID code ChIo-Si
   IDR->IDcode = kIDCode4;
   return kTRUE;
}

