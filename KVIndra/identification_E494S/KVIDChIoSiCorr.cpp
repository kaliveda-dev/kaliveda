/*
$Id: KVIDChIoSiCorr.cpp,v 1.2 2009/04/06 15:20:28 franklan Exp $
$Revision: 1.2 $
$Date: 2009/04/06 15:20:28 $
*/

//Created by KVClassFactory on Mon Dec 10 12:58:26 2007
//Author: franklan

#include "KVIDChIoSiCorr.h"
#include "KVINDRACodes.h"

ClassImp(KVIDChIoSiCorr)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoSiCorr</h2>
<h4>INDRA E503/E494s ChIo-Si identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//________________________________________________________________________________________//

Double_t KVIDChIoSiCorr::GetIDMapX(Option_t* opt)
{
   // This method gives the X-coordinate in a 2D identification map
   // associated whith the ChIo-Si identification telescope.
   // The X-coordinate is the silicon current petit gain coder data minus the petit gain pedestal. If the grand gain coder
   // data is less than 3900 then the petit gain value is calculated
   // from the current grand gain coder data (see KVINDRADetector::GetPGFromGG())

   return GetIDMapXY((KVINDRADetector*)fsi, opt);
}
//________________________________________________________________________________________//

Double_t KVIDChIoSiCorr::GetIDMapY(Option_t* opt)
{
   // This method gives the Y-coordinate in a 2D identification map
   // associated whith the ChIo-Si identification telescope.
   // The Y-coordinate is the ionisation chamber's current petit gain coder data minus the petit gain pedestal. If the grand gain coder
   // data is less than 3900 then the petit gain value is calculated
   // from the current grand gain coder data (see KVINDRADetector::GetPGFromGG())
   // We set the pedestal to zero in order to not cut the physics
   // as the pedestal GG seems to be too high.
   // PG-GG conversion has been calculated without ChIo GG pedestal.

   fchio->SetPedestal("GG", 0.);
   return GetIDMapXY((KVINDRADetector*)fchio, opt);
}
//________________________________________________________________________________________//

Double_t KVIDChIoSiCorr::GetIDMapXY(KVINDRADetector* det, Option_t* opt)
{
   IGNORE_UNUSED(opt);

   if (det) {
      if (det->GetGG() < 3900.) return det->GetPGfromGG() - det->GetPedestal("PG");
      return det->GetPG() - det->GetPedestal("PG");
   }
   return 10000.;
}
//________________________________________________________________________________________//

//Bool_t KVIDChIoSiCorr::Identify(KVIdentificationResult *IDR, Double_t x, Double_t y){
//
// Bool_t identOK = KVIDChIoSi::Identify(IDR,x,y);
//
// Int_t quality = ChIoSiGrid->GetQualityCode();
// if(quality==KVIDGChIoSi_e494s::k_BelowSeuilChIo)
//    IDR->IDcode = kIDCode15;
//
// return identOK;
//}
