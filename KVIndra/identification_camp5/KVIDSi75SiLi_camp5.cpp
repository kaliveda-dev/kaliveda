/*
$Id: KVIDSi75SiLi_camp5.cpp,v 1.7 2009/04/06 15:22:02 franklan Exp $
$Revision: 1.7 $
$Date: 2009/04/06 15:22:02 $
*/

//Created by KVClassFactory on Mon Oct 29 16:45:49 2007
//Author: franklan

#include "KVIDSi75SiLi_camp5.h"
#include "KVINDRACodeMask.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDSi75SiLi_camp5)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDSi75SiLi_camp5</h2>
<h4>Identification in Si75-SiLi telescopes for INDRA_camp5 dataset</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDSi75SiLi_camp5::KVIDSi75SiLi_camp5()
{
   //Default constructor
   fGGgrid = 0;
   fPGgrid = 0;
   fPGZgrid = 0;
   fSiLi = fSi75 = 0;
}

KVIDSi75SiLi_camp5::~KVIDSi75SiLi_camp5()
{
   //Destructor
}

//___________________________________________________________________________________________

void KVIDSi75SiLi_camp5::Initialize()
{
   // Initialize telescope for current run.
   // If there is at least 1 (GG) grid, we set IsReadyForID = kTRUE
   // "Natural" line widths are calculated for grids.

   fSiLi = GetDetector(2);
   fSi75 = GetDetector(1);
   fGGgrid = fPGgrid = fPGZgrid = 0;
   TIter next(GetListOfIDGrids());
   KVIDGrid* grid = 0;
   while ((grid = (KVIDGrid*)next())) {
      if (!strcmp(grid->GetVarY(), "SI75_GG")) fGGgrid = (KVIDZAGrid*)grid;
      else if (!strcmp(grid->GetVarY(), "SI75_PG")) {
         if (!grid->HasMassIDCapability()) fPGZgrid = (KVIDZAGrid*)grid;
         else fPGgrid = (KVIDZAGrid*)grid;
      }
   }
   if (fGGgrid) {
      SetBit(kReadyForID);
      fGGgrid->Initialize();
      if (fPGgrid) fPGgrid->Initialize();
      if (fPGZgrid) fPGZgrid->Initialize();
   }
   else
      ResetBit(kReadyForID);
}

//___________________________________________________________________________________________

Double_t KVIDSi75SiLi_camp5::GetIDMapX(Option_t*)
{
   //X-coordinate for Si75-SiLi identification map is raw "PG" coder value for SiLi detector
   return (Double_t)fSiLi->GetACQData("PG");
}

//____________________________________________________________________________________

Double_t KVIDSi75SiLi_camp5::GetIDMapY(Option_t* opt)
{
   //Y-coordinate for Si75-SiLi identification map is raw "GG" or "PG" coder value of Si75,
   //depending on 'opt' string.
   return (Double_t)fSi75->GetACQData(opt);
}

//________________________________________________________________________________________//

Bool_t KVIDSi75SiLi_camp5::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grids.

   //perform identification in Si75(GG) - SiLi(PG) map
   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;

   Double_t si75 = (y < 0. ? GetIDMapY("GG") : y);
   Double_t sili = (x < 0. ? GetIDMapX() : x);

   KVIDGrid* theIdentifyingGrid = 0;

   fGGgrid->Identify(sili, si75, IDR);
   theIdentifyingGrid = (KVIDGrid*)fGGgrid;

   if (fGGgrid->GetQualityCode() > KVIDZAGrid::kICODE6 && fPGgrid) { //we have to try PG grid (if there is one)

      // try Z & A identification in Si75(PG)-SiLi(PG) map
      si75 = (y < 0. ? GetIDMapY("PG") : y);
      fPGgrid->Identify(sili, si75, IDR);
      theIdentifyingGrid = (KVIDGrid*)fPGgrid;

      if (fPGgrid->GetQualityCode() > KVIDZAGrid::kICODE6 && fPGZgrid) { //we have to try PGZ grid (if there is one)

         fPGZgrid->Identify(sili, si75, IDR);
         theIdentifyingGrid = (KVIDGrid*)fPGZgrid;
      }
   }

   if (theIdentifyingGrid->GetQualityCode() == KVIDZAGrid::kICODE8) {
      // only if the final quality code is kICODE8 do we consider that it is
      // worthwhile looking elsewhere. In all other cases, the particle has been
      // "identified", even if we still don't know its Z and/or A (in this case
      // we consider that we have established that they are unknowable).
      return kFALSE;
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

   // set general ID code Si75-SiLi
   IDR->IDcode = kIDCode3;
   return kTRUE;
}


