//Created by KVClassFactory on Wed Jun 10 16:45:12 2009
//Author: John Frankland,,,

#include "KVIDSi150CsI_camp5.h"
#include "KVINDRACodeMask.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDSi150CsI_camp5)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDSi150CsI_camp5</h2>
<h4>Si(150)-CsI identifications for 5th campaign INDRA data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDSi150CsI_camp5::KVIDSi150CsI_camp5()
{
   // Default constructor
   fZGrid = fZAGrid = 0;
   fSi = 0;
   fCsI = 0;
}

KVIDSi150CsI_camp5::~KVIDSi150CsI_camp5()
{
   // Destructor
}

//___________________________________________________________________________________________

void KVIDSi150CsI_camp5::Initialize()
{
   // Initialize telescope for current run.
   // If there is at least 1 grid, we set IsReadyForID = kTRUE
   // "Natural" line widths are calculated for grids.

   fSi = (KVSilicon*)GetDetector(1);
   fSIPG = fSi->GetACQParam("PG");
   fCsI = (KVCsI*)GetDetector(2);
   fZGrid = fZAGrid = 0;
   TIter next(GetListOfIDGrids());
   KVIDGrid* grid = 0;
   while ((grid = (KVIDGrid*)next())) {
      if (!grid->HasMassIDCapability()) fZGrid = (KVIDZAGrid*)grid;
      else fZAGrid = (KVIDZAGrid*)grid;
   }
   if (fZAGrid) {
      SetBit(kReadyForID);
      fZAGrid->Initialize();
      if (fZGrid) fZGrid->Initialize();
   }
   else
      ResetBit(kReadyForID);
}

//___________________________________________________________________________________________

Double_t KVIDSi150CsI_camp5::GetIDMapX(Option_t*)
{
   //X-coordinate for Si150-CsI identification map is corrected light output of CsI
   return fCsI->GetCorrectedLumiereTotale();
}

//____________________________________________________________________________________

Double_t KVIDSi150CsI_camp5::GetIDMapY(Option_t*)
{
   //Y-coordinate for Si150-CsI identification map is raw "PG" coder value of Silicon
   return (Double_t)fSIPG->GetData();
}

//____________________________________________________________________________________

Double_t KVIDSi150CsI_camp5::GetPedestalY(Option_t*)
{
   //Y-coordinate pedestal for Si150-CsI identification map is "PG" pedestal for Silicon
   return (Double_t)fSIPG->GetPedestal();
}

//________________________________________________________________________________________//

Bool_t KVIDSi150CsI_camp5::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grids

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;
   KVIDGrid* theIdentifyingGrid = 0;

   // try full isotopic identification
   Double_t sili = (y < 0. ? GetIDMapY() : y);
   Double_t csi = (x < 0. ? GetIDMapX() : x);
   fZAGrid->Identify(csi, sili, IDR);
   theIdentifyingGrid = fZAGrid;

   if (fZAGrid->GetQualityCode() > KVIDZAGrid::kICODE6 && fZGrid) {

      // particle is above Z&A grid: try Z only ID
      fZGrid->Identify(csi, sili, IDR);
      theIdentifyingGrid = fZGrid;
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

   // set general ID code Si150-CsI
   IDR->IDcode = kIDCode3;
   return kTRUE;
}


