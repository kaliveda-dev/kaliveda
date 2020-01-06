/*
$Id: KVIDChIoCsI_camp5.cpp,v 1.2 2009/04/06 15:21:31 franklan Exp $
$Revision: 1.2 $
$Date: 2009/04/06 15:21:31 $
*/

//Created by KVClassFactory on Mon Mar 30 16:44:34 2009
//Author: John Frankland,,,

#include "KVIDChIoCsI_camp5.h"
#include "KVINDRACodeMask.h"
#include "KVIdentificationResult.h"

ClassImp(KVIDChIoCsI_camp5)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoCsI_camp5</h2>
<h4>ChIo-CsI id with grids for INDRA_camp5</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoCsI_camp5::KVIDChIoCsI_camp5()
{
   // Default constructor

   fGGgrid = fPGgrid = 0;
   fChIo = 0;
   fCsI = 0;
}

KVIDChIoCsI_camp5::~KVIDChIoCsI_camp5()
{
   // Destructor
}

//___________________________________________________________________________________________

void KVIDChIoCsI_camp5::Initialize()
{
   // Initialize telescope for current run.
   // If there is at least 1 grid, we set fCanIdentify = kTRUE
   // "Natural" line widths are calculated for KVIDZAGrids.

   KVIDChIoCsI::Initialize();
   fCsIRPedestal = fCsI->GetPedestal("R");
   fCsILPedestal = fCsI->GetPedestal("L");
   fGGgrid = fPGgrid = 0;
   TIter next(GetListOfIDGrids());
   KVIDGraph* grid;
   while ((grid = (KVIDGraph*)next())) {
      if (!strcmp(grid->GetVarY(), "CHIO-GG")) fGGgrid = (KVIDZAGrid*)grid;
      else if (!strcmp(grid->GetVarY(), "CHIO-PG")) fPGgrid = (KVIDZAGrid*)grid;
   }
   if (fGGgrid) {
      SetBit(kReadyForID);
      fGGgrid->Initialize();
      if (fPGgrid) fPGgrid->Initialize();
   }
   else ResetBit(kReadyForID);
}


Double_t KVIDChIoCsI_camp5::GetIDMapX(Option_t*)
{
   //Calculates current X coordinate for identification.
   //It is the CsI detector's total light output calculated from current values of 'R' and 'L'
   //raw data without pedestal correction (because identification maps were drawn without
   //correcting).
   //'opt' has no effect.
   Double_t rapide = (Double_t)fCsI->GetR() + fCsIRPedestal;
   Double_t lente = (Double_t)fCsI->GetL() + fCsILPedestal;
   Double_t h = (Double_t)fCsI->GetLumiereTotale(rapide, lente);
   return h;
}

//__________________________________________________________________________//

Double_t KVIDChIoCsI_camp5::GetIDMapY(Option_t* opt)
{
   //Calculates current Y coordinate for identification.
   //It is the ionisation chamber's current grand gain (if opt="GG") or petit gain (opt != "GG")
   //coder data, without pedestal correction.
   Double_t si;
   if (!strcmp(opt, "GG")) {
      si = (Double_t)fChIo->GetGG();
   }
   else {
      si = (Double_t)fChIo->GetPG();
   }
   return si;
}

//________________________________________________________________________________________//

Bool_t KVIDChIoCsI_camp5::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grids.

   //perform identification in ChIo(GG) - CsI(H) map

   idr->SetIDType(GetType());
   idr->IDattempted = kTRUE;

   Double_t cigg = (y < 0. ? GetIDMapY("GG") : y);
   Double_t lumtot = (x < 0. ? GetIDMapX() : x);

   KVIDZAGrid* theIdentifyingGrid = 0;

   fGGgrid->Identify(lumtot, cigg, idr);
   theIdentifyingGrid = (KVIDZAGrid*)fGGgrid;

   if (idr->IDOK && idr->Z == theIdentifyingGrid->GetZmax() && TMath::Nint(GetIDMapY("GG")) == 4095) {
      //Gestion des saturations GG
      //on teste l identification PG
      if (fPGgrid) {
         Double_t cipg = (y < 0. ? GetIDMapY("PG") : y);
         fPGgrid->Identify(lumtot, cipg, idr);
         //on garde l identification PG si celle ci renvoie un code
         //de 0 a 4 ou 7
         if (idr->Zident) {
            theIdentifyingGrid = (KVIDZAGrid*)fPGgrid;
         }
      }
   }
   if (theIdentifyingGrid == fGGgrid) {
      if (fGGgrid->GetQualityCode() > KVIDZAGrid::kICODE6 && fPGgrid) { //we have to try PG grid (if there is one)
         // try Z & A identification in ChIo(PG)-CsI(H) map
         Double_t cipg = (y < 0. ? GetIDMapY("PG") : y);
         fPGgrid->Identify(lumtot, cipg, idr);
         theIdentifyingGrid = (KVIDZAGrid*)fPGgrid;
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
      idr->IDcode = kIDCode5;
      return kTRUE;
   }

   if (theIdentifyingGrid->GetQualityCode() > KVIDZAGrid::kICODE3 &&
         theIdentifyingGrid->GetQualityCode() < KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE4, kICODE5 or kICODE6 then this "nucleus"
      // corresponds to a point which is inbetween the lines, i.e. noise
      idr->IDcode = kIDCode10;
      return kTRUE;
   }

   // set general ID code ChIo-CsI
   idr->IDcode = kIDCode4;
   return kTRUE;
}


