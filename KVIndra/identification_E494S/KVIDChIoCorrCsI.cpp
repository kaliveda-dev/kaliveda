/*
   $Id: KVIDChIoCorrCsI.cpp,v 1.2 2009/04/06 15:21:31 franklan Exp $
   $Revision: 1.2 $
   $Date: 2009/04/06 15:21:31 $
   */

//Created by KVClassFactory on Mon Mar 30 16:44:34 2009
//Author: John Frankland,,,

#include "KVIDChIoCorrCsI.h"
#include "KVINDRACodeMask.h"
#include "KVIdentificationResult.h"
#include "KVIDGChIoSi_e494s.h"

ClassImp(KVIDChIoCorrCsI)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
   <h2>KVIDChIoCorrCsI</h2>
   <h4>ChIo-CsI identification with grids for E503 </h4>
   <!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoCorrCsI::KVIDChIoCorrCsI()
{

}

KVIDChIoCorrCsI::~KVIDChIoCorrCsI()
{

}

//___________________________________________________________________________________________

void KVIDChIoCorrCsI::Initialize()
{
   // Initialize telescope for current run.

   fChIo = 0;
   fChIo = (KVChIo*) GetDetector(1);

   fCsI = 0;
   fCsI = (KVCsI*) GetDetector(2);

   fGrid = 0;
   fGrid = (KVIDZAGrid*) GetIDGrid();

   if (fChIo && fCsI && fGrid) {
      fGrid->Initialize();
      fGrid->SetOnlyZId(kTRUE);
      SetBit(kReadyForID);
   }
   else ResetBit(kReadyForID);

}

//___________________________________________________________________________________________

Double_t KVIDChIoCorrCsI::GetIDMapX(Option_t* opt)
{
   // This method gives the X-coordinate in a 2D identification map
   // associated whith the ChIo-CsI identification telescope.
   // The X-coordinate is the CsI current total light minus.

   IGNORE_UNUSED(opt);
   return fCsI->GetLumiereTotale();
}

//__________________________________________________________________________//

Double_t KVIDChIoCorrCsI::GetIDMapY(Option_t* opt)
{
   // This method gives the Y-coordinate in a 2D identification map
   // associated whith the ChIo-CsI identification telescope.
   // The Y-coordinate is the ionisation chamber's current petit gain coder data minus the petit gain pedestal.

   IGNORE_UNUSED(opt);
   return fChIo->GetPG() - fChIo->GetPedestal("PG");
}

//________________________________________________________________________________________//

Bool_t KVIDChIoCorrCsI::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   idr->SetIDType(GetType());
   idr->IDattempted = kTRUE;

   Double_t chIoCorr = (y < 0. ? GetIDMapY() : y);
   Double_t csiLight = (x < 0. ? GetIDMapX() : x);

   if (fGrid->IsIdentifiable(csiLight, chIoCorr))
      fGrid->Identify(csiLight, chIoCorr, idr);

   if (fGrid->GetQualityCode() == KVIDZAGrid::kICODE8) {
      // only if the final quality code is kICODE8 do we consider that it is
      // worthwhile looking elsewhere. In all other cases, the particle has been
      // "identified", even if we still don't know its Z and/or A (in this case
      // we consider that we have established that they are unknowable).
      idr->IDcode = kIDCode15;
      return kFALSE;
   }

   if (fGrid->GetQualityCode() == KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE7 (above last line in grid) then the estimated
      // Z is only a minimum value (Zmin)
      idr->IDcode = kIDCode5;
      return kTRUE;
   }

   if (fGrid->GetQualityCode() > KVIDZAGrid::kICODE3 && fGrid->GetQualityCode() < KVIDZAGrid::kICODE7) {
      // if the final quality code is kICODE4, kICODE5 or kICODE6 then this "nucleus"
      // corresponds to a point which is inbetween the lines, i.e. noise
      idr->IDcode = kIDCode10;
      return kTRUE;
   }
   if (fGrid->GetQualityCode() == KVIDGChIoSi_e494s::k_BelowSeuilChIo) {

      idr->IDcode = kIDCode15;
      return kTRUE;
   }

   // set general ID code ChIo-CsI
   idr->IDcode = kIDCode4;

   return kTRUE;
}
